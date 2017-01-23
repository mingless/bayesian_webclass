//
// Created by bartek on 14.01.17.
//

#ifndef BAYESIAN_WEBCLASS_CSV_H
#define BAYESIAN_WEBCLASS_CSV_H

#include <map>
#include <string>
#include <boost/tokenizer.hpp>
#include <bits/unique_ptr.h>

/** \class Csv
 *  \brief Class for managing csv files.
 * A simple class for read/write operations on csv files.
 */

class Csv
{
    public:
        typedef boost::tokenizer<boost::char_separator<char>> t_tokenizer;
        typedef std::map<int, std::string> map;
    private:
        std::unique_ptr<map> _id_url_map;
        const int _max_invalid_ids; //defaults to 6
    public:

        Csv(): _id_url_map(new map()), _max_invalid_ids(6){};
        Csv(const int max_invalid_ids): _id_url_map(new map()), _max_invalid_ids(max_invalid_ids){};

        const std::unique_ptr<map> &getId_url_map() const;

        bool csv2map(const std::string &filename,
                     const int col1, const int col2);

        void delete_disabled_urls(map &id_url_map);
        void match_values();

};


#endif //BAYESIAN_WEBCLASS_CSV_H
