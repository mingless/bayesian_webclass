/**
 * \file Svm.hpp
 * \brief The Support Vector Machine Classifier
 */

#ifndef FAIF_SVM_HPP_
#define FAIF_SVM_HPP_

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
//msvc14.0 warnings for Boost.Serialization
#pragma warning(disable:4100)
#pragma warning(disable:4512)
#endif


#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <vector>
#include <utility>
#include <math.h>
#include <ctime>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>

#include "../Value.hpp"
#include "../Point.hpp"
#include "../utils/Random.hpp"
#include "Belief.hpp"

namespace faif {
    namespace ml {

        /** Support vector machine classifier class */
        template <typename Val, typename DomainVal>
        class SvmClassifier {

        public:

            /** \brief serialization using boost::serialization */
            friend class boost::serialization::access;

            template<class Archive>
            void serialize( Archive &ar, const unsigned int file_version ){
                boost::serialization::split_member(ar, *this, file_version);
            }

            template<class Archive>
            void save(Archive & ar, const unsigned int /* file_version */) const {
                ar & boost::serialization::make_nvp("Dimension", dimension );
                ar & boost::serialization::make_nvp("TrainingExamples", trainingExamples );
                ar & boost::serialization::make_nvp("Smo", smo );
                ar & boost::serialization::make_nvp("Categories", categories);

            }

            template<class Archive>
            void load(Archive & ar, const unsigned int /* file_version */) {
                ar >> boost::serialization::make_nvp("Dimension", dimension );
                ar >> boost::serialization::make_nvp("TrainingExamples", trainingExamples );
                ar >> boost::serialization::make_nvp("Smo", smo );
                ar >> boost::serialization::make_nvp("Categories", categories);
            }

            /** Classify example is n-dimensional vector */
            typedef typename std::vector<Val> ClassifyExample;

            /** The attribute domain for learning */
            typedef typename DomainVal::DomainType AttrDomain;

            /** Attribute value representation in learning */
            typedef typename DomainVal::Value AttrValue;

            /** Collection of pair (AttrIdd, Probability) */
            typedef typename Belief<DomainVal>::Beliefs Beliefs;

        private:

            /** Dimension is the number of attributes used at classification, i.e. dimension of train/classify vectors */
            size_t dimension;

            /** Sequential Minimal Optimization algorithm */
            class SmoAlgorithm {

            public:

                /** Classify example is n-dimensional real vector */
                typedef typename boost::numeric::ublas::vector<Val> ClassifyExampleSmo;

                /** Train example is pair of category (1 or -1) and n-dimensional vector */
                typedef typename std::pair < int, ClassifyExampleSmo > TrainExampleSmo;

                enum Kernel_type{
                    default_type,
                    gauss_type,
                    linear_type,
                    polynomial_type,
                    hyperbolic_tangent_type
                };

                friend class SvmClassifier<Val, DomainVal>;

            private:

                /** \brief serialization using boost::serialization */
                friend class boost::serialization::access;

                /** Serialize function */
                template<class Archive>
                void serialize( Archive &ar, const unsigned int file_version ){
                    boost::serialization::split_member(ar, *this, file_version);
                }

                /** Serialize function */
                template<class Archive>
                void save(Archive & ar, const unsigned int /* file_version */) const {
                    ar & boost::serialization::make_nvp("C", C );
                    ar & boost::serialization::make_nvp("margin", margin );
                    ar & boost::serialization::make_nvp("epsilon", epsilon );
                    ar & boost::serialization::make_nvp("b", b );
                    ar & boost::serialization::make_nvp("delta_b", delta_b );
                    ar & boost::serialization::make_nvp("gaussParameter", gaussParameter );
                    ar & boost::serialization::make_nvp("polynomialInhomogeneousParameter", polynomialInhomogeneousParameter );
                    ar & boost::serialization::make_nvp("polynomialDegree", polynomialDegree );
                    ar & boost::serialization::make_nvp("tangentFrequency", tangentFrequency );
                    ar & boost::serialization::make_nvp("tangentShift", tangentShift );
                    ar & boost::serialization::make_nvp("sigmoidScaleFactor", sigmoidScaleFactor );
                    ar & boost::serialization::make_nvp("finiteStopCondition", finiteStopCondition );
                    ar & boost::serialization::make_nvp("stepsStopCondition", stepsStopCondition );
                    ar & boost::serialization::make_nvp("alpha", alpha );
                    ar & boost::serialization::make_nvp("error_cache", error_cache );
                    ar & boost::serialization::make_nvp("trainingExamples", trainingExamples );
                    ar & boost::serialization::make_nvp("kernelType", kernel_type );
                }

