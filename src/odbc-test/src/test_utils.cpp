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

  // On Windows, reallen is in bytes, on Linux reallen is in chars.
  // Can't rely on returned length.
  SQLGetDiagRec(handleType, handle, 1, sqlstate, &nativeCode, message,
                ODBC_BUFFER_SIZE, &reallen);

  return OdbcClientError(utility::SqlWcharToString(sqlstate),
                         utility::SqlWcharToString(message));
}

std::string GetOdbcErrorState(SQLSMALLINT handleType, SQLHANDLE handle,
                              int idx) {
  SQLWCHAR sqlstate[7] = {};
  SQLINTEGER nativeCode;

  SQLWCHAR message[ODBC_BUFFER_SIZE];
  SQLSMALLINT reallen = 0;

  // On Windows, reallen is in bytes, on Linux reallen is in chars.
  // Can't rely on returned length.
  SQLGetDiagRec(handleType, handle, idx, sqlstate, &nativeCode, message,
                ODBC_BUFFER_SIZE, &reallen);

  return utility::SqlWcharToString(sqlstate);
}

std::string GetOdbcErrorMessage(SQLSMALLINT handleType, SQLHANDLE handle,
                                int idx) {
  SQLWCHAR sqlstate[7] = {};
  SQLINTEGER nativeCode;

  SQLWCHAR message[ODBC_BUFFER_SIZE];
  SQLSMALLINT reallen = 0;

  // On Windows, reallen is in bytes, on Linux reallen is in chars.
  // Can't rely on returned length.
  SQLGetDiagRec(handleType, handle, idx, sqlstate, &nativeCode, message,
                ODBC_BUFFER_SIZE, &reallen);

  std::string res = utility::SqlWcharToString(sqlstate);

  if (!res.empty()) {
    res.append(": ").append(utility::SqlWcharToString(message));
  } else {
    res = "No results";
  }

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
}  // namespace ignite_test
