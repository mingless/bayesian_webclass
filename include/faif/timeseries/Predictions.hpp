#ifndef FAIF_TS_PREDICTIONS
#define FAIF_TS_PREDICTIONS

// file with predictions -> calculates the digit time series based on history

#include <ostream>
#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/tuple/tuple.hpp>

#include "TimeSeries.hpp"

namespace faif {
    namespace timeseries {

        /* forward declaration */
        class PredictionVisitor;

        /** base class to comp block */
        class Prediction : boost::noncopyable {
        public:
            Prediction(const TimeSeriesDigit& history) : history_(history), default_(0, 0.0) { }

            virtual ~Prediction(){ }

            /** accessor */
            const TimeSeriesDigit& getHistory() const { return history_; }

            /** the visitor pattern for Prediction hierarchy */
            virtual void accept(PredictionVisitor& v) const = 0;

            /** find the value for t (the lower_bound) */
            const TimeValueDigit& getHistoricalValue(DigitTime t) const {
                if( history_.size() == 0)
                    return default_;

                //TODO: there is workaroud of msvc 9.0 debug (no lower_bound template with different types)
                TimeValueDigit v(t,0.0); //TODO: temporary object, workaround of bug in msvc 9.0 debug

                TimeSeriesDigit::const_iterator it =
                    std::lower_bound( history_.begin(), history_.end(), v,
                                      boost::bind(&TimeValueDigit::getTime, _1) < boost::bind(&TimeValueDigit::getTime, _2) );

                if( it != history_.end() ) {
                    return *it;
                }
                else {
                    return history_.back(); //history is not empty (it was checked before)
                }

            }

            /** calculate the prediction for period <from, to>,
                values for negative timestamp are readed from the history
            */
            TimeSeriesDigit calculatePrediction(DigitTime from, DigitTime to) {
                if(from > to)
                    throw PredictionRangeException(from, to); //bad range

                TimeSeriesDigit out;

                DigitTime history_last = std::min(to + 1, 0);

                //copy history (if needed)
                for(DigitTime t = from; t < history_last; ++t)
                    out.push_back( getHistoricalValue(t) );

                //calculate prediction (if needed)
                if(to >= 0) {
                    TimeSeriesDigit pred = doCalcPrediction(to);
                    TimeSeriesDigit::const_iterator first = pred.begin();
                    if(from > 0)
                        advance(first, from);
                    for(;first != pred.end(); ++first)
                        out.push_back( *first );
                }
                return out;
            }
        private:
            TimeSeriesDigit history_; // \brief historic data
            TimeValueDigit default_; //default value

            /** calculate the prediction for f(t), f(t+1), ... f(t+n), calculates colection of probes.
                It isa always prediction, because the period is <0,n> */
            virtual TimeSeriesDigit doCalcPrediction(DigitTime n) = 0;
        };

        /* forward declaration */
        class PredictionAR;
        /* forward declaration */
        class PredictionKNN;

        /** base visitor of prediction hierarchy */
        class PredictionVisitor {
        public:
            virtual void visit(const PredictionAR& v) = 0;
            virtual void visit(const PredictionKNN& v) = 0;
        };


        /**
           \brief the AR parameter collection
           f(t) = a[0]f(t-1) + a[1]f(t-2) + ... a[n-1]f(t-n)
        */
        typedef std::vector<double> ARDef;

        /**
           \brief the KNN parameters, k = num_neighbours, ref_size = size of reference block
        */
        struct KNNDef : public boost::tuple<int,size_t> {

            /** c-tor */
            KNNDef(int k, size_t ref_size) : boost::tuple<int,size_t>(k, ref_size) {}
            /** accessor */
            int getK() const { return get<0>(); }
            /** accessor */
            size_t getRefSize() const { return get<1>(); }
        };


        /** the auto-regressive (AR) computation block

            f(t+1) = a(t)f(t) + a(t-1)f(t-1) + ... + a(t-n+1)f(t-n+1)
            or
            f(t+1) = a[0]f(t) + a[1]f(t-1) + ... a[n-1]f(t-n+1)
        */
        class PredictionAR : public Prediction {
        public:
            /** c-tor */
            PredictionAR(const TimeSeriesDigit& history, const ARDef& definition)
                : Prediction(history), definition_(definition)
            { }

            /** d-tor */
            virtual ~PredictionAR() {}

            /** visitor pattern */
            virtual void accept(PredictionVisitor& v) const { v.visit(*this); }

            /** accessor */
            const ARDef& getARDef() const { return definition_; }
        private:
            ARDef definition_; //opis modelu auto-regresywnego (wspolczynniki)
            TimeSeriesDigit predictions_; //the prediction

