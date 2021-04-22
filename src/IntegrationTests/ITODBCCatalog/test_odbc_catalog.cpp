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
// #define NOMINMAX 1
#include "pch.h"
#include "unit_test_helper.h"
#include "it_odbc_helper.h"
// clang-format on

// General test constants and structures
#define BIND_SIZE 512
typedef struct bind_info {
    SQLUSMALLINT ordinal;
    SQLSMALLINT target_type;
    SQLPOINTER target;
    SQLLEN buffer_len;
    SQLLEN out_len;
    bind_info(SQLUSMALLINT _ordinal, SQLSMALLINT _target_type) {
        ordinal = _ordinal;
        target_type = _target_type;
        out_len = 0;
        data.resize(BIND_SIZE, '\0');
        buffer_len = data.size();
        target = data.data();
    }
    std::string AsString() {
        if (out_len == SQL_NULL_DATA) {
            return "";
        }
        switch (target_type) {
            case SQL_C_CHAR:
                return reinterpret_cast< char* >(data.data());
                break;
            case SQL_C_LONG:
                return std::to_string(
                    *reinterpret_cast< unsigned long* >(data.data()));
                break;
            case SQL_C_SLONG:
                return std::to_string(
                    *reinterpret_cast< signed long* >(data.data()));
                break;
            case SQL_C_SHORT:
                return std::to_string(
                    *reinterpret_cast< signed short* >(data.data()));
                break;
            case SQL_C_SSHORT:
                return std::to_string(
                    *reinterpret_cast< unsigned short* >(data.data()));
                break;
            default:
                return "Unknown conversion type (" + std::to_string(target_type)
                       + ")";
                break;
        }
    }

   private:
    std::vector< SQLCHAR > data;
} bind_info;
//
//// Column test constants and macro
//const std::vector< std::string > flights_column_name = {
//    "FlightNum",       "Origin",         "FlightDelay",
//    "DistanceMiles",   "FlightTimeMin",  "OriginWeather",
//    "dayOfWeek",       "AvgTicketPrice", "Carrier",
//    "FlightDelayMin",  "OriginRegion",   "DestAirportID",
//    "FlightDelayType", "timestamp",      "Dest",
//    "FlightTimeHour",  "Cancelled",      "DistanceKilometers",
//    "OriginCityName",  "DestWeather",    "OriginCountry",
//    "DestCountry",     "DestRegion",     "DestCityName",
//    "OriginAirportID"};
//const std::vector< std::string > flights_data_type = {
//    "keyword", "keyword", "boolean", "float",   "float",   "keyword", "integer",
//    "float",   "keyword", "integer", "keyword", "keyword", "keyword", "date",
//    "keyword", "keyword", "boolean", "float",   "keyword", "keyword", "keyword",
//    "keyword", "keyword", "keyword", "keyword"};
//const std::vector< short > flights_sql_data_type = {
//    SQL_WVARCHAR, SQL_WVARCHAR, SQL_BIT,      SQL_REAL,           SQL_REAL,
//    SQL_WVARCHAR, SQL_INTEGER,  SQL_REAL,     SQL_WVARCHAR,       SQL_INTEGER,
//    SQL_WVARCHAR, SQL_WVARCHAR, SQL_WVARCHAR, SQL_TYPE_TIMESTAMP, SQL_WVARCHAR,
//    SQL_WVARCHAR, SQL_BIT,      SQL_REAL,     SQL_WVARCHAR,       SQL_WVARCHAR,
//    SQL_WVARCHAR, SQL_WVARCHAR, SQL_WVARCHAR, SQL_WVARCHAR,       SQL_WVARCHAR};
//const std::string flights_catalog_odfe = "odfe-cluster";
//const std::string flights_catalog_elas = "elasticsearch";
//const std::string flights_table_name = "kibana_sample_data_flights";
//const std::string flights_decimal_digits = "10";
//const std::string flights_num_prec_radix = "2";
//
//// Table test constants and macro
//typedef struct table_data {
//    std::string catalog_name;
//    std::string schema_name;
//    std::string table_name;
//    std::string table_type;
//    std::string remarks;
//} table_data;
//
//const std::vector< table_data > table_data_filtered{
//    {"", "", "kibana_sample_data_ecommerce", "BASE TABLE", ""},
//    {"", "", "kibana_sample_data_flights", "BASE TABLE", ""},
//    {"", "", "kibana_sample_data_types", "BASE TABLE", ""}};
//const std::vector< table_data > table_data_single{
//    {"", "", "kibana_sample_data_flights", "BASE TABLE", ""}};
//const std::vector< table_data > table_data_all{
//    {"", "", "kibana_sample_data_ecommerce", "BASE TABLE", ""},
//    {"", "", "kibana_sample_data_flights", "BASE TABLE", ""},
//    {"", "", "kibana_sample_data_types", "BASE TABLE", ""},
//};
//const std::vector< table_data > excel_table_data_all{
//    {"", "", "kibana_sample_data_ecommerce", "TABLE", ""},
//    {"", "", "kibana_sample_data_flights", "TABLE", ""},
//    {"", "", "kibana_sample_data_types", "TABLE", ""},
//};
//const std::vector< table_data > table_data_types{
//    {"", "", "", "BASE TABLE", ""}};

class TestSQLTables : public Fixture {};

class TestSQLColumns : public Fixture {};

class TestSQLGetTypeInfo : public Fixture {};
//
//class TestSQLCatalogKeys : public testing::Test {
//   public:
//    TestSQLCatalogKeys() {
//    }
//    void SetUp() {
//        AllocStatement((SQLTCHAR*)conn_string.c_str(), &m_env, &m_conn,
//                       &m_hstmt, true, true);
//    }
//    void TearDown() {
//        SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);
//        SQLDisconnect(m_conn);
//        SQLFreeHandle(SQL_HANDLE_ENV, m_env);
//    }
//    ~TestSQLCatalogKeys() {
//        // cleanup any pending stuff, but no exceptions allowed
//    }
//
//    SQLHENV m_env = SQL_NULL_HENV;
//    SQLHDBC m_conn = SQL_NULL_HDBC;
//    SQLHSTMT m_hstmt = SQL_NULL_HSTMT;
//};

//#define TEST_SQL_KEYS(test_name, test_function, ...)                     \
//    TEST_F(TestSQLCatalogKeys, test_name) {                              \
//        EXPECT_TRUE(SQL_SUCCEEDED(test_function(m_hstmt, __VA_ARGS__))); \
//        size_t result_count = 0;                                         \
//        SQLRETURN ret;                                                   \
//        while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS)                 \
//            result_count++;                                              \
//        EXPECT_EQ(ret, SQL_NO_DATA);                                     \
//        EXPECT_EQ(result_count, static_cast< size_t >(0));               \
//    }

