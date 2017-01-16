//
// Created by bartek on 15.01.17.
//

#include <iostream>
#include <fstream>
#include <boost/filesystem/operations.hpp>
#include "bayesian_webclass/data_preprocessor.h"

DataPreprocessor::DataPreprocessor(std::string curl_out_folder) :
    ptr_http(new HTTPDownloader()),
    ptr_csv(new Csv()),
    _curl_output_folder(curl_out_folder){}

bool DataPreprocessor::filter_valid_domains(const std::string& input_filename,const std::string& output_filename)
{
    bool success = this->ptr_csv->csv2map(input_filename, 0, 1);
    Csv::map::iterator map_it;
    bool is_downloadable = true;
    int good_links = 0,all_links = 0; //counter of usable links
    std::ofstream valid_domains_file;
    valid_domains_file.open(output_filename);
    if (!valid_domains_file.is_open()) //invalid file
    {
        return false;
    } else
    {
        for (map_it = ptr_csv->getId_domain_map()->begin(); map_it != ptr_csv->getId_domain_map()->end(); ++map_it)
        {
            std::cout << map_it->first << "  " << map_it->second << std::endl << std::flush;
            all_links++;
            std::string a;
            is_downloadable = this->ptr_http->download(map_it->second,a);

            if (is_downloadable)
            {
                valid_domains_file << map_it->first << ";" << map_it->second << "\n";
                ++good_links;
            }
        }
        valid_domains_file.close();
        double prc = good_links / (double) all_links;
        prc = prc * 100;

        std::cout << "All links: " << all_links << "\nGood links: " << good_links << "\nPercentage: " << prc << "%"
            << std::endl << std::flush;
    }
    return success;
}

bool DataPreprocessor::parse_htmls(const std::string &filename,const std::string& from_which_tags) //TODO eg. "/html/body" which tags
{
    //get html addresses from textfile to vector of string
    string_vec addresses = ptr_http->get_urls_from_file(filename);
    std::string html_text,file_path;
    std::string path_root(this->_curl_output_folder+"/");
    int count = 1;
    bool success = false;
    boost::filesystem::create_directories (this->_curl_output_folder); //create a directory for results

    for (std::string i : addresses)
    {
        success = ptr_http->download(i, html_text); //for every link from file download the html code
        file_path = path_root + "html/kod_html" + std::to_string(count) + ".txt";
        ptr_http->write_str_to_file(file_path, html_text);
        html_text = ptr_http->cleanhtml(html_text); //clean downloaded html code
        file_path = path_root + "parsed/kod_html" + std::to_string(count) + "_parsed.txt";
        std::string output_of_parsing;
        ptr_http->parse_html_and_save(html_text, from_which_tags, output_of_parsing);  //parse html_text and save to file in /output directory text from "html/body" node, that means only the text between <body></body>

        ptr_http->write_str_to_file(file_path, output_of_parsing); //write to file parsed html

        count++;
    }
    return success;
}


