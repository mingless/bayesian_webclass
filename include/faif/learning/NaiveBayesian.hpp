//   The Naive Bayesian classifier

#ifndef FAIF_NAIVE_BAYESIAN_HPP_
#define FAIF_NAIVE_BAYESIAN_HPP_

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
//msvc14.0 warnings for Boost.Serialization
#pragma warning(disable:4100)
#pragma warning(disable:4512)
#endif

#include <string>
#include <memory>
#include <algorithm>

#include <boost/bind.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>

#include "Classifier.hpp"

namespace faif {
    namespace ml {

        /** \brief Naive Bayesian Classifier.

            Contains the attributes, attribute values and categories,
            train examples, test examples and classifier methods.
        */
        template<typename Val>
        class NaiveBayesian : public Classifier<Val> {
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
            NaiveBayesian();
            NaiveBayesian(const Domains& attr_domains, const AttrDomain& category_domain);
            virtual ~NaiveBayesian() { }

            /** the clear the learned parameters */
            virtual void reset();

            /** \brief learn classifier (on the collection of training examples) */
            virtual void train(const ExamplesTrain& e) {
                std::for_each( e.begin(), e.end(), boost::bind( &NaiveBayesian::trainIncremental, this, _1 ) );
            }

            /** classify (Naive Bayes Classifier) */
            virtual AttrIdd getCategory(const ExampleTest&) const;

            /** \brief classify and return all classes with belief that the example is from given class */
            virtual Beliefs getCategories(const ExampleTest&) const;

            /** incremental learn - add training example  */
            void trainIncremental(const ExampleTrain&);

            /** the ostream method */
            virtual void write(std::ostream& os) const;

            /** change the internal obj to classify add return result of classification */
            AttrIdd switchGetCategory(const ExampleTest& example);
            /** change the internal obj to classify add return result of classification */
            Beliefs switchGetCategories(const ExampleTest& example);
            /** change the internal obj to train and add new example */
            void switchAddTraining(const ExampleTrain& example);
            /** change the internal obj to classify, because this object store internal state */
            void switchLoadSaveState();
        private:
            /** change the internal obj to classify if necessary */
            void loadSaveState() const;

            /** \brief serialization using boost::serialization */
            friend class boost::serialization::access;

            template<class Archive>
            void save(Archive & ar, const unsigned int /* file_version */) const;

            template<class Archive>
            void load(Archive & ar, const unsigned int /* file_version */);

            template<class Archive>
            void serialize( Archive &ar, const unsigned int file_version ){
                boost::serialization::split_member(ar, *this, file_version);
            }
        private:
            /** copy c-tor not allowed */
            NaiveBayesian(const NaiveBayesian&);
            /** assignment not allowed */
            NaiveBayesian& operator=(const NaiveBayesian&);

            //forward declaration
            class NaiveBayesianTraining;

            std::auto_ptr<NaiveBayesianTraining> impl_;

            /** \brief internal class to connect category and counter or probability

                the category collection; connection between category and attribute value.
                In learining mode used to count train examples,
                in classifier mode used to calculate probability
            */
            template<class T> class CategoryData {
            public:
                typedef std::map<AttrIdd,T> AttrData;

                CategoryData() : data_(0), attrData_() { }
                CategoryData(const T& d) : data_(d), attrData_() { }
                CategoryData(const T& d, const AttrData& ad) : data_(d), attrData_(ad) { }
                CategoryData(const CategoryData& cd) : data_(cd.data_), attrData_(cd.attrData_) { }
                CategoryData& operator=(const CategoryData& cd) {
                    data_ = cd.data_;
                    attrData_ = cd.attrData_;
                    return *this;
                }
                ~CategoryData(){}

                T data_;
                AttrData attrData_;
            private:
                /** \brief serialization using boost::serialization */
                friend class boost::serialization::access;

                template<class Archive>
                void save(Archive & ar, const unsigned int /* file_version */) const {
                    ar << boost::serialization::make_nvp("Category", data_ );
                    ar << boost::serialization::make_nvp("Data", attrData_ );
                }

                template<class Archive>
                void load(Archive & ar, const unsigned int /* file_version */) {
                    ar >> boost::serialization::make_nvp("Category", data_ );
                    typedef std::map<AttrIddSerialize,T> Map;
                    Map m;
                    ar >> boost::serialization::make_nvp("Data", m );
                    attrData_.clear();
                    for(typename Map::const_iterator ii = m.begin(); ii != m.end(); ++ii) {
                        //transform from loaded std::pair (with not const key) to stored std::pair is required
                        attrData_.insert(typename AttrData::value_type(ii->first, ii->second) );
                    }
                }

                template<class Archive>
                void serialize( Archive &ar, const unsigned int file_version ){
                    boost::serialization::split_member(ar, *this, file_version);
                }
            };