/**
 * SQLTables Tests
 * This integration required manual setup.
 * Setup
 * <Database>.<Table> in Amazon Timestream
 * ODBCTest.DevOps
 * ODBCTest.IoT
 * promDB.promTB
 */
auto CheckRows = [](const std::vector< std::vector< std::string > >& expected, 
    const std::vector< std::vector< std::string > >& result) {
    EXPECT_EQ(expected.size(), result.size());
    if (expected.size() > 0) {
        EXPECT_EQ(expected[0].size(), result[0].size());
        for (int i = 0; i < (int)expected.size(); i++) {
            for (int j = 0; j < (int)expected[i].size(); j++) {
                EXPECT_EQ(expected[i][j], result[i][j]);
            }
        }
    }
};

TEST_F(TestSQLTables, TEST_ALL_NULL) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLTables(m_hstmt, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::vector< bind_info > binds;
    for (int i = 1; i <= (int)column_count; i++) {
        binds.push_back(bind_info((SQLUSMALLINT)i, SQL_C_CHAR));
    }
    for (auto& it : binds) {
        ret = SQLBindCol(m_hstmt, it.ordinal, it.target_type, it.target,
                         it.buffer_len, &it.out_len);
        EXPECT_TRUE(SQL_SUCCEEDED(ret));
    }
    std::vector< std::vector< std::string > > result;
    while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS) {
        std::vector< std::string > row;
        for (int i = 0; i < (int)column_count; i++) {
            row.push_back(binds[i].AsString());
        }
        result.push_back(row);
    }
    std::vector< std::vector< std::string > > expected{
        {"ODBCTest", "", "DevOps", "TABLE", ""},
        {"ODBCTest", "", "IoT", "TABLE", ""},
        {"promDB", "", "promTB", "TABLE", ""}};
    CheckRows(expected, result);
}

TEST_F(TestSQLTables, TEST_ALL_DATABASES_EXCEL) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLTables(m_hstmt, (SQLWCHAR *)L"%", 1, (SQLWCHAR *)L"", 0, (SQLWCHAR *)L"", 0, (SQLWCHAR *)L"", 0);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::vector< bind_info > binds;
    for (int i = 1; i <= (int)column_count; i++) {
        binds.push_back(bind_info((SQLUSMALLINT)i, SQL_C_CHAR));
    }
    for (auto& it : binds) {
        ret = SQLBindCol(m_hstmt, it.ordinal, it.target_type, it.target, it.buffer_len, &it.out_len);
        EXPECT_TRUE(SQL_SUCCEEDED(ret));
    }
    std::vector< std::vector< std::string > > result;
    while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS) {
        std::vector< std::string > row;
        for (int i = 0; i < (int)column_count; i++) {
            row.push_back(binds[i].AsString());
        }
        result.push_back(row);
    }
    std::vector< std::vector< std::string > > expected{
        {"ODBCTest", "", "", "", ""},
        {"promDB", "", "", "", ""}};
    CheckRows(expected, result);
}

TEST_F(TestSQLTables, TEST_ALL_DATABASES_NULL) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLTables(m_hstmt, (SQLWCHAR *)L"%", SQL_NTS, nullptr, 0, nullptr, 0, nullptr, 0);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::vector< bind_info > binds;
    for (int i = 1; i <= (int)column_count; i++) {
        binds.push_back(bind_info((SQLUSMALLINT)i, SQL_C_CHAR));
    }
    for (auto& it : binds) {
        ret = SQLBindCol(m_hstmt, it.ordinal, it.target_type, it.target,
                         it.buffer_len, &it.out_len);
        EXPECT_TRUE(SQL_SUCCEEDED(ret));
    }
    std::vector< std::vector< std::string > > result;
    while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS) {
        std::vector< std::string > row;
        for (int i = 0; i < (int)column_count; i++) {
            row.push_back(binds[i].AsString());
        }
        result.push_back(row);
    }
    std::vector< std::vector< std::string > > expected{
        {"ODBCTest", "", "DevOps", "TABLE", ""},
        {"ODBCTest", "", "IoT", "TABLE", ""},
        {"promDB", "", "promTB", "TABLE", ""}};
    CheckRows(expected, result);
}

TEST_F(TestSQLTables, TEST_ALL_DATABASES_SEARCH_PATTERN) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLTables(m_hstmt, (SQLWCHAR *)L"%BC%", SQL_NTS, (SQLWCHAR *)L"", SQL_NTS, (SQLWCHAR *)L"", SQL_NTS, (SQLWCHAR *)L"", SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::vector< bind_info > binds;
    for (int i = 1; i <= (int)column_count; i++) {
        binds.push_back(bind_info((SQLUSMALLINT)i, SQL_C_CHAR));
    }
    for (auto& it : binds) {
        ret = SQLBindCol(m_hstmt, it.ordinal, it.target_type, it.target,
                         it.buffer_len, &it.out_len);
        EXPECT_TRUE(SQL_SUCCEEDED(ret));
    }
    std::vector< std::vector< std::string > > result;
    while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS) {
        std::vector< std::string > row;
        for (int i = 0; i < (int)column_count; i++) {
            row.push_back(binds[i].AsString());
        }
        result.push_back(row);
    }
    std::vector< std::vector< std::string > > expected{
        {"ODBCTest", "", "DevOps", "TABLE", ""},
        {"ODBCTest", "", "IoT", "TABLE", ""}};
    CheckRows(expected, result);
}

TEST_F(TestSQLTables, TEST_ALL_TABLES_VIEWS_TYPES_BIND_EXCEL) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLTables(m_hstmt, nullptr, 0, nullptr, 0, nullptr, 0, (SQLWCHAR *)L"TABLE,VIEW", 10);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::vector< bind_info > binds;
    for (int i = 1; i <= (int)column_count; i++) {
        binds.push_back(bind_info((SQLUSMALLINT)i, SQL_C_CHAR));
    }
    for (auto& it : binds) {
        ret = SQLBindCol(m_hstmt, it.ordinal, it.target_type, it.target,
                         it.buffer_len, &it.out_len);
        EXPECT_TRUE(SQL_SUCCEEDED(ret));
    }
    std::vector< std::vector< std::string > > result;
    while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS) {
        std::vector< std::string > row;
        for (int i = 0; i < (int)column_count; i++) {
            row.push_back(binds[i].AsString());
        }
        result.push_back(row);
    }
    std::vector< std::vector< std::string > > expected{
        {"ODBCTest", "", "DevOps", "TABLE", ""},
        {"ODBCTest", "", "IoT", "TABLE", ""},
        {"promDB", "", "promTB", "TABLE", ""}};
    CheckRows(expected, result);
}

