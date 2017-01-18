/**
 * \file DecisionTree.hpp
 * \brief The Decision Tree Classifier, inspired ID3 algorithm (Iterate Dichotomizer)
 */

#ifndef FAIF_DECISION_TREE_HPP
#define FAIF_DECISION_TREE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
//msvc14.0 warnings for Boost.Serialization
#pragma warning(disable:4100)
#pragma warning(disable:4512)
#endif



#include "Classifier.hpp"

#include <list>
#include <set>
#include <algorithm>
#include <iterator>
#include <limits>

#include <boost/ref.hpp>
#include <boost/bind.hpp>

#include <boost/lambda/bind.hpp>
#include <boost/lambda/construct.hpp>
#include <boost/lambda/core.hpp>
#include <boost/lambda/lambda.hpp>

#include <boost/serialization/split_member.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/singleton.hpp>
#include <boost/serialization/extended_type_info.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>

namespace faif {
    namespace ml {

        /** \brief param for training decision tree */
        struct DecisionTreeTrainParam {
            DecisionTreeTrainParam() : allowedNbrMiscEx(1) {}
            int allowedNbrMiscEx; //allowed number of badly classified examples for each category
        };

        const double MIN_INF_GAIN = 0.000001; // minimal information gain to accept test

        /** \brief Decision Tree Classifier.

            Contains the attributes, attribute values and categories,
            train examples, test examples and classifier methods.
        */
        template<typename Val>
        class DecisionTree : public Classifier<Val> {
        public:
            typedef typename Classifier<Val>::AttrValue AttrValue;
            typedef typename Classifier<Val>::AttrDomain AttrDomain;
            typedef typename Classifier<Val>::AttrIdd AttrIdd;
            typedef typename Classifier<Val>::AttrIddSerialize AttrIddSerialize;
            typedef typename Classifier<Val>::Domains Domains;
            typedef typename Classifier<Val>::Beliefs Beliefs;
            typedef typename Classifier<Val>::ExampleTest ExampleTest;
            typedef typename Classifier<Val>::ExampleTrain ExampleTrain;
            typedef typename Classifier<Val>::ExamplesTrain ExamplesTrain;
        public:
            DecisionTree();
            DecisionTree(const Domains& attr_domains, const AttrDomain& category_domain);

            virtual ~DecisionTree() { }

            /** clear the tree */
            virtual void reset();

            /** \brief learn classifier (on the collection of training examples).
             */
            virtual void train(const ExamplesTrain& e);

            /** classify */
            virtual AttrIdd getCategory(const ExampleTest&) const;

            /** \brief classify and return all classes with belief that the example is from given class */
            virtual Beliefs getCategories(const ExampleTest&) const;

            /** the ostream method */
            virtual void write(std::ostream& os) const;

            /** accessor - get training parameters */
            const DecisionTreeTrainParam& getTrainParam() const { return param_; }

            /** mutator - set training parameters */
            void setTrainParam(const DecisionTreeTrainParam& p) { param_ = p; }

            /**
               \brief prune tree - plase not use the example set used for training

               Return the (smart)pointer to node which
               replace the old one. If no prunning is performed the input pointer and the output are the same.

               bottom-up method, the uneven distribution of categories is not considered
            */
            void prune(const ExamplesTrain& e);
        private:
            //forward declaration
            class DTNode;
            typedef boost::shared_ptr<DTNode> PDTNode;

            PDTNode root_; //main node for decision tree
            DecisionTreeTrainParam param_; //params for training decision tree

            /** copy c-tor not allowed */
            DecisionTree(const DecisionTree&);
            /** assignment not allowed */
            DecisionTree& operator=(const DecisionTree&);

        private:
            /**
               internal class - binary test (stored in each node), currently (for nomial values) equality test
            */
            class DTTest
            {
            public:
                DTTest( ): idd_(AttrDomain::getUnknownId()) {} //for de-serialization
                explicit DTTest( AttrIdd idd ): idd_(idd) {}
                ~DTTest() {}

