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
#include <codecvt>
#ifdef WIN32
#include <windows.h>
#endif
#include <sql.h>
#include <sqlext.h>
#include <iostream>
#include <chrono>

// clang-format on

// SQLConnect constants
namespace {
test_string default_credential_chain = CREATE_STRING("database-default");
test_string user = CREATE_STRING("test-user");
test_string pass = CREATE_STRING("test-pass");
test_string empty = CREATE_STRING("");
test_string dsn_conn_string = CREATE_STRING(
    "DSN=database-default;UID=conn-string-user;PWD=conn-string-pass");
}  // namespace

class TestSQLConnect : public testing::Test {
   public:
    void SetUp() {
        if (std::getenv("NOT_CONNECTED") && !(std::getenv("FAKE_CONNECTION"))) {
            GTEST_SKIP();
        }
        AllocConnection(&m_env, &m_conn, true, true);
    }
    void TearDown() {
        if (std::getenv("NOT_CONNECTED") && !(std::getenv("FAKE_CONNECTION"))) {
            GTEST_SKIP();
        }
        if (SQL_NULL_HDBC != m_conn) {
            SQLDisconnect(m_conn);
            SQLFreeHandle(SQL_HANDLE_DBC, m_conn);
        }
        if (SQL_NULL_HENV != m_env) {
            SQLFreeHandle(SQL_HANDLE_ENV, m_env);
        }
    }
    SQLHENV m_env;
    SQLHDBC m_conn;
};

TEST_F(TestSQLConnect, Default_Complete_DSN_UID_PWD) {
    SQLRETURN ret = SQLConnect(
        m_conn, AS_SQLTCHAR(default_credential_chain.c_str()), SQL_NTS,
        AS_SQLTCHAR(user.c_str()), static_cast< SQLSMALLINT >(user.length()),
        AS_SQLTCHAR(pass.c_str()), static_cast< SQLSMALLINT >(pass.length()));
    EXPECT_EQ(SQL_SUCCESS, ret);
    LogAnyDiagnostics(SQL_HANDLE_DBC, m_conn, ret);
}

TEST_F(TestSQLConnect, Default_Empty_Server_Name) {
    SQLRETURN ret = SQLConnect(
        m_conn, AS_SQLTCHAR(empty.c_str()), SQL_NTS, AS_SQLTCHAR(user.c_str()),
        static_cast< SQLSMALLINT >(user.length()), AS_SQLTCHAR(pass.c_str()),
        static_cast< SQLSMALLINT >(pass.length()));
    EXPECT_EQ(SQL_SUCCESS, ret);
    LogAnyDiagnostics(SQL_HANDLE_DBC, m_conn, ret);
}

TEST_F(TestSQLConnect, Default_Empty_UID) {
    // User ID is required
    SQLRETURN ret = SQLConnect(
        m_conn, AS_SQLTCHAR(default_credential_chain.c_str()), SQL_NTS,
        AS_SQLTCHAR(empty.c_str()), static_cast< SQLSMALLINT >(empty.length()),
        AS_SQLTCHAR(pass.c_str()), static_cast< SQLSMALLINT >(pass.length()));
    EXPECT_EQ(SQL_ERROR, ret);
}

TEST_F(TestSQLConnect, Default_Empty_PWD) {
    // Password is required
    SQLRETURN ret = SQLConnect(
        m_conn, AS_SQLTCHAR(default_credential_chain.c_str()), SQL_NTS,
        AS_SQLTCHAR(user.c_str()), static_cast< SQLSMALLINT >(user.length()),
        AS_SQLTCHAR(empty.c_str()), static_cast< SQLSMALLINT >(empty.length()));
    EXPECT_EQ(SQL_ERROR, ret);
}

class TestSQLDriverConnect : public testing::Test {
   public:
    void SetUp() {
        if (std::getenv("NOT_CONNECTED") && !(std::getenv("FAKE_CONNECTION"))) {
            GTEST_SKIP();
        }
        AllocConnection(&m_env, &m_conn, true, true);
    }

