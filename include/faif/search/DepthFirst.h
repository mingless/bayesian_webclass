#ifndef FAIF_DEPTH_FIRST_SEARCH_HPP
#define FAIF_DEPTH_FIRST_SEARCH_HPP

#include    <vector>
#include    <stack>

#include <boost/concept_check.hpp>

#include "TreeNodeImpl.hpp"


namespace faif {
    namespace search {


        /**
           @brief  The depth-first search algorithm (DFS)

           from starting state the function build the tree structure,
           explores as far as possible, backtracks. Function uses the protection
           against the cycles in the graph and the maximum depth of tree border.

           @return the first path from the starting to the finishing state.
        */
        template<typename T>
        typename Node<T>::Path searchDepthFirst(boost::shared_ptr<T> start, int max = 200) {
			//the methods to create tree-like structures are required
            BOOST_CONCEPT_ASSERT((NodeWithChildrenConcept<T>));
            BOOST_CONCEPT_ASSERT((NodeWithFinalFlagConcept<T>));

            std::stack< TreeNode<T>* > buffer;
            std::stack< TreeNode<T>* > trash;

            TreeNode<T> root(start);
            TreeNode<T>* curr = &root;

            buffer.push(&root);

            while (!buffer.empty()) {

                curr = buffer.top();
                buffer.pop();

                //cout << "Buffer size " << buffer.size()  << " curr " << endl << *(curr->getPoint()) << endl;

                if ( !trash.empty() )
                    while (curr->getLevel() <= trash.top()->getLevel()) {
                        trash.top()->eraseChildren();
                        trash.pop();
                    }

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

                trash.push(curr);
            }

            typename Node<T>::Path path;

            if (curr->getPoint()->isFinal() )
                path = curr->generatePathToRoot();

            return path;
        }

    }//namespace search
} //namespace faif

#endif //FAIF_DEPTH_FIRST_SEARCH_HPP