                AttrIdd get() const { return idd_; }

                /** \brief perform the test for given example */
                bool test( const ExampleTest& e ) const;

                /** \brief calculate entropy gain for given test. The return value is normalized. */
                double entropyGain(typename ExamplesTrain::const_iterator eBeg, typename ExamplesTrain::const_iterator eEnd) const;

                /** \brief ostream method */
                void write(std::ostream& os) const;
            private:

                /** \brief serialization using boost::serialization */
                friend class boost::serialization::access;

                template<class Archive>
                void save(Archive & ar, const unsigned int /* file_version */) const {
                    ar & boost::serialization::make_nvp("Idd", idd_ );
                }

                template<class Archive>
                void load(Archive & ar, const unsigned int /* file_version */) {
                    AttrIddSerialize i;
                    ar >> boost::serialization::make_nvp("Idd", i);
                    idd_ = const_cast<AttrIdd>(i);
                }

                template<class Archive>
                void serialize( Archive &ar, const unsigned int file_version ){
                    boost::serialization::split_member(ar, *this, file_version);
                }

            private:
                AttrIdd idd_;
            };

            /** collection of tests */
            typedef std::list<DTTest> DTTests;

            /**
               \brief internal class - Node in decision tree classifier (leaf)
            */
            class DTNode
            {
            public:
                DTNode() {} //for de-serialization
                DTNode(Beliefs catBel) : catBel_(catBel) {}
                virtual ~DTNode() {}

                //major category for given node
                AttrIdd getMajorCategory() const {
                    if( catBel_.empty() )
                        return AttrDomain::getUnknownId();
                    else
                        return catBel_.front().getValue();
                }

                //categories with belief for given node
                const Beliefs& getBeliefs() const { return catBel_; }

                //test if the node is leaf node
                bool isLeaf() const { return getTest() == 0L; }

                //factory method
                static PDTNode createLeaf(const Beliefs& catBel);
                //factory method
                static PDTNode createInternal(const Beliefs& catBel, const DTTest& test, PDTNode nTrue, PDTNode nFalse);

                //empty node - node when test return true
                virtual PDTNode getNodeTrue() const { return PDTNode(); }
                //empty node - node when test return false
                virtual PDTNode getNodeFalse() const { return PDTNode(); }
                //test stored in node (null)
                virtual const DTTest* getTest() const { return 0L; }
                /** mutator - set node when test return true. Empty operation for LeafNode. */
                virtual void setNodeTrue(PDTNode) { }
                /*** mutator - set node when test return false. Empty operation for LeafNode. */
                virtual void setNodeFalse(PDTNode) { }

                /** \brief classify, return category for given testing example */
                virtual AttrIdd getCategory(const ExampleTest& e) const {
                    return getMajorCategory();
                }

                /** \brief classify, return categories and belief for given testing example */
                virtual const Beliefs& getCategories(const ExampleTest& e) const {
                    return getBeliefs();
                }

                //for debugging
                virtual void write(std::ostream& os) const {
                    os << "Leaf (Major:" << getMajorCategory()->get() << ", Beliefs:" << getBeliefs() << ");";
                }
            private:
                /** \brief serialization using boost::serialization */
                friend class boost::serialization::access;

                template<class Archive>
                void serialize( Archive &ar, const unsigned int /* file_version*/ ){
                    ar & boost::serialization::make_nvp("CatBel", catBel_ );
                }

            private:
                Beliefs catBel_;
            };



            /**
               \brief interanal class - internal node (with test and left and right children)
            */
            class DTNodeInternal : public DTNode {
            public:
                DTNodeInternal() {} //for de-serialization
                DTNodeInternal(const Beliefs& catBel, const DTTest& test, PDTNode nTrue, PDTNode nFalse)
                    : DTNode(catBel), test_(test), nodeTrue_(nTrue), nodeFalse_(nFalse)
                {}

