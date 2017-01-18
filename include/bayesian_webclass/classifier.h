#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include "faif/learning/NaiveBayesian.hpp"
#include "faif/learning/Validator.hpp"

typedef faif::ml::NaiveBayesian<faif::ValueNominal<std::string>> NB;
typedef NB::AttrDomain AttrDomain;
typedef NB::Domains Domains;
typedef NB::ExampleTest ExampleTest;
typedef NB::ExamplesTrain ExamplesTrain;

class Classifier {
    public:
        Classifier(){};
        void addAttribs(std::vector<std::string> val, const std::string& name);
        void addExample(std::vector<std::string> val, const std::string& lbl);
        void classify();
        void train(std::string filename);

    private:
        Domains _attribs;
        ExamplesTrain _ex;
};


#endif