            /** return the TimeValue for time t. The ranges are not checked (if bad -- assertion) */
            const TimeValueDigit& get(DigitTime t) const {
                if(t < 0) {//find in history
                    return getHistoricalValue(t);
                }
                else { //return from prediction
                    assert( t < static_cast<int>(predictions_.size()) );
                    return predictions_[t];
                }
            }

            /** prediction calculated by AR (n >= 0) */
            TimeValueDigit calculateAR(DigitTime time) const {
                Value val = 0.0;
                for(DigitTime i=0;i<static_cast<int>(definition_.size());++i) {
                    val += definition_[i] * get(time - i - 1).getValue();
                }
                //cout << "calculate AR t=" << time << " val = " << val << endl;
                return TimeValueDigit(time, val);
            }

            /** calculate the prediction for f(t), f(t+1), ... f(t+n), calculates colection of probes  */
            virtual TimeSeriesDigit doCalcPrediction(DigitTime n) {
                DigitTime last_predicted = -1;
                if( ! predictions_.empty() )
                    last_predicted = predictions_.back().getTime();

                if(n > last_predicted) { //check if the prediction should be calculated
                    for(DigitTime t = last_predicted + 1; t <= n; ++t ) {
                        //LOG4CXX_INFO( logger(), "AR::calcPrediction t = " << t << " pred " << calculateAR(t) );
                        predictions_.push_back( calculateAR(t) );
                    }
                }
                return predictions_;
            }
        };

        /** to debug */
        inline std::ostream& operator<<(std::ostream& os, const PredictionAR& ar) {
            os << "AR:";
            std::copy(ar.getARDef().begin(), ar.getARDef().end(), std::ostream_iterator<double>(os,",") );
            os << ';';
            return os;
        }

        namespace {

            typedef TimeSeriesDigit::const_iterator TSIter;

            //the distance between two time value digit elements of the same timestamp
            Value element_distance(const TimeValueDigit& a, const TimeValueDigit& b) {
                //cout << "element distance " << a.getValue() << "," << b.getValue() << endl;
                return std::abs(a.getValue() - b.getValue() );
            }

            // the sum of element_distances, distance between two timeseries
            Value series_distance(TSIter begin1, TSIter end1, TSIter begin2, TSIter end2) {
                if( distance(begin1, end1) > distance(begin2, end2) ) {
                    //cout << "dist " << distance(begin1, end1) << " dist " << distance(begin2, end2) << endl;
                    return 0.0;
                }
                //cout << "dist beg1:" << begin1->getValue() << " beg2: " << begin2->getValue() << endl;
                //cout << "size1 " << distance(begin1,end1) << " size2 " << distance(begin2,end2) << endl;
                return std::inner_product( begin1, end1, begin2, 0.0, std::plus<double>(), boost::bind(&element_distance, _1, _2 ) );
            }

            struct BestRangesFunctor : boost::noncopyable {
                //initializes the associate table to the best offsets
                BestRangesFunctor(TSIter begin, TSIter end, TSIter last_begin,
                                  const TimeSeriesDigit& reference,
                                  int num_neighbours)

                    : NUM_NEIGHBOURS(num_neighbours)
                {
                    for(; begin != last_begin; ++begin) {
                        Value dist = series_distance( reference.begin(), reference.end(), begin, end );
                        add( dist, begin );
                    }
                }
                //adds the new element to collection (if it is good enough)
                void add(Value dist, TSIter it) {
                    //LOG4CXX_INFO( logger(), "add dist= " << dist << " for time=" << it->getTime() );
                    best_.insert( make_pair(dist, it) );
                    if( best_.size() > NUM_NEIGHBOURS )
                        best_.erase(--best_.end() );
                }

                Value avg(DigitTime offset, int reference_length, TSIter last_iter) const {
                    Value v = 0.0;
                    for(BestIter ii = best_.begin(); ii != best_.end(); ++ii ) {
                        TSIter it_best = ii->second;
                        if( distance(it_best, last_iter) > (reference_length + offset) ) {
                            advance(it_best, reference_length + offset );
                            v += it_best->getValue();
                        }
                    }
                    if(!best_.empty())
                        return v/best_.size();
                    else
                        return 0.0;

                }

                const unsigned int NUM_NEIGHBOURS; //size of collection

                //collection: distance(value) and the iterator to the begin of block
                std::multimap<Value,TSIter> best_;
                typedef std::multimap<Value,TSIter>::const_iterator BestIter;
            };

