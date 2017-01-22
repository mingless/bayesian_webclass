#include <bayesian_webclass/data_preprocessor.h>
#include <bayesian_webclass/classifier.h>
#include <iostream>
#include <string>
int main(int argc, char* argv[]) {  //standalone bayesian classifier; used in calcpy.cpp

    DataPreprocessor dataPrep;
    
    std::string link(argv[1]);

    dataPrep.get_attribs_from_link(link);

    Classifier c;
    c.init("/home/apiotro/zpr/catkin_ws/src/bayesian_webclass/txt/all_atributes.txt.txt",
           "/home/apiotro/zpr/catkin_ws/src/bayesian_webclass/txt/categories/list_of_categories.txt",
           "/home/apiotro/zpr/catkin_ws/src/bayesian_webclass/txt/output/",
           224);
    std::cout<<c.classify("example/attribs.txt")<<std::endl;

    return 0;
}
