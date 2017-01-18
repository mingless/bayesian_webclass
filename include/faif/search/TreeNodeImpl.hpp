#ifndef FAIF_SEARCHING_TREE_NODE_HPP
#define FAIF_SEARCHING_TREE_NODE_HPP

#include <algorithm>
#include "Node.hpp"

namespace faif {

    namespace search {

        /**
           \brief  the template to create the node in tree-based search methods

           \note   The class should not be used by library user, it is used internally by search algorithms
        */
        template<typename T>
        class TreeNode {
		public:
			typedef std::vector< TreeNode<T>* > Children;

            /** Constructor */
            TreeNode(const typename Node<T>::PNode& point, TreeNode<T>* parent = 0L)
                : point_(point), parent_(parent), weight_(0.0), eval_(false), level_(0)
            {
                if (parent_) level_  = parent_->level_ + 1;
				//cout << "TreeNode c-tor level " << level_ << "point: " << *point_.get() << endl;
            }

            /** Constructor */
			TreeNode(const typename Node<T>::PNode& point, TreeNode<T>* parent, double weight)
                : point_(point), parent_(parent), weight_(weight), eval_(false), level_(0)
            {
                if (parent_) level_  = parent_->level_ + 1;
				//cout << "TreeNode c-tor level " << level_ << "point: " << *point_.get() << endl;
            }


            /** Destructor */
            ~TreeNode() {
				eraseChildren();
				//cout << "d-tor" << endl;
			}

            /** accessor */
			boost::shared_ptr<T> getPoint() const {return point_; }

            /** accessor */
            const TreeNode<T>* getParent() const {return parent_; }

            /** accessor, the children. Creates the children in the first call */
			Children getChildren();

			/** accessor, the children. Creates the children in the first call.
				Calculates the weight as a sum of weight of parent and weight of child. */
			Children getChildrenWithWeight();

			/** accessor */
			double getWeight() const { return weight_; }

            /** accessor */
            short getLevel() const { return level_; }

            /** erase the child tree nodes */
            void eraseChildren();

			/** generate the path from given state to the root */
			typename Node<T>::Path generatePathToRoot() const;
        private:
			typename Node<T>::PNode point_; //!< the node member
            TreeNode<T>* parent_; //!< pointer to the parent
			Children children_;

			double weight_; //!< the weight of the node (used by informed search algorithms)

			bool eval_; //!< the flag if the children were generated
            short level_; //!< the node level
        };


		/** check if the node is twice on the path */
		template<typename T>
		bool checkNodeInPath( const TreeNode<T>& n) {

			const T& state = *(n.getPoint());

			for( const TreeNode<T>* p = n.getParent(); p != 0L; p = p->getParent() ) {
				if( *(p->getPoint()) == state )
					return true;
			}
			return false;
		}

        /** accessor, the children. Creates the children in the first call */
		template<typename T>
		typename TreeNode<T>::Children TreeNode<T>::getChildren() {
			if (!eval_) {
				std::vector< boost::shared_ptr<T> >  ch  = point_->getChildren();

				for (unsigned i = 0; i < ch.size(); ++i) {
					TreeNode<T>* p = new TreeNode(ch[i], this );
					children_.push_back(p);
				}
				eval_   = true;
			}
			return children_;
		}

        /** accessor, the children. Creates the children in the first call.
		 Calculates the weight as a sum of weight of parent and weight of child. */
		template<typename T>
		typename TreeNode<T>::Children TreeNode<T>::getChildrenWithWeight() {
			if (!eval_) {
				std::vector< boost::shared_ptr<T> >  ch  = point_->getChildren();

				for (unsigned i = 0; i < ch.size(); ++i) {
					TreeNode<T>* p = new TreeNode(ch[i], this, ch[i]->getWeight() + weight_ );
					children_.push_back(p);
				}
				eval_   = true;
			}
			return children_;
		}

        /** erase the child tree nodes */
        template<typename T>
        void TreeNode<T>::eraseChildren() {
			for(typename Children::iterator i = children_.begin(); i != children_.end(); ++i )
				delete *i;
            children_.clear();
            eval_ = false;
        }

		/** generate the path from given state to the root */
		template<typename T>
		typename Node<T>::Path TreeNode<T>::generatePathToRoot( ) const {

			typename Node<T>::Path path;

			for(const TreeNode<T>* s = this; s != 0L; ) {
				path.push_back( s->getPoint() );
				s = s->getParent();
			}
			return path;
		}

    } //namespace search
} //namespace faif

#endif //FAIF_SEARCHING_TREE_NODE_HPP
