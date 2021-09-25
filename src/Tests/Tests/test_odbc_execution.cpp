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

#ifdef WIN32
#include <windows.h>
#else
#endif
#include <sql.h>
#include <sqlext.h>
#include <iostream>
#include <thread>
#include <chrono>
// clang-format on
#ifdef WIN32
#define Sleep(milliseconds) Sleep(milliseconds)
#else
#define Sleep(milliseconds) usleep(milliseconds * 1000)
#endif

class TestSQLFetch : public Fixture {};

class TestSQLExecute : public Fixture {};

class TestSQLExecDirect : public Fixture {};

class TestSQLPrepare : public Fixture {};

class TestSQLDescribeParam : public Fixture {};

class TestSQLNumParams : public Fixture {};

class TestSQLCancel : public Fixture {};

class TestSQLEndTran : public Fixture {};

class TestSQLTransact : public Fixture {};

class TestSQLNativeSql : public Fixture {};

class TestSQLParamData : public Fixture {};

class TestSQLPutData : public Fixture {};

class TestSQLBindParameter : public Fixture {};

class TestSQLSetCursorName : public Fixture {
   public:
    test_string m_cursor_name = CREATE_STRING("test_cursor");
};

class TestSQLGetCursorName : public Fixture {
   public:
    void SetUp() override {
        Fixture::SetUp();
        if (m_hstmt != SQL_NULL_HSTMT) {
            ASSERT_EQ(SQLSetCursorName(
                          m_hstmt, AS_SQLTCHAR(m_cursor_name.c_str()), SQL_NTS), SQL_SUCCESS);
        }
    }

    test_string m_cursor_name = CREATE_STRING("test_cursor");
    SQLSMALLINT m_wrong_buffer_length = 1;
    SQLTCHAR m_cursor_name_buf[20] = {0};
    SQLSMALLINT m_cursor_name_length = 0;
};

class TestSQLBulkOperations : public Fixture {};

class TestSQLSetPos : public Fixture {};

