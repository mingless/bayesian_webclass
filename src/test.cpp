#include <html_downloader.h>


int main(){
	HTTPDownloader downloader;

    //download html text from html_address
    //std::string content = downloader.download(html_address);
    std::vector<std::string> addresses = downloader.get_urls_from_file("http_addresses.txt");
    for (auto i : addresses)
    {
    	std::cout << i << std::endl;
    }
}