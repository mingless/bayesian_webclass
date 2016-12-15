#include "http_downloader.h"
#include <libxml++/libxml++.h>
#include <cstdlib>
#include <boost/filesystem.hpp>

int main(){
	HTTPDownloader downloader;

    //download html text from html_address
    //std::string content = downloader.download(html_address);
    std::vector<std::string> addresses = downloader.get_urls_from_file("http_addresses.txt");
  	xmlpp::DomParser parser;
    std::string html_text,filename;
    std::string path_root("xml/");
    int count = 1;

    boost::filesystem::path dir("xml");
 
    for (std::string i : addresses)
    {
    	html_text = downloader.download(i); //for every link from file download html code
    	filename = path_root + "kod_html" + std::to_string(count) + ".txt";
    	downloader.write_str_to_file(filename, html_text);
    	html_text = downloader.cleanhtml(html_text); //clean downloaded html code
    	filename = path_root + "kod_html" + std::to_string(count) + "_clean.txt";
    	downloader.write_str_to_file(filename, html_text);
    	parser.parse_memory(html_text);	 //parse html code
    	// xmlpp::Node* rootNode = parser.get_document()->get_root_node();

    	// // Xpath query
    	// xmlpp::NodeSet result = rootNode->find("/html/head/meta/@content");

    	// // Get first node from result
    	// xmlpp::Node *firstNodeInResult = result.at(0);
    	// // Cast to Attribute node (dynamic_cast on reference can throw [fail fast])
    	// xmlpp::Attribute &attribute = dynamic_cast<xmlpp::Attribute&>(*firstNodeInResult);

    	// // Get value of the attribute
    	// Glib::ustring attributeValue = attribute.get_value();
    	count++;
    	// Print attribute value
 		// std::cout << attributeValue << std::endl;
    }

}