#include <bayesian_webclass/http_downloader.h>
#include <libxml++/libxml++.h>
#include <cstdlib>
#include <boost/filesystem.hpp>

int main(){
	HTTPDownloader downloader;

    //download html text from html_address
    std::vector<std::string> addresses = downloader.get_urls_from_file("html/http_addresses.txt");
    std::string html_text,filename;
    std::string path_root("output/");
    int count = 1;

    boost::filesystem::create_directories ("output"); //create a directory for results

    for (std::string i : addresses)
    {

    	html_text = downloader.download(i); //for every link from file download the html code
    	filename = path_root + "kod_html" + std::to_string(count) + ".txt";
    	downloader.write_str_to_file(filename, html_text);
    	html_text = downloader.cleanhtml(html_text); //clean downloaded html code
    	filename = path_root + "kod_html" + std::to_string(count) + "_clean.txt";
    	downloader.write_str_to_file(filename, html_text);
    	std::string output_of_parsing;

    	//false is you want to save output of parsing in string, else if you want to save it in file ,set true, and not set last parameter
    	downloader.parse_html_and_save(html_text, "/html/body", count, false, output_of_parsing);  //parse html_text and save to file in /output directory text from "html/body" node, that means only the text between <body></body>

      	std::cout << output_of_parsing <<std::endl; //outputs to the console result of parsing
      	count++;
    }

}
