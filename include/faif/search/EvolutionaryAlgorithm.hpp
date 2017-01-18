#ifndef FAIF_SEARCH_EA_H
#define FAIF_SEARCH_EA_H

//
//file with Evolutionary Algorithm implementation
//

#include <vector>
#include <iterator>
#include <algorithm>
#include <numeric>
#include <functional>
#include <boost/bind.hpp>
#include <boost/concept_check.hpp>

#include "../utils/Random.hpp"
#include "Space.hpp"

namespace faif {
    namespace search {

        /** \brief the typedef-s for space for evolutionary algorithm, where the population is a vector of individuals,
            and the fitness is the double */
        template<typename Ind> struct EvolutionaryAlgorithmSpace : public Space<Ind> {
            typedef std::vector<Ind> Population;
        };

        /** \brief the concept for evolutionary algorithm space
         */
        template<typename Space>
        struct EvolutionaryAlgorithmSpaceConcept {
            typedef typename Space::Individual Individual;
            typedef typename Space::Population Population;
            typedef typename Space::Fitness Fitness;

            BOOST_CONCEPT_USAGE(EvolutionaryAlgorithmSpaceConcept)
            {
                //the method to generate the fitness is required
                Space::fitness(ind);
            }
            Individual ind;
        };

        /** \brief the concept for evolutionary algorithm space
         */
        template<typename Space>
        struct EvolutionaryAlgorithmSpaceWithMutationConcept : public EvolutionaryAlgorithmSpaceConcept<Space> {
            typedef typename Space::Individual Individual;
            typedef typename Space::Population Population;
            typedef typename Space::Fitness Fitness;

            BOOST_CONCEPT_USAGE(EvolutionaryAlgorithmSpaceWithMutationConcept)
            {
                //the method to generate the mutation of individual is required
                Space::mutation(ind);
            }
            Individual ind;
        };

        /** \brief the concept for evolutionary algorithm space
         */
        template<typename Space>
        struct EvolutionaryAlgorithmSpaceWithCrossoverConcept : public EvolutionaryAlgorithmSpaceConcept<Space> {
            typedef typename Space::Individual Individual;
            typedef typename Space::Population Population;
            typedef typename Space::Fitness Fitness;

            BOOST_CONCEPT_USAGE(EvolutionaryAlgorithmSpaceWithCrossoverConcept)
            {
                //the method to crossover individuals required
                Space::crossover(ind, population);
            }
            Individual ind;
            Population population;
        };


        /** \brief trait tag, no transformation should be executed */
        struct TransformationNoneTag {};
        /** \brief trait tag, user tranformation should be executed */
        struct TransformationCustomTag : public TransformationNoneTag {};


        /** \brief mutation policy - no mutation */
        template<typename Space> struct MutationNone {

            /** \brief tranformation policy (none) */
            typedef TransformationNoneTag TransformationCategory;

            /** \brief mutation is empty operation */
            static typename Space::Individual& mutation(typename Space::Individual& ind) {
                return ind;
            }
        };

        /** \brief mutation policy - mutation from Space
         */
        template<typename Space> struct MutationCustom {

            /** \brief tranformation policy (custom) */
            typedef TransformationCustomTag TransformationCategory;

            //the methods form mutation are required
            BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmSpaceWithMutationConcept<Space>));

            /** \brief calls the mutation function from Space */
            static typename Space::Individual& mutation(typename Space::Individual& ind) {
                return Space::mutation(ind);
            }
        };

        /** \brief crossover policy - no crossover */
        template<typename Space> struct CrossoverNone {

            /** \brief tranformation policy (none) */
            typedef TransformationNoneTag TransformationCategory;

            /** \brief crossover is empty operation */
            static typename Space::Individual& crossover(typename Space::Individual& ind, typename Space::Population& ) {
                return ind;
            }

        };

        /** \brief crossover policy - crossover from Space
         */
        template<typename Space> struct CrossoverCustom {

            /** \brief tranformation policy (custom) */
            typedef TransformationCustomTag TransformationCategory;

            //the methods form mutation are required
            BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmSpaceWithCrossoverConcept<Space>));

            /** \brief calls the mutation function from Space */
            static typename Space::Individual& crossover(typename Space::Individual& ind, typename Space::Population& pop) {
                return Space::crossover(ind, pop);
            }
        };

        /** \brief succession and selection policy - the n-th best individuals survive
         */
        template<typename Space> struct SelectionRanking {

            /** \brief sort (partial) the population and removes the worst elements.
                Population is returned having n elements. The function 'Space::fitness' is used.
            */
            static typename Space::Population& selection(typename Space::Population& population, int size) {

                typename Space::Population::iterator middle = population.begin();
                std::advance(middle, size);

                //the n-th best elements is moved to the beginning of contener
                std::nth_element(population.begin(), middle, population.end(),
                                 boost::bind(Space::fitness, _1) > boost::bind(Space::fitness, _2) );

                population.erase( middle, population.end() );
                return population;
            }
        };

        /** \brief helping class implemented the M.D.Vose (1991) algorithm */
        class VoseAlg {
            typedef double Fitness;
            typedef double Probability;
            typedef std::vector<Fitness> Fitnesses;
            typedef std::vector<Probability> Probabilities;
            typedef std::vector<int> Indexes;
        public:

            VoseAlg(const Fitnesses& fitnesses)
                                : N_(static_cast<int>(fitnesses.size())),
                                  gen_(0.0, static_cast<double>(N_) ),
                                  prob_(N_,0.0),
                                  alias_(N_,0)
            {
                                Fitness min_fitness = std::numeric_limits<Fitness>::min();
                                Fitnesses::const_iterator ind_min = std::min_element(fitnesses.begin(), fitnesses.end() );
                                if( ind_min != fitnesses.end() )
                                        min_fitness = *ind_min;
                                //Fitnesses should be non-negative -> offset all
                                Fitness sum = std::accumulate( fitnesses.begin(), fitnesses.end(), 0.0 );
                                sum -= N_* min_fitness;

                                Probabilities norm_fitnesses; //the vector with sum of fitness of adjacent individuals
                                norm_fitnesses.reserve(N_);
                                if( sum < std::numeric_limits<Fitness>::epsilon() ) {
                                        fill_n( std::back_inserter(norm_fitnesses), N_, 0.0 );
                                } else {
                                        std::transform( fitnesses.begin(), fitnesses.end(), std::back_inserter(norm_fitnesses),
                                                                        boost::bind( std::divides<Fitness>(), boost::bind( std::minus<Fitness>(), _1, min_fitness ), sum ) );
                                }

                                //std::cout << "normalzed fitnesses " << std::endl;
                                //std::copy( norm_fitnesses.begin(), norm_fitnesses.end(),
                                //           std::ostream_iterator<Fitness>(std::cout," ") );
                                //std::cout << std::endl;

                                Indexes small, large;
                                const Probability one_div_n = 1.0/static_cast<double>(N_);
                                for(int index = 0; index < N_; ++index) {
                                        if(norm_fitnesses[index] < one_div_n)
                                                small.push_back(index);
                                        else
                                                large.push_back(index);
                                }
                                while(!small.empty() && !large.empty()) {
                                        int j = small.back(); small.pop_back();
                                        int k = large.back(); large.pop_back();
                                        prob_[j] = N_ * norm_fitnesses[j];
                                        alias_[j] = k;
                                        norm_fitnesses[k] += norm_fitnesses[j] - one_div_n;
                                        if(norm_fitnesses[k] > one_div_n)
                                                large.push_back(k);
                                        else {
                                                small.push_back(j);
                                        }
                                }
                                while(!small.empty()) {
                                        prob_[small.back()] = 1.0;
                                        small.pop_back();
                                }
                                while(!large.empty()) {
                                        prob_[large.back()] = 1.0;
                                        large.pop_back();
                                }

            }
            int getIndexForRandom() {
                                double x = gen_();
                                int index = static_cast<int>(x); // floor of x
                                if(( x - static_cast<Probability>(index)) > prob_[index])
                                        index = alias_[index];
                                return index;
            }
        private:
            const int N_;
            RandomDouble gen_;
            Probabilities prob_;
            Indexes alias_;
        };

        /** \brief succession and selection policy - roulette wheel
            (probability of selection of an idividual is equal to its normalized fitness)
        */
        template<typename Space> struct SelectionRoulette {

            /** \brief select individuals with probability proportional to their fitness*/
            static typename Space::Population& selection(typename Space::Population& population, int size) {

                typedef typename Space::Population Population;

                std::vector<typename Space::Fitness> fitnesses; //the fitness of individuals
                std::transform( population.begin(), population.end(),
                                std::back_inserter(fitnesses), boost::bind( &Space::fitness, _1 ) );
                VoseAlg vose(fitnesses);
                Population old_population(population); //copy of population
                population.clear();

                std::generate_n( std::back_inserter(population), size,
                                 boost::bind(&SelectionRoulette<Space>::generateIndividualFun, &old_population, &vose ) );
                return population;
            }
        private:
            //! \brief helping function (used in generate_n algorithm), to generate individual from old population)
            static const typename Space::Individual& generateIndividualFun(const typename Space::Population* population,
                                                                           VoseAlg* vose) {
                int index = vose->getIndexForRandom();
                return population->at(index);
            }
        };

        /** \brief the evolutionary algorithm

            \param Mutation - MutationNone, MutationCustom
            \param Selection - SelectionRanking
            \param StopCondition - StopAfterNSteps
        */
        template<  typename Space,
                   template <typename> class Mutation = MutationNone,
                   template <typename> class Crossover = CrossoverNone,
                   template <typename> class Selection = SelectionRanking,
                   typename StopCondition = StopAfterNSteps<100>
                   >
        class EvolutionaryAlgorithm {
        public:
            typedef typename Space::Individual Individual;
            typedef typename Space::Population Population;
            typedef typename Space::Fitness Fitness;

            //the methods for fitness calculation are required
            BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmSpaceConcept<Space>));

            EvolutionaryAlgorithm( ) { }

            /** \brief the evolutionary algorithm - until stop repeat mutation, cross-over, selection, succession.
                Modifies the initial population.
            */
            Individual& solve(Population& init_population, StopCondition stop = StopCondition() ) {

                int pop_size = static_cast<int>(init_population.size());
                Population& current = init_population;
                //StopCondition stop;

                do {
                    // std::cout << "population " << std::endl;
                    // std::copy( current.begin(), current.end(), std::ostream_iterator<Individual>(std::cout," ") );
                    // std::cout << std::endl;

                    Population old_population(current);
                    //mutation
                    doMutation( current, typename Mutation<Space>::TransformationCategory() );

                    // std::cout << "after mutation " << std::endl;
                    // std::copy( current.begin(), current.end(), std::ostream_iterator<Individual>(std::cout," ") );
                    // std::cout << std::endl;

                    //crossover
                    doCrossover(current, typename Crossover<Space>::TransformationCategory() );

                    // std::cout << "after crossover " << std::endl;
                    // std::copy( current.begin(), current.end(), std::ostream_iterator<Individual>(std::cout," ") );
                    // std::cout << std::endl;


                    //join the old and the current population
                    std::copy(old_population.begin(), old_population.end(), back_inserter(current) );

                    // std::cout << "before selection " << std::endl;
                    // std::copy( current.begin(), current.end(), std::ostream_iterator<Individual>(std::cout," ") );
                    // std::cout << std::endl;

                    //selection on join populations
                    Selection<Space>::selection(current, pop_size);

                    // std::cout << "after selection " << std::endl;
                    // std::copy( current.begin(), current.end(), std::ostream_iterator<Individual>(std::cout," ") );
                    // std::cout << std::endl;

                    stop.update(current);
                }
                while(! stop.isFinished() );

                typename Population::iterator best =
                    std::max_element(current.begin(), current.end(),
                                     boost::bind(Space::fitness, _1) < boost::bind(Space::fitness, _2) );
                return *best;
            }
        private:
            /** \brief No action for MutationNone. */
            void doMutation(Population&, TransformationNoneTag) { /* do nothing */ }
            /** \brief Called when MutationCustom is given. */
            void doMutation(Population& population, TransformationCustomTag) {
                std::transform( population.begin(), population.end(), population.begin(),
                                boost::bind( &Mutation<Space>::mutation, _1 ) );
            }
            /** \brief No action for CrossoverNone. */
            void doCrossover(Population&, TransformationNoneTag) { /* do nothing */ }
            /** \brief Called when CrossoverCustom is given. */
            void doCrossover(Population& population, TransformationCustomTag) {
                std::transform(population.begin(), population.end(), population.begin(),
                               boost::bind( &Crossover<Space>::crossover, _1, population));
            }

        };

} //namespace search
    } //namespace faif

#endif // FAIF_SEARCH_EA_H
