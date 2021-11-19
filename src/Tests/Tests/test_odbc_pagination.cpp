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
#include <vector>
// clang-format on

// Size of the cell
// Modify to a larger value if necessary
#define BIND_SIZE 1024

struct Cell {
    SQLLEN len;
    SQLCHAR data[BIND_SIZE];
};

constexpr int NUM_ROWS_IOT = 1600;
constexpr int NUM_ROWS_DEVOPS = 2100;

/**
 * Setup before running the test
 * 1. Create sample database in Amazon Database service
 * 2. Check to create sample table IoT and DevOps
 */

class TestPagination : public Fixture {};

TEST_F(TestPagination, DatabaseSampleDatabase_IoT_SQLGetData) {
    test_string query = CREATE_STRING("SELECT * FROM sampleDB.IoT");
    auto ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    EXPECT_EQ(SQL_SUCCESS, ret);
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    int count = 0;
    int expected_count = NUM_ROWS_IOT;
    while ((ret = SQLFetch(m_hstmt)) != SQL_NO_DATA) {
        if (SQL_SUCCEEDED(ret)) {
            for (SQLUSMALLINT i = 0; i < column_count; i++) {
                SQLCHAR data[BIND_SIZE] = {0};
                SQLLEN indicator = 0;
                ret = SQLGetData(m_hstmt, i+1, SQL_C_CHAR, data, BIND_SIZE,
                    &indicator);
                EXPECT_TRUE(SQL_SUCCEEDED(ret));
            }
            count++;
        }
    }
    EXPECT_EQ(expected_count, count);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestPagination, DatabaseSampleDatabase_IoT_SQLBindCol) {
    test_string query = CREATE_STRING("SELECT * FROM sampleDB.IoT");
    auto ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    EXPECT_EQ(SQL_SUCCESS, ret);
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    int count = 0;
    int expected_count = NUM_ROWS_IOT;
    std::vector< Cell > row(column_count);
    for (SQLUSMALLINT i = 0; i < row.size(); i++) {
        ret = SQLBindCol(m_hstmt, i + 1, SQL_C_CHAR, row[i].data, BIND_SIZE, &row[i].len);
        EXPECT_TRUE(SQL_SUCCEEDED(ret));
    }
    while ((ret = SQLFetch(m_hstmt)) != SQL_NO_DATA) {
        if (SQL_SUCCEEDED(ret)) {
            count++;
        }
    }
    EXPECT_EQ(expected_count, count);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestPagination, DatabaseSampleDatabase_DevOps_SQLGetData) {
    test_string query = CREATE_STRING("SELECT * FROM sampleDB.DevOps");
    auto ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    EXPECT_EQ(SQL_SUCCESS, ret);
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    int count = 0;
    int expected_count = NUM_ROWS_DEVOPS;
    while ((ret = SQLFetch(m_hstmt)) != SQL_NO_DATA) {
        if (SQL_SUCCEEDED(ret)) {
            for (SQLUSMALLINT i = 0; i < column_count; i++) {
                SQLCHAR data[BIND_SIZE] = {0};
                SQLLEN indicator = 0;
                ret = SQLGetData(m_hstmt, i + 1, SQL_C_CHAR, data, BIND_SIZE,
                                 &indicator);
                EXPECT_TRUE(SQL_SUCCEEDED(ret));
            }
            count++;
        }
    }
    EXPECT_EQ(expected_count, count);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestPagination, DatabaseSampleDatabase_DevOps_SQLBindCol) {
    test_string query = CREATE_STRING("SELECT * FROM sampleDB.DevOps");
    auto ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    EXPECT_EQ(SQL_SUCCESS, ret);
    SQLSMALLINT column_count = 0;
    ret = SQLNumResultCols(m_hstmt, &column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    int count = 0;
    int expected_count = NUM_ROWS_DEVOPS;
    std::vector< Cell > row(column_count);
    for (SQLUSMALLINT i = 0; i < row.size(); i++) {
        ret = SQLBindCol(m_hstmt, i + 1, SQL_C_CHAR, row[i].data, BIND_SIZE,
                         &row[i].len);
        EXPECT_TRUE(SQL_SUCCEEDED(ret));
    }
    while ((ret = SQLFetch(m_hstmt)) != SQL_NO_DATA) {
        if (SQL_SUCCEEDED(ret)) {
            count++;
        }
    }
    EXPECT_EQ(expected_count, count);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}