                /** Serialize function */
                template<class Archive>
                void load(Archive & ar, const unsigned int /* file_version */) {
                    ar >> boost::serialization::make_nvp("C", C );
                    ar >> boost::serialization::make_nvp("margin", margin );
                    ar >> boost::serialization::make_nvp("epsilon", epsilon );
                    ar >> boost::serialization::make_nvp("b", b );
                    ar >> boost::serialization::make_nvp("delta_b", delta_b );
                    ar >> boost::serialization::make_nvp("gaussParameter", gaussParameter );
                    ar >> boost::serialization::make_nvp("polynomialInhomogeneousParameter", polynomialInhomogeneousParameter );
                    ar >> boost::serialization::make_nvp("polynomialDegree", polynomialDegree );
                    ar >> boost::serialization::make_nvp("tangentFrequency", tangentFrequency );
                    ar >> boost::serialization::make_nvp("tangentShift", tangentShift );
                    ar >> boost::serialization::make_nvp("sigmoidScaleFactor", sigmoidScaleFactor );
                    ar >> boost::serialization::make_nvp("finiteStopCondition", finiteStopCondition );
                    ar >> boost::serialization::make_nvp("stepsStopCondition", stepsStopCondition );
                    ar >> boost::serialization::make_nvp("alpha", alpha );
                    ar >> boost::serialization::make_nvp("error_cache", error_cache );
                    ar >> boost::serialization::make_nvp("trainingExamples", trainingExamples );
                    ar >> boost::serialization::make_nvp("kernelType", kernel_type );
                    setKernel(kernel_type);
                }

                /** Gauss kernel function */
                Val kernel_gauss(const ClassifyExampleSmo& vec1, const ClassifyExampleSmo& vec2);

                /** Linear kernel function */
                Val kernel_linear(const ClassifyExampleSmo& vec1, const ClassifyExampleSmo& vec2);

                /** Polynomial kernel function */
                Val kernel_polynomial(const ClassifyExampleSmo& vec1, const ClassifyExampleSmo& vec2);

                /** Tanh(x) kernel function */
                Val kernel_hyperbolic_tangent(const ClassifyExampleSmo& vec1, const ClassifyExampleSmo& vec2);

                /** Pointer to kernel function */
                Val (faif::ml::SvmClassifier<Val, DomainVal>::SmoAlgorithm::*kernel) (const ClassifyExampleSmo&, const ClassifyExampleSmo&);

                /** Enum representing which kernel is used for SMO algorithm */
                Kernel_type kernel_type;

                /** Setting kernel type */
                void setKernel(Kernel_type);

                /** Sigmoid function, used for scaling classification result from ]-inf;inf[ to [0;1] */
                Val sigmoid_function(Val);

                /** Lagrangian multipliers */
                std::vector<Val> alpha;

                /** SVM threshold (hyperplane) */
                Val b;

                /** b error */
                Val delta_b;

                /** SVM parameter, additional constraint for soft margin */
                Val C;

                /** Parameter used for stop condition (check if violates KKT condition)*/
                Val margin;

                /** Parameter used for stop condition (check if violates KKT condition)*/
                Val epsilon;

                /** Parameter of gaussian kernel function: exp(-gaussParameter*||x1-x2||)*/
                Val gaussParameter;

                /** Parameter 'c' of polynomial kernel function: (x1.x2 + c)^d */
                Val polynomialInhomogeneousParameter;

                /** Parameter 'd' of polynomial kernel function: (x1.x2 + c)^d */
                Val polynomialDegree;

                /** Parameter 'w' of hyperbolic tangent kernel function: tanh(w*x1.x2 - c) */
                Val tangentFrequency;

                /** Parameter 'c' of hyperbolic tangent kernel function: tanh(w*x1.x2 - c) */
                Val tangentShift;

                /** If finiteStopConditon is true SMO algorithm stops after stepsStopCondition*numExamples steps */
                bool finiteStopCondition;
                double stepsStopCondition;

                /** Parameter 'p' of sigmoid function 1 / ( 1 + exp(-px) ), should be > 0*/
                Val sigmoidScaleFactor;

                /** Cache error vector */
                std::vector<Val> error_cache;

                /** Vector of all training examples */
                std::vector< TrainExampleSmo > trainingExamples;

                /** Calculate current classification function for i-th training vector */
                Val learned_func(size_t i);

                /** Optimizes Lagrangian multipliers i1 and i2, return 1 if succes, else return 0 */
                size_t optimizeTwoAlphas(size_t i1, size_t i2);

