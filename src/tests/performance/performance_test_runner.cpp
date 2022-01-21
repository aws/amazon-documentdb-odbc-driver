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

#include "performance_test_runner.h"

#include <chrono>
#include <cstdio>
#include <numeric>
#include <stdexcept>

// Implement PerformanceTestRunner class

// CLASS STATIC METHODS

void performance::PerformanceTestRunner::ListDriversInstalled() {
    SQLHENV env;
    SQLRETURN ret;
    SQLWCHAR driver[256];
    SQLWCHAR attr[256];
    SQLSMALLINT driver_ret;
    SQLSMALLINT attr_ret;
    SQLUSMALLINT direction;

    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

    direction = SQL_FETCH_FIRST;
    printf("List of drivers installed:\n");
    while (SQL_SUCCEEDED(ret = SQLDrivers(env, direction, driver,
                                          sizeof(driver), &driver_ret, attr,
                                          sizeof(attr), &attr_ret))) {
        direction = SQL_FETCH_NEXT;
        printf("%s - %s\n", wchar_to_string(driver).c_str(),
               wchar_to_string(attr).c_str());
        if (ret == SQL_SUCCESS_WITH_INFO)
            printf("\tdata truncation\n");
    }
    printf("\n");
}

void performance::PerformanceTestRunner::ListDataSourcesInstalled() {
    SQLHENV env;
    SQLRETURN ret;
    SQLWCHAR dsn[256];
    SQLWCHAR desc[256];
    SQLSMALLINT dsn_ret;
    SQLSMALLINT desc_ret;
    SQLUSMALLINT direction;

    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

    direction = SQL_FETCH_FIRST;
    printf("List of data sources installed:\n");
    while (SQL_SUCCEEDED(ret = SQLDataSources(env, direction, dsn, sizeof(dsn),
                                              &dsn_ret, desc, sizeof(desc),
                                              &desc_ret))) {
        direction = SQL_FETCH_NEXT;
        printf("%s - %s\n", wchar_to_string(dsn).c_str(),
               wchar_to_string(desc).c_str());
        if (ret == SQL_SUCCESS_WITH_INFO)
            printf("\tdata truncation\n");
    }
    printf("\n");
}

SQLRETURN performance::PerformanceTestRunner::testDefaultDSN() {
    SQLRETURN ret;
    SQLHENV env = SQL_NULL_HENV;
    SQLHDBC conn = SQL_NULL_HDBC;
    SQLHSTMT hstmt = SQL_NULL_HSTMT;

    SQLTCHAR out_conn_string[1024];
    SQLSMALLINT out_conn_string_length;

    test_string dsn = to_test_string("DSN=" + dsn_default);
    test_string query = to_test_string(test_query);

    // SQLAllocHandle (env) -> SQLSetEnvAttr -> SQLAllocHandle (conn) ->
    // SQLDriverConnect (conn) -> SQLAllocHandle (statement) -> SQLExecDirec
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    if (ret != SQL_SUCCESS) {
        return ret;
    }

    ret = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
    if (ret != SQL_SUCCESS) {
        return ret;
    }

    ret = SQLAllocHandle(SQL_HANDLE_DBC, env, &conn);
    if (ret != SQL_SUCCESS) {
        return ret;
    }

    ret = SQLDriverConnect(conn, NULL, (SQLTCHAR*)dsn.c_str(), SQL_NTS,
                           out_conn_string, IT_SIZEOF(out_conn_string),
                           &out_conn_string_length, SQL_DRIVER_COMPLETE);
    if (ret != SQL_SUCCESS) {
        return ret;
    }

    ret = SQLAllocHandle(SQL_HANDLE_STMT, conn, &hstmt);
    if (ret != SQL_SUCCESS) {
        return ret;
    }

    ret = SQLExecDirect(hstmt, AS_SQLTCHAR(query.c_str()), SQL_NTS);

    return ret;
}

// CLASS PRIVATE HELPER METHODS

void performance::PerformanceTestRunner::checkFileExtension(
    const std::string filename, const std::string extension) {
    std::size_t input_length = filename.length();
    std::string error_msg;

    // assume extension has length = 4 (e.g. ".xxx")
    if (input_length < 6
        || extension.compare(filename.substr(input_length - 4, 4)) != 0) {
        error_msg = "CONSTRUCTOR ERROR: " + filename + "must be a " + extension
                    + " file.";
        throw std::runtime_error(error_msg);
    }
}

