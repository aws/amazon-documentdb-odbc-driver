/*
 * Copyright <2021> Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 */

// clang-format off
#include "pch.h"
#include "unit_test_helper.h"
#include "it_odbc_helper.h"
#include "chrono"
#include <vector>
#include <numeric>
#include <iostream>
#include <fstream>
#include "csv_parser.h"
#include <string_view>
#include <cstdlib>

// clang-format on

#define IT_SIZEOF(x) (NULL == (x) ? 0 : (sizeof((x)) / sizeof((x)[0])))
#ifndef WIN32
typedef SQLULEN SQLROWCOUNT;
typedef SQLULEN SQLROWSETSIZE;
typedef SQLULEN SQLTRANSID;
typedef SQLLEN SQLROWOFFSET;
#endif

/******************************************
 * Global Constants
 *****************************************/

#define BIND_SIZE 255
#define ROWSET_SIZE_5 5
#define ROWSET_SIZE_50 50
#define SINGLE_ROW 1
#define ITERATION_COUNT 10

// Header definitions (input csv file must match)
#define QUERY_HEADER "query"
#define LIMIT_HEADER "limit"
#define TEST_NAME_HEADER "test_name"
#define ITERATION_COUNT_HEADER "loop_count"
#define SKIP_TEST_HEADER "skip_test"
#define AVG_TIME_MS_HEADER "result"

// Ensure DSN is setup on machine before running test
const std::string input_file = "Performance_Test_Plan.csv";
const std::string output_file = "Performance_Test_Results.csv";
const test_string dsn_conn_string = CREATE_STRING("DSN=documentdb-perf-test");

// Constants used for ReportTime function
const std::string sync_start = "%%__PARSE__SYNC__START__%%";
const std::string sync_query = "%%__QUERY__%%";
const std::string sync_case = "%%__CASE__%%";
const std::string sync_min = "%%__MIN__%%";
const std::string sync_max = "%%__MAX__%%";
const std::string sync_mean = "%%__MEAN__%%";
const std::string sync_stdev = "%%__STDEV__%%";
const std::string sync_median = "%%__MEDIAN__%%";
const std::string sync_end = "%%__PARSE__SYNC__END__%%";

/******************************************
 * Define Data Structures
 *****************************************/
typedef struct Col {
    SQLLEN data_len;
    SQLCHAR data_dat[BIND_SIZE];
} Col;

struct CSVHeaders {
    int idx_query = -1;
    int idx_limit = -1;
    int idx_skip_test = -1;
    int idx_test_name = -1;
    int idx_iteration_count = -1;
    int idx_avg_time_ms = -1;
};

struct StatisticalInfo {
    long long avg;
    long long min;
    long long max;
    long long median;
    long long stdev;
};

struct TestCase {
    int test_case_num;
    std::string test_name;
    std::string query;
    int limit;
    int num_iterations;
    std::vector< long long > time_ms;
    StatisticalInfo stat_info;
};

/******************************************
 * Define Helper Functions
 *****************************************/

// Calculate statistical info from data vector
void calcStats(TestCase& test_case) {
    size_t size = test_case.time_ms.size();

    // calculate max and min
    test_case.stat_info.max =
        *std::max_element(test_case.time_ms.begin(), test_case.time_ms.end());
    test_case.stat_info.min =
        *std::min_element(test_case.time_ms.begin(), test_case.time_ms.end());

    // calculate average
    test_case.stat_info.avg = std::accumulate(std::begin(test_case.time_ms),
                                              std::end(test_case.time_ms), 0ll)
                              / size;

    // calculate median
    test_case.stat_info.median =
        (size % 2)
            ? test_case.time_ms[size / 2]
            : ((test_case.time_ms[(size / 2) - 1] + test_case.time_ms[size / 2])
               / 2);

    // calculate standard deviation
    // TODO: calculate stdev from vector of data
    // e.g. test_case.stat_info.stdev = stdev;
}