            inline std::ostream& operator<<(std::ostream& os, const BestRangesFunctor& b) {
                for( BestRangesFunctor::BestIter ii = b.best_.begin(); ii != b.best_.end(); ++ii ) {
                    os << (ii->second)->getTime() << ',';
                }
                return os;
            }

        } //namespace

        /** memory based predictor */
        class PredictionKNN : public Prediction {
        public:
            /* c-tor */
            PredictionKNN(const TimeSeriesDigit& history, const KNNDef& definition)
                : Prediction(history), definition_(definition)
            {
                const TimeSeriesDigit& historical = getHistory();
                TimeSeriesDigit::const_iterator reference_begin = historical.end();
                TimeSeriesDigit::const_iterator reference_end  = historical.end();

                int ref_size = static_cast<int>(definition.getRefSize() );

                //set the begin iterator to first available for reference block
                if( distance(historical.begin(), historical.end() ) > ref_size )
                    std::advance( reference_begin, - ref_size);
                else
                    reference_begin = historical.begin();

                reference_ = TimeSeriesDigit(reference_begin, reference_end);
            }
            /* d-tor */
            virtual ~PredictionKNN() {}
            /** accessor */
            const KNNDef& getKNNDef() const { return definition_; }

            // \brief visitor pattern
            virtual void accept(PredictionVisitor& v) const { return v.visit(*this); }
        private:
            KNNDef definition_; //KNN definition
            TimeSeriesDigit reference_; //KNN reference block
            TimeSeriesDigit predictions_; //the prediction

            /** return the TimeValue for time t. The ranges are not checked (if bad -- assertion) */
            const TimeValueDigit& get(DigitTime t) const {
                if(t < 0)
                    return getHistoricalValue(t);
                else {
                    assert( t < static_cast<int>(predictions_.size()) );
                    return predictions_[t];
                }
            }

            /** calculate the prediction for f(t), f(t+1), ... f(t+n), calculates colection of probes  */
            virtual TimeSeriesDigit doCalcPrediction(DigitTime n) {
                                //std::cout << " reference: " << reference_ << std::endl;
                                //LOG4CXX_INFO( logger(), "KNN::calcPredictions n=" << n);
                                predictions_.clear();

                                // dostarcza danych historycznych, ktore sa porownywane blokiem referencyjnym
                                const TimeSeriesDigit& ts = getHistory();

                                //musi miec wielkosc bloku (reference_.size) oraz jeszcze przynajmniej jeden pomiar
                                int min_hist_size = static_cast<int>( reference_.size() ) + 1;
                                TSIter last_begin = ts.end();
                                if(min_hist_size < static_cast<int>( ts.size() ) )
                                        advance(last_begin, -min_hist_size);
                                else
                                        last_begin = ts.begin();

                                //          LOG4CXX_INFO( logger(), "distance (begin, last_begin) " << distance(ts.begin(), last_begin ) );
                                //          LOG4CXX_INFO( logger(), "distance (last_begin, end) " << distance(last_begin, ts.end() ) );

                                BestRangesFunctor best(ts.begin(), ts.end(), last_begin, reference_, definition_.getK() );
                                //std::cout << " best " << best << std::endl;

                                for(DigitTime t = 0; t <= n; ++t ) {
                                        Value v = best.avg(t, static_cast<int>(definition_.getRefSize()), ts.end() );
                                        //std::cout << " result: t=" << t << " v=" << v << std::endl;
                                        //LOG4CXX_INFO( logger(), "result: t = " << t << " v = " << v );
                                        predictions_.push_back( TimeValueDigit(t,v) );
                                }
                                return predictions_;
            }
        };

        /** to debugging */
        inline std::ostream& operator<<(std::ostream& os, const PredictionKNN& knn) {
            os << "KNN: k=" << knn.getKNNDef().getK() << " ref_size= " << knn.getKNNDef().getRefSize() << ';';
            return os;
        }

        namespace {
            // \brief print visitor - used in print operator
            struct PrintVisitor : public PredictionVisitor {
                PrintVisitor(std::ostream& os) : os_(os) {}
                virtual void visit(const PredictionAR& v) { os_  << v; }
                virtual void visit(const PredictionKNN& v) { os_ << v; }
                std::ostream& os_;
            };

        }//namespace

        //ostream operator (to debug)
        inline std::ostream& operator<<(std::ostream& os, const Prediction& pred) {
            PrintVisitor v(os);
            pred.accept(v);
            return os;
        }



    } //namespace timeseries
} //namespace faif


#endif //FAIF_TS_PREDICTIONS
