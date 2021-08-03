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
char* access_key = std::getenv("AWS_ACCESS_KEY_ID");
char* secret_key = std::getenv("AWS_SECRET_ACCESS_KEY");
test_string default_credential_chain = CREATE_STRING("timestream-aws-profile");
test_string wdsn_name = CREATE_STRING("timestream-iam");
test_string user =
    to_test_string(std::string((access_key == NULL) ? "" : access_key));
test_string pass =
    to_test_string(std::string((secret_key == NULL) ? "" : secret_key));
test_string wrong = CREATE_STRING("wrong");
test_string empty = CREATE_STRING("");
test_string dsn_conn_string = CREATE_STRING("DSN=timestream-aws-profile");
}

class TestSQLConnect : public testing::Test {
   public:
    void SetUp() {
        AllocConnection(&m_env, &m_conn, true, true);
    }
    void TearDown() {
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

TEST_F(TestSQLConnect, AWS_Profile_Default_credential_chain) {
    SQLRETURN ret = SQLConnect(
        m_conn, AS_SQLTCHAR(default_credential_chain.c_str()), SQL_NTS,
        AS_SQLTCHAR(empty.c_str()), static_cast< SQLSMALLINT >(empty.length()),
        AS_SQLTCHAR(empty.c_str()), static_cast< SQLSMALLINT >(empty.length()));
    EXPECT_EQ(SQL_SUCCESS, ret);
    LogAnyDiagnostics(SQL_HANDLE_DBC, m_conn, ret);
}

TEST_F(TestSQLConnect, IAM_Success) {
    SQLRETURN ret = SQLConnect(
        m_conn, AS_SQLTCHAR(wdsn_name.c_str()), SQL_NTS,
        AS_SQLTCHAR(user.c_str()), static_cast< SQLSMALLINT >(user.length()),
        AS_SQLTCHAR(pass.c_str()), static_cast< SQLSMALLINT >(pass.length()));
    EXPECT_EQ(SQL_SUCCESS, ret);
    LogAnyDiagnostics(SQL_HANDLE_DBC, m_conn, ret);
}

TEST_F(TestSQLConnect, IAM_empty_server_used_default) {
    SQLRETURN ret = SQLConnect(
        m_conn, AS_SQLTCHAR(empty.c_str()), SQL_NTS, AS_SQLTCHAR(user.c_str()),
        static_cast< SQLSMALLINT >(user.length()), AS_SQLTCHAR(pass.c_str()),
        static_cast< SQLSMALLINT >(pass.length()));
    EXPECT_EQ(SQL_SUCCESS, ret);
    LogAnyDiagnostics(SQL_HANDLE_DBC, m_conn, ret);
}

TEST_F(TestSQLConnect, IAM_WrongUser) {
    SQLRETURN ret = SQLConnect(
        m_conn, AS_SQLTCHAR(wdsn_name.c_str()), SQL_NTS,
        AS_SQLTCHAR(wrong.c_str()), static_cast< SQLSMALLINT >(wrong.length()),
        AS_SQLTCHAR(pass.c_str()), static_cast< SQLSMALLINT >(pass.length()));
    EXPECT_EQ(SQL_ERROR, ret);
    LogAnyDiagnostics(SQL_HANDLE_DBC, m_conn, ret);
}

TEST_F(TestSQLConnect, IAM_WrongPassword) {
    SQLRETURN ret = SQLConnect(
        m_conn, AS_SQLTCHAR(wdsn_name.c_str()), SQL_NTS,
        AS_SQLTCHAR(user.c_str()), static_cast< SQLSMALLINT >(user.length()),
        AS_SQLTCHAR(wrong.c_str()), static_cast< SQLSMALLINT >(wrong.length()));
    EXPECT_EQ(SQL_ERROR, ret);
    LogAnyDiagnostics(SQL_HANDLE_DBC, m_conn, ret);
}

class TestSQLDriverConnect : public testing::Test {
   public:
    void SetUp() {
        AllocConnection(&m_env, &m_conn, true, true);
    }

    void TearDown() {
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

TEST_F(TestSQLDriverConnect, IAM_DSNConnectionString) {
    SQLRETURN ret = SQLDriverConnect(
        m_conn, NULL, (SQLTCHAR*)dsn_conn_string.c_str(), SQL_NTS,
        m_out_conn_string, IT_SIZEOF(m_out_conn_string),
        &m_out_conn_string_length, SQL_DRIVER_COMPLETE);
    EXPECT_EQ(SQL_SUCCESS, ret);
}

TEST_F(TestSQLDriverConnect, IAM_MinimalConnectionString) {
    test_string wstr;
    wstr += CREATE_STRING("Driver=timestreamodbc;");
    wstr += (CREATE_STRING("UID=") + user + CREATE_STRING(";"));
    wstr += (CREATE_STRING("PWD=") + pass + CREATE_STRING(";"));
    SQLRETURN ret =
        SQLDriverConnect(m_conn, NULL, AS_SQLTCHAR(wstr.c_str()), SQL_NTS,
                         m_out_conn_string, IT_SIZEOF(m_out_conn_string),
                         &m_out_conn_string_length, SQL_DRIVER_COMPLETE);
    EXPECT_EQ(SQL_SUCCESS, ret);
}

TEST_F(TestSQLDriverConnect, IAM_MinimalAliasConnectionString) {
    test_string wstr;
    wstr += CREATE_STRING("Driver=timestreamodbc;");
    wstr += (CREATE_STRING("AccessKeyId=") + user + CREATE_STRING(";"));
    wstr += (CREATE_STRING("SecretAccessKey=") + pass + CREATE_STRING(";"));
    SQLRETURN ret =
        SQLDriverConnect(m_conn, NULL, AS_SQLTCHAR(wstr.c_str()), SQL_NTS,
                         m_out_conn_string, IT_SIZEOF(m_out_conn_string),
                         &m_out_conn_string_length, SQL_DRIVER_COMPLETE);
    EXPECT_EQ(SQL_SUCCESS, ret);
}

TEST_F(TestSQLDriverConnect, IAM_MinimalAliasConnectionString_Cross1) {
    test_string wstr;
    wstr += CREATE_STRING("Driver=timestreamodbc;");
    wstr += (CREATE_STRING("UID=") + user + CREATE_STRING(";"));
    wstr += (CREATE_STRING("SecretAccessKey=") + pass + CREATE_STRING(";"));
    SQLRETURN ret =
        SQLDriverConnect(m_conn, NULL, AS_SQLTCHAR(wstr.c_str()), SQL_NTS,
                         m_out_conn_string, IT_SIZEOF(m_out_conn_string),
                         &m_out_conn_string_length, SQL_DRIVER_COMPLETE);
    EXPECT_EQ(SQL_SUCCESS, ret);
}

TEST_F(TestSQLDriverConnect, IAM_MinimalAliasConnectionString_Cross2) {
    test_string wstr;
    wstr += CREATE_STRING("Driver=timestreamodbc;");
    wstr += (CREATE_STRING("AccessKeyId=") + user + CREATE_STRING(";"));
    wstr += (CREATE_STRING("PWD=") + pass + CREATE_STRING(";"));
    SQLRETURN ret =
        SQLDriverConnect(m_conn, NULL, AS_SQLTCHAR(wstr.c_str()), SQL_NTS,
                         m_out_conn_string, IT_SIZEOF(m_out_conn_string),
                         &m_out_conn_string_length, SQL_DRIVER_COMPLETE);
    EXPECT_EQ(SQL_SUCCESS, ret);
}

TEST_F(TestSQLDriverConnect, SqlDriverPrompt) {
    SQLRETURN ret = SQLDriverConnect(
        m_conn, NULL, AS_SQLTCHAR(conn_string().c_str()), SQL_NTS,
        m_out_conn_string, IT_SIZEOF(m_out_conn_string),
        &m_out_conn_string_length, SQL_DRIVER_PROMPT);

    EXPECT_EQ(SQL_SUCCESS, ret);
}

TEST_F(TestSQLDriverConnect, SqlDriverComplete) {
    SQLRETURN ret = SQLDriverConnect(
        m_conn, NULL, AS_SQLTCHAR(conn_string().c_str()), SQL_NTS,
        m_out_conn_string, IT_SIZEOF(m_out_conn_string),
        &m_out_conn_string_length, SQL_DRIVER_COMPLETE);

    EXPECT_EQ(SQL_SUCCESS, ret);
}

TEST_F(TestSQLDriverConnect, SqlDriverCompleteRequired) {
    SQLRETURN ret = SQLDriverConnect(
        m_conn, NULL, AS_SQLTCHAR(conn_string().c_str()), SQL_NTS,
        m_out_conn_string, IT_SIZEOF(m_out_conn_string),
        &m_out_conn_string_length, SQL_DRIVER_COMPLETE_REQUIRED);

    EXPECT_EQ(SQL_SUCCESS, ret);
}

TEST_F(TestSQLDriverConnect, SqlDriverNoprompt) {
    SQLRETURN ret = SQLDriverConnect(
        m_conn, NULL, AS_SQLTCHAR(conn_string().c_str()), SQL_NTS,
        m_out_conn_string, IT_SIZEOF(m_out_conn_string),
        &m_out_conn_string_length, SQL_DRIVER_NOPROMPT);

    EXPECT_EQ(SQL_SUCCESS, ret);
}

//// TODO - Revisit when parser code
//// This should return SQL_SUCCESS_WITH_INFO (SQLSTATE 01S00 - Invalid
/// connection / string attribute)
TEST_F(TestSQLDriverConnect, UnsupportedKeyword) {
    test_string unsupported_keyword_conn_string;
    unsupported_keyword_conn_string += CREATE_STRING("Driver=timestreamodbc;");
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

 class TestSQLDisconnect : public testing::Test {
   public:
    void SetUp() {
        m_env = SQL_NULL_HENV;
        m_conn = SQL_NULL_HDBC;
    }

    void TearDown() {
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
    }
}

TEST_F(TestSQLDisconnect, TestReconnectMultipleTimes) {
    for (int i = 0; i <= 10; i++) {
        ASSERT_NO_THROW((ITDriverConnect(AS_SQLTCHAR(conn_string().c_str()),
                                         &m_env, &m_conn, true, true)));
        EXPECT_EQ(SQL_SUCCESS, SQLDisconnect(m_conn));
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
        Fixture::SetUp();
        m_env2 = SQL_NULL_HENV;
        m_conn2 = SQL_NULL_HDBC;
        m_hstmt2 = SQL_NULL_HSTMT;
        ASSERT_NO_THROW(AllocStatement(AS_SQLTCHAR(conn_string().c_str()),
                                       &m_env2, &m_conn2, &m_hstmt2, true,
                                       true));
    }

    void TearDown() override {
        Fixture::TearDown();
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

TEST_F(TestSQLDriverConnectMultiConnection, AWSAPI_INIT_SHUTDOWN) {
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
    EXPECT_TRUE(SQL_SUCCEEDED(ret));
    LogAnyDiagnostics(SQL_HANDLE_STMT, m_hstmt, ret);
}