void performance::PerformanceTestRunner::checkDSN(const std::string dsn) {
    SQLHENV env;
    SQLRETURN ret;
    SQLWCHAR dsn_wchar[256];
    SQLWCHAR desc[256];
    SQLSMALLINT dsn_ret;
    SQLSMALLINT desc_ret;
    SQLUSMALLINT direction;

    std::string dsn_str;
    std::string error_msg;

    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

    direction = SQL_FETCH_FIRST;
    while (SQL_SUCCEEDED(ret = SQLDataSources(env, direction, dsn_wchar,
                                              sizeof(dsn_wchar), &dsn_ret, desc,
                                              sizeof(desc), &desc_ret))) {
        dsn_str = wchar_to_string(dsn_wchar);
        if (dsn_str.compare(dsn) == 0) {
            return;
        }
        direction = SQL_FETCH_NEXT;
    }
    error_msg = "DSN ERROR: " + dsn + " is not installed.";
    throw std::runtime_error(error_msg);
}

void performance::PerformanceTestRunner::checkOutputMode(
    const int output_mode) {
    if (output_mode < 0 || output_mode > 3) {
        throw std::invalid_argument("ERROR: output_mode must be 0, 1, 2 or 3.");
    }
}

void performance::PerformanceTestRunner::checkCSVHeaders() {
    std::string header_value, error_msg;
    bool has_escaped_quotes = false;

    // set column index in CSVHeaders structure
    for (std::size_t column = 0; column < _cell_refs.size(); ++column) {
        const auto& cell = _cell_refs[column][0];
        if (cell.getType() == Csv::CellType::String) {
            header_value =
                cell.getOriginalStringView(&has_escaped_quotes).value();
            if (header_value.compare(QUERY_HEADER) == 0) {
                _headers.idx_query = static_cast< int >(column);
            } else if ((header_value.compare(LIMIT_HEADER)) == 0) {
                _headers.idx_limit = static_cast< int >(column);
            } else if ((header_value.compare(TEST_NAME_HEADER)) == 0) {
                _headers.idx_test_name = static_cast< int >(column);
            } else if ((header_value.compare(ITERATION_COUNT_HEADER)) == 0) {
                _headers.idx_iteration_count = static_cast< int >(column);
            } else if ((header_value.compare(SKIP_TEST_HEADER)) == 0) {
                _headers.idx_skip_test = static_cast< int >(column);
            }
        } else {
            error_msg =
                "INPUT FILE ERROR: header values must be of string type in "
                "input "
                "file "
                + _input_file + ".";
            throw std::runtime_error(error_msg);
        }
    }

    // Check that all headers have been found
    if (_headers.idx_query < 0 || _headers.idx_limit < 0
        || _headers.idx_test_name < 0 || _headers.idx_iteration_count < 0
        || _headers.idx_skip_test < 0) {
        error_msg = "INPUT FILE ERROR: header value(s) missing in input file "
                    + _input_file + ".";
        throw std::runtime_error(error_msg);
    }
}

void performance::PerformanceTestRunner::outputHeaders(
    std::ofstream& ofs) const {
    std::string test_name, query, limit, iter;
    test_name = TEST_NAME_HEADER;
    query = QUERY_HEADER;
    limit = LIMIT_HEADER;
    iter = ITERATION_COUNT_HEADER;
    std::string header_str;

    if (_output_mode == 0) {
        header_str =
        "Test #," + test_name + "," + query + "," + limit + "," + iter +
        ",Status,Average Time (ms),Max Time (ms),Min Time (ms),Median Time (ms)\n";
    } else if (_output_mode == 1) {
        header_str =
        "Test #," + test_name + "," + query + "," + limit + "," + iter +
        ",Status,Average Execution Time (ms),Max Execution Time (ms),Min Execution Time (ms),Median Execution Time (ms)\n";
    } else if (_output_mode == 2) {
        header_str =
        "Test #," + test_name + "," + query + "," + limit + "," + iter +
        ",Status,Average Bind and Fetch Time (ms),Max Bind and Fetch Time (ms),Min Bind and Fetch Time (ms),Median Bind and Fetch Time (ms)\n";
    } else {
        header_str =
        "Test #," + test_name + "," + query + "," + limit + "," + iter +
        ",Status,Average Total Time (ms),Max Total Time (ms),Min Total Time (ms),Median Total Time (ms)"
        "Average Execution Time (ms),Max Execution Time (ms),Min Execution Time (ms),Median Execution Time (ms),"
        "Average Bind and Fetch Time (ms),Max Bind and Fetch Time (ms),Min "
        "Bind and Fetch Time (ms),Median Bind and Fetch Time (ms)\n";
    }

    ofs << header_str;
}

