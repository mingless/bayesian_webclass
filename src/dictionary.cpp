#include <bayesian_webclass/dictionary.h>

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
    std::vector<std::string> word_list; //stores addresses of html
    std::string line;
    std::ifstream word_list_file (filename); //in every line should be other http address
    if (word_list_file.is_open()) {
        while (! word_list_file.eof()) {
            std::getline(word_list_file,line,"="); // delimitation with "=", line format is "word=freq"
            // only word is needed here
            word_list.push_back(line);
        }
        word_list_file.close();
    } else {
        std::cout << "Failed to open the file." << std::endl;
    }
    this->_word_list = word_list;
}

int Dictionary::compare(const std::string& filename) {
    std::map<std::string, int> wordCnt;
    std::ifstream input;
    input.imbue(std::locale(std::locale(), new letter_only()));
    input.open(filename);
    std::string word;

    while(input >> word) {
        ++wordCnt[word];
    }

    int matchedCnt;
    for(auto w : _word_list) {
        matchedCnt += wordCnt[w]
    }
    return matchedCnt;
}
