#include <bayesian_webclass/http_downloader.h>


HTTPDownloader::HTTPDownloader()
{
    curl = curl_easy_init();
}

HTTPDownloader::~HTTPDownloader()
{
    curl_easy_cleanup(curl);
}


std::size_t write_data(void *ptr, std::size_t size, std::size_t nmemb, void *stream)
{
    std::string data((const char *) ptr, (std::size_t) size * nmemb);
    *((std::stringstream *) stream) << data << std::endl;
    return size * nmemb;
}

std::string HTTPDownloader::download(const std::string &url, bool& is_downloadable) //TODO prawdopodobnie będzie jednak jeszcze potrzebna wersja nie uzywajaca flagi i po prostu zwracajaca stringa
{
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1); //Prevent "longjmp causes uninitialized stack frame" bug
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "deflate");
    std::stringstream out;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
    /* Perform the request, res will get the return code */
    CURLcode res = curl_easy_perform(curl);
    /* Check for errors */
    if (res != CURLE_OK)
    {
        is_downloadable=false;
    }
    return out.str();
}


void HTTPDownloader::write_str_to_file(std::string filename, std::string str)
{
    std::ofstream myfile;
    myfile.open(filename);
    myfile << str;
    myfile.close();

}

std::string HTTPDownloader::cleanhtml(const std::string &html)
{
    // init a tidy document
    TidyDoc tidy_doc = tidyCreate();
    TidyBuffer output_buffer = {0};

    // configure tidy
    // the flags tell tidy to output xml and disable warnings
    bool config_success = tidyOptSetBool(tidy_doc, TidyXmlOut, yes)
                          && tidyOptSetBool(tidy_doc, TidyQuiet, yes)
                          && tidyOptSetBool(tidy_doc, TidyNumEntities, yes)
                          && tidyOptSetBool(tidy_doc, TidyShowWarnings, no);

    int tidy_rescode = -1;

    // parse input
    if (config_success)
        tidy_rescode = tidyParseString(tidy_doc, html.c_str());

    // process html
    if (tidy_rescode >= 0)
        tidy_rescode = tidySaveBuffer(tidy_doc, &output_buffer);

    if (tidy_rescode < 0)
        throw ("tidy has a error: " + tidy_rescode);

    std::string result = (char *) output_buffer.bp;
    tidyBufFree(&output_buffer);
    tidyRelease(tidy_doc);

    return result;
}

std::string HTTPDownloader::get_url_address_from_console()
{
    std::cout << "Enter address of html to download:" << std::endl;
    std::string address;
    std::cin >> address;
    return address;
}

std::vector<std::string> HTTPDownloader::get_urls_from_file(std::string filename)
{
    std::vector<std::string> http_address; //stores html addresses
    std::string line;
    std::ifstream http_address_file(filename); //in every line should be other http address
    if (http_address_file.is_open())
    {
        while (!http_address_file.eof())
        {
            getline(http_address_file, line);
            http_address.push_back(line);
        }
        http_address_file.close();
    } else
    {
        std::cout << "nie udalo sie otworzyc pliku" << std::endl;
    }

    return http_address;
}

void print_node_and_children(const xmlpp::Node *node, std::ofstream &text_from_body_file);

std::string print_node_and_children(const xmlpp::Node *node);

bool HTTPDownloader::parse_html_and_save(const std::string &html_text, const std::string &node_of_html_tree, int count,
                                         bool file_or_string, std::string &output)
{
    //parse html_text and save to file only the text from given node_of_html_tree
    //generated file is output/raw_'count'
    xmlpp::DomParser parser;
    std::ofstream body_text_file;
    std::string path_root("output/"), filename;
    parser.parse_memory(html_text);  //parse html code from string
    //xmlpp::Node* rootNode = parser.get_document()->get_root_node();

    if (parser)
    {
        //Walk the tree
        const xmlpp::Node *pNode = parser.get_document()->get_root_node();
        xmlpp::NodeSet result = pNode->find(node_of_html_tree);         //find node given in function parameter

        if (file_or_string) //jeżeli chcemy azpisywać wynik w pliku
        {
            filename = path_root + "raw_" + std::to_string(count) + ".txt";
            body_text_file.open(filename);
        }
        for (auto i : result) //for every result print text from node to file
        {
            if (file_or_string)
            {
                print_node_and_children(i, body_text_file);
            } else
            {
                output += print_node_and_children(i);
            }
        }
        if (file_or_string)
        {
            std::cout << "Raw text from site " << count << " is in " << filename << std::endl;
            body_text_file.close();
        }
    }
}

std::string print_node_and_children(const xmlpp::Node *node)
{
    //recursive function that prints to given ofstream (exp file) the text of give node from parsed xml file
    const xmlpp::ContentNode *nodeContent = dynamic_cast<const xmlpp::ContentNode *>(node);
    const xmlpp::TextNode *nodeText = dynamic_cast<const xmlpp::TextNode *>(node);

    std::string output;

    if (nodeText && nodeText->is_white_space()) //Let's ignore the indenting - you don't always want to do this.
        return "";

    if (nodeText)
    {
        output += nodeText->get_content();
    }

    if (!nodeContent)
    {
        //Recurse through child nodes:
        xmlpp::Node::NodeList list = node->get_children();
        for (xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
        {
            output += print_node_and_children(*iter); //recursive
        }
    }
    return output;
}

void print_node_and_children(const xmlpp::Node *node, std::ofstream &text_from_body_file)
{
    //recursive function that prints to given ofstream (exp file) the text of give node from parsed xml file 
    const xmlpp::ContentNode *nodeContent = dynamic_cast<const xmlpp::ContentNode *>(node);
    const xmlpp::TextNode *nodeText = dynamic_cast<const xmlpp::TextNode *>(node);

    if (nodeText && nodeText->is_white_space()) //Let's ignore the indenting - you don't always want to do this.
        return;

    if (nodeText)
    {
        text_from_body_file << nodeText->get_content() << std::endl;
    }

    if (!nodeContent)
    {
        //Recurse through child nodes:
        xmlpp::Node::NodeList list = node->get_children();
        for (xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
        {
            print_node_and_children(*iter, text_from_body_file); //recursive
        }
    }
}










