#include <iostream>
#include <sstream>
#include <fstream>
#include "bayesian_webclass/http_downloader.h"
#include "curl/curl.h"
#include <tidy/tidy.h>
#include <tidy/buffio.h>
#include <libxml++/libxml++.h>
#include <ostream>
#include <glibmm.h>
#include <algorithm>
#include <set>


/**Constructor
 *
 */
HTTPDownloader::HTTPDownloader() {
    curl = curl_easy_init();
}

/**Destructor
 *
 */
HTTPDownloader::~HTTPDownloader() {
    curl_easy_cleanup(curl);
}

std::size_t write_data(void *ptr, std::size_t size, std::size_t nmemb, void *stream) {
    std::string data((const char *) ptr, (std::size_t) size * nmemb);
    *((std::stringstream *) stream) << data << std::endl;
    return size * nmemb;
}
/**Download html code
* Downloads html text from given website, waits 5 seconds for response
* otherwise returns false
* @param[in] url - url address of website to curl
* @param[out] output - html code from given url will be saved here
* @return true if everything went right, false if not
*/
bool HTTPDownloader::download(const std::string &url,
                              std::string &output) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L); //wait for website response max 5s
    std::stringstream out;
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1); //Prevent "longjmp causes uninitialized stack frame" bug
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "deflate");
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    CURLcode res = curl_easy_perform(curl);
    output += out.str();
    if (res != CURLE_OK) {
        return false;
    } else {
        return true;
    }
}

/**Save set of strings to file. Every string in new line. If file given as argument does not exist
*it will be created
*@param filename - name of file
*@param set - set of strings to save in file
*/
void HTTPDownloader::writeSetToFile(const std::string &filename, const std::set<std::string> &set) {
    std::ofstream myfile;
    myfile.open(filename + ".txt");

    for (auto i : set) {
        myfile << i << std::endl;
    }
    myfile.close();
}

/**Save string to file.
* If file given as argument does not exist
*it will be created
*@param filename - name of file
*@param str - string to save in file
*/
void HTTPDownloader::writeStrToFile(const std::string &filename, const std::string &str) {
    std::ofstream myfile;
    myfile.open(filename);
    myfile << str;
    myfile.close();
}


/**Clean html. That means close open tags and reformat dirty html code.
*Detailed informations cout be found on libtidy library
* <a href="http://tidy.sourceforge.net/">website</a>
*@param html - string with html code to clean
*@return Cleand html code.
*/
std::string HTTPDownloader::cleanhtml(const std::string &html) {
    // init a tidy document
    TidyDoc tidy_doc = tidyCreate();
    TidyBuffer output_buffer = {0};

    // configure tidy
    // the flags tell tidy to output xml and disable warnings
    bool config_success = tidyOptSetBool(tidy_doc, TidyXmlOut, yes)
                          && tidyOptSetBool(tidy_doc, TidyQuiet, yes)
                          && tidyOptSetBool(tidy_doc, TidyNumEntities, yes)
                          && tidyOptSetBool(tidy_doc, TidyShowWarnings, no);

    int tidy_rescode = -1;

    // parse input
    if (config_success)
        tidy_rescode = tidyParseString(tidy_doc, html.c_str());
    if (tidy_rescode >= 0)
        tidy_rescode = tidyCleanAndRepair(tidy_doc);
    if (tidy_rescode >= 0)
        tidy_rescode = tidySaveBuffer(tidy_doc, &output_buffer);
    if (tidy_rescode < 0)
        throw ("Tidy error: " + tidy_rescode);

    std::string result = (char *) output_buffer.bp;
    tidyBufFree(&output_buffer);
    tidyRelease(tidy_doc);

    return result;
}

/**Get lines from file and save them in vector of string
 * @param filename - name of file
 * @return vector of strings
 */
std::vector<std::string> HTTPDownloader::getLinesFromFile(std::string filename) {
    std::vector<std::string> http_address; //stores html addresses
    std::string line;
    std::ifstream http_address_file(filename); //in every line should be other http address

    if (http_address_file.is_open()) {
        while (!http_address_file.eof()) {
            getline(http_address_file, line);
            http_address.push_back(line);
        }
        http_address_file.close();
    } else {
        std::cout << "nie udalo sie otworzyc pliku" << std::endl;
    }

    return http_address;
}

/**Get node of parsed html code and its children
 * Gets unique attribues from html text. These attributes are
 * hrefs from "<a href="">". This function is recursive.
 * @param node of xml/html structure
 * @param textFromBodyFile html code
 * @param indentation
 * @param uniqueAttribs store attributes get from textFromBodyFile
 */
