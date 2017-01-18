#ifndef FAIF_ASTAR_SEARCH_HPP
#define FAIF_ASTAR_SEARCH_HPP

#include    <vector>
#include    <queue>

#include <boost/bind.hpp>
#include <boost/concept_check.hpp>

#include "TreeNodeImpl.hpp"

namespace faif {
    namespace search {

		/** \brief the comparizon used by AStar */
		template<typename T>
		struct compareWeightAndHeuristic {

			bool operator()(const TreeNode<T>* a, const TreeNode<T>* b) {
				return (a->getWeight() + a->getPoint()->getHeuristic()) >
					(b->getWeight() + b->getPoint()->getHeuristic());
			}
		};
        /**
           \brief A* (A star) search algorithm

           heuristic searching tree-like structures, finds the least-cost path
           from initial node to goal node, uses a distance function which is a sum
           of path-cost and a heuristic estimate of the distance to the goal

           Function is protected against the cycles in the graph.
           The maximum depth of tree is limited.
        */
        template<typename T>
        typename Node<T>::Path searchAStar(boost::shared_ptr<T> start, int max = 200) {

            //the methods to create tree-like structures are required
            BOOST_CONCEPT_ASSERT((NodeWithChildrenConcept<T>));
            BOOST_CONCEPT_ASSERT((NodeWithFinalFlagConcept<T>));
            //the methods to return a weight of node is required
            BOOST_CONCEPT_ASSERT((TreeNodeWeightConcept<T>));
            //the methods to return a heuristic to the goal is required
            BOOST_CONCEPT_ASSERT((TreeNodeHeuristicConcept<T>));

            std::priority_queue<TreeNode<T>*, std::deque<TreeNode<T>*>, compareWeightAndHeuristic<T> > buffer;

            TreeNode<T> root(start);
            TreeNode<T>* curr = &root;

            buffer.push(curr);

            while (!buffer.empty()) {
                curr    = buffer.top();
                buffer.pop();

                //check if the node is twice on the path from node to root
                if ( checkNodeInPath( *curr ) )
                    continue;

                //check if the node is the final node
                if (curr->getPoint()->isFinal() )
                    break;

                //check if the maximum depth
                if (curr->getLevel() == max)
                    continue;

                typename TreeNode<T>::Children ch = curr->getChildrenWithWeight();
                for( typename TreeNode<T>::Children::const_iterator it = ch.begin(); it != ch.end(); ++it )
                    buffer.push( *it );
            }

            typename Node<T>::Path path;

            if (curr->getPoint()->isFinal() )
                path = curr->generatePathToRoot();

            return path;

        }

    } //namespace search
}//namespace faif


#endif //FAIF_ASTAR_SEARCH_HPP

