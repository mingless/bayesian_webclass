//
// Created by bartek on 15.01.17.
//

#ifndef BAYESIAN_WEBCLASS_DATAPREPROCESSOR_H
#define BAYESIAN_WEBCLASS_DATAPREPROCESSOR_H


#include "csv.h"
#include "http_downloader.h"

class DataPreprocessor
{
    typedef std::vector<std::string> string_vec;
    private:
        std::unique_ptr<HTTPDownloader> ptr_http;
        std::unique_ptr<Csv> ptr_csv;
        std::string _curl_output_folder;

    public:
        DataPreprocessor(std::string curl_out_folder = "output");;
        bool filter_valid_domains (const std::string& input_file,const std::string& output_file);
        bool parse_htmls(const std::string& filename,const std::string& from_which_tags);

};


#endif //BAYESIAN_WEBCLASS_DATAPREPROCESSOR_H
