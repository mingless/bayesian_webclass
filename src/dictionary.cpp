#include <bayesian_webclass/dictionary.h>


void Dictionary::write_str_to_file(std::string filename, std::string str) {
    std::ofstream myfile;
    myfile.open(filename);
    myfile << str;
    myfile.close();
}

std::vector<std::string> Dictionary::fetch_from_file(std::string filename) {
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