                //node when test return true
                virtual PDTNode getNodeTrue() const { return nodeTrue_; }
                //node when test return false
                virtual PDTNode getNodeFalse() const { return nodeFalse_; }
                //test stored in node
                virtual const DTTest* getTest() const { return &test_; }
                /** mutator - set node when test return true. Empty operation for LeafNode. */
                virtual void setNodeTrue(PDTNode n) { nodeTrue_ = n; }
                /*** mutator - set node when test return false. Empty operation for LeafNode. */
                virtual void setNodeFalse(PDTNode n) { nodeFalse_ = n; }

                /** \brief classify, return category for given testing example */
                virtual AttrIdd getCategory(const ExampleTest& e) const {
                    if( test_.test(e) )
                        return nodeTrue_->getCategory(e);
                    else
                        return nodeFalse_->getCategory(e);
                }

                /** \brief classify, return categories and belief for given testing example */
                virtual const Beliefs& getCategories(const ExampleTest& e) const {
                    if( test_.test(e) )
                        return nodeTrue_->getCategories(e);
                    else
                        return nodeFalse_->getCategories(e);
                }

                //for debugging
                virtual void write(std::ostream& os) const {
                    os << "Internal (Major:" << this->getMajorCategory()->get() << ", Beliefs:" << this->getBeliefs() << ", test:";
                    test_.write(os);
                    os << ");";
                }
            private:
                /** \brief serialization using boost::serialization */
                friend class boost::serialization::access;

                template<class Archive>
                void serialize( Archive &ar, const unsigned int /* file_version*/ ){
                    ar & boost::serialization::make_nvp("NodeBase", boost::serialization::base_object<DTNode>(*this) );
                    ar & boost::serialization::make_nvp("Test", test_ );
                    ar & boost::serialization::make_nvp("NodeTrue", nodeTrue_ );
                    ar & boost::serialization::make_nvp("NodeFalse", nodeFalse_ );
                }
            private:
                DTTest test_; //binary test
                PDTNode nodeTrue_; //node when test return true
                PDTNode nodeFalse_; //node when test return false
            };

            /**
               \brief recurent function to build decision tree
               \param eBeg training examples collection (iterator). The examples are re-order (partitioned) by tests (split)
               \param eEnd training examples collection (iterator).
               \param inTest the initial collection of tests
               \param ALLOWED_NBR_MISC_EX allowed number of badly classified examples for each category
            */
            static PDTNode buildTreeRecur(typename ExamplesTrain::iterator eBeg, typename ExamplesTrain::iterator eEnd,
                                          const DTTests& inTests, const int ALLOWED_NBR_MISC_EX);

            /**
               \brief recurent function to prune decision tree
               \param eBeg pruning examples collection (iterator). The examples are re-order (partitioned) by tests (split)
               \param eEnd pruning examples collection (iterator).
               \param node the considered node. It is returned or changed into other node (internal node into leaf node).
            */
            static PDTNode pruneTreeRecur(typename ExamplesTrain::iterator eBeg, typename ExamplesTrain::iterator eEnd, PDTNode node);

            /**
               \brief helping function for ostream operator
            */
            static void writeDecTreeNodes(std::ostream& os, typename DecisionTree<Val>::PDTNode node, int level = 0);

            /** \brief serialization using boost::serialization */
            friend class boost::serialization::access;

            template<class Archive>
            void serialize( Archive &ar, const unsigned int /* file_version */ ) {
				ar.template register_type<DTNode>();
				ar.template register_type<DTNodeInternal>();

				ar & boost::serialization::make_nvp("DTCBase", boost::serialization::base_object<Classifier<Val> >(*this) );
				ar & boost::serialization::make_nvp("Node", root_ );
            }

        }; //class DecisionTree

        //////////////////////////////////////////////////////////////////////////////////////////////////
        // class DecisionTree implementation
        //////////////////////////////////////////////////////////////////////////////////////////////////