                /** Checks KKT condition is violated by more than margin for alpha[i1]. If so, looks for 2nd multiplier and jointly optimize multipliers using optimizeTwoAlphas function */
                size_t examineKKTViolation(size_t i1);

                /** Restrict copy */
                SmoAlgorithm(const SmoAlgorithm& smo);
                const SmoAlgorithm& operator=(const SmoAlgorithm& smo);

                /**C-tor, initialize parameters*/
                SmoAlgorithm(Kernel_type _kernel_type = default_type) : kernel_type(_kernel_type), b(0.0), C(2.0), margin(0.001), epsilon(0.000000000001), gaussParameter(1.0), polynomialInhomogeneousParameter(0.0), polynomialDegree(2.0), tangentFrequency(1.0), tangentShift(0.0), finiteStopCondition(true), stepsStopCondition(1.0), sigmoidScaleFactor(0.1) {
                    // Initialize kernel function
                    setKernel(kernel_type);
                }

                /** Use train example to train svm classifier */
                void train(std::vector<typename SmoAlgorithm::TrainExampleSmo> _trainingExamples);

                /** Classify example (return positive or negative number) based on what have been trained */
                Val classify(const ClassifyExampleSmo& vec);

                /** Erase all the added train examples added to svm classifier */
                void reset();

                /** Set parameter C, should be >0 */
                void setC(Val C);

                /** Set parameter margin, should be >0 */
                void setMargin(Val margin);

                /** Set parameter epsilon, should be >0 */
                void setEpsilon(Val epsilon);

                /** Set gaussian parameter t : exp(-t*||x1-x2||), should be >0 */
                void setGaussParameter(Val gaussParameter);

                /** Set parameter 'c' of hyperbolic tangent kernel function: tanh(w*x1.x2 - c) */
                void setPolynomialInhomogeneousParameter(Val polynomialInhomogeneousParameter);

                /** Set parameter 'd' of polynomial kernel function: (x1.x2 + c)^d */
                void setPolynomialDegree(Val polynomialDegree);

                /** Set parameter 'w' of hyperbolic tangent kernel function: tanh(w*x1.x2 - c) , should be > 0 */
                void setTangentFrequency(Val tangentFrequency);

                /** Set parameter 'c' of polynomial kernel function: (x1.x2 + c)^d , should be >= 0 */
                void setTangentShift(Val tangentShift);

                /** Set stop condition for SMO algorithm: when N training examples, SMO stops after stop*N steps. stop should be >0 */
                void setFiniteStepsStopCondition(double stop);

                /** Unset stop condition for SMO algorithm  */
                void unsetFiniteStepsStopCondition();

                /** Set parameter 'p' of sigmoid function: 1 / ( 1 + exp(-px) ) */
                void setSigmoidScaleFactor(Val sigmoidScaleFactor);

            };//class SmoAlgorithm

            /** Categories used for examples labeling (svm assumption: two categories only)*/
            AttrDomain categories;

            /** Instance of smo algorithm */
            SmoAlgorithm smo;

            /** Vector of training vectors with corresponding category used for classifier training*/
            std::vector< typename SmoAlgorithm::TrainExampleSmo > trainingExamples;

            /** Restrict copy */
            SvmClassifier(const SvmClassifier& svm);
            const SvmClassifier& operator=(const SvmClassifier& svm);

            /** Classify example (return positive or negative number) based on what have been trained */
            Val classify(const ClassifyExample& vec);

            /** Convert std::vector with train/classify example to boost::numeric::ublas vector to achieve better time performance */
            typename SmoAlgorithm::ClassifyExampleSmo convertVectorToUblas(const ClassifyExample&);

        public:

            /** Empty c-tor */
            SvmClassifier() : dimension(0) {} ;

            /** C-tor creates svm classifier for given dimensionality of a problem */
            SvmClassifier(size_t dimension_, const AttrDomain& category_domain) : dimension(dimension_) {
                //Svm classifier works for two class problems only
                if(category_domain.getSize()==2)
                    categories = category_domain;
                else
                    throw std::domain_error("For SVM classifier category domain should have exactly 2 classes");
            };

            /** Add train example with known category to svm classifier */
            void addExample(const ClassifyExample& example, const AttrValue& category);

            /** Classify and return all classes (svm assumption: two classes) with belief that the example is from given class */
            Beliefs getCategories(const ClassifyExample& vec);

            /** Classify and return the belief of the most probable class */
            Belief<DomainVal> getCategory(const ClassifyExample& vec);

            /** Use train example to train svm classifier */
            void train();

            /** Return dimensionality of a svm classifier */
            size_t getDimension();

            /** Return the number of train examples added to svm classifier */
            size_t countTrainExamples();

