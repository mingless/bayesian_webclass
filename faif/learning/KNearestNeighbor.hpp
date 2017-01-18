//   The k-Nearest Neighbor classifier

#ifndef FAIF_K_NEAREST_NEIGHBOR_CLASSIFIER_HPP
#define FAIF_K_NEAREST_NEIGHBOR_CLASSIFIER_HPP

#include <vector>
#include <functional>

#include <boost/bind.hpp>

#include "Classifier.hpp"

#include <boost/serialization/split_member.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>

namespace faif {
    namespace ml {


        /** \brief Distance metrics for Nomianal Values Collection, used in K Nearest Neighbors classifier

			distance between values is 0.0 (values equal), 0.5 (unknown value and other value) or 1.0 (different values)
			distance between points are sum of distances of coordinates
        */
		template<typename Val> class DistanceNominalValue {
            BOOST_CONCEPT_ASSERT((ValueConcept<Val>));
		public:
            typedef typename Classifier<Val>::AttrValue AttrValue;
            typedef typename Classifier<Val>::AttrDomain AttrDomain;
            typedef typename Classifier<Val>::AttrIdd AttrIdd;
            typedef typename Classifier<Val>::Domains Domains;
            typedef typename Classifier<Val>::ExampleTest ExampleTest;

			static double distance(const ExampleTest& a, const ExampleTest& b) {
				double distance = 0.0;
				typename ExampleTest::const_iterator i = a.begin(), j = b.begin();
				for(; i != a.end() && j != b.end(); ++i, ++j) {
					if(*i != *j) {
						if(*i == AttrDomain::getUnknownId() || *j == AttrDomain::getUnknownId() )
							distance += 0.5;
						else
							distance += 1.0;
					}
				}
				for(; i != a.end(); ++i) { distance += 1.0; } //not matched values from 'a' example
				for(; j != b.end(); ++j) { distance += 1.0; } //not matched values from 'b' example
				return distance;
			}
		};

        /** \brief k Nearest Neighbor classifier

            Contains the attributes, attribute values and categories,
            train examples, test examples and classifier methods.
        */
        template<typename Val,
				 template <typename> class Distance = DistanceNominalValue
				 >
        class KNearestNeighbor : public Classifier<Val> {
        public:
            typedef typename Classifier<Val>::AttrValue AttrValue;
            typedef typename Classifier<Val>::AttrDomain AttrDomain;
            typedef typename Classifier<Val>::AttrIdd AttrIdd;
            typedef typename Classifier<Val>::AttrIddSerialize AttrIddSerialize;
            typedef typename Classifier<Val>::Domains Domains;
            typedef typename Classifier<Val>::Beliefs Beliefs;
            typedef typename Classifier<Val>::ExampleTest ExampleTest;
            typedef typename Classifier<Val>::ExampleTrain ExampleTrain;
            typedef typename Classifier<Val>::ExamplesTrain ExamplesTrain;
        public:
            KNearestNeighbor();
            KNearestNeighbor(const Domains& attr_domains, const AttrDomain& category_domain);

            virtual ~KNearestNeighbor() { }

            /** clear the classifier */
            virtual void reset();

            /** \brief learn classifier (on the collection of training examples), here store all train examples.
			 */
            virtual void train(const ExamplesTrain& e);

            /** classify - find the category in neighbors, use default K (number of neighbors) */
            virtual AttrIdd getCategory(const ExampleTest& e) const { return getCategoryK(e, defaultK_); }

            /** classify - find the category in neighbors, use given K (number of neighbors) */
            AttrIdd getCategoryK(const ExampleTest& e, int K) const;


            /** \brief classify and return all classes with belief that the example is from given class

				use default K (number of neighbors)
			*/
            virtual Beliefs getCategories(const ExampleTest& e) const { return getCategoriesK(e, defaultK_); }

            /** \brief classify and return all classes with belief that the example is from given class

				use given K (number of neighbors)
			*/
			Beliefs getCategoriesK(const ExampleTest& e, int K) const;

            /** the ostream method */
            virtual void write(std::ostream& os) const;

            /** accessor - get the default number of neighbors used in calculation */
            int getDefaultK() const { return defaultK_; }

            /** mutator - set the default number of neighbors used in calculation */
            void setDefaultK(int k) { defaultK_ = k; }
        private:
            /** copy c-tor not allowed */
            KNearestNeighbor(const KNearestNeighbor&);
            /** assignment not allowed */
            KNearestNeighbor& operator=(const KNearestNeighbor&);