bool performance::PerformanceTestRunner::skipTest(
    const Csv::CellReference& cell_skip_test) const {
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
                "incorrect \"" + skip_test_header
                + "\" field (must be TRUE or FALSE) for test case ");
        }
    } else {
        throw std::runtime_error("incorrect \"" + skip_test_header
                                 + "\" field type for test case ");
    }
    return skip_test;
}

std::string performance::PerformanceTestRunner::getQueryString(
    const Csv::CellReference& cell_query) const {
    std::string query;
    std::string query_header = QUERY_HEADER;

    // cell value for query should be String type
    if (cell_query.getType() == Csv::CellType::String) {
        query = cell_query.getCleanString().value();
    } else {
        throw std::runtime_error("incorrect \"" + query_header
                                 + "\" field type for test case ");
    }
    return query;
}

int performance::PerformanceTestRunner::getLimit(
    const Csv::CellReference& cell_limit) const {
    double limit_dbl;
    int limit_int;
    std::string limit_header = LIMIT_HEADER;

    // cell value for limit should be Double type
    if (cell_limit.getType() == Csv::CellType::Double) {
        limit_dbl = cell_limit.getDouble().value();
        limit_dbl = limit_dbl + 0.5;  // convert double to int (truncation)
        limit_int = (int)limit_dbl;
        if (limit_int < 1) {
            throw std::runtime_error("incorrect \"" + limit_header
                                     + "\" field (must be > 0) for test case ");
        }
    } else {
        throw std::runtime_error("incorrect \"" + limit_header
                                 + "\" field type for test case ");
    }
    return limit_int;
}

std::string performance::PerformanceTestRunner::getTestName(
    const Csv::CellReference& cell_test_name) const {
    std::string test_name;
    std::string test_name_header = TEST_NAME_HEADER;

    // cell value for query should be String type
    if (cell_test_name.getType() == Csv::CellType::String) {
        test_name = cell_test_name.getCleanString().value();
    } else {
        throw std::runtime_error("incorrect \"" + test_name_header
                                 + "\" field for test case ");
    }
    return test_name;
}

int performance::PerformanceTestRunner::getIterationCount(
    const Csv::CellReference& cell_iteration_count) const {
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
            throw std::runtime_error("incorrect \"" + iteration_count_header
                                     + "\" field (must be > 0) for test case ");
        }
    } else {
        throw std::runtime_error("incorrect \"" + iteration_count_header
                                 + "\" field type for test case ");
    }
    return iteration_count_int;
}

bool performance::PerformanceTestRunner::hasCommaOrNewLine(
    const std::string str) const {
    std::size_t comma_pos, newline_pos;
    comma_pos = str.find(',');
    newline_pos = str.find('\n');
    if (comma_pos != std::string::npos || newline_pos != std::string::npos) {
        return true;
    } else {
        return false;
    }
}