            /**
               inner class - the learning state of Naive Bayesian Classifier
            */
            class NaiveBayesianTraining {
            public:
                typedef NaiveBayesian Classifier;
                /** the data connected with each category */
                typedef std::map<AttrIdd, CategoryData<int> > CategoryCounters;
                /** the counter */
                typedef typename CategoryData<int>::AttrData SimpleCounters;

                /** for load/save */
                NaiveBayesianTraining() : parent_(0L) {}

                NaiveBayesianTraining(NaiveBayesian& parent) : parent_(&parent) {}

                virtual ~NaiveBayesianTraining() {}

                /** adds the training example, actualize counters */
                virtual void addTraining(const ExampleTrain& example);

                /** classifies the given example. Here the re-load of classifier type and re-calling the method */
                virtual AttrIdd getCategory(const ExampleTest& example) {
                    return parent_->switchGetCategory(example);
                }

                /** classifies the given example. Here the re-load of classifier type and re-calling the method */
                virtual Beliefs getCategories(const ExampleTest& example) {
                    return parent_->switchGetCategories(example);
                }

                /** the load or save state pushes re-load classifier, because internal state is stored in classify */
                virtual void loadSaveState() {
                    parent_->switchLoadSaveState();
                }

                /** the helping string */
                virtual void write(std::ostream& os) const;

                /** the counter for given category */
                int getCategoryCounter(AttrIdd cat_val) const;

                /** the counter for given category and attribute */
                int getCategoryValCounter(AttrIdd cat_val, AttrIdd value) const;
            protected:
                NaiveBayesian* parent_;
            private:
                /** \brief serialization using boost::serialization */
                friend class boost::serialization::access;
                template<class Archive>
                void serialize(Archive & ar, const unsigned int /* file_version */){
                    //state is stored in derived class NaiveBayesianTraining
                    ar & boost::serialization::make_nvp("Parent", parent_ );
                }

            private:
                /** the counters - for each category each non-zero occurence of attribute */
                CategoryCounters counters_;
                /** noncopyable */
                NaiveBayesianTraining(const NaiveBayesianTraining&);
                /** noncopyable */
                NaiveBayesianTraining& operator=(const NaiveBayesianTraining&);
            };


            /** inner class - the classify state of Naive Bayesian Classifier */
            class NaiveBayesianClasify : public NaiveBayesianTraining {
            public:
                typedef typename CategoryData<Probability>::AttrData Counters;

                typedef std::map<AttrIdd, CategoryData<Probability> > InternalProbabilities;

                //for load/save
                NaiveBayesianClasify() {}

                NaiveBayesianClasify(NaiveBayesian& parent, const NaiveBayesianTraining& nb_train) : NaiveBayesianTraining(parent) {
                    //calculate probabilities
                    calculate(nb_train);
                }

                virtual ~NaiveBayesianClasify() {}

                /** adds the training example */
                virtual void addTraining(const ExampleTrain& example) {
                    this->parent_->switchAddTraining(example);
                }

