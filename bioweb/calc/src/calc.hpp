/**
 * \file calc.hpp
 * \brief the C++ calculation library interface
 */

#ifndef CALC_HPP
#define CALC_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
//msvc disable warnings for sheduler_ and history_ member
#pragma warning(disable:4251)
#endif
#ifdef CALC_EXPORTS
/** Workaround for Windows DLL library exports */
#define CALC_DLL(X) __declspec(dllexport)X
#else
/** Workaround for Unix Shared Library exports */
#define CALC_DLL(X) X
#endif

#include <string>
CALC_DLL( std::string ping(); )	//test function???

#include <mt4cpp/Scheduler.hpp>
#include <mt4cpp/CommandHistory.hpp>


#endif //CALC_HPP
