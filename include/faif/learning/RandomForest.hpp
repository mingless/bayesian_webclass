//   The Random Forest classifier

#ifndef FAIF_RANDOM_FOREST_HPP
#define FAIF_RANDOM_FOREST_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
//msvc14.0 warnings for Boost.Serialization
#pragma warning(disable:4100)
#pragma warning(disable:4512)
#endif


#include "Classifier.hpp"
#include "DecisionTree.hpp"
#include "../utils/Random.hpp"

#include <list>
#include <set>
#include <algorithm>
#include <iterator>
#include <limits>
#include <cassert>
#include <memory>

#include <boost/serialization/list.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>

namespace faif {
	namespace ml {

		/** \brief Random Forest Classifier.

			Contains the attributes, attribute values and categories,
			train examples, test examples and classifier methods.
		*/
		template<typename Val>
		class RandomForest : public Classifier<Val> {
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
			RandomForest();
			RandomForest(const Domains& attr_domains, const AttrDomain& category_domain);

			virtual ~RandomForest() { }

			/** \brief set manually the parameters
			 *  \param K trees number
			 *  \param features in each tree number
			 */
			virtual void tune(int K_, int N_);

			/** clear forest */
			virtual void reset();

			/** \brief learn classifier (on the collection of training examples).
			 */
			virtual void train(const ExamplesTrain& e);

			/** classify */
			virtual AttrIdd getCategory(const ExampleTest&) const;

			/** \brief classify and return all classes with belief that the example is from given class
			 */
			virtual Beliefs getCategories(const ExampleTest&) const;
		private:
			/** \brief create a set of trees using bootstrapping
			 */
			void createForest();

			/**
				\brief transform a collection of trees results into beliefs using the voting idea
				\param list_ collection of the results of all classifiers
			*/
			Beliefs prepareResults(const std::list<typename RandomForest<Val>::AttrIdd>&) const;

			/** \brief randomly select a subset of ExamplesTrain
				size of subset = size of ExamplesTrain
			 */
			ExamplesTrain exampleBootstrap(const ExamplesTrain&);

			/** \brief generate pseudorandom collection of numbers with uniform distribution
				\param size size of returning collection (number of digits to generate)
				\param range the upper bound of the sampling set (lower bound is 0)
				\param gen random engine generator
				\param isReplacementON if the sampling is to be with replacement or not
			 */
			std::vector<int> uniformRandomGenerator(std::size_t size, std::size_t range, bool isReplacementON);

			/** copy c-tor not allowed */
			RandomForest(const RandomForest&);

			/** assignment not allowed */
			RandomForest& operator=(const RandomForest&);

			/**
				internal class - implementation of a decision tree with attributes covering (based on the vector of allowed attributes numbers)
			*/
			class RandomTree : public DecisionTree<Val>
			{
			public:
				RandomTree(): DecisionTree<Val>(){}
				RandomTree(const Domains& attr_domains, const AttrDomain& category_domain)
				: DecisionTree<Val>(attr_domains, category_domain){
				}

				/** clear forest */
				void reset()
				{
					DecisionTree<Val>::reset();
				}

				/** \brief learn classifier (on the collection of training examples).
				 */
				void train(const ExamplesTrain& e)
				{
					DecisionTree<Val>::train(e);
				}

				/** classify */
				AttrIdd getCategory(const ExampleTest& e) const
				{
					return DecisionTree<Val>::getCategory(e);
				}

				/** \brief classify and return all classes with belief that the example is from given class
				 */
				typename RandomForest<Val>::Beliefs getCategories(const ExampleTest& e) const
				{
					return DecisionTree<Val>::getCategories(e);
				}

				template < class Tcontainer >
				static Tcontainer CoverDomains(Tcontainer domains_, std::vector<int> attribs_allowed)
				{
					Tcontainer newDomains_;
					for (std::vector<int>::const_iterator it=attribs_allowed.begin(); it != attribs_allowed.end(); ++it)
					{
						typename Tcontainer::iterator it_d=domains_.begin();
						std::advance(it_d,*it);
						newDomains_.push_back(*it_d);
						//std::cout << *it_d << std::endl;
					}
					return newDomains_;
				}
				private:
				/** \brief serialization using boost::serialization */
				friend class boost::serialization::access;