TEST_F(TestSQLTables, TEST_ALL_TABLES_VIEWS_TYPES_GETDATA) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLTables(m_hstmt, nullptr, SQL_NTS, nullptr, SQL_NTS, nullptr, SQL_NTS, (SQLWCHAR *)L"TABLE,VIEW", SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::vector< std::vector< std::string > > result;
    while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS) {
        std::vector< std::string > row;
        for (int i = 1; i <= (int)column_count; i++) {
            SQLCHAR data[1024] = {0};
            SQLLEN indicator = 0;
            ret = SQLGetData(m_hstmt, (SQLUSMALLINT)i, SQL_C_CHAR, data, 1024, &indicator);
            EXPECT_TRUE(SQL_SUCCEEDED(ret));
            std::string data_as_string;
            if (indicator != SQL_NULL_DATA) {
                data_as_string.assign((const char *)data, indicator);
            }
            row.push_back(data_as_string);
        }
        result.push_back(row);
    }
    std::vector< std::vector< std::string > > expected{
        {"ODBCTest", "", "DevOps", "TABLE", ""},
        {"ODBCTest", "", "IoT", "TABLE", ""},
        {"promDB", "", "promTB", "TABLE", ""}};
    CheckRows(expected, result);
}

TEST_F(TestSQLTables, TEST_ALL_VIEWS_TYPES) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLTables(m_hstmt, nullptr, 0, nullptr, 0, nullptr, 0, (SQLWCHAR *)L"VIEW", SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::vector< std::vector< std::string > > result;
    while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS) {
        std::vector< std::string > row;
        for (int i = 1; i <= (int)column_count; i++) {
            SQLCHAR data[1024] = {0};
            SQLLEN indicator = 0;
            ret = SQLGetData(m_hstmt, (SQLUSMALLINT)i, SQL_C_CHAR, data, 1024,
                             &indicator);
            EXPECT_TRUE(SQL_SUCCEEDED(ret));
            std::string data_as_string;
            data_as_string.assign((const char*)data, indicator);
            row.push_back(data_as_string);
        }
        result.push_back(row);
    }
    std::vector< std::vector< std::string > > expected{};
    CheckRows(expected, result);
}

TEST_F(TestSQLTables, TEST_ALL_TABLE_TYPES) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLTables(m_hstmt, nullptr, 0, nullptr, 0, nullptr, 0, (SQLWCHAR *)L"TABLE", SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::vector< std::vector< std::string > > result;
    while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS) {
        std::vector< std::string > row;
        for (int i = 1; i <= (int)column_count; i++) {
            SQLCHAR data[1024] = {0};
            SQLLEN indicator = 0;
            ret = SQLGetData(m_hstmt, (SQLUSMALLINT)i, SQL_C_CHAR, data, 1024,
                             &indicator);
            EXPECT_TRUE(SQL_SUCCEEDED(ret));
            std::string data_as_string;
            if (indicator != SQL_NULL_DATA) {
                data_as_string.assign((const char *)data, indicator);
            }
            row.push_back(data_as_string);
        }
        result.push_back(row);
    }
    std::vector< std::vector< std::string > > expected{
        {"ODBCTest", "", "DevOps", "TABLE", ""},
        {"ODBCTest", "", "IoT", "TABLE", ""},
        {"promDB", "", "promTB", "TABLE", ""}};
    CheckRows(expected, result);
}

TEST_F(TestSQLTables, TEST_SQL_ALL_TABLE_TYPES) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLTables(m_hstmt, nullptr, 0, nullptr, 0, nullptr, 0, (SQLWCHAR *)L"%", SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::vector< std::vector< std::string > > result;
    while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS) {
        std::vector< std::string > row;
        for (int i = 1; i <= (int)column_count; i++) {
            SQLCHAR data[1024] = {0};
            SQLLEN indicator = 0;
            ret = SQLGetData(m_hstmt, (SQLUSMALLINT)i, SQL_C_CHAR, data, 1024,
                             &indicator);
            EXPECT_TRUE(SQL_SUCCEEDED(ret));
            std::string data_as_string;
            if (indicator != SQL_NULL_DATA) {
                data_as_string.assign((const char *)data, indicator);
            }
            row.push_back(data_as_string);
        }
        result.push_back(row);
    }
    std::vector< std::vector< std::string > > expected{
        {"ODBCTest", "", "DevOps", "TABLE", ""},
        {"ODBCTest", "", "IoT", "TABLE", ""},
        {"promDB", "", "promTB", "TABLE", ""}};
    CheckRows(expected, result);
}

TEST_F(TestSQLTables, TEST_SQL_ALL_TABLE_TYPES_OTHER_EMPTY) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLTables(m_hstmt, (SQLWCHAR *)L"", 0, (SQLWCHAR *)L"", 0, (SQLWCHAR *)L"", 0, (SQLWCHAR *)L"%", SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::vector< std::vector< std::string > > result;
    while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS) {
        std::vector< std::string > row;
        for (int i = 1; i <= (int)column_count; i++) {
            SQLCHAR data[1024] = {0};
            SQLLEN indicator = 0;
            ret = SQLGetData(m_hstmt, (SQLUSMALLINT)i, SQL_C_CHAR, data, 1024,
                             &indicator);
            EXPECT_TRUE(SQL_SUCCEEDED(ret));
            std::string data_as_string;
            if (indicator != SQL_NULL_DATA) {
                data_as_string.assign((const char *)data, indicator);
            }
            row.push_back(data_as_string);
        }
        result.push_back(row);
    }
    std::vector< std::vector< std::string > > expected{
        {"", "", "", "TABLE", ""}};
    CheckRows(expected, result);
}

TEST_F(TestSQLTables, TEST_INVALID_TABLE_TYPES) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLTables(m_hstmt, nullptr, 0, nullptr, 0, nullptr, 0, (SQLWCHAR *)L"INVALID", SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::vector< std::vector< std::string > > result;
    while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS) {
        std::vector< std::string > row;
        for (int i = 1; i <= (int)column_count; i++) {
            SQLCHAR data[1024] = {0};
            SQLLEN indicator = 0;
            ret = SQLGetData(m_hstmt, (SQLUSMALLINT)i, SQL_C_CHAR, data, 1024,
                             &indicator);
            EXPECT_TRUE(SQL_SUCCEEDED(ret));
            std::string data_as_string;
            data_as_string.assign((const char*)data, indicator);
            row.push_back(data_as_string);
        }
        result.push_back(row);
    }
    std::vector< std::vector< std::string > > expected{};
    CheckRows(expected, result);
}

