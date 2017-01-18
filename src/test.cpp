#include <bayesian_webclass/data_preprocessor.h>

int main(){
//	HTTPDownloader downloader;
//
//    //get html addresses from textfile to vector of string
//    std::vector<std::string> addresses = downloader.get_urls_from_file("html/http_addresses.txt");
//    std::string html_text,filename;
//    std::string path_root("output/");
//    int count = 1;
//
//    boost::filesystem::create_directories ("output"); //create a directory for results
//
//    for (std::string i : addresses)
//    {
//
//    	html_text = downloader.download(i); //for every link from file download the html code
//    	filename = path_root + "kod_html" + std::to_string(count) + ".txt";
//    	downloader.write_str_to_file(filename, html_text);
//    	html_text = downloader.cleanhtml(html_text); //clean downloaded html code
//    	filename = path_root + "kod_html" + std::to_string(count) + "_clean.txt";
//    	downloader.write_str_to_file(filename, html_text); //writo to file cleaned html code
//    	std::string output_of_parsing;
//
//    	//false is you want to save output of parsing in string, else if you want to save it in file ,set true, and not set last parameter
//    	downloader.parse_html_and_save(html_text, "/html/body", count, false, output_of_parsing);  //parse html_text and save to file in /output directory text from "html/body" node, that means only the text between <body></body>
//
//      	std::cout << output_of_parsing <<std::endl; //outputs to the console result of parsing
//      	count++;
//
//    }
    DataPreprocessor dataPreprocessor;
    dataPreprocessor.get_attribs("linki.txt");


   return 0;
}