				template<class Archive>
				void serialize( Archive &ar, const unsigned int /* file_version*/ ){
					ar & boost::serialization::make_nvp("BaseTree",boost::serialization::base_object<DecisionTree<Val> >(*this));

				}

			};

		private:
			/** \brief serialization using boost::serialization */
			friend class boost::serialization::access;

			template<class Archive>
			void serialize( Archive &ar, const unsigned int /* file_version*/ ){
				ar.template register_type<RandomTree>();

				ar & boost::serialization::make_nvp("RFCBase", boost::serialization::base_object<Classifier<Val> >(*this) );
				ar & boost::serialization::make_nvp("RTrees", trees_ );
				ar & boost::serialization::make_nvp("K", K_ );
			}

		private:
			/** collection of trees */
			typedef std::list<boost::shared_ptr<RandomTree>> RTrees;
			RTrees trees_;
			/** features number in each tree parameter */
			int N_;
			/** forest size parameter */
			int K_;

		}; //class RandomForest

		//////////////////////////////////////////////////////////////////////////////////////////////////
		// class RandomForest implementation
		//////////////////////////////////////////////////////////////////////////////////////////////////

		template<typename Val>
		RandomForest<Val>::RandomForest() : Classifier<Val>(), N_(0), K_(0){}

		template<typename Val>
		RandomForest<Val>::RandomForest(const Domains& attr_domains, const AttrDomain& category_domain)
				: Classifier<Val>(attr_domains, category_domain), N_(0), K_(0){}

		template<typename Val>
		void RandomForest<Val>::tune(int K_in, int N_in)
		{
			this->K_ = K_in;
			this->N_ = N_in;
		}

		/** clear the forest */
		template<typename Val>
		void RandomForest<Val>::reset()
		{
			K_ = 0;
			N_ = 0;
			trees_.clear();
		}

		template<typename Val>
		void RandomForest<Val>::train(const ExamplesTrain& e)
		{
			int featuresNum_ = static_cast<int>(Classifier<Val>::getAttrDomains().size());

			// Breiman Forest-RI recommendation
			K_ = K_ != 0 ? K_ : std::max(ceil(sqrt(2*e.size())) ,ceil(2*featuresNum_/ceil(sqrt(featuresNum_))) );
			createForest();
			for( typename RTrees::iterator it=trees_.begin(); it != trees_.end(); it++ )
			{
				RandomTree & obj = *(*it); // dereference iterator, dereference pointer
				obj.train(exampleBootstrap(e));
				//std::cout << obj << std::endl << std::endl;
			}
		}

		template<typename Val>
		typename RandomForest<Val>::AttrIdd RandomForest<Val>::getCategory(const ExampleTest& e) const
		{
			//std::list< typename Val::Value> results_;
			std::list< RandomForest<Val>::AttrIdd> results_;
			for( typename RTrees::const_iterator it=trees_.begin(); it != trees_.end(); it++ )
			{
				RandomTree & obj = *(*it); // dereference iterator, dereference pointer
				results_.push_back(obj.getCategory(e));
			}
			Beliefs bel_ = prepareResults(results_);
			if(bel_.empty() )
				return AttrDomain::getUnknownId();
			else
				return bel_.at(0).getValue();
		}

		template<typename Val>
		typename RandomForest<Val>::Beliefs RandomForest<Val>::getCategories(const ExampleTest& e) const
		{
			std::list< RandomForest<Val>::AttrIdd> results_;
			for( typename RTrees::const_iterator it=trees_.begin(); it != trees_.end(); it++ )
			{
				RandomTree & obj = *(*it); // dereference iterator, dereference pointer
				results_.push_back(obj.getCategory(e));
				//std::cout << obj.getCategory(e)->get()  << std::endl;
			}
			Beliefs bel_ = prepareResults(results_);
			if(bel_.empty() )
				return Beliefs();
			else
				return bel_;
		}

