#include <iomanip>
#include <iostream>
#include <curl/curl.h>
#include <sstream>
#include <curl/easy.h>
#include <curl/curlbuild.h>
#include <libxml++/libxml++.h>


class HTTPDownloader {
public:
    HTTPDownloader();
    ~HTTPDownloader();
    /**
     * Download a file using HTTP GET and store in in a std::string
     * @param url The URL to download
     * @return The download result
     */
    std::string download(const std::string& url);
private:
    void* curl;
};


std::size_t write_data(void *ptr, std::size_t size, std::size_t nmemb, void *stream) {
    std::string data((const char*) ptr, (std::size_t) size * nmemb);
    *((std::stringstream*) stream) << data << std::endl;
    return size * nmemb;
}

HTTPDownloader::HTTPDownloader() {
    curl = curl_easy_init();
}

HTTPDownloader::~HTTPDownloader() {
    curl_easy_cleanup(curl);
}

std::string HTTPDownloader::download(const std::string& url) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    /* example.com is redirected, so we tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1); //Prevent "longjmp causes uninitialized stack frame" bug
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "deflate");
    std::stringstream out;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
    /* Perform the request, res will get the return code */
    CURLcode res = curl_easy_perform(curl);
    /* Check for errors */
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    }
    return out.str();
}




int main(void)
{
  HTTPDownloader downloader;

  std::string content = downloader.download("http://libxmlplusplus.sourceforge.net/");
  int cnt = 0;
  for (auto i : content){
    if (cnt<10){
      std::cout<<i;
      cnt++;
    }else 
      break;
  }
  

  
  return 0;
}

