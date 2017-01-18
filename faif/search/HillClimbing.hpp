#ifndef HILL_CLIMBING_HPP
#define HILL_CLIMBING_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
//msvc10.0 warnings for concepts
#pragma warning(disable:4510)
#pragma warning(disable:4610)
#endif

#include <boost/concept_check.hpp>

#include "Node.hpp"
#include "Space.hpp"

namespace faif {
    namespace search {

		/** \brief the policy class for HillClimbing, check all neighbours */
        template<typename Space> struct NextNodeCheckAll {

			typedef typename Space::Individual Individual;
			typedef typename Individual::PNode PNode;

			/** \brief next node as the better neighbour */
			static PNode nextNode(const PNode& initial) {
				//the fitness is required
				BOOST_CONCEPT_ASSERT((SpaceConcept<Space>));
			     //the methods to create tree-like structures are required
				BOOST_CONCEPT_ASSERT((NodeWithChildrenConcept<Individual>));

				 typedef typename Space::Fitness Fitness;
				 Fitness best_fitness = Space::fitness(*initial);

				 PNode best_neighbour;

				 typedef typename Individual::Children Children;
				 Children ch = initial->getChildren();
				 for( typename Children::const_iterator it = ch.begin(); it != ch.end(); ++it ) {
				 	 const PNode neighbour = *it;
				 	 Fitness neighbour_fitness = Space::fitness(*neighbour);
				 	 if (best_fitness < neighbour_fitness) {
				 		 best_neighbour = neighbour;
				 		 best_fitness = neighbour_fitness;
				 	 }
			     }
				 return best_neighbour;
			}
		};

		/** \brief the hill climbing algorithm. Search the neighbour for the better solution
		*/
		template<typename Space,
				 template <typename> class NextNodeStrategy = NextNodeCheckAll >
		class HillClimbing {
		public:
			typedef typename Space::Individual Individual;
			typedef typename Individual::PNode PNode;
			typedef typename Space::Fitness Fitness;

			HillClimbing() { }

			/** \brief the hill climbing algorithm - until stop repeat searching all adjacent nodes. */
			PNode solve(const PNode& initial) {
				//the fitness is required
				BOOST_CONCEPT_ASSERT((SpaceConcept<Space>));
				//the methods to create tree-like structures are required
				BOOST_CONCEPT_ASSERT((NodeWithChildrenConcept<Individual>));

				typename Individual::PNode current = initial;
				typename Individual::PNode best_neighbour = current;
				do {
					current = best_neighbour;
					best_neighbour = NextNodeStrategy<Space>::nextNode(current);
				}
				while( best_neighbour != 0L );
				return current;
			}
		};

	} //namespace search
} //namespace faif


#endif // HILL_CLIMBING_HPP
