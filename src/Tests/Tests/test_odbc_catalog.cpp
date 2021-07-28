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
#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
#include "pch.h"
#include "unit_test_helper.h"
#include "it_odbc_helper.h"
#include "catfunc.h"
#include "algorithm"
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

auto CheckRows = [](const std::vector< std::vector< std::string > > &expected,
                    const std::vector< std::vector< std::string > > &result) {
    EXPECT_EQ(expected.size(), result.size());
    if (expected.size() > 0) {
        for (size_t i = 0; i < expected.size(); i++) {
            EXPECT_EQ(expected[i].size(), result[i].size());
            for (size_t j = 0; j < expected[i].size(); j++) {
                EXPECT_EQ(expected[i][j], result[i][j]);
            }
        }
    }
};

auto CheckSQLColumnsRows =
    [](std::vector< std::vector< std::string > > expected,
       std::vector< std::vector< std::string > > result) {
        EXPECT_EQ(expected.size(), result.size());
        std::sort(expected.begin(), expected.end());
        std::sort(result.begin(), result.end());
        for (size_t i = 0; i < expected.size(); i++) {
            EXPECT_EQ(expected[i].size(), result[i].size());
            for (size_t j = 0; j < expected[i].size(); j++) {
                // not checking for ordinal position since Timestream may
                // change it at any time
                if (j == COLUMNS_ORDINAL_POSITION)
                    continue;
                EXPECT_EQ(expected[i][j], result[i][j]);
            }
        }
    };

auto CheckForEmptyResultSet = [](SQLHSTMT &hstmt, int expected_col_count) {
    SQLSMALLINT column_count = 0;
    SQLRETURN ret = SQLNumResultCols(hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ(expected_col_count, static_cast< int >(column_count));
    int row_count = 0;
    while ((ret = SQLFetch(hstmt)) == SQL_SUCCESS)
        row_count++;
    EXPECT_EQ(ret, SQL_NO_DATA);
    EXPECT_EQ(row_count, 0);
};

class TestSQLTables : public Fixture {};

class TestSQLColumns : public Fixture {};

class TestSQLColAttribute : public Fixture {};

class TestCatalogSQLDescribeCol : public Fixture {};

class TestSQLGetTypeInfo : public Fixture {};

class TestSQLColumnPrivileges : public Fixture {};

class TestSQLTablePrivileges : public Fixture {};

class TestSQLPrimaryKeys : public Fixture {};

class TestSQLForeignKeys : public Fixture {};

class TestSQLProcedureColumns : public Fixture {};

class TestSQLProcedures : public Fixture {};

class TestSQLSpecialColumns : public Fixture {};

class TestSQLStatistics : public Fixture {};

//
//class TestSQLCatalogKeys : public testing::Test {
//   public:
//    TestSQLCatalogKeys() {
//    }
//    void SetUp() {
//        AllocStatement((SQLTCHAR*)(conn_string().c_str()), &m_env, &m_conn,
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

/**
 * SQLTables Tests
 * This integration required manual setup.
 * Setup
 * <Database>.<Table> in Amazon Timestream
 * ODBCTest.DevOps
 * ODBCTest.IoT
 * promDB.promTB
 */

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
        {"promDB", "", "promTB", "TABLE", ""},
        {"sampleDB", "", "DevOps", "TABLE", ""},
        {"sampleDB", "", "IoT", "TABLE", ""}};
    CheckRows(expected, result);
}

TEST_F(TestSQLTables, TEST_ALL_DATABASES_EXCEL) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLTables(m_hstmt, AS_SQLTCHAR(CREATE_STRING("%")), 1, 
        AS_SQLTCHAR(CREATE_STRING("")), 0, AS_SQLTCHAR(CREATE_STRING("")), 0, 
        AS_SQLTCHAR(CREATE_STRING("")), 0);
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
        {"promDB", "", "", "", ""},
        {"sampleDB", "", "", "", ""}};
    CheckRows(expected, result);
}

