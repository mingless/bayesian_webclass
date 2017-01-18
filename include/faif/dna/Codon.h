// codon - nucleotide triplet, coding single amino acid
//  @author Robert Nowak

#ifndef CODON_H
#define CODON_H

#include<boost/tuple/tuple.hpp>
#include<boost/tuple/tuple_comparison.hpp>

#include "Nucleotide.h"
#include "ExceptionsDna.h"

namespace faif {
    namespace dna {

        /** \brief the triplet of nucleotides, codon.

            The triplet of nucleotides representing codon (each codon corresponds to given amino
        */
        class Codon : public boost::tuple<Nucleotide, Nucleotide, Nucleotide> {
        public:
            /** c-tor */
			Codon(const Nucleotide& n1v = Nucleotide(),
				  const Nucleotide& n2v = Nucleotide(),
				  const Nucleotide& n3v = Nucleotide() )
				: boost::tuple<Nucleotide,Nucleotide,Nucleotide>(n1v, n2v, n3v)
            {}

            /** c-tor from string */
            Codon(const std::string& str) {
                if (str.size() < 3)
                    throw CodonStringTooShortException();
                get<0>() = create(str[0]);
                get<1>() = create(str[1]);
                get<2>() = create(str[2]);
            }

            /** \brief accessor */
            const Nucleotide& getFirst() const { return get<0>(); }

            /** \brief accessor */
            const Nucleotide& getSecond() const { return get<1>(); }

            /** \brief accessor */
            const Nucleotide& getThird() const { return get<2>(); }
        };

        /** ostream operator for codon */
        inline std::ostream& operator<<(std::ostream& os, const Codon& codon) {
			os << "(" << codon.getFirst() << "," << codon.getSecond() << "," << codon.getThird() << ")";
			return os;
		}
    } //namespace dna
} //namespace faif

#endif
