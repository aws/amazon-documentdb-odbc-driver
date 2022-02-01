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

#ifndef PERFORMANCE_TEST_RUNNER_H
#define PERFORMANCE_TEST_RUNNER_H

#include "csv_parser.h"
#include "it_odbc_helper.h"

#ifndef WIN32
typedef SQLULEN SQLROWCOUNT;
typedef SQLULEN SQLROWSETSIZE;
typedef SQLULEN SQLTRANSID;
typedef SQLLEN SQLROWOFFSET;
#endif

// GLOBAL CONSTANTS

#define BIND_SIZE 255

// CSV header definitions (input csv file must match)
#define QUERY_HEADER "query"
#define LIMIT_HEADER "limit"
#define TEST_NAME_HEADER "test_name"
#define ITERATION_COUNT_HEADER "loop_count"
#define SKIP_TEST_HEADER "skip_test"

// Ensure DSN is setup on machine before running test
// DSN name should be "documentdb-perf-test"
const std::string input_file =
    "..\\..\\cmake\\tests\\performance\\Performance_Test_Plan.csv";
const std::string output_file =
    "..\\..\\cmake\\tests\\performance\\Performance_Test_Results."
    "csv";
const std::string dsn_default = "documentdb-perf-test";
const std::string test_query =
    "SELECT * from performance.employer_employees LIMIT 10000";

// Constants used for reportTime function
const std::string sync_start = "%%__PARSE__SYNC__START__%% ";
const std::string sync_query = "%%__QUERY__%% ";
const std::string sync_case = "%%__CASE__%% ";
const std::string sync_status = "%%__STATUS__%% ";
const std::string sync_min = "%%__MIN__%% ";
const std::string sync_max = "%%__MAX__%% ";
const std::string sync_mean = "%%__MEAN__%% ";
const std::string sync_stdev = "%%__STDEV__%% ";
const std::string sync_median = "%%__MEDIAN__%% ";
const std::string sync_end = "%%__PARSE__SYNC__END__%% ";

// DATA STRUCTURES

enum testCaseStatus { success, error, skip };

typedef struct Col {
    SQLLEN data_len;
    SQLCHAR data_dat[BIND_SIZE];
} Col;

struct CsvHeaders {
    int idx_query = -1;
    int idx_limit = -1;
    int idx_skip_test = -1;
    int idx_test_name = -1;
    int idx_iteration_count = -1;
};

struct StatisticalInfo {
    long long avg = 0;
    long long min = 0;
    long long max = 0;
    long long median = 0;
    long long stdev = 0;
};

struct TestCase {
    testCaseStatus status; // PASS/SKIP/ERROR
    std::string err_msg = "";
    int test_case_num = 0;
    std::string test_name = "";
    std::string query = "";
    int limit = 0;
    int num_iterations = 0;
    std::vector< long long > time_ms;
    std::vector< long long > time_exec_ms;
    std::vector< long long > time_bind_fetch_ms;
    StatisticalInfo stat_info;
    StatisticalInfo stat_info_exec;
    StatisticalInfo stat_info_bind_fetch;
};

// CLASS DEFINITION
// Class performance::PerformanceTestRunner

namespace performance {

class PerformanceTestRunner {
   private:
    // DATA MEMBERS

    SQLHENV _env = SQL_NULL_HENV;
    SQLHDBC _conn = SQL_NULL_HDBC;
    SQLHSTMT _hstmt = SQL_NULL_HSTMT;
    std::string _dsn = dsn_default;

    std::vector< std::vector< Csv::CellReference > >
        _cell_refs;                          // references _csv_data
    std::string _input_file = input_file;    // input test plan csv file
    std::string _output_file = output_file;  // output test results csv file
    std::string _csv_data = "";
    CsvHeaders _headers;  // col index of headers
    std::vector< TestCase > _results;

    // _output_mode = 0 - output time for exec/bind/fetch
    // _output_mode = 1 - output time for exec only
    // _output_mode = 2 - output time for bind and fetch only
    // _output_mode = 3 - output all above
    int _output_mode = 0;

    // HELPER FUNCTIONS

