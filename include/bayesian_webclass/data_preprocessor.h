//
// Created by bartek on 15.01.17.
//

#ifndef BAYESIAN_WEBCLASS_DATAPREPROCESSOR_H
#define BAYESIAN_WEBCLASS_DATAPREPROCESSOR_H

#include "csv.h"
#include "http_downloader.h"
/** \class DataPreprocessor
 * \brief Class contining data preprocessing tools.
 * Data Preprocessot contains methods to get training and testing
 * data from en.wikipedia.org for classification including parsing and
 * attributes extracting
 */
class DataPreprocessor {
    typedef std::vector<std::string> string_vec;
private:
    std::unique_ptr<Csv> ptr_csv;
    std::string _curl_output_folder;
    int fileCounter;
    std::set<std::string> all_atribs;
public:
    std::unique_ptr<HTTPDownloader> ptr_http;

    DataPreprocessor(std::string curl_out_folder = "output");;
    const std::set<std::string> &getAll_atribs() const;
    bool filterValidDomains(const std::string &input_file, const std::string &output_file);
    bool parseHtmls(const std::string &filename, const std::string &from_which_tags);
    void getAttribs(const std::string &filename);
    bool get_attribs_from_link(const std::string& url);
    void chooseTrainData(const std::string &filename);
};

#endif //BAYESIAN_WEBCLASS_DATAPREPROCESSOR_H
