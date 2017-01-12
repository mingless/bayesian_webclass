//
// Created by bartek on 12.01.17.
//
#include <boost/tokenizer.hpp>
#include <string>
#include <iostream>
#include "bayesian_webclass/csv.h"
#include <typeinfo>
#include <vector>
#include <fstream>

typedef boost::tokenizer<boost::char_separator<char>> t_tokenizer;

std::vector<std::string> tokenizeCsv()
{
    std::string filename("csv/dns.csv");
    std::ifstream dns_csv(filename); //in every line should be other http address
    if (dns_csv.is_open())
    {
        std::string line;
        boost::char_separator<char> sep{";"};

        while (!dns_csv.eof())
        {
            getline(dns_csv, line);
            t_tokenizer tok{line, sep};
            int i = 0;
            for (auto &t : tok)
            {
                if (i == 2)
                {
                    std::cout << t << std::endl;
                }
                ++i;
            }
        }
    } else
    {
        std::cout << "nie udalo sie otworzyc pliku:" << filename << std::endl;
    }


//    std::string s(
//            "ID;wwwID;nowwwID;UserID;Date;Hosting Type;Language;Adult Content;Other Problem;Web Spam;News/Editorial;Commercial;Educational/Research;Discussion;Personal/Leisure;Media;Database;Readability-Vis;Readability-Lang;Neutrality;Bias;Trustiness;Confidence;auto_lang");
//    boost::char_separator<char> sep{";"};
//    tokenizer tok{s, sep};
//    std::vector<std::string> csv_labels;
//    for(tokenizer<>::iterator beg=tok.begin(); beg!=tok.end();++beg)
//    {
//        csv_labels.push_back(std::move(t));
//    }
//    for (const auto &t : csv_labels)
//    {
//        std::cout << t << std::endl;
//    }
//    return csv_labels;
//
//}

}