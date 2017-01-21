#include <bayesian_webclass/data_preprocessor.h>
#include <bayesian_webclass/classifier.h>
#include <iostream>

int main() {
    /*
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
    */

    Classifier c;
    c.init("/home/m/catkin_ws/src/bayesian_webclass/txt/all_atributes.txt.txt",
           "/home/m/catkin_ws/src/bayesian_webclass/txt/categories/list_of_categories.txt",
           "/home/m/catkin_ws/src/bayesian_webclass/txt/output/",
           224);
    std::cout<<"TEST"<<std::endl;
    std::cout<<c.classify("/home/m/catkin_ws/src/bayesian_webclass/txt/output/34.txt")<<std::endl;
    std::cout<<c.classify("/home/m/catkin_ws/src/bayesian_webclass/txt/output/1.txt")<<std::endl;
    std::cout<<c.classify("/home/m/catkin_ws/src/bayesian_webclass/txt/output/2.txt")<<std::endl;
    std::cout<<c.classify("/home/m/catkin_ws/src/bayesian_webclass/txt/output/4.txt")<<std::endl;
    std::cout<<c.classify("/home/m/catkin_ws/src/bayesian_webclass/txt/output/104.txt")<<std::endl;
    std::cout<<c.classify("/home/m/catkin_ws/src/bayesian_webclass/txt/output/54.txt")<<std::endl;
    std::cout<<c.classify("/home/m/catkin_ws/src/bayesian_webclass/txt/output/84.txt")<<std::endl;
    std::cout<<c.classify("/home/m/catkin_ws/src/bayesian_webclass/txt/output/74.txt")<<std::endl;


    return 0;
}