// Checks header values in CSV file
// Sets column index of each header in CSVHeaders struct
void extractCSVHeaders(
    CSVHeaders& headers,
    std::vector< std::vector< Csv::CellReference > >& cell_refs) {
    std::string header_value;

    // set column index in CSVHeaders structure
    for (std::size_t column = 0; column < cell_refs.size(); ++column) {
        const auto& cell = cell_refs[column][0];
        if (cell.getType() == Csv::CellType::String) {
            header_value = cell.getOriginalStringView(false).value();
            if (header_value.compare(QUERY_HEADER) == 0) {
                headers.idx_query = static_cast< int >(column);
            } else if ((header_value.compare(LIMIT_HEADER)) == 0) {
                headers.idx_limit = static_cast< int >(column);
            } else if ((header_value.compare(TEST_NAME_HEADER)) == 0) {
                headers.idx_test_name = static_cast< int >(column);
            } else if ((header_value.compare(ITERATION_COUNT_HEADER)) == 0) {
                headers.idx_iteration_count = static_cast< int >(column);
            } else if ((header_value.compare(SKIP_TEST_HEADER)) == 0) {
                headers.idx_skip_test = static_cast< int >(column);
            } else if ((header_value.compare(AVG_TIME_MS_HEADER)) == 0) {
                headers.idx_avg_time_ms = static_cast< int >(column);
            }
        } else {
            throw std::runtime_error(
                "Header values must be of string type in csv file: "
                + input_file);
        }
    }

    // Check that all headers have been found
    if (headers.idx_query < 0 || headers.idx_limit < 0
        || headers.idx_test_name < 0 || headers.idx_iteration_count < 0
        || headers.idx_skip_test < 0 || headers.idx_avg_time_ms < 0) {
        throw std::runtime_error("Header value(s) missing in csv file: "
                                 + input_file);
    }
}

// Returns true if test should be skipped, otherwise false
bool skipTest(const Csv::CellReference& cell_skip_test) {
    bool skip_test;
    std::string skip_test_str;
    std::string skip_test_header = SKIP_TEST_HEADER;

    // cell value should be string = "TRUE" or "FALSE"
    if (cell_skip_test.getType() == Csv::CellType::String) {
        skip_test_str = cell_skip_test.getCleanString().value();
        if (skip_test_str.compare("TRUE") == 0) {
            skip_test = true;
        } else if (skip_test_str.compare("FALSE") == 0) {
            skip_test = false;
        } else {
            throw std::runtime_error(
                "Incorrect \"" + skip_test_header
                + "\" field (must be TRUE or FALSE) for test case ");
        }
    } else {
        throw std::runtime_error("Incorrect \"" + skip_test_header
                                 + "\" field type for test case ");
    }
    return skip_test;
}

// Returns query string from csv data
std::string getQueryString(const Csv::CellReference& cell_query) {
    std::string query;
    std::string query_header = QUERY_HEADER;

    // cell value for query should be String type
    if (cell_query.getType() == Csv::CellType::String) {
        query = cell_query.getCleanString().value();
    } else {
        throw std::runtime_error("Incorrect \"" + query_header
                                 + "\" field type for test case ");
    }
    return query;
}

// Return limit from csv data
int getLimit(const Csv::CellReference& cell_limit) {
    double limit_dbl;
    int limit_int;
    std::string limit_header = LIMIT_HEADER;

    // cell value for limit should be Double type
    if (cell_limit.getType() == Csv::CellType::Double) {
        limit_dbl = cell_limit.getDouble().value();
        limit_dbl = limit_dbl + 0.5;  // convert double to int (truncation)
        limit_int = (int)limit_dbl;
        if (limit_int < 1) {
            throw std::runtime_error("Incorrect \"" + limit_header
                                     + "\" field (must be > 0) for test case ");
        }
    } else {
        throw std::runtime_error("Incorrect \"" + limit_header
                                 + "\" field type for test case ");
    }
    return limit_int;
}

