// singleton , represents the genetic code (corresponding amino acid and codons)

#ifndef CODON_AMINO_TABLE_H
#define CODON_AMINO_TABLE_H

#include"Codon.h"

#include<set>
#include<map>
#include <boost/bind.hpp>

namespace faif {
    namespace dna {


		/** the enum represents amino */
		enum AminoAcid {
			PHENYLALANINE = 'F',
			LEUCINE = 'L',
			ISOLEUCINE = 'I',
			METHIONINE = 'M',
			VALINE = 'V',
			SERINE = 'S',
			PROLINE = 'P',
			THREONINE = 'T',
			ALANINE = 'A',
			TYROSINE = 'Y',
			STOP_CODON = '.',
			HISTIDYNE = 'H',
			GLUTAMINE = 'Q',
			ASPARAGINE = 'N',
			LYSINE = 'K',
			ASPARTIC = 'D',
			GLUTAMIC = 'E',
			CYSTEINE = 'C',
			TRYPTOPHAN = 'W',
			ARGININE = 'R',
			GLYCINE = 'G',
			UNKNOWN = '?'};

        /** \brief codons for given amino, amino for given codon, singleton

			The objects store the maps between codons and aminos.
		*/
        class CodonAminoTable {
        public:
            static CodonAminoTable& getInstance() {
                static CodonAminoTable instance;
                return instance;
            }

            /** return amino for given codon */
            AminoAcid getAmino(const Codon& codon) const;

            /** return set of codons for given amino (all codons)  */
            std::set<Codon> getCodons(AminoAcid amino) const;

            /** return the set of codons for given codon which are equivalent (codes the same amino) */
            std::set<Codon> getCodons(const Codon& codon) const;

        private:
			/** singleton, c-tor not allowed */
            CodonAminoTable();
			/** singleton, copy c-tor not allowed */
            CodonAminoTable(CodonAminoTable&);

            void makeInvertedMap();

            typedef std::map<Codon, AminoAcid> CodonMap;
            typedef std::multimap<AminoAcid, Codon> InvertedCodonMap;
            CodonMap codonMap_;
            InvertedCodonMap invertedCodonMap_;
        };

