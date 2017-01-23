#include "bayesian_webclass/classifier.h"


/** \brief Method for classifier initialization.
 * Initializes the classifier with given attribute list,
 * categories and trains the classifier on a number of examples
 * in the given directory.
 * @param attributes name of the file listing all attributes
 * @param categories name of the file listing all categories
 * @param examples_dir directory containing examples
 * @param examples_num number of examples to be used (and available
 *        in the examples_dir)
 */

void Classifier::init(std::string attributes,
                      std::string categories,
                      std::string examples_dir,
                      int examples_num) {
    loadAttributes(attributes);
    loadCategories(categories);

    _nb = new NBint(_attribs, _cat);

    std::string example;
    for(int i = 0; i <= examples_num; ++i) {
        example = examples_dir + std::to_string(i) + ".txt";
        loadExample(example);
    }

    _nb->train(_ex);
}

/** \brief Method loading attributes from a given file.
 * Loads attributes from the given file into the internal
 * faif::ml::NaiveBayesian<faif::ValueNominal<int>>::Domains
 * object.
 * @param attributes name of the file listing all attributes
 */

void Classifier::loadAttributes(std::string attributes) {
    std::vector<std::string> word_list;
    std::ifstream input;
    input.open(attributes);
    std::string word;

    while(input >> word) {
        word_list.push_back(word);
    }

    int A[] = {0, 1};   // A generic binary attribute,
                        // using int to allow int category

    int index = 0;
    for(std::string w : word_list) {
        _attribs.push_back(faif::createDomain(w, A, A+2));
        _attrib_index[w] = index++;
    }
}

/** \brief Method loading categories from a given file.
 * Loads categories from the given file into the internal
 * faif::ml::NaiveBayesian<faif::ValueNominal<int>>::AttrDomain
 * object.
 * @param categories name of the file listing all categories
 */

void Classifier::loadCategories(std::string categories) {
    std::ifstream input;
    input.open(categories);
    std::string word;
    int index = 0;

    while(input >> word) {
        _cat_index[word] = index++;
        _cat_list.push_back(word);
    }

    int C[_cat_list.size()];   // An int category mapping strings,
                                // to ints (NB is highly templated
                                // as such using ints seemed to be
                                // a hasle free way to go)

    for(std::size_t i = 0; i < _cat_list.size(); ++i) {
        C[i] = i;
    }

    _cat = faif::createDomain("", C, C+_cat_list.size());
}

/** \brief Method loading an example from a given file.
 * Loads an example from the given file into the internal
 * faif::ml::NaiveBayesian<faif::ValueNominal<int>>::ExamplesTrain
 * object.
 * @param example name of the file with the example
 */

void Classifier::loadExample(std::string example) {
    std::ifstream input;
    input.open(example);
    std::string word;
    int cat;
    if(!(input >> word)) {
        throw;
    }
    cat = _cat_index.at(word);

    int E[_attrib_index.size()]{};
    while(input >> word) {
        if(_attrib_index.find(word) != _attrib_index.end()) {
            E[_attrib_index.at(word)] = 1;
        }
    }
    _ex.push_back(faif::ml::createExample(E, E+_attrib_index.size(), cat, *_nb));
}

/** \brief Method classifying a given test example.
 * Loads the example from the given file into the internal
 * faif::ml::NaiveBayesian<faif::ValueNominal<int>>::ExampleTest
 * object.
 * @param example name of the file with the test example
 */

std::string Classifier::classify(std::string example) {
    std::ifstream input;
    input.open(example);
    std::string word;

    int E[_attrib_index.size()]{};
    while(input >> word) {
        if(_attrib_index.find(word) != _attrib_index.end())
            E[_attrib_index.at(word)] = 1;
    }
    ExampleTest et = createExample(E, E+_attrib_index.size(), *_nb);

    std::stringstream ss;
    ss << _nb->getCategory(et);
    word = ss.str();
    word.erase(0, 1);
    return _cat_list.at(std::stoi(word));
}
