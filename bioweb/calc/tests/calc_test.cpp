/**
   calc library tests
   defines 'main' for cpp unit test
**/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>


#include "../src/calc.hpp"

using namespace boost;
using boost::unit_test::test_suite;

BOOST_AUTO_TEST_SUITE( calc_test )	//creating test suite that contains few test cases;

BOOST_AUTO_TEST_CASE( TestPing ) {
	
	BOOST_CHECK_EQUAL( ping(), "ping" );
}

BOOST_AUTO_TEST_SUITE_END()