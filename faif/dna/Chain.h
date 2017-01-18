// dna single stranded chain
// @author Robert Nowak

#ifndef CHAIN_H
#define CHAIN_H

#include <cassert>
#include <ostream>
#include <limits>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>

#include "Nucleotide.h"

namespace faif {
    namespace dna {

        /** \brief The DNA strand (single).

            Sequence of nucleotides,
            the begin of sequence = 5' end of molecule, the end of sequence = 3' end of molecule
        */
        class Chain {
            /** support type, collects nucleotides */
            typedef std::vector<Nucleotide> NucleotideChain;
            //support (shorter code)
            typedef NucleotideChain::iterator NucleotideIter;
            //support (shorter code)
            typedef NucleotideChain::const_iterator ConstNucleotideIter;
        public:
            class const_iterator;

            /** iterator for nucleotide */
            class iterator : public NucleotideIter {
                friend class const_iterator;
            public:
                iterator(Chain& ch, NucleotideIter it) : NucleotideIter(it), chain_(ch) {}
                Chain& getChain() { return chain_; }

                /** comparison take into account the chain */
                bool operator==(const iterator& it) const {
                    if(&chain_ != &it.chain_)
                        return false;
                    return static_cast<NucleotideIter>(*this) == static_cast<NucleotideIter>(it);
                }
            private:
                iterator& operator=(const iterator&); //zabronione przypisanie
                Chain& chain_;
            };

            /** const iterator for nucleotide */
            class const_iterator : public ConstNucleotideIter {
            public:
                const_iterator(const Chain& ch, ConstNucleotideIter it) : ConstNucleotideIter(it), chain_(ch) {}
                const_iterator(const iterator& it) : ConstNucleotideIter(it), chain_(it.chain_) {}
                const Chain& getChain() const { return chain_; }

                /** comparison take into account the chain */
                bool operator==(const const_iterator& it) const {
                    if(&chain_ != &it.chain_)
                        return false;
                    return static_cast<ConstNucleotideIter>(*this) == static_cast<ConstNucleotideIter>(it);
                }
            private:
                const_iterator& operator=(const const_iterator&); //zabronione przypisanie
                const Chain& chain_;
            };

            explicit Chain(){}
            // Chain(const NucleotideChain& n) : nucleos_(n) {}
            Chain(const Chain& ch) : nucleos_(ch.nucleos_) {}
            Chain(const std::string& seq) {
                for(std::string::const_iterator it = seq.begin(); it != seq.end(); ++it)
                    operator+=( create(*it) );
            }
            /** copy constructor from range */
            Chain(const const_iterator& begin, const const_iterator& end) {
                std::copy( begin, end, std::back_inserter(nucleos_) );
            }
            ~Chain(){}

            Chain& operator=(const Chain& ch) {
                nucleos_=ch.nucleos_;
                return *this;
            }

            /** insert a new nucleotide at the and */
            Chain& operator+=(const Nucleotide& val) {
                nucleos_.push_back(val);
                return *this;
            }

            Chain& operator+=(const Chain& ch) {
                std::copy(ch.nucleos_.begin(), ch.nucleos_.end(), std::back_insert_iterator<NucleotideChain>(nucleos_) );
                return *this;
			}

			/** comparison of chains */
            bool operator==(const Chain& ch)const { return nucleos_ == ch.nucleos_; }
			/** comparison of chains */
			bool operator!=(const Chain& ch)const { return nucleos_ != ch.nucleos_; }

			/** accessor for index, throws out_of_range */
			const Nucleotide& operator[](int index) const { return nucleos_.at(index); }

			/** accessor for index, throws out_of_range */
			Nucleotide& operator[](int index) {
				return const_cast<Nucleotide&>(static_cast<const Chain*>(this)->operator[](index) );
			}

			/** accessor - length */
			int getLength() const { return static_cast<int>(nucleos_.size() ); }

			/** mutator - iterator */
			iterator begin() { return iterator(*this,nucleos_.begin() ); }
			/** mutator - const iterator */
			const_iterator begin() const { return const_iterator(*this,nucleos_.begin()); }
			/** mutator - iterator */
			iterator end() { return iterator(*this,nucleos_.end()); }
			/** mutator - const iterator */
			const_iterator end() const { return const_iterator(*this,nucleos_.end()); }
			/** the string for chain */
			std::string getString()const {
				std::string str;
				for(NucleotideChain::const_iterator it = nucleos_.begin(); it != nucleos_.end(); ++it) {
					str += static_cast<char>(it->get() );
				}
				return str;
			}
			/** creates new complementary chain (always the 5' end on index 0) */
			Chain complementary() const {
				Chain complChain;
				for(NucleotideChain::const_reverse_iterator it = nucleos_.rbegin(); it != nucleos_.rend(); ++it)
					complChain += it->complementary();
				return complChain;
			}
		private:
			NucleotideChain nucleos_;
		};

		/** creator (support): sub-chain, using index).
			Returns the largest chain starting from given index and having length less or equal given length. */
		inline Chain createSubChain(const Chain& chain, int start, int length) {
			if(start >= chain.getLength() )
				start = chain.getLength();

			int finish = start + length;
			if( start + length >= chain.getLength() )
				finish = chain.getLength();

			Chain::const_iterator beg = chain.begin();
			beg += start;
			Chain::const_iterator end = chain.begin();
			end += finish;
			return Chain(beg, end);
		}


		/** stream operator (support for debugging) */
		inline std::ostream& operator<<(std::ostream& os, const Chain& chain) {
			os << chain.getString();
			return os;
		}

	} //namespace dna
} //namespace faif

#endif //CHAIN_H