            /** Erase all the added train examples added to svm classifier */
            void reset();

            /** Erase all the added train examples added to svm classifier and change dimension of classifier*/
            void resetAndChangeDimension(size_t);

            /** Set parameter C, should be >0 */
            void setC(Val C);

            /** Set parameter margin, should be >0 */
            void setMargin(Val margin);

            /** Set parameter epsilon, should be >0 */
            void setEpsilon(Val epsilon);

            /** Set gaussian parameter t : exp(-t*||x1-x2||), should be >0 */
            void setGaussParameter(Val gaussParameter);

            /** Set parameter 'c' of hyperbolic tangent kernel function: tanh(w*x1.x2 - c) */
            void setPolynomialInhomogeneousParameter(Val polynomialInhomogeneousParameter);

            /** Set parameter 'd' of polynomial kernel function: (x1.x2 + c)^d */
            void setPolynomialDegree(Val polynomialDegree);

            /** Set parameter 'w' of hyperbolic tangent kernel function: tanh(w*x1.x2 - c) , should be > 0 */
            void setTangentFrequency(Val tangentFrequency);

            /** Set parameter 'c' of polynomial kernel function: (x1.x2 + c)^d , should be >= 0 */
            void setTangentShift(Val tangentShift);

            /** Set stop condition for SMO algorithm: when N training examples, SMO stops after stop*N steps. stop should be >0 */
            void setFiniteStepsStopCondition(double stop);

            /** Unset stop condition for SMO algorithm  */
            void unsetFiniteStepsStopCondition();

            /** Set parameter 'p' of sigmoid function: 1 / ( 1 + exp(-px) ), should be > 0 */
            void setSigmoidScaleFactor(Val sigmoidScaleFactor);

            /** Set linear kernel for SMO algorithm */
            void setLinearKernel();

            /** Set gaussian kernel for SMO algorithm */
            void setGaussKernel();

            /** Set polynomial kernel for SMO algorithm */
            void setPolynomialKernel();

            /** Set hyperbolic tangent kernel for SMO algorithm */
            void setHyperbolicTangentKernel();

        }; //class SvmClassifier



//////////////////////////////////////////
/*     SvmClassifier Implementation     */
//////////////////////////////////////////

        template <typename Val, typename DomainVal>
        typename SvmClassifier<Val, DomainVal>::SmoAlgorithm::ClassifyExampleSmo SvmClassifier<Val, DomainVal>::convertVectorToUblas(const ClassifyExample& vec) { 
            typename SmoAlgorithm::ClassifyExampleSmo toRet(vec.size());
            for(size_t i=0; i<vec.size(); ++i)
                toRet(i) = vec[i];
            return toRet;
        }

        template <typename Val, typename DomainVal>
        Val SvmClassifier<Val, DomainVal>::classify(const ClassifyExample& vec) { 
            //Convert example to ublas::vector and classify
            return smo.classify( convertVectorToUblas( vec ) );
        }

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::addExample(const ClassifyExample& example, const AttrValue& category){
            try{
                // Find category in categories, if not found then NotFoundException
                categories.find(category);
                if(example.size() == this->dimension){ /* Size of train example and dimensionality of svm must be equal */
                    // Svm assumption: two category classification
                    // Another assumption: first category correspond to svm class y_i = +1, second category to -1
                    // Convert example to ublas::vector when add to train examples
                    if(category==categories.begin()->get())
                        this->trainingExamples.push_back(std::make_pair(1, convertVectorToUblas(example) ));
                    else
                        this->trainingExamples.push_back(std::make_pair(-1, convertVectorToUblas(example) ));
                }
            }
            catch(NotFoundException ex){//If example's category is unknown, the example is not added to train examples
                throw std::domain_error("Train example's category: "+category+" is not in classifier's domain.");
            }
        }

        template <typename Val, typename DomainVal>
        typename SvmClassifier<Val, DomainVal>::Beliefs SvmClassifier<Val, DomainVal>::getCategories(const ClassifyExample& vec){
            // Svm assumption: two category classification

            // Classify example, the classify_value is probability of first category
            auto classify_value = this->classify(vec);

            Beliefs toRet;

            // Push the pair of the first category and classification result (probability) to returned Beliefs
            auto it = categories.begin();
            toRet.push_back(typename Beliefs::value_type(it->getDomain()->getValueId(it), classify_value));

            // Push the pair of the second category and 1.0-probability to returned Beliefs
            ++it;
            toRet.push_back(typename Beliefs::value_type(it->getDomain()->getValueId(it), 1.0-classify_value));

            // Returned sorted Beliefs
            std::sort( toRet.begin(), toRet.end() );
            return toRet;
        }