    void TearDown() {
        if (std::getenv("NOT_CONNECTED") && !(std::getenv("FAKE_CONNECTION"))) {
            GTEST_SKIP();
        }
        if (SQL_NULL_HDBC != m_conn) {
            SQLFreeHandle(SQL_HANDLE_DBC, m_conn);
        }
        if (SQL_NULL_HENV != m_env) {
            SQLFreeHandle(SQL_HANDLE_ENV, m_env);
        }
    }
    SQLHENV m_env = SQL_NULL_HENV;
    SQLHDBC m_conn = SQL_NULL_HDBC;
    SQLTCHAR m_out_conn_string[1024];
    SQLSMALLINT m_out_conn_string_length;
};

TEST_F(TestSQLDriverConnect, Default_DSNConnectionString) {
    SQLRETURN ret = SQLDriverConnect(
        m_conn, NULL, (SQLTCHAR*)dsn_conn_string.c_str(), SQL_NTS,
        m_out_conn_string, IT_SIZEOF(m_out_conn_string),
        &m_out_conn_string_length, SQL_DRIVER_COMPLETE);
    EXPECT_EQ(SQL_SUCCESS, ret);
}

TEST_F(TestSQLDriverConnect, Default_DriverConnectionString) {
    test_string wstr;
    wstr += CREATE_STRING("Driver=databaseodbc;");
    wstr += (CREATE_STRING("UID=") + user + CREATE_STRING(";"));
    wstr += (CREATE_STRING("PWD=") + pass + CREATE_STRING(";"));
    SQLRETURN ret =
        SQLDriverConnect(m_conn, NULL, AS_SQLTCHAR(wstr.c_str()), SQL_NTS,
                         m_out_conn_string, IT_SIZEOF(m_out_conn_string),
                         &m_out_conn_string_length, SQL_DRIVER_COMPLETE);
    EXPECT_EQ(SQL_SUCCESS, ret);
}

TEST_F(TestSQLDriverConnect, Default_SqlDriverPrompt) {
    SQLRETURN ret = SQLDriverConnect(
        m_conn, NULL, AS_SQLTCHAR(conn_string().c_str()), SQL_NTS,
        m_out_conn_string, IT_SIZEOF(m_out_conn_string),
        &m_out_conn_string_length, SQL_DRIVER_PROMPT);

    EXPECT_EQ(SQL_SUCCESS, ret);
}

TEST_F(TestSQLDriverConnect, Default_SqlDriverComplete) {
    SQLRETURN ret = SQLDriverConnect(
        m_conn, NULL, AS_SQLTCHAR(conn_string().c_str()), SQL_NTS,
        m_out_conn_string, IT_SIZEOF(m_out_conn_string),
        &m_out_conn_string_length, SQL_DRIVER_COMPLETE);

    EXPECT_EQ(SQL_SUCCESS, ret);
}

TEST_F(TestSQLDriverConnect, Default_SqlDriverCompleteRequired) {
    SQLRETURN ret = SQLDriverConnect(
        m_conn, NULL, AS_SQLTCHAR(conn_string().c_str()), SQL_NTS,
        m_out_conn_string, IT_SIZEOF(m_out_conn_string),
        &m_out_conn_string_length, SQL_DRIVER_COMPLETE_REQUIRED);

    EXPECT_EQ(SQL_SUCCESS, ret);
}

TEST_F(TestSQLDriverConnect, Default_SqlDriverNoprompt) {
    SQLRETURN ret = SQLDriverConnect(
        m_conn, NULL, AS_SQLTCHAR(conn_string().c_str()), SQL_NTS,
        m_out_conn_string, IT_SIZEOF(m_out_conn_string),
        &m_out_conn_string_length, SQL_DRIVER_NOPROMPT);

    EXPECT_EQ(SQL_SUCCESS, ret);
}

