/**
 * \file calc.cpp
 * \brief the C++ file with example calculation library
 */
#include "calc.hpp"

#include <mt4cpp/TickCommand.hpp>
#include <string>

//test function implementation
CALC_DLL( std::string ping() ) {
	return "ping";
}