TEST_F(TestSQLTables, TEST_ALL_DATABASES_NULL) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLTables(m_hstmt, AS_SQLTCHAR(CREATE_STRING("%")), SQL_NTS, nullptr, 0, nullptr, 0, nullptr, 0);
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
        {"promDB", "", "promTB", "TABLE", ""},
        {"sampleDB", "", "DevOps", "TABLE", ""},
        {"sampleDB", "", "IoT", "TABLE", ""}};
    CheckRows(expected, result);
}

TEST_F(TestSQLTables, TEST_ALL_DATABASES_SEARCH_PATTERN) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLTables(m_hstmt, AS_SQLTCHAR(CREATE_STRING("%BC%")), SQL_NTS, AS_SQLTCHAR(CREATE_STRING("")), 
            SQL_NTS, AS_SQLTCHAR(CREATE_STRING("")), SQL_NTS, AS_SQLTCHAR(CREATE_STRING("")), SQL_NTS);
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
    ret = SQLTables(m_hstmt, nullptr, 0, nullptr, 0, nullptr, 0, AS_SQLTCHAR(CREATE_STRING("TABLE,VIEW")), 10);
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
        {"promDB", "", "promTB", "TABLE", ""},
        {"sampleDB", "", "DevOps", "TABLE", ""},
        {"sampleDB", "", "IoT", "TABLE", ""}};
    CheckRows(expected, result);
}

TEST_F(TestSQLTables, TEST_ALL_TABLES_VIEWS_TYPES_GETDATA) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLTables(m_hstmt, nullptr, SQL_NTS, nullptr, SQL_NTS, nullptr, SQL_NTS, AS_SQLTCHAR(CREATE_STRING("TABLE,VIEW")), SQL_NTS);
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
        {"promDB", "", "promTB", "TABLE", ""},
        {"sampleDB", "", "DevOps", "TABLE", ""},
        {"sampleDB", "", "IoT", "TABLE", ""}};
    CheckRows(expected, result);
}

TEST_F(TestSQLTables, TEST_ALL_VIEWS_TYPES) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLTables(m_hstmt, nullptr, 0, nullptr, 0, nullptr, 0, AS_SQLTCHAR("VIEW"), SQL_NTS);
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
    ret = SQLTables(m_hstmt, nullptr, 0, nullptr, 0, nullptr, 0, AS_SQLTCHAR(CREATE_STRING("TABLE")), SQL_NTS);
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
        {"promDB", "", "promTB", "TABLE", ""},
        {"sampleDB", "", "DevOps", "TABLE", ""},
        {"sampleDB", "", "IoT", "TABLE", ""}};
    CheckRows(expected, result);
}

TEST_F(TestSQLTables, TEST_SQL_ALL_TABLE_TYPES) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLTables(m_hstmt, nullptr, 0, nullptr, 0, nullptr, 0, AS_SQLTCHAR(CREATE_STRING("%")), SQL_NTS);
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
        {"promDB", "", "promTB", "TABLE", ""},
        {"sampleDB", "", "DevOps", "TABLE", ""},
        {"sampleDB", "", "IoT", "TABLE", ""}};
    CheckRows(expected, result);
}

