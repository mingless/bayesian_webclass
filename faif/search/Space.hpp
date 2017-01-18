#ifndef FAIF_SEARCH_SPACE_HPP
#define FAIF_SEARCH_SPACE_HPP

//
//file with the Space concept
//

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
//msvc10.0 warnings for concepts
#pragma warning(disable:4510)
#pragma warning(disable:4610)
#endif


#include <iterator>
#include <algorithm>
#include <vector>
#include <boost/bind.hpp>
#include <boost/concept_check.hpp>

namespace faif {
    namespace search {

        /** \brief the typedef-s for space, where the fitness is defined as double */
        template<typename Ind> struct Space {
            typedef Ind Individual;
			typedef double Fitness;
        };

		/** \brief the concept for space with fitness
		*/
        template<typename Space>
		struct SpaceConcept {
			typedef typename Space::Individual Individual;
			typedef typename Space::Fitness Fitness;

			BOOST_CONCEPT_USAGE(SpaceConcept)
			{
				//the method to generate the fitness is required
				Space::fitness(ind);
			}
			Individual ind;
		};

		/** \brief Stop condition, finish the algorithm after STEPS_NUM iterations */
		template< unsigned STEPS_NUM > struct StopAfterNSteps {
			StopAfterNSteps() : steps_(0) { }
			template<typename Population> void update(const Population& ) { ++steps_; }
			bool isFinished() const { return steps_ >= STEPS_NUM; }
		private:
			unsigned int steps_;
		};


	} //namespace search
} //namespace faif


#endif
