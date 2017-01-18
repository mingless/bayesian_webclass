#ifndef FAIF_SEARCH_EM_H
#define FAIF_SEARCH_EM_H

//
//file with Excpectation Maximization algorithm
//

#include "Space.hpp"

namespace faif {
    namespace search {

        /** \brief expectation policy (empty)
         */
        template<typename Space> struct ExpectationNone {

			typedef typename Space::Individual Individual;

            /** \brief expectation is an empty operation */
            static Individual& expectation(Individual& p) {
                return p;
            }
        };

        /** \brief expectation policy - custiom
         */
        template<typename Space> struct ExpectationCustom {

			typedef typename Space::Individual Individual;

            /** \brief expectation calls the method from template parameter */
            static Individual& expectation(Individual& p) {
				return Space::expectation(p);
			}
        };

        /** \brief maximization policy (empty)
n
         */
        template<typename Space> struct MaximizationNone {

			typedef typename Space::Individual Individual;

            /** \brief expectation is an empty operation */
            static  Individual& maximization( Individual& p) {
                return p;
            }
        };

        /** \brief maximization policy - custiom
         */
        template<typename Space> struct MaximizationCustom {

			typedef typename Space::Individual Individual;

            /** \brief expectation calls the method from template parameter */
            static  Individual& maximization( Individual& p) {
				return Space::maximization(p);
			}
        };

		/** \brief the Expectation-Maximization algorithm

			\param Expectation: ExpectationNone, ExpectationCustom
			\param Maximization: MaximizationNone, MaximizationCustom
			\param Stop: StopAfterNSteps
		 */
        template<  typename Space,
                   template <typename> class Expectation = ExpectationNone,
                   template <typename> class Maximization = MaximizationNone,
				   typename StopCondition = StopAfterNSteps<100>
                   >
        class ExpectationMaximization {
        public:
            ExpectationMaximization( ) { }

			typedef typename Space::Individual Individual;
			typedef typename Space::Fitness Fitness;

            /** \brief the evolutionary algorithm - until stop repeat mutation, cross-over, selection, succession.
				Modifies the initial population.
             */
            Individual& solve(Individual& init, StopCondition stop = StopCondition() ) {

				//the fitness is required
				BOOST_CONCEPT_ASSERT((SpaceConcept<Space>));

                Individual& current(init);
				stop.update(current);
				while( !stop.isFinished() ) {
					Expectation<Space>::expectation(current);
					Maximization<Space>::maximization(current);
					stop.update(current);
                }
				return current;
            }
        };

	} //namespace search
} //namespace faif

#endif // FAIF_SEARCH_EM_H
