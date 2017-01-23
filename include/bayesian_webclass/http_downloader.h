#ifndef HTTP_DOWNLOADER_H
#define HTTP_DOWNLOADER_H

#include <string>
#include <vector>
#include <set>

/**\class HTTPDownloader
 * \brief Class for downloading and cleaning html code
 * Class contain methods to download and clean html code
 * from given urls. It has also tools for parsing html code.
 */
class HTTPDownloader
{
public:
	HTTPDownloader();
	virtual ~HTTPDownloader();
    void writeStrToFile(const std::string &filename, const std::string &str);
	void writeSetToFile(const std::string &filename, const std::set<std::string> &set);
    
    bool download(const std::string& url, std::string& output); //downloads html text from given url
	bool check_link(const std::string& url);


	std::string cleanhtml(const std::string &html); //makes html tidy, brackets are closed and  that html code is ready to be parsed

	std::vector<std::string> getLinesFromFile(std::string filename); //gets list of html addresses from given file in every line should be one http_address
	int parseHtmlAndSave(const std::string &htmlText, const std::string &nodeOfHtmlTree, std::string &output,
						 std::set<std::string> &uniqueAttributes); //parse html_text and save to file only the text from given node_of_html_tree


private:
    void* curl;
};


#endif
