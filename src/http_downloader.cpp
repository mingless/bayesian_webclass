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

HTTPDownloader::HTTPDownloader()
{
    curl = curl_easy_init();
}

HTTPDownloader::~HTTPDownloader()
{
    curl_easy_cleanup(curl);
}


std::size_t write_data(void *ptr, std::size_t size, std::size_t nmemb, void *stream)
{
    std::string data((const char *) ptr, (std::size_t) size * nmemb);
    *((std::stringstream *) stream) << data << std::endl;
    return size * nmemb;
}


bool HTTPDownloader::download(const std::string &url,
                              std::string &output) //TODO prawdopodobnie będzie jednak jeszcze potrzebna wersja nie uzywajaca flagi i po prostu zwracajaca stringa
{
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
    if (res != CURLE_OK)
    {
        return false;
    } else
    {
        return true;
    }
}

void HTTPDownloader::write_set_to_file(const std::string& filename,const std::set<std::string>& set)
{
    std::ofstream myfile;
    myfile.open(filename);
    for(auto i : set)
    {
        myfile << i <<std::endl;
    }
    myfile.close();
}

void HTTPDownloader::write_str_to_file(const std::string& filename,const std::string& str)
{
    std::ofstream myfile;
    myfile.open(filename);
    myfile << str;
    myfile.close();
}

std::string HTTPDownloader::cleanhtml(const std::string &html)
{
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
////        tidy_rescode = tidySetErrorBuffer( tidy_doc, &errbuf );      // Capture diagnostics
//    if(tidy_rescode >=0)
        tidy_rescode = tidyParseString(tidy_doc, html.c_str());
    if (tidy_rescode >= 0)
        tidy_rescode = tidyCleanAndRepair(tidy_doc);
//    if (tidy_rescode >=0)
//        tidy_rescode = tidyRunDiagnostics(tidy_doc);
//    if ( tidy_rescode > 1 )                                    // If error, force output.
//        tidy_rescode = ( tidyOptSetBool(tidy_doc, TidyForceOutput, yes) ? tidy_rescode : -1 );
    // process html
    if (tidy_rescode >= 0)
        tidy_rescode = tidySaveBuffer(tidy_doc, &output_buffer);

    if (tidy_rescode < 0)
        throw ("tidy has a error: " + tidy_rescode);
//    if ( tidy_rescode >= 0 )
//    {
//        if ( tidy_rescode > 0 )
//            printf( "\nDiagnostics:\n\n%s", errbuf.bp );
//        printf( "\nAnd here is the result:\n\n%s", output_buffer.bp );
//    }

    std::string result = (char *) output_buffer.bp;
    tidyBufFree(&output_buffer);
    tidyRelease(tidy_doc);

    return result;
}
//
//std::string HTTPDownloader::get_url_address_from_console()
//{
//    std::cout << "Enter address of html to download:" << std::endl;
//    std::string address;
//    std::cin >> address;
//    return address;
//}

std::vector<std::string> HTTPDownloader::get_urls_from_file(std::string filename)
{
    std::vector<std::string> http_address; //stores html addresses
    std::string line;
    std::ifstream http_address_file(filename); //in every line should be other http address

    if (http_address_file.is_open())
    {
        while (!http_address_file.eof())
        {
            getline(http_address_file, line);
            http_address.push_back(line);
        }
        http_address_file.close();
    } else
    {
        std::cout << "nie udalo sie otworzyc pliku" << std::endl;
    }

    return http_address;
}

void print_node_and_children(const xmlpp::Node *node, std::ofstream &text_from_body_file);

void print_node_and_children(const xmlpp::Node *node, std::ostream &text_from_body_file, unsigned int indentation,
                             std::set<std::string> &unique_attribs);

std::string print_node_and_children(const xmlpp::Node *node);

