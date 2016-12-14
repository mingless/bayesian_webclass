#ifndef HTTP_DOWNLOADEr_H
#define HTTP_DOWNLOADEr_H

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
    void write_html_to_file(std::string filename, std::string url);
    
    std::string download(const std::string& url); //downloads html text from given url

	std::string cleanhtml(const std::string &html); //makes html tidy, brackets are closed, makes
													//sure that html code is ready to be parsed 
	std::string get_url_address_from_console(); //lets user to enter url file in console

	std::vector<std::string> get_urls_from_file(std::string filename); //gets list of html addresses from given file
																		//in every line should be one http_address 
private:
    void* curl;
};


#endif