TEST_F(TestSQLTables, TABLE_UNDER_DATABASE) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLTables(m_hstmt, (SQLWCHAR *)L"%BCTes_", SQL_NTS, nullptr, 0, (SQLWCHAR *)L"I_T", SQL_NTS, (SQLWCHAR *)L"TABLE", SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::vector< bind_info > binds;
    for (int i = 1; i <= (int)column_count; i++) {
        binds.push_back(bind_info((SQLUSMALLINT)i, SQL_C_CHAR));
    }
    for (auto& it : binds) {
        ret = SQLBindCol(m_hstmt, it.ordinal, it.target_type, it.target,
                         it.buffer_len, &it.out_len);
        EXPECT_TRUE(SQL_SUCCEEDED(ret));
    }
    std::vector< std::vector< std::string > > result;
    while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS) {
        std::vector< std::string > row;
        for (int i = 0; i < (int)column_count; i++) {
            row.push_back(binds[i].AsString());
        }
        result.push_back(row);
    }
    std::vector< std::vector< std::string > > expected{
        {"ODBCTest", "", "IoT", "TABLE", ""}};
    CheckRows(expected, result);
}

TEST_F(TestSQLTables, EXACT_MATCH_META_DATA) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLSetStmtAttr(m_hstmt, SQL_ATTR_METADATA_ID, (SQLPOINTER)true, 0);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    ret = SQLTables(m_hstmt, (SQLWCHAR *)L"ODBCTest", SQL_NTS, (SQLWCHAR *)L"", SQL_NTS,
                    (SQLWCHAR *)L"IoT", SQL_NTS, (SQLWCHAR *)L"%", SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::vector< bind_info > binds;
    for (int i = 1; i <= (int)column_count; i++) {
        binds.push_back(bind_info((SQLUSMALLINT)i, SQL_C_CHAR));
    }
    for (auto &it : binds) {
        ret = SQLBindCol(m_hstmt, it.ordinal, it.target_type, it.target,
                         it.buffer_len, &it.out_len);
        EXPECT_TRUE(SQL_SUCCEEDED(ret));
    }
    std::vector< std::vector< std::string > > result;
    while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS) {
        std::vector< std::string > row;
        for (int i = 0; i < (int)column_count; i++) {
            row.push_back(binds[i].AsString());
        }
        result.push_back(row);
    }
    std::vector< std::vector< std::string > > expected{
        {"ODBCTest", "", "IoT", "TABLE", ""}};
    CheckRows(expected, result);
}

TEST_F(TestSQLTables, EXACT_MATCH_META_DATA_CASE_INSENSITIVE) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLSetStmtAttr(m_hstmt, SQL_ATTR_METADATA_ID, (SQLPOINTER) true, 0);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    ret = SQLTables(m_hstmt, (SQLWCHAR *)L"odbctest", SQL_NTS, (SQLWCHAR *)L"",
                    SQL_NTS, (SQLWCHAR *)L"iot", SQL_NTS, (SQLWCHAR *)L"%",
                    SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::vector< bind_info > binds;
    for (int i = 1; i <= (int)column_count; i++) {
        binds.push_back(bind_info((SQLUSMALLINT)i, SQL_C_CHAR));
    }
    for (auto &it : binds) {
        ret = SQLBindCol(m_hstmt, it.ordinal, it.target_type, it.target,
                         it.buffer_len, &it.out_len);
        EXPECT_TRUE(SQL_SUCCEEDED(ret));
    }
    std::vector< std::vector< std::string > > result;
    while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS) {
        std::vector< std::string > row;
        for (int i = 0; i < (int)column_count; i++) {
            row.push_back(binds[i].AsString());
        }
        result.push_back(row);
    }
    std::vector< std::vector< std::string > > expected{
        {"ODBCTest", "", "IoT", "TABLE", ""}};
    CheckRows(expected, result);
}

TEST_F(TestSQLTables, EXACT_MATCH_META_DATA_NOT_FOUND) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLSetStmtAttr(m_hstmt, SQL_ATTR_METADATA_ID, (SQLPOINTER) true, 0);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    ret = SQLTables(m_hstmt, (SQLWCHAR *)L"ODBC%est", SQL_NTS, (SQLWCHAR *)L"", SQL_NTS,
                    (SQLWCHAR *)L"I%T", SQL_NTS, (SQLWCHAR *)L"%", SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::vector< bind_info > binds;
    for (int i = 1; i <= (int)column_count; i++) {
        binds.push_back(bind_info((SQLUSMALLINT)i, SQL_C_CHAR));
    }
    for (auto &it : binds) {
        ret = SQLBindCol(m_hstmt, it.ordinal, it.target_type, it.target,
                         it.buffer_len, &it.out_len);
        EXPECT_TRUE(SQL_SUCCEEDED(ret));
    }
    std::vector< std::vector< std::string > > result;
    while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS) {
        std::vector< std::string > row;
        for (int i = 0; i < (int)column_count; i++) {
            row.push_back(binds[i].AsString());
        }
        result.push_back(row);
    }
    std::vector< std::vector< std::string > > expected{};
    CheckRows(expected, result);
}

TEST_F(TestSQLTables, INVALID_USE_OF_NULLPTR_CATALOG) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLSetStmtAttr(m_hstmt, SQL_ATTR_METADATA_ID, (SQLPOINTER) true, 0);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    ret = SQLTables(m_hstmt, (SQLWCHAR *)nullptr, 0, (SQLWCHAR *)L"",
                    SQL_NTS, (SQLWCHAR *)L"", SQL_NTS, (SQLWCHAR *)L"", SQL_NTS);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt, SQLSTATE_INVALID_USE_OF_NULL_POINTER));
}

TEST_F(TestSQLTables, INVALID_USE_OF_NULLPTR_SCHEMAS) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLSetStmtAttr(m_hstmt, SQL_ATTR_METADATA_ID, (SQLPOINTER) true, 0);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    ret = SQLTables(m_hstmt, (SQLWCHAR *)L"", SQL_NTS, (SQLWCHAR *)nullptr, 0,
                    (SQLWCHAR *)L"", SQL_NTS, (SQLWCHAR *)L"", SQL_NTS);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt, SQLSTATE_INVALID_USE_OF_NULL_POINTER));
}

TEST_F(TestSQLTables, INVALID_USE_OF_NULLPTR_TABLE_NAME) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLSetStmtAttr(m_hstmt, SQL_ATTR_METADATA_ID, (SQLPOINTER) true, 0);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    ret = SQLTables(m_hstmt, (SQLWCHAR *)L"", SQL_NTS, (SQLWCHAR *)L"", SQL_NTS,
                    (SQLWCHAR *)nullptr, 0, (SQLWCHAR *)L"", SQL_NTS);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt, SQLSTATE_INVALID_USE_OF_NULL_POINTER));
}