void performance::PerformanceTestRunner::recordExecBindFetch(
    SQLHSTMT* hstmt, TestCase& test_case) {
    // Initialize variables
    SQLRETURN ret;
    SQLSMALLINT total_columns = 0;
    SQLROWSETSIZE row_count;
    SQLROWSETSIZE rows_fetched;
    SQLUSMALLINT* row_status;
    long long time_exec_ms;
    long long time_bind_fetch_ms;

    row_status = new SQLUSMALLINT[test_case.limit];

    // Query
    std::string temp_str =
        test_case.query + " LIMIT " + std::to_string(test_case.limit);
    std::wstring_convert< std::codecvt_utf8< wchar_t > > converter;
    test_string query = converter.from_bytes(temp_str);

    // Iterate and execute query -> bind and fetch results
    for (int iter = 0; iter < test_case.num_iterations; iter++) {
        row_count = 0;

        // Execute query
        auto time_exec_start = std::chrono::steady_clock::now();
        ret = SQLExecDirect(*hstmt, AS_SQLTCHAR(query.c_str()), SQL_NTS);
        auto time_exec_end = std::chrono::steady_clock::now();

        if (ret != SQL_SUCCESS) {
            delete[] row_status;
            LogAnyDiagnostics(SQL_HANDLE_STMT, *hstmt, ret);
            test_case.status = error;
            test_case.err_msg = "SQLExecDirect failed";
            return;
        }

        // Store execution time
        time_exec_ms = std::chrono::duration_cast< std::chrono::milliseconds >(
                           time_exec_end - time_exec_start)
                           .count();
        test_case.time_exec_ms.push_back(time_exec_ms);

        // Get column count
        ret = SQLNumResultCols(*hstmt, &total_columns);

        if (ret != SQL_SUCCESS) {
            delete[] row_status;
            LogAnyDiagnostics(SQL_HANDLE_STMT, *hstmt, ret);
            test_case.status = error;
            test_case.err_msg = "SQLNumResultCols failed";
            return;
        }

        std::vector< std::vector< Col > > cols(total_columns);
        for (size_t i = 0; i < cols.size(); i++) {
            cols[i].resize(test_case.limit);
        }

        // Bind and fetch
        auto time_bind_fetch_start = std::chrono::steady_clock::now();
        for (size_t i = 0; i < cols.size(); i++) {
            ret = SQLBindCol(*hstmt, static_cast< SQLUSMALLINT >(i + 1),
                             SQL_C_CHAR,
                             static_cast< SQLPOINTER >(&cols[i][0].data_dat[i]),
                             BIND_SIZE, &cols[i][0].data_len);
            if (ret != SQL_SUCCESS) {
                delete[] row_status;
                LogAnyDiagnostics(SQL_HANDLE_STMT, *hstmt, ret);
                test_case.status = error;
                test_case.err_msg = "SQLBindCol failed";
                return;
            }
        }

        while (SQLExtendedFetch(*hstmt, SQL_FETCH_NEXT, 0, &rows_fetched,
                                row_status)
               == SQL_SUCCESS) {
            row_count++;
        }
        auto time_bind_fetch_end = std::chrono::steady_clock::now();

        // Store bind and fetch time
        time_bind_fetch_ms =
            std::chrono::duration_cast< std::chrono::milliseconds >(
                time_bind_fetch_end - time_bind_fetch_start)
                .count();
        test_case.time_bind_fetch_ms.push_back(time_bind_fetch_ms);

        test_case.time_ms.push_back(time_exec_ms + time_bind_fetch_ms);

        // ret = SQLCloseCursor(*hstmt);
        if (ret != SQL_SUCCESS) {
            delete[] row_status;
            LogAnyDiagnostics(SQL_HANDLE_STMT, *hstmt, ret);
            test_case.status = error;
            test_case.err_msg = "SQLCloseCursor failed";
            return;
        }
    }
    delete[] row_status;
    test_case.status = success;
}

