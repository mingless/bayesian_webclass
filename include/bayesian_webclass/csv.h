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
        std::unique_ptr<map> _id_domain_map;
        const int _max_invalid_ids; //default 6
    public:

        Csv(): _id_domain_map(new map()), _max_invalid_ids(6){};
        Csv(const int max_invalid_ids): _id_domain_map(new map()), _max_invalid_ids(max_invalid_ids){};

        //    Csv(map_ptr id_domain_map) : _id_domain_map(id_domain_map)
        //    {};

        const std::unique_ptr<map> &getId_domain_map() const;

        bool csv2map(const std::string &filename,
                const int col1, const int col2);
        /*  Parses csv file given with the filename to a map (int-string).
         *  Takes name of the csv file and numbers of the columns in the csv
         *  that will be used respectively as keys and values od the map.
         *  Keys should be type compatible with ints and values with strings.
         *  @param col1 csv column number holding keys for the map.
         *  @param col2 csv column number holding values for the map.
         *  @return Boolean, true if function succeded.
         */

        void delete_disabled_domains(map &id_domain_map);  // Remove invalid urls from a domain map.
        void match_values();

};


#endif //BAYESIAN_WEBCLASS_CSV_H
