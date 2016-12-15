#ifndef HTTP_DOWNLOADER_H
#define HTTP_DOWNLOADER_H

#include <iomanip>
#include <iostream>
#include <curl/curl.h>
#include <sstream>
#include <curl/easy.h>
#include <curl/curlbuild.h>
#include <libxml++/libxml++.h>
#include <fstream>
#include <string>
#include <tidy/tidy.h>
#include <tidy/buffio.h>
#include <vector>

class HTTPDownloader {
public:
    HTTPDownloader();
    ~HTTPDownloader();
    void write_str_to_file(std::string filename, std::string str);
    
    std::string download(const std::string& url); //downloads html text from given url

	std::string cleanhtml(const std::string &html); //makes html tidy, brackets are closed and  that html code is ready to be parsed 
	std::string get_url_address_from_console(); //lets user to enter url file in console

	std::vector<std::string> get_urls_from_file(std::string filename); //gets list of html addresses from given file in every line should be one http_address 
	bool parse_html_and_save(const std::string& html_text, const std::string& node_of_html_tree, int count, bool file_or_string, std::string& output); //parse html_text and save to file only the text from given node_of_html_tree
	

private:
    void* curl;
};


#endif