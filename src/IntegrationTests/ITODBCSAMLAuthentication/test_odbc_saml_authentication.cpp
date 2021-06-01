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
std::wstring azure_ad_dsn = L"timestream-aad";
std::wstring okta_dsn = L"timestream-okta";
std::wstring empty;
std::wstring wrong = L"wrong";

// Remember to replace with your IDP username and password
std::wstring correct_aad_username = L"";
std::wstring correct_aad_password = L"";
std::wstring correct_okta_username = L"";
std::wstring correct_okta_password = L"";

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

void SQLConnectWithDSN(SQLHDBC conn, std::wstring dsn,
                       std::wstring idp_username, std::wstring idp_password,
                       SQLRETURN expected_ret) {
    SQLRETURN ret =
        SQLConnect(conn, const_cast< SQLTCHAR* >(dsn.c_str()), SQL_NTS,
                   const_cast< SQLTCHAR* >(idp_username.c_str()),
                   static_cast< SQLSMALLINT >(idp_username.length()),
                   const_cast< SQLTCHAR* >(idp_password.c_str()),
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
    std::wstring wstr;
    wstr += L"Driver=timestreamodbc;";
    wstr += (L"UID=;");
    wstr += (L"PWD=;");
    wstr += (L"Auth=AAD;");
    wstr += (L"IdpName=AzureAD;");
    wstr += (L"AADApplicationID=;");
    wstr += (L"AADClientSecret=;");
    wstr += (L"AADTenant=;");
    wstr += (L"RoleARN=;");
    wstr += (L"IdpARN=;");
    SQLRETURN ret =
        SQLDriverConnect(m_conn, NULL, (SQLTCHAR*)wstr.c_str(), SQL_NTS,
                         m_out_conn_string, IT_SIZEOF(m_out_conn_string),
                         &m_out_conn_string_length, SQL_DRIVER_COMPLETE);
    EXPECT_EQ(SQL_SUCCESS, ret);
}

TEST_F(TestSQLDriverConnectSAMLAuth, Okta_ConnectionString) {
    std::wstring wstr;
    wstr += L"Driver=timestreamodbc;";
    wstr += (L"UID=;");
    wstr += (L"PWD=;");
    wstr += (L"Auth=OKTA;");
    wstr += (L"IdpName=Okta;");
    wstr += (L"IdpHost=;");
    wstr += (L"OktaApplicationID=;");
    wstr += (L"RoleARN=;");
    wstr += (L"IdpARN=;");
    SQLRETURN ret =
        SQLDriverConnect(m_conn, NULL, (SQLTCHAR*)wstr.c_str(), SQL_NTS,
                         m_out_conn_string, IT_SIZEOF(m_out_conn_string),
                         &m_out_conn_string_length, SQL_DRIVER_COMPLETE);
    EXPECT_EQ(SQL_SUCCESS, ret);
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
   system("leaks itodbc_saml_authentication > leaks_itodbc_saml_authentication");
#endif
   return failures;
}