    // return true if filename has extension = ".xxx"
    void checkFileExtension(const std::string filename,
                            const std::string extension);

    // return true if DSN matches any data sources installed
    void checkDSN(const std::string dsn);

    // return true if output mode = 0, 1, 2 or 3
    void checkOutputMode(const int output_mode);

    // get csv input file header col index
    void checkCSVHeaders();

    // Output headers to output file
    // col 1 = test #
    // col 2 = test name
    // col 3 = status
    // col 4 = query
    // col 5 = limit
    // col 6 = iteration count
    // col 7 = avg time
    // col 8 = max time
    // col 9 = min time
    // col 10 = median time
    void outputHeaders(std::ofstream& ofs) const;

    // Returns true if test should be skipped, otherwise false
    bool skipTest(const Csv::CellReference& cell_skip_test) const;

    // Returns query string from current test case csv data
    std::string getQueryString(const Csv::CellReference& cell_query) const;

    // Return limit from current test case csv data
    int getLimit(const Csv::CellReference& cell_limit) const;

    // Returns test name string from current test case csv data
    std::string getTestName(const Csv::CellReference& cell_test_name) const;

    // Returns iteration/loop count from current test case csv data
    int getIterationCount(const Csv::CellReference& cell_iteration_count) const;

    // Returns true if string has comma or newline
    bool hasCommaOrNewLine(const std::string str) const;

    // Calculate statistical info from current test case
    void calcStats(TestCase& test_case);

    // Output test case to output file
    void outputTestCase(std::ofstream& ofs, const TestCase& test_case) const;

    // Record time_ms for current test case: exec->bind->fetch
    void recordExecBindFetch(SQLHSTMT* hstmt, TestCase& test_case);

    // ReportTime for current test case
    void reportTime(const TestCase& test_case);

   public:
    // STATIC METHODS

    static void ListDriversInstalled();
    static void ListDataSourcesInstalled();

    // Test if connection to test database DSN=documentdb-perf-test can be
    // established and if query "SELECT * from performance.employer_employees
    // LIMIT 10000" can be executed
    static SQLRETURN testDefaultDSN();

    // CONSTRUCTORS AND DESTRUCTOR

    // Default constructor
    PerformanceTestRunner();

    // Specify input (test plan) csv file, output (results) csv file and dsn
    // connection string to test database
    PerformanceTestRunner(const std::string test_plan_csv,
                          const std::string output_file_csv,
                          const std::string dsn, const int output_mode = 0);

    // Destructor
    virtual ~PerformanceTestRunner();

    // SETTERS AND GETTERS

    // get input test plan csv file
    inline std::string getInputTestPlanCsvFile() const {
        return _input_file;
    }

    // set input test plan csv file
    inline void setInputTestPlanCsvFile(const std::string filename) {
        _input_file = filename;
    }

    // get output results csv file
    inline std::string getOutputResultsCsvFile() const {
        return _output_file;
    }

    // set output results csv file
    inline void setOutputResultsCsvFile(const std::string filename) {
        _output_file = filename;
    }

    // get data source name
    inline std::string getDSN() const {
        return _dsn;
    }

    // set data source name
    void setDSN(const std::string dsn);

    // get results
    inline std::vector< TestCase > getTestResults() const {
        return _results;
    }

    // get output mode
    inline int getOutputMode() const {
        return _output_mode;
    }

    // set output mode (must = 0, 1, 2 or 3)
    void setOutputMode(const int output_mode);

    // METHODS TO RUN PEFORMANCE TEST PLAN

    // read input test plan csv file
    // assumptions:
    //  1. csv file contains following headers (order does not matter):
    //     "query" : string (can contain new lines and commas)
    //     "limit" : integer (limit number of rows returned in query)
    //     "test_name" : string (can contain new lines and commas)
    //     "loop_count" : integer (number of times to execute query)
    //     "skip_test" : string (must be "TRUE" or "FALSE")
    void readPerformanceTestPlan();

    // allocate env, conn and stmt
    void setupConnection();

    // run performance test plan and record results to output file
    void runPerformanceTestPlan();
};

}  // namespace performance

#endif  // PERFORMANCE_TEST_RUNNER_H