void PopulateSQLColumnsBinds(std::vector< bind_info > &binds) {
    binds.push_back(bind_info(1, SQL_C_CHAR));
    binds.push_back(bind_info(2, SQL_C_CHAR));
    binds.push_back(bind_info(3, SQL_C_CHAR));
    binds.push_back(bind_info(4, SQL_C_CHAR));
    binds.push_back(bind_info(5, SQL_C_SSHORT));
    binds.push_back(bind_info(6, SQL_C_CHAR));
    binds.push_back(bind_info(7, SQL_C_SLONG));
    binds.push_back(bind_info(8, SQL_C_SLONG));
    binds.push_back(bind_info(9, SQL_C_SSHORT));
    binds.push_back(bind_info(10, SQL_C_SSHORT));
    binds.push_back(bind_info(11, SQL_C_SSHORT));
    binds.push_back(bind_info(12, SQL_C_CHAR));
    binds.push_back(bind_info(13, SQL_C_CHAR));
    binds.push_back(bind_info(14, SQL_C_SSHORT));
    binds.push_back(bind_info(15, SQL_C_SSHORT));
    binds.push_back(bind_info(16, SQL_C_SLONG));
    binds.push_back(bind_info(17, SQL_C_SLONG));
    binds.push_back(bind_info(18, SQL_C_CHAR));
}

/**
 * SQLColumns Tests
 * This integration required manual setup.
 * Setup
 * <Database>.<Table> in Amazon Timestream
 * ODBCTest.DevOps
 * ODBCTest.IoT
 * promDB.promTB
 */

TEST_F(TestSQLColumns, EXACT_MATCH_ONE_COLUMN) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLColumns(m_hstmt, (SQLWCHAR*)L"ODBCTest", SQL_NTS, nullptr, 0,
                     (SQLWCHAR*)L"IoT", SQL_NTS, (SQLWCHAR*)L"time", SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ(18, (int)column_count);
    std::vector< bind_info > binds;
    PopulateSQLColumnsBinds(binds);
    for (auto& it : binds) {
        ret = SQLBindCol(m_hstmt, it.ordinal, it.target_type, it.target,
                         it.buffer_len, &it.out_len);
        EXPECT_TRUE(SQL_SUCCEEDED(ret));
    }
    std::vector< std::vector< std::string > > result;
    while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS) {
        std::vector< std::string > row;
        for (int i = 0; i < (int)column_count; i++) {
            row.push_back(binds[i].AsString());
        }
        result.push_back(row);
    }
    std::vector< std::vector< std::string > > expected{
        {"ODBCTest", "", "IoT", "time", std::to_string(SQL_TYPE_TIMESTAMP),
         "timestamp", "29", std::to_string(sizeof(TIMESTAMP_STRUCT)), "9", "",
         std::to_string(SQL_NULLABLE), "", "", std::to_string(SQL_DATETIME),
         std::to_string(SQL_CODE_TIMESTAMP), "", "36", "YES"}};
    CheckRows(expected, result);
}

TEST_F(TestSQLColumns, EXACT_MATCH_ALL_COLUMNS) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLColumns(m_hstmt, (SQLWCHAR*)L"ODBCTest", SQL_NTS, nullptr, 0,
                     (SQLWCHAR*)L"DevOps", SQL_NTS, nullptr, 0);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ(18, (int)column_count);
    std::vector< bind_info > binds;
    PopulateSQLColumnsBinds(binds);
    for (auto& it : binds) {
        ret = SQLBindCol(m_hstmt, it.ordinal, it.target_type, it.target,
                         it.buffer_len, &it.out_len);
        EXPECT_TRUE(SQL_SUCCEEDED(ret));
    }
    std::vector< std::vector< std::string > > result;
    while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS) {
        std::vector< std::string > row;
        for (int i = 0; i < (int)column_count; i++) {
            row.push_back(binds[i].AsString());
        }
        result.push_back(row);
    }
    std::vector< std::vector< std::string > > expected{
        {"ODBCTest", "", "DevOps", "hostname", std::to_string(SQL_VARCHAR),
         "varchar", std::to_string(INT_MAX), "256", "", "",
         std::to_string(SQL_NULLABLE), "", "", std::to_string(SQL_VARCHAR), "",
         std::to_string(INT_MAX), "1", "YES"},
        {"ODBCTest", "", "DevOps", "az", std::to_string(SQL_VARCHAR), "varchar",
         std::to_string(INT_MAX), "256", "", "", std::to_string(SQL_NULLABLE),
         "", "", std::to_string(SQL_VARCHAR), "", std::to_string(INT_MAX), "2",
         "YES"},
        {"ODBCTest", "", "DevOps", "region", std::to_string(SQL_VARCHAR),
         "varchar", std::to_string(INT_MAX), "256", "", "",
         std::to_string(SQL_NULLABLE), "", "", std::to_string(SQL_VARCHAR), "",
         std::to_string(INT_MAX), "3", "YES"},
        {"ODBCTest", "", "DevOps", "measure_value::double",
         std::to_string(SQL_DOUBLE), "double", "15",
         std::to_string(sizeof(double)), "", "10", std::to_string(SQL_NULLABLE),
         "", "", std::to_string(SQL_DOUBLE), "", "", "4", "YES"},
        {"ODBCTest", "", "DevOps", "measure_name", std::to_string(SQL_VARCHAR),
         "varchar", std::to_string(INT_MAX), "256", "", "",
         std::to_string(SQL_NULLABLE), "", "", std::to_string(SQL_VARCHAR), "",
         std::to_string(INT_MAX), "5", "YES"},
        {"ODBCTest", "", "DevOps", "time", std::to_string(SQL_TYPE_TIMESTAMP),
         "timestamp", "29", std::to_string(sizeof(TIMESTAMP_STRUCT)), "9", "",
         std::to_string(SQL_NULLABLE), "", "", std::to_string(SQL_DATETIME),
         std::to_string(SQL_CODE_TIMESTAMP), "", "6", "YES"}};
    CheckRows(expected, result);
}

