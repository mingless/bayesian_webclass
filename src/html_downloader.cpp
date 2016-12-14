#include "http_downloader.h"

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


void HTTPDownloader::write_html_to_file(std::string filename, std::string html_address{
    HTTPDownloader downloader;

    //download html text from html_address
    std::string content = downloader.download(html_address);
    
    //save html to file
    std::ofstream myfile;
    myfile.open(filename);
    myfile << content;
    myfile.close();
        
}

std::string cleanhtml(const std::string &html)
{
    // init a tidy document
    TidyDoc tidy_doc=tidyCreate();
    TidyBuffer output_buffer= {0};
 
    // configure tidy
    // the flags tell tidy to output xml and disable warnings
    bool config_success=tidyOptSetBool(tidy_doc,TidyXmlOut,yes)
                        && tidyOptSetBool(tidy_doc,TidyQuiet,yes)
                        && tidyOptSetBool(tidy_doc,TidyNumEntities,yes)
                        && tidyOptSetBool(tidy_doc,TidyShowWarnings,no);
 
    int tidy_rescode=-1;
 
    // parse input
    if(config_success)
        tidy_rescode=tidyParseString(tidy_doc,html.c_str());
 
    // process html
    if(tidy_rescode>=0)
        tidy_rescode=tidySaveBuffer(tidy_doc,&output_buffer);
 
    if(tidy_rescode<0)
        throw("tidy has a error: "+tidy_rescode);
 
    std::string result=(char *)output_buffer.bp;
    tidyBufFree(&output_buffer);
    tidyRelease(tidy_doc);
 
    return result;
}

std::string HTTPDownloader::get_url_address(){
    std::cout << "Enter address of html to download:" << std::endl;
    std::cin >> std::string address;
    return address;
}

std::vector<std::string> HTTPDownloader get_urls_from_file(std::string filename){
    std::vector<std::string> http_address; //stores addresses of html
    string line;
    ifstream http_address_file (filename); //in every line should be other http address 
    if (http_address_file.is_open())
    {
        while (! http_address_file.eof() )
        {
            getline (http_address_file,line);
            http_address.push_back(line);
        }
    http_address_file.close();
  }


}










