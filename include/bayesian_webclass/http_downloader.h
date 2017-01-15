#ifndef HTTP_DOWNLOADER_H
#define HTTP_DOWNLOADER_H


/*TODO
    *tworzenie mapy słów gdzie kluczem jest słowo a wartością ilość jego wystąpień
    *pobieranie linków z bazy danych i sprawdzanie czy się uruchamiają
    *dopasowywanie atrybutów do mapy -> dodawanie wystąpień danego słowa w tekście na podstawie mapy
 */

#include <string>
#include <vector>

class HTTPDownloader
{
public:
	HTTPDownloader();

	virtual ~HTTPDownloader();
    void write_str_to_file(std::string filename, std::string str);

    
    bool download(const std::string& url, std::string& output); //downloads html text from given url
	bool check_link(const std::string& url);

//    std::string download(const std::string& url); //downloads html text from given url
//    std::string download(const std::string& url, int &result); //downloads html text from given url and allows retrieval of any error code


	std::string cleanhtml(const std::string &html); //makes html tidy, brackets are closed and  that html code is ready to be parsed
	std::string get_url_address_from_console(); //lets user to enter url file in console

	std::vector<std::string> get_urls_from_file(std::string filename); //gets list of html addresses from given file in every line should be one http_address
	void parse_html_and_save(const std::string& html_text, const std::string& node_of_html_tree, std::string& output); //parse html_text and save to file only the text from given node_of_html_tree


private:
    void* curl;
};


#endif
