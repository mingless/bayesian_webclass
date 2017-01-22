#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

/** \class Dictionary
 *  \brief Dictionary class for word comparision.
 * A simple class for comparing sets of words against the internal
 * list of words.
 */

class Dictionary {
    public:
        Dictionary(){};
        void write_str_to_file(std::string filename, std::string str);
        void fetch_from_file(std::string filename);
        int compare(const std::string& filename);
        std::vector<std::string> word_list; /**< public internal word list */
};


#endif
