// collections of connections between DNA pairs, represent secondary structures
//  @author Robert Nowak

#ifndef SECSTRUCT_H
#define SECSTRUCT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
//visual studio 8.0 - the + - operators for iterators and conversion iterator to int
#pragma warning(disable:4244)
#endif

#include <set>
#include <algorithm>
#include <numeric>
#include <iterator>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>

#include "Chain.h"
#include "EnergyNucleo.h"

namespace faif {
    namespace dna {

        /** \brief The pair of nucleotides which are join by Watson-Crick interaction

			The pair of iterators to nucleotides which are join.
			The nucleotides are from the same chain or from the different chains
		*/
		struct ConnectPair : public std::pair<Chain::const_iterator, Chain::const_iterator> {

			/** c-tor */
			explicit ConnectPair(const Chain::const_iterator& f, const Chain::const_iterator& s)
				: std::pair<Chain::const_iterator, Chain::const_iterator>(f,s) {}

			/** the comparison, the view order is enforced, e.g. (0,15) < (0,14) < (1,16) */
			bool operator<(const ConnectPair& pair) const {
				if(first != pair.first)
					return first < pair.first;
				return pair.second < second;
			}
		};

        /** support - to debuggiog */
        inline std::ostream& operator<<(std::ostream& os, const ConnectPair& p) {
            os << '(' << p.first - p.first.getChain().begin()
               << ',' << p.second - p.second.getChain().begin() << ')';
            return os;
		}

        /** \brief the secondary structure

			The collection of ConnectPair, represents the secondary structure.
        */
        class SecStruct {
        public:
            /** the folded pairs collection - secondary structure */
            typedef std::set<ConnectPair> Foldings;

			/** constructor */
            SecStruct(){}

			/** copy constructor */
            SecStruct(const SecStruct& s) : foldings_(s.foldings_) {}

			/** destructor */
            ~SecStruct(){}

			/** assign operator */
			SecStruct& operator=(const SecStruct& s) {
				foldings_ = s.foldings_;
				return *this;
			}

            /** adds the pair to secondary structure */
            void addPair(const ConnectPair& p) { foldings_.insert(p); }

            /** append the collection of pairs to secondary structure */
            void append(const SecStruct& s) { foldings_.insert(s.foldings_.begin(), s.foldings_.end() ); }

			/** identity */
            bool operator==(const SecStruct& s) const { return foldings_ == s.foldings_; }

			/** comparison */
            bool operator<(const SecStruct& s) const { return foldings_ < s.foldings_; }

            /** number of folded pairs */
            int size() const { return static_cast<int>( foldings_.size() ); }

            /** calculates the energy of secondary structure. Summarizes all pairs. */
            EnergyValue energy(const EnergyNucleo& energy_matrix) const;

			/** accessor - returns the pair collection */
			const Foldings& getFoldings() const { return foldings_; }

            /** support - to debugging */
            friend std::ostream& operator<<(std::ostream& os, const SecStruct& sec_struct);
        private:
            //Foldings forbidden
            Foldings foldings_;
        };

        /** secondary structure collection */
        typedef std::set<SecStruct> SecStructures;

        namespace {
            //helping functor - energy for single connection
			struct EnergyFunctor  {
                EnergyFunctor(const EnergyNucleo& energy_matrix) : energy_(energy_matrix) {}
                EnergyValue operator()(EnergyValue sum, const ConnectPair& p) {
                    return sum + energy_.getEnergy( *(p.first), *(p.second) );
                }
                const EnergyNucleo& energy_;
			private:
				//forbidden, operator= (warning w msvc)
				EnergyFunctor& operator=(const EnergyFunctor&);
            };
        } //namespace

        inline EnergyValue SecStruct::energy(const EnergyNucleo& energy_matrix) const {
            EnergyFunctor functor(energy_matrix);
            return std::accumulate( foldings_.begin(),  foldings_.end(), 0, functor );
        }

		/** support - to debugging */
        inline std::ostream& operator<<(std::ostream& os,const SecStruct& sec_struct) {
			std::copy(sec_struct.foldings_.begin(), sec_struct.foldings_.end(), std::ostream_iterator<ConnectPair>(os,"") );
            return os;
        }



    } //namespace
} //namespace

#endif //SECSTRUCT_H
