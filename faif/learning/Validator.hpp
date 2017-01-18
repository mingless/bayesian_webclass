#ifndef FAIF_VALIDATOR_H_
#define FAIF_VALIDATOR_H_

//   File with classes and functions to check classifier quality and error

#include <algorithm>
#include <cassert>
#include <vector>
#include <boost/lambda/lambda.hpp>

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#include "Classifier.hpp"
#include "../utils/Random.hpp"

namespace faif {
    namespace ml {

        namespace {
            /** helping functor (predicate), return true if the classifier result equal to given category */
            template<typename Val>
            struct CheckExampleFunctor {
                Classifier<Val>& cl_;
                CheckExampleFunctor(Classifier<Val>& c) : cl_(c)
                { }

                bool operator()(const typename Classifier<Val>::ExampleTrain& example) {
                    return cl_.getCategory(example) == example.getFeature();
                }
            private:
                CheckExampleFunctor& operator=(const CheckExampleFunctor&); //forbidden
            };
        } //namespace

        /**
           \brief check the classifier
           \return the number of test examples correctly classified

           \param test examples
           \param classifier
        */
        template<typename Val>
        int checkClassifier(const typename Classifier<Val>::ExamplesTrain& test, Classifier<Val>& classifier ) {
            CheckExampleFunctor<Val> checkFunctor(classifier);
            return static_cast<int>( std::count_if( test.begin(), test.end(), checkFunctor ) );
        }


		/** f. pomocnicza, zwraca liczbe prawidlowo zaklasyfikowanych przykladow z tcollect.
			Przyklady testujace to <start_idx, end_idx). Reszta - to przyklady trenujace. */
		template<typename Val>
		int testRange(std::vector<const typename Classifier<Val>::ExampleTrain*>& tcollect, int start_idx, int end_idx, Classifier<Val>& classifier ) {

			typename Classifier<Val>::ExamplesTrain test;
			typename Classifier<Val>::ExamplesTrain train;

			typename std::vector<const typename Classifier<Val>::ExampleTrain* >::iterator it = tcollect.begin();
			for(int idx=0; it != tcollect.end(); ++it, ++idx ) {
				if( idx < start_idx )
					train.push_back( **it );
				else if( idx < end_idx )
					test.push_back( **it );
				else
					train.push_back( **it );
			}

			classifier.reset();
			classifier.train(train);
			return checkClassifier( test, classifier );
		}

        namespace {

            /** pomocniczy funktor */
            struct ShuffleFunctor {
                RandomInt& r_;
                ShuffleFunctor(RandomInt& r)
                    : r_(r) { }
                int operator()(int){ return r_(); }
            private:
                ShuffleFunctor& operator=(const ShuffleFunctor&); //zabronione przypisanie bo skladowe referencyjne
            };

        } //namespace



        /**
           check the classifier, return the probability of proper classification result.
           The example set is divided on k sections (randomly), one section is the testing set,
           the rest k-1 sections are the training set. This test is repeated k times.

           \param examples training examples (the part is randomly choosen as testing)
           \param k num sections for cross-validation
           \param classifier classifier
           \return the probability of proper classification
        */
        template<typename Val>
        double checkCross( const typename Classifier<Val>::ExamplesTrain& examples, int k, Classifier<Val>& classifier) {

            typedef typename Classifier<Val>::ExampleTrain ExampleTrain;

            int n = (int)examples.size();

            assert( k > 0 );
            assert( n >= k );

            typedef std::vector<const ExampleTrain*> TrainCollection;
            TrainCollection tcollect( n );
            std::transform( examples.begin(), examples.end(), tcollect.begin(), & boost::lambda::_1 );
            RandomInt gen(0, n-1);
            ShuffleFunctor shuffleFunctor( gen );
            std::random_shuffle( tcollect.begin(), tcollect.end(), shuffleFunctor );

            int start_idx = 0;
            int end_idx = 0;

            int num_proper = 0; //liczba prawidlowo zaklasyfikowanych przykladow testujacych

            for(int i = 0; i < k; i++ ) {
                start_idx = end_idx;
                end_idx = (n * (i + 1) )/k;
                //k-ty przedzial jest przedzialem testujacym
                num_proper += testRange<Val>(tcollect, start_idx, end_idx, classifier );
            }

            int num_all = static_cast<int>( tcollect.size() );
            return num_proper /(double)num_all ;
        }

    } //namespace ml
} //namespace faif

#endif //FAIF_VALIDATOR_H_