        template <typename Val, typename DomainVal>
        Belief<DomainVal> SvmClassifier<Val, DomainVal>::getCategory(const ClassifyExample& vec){
            return getCategories(vec)[0];
        }

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::train(){ smo.train(this->trainingExamples);}

        template <typename Val, typename DomainVal>
        size_t SvmClassifier<Val, DomainVal>::getDimension(){ return this->dimension;}

        template <typename Val, typename DomainVal>
        size_t SvmClassifier<Val, DomainVal>::countTrainExamples(){ return trainingExamples.size();}

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::reset() { trainingExamples.clear(); smo.reset();}

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::resetAndChangeDimension(size_t _dimension) { trainingExamples.clear(); smo.reset(); this->dimension = _dimension;}

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::setC(Val C){ smo.setC(C);}

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::setMargin(Val margin){ smo.setMargin(margin);}

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::setEpsilon(Val epsilon){ smo.setEpsilon(epsilon);}

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::setGaussParameter(Val gaussParameter){ smo.setGaussParameter(gaussParameter);}

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::setPolynomialInhomogeneousParameter(Val p){ smo.setPolynomialInhomogeneousParameter(p);}

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::setPolynomialDegree(Val polynomialDegree){smo.setPolynomialDegree(polynomialDegree);}

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::setTangentFrequency(Val f){ smo.setTangentFrequency(f);}

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::setTangentShift(Val tangentShift){ smo.setTangentShift(tangentShift);}

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::setFiniteStepsStopCondition(double stop){ smo.setFiniteStepsStopCondition(stop);}

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::unsetFiniteStepsStopCondition(){ smo.unsetFiniteStepsStopCondition();}

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::setSigmoidScaleFactor(Val sigmoidScaleFactor){ smo.setSigmoidScaleFactor(sigmoidScaleFactor);}

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::setLinearKernel(){ smo.setKernel(SmoAlgorithm::Kernel_type::linear_type);}

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::setGaussKernel(){
            smo.setKernel(SmoAlgorithm::Kernel_type::gauss_type);
        }

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::setPolynomialKernel(){
            smo.setKernel(SmoAlgorithm::Kernel_type::polynomial_type);
        }

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::setHyperbolicTangentKernel(){
            smo.setKernel(SmoAlgorithm::Kernel_type::hyperbolic_tangent_type);
        }


//////////////////////////////////////////
/*     SmoAlgorithm Implementation      */
//////////////////////////////////////////


        template <typename Val, typename DomainVal>
        Val SvmClassifier<Val, DomainVal>::SmoAlgorithm::kernel_gauss(const ClassifyExampleSmo& vec1, const ClassifyExampleSmo& vec2){
            assert(vec1.size() == vec2.size());
            Val squared_euclidean_distance = boost::numeric::ublas::sum(boost::numeric::ublas::element_prod(vec1-vec2, vec1-vec2));
            return exp(-gaussParameter*squared_euclidean_distance);
        }

        template <typename Val, typename DomainVal>
        Val SvmClassifier<Val, DomainVal>::SmoAlgorithm::kernel_linear(const ClassifyExampleSmo& vec1, const ClassifyExampleSmo& vec2){
            assert(vec1.size() == vec2.size());
            return boost::numeric::ublas::sum(boost::numeric::ublas::element_prod(vec1, vec2));
        }

        template <typename Val, typename DomainVal>
        Val SvmClassifier<Val, DomainVal>::SmoAlgorithm::kernel_polynomial(const ClassifyExampleSmo& vec1, const ClassifyExampleSmo& vec2){
            assert(vec1.size() == vec2.size());
            return pow(kernel_linear(vec1, vec2) + this->polynomialInhomogeneousParameter, this->polynomialDegree);
        }

        template <typename Val, typename DomainVal>
        Val SvmClassifier<Val, DomainVal>::SmoAlgorithm::kernel_hyperbolic_tangent(const ClassifyExampleSmo& vec1, const ClassifyExampleSmo& vec2){
            assert(vec1.size() == vec2.size());
            return tanh(this->tangentFrequency * kernel_linear(vec1, vec2) - this->tangentShift);
        }

        template <typename Val, typename DomainVal>
        Val SvmClassifier<Val, DomainVal>::SmoAlgorithm::sigmoid_function(Val x){
            return 1.0 / (1.0 + exp( (-this->sigmoidScaleFactor) * x) );
        }