//// This should return SQL_SUCCESS_WITH_INFO (SQLSTATE 01S00 - Invalid
/// connection / string attribute)
TEST_F(TestSQLDriverConnect, UnsupportedKeyword) {
    test_string unsupported_keyword_conn_string;
    unsupported_keyword_conn_string += CREATE_STRING("Driver=databaseodbc;");
    unsupported_keyword_conn_string +=
        (CREATE_STRING("UID=") + user + CREATE_STRING(";"));
    unsupported_keyword_conn_string +=
        (CREATE_STRING("PWD=") + pass + CREATE_STRING(";"));
    unsupported_keyword_conn_string += CREATE_STRING("extraKeyword=1;");

    SQLRETURN ret = SQLDriverConnect(
        m_conn, NULL, AS_SQLTCHAR(unsupported_keyword_conn_string.c_str()),
        SQL_NTS, m_out_conn_string, IT_SIZEOF(m_out_conn_string),
        &m_out_conn_string_length, SQL_DRIVER_COMPLETE);
    EXPECT_EQ(SQL_SUCCESS, ret);
}

// TODO: ADD UNIT TESTS TO TEST DBCommunication::Disconnect()
// Currently TestSQLDisconnect does not test DBCommunication::Disconnect()
class TestSQLDisconnect : public testing::Test {
   public:
    void SetUp() {
        if (std::getenv("NOT_CONNECTED") && !(std::getenv("FAKE_CONNECTION"))) {
            GTEST_SKIP();
        }
        m_env = SQL_NULL_HENV;
        m_conn = SQL_NULL_HDBC;
    }

    void TearDown() {
        if (std::getenv("NOT_CONNECTED") && !(std::getenv("FAKE_CONNECTION"))) {
            GTEST_SKIP();
        }
        if (SQL_NULL_HDBC != m_conn) {
            SQLDisconnect(m_conn);
            SQLFreeHandle(SQL_HANDLE_DBC, m_conn);
        }
        if (SQL_NULL_HENV != m_env) {
            SQLFreeHandle(SQL_HANDLE_ENV, m_env);
        }
    }

    SQLHENV m_env = SQL_NULL_HENV;
    SQLHDBC m_conn = SQL_NULL_HDBC;
};

TEST_F(TestSQLDisconnect, TestSuccess) {
    ASSERT_NO_THROW(ITDriverConnect(AS_SQLTCHAR(conn_string().c_str()), &m_env,
                                    &m_conn, true, true));
    EXPECT_EQ(SQL_SUCCESS, SQLDisconnect(m_conn));
}

TEST_F(TestSQLDisconnect, TestReconnectOnce) {
    for (int i = 0; i <= 1; i++) {
        ASSERT_NO_THROW((ITDriverConnect(AS_SQLTCHAR(conn_string().c_str()),
                                         &m_env, &m_conn, true, true)));
        EXPECT_EQ(SQL_SUCCESS, SQLDisconnect(m_conn));

        // Explicitly deallocate memory for connection and environment handles
        // Otherwise in the for loop, handles get allocated to memory multiple
        // times but only deallocated once at end of unit test via teardown()
        // function which leads to memory leaks
        SQLFreeHandle(SQL_HANDLE_DBC, m_conn);
        SQLFreeHandle(SQL_HANDLE_ENV, m_env);
        m_env = SQL_NULL_HENV;
        m_conn = SQL_NULL_HDBC;
    }
}

TEST_F(TestSQLDisconnect, TestReconnectMultipleTimes) {
    for (int i = 0; i <= 10; i++) {
        ASSERT_NO_THROW((ITDriverConnect(AS_SQLTCHAR(conn_string().c_str()),
                                         &m_env, &m_conn, true, true)));
        EXPECT_EQ(SQL_SUCCESS, SQLDisconnect(m_conn));

        // Explicitly deallocate memory for connection and environment handles
        // Otherwise in the for loop, handles get allocated to memory multiple
        // times but only deallocated once at end of unit test via teardown()
        // function which leads to memory leaks
        SQLFreeHandle(SQL_HANDLE_DBC, m_conn);
        SQLFreeHandle(SQL_HANDLE_ENV, m_env);
        m_env = SQL_NULL_HENV;
        m_conn = SQL_NULL_HDBC;
    }
}

