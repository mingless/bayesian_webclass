#include <iostream>
#include <fstream>
#include <boost/filesystem/operations.hpp>
#include "bayesian_webclass/data_preprocessor.h"

/**Constructor
 * @param curl_out_folder folder in which output of data preprocessing will be stored
 */
DataPreprocessor::DataPreprocessor(std::string curl_out_folder) : ptr_csv(new Csv()), ptr_http(new HTTPDownloader()),
                                                                  _curl_output_folder(curl_out_folder), fileCounter(0){}

/** Filter domains that return quick http response
 * Filer domains that give http reponse in less than 5 seconds and save them
 * in "output_filename" file
 * @param input_filename name of file with links, every link in other line
 * @param output_filename links that can be opened will be stored in this file
 * @return false is cannot open file, otherwise return true
 */
bool DataPreprocessor::filterValidDomains(const std::string &input_filename, const std::string &output_filename) {

    int csv_columns[2] = {0, 1};
    bool success = this->ptr_csv->csv2map(input_filename, csv_columns[0],csv_columns[1]);
    Csv::map::iterator map_it;
    bool is_downloadable = true;
    int good_links = 0, all_links = 0; //counter of usable links
    std::ofstream valid_domains_file;
    valid_domains_file.open(output_filename);
    if (!valid_domains_file.is_open()) //invalid file
    {
        return false;
    } else {
        for (map_it = ptr_csv->getId_url_map()->begin(); map_it != ptr_csv->getId_url_map()->end(); ++map_it) {
            std::cout << map_it->first << "  " << map_it->second << std::endl << std::flush;
            all_links++;
            std::string a;
            is_downloadable = this->ptr_http->download(map_it->second, a);

            if (is_downloadable) {
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

/**Parse html code.
 * Parse html code from all links in filename. Html code is parsed depending on fromWhickTags parameter
 * which is a
*@param filename - file where html code is
*@param from_which_tags - from which tags from html (or xml) code structure the content should be
 */
bool DataPreprocessor::parseHtmls(const std::string &filename,
                                  const std::string &from_which_tags) {
    //get html addresses from textfile to vector of string
    string_vec addresses = ptr_http->getLinesFromFile(filename + ".txt");
    std::string html_text, file_path;
    std::string path_root(this->_curl_output_folder + "/");

    bool success = false;
    boost::filesystem::create_directories(this->_curl_output_folder); //create a directory for results
    int count = 0;
    for (std::string i : addresses) {
        html_text = "";
        success = ptr_http->download(i, html_text); //for every link from file download the html code
        if (!success) {
            break;
        }
        file_path = path_root + "tmp.txt";
        ptr_http->writeStrToFile(file_path, html_text);
        std::string line, line1;
        std::ifstream ifs(file_path);
        html_text = "";
        file_path = path_root + std::to_string(this->fileCounter) + ".txt";
        while (!ifs.eof()) {
            getline(ifs, line);

            while (line.back() != '>') {
                if (!ifs.eof()) {
                    getline(ifs, line1);
                    line += line1;
                } else {
                    break;//TODO cos tu sie zapetla, trzeba naprawic
                }
            }
            html_text += line;
            html_text += '\n';
        }

        std::string output_of_parsing(filename + "\n");
        output_of_parsing.erase(0,11); //erase train_data
        std::set<std::string> map_of_attribs;
        ptr_http->parseHtmlAndSave(html_text, from_which_tags,
                                   output_of_parsing,
                                   map_of_attribs);  //parse html_text and save to file in /output directory text from "html/body" node, that means only the text between <body></body>
        all_atribs.insert(map_of_attribs.begin(), map_of_attribs.end());
        ptr_http->writeStrToFile(file_path, output_of_parsing); //write to file parsed html

        this->fileCounter++;
        count++;
        if(count>20){
            break;
        }
    }
   std::cout << "All attributes" << all_atribs.size() << std::endl;
    return success;
}

/** Get attributes from all websites in file
 * Get attribites from websites in "filename" file and save them
 * in curl_output_folder. Output files will be named from 1 to number
 * of links. Every output will have name of file in first line as a name of
 * category and attributes as all links found in text of link listed every in next
 * line below.
 * @param filename file with links from en.wikipedia.org
 */
void DataPreprocessor::getAttribs(const std::string &filename) {
    parseHtmls(filename,
               "/html/body/div[@id='content']/div[@id='bodyContent']/div[@id='mw-content-text']/p"); //second argument is written i
}

/**Split data to train and test
 * Split data to training and testing set. 1/4 -testing, 3/4 training
 * @param inFilename file with links, every in new line
 */
void DataPreprocessor::chooseTrainData(const std::string &inFilename) {

    std::ifstream file(inFilename + ".txt");
    int lineCount = 0;
    std::string line, category(inFilename);

    boost::filesystem::create_directories("train_data");
    boost::filesystem::create_directories("test_data");


    std::string to_remove = "categories/";
    std::string::size_type i = category.find(to_remove); //don't want direcotry name as a category name

    if (i != std::string::npos)
        category.erase(i, to_remove.length());


    std::string trainFilename("train_data/"+category + ".txt");
    std::string testFilename("test_data/" +category + ".txt");
    std::string train, test;
    while (getline(file, line)) {
        if (lineCount % 4) {
            train += line;
            train += "\n";
        } else {
            test += line;
            test += "\n";
        }
        lineCount++;
    }

    ptr_http->writeStrToFile(trainFilename, train);
    ptr_http->writeStrToFile(testFilename, test);
}

/**Getter
 *
 * @return allAttribs
 */
const std::set<std::string> &DataPreprocessor::getAll_atribs() const {
    return all_atribs;
}

/**Get attributes from wiki link
 * Get attributes from en.wikipedia.org article and save it
 * in example/attribs.txt file. Attributes are links found in text of article.
 * Every attribute is in new line.
 * @param url url address of en.wikipedia.org article
 * @return true - all went good, false - cannot open article/ no internet connection
 */
bool DataPreprocessor::get_attribs_from_link(const std::string &url) {
    std::string html_text, file_path;
    std::string path_root("example/");
    std::string from_which_tags("/html/body/div[@id='content']/div[@id='bodyContent']/div[@id='mw-content-text']/p");
    bool success = false;
    boost::filesystem::create_directories("example"); //create a directory for results
    html_text = "";
    success = ptr_http->download(url, html_text); //for every link from file download the html code
    if (!success) {
        return false;
    }
    file_path = path_root + "tmp.txt";
    ptr_http->writeStrToFile(file_path, html_text);
    std::string line, line1;
    std::ifstream ifs(file_path);
    html_text = "";
    file_path = path_root + "attribs.txt";
    while (!ifs.eof()) { //prepare code for parsing
        getline(ifs, line);

        while (line.back() != '>') {
            if (!ifs.eof()) {
                getline(ifs, line1);
                line += line1;
            } else {
                break;
            }
        }
        html_text += line;
        html_text += '\n';
    }

    std::string output_of_parsing;
    std::set<std::string> map_of_attribs;
    ptr_http->parseHtmlAndSave(html_text, from_which_tags,
                                output_of_parsing,
                                map_of_attribs);  //parse html_text and save to file in /output directory text from "html/body" node, that means only the text between <body></body>
    ptr_http->writeStrToFile(file_path, output_of_parsing); //write to file parsed html
    return success;
}



