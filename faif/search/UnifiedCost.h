#ifndef FAIF_UNIFIED_COST_SEARCH_HPP
#define FAIF_UNIFIED_COST_SEARCH_HPP

#include <vector>
#include <deque>
#include <functional>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/concept_check.hpp>

#include "TreeNodeImpl.hpp"

namespace faif {
    namespace search {

			/** \brief the comparizon used by searchUnifiedCost */
			template<typename T>
			struct compareWeight {
				bool operator()(const TreeNode<T>* a, const TreeNode<T>* b) {
					return a->getWeight() > b->getWeight();
				}
			};

        /**
           \brief  uniform-cost search algorithm (informed search)

           the uniform-cost search algorithm, traversing a tree structure,
           visit the next node which has the least total cost from the root.
           Algorithm use the 'getWeight' function of node.

           Function is protected against the cycles in the graph.
           The maximum depth of tree is limited.
        */
        template<typename T>
        typename Node<T>::Path searchUnifiedCost(boost::shared_ptr<T> start, int max = 200) {

            //the methods to create tree-like structures are required
            BOOST_CONCEPT_ASSERT((NodeWithChildrenConcept<T>));
            BOOST_CONCEPT_ASSERT((NodeWithFinalFlagConcept<T>));
            //the methods to return a weight of node is required
            BOOST_CONCEPT_ASSERT((TreeNodeWeightConcept<T>));

            std::priority_queue<TreeNode<T>*, std::deque<TreeNode<T>*>, compareWeight<T> > buffer;

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

#endif //FAIF_UNIFIED_COST_SEARCH_HPP