void performance::PerformanceTestRunner::calcStats(TestCase& test_case) {
    size_t size = test_case.time_ms.size();
    // check if there is any time recorded data

    if (test_case.status == error) {
        return;
    }

    if (size < 1) {
        throw std::runtime_error(
            "there are no time data points to calculate performance stats. "
            "Potential errors may be incorrect iteration count or "
            "SQLExecDirect "
            "failed for test case ");
    }

    try {
        if (size == 1) {
            test_case.stat_info.max = test_case.time_ms[0];
            test_case.stat_info.min = test_case.time_ms[0];
            test_case.stat_info.avg = test_case.time_ms[0];
            test_case.stat_info.median = test_case.time_ms[0];
            test_case.stat_info_exec.max = test_case.time_exec_ms[0];
            test_case.stat_info_exec.min = test_case.time_exec_ms[0];
            test_case.stat_info_exec.avg = test_case.time_exec_ms[0];
            test_case.stat_info_exec.median = test_case.time_exec_ms[0];
            test_case.stat_info_exec.max = test_case.time_bind_fetch_ms[0];
            test_case.stat_info_exec.min = test_case.time_bind_fetch_ms[0];
            test_case.stat_info_exec.avg = test_case.time_bind_fetch_ms[0];
            test_case.stat_info_exec.median = test_case.time_bind_fetch_ms[0];

        } else {
            // calculate max and min
            test_case.stat_info.max = *std::max_element(
                test_case.time_ms.begin(), test_case.time_ms.end());
            test_case.stat_info.min = *std::min_element(
                test_case.time_ms.begin(), test_case.time_ms.end());
            test_case.stat_info_exec.max = *std::max_element(
                test_case.time_exec_ms.begin(), test_case.time_exec_ms.end());
            test_case.stat_info_exec.min = *std::min_element(
                test_case.time_exec_ms.begin(), test_case.time_exec_ms.end());
            test_case.stat_info_bind_fetch.max =
                *std::max_element(test_case.time_bind_fetch_ms.begin(),
                                  test_case.time_bind_fetch_ms.end());
            test_case.stat_info_bind_fetch.min =
                *std::min_element(test_case.time_bind_fetch_ms.begin(),
                                  test_case.time_bind_fetch_ms.end());

            // calculate average
            test_case.stat_info.avg =
                std::accumulate(std::begin(test_case.time_ms),
                                std::end(test_case.time_ms), 0ll)
                / size;

            test_case.stat_info_exec.avg =
                std::accumulate(std::begin(test_case.time_exec_ms),
                                std::end(test_case.time_exec_ms), 0ll)
                / size;

            test_case.stat_info_bind_fetch.avg =
                std::accumulate(std::begin(test_case.time_bind_fetch_ms),
                                std::end(test_case.time_bind_fetch_ms), 0ll)
                / size;

            // calculate median
            std::sort(test_case.time_ms.begin(), test_case.time_ms.end());
            std::sort(test_case.time_exec_ms.begin(),
                      test_case.time_exec_ms.end());
            std::sort(test_case.time_bind_fetch_ms.begin(),
                      test_case.time_bind_fetch_ms.end());

            test_case.stat_info.median =
                (size % 2) ? test_case.time_ms[size / 2]
                           : ((test_case.time_ms[(size / 2) - 1]
                               + test_case.time_ms[size / 2])
                              / 2);
            test_case.stat_info_exec.median =
                (size % 2) ? test_case.time_exec_ms[size / 2]
                           : ((test_case.time_exec_ms[(size / 2) - 1]
                               + test_case.time_exec_ms[size / 2])
                              / 2);
            test_case.stat_info_bind_fetch.median =
                (size % 2) ? test_case.time_bind_fetch_ms[size / 2]
                           : ((test_case.time_bind_fetch_ms[(size / 2) - 1]
                               + test_case.time_bind_fetch_ms[size / 2])
                              / 2);

            // calculate standard deviation
            // TODO: calculate stdev from vector of data
            // e.g. test_case.stat_info.stdev = stdev;
        }
    } catch (...) {
        throw std::runtime_error("could not calculate results for test case ");
    }
}