TEST_F(TestSQLColumns, SEARCH_PATTERN_ALL_COLUMNS) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLColumns(m_hstmt, (SQLWCHAR*)L"ODBCTest", SQL_NTS, nullptr, 0,
                     (SQLWCHAR*)L"DevOps", SQL_NTS, (SQLWCHAR*)L"%", SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ(18, (int)column_count);
    std::vector< bind_info > binds;
    PopulateSQLColumnsBinds(binds);
    for (auto& it : binds) {
        ret = SQLBindCol(m_hstmt, it.ordinal, it.target_type, it.target,
                         it.buffer_len, &it.out_len);
        EXPECT_TRUE(SQL_SUCCEEDED(ret));
    }
    std::vector< std::vector< std::string > > result;
    while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS) {
        std::vector< std::string > row;
        for (int i = 0; i < (int)column_count; i++) {
            row.push_back(binds[i].AsString());
        }
        result.push_back(row);
    }
    std::vector< std::vector< std::string > > expected{
        {"ODBCTest", "", "DevOps", "hostname", std::to_string(SQL_VARCHAR),
         "varchar", std::to_string(INT_MAX), "256", "", "",
         std::to_string(SQL_NULLABLE), "", "", std::to_string(SQL_VARCHAR), "",
         std::to_string(INT_MAX), "1", "YES"},
        {"ODBCTest", "", "DevOps", "az", std::to_string(SQL_VARCHAR), "varchar",
         std::to_string(INT_MAX), "256", "", "", std::to_string(SQL_NULLABLE),
         "", "", std::to_string(SQL_VARCHAR), "", std::to_string(INT_MAX), "2",
         "YES"},
        {"ODBCTest", "", "DevOps", "region", std::to_string(SQL_VARCHAR),
         "varchar", std::to_string(INT_MAX), "256", "", "",
         std::to_string(SQL_NULLABLE), "", "", std::to_string(SQL_VARCHAR), "",
         std::to_string(INT_MAX), "3", "YES"},
        {"ODBCTest", "", "DevOps", "measure_value::double",
         std::to_string(SQL_DOUBLE), "double", "15",
         std::to_string(sizeof(double)), "", "10", std::to_string(SQL_NULLABLE),
         "", "", std::to_string(SQL_DOUBLE), "", "", "4", "YES"},
        {"ODBCTest", "", "DevOps", "measure_name", std::to_string(SQL_VARCHAR),
         "varchar", std::to_string(INT_MAX), "256", "", "",
         std::to_string(SQL_NULLABLE), "", "", std::to_string(SQL_VARCHAR), "",
         std::to_string(INT_MAX), "5", "YES"},
        {"ODBCTest", "", "DevOps", "time", std::to_string(SQL_TYPE_TIMESTAMP),
         "timestamp", "29", std::to_string(sizeof(TIMESTAMP_STRUCT)), "9", "",
         std::to_string(SQL_NULLABLE), "", "", std::to_string(SQL_DATETIME),
         std::to_string(SQL_CODE_TIMESTAMP), "", "6", "YES"}};
    CheckRows(expected, result);
}

TEST_F(TestSQLColumns, SEARCH_PATTERN_SOME_COLUMNS) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLColumns(m_hstmt, (SQLWCHAR*)L"ODBCTest", SQL_NTS, nullptr, 0,
                     (SQLWCHAR*)L"DevOps", SQL_NTS, (SQLWCHAR*)L"measure%",
                     SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ(18, (int)column_count);
    std::vector< bind_info > binds;
    PopulateSQLColumnsBinds(binds);
    for (auto& it : binds) {
        ret = SQLBindCol(m_hstmt, it.ordinal, it.target_type, it.target,
                         it.buffer_len, &it.out_len);
        EXPECT_TRUE(SQL_SUCCEEDED(ret));
    }
    std::vector< std::vector< std::string > > result;
    while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS) {
        std::vector< std::string > row;
        for (int i = 0; i < (int)column_count; i++) {
            row.push_back(binds[i].AsString());
        }
        result.push_back(row);
    }
    std::vector< std::vector< std::string > > expected{
        {"ODBCTest", "", "DevOps", "measure_value::double",
         std::to_string(SQL_DOUBLE), "double", "15",
         std::to_string(sizeof(double)), "", "10", std::to_string(SQL_NULLABLE),
         "", "", std::to_string(SQL_DOUBLE), "", "", "4", "YES"},
        {"ODBCTest", "", "DevOps", "measure_name", std::to_string(SQL_VARCHAR),
         "varchar", std::to_string(INT_MAX), "256", "", "",
         std::to_string(SQL_NULLABLE), "", "", std::to_string(SQL_VARCHAR), "",
         std::to_string(INT_MAX), "5", "YES"}};
    CheckRows(expected, result);
}

TEST_F(TestSQLColumns, SEARCH_PATTERN_MULTI_TABLES_COLUMNS) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLColumns(m_hstmt, (SQLWCHAR*)L"ODBCTest", SQL_NTS, nullptr, 0,
                     (SQLWCHAR*)L"%", SQL_NTS, (SQLWCHAR*)L"tim_", SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ(18, (int)column_count);
    std::vector< bind_info > binds;
    PopulateSQLColumnsBinds(binds);
    for (auto& it : binds) {
        ret = SQLBindCol(m_hstmt, it.ordinal, it.target_type, it.target,
                         it.buffer_len, &it.out_len);
        EXPECT_TRUE(SQL_SUCCEEDED(ret));
    }
    std::vector< std::vector< std::string > > result;
    while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS) {
        std::vector< std::string > row;
        for (int i = 0; i < (int)column_count; i++) {
            row.push_back(binds[i].AsString());
        }
        result.push_back(row);
    }
    std::vector< std::vector< std::string > > expected{
        {"ODBCTest", "", "DevOps", "time", std::to_string(SQL_TYPE_TIMESTAMP),
         "timestamp", "29", std::to_string(sizeof(TIMESTAMP_STRUCT)), "9", "",
         std::to_string(SQL_NULLABLE), "", "", std::to_string(SQL_DATETIME),
         std::to_string(SQL_CODE_TIMESTAMP), "", "6", "YES"},
        {"ODBCTest", "", "IoT", "time", std::to_string(SQL_TYPE_TIMESTAMP),
         "timestamp", "29", std::to_string(sizeof(TIMESTAMP_STRUCT)), "9", "",
         std::to_string(SQL_NULLABLE), "", "", std::to_string(SQL_DATETIME),
         std::to_string(SQL_CODE_TIMESTAMP), "", "36", "YES"}};
    CheckRows(expected, result);
}

TEST_F(TestSQLColumns, META_DATA_CASE_INSENSITIVE) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLSetStmtAttr(m_hstmt, SQL_ATTR_METADATA_ID, (SQLPOINTER) true, 0);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    ret = SQLColumns(m_hstmt, (SQLWCHAR*)L"odbctest", SQL_NTS, (SQLWCHAR*)L"",
                     SQL_NTS, (SQLWCHAR*)L"devops", SQL_NTS, (SQLWCHAR*)L"TIME",
                     SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ(18, (int)column_count);
    std::vector< bind_info > binds;
    PopulateSQLColumnsBinds(binds);
    for (auto& it : binds) {
        ret = SQLBindCol(m_hstmt, it.ordinal, it.target_type, it.target,
                         it.buffer_len, &it.out_len);
        EXPECT_TRUE(SQL_SUCCEEDED(ret));
    }
    std::vector< std::vector< std::string > > result;
    while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS) {
        std::vector< std::string > row;
        for (int i = 0; i < (int)column_count; i++) {
            row.push_back(binds[i].AsString());
        }
        result.push_back(row);
    }
    std::vector< std::vector< std::string > > expected{
        {"ODBCTest", "", "DevOps", "time", std::to_string(SQL_TYPE_TIMESTAMP),
         "timestamp", "29", std::to_string(sizeof(TIMESTAMP_STRUCT)), "9", "",
         std::to_string(SQL_NULLABLE), "", "", std::to_string(SQL_DATETIME),
         std::to_string(SQL_CODE_TIMESTAMP), "", "6", "YES"}};
    CheckRows(expected, result);
}

