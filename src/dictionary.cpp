#include "bayesian_webclass/dictionary.h"

struct letter_only : std::ctype<char> {
    letter_only() : std::ctype<char>(get_table()) {}

    static std::ctype_base::mask const* get_table() {
        static std::vector<std::ctype_base::mask>
            rc(std::ctype<char>::table_size,std::ctype_base::space);

        std::fill(&rc['A'], &rc['z'+1], std::ctype_base::alpha);
        return &rc[0];
    }
};

void Dictionary::write_str_to_file(std::string filename, std::string str) {
    std::ofstream myfile;
    myfile.open(filename);
    myfile << str;
    myfile.close();
}

void Dictionary::fetch_from_file(std::string filename) {
    word_list.clear();
    std::ifstream input;
    input.open(filename);
    std::string word;

    while(input >> word) {
        word_list.push_back(word);
    }
}

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