// Returns test name string
std::string getTestName(const Csv::CellReference& cell_test_name) {
    std::string test_name;
    std::string test_name_header = TEST_NAME_HEADER;

    // cell value for query should be String type
    if (cell_test_name.getType() == Csv::CellType::String) {
        test_name = cell_test_name.getCleanString().value();
    } else {
        throw std::runtime_error("Incorrect \"" + test_name_header
                                 + "\" field for test case ");
    }
    return test_name;
}

// Returns iteration/loop count
int getIterationCount(const Csv::CellReference& cell_iteration_count) {
    double iteration_count_dbl;
    int iteration_count_int;
    std::string iteration_count_header = ITERATION_COUNT_HEADER;

    // cell value for iteration count should be Double type
    if (cell_iteration_count.getType() == Csv::CellType::Double) {
        iteration_count_dbl = cell_iteration_count.getDouble().value();
        iteration_count_dbl =
            iteration_count_dbl + 0.5;  // convert double to int (truncation)
        iteration_count_int = (int)iteration_count_dbl;
        if (iteration_count_int < 1) {
            throw std::runtime_error("Incorrect \"" + iteration_count_header
                                     + "\" field (must be > 0) for test case ");
        }
    } else {
        throw std::runtime_error("Incorrect \"" + iteration_count_header
                                 + "\" field type for test case ");
    }
    return iteration_count_int;
}

// Returns true if string has comma or newline
bool hasCommaOrNewLine(const std::string str) {
    std::size_t comma_pos, newline_pos;
    comma_pos = str.find(',');
    newline_pos = str.find('\n');
    if (comma_pos != std::string::npos || newline_pos != std::string::npos) {
        return true;
    } else {
        return false;
    }
}

// Read csv data from input_file (performance test plan)
std::string readPerformanceTestPlan() {
    // Read the file to string
    std::ifstream ifs(input_file, std::ios::binary);

    if (!ifs.is_open()) {
        throw std::runtime_error("Failed to open input file " + input_file);
    }

    std::string csv_data((std::istreambuf_iterator< char >(ifs)),
                         (std::istreambuf_iterator< char >()));

    ifs.close();  // close csv file
    if (ifs.is_open()) {
        throw std::runtime_error("Failed to close input file " + input_file);
    }
    return csv_data;
}

// Output headers to output file
// col 1 = test #
// col 2 = test name
// col 3 = query
// col 4 = limit
// col 5 = iteration count
// col 6 = avg time
// col 7 = max time
// col 8 = min time
// col 9 = median time
void outputHeaders(std::ofstream& ofs) {
    std::string test_name, query, limit, iter;
    test_name = TEST_NAME_HEADER;
    query = QUERY_HEADER;
    limit = LIMIT_HEADER;
    iter = ITERATION_COUNT_HEADER;
    std::string header_str = "Test #," + test_name + "," + query + "," + limit + "," + iter + ",Average Time (ms),Max Time (ms),Min Time (ms),Median Time (ms)\n";

    ofs << header_str;
}

// Output test case to output file
void outputTestCase(std::ofstream& ofs, TestCase& test_case) {
    // col 1 = test #
    ofs << test_case.test_case_num << ",";

    // col 2 = test name
    if (hasCommaOrNewLine(test_case.test_name)) {
        ofs << "\"" << test_case.test_name << "\"" << ",";
    } else {
        ofs << test_case.test_name << ",";
    }

    // col 3 = query
    if (hasCommaOrNewLine(test_case.query)) {
        ofs << "\"" << test_case.query << "\""
            << ",";
    } else {
        ofs << test_case.query << ",";
    }

    // col 4 - 9
    ofs << test_case.limit << ",";
    ofs << test_case.num_iterations << ",";
    ofs << test_case.stat_info.avg << ",";
    //ofs << test_case.stat_info.stdev << ",";
    ofs << test_case.stat_info.max << ",";
    ofs << test_case.stat_info.min << ",";
    ofs << test_case.stat_info.median << "\n";
}