                /** classifies the given example. Using Naive Bayesian approach */
                virtual AttrIdd getCategory(const ExampleTest& example);

                /** classifies the given example. Using Naive Bayesian approach, return the AttrIdd and belief pairs */
                virtual Beliefs getCategories(const ExampleTest& example);

                /** the load or save state pushes re-load classifier, because internal state is stored in classify */
                virtual void loadSaveState() {
                    //empty operation in this context
                }

                /** the helping string */
                virtual void write(std::ostream& os) const;

                /** the probability for given category */
                Probability getCategoryCounter(AttrIdd cat_val) const;
                /** the log-probability for given category and attribute */
                Probability getCategoryCounterLog(AttrIdd cat_val) const;
                /** the probability for given category and attribute */
                Probability getCategoryValCounter(AttrIdd cat_val, AttrIdd value) const;
                /** the log-probability for given category and attribute */
                Probability getCategoryValCounterLog(AttrIdd cat_val, AttrIdd value) const;
            private:
                /** \brief serialization using boost::serialization */
                friend class boost::serialization::access;

                template<class Archive>
                void save(Archive & ar, const unsigned int /* file_version */) const;

                template<class Archive>
                void load(Archive & ar, const unsigned int /* file_version */);

                template<class Archive>
                void serialize( Archive &ar, const unsigned int file_version ){
                    boost::serialization::split_member(ar, *this, file_version);
                }
            private:
                /** the internal probabilities */
                InternalProbabilities probabl_;
                /** calculate the probabilities */
                void calculate(const NaiveBayesianTraining& nb_train);
                /** calculate log-probability for given example and category */
                Probability calcProbabilityForExample(const ExampleTest& example, AttrIdd cat_val) const;

                /** noncopyable */
                NaiveBayesianClasify(const NaiveBayesianClasify&);
                /** noncopyable */
                NaiveBayesianClasify& operator=(const NaiveBayesianClasify&);
            };


        }; //class NaiveBayesian

        //////////////////////////////////////////////////////////////////////////////////////////////////
        // class NaiveBayesian implementation
        //////////////////////////////////////////////////////////////////////////////////////////////////

        template<typename Val>
        NaiveBayesian<Val>::NaiveBayesian() : Classifier<Val>()
        {
            impl_.reset( new NaiveBayesianTraining(*this) );
        }

        template<typename Val>
        NaiveBayesian<Val>::NaiveBayesian(const Domains& attr_domains, const AttrDomain& category_domain)
            : Classifier<Val>(attr_domains, category_domain)
        {
            impl_.reset( new NaiveBayesianTraining(*this) );
        }

        /** the clear the learned parameters */
        template<typename Val>
        void NaiveBayesian<Val>::reset() {
            impl_.reset( new NaiveBayesianTraining(*this) );
        }

        /** classify (Naive Bayes Classifier) */
        template<typename Val>
        typename NaiveBayesian<Val>::AttrIdd
        NaiveBayesian<Val>::getCategory(const ExampleTest& example) const {
            return impl_->getCategory(example);
        }

        /** \brief classify and return all classes with belief that the example is from each class*/
        template<typename Val>
        typename NaiveBayesian<Val>::Beliefs
        NaiveBayesian<Val>::getCategories(const ExampleTest& example) const {
            return impl_->getCategories(example);
        }

        /** incremental learn - add training example  */
        template<typename Val>
        void NaiveBayesian<Val>::trainIncremental(const ExampleTrain& example) {
            impl_->addTraining(example);
        }

        /** ostraem method */
        template<typename Val>
        void NaiveBayesian<Val>::write(std::ostream& os) const {
            Classifier<Val>::write(os);
            os << std::endl << "State: ";
            impl_->write(os);
            os << std::endl;
        }

        /** the internal state - load */
        template<typename Val>
        void NaiveBayesian<Val>::loadSaveState() const {
            impl_->loadSaveState();
        }