TEST_F(TestSQLColumns, META_DATA_CASE_INSENSITIVE_NOT_FOUND) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLSetStmtAttr(m_hstmt, SQL_ATTR_METADATA_ID, (SQLPOINTER) true, 0);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    ret = SQLColumns(m_hstmt, (SQLWCHAR*)L"ODBCTest", SQL_NTS, (SQLWCHAR*)L"",
                     SQL_NTS, (SQLWCHAR*)L"DevOps", SQL_NTS, (SQLWCHAR*)L"%",
                     SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ(18, (int)column_count);
    std::vector< bind_info > binds;
    PopulateSQLColumnsBinds(binds);
    for (auto& it : binds) {
        ret = SQLBindCol(m_hstmt, it.ordinal, it.target_type, it.target,
                         it.buffer_len, &it.out_len);
        EXPECT_TRUE(SQL_SUCCEEDED(ret));
    }
    std::vector< std::vector< std::string > > result;
    while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS) {
        std::vector< std::string > row;
        for (int i = 0; i < (int)column_count; i++) {
            row.push_back(binds[i].AsString());
        }
        result.push_back(row);
    }
    std::vector< std::vector< std::string > > expected{};
    CheckRows(expected, result);
}

//// We expect an empty result set for PrimaryKeys and ForeignKeys
//// Tableau specified catalog and table
//// NULL args
//TEST_SQL_KEYS(PrimaryKeys_NULL, SQLPrimaryKeys, NULL, SQL_NTS, NULL, SQL_NTS,
//              NULL, SQL_NTS)
//TEST_SQL_KEYS(ForeignKeys_NULL, SQLForeignKeys, NULL, SQL_NTS, NULL, SQL_NTS,
//              NULL, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS)
//
//// Catalog specified
//TEST_SQL_KEYS(PrimaryKeys_Catalog, SQLPrimaryKeys, NULL, SQL_NTS,
//              (SQLTCHAR*)L"odfe-cluster", SQL_NTS, NULL, SQL_NTS)
//TEST_SQL_KEYS(ForeignKeys_Catalog, SQLForeignKeys, NULL, SQL_NTS, NULL, SQL_NTS,
//              NULL, SQL_NTS, NULL, SQL_NTS, (SQLTCHAR*)L"odfe-cluster", SQL_NTS,
//              NULL, SQL_NTS)
//
//// Table specified
//TEST_SQL_KEYS(PrimaryKeys_Table, SQLPrimaryKeys, NULL, SQL_NTS, NULL, SQL_NTS,
//              (SQLTCHAR*)L"kibana_sample_data_flights", SQL_NTS)
//TEST_SQL_KEYS(ForeignKeys_Table, SQLForeignKeys, NULL, SQL_NTS, NULL, SQL_NTS,
//              NULL, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS,
//              (SQLTCHAR*)L"kibana_sample_data_flights", SQL_NTS)
//
//// Catalog and table specified
//TEST_SQL_KEYS(PrimaryKeys_CatalogTable, SQLPrimaryKeys, NULL, SQL_NTS,
//              (SQLTCHAR*)L"odfe-cluster", SQL_NTS,
//              (SQLTCHAR*)L"kibana_sample_data_flights", SQL_NTS)
//TEST_SQL_KEYS(ForeignKeys_CatalogTable, SQLForeignKeys, NULL, SQL_NTS, NULL,
//              SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS, (SQLTCHAR*)L"odfe-cluster",
//              SQL_NTS, (SQLTCHAR*)L"kibana_sample_data_flights", SQL_NTS)
//

TEST_F(TestSQLGetTypeInfo, TEST_SQL_ALL_TYPES) {
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetTypeInfo(m_hstmt, SQL_ALL_TYPES);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::vector< std::vector< std::string > > result;
    while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS) {
        std::vector< std::string > row;
        for (int i = 1; i <= (int)column_count; i++) {
            SQLCHAR data[1024] = {0};
            SQLLEN indicator = 0;
            ret = SQLGetData(m_hstmt, (SQLUSMALLINT)i, SQL_C_CHAR, data, 1024, &indicator);
            EXPECT_TRUE(SQL_SUCCEEDED(ret));
            std::string data_as_string;
            if (indicator != SQL_NULL_DATA) {
                data_as_string.assign((const char *)data, indicator);
            }
            row.push_back(data_as_string);
        }
        result.push_back(row);
    }
    std::vector< std::vector< std::string > > expected{
        {"array[T,...]", std::to_string(SQL_WVARCHAR), std::to_string(INT_MAX),
         "", "", "", "1", "0", "3", "", "0", "0", "", "", "",
         std::to_string(SQL_WVARCHAR), "", "", "0"},
        {"interval day to second", std::to_string(SQL_WVARCHAR), std::to_string(INT_MAX),
         "", "", "", "1", "0", "3", "", "0", "0", "", "", "",
         std::to_string(SQL_WVARCHAR), "", "", "0"},
        {"interval year to month", std::to_string(SQL_WVARCHAR),
         std::to_string(INT_MAX), "", "", "", "1", "0", "3", "", "0", "0", "",
         "", "", std::to_string(SQL_WVARCHAR), "", "", "0"},
        {"row(T,...)", std::to_string(SQL_WVARCHAR),
         std::to_string(INT_MAX), "", "", "", "1", "0", "3", "", "0", "0", "",
         "", "", std::to_string(SQL_WVARCHAR), "", "", "0"},
        {"timeseries[row(timestamp, T,...)]", std::to_string(SQL_WVARCHAR),
         std::to_string(INT_MAX), "", "", "", "1", "0", "3", "", "0", "0", "",
         "", "", std::to_string(SQL_WVARCHAR), "", "", "0"},
        {"varchar", std::to_string(SQL_WVARCHAR),
         std::to_string(INT_MAX), "'", "'", "", "1", "1", "3", "", "0", "0", "",
         "", "", std::to_string(SQL_WVARCHAR), "", "", "0"},
        {"unknown", std::to_string(SQL_WVARCHAR),
         std::to_string(INT_MAX), "", "", "", "1", "0", "3", "", "0", "0", "",
         "", "", std::to_string(SQL_WVARCHAR), "", "", "0"},
        {"boolean", std::to_string(SQL_BIT), "1", "",
         "", "", "1", "0", "3", "1", "0", "0", "", "", "",
         std::to_string(SQL_BIT), "", "", "0"},
        {"bigint", std::to_string(SQL_BIGINT), "20", "BIGINT '",
         "'", "", "1", "0", "3", "0", "0", "0", "", "", "",
         std::to_string(SQL_BIGINT), "", "10", "0"},
        {"int", std::to_string(SQL_INTEGER), "11", "INTEGER '",
         "'", "", "1", "0", "3", "0", "0", "0", "", "", "",
         std::to_string(SQL_INTEGER), "", "10", "0"},
        {"double", std::to_string(SQL_DOUBLE), "15", "DOUBLE '",
         "'", "", "1", "0", "3", "0", "0", "0", "", "", "",
         std::to_string(SQL_DOUBLE), "", "10", "0"},
        {"date", std::to_string(SQL_DATE), "10", "",
         "", "", "1", "0", "3", "", "0", "0", "", "", "",
         std::to_string(SQL_DATE), "1", "", "0"},
        {"time", std::to_string(SQL_TIME), "18", "",
         "", "", "1", "0", "3", "", "0", "0", "", "9", "9",
         std::to_string(SQL_TIME), "2", "", "0"},
        {"timestamp", std::to_string(SQL_TIMESTAMP), "29", "",
         "", "", "1", "0", "3", "", "0", "0", "", "9", "9",
         std::to_string(SQL_TIMESTAMP), "3", "", "0"}
    };
    CheckRows(expected, result);
}

