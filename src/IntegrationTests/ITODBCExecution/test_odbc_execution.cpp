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

/*class TestSQLSetCursorName : public testing::Test {
   public:
    TestSQLSetCursorName() {
    }

    void SetUp() {
        ASSERT_NO_THROW(AllocStatement((SQLTCHAR*)conn_string.c_str(), &m_env,
                                       &m_conn, &m_hstmt, true, true));
    }

    void TearDown() {
        CloseCursor(&m_hstmt, true, true);
        SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);
        SQLDisconnect(m_conn);
        SQLFreeHandle(SQL_HANDLE_ENV, m_env);
    }

    ~TestSQLSetCursorName() {
        // cleanup any pending stuff, but no exceptions allowed
    }

    std::wstring m_cursor_name = L"test_cursor";
    SQLHENV m_env = SQL_NULL_HENV;
    SQLHDBC m_conn = SQL_NULL_HDBC;
    SQLHSTMT m_hstmt = SQL_NULL_HSTMT;
};

class TestSQLGetCursorName : public testing::Test {
   public:
    TestSQLGetCursorName() {
    }

    void SetUp() {
        ASSERT_NO_THROW(AllocStatement((SQLTCHAR*)conn_string.c_str(), &m_env,
                                       &m_conn, &m_hstmt, true, true));
        ASSERT_EQ(SQLSetCursorName(m_hstmt, (SQLTCHAR*)m_cursor_name.c_str(),
                                   SQL_NTS),
                  SQL_SUCCESS);
    }

    void TearDown() {
        CloseCursor(&m_hstmt, true, true);
        SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);
        SQLDisconnect(m_conn);
        SQLFreeHandle(SQL_HANDLE_ENV, m_env);
    }

    ~TestSQLGetCursorName() {
        // cleanup any pending stuff, but no exceptions allowed
    }

    std::wstring m_cursor_name = L"test_cursor";
    SQLSMALLINT m_wrong_buffer_length = 1;
    SQLTCHAR m_cursor_name_buf[20];
    SQLSMALLINT m_cursor_name_length;
    SQLHENV m_env = SQL_NULL_HENV;
    SQLHDBC m_conn = SQL_NULL_HDBC;
    SQLHSTMT m_hstmt = SQL_NULL_HSTMT;
};
*/
TEST_F(TestSQLExecute, NO_SQLPREPARE) {
    SQLRETURN ret = SQLExecute(m_hstmt);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt, SQLSTATE_SEQUENCE_ERROR));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLExecute, BASIC) {
    std::wstring query = L"SELECT 12345 FROM ODBCTest.IoT LIMIT 5";
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

TEST_F(TestSQLExecute, DOUBLE_EXECUTE) {
    std::wstring query = L"SELECT 12345 FROM ODBCTest.IoT LIMIT 5";
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
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLExecute, UPDATE_PREPARE) {
    std::wstring query = L"SELECT 12345 FROM ODBCTest.IoT LIMIT 5";
    SQLRETURN ret = SQLPrepare(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    query = L"SELECT 34567 FROM ODBCTest.IoT LIMIT 3";
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
    std::wstring query = L"SELECT 1 FROM ODBCTest.IoT LIMIT 1";
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
    std::wstring query = L"SELECT 1 FROM ODBCTest.IoT LIMIT 1";
    SQLRETURN ret = SQLPrepare(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    EXPECT_EQ(SQL_SUCCESS, ret);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLPrepare, ERROR_WITH_PARAM) {
    std::wstring query = L"SELECT 1 FROM ODBCTest.IoT where time = ? LIMIT 1";
    SQLRETURN ret = SQLPrepare(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    EXPECT_EQ(SQL_ERROR, ret);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLPrepare, PREPARE_METADATA) {
    std::wstring query = L"SELECT 1, 2, 3, 4 FROM ODBCTest.IoT LIMIT 1";
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
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt, SQLSTATE_MEMORY_ALLOCATION_ERROR));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLDescribeParam, DESCRIBE_PARAM) {
    std::wstring query = L"SELECT 1 FROM ODBCTest.IoT where time = ? LIMIT 1";
    SQLRETURN ret = SQLPrepare(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    EXPECT_EQ(SQL_ERROR, ret);
    SQLSMALLINT sqlType;
    SQLULEN paramDef;
    SQLSMALLINT scale;
    SQLSMALLINT nullable;
    ret = SQLDescribeParam(m_hstmt, 1, &sqlType, &paramDef, &scale, &nullable);
    EXPECT_EQ(SQL_ERROR, ret);
    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
                              SQLSTATE_INVALID_DESCRIPTOR_INDEX));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLNumParams, NUM_PARAMS) {
    SQLSMALLINT num = 0;
    SQLRETURN ret = SQLNumParams(nullptr, &num);
    EXPECT_EQ(SQL_ERROR, ret);
    ret = SQLNumParams(m_hstmt, &num);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
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
    std::wstring query = L"SELECT 1 FROM ODBCTest.IoT LIMIT 1";
    SQLRETURN ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ(SQL_SUCCESS, SQLFetch(m_hstmt));
}

TEST_F(TestSQLFetch, NO_MORE_DATA) {
    std::wstring query = L"SELECT 1 FROM ODBCTest.IoT LIMIT 1";
    SQLRETURN ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    EXPECT_EQ(SQL_SUCCESS, SQLFetch(m_hstmt));
    EXPECT_EQ(SQL_NO_DATA, SQLFetch(m_hstmt));
}

TEST_F(TestSQLFetch, FRACTIONAL_TRUNCATION) {
    std::wstring query = L"SELECT VARCHAR \'1.5\' FROM ODBCTest.IoT LIMIT 1";
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
    std::wstring query = L"SELECT VARCHAR \'741370\' FROM ODBCTest.IoT LIMIT 1";
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
    std::wstring query = L"SELECT VARCHAR \'2\' FROM ODBCTest.IoT LIMIT 1";
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
    std::wstring query = L"SELECT DATE \'2021-01-02\' FROM ODBCTest.IoT LIMIT 1";
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
    std::wstring query = L"SELECT VARCHAR \'1.a\' FROM ODBCTest.IoT LIMIT 1";
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
    std::wstring query =
        L"SELECT * FROM ODBCTest.IoT LIMIT " + std::to_wstring(limit);
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
    std::wstring query =
        L"SELECT * FROM ODBCTest.IoT LIMIT " + std::to_wstring(limit);
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
    std::wstring query =
        L"SELECT * FROM ODBCTest.IoT LIMIT " + std::to_wstring(limit);
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
    std::wstring query =
        L"SELECT * FROM ODBCTest.IoT LIMIT " + std::to_wstring(limit);
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

/*
TEST_F(TestSQLSetCursorName, Success) {
    SQLRETURN ret =
        SQLSetCursorName(m_hstmt, (SQLTCHAR*)m_cursor_name.c_str(), SQL_NTS);
    EXPECT_EQ(SQL_SUCCESS, ret);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetCursorName, Success) {
    SQLRETURN ret =
        SQLGetCursorName(m_hstmt, m_cursor_name_buf,
                         IT_SIZEOF(m_cursor_name_buf), &m_cursor_name_length);
    EXPECT_EQ(SQL_SUCCESS, ret);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}

TEST_F(TestSQLGetCursorName, WrongLengthForCursorName) {
    SQLRETURN ret =
        SQLGetCursorName(m_hstmt, m_cursor_name_buf, m_wrong_buffer_length,
                         &m_cursor_name_length);
    EXPECT_EQ(SQL_SUCCESS_WITH_INFO, ret);
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}
*/

TEST_F(TestSQLCancel, NULLHandle) {
    SQLRETURN ret_exec = SQLCancel(NULL);
    EXPECT_EQ(ret_exec, SQL_INVALID_HANDLE);
}

TEST_F(TestSQLCancel, QueryInProgress) {
    int limit = 32925;
    std::wstring query =
        L"SELECT * FROM ODBCTest.IoT LIMIT " + std::to_wstring(limit);
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

// The following test case needs to change the logic of ts_communication before running.
// The objective of the test is to simulate a long query in SQLExecDirect
// and caller issues a SQLCancel to cancel the operation
// We need to add a sleep e.g. Sleep(1000) in the first line of function QueryCallback to
// simulate that
//TEST_F(TestSQLCancel, QueryInProgressMultithread) {
//    std::wstring query =
//        L"SELECT * FROM ODBCTest.IoT";
//    std::thread th1([&]() {
//        auto ret = SQLExecDirect(m_hstmt, (SQLTCHAR*)query.c_str(), SQL_NTS);
//        EXPECT_EQ(SQL_ERROR, ret);
//        EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
//                                  SQLSTATE_OPERATION_CANCELLED));
//    });
//    std::thread th2([&]() {
//        Sleep(500);
//        auto ret = SQLCancel(m_hstmt);
//        EXPECT_EQ(SQL_SUCCESS, ret);
//    });
//    th2.join();
//    th1.join();
//    SQLRETURN ret = SQLFetch(m_hstmt);
//    EXPECT_EQ(SQL_ERROR, ret);
//    EXPECT_TRUE(CheckSQLSTATE(SQL_HANDLE_STMT, m_hstmt,
//        SQLSTATE_INVALID_CURSUR_STATE));
//    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
//}

TEST_F(TestSQLCancel, QueryNotSent) {
    SQLRETURN ret_exec = SQLCancel(m_hstmt);
    EXPECT_EQ(ret_exec, SQL_SUCCESS);
}

TEST_F(TestSQLCancel, QueryFinished) {
    int limit = 10000;
    std::wstring query =
        L"SELECT * FROM ODBCTest.IoT LIMIT " + std::to_wstring(limit);
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

    int failures = RUN_ALL_TESTS();

    std::string output = testing::internal::GetCapturedStdout();
    std::cout << output << std::endl;
    std::cout << (failures ? "Not all tests passed." : "All tests passed")
              << std::endl;
    WriteFileIfSpecified(argv, argv + argc, "-fout", output);

#ifdef __APPLE__
    // Disable malloc logging and report memory leaks
    system("unset MallocStackLogging");
    system("leaks itodbc_execution > leaks_itodbc_execution");
#endif
    return failures;
}
