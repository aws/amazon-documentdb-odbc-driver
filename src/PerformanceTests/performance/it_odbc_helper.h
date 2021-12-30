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

#ifdef __linux__
#include <climits>
#endif

#ifdef WIN32
#include <Windows.h>
#endif

#pragma comment(lib, "odbc32.lib")

#ifndef ODBCVER
#define ODBCVER 0x0351
#endif

#include <odbcinst.h>
#include <sql.h>
#include <sqlext.h>


#include <codecvt>
#include <iostream>
#include <locale>
#include <string>
#include <vector>

#include <fstream>
#include <algorithm>

#ifdef __linux__
typedef std::u16string test_string;
#define CREATE_STRING(str) u"" str
#else
typedef std::wstring test_string;
#define CREATE_STRING(str) L"" str
#endif

#define AS_SQLTCHAR(str) \
  const_cast<SQLTCHAR*>(reinterpret_cast<const SQLTCHAR*>(str))

#define convert_to_test_string(t) to_test_string(std::to_string(t))
test_string to_test_string(const std::string& src);

// SQLSTATEs
#define SQLSTATE_STRING_DATA_RIGHT_TRUNCATED (SQLWCHAR*)CREATE_STRING("01004")
#define SQLSTATE_RESTRICTED_DATA_TYPE_ERROR (SQLWCHAR*)CREATE_STRING("07006")
#define SQLSTATE_INVALID_DESCRIPTOR_INDEX (SQLWCHAR*)CREATE_STRING("07009")
#define SQLSTATE_GENERAL_ERROR (SQLWCHAR*)CREATE_STRING("HY000")
#define SQLSTATE_MEMORY_ALLOCATION_ERROR (SQLWCHAR*)CREATE_STRING("HY001")
#define SQLSTATE_INVALID_STRING_OR_BUFFER_LENGTH \
  (SQLWCHAR*)CREATE_STRING("HY090")
#define SQLSTATE_INVALID_DESCRIPTOR_FIELD_IDENTIFIER \
  (SQLWCHAR*)CREATE_STRING("HY091")
#define SQLSTATE_NUMERIC_VALUE_OUT_OF_RANGE (SQLWCHAR*)CREATE_STRING("HY019")
#define SQLSTATE_NOT_IMPLEMENTED_ERROR (SQLWCHAR*)CREATE_STRING("HYC00")
#define SQLSTATE_STRING_CONVERSION_ERROR (SQLWCHAR*)CREATE_STRING("22018")
#define SQLSTATE_INVALID_CURSUR_STATE (SQLWCHAR*)CREATE_STRING("07005")
#define SQLSTATE_FRACTIONAL_TRUNCATION (SQLWCHAR*)CREATE_STRING("01S07")
#define SQLSTATE_CANNOT_MODIFY_IRD (SQLWCHAR*)CREATE_STRING("HY016")
#define SQLSTATE_SEQUENCE_ERROR (SQLWCHAR*)CREATE_STRING("HY010")
#define SQLSTATE_INVALID_USE_OF_NULL_POINTER (SQLWCHAR*)CREATE_STRING("HY009")
#define SQLSTATE_OPERATION_CANCELLED (SQLWCHAR*)CREATE_STRING("HY008")

#define IT_SIZEOF(x) (NULL == (x) ? 0 : (sizeof((x)) / sizeof((x)[0])))

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
test_string QueryBuilder(const test_string& column, const test_string& dataset,
                         const test_string& count);
test_string QueryBuilder(const test_string& column, const test_string& dataset);
void CloseCursor(SQLHSTMT* h_statement, bool throw_on_error, bool log_diag);
std::string wstring_to_string(const std::wstring& src);
std::string u16string_to_string(const std::u16string& src);
std::string u32string_to_string(const std::u32string& src);
std::u16string string_to_u16string(const std::string& src);
std::string tchar_to_string(const SQLTCHAR* tchar);
std::string wchar_to_string(const SQLWCHAR* tchar);
void WriteFileIfSpecified(char** begin, char** end, const std::string& option,
                          std::string& output);

#endif