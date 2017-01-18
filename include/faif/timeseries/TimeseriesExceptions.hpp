#ifndef FAIF_TS_EXCEPTIONS
#define FAIF_TS_EXCEPTIONS

#include "../ExceptionsFaif.hpp"
#include "TimeSeries.hpp"

namespace faif {
    namespace timeseries {

		//! bad prediction range exception
		class PredictionRangeException : public FaifException {
		public:
			//! constructor
			PredictionRangeException(DigitTime f, DigitTime t) : from_(f), to_(t) {}
			//! what
			virtual const char* what() const throw() { return "Prediction Range Exception "; }
			//! print
			virtual std::ostream& print(std::ostream& os) const throw() {
				os << "Prediction range error: from = '" << from_ << "' to = '" << to_ << "'";
				return os;
			}
		private:
			DigitTime from_;
			DigitTime to_;
		};

	} //namespace timeseries
} //namespace faif

#endif //FAIF_TS_EXCEPTIONS
