/**
 * \file calcpy.cpp
 * \brief the Python interface for C++ calculation library
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
//msvc disable warnings for Boost.Python
#pragma warning(disable:4100)
#pragma warning(disable:4127)
#pragma warning(disable:4244)
#pragma warning(disable:4267)
#pragma warning(disable:4512)
#endif


#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include "calc.hpp"

using namespace boost::python;

/** Python intreface to CommandManager
 */
class CommandManagerPy {
public:
    std::vector<long> getIds() {
		return CommandManager::getInstance().commandKeys();
	}

    long startTick() {
		return CommandManager::getInstance().runTickCommand(200); //4 sec here! (in C++ tests 0.2 s. command is used)
	}
    void breakCmd(long) {
        //TODO
    }
    mt4cpp::CommandDesc::State getState(long id) { return CommandManager::getInstance().findCommandDesc(id).state_; }
    double getProgress(long id) { return CommandManager::getInstance().findCommandDesc(id).progress_; }
};

/**
 * Python wrapper using Boost.Python
 */
BOOST_PYTHON_MODULE( calc )
{
    //! exports getNumber to Python
    boost::python::def( "getNumber", getNumber );

    boost::python::enum_<mt4cpp::CommandDesc::State>("CommandState")
        .value("NONE",mt4cpp::CommandDesc::NONE)
        .value("QUEUED",mt4cpp::CommandDesc::QUEUED)
        .value("PENDING",mt4cpp::CommandDesc::PENDING)
        .value("INTERRUPTED",mt4cpp::CommandDesc::INTERRUPTED)
        .value("EXCEPTION",mt4cpp::CommandDesc::EXCEPTION)
        .value("DONE",mt4cpp::CommandDesc::DONE)
        .export_values()
        ;

	class_<std::vector<long> >("CommandKeys")
		.def(vector_indexing_suite<std::vector<long> >() )
		;

    class_<CommandManagerPy>("CommandManager")
        .def( "getIds", &CommandManagerPy::getIds )
        .def( "start", &CommandManagerPy::startTick )
        .def( "break", &CommandManagerPy::breakCmd )
        .def( "getState", &CommandManagerPy::getState )
        .def( "getProgress", &CommandManagerPy::getProgress )
        ;

}