TEST_F(TestSQLTables, TEST_SQL_ALL_TABLE_TYPES_OTHER_EMPTY) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLTables(m_hstmt, AS_SQLTCHAR(CREATE_STRING("")), 0, AS_SQLTCHAR(CREATE_STRING("")), 0, AS_SQLTCHAR(CREATE_STRING("")), 0, AS_SQLTCHAR(CREATE_STRING("%")), SQL_NTS);
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
    ret = SQLTables(m_hstmt, nullptr, 0, nullptr, 0, nullptr, 0, AS_SQLTCHAR(CREATE_STRING("INVALID")), SQL_NTS);
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
    ret = SQLTables(m_hstmt, AS_SQLTCHAR(CREATE_STRING("%BCTes_")), SQL_NTS, nullptr, 0, AS_SQLTCHAR(CREATE_STRING("I_T")), SQL_NTS, AS_SQLTCHAR(CREATE_STRING("TABLE")), SQL_NTS);
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
    ret = SQLTables(m_hstmt, AS_SQLTCHAR(CREATE_STRING("ODBCTest")), SQL_NTS, AS_SQLTCHAR(CREATE_STRING("")), SQL_NTS,
                    AS_SQLTCHAR(CREATE_STRING("IoT")), SQL_NTS, AS_SQLTCHAR(CREATE_STRING("%")), SQL_NTS);
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
    ret = SQLTables(m_hstmt, AS_SQLTCHAR(CREATE_STRING("odbctest")), SQL_NTS, AS_SQLTCHAR(CREATE_STRING("")),
                    SQL_NTS, AS_SQLTCHAR(CREATE_STRING("iot")), SQL_NTS, AS_SQLTCHAR(CREATE_STRING("%")),
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
    ret = SQLTables(m_hstmt, AS_SQLTCHAR(CREATE_STRING("ODBC%est")), SQL_NTS, AS_SQLTCHAR(CREATE_STRING("")), SQL_NTS,
                    AS_SQLTCHAR(CREATE_STRING("I%T")), SQL_NTS, AS_SQLTCHAR(CREATE_STRING("%")), SQL_NTS);
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
    ret = SQLTables(m_hstmt, (SQLTCHAR *)nullptr, 0, AS_SQLTCHAR(CREATE_STRING("")),
                    SQL_NTS, AS_SQLTCHAR(CREATE_STRING("")), SQL_NTS, AS_SQLTCHAR(CREATE_STRING("")), SQL_NTS);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt, SQLSTATE_INVALID_USE_OF_NULL_POINTER));
}

TEST_F(TestSQLTables, INVALID_USE_OF_NULLPTR_SCHEMAS) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLSetStmtAttr(m_hstmt, SQL_ATTR_METADATA_ID, (SQLPOINTER) true, 0);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    ret = SQLTables(m_hstmt, AS_SQLTCHAR(CREATE_STRING("")), SQL_NTS, (SQLTCHAR *)nullptr, 0,
                    AS_SQLTCHAR(CREATE_STRING("")), SQL_NTS, AS_SQLTCHAR(CREATE_STRING("")), SQL_NTS);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt, SQLSTATE_INVALID_USE_OF_NULL_POINTER));
}

TEST_F(TestSQLTables, INVALID_USE_OF_NULLPTR_TABLE_NAME) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLSetStmtAttr(m_hstmt, SQL_ATTR_METADATA_ID, (SQLPOINTER) true, 0);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    ret = SQLTables(m_hstmt, AS_SQLTCHAR(CREATE_STRING("")), SQL_NTS, AS_SQLTCHAR(CREATE_STRING("")), SQL_NTS,
                    (SQLTCHAR *)nullptr, 0, AS_SQLTCHAR(CREATE_STRING("")), SQL_NTS);
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
    ret = SQLColumns(m_hstmt, (SQLTCHAR*)CREATE_STRING("ODBCTest"), SQL_NTS, nullptr, 0,
                     (SQLTCHAR*)CREATE_STRING("IoT"), SQL_NTS, (SQLTCHAR*)CREATE_STRING("time"), SQL_NTS);
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
    CheckSQLColumnsRows(expected, result);
}

TEST_F(TestSQLColumns, EXACT_MATCH_ALL_COLUMNS) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLColumns(m_hstmt, (SQLTCHAR*)CREATE_STRING("ODBCTest"), SQL_NTS, nullptr, 0,
                     (SQLTCHAR*)CREATE_STRING("DevOps"), SQL_NTS, nullptr, 0);
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
    CheckSQLColumnsRows(expected, result);
}

TEST_F(TestSQLColumns, SEARCH_PATTERN_ALL_COLUMNS) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLColumns(m_hstmt, (SQLTCHAR*)CREATE_STRING("ODBCTest"), SQL_NTS, nullptr, 0,
                     (SQLTCHAR*)CREATE_STRING("DevOps"), SQL_NTS, (SQLTCHAR*)CREATE_STRING("%"), SQL_NTS);
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
    CheckSQLColumnsRows(expected, result);
}