TEST_F(TestSQLExecute, NO_SQLPREPARE) {
    SQLRETURN ret = SQLExecute(m_hstmt);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt, SQLSTATE_SEQUENCE_ERROR));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLExecute, BASIC) {
    test_string query = CREATE_STRING("SELECT 12345 FROM ODBCTest.IoT LIMIT 5");
    SQLRETURN ret = SQLPrepare(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    ret = SQLExecute(m_hstmt);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    int cnt = 0;
    while ((ret = SQLFetch(m_hstmt)) != SQL_NO_DATA) {
        if (SQL_SUCCEEDED(ret)) {
            cnt++;
        }
        SQLINTEGER data = 0;
        SQLLEN indicator = 0;
        ret = SQLGetData(m_hstmt, 1, SQL_C_SLONG, &data, 0, &indicator);
        EXPECT_TRUE(SQL_SUCCEEDED(ret));
        EXPECT_EQ((SQLLEN)sizeof(SQLINTEGER), indicator);
        EXPECT_EQ(12345, data);
    }
    EXPECT_EQ(5, cnt);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLExecute, EXECUTE_TWICE) {
    test_string query = CREATE_STRING("SELECT 12345 FROM ODBCTest.IoT LIMIT 5");
    SQLRETURN ret = SQLPrepare(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    ret = SQLExecute(m_hstmt);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    int cnt = 0;
    while ((ret = SQLFetch(m_hstmt)) != SQL_NO_DATA) {
        if (SQL_SUCCEEDED(ret)) {
            cnt++;
        }
        SQLINTEGER data = 0;
        SQLLEN indicator = 0;
        ret = SQLGetData(m_hstmt, 1, SQL_C_SLONG, &data, 0, &indicator);
        EXPECT_TRUE(SQL_SUCCEEDED(ret));
        EXPECT_EQ((SQLLEN)sizeof(SQLINTEGER), indicator);
        EXPECT_EQ(12345, data);
    }
    EXPECT_EQ(5, cnt);
    // Second trial
    ret = SQLExecute(m_hstmt);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(
        CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt, SQLSTATE_SEQUENCE_ERROR));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLExecute, EXECUTE_TWICE_WITH_CLOSECURSOR) {
    test_string query = CREATE_STRING("SELECT 12345 FROM ODBCTest.IoT LIMIT 5");
    SQLRETURN ret = SQLPrepare(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    ret = SQLExecute(m_hstmt);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    int cnt = 0;
    while ((ret = SQLFetch(m_hstmt)) != SQL_NO_DATA) {
        if (SQL_SUCCEEDED(ret)) {
            cnt++;
        }
        SQLINTEGER data = 0;
        SQLLEN indicator = 0;
        ret = SQLGetData(m_hstmt, 1, SQL_C_SLONG, &data, 0, &indicator);
        EXPECT_TRUE(SQL_SUCCEEDED(ret));
        EXPECT_EQ((SQLLEN)sizeof(SQLINTEGER), indicator);
        EXPECT_EQ(12345, data);
    }
    EXPECT_EQ(5, cnt);
    // Second trial
    ret = SQLCloseCursor(m_hstmt);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    ret = SQLExecute(m_hstmt);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLExecute, PREPARE_AND_EXECDIRECT) {
    test_string query = CREATE_STRING("SELECT 12345 FROM ODBCTest.IoT LIMIT 5");
    SQLRETURN ret = SQLPrepare(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    int cnt = 0;
    while ((ret = SQLFetch(m_hstmt)) != SQL_NO_DATA) {
        if (SQL_SUCCEEDED(ret)) {
            cnt++;
        }
        SQLINTEGER data = 0;
        SQLLEN indicator = 0;
        ret = SQLGetData(m_hstmt, 1, SQL_C_SLONG, &data, 0, &indicator);
        EXPECT_TRUE(SQL_SUCCEEDED(ret));
        EXPECT_EQ((SQLLEN)sizeof(SQLINTEGER), indicator);
        EXPECT_EQ(12345, data);
    }
    EXPECT_EQ(5, cnt);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLExecute, UPDATE_PREPARE) {
    test_string query = CREATE_STRING("SELECT 12345 FROM ODBCTest.IoT LIMIT 5");
    SQLRETURN ret = SQLPrepare(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    query = CREATE_STRING("SELECT 34567 FROM ODBCTest.IoT LIMIT 3");
    ret = SQLPrepare(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    ret = SQLExecute(m_hstmt);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    int cnt = 0;
    while ((ret = SQLFetch(m_hstmt)) != SQL_NO_DATA) {
        if (SQL_SUCCEEDED(ret)) {
            cnt++;
        }
        SQLINTEGER data = 0;
        SQLLEN indicator = 0;
        ret = SQLGetData(m_hstmt, 1, SQL_C_SLONG, &data, 0, &indicator);
        EXPECT_TRUE(SQL_SUCCEEDED(ret));
        EXPECT_EQ((SQLLEN)sizeof(SQLINTEGER), indicator);
        EXPECT_EQ(34567, data);
    }
    EXPECT_EQ(3, cnt);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLExecute, CLEAR_PREPARE) {
    test_string query = CREATE_STRING("SELECT 1 FROM ODBCTest.IoT LIMIT 1");
    SQLRETURN ret = SQLPrepare(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    ret = SQLPrepare(m_hstmt, NULL, SQL_NTS);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    EXPECT_EQ(SQL_ERROR, ret);
    ret = SQLExecute(m_hstmt);
    EXPECT_EQ(SQL_ERROR, ret);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLPrepare, SUCCESS_NO_PARAM) {
    test_string query = CREATE_STRING("SELECT 1 FROM ODBCTest.IoT LIMIT 1");
    SQLRETURN ret = SQLPrepare(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    EXPECT_EQ(SQL_SUCCESS, ret);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLPrepare, ERROR_WITH_PARAM) {
    test_string query = CREATE_STRING("SELECT 1 FROM ODBCTest.IoT where time = ? LIMIT 1");
    SQLRETURN ret = SQLPrepare(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    EXPECT_EQ(SQL_ERROR, ret);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLPrepare, PREPARE_METADATA) {
    test_string query = CREATE_STRING("SELECT 1, 2, 3, 4 FROM ODBCTest.IoT LIMIT 1");
    SQLRETURN ret = SQLPrepare(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    EXPECT_EQ(SQL_SUCCESS, ret);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    SQLSMALLINT column_count = 0;
    EXPECT_TRUE(SQL_SUCCEEDED(SQLNumResultCols(m_hstmt, &column_count)));
    EXPECT_EQ(4, column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(SQLFreeStmt(m_hstmt, SQL_CLOSE)));
}

TEST_F(TestSQLPrepare, ERROR_NULL_QUERY) {
    SQLRETURN ret = SQLPrepare(m_hstmt, NULL, SQL_NTS);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_INVALID_USE_OF_NULL_POINTER));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLPrepare, SHOW_STATEMENT) {
    test_string query = CREATE_STRING("SHOW DATABASES LIKE 'ODBC%'");
    SQLRETURN ret = SQLPrepare(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    int cnt = 0;
    while ((ret = SQLFetch(m_hstmt)) != SQL_NO_DATA) {
        if (SQL_SUCCEEDED(ret)) {
            cnt++;
        }
        SQLCHAR data[1024] = {0};
        SQLLEN indicator = 0;
        ret = SQLGetData(m_hstmt, 1, SQL_C_CHAR, &data, 1024, &indicator);
        test_string expected = CREATE_STRING("ODBCTest");
        CompareStrNumBytes(expected, indicator, data, ret);
    }
    EXPECT_EQ(1, cnt);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLPrepare, SHOW_STATEMENT_LEADING_SPACES) {
    test_string query = CREATE_STRING("      SHOW DATABASES LIKE 'ODBC%'");
    SQLRETURN ret = SQLPrepare(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    int cnt = 0;
    while ((ret = SQLFetch(m_hstmt)) != SQL_NO_DATA) {
        if (SQL_SUCCEEDED(ret)) {
            cnt++;
        }
        SQLCHAR data[1024] = {0};
        SQLLEN indicator = 0;
        ret = SQLGetData(m_hstmt, 1, SQL_C_CHAR, &data, 1024, &indicator);
        test_string expected = CREATE_STRING("ODBCTest");
        CompareStrNumBytes(expected, indicator, data, ret);
    }
    EXPECT_EQ(1, cnt);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLPrepare, DESCRIBE_STATEMENT) {
    test_string query = CREATE_STRING("DESCRIBE ODBCTest.DevOps");
    SQLRETURN ret = SQLPrepare(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLSMALLINT column_count = 0;
    EXPECT_TRUE(SQL_SUCCEEDED(SQLNumResultCols(m_hstmt, &column_count)));
    EXPECT_EQ(0, column_count);
    ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    int cnt = 0;
    while ((ret = SQLFetch(m_hstmt)) != SQL_NO_DATA) {
        if (SQL_SUCCEEDED(ret)) {
            cnt++;
        }
    }
    EXPECT_EQ(6, cnt);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLPrepare, DESCRIBE_STATEMENT_SIZE_0) {
    test_string query = CREATE_STRING("DESCRIBE ODBCTest.DevOps");
    SQLRETURN ret = SQLPrepare(m_hstmt, (SQLTCHAR*)query.c_str(), 0);
    EXPECT_EQ(SQL_SUCCESS, ret);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    SQLSMALLINT column_count = 0;
    EXPECT_TRUE(SQL_SUCCEEDED(SQLNumResultCols(m_hstmt, &column_count)));
    EXPECT_EQ(0, column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(SQLFreeStmt(m_hstmt, SQL_CLOSE)));
}

TEST_F(TestSQLPrepare, DESCRIBE_STATEMENT_GREATER_THAN_0) {
    test_string query = CREATE_STRING("DESCRIBE ODBCTest.DevOps");
    SQLRETURN ret = SQLPrepare(m_hstmt, (SQLTCHAR*)query.c_str(), 5);
    EXPECT_EQ(SQL_SUCCESS, ret);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    SQLSMALLINT column_count = 0;
    EXPECT_TRUE(SQL_SUCCEEDED(SQLNumResultCols(m_hstmt, &column_count)));
    EXPECT_EQ(0, column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(SQLFreeStmt(m_hstmt, SQL_CLOSE)));
}


TEST_F(TestSQLPrepare, WITH_SELECT_STATEMENT) {
    test_string query =
        CREATE_STRING("WITH binned_timeseries AS(SELECT TIMESTAMP'2021-03-05 ")
        CREATE_STRING("14:18:30.123456789' AS binned_timestamp, ARRAY[1,2,3] ")
        CREATE_STRING("AS data FROM ODBCTest.IoT LIMIT ")
        CREATE_STRING("1), interpolated_timeseries AS(SELECT ")
        CREATE_STRING("CREATE_TIME_SERIES(binned_timestamp, data) FROM binned_timeseries) ")
        CREATE_STRING("SELECT *FROM interpolated_timeseries");
    SQLRETURN ret = SQLPrepare(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    EXPECT_EQ(SQL_SUCCESS, ret);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    SQLSMALLINT column_count = 0;
    EXPECT_TRUE(SQL_SUCCEEDED(SQLNumResultCols(m_hstmt, &column_count)));
    EXPECT_EQ(1, column_count);
    EXPECT_TRUE(SQL_SUCCEEDED(SQLFreeStmt(m_hstmt, SQL_CLOSE)));
}

TEST_F(TestSQLDescribeParam, DESCRIBE_PARAM) {
    test_string query = CREATE_STRING("SELECT 1 FROM ODBCTest.IoT where time = ? LIMIT 1");
    SQLRETURN ret = SQLPrepare(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    EXPECT_EQ(SQL_ERROR, ret);
    SQLSMALLINT sqlType;
    SQLULEN paramDef;
    SQLSMALLINT scale;
    SQLSMALLINT nullable;
    ret = SQLDescribeParam(m_hstmt, 1, &sqlType, &paramDef, &scale, &nullable);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NOT_IMPLEMENTED_ERROR));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLNumParams, NUM_PARAMS) {
    SQLSMALLINT num = 0;
    SQLRETURN ret = SQLNumParams(nullptr, &num);
    EXPECT_EQ(SQL_ERROR, ret);
    ret = SQLNumParams(m_hstmt, &num);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt, SQLSTATE_NOT_IMPLEMENTED_ERROR));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLFetch, INVALID_HANDLE) {
    EXPECT_EQ(SQL_INVALID_HANDLE, SQLFetch(nullptr));
}

TEST_F(TestSQLFetch, INVALID_CURSOR_STATE) {
    EXPECT_EQ(SQL_ERROR, SQLFetch(m_hstmt));
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt, SQLSTATE_INVALID_CURSUR_STATE));
}

TEST_F(TestSQLFetch, SUCCESS) {
    test_string query = CREATE_STRING("SELECT 1 FROM ODBCTest.IoT LIMIT 1");
    SQLRETURN ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ(SQL_SUCCESS, SQLFetch(m_hstmt));
}

TEST_F(TestSQLFetch, NO_MORE_DATA) {
    test_string query = CREATE_STRING("SELECT 1 FROM ODBCTest.IoT LIMIT 1");
    SQLRETURN ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ(SQL_SUCCESS, SQLFetch(m_hstmt));
    EXPECT_EQ(SQL_NO_DATA, SQLFetch(m_hstmt));
}

TEST_F(TestSQLFetch, FRACTIONAL_TRUNCATION) {
    test_string query = CREATE_STRING("SELECT VARCHAR \'1.5\' FROM ODBCTest.IoT LIMIT 1");
    SQLRETURN ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLCHAR data;
    SQLLEN indicator = 0;
    ret = SQLBindCol(m_hstmt, 1, SQL_C_BIT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, SQLFetch(m_hstmt));
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt, SQLSTATE_FRACTIONAL_TRUNCATION));
}

TEST_F(TestSQLFetch, RIGHT_TRUNCATION) {
    test_string query = CREATE_STRING("SELECT VARCHAR \'741370\' FROM ODBCTest.IoT LIMIT 1");
    SQLRETURN ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLCHAR data[1024] = {0};
    SQLLEN indicator = 0;
    ret = SQLBindCol(m_hstmt, 1, SQL_C_CHAR, &data, 6, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, SQLFetch(m_hstmt));
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_STRING_DATA_RIGHT_TRUNCATED));
}

TEST_F(TestSQLFetch, OUT_OF_RANGE) {
    test_string query = CREATE_STRING("SELECT VARCHAR \'2\' FROM ODBCTest.IoT LIMIT 1");
    SQLRETURN ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLCHAR data;
    SQLLEN indicator = 0;
    ret = SQLBindCol(m_hstmt, 1, SQL_C_BIT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ(SQL_ERROR, SQLFetch(m_hstmt));
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt, SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE));
}

TEST_F(TestSQLFetch, RESTRICTED_DATA_TYPE_VIOLATION) {
    test_string query = CREATE_STRING("SELECT DATE \'2021-01-02\' FROM ODBCTest.IoT LIMIT 1");
    SQLRETURN ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    TIME_STRUCT data;
    SQLLEN indicator = 0;
    ret = SQLBindCol(m_hstmt, 1, SQL_C_TIME, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ(SQL_ERROR, SQLFetch(m_hstmt));
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt, SQLSTATE_RESTRICTED_DATA_TYPE_ERROR));
}

