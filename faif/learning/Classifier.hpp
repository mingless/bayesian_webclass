#ifndef FAIF_CLASIFIER_HPP_
#define FAIF_CLASIFIER_HPP_


#include <memory>
#include <map>
#include <algorithm>
#include <cmath>

#include <boost/bind.hpp>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>

#include "../Value.hpp"
#include "../Point.hpp"
#include "Belief.hpp"

namespace faif {

	/** \brief machine learning namespace (mainly classifier algorithms)
	 */
    namespace ml {

        /** \brief calculate x * log(x) value. If x == 0 return 0. */
        inline double calcEntropy(double freq) {
            if( freq > 0.0 )
                return -(freq * std::log(freq));
            else
                return 0.0;
        }

        /** \brief the clasiffier interface

            type definitions for AttrValue, AttrDomain, AttrIdd and others

            Store attribute domains and category domain, methods to create training/testing example,
            load/save (boost::serialization), pure virtual for training and classifing.
        */
        template<typename Val>
        class Classifier {
            BOOST_CONCEPT_ASSERT((ValueConcept<Val>));
        public:
            typedef Val Value;

            /** \brief  attribute value representation in learning */
            typedef typename Val::Value AttrValue;

            /** \brief the attribute domain for learning */
            typedef typename Val::DomainType AttrDomain;

            /** \brief  attribute id representation in learning */
            typedef typename Val::DomainType::ValueId AttrIdd;

            /** \brief  for serialization the const interferes */
            typedef typename Val::DomainType::ValueIdSerialize AttrIddSerialize;

            /** \breif the domain collection; the pointers should be valid */
            typedef Space<AttrDomain> Domains;

            /** \brief collection of pair (AttrIdd, Probability) */
            typedef typename Belief<Val>::Beliefs Beliefs;

            /** \brief the test example (collection of AttrIdd) */
            typedef Point<Val> ExampleTest;

            /** \brief the helping inner class to init PointAndFeature structure using getUnknownId method
             */
            template<typename Feature> struct InitValueId {
                static Feature init() {
                    return Val::DomainType::getUnknownId();
                }
            };

            /** \brief the train example (test example and the category) */
            typedef PointAndFeature<Val, AttrIdd, InitValueId> ExampleTrain;


            /** \brief inner class - examples train collection */
            class ExamplesTrain : public std::vector<ExampleTrain> {
            public:
                /** \brief the most common category in the training example container */
                AttrIdd getMajorCategory() const;

                /** \brief entropy of set of examples */
                double entropy() const;
			private:
				/** \brief serialization using boost::serialization */
				friend class boost::serialization::access;

				template<class Archive>
				void serialize( Archive &ar, const unsigned int file_version ){
					ar & boost::serialization::make_nvp("Examples", boost::serialization::base_object< std::vector<ExampleTrain> >(*this) );
				}

            };

        public:
            Classifier() {}

            Classifier(const Domains& attr_domains, const AttrDomain& category_domain)
                : domains_(attr_domains), category_(category_domain) {}

            virtual ~Classifier(){}

            /** \brief accessor */
            const Domains& getAttrDomains() const { return domains_; }

            /** \brief accessor */
            const AttrDomain& getCategoryDomain() const { return category_; }

            /** \brief accessor (helper) */
            AttrIdd getCategoryIdd(const AttrValue& val) const { return category_.find(val); }

            /** \brief the clasiffier will have no knowledge */
            virtual void reset() = 0;

            /** \brief learn classifier (on the collection of training examples) */
            virtual void train(const ExamplesTrain&) = 0;

            /** \brief classify */
            virtual AttrIdd getCategory(const ExampleTest& example) const = 0;

            /** \brief classify and return all classes with belief that the example is from each class
             * classes are sorted from the best (index 0) to the worst */
            virtual Beliefs getCategories(const ExampleTest& example) const = 0;

			/** the ostream method */
			virtual void write(std::ostream& os) const;
        private:
            /** \brief serialization using boost::serialization */
            friend class boost::serialization::access;

