//
// Created by bartek on 14.01.17.
//
#include <gtest/gtest.h>
#include <bayesian_webclass/csv.h>
#include <fstream>
#include <bayesian_webclass/data_preprocessor.h>

struct csv_content
{
    std::string filename;
    int col1;
    int col2;
    int id;
    std::string domain_name;
    bool success;
};


struct CsvTest : ::testing::Test, ::testing::WithParamInterface<csv_content>
{
    std::unique_ptr<Csv> csv;

    CsvTest() : csv(new Csv())
    {};
};

TEST_P(CsvTest, ValidCsvs)
{
    auto as = GetParam();
    int column_numbers[2];
    column_numbers[0] = as.col1;
    column_numbers[1] = as.col2;
    bool res = csv->get_2_columns_from_csv(as.filename, column_numbers);
    auto rec = csv->getId_domain_map()->begin(); //each testing csv has one line
    if (!as.success)
    {
        EXPECT_EQ(as.success, res);
    }else
    {
        EXPECT_EQ(as.id, rec->first);
        EXPECT_EQ(as.domain_name, rec->second);
    }
}

INSTANTIATE_TEST_CASE_P(Default, CsvTest, ::testing::Values(
        csv_content{"data/valid_formatting.csv", 0, 2, 31, "costam", true},
        csv_content{"data/valid_formatting.csv", 1, 3, 45, "onet.pl*&321':", true},
        csv_content{"data/valid_formatting.csv", 1, 7, 0, "", false},
        csv_content{"data/non_int_id.csv", 0, 2, 0, "", false},
        csv_content{"data/one_non_int_id.csv", 0, 2, 69, "lozko", true},
        csv_content{"data/reverse_formating.csv", 1, 0, 4232, "onet.pl", true},
        csv_content{"fasdfas",1,3,0,"",false} //wrong file
));



struct input
{
    std::string filename;
    bool success;
    int id;
    std::string address;

};


struct DataPrepTest : ::testing::Test, ::testing::WithParamInterface<input>
{
    std::unique_ptr<DataPreprocessor> data_prep;

    DataPrepTest() : data_prep(new DataPreprocessor()) {};
};

TEST_P(DataPrepTest, DataPrepTest_GoodProcessing_Test)
{
    auto as = GetParam();
    std::string output("output");
    EXPECT_EQ(as.success, data_prep->filter_valid_domains(as.filename,output));
}

INSTANTIATE_TEST_CASE_P(Default, DataPrepTest, ::testing::Values(
        input {"wrong_filename", false},
        input {"csv/4dns.csv", true}
));


int main(int argc, char **argv)
{
    try
    {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }
    catch (std::exception &e)
    {
        std::cerr << "Unhandled Exception: " << e.what() << std::endl;
    }
    return 1;
}