TEST_F(TestSQLFetch, STRING_CONVERSION_ERROR) {
    test_string query = CREATE_STRING("SELECT VARCHAR \'1.a\' FROM ODBCTest.IoT LIMIT 1");
    SQLRETURN ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    SQLCHAR data;
    SQLLEN indicator = 0;
    ret = SQLBindCol(m_hstmt, 1, SQL_C_BIT, &data, 0, &indicator);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ(SQL_ERROR, SQLFetch(m_hstmt));
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt, SQLSTATE_STRING_CONVERSION_ERROR));
}

TEST_F(TestSQLExecDirect, Success_100) {
    int limit = 100;
    test_string query =
        CREATE_STRING("SELECT * FROM ODBCTest.IoT LIMIT ") + to_test_string(std::to_string(limit));
    SQLRETURN ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    ASSERT_EQ(SQL_SUCCESS, ret);
    int cnt = 0;
    while ((ret = SQLFetch(m_hstmt)) != SQL_NO_DATA) {
        if (SQL_SUCCEEDED(ret)) {
            cnt++;
        }
    }
    EXPECT_EQ(limit, cnt);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLExecDirect, Success_400) {
    int limit = 400;
    test_string query =
        CREATE_STRING("SELECT * FROM ODBCTest.IoT LIMIT ")+ to_test_string(std::to_string(limit));
    SQLRETURN ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    ASSERT_EQ(SQL_SUCCESS, ret);
    int cnt = 0;
    while ((ret = SQLFetch(m_hstmt)) != SQL_NO_DATA) {
        if (SQL_SUCCEEDED(ret)) {
            cnt++;
        }
    }
    EXPECT_EQ(limit, cnt);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLExecDirect, Success_5000) {
    int limit = 5000;
    test_string query =
        CREATE_STRING("SELECT * FROM ODBCTest.IoT LIMIT ")+ to_test_string(std::to_string(limit));
    SQLRETURN ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    ASSERT_EQ(SQL_SUCCESS, ret);
    int cnt = 0;
    while ((ret = SQLFetch(m_hstmt)) != SQL_NO_DATA) {
        if (SQL_SUCCEEDED(ret)) {
            cnt++;
        }
    }
    EXPECT_EQ(limit, cnt);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLExecDirect, Success_10000) {
    int limit = 10000;
    test_string query =
        CREATE_STRING("SELECT * FROM ODBCTest.IoT LIMIT ")+ to_test_string(std::to_string(limit));
    SQLRETURN ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    ASSERT_EQ(SQL_SUCCESS, ret);
    int cnt = 0;
    while ((ret = SQLFetch(m_hstmt)) != SQL_NO_DATA) {
        if (SQL_SUCCEEDED(ret)) {
            cnt++;
        }
    }
    EXPECT_EQ(limit, cnt);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLExecDirect, NullQueryError) {
    SQLRETURN ret = SQLExecDirect(m_hstmt, NULL, SQL_NTS);
    EXPECT_EQ(SQL_ERROR, ret);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLSetCursorName, Success) {
    SQLRETURN ret =
        SQLSetCursorName(m_hstmt, AS_SQLTCHAR(m_cursor_name.c_str()), SQL_NTS);
    EXPECT_EQ(SQL_SUCCESS, ret);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetCursorName, Success) {
    SQLRETURN ret =
        SQLGetCursorName(m_hstmt, m_cursor_name_buf,
                         IT_SIZEOF(m_cursor_name_buf), &m_cursor_name_length);
    CompareStrNumChar(m_cursor_name,
                      static_cast< SQLINTEGER >(m_cursor_name_length),
                      m_cursor_name_buf, ret);
    EXPECT_EQ(SQL_SUCCESS, ret);
}