        template <typename Val, typename DomainVal>
        Val SvmClassifier<Val, DomainVal>::SmoAlgorithm::learned_func(size_t i){
            Val result = 0.0;
            size_t numExamples = trainingExamples.size();
            for(size_t j = 0; j<numExamples; ++j)
                if(alpha[i]>0)
                    result += alpha[i]*trainingExamples[i].first*((this->*kernel)(trainingExamples[i].second, trainingExamples[j].second));
            result -= b;
            return result;
        }

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::SmoAlgorithm::setKernel(Kernel_type kernel_type_in){
            this->kernel_type = kernel_type_in;
            switch(kernel_type){
                case gauss_type:
                    faif::ml::SvmClassifier<Val, DomainVal>::SmoAlgorithm::kernel = &faif::ml::SvmClassifier<Val, DomainVal>::SmoAlgorithm::kernel_gauss;
                    break;
                case linear_type:
                    faif::ml::SvmClassifier<Val, DomainVal>::SmoAlgorithm::kernel = &faif::ml::SvmClassifier<Val, DomainVal>::SmoAlgorithm::kernel_linear;
                    break;
                case polynomial_type:
                    faif::ml::SvmClassifier<Val, DomainVal>::SmoAlgorithm::kernel = &faif::ml::SvmClassifier<Val, DomainVal>::SmoAlgorithm::kernel_polynomial;
                    break;
                case hyperbolic_tangent_type:
                    faif::ml::SvmClassifier<Val, DomainVal>::SmoAlgorithm::kernel = &faif::ml::SvmClassifier<Val, DomainVal>::SmoAlgorithm::kernel_hyperbolic_tangent;
                    break;
                default:
                    faif::ml::SvmClassifier<Val, DomainVal>::SmoAlgorithm::kernel = &faif::ml::SvmClassifier<Val, DomainVal>::SmoAlgorithm::kernel_gauss;
                    break;
            }
        }

        template <typename Val, typename DomainVal>
        size_t SvmClassifier<Val, DomainVal>::SmoAlgorithm::optimizeTwoAlphas(size_t i1, size_t i2){
            if(i1 == i2)
                return 0;

            //Compute E1, E2, y1, y2, alpha1_old, alpha2_old
            Val alpha1_old = alpha[i1];
            int y1 = this->trainingExamples[i1].first;
            Val E1; 
            if(alpha1_old > 0 && alpha1_old < this->C)
                E1 = this->error_cache[i1];
            else
                E1 = learned_func(i1) - y1;

            Val alpha2_old = this->alpha[i2];
            int y2 = this->trainingExamples[i2].first;
            Val E2;
            if(alpha2_old > 0 && alpha2_old < this->C)
                E2 = this->error_cache[i2];
            else
                E2 = learned_func(i2) - y2;

            int s = y1 * y2;

            //Compute L, H
            Val L, H;
            if(y1 == y2){
                Val gamma = alpha1_old + alpha2_old;
                if(gamma > this->C){
                    L = gamma - this->C;
                    H = this->C;
                }
                else{
                    L = 0;
                    H = gamma;
                }
            }
            else{
                Val gamma = alpha1_old - alpha2_old;
                if(gamma > 0){
                    L = 0;
                    H = this->C-gamma;
                }
                else{
                    L = -gamma;
                    H = this->C;
                }
            }
            //Some software use L>=H condition
            if(L == H)
                return 0;

            //Compute eta
            Val k11, k22;
            if(kernel_type==gauss_type || kernel_type==default_type){
                k11 = k22 = 1.0;
            }
            else{
                k11 = (this->*kernel)(this->trainingExamples[i1].second, this->trainingExamples[i1].second);
                k22 = (this->*kernel)(this->trainingExamples[i2].second, this->trainingExamples[i2].second);
            }
            Val k12 = (this->*kernel)(this->trainingExamples[i1].second, this->trainingExamples[i2].second);
            Val eta = 2*k12 - k11 - k22;
            Val alpha2_new;
            Val Lobj, Hobj;
            if(eta < 0){
                alpha2_new = alpha2_old + y2 * (E2 - E1) / eta;
                if(alpha2_new < L)
                    alpha2_new = L;
                else if(alpha2_new > H)
                    alpha2_new = H;
            }
            else{
                //Compute Lobj, Hobj at alpha2_new = L, alpha2_new = H
                Lobj = (eta/2) * L * L + L * ( y2 * (E1-E2) - eta * alpha2_old);
                Hobj = (eta/2) * H * H + H * ( y2 * (E1-E2) - eta * alpha2_old);

                if(Lobj > Hobj + epsilon)
                    alpha2_new = L;
                else if(Lobj < Hobj - epsilon)
                    alpha2_new = H;
                else
                    alpha2_new = alpha2_old;
            }

            if( ( (alpha2_new > alpha2_old) ? (alpha2_new - alpha2_old) : ( alpha2_old - alpha2_new ) ) < epsilon * (epsilon + alpha2_new + alpha2_old) )
                return 0;

            Val alpha1_new = alpha1_old -s * (alpha2_new - alpha2_old);
            if(alpha1_new < 0){
                alpha2_new += s * alpha1_new;
                alpha1_new = 0;
            }
            else if(alpha1_new > this->C){
                alpha2_new += s * (alpha1_new - this->C);
                alpha1_new = this->C;
            }

            //Update threshold to reflect change in Lagrange multipliers
            {
                Val bnew;
                if(alpha1_new > 0 && alpha1_new < this->C)
                    bnew = b + E1 + y1 * (alpha1_new - alpha1_old) * k11 + y2 * (alpha2_new - alpha2_old) * k12;
                else{
                    if(alpha2_new > 0 && alpha2_new < this->C)
                        bnew = b + E2 + y1 * (alpha1_new - alpha1_old) * k12 + y2 * (alpha2_new - alpha2_old) * k22;
                    else{
                        Val b1 = b + E1 + y1 * (alpha1_new - alpha1_old) * k11 + y2 * (alpha2_new - alpha2_old) * k12;
                        Val b2 = b + E2 + y1 * (alpha1_new - alpha1_old) * k12 + y2 * (alpha2_new - alpha2_old) * k22;
                        bnew = (b1 + b2)/2;
                    }
                }
                this->delta_b = bnew - this->b;
                this->b = bnew;
            }

            //Update error cache with new multipliers
            {
                Val t1 = y1 * (alpha1_new - alpha1_old);
                Val t2 = y2 * (alpha2_new - alpha2_old);
                size_t numExamples = trainingExamples.size();
                for(size_t i=0; i<numExamples; ++i)
                    if( alpha[i] > 0 && alpha[i] < this->C)
                        this->error_cache[i] += t1 * (this->*kernel)(trainingExamples[i1].second, trainingExamples[i].second) + t2 * (this->*kernel)(trainingExamples[i2].second,     trainingExamples[i].second) - this->delta_b;
                this->error_cache[i1] = 0.0;
                this->error_cache[i2] = 0.0;
            }

            alpha[i1] = alpha1_new;
            alpha[i2] = alpha2_new;
            return 1;
        }

