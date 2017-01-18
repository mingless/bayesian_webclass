#ifndef RANDOM_CUSTOM_DISTR_H
#define RANDOM_CUSTOM_DISTR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
//msvc9.0 warning, rubbish for boost::random
#pragma warning(disable:4512)
//msvc9.0 waring for boost::math
#pragma warning(disable:4127)
#endif

#include <cmath>
#include <list>
#include <vector>
#include <map>
#include <algorithm>
#include <iterator>
#include <numeric>
#include <limits>
#include <ostream>

#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/bind.hpp>
#include <boost/math/distributions/normal.hpp>


#include "Random.hpp"

namespace faif {


    /** the value in histogram, part of distribution (single range) */
    class DistrValue {
    public:
        /** \brief c-tor */
        DistrValue(double from, double to, double value)
            : impl_( (std::min)(from, to), (std::max)(from, to), value)
        {}

        /** \brief copy c-tor */
        DistrValue(const DistrValue& v) : impl_(v.impl_) {}
        /** \brief assignment */
        DistrValue& operator=(const DistrValue& v) {
            impl_ = v.impl_;
            return *this;
        }
        /** \brief d-tor */
        ~DistrValue() {}
        /** accessor */
        double getFrom() const { return boost::get<0>(impl_); }
        /** accessor */
        double getTo() const { return boost::get<1>(impl_); }
        /** accessor */
        double getValue() const { return boost::get<2>(impl_); }
        /** mutator */
        void setValue(double val) { boost::get<2>(impl_) = val; }

        //comparison
        bool operator==(const DistrValue& v) const { return impl_ == v.impl_; }

        //comparison - to sort ranges
        bool operator<(const DistrValue& v) const {
            if(getFrom() == v.getFrom() )
                return getTo() < v.getTo();
            return getFrom() < v.getFrom();
        }

    private:
        boost::tuple<double, double, double> impl_;
    };

    //stream operator (to debug)
    inline std::ostream& operator<<(std::ostream& os, const DistrValue& v) {
        os << "[" << v.getFrom() << ", " << v.getTo() << "]:" << v.getValue();
        return os;
    }

    /** \brief collection of histogram values, data for histogram distribution */
    typedef std::vector<DistrValue> DistrValues;

    namespace {

        //the lenght of histogram value
        double getLength(const DistrValue& v) { return v.getTo() - v.getFrom(); }
        //the maximum value (because histogram value is a part of probability density)
        double getMaximumValue(const DistrValue& v) { return 1./getLength(v); }
        //the area of range
        double getArea(const DistrValue& v) { return getLength(v)*v.getValue(); }
        //the arithmetic mean of range
        double getMeanTerm(const DistrValue& v) {
            return 0.5*(v.getFrom() + v.getTo())*getArea(v);
        }
        //the term to calcualte variation
        double getVariationTerm(const DistrValue& v, const double mi) {
            double a = v.getFrom() - mi;
            double b = v.getTo() - mi;
            return v.getValue()* ( pow(b, 3.0) - pow(a, 3.0) )/ 3.0;
        }
    }

    /** modify the values in vector of ranges to make the integral = 1,
        the ranges are sorted, dense (no gaps), no overlapped (no common parts)
    */
    inline DistrValues makeProbabilityDensity(const DistrValues& in) {
        typedef std::list<DistrValue> Val;
        Val val(in.begin(), in.end() );
        val.sort();

        //remove gaps and overllaped ranges
        Val::iterator curr = val.begin();
        Val::iterator next = curr;
        if( next != val.end() )
            ++next;
        while( next != val.end() ) {
            if(next->getFrom() < curr->getTo() ) { //overlapped ranges
                if( next->getTo() <= curr->getTo() ) {
                    //remove the next period
                    Val::iterator it = next;
                    ++next;
                    val.erase(it);
                    continue;
                }
                else {
                    //change the from parameter for the next period
                    *next = DistrValue( curr->getTo(), next->getTo(), next->getValue() );
                }
            }
            else if(next->getFrom() > curr->getTo() ) { //gap
                val.insert(next, DistrValue( curr->getTo(), next->getFrom(), 0.0 ) );
            }
            //accept
            curr = next;
            ++next;
        }
        double sum = std::accumulate( val.begin(), val.end(), 0.0,
                                      boost::bind( std::plus<double>(), _1,
                                                   boost::bind(&getArea, _2) ) );

        if(sum == 0.0) sum = 1.0;

        //modify values
        for(Val::iterator i = val.begin(); i != val.end(); ++i ) {
            i->setValue( i->getValue() / sum );
        }
        return DistrValues( val.begin(), val.end() );
    }

    //stream operator (to debug)
    inline std::ostream& operator<<(std::ostream& os, const DistrValues& v) {
        std::copy(v.begin(), v.end(), std::ostream_iterator<DistrValue>(os,"; ") );
        return os;
    }

    /** \brief the distribution described by histogram (sum of ranges) */
    class RandomCustomDistr {
    public:
        RandomCustomDistr() :  gen_()  {
            values_.push_back( DistrValue(0.0, 1.0, 1.0) );
        }
        RandomCustomDistr( const DistrValues& values )
            : values_( makeProbabilityDensity(values) ), gen_()
        { }

        /** assignment */
        RandomCustomDistr& operator=(const RandomCustomDistr& h) {
            values_ = h.values_;
            return *this;
        }

        RandomCustomDistr(const RandomCustomDistr& h) : values_(h.values_), gen_(h.gen_) {}
        virtual ~RandomCustomDistr() {}

        /** accessor */
        const DistrValues& getValues() const { return values_; }

        /** \brief the method to generate the random variable with given distribution */
        double operator()() {
            return getQuantile( gen_() );
        }

