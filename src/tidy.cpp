#include <iostream>
#include <string>
#include <tidy/tidy.h>
#include <tidy/buffio.h>
#include <fstream>

std::string cleanhtml(const std::string &html)
{
    // init a tidy document
    TidyDoc tidy_doc=tidyCreate();
    TidyBuffer output_buffer= {0};

    // configure tidy
    // the flags tell tidy to output xml and disable warnings
    bool config_success=tidyOptSetBool(tidy_doc,TidyXmlOut,yes)
                        && tidyOptSetBool(tidy_doc,TidyQuiet,yes)
                        && tidyOptSetBool(tidy_doc,TidyNumEntities,yes)
                        && tidyOptSetBool(tidy_doc,TidyShowWarnings,no);

    int tidy_rescode=-1;

    // parse input
    if(config_success)
        tidy_rescode=tidyParseString(tidy_doc,html.c_str());

    // process html
    if(tidy_rescode>=0)
        tidy_rescode=tidySaveBuffer(tidy_doc,&output_buffer);

    if(tidy_rescode<0)
        throw("tidy has a error: " + tidy_rescode); // TODO(bartek): int throw deprecated in c++11, string+int shouldn't be here either

    std::string result=(char *)output_buffer.bp;
    tidyBufFree(&output_buffer);
    tidyRelease(tidy_doc);

    return result;
}

int main(){
    std::ifstream ifs ("kod_html_do_tidy.xml");
    std::string html((std::istreambuf_iterator<char> (ifs)), (std::istreambuf_iterator<char>()));
    std::string fixed_html = cleanhtml(html);
 	std::ofstream myfile;
    myfile.open("kod_html1.xml");
  	myfile << fixed_html;
    std::cout << std::endl << "File fixed with tidy" << std::endl;
  	myfile.close();
}
