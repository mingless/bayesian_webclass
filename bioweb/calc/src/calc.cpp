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

CommandManager::CommandManager() : scheduler_(16), history_ () {
}

CommandManager& CommandManager::getInstance() {
	static CommandManager instance;
	return instance;
}

mt4cpp::CommandID CommandManager::runTickCommand(int steps ) {
	return mt4cpp::executeAsynchronouslyAndRemember(scheduler_, history_, mt4cpp::PCommand(new mt4cpp::TickCommand(steps) ) );
}

std::vector<mt4cpp::CommandID> CommandManager::commandKeys() const {
	return history_.keys();
}

mt4cpp::CommandDesc CommandManager::findCommandDesc(mt4cpp::CommandID id) const {
	return mt4cpp::findCommandDescriptor(history_, id);
}

void CommandManager::clearHistory() {
	history_.clear();
}