// Record times_ms for exec->bind->fetch
auto RecordBindingFetching = [](SQLHSTMT& hstmt, TestCase& test_case) {
    SQLSMALLINT total_columns = 0;
    int row_count = 0;
    std::string temp_str = test_case.query + "\nLIMIT " + std::to_string(test_case.limit);
    test_string query = CREATE_STRING(temp_str);

    for (size_t iter = 0; iter < test_case.num_iterations; iter++) {
        row_count = 0;
        // Execute query
        auto start = std::chrono::steady_clock::now();
        SQLRETURN ret =
            SQLExecDirect(hstmt, AS_SQLTCHAR(query.c_str()), SQL_NTS);
        ASSERT_TRUE(SQL_SUCCEEDED(ret));

        // Get column count
        SQLNumResultCols(hstmt, &total_columns);
        std::vector< std::vector< Col > > cols(total_columns);
        // std::cout << "Total columns: " << total_columns << std::endl;
        for (size_t i = 0; i < cols.size(); i++)
            ret = SQLBindCol(hstmt, static_cast< SQLUSMALLINT >(i + 1),
                             SQL_C_CHAR,
                             static_cast< SQLPOINTER >(&cols[i][0].data_dat[i]),
                             BIND_SIZE, &cols[i][0].data_len);

        while (SQLFetch(hstmt) == SQL_SUCCESS)
            row_count++;
        auto end = std::chrono::steady_clock::now();
        std::cout << "Total rows: " << row_count << std::endl;
        ASSERT_TRUE(SQL_SUCCEEDED(SQLCloseCursor(hstmt)));
        test_case.time_ms.push_back(
            std::chrono::duration_cast< std::chrono::milliseconds >(end - start)
                .count());
        LogAnyDiagnostics(SQL_HANDLE_STMT, hstmt, ret);
    }
};

// ReportTime
void ReportTime(const TestCase& test_case) {
    // Output results
    std::cout << sync_start << std::endl;
    std::cout << sync_query;
    std::cout << tchar_to_string(AS_SQLTCHAR(test_case.query.c_str()))
              << std::endl;
    std::cout << sync_case << test_case.test_case_num << ": " << test_case.test_name
              << std::endl;
    std::cout << sync_min << test_case.stat_info.min << " ms" << std::endl;
    std::cout << sync_max << test_case.stat_info.max << " ms" << std::endl;
    std::cout << sync_mean << test_case.stat_info.avg << " ms" << std::endl;
    // std::cout << sync_stdev << test_case.stat_info.stdev << " ms" <<
    // std::endl;
    std::cout << sync_median << test_case.stat_info.median << " ms"
              << std::endl;
    std::cout << sync_end << std::endl;

    std::cout << "Time dump: ";
    for (size_t i = 0; i < test_case.time_ms.size(); i++) {
        std::cout << test_case.time_ms[i] << " ms";
        if (i != (test_case.time_ms.size() - 1))
            std::cout << ", ";
    }
    std::cout << std::endl;
}

/******************************************
 * Google Test
 *****************************************/

class TestPerformance : public Fixture {
   public:
    void SetUp() override {
        if (std::getenv("NOT_CONNECTED")) {
            // GTEST_SKIP();
        }
        m_env = SQL_NULL_HENV;
        m_conn = SQL_NULL_HDBC;
        m_hstmt = SQL_NULL_HSTMT;

        ASSERT_NO_THROW(AllocStatement(AS_SQLTCHAR(dsn_conn_string().c_str()),
                                      &m_env, &m_conn, &m_hstmt, true, true));
    }
};