void getNodeAndChildren(const xmlpp::Node *node, std::ostream &textFromBodyFile, unsigned int indentation,
                        std::set<std::string> &uniqueAttribs) {

    bool get_href_from_this_node = false;
    const xmlpp::ContentNode *nodeContent = dynamic_cast<const xmlpp::ContentNode *>(node);
    const xmlpp::TextNode *nodeText = dynamic_cast<const xmlpp::TextNode *>(node);
    const xmlpp::CommentNode *nodeComment = dynamic_cast<const xmlpp::CommentNode *>(node);

    if (nodeText && nodeText->is_white_space()) //Let's ignore the indenting - you don't always want to do this.
        return;

    Glib::ustring nodename = node->get_name();

    if (!nodeText && !nodeComment && !nodename.empty()) //Let's not say "name: text".
    {
        if (node->get_name() == "a") {
            get_href_from_this_node = true;
        }
    }
    //Treat the various node types differently:
    try {
        if (nodeText) {
            textFromBodyFile << nodeText->get_content() << std::endl;
        } else if (const xmlpp::Element *nodeElement = dynamic_cast<const xmlpp::Element *>(node)) {
            const xmlpp::Element::AttributeList &attributes = nodeElement->get_attributes();
            for (xmlpp::Element::AttributeList::const_iterator iter = attributes.begin();
                 iter != attributes.end(); ++iter) {
                const xmlpp::Attribute *attribute = *iter;
                if (get_href_from_this_node && attribute->get_name() == "href") {
                    std::string href(attribute->get_value());
                    std::string s = href.substr(0, 6); //we want /wiki/ here
                    if ((href.find(":") != std::string::npos) ||
                        s != "/wiki/") //if in href is ":" or there is no /wiki/ we don't want it
                    {
                    } else {
                        std::string dont_want_this("/wiki/");
                        std::string::size_type i = href.find(dont_want_this);

                        if (i != std::string::npos)
                            href.erase(i, dont_want_this.length());
                        uniqueAttribs.insert(href);
                    }
                    get_href_from_this_node = false;
                }
            }
        }
    }
    catch (Glib::ConvertError e) {
        //std::cout << "Error ocured, but whatever" << std::endl;
    }
    if (!nodeContent) {
        //Recurse through child nodes:
        xmlpp::Node::NodeList list = node->get_children();
        for (xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter) {
            getNodeAndChildren(*iter, textFromBodyFile, indentation + 2, uniqueAttribs); //recursive
        }
    }
}

/** Parse html code
 * Save output in string
 * @param node node of html structure
 * @return output of parsing
 */
std::string printNodeAndChildren(const xmlpp::Node *node) {
    //recursive function that prints to given ofstream (exp file) the text of give node from parsed xml file
    const xmlpp::ContentNode *nodeContent = dynamic_cast<const xmlpp::ContentNode *>(node);
    const xmlpp::TextNode *nodeText = dynamic_cast<const xmlpp::TextNode *>(node);

    std::string output;

    if (nodeText && nodeText->is_white_space()) //Let's ignore the indenting - you don't always want to do this.
        return "";
    if (nodeText) {
        output += nodeText->get_content();
        output += '\n';
    }
    if (!nodeContent) {
        //Recurse through child nodes:
        xmlpp::Node::NodeList list = node->get_children();
        for (xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter) {
            output += printNodeAndChildren(*iter); //recursive
        }
    }
    return output;
}

/**Parse html code
 *
 * @param node  node of html structure
 * @param textFromBodyFile output of parsing
 */
void printNodeAndChildren(const xmlpp::Node *node, std::ofstream &textFromBodyFile) {

    //recursive function that prints to given ofstream (exp file) the text of give node from parsed xml file
    const xmlpp::ContentNode *nodeContent = dynamic_cast<const xmlpp::ContentNode *>(node);
    const xmlpp::TextNode *nodeText = dynamic_cast<const xmlpp::TextNode *>(node);

    if (nodeText && nodeText->is_white_space()) //Let's ignore the indenting - you don't always want to do this.
        return;

    if (nodeText) {
        textFromBodyFile << nodeText->get_content() << std::endl;
    }
    if (!nodeContent) {
        //Recurse through child nodes:
        xmlpp::Node::NodeList list = node->get_children();
        for (xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter) {
            printNodeAndChildren(*iter, textFromBodyFile); //recursive
        }
    }
}
/** Parse html code and get attributes
 *parse html code and get unique attributes as a set.
 * @param[in] htmlText text of html code
 * @param[in] nodeOfHtmlTree xpath from which get the content and attributes
 * @param[out] output content of node given in nde_of_html_tree
 * @param[out] uniqueAttributes set of unique attributes
 * @return number of unique attributes
 */
int HTTPDownloader::parseHtmlAndSave(const std::string &htmlText, const std::string &nodeOfHtmlTree,
                                     std::string &output, std::set<std::string> &uniqueAttributes) {
    //parse htmlText and save to file only the text from given nodeOfHtmlTree
    xmlpp::DomParser parser;

    parser.parse_memory(htmlText);  //parse html code from string

    if (parser) {
        //Walk the tree
        const xmlpp::Node *pNode = parser.get_document()->get_root_node();
        xmlpp::NodeSet result = pNode->find(nodeOfHtmlTree);         //find node given in function parameter
        std::stringstream ss;
        for (auto i : result) //for every result print text from node to file
        {
            getNodeAndChildren(i, ss, 0, uniqueAttributes);
        }
        for (auto i : uniqueAttributes) {
            output += i;
            output += '\n';
        }

        std::cout << "Ilosc atrybutow:" << uniqueAttributes.size() << std::endl;
    }
    return uniqueAttributes.size();
}
void print_indentation(unsigned int indentation) {
    for (unsigned int i = 0; i < indentation; ++i)
        std::cout << " ";
}










