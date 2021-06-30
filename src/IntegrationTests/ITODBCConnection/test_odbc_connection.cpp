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

// SQLDriverConnect constants
class TestSQLConnect : public testing::Test {
   public:
    void SetUp() {
        wdsn_name = CREATE_STRING("timestream-iam");
        wrong = CREATE_STRING("wrong");
        empty = CREATE_STRING("");
#ifndef __linux__
        char* access_key = std::getenv("AWS_ACCESS_KEY_ID");
        char* secret_key = std::getenv("AWS_SECRET_ACCESS_KEY");
        user = to_test_string(std::string((access_key == NULL) ? "fake_user" : access_key));
        pass = to_test_string(std::string((secret_key == NULL) ? "fake_secret" : secret_key));
#else
        user = CREATE_STRING("fake_user");
        pass = CREATE_STRING("fake-secret");
#endif
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
    test_string wdsn_name;
    test_string wrong;
    test_string empty;
    test_string user;
    test_string pass;
    SQLHENV m_env;
    SQLHDBC m_conn;
};


TEST_F(TestSQLConnect, AWS_Profile_Default_credential_chain) {
    test_string default_credential_chain = CREATE_STRING("timestream-aws-profile");
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
        AS_SQLTCHAR(user.c_str()),
        static_cast< SQLSMALLINT >(user.length()), AS_SQLTCHAR(pass.c_str()),
        static_cast< SQLSMALLINT >(pass.length()));
    EXPECT_EQ(SQL_SUCCESS, ret);
    LogAnyDiagnostics(SQL_HANDLE_DBC, m_conn, ret);
}


TEST_F(TestSQLConnect, IAM_empty_server_used_default) {
    SQLRETURN ret = SQLConnect(
        m_conn, AS_SQLTCHAR(empty.c_str()), SQL_NTS,
        AS_SQLTCHAR(user.c_str()), static_cast< SQLSMALLINT >(user.length()),
        AS_SQLTCHAR(pass.c_str()), static_cast< SQLSMALLINT >(pass.length()));
    EXPECT_EQ(SQL_SUCCESS, ret);
    LogAnyDiagnostics(SQL_HANDLE_DBC, m_conn, ret);
}

#ifndef __linux__
// TODO AT-864: Linux currently fails on these because the DSN is not configured correctly in GitHub actions.
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
        m_conn, AS_SQLTCHAR(wdsn_name.c_str()), SQL_NTS, AS_SQLTCHAR(user.c_str()),
        static_cast< SQLSMALLINT >(user.length()),     AS_SQLTCHAR(wrong.c_str()),
        static_cast< SQLSMALLINT >(wrong.length()));
    EXPECT_EQ(SQL_ERROR, ret);
    LogAnyDiagnostics(SQL_HANDLE_DBC, m_conn, ret);
}
#endif