void HTTPDownloader::parse_html_and_save(const std::string &html_text, const std::string &node_of_html_tree,
                                         std::string &output,std::set<std::string>& unique_attributes)
{
    //parse html_text and save to file only the text from given node_of_html_tree
    //generated file is output/raw_'count'
    xmlpp::DomParser parser;
    //std::ofstream body_text_file;
    // std::string path_root("output/"), filename;
    parser.parse_memory(html_text);  //parse html code from string //TODO tu jest blad z kodowaniem
    //xmlpp::Node* rootNode = parser.get_document()->get_root_node();

    if (parser)
    {

        //Walk the tree
        const xmlpp::Node *pNode = parser.get_document()->get_root_node();
        xmlpp::NodeSet result = pNode->find(node_of_html_tree);         //find node given in function parameter
        std::stringstream ss;
//        if (file_or_string) //jeżeli chcemy azpisywać wynik w pliku
//        {
//            filename = path_root + "raw_" + std::to_string(count) + ".txt";
//            body_text_file.open(filename);
//        }
//        std::cout << "ilosc nodow: " << result.size() << std::endl;
        for (auto i : result) //for every result print text from node to file
        {
//            if (file_or_string)
//            {
//                print_node_and_children(i, body_text_file);
//            } else
//            {
            //output += print_node_and_children(i);
            print_node_and_children(i, ss, 0, unique_attributes);
//            }

        }
        output += ss.str();
        output += "\n\n****************************************************\n\n";
        for (auto i : unique_attributes)
        {
            output += i;
            output += '\n';
        }
        std::cout << "Ilosc atrybutow:" << unique_attributes.size() << std::endl;
//        if (file_or_string)
//        {
//            std::cout << "Raw text from site " << count << " is in " << filename << std::endl;
//            body_text_file.close();
//        }
    }
}


void print_indentation(unsigned int indentation)
{
    for (unsigned int i = 0; i < indentation; ++i)
        std::cout << " ";
}

