#include "bayesian_webclass/classifier.h"


void Classifier::addAttribs(
        std::vector<std::string> val,
        const std::string& name) {
    _attribs.push_back(faif::createDomain(name, val.begin(), val.end());
}

void Classifier::addExample(
        std::vector<std::string> val,
        const std::string& lbl) {
    _ex.push_back(_nb.createExample(val.begin(), val.end(), lbl);
}

void Classifier::classify() {

}

void Classifier::train(std::string filename) {
    std::map<std::string, int> wordCnt;
    std::ifstream input;
    input.open(filename);
    std::string word;

    while(input >> word) {
        ++wordCnt[word];
    }
}