            template<class Archive>
            void save(Archive & ar, const unsigned int /* file_version */) const {
             //not used boost::serialization::list (for list of lists) because load require
             //instantiate containter item and load to them
             //because address_restarting works only once (I'm not sure, but this is my experience)
	      unsigned int size = static_cast<unsigned int>(domains_.size());
             ar & boost::serialization::make_nvp("ClassifierDomainsCount", size );
             for(typename Domains::const_iterator i = domains_.begin(); i != domains_.end(); ++i) {
                 ar & boost::serialization::make_nvp("ClassifierDomain", *i);
             }
             ar & boost::serialization::make_nvp("ClassifierCategory", category_);
            }

            template<class Archive>
            void load(Archive & ar, const unsigned int /* file_version */) {
             unsigned int size;
             ar & boost::serialization::make_nvp("ClassifierDomainsCount", size );
             domains_.clear();
             for(unsigned int i = 0; i < size; ++i) {
                 domains_.push_back(AttrDomain()); //add empty item to containter
                 AttrDomain& d = domains_.back(); //upload to this item
                 // so there is no need to additional address_restarting for  boost::serialization
                 ar >> boost::serialization::make_nvp("ClassifierDomain", d);
             }
             ar & boost::serialization::make_nvp("ClassifierCategory", category_);
            }

            template<class Archive>
            void serialize( Archive &ar, const unsigned int file_version ){
                boost::serialization::split_member(ar, *this, file_version);
            }

        private:
            Domains domains_;
            AttrDomain category_;
        };

		/** the ostream method */
		template<typename Val>
		void Classifier<Val>::write(std::ostream& os) const {
            os << "Categories(" << category_.getSize() << ")" << category_ << std::endl;
            os << "Attributes(" << static_cast<int>(domains_.size()) << "):";
            std::copy(domains_.begin(), domains_.end(), std::ostream_iterator<AttrDomain>(os,"") );
		}

		/**
		   ostream operator
		*/
		template<typename Val>
		std::ostream& operator<<(std::ostream& os, const Classifier<Val>& c) {
			c.write(os);
			return os;
		}

        /** \brief create the test example from iterator range or C-like table of values */
        template<typename It, typename Val>
        typename Classifier<Val>::ExampleTest
        createExample(It begin, It end, const Classifier<Val>& classifier) {
            return classifier.getAttrDomains().createPoint(begin, end );
        }

        /** \brief create the train example from range or C-like table of values */
        template<typename It, typename Val>
        typename Classifier<Val>::ExampleTrain
        createExample(It begin, It end, const typename Classifier<Val>::AttrValue& cat, const Classifier<Val>& classifier) {
            typedef typename Classifier<Val>::ExampleTrain ExampleTrain;
            return ExampleTrain( classifier.getAttrDomains().createPoint(begin, end ), classifier.getCategoryDomain().find(cat) );
        }

        /** \brief create the test example from collection of pairs: attribute(domain) identifier and attribute value */
        template<typename Val>
        typename Classifier<Val>::ExampleTest
        createExample(const std::vector<std::pair<std::string, typename Classifier<Val>::AttrValue> >& collection, const Classifier<Val>& classifier) {
            return classifier.getAttrDomains().createPoint(collection);
        }

        /** \brief create the test example from collection of pairs: attribute(domain) identifier and attribute value.
            Throws exception if the string identifiers not match the required domains identifiers */
        template<typename Val>
        typename Classifier<Val>::ExampleTest
        createExampleStrict(const std::vector<std::pair<std::string, typename Classifier<Val>::AttrValue> >& collection, const Classifier<Val>& classifier) {
            return classifier.getAttrDomains().createPointStrict(collection);
        }

        /** \brief create the train example from collection of pairs: attribute(domain) identifier and attribute value */
        template<typename Val>
        typename Classifier<Val>::ExampleTrain
        createExample(const std::vector<std::pair<std::string, typename Classifier<Val>::AttrValue> >& collection, const typename Classifier<Val>::AttrValue& cat,
                      const Classifier<Val>& classifier) {
            typedef typename Classifier<Val>::ExampleTrain ExampleTrain;
            return ExampleTrain( classifier.getAttrDomains().createPoint(collection), classifier.getCategoryDomain().find(cat) );
        }


