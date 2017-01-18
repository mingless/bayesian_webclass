#ifndef FAIF_SEARCH_VECTOR_INDIVIDUAL_H
#define FAIF_SEARCH_VECTOR_INDIVIDUAL_H

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
//msvc9.0 generuje smieci dla boost/random
#pragma warning(disable:4127)
#endif

#include <vector>
#include <algorithm>
#include <boost/concept_check.hpp>
#include <boost/bind.hpp>

#include "../utils/Random.hpp"

namespace faif {
    namespace search {

		/** \brief the concept for evolutionary algorithm gene
			the type gives the generateRandom method and the mutate method
		*/
        template<typename T>
		struct EvolutionaryAlgorithmGeneConcept {
			typedef typename T::value_type value_type;
			BOOST_CONCEPT_USAGE(EvolutionaryAlgorithmGeneConcept)
			{
				//the method to generate random gene is required
				T::generateRandom();
				//the metod to mutate gene is requred
				T::mutate(0.1, t );
			}
			value_type t;
		};

		/** gene as boolean variable */
		struct BooleanGene {
			typedef bool value_type;
			static value_type generateRandom() {
				RandomDouble random;
				return random() < 0.5;
			}
			static value_type mutate(double prob_mutation, const value_type& val) {
				RandomDouble random;
				if( random() < prob_mutation ) {
					return !val;
				}
				else {
					return val;
				}
			}
		};

		/** \brief Template to generate individual which is the vector of Genes */
		template<typename Gene> class VectorIndividual {
		public:
			//the methods for gene are required
			BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmGeneConcept<Gene>));

			typedef typename Gene::value_type value_type;
			typedef std::vector<value_type> Container;
			typedef typename Container::iterator iterator;
			typedef typename Container::const_iterator const_iterator;

			/** create the random initial individual. The vector size is parameter */
			explicit VectorIndividual(int size) {
				std::generate_n( std::back_inserter( chromosome_ ), size, &Gene::generateRandom );
			}

			/** init individual with given data */
			explicit VectorIndividual(Container value)
				: chromosome_(value) {}

			VectorIndividual(const VectorIndividual& i) : chromosome_(i.chromosome_) {}
			~VectorIndividual() {}
			VectorIndividual& operator=(const VectorIndividual& i) {
				chromosome_ = i.chromosome_;
				return *this;
			}

			/** \brief change the object at random positions */
			void mutate(double prob_mutation) {
				std::transform( chromosome_.begin(), chromosome_.end(), chromosome_.begin(),
								boost::bind( &Gene::mutate, prob_mutation, _1) );
			}

			bool operator==(const VectorIndividual& i) const {
				return chromosome_ == i.chromosome_;
			}
			bool operator!=(const VectorIndividual& i) const {
				return chromosome_ != i.chromosome_;
			}
			/** accessor */
			const Container& getChromosome() const { return chromosome_; }
		private:
			Container chromosome_;
		};

	} //namespace search
} //namespace faif

#endif //FAIF_SEARCH_VECTOR_INDIVIDUAL_H
