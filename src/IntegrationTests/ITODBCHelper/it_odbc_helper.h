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

#ifndef IT_ODBC_HELPER_H
#define IT_ODBC_HELPER_H

#ifdef WIN32
#include <windows.h>
#endif
#include <sql.h>
#include <sqlext.h>

#include <iostream>
#include <vector>

#include "unit_test_helper.h"

// SQLSTATEs
#define SQLSTATE_STRING_DATA_RIGHT_TRUNCATED (SQLWCHAR*)L"01004"
#define SQLSTATE_RESTRICTED_DATA_TYPE_ERROR (SQLWCHAR*)L"07006"
#define SQLSTATE_COLUMN_ERROR (SQLWCHAR*)L"07009"
#define SQLSTATE_INVALID_DESCRIPTOR_INDEX (SQLWCHAR*)L"07009"
#define SQLSTATE_GENERAL_ERROR (SQLWCHAR*)L"HY000"
#define SQLSTATE_MEMORY_ALLOCATION_ERROR (SQLWCHAR*)L"HY001"
#define SQLSTATE_INVALID_STRING_OR_BUFFER_LENGTH (SQLWCHAR*)L"HY090"
#define SQLSTATE_INVALID_DESCRIPTOR_FIELD_IDENTIFIER (SQLWCHAR*)L"HY091"
#define SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE (SQLWCHAR*) L"HY019"
#define SQLSTATE_NOT_IMPLEMENTED_ERROR (SQLWCHAR*)L"HYC00"
#define SQLSTATE_STRING_CONVERSION_ERROR (SQLWCHAR*)L"22018"
#define SQLSTATE_INVALID_CURSUR_STATE (SQLWCHAR*)L"07005"
#define SQLSTATE_FRACTIONAL_TRUNCATION (SQLWCHAR*)L"01S07"

#define IT_SIZEOF(x) (NULL == (x) ? 0 : (sizeof((x)) / sizeof((x)[0])))

#define IT_DRIVER L"Driver"
#define IT_ACCESSKEYID L"AccessKeyId"
#define IT_SECRETACCESSKEY L"SecretAccessKey"
#define IT_REGION L"Region"
#define IT_AUTH L"Auth"
#define IT_LOGLEVEL L"LogLevel"
#define IT_LOGOUTPUT L"LogOutput"

std::vector< std::pair< std::wstring, std::wstring > > conn_str_pair = {
    {IT_DRIVER, L"timestreamodbc"},
    {IT_REGION, L"us-east-1"},
    {IT_AUTH, L"AWS_PROFILE"},
    {IT_LOGLEVEL, L"7"},
#ifdef __APPLE__
    {IT_LOGOUTPUT, L"/tmp/"}
#else
    {IT_LOGOUTPUT, L"C:\\"}
#endif
    };

std::wstring conn_string = []() {
    std::wstring temp;
    for (auto it : conn_str_pair)
        temp += it.first + L"=" + it.second + L";";
    return temp;
}();

void AllocConnection(SQLHENV* db_environment, SQLHDBC* db_connection,
                     bool throw_on_error, bool log_diag);
void ITDriverConnect(SQLTCHAR* connection_string, SQLHENV* db_environment,
                     SQLHDBC* db_connection, bool throw_on_error,
                     bool log_diag);
void AllocStatement(SQLTCHAR* connection_string, SQLHENV* db_environment,
                    SQLHDBC* db_connection, SQLHSTMT* h_statement,
                    bool throw_on_error, bool log_diag);
void LogAnyDiagnostics(SQLSMALLINT handle_type, SQLHANDLE handle, SQLRETURN ret,
                       SQLTCHAR* msg_return = NULL, const SQLSMALLINT sz = 0);
bool CheckSQLSTATE(SQLSMALLINT handle_type, SQLHANDLE handle,
                   SQLWCHAR* expected_sqlstate, bool log_message);
bool CheckSQLSTATE(SQLSMALLINT handle_type, SQLHANDLE handle,
                   SQLWCHAR* expected_sqlstate);
std::wstring QueryBuilder(const std::wstring& column,
                          const std::wstring& dataset,
                          const std::wstring& count);
std::wstring QueryBuilder(const std::wstring& column,
                          const std::wstring& dataset);
void CloseCursor(SQLHSTMT* h_statement, bool throw_on_error, bool log_diag);
std::string u16string_to_string(const std::u16string& src);
std::u16string string_to_u16string(const std::string& src);

#endif
