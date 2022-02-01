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

#include "it_odbc_helper.h"

#define EXECUTION_HANDLER(throw_on_error, log_diag, handle_type, handle,    \
                          ret_code, statement, error_msg)                   \
  do {                                                                      \
    (ret_code) = (statement);                                               \
    if ((log_diag)) LogAnyDiagnostics((handle_type), (handle), (ret_code)); \
    if ((throw_on_error) && !SQL_SUCCEEDED((ret_code)))                     \
      throw std::runtime_error((error_msg));                                \
  } while (0);

#ifdef __linux__
test_string to_test_string(const std::string& src) {
  return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}
      .from_bytes(src);
}
#else
test_string to_test_string(const std::string& src) {
  return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>{}.from_bytes(
      src);
}
#endif

void LogAnyDiagnostics(SQLSMALLINT handle_type, SQLHANDLE handle, SQLRETURN ret,
                       SQLTCHAR* msg_return, const SQLSMALLINT sz) {
  if (handle == NULL) {
    printf("Failed to log diagnostics, handle is NULL\n");
    return;
  }

  // Only log diagnostics when there's something to log.
  switch (ret) {
    case SQL_SUCCESS_WITH_INFO:
      printf("SQL_SUCCESS_WITH_INFO: ");
      break;
    case SQL_ERROR:
      printf("SQL_ERROR: ");
      break;
    default:
      return;
  }

  SQLRETURN diag_ret;
  SQLTCHAR sqlstate[6];
  SQLINTEGER native_error_code;
  SQLTCHAR diag_message[SQL_MAX_MESSAGE_LENGTH];
  SQLSMALLINT message_length;

  SQLSMALLINT rec_number = 0;
  do {
    rec_number++;
    diag_ret = SQLGetDiagRec(
        handle_type, handle, rec_number, sqlstate, &native_error_code,
        msg_return == NULL ? diag_message : msg_return,
        msg_return == NULL ? IT_SIZEOF(diag_message) : sz, &message_length);
    std::string diag_str =
        tchar_to_string((msg_return == NULL) ? diag_message : msg_return);
    std::string state_str = tchar_to_string(sqlstate);
    if (diag_ret == SQL_INVALID_HANDLE)
      printf("Invalid handle\n");
    else if (SQL_SUCCEEDED(diag_ret))
      printf("SQLState: %s: %s\n", state_str.c_str(), diag_str.c_str());
  } while (diag_ret == SQL_SUCCESS);

  if (diag_ret == SQL_NO_DATA && rec_number == 1)
    printf("No error information\n");
}

void CloseCursor(SQLHSTMT* h_statement, bool throw_on_error, bool log_diag) {
  SQLRETURN ret_code;
  EXECUTION_HANDLER(throw_on_error, log_diag, SQL_HANDLE_STMT, *h_statement,
                    ret_code, SQLCloseCursor(*h_statement),
                    "Failed to set allocate handle for statement.");
}

std::string wstring_to_string(const std::wstring& src) {
  return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>{}.to_bytes(
      src);
}

std::string u16string_to_string(const std::u16string& src) {
  return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}
      .to_bytes(src);
}

std::string u32string_to_string(const std::u32string& src) {
  return std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}.to_bytes(
      src);
}

std::u16string string_to_u16string(const std::string& src) {
  return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}
      .from_bytes(src);
}

std::string tchar_to_string(const SQLTCHAR* tchar) {
  if constexpr (sizeof(SQLTCHAR) == 2) {
    std::u16string temp((const char16_t*)tchar);
    return u16string_to_string(temp);
  } else if constexpr (sizeof(SQLTCHAR) == 4) {
    std::u32string temp((const char32_t*)tchar);
    return u32string_to_string(temp);
  } else {
    return std::string((const char*)tchar);
  }
}

std::string wchar_to_string(const SQLWCHAR* tchar) {
  if constexpr (sizeof(SQLWCHAR) == 2) {
    std::u16string temp((const char16_t*)tchar);
    return u16string_to_string(temp);
  } else if constexpr (sizeof(SQLWCHAR) == 4) {
    std::u32string temp((const char32_t*)tchar);
    return u32string_to_string(temp);
  } else {
    return std::string((const char*)tchar);
  }
}