        /** helping functor for calculate histogram based on categories for collection of train examples.
            It could be used to find major category. */
        template<typename Val>
        class TrainExampleCategoryCounters {
        public:
            typedef typename Classifier<Val>::AttrDomain AttrDomain;
            typedef typename Classifier<Val>::Domains Domains;
            typedef typename Classifier<Val>::AttrIdd AttrIdd;
            typedef typename Classifier<Val>::Beliefs Beliefs;
            typedef typename Classifier<Val>::ExampleTrain ExampleTrain;
            typedef typename Classifier<Val>::ExamplesTrain ExamplesTrain;

            typedef std::map<AttrIdd,int> Counters;

            /** \brief c-tor, empty counters */
            TrainExampleCategoryCounters() : sum_(0) {}

            /** \brief c-tor, counters initialized by train examples collection */
            TrainExampleCategoryCounters(typename ExamplesTrain::const_iterator beg,
                                         typename ExamplesTrain::const_iterator end) : sum_(0) {
                std::for_each( beg, end, boost::bind(&TrainExampleCategoryCounters::inc, this, _1 ) );
            }

            //increment counters
            void inc(const ExampleTrain& e) {

                ++sum_;
                typename Counters::iterator it = counters_.find(e.getFeature() );
				if(it == counters_.end() ) {
					counters_.insert( std::make_pair(e.getFeature(), 1 ) );
				}
				else {
					++it->second;
				}
			}
			//key for maximum value
			AttrIdd maxCount() const {
				typename Counters::const_iterator it  =
					std::max_element( counters_.begin(), counters_.end(),
									  boost::bind(&Counters::value_type::second, _1) < boost::bind(&Counters::value_type::second, _2) );
				if(it != counters_.end() )
					return it->first;
				else
					return Val::DomainType::getUnknownId(); //not found max i.e. empty container, return the unknown id
			}

			/** \brief access to counters */
			const Counters& get() const { return counters_; }

			/** \brief optimization: instead of accumulate all values from counters container keep the integer member */
			int getSum() const { return sum_; }

			/** \brief entropy of counters */
			double entropy() const {
				double entr = 0.0;
				if( sum_ > 0 ) {
					double sum = static_cast<double>( sum_ );
					for(typename Counters::const_iterator i = counters_.begin(); i != counters_.end(); ++i) {
						entr += calcEntropy( static_cast<double>(i->second) / sum );
					}
				}
				return entr;
			}
			/** \brief histogram from counters - Beliefs class where each position is counter divided by counters sum.

				Histogram is sorted from biggest to smallest probability
			*/
			Beliefs getHistogram() const {
				Beliefs histogram;
				for(typename Counters::const_iterator i = counters_.begin(); i != counters_.end(); ++i) {
					histogram.push_back( typename Beliefs::value_type(i->first,
																	  static_cast<Probability>(i->second) / static_cast<Probability>(sum_) ) );
				}
				std::sort(histogram.begin(), histogram.end());
				return histogram;
			}
		private:
			Counters counters_;
			int sum_;
		};

		/** method implementation for Classifier<Val>::ExamplesTrain */
        template<typename Val>
		typename Classifier<Val>::AttrIdd
		Classifier<Val>::ExamplesTrain::getMajorCategory() const {
			TrainExampleCategoryCounters<Val> counters(this->begin(), this->end());
			return counters.maxCount();
		}

		/** method implementation for Classifier<Val>::ExamplesTrain */
        template<typename Val>
		double Classifier<Val>::ExamplesTrain::entropy() const {
			TrainExampleCategoryCounters<Val> counters(this->begin(), this->end());
			return counters.entropy();
		}

    } //namespace ml
} //namespace faif

#endif //FAIF_CLASIFIER_HPP_