void print_node_and_children(const xmlpp::Node *node, std::ostream &text_from_body_file, unsigned int indentation,
                             std::set<std::string> &unique_attribs)
{
    //std::cout << std::endl; //Separate nodes by an empty line.
    bool get_href_from_this_node = false;
    const xmlpp::ContentNode *nodeContent = dynamic_cast<const xmlpp::ContentNode *>(node);
    const xmlpp::TextNode *nodeText = dynamic_cast<const xmlpp::TextNode *>(node);
    const xmlpp::CommentNode *nodeComment = dynamic_cast<const xmlpp::CommentNode *>(node);

    if (nodeText && nodeText->is_white_space()) //Let's ignore the indenting - you don't always want to do this.
        return;

    Glib::ustring nodename = node->get_name();


    if (!nodeText && !nodeComment && !nodename.empty()) //Let's not say "name: text".
    {
        // print_indentation(indentation);
        if (node->get_name() == "a")
        {
            get_href_from_this_node = true;
        }
        // std::cout << "Node name = " << node->get_name() << std::endl;
//        std::cout << "Node name = " << nodename << std::endl;
    } else if (nodeText) //Let's say when it's text. - e.g. let's say what that white space is.
        if (nodeText)
        {

            //   print_indentation(indentation);
            // std::cout << "Text Node" << std::endl;
        }

    //Treat the various node types differently:
    try
    {
        if (nodeText)
        {
            //   print_indentation(indentation);
            text_from_body_file << nodeText->get_content() <<std::endl;
            //  std::cout << "text = \"" << nodeText->get_content() << "\"" << std::endl;
            // text_from_body_file << nodeText->get_content() << std::endl;

        } else if (nodeComment)
        {
            //  print_indentation(indentation);
            // std::cout << "comment = " << nodeComment->get_content() << std::endl;
        } else if (nodeContent)
        {
            // print_indentation(indentation);
            // std::cout << "content = " << nodeContent->get_content() << std::endl;
        } else if (const xmlpp::Element *nodeElement = dynamic_cast<const xmlpp::Element *>(node))
        {
            //A normal Element node:

            //line() works only for ElementNodes.
            //  print_indentation(indentation);
            //std::cout << "     line = " << node->get_line() << std::endl;

            //Print attributes:
            const xmlpp::Element::AttributeList &attributes = nodeElement->get_attributes();
            for (xmlpp::Element::AttributeList::const_iterator iter = attributes.begin();
                 iter != attributes.end(); ++iter)
            {
                const xmlpp::Attribute *attribute = *iter;
                if (get_href_from_this_node && attribute->get_name() == "href")
                {
                    std::string href(attribute->get_value());
                    std::string s = href.substr(0, 6); //we want /wiki/ here
                    //std::cout << "CZY TO JEST /wiki/? =>>" <<s<<std::endl;
                    if ((href.find(":") != std::string::npos) || /*(href.find("/wiki/") ==
                                                               std::string::npos*/s !=
                                                                                  "/wiki/") //if in href is ":" or there is no /wiki/ we don't want it
                    {
                        // std::cout << "TEGO NIE POTRZEBUJEMY" << href<< std::endl;
                    } else
                    {
                        //href.erase(std::remove(href.begin(), href.end(), '/wiki/'), href.end());
                        std::string dont_want_this("/wiki/");
                        std::string::size_type i = href.find(dont_want_this);

                        if (i != std::string::npos)
                            href.erase(i, dont_want_this.length());

                        //std::cout << "TEGO POTRZEBUJEMY; " << href << std::endl;

                        unique_attribs.insert(href);
                        //std::cout << "TEGO POTRZEBUJEMY; " << href << std::endl;
                    }
                    get_href_from_this_node = false;
                }
                //  print_indentation(indentation);
                //  std::cout << "  Attribute " << attribute->get_name() << " = " << attribute->get_value() << std::endl;
            }

            const xmlpp::Attribute *attribute = nodeElement->get_attribute("title");
//            if (attribute)
//            {
//                //std::cout << "title found: =" << attribute->get_value() << std::endl;
//
//            }
        }
    }
    catch (Glib::ConvertError e)
    {
        //std::cout << "Error ocured, but whatever" << std::endl;
    }

    if (!nodeContent)
    {
        //Recurse through child nodes:
        xmlpp::Node::NodeList list = node->get_children();
        for (xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
        {
            print_node_and_children(*iter, text_from_body_file, indentation + 2, unique_attribs); //recursive
        }
    }
}


std::string print_node_and_children(const xmlpp::Node *node)
{
    //recursive function that prints to given ofstream (exp file) the text of give node from parsed xml file
    const xmlpp::ContentNode *nodeContent = dynamic_cast<const xmlpp::ContentNode *>(node);
    const xmlpp::TextNode *nodeText = dynamic_cast<const xmlpp::TextNode *>(node);

    std::string output;

    if (nodeText && nodeText->is_white_space()) //Let's ignore the indenting - you don't always want to do this.
        return "";


    if (nodeText)
    {
        output += nodeText->get_content();
        output += '\n';
    }


    if (!nodeContent)
    {
        //Recurse through child nodes:
        xmlpp::Node::NodeList list = node->get_children();
        for (xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
        {
            output += print_node_and_children(*iter); //recursive
        }
    }
    return output;
}

void print_node_and_children(const xmlpp::Node *node, std::ofstream &text_from_body_file)
{

    //recursive function that prints to given ofstream (exp file) the text of give node from parsed xml file
    const xmlpp::ContentNode *nodeContent = dynamic_cast<const xmlpp::ContentNode *>(node);
    const xmlpp::TextNode *nodeText = dynamic_cast<const xmlpp::TextNode *>(node);


    if (nodeText && nodeText->is_white_space()) //Let's ignore the indenting - you don't always want to do this.
        return;


    if (nodeText)
    {
        text_from_body_file << nodeText->get_content() << std::endl;
    }


    if (!nodeContent)
    {
        //Recurse through child nodes:
        xmlpp::Node::NodeList list = node->get_children();
        for (xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
        {
            print_node_and_children(*iter, text_from_body_file); //recursive
        }
    }
}