        /** change the internal obj to classify add return result of classification */
        template<typename Val>
        typename NaiveBayesian<Val>::AttrIdd
        NaiveBayesian<Val>::switchGetCategory(const ExampleTest& example) {
            NaiveBayesianClasify* classify = new NaiveBayesianClasify(*this, *impl_.get());
            impl_.reset( classify );
            return getCategory(example); //re-call the method
        }

        /** change the internal obj to classify add return result of classification */
        template<typename Val>
        typename NaiveBayesian<Val>::Beliefs
        NaiveBayesian<Val>::switchGetCategories(const ExampleTest& example) {
            NaiveBayesianClasify* classify = new NaiveBayesianClasify(*this, *impl_.get());
            impl_.reset( classify );
            return getCategories(example); //re-call the method
        }

        /** change the internal obj to train, clear the train, and add new example */
        template<typename Val>
        void NaiveBayesian<Val>::switchAddTraining(const ExampleTrain& example) {
            reset();
            trainIncremental(example); //re-call the method
        }

        /** change the internal obj to classify and return the internal state */
        template<typename Val>
        void NaiveBayesian<Val>::switchLoadSaveState() {
            NaiveBayesianClasify* classify = new NaiveBayesianClasify(*this, *impl_.get());
            impl_.reset( classify );
        }


        template<typename Val>
        template<class Archive>
        void NaiveBayesian<Val>::save(Archive & ar, const unsigned int /* file_version */) const {
            ar.template register_type<NaiveBayesianClasify>();
            ar << boost::serialization::make_nvp("NBCBase", boost::serialization::base_object<Classifier<Val> >(*this) );
            loadSaveState(); //change state to NaiveBayesianClassify, because only there are counters
            const NaiveBayesianTraining* const t = impl_.get();
            ar << boost::serialization::make_nvp("NBCImpl",t); //only raw pointer is stored
        }

        template<typename Val>
        template<class Archive>
        void NaiveBayesian<Val>::load(Archive & ar, const unsigned int /* file_version */) {
            ar.template register_type<NaiveBayesianClasify>();
            ar >> boost::serialization::make_nvp("NBCBase", boost::serialization::base_object<Classifier<Val> >(*this) );
            NaiveBayesianTraining* t;
            ar >> boost::serialization::make_nvp("NBCImpl",t); //restore raw pointer
            impl_.reset(t);
        }

        //////////////////////////////////////////////////////////////////////////////////////////////////
        // helping classes to print counters
        // used by  NaiveBayesian::NaiveBayesianTraining and NaiveBayesian::NaiveBayesianClassify
        //////////////////////////////////////////////////////////////////////////////////////////////////

        /** \brief class to show the classifier state, print the attribs and counters */
        template<class Categories>
        struct PrintCountAttrFunctor {

            typedef typename Categories::Classifier Cl;
            typedef typename Cl::AttrIdd AttrIdd;
            typedef typename Cl::AttrDomain AttrDomain;
            typedef typename  Cl::Domains Domains;
            typedef typename Cl::ExamplesTrain ExamplesTrain;

            std::ostream& os_;
            AttrIdd catVal_;
            const Categories& categories_;

            PrintCountAttrFunctor(std::ostream& os, AttrIdd cat_val, const Categories& categories)
                : os_(os), catVal_(cat_val), categories_(categories) {
            }
            void operator()(const AttrDomain& attr) {
                for(typename AttrDomain::const_iterator ii = attr.begin(); ii != attr.end(); ++ii ) {
                    AttrIdd val = &(*ii);
                    // drukuje licznik dla danej wartosci w danej kategorii
                    os_ << val->get() << "(" << categories_.getCategoryValCounter(catVal_, val) << "),";// << " addr:" << val << ' ';
                }
                os_ << std::endl;
            }
        private:
            PrintCountAttrFunctor& operator=(const PrintCountAttrFunctor&); //not allwed because references
        };

