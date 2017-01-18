#ifndef FAIF_BREADTH_FIRST_SEARCH_HPP
#define FAIF_BREADTH_FIRST_SEARCH_HPP

#include    <vector>
#include    <queue>
#include    <deque>
#include    <algorithm>

#include "TreeNodeImpl.hpp"

#include <iostream>
using namespace std;


namespace faif {
    namespace   search {

		/**
		   @brief  The breadth-first search algorithm


           from starting state the function build the tree structure,
		   explores all the nearest nodes (children), and if it not find the goal generates
		   the nearest nodes to the children.
		   The return is the shortest path from root node to the goal node.
		   Function is protected against the cycles in the graph.
		   The maximum depth of tree is limited.
		*/
        template<typename T>
        typename Node<T>::Path searchBreadthFirst(boost::shared_ptr<T> start, int max = 200) {
            //the methods to create tree-like structures are required
            BOOST_CONCEPT_ASSERT((NodeWithChildrenConcept<T>));
            BOOST_CONCEPT_ASSERT((NodeWithFinalFlagConcept<T>));

            std::queue< TreeNode<T>* > buffer;

            TreeNode<T> root(start);
            TreeNode<T>* curr = &root;

            buffer.push(&root);

            while (!buffer.empty()) {

                curr = buffer.front();
                buffer.pop();

				// cout << "Buffer size " << buffer.size() << " curr " << endl << *curr->getPoint() << endl;

                //check if the node is twice on the path from node to root
                if ( checkNodeInPath( *curr ) )
                    continue;

                //check if the node is the final node
                if (curr->getPoint()->isFinal() )
                    break;

                //check if the maximum depth
                if (curr->getLevel() == max)
                    continue;

                typename TreeNode<T>::Children ch = curr->getChildren();
                for( typename TreeNode<T>::Children::const_iterator it = ch.begin(); it != ch.end(); ++it )
                    buffer.push( *it );
            }

            typename Node<T>::Path path;

            if (curr->getPoint()->isFinal() )
                path = curr->generatePathToRoot();

            return path;
        }


    } //namespace search
} //namespace faif

#endif //FAIF_BREADTH_FIRST_SEARCH_HPP