        template <typename Val, typename DomainVal>
        size_t SvmClassifier<Val, DomainVal>::SmoAlgorithm::examineKKTViolation(size_t i1){
            Val y1 = this->trainingExamples[i1].first;
            Val alpha1 = alpha[i1];
            Val E1 = (alpha1 > 0 && alpha1 < this->C) ? (this->error_cache[i1]) : (learned_func(i1) - y1);
            Val r1 = y1 * E1;
            size_t numExamples = trainingExamples.size();
            if((r1 < -(this->margin) && alpha1 < (this->C)) || (r1 > this->margin && alpha1 > 0)){
                //Try argmax E1-E2
                {
                    Val tmax = 0.0;
                    size_t i2 = 0;
                    for(size_t k = 0; k < numExamples; ++k)
                        if(alpha[k] > 0 && alpha[k] < C){
                            Val E2 = error_cache[k];
                            Val temp = (E1>E2) ? (E1-E2) : (E2-E1);
                            if(temp > tmax){
                                tmax = temp;
                                i2 = k;
                            }
                        }
                    if(tmax>0.0)
                        if(optimizeTwoAlphas(i1, i2))
                            return 1;
                }
                {
                    RandomInt rnd(0, static_cast<int>(numExamples)-1);

                    // First try iterate through the non-bound examples
                    size_t j0 = static_cast<size_t>(rnd());
                    for(size_t j=j0; j < numExamples+j0; ++j){
                        size_t i2 = j%numExamples;
                        if(alpha[i2] > 0 && alpha[i2] < this->C)
                            if(optimizeTwoAlphas(i1, i2))
                                return 1;
                    }

                    // After try iterate through the entire training set
                    j0 = static_cast<size_t>(rnd());
                    for(size_t j=j0; j < numExamples+j0; ++j){
                        size_t i2 = j%numExamples;
                        if(optimizeTwoAlphas(i1, i2))
                            return 1;
                    }
                }
            }
            return 0;
        }

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::SmoAlgorithm::train(std::vector<typename SmoAlgorithm::TrainExampleSmo > _trainingExamples){
            for(const auto& example : _trainingExamples)
                trainingExamples.push_back(example);

            size_t numExamples = trainingExamples.size();
            if (numExamples < 1)
                return;

            size_t alphasImproved = 0;
            bool checkAllExamples = true;
            alpha.resize(numExamples, 0.);
            error_cache.resize(numExamples, 0.);

            size_t numSteps = static_cast<size_t>(numExamples*this->stepsStopCondition);
            //Stop after numSteps while finiteStopCondition is true
            while( (alphasImproved > 0 || checkAllExamples) && ( !(this->finiteStopCondition) || ((numSteps--) > 0 ) ) ){
                alphasImproved = 0;
                if(checkAllExamples){
                    for(size_t i=0; i<numExamples; ++i)
                        alphasImproved += examineKKTViolation(i);
                }
                else{
                    for(size_t i=0; i<numExamples; ++i)
                        if((alpha[i] > C+margin || alpha[i] < C-margin) && (alpha[i] > margin || alpha[i] < -(margin) ))
                            alphasImproved += examineKKTViolation(i);
                }
                if(checkAllExamples)
                    checkAllExamples = false;
                else if(alphasImproved == 0)
                    checkAllExamples = true;
            }//while
        }