TEST_F(TestSQLGetCursorName, WrongLengthForCursorName) {
    SQLRETURN ret =
        SQLGetCursorName(m_hstmt, m_cursor_name_buf, m_wrong_buffer_length,
                         &m_cursor_name_length);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_STRING_DATA_RIGHT_TRUNCATED));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLCancel, NULLHandle) {
    SQLRETURN ret_exec = SQLCancel(NULL);
    EXPECT_EQ(ret_exec, SQL_INVALID_HANDLE);
}

TEST_F(TestSQLCancel, QueryInProgress) {
    int limit = 32925;
    test_string query =
        CREATE_STRING("SELECT * FROM ODBCTest.IoT LIMIT ")+ to_test_string(std::to_string(limit));
    SQLRETURN ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    ASSERT_EQ(SQL_SUCCESS, ret);
    ret = SQLCancel(m_hstmt);
    EXPECT_EQ(SQL_SUCCESS, ret);
    int cnt = 0;
    while ((ret = SQLFetch(m_hstmt)) != SQL_NO_DATA) {
        if (SQL_SUCCEEDED(ret)) {
            cnt++;
        }
    }
    EXPECT_LT(cnt, limit);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    Sleep(1000);
}

TEST_F(TestSQLCancel, QueryInProgressMultithread) {
    test_string query =
        CREATE_STRING("SELECT * FROM ODBCTest.IoT");
    std::thread th1([&]() {
        auto ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
        EXPECT_EQ(SQL_ERROR, ret);
        EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                                  SQLSTATE_OPERATION_CANCELLED));
    });
    std::thread th2([&]() {
        Sleep(100);
        auto ret = SQLCancel(m_hstmt);
        EXPECT_EQ(SQL_SUCCESS, ret);
    });
    th2.join();
    th1.join();
    SQLRETURN ret = SQLFetch(m_hstmt);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
        SQLSTATE_INVALID_CURSUR_STATE));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    Sleep(1000);
}

