// DNA single stranded chain with second structures
//  @author Robert Nowak

#ifndef FOLDED_CHAIN_H
#define FOLDED_CHAIN_H

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

        /** \brief DNA strand with secondary structure

            Single DNA strand with secondary structures calculated by given algorithm.
        */
        class FoldedChain : boost::noncopyable  {
        public:
            explicit FoldedChain(const Chain& chain, const EnergyNucleo& energy, unsigned int max_foldings = 100);

            ~FoldedChain(){}

            /** accessor */
            const Chain& getChain() const { return chain_; }

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
            /** the analyzed DNA chain */
            Chain chain_;
            /** the energy of nucleotide pairs */
            const EnergyNucleo& energy_;

            /** maximum number of returned second structures */
            unsigned int max_foldings_;

            /** strategy for FoldedMatrix to index single chain DNA */
            FoldedMatrixStrategyPtr strategy_;

            /** obiekt, ktory przechowuje obliczone wartosci */
            mutable FoldedMatrixPtr proxy_;

            /** creates the proxy if necessary */
            FoldedMatrixPtr& lazyCreate() const {
				if( proxy_.get() == 0L )
					proxy_.reset( new FoldedMatrix(*strategy_,  max_foldings_ ) );
				return proxy_;
            }
        };

        namespace {

            /** strategy to calculate FoldedMatrix, when one chain (single chain) is calculated */
            class SingleMatrixStrategy : public FoldedMatrixStrategy {
            public:
                SingleMatrixStrategy( const EnergyNucleo& energy, const Chain& chain )
                    : FoldedMatrixStrategy(energy), chain_(chain) {}
                virtual ~SingleMatrixStrategy() {}

                /** return the iterator for nuleotide of i-th index in matrix */
                virtual Chain::const_iterator getNucleotide(int index) const {
                    Chain::const_iterator it = chain_.begin();
                    it += index;
                    return it;
                }

                /** returns the split index (the first index belonging to the second chain) */
                virtual int getSplitIndex() const { return chain_.getLength(); }

                /** returns the last valid index */
                virtual int getLength() const { return chain_.getLength(); }

            private:
                const Chain& chain_;
            };

        } //namespace

        inline FoldedChain::FoldedChain(const Chain& chain, const EnergyNucleo& energy, unsigned int max_foldings)
            : chain_(chain),
              energy_(energy),
              max_foldings_(max_foldings),
              strategy_( new SingleMatrixStrategy(energy_, chain_) ),
              proxy_(0L)
        { }


        /** print the chain sequence and one secondary structure */
        inline std::ostream& operator<<(std::ostream& os, const FoldedChain& folded) {
            os << folded.getChain().getString();
            os << folded.findInDepth();
            return os;
        }




    } //namespace dna
} //namespace faif

#endif //CHAIN_H
