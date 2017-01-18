// singleton, pair of dna nucleotides energies
//  @author Robert Nowak

#ifndef ENERGY_NUCLEO_H
#define ENERGY_NUCLEO_H

#include <utility>
#include <map>

#include "Nucleotide.h"

namespace faif {
    namespace dna {
        /** the energy value */
        typedef int EnergyValue;

        /** the pair of nucleotides */
        typedef std::pair<faif::dna::Nucleotide, faif::dna::Nucleotide> Complementary;

        /** comparison of pair of nucleotides (support) */
        struct lessComplementary {
            bool operator()(const Complementary& a, const Complementary& b) const {
                if(a.first == b.first )
                    return a.second.get() < b.second.get();
                else
                    return a.first.get() < b.first.get();
            }
        };

        /** the default energy value */
        static const EnergyValue DEFAULT_NUCLEO_ENERGY = -1000;

        /** \brief the maps between pair of nucleotides and its energy

            Gives energy for two nucleotides, used f.e. in secondary structure calculation
        */
        class EnergyNucleo
        {
        public:
            /** c-tor */
            explicit EnergyNucleo(EnergyValue defaultEnergy) : defaultEnergy_(defaultEnergy) {}
            /** copy c-tor */
			EnergyNucleo(const EnergyNucleo& energy) : data_(energy.data_), defaultEnergy_(energy.defaultEnergy_) {}
            ~EnergyNucleo(){}
            /** \brief return the energy of given pair */
            EnergyValue getEnergy ( const faif::dna::Nucleotide& a, const faif::dna::Nucleotide& b ) const {
				EnergyMap::const_iterator it = data_.find( Complementary(a,b) );
				if( it != data_.end() )
					return it->second;
				else
					return defaultEnergy_;
            }

            /** \brief stores the pair and its energy */
            void addPair( const faif::dna::Nucleotide& a, const faif::dna::Nucleotide& b, const EnergyValue& val) {
				data_.insert( EnergyMap::value_type( Complementary(a,b), val ) );
            }
        private:
            typedef std::map<Complementary, EnergyValue, lessComplementary > EnergyMap;
            EnergyMap data_;

            // default energy - returned when pair is not found
            const EnergyValue defaultEnergy_;

            // assignment not allowed
            EnergyNucleo& operator=(const EnergyNucleo& energy);
        };

        /** default energy object: GC pair: energy = 3, AT pair: energy = 2, GT pair: energy = 1,
            otherwise energy = -1000 */
        inline EnergyNucleo createDefaultEnergy(EnergyValue defaultEnergy = DEFAULT_NUCLEO_ENERGY) {
            EnergyNucleo energy(defaultEnergy);
            energy.addPair( Nucleotide(GUANINE),  Nucleotide(CYTOSINE), 3 );
			energy.addPair( Nucleotide(CYTOSINE), Nucleotide(GUANINE),  3 );
			energy.addPair( Nucleotide(ADENINE),  Nucleotide(THYMINE),  2 );
			energy.addPair( Nucleotide(THYMINE),  Nucleotide(ADENINE),  2 );
			energy.addPair( Nucleotide(GUANINE),  Nucleotide(THYMINE),  1 );
			energy.addPair( Nucleotide(THYMINE),  Nucleotide(GUANINE),  1 );
            return energy;
		}

    } //namespace dna
} //namespace faif


#endif //ENERGY_NUCLEO