// Run performance test plan
TEST_F(TestPerformance, PERFORMANCE_TEST_PLAN) {
    // read test plan to string
    std::string csv_data = readPerformanceTestPlan();

    // Let "cell_refs" be a vector of columns.
    // After parsing, each element will contain std::string_view referencing a
    // part of the original data. Note: CellReference must NOT outlive csv_data.
    // If it has to, use CellValue class instead.
    std::vector< std::vector< Csv::CellReference > > cell_refs;
    CSVHeaders csv_headers;
    std::size_t num_test_cases;
    bool skip_test = true;
    TestCase test_case;
    std::vector< TestCase > results;
    std::string query;
    std::string test_name;
    int iteration_count;
    int limit;
    std::ofstream ofs;
    std::size_t comma_pos;

    try {
        Csv::Parser parser;

        // parse data into cell_refs and check headers
        parser.parseTo(csv_data, cell_refs);
        extractCSVHeaders(csv_headers, cell_refs);

        // Setup output file
        ofs.open(output_file);
        if (!ofs.is_open()) {
            throw std::runtime_error("Failed to open output file "
                                     + output_file);
        }
        outputHeaders(ofs);

        // iterate through each test case and output results
        num_test_cases = cell_refs[csv_headers.idx_query].size();
        for (std::size_t row = 1; row < num_test_cases; ++row) {
            try {
                // check skip_test field
                const auto& cell_skip_test =
                    cell_refs[csv_headers.idx_skip_test][row];
                skip_test = skipTest(cell_skip_test);
                if (skip_test) {
                    continue;  // skip test case
                }

                const auto& cell_query = cell_refs[csv_headers.idx_query][row];
                const auto& cell_limit = cell_refs[csv_headers.idx_limit][row];
                const auto& cell_test_name =
                    cell_refs[csv_headers.idx_test_name][row];
                const auto& cell_iteration_count =
                    cell_refs[csv_headers.idx_iteration_count][row];

                query = getQueryString(cell_query);
                limit = getLimit(cell_limit);
                test_name = getTestName(cell_test_name);
                iteration_count = getIterationCount(cell_iteration_count);
            } catch (std::runtime_error& err) {
                throw std::runtime_error(err.what() + std::to_string(row));
            }

            // Store test case info
            test_case.test_case_num = row;
            test_case.test_name = test_name;
            test_case.query = query;
            test_case.limit = limit;
            test_case.num_iterations = iteration_count;

            // Exec -> Bind -> Fetch (record time)
            RecordBindingFetching(m_hstmt, test_case);

            // Calcualte stats for recorded time vs iterations
            calcStats(test_case);
            ReportTime(test_case);
            // Output results to csv file
            outputTestCase(ofs, test_case);
            results.push_back(test_case);

            // Reset test_case variable
            test_case.query.clear();
            test_case.test_name.clear();
            test_case.time_ms.clear();
        }
        ofs.close();
    } catch (std::exception e) {
        if (ofs.is_open()) {
            ofs.close();
        }
        throw e;
    }
}

/******************************************
 * Main
 *****************************************/
int main(int argc, char** argv) {
#ifdef WIN32
    // Enable CRT for detecting memory leaks
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#ifdef __APPLE__
    // Enable malloc logging for detecting memory leaks.
    system("export MallocStackLogging=1");
#endif
    testing::internal::CaptureStdout();
    ::testing::InitGoogleTest(&argc, argv);
    
    int failures;
    try {
        int failures = RUN_ALL_TESTS();
    } catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::string output = testing::internal::GetCapturedStdout();
    std::cout << output << std::endl;
    std::cout << (failures ? "Not all tests passed." : "All tests passed")
              << std::endl;
    WriteFileIfSpecified(argv, argv + argc, "-fout", output);

#ifdef __APPLE__
    // Disable malloc logging and report memory leaks
    system("unset MallocStackLogging");
    system("leaks performance_results > leaks_performance_results");
#endif
    return failures;
}
