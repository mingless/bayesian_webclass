//
// Created by bartek on 14.01.17.
//

#ifndef BAYESIAN_WEBCLASS_CSV_H
#define BAYESIAN_WEBCLASS_CSV_H

#include <map>
#include <string>
#include <boost/tokenizer.hpp>
#include <bits/unique_ptr.h>

class Csv
{
    public:
        typedef boost::tokenizer<boost::char_separator<char>> t_tokenizer;
        typedef std::map<int, std::string> map;
    private:
        std::unique_ptr<map> _id_url_map;
        const int _max_invalid_ids; //default 6
    public:

        Csv(): _id_url_map(new map()), _max_invalid_ids(6){};
        Csv(const int max_invalid_ids): _id_url_map(new map()), _max_invalid_ids(max_invalid_ids){};

        //    Csv(map_ptr id_url_map) : _id_url_map(id_url_map)
        //    {};

        const std::unique_ptr<map> &getId_url_map() const;

        bool csv2map(const std::string &filename,
                     const int col1, const int col2);

        void delete_disabled_urls(map &id_url_map);  // Remove invalid urls from a url map.
        void match_values();

};


#endif //BAYESIAN_WEBCLASS_CSV_H