        /** \brief print the cauters for given category */
        template<class Categories>
        struct PrintCountersFunctor {

            typedef typename Categories::Classifier Cl;
            typedef typename Cl::AttrIdd AttrIdd;
            typedef typename Cl::AttrDomain AttrDomain;
            typedef typename Cl::Domains Domains;
            typedef typename Cl::ExamplesTrain ExamplesTrain;
			typedef typename Cl::Value Value;

            std::ostream& os_;
            const Domains& attributes_;
            const Categories& categories_;

            PrintCountersFunctor( std::ostream& os, const Domains& attrib, const Categories& categories)
                : os_(os), attributes_(attrib), categories_(categories) {
            }

			//void operator()(const ValueNominalString& cat_val) {
			void operator()(const Value& cat_val) {
                os_ << cat_val << "(" << categories_.getCategoryCounter(&cat_val) << "):" << std::endl;
                PrintCountAttrFunctor<Categories> printCountAttr(os_, &cat_val, categories_);
                std::for_each( attributes_.begin(), attributes_.end(), printCountAttr );
            }
        private:
            PrintCountersFunctor& operator=(const PrintCountersFunctor&); //zabronione przypisanie bo skladowe referencyjne
        };

        //////////////////////////////////////////////////////////////////////////////////////////////////
        // class NaiveBayesian::NaiveBayesianTraining implementation
        //////////////////////////////////////////////////////////////////////////////////////////////////

        /** adds the training example, actualize counters */
        template<typename Val>
        void NaiveBayesian<Val>::NaiveBayesianTraining::addTraining(const ExampleTrain& example) {
            AttrIdd cat_val = example.getFeature();
            typename CategoryCounters::iterator ii = counters_.find(cat_val);
            if( ii != counters_.end() )
                ++(*ii).second.data_;
            else
                ii = counters_.insert( typename CategoryCounters::value_type(cat_val,1) ).first;
            assert( ii != counters_.end() );
            SimpleCounters& count = (*ii).second.attrData_;
            for(typename ExampleTrain::const_iterator i = example.begin(); i != example.end(); ++i ) {
                AttrIdd value = *i;
                typename SimpleCounters::iterator iii = count.find(value);
                if( iii != count.end() )
                    ++(*iii).second; //zwieksza odpowiedni licznik
                else
                    count.insert( std::pair<AttrIdd,int>(value,1) );
            }
        }

        /** ostream method */
        template<typename Val>
        void NaiveBayesian<Val>::NaiveBayesianTraining::write(std::ostream& os) const {
            os << "TRAINING:" << std::endl;
            PrintCountersFunctor<NaiveBayesianTraining> printCounters(os, parent_->getAttrDomains(), *this );
            std::for_each(parent_->getCategoryDomain().begin(), parent_->getCategoryDomain().end(), printCounters );

        }

        /** the counter for given category */
        template<typename Val>
        int NaiveBayesian<Val>::NaiveBayesianTraining::getCategoryCounter(AttrIdd cat_val) const {
            typename CategoryCounters::const_iterator ii = counters_.find(cat_val);
            if( ii != counters_.end() )
                return (*ii).second.data_;
            else
                return 0;
        }

        /** the counter for given category and attribute */
        template<typename Val>
        int NaiveBayesian<Val>::NaiveBayesianTraining::getCategoryValCounter(AttrIdd cat_val, AttrIdd value) const {
            typename CategoryCounters::const_iterator ii = counters_.find(cat_val);
            if( ii == counters_.end() )
                return 0;
            const SimpleCounters& count = (*ii).second.attrData_;
            //przeszukuje dana kategorie;
            typename SimpleCounters::const_iterator jj = count.find(value);
            if( jj != count.end() )
                return (*jj).second;
            else
                return 0;
        }

        //////////////////////////////////////////////////////////////////////////////////////////////////
        // class NaiveBayesian::NaiveBayesianClasify implementation
        //////////////////////////////////////////////////////////////////////////////////////////////////

