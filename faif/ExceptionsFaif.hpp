#ifndef FAIF_EXCEPTIONS_H_
#define FAIF_EXCEPTIONS_H_

//   the base exception class for faif library

#if defined(_MSC_VER)
//  msvc warning 'std::copy deprecated'
#pragma warning(disable:4996)
#endif

#include <exception>
#include <iostream>
#include <cstring>

namespace faif {

    /** \brief the base exception class for faif library */
    class FaifException : public std::exception {
	public:
		FaifException(){}
		virtual ~FaifException() throw() {}
		virtual const char *what() const throw() { return "FaifException"; }

		/** \brief the exception info written to ostream */
		virtual std::ostream& print(std::ostream& os) const throw() {
			os << what() << std::endl;
			return os;
		}
	};

	/** \brief the exception thrown when the value for given attribute is not found  */
	class NotFoundException : public FaifException {
		static const int SIZE = 30;
	public:
		NotFoundException(const char* domain_id) {
			strncpy(domainID_,domain_id,SIZE);
			domainID_[SIZE-1]='\0';
		}
		virtual ~NotFoundException() throw() {}
		virtual const char *what() const throw(){ return "NotFoundException"; }
		virtual std::ostream& print(std::ostream& os) const throw() {
			os << "Value in domain " << domainID_ << " not found";
			return os;
		}
	private:
		// the std::string is not used
		char domainID_[SIZE];
	};

	//the global function called ex.print(os)
	inline std::ostream& operator<<(std::ostream& os, const FaifException& ex) {
		ex.print(os);
		return os;
	}


} //namespace faif

#endif //FAIF_EXCEPTIONS_H_
