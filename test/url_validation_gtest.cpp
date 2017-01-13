#include <bayesian_webclass/http_downloader.h>
#include <gtest/gtest.h>


// Test declaration
TEST(HTTPDownloaderTestSuite, urlValidCase) {
    HTTPDownloader downloader;
    bool success = 1;
    int result;
    std::string s;
    std::vector<std::string> addresses = downloader.get_urls_from_file("data/invalid_urls.txt");
    for (std::string i : addresses)
    {
        try {
            s = downloader.download(i, result);
        }
        catch(...) {
            success = 0;
        }
    }
    if(result)
    {
        std::cerr << "Libcurl error code " << result << " : " << s << std::endl;
    }
    EXPECT_TRUE(success);
    EXPECT_EQ(0, result);
}

TEST(HTTPDownloaderTestSuite, urlInvalidCase) {
    HTTPDownloader downloader;
    bool success = 1;
    int result;
    std::string s;
    std::vector<std::string> addresses = downloader.get_urls_from_file("data/invalid_urls.txt");
    for (std::string i : addresses)
    {
        try {
            s = downloader.download(i, result);
        }
        catch(...) {
            success = 0;
        }
    }
    if(result)
    {
        std::cerr << "Libcurl error code " << result << " : " << s << std::endl;
    }
    EXPECT_TRUE(success);
    EXPECT_NE(0, result);
}

int main(int argc, char **argv)
{
    try {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }
    catch (std::exception &e) {
        std::cerr << "Unhandled Exception: " << e.what() << std::endl;
    }
  return 1;
}