class TestSQLDriverConnect : public testing::Test {
   public:
    void SetUp() {
        dsn_conn_string = CREATE_STRING("DSN=timestream-aws-profile");
        m_env = SQL_NULL_HENV;
        m_conn = SQL_NULL_HDBC;
#ifndef __linux__
        char* access_key = std::getenv("AWS_ACCESS_KEY_ID");
        char* secret_key = std::getenv("AWS_SECRET_ACCESS_KEY");
        user = to_test_string(std::string((access_key == NULL) ? "fake_user" : access_key));
        pass = to_test_string(std::string((secret_key == NULL) ? "fake_secret" : secret_key));
#else
        user = CREATE_STRING("fake_user");
        pass = CREATE_STRING("fake-secret");
#endif
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
    test_string dsn_conn_string;
    SQLHENV m_env = SQL_NULL_HENV;
    SQLHDBC m_conn = SQL_NULL_HDBC;
    SQLTCHAR m_out_conn_string[1024];
    SQLSMALLINT m_out_conn_string_length;
    test_string user;
    test_string pass;
};

TEST_F(TestSQLDriverConnect, IAM_DSNConnectionString) {
    SQLRETURN ret = SQLDriverConnect(
        m_conn, NULL, (SQLTCHAR*)dsn_conn_string.c_str(), SQL_NTS,
        m_out_conn_string, IT_SIZEOF(m_out_conn_string),
        &m_out_conn_string_length, SQL_DRIVER_COMPLETE);
    EXPECT_EQ(SQL_SUCCESS, ret);
}

TEST_F(TestSQLDriverConnect, IAM_MinimalConnectionString) {
    printf("Entering IAM_MinimalConnectionString\n");
    test_string wstr;
    wstr += CREATE_STRING("Driver=timestreamodbc;");
    wstr += (CREATE_STRING("UID=") + user + CREATE_STRING(";"));
    wstr += (CREATE_STRING("SecretAccessKey=") + pass + CREATE_STRING(";"));
    printf("SQLRETURN ret = SQLDriverConnect\n");
    SQLRETURN ret = SQLDriverConnect(
        m_conn, NULL, AS_SQLTCHAR(wstr.c_str()), SQL_NTS,
        m_out_conn_string, IT_SIZEOF(m_out_conn_string),
        &m_out_conn_string_length, SQL_DRIVER_COMPLETE);
    printf("EXPECT_EQ(SQL_SUCCESS, ret);\n");
    EXPECT_EQ(SQL_SUCCESS, ret);
}

TEST_F(TestSQLDriverConnect, IAM_MinimalAliasConnectionString) {
    test_string wstr;
    wstr += CREATE_STRING("Driver=timestreamodbc;");
    wstr += (CREATE_STRING("AccessKeyId=") + user + CREATE_STRING(";"));
    wstr += (CREATE_STRING("SecretAccessKey=") + pass + CREATE_STRING(";"));
    SQLRETURN ret =
        SQLDriverConnect(m_conn, NULL, (SQLTCHAR*)wstr.c_str(), SQL_NTS,
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
        SQLDriverConnect(m_conn, NULL, (SQLTCHAR*)wstr.c_str(), SQL_NTS,
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
        SQLDriverConnect(m_conn, NULL, (SQLTCHAR*)wstr.c_str(), SQL_NTS,
                         m_out_conn_string, IT_SIZEOF(m_out_conn_string),
                         &m_out_conn_string_length, SQL_DRIVER_COMPLETE);
    EXPECT_EQ(SQL_SUCCESS, ret);
}

// TODO: enable after aligning the connection string
//TEST_F(TestSQLDriverConnect, SqlDriverPrompt) {
//    SQLRETURN ret =
//        SQLDriverConnect(m_conn, NULL, (SQLTCHAR*)(conn_string().c_str()), SQL_NTS,
//                         m_out_conn_string, IT_SIZEOF(m_out_conn_string),
//                         &m_out_conn_string_length, SQL_DRIVER_PROMPT);
//
//    EXPECT_EQ(SQL_SUCCESS, ret);
//}

//TEST_F(TestSQLDriverConnect, SqlDriverComplete) {
//    SQLRETURN ret =
//        SQLDriverConnect(m_conn, NULL, (SQLTCHAR*)(conn_string().c_str()), SQL_NTS,
//                         m_out_conn_string, IT_SIZEOF(m_out_conn_string),
//                         &m_out_conn_string_length, SQL_DRIVER_COMPLETE);
//
//    EXPECT_EQ(SQL_SUCCESS, ret);
//}
//
//TEST_F(TestSQLDriverConnect, SqlDriverCompleteRequired) {
//    SQLRETURN ret = SQLDriverConnect(
//        m_conn, NULL, (SQLTCHAR*)(conn_string().c_str()), SQL_NTS,
//        m_out_conn_string, IT_SIZEOF(m_out_conn_string),
//        &m_out_conn_string_length, SQL_DRIVER_COMPLETE_REQUIRED);
//
//    EXPECT_EQ(SQL_SUCCESS, ret);
//}
//
//TEST_F(TestSQLDriverConnect, SqlDriverNoprompt) {
//    SQLRETURN ret =
//        SQLDriverConnect(m_conn, NULL, (SQLTCHAR*)(conn_string().c_str()), SQL_NTS,
//                         m_out_conn_string, IT_SIZEOF(m_out_conn_string),
//                         &m_out_conn_string_length, SQL_DRIVER_NOPROMPT);
//
//    EXPECT_EQ(SQL_SUCCESS, ret);
//}
//
//// TODO #41 - Revisit when parser code
//// This should return SQL_SUCCESS_WITH_INFO
//TEST_F(TestSQLDriverConnect, InvalidDriver) {
//    test_string invalid_driver_conn_string =
//         use_ssl ? L"Driver=xxxx;"
//                   L"host=https://localhost;port=5432;"
//                   L"UID=admin;PWD=admin;auth=IAM;"
//                   L"logLevel=0;logOutput=C:\\;"
//                   L"responseTimeout=10;"
//                 : L"Driver=xxxx;"
//                   L"host=localhost;port=5432;"
//                   L"UID=admin;PWD=admin;auth=IAM;"
//                   L"logLevel=0;logOutput=C:\\;"
//                   L"responseTimeout=10;";
//
//    SQLRETURN ret = SQLDriverConnect(
//        m_conn, NULL, (SQLTCHAR*)invalid_driver_conn_string.c_str(), SQL_NTS,
//        m_out_conn_string, IT_SIZEOF(m_out_conn_string),
//        &m_out_conn_string_length, SQL_DRIVER_COMPLETE);
//    EXPECT_EQ(SQL_ERROR, ret);
// }
//
//TEST_F(TestSQLDriverConnect, InvalidHost) {
//    test_string invalid_host_conn_string =
//        use_ssl ? L"Driver={Elasticsearch ODBC};"
//                  L"host=https://8.8.8.8;port=9200;"
//                  L"UID=admin;PWD=admin;auth=IAM;"
//                  L"logLevel=0;logOutput=C:\\;"
//                  L"responseTimeout=1;"
//                : L"Driver={Elasticsearch ODBC};"
//                  L"host=8.8.8.8;port=9200;"
//                  L"UID=admin;PWD=admin;auth=IAM;"
//                  L"logLevel=0;logOutput=C:\\;"
//                  L"responseTimeout=1;";
//
//    SQLRETURN ret = SQLDriverConnect(
//        m_conn, NULL, (SQLTCHAR*)invalid_host_conn_string.c_str(), SQL_NTS,
//        m_out_conn_string, IT_SIZEOF(m_out_conn_string),
//        &m_out_conn_string_length, SQL_DRIVER_COMPLETE);
//    EXPECT_EQ(SQL_ERROR, ret);
//}
//
//TEST_F(TestSQLDriverConnect, InvalidPort) {
//    test_string invalid_port_conn_string =
//        use_ssl ? L"Driver={Elasticsearch ODBC};"
//                  L"host=https://localhost;port=5432;"
//                  L"UID=admin;PWD=admin;auth=IAM;"
//                  L"logLevel=0;logOutput=C:\\;"
//                  L"responseTimeout=10;"
//                : L"Driver={Elasticsearch ODBC};"
//                  L"host=localhost;port=5432;"
//                  L"UID=admin;PWD=admin;auth=IAM;"
//                  L"logLevel=0;logOutput=C:\\;"
//                  L"responseTimeout=10;";
//
//    SQLRETURN ret = SQLDriverConnect(
//        m_conn, NULL, (SQLTCHAR*)invalid_port_conn_string.c_str(), SQL_NTS,
//        m_out_conn_string, IT_SIZEOF(m_out_conn_string),
//        &m_out_conn_string_length, SQL_DRIVER_COMPLETE);
//    EXPECT_EQ(SQL_ERROR, ret);
//}
//
//// TODO #41 - Revisit when parser code
//// This should return SQL_SUCCESS_WITH_INFO (SQLSTATE 01S00 - Invalid connection
//// string attribute)
//TEST_F(TestSQLDriverConnect, UnsupportedKeyword) {
//    test_string unsupported_keyword_conn_string =
//        use_ssl ? L"Driver={Elasticsearch ODBC};"
//                  L"host=https://localhost;port=5432;"
//                  L"UID=admin;PWD=admin;auth=IAM;"
//                  L"logLevel=0;logOutput=C:\\;"
//                  L"responseTimeout=10;extra=1"
//                : L"Driver={Elasticsearch ODBC};"
//                  L"host=localhost;port=5432;"
//                  L"UID=admin;PWD=admin;auth=IAM;"
//                  L"logLevel=0;logOutput=C:\\;"
//                  L"responseTimeout=10;extra=1";
//
//    SQLRETURN ret = SQLDriverConnect(
//        m_conn, NULL, (SQLTCHAR*)unsupported_keyword_conn_string.c_str(),
//        SQL_NTS, m_out_conn_string, IT_SIZEOF(m_out_conn_string),
//        &m_out_conn_string_length, SQL_DRIVER_COMPLETE);
//    EXPECT_EQ(SQL_ERROR, ret);
//}
//
//TEST_F(TestSQLDriverConnect, ConnStringAbbrevsUID) {
//    test_string abbrev_str =
//        use_ssl ? L"Driver={Elasticsearch ODBC};"
//                  L"host=https://localhost;port=9200;"
//                  L"UID=admin;PWD=admin;auth=IAM;"
//                  L"logLevel=0;logOutput=C:\\;"
//                  L"responseTimeout=10;"
//                : L"Driver={Elasticsearch ODBC};"
//                  L"host=localhost;port=9200;"
//                  L"UID=admin;PWD=admin;auth=IAM;"
//                  L"logLevel=0;logOutput=C:\\;"
//                  L"responseTimeout=10;";
//
//    SQLRETURN ret =
//        SQLDriverConnect(m_conn, NULL, (SQLTCHAR*)abbrev_str.c_str(), SQL_NTS,
//                         m_out_conn_string, IT_SIZEOF(m_out_conn_string),
//                         &m_out_conn_string_length, SQL_DRIVER_NOPROMPT);
//    EXPECT_EQ(SQL_SUCCESS, ret);
//}
//
//TEST_F(TestSQLDriverConnect, ConnStringAbbrevsPWD) {
//    test_string abbrev_str =
//        use_ssl ? L"Driver={Elasticsearch ODBC};"
//                  L"host=https://localhost;port=9200;"
//                  L"UID=admin;PWD=admin;auth=IAM;"
//                  L"logLevel=0;logOutput=C:\\;"
//                  L"responseTimeout=10;"
//                : L"Driver={Elasticsearch ODBC};"
//                  L"host=localhost;port=9200;"
//                  L"UID=admin;PWD=admin;auth=IAM;"
//                  L"logLevel=0;logOutput=C:\\;"
//                  L"responseTimeout=10;";
//
//    SQLRETURN ret =
//        SQLDriverConnect(m_conn, NULL, (SQLTCHAR*)abbrev_str.c_str(), SQL_NTS,
//                         m_out_conn_string, IT_SIZEOF(m_out_conn_string),
//                         &m_out_conn_string_length, SQL_DRIVER_NOPROMPT);
//    EXPECT_EQ(SQL_SUCCESS, ret);
//}
//
//TEST_F(TestSQLDriverConnect, ConnStringAbbrevsUIDPWD) {
//    test_string abbrev_str =
//        use_ssl ? L"Driver={Elasticsearch ODBC};"
//                  L"host=https://localhost;port=9200;"
//                  L"UID=admin;PWD=admin;auth=IAM;"
//                  L"logLevel=0;logOutput=C:\\;"
//                  L"responseTimeout=10;"
//                : L"Driver={Elasticsearch ODBC};"
//                  L"host=localhost;port=9200;"
//                  L"UID=admin;PWD=admin;auth=IAM;"
//                  L"logLevel=0;logOutput=C:\\;"
//                  L"responseTimeout=10;";
//
//    SQLRETURN ret =
//        SQLDriverConnect(m_conn, NULL, (SQLTCHAR*)abbrev_str.c_str(), SQL_NTS,
//                         m_out_conn_string, IT_SIZEOF(m_out_conn_string),
//                         &m_out_conn_string_length, SQL_DRIVER_NOPROMPT);
//    EXPECT_EQ(SQL_SUCCESS, ret);
//}
//
//TEST_F(TestSQLDriverConnect, ConnStringAbbrevsServer) {
//    test_string abbrev_str =
//        use_ssl ? L"Driver={Elasticsearch ODBC};"
//                  L"UID=admin;PWD=admin;auth=IAM;"
//                  L"logLevel=0;logOutput=C:\\;"
//                  L"responseTimeout=10;"
//                : L"Driver={Elasticsearch ODBC};"
//                  L"UID=admin;PWD=admin;auth=IAM;"
//                  L"logLevel=0;logOutput=C:\\;"
//                  L"responseTimeout=10;";
//
//    SQLRETURN ret =
//        SQLDriverConnect(m_conn, NULL, (SQLTCHAR*)abbrev_str.c_str(), SQL_NTS,
//                         m_out_conn_string, IT_SIZEOF(m_out_conn_string),
//                         &m_out_conn_string_length, SQL_DRIVER_NOPROMPT);
//    EXPECT_EQ(SQL_SUCCESS, ret);
//}
//
//TEST_F(TestSQLDriverConnect, ConnStringAbbrevsServerUIDPWD) {
//    test_string abbrev_str =
//        use_ssl ? L"Driver={Elasticsearch ODBC};"
//                  L"UID=admin;PWD=admin;auth=IAM;"
//                  L"logLevel=0;logOutput=C:\\;"
//                  L"responseTimeout=10;"
//                : L"Driver={Elasticsearch ODBC};"
//                  L"UID=admin;PWD=admin;auth=IAM;"
//                  L"logLevel=0;logOutput=C:\\;"
//                  L"responseTimeout=10;";
//
//    SQLRETURN ret =
//        SQLDriverConnect(m_conn, NULL, (SQLTCHAR*)abbrev_str.c_str(), SQL_NTS,
//                         m_out_conn_string, IT_SIZEOF(m_out_conn_string),
//                         &m_out_conn_string_length, SQL_DRIVER_NOPROMPT);
//    EXPECT_EQ(SQL_SUCCESS, ret);
//}
//
//TEST_F(TestSQLDriverConnect, Timeout1Second) {
//    test_string one_second_timeout =
//        use_ssl ? L"Driver={Elasticsearch ODBC};"
//                  L"host=https://8.8.8.8;port=9200;"
//                  L"UID=admin;PWD=admin;auth=IAM;"
//                  L"logLevel=0;logOutput=C:\\;"
//                  L"responseTimeout=1;"
//                : L"Driver={Elasticsearch ODBC};"
//                  L"host=8.8.8.8;port=9200;"
//                  L"UID=admin;PWD=admin;auth=IAM;"
//                  L"logLevel=0;logOutput=C:\\;"
//                  L"responseTimeout=1;";
//
//    auto start = std::chrono::steady_clock::now();
//    SQLRETURN ret = SQLDriverConnect(
//        m_conn, NULL, (SQLTCHAR*)one_second_timeout.c_str(), SQL_NTS,
//        m_out_conn_string, IT_SIZEOF(m_out_conn_string),
//        &m_out_conn_string_length, SQL_DRIVER_COMPLETE);
//    auto end = std::chrono::steady_clock::now();
//    auto time =
//        std::chrono::duration_cast< std::chrono::milliseconds >(end - start)
//            .count();
//    std::cout << "TIME: " << time << std::endl;
//    EXPECT_EQ(SQL_ERROR, ret);
//#ifdef WIN32
//    // Windows rounds up to nearest 4s with timeout, another user reported this
//    // issue:
//    // https://social.msdn.microsoft.com/Forums/vstudio/en-US/42ae1b2f-b120-4b46-9417-e594c3d02a5f/does-winhttpsettimeouts-support-small-timeouts?forum=vcgeneral
//    EXPECT_GT(time, 3400);
//    EXPECT_LT(time, 4500);
//#else
//    EXPECT_GT(time, 500);
//    EXPECT_LT(time, 1500);
//#endif
//}
//
//TEST_F(TestSQLDriverConnect, Timeout3Second) {
//    test_string one_second_timeout =
//        use_ssl ? L"Driver={Elasticsearch ODBC};"
//                  L"host=https://8.8.8.8;port=9200;"
//                  L"UID=admin;PWD=admin;auth=IAM;"
//                  L"logLevel=0;logOutput=C:\\;"
//                  L"responseTimeout=3;"
//                : L"Driver={Elasticsearch ODBC};"
//                  L"host=8.8.8.8;port=9200;"
//                  L"UID=admin;PWD=admin;auth=IAM;"
//                  L"logLevel=0;logOutput=C:\\;"
//                  L"responseTimeout=3;";
//
//    auto start = std::chrono::steady_clock::now();
//    SQLRETURN ret = SQLDriverConnect(
//        m_conn, NULL, (SQLTCHAR*)one_second_timeout.c_str(), SQL_NTS,
//        m_out_conn_string, IT_SIZEOF(m_out_conn_string),
//        &m_out_conn_string_length, SQL_DRIVER_COMPLETE);
//    auto end = std::chrono::steady_clock::now();
//    auto time =
//        std::chrono::duration_cast< std::chrono::milliseconds >(end - start)
//            .count();
//    std::cout << "TIME: " << time << std::endl;
//    EXPECT_EQ(SQL_ERROR, ret);
//#ifdef WIN32
//    // Windows rounds up to nearest 4s with timeout, another user reported this
//    // issue:
//    // https://social.msdn.microsoft.com/Forums/vstudio/en-US/42ae1b2f-b120-4b46-9417-e594c3d02a5f/does-winhttpsettimeouts-support-small-timeouts?forum=vcgeneral
//    EXPECT_GT(time, 3500);
//    EXPECT_LT(time, 4500);
//#else
//    EXPECT_GT(time, 2500);
//    EXPECT_LT(time, 3500);
//#endif
//}
//
//TEST_F(TestSQLDriverConnect, Timeout7Second) {
//    test_string seven_second_timeout =
//        use_ssl ? L"Driver={Elasticsearch ODBC};"
//                  L"host=https://8.8.8.8;port=9200;"
//                  L"UID=admin;PWD=admin;auth=IAM;"
//                  L"logLevel=0;logOutput=C:\\;"
//                  L"responseTimeout=7;"
//                : L"Driver={Elasticsearch ODBC};"
//                  L"host=8.8.8.8;port=9200;"
//                  L"UID=admin;PWD=admin;auth=IAM;"
//                  L"logLevel=0;logOutput=C:\\;"
//                  L"responseTimeout=7;";
//
//    auto start = std::chrono::steady_clock::now();
//    SQLRETURN ret = SQLDriverConnect(
//        m_conn, NULL, (SQLTCHAR*)seven_second_timeout.c_str(), SQL_NTS,
//        m_out_conn_string, IT_SIZEOF(m_out_conn_string),
//        &m_out_conn_string_length, SQL_DRIVER_COMPLETE);
//    auto end = std::chrono::steady_clock::now();
//    auto time =
//        std::chrono::duration_cast< std::chrono::milliseconds >(end - start)
//            .count();
//    std::cout << "TIME: " << time << std::endl;
//    EXPECT_EQ(SQL_ERROR, ret);
//#ifdef WIN32
//    // Windows rounds up to nearest 4s with timeout, another user reported this
//    // issue:
//    // https://social.msdn.microsoft.com/Forums/vstudio/en-US/42ae1b2f-b120-4b46-9417-e594c3d02a5f/does-winhttpsettimeouts-support-small-timeouts?forum=vcgeneral
//    EXPECT_GT(time, 7500);
//    EXPECT_LT(time, 8500);
//#else
//    EXPECT_GT(time, 6500);
//    EXPECT_LT(time, 7500);
//#endif
//}
//
//class TestSQLDisconnect : public testing::Test {
//   public:
//    TestSQLDisconnect() {
//    }
//
//    void SetUp() {
//    }
//
//    void TearDown() {
//        if (m_conn != SQL_NULL_HDBC) {
//            SQLFreeHandle(SQL_HANDLE_DBC, m_conn);
//        }
//        SQLFreeHandle(SQL_HANDLE_ENV, m_env);
//    }
//
//    ~TestSQLDisconnect() {
//        // cleanup any pending stuff, but no exceptions allowed
//    }
//
//    SQLHENV m_env = SQL_NULL_HENV;
//    SQLHDBC m_conn = SQL_NULL_HDBC;
//};
//
//TEST_F(TestSQLDisconnect, TestSuccess) {
//    ASSERT_NO_THROW(ITDriverConnect((SQLTCHAR*)(conn_string().c_str()), &m_env,
//                                    &m_conn, true, true));
//    EXPECT_EQ(SQL_SUCCESS, SQLDisconnect(m_conn));
//}
//
//TEST_F(TestSQLDisconnect, TestReconnectOnce) {
//    for (int i = 0; i <= 1; i++) {
//        ASSERT_NO_THROW((ITDriverConnect((SQLTCHAR*)(conn_string().c_str()), &m_env,
//                                         &m_conn, true, true)));
//        EXPECT_EQ(SQL_SUCCESS, SQLDisconnect(m_conn));
//    }
//}
//
//TEST_F(TestSQLDisconnect, TestReconnectMultipleTimes) {
//    for (int i = 0; i <= 10; i++) {
//        ASSERT_NO_THROW((ITDriverConnect((SQLTCHAR*)(conn_string().c_str()), &m_env,
//                                         &m_conn, true, true)));
//        EXPECT_EQ(SQL_SUCCESS, SQLDisconnect(m_conn));
//    }
//}
//
//TEST_F(TestSQLDisconnect, TestDisconnectWithoutConnect) {
//    ASSERT_NO_THROW(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_env));
//    ASSERT_NO_THROW(SQLAllocHandle(SQL_HANDLE_DBC, m_env, &m_conn));
//    EXPECT_EQ(SQL_ERROR, SQLDisconnect(m_conn));
//}
//

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
   ::testing::InitGoogleTest(&argc, argv);

   int failures = RUN_ALL_TESTS();

   // TODO: Fix
   std::string output = "No output";
   std::cout << output << std::endl;
   std::cout << (failures ? "Not all tests passed." : "All tests passed.")
             << std::endl;
   WriteFileIfSpecified(argv, argv + argc, "-fout", output);

#ifdef __APPLE__
   // Disable malloc logging and report memory leaks
   system("unset MallocStackLogging");
   system("leaks itodbc_connection > leaks_itodbc_connection");
#endif
   return failures;
}
