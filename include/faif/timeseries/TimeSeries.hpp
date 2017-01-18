#ifndef FAIF_TIME_SERIES
#define FAIF_TIME_SERIES

// the file with timeseries tools

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
//msvc8.0 warnings for boost::date_time
#pragma warning(disable:4100)
#pragma warning(disable:4512)
#pragma warning(disable:4127)
#pragma warning(disable:4245)
//msvc8.0 warnings for std::transform
#pragma warning(disable:4996)
#endif

#include <cmath>
#include <ostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <numeric>
#include <functional>

#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace faif {
    /** \brief TimeSeries (collection of triples<timestamp, value, quality>) tools
     */
    namespace timeseries {

        /** \brief the real time type */
        typedef boost::posix_time::ptime RealTime;
        /** \brief the real time duration */
        typedef boost::posix_time::time_duration RealDuration;

        /** \brief convert from RealTime to posix_t (num of seconds from 1970) */
        inline long to_time_t(const RealTime& real_time) {
			boost::posix_time::ptime start(boost::gregorian::date(1970,1,1),boost::posix_time::time_duration(0,0,0));
            return (real_time-start).total_seconds();
        }

        /** \brief digit time type.
            DigitTime < 0 past, DigitTime >= 0 future, DigitTime == 0 now. */
        typedef int DigitTime;
        /** \brief value - real number */
        typedef double Value;
        /** \brief quality - real  0..1 */
        typedef double Quality;

        /** \brief timeseries value, single value in given real time. Plain old data */
        struct TimeValueReal : public boost::tuple<RealTime,Value,Quality> {

            /** c-tor */
            TimeValueReal(RealTime t, const Value& v, const Quality& q = 0.0)
                : boost::tuple<RealTime,Value,Quality>(t,v,q)
            {}
            /** accessor */
            const RealTime& getTime() const { return get<0>(); }
            /** accessor */
            const Value& getValue() const { return get<1>(); }
            /** accessor */
            const Quality& getQuality() const { return get<2>(); }

        };
        /** (for debugging) */
        inline std::ostream& operator<<(std::ostream& os, const TimeValueReal& time_val) {
            os << '(' << time_val.getTime() << ',' << time_val.getValue() << ')';
            return os;
        }


        /** \brief timeseries - time hold as RealTime */
        class TimeSeriesReal : public std::vector<TimeValueReal> {
        public:
            typedef std::vector<TimeValueReal>::iterator iterator;
            typedef std::vector<TimeValueReal>::const_iterator const_iterator;

            /** \brief empty collection */
            TimeSeriesReal(){}

            /** \brief copy c-tor */
            TimeSeriesReal(const TimeSeriesReal& t) : std::vector<TimeValueReal>(t) {}

            /** \brief c-tor from a range */
            TimeSeriesReal(const TimeValueReal* begin, const TimeValueReal* end)
                : std::vector<TimeValueReal>(begin,end) {}
            /** \brief c-tor from a range */
            TimeSeriesReal(const_iterator begin, const_iterator end)
                : std::vector<TimeValueReal>(begin,end) {}

            /** \brief c-tor from a C style table of timestamps (as long - posix time), and C style table of values */
            TimeSeriesReal(const long* time_begin, const long* time_end, const Value* value_begin);

            /** \brief c-tor from C style table of values.
                The timestamps are calculated from start_time and delta */
            TimeSeriesReal(const RealTime& start_time, const RealDuration& delta,
                           const Value* value_begin, const Value* value_end );

            /** \brief c-tor from timeseries - the timestams are modified (the offset is added) */
            TimeSeriesReal(const TimeSeriesReal& ts, const RealTime& offset);
            /** operator = */
            TimeSeriesReal& operator=(const TimeSeriesReal& t){
                std::vector<TimeValueReal>::operator=(t);
                return *this;
            }
            /** d-tor */
            ~TimeSeriesReal(){}

            /** the sum of values of timeseries */
            double getSum() const {
				return std::accumulate( begin(), end(), 0.0, boost::bind( std::plus<double>(), _1, boost::bind(&TimeValueReal::getValue, _2) ) );
			}

            /** the average of values of timeseries, 0.0 if the timeseries is empty. */
            double getAvg() const { return empty() ? 0.0 : getSum()/size(); }

            /** the integral (the area under the line) for timeseries */
            double getIntegral() const;
        };

        /** (for debugging) */
        inline std::ostream& operator<<(std::ostream& os, const TimeSeriesReal& time_series) {
            std::copy( time_series.begin(), time_series.end(), std::ostream_iterator<TimeValueReal>(os," ") );
            return os;
        }

        /** the timeseries value, the time is the offset (DigitTime), plain old data. */
        struct TimeValueDigit : public boost::tuple<DigitTime,Value,Quality> {

            /** c-tor */
            TimeValueDigit(DigitTime t = 0, const Value& v = 0.0, const Quality& q = 0.0)
                : boost::tuple<DigitTime,Value,Quality>(t,v,q)
            {}
            /** accessor */
            const DigitTime& getTime() const { return get<0>(); }
            /** accessor */
            const Value& getValue() const { return get<1>(); }
            /** accessor */
            const Quality& getQuality() const { return get<2>(); }
        };
        /** for debugging */
        inline std::ostream& operator<<(std::ostream& os, const TimeValueDigit& time_val) {
            os << '(' << time_val.getTime() << ',' << time_val.getValue() << ')';
            return os;
        }

        /** the timeseries (collection), time as DigitTime */
        class TimeSeriesDigit : public std::vector<TimeValueDigit> {
        public:
            typedef std::vector<TimeValueDigit>::iterator iterator;
            typedef std::vector<TimeValueDigit>::const_iterator const_iterator;

            /** \brief c-tor, empty timeseries */
            TimeSeriesDigit(){}
            /** \brief c-tor, from range */
            TimeSeriesDigit(const TimeValueDigit* begin, const TimeValueDigit* end)
                : std::vector<TimeValueDigit>(begin,end) {}
            /** \brief c-tor, from range */
            TimeSeriesDigit(const_iterator begin, const_iterator end)
                : std::vector<TimeValueDigit>(begin,end) {}
            /** \brief, c-tor from C style table of values. The timestamps are 0,1,..,n */
            TimeSeriesDigit(const Value* begin, const Value* end);

            /** \brief copy c-tor */
            TimeSeriesDigit(const TimeSeriesDigit& t) : std::vector<TimeValueDigit>(t) {}

            /** \brief c-tor from timeseries, the timestamps are modified, the offset is added */
            TimeSeriesDigit(const TimeSeriesDigit& ts, const DigitTime& offset);

            /** operator = */
            TimeSeriesDigit& operator=(const TimeSeriesDigit& t){
                std::vector<TimeValueDigit>::operator=(t);
                return *this;
            }
            ~TimeSeriesDigit(){}

            /** the autocorelation of timeseries. The average is not substracted.
                out(0) = sum(i=0, i<n) in(i)*in(i)
                out(1) = sum(i=0, i<n-1) in(i)*in(i+1)
                out(2) = sum(i=0, i<n-2) in(i)*in(i+2)
                ...
			*/
            TimeSeriesDigit autoCorrelationE(int scope) const;

            /** the sum of values of timeseries */
            double getSum() const {
                return std::accumulate( begin(), end(), 0.0,
                                        boost::bind( std::plus<double>(), _1, boost::bind(&TimeValueDigit::getValue, _2) ) );
            }

            /** the sum of square of values of timeseries */
            double getSumSquared() const {
                return std::accumulate( begin(), end(), 0.0,
                                        boost::bind( std::plus<double>(), _1,
                                                     boost::bind( std::multiplies<double>(),
                                                                  boost::bind(&TimeValueDigit::getValue, _2),
                                                                  boost::bind(&TimeValueDigit::getValue, _2) )
                                                     )
                                        );
            }

            /** the average of values of timeseries or 0.0 if the timeseries is empty */
            double getAvg() const { return empty() ? 0.0 : getSum()/size(); }

            /** the square of variation */
            double getSigmaSquared() const { return size()*getSumSquared() - getSum()*getSum(); }

            /** the variation (slower calculation) */
            double getSigma() const { return empty()? 0.0 : std::sqrt( getSigmaSquared() )/ size(); }

            //      TimeSeriesDigit operator+(const TimeSeriesDigit& ts) const;

        };


        /** the average of difference for two timeserieses */
        inline double getAvgAbsDiff(const TimeSeriesDigit& ts1, const TimeSeriesDigit& ts2) {

			typedef TimeSeriesDigit::const_iterator TSIter;

            TSIter it1 = ts1.begin();
            TSIter it2 = ts2.begin();

            double sum_abs = 0.0;
            int count = 0;
            //przynajmniej jeden do rozpatrzenia
            while( it1 != ts1.end() || it2 != ts2.end() ) {
                ++count;
                //druga seria juz rozpatrzona lub wyraz w it1 istnieje i jest mlodszy niz w it2
                if(it2 == ts2.end() || (it1 != ts1.end() && it1->getTime() < it2->getTime() ) ) {
                    sum_abs += std::abs(it1->getValue() );
                    ++it1;
                } else if(it1 == ts1.end() || it2->getTime() < it1->getTime() ) { //tutaj it2 != end
                    sum_abs += std::abs(it2->getValue() );
                    ++it2;
                } else  { //tutaj it1 != end() oraz it2 != end() oraz it1->getTime() == it2->getTime()
                    sum_abs += std::abs(it1->getValue() - it2->getValue() );
                    ++it1;
                    ++it2;
                }
            }
            return count > 0 ? sum_abs/(double)count : 0.0;
        }

        /** the average of difference (relative) for two timeserieses */
        inline double getAvgRelDiff(const TimeSeriesDigit& ts1, const TimeSeriesDigit& ts2) {

            typedef TimeSeriesDigit::const_iterator TSIter;

            TSIter it1 = ts1.begin();
            TSIter it2 = ts2.begin();

            double sum_rel = 0.0;
            int count = 0;
            //przynajmniej jeden do rozpatrzenia
            while( it1 != ts1.end() || it2 != ts2.end() ) {
                //druga seria juz rozpatrzona lub wyraz w it1 istnieje i jest mlodszy niz w it2
                if(it2 == ts2.end() || (it1 != ts1.end() && it1->getTime() < it2->getTime() ) ) {
                    ++count;
                    sum_rel += 1.0;
                    ++it1;
                } else if(it1 == ts1.end() || it2->getTime() < it1->getTime() ) { //tutaj it2 != end
                    ++count;
                    sum_rel += 1.0;
                    ++it2;
                } else  { //tutaj it1 != end() oraz it2 != end() oraz it1->getTime() == it2->getTime()
                    double m = ( it1->getValue() + it2->getValue() ) / 2.0;
                    //pomija 'smieci', ale uwzglednia NaN
                    if( ( m > (2 * std::numeric_limits<double>::epsilon() ) )
                        || ( m != m ) ) {
                        ++count;
                        sum_rel += std::abs( (it1->getValue() - it2->getValue() )/ m );
                    }
                    ++it1;
                    ++it2;
                }
            }
            return count > 0 ? sum_rel/(double)count : 0.0;
        }

        /** for debugging */
        inline std::ostream& operator<<(std::ostream& os, const TimeSeriesDigit& time_series) {
            std::copy( time_series.begin(), time_series.end(), std::ostream_iterator<TimeValueDigit>(os," ") );
            return os;
        }

		// ---------------------------------------------------------------------------------------------------------
		//
		//  TimeSeriesReal - implementation
		//
		// ---------------------------------------------------------------------------------------------------------

        namespace {

            struct FromTimestampValueGenerator {

                TimeValueReal operator()(const long& time, const double& value) {
                    return TimeValueReal( boost::posix_time::from_time_t(time), value );
                }

            };

            //generator kolejnych timestamp-ow
            struct FromValueGenerator {
                FromValueGenerator(const RealTime& t, const RealDuration& d)
                    : curr_(t), delta_(d) {}

                //generuje kolejna probke czasu
                TimeValueReal operator()(const double& value) {
                    RealTime out = curr_;
                    curr_ += delta_;
                    return TimeValueReal(out, value);
                }
                RealTime curr_;
                RealDuration delta_;
            };
        } //namespace


		/** tworzy szereg czasowy na podstawie tablicy timestamp (posix) oraz tablicy wartosci */
		inline TimeSeriesReal::TimeSeriesReal(const long* begin, const long* end, const Value* input2) {
			FromTimestampValueGenerator gen;
			std::transform( begin, end, input2, back_inserter(*this), gen );
		}

		/** tworzy szereg czasowy na podstawie tablicy wartosci, czas dla pierwszego pomiaru, delta */
		inline TimeSeriesReal::TimeSeriesReal(const RealTime& start_time, const RealDuration& delta,
											  const Value* value_begin, const Value* value_end ) {

			FromValueGenerator gen(start_time, delta);
			std::transform( value_begin, value_end, back_inserter(*this), gen );

		}

		namespace {
			typedef std::vector<TimeValueReal>::const_iterator RealConstIter;

			//the trapezium square equation
			double trapeziumSquare(double a, double b, double h) {
				return (a+b)*h/2.0;
			}

			//the it+1 must be valid
			double trapeziumSquare(const std::vector<TimeValueReal>::const_iterator it) {
				RealConstIter it_next(it);
				++it_next;
				long len_sec = boost::posix_time::time_period(it_next->getTime(), it->getTime()).length().total_seconds();
				return trapeziumSquare(it->getValue(), it_next->getValue(), std::abs(len_sec) );
			}
		}

		/** oblicza calke (pole pod krzywa) dla danego szeregu czasowego */
		inline double TimeSeriesReal::getIntegral() const {
			if( size() < 2)
				return 0.0;

			std::vector<TimeValueReal>::const_iterator last = end();
			--last; //mozna, bo size() >= 2
			double sum = 0.0;
			for(RealConstIter it = begin(); it != last; ++it ) {
				sum += trapeziumSquare( it );
			}
			return sum;
		}


		// ---------------------------------------------------------------------------------------------------------
		//
		//  TimeSeriesDigit - implementation
		//
		// ---------------------------------------------------------------------------------------------------------


        namespace {
            /** helping function, moves the time by the given offset */
            inline TimeValueDigit moveFun(const TimeValueDigit& v, const DigitTime& offset) {
                return TimeValueDigit(v.getTime() + offset, v.getValue(), v.getQuality() );
            }

            /** helping functor, create TimeValueDigit for next points with given quality and for table of values */
            struct NextStepFunctor {

                NextStepFunctor() : curr_(-1) { }
                TimeValueDigit operator()(const Value& v) { return TimeValueDigit(++curr_, v); }
                DigitTime curr_;
            };

            typedef TimeSeriesDigit::const_iterator TSIter;
            //helping functor - scalar multiplication two time stamp collections (with offset)
            struct CalcCorrelationFunctor {
                CalcCorrelationFunctor(const TimeSeriesDigit& ts1, const TimeSeriesDigit& ts2,
                                       double avg1 = 0.0, double avg2 = 0.0, double sig1 = 1.0, double sig2 = 1.0)
                    : ts1_(ts1), ts2_(ts2), offset_(0), avg1_(avg1), avg2_(avg2), sig1_(sig1), sig2_(sig2)
                { }

                //the scalar multiplication for given offset
                TimeValueDigit operator()() {
                    Value val = 0.0;
                    TSIter it1 = ts1_.begin(); //znajduje pierwszy czas, ktory jest i tu i tu
                    TSIter it2 = ts2_.begin();
                    while( it1 != ts1_.end() && it2 != ts2_.end() && it1->getTime() != (it2->getTime() + offset_) ) {
                        if( it1->getTime() < (it2->getTime() + offset_) ) ++it1;
                        else ++it2;
                    }
                    int count = 0;
                    //teraz ustawione tak ze t0 = max( t1, t2 + offset )
                    for(; it1 != ts1_.end() && it2 != ts2_.end(); ++it1, ++it2 ) {
                        val += it1->getValue() * it2->getValue();
                        ++count;
                    }

                    val -= count * avg1_* avg2_;
                    if(count > 1)
                        val /= (count - 1)* sig1_ * sig2_;
                    else
                        val /= sig1_ * sig2_;
                    TimeValueDigit out(offset_, val);
                    ++offset_;
                    return out;
                }
                const TimeSeriesDigit& ts1_;
                const TimeSeriesDigit& ts2_;
                DigitTime offset_;
                double avg1_;
                double avg2_;
                double sig1_;
                double sig2_;
            };

        } //namespace

		/** \brief, c-tor from C style table of values. The timestamps are 0,1,..,n */
        inline TimeSeriesDigit::TimeSeriesDigit(const Value* begin, const Value* end) {
            NextStepFunctor functor;
			std::transform( begin, end, std::back_inserter(*this), functor );

        }

		/** \brief c-tor from timeseries, the timestamps are modified, the offset is added */
        inline TimeSeriesDigit::TimeSeriesDigit(const TimeSeriesDigit& ts, const DigitTime& offset) {
            reserve(ts.size());
			std::transform( ts.begin(), ts.end(), std::back_inserter(*this), boost::bind(&moveFun, _1, offset ) );
        }

		/** the autocorelation of timeseries. The average is not substracted.
			out(0) = sum(i=0, i<n) in(i)*in(i)
			out(1) = sum(i=0, i<n-1) in(i)*in(i+1)
			out(2) = sum(i=0, i<n-2) in(i)*in(i+2)
			...
		*/
        inline TimeSeriesDigit TimeSeriesDigit::autoCorrelationE(int scope) const {
            TimeSeriesDigit out;
            CalcCorrelationFunctor gen(*this, *this);
			std::generate_n(back_inserter(out), scope, gen );
            return out;
        }

        /** the correlation of timeserieses in range 0..scope */
        inline TimeSeriesDigit correlation(const TimeSeriesDigit& ts1, const TimeSeriesDigit& ts2, int scope) {
            TimeSeriesDigit out;
            CalcCorrelationFunctor gen(ts1, ts2, ts1.getAvg(), ts2.getAvg(), ts1.getSigma(), ts2.getSigma() );
            generate_n(back_inserter(out), scope, gen );
            return out;
        }


    } //namespace timeseries
} //namespace faif

#endif //FAIF_TIME_SERIES