        template<typename Val>
        DecisionTree<Val>::DecisionTree() : Classifier<Val>()
        {
        }

        template<typename Val>
        DecisionTree<Val>::DecisionTree(const Domains& attr_domains, const AttrDomain& category_domain)
            : Classifier<Val>(attr_domains, category_domain)
        {
        }

        /** clear the tree */
        template<typename Val>
        void DecisionTree<Val>::reset() {
            root_ = PDTNode();
        }

        /**
           \brief learn classifier (on the collection of training examples), the decision tree using given train examples
           \param e training examples collection
           \param ALLOWED_NBR_MISC_EX allowed number of badly classified examples for each category
        */
        template<typename Val>
        void DecisionTree<Val>::train(const ExamplesTrain& e) {

        	ExamplesTrain ex(e); //make a copy, because the container will be changed (split re-order examples in sets)
        	std::set<AttrIdd> attrib; // structure for available tests for train examples collection

        	// Look through all examples and remove redundant attributes, if exists
        	for (typename ExamplesTrain::iterator it = ex.begin(); it != ex.end(); ++it) {
        		for(typename ExampleTrain::iterator at = it->begin(); at != it->end();) {
            		AttrIdd& value = *at;
                	if( std::find(Classifier<Val>::getAttrDomains().begin(),
                			Classifier<Val>::getAttrDomains().end(),
                			value->getDomain()->getId()) != Classifier<Val>::getAttrDomains().end())
                		++at;
                	else
                		at = it->erase(at);

            	}
                // generate available tests for train examples collection
                const ExampleTrain& ee = *it;
                std::copy(ee.begin(), ee.end(), std::inserter(attrib, attrib.begin() ) );
            }

            DTTests tests;
            std::transform(attrib.begin(), attrib.end(), std::back_inserter(tests),
                           boost::lambda::bind(boost::lambda::constructor<DTTest>(), boost::lambda::_1 ) );

            root_ =  buildTreeRecur(ex.begin(), ex.end(), tests, param_.allowedNbrMiscEx);

        }

        /** classify - return the major category for best node from decision tree */
        template<typename Val>
        typename DecisionTree<Val>::AttrIdd DecisionTree<Val>::getCategory(const ExampleTest& e) const {
            if(!root_) { //empty tree
                return AttrIdd(AttrDomain::getUnknownId());
            } else if(e.empty() ) { //no  common attrib (domains) between example and classifier
                return root_->getMajorCategory();
            } else { //classify using decision tree
                return root_->getCategory(e);
            }
        }

        /** \brief classify and return all classes with belief that the example is from given class */
        template<typename Val>
        typename DecisionTree<Val>::Beliefs DecisionTree<Val>::getCategories(const ExampleTest& e) const {
            if(!root_) { //empty tree
                return Beliefs();
            } else if(e.empty() ) { //no  common attrib (domains) between example and classifier
                return root_->getBeliefs();
            } else { //classify using decision tree
                return root_->getCategories(e);
            }
        }

        /** ostream method */
        template<typename Val>
        void DecisionTree<Val>::write(std::ostream& os) const {
            if(!root_)
                os << "Empty DTC" << std::endl;
            else
                writeDecTreeNodes(os, root_, 0 );
        }

        /**
           \brief prune tree - plase not use the example set used for training

           Return the (smart)pointer to node which
           replace the old one. If no prunning is performed the input pointer and the output are the same.

           bottom-up method, the uneven distribution of categories is not considered
        */
        template<typename Val>
        void DecisionTree<Val>::prune(const ExamplesTrain& e) {
            if(root_) {
                ExamplesTrain ex(e); //make a copy, because the container will be changed (split re-order examples in sets)
                pruneTreeRecur(ex.begin(), ex.end(), root_ );
            }
        }