        /** classifies the given example. Using Naive Bayesian approach */
        template<typename Val>
        typename NaiveBayesian<Val>::AttrIdd
        NaiveBayesian<Val>::NaiveBayesianClasify::getCategory(const ExampleTest& example) {
            if( probabl_.empty() )
                return AttrDomain::getUnknownId();

            AttrIdd cat_val_max = probabl_.begin()->first; //init not important
            Probability max_prob = -std::numeric_limits<Probability>::max();
            //look the categories and find the max prob of category for given example (compares the log of probability)
            for(typename InternalProbabilities::const_iterator ii = probabl_.begin(); ii != probabl_.end(); ++ii ) {
                AttrIdd cat_val = (*ii).first;
                Probability prob = calcProbabilityForExample(example, cat_val);
                if( prob > max_prob ) {
                    max_prob = prob;
                    cat_val_max = cat_val;
                }
            }
            return cat_val_max;
        }

        /** classifies the given example. Using Naive Bayesian approach, return the AttrIdd and belief pairs */
        template<typename Val>
        typename NaiveBayesian<Val>::Beliefs
        NaiveBayesian<Val>::NaiveBayesianClasify::getCategories(const ExampleTest& example) {
            Probability sum = 0.0;
            for(typename InternalProbabilities::const_iterator ii = probabl_.begin(); ii != probabl_.end(); ++ii ) {
                sum += std::exp( calcProbabilityForExample(example, (*ii).first ) );
            }

            Beliefs toRet;
            for(typename InternalProbabilities::const_iterator ii = probabl_.begin(); ii != probabl_.end(); ++ii ) {
                AttrIdd cat_val = (*ii).first;
                Probability prob = exp( calcProbabilityForExample(example, cat_val) ) / sum;
                toRet.push_back(typename Beliefs::value_type(cat_val, prob));
            }
            std::sort( toRet.begin(), toRet.end() );
            return toRet;
        }

        /** ostream method */
        template<typename Val>
        void NaiveBayesian<Val>::NaiveBayesianClasify::write(std::ostream& os) const {
            os << "CLASIFY:" << std::endl;
            PrintCountersFunctor<NaiveBayesianClasify> printCounters(os, this->parent_->getAttrDomains(), *this );
            std::for_each(this->parent_->getCategoryDomain().begin(), this->parent_->getCategoryDomain().end(), printCounters );
        }

        /** the probability for given category */
        template<typename Val>
        Probability NaiveBayesian<Val>::NaiveBayesianClasify::getCategoryCounter(AttrIdd cat_val) const {
            return std::exp( getCategoryCounterLog(cat_val) );
        }

        /** the log-probability for given category and attribute */
        template<typename Val>
        Probability NaiveBayesian<Val>::NaiveBayesianClasify::getCategoryCounterLog(AttrIdd cat_val) const {
            typename InternalProbabilities::const_iterator ii = probabl_.find(cat_val);
            if( ii != probabl_.end() )
                return (*ii).second.data_;
            else
                return 0.0;
        }

        /** the probability for given category and attribute */
        template<typename Val>
        Probability NaiveBayesian<Val>::NaiveBayesianClasify::getCategoryValCounter(AttrIdd cat_val, AttrIdd value) const {
            return exp( getCategoryValCounterLog(cat_val,value) );
        }

        /** the log-probability for given category and attribute */
        template<typename Val>
        Probability NaiveBayesian<Val>::NaiveBayesianClasify::getCategoryValCounterLog(AttrIdd cat_val, AttrIdd value) const {
            typename InternalProbabilities::const_iterator ii = probabl_.find(cat_val);
            if( ii == probabl_.end() )
                return 0.0;

            const Counters& counters = (*ii).second.attrData_;
            //przeszukuje dana kategorie;
            typename Counters::const_iterator jj = counters.find(value);
            if( jj != counters.end() )
                return (*jj).second;
            else
                return 0.0;
        }

