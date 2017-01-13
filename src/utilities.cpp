//
// Created by bartek on 12.01.17.
//
#include <boost/tokenizer.hpp>
#include <iostream>
#include <fstream>
#include <map>
#include <curl/curl.h>
#include "bayesian_webclass/http_downloader.h"
typedef boost::tokenizer<boost::char_separator<char>> t_tokenizer;
typedef std::map<int,std::string> map;

map tokenizeCsv() //return sorted by id map with domain names
{
    std::string filename("csv/dns.csv");
    std::ifstream domain_csv(filename); //in every line should be other http address
    std::string::size_type sz; //needed to stoi -> string to int
    map id_domain_map; //key -> id of record in csv, value domain name
    if (domain_csv.is_open())
    {
        std::string line;
        boost::char_separator<char> sep{";"};

        getline(domain_csv, line); //in first line are identifiers, don't need them
        while (!domain_csv.eof())
        {
            getline(domain_csv, line);
            t_tokenizer tok{line, sep};
            int i = 0, id = 0;
            std::string domain_address;
            for (auto &t : tok)             //TODO zoptymalizować żeby przechodziło pętle tylko do i = 0 i i = 2, a nie, bo potrzebujemy tylko 0 i 2 kolumny
            { //in csv file the 1st column is id of website and in 3rd there is its domain
                if (i == 0)
                {
                    id = std::stoi(t, &sz);
                } else if (i == 2)
                {
                    domain_address = t;
                } else if (i == 3) //TODO moznaby to zrobić bardziej elegancko
                {
                    id_domain_map.insert(std::pair<int, std::string>(id, domain_address));
                }
                ++i;
            }


        }
    } else
    {
        std::cout << "nie udalo sie otworzyc pliku:" << filename << std::endl;
    }

    return id_domain_map;
}

 void delete_disabled_domains(map& id_domain_map) //delets from map records which domains cannot be opened
{
    std::unique_ptr<HTTPDownloader> d(new HTTPDownloader());
    bool is_downloadable = true; //flag used to tell if link works
    map::iterator map_it;
    for (map_it = id_domain_map.begin(); map_it != id_domain_map.end(); ++map_it)
    {
        d->download(map_it->second,is_downloadable);
        if (!is_downloadable)
        {
            id_domain_map.erase(map_it);
        }
    }
}