        /** \brief recurent function to build decision tree
            \param eBeg training examples collection (iterator). The examples are re-order (partitioned) by tests (split)
            \param eEnd training examples collection (iterator).
            \param inTest the initial collection of tests
            \param ALLOWED_NBR_MISC_EX allowed number of badly classified examples for each category
        */
        template<typename Val>
        typename DecisionTree<Val>::PDTNode
        DecisionTree<Val>::buildTreeRecur(typename ExamplesTrain::iterator eBeg, typename ExamplesTrain::iterator eEnd,
                                          const DTTests& inTests, const int ALLOWED_NBR_MISC_EX) {

            //calculate histogram of categories for train example collection

            TrainExampleCategoryCounters<Val> counters(eBeg, eEnd);
            Beliefs histogram = counters.getHistogram();

            int numCatWithManyExamples = 0;
            const std::map<AttrIdd,int>& c = counters.get();
            for(typename std::map<AttrIdd,int>::const_iterator i = c.begin(); i != c.end(); ++i) {
                if( i->second > ALLOWED_NBR_MISC_EX) {
                    ++numCatWithManyExamples;
                }
            }
            if(static_cast<int>(std::distance(eBeg, eEnd)) <= ALLOWED_NBR_MISC_EX || //no enough training examples - split is not sensible
               numCatWithManyExamples < 2) { //only few examples in not-major category
                return DTNode::createLeaf(histogram);
            }
            //find the best test
            DTTests tests(inTests);
            typename DTTests::iterator best = tests.end();
            double bestEntropy = std::numeric_limits<double>::min();

            for(typename DTTests::iterator i = tests.begin(); i != tests.end(); ++i ) {
                double entr = i->entropyGain(eBeg, eEnd);
                if(entr > bestEntropy) {
                    bestEntropy = entr;
                    best = i;
                }
            }
            if( best == tests.end() || bestEntropy < MIN_INF_GAIN ) { //no tests in tests set or no goot tests
                return DTNode::createLeaf(histogram);
            }
            // std::cout << "best test:" << *best << " entropy gain:" << bestEntropy << std::endl;

            //split the examples using best test
            typename ExamplesTrain::iterator middle = std::stable_partition(eBeg, eEnd, boost::bind(&DTTest::test, boost::ref(*best), _1) );
            DTTest bestTest(*best);
            tests.erase(best);
            PDTNode nTrue = buildTreeRecur(eBeg, middle, tests, ALLOWED_NBR_MISC_EX);
            PDTNode nFalse = buildTreeRecur(middle, eEnd, tests, ALLOWED_NBR_MISC_EX);
            return DTNode::createInternal(histogram, bestTest, nTrue, nFalse);
        }

        /** \brief recurent function to prune decision tree
            \param eBeg pruning examples collection (iterator). The examples are re-order (partitioned) by tests (split)
            \param eEnd pruning examples collection (iterator).
            \param node the considered node. It is returned or changed into other node (internal node into leaf node).
        */
        template<typename Val>
        typename DecisionTree<Val>::PDTNode
        DecisionTree<Val>::pruneTreeRecur(typename ExamplesTrain::iterator eBeg, typename ExamplesTrain::iterator eEnd, PDTNode node) {

            if(node->isLeaf() || std::distance(eBeg, eEnd) < 1)
                return node;

            //here assertion that for !node->isLeaf() node->getTest() return valid address
            const DTTest& t = *(node->getTest());
            //split the examples using the test from node
            typename ExamplesTrain::iterator
                middle = std::stable_partition(eBeg, eEnd, boost::bind(&DTTest::test, boost::ref(t), _1) );

            node->setNodeTrue( pruneTreeRecur( eBeg, middle, node->getNodeTrue() ) );
            node->setNodeFalse( pruneTreeRecur( middle, eEnd, node->getNodeFalse() ) );

            //count the number of correctly classified pruning exampes
            int leafCount = 0, treeCount = 0;
            for(typename ExamplesTrain::const_iterator i = eBeg; i != eEnd; ++i) {
                const ExampleTrain& e = *i;
                if(node->getMajorCategory() == e.getFeature() )
                    ++leafCount;
                if(node->getCategory(e) == e.getFeature())
                    ++treeCount;
            }
            // std::cout << "pruning:" << *node << " examples:" << std::distance(eBeg, eEnd)
            //        << " leafCount: " << leafCount << " treeCount: " << treeCount << std::endl;
            if( leafCount >= treeCount ) { //major category for node is good enough
                return DTNode::createLeaf(node->getBeliefs()); //switch to leaf
            }
            return node;
        }

