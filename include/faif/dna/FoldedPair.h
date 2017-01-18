// DNA two single stranded chain with second structures
//  @author Robert Nowak

#ifndef FOLDED_PAIR_H
#define FOLDED_PAIR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
//visual studio 8.0 - konwersja pomiedzy unsigned int a size_t
#pragma warning(disable:4267)
//visual studio 8.0 - arytmetyka dla iteratorow przy konwersji na inta
#pragma warning(disable:4244)

#endif

#include "Chain.h"
#include "SecStruct.h"
#include "FoldedMatrix.h"

#include <boost/scoped_ptr.hpp>

namespace faif {
    namespace dna {

        /** matrix with energy */
        class FoldedMatrix;
        /** startegy to calculate matrix */
        class FoldedMatrixStrategy;

        /** \brief Two DNA chains with secondary structures.

            Two DNA chains with secondary structures
            (created between them as well as the self-secondary structures)
        */
        class FoldedPair : boost::noncopyable  {
        public:
            explicit FoldedPair(const Chain& first_chain, const Chain& second_chain,
                                const EnergyNucleo& energy, unsigned int max_foldings = 100);

            ~FoldedPair() {}

            /** accessor */
            const Chain& getFirstChain() const { return first_; }

            /** accessor */
            const Chain& getSecondChain() const { return second_; }

            /** the second structure energy. Calculated on the first time, when method is called. */
            EnergyValue getSecStructEnergy() const {
                return lazyCreate()->getSecStructEnergy();
            }

            /** single second structure, algorithm search graph in depth */
            SecStruct findInDepth() const {
                return lazyCreate()->findInDepth();
            }

            /** collection of second structures, algorithm search graph in width,
                the max_foldings parameters restricts the number of structures */
            const SecStructures& getStructures() const {
                return lazyCreate()->getStructures();
            }

            /** print the matrix (for testing and debugging) */
            std::ostream& printMatrix(std::ostream& os, int print_width = 4) const {
                if( proxy_.get() != 0L)
                    proxy_->printMatrix(os, print_width);
                else
                    os << "energy matrix not created" << std::endl;
                return os;
            }

            /** print the secondary structures (for testing and debugging) */
            std::ostream& printStructures(std::ostream& os, int print_width = 4) const {
                if( proxy_.get() != 0L)
                    proxy_->printStructures(os, print_width);
                else
                    os << "energy matrix not created" << std::endl;
                return os;
            }

            /** the support class - holding the matrix with energy and secondary structures */
            typedef boost::scoped_ptr<FoldedMatrix> FoldedMatrixPtr;
            /** the strategy to access for FoldedMatrix (one or two DNA chains) */
            typedef boost::scoped_ptr<FoldedMatrixStrategy> FoldedMatrixStrategyPtr;
        private:
            /** the analyzed DNA chains */
            Chain first_;
            Chain second_;
            /** the energy of nucleotide pairs */
            const EnergyNucleo& energy_;

            /** maximum number of returned second structures */
            unsigned int max_foldings_;

            /** strategy for FoldedMatrix to index two DNA chains */
            FoldedMatrixStrategyPtr strategy_;

            /** obiekt, ktory przechowuje obliczone wartosci */
            mutable FoldedMatrixPtr proxy_;

            /** creates the proxy if necessary */
            FoldedMatrixPtr& lazyCreate() const {
                if( proxy_.get() == 0L )
                    proxy_.reset( new FoldedMatrix(*strategy_, max_foldings_) );
                return proxy_;
            }
        };

        namespace {

            /** strategy to calculate FoldedMatrix, when one chain (single chain) is calculated */
            class DoubleMatrixStrategy : public FoldedMatrixStrategy {
            public:
                DoubleMatrixStrategy( const EnergyNucleo& energy, const Chain& first, const Chain& second )
                    : FoldedMatrixStrategy(energy),
                      first_(first),
                      second_(second),
                      splitIdx_(first_.getLength() )
                {}

                virtual ~DoubleMatrixStrategy() {}

                /** return the iterator for nuleotide of i-th index in matrix */
                virtual Chain::const_iterator getNucleotide(int index) const {

                    if(index < splitIdx_) {
						Chain::const_iterator it = first_.begin();
						it += index;
						return it;
                    }
                    else {
                        index -= splitIdx_;
                        Chain::const_iterator it = second_.begin();
                        it += index;
                        return it;
                    }
                }

                /** returns the split index (the first index belonging to the second chain) */
                virtual int getSplitIndex() const { return splitIdx_; }

                /** returns the last valid index */
                virtual int getLength() const { return first_.getLength() + second_.getLength(); }
            private:
                const Chain& first_;
                const Chain& second_;
                int splitIdx_;
            };

        } //namespace

        inline FoldedPair::FoldedPair(const Chain& first_chain, const Chain& second_chain,
                               const EnergyNucleo& energy, unsigned int max_foldings)
            : first_(first_chain),
              second_(second_chain),
              energy_(energy),
              max_foldings_(max_foldings),
              strategy_( new DoubleMatrixStrategy(energy_, first_, second_) ),
              proxy_(0L)
        { }



        //print the chain sequences and one secondary structure
        inline std::ostream& operator<<(std::ostream& os, const FoldedPair& folded) {
            os << folded.getFirstChain().getString() << std::endl;
            os << folded.getSecondChain().getString() << std::endl;
            os << folded.findInDepth();
            return os;
        }

    } //namespace dna
} //namespace faif

#endif //FOLDER_PAIR_H