        template <typename Val, typename DomainVal>
        Val SvmClassifier<Val, DomainVal>::SmoAlgorithm::classify(const ClassifyExampleSmo& vec){
            // Default classification value: 0.5
            if(alpha.size() == 0)
                return 0.5;
            Val result = 0.0;
            size_t numExamples = trainingExamples.size();
            for(size_t i=0; i<numExamples; ++i)
                result += alpha[i] * trainingExamples[i].first * ( (this->*kernel)(trainingExamples[i].second, vec) );
            return sigmoid_function(result-b);
        }

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::SmoAlgorithm::reset(){
            trainingExamples.clear();
            alpha.clear();
            error_cache.clear();
            b = 0.0;
            delta_b = 0.0;
        }

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::SmoAlgorithm::setC(Val C_in){
            if( C_in > 0 )
                this->C = C_in;
            else throw std::invalid_argument("SVM's parameter 'C' should be > 0");
        }

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::SmoAlgorithm::setMargin(Val margin_in){
            if( margin_in > 0 )
                this->margin = margin_in;
            else throw std::invalid_argument("SVM's parameter 'margin' should be > 0");
        }

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::SmoAlgorithm::setEpsilon(Val epsilon_in){
            if( epsilon_in > 0 )
                this->epsilon = epsilon_in;
            else throw std::invalid_argument("SVM's parameter 'epsilon' should be > 0");
        }

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::SmoAlgorithm::setGaussParameter(Val gaussParameter_in){
            if( gaussParameter_in > 0 )
                this->gaussParameter = gaussParameter_in;
            else throw std::invalid_argument("SVM's parameter 'gaussParameter' should be > 0");
        }

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::SmoAlgorithm::setPolynomialInhomogeneousParameter(Val polynomialInhomogeneousParameter_in){
            this->polynomialInhomogeneousParameter = polynomialInhomogeneousParameter_in;
        }

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::SmoAlgorithm::setPolynomialDegree(Val polynomialDegree_in){
            this->polynomialDegree = polynomialDegree_in;
        }

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::SmoAlgorithm::setTangentFrequency(Val tangentFrequency_in){
            if( tangentFrequency_in > 0 )
                this->tangentFrequency = tangentFrequency_in;
            else throw std::invalid_argument("SVM's parameter 'tangentFrequency' should be > 0");
        }

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::SmoAlgorithm::setTangentShift(Val tangentShift_in){
            if( tangentShift_in >= 0 )
                this->tangentShift = tangentShift_in;
            else throw std::invalid_argument("SVM's parameter 'tangentShift' should be >= 0");
        }

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::SmoAlgorithm::setFiniteStepsStopCondition(double stop){
            if( stop > 0 ){
                this->stepsStopCondition = stop;
                this->finiteStopCondition = true;
            }
            else throw std::invalid_argument("SVM's parameter 'stepsStopCondition' should be > 0");
        }

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::SmoAlgorithm::unsetFiniteStepsStopCondition(){
            this->finiteStopCondition = false;
        }

        template <typename Val, typename DomainVal>
        void SvmClassifier<Val, DomainVal>::SmoAlgorithm::setSigmoidScaleFactor(Val sigmoidScaleFactor_in){
            if( sigmoidScaleFactor_in > 0 )
                this->sigmoidScaleFactor = sigmoidScaleFactor_in;
            else throw std::invalid_argument("SVM's parameter 'sigmoidScaleFactor' should be > 0");
        }
    } //namespace ml
} //namespace faif

#endif //FAIF_SVM_HPP_