        namespace {
            /** helping - calculates the probability of given value, laplace smoothing (m-szacowanie) and log */
            Probability calcProbability( int val_count, int count_all, int val_size) {
                return std::log((val_count + 1)/ (Probability)(count_all + val_size ));
            }

        } //namespace


        /** calculate the probabilities */
        template<typename Val>
        void NaiveBayesian<Val>::NaiveBayesianClasify::calculate(const NaiveBayesianTraining& nb_train) {

            //calculates the sum of category counters
            int sumTraining = 0;
            const AttrDomain& category = this->parent_->getCategoryDomain();
            for(typename AttrDomain::const_iterator ii = category.begin(); ii != category.end(); ++ii ) {
                AttrIdd catVal = AttrDomain::getValueId(ii);
                sumTraining += nb_train.getCategoryCounter(catVal);
            }

            //calculate probability fo each attribute value
            int catSize = category.getSize();
            for(typename AttrDomain::const_iterator ii = category.begin(); ii != category.end(); ++ii ) {
                AttrIdd catVal = AttrDomain::getValueId(ii);
                int catCounter = nb_train.getCategoryCounter(catVal);
                Probability catProb = calcProbability( catCounter, catSize, sumTraining );
                Counters counters; //empty counters
                const Domains& attribs = this->parent_->getAttrDomains();
                for(typename Domains::const_iterator jj = attribs.begin(); jj != attribs.end(); ++jj) {
                    const AttrDomain& attr = *jj;
                    int valSize = attr.getSize(); //number values for given attribute (for Laplace' smoothing)
                    for(typename AttrDomain::const_iterator kk = attr.begin(); kk != attr.end(); ++kk) {
                        AttrIdd val = AttrDomain::getValueId(kk);
                        Probability valProb = calcProbability( nb_train.getCategoryValCounter(catVal, val), valSize, catCounter );
                        counters.insert( typename Counters::value_type(val, valProb) );
                    }
                }
                probabl_.insert( typename InternalProbabilities::value_type(catVal, CategoryData<Probability>(catProb, counters) ) );
            }
        }

        /** calculate log-probability for given example and category */
        template<typename Val>
        Probability NaiveBayesian<Val>::NaiveBayesianClasify::calcProbabilityForExample(const ExampleTest& example, AttrIdd cat_val) const {
            Probability prob = getCategoryCounterLog(cat_val);
            for(typename ExampleTest::const_iterator ii = example.begin(); ii !=  example.end(); ++ii )
                prob += getCategoryValCounterLog(cat_val, *ii );
            return prob;
        }

        /** \brief serialization using boost::serialization */
        template<typename Val>
        template<class Archive>
        void NaiveBayesian<Val>::NaiveBayesianClasify::save(Archive & ar, const unsigned int /* file_version */) const {
            ar << boost::serialization::make_nvp("Base", boost::serialization::base_object<NaiveBayesianTraining>(*this));
            ar << boost::serialization::make_nvp("InternalProb",probabl_);
        }

        /** \brief serialization using boost::serialization */
        template<typename Val>
        template<class Archive>
        void NaiveBayesian<Val>::NaiveBayesianClasify::load(Archive & ar, const unsigned int /* file_version */) {
            ar >> boost::serialization::make_nvp("Base", boost::serialization::base_object<NaiveBayesianTraining>(*this));
            typedef std::map<AttrIddSerialize, CategoryData<Probability> > Map;
            Map m;
            ar >> boost::serialization::make_nvp("InternalProb",m);
            probabl_.clear();
            for(typename Map::const_iterator ii = m.begin(); ii != m.end(); ++ii) {
                probabl_.insert( typename InternalProbabilities::value_type(ii->first, ii->second) );
		    }
	    }

    } //namespace ml
} //namespace faif

#endif //FAIF_NAIVE_BAYESIAN_HPP_
