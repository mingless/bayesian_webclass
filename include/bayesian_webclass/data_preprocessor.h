//
// Created by bartek on 15.01.17.
//

#ifndef BAYESIAN_WEBCLASS_DATAPREPROCESSOR_H
#define BAYESIAN_WEBCLASS_DATAPREPROCESSOR_H


#include "csv.h"
#include "http_downloader.h"

class DataPreprocessor {

    typedef std::vector<std::string> string_vec;
private:

    std::unique_ptr<Csv> ptr_csv;
    std::string _curl_output_folder;
    long atrybuty;
    int fileCounter;
    std::set<std::string> all_atribs;
public:
    const std::set<std::string> &getAll_atribs() const;

public:
    std::unique_ptr<HTTPDownloader> ptr_http;
    long getAtrybuty() const;

public:

    DataPreprocessor(std::string curl_out_folder = "output");;

    bool filter_valid_domains(const std::string &input_file, const std::string &output_file);
    std::set<std::string> parse_html(const std::string& url);
    bool parse_htmls(const std::string &filename, const std::string &from_which_tags);

    void get_attribs(const std::string &filename);
    bool get_attribs_from_link(const std::string& url);
    void choose_train_data(const std::string& filename);
};

#endif //BAYESIAN_WEBCLASS_DATAPREPROCESSOR_H