TEST_F(TestSQLColumns, SEARCH_PATTERN_SOME_COLUMNS) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLColumns(m_hstmt, (SQLTCHAR*)CREATE_STRING("ODBCTest"), SQL_NTS, nullptr, 0,
                     (SQLTCHAR*)CREATE_STRING("DevOps"), SQL_NTS, (SQLTCHAR*)CREATE_STRING("measure%"),
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
    CheckSQLColumnsRows(expected, result);
}

TEST_F(TestSQLColumns, SEARCH_PATTERN_MULTI_TABLES_COLUMNS) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLColumns(m_hstmt, (SQLTCHAR*)CREATE_STRING("ODBCTest"), SQL_NTS, nullptr, 0,
                     (SQLTCHAR*)CREATE_STRING("%"), SQL_NTS, (SQLTCHAR*)CREATE_STRING("tim_"), SQL_NTS);
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
    CheckSQLColumnsRows(expected, result);
}

TEST_F(TestSQLColumns, META_DATA_CASE_INSENSITIVE) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLSetStmtAttr(m_hstmt, SQL_ATTR_METADATA_ID, (SQLPOINTER) true, 0);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    ret = SQLColumns(m_hstmt, (SQLTCHAR*)CREATE_STRING("odbctest"), SQL_NTS, (SQLTCHAR*)CREATE_STRING(""),
                     SQL_NTS, (SQLTCHAR*)CREATE_STRING("devops"), SQL_NTS, (SQLTCHAR*)CREATE_STRING("TIME"),
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
    CheckSQLColumnsRows(expected, result);
}

TEST_F(TestSQLColumns, META_DATA_CASE_INSENSITIVE_NOT_FOUND) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLSetStmtAttr(m_hstmt, SQL_ATTR_METADATA_ID, (SQLPOINTER) true, 0);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    ret = SQLColumns(m_hstmt, (SQLTCHAR*)CREATE_STRING("ODBCTest"), SQL_NTS, (SQLTCHAR*)CREATE_STRING(""),
                     SQL_NTS, (SQLTCHAR*)CREATE_STRING("DevOps"), SQL_NTS, (SQLTCHAR*)CREATE_STRING("%"),
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
    CheckSQLColumnsRows(expected, result);
}

