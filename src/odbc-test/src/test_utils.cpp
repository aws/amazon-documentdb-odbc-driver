/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "test_utils.h"

#include <ignite/odbc/common/platform_utils.h>
#include <ignite/odbc/utility.h>

#include <boost/test/unit_test.hpp>
#include <cassert>

#include "ignite/odbc/jni/utils.h"

using namespace ignite::odbc;

namespace ignite_test {
OdbcClientError GetOdbcError(SQLSMALLINT handleType, SQLHANDLE handle) {
  SQLWCHAR sqlstate[7] = {};
  SQLINTEGER nativeCode;

  SQLWCHAR message[ODBC_BUFFER_SIZE];
  SQLSMALLINT reallen = 0;

  SQLGetDiagRec(handleType, handle, 1, sqlstate, &nativeCode, message,
                sizeof(message), &reallen);

  return OdbcClientError(utility::SqlStringToString(sqlstate, SQL_NTS),
                         utility::SqlStringToString(message, reallen, true));
}

std::string GetOdbcErrorState(SQLSMALLINT handleType, SQLHANDLE handle,
                              int idx) {
  SQLWCHAR sqlstate[7] = {};
  SQLINTEGER nativeCode;

  SQLWCHAR message[ODBC_BUFFER_SIZE];
  SQLSMALLINT reallen = 0;

  SQLGetDiagRec(handleType, handle, idx, sqlstate, &nativeCode, message,
                sizeof(message), &reallen);

  return utility::SqlStringToString(sqlstate, SQL_NTS);
}

std::string GetOdbcErrorMessage(SQLSMALLINT handleType, SQLHANDLE handle,
                                int idx) {
  SQLWCHAR sqlstate[7] = {};
  SQLINTEGER nativeCode;

  SQLWCHAR message[ODBC_BUFFER_SIZE];
  SQLSMALLINT reallen = 0;

  SQLGetDiagRec(handleType, handle, idx, sqlstate, &nativeCode, message,
                sizeof(message), &reallen);

  std::string res = utility::SqlStringToString(sqlstate, SQL_NTS);

  if (!res.empty())
    // In bytes
    res.append(": ").append(utility::SqlStringToString(message, reallen, true));
  else
    res = "No results";

  return res;
}

std::string GetTestConfigDir() {
  using namespace ignite::odbc;

  std::string cfgPath = common::GetEnv("IGNITE_NATIVE_TEST_ODBC_CONFIG_PATH");

  if (!cfgPath.empty())
    return cfgPath;

  std::string home = jni::ResolveDocumentDbHome();

  if (home.empty())
    return home;

  std::stringstream path;

  path << home << common::Fs << "modules" << common::Fs << "platforms"
       << common::Fs << "cpp" << common::Fs << "odbc-test" << common::Fs
       << "config";

  return path.str();
}

std::string AppendPath(const std::string& base, const std::string& toAdd) {
  std::stringstream stream;

  stream << base << ignite::odbc::common::Fs << toAdd;

  return stream.str();
}

void ClearLfs() {
  std::string home = ignite::odbc::jni::ResolveDocumentDbHome();
  std::string workDir = AppendPath(home, "work");

  ignite::odbc::common::DeletePath(workDir);
}

std::vector< SQLWCHAR > NewSqlWchar(const std::wstring& value) {
  std::vector< SQLWCHAR > result(value.size() + 1);
  for (int i = 0; i < value.size(); i++) {
    result[i] = value[i];
  }
  result[value.size()] = 0;
  return result;
}

std::wstring FromSQLWCHAR(const SQLWCHAR* value, const size_t len) {
  std::wstring result;
  if (!value || (len != SQL_NTS) && len <= 0) {
    return result;
  }

  if (len == SQL_NTS) {
    for (int i = 0; value[i] != 0; i++) {
      result.push_back(value[i]);
    }
  } else {
    size_t charsToCopy = len / sizeof(SQLWCHAR);
    for (int i = 0; i < charsToCopy; i++) {
      result.push_back(value[i]);
    }
  }

  return result;
}
}  // namespace ignite_test