        /**
           \brief helping function for ostream operator
        */
        template<typename Val>
        void DecisionTree<Val>::writeDecTreeNodes(std::ostream& os, typename DecisionTree<Val>::PDTNode node, int level) {
            if( node ) {
                os << std::string(level,' ');
                node->write(os);
                os << std::endl;
                writeDecTreeNodes(os, node->getNodeTrue(), level+1);
                writeDecTreeNodes(os, node->getNodeFalse(), level+1);
            }
        }

        //////////////////////////////////////////////////////////////////////////////////////////////////
        // class DecisionTree::DTTest implementation
        //////////////////////////////////////////////////////////////////////////////////////////////////

        /** \brief calculate entropy gain for given test. The return value is normalized. */
        template<typename Val>
        double DecisionTree<Val>::DTTest::entropyGain(typename ExamplesTrain::const_iterator eBeg, typename ExamplesTrain::const_iterator eEnd) const {

            if( eBeg == eEnd ) //not start calculation for empty set
                return 0.0;

            TrainExampleCategoryCounters<Val> acc;
            TrainExampleCategoryCounters<Val> nacc;

            for(typename ExamplesTrain::const_iterator i = eBeg; i != eEnd; ++ i) {
                const ExampleTrain& ex = *i;
                if( this->test(ex) ) {
                    acc.inc(ex);
                }
                else {
                    nacc.inc(ex);
                }
            }
            double sum = static_cast<double>( std::distance(eBeg, eEnd) );
            double nrAcc = static_cast<double>(acc.getSum() );
            double nrNAcc = static_cast<double>(nacc.getSum() );
            double testIc =  calcEntropy( nrAcc / sum ) + calcEntropy( nrNAcc/sum );
            if( testIc < MIN_INF_GAIN ) {
                return 0.0;
            }
            else {
                double entropy = acc.entropy() * nrAcc / sum + nacc.entropy() * nrNAcc / sum;
                TrainExampleCategoryCounters<Val> befSplit(eBeg, eEnd);
                double gain = befSplit.entropy() - entropy;
                return gain / testIc;
            }
        }

        /** \brief perform the test for given example */
        template<typename Val>
        bool DecisionTree<Val>::DTTest::test( const ExampleTest& e ) const {
            return std::find(e.begin(), e.end(), idd_) != e.end();
        }

        /** \brief ostream method */
        template<typename Val>
        void DecisionTree<Val>::DTTest::write(std::ostream& os) const {
            os << "Domain: " << idd_->getDomain()->getId() << ", Value:" << idd_->get();
        }

        //////////////////////////////////////////////////////////////////////////////////////////////////
        // class DecisionTree::DTNode implementation
        //////////////////////////////////////////////////////////////////////////////////////////////////

        //factory method
        template<typename Val>
        typename DecisionTree<Val>::PDTNode
        DecisionTree<Val>::DTNode::createLeaf(const Beliefs& catBel) {
            return PDTNode(new DTNode(catBel) );
        }

        //factory method
        template<typename Val>
        typename DecisionTree<Val>::PDTNode
        DecisionTree<Val>::DTNode::createInternal(const Beliefs& catBel, const DTTest& test, PDTNode nTrue, PDTNode nFalse) {
            return PDTNode(new DTNodeInternal(catBel, test, nTrue, nFalse) );
        }

    }//namespace ml
} //namespace faif



#endif //FAIF_DECISION_TREE_HPP
