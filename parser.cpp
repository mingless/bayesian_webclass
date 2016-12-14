#include <iostream>
#include <libxml++/libxml++.h>
#include <string>
#include <tidy/tidy.h>
#include <tidy/buffio.h>



using namespace std;
using namespace Glib;
using namespace xmlpp;


 
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
        throw("tidy has a error: "+tidy_rescode);
 
    std::string result=(char *)output_buffer.bp;
    tidyBufFree(&output_buffer);
    tidyRelease(tidy_doc);
 
    return result;
}


int main(int argc, char* argv[])
{
    // Parse the file
    DomParser parser;
    parser.parse_file("kod_html.xml");
    Node* rootNode = parser.get_document()->get_root_node();

    // Xpath query
    NodeSet result = rootNode->find("/html/head/meta/@content");

    // Get first node from result
    Node *firstNodeInResult = result.at(0);
    // Cast to Attribute node (dynamic_cast on reference can throw [fail fast])
    Attribute &attribute = dynamic_cast<Attribute&>(*firstNodeInResult);

    // Get value of the attribute
    ustring attributeValue = attribute.get_value();

    // Print attribute value
    cout << attributeValue << endl;
}