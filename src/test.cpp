#include <bayesian_webclass/data_preprocessor.h>
#include <iostream>

int main() {
    DataPreprocessor dataPreprocessor;
    std::vector<std::string> categories = dataPreprocessor.ptr_http->get_lines_from_file("categories/list_of_categories.txt");
   // dataPreprocessor.choose_train_data("categories/Gravitational_lensing");

    for (auto i : categories)
    {
        std::cout << i <<std::endl;
        //dataPreprocessor.choose_train_data("categories/"+i);
        dataPreprocessor.get_attribs("train_data/" +i);
    }
    dataPreprocessor.ptr_http->write_set_to_file("all_atributes", dataPreprocessor.getAll_atribs());

//    dataPreprocessor.parse_htmls("kategorie.txt", "/html/body/div[@id='content']/div[@id='bodyContent']/div[@id='mw-content-text']");
//    dataPreprocessor.parse_htmls("pages.txt", "/html/body/div[@id='content']/div[@id='bodyContent']/div[@id='mw-content-text']/div[@class='mw-category-generated']/div[@id='mw-pages']");

    return 0;
}