            ExamplesTrain memory_; //store training examples
            int defaultK_; //default number of neighbors used in calculation
        private:
            /** \brief serialization using boost::serialization */
            friend class boost::serialization::access;

            template<class Archive>
            void serialize( Archive &ar, const unsigned int ) {
				ar & boost::serialization::make_nvp("KNNBase", boost::serialization::base_object<Classifier<Val> >(*this) );
				ar & boost::serialization::make_nvp("memory", memory_ );
				ar & boost::serialization::make_nvp("defaultK", defaultK_ );
            }

        };

        //////////////////////////////////////////////////////////////////////////////////////////////////
        // class KNearestNeighbor implementation
        //////////////////////////////////////////////////////////////////////////////////////////////////

        template<typename Val, template <typename> class Distance>
        KNearestNeighbor<Val, Distance>::KNearestNeighbor() : Classifier<Val>(), memory_(), defaultK_(3)
        { }

        template<typename Val, template <typename> class Distance>
        KNearestNeighbor<Val, Distance>::KNearestNeighbor(const Domains& attr_domains, const AttrDomain& category_domain)
            : Classifier<Val>(attr_domains, category_domain), memory_(), defaultK_(3)
        { }

        /** \brief reset - clear the memory */
        template<typename Val, template <typename> class Distance>
        void KNearestNeighbor<Val, Distance>::reset() {
            memory_.clear();
        }

        /** \brief learn classifier (on the collection of training examples) - remember training examples */
        template<typename Val, template <typename> class Distance>
        void KNearestNeighbor<Val, Distance>::train(const ExamplesTrain& e) {
            memory_ = e;
        }

        /** classify - return the major category for best node from decision tree */
        template<typename Val, template <typename> class Distance>
        typename KNearestNeighbor<Val, Distance>::AttrIdd
		KNearestNeighbor<Val, Distance>::getCategoryK(const ExampleTest& e, int K) const {
			Beliefs bel = getCategoriesK(e, K);
			if( bel.empty() )
				return AttrDomain::getUnknownId();
			else
				return bel.front().getValue(); //histogram is sorted
        }

        /** \brief classify and return all classes with belief that the example is from given class */
        template<typename Val, template <typename> class Distance>
        typename KNearestNeighbor<Val, Distance>::Beliefs
		KNearestNeighbor<Val, Distance>::getCategoriesK(const ExampleTest& e, int K) const {

			typedef std::pair<typename ExamplesTrain::const_iterator, double> DistanceDescr;
			typedef std::vector<DistanceDescr> DistanceDescrVec;

			//std::cout << "Neighbors for: " << e << std::endl;
			DistanceDescrVec distances;
			distances.reserve( memory_.size() );
			for(typename ExamplesTrain::const_iterator ii = memory_.begin(); ii != memory_.end(); ++ii) {
				//std::cout << "Memory: " << *ii << " Distance: " << Distance<Val>::distance( *ii, e ) << std::endl;
				distances.push_back( DistanceDescr(ii, Distance<Val>::distance( *ii, e ) ) );
			}
			typename DistanceDescrVec::iterator middle = distances.end(); //for too small collection
			if( distances.end() - distances.begin() > K) {
				middle = distances.begin() + K; //middle between begin and end
			}
			std::partial_sort( distances.begin(), middle, distances.end(),
							   boost::bind( std::less<double>(), boost::bind(&DistanceDescr::second, _1), boost::bind(&DistanceDescr::second, _2) ) );

			TrainExampleCategoryCounters<Val> counters;
			for(typename DistanceDescrVec::const_iterator jj = distances.begin(); jj != middle; ++jj) {
				const ExampleTrain& ex = *(jj->first);
				counters.inc(ex);
			}
			return counters.getHistogram();
        }

        /** ostream method */
        template<typename Val, template <typename> class Distance>
        void KNearestNeighbor<Val, Distance>::write(std::ostream& os) const {
            os << "KNN classifier, defaultK=" << defaultK_ << ", memSize=" << memory_.size() << ":" << std::endl;
            std::copy(memory_.begin(), memory_.end(), std::ostream_iterator<ExampleTrain>(os,";") );
            os << std::endl;
        }

    }//namespace ml
} //namespace faif

#endif //FAIF_K_NEAREST_NEIGHBOR_CLASSIFIER_HPP
