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

/**
   singleton, active object design pattern
*/
class CALC_DLL( CommandManager ) {
public:
	static CommandManager& getInstance();

	/** execute testing command (this command is finished after 0.2 s) */
	mt4cpp::CommandID runTickCommand( int steeps = 10 );

	/** keys for stored commands */
	std::vector<mt4cpp::CommandID> commandKeys() const;

	/** description for command with given id */
	mt4cpp::CommandDesc findCommandDesc(mt4cpp::CommandID id) const;

	void clearHistory();
private:
	CommandManager();

	mt4cpp::Scheduler scheduler_;
	mt4cpp::CommandHistory history_;

	CommandManager(const CommandManager&) = delete;
	CommandManager operator=(const CommandManager&) = delete;
};


#endif //CALC_HPP
