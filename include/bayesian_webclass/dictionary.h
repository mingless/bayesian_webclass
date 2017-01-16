#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

class Dictionary {
    public:
        Dictionary();
        ~Dictionary();
        void write_str_to_file(std::string filename, std::string str);
        void fetch_from_file(std::string filename);  // fetches a list of words from given file with one word per line assumed
        int compare(const std::string& filename);

    private:
        std::vector<std::string> _word_list;
};


#endif