TEST_F(TestSQLCancel, QueryNotSent) {
    SQLRETURN ret_exec = SQLCancel(m_hstmt);
    EXPECT_EQ(ret_exec, SQL_SUCCESS);
}

TEST_F(TestSQLCancel, QueryFinished) {
    int limit = 10000;
    test_string query =
        CREATE_STRING("SELECT * FROM ODBCTest.IoT LIMIT ") + to_test_string(std::to_string(limit));
    SQLRETURN ret =
        SQLExecDirect(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    ASSERT_EQ(ret, SQL_SUCCESS);
    int cnt = 0;
    while ((ret = SQLFetch(m_hstmt)) != SQL_NO_DATA) {
        if (SQL_SUCCEEDED(ret)) {
            cnt++;
        }
    }
    EXPECT_EQ(cnt, limit);
    ret = SQLCancel(m_hstmt);
    EXPECT_EQ(ret, SQL_SUCCESS);
}

TEST_F(TestSQLEndTran, NOT_SUPPORTED) {
    SQLRETURN ret = SQLEndTran(SQL_HANDLE_DBC, m_conn, SQL_COMMIT);
    EXPECT_EQ(ret, SQL_ERROR);
    EXPECT_TRUE(
        CheckSQLSTATE(SQL_HANDLE_DBC, m_conn, SQLSTATE_NOT_IMPLEMENTED_ERROR));
    LogAnyDiagnostics(SQL_HANDLE_DBC, m_conn, ret);
}

TEST_F(TestSQLTransact, NOT_SUPPORTED) {
    SQLRETURN ret = SQLTransact(m_env, m_conn, SQL_COMMIT);
    EXPECT_EQ(ret, SQL_ERROR);
    EXPECT_TRUE(
        CheckSQLSTATE(SQL_HANDLE_DBC, m_conn, SQLSTATE_NOT_IMPLEMENTED_ERROR));
    LogAnyDiagnostics(SQL_HANDLE_DBC, m_conn, ret);
}

TEST_F(TestSQLNativeSql, SUCCESS) {
    test_string query = CREATE_STRING("SELECT * FROM ODBCTest.IoT LIMIT 1");
    SQLTCHAR out_query[1024] = {0};
    SQLINTEGER out_length = 0;
    SQLRETURN ret = SQLNativeSql(m_conn, (SQLTCHAR*)query.c_str(), SQL_NTS,
                                 out_query, 1024, &out_length);
    CompareStrNumChar(query, out_length, out_query, ret);
}

TEST_F(TestSQLParamData, NOT_SUPPORTED) {
    SQLRETURN ret = SQLParamData(m_hstmt, NULL);
    EXPECT_EQ(ret, SQL_ERROR);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NOT_IMPLEMENTED_ERROR));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLPutData, NOT_SUPPORTED) {
    SQLCHAR data;
    SQLRETURN ret = SQLPutData(m_hstmt, &data, SQL_NULL_DATA);
    EXPECT_EQ(ret, SQL_ERROR);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NOT_IMPLEMENTED_ERROR));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLBindParameter, NOT_SUPPORTED) {
    SQLCHAR data[1024];  
    SQLLEN indicator;
    SQLRETURN ret = SQLBindParameter(m_hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR,
                                     SQL_CHAR, 1024, 0, data, 0, &indicator);
    EXPECT_EQ(ret, SQL_ERROR);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NOT_IMPLEMENTED_ERROR));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLBulkOperations, NOT_SUPPORTED) {
    SQLRETURN ret = SQLBulkOperations(m_hstmt, SQL_ADD);
    EXPECT_EQ(ret, SQL_ERROR);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NOT_IMPLEMENTED_ERROR));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLSetPos, NOT_SUPPORTED) {
    SQLRETURN ret = SQLSetPos(m_hstmt, 0, SQL_POSITION, SQL_LOCK_NO_CHANGE);
    EXPECT_EQ(ret, SQL_ERROR);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_NOT_IMPLEMENTED_ERROR));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}
