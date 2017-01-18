#ifndef FAIF_SEARCHING_NODE_HPP
#define FAIF_SEARCHING_NODE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
//msvc10.0 warnings for concepts
#pragma warning(disable:4510)
#pragma warning(disable:4610)
#endif

#include <vector>
#include <ostream>
#include <algorithm>
#include <iterator>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/concept_check.hpp>

namespace faif {

    /**
       \brief the namespace of searching algorithms and optimization algorithms
    */
    namespace search {

		/** \brief the struct to create node in search space from individual */
		template<typename Individual> struct Node {
			typedef boost::shared_ptr<Individual> PNode;
			typedef std::vector<PNode> Path;
			typedef std::vector<PNode> Children;
		};

		/** \brief helping function for debugging */
		template<typename Node>
		inline std::ostream& operator<<(std::ostream& os, const std::vector<boost::shared_ptr<Node> >& path) {
			std::transform( path.begin(), path.end(), std::ostream_iterator<Node>(os," "),
							boost::bind(&boost::shared_ptr<Node>::operator*, _1) );
			return os;
		}


		/** \brief the concept for node with children
		*/
		template<typename Node>
		struct NodeWithChildrenConcept : boost::EqualityComparable<Node> {
			typedef typename Node::PNode PNode;
			typedef typename Node::Children Children;

			BOOST_CONCEPT_USAGE(NodeWithChildrenConcept)
			{
				Children ch = n.getChildren(); //the method getChildren is required
				ch.size(); //clear the warnings of unused variables
			}
			Node n;
		};

		/** \brief the concept for node with final flag for search in tree-like structures
			The function 'searchDepthFirst' and 'searchBreadthFirst' require this concept
		*/
        template<typename Node>
		struct NodeWithFinalFlagConcept : boost::EqualityComparable<Node> {
			BOOST_CONCEPT_USAGE(NodeWithFinalFlagConcept)
			{
				bool fin = n.isFinal(); //the method isFinal is required
				fin = fin == false; //clear the warnings of unused variables
			}
			Node n;
		};

		/** \brief the concept for informed search algorithms, it check the presence of 'getWeight' method,
			used by informed search functions e.g. 'searchUniformCost'
		*/
		template<typename Node>
		struct TreeNodeWeightConcept {
			BOOST_CONCEPT_USAGE(TreeNodeWeightConcept)
			{
				n.getWeight();
			}
			Node n;
		};

		/** \brief the concept for heuristic search algorithms, it check the presence of 'getHeuristic' method,
			used by heuristic search functions e.g. 'searchAStar'
		*/
		template<typename Node>
		struct TreeNodeHeuristicConcept {
			BOOST_CONCEPT_USAGE(TreeNodeHeuristicConcept)
			{
				n.getHeuristic();
			}
			Node n;
		};

    } //namespace search
} //namespace faif

#endif //FAIF_SEARCHING_NODE_HPP
