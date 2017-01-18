#ifndef FAIF_TS_DISCRETIZERS
#define FAIF_TS_DISCRETIZERS

#if defined(_MSC_VER) && (_MSC_VER >= 1700)
//msvc14.0 warnings for Boost.Serialization
#pragma warning(disable:4100)
#pragma warning(disable:4512)
#endif

#include <ostream>

#include <vector>
#include <algorithm>
#include <numeric>
#include <iterator>
#include <cmath>

#include <boost/concept_check.hpp>
#include <boost/scoped_array.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/minmax_element.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/construct.hpp>
#include <boost/lambda/core.hpp>
#include <boost/lambda/lambda.hpp>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>

namespace faif {
    namespace timeseries {

        /** The section representation */
        template<typename V> class  Section : public std::pair<V, V> {

            BOOST_CONCEPT_ASSERT((boost::EqualityComparable<V>));

        public:
            Section() {} //!< required by serialization
            Section(V min_val, V max_val) : std::pair<V, V>(min_val, max_val) { }
            V getMin() const { return this->first; }
            V getMax() const { return this->second; }
            bool operator==(const Section& s) const { return this->first == s.first && this->second == s.second; }

                        /** \brief serialization using boost::serialization */
                        template<class Archive>
                        void serialize(Archive & ar, const unsigned int /* file_version */){
                                ar & boost::serialization::make_nvp("Base",
								    boost::serialization::base_object<std::pair<V,V> >(*this)
								    );
                        }

        };

      template<typename V>
      inline std::ostream& operator<<(std::ostream& os, const Section<V>& s) {
          os << "(Min: " <<  s.getMin() << ", Max:" << s.getMax() << ")";
	  return os;
      }

        /** discretizer - collection of sections */
        template<typename V> class Discretizer : public std::vector<Section<V> > {
                public:
                        /** calculete the class for given sections */
                        int discretize(V value) const {
                                /** sections are sorted */
                                typename Discretizer<V>::const_iterator it =
                                        std::lower_bound( this->begin(), this->end(), value,
                                                                          boost::bind(std::less<V>(), boost::bind(&Section<V>::getMax, _1), _2 ) );
                                if( it == this->end() )
                                        return static_cast<int>(this->size()) - 1; //the last class
                                else
                                        return static_cast<int>(it - this->begin()); //return the index of founded class
                        }


                        /** \brief serialization using boost::serialization */
                        template<class Archive>
                        void serialize(Archive & ar, const unsigned int /* file_version */){
                                ar & boost::serialization::make_nvp("Base",
								    boost::serialization::base_object<std::vector<Section<V> > >(*this)
								    );
                        }
        };

                template<typename V>
                inline std::ostream& operator<<(std::ostream& os, const Discretizer<V>& d) {
                        std::copy(d.begin(), d.end(), std::ostream_iterator< Section<V> >(os, ",") );
                        return os;
                }

        //** creates the discretizer - sections of equal width from sample set of values */
        template<typename It>
        Discretizer<typename It::value_type> createEqualWidthSections(It begin, It end, unsigned int num_sections) {

                        BOOST_CONCEPT_ASSERT((boost::RandomAccessIterator<It>));

            typedef typename It::value_type Value;
            Discretizer<Value> ret;
                        if( begin == end || num_sections == 0)
                                return ret;
                        std::pair<It, It> mm = boost::minmax_element(begin, end );
                        Value length = (*(mm.second) - *(mm.first)) / static_cast<Value>(num_sections);
                        Value curr_min = *(mm.first) - length; //the current value - to generator
                        Value curr_max = *(mm.first); //the current value - to generator
                        typename boost::lambda::var_type<Value>::type vmin(boost::lambda::var(curr_min));
                        typename boost::lambda::var_type<Value>::type vmax(boost::lambda::var(curr_max));
                        std::generate_n(back_inserter(ret), num_sections,
                                                        boost::lambda::bind(boost::lambda::constructor< Section<Value> >(),
                                                                                                vmin += boost::lambda::constant(length),
                                                                                                vmax += boost::lambda::constant(length)));
            return ret;
        }


        /** make the discretizer based on sequence of values- use k-means method */
        template<typename It>
                Discretizer<typename It::value_type> createKMeansSections( It begin, It end, const unsigned int num_sections ) {

                        BOOST_CONCEPT_ASSERT((boost::ForwardIterator<It>));

            typedef typename It::value_type Value;
                        typedef typename std::vector<Value> ValVect;

                        if( begin == end || num_sections == 0)
                                return Discretizer<Value>();
                        ValVect values(begin, end);
                        std::sort(values.begin(), values.end() ); //working copy, sorted vector of values
                        const Value& minValue = values.front();
                        const Value& maxValue = values.back();
                        //initialisation - equal lenghth sections
                        Discretizer<Value> current = createEqualWidthSections(values.begin(), values.end(), num_sections );

                        const int MAX_NUMBER_OF_ITERATIONS = 100;
                        const Value MEANS_NOT_CHANGE_EPSILON = 0.001*(maxValue - minValue); //distance between means that alows to stop algorithm

                        for(int i = 0; i < MAX_NUMBER_OF_ITERATIONS; ++i ) { //k-means algorithm

                                // std::cout << "Iteration " << i << " Disc " << current << std::endl;

                                //discretize using 'current'
                                boost::scoped_array< ValVect > result( new ValVect[num_sections] ); //results of discretizers

                                for(typename ValVect::const_iterator it = values.begin(); it != values.end(); ++it) {
                                        result[current.discretize(*it)].push_back(*it);
                                }
                                //create new central points
                                boost::scoped_array<Value> means( new Value[num_sections] );
                                Value distance = 0.0; //the sum of distances between new central points (means) and the old central points
                                const ValVect* disRes = result.get(); //iterator
                                Value* mu = means.get(); //iterator
                                for(typename Discretizer<Value>::const_iterator sec = current.begin(); sec != current.end(); ++sec ) {
                                        Value secMean = ( sec->getMax() + sec->getMin() ) / 2.0; //central point of current section
                                        if(disRes->empty() ) {
                                                *mu =  secMean; //if no result examples -> central point is not changed
                                        }
                                        else {
                                                Value acc = std::accumulate(disRes->begin(), disRes->end(), 0.0);
                                                Value m = acc / static_cast<Value>(disRes->size());
                                                distance += std::fabs( secMean - m );
                                                *mu = m;
                                        }
                                        ++disRes;
                                        ++mu;
                                }
                                if(distance < MEANS_NOT_CHANGE_EPSILON) //central points are not changed enough
                                        break;

                                Discretizer<Value> newDisc; //create new discretizer
                                Value secStart = minValue;
                                for(unsigned int j = 0; j < num_sections - 1; ++j ) {
                                        Value secStop = (means[j]+means[j+1])/2.0;
                                        newDisc.push_back( Section<Value>(secStart, secStop) );
                                        secStart = secStop;
                                }
                                newDisc.push_back( Section<Value>(secStart, maxValue) );
                                current = newDisc;
                        }
                        return current;
        }

    } //namespace timeseries
} //namespace faif

#endif //FAIF_TS_DISCRETIZERS