        /** method to calc mean  */
        double getMean() const {
            return std::accumulate( values_.begin(), values_.end(), 0.0,
                                    boost::bind( std::plus<double>(), _1,
                                                 boost::bind(&getMeanTerm, _2) ) );
        }

        /** method to calc standard deviation */
        double getStandardDeviation() const {
            double mi = getMean();
            double variation = std::accumulate( values_.begin(), values_.end(), 0.0,
                                                boost::bind( std::plus<double>(), _1,
                                                             boost::bind(&getVariationTerm, _2, mi) ) );
            return std::sqrt(variation);
        }

        /**  method to calc quantile */
        double getQuantile(double k) const {
            if( values_.empty() )
                return std::numeric_limits<double>::quiet_NaN();

            double sum = 0.0;
            for(DistrValues::const_iterator it = values_.begin(); it != values_.end(); ++it ) {
                double area = getArea(*it);
                if( k >= sum && k < sum + area) {
                    //the area > 0 (because if area == 0 the condition is false (k >= sum && k < sum )
                    return getLength(*it) * ( k - sum ) / area + it->getFrom();
                }
                sum += area;
            }
            //return the last value
            return values_.back().getTo();
        }
        /** method to calc probability density */
        double getProbabilityDensity(double x) const;

        /** method to calc distribution */
        double getDistribution(double x) const;
    private:
        DistrValues values_; //probablity distribution given by sum ranges
        RandomDouble gen_; //generates uniform distribution <0,1)
    };




    // \brief performs the simulation, create the random custom distribution based on given values
    class RandomCustomCreator {
    public:
        /** the epsilon (length of interval in histogram) is dependent of num_steps in Monte Carlo simulation.
            It is assumed
            Normal(mi,sigma)(x = mi + 3*sigma - epsilon/2, x = mi + 3*sigma + epsilon/2) >= sqrt(num_steps),
            so y = (x - mi)/sigma
            Normal(0,1)(y = 3 - epsil/2, y = 3 + epsil/2) >= sqrt(num_steps),
            so:

            epsilon = sigma/(sqrt(N)*pdf(3), where pdf(3) = N(0,1) dla x = 3
        */
        static double calculateEpsilon(double sigma, long num_steps) {
            const double SIG = sigma;
            const double N = num_steps >= 0 ? static_cast<double>(num_steps) : 1.0;
            boost::math::normal normal01(0.0,1.0);
            const double EPSILON = 0.5 * SIG / (boost::math::pdf(normal01, 3.0) * sqrt(N) );
            return EPSILON;
        }

        /** c-tor, the length of interval in histogram */
        RandomCustomCreator(double e) : epsilon_(e), numValues_(0L) { }

        /** c-tor */
        RandomCustomCreator(const RandomCustomCreator& c)
            : epsilon_(c.epsilon_), histogram_(c.histogram_),  numValues_(c.numValues_)
        {}

        /** d-tor */
        virtual ~RandomCustomCreator() {}

        //add the value to histogram, increments the proper interval
        double addValue(double value) {
            ++numValues_;

            //the mean of interval for given value
            double key = floor(value/epsilon_)*epsilon_;

            Histogram::iterator it = histogram_.find(key);
            if(it != histogram_.end() )
                ++(it->second);
            else
                histogram_.insert( std::make_pair( key, 1L ) );

            return value;
        }

        /** accessor - return the number of added */
        long getNumValues() const { return numValues_; }

        /** accessor - transform internal data to histogram distribution */
        RandomCustomDistr getRandomCustomDistr() const {
			const double epsilon2 = epsilon_/2.0;
			DistrValues values;
			for(Histogram::const_iterator it = histogram_.begin(); it != histogram_.end(); ++it) {
				values.push_back( DistrValue( it->first - epsilon2,
											  it->first + epsilon2,
											  static_cast<double>(it->second) ) );
			}
			return RandomCustomDistr(values);
        }
    private:
        //forbidden the assign operator
        RandomCustomCreator& operator=(const RandomCustomCreator&);

        double epsilon_; //!< the length of the segment

        typedef std::map<double,long> Histogram;
        Histogram histogram_; //the interval of variable and the num of simulations in given interval
        long numValues_; //the number of values
    };

    namespace {
        //the comparison functor
        bool rangeLessThanValue(const DistrValue& range, const DistrValue& x ) {
            return range.getTo() <= x.getTo();
        }
    }

    /** method to calc probability density */
    inline double RandomCustomDistr::getProbabilityDensity(double x) const {
        //TODO: msvc not implements lower_bound with diff types, so temporary object
        DistrValue v(x, x, 0.0);
        DistrValues::const_iterator it =
            std::lower_bound( values_.begin(), values_.end(), v, rangeLessThanValue );
        if( it == values_.end()  )
            return 0.0; //above histogram or empty
        if( it == values_.begin() && x < it->getFrom() )
            return 0.0; //below histogram
        return it->getValue();
    }

    /** method to calc distribution */
    inline double RandomCustomDistr::getDistribution(double x) const {
        //TODO: msvc not implements lower_bound with diff types, so temporary object
        DistrValue v(x, x, 0.0);
        DistrValues::const_iterator it =
            std::lower_bound( values_.begin(), values_.end(), v, rangeLessThanValue );
        //sum the values before x
        double sum = std::accumulate( values_.begin(), it, 0.0, boost::bind( std::plus<double>(), _1, boost::bind(&getArea, _2) ) );
        if( it != values_.end() )
            if(it->getFrom() <= x && x <= it->getTo() )
                sum += ( x - it->getFrom() ) * it->getValue();
        return sum;
    }

} //namespace faif

#endif //RANDOM_CUSTOM_DISTR_H