TEST_F(TestSQLDisconnect, TestDisconnectWithoutConnect) {
    ASSERT_NO_THROW(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_env));
    ASSERT_NO_THROW(SQLAllocHandle(SQL_HANDLE_DBC, m_env, &m_conn));
    EXPECT_EQ(SQL_SUCCESS, SQLDisconnect(m_conn));
}

class TestSQLDriverConnectMultiConnection : public Fixture {
   public:
    void SetUp() override {
        if (std::getenv("NOT_CONNECTED") && !(std::getenv("FAKE_CONNECTION"))) {
            GTEST_SKIP();
        }

        // Connection 1
        m_env = SQL_NULL_HENV;
        m_conn = SQL_NULL_HDBC;
        m_hstmt = SQL_NULL_HSTMT;
        ASSERT_NO_THROW(AllocStatement(AS_SQLTCHAR(conn_string().c_str()),
                                       &m_env, &m_conn, &m_hstmt, true, true));

        // Connection 2
        m_env2 = SQL_NULL_HENV;
        m_conn2 = SQL_NULL_HDBC;
        m_hstmt2 = SQL_NULL_HSTMT;
        ASSERT_NO_THROW(AllocStatement(AS_SQLTCHAR(conn_string().c_str()),
                                       &m_env2, &m_conn2, &m_hstmt2, true,
                                       true));
    }

    void TearDown() override {
        if (std::getenv("NOT_CONNECTED") && !(std::getenv("FAKE_CONNECTION"))) {
            GTEST_SKIP();
        }

        // Connection 1
        if (SQL_NULL_HSTMT != m_hstmt) {
            ASSERT_NO_THROW(CloseCursor(&m_hstmt, true, true));
            SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);
        }
        if (SQL_NULL_HDBC != m_conn) {
            SQLDisconnect(m_conn);
            SQLFreeHandle(SQL_HANDLE_DBC, m_conn);
        }
        if (SQL_NULL_HENV != m_env) {
            SQLFreeHandle(SQL_HANDLE_ENV, m_env);
        }

        // Connection 2
        if (SQL_NULL_HSTMT != m_hstmt2) {
            ASSERT_NO_THROW(CloseCursor(&m_hstmt2, true, true));
            SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt2);
        }
        if (SQL_NULL_HDBC != m_conn2) {
            SQLDisconnect(m_conn2);
            SQLFreeHandle(SQL_HANDLE_DBC, m_conn2);
        }
        if (SQL_NULL_HENV != m_env2) {
            SQLFreeHandle(SQL_HANDLE_ENV, m_env2);
        }
    }
    SQLHENV m_env2 = SQL_NULL_HENV;
    SQLHDBC m_conn2 = SQL_NULL_HDBC;
    SQLHSTMT m_hstmt2 = SQL_NULL_HSTMT;
};

TEST_F(TestSQLDriverConnectMultiConnection, Default_MultiConnection) {
    if (SQL_NULL_HSTMT != m_hstmt2) {
        ASSERT_NO_THROW(CloseCursor(&m_hstmt2, true, true));
        SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt2);
        m_hstmt2 = SQL_NULL_HSTMT;
    }
    if (SQL_NULL_HDBC != m_conn2) {
        SQLDisconnect(m_conn2);
        SQLFreeHandle(SQL_HANDLE_DBC, m_conn2);
        m_conn2 = SQL_NULL_HDBC;
    }
    if (SQL_NULL_HENV != m_env2) {
        SQLFreeHandle(SQL_HANDLE_ENV, m_env2);
        m_env2 = SQL_NULL_HENV;
    }

    test_string query = CREATE_STRING("SELECT 12345 FROM ODBCTest.IoT LIMIT 5");
    SQLRETURN ret = SQLExecDirect(m_hstmt, AS_SQLTCHAR(query.c_str()), SQL_NTS);

    if (std::getenv("FAKE_CONNECTION")) {
        // Connected to a fake database, no data available to query
        EXPECT_EQ(SQL_ERROR, ret);
    } else {  // Connected to a real database
        EXPECT_TRUE(SQL_SUCCEEDED(ret));
        LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
    }
}