        inline CodonAminoTable::CodonAminoTable() {
            codonMap_.insert(std::make_pair(Codon("TTT"),PHENYLALANINE));
            codonMap_.insert(std::make_pair(Codon("TTC"),PHENYLALANINE));
            codonMap_.insert(std::make_pair(Codon("TTA"),LEUCINE));
            codonMap_.insert(std::make_pair(Codon("TTG"),LEUCINE));
            codonMap_.insert(std::make_pair(Codon("CTT"),LEUCINE));
            codonMap_.insert(std::make_pair(Codon("CTC"),LEUCINE));
            codonMap_.insert(std::make_pair(Codon("CTA"),LEUCINE));
            codonMap_.insert(std::make_pair(Codon("CTG"),LEUCINE));
            codonMap_.insert(std::make_pair(Codon("ATT"),ISOLEUCINE));
            codonMap_.insert(std::make_pair(Codon("ATC"),ISOLEUCINE));
            codonMap_.insert(std::make_pair(Codon("ATA"),ISOLEUCINE));
            codonMap_.insert(std::make_pair(Codon("ATG"),METHIONINE));
            codonMap_.insert(std::make_pair(Codon("GTT"),VALINE));
            codonMap_.insert(std::make_pair(Codon("GTC"),VALINE));
            codonMap_.insert(std::make_pair(Codon("GTA"),VALINE));
            codonMap_.insert(std::make_pair(Codon("GTG"),VALINE));
            codonMap_.insert(std::make_pair(Codon("TCT"),SERINE));
            codonMap_.insert(std::make_pair(Codon("TCC"),SERINE));
            codonMap_.insert(std::make_pair(Codon("TCA"),SERINE));
            codonMap_.insert(std::make_pair(Codon("TCG"),SERINE));
            codonMap_.insert(std::make_pair(Codon("CCT"),PROLINE));
            codonMap_.insert(std::make_pair(Codon("CCC"),PROLINE));
            codonMap_.insert(std::make_pair(Codon("CCA"),PROLINE));
            codonMap_.insert(std::make_pair(Codon("CCG"),PROLINE));
            codonMap_.insert(std::make_pair(Codon("ACT"),THREONINE));
            codonMap_.insert(std::make_pair(Codon("ACC"),THREONINE));
            codonMap_.insert(std::make_pair(Codon("ACA"),THREONINE));
            codonMap_.insert(std::make_pair(Codon("ACG"),THREONINE));
            codonMap_.insert(std::make_pair(Codon("GCT"),ALANINE));
            codonMap_.insert(std::make_pair(Codon("GCC"),ALANINE));
            codonMap_.insert(std::make_pair(Codon("GCA"),ALANINE));
            codonMap_.insert(std::make_pair(Codon("GCG"),ALANINE));
            codonMap_.insert(std::make_pair(Codon("TAT"),TYROSINE));
            codonMap_.insert(std::make_pair(Codon("TAC"),TYROSINE));
            codonMap_.insert(std::make_pair(Codon("TAA"),STOP_CODON));//stop
            codonMap_.insert(std::make_pair(Codon("TAG"),STOP_CODON));//stop
            codonMap_.insert(std::make_pair(Codon("CAT"),HISTIDYNE));
            codonMap_.insert(std::make_pair(Codon("CAC"),HISTIDYNE));
            codonMap_.insert(std::make_pair(Codon("CAA"),GLUTAMINE));
            codonMap_.insert(std::make_pair(Codon("CAG"),GLUTAMINE));
            codonMap_.insert(std::make_pair(Codon("AAT"),ASPARAGINE));
            codonMap_.insert(std::make_pair(Codon("AAC"),ASPARAGINE));
            codonMap_.insert(std::make_pair(Codon("AAA"),LYSINE));
            codonMap_.insert(std::make_pair(Codon("AAG"),LYSINE));
            codonMap_.insert(std::make_pair(Codon("GAT"),ASPARTIC));
            codonMap_.insert(std::make_pair(Codon("GAC"),ASPARTIC));
            codonMap_.insert(std::make_pair(Codon("GAA"),GLUTAMIC));
            codonMap_.insert(std::make_pair(Codon("GAG"),GLUTAMIC));
            codonMap_.insert(std::make_pair(Codon("TGT"),CYSTEINE));
            codonMap_.insert(std::make_pair(Codon("TGC"),CYSTEINE));//stop
            codonMap_.insert(std::make_pair(Codon("TGA"),STOP_CODON));
            codonMap_.insert(std::make_pair(Codon("TGG"),TRYPTOPHAN));
            codonMap_.insert(std::make_pair(Codon("CGT"),ARGININE));
            codonMap_.insert(std::make_pair(Codon("CGA"),ARGININE));
            codonMap_.insert(std::make_pair(Codon("CGC"),ARGININE));
            codonMap_.insert(std::make_pair(Codon("CGG"),ARGININE));
            codonMap_.insert(std::make_pair(Codon("AGT"),SERINE));
            codonMap_.insert(std::make_pair(Codon("AGC"),SERINE));
            codonMap_.insert(std::make_pair(Codon("AGA"),ARGININE));
            codonMap_.insert(std::make_pair(Codon("AGG"),ARGININE));
            codonMap_.insert(std::make_pair(Codon("GGT"),GLYCINE));
            codonMap_.insert(std::make_pair(Codon("GGC"),GLYCINE));
            codonMap_.insert(std::make_pair(Codon("GGA"),GLYCINE));
            codonMap_.insert(std::make_pair(Codon("GGG"),GLYCINE));

            makeInvertedMap();
        }

        inline void CodonAminoTable::makeInvertedMap() {
            for(CodonMap::const_iterator it = codonMap_.begin(); it != codonMap_.end(); ++it) {
                invertedCodonMap_.insert(std::make_pair(it->second, it->first));
            }
        }

        /** \brief the amino for given codon */
        inline AminoAcid CodonAminoTable::getAmino(const Codon& codon) const {
            CodonMap::const_iterator p = codonMap_.find(codon);
            if( p != codonMap_.end() )
                return p->second;
            else
                return UNKNOWN;
        }

        /** \brief codons for given amino */
        inline std::set<Codon> CodonAminoTable::getCodons(AminoAcid amino) const {
            typedef InvertedCodonMap::const_iterator It;

			std::pair<It,It> p = invertedCodonMap_.equal_range(amino);
			std::set<Codon> codons;
            for(It i = p.first; i != p.second; ++i )
                codons.insert(i->second );
            return codons;
        }

        /** \brief equiwalent codons */
        inline std::set<Codon> CodonAminoTable::getCodons(const Codon& codon) const {
            return getCodons( getAmino(codon) );
        }


        /** \brief ostream for amino */
        inline std::ostream& operator<<(std::ostream& os, const AminoAcid& n) {
            os << static_cast<char>(n);
            return os;
        }


    } //namespace dna
} //namespace faif

#endif