void performance::PerformanceTestRunner::reportTime(const TestCase& test_case) {
    // Output results
    std::cout << sync_start << std::endl;
    std::cout << sync_query;
    std::cout << test_case.query << " limit " << test_case.limit << std::endl;
    std::cout << sync_case << test_case.test_case_num << ": "
              << test_case.test_name << std::endl;

    if (test_case.status == error) {
        std::cout << sync_status << "Error: " << test_case.err_msg << "\n\n";
        return;
    }

    if (test_case.status == skip) {
        std::cout << sync_status << "Skip test\n\n";
        return;
    }

    std::cout << sync_status << "Success" << std::endl;

    if (_output_mode == 0 || _output_mode == 3) {
        std::cout << sync_min << test_case.stat_info.min << " ms" << std::endl;
        std::cout << sync_max << test_case.stat_info.max << " ms" << std::endl;
        std::cout << sync_mean << test_case.stat_info.avg << " ms" << std::endl;
        // std::cout << sync_stdev << test_case.stat_info.stdev << " ms" <<
        // std::endl;
        std::cout << sync_median << test_case.stat_info.median << " ms"
                  << std::endl;
        std::cout << sync_end << std::endl;

        std::cout << "SQLExecDirect->SQLBindCol->SQLFetch Time dump: ";
        for (size_t i = 0; i < test_case.time_ms.size(); i++) {
            std::cout << test_case.time_ms[i] << " ms";
            if (i != (test_case.time_ms.size() - 1))
                std::cout << ", ";
        }
        std::cout << "\n\n";

    } else if (_output_mode == 1) {
        std::cout << sync_min << test_case.stat_info_exec.min << " ms"
                  << std::endl;
        std::cout << sync_max << test_case.stat_info_exec.max << " ms"
                  << std::endl;
        std::cout << sync_mean << test_case.stat_info_exec.avg << " ms"
                  << std::endl;
        // std::cout << sync_stdev << test_case.stat_info_exec.stdev << " ms" <<
        // std::endl;
        std::cout << sync_median << test_case.stat_info_exec.median << " ms"
                  << std::endl;
        std::cout << sync_end << std::endl;

        std::cout << "SQLExecDirect time dump: ";
        for (size_t i = 0; i < test_case.time_exec_ms.size(); i++) {
            std::cout << test_case.time_exec_ms[i] << " ms";
            if (i != (test_case.time_exec_ms.size() - 1))
                std::cout << ", ";
        }
        std::cout << "\n\n";

    } else if (_output_mode == 2) {
        std::cout << sync_min << test_case.stat_info_bind_fetch.min << " ms"
                  << std::endl;
        std::cout << sync_max << test_case.stat_info_bind_fetch.max << " ms"
                  << std::endl;
        std::cout << sync_mean << test_case.stat_info_bind_fetch.avg << " ms"
                  << std::endl;
        // std::cout << sync_stdev << test_case.stat_info_bind_fetch.stdev << "
        // ms"
        // << std::endl;
        std::cout << sync_median << test_case.stat_info_bind_fetch.median
                  << " ms" << std::endl;
        std::cout << sync_end << std::endl;

        std::cout << "SQLBindCol and SQLFetch time dump: ";
        for (size_t i = 0; i < test_case.time_ms.size(); i++) {
            std::cout << test_case.time_ms[i] << " ms";
            if (i != (test_case.time_ms.size() - 1))
                std::cout << ", ";
        }
        std::cout << "\n\n";
    }
}