		template<typename Val>
		void RandomForest<Val>::createForest()
		{
			size_t size_ = Classifier<Val>::getAttrDomains().size();
			// Breiman Forest-RI recommendation
			N_ =  N_ != 0 ? N_ : ceil(sqrt(static_cast<int>(size_)));
			//std::cout << "K:"<<K_<< std::endl;
			//std::cout << "N:"<<N_<< std::endl;
			// create collection of trees with random attributes
			for(int i=0;i<K_;++i)
				trees_.push_back(boost::shared_ptr<RandomTree>(new RandomTree(   RandomTree::CoverDomains(Classifier<Val>::getAttrDomains(),uniformRandomGenerator(N_,size_-1,false)),Classifier<Val>::getCategoryDomain())));

		}

		template<typename Val>
		typename RandomForest<Val>::Beliefs RandomForest<Val>::prepareResults(const std::list<typename RandomForest<Val>::AttrIdd>& list_) const
		{
			std::vector<std::pair<typename RandomForest<Val>::AttrIdd,double>> resultsList_;
			std::map<typename RandomForest<Val>::AttrIdd,double> count_;
			Beliefs toRet_;
			// Create map of occurrence
			for( typename std::list<typename RandomForest<Val>::AttrIdd>::const_iterator it=list_.begin(); it != list_.end(); it++ )
			{
				// Get the feature name
				typename Val::Value class_ = ((typename RandomForest<Val>::AttrIdd) *it)->get();
				// And check whether this feature already exists in map
				auto itClass_ = find_if(count_.begin(), count_.end(), [&class_](const std::pair<typename RandomForest<Val>::AttrIdd,double>& obj) {return obj.first->get()==class_;});
				// Create new element if not or increase the counter if so
				if (itClass_ != std::end(count_))
					++itClass_->second;
			    else
			    	++count_[*it];
			}
			// Normalize map values by list_ size - switch replicates into probability
			// Convert std::map into std::vector of pairs
			for( typename std::map<typename RandomForest<Val>::AttrIdd,double>::const_iterator it = count_.begin(); it != count_.end(); ++it )
				resultsList_.push_back(std::pair<typename RandomForest<Val>::AttrIdd,double>(it->first,it->second/list_.size()));
			// Sort a vector of pairs based on the probability
			std::sort(resultsList_.begin(), resultsList_.end(),
					boost::bind(&std::pair<typename RandomForest<Val>::AttrIdd,double>::second, _1) >
					boost::bind(&std::pair<typename RandomForest<Val>::AttrIdd,double>::second, _2));
			// Create Beliefs
			for( typename std::vector<std::pair<typename RandomForest<Val>::AttrIdd,double>>::const_iterator it = resultsList_.begin(); it != resultsList_.end(); ++it )
				toRet_.push_back(typename Beliefs::value_type(it->first,it->second));
			return toRet_;
		}

		template<typename Val>
		typename RandomForest<Val>::ExamplesTrain RandomForest<Val>::exampleBootstrap(const ExamplesTrain& example_)
		{
			size_t S_ = example_.size();
			ExamplesTrain subset_;
			std::vector<int> sample_ = uniformRandomGenerator(S_, S_-1, true);
			for( std::vector<int>::iterator it = sample_.begin(); it != sample_.end(); ++it )
				subset_.push_back(*std::next(example_.begin(),*it));

			return subset_;
		}

		template<typename Val>
		std::vector<int> RandomForest<Val>::uniformRandomGenerator(std::size_t size, std::size_t range, bool isReplacementON)
		{
			std::vector<int> toRet_(size, -1);
			// \-> by default whole vector is initiate with zeros
			// and it would be dangerous if we generate numbers from zero
			RandomInt uniform_generator(0, static_cast<int>(range) );
			for( std::vector<int>::iterator it = toRet_.begin(); it != toRet_.end(); ++it )
			{
				int sample_ = uniform_generator();
				while( !isReplacementON && std::find(toRet_.begin(), toRet_.end(), sample_) != toRet_.end() )
				{
					sample_ = uniform_generator();
				}
				*it = sample_;
			}

			return toRet_;
		}


	}//namespace ml
}//namespace faif

#endif //FAIF_RANDOM_FOREST_HPP

