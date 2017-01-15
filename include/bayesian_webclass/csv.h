//
// Created by bartek on 14.01.17.
//

#ifndef BAYESIAN_WEBCLASS_CSV_H
#define BAYESIAN_WEBCLASS_CSV_H

#include <map>
#include <string>
#include <boost/tokenizer.hpp>


class Csv
{
public:
    typedef boost::tokenizer<boost::char_separator<char>> t_tokenizer;
    typedef std::map<int, std::string> map;
    //typedef std::shared_ptr<map> map_ptr;
private:
 //   map _id_domain_map;

public:


//    Csv(map_ptr id_domain_map) : _id_domain_map(id_domain_map)
//    {};


    map get2ColumnsFromCsv(const std::string& filename,
                    const int columns_numbers[2]); //parses csv-type file from filename to the map, takes data from only
    // two columns given in columns_numbers. 1st element of array -> key of map, 2nd -> value.
    // Key should be value castable to int.
    // Returns empty map (0=>"") if something went wrong

    void delete_disabled_domains(map &id_domain_map); //delets misworking links from given map
    void match_values();

};


#endif //BAYESIAN_WEBCLASS_CSV_H