TEST_F(TestSQLColAttribute, SQL_DESC_CONCISE_TYPE) {
    SQLRETURN ret = SQL_ERROR;
    test_string query = CREATE_STRING("SELECT INTEGER \'1\' from ODBCTest.IoT LIMIT 1");
    ret = SQLPrepare(m_hstmt, (SQLTCHAR *)query.c_str(), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLLEN numeric_attr = 0;

    ret = SQLColAttribute(m_hstmt, 1, SQL_DESC_CONCISE_TYPE, nullptr, 0,
                          nullptr, &numeric_attr);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ(SQL_INTEGER, numeric_attr);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestCatalogSQLDescribeCol, INTEGER_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string query = CREATE_STRING("SELECT INTEGER \'1\' from ODBCTest.IoT LIMIT 1");
    ret = SQLPrepare(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_INTEGER, data_type);
    EXPECT_EQ((SQLULEN)11, column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestCatalogSQLDescribeCol, DOUBLE_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string query = CREATE_STRING("SELECT DOUBLE \'1.0\' from ODBCTest.IoT LIMIT 1");
    ret = SQLPrepare(m_hstmt, (SQLTCHAR *)query.c_str(), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_DOUBLE, data_type);
    EXPECT_EQ((SQLULEN)15, column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestCatalogSQLDescribeCol, BIGINT_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string query = CREATE_STRING("SELECT BIGINT \'2147483648\' from ODBCTest.IoT LIMIT 1");
    ret = SQLPrepare(m_hstmt, (SQLTCHAR *)query.c_str(), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
     std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_BIGINT, data_type);
    EXPECT_EQ((SQLULEN)20, column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestCatalogSQLDescribeCol, BOOLEAN_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string query =
        CREATE_STRING("SELECT true from ODBCTest.IoT LIMIT 1");
    ret = SQLPrepare(m_hstmt, (SQLTCHAR *)query.c_str(), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_BIT, data_type);
    EXPECT_EQ((SQLULEN)1, column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestCatalogSQLDescribeCol, VARCHAR_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string query = CREATE_STRING("SELECT VARCHAR\'ABCDEFG\' from ODBCTest.IoT LIMIT 1");
    ret = SQLPrepare(m_hstmt, (SQLTCHAR *)query.c_str(), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_WVARCHAR, data_type);
    EXPECT_EQ((SQLULEN)INT_MAX, column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestCatalogSQLDescribeCol, TIMESERIES_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string query =
        CREATE_STRING("WITH binned_timeseries AS(SELECT TIMESTAMP'2021-03-05 "
                      "14:18:30.123456789' AS binned_timestamp, ROW(null, "
                      "ARRAY[ARRAY[ROW(12345, ARRAY[1, 2, 3])]]) "
                      "AS data FROM ODBCTest.IoT LIMIT "
                      "1), interpolated_timeseries AS(SELECT "
                      "CREATE_TIME_SERIES(binned_timestamp, data) FROM binned_timeseries) "
                      "SELECT *FROM interpolated_timeseries");
    ret = SQLPrepare(m_hstmt, (SQLTCHAR *)query.c_str(), SQL_NTS);
    ASSERT_TRUE(SQL_SUCCEEDED(ret));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
     std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_WVARCHAR, data_type);
    EXPECT_EQ((SQLULEN)INT_MAX, column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestCatalogSQLDescribeCol, ARRAY_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string query =
        CREATE_STRING("SELECT ARRAY[ARRAY[ARRAY[ARRAY[1.1, 2.3], ARRAY[1.1, 2.3]]], "
                      "ARRAY[ARRAY[ARRAY[1.1, 2.3], ARRAY[1.1, 2.3]]]] from ODBCTest.IoT "
                      "LIMIT 1");
    ret = SQLPrepare(m_hstmt, (SQLTCHAR *)query.c_str(), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
     std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_WVARCHAR, data_type);
    EXPECT_EQ((SQLULEN)INT_MAX, column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestCatalogSQLDescribeCol, ROW_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string query =
        CREATE_STRING("SELECT ROW(ROW(ROW(INTEGER '03', BIGINT '10', true), "
                      "ARRAY[ARRAY[1,2],ARRAY[1.1,2.2]])) from ODBCTest.IoT "
                      "LIMIT 1");
    ret = SQLPrepare(m_hstmt, (SQLTCHAR *)query.c_str(), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
     std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_WVARCHAR, data_type);
    EXPECT_EQ((SQLULEN)INT_MAX, column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestCatalogSQLDescribeCol, NULL_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string query = CREATE_STRING("SELECT null from ODBCTest.IoT LIMIT 1");
    ret = SQLPrepare(m_hstmt, (SQLTCHAR *)query.c_str(), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
     std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_WVARCHAR, data_type);
    EXPECT_EQ((SQLULEN)INT_MAX, column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestCatalogSQLDescribeCol, TIMESTAMP_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string query =
        CREATE_STRING("SELECT TIMESTAMP \'2021-01-02 18:01:13.000000000\' from ODBCTest.IoT "
                      "LIMIT 1");
    ret = SQLPrepare(m_hstmt, (SQLTCHAR *)query.c_str(), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
     std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_TYPE_TIMESTAMP, data_type);
    std::string expected = "2021-01-02 18:01:13.000000000";
    EXPECT_EQ((SQLULEN)expected.size(), column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestCatalogSQLDescribeCol, DATE_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string query =
        CREATE_STRING("SELECT DATE \'2021-01-02\' from ODBCTest.IoT LIMIT 1");
    ret = SQLPrepare(m_hstmt, (SQLTCHAR *)query.c_str(), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
     std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_TYPE_DATE, data_type);
    std::string expected = "2021-01-02";
    EXPECT_EQ((SQLULEN)expected.size(), column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestCatalogSQLDescribeCol, TIME_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string query =
        CREATE_STRING("SELECT TIME \'06:39:45.123456789\' from ODBCTest.IoT LIMIT 1");
    ret = SQLPrepare(m_hstmt, (SQLTCHAR *)query.c_str(), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
     std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_TYPE_TIME, data_type);
    std::string expected = "06:39:45.123456789";
    EXPECT_EQ((SQLULEN)expected.size(), column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestCatalogSQLDescribeCol, INTERVAL_YEAR_TO_MONTH_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string query = CREATE_STRING("SELECT 1year from ODBCTest.IoT LIMIT 1");
    ret = SQLPrepare(m_hstmt, (SQLTCHAR *)query.c_str(), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
     std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_WVARCHAR, data_type);
    EXPECT_EQ((SQLULEN)INT_MAX, column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestCatalogSQLDescribeCol, INTERVAL_DAY_TO_SECOND_COLUMN) {
    SQLRETURN ret = SQL_ERROR;
    test_string query = CREATE_STRING("SELECT 1d from ODBCTest.IoT LIMIT 1");
    ret = SQLPrepare(m_hstmt, (SQLTCHAR *)query.c_str(), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLTCHAR column_name[60];
    SQLSMALLINT column_name_length;
    SQLSMALLINT data_type;
    SQLULEN column_size;
    SQLSMALLINT decimal_digits;
    SQLSMALLINT nullable;
    ret = SQLDescribeCol(m_hstmt, 1, column_name, 60, &column_name_length,
                         &data_type, &column_size, &decimal_digits, &nullable);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
     std::string expected_column_name = "_col0";
    EXPECT_EQ(expected_column_name, tchar_to_string(column_name));
    EXPECT_EQ(SQL_WVARCHAR, data_type);
    EXPECT_EQ((SQLULEN)INT_MAX, column_size);
    EXPECT_EQ(0, decimal_digits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}


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
         "ARRAY [", "]", "", "1", "0", "3", "", "0", "0", "", "", "",
         std::to_string(SQL_WVARCHAR), "", "", "0"},
        {"interval day to second", std::to_string(SQL_WVARCHAR), std::to_string(INT_MAX),
         "", "", "", "1", "0", "3", "", "0", "0", "", "", "",
         std::to_string(SQL_WVARCHAR), "", "", "0"},
        {"interval year to month", std::to_string(SQL_WVARCHAR),
         std::to_string(INT_MAX), "", "", "", "1", "0", "3", "", "0", "0", "",
         "", "", std::to_string(SQL_WVARCHAR), "", "", "0"},
        {"row(T,...)", std::to_string(SQL_WVARCHAR),
         std::to_string(INT_MAX), "ROW (", ")", "", "1", "0", "3", "", "0", "0", "",
         "", "", std::to_string(SQL_WVARCHAR), "", "", "0"},
        {"timeseries[row(timestamp, T,...)]", std::to_string(SQL_WVARCHAR),
         std::to_string(INT_MAX), "", "", "", "1", "0", "3", "", "0", "0", "",
         "", "", std::to_string(SQL_WVARCHAR), "", "", "0"},
        {"varchar", std::to_string(SQL_WVARCHAR),
         std::to_string(INT_MAX), "VARCHAR '", "'", "", "1", "1", "3", "", "0", "0", "",
         "", "", std::to_string(SQL_WVARCHAR), "", "", "0"},
        {"unknown", std::to_string(SQL_WVARCHAR),
         std::to_string(INT_MAX), "", "", "", "1", "0", "3", "", "0", "0", "",
         "", "", std::to_string(SQL_WVARCHAR), "", "", "0"},
        {"boolean", std::to_string(SQL_BIT), "1", "BOOLEAN '",
         "'", "", "1", "0", "3", "1", "0", "0", "", "", "",
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
        {"date", std::to_string(SQL_DATE), "10", "DATE '",
         "'", "", "1", "0", "3", "", "0", "0", "", "", "",
         std::to_string(SQL_DATE), "1", "", "0"},
        {"time", std::to_string(SQL_TIME), "18", "TIME '",
         "'", "", "1", "0", "3", "", "0", "0", "", "9", "9",
         std::to_string(SQL_TIME), "2", "", "0"},
        {"timestamp", std::to_string(SQL_TIMESTAMP), "29", "TIMESTAMP '",
         "'", "", "1", "0", "3", "", "0", "0", "", "9", "9",
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
         "ARRAY [", "]", "", "1", "0", "3", "", "0", "0", "", "", "",
         std::to_string(SQL_WVARCHAR), "", "", "0"},
        {"interval day to second", std::to_string(SQL_WVARCHAR),
         std::to_string(INT_MAX), "", "", "", "1", "0", "3", "", "0", "0", "",
         "", "", std::to_string(SQL_WVARCHAR), "", "", "0"},
        {"interval year to month", std::to_string(SQL_WVARCHAR),
         std::to_string(INT_MAX), "", "", "", "1", "0", "3", "", "0", "0", "",
         "", "", std::to_string(SQL_WVARCHAR), "", "", "0"},
        {"row(T,...)", std::to_string(SQL_WVARCHAR), std::to_string(INT_MAX),
         "ROW (", ")", "", "1", "0", "3", "", "0", "0", "", "", "",
         std::to_string(SQL_WVARCHAR), "", "", "0"},
        {"timeseries[row(timestamp, T,...)]", std::to_string(SQL_WVARCHAR),
         std::to_string(INT_MAX), "", "", "", "1", "0", "3", "", "0", "0", "",
         "", "", std::to_string(SQL_WVARCHAR), "", "", "0"},
        {"varchar", std::to_string(SQL_WVARCHAR), std::to_string(INT_MAX), "VARCHAR '",
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
        {"boolean", std::to_string(SQL_BIT), "1", "BOOLEAN '", "'", "", "1", "0", "3",
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

TEST_F(TestSQLColumnPrivileges, EMPTY) {
    SQLRETURN ret = SQL_ERROR;
    ret = SQLColumnPrivileges(m_hstmt, (SQLTCHAR *)CREATE_STRING("ODBCTest"),
                              SQL_NTS, (SQLTCHAR *)CREATE_STRING(""), SQL_NTS,
                              (SQLTCHAR *)CREATE_STRING("DevOps"), SQL_NTS,
                              (SQLTCHAR *)CREATE_STRING("%"), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    CheckForEmptyResultSet(m_hstmt, NUM_OF_COLPRIV_FIELDS);
}

TEST_F(TestSQLTablePrivileges, EMPTY) {
    SQLRETURN ret = SQL_ERROR;
    ret = SQLTablePrivileges(m_hstmt, (SQLTCHAR *)CREATE_STRING("ODBCTest"),
                             SQL_NTS, (SQLTCHAR *)CREATE_STRING(""), SQL_NTS,
                             (SQLTCHAR *)CREATE_STRING("DevOps"), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    CheckForEmptyResultSet(m_hstmt, NUM_OF_TABPRIV_FIELDS);
}

// We expect an empty result set for PrimaryKeys and ForeignKeys
// Tableau specified catalog, table and NULL args
TEST_F(TestSQLPrimaryKeys, EMPTY_RESULT_SET_WITH_CATALOG) {
    SQLRETURN ret = SQL_ERROR;
    ret = SQLPrimaryKeys(m_hstmt, (SQLTCHAR*)CREATE_STRING("ODBCTest"), SQL_NTS,
                         NULL, 0, NULL, 0);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    CheckForEmptyResultSet(m_hstmt, NUM_OF_PKS_FIELDS);
}

TEST_F(TestSQLPrimaryKeys, EMPTY_RESULT_SET_WITH_TABLE) {
    SQLRETURN ret = SQL_ERROR;
    ret = SQLPrimaryKeys(m_hstmt, NULL, 0, NULL, 0,
                         (SQLTCHAR*)CREATE_STRING("DevOps"), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    CheckForEmptyResultSet(m_hstmt, NUM_OF_PKS_FIELDS);
}

TEST_F(TestSQLPrimaryKeys, EMPTY_RESULT_SET_WITH_NULL) {
    SQLRETURN ret = SQL_ERROR;
    ret = SQLPrimaryKeys(m_hstmt, NULL, 0, NULL, 0, NULL, 0);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    CheckForEmptyResultSet(m_hstmt, NUM_OF_PKS_FIELDS);
}

TEST_F(TestSQLForeignKeys, EMPTY_RESULT_SET_WITH_CATALOG) {
    SQLRETURN ret = SQL_ERROR;
    ret = SQLForeignKeys(m_hstmt, NULL, 0, NULL, 0, NULL, 0, NULL, 0,
                         (SQLTCHAR*)CREATE_STRING("ODBCTest"), SQL_NTS, NULL, 0);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    CheckForEmptyResultSet(m_hstmt, NUM_OF_FKS_FIELDS);
}

TEST_F(TestSQLForeignKeys, EMPTY_RESULT_SET_WITH_TABLE) {
    SQLRETURN ret = SQL_ERROR;
    ret = SQLForeignKeys(m_hstmt, NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0,
                         (SQLTCHAR*)CREATE_STRING("DevOps"), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    CheckForEmptyResultSet(m_hstmt, NUM_OF_FKS_FIELDS);
}

TEST_F(TestSQLForeignKeys, EMPTY_RESULT_SET_WITH_NULL) {
    SQLRETURN ret = SQL_ERROR;
    ret = SQLForeignKeys(m_hstmt, NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0,
                         NULL, 0);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    CheckForEmptyResultSet(m_hstmt, NUM_OF_FKS_FIELDS);
}

TEST_F(TestSQLProcedureColumns, EMPTY) {
    SQLRETURN ret = SQL_ERROR;
    ret = SQLProcedureColumns(m_hstmt, (SQLTCHAR*)CREATE_STRING("ODBCTest"),
                              SQL_NTS, NULL, 0, (SQLTCHAR*)CREATE_STRING("%"),
                              SQL_NTS, (SQLTCHAR*)CREATE_STRING("%"), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    CheckForEmptyResultSet(m_hstmt, NUM_OF_PROCOLS_FIELDS);
}

TEST_F(TestSQLProcedures, EMPTY) {
    SQLRETURN ret = SQL_ERROR;
    ret = SQLProcedures(m_hstmt, (SQLTCHAR*)CREATE_STRING("ODBCTest"), SQL_NTS,
                        (SQLTCHAR*)CREATE_STRING("%"), SQL_NTS,
                        (SQLTCHAR*)CREATE_STRING("%"), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    CheckForEmptyResultSet(m_hstmt, NUM_OF_PRO_FIELDS);
}

TEST_F(TestSQLSpecialColumns, EMPTY) {
    SQLRETURN ret = SQL_ERROR;
    ret = SQLSpecialColumns(m_hstmt, SQL_BEST_ROWID,
                            (SQLTCHAR*)CREATE_STRING("ODBCTest"), SQL_NTS, NULL,
                            0, (SQLTCHAR*)CREATE_STRING("%"), SQL_NTS,
                            SQL_SCOPE_CURROW, SQL_NULLABLE);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    CheckForEmptyResultSet(m_hstmt, NUM_OF_SPECOLS_FIELDS);
}

TEST_F(TestSQLStatistics, EMPTY) {
    SQLRETURN ret = SQL_ERROR;
    ret = SQLStatistics(m_hstmt, (SQLTCHAR*)CREATE_STRING("ODBCTest"), SQL_NTS,
                        NULL, 0, (SQLTCHAR*)CREATE_STRING("%"), SQL_NTS,
                        SQL_INDEX_ALL, SQL_ENSURE);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    CheckForEmptyResultSet(m_hstmt, NUM_OF_STATS_FIELDS);
}
