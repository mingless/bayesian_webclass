//
// Created by bartek on 14.01.17.
//

#include <bayesian_webclass/csv.h>
#include <fstream>
#include <iostream>

bool Csv::get_2_columns_from_csv(const std::string &filename, const int *columns_numbers)
{
    std::ifstream domain_csv(filename); //in every line should be other http address
    std::string::size_type sz; //needed to stoi -> string to int
    //key -> id of record in csv, value domain name

    if (domain_csv.is_open())
    {
        std::string line;
        boost::char_separator<char> sep{";"}; //used to tokenize csv row

        getline(domain_csv, line); //in first line are identifiers, don't need them
        int s = *std::max_element(columns_numbers, columns_numbers + 2);
        int counter_invalid_id = _max_invalid_ids; //number of ommited invalid ids
        while (!domain_csv.eof())
        {
            //tokenizing csv file line by line
            getline(domain_csv, line);
            if (line == "")
            {
                break;
            }
            t_tokenizer tok{line, sep};

            int column_num = 0, id = 0;
            std::string domain_address;
            if (counter_invalid_id < 0)
            {
                std::cerr << "Too many invalid ids" << std::endl;
                return false;
            }
            try
            {
                for (auto &t : tok)
                {

                    if (column_num == columns_numbers[0]) //id column
                    {
                        id = std::stoi(t, &sz);
                    } else if (column_num == columns_numbers[1]) //domain name column
                    {
                        domain_address = t;
                    } else if (column_num > s) //max element in column_numbers
                    {
                        //don't need to check further columns
                        break;
                    }
                    ++column_num;
                }
            }
            catch (std::invalid_argument &) //if std::stoi throws exeption, we ignore this row with invalid id
            {
             //   std::cerr << "invalid id, counter:" << counter_invalid_id<< std::endl;
                --counter_invalid_id;
                continue;
            }

            if (column_num <= s)
            {   //data from column_numbers and csv file doesn't match, csv hasn't enough columns
                std::cerr << "Number of columns from csv file is smaller than values wanted" << std::endl;
              //  std::cerr << "cols: " << column_num << "s: " << s << std::endl;
                return false; //TODO jakos tu rzucac wyjatek
            } else
            {
                _id_domain_map->insert(std::pair<int, std::string>(id, domain_address));
            }
        }
    } else
    {
        std::cerr << "Cannot open file: " << filename
                  << std::endl; //TODO jakis plik z logami czy cos, nie wypisywanie bledow na konsole, ewentualnie wyjatek
        return false; //TODO jakos tu rzucac wyjatek
    }
    domain_csv.close();
    return true;
}

const std::unique_ptr<Csv::map, std::default_delete<Csv::map>> &Csv::getId_domain_map() const
{
    return _id_domain_map;
}

//void Csv::delete_disabled_domains(Csv::map &id_domain_map) //delets from map records which domains cannot be opened
//{
//    std::unique_ptr<HTTPDownloader> d(new HTTPDownloader());
//    bool is_downloadable = true; //flag used to tell if link works
//    map::iterator map_it;
//    for (map_it = id_domain_map.begin(); map_it != id_domain_map.end(); ++map_it)
//    {
//        d->download(map_it->second, is_downloadable);
//        if (!is_downloadable)
//        {
//            id_domain_map.erase(map_it);
//        }
//    }
//}