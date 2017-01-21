#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include "faif/learning/NaiveBayesian.hpp"
#include "faif/learning/Validator.hpp"

typedef faif::ml::NaiveBayesian<faif::ValueNominal<std::string>> NBstr;
typedef faif::ml::NaiveBayesian<faif::ValueNominal<int>> NBint;
typedef NBint::AttrDomain AttrDomain;
typedef NBint::Domains Domains;
typedef NBint::ExampleTest ExampleTest;
typedef NBint::ExamplesTrain ExamplesTrain;

class Classifier {
    public:
        void init(std::string attributes,
                  std::string categories,
                  std::string examples_dir,
                  int examples_num);
        void loadAttributes(std::string attributes);
        void loadCategories(std::string categories);
        void loadExample(std::string example);
        std::string classify(std::string example);

    private:
        AttrDomain _cat;
        Domains _attribs;
        std::map<std::string, int> _attrib_index;
        std::map<std::string, int> _cat_index;
        std::vector<std::string> _cat_list;
        ExamplesTrain _ex;
        NBint* _nb;
};


#endif
