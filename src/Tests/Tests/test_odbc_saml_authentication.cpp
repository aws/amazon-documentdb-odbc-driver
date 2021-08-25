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
#endif
#include <sql.h>
#include <sqlext.h>
// clang-format on

// SQLConnect constants
namespace {
    test_string azure_ad_dsn = CREATE_STRING("timestream-aad");
    test_string okta_dsn = CREATE_STRING("timestream-okta");
    test_string empty;
    test_string wrong = CREATE_STRING("wrong");

    // Remember to set your environment variables
    char* aad_userid = std::getenv("AAD_USERID");
    char* aad_password = std::getenv("AAD_PASSWORD");
    char* okta_userid = std::getenv("OKTA_USERID");
    char* okta_password = std::getenv("OKTA_PASSWORD");
    char* env_aad_connect_string = std::getenv("AAD_CONNECT_STRING");
    char* env_okta_connect_string = std::getenv("OKTA_CONNECT_STRING");

    test_string correct_aad_username = to_test_string(std::string((aad_userid == NULL) ? "" : aad_userid));
    test_string correct_aad_password = to_test_string(std::string((aad_password == NULL) ? "" : aad_password));
    test_string correct_okta_username = to_test_string(std::string((okta_userid == NULL) ? "" : okta_userid));
    test_string correct_okta_password = to_test_string(std::string((okta_password == NULL) ? "" : okta_password));
    test_string correct_aad_connect_string = to_test_string(std::string((env_aad_connect_string == NULL) ? "" : env_aad_connect_string));
    test_string correct_okta_connect_string = to_test_string(std::string((env_okta_connect_string == NULL) ? "" : env_okta_connect_string));
}

class TestSQLConnectSAMLAuth : public testing::Test {
   public:
    void SetUp() {
        m_env = SQL_NULL_HENV;
        m_conn = SQL_NULL_HDBC;
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
        m_env = SQL_NULL_HENV;
        m_conn = SQL_NULL_HDBC;
    }

    SQLHENV m_env = SQL_NULL_HENV;
    SQLHDBC m_conn = SQL_NULL_HDBC;
};

void SQLConnectWithDSN(SQLHDBC conn, test_string dsn,
                       test_string idp_username, test_string idp_password,
                       SQLRETURN expected_ret) {
    SQLRETURN ret =
        SQLConnect(conn, AS_SQLTCHAR(dsn.c_str()), SQL_NTS,
                   AS_SQLTCHAR(idp_username.c_str()),
                   static_cast< SQLSMALLINT >(idp_username.length()),
                   AS_SQLTCHAR(idp_password.c_str()),
                   static_cast< SQLSMALLINT >(idp_password.length()));
    EXPECT_EQ(expected_ret, ret);
    LogAnyDiagnostics(SQL_HANDLE_DBC, conn, ret);
}

TEST_F(TestSQLConnectSAMLAuth, AzureAD_DSN_empty_UID_PWD) {
    SQLConnectWithDSN(m_conn, azure_ad_dsn, empty, empty, SQL_SUCCESS);
}

TEST_F(TestSQLConnectSAMLAuth, Okta_DSN_empty_UID_PWD) {
    SQLConnectWithDSN(m_conn, okta_dsn, empty, empty, SQL_SUCCESS);
}

TEST_F(TestSQLConnectSAMLAuth, AzureAD_DSN_correct_UID_PWD) {
    SQLConnectWithDSN(m_conn, azure_ad_dsn, correct_aad_username,
                      correct_aad_password, SQL_SUCCESS);
}

TEST_F(TestSQLConnectSAMLAuth, Okta_DSN_correct_UID_PWD) {
    SQLConnectWithDSN(m_conn, okta_dsn, correct_okta_username,
                      correct_okta_password, SQL_SUCCESS);
}

TEST_F(TestSQLConnectSAMLAuth, AzureAD_DSN_wrong_UID_PWD) {
    SQLConnectWithDSN(m_conn, azure_ad_dsn, wrong, wrong, SQL_ERROR);
}

TEST_F(TestSQLConnectSAMLAuth, Okta_DSN_wrong_UID_PWD) {
    SQLConnectWithDSN(m_conn, okta_dsn, wrong, wrong, SQL_ERROR);
}

class TestSQLDriverConnectSAMLAuth : public testing::Test {
   public:
    void SetUp() {
        m_env = SQL_NULL_HENV;
        m_conn = SQL_NULL_HDBC;
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
        m_env = SQL_NULL_HENV;
        m_conn = SQL_NULL_HDBC;
    }

    SQLHENV m_env = SQL_NULL_HENV;
    SQLHDBC m_conn = SQL_NULL_HDBC;
    SQLTCHAR m_out_conn_string[1024];
    SQLSMALLINT m_out_conn_string_length;
};

TEST_F(TestSQLDriverConnectSAMLAuth, AzureAD_ConnectionString) {
    test_string wstr;
    wstr = correct_aad_connect_string;

    SQLRETURN ret =
        SQLDriverConnect(m_conn, NULL, (SQLTCHAR*)wstr.c_str(), SQL_NTS,
                         m_out_conn_string, IT_SIZEOF(m_out_conn_string),
                         &m_out_conn_string_length, SQL_DRIVER_COMPLETE);
    EXPECT_EQ(SQL_SUCCESS, ret);
}

TEST_F(TestSQLDriverConnectSAMLAuth, Okta_ConnectionString) {
    test_string wstr;
    wstr = correct_okta_connect_string;
    
    SQLRETURN ret =
        SQLDriverConnect(m_conn, NULL, (SQLTCHAR*)wstr.c_str(), SQL_NTS,
                         m_out_conn_string, IT_SIZEOF(m_out_conn_string),
                         &m_out_conn_string_length, SQL_DRIVER_COMPLETE);
    EXPECT_EQ(SQL_SUCCESS, ret);
}
