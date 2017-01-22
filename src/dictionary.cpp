#include "bayesian_webclass/dictionary.h"

/** \struct Letter only struct for locale initialization.
 * Provided struct constructs std::ctype::ctype by supplying
 * the ctype<char> constructor with ctype::mask. Returned
 * std::ctype<char> is a locale::facet which can be used to filter
 * valid english alphanumeric characters by using:
 * std::ifstream.imbue(std::locale(std::locale(), new letter_only()));
 */

struct letter_only : std::ctype<char> {
    letter_only() : std::ctype<char>(get_table()) {}

    static std::ctype_base::mask const* get_table() {
        static std::vector<std::ctype_base::mask>
            rc(std::ctype<char>::table_size,std::ctype_base::space);

        std::fill(&rc['A'], &rc['z'+1], std::ctype_base::alpha);
        return &rc[0];
    }
};

/** Write string to file method.
 * Mostly intended for debugging, appends given string to a file
 * with supplied filename.
 * @param filename name of the file to write to
 * @param str string to be written
 */

void Dictionary::write_str_to_file(std::string filename, std::string str) {
    std::ofstream myfile;
    myfile.open(filename);
    myfile << str;
    myfile.close();
}

/** Fetch (dictionary word list) from file method.
 * Method used to fetch the list of words used by the dictionary
 * for comparisions from a file given with filename.
 * @param filename name of the file to read the list from
 */

void Dictionary::fetch_from_file(std::string filename) {
    word_list.clear();
    std::ifstream input;
    input.open(filename);
    std::string word;

    while(input >> word) {
        word_list.push_back(word);
    }
}

/** Compare words in given file with the current word_list.
 * Method intended for comparing words in the given file with
 * the current word_list.
 * @param filename name of the file to be compared with the word_list
 * @return integer, number of unique words in file that match the word_list
 */

int Dictionary::compare(const std::string& filename) {
    std::map<std::string, int> wordCnt;
    std::ifstream input;
    input.open(filename);
    std::string word;

    while(input >> word) {
        ++wordCnt[word];
    }

    int matchedCnt = 0;
    for(auto w : word_list) {
        if(wordCnt.count(w))
            ++matchedCnt;
    }

    return matchedCnt;
}