TEST_F(TestSQLGetTypeInfo, TEST_SQL_WVARCHAR) {
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetTypeInfo(m_hstmt, SQL_WVARCHAR);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::vector< std::vector< std::string > > result;
    while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS) {
        std::vector< std::string > row;
        for (int i = 1; i <= (int)column_count; i++) {
            SQLCHAR data[1024] = {0};
            SQLLEN indicator = 0;
            ret = SQLGetData(m_hstmt, (SQLUSMALLINT)i, SQL_C_CHAR, data, 1024,
                             &indicator);
            EXPECT_TRUE(SQL_SUCCEEDED(ret));
            std::string data_as_string;
            if (indicator != SQL_NULL_DATA) {
                data_as_string.assign((const char *)data, indicator);
            }
            row.push_back(data_as_string);
        }
        result.push_back(row);
    }
    std::vector< std::vector< std::string > > expected{
        {"array[T,...]", std::to_string(SQL_WVARCHAR), std::to_string(INT_MAX),
         "", "", "", "1", "0", "3", "", "0", "0", "", "", "",
         std::to_string(SQL_WVARCHAR), "", "", "0"},
        {"interval day to second", std::to_string(SQL_WVARCHAR),
         std::to_string(INT_MAX), "", "", "", "1", "0", "3", "", "0", "0", "",
         "", "", std::to_string(SQL_WVARCHAR), "", "", "0"},
        {"interval year to month", std::to_string(SQL_WVARCHAR),
         std::to_string(INT_MAX), "", "", "", "1", "0", "3", "", "0", "0", "",
         "", "", std::to_string(SQL_WVARCHAR), "", "", "0"},
        {"row(T,...)", std::to_string(SQL_WVARCHAR), std::to_string(INT_MAX),
         "", "", "", "1", "0", "3", "", "0", "0", "", "", "",
         std::to_string(SQL_WVARCHAR), "", "", "0"},
        {"timeseries[row(timestamp, T,...)]", std::to_string(SQL_WVARCHAR),
         std::to_string(INT_MAX), "", "", "", "1", "0", "3", "", "0", "0", "",
         "", "", std::to_string(SQL_WVARCHAR), "", "", "0"},
        {"varchar", std::to_string(SQL_WVARCHAR), std::to_string(INT_MAX), "'",
         "'", "", "1", "1", "3", "", "0", "0", "", "", "",
         std::to_string(SQL_WVARCHAR), "", "", "0"},
        {"unknown", std::to_string(SQL_WVARCHAR), std::to_string(INT_MAX), "",
         "", "", "1", "0", "3", "", "0", "0", "", "", "",
         std::to_string(SQL_WVARCHAR), "", "", "0"}
    };
    CheckRows(expected, result);
}

TEST_F(TestSQLGetTypeInfo, TEST_SQL_BIT) {
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetTypeInfo(m_hstmt, SQL_BIT);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::vector< std::vector< std::string > > result;
    while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS) {
        std::vector< std::string > row;
        for (int i = 1; i <= (int)column_count; i++) {
            SQLCHAR data[1024] = {0};
            SQLLEN indicator = 0;
            ret = SQLGetData(m_hstmt, (SQLUSMALLINT)i, SQL_C_CHAR, data, 1024,
                             &indicator);
            EXPECT_TRUE(SQL_SUCCEEDED(ret));
            std::string data_as_string;
            if (indicator != SQL_NULL_DATA) {
                data_as_string.assign((const char *)data, indicator);
            }
            row.push_back(data_as_string);
        }
        result.push_back(row);
    }
    std::vector< std::vector< std::string > > expected{
        {"boolean", std::to_string(SQL_BIT), "1", "", "", "", "1", "0", "3",
         "1", "0", "0", "", "", "", std::to_string(SQL_BIT), "", "", "0"}
    };
    CheckRows(expected, result);
}

TEST_F(TestSQLGetTypeInfo, TEST_SQL_DECIMAL) {
    SQLRETURN ret = SQL_ERROR;
    ret = SQLGetTypeInfo(m_hstmt, SQL_DECIMAL);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::vector< std::vector< std::string > > result;
    while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS) {
        std::vector< std::string > row;
        for (int i = 1; i <= (int)column_count; i++) {
            SQLCHAR data[1024] = {0};
            SQLLEN indicator = 0;
            ret = SQLGetData(m_hstmt, (SQLUSMALLINT)i, SQL_C_CHAR, data, 1024,
                             &indicator);
            EXPECT_TRUE(SQL_SUCCEEDED(ret));
            std::string data_as_string;
            if (indicator != SQL_NULL_DATA) {
                data_as_string.assign((const char *)data, indicator);
            }
            row.push_back(data_as_string);
        }
        result.push_back(row);
    }
    std::vector< std::vector< std::string > > expected{};
    CheckRows(expected, result);
}

int main(int argc, char** argv) {
#ifdef __APPLE__
    // Enable malloc logging for detecting memory leaks.
    system("export MallocStackLogging=1");
#endif
    testing::internal::CaptureStdout();
    ::testing::InitGoogleTest(&argc, argv);

    int failures = RUN_ALL_TESTS();

    std::string output = testing::internal::GetCapturedStdout();
    std::cout << output << std::endl;
    std::cout << (failures ? "Not all tests passed." : "All tests passed")
              << std::endl;
    WriteFileIfSpecified(argv, argv + argc, "-fout", output);

#ifdef __APPLE__
    // Disable malloc logging and report memory leaks
    system("unset MallocStackLogging");
    system("leaks itodbc_catalog > leaks_itodbc_catalog");
#endif
    return failures;
}
