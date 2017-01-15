//
// Created by bartek on 14.01.17.
//
#include <gtest/gtest.h>
#include <bayesian_webclass/csv.h>
#include <fstream>

struct csv_content
{
    std::string filename;
    int col1;
    int col2;
    int id;
    std::string domain_name;
};


struct CsvTest : ::testing::Test, ::testing::WithParamInterface<csv_content>
{
    std::unique_ptr<Csv> csv;

    CsvTest(): csv(new Csv()) {};
};

TEST_P(CsvTest, ValidCsvs)
{
    Csv::map id_domain_map;
    auto as = GetParam();
    int column_numbers[2];
    column_numbers[0]= as.col1;
    column_numbers[1]= as.col2;
    id_domain_map = csv->get2ColumnsFromCsv(as.filename,column_numbers);
    auto rec = id_domain_map.begin(); //each testing csv has one line
    EXPECT_EQ(as.id, rec->first);
    EXPECT_EQ(as.domain_name, rec->second);
}

INSTANTIATE_TEST_CASE_P(Default, CsvTest, ::testing::Values(
        csv_content{"data/valid_formatting.csv",0,2,31,"costam"},
        csv_content{"data/valid_formatting.csv",1,3,45,"onet.pl*&321':"},
        csv_content{"data/valid_formatting.csv",1,5,0,""},
        csv_content{"data/non_int_id.csv",0,2,0,""},
        csv_content{"data/reverse_formating.csv",1,0,4232,"onet.pl"}
));

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