void performance::PerformanceTestRunner::outputTestCase(
    std::ofstream& ofs, const TestCase& test_case) const {
    // col 1 = test #
    ofs << test_case.test_case_num << ",";

    // col 2 = test name
    if (hasCommaOrNewLine(test_case.test_name)) {
        ofs << "\"" << test_case.test_name << "\""
            << ",";
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

    // col 4 = limit
    ofs << test_case.limit << ",";

    // col 5 = iterations
    ofs << test_case.num_iterations << ",";

    // col 6 = status
    if (test_case.status == error) {
        ofs << "ERROR: " << test_case.err_msg << ",";
    } else if (test_case.status == skip) {
        ofs << "SKIP TEST,";
    } else {
        ofs << "SUCCESS,";
    }

    // col 7+ = results
    if (_output_mode == 0) {
        if (test_case.status != success) {
            ofs << ",,,\n";
            return;
        }
        ofs << test_case.stat_info.avg << ",";
        // ofs << test_case.stat_info.stdev << ",";
        ofs << test_case.stat_info.max << ",";
        ofs << test_case.stat_info.min << ",";
        ofs << test_case.stat_info.median << "\n";
    } else if (_output_mode == 1) {
        if (test_case.status != success) {
            ofs << ",,,\n";
            return;
        }
        ofs << test_case.stat_info_exec.avg << ",";
        // ofs << test_case.stat_info_exec.stdev << ",";
        ofs << test_case.stat_info_exec.max << ",";
        ofs << test_case.stat_info_exec.min << ",";
        ofs << test_case.stat_info_exec.median << "\n";
    } else if (_output_mode == 2) {
        if (test_case.status != success) {
            ofs << ",,,\n";
            return;
        }
        ofs << test_case.stat_info_bind_fetch.avg << ",";
        // ofs << test_case.stat_info_bind_fetch.stdev << ",";
        ofs << test_case.stat_info_bind_fetch.max << ",";
        ofs << test_case.stat_info_bind_fetch.min << ",";
        ofs << test_case.stat_info_bind_fetch.median << "\n";
    } else if (_output_mode == 3) {
        if (test_case.status != success) {
            ofs << ",,,,,,,,,,,\n";
            return;
        }
        ofs << test_case.stat_info.avg << ",";
        // ofs << test_case.stat_info.stdev << ",";
        ofs << test_case.stat_info.max << ",";
        ofs << test_case.stat_info.min << ",";
        ofs << test_case.stat_info.median << ",";

        ofs << test_case.stat_info_exec.avg << ",";
        // ofs << test_case.stat_info_exec.stdev << ",";
        ofs << test_case.stat_info_exec.max << ",";
        ofs << test_case.stat_info_exec.min << ",";
        ofs << test_case.stat_info_exec.median << ",";

        ofs << test_case.stat_info_bind_fetch.avg << ",";
        // ofs << test_case.stat_info_bind_fetch.stdev << ",";
        ofs << test_case.stat_info_bind_fetch.max << ",";
        ofs << test_case.stat_info_bind_fetch.min << ",";
        ofs << test_case.stat_info_bind_fetch.median << "\n";
    }
}

// CLASS CONSTRUCTORS AND DESTRUCTORS

performance::PerformanceTestRunner::PerformanceTestRunner() {
    // check if DSN is installed
    try {
        checkDSN(dsn_default);
    } catch (...) {
        throw;
    }
}

performance::PerformanceTestRunner::PerformanceTestRunner(
    const std::string test_plan_csv, const std::string output_file_csv,
    const std::string dsn, const int output_mode) {
    // check input arguments
    try {
        checkFileExtension(test_plan_csv, ".csv");
        checkFileExtension(output_file_csv, ".csv");
        checkDSN(dsn);
        checkOutputMode(output_mode);
    } catch (...) {
        throw;
    }

    _input_file = test_plan_csv;
    _output_file = output_file_csv;
    _dsn = dsn;
    _output_mode = output_mode;
}

performance::PerformanceTestRunner::~PerformanceTestRunner() {
    if (SQL_NULL_HSTMT != _hstmt) {
        CloseCursor(&_hstmt, true, true);
        SQLFreeHandle(SQL_HANDLE_STMT, _hstmt);
    }
    if (SQL_NULL_HDBC != _conn) {
        SQLDisconnect(_conn);
        SQLFreeHandle(SQL_HANDLE_DBC, _conn);
    }
    if (SQL_NULL_HENV != _env) {
        SQLFreeHandle(SQL_HANDLE_ENV, _env);
    }
}

// CLASS SETTERS AND GETTERS

void performance::PerformanceTestRunner::setDSN(const std::string dsn) {
    try {
        checkDSN(dsn_default);
    } catch (...) {
        throw;
    }
    _dsn = dsn;
}

void performance::PerformanceTestRunner::setOutputMode(const int output_mode) {
    try {
        checkOutputMode(output_mode);
    } catch (...) {
        throw;
    }
    _output_mode = output_mode;
}

// CLASS PUBLIC METHODS

void performance::PerformanceTestRunner::readPerformanceTestPlan() {
    // Read input csv file to string
    std::ifstream ifs(_input_file, std::ios::binary);
    Csv::Parser parser;
    std::string error_msg;

    if (!ifs.is_open()) {
        error_msg =
            "INPUT FILE ERROR: failed to open input file " + _input_file + ".";
        throw std::runtime_error(error_msg);
    }

    std::string csv_data((std::istreambuf_iterator< char >(ifs)),
                         (std::istreambuf_iterator< char >()));

    _csv_data = csv_data;  // store csv_data

    ifs.close();  // close csv file
    if (ifs.is_open()) {
        error_msg =
            "INPUT FILE ERROR: failed to close input file " + _input_file + ".";
        throw std::runtime_error(error_msg);
    }

    // parse data into _cell_refs and check headers
    parser.parseTo(_csv_data, _cell_refs);

    try {
        checkCSVHeaders();
    } catch (...) {
        throw;
    }
}

void performance::PerformanceTestRunner::setupConnection() {
    SQLTCHAR out_conn_string[1024];
    SQLSMALLINT out_conn_string_length;
    SQLRETURN ret;
    test_string dsn = to_test_string("DSN=" + _dsn);
    std::string error_msg;

    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_env);
    if (ret == SQL_INVALID_HANDLE || ret == SQL_ERROR) {
        error_msg =
            "SQLAllocHandle ERROR: failed to allocate environment handle.";
        throw std::runtime_error(error_msg);
    }

    ret = SQLSetEnvAttr(_env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
    if (ret == SQL_INVALID_HANDLE || ret == SQL_ERROR) {
        error_msg = "SQLSetEnvAttr ERROR: failed to set environment attribute.";
        throw std::runtime_error(error_msg);
    }

    ret = SQLAllocHandle(SQL_HANDLE_DBC, _env, &_conn);
    if (ret == SQL_INVALID_HANDLE || ret == SQL_ERROR) {
        error_msg =
            "SQLAllocHandle ERROR: failed to allocate connection handle.";
        throw std::runtime_error(error_msg);
    }

    ret = SQLDriverConnect(_conn, NULL, (SQLTCHAR*)dsn.c_str(), SQL_NTS,
                           out_conn_string, IT_SIZEOF(out_conn_string),
                           &out_conn_string_length, SQL_DRIVER_COMPLETE);
    if (ret == SQL_INVALID_HANDLE || ret == SQL_ERROR) {
        error_msg =
            "SQLDriverConnect ERROR: failed to connect to DSN = " + _dsn + ".";
        throw std::runtime_error(error_msg);
    } else if (ret == SQL_NO_DATA) {
        error_msg = "SQLDriverConnect ERROR: no data in DSN = " + _dsn + ".";
        throw std::runtime_error(error_msg);
    } else if (ret == SQL_STILL_EXECUTING) {
        error_msg = "SQLDriverConnect ERROR: still trying to connect to DSN = "
                    + _dsn + ".";
        throw std::runtime_error(error_msg);
    }

    ret = SQLAllocHandle(SQL_HANDLE_STMT, _conn, &_hstmt);
    if (ret == SQL_INVALID_HANDLE || ret == SQL_ERROR) {
        error_msg =
            "SQLAllocHandle ERROR: failed to allocate connection statement "
            "handle.";
        throw std::runtime_error(error_msg);
    }
}

void performance::PerformanceTestRunner::runPerformanceTestPlan() {
    std::size_t num_test_cases;
    bool skip_test = true;
    TestCase test_case;
    std::string query;
    std::string test_name;
    int iteration_count;
    int limit;
    std::string error_msg;

    // setup output file
    std::ofstream ofs;
    ofs.open(_output_file);
    if (!ofs.is_open()) {
        error_msg = "OUTPUT FILE ERROR: failed to open output file "
                    + _output_file + ".";
        throw std::runtime_error(error_msg);
    }
    outputHeaders(ofs);

    // iterate through each test case and output results
    num_test_cases = _cell_refs[_headers.idx_query].size();
    for (std::size_t row = 1; row < num_test_cases; ++row) {
        try {
            // get test case fields
            const auto& cell_query = _cell_refs[_headers.idx_query][row];
            const auto& cell_limit = _cell_refs[_headers.idx_limit][row];
            const auto& cell_test_name =
                _cell_refs[_headers.idx_test_name][row];
            const auto& cell_iteration_count =
                _cell_refs[_headers.idx_iteration_count][row];

            query = getQueryString(cell_query);
            limit = getLimit(cell_limit);
            test_name = getTestName(cell_test_name);
            iteration_count = getIterationCount(cell_iteration_count);

            // Store test case info
            test_case.test_case_num = static_cast< int >(row);
            test_case.test_name = test_name;
            test_case.query = query;
            test_case.limit = limit;
            test_case.num_iterations = iteration_count;

            // check skip_test field
            const auto& cell_skip_test =
                _cell_refs[_headers.idx_skip_test][row];
            skip_test = skipTest(cell_skip_test);
            if (skip_test) {
                test_case.status = skip;
                reportTime(test_case);  // reports that test was skipped
                outputTestCase(ofs, test_case);
                _results.push_back(test_case);
                continue;  // skip test case
            }

            // Exec -> Bind -> Fetch (record time)
            recordExecBindFetch(&_hstmt, test_case);

            // Calcualte stats for recorded time
            calcStats(test_case);
            reportTime(test_case);

            // Output results to csv file
            outputTestCase(ofs, test_case);
            _results.push_back(test_case);  // store test case data

            // Reset test_case variable
            test_case.query.clear();
            test_case.test_name.clear();
            test_case.time_ms.clear();

        } catch (std::runtime_error& err) {
            error_msg = "TESTCASE ERROR: " + (std::string)err.what()
                        + std::to_string(row) + ".";
            ofs.close();
            throw std::runtime_error(error_msg);
        }
    }
    ofs.close();
}