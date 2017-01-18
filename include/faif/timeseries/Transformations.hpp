#ifndef FAIF_TS_TRANSFORMATIONS
#define FAIF_TS_TRANSFORMATIONS

// file with transformations -> changest real time series into digit one and digit time series into real one

#include <ostream>
#include <algorithm>
#include <boost/bind.hpp>

#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/construct.hpp>

#include "TimeSeries.hpp"

namespace faif {
    namespace timeseries {

        /** \brief transformation - class to change timeseries stored in TimeSeriesDigit and/or TimeSeriesReal
         */
        class Transformation {
        public:
            Transformation(const RealTime& present, const RealDuration& delta)
                : present_(present), delta_(delta) {}
            Transformation(const Transformation& t)
                : present_(t.present_), delta_(t.delta_) {}
            ~Transformation() {}

            Transformation& operator=(const Transformation& t) {
                present_ = t.present_;
                delta_ = t.delta_;
                return *this;
            }

            /** accessor - reference point in time */
            const RealTime& getPresent() const { return present_; }
            /** accessor - distance between probes */
            const RealDuration& getDelta() const { return delta_; }

            /** transform digit time to real time (gives the reference points in time) */
            RealTime toReal(const DigitTime& d) const {
                return RealTime( present_ + delta_ * d);
            }

            /** transform real time to digit time
                D = toDigit(R), then
                toReal(D) in <R - delta/2;R + delta/2)
                -- includes R - delta/2
                -- excludes R + delta/2

				example: delta=4

				-2  -1  0   1   2
				|---|---|---|---|---
				-2111100001111222233
			*/
            DigitTime toDigit(const RealTime& r) const {
				if(delta_.total_seconds() != 0) {
					//accuracy of the sec
					long delta_2 = delta_.total_seconds() / 2;
					RealDuration dur = boost::posix_time::time_period(present_, r).length();
					long duration_sec = dur.total_seconds() + delta_2;
					int correction_for_minus = ( (duration_sec >= 0) ? 0 : 1 );
					long out = (duration_sec + correction_for_minus) / delta_.total_seconds() - correction_for_minus;
					return out;
				} else {
					//accuracy of the millisec
					long delta_2 = static_cast<long>( delta_.total_milliseconds() / 2 ); //TODO: use long long or __int64 when switch to C++0x
					RealDuration dur = boost::posix_time::time_period(present_, r).length();
					long duration_sec = static_cast<long>(dur.total_milliseconds() + delta_2); //TODO: use long long or __int64 when switch to C++0x
					int correction_for_minus = ( (duration_sec >= 0) ? 0 : 1 );
					long out = (duration_sec + correction_for_minus) / static_cast<long>(delta_.total_milliseconds() ) - correction_for_minus;
					return out;
				}

            }
        private:
            RealTime present_;
            RealDuration delta_;
        };


        namespace {

            /** function to linear approximation. Given two points (start and end).
                Calculates all middle points, and add it to collection.
			*/
            void linearApproximate(DigitTime start_time, DigitTime end_time,
								   Value start_value, Value end_value,
								   Quality start_quality, Quality end_quality,
								   TimeSeriesDigit& out_ts) {

                for(DigitTime t = start_time + 1; t < end_time; ++t ) {

                    DigitTime proportion_A = end_time - t;
                    DigitTime proportion_B = t - start_time;
                    DigitTime proportion_weight = end_time - start_time;

                    Value v = (start_value * proportion_A + end_value * proportion_B) / proportion_weight;
                    Quality q = (start_quality * proportion_A + end_quality * proportion_B) / proportion_weight;
                    out_ts.push_back( TimeValueDigit(t, v, q) );
                }

            }


            /** counted time value digit - to agregate many probes */
            struct CountedTimeValue {
                CountedTimeValue(const DigitTime& time, const TimeValueReal& tv)
                    : time_(time), value_(tv.getValue()), quality_(tv.getQuality()), counter_(1)
                {}

                /** add another time value with the same time (aggregate) */
                void append(const TimeValueReal& tv) {
                    //modyfikuje wartosc
                    value_ += tv.getValue();
                    quality_ += tv.getQuality();
                    ++counter_;
                }
                /** accessor */
                const DigitTime& getTime() const { return time_; }

                /** return the aggregated time value */
                TimeValueDigit get() { return TimeValueDigit(time_, value_/counter_, quality_/counter_); }
            private:
                DigitTime time_;
                Value value_;
                Quality quality_;
                int counter_;
            };

            /** class to transform real time series into digit one */
            class LinearResampler : boost::noncopyable {
            public:
                LinearResampler(const Transformation& trans, const TimeValueReal& first)
                    : trans_(trans),
                      current_(trans_.toDigit(first.getTime()), first ) {}
                ~LinearResampler() {}

                /** method called when the given time-value is added to result time-series */
                void addTimeValue(const TimeValueReal& tv) {
                    DigitTime act_time = trans_.toDigit( tv.getTime() );
					//cout << tv.getTime() << ' ' << act_time << ' ' << tv.getValue() << endl;
                    if(act_time == current_.getTime() )
                        current_.append(tv);
                    else {
                        addCurrentValue();
                        //check if linear approximation is necessary
                        if(act_time > current_.getTime() + 1) {
                            const TimeValueDigit& last_tv = ts_.back();
                            //linear approximation:
                            linearApproximate( last_tv.getTime(), act_time,
                                               last_tv.getValue(), tv.getValue(),
                                               last_tv.getQuality(), tv.getQuality(),
                                               ts_ );
                        }
                        current_ = CountedTimeValue( act_time, tv );
                    }
                }

                /** method called when the last time value could be calculated */
                void finish() { addCurrentValue(); }

                /** returns the calculated time series */
                const TimeSeriesDigit& getTimeSeries() { return ts_; }
            private:
                void addCurrentValue() { ts_.push_back( current_.get() ); }
                const Transformation& trans_;
                CountedTimeValue current_;
                TimeSeriesDigit ts_;
            };
        } //namespace


        /** creates digit time series from real time series.
            Linear resampling: arythmetic everage for aggregation,
            the linear approxymation for missing data.
        */
        inline TimeSeriesDigit create(const TimeSeriesReal& in, const Transformation& transformation) {
            if( in.empty() )
                return TimeSeriesDigit();
            TimeSeriesReal::const_iterator it = in.begin();
            //initialize with first time value
            LinearResampler resampler(transformation, *it);
            ++it;
			std::for_each( it, in.end(), boost::bind(&LinearResampler::addTimeValue, boost::ref(resampler), _1 ) );
            resampler.finish();
            return resampler.getTimeSeries();
		}

        /** creates real time series from digit time series.
		 */
        inline TimeSeriesReal create(const TimeSeriesDigit& in, const Transformation& transformation) {
			TimeSeriesReal out;
			std::transform( in.begin(), in.end(), back_inserter(out),
							boost::lambda::bind( boost::lambda::constructor<TimeValueReal>(),
												 boost::lambda::bind(&Transformation::toReal, boost::ref(transformation),
																	 boost::lambda::bind(&TimeValueDigit::getTime, boost::lambda::_1) ),
												 boost::lambda::bind(&TimeValueDigit::getValue, boost::lambda::_1 ),
												 boost::lambda::bind(&TimeValueDigit::getQuality, boost::lambda::_1 ) ) );
			//						 bind(&createTV, _1, transformation) );
			return out;
		}

    } //namespace timeseries
} //namespace faif

#endif //FAIF_TS_TRANSFORMATIONS
