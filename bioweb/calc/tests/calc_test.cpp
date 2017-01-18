/**
   calc library tests
   defines 'main' for cpp unit test
**/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>


#include "../src/calc.hpp"
/*
using namespace boost;
using boost::unit_test::test_suite;

BOOST_AUTO_TEST_SUITE( calc_test )

BOOST_AUTO_TEST_CASE( TestGetNumber )
{
	BOOST_CHECK_EQUAL( getNumber(), 1234 );
}

BOOST_AUTO_TEST_CASE( CommandMgrTest )
{
	mt4cpp::CommandID id1 = CommandManager::getInstance().runTickCommand();
	mt4cpp::CommandID id2 = CommandManager::getInstance().runTickCommand();
	mt4cpp::CommandID id3 = CommandManager::getInstance().runTickCommand();

	std::vector<mt4cpp::CommandID> keys = CommandManager::getInstance().commandKeys();

	BOOST_CHECK_EQUAL( keys.size(), 3U );
	BOOST_CHECK_EQUAL( keys[0], id1 );
	BOOST_CHECK_EQUAL( keys[1], id2 );
	BOOST_CHECK_EQUAL( keys[2], id3 );

    BOOST_CHECK( CommandManager::getInstance().findCommandDesc(id1).state_ != mt4cpp::CommandDesc::DONE );
    BOOST_CHECK( CommandManager::getInstance().findCommandDesc(id2).state_ != mt4cpp::CommandDesc::DONE );
    BOOST_CHECK( CommandManager::getInstance().findCommandDesc(id3).state_ != mt4cpp::CommandDesc::DONE );

    boost::this_thread::sleep(boost::posix_time::millisec(500)); //po 0.5 sec need to be finished (command executes 0.2s)

    BOOST_CHECK( CommandManager::getInstance().findCommandDesc(id1).state_ == mt4cpp::CommandDesc::DONE );
    BOOST_CHECK( CommandManager::getInstance().findCommandDesc(id2).state_ == mt4cpp::CommandDesc::DONE );
    BOOST_CHECK( CommandManager::getInstance().findCommandDesc(id3).state_ == mt4cpp::CommandDesc::DONE );

	BOOST_CHECK_EQUAL( CommandManager::getInstance().commandKeys().size(), 3U );
	CommandManager::getInstance().clearHistory();
	BOOST_CHECK_EQUAL( CommandManager::getInstance().commandKeys().size(), 0U );
}

BOOST_AUTO_TEST_SUITE_END()*/
