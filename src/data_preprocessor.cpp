//
// Created by bartek on 15.01.17.
//

#include <iostream>
#include <fstream>
#include <boost/filesystem/operations.hpp>
#include "bayesian_webclass/data_preprocessor.h"

DataPreprocessor::DataPreprocessor(std::string curl_out_folder) : ptr_csv(new Csv()), ptr_http(new HTTPDownloader()),
                                                                  _curl_output_folder(curl_out_folder)
{}

bool DataPreprocessor::filter_valid_domains(const std::string &input_filename, const std::string &output_filename)
{
    int csv_columns[2] = {0, 1};
    bool success = this->ptr_csv->get_2_columns_from_csv(input_filename, csv_columns);
    Csv::map::iterator map_it;
    bool is_downloadable = true;
    int good_links = 0, all_links = 0; //counter of usable links
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
            is_downloadable = this->ptr_http->download(map_it->second, a);

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

bool DataPreprocessor::parse_htmls(const std::string &filename,
                                   const std::string &from_which_tags) //TODO eg. "/html/body" which tags
{
    //get html addresses from textfile to vector of string
    string_vec addresses = ptr_http->get_urls_from_file(filename);
    std::string html_text, file_path;
    std::string path_root(this->_curl_output_folder + "/");
    std::set<std::string> all_atribs;
    int count = 1;
    bool success = false;
    boost::filesystem::create_directories(this->_curl_output_folder); //create a directory for results

    for (std::string i : addresses)
    {
        html_text = "";
        success = ptr_http->download(i, html_text); //for every link from file download the html code
        if (!success)
        {
            //std::cout << "nie udalo sie pobrac" << std::endl;
            return false;
        }
        file_path = path_root + "kod_html" + std::to_string(count) + ".txt";
        ptr_http->write_str_to_file(file_path, html_text);
        std::string line, line1;
        std::ifstream ifs(file_path);
        html_text = "";
        file_path = path_root + "kod_html" + std::to_string(count) + "_parsed.txt";
        while (!ifs.eof())
        {
            getline(ifs, line);

            while (line.back() != '>')
            {
                if (!ifs.eof())
                {
                    getline(ifs, line1);
                    line += line1;
                } else
                {
                    break;//TODO cos tu sie zapetla, trzeba naprawic
                }//line.erase(std::remove(line.begin(),line.end(),' '), line.end());
            }
            html_text += line;
            html_text += '\n';
        }
        // ptr_http->write_str_to_file("kod_przed_clean.txt", html_text);
        //html_text = ptr_http->cleanhtml(html_text); //clean downloaded html code
//        file_path = path_root + "kod_html" + std::to_string(count) + "_parsed.txt";
        //   ptr_http->write_str_to_file("kod_po_clean.txt", html_text);
        std::string output_of_parsing;
        std::set<std::string> map_of_attribs;
        ptr_http->parse_html_and_save(html_text, from_which_tags,
                                      output_of_parsing,
                                      map_of_attribs);  //parse html_text and save to file in /output directory text from "html/body" node, that means only the text between <body></body>
        all_atribs.insert(map_of_attribs.begin(), map_of_attribs.end());
        ptr_http->write_str_to_file(file_path, output_of_parsing); //write to file parsed html

        count++;
    }

    ptr_http->write_set_to_file(path_root+"all_attributes.txt",all_atribs);
    std::cout <<"Ilosc wszystkich atrybutow: " <<all_atribs.size() << std::endl;
    return success;
}

void DataPreprocessor::get_attribs(const std::string &filename)
{
    parse_htmls("linki.txt","/html/body/div[@id='content']/div[@id='bodyContent']/div[@id='mw-content-text']/p"); //second argument is written i
}


