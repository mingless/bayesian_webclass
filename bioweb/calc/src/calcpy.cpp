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
#include <string>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <array>

/** Python intreface to CommandManager
 */

std::string exec(const char* cmd) { //system("") wrapper to execute command and get the output
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != NULL)
            result += buffer.data();
    }
    return std::string("") + static_cast<std::string>(result);
}

std::string classify(const std::string word) 
{   

    std::string response;   //too complicated to export whole classifier as a python module.......
    std::string query =  std::string("/home/apiotro/zpr/catkin_ws/install/lib/bayesian_webclass/test_p ") + "\"" + std::string(word) + "\""; 
    response = exec(query.c_str());         
    std::cout << response;
    return word + " " + response;  
}

/**
 * Python wrapper using Boost.Python
 */
BOOST_PYTHON_MODULE( calc )
{
    using namespace boost::python;
    def("classify", classify);
}

