// exceptions for dna module
//  @author Robert Nowak


#ifndef EXCEPTIONS_DNA_H
#define EXCEPTIONS_DNA_H

#include "../ExceptionsFaif.hpp"

namespace faif {

	/** \brief primitives for bioinformatics
	 */
    namespace dna {

        /** \brief the exception thrown when unknown nucleotide (bad letter) occures */
        class NucleotideBadCharException : public FaifException {
        public:
			NucleotideBadCharException(char c) : c_(c) {}
			virtual ~NucleotideBadCharException() throw() {}
			virtual const char *what() const throw() { return "NucleotideBadCharException"; }
			/** prints detailed information into given output stream */
			virtual std::ostream& print(std::ostream& os) const throw(){
				os << "Char '" << c_ << "' is not nucleotide name.";
				return os;
			}
		private:
			char c_;
        };

        /** \brief the exception when chain representing codon is shorted than 3 nucleotides */
        class CodonStringTooShortException : public FaifException {
        public:
			CodonStringTooShortException() {}
			virtual ~CodonStringTooShortException() throw() {}
			virtual const char *what() const throw() { return "CodonStringTooShortException"; }
			/** prints detailed information into given output stream */
			virtual std::ostream& print(std::ostream& os) const throw(){
				os << "String for codon is too short";
				return os;
			}
        };



    } //namespace dna


} //namespace faif

#endif //EXCEPTIONS_DNA_H
