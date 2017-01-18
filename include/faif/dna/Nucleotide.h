// dna nucleotide
//  @author Robert Nowak

#ifndef NUCLEOTIDE_H
#define NUCLEOTIDE_H

#include <ostream>
#include "ExceptionsDna.h"

namespace faif {

    namespace dna {

        /** the nucleotide value (enum) */
        enum NucleotideValue { ADENINE = 'A', CYTOSINE = 'C', GUANINE = 'G' , THYMINE = 'T', ANY_NUCLEOTIDE = 'N'};

        /** \brief the DNA nucleotide

            There are four values: A, T, G, C and the N (denotes any nucleotide)
        */
        class Nucleotide {
        private:
            NucleotideValue val_;
        public:
            /** c-tor */
            explicit Nucleotide(const NucleotideValue& val = ANY_NUCLEOTIDE) : val_(val) {}
            Nucleotide(const Nucleotide& n) : val_(n.val_) {}

            Nucleotide& operator=(const Nucleotide& n) { val_ = n.val_;return *this; }
            ~Nucleotide(){}

            /** accessor */
            NucleotideValue get() const { return val_; }

            /** comparison */
            bool operator==(const Nucleotide& n)const{ return val_ == n.val_; }
            /** comparison */
            bool operator!=(const Nucleotide& n)const{ return val_ != n.val_; }
            /** order */
            bool operator<(const Nucleotide& n) const { return val_ < n.val_; }

            /** return the complementary nucleotide */
            Nucleotide complementary() const {
                switch(val_) {
                case ADENINE:
                    return Nucleotide(THYMINE);
                case CYTOSINE:
                    return Nucleotide(GUANINE);
                case GUANINE:
                    return Nucleotide(CYTOSINE);
                case THYMINE:
                    return Nucleotide(ADENINE);
                default:
                    throw NucleotideBadCharException(static_cast<char>(val_) );
                }
            }
        };

        /** creates from the char representing nucleotide */
        inline Nucleotide create(const char& n) {
            switch(n) {
            case 'A':
            case 'a':
                return Nucleotide(ADENINE);
            case 'C':
            case 'c':
                return Nucleotide(CYTOSINE);
            case 'G':
            case 'g':
                return Nucleotide(GUANINE);
            case 'T':
            case 't':
                return Nucleotide(THYMINE);
            }
            throw NucleotideBadCharException(n);
        }

        /** stream operator, support, for debugging */
        inline std::ostream& operator<<(std::ostream& os, const Nucleotide& n) {
            os << static_cast<char>(n.get());
            return os;
        }

    } //namespace dna
} //namespace faif

#endif //NUCLEOTIDE_H
