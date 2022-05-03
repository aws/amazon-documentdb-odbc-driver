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

#include <ignite/odbc/common/concurrent.h>
#include <ignite/odbc/jni/java.h>
#include <ignite/odbc/jni/result_set.h>
#include <ignite/odbc/log.h>

using ignite::odbc::common::concurrent::SharedPointer;
using ignite::odbc::jni::java::GlobalJObject;
using ignite::odbc::jni::java::JniContext;
using ignite::odbc::jni::java::JniErrorInfo;

namespace ignite {
namespace odbc {
namespace jni {
ResultSet::ResultSet(SharedPointer< JniContext >& jniContext,
                     SharedPointer< GlobalJObject >& resultSet)
    : _jniContext(jniContext), _resultSet(resultSet) {
}

ResultSet::~ResultSet() {
  JniErrorInfo errInfo;
  Close(errInfo);
  if (_jniContext.IsValid()) {
    _jniContext = nullptr;
  }
}

JniErrorCode ResultSet::Close(JniErrorInfo& errInfo) {
  if (_jniContext.IsValid() && _resultSet.IsValid()) {
    JniErrorCode success =
        _jniContext.Get()->ResultSetClose(_resultSet, errInfo);
    if (errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      LOG_MSG(errInfo.errMsg);
    }
    _resultSet = nullptr;
    return success;
  }
  return JniErrorCode::IGNITE_JNI_ERR_SUCCESS;
}

JniErrorCode ResultSet::Next(bool& hasNext, JniErrorInfo& errInfo) {
  return _jniContext.Get()->ResultSetNext(_resultSet, hasNext, errInfo);
}

JniErrorCode ResultSet::GetString(const int columnIndex,
                                  boost::optional< std::string >& value,
                                  JniErrorInfo& errInfo) {
  return _jniContext.Get()->ResultSetGetString(_resultSet, columnIndex, value,
                                               errInfo);
}

JniErrorCode ResultSet::GetString(const std::string& columnName,
                                  boost::optional< std::string >& value,
                                  JniErrorInfo& errInfo) {
  return _jniContext.Get()->ResultSetGetString(_resultSet, columnName, value,
                                               errInfo);
}

JniErrorCode ResultSet::GetInt(const int columnIndex,
                               boost::optional< int >& value,
                               JniErrorInfo& errInfo) {
  return _jniContext.Get()->ResultSetGetInt(_resultSet, columnIndex, value,
                                            errInfo);
}

JniErrorCode ResultSet::GetInt(const std::string& columnName,
                               boost::optional< int >& value,
                               JniErrorInfo& errInfo) {
  return _jniContext.Get()->ResultSetGetInt(_resultSet, columnName, value,
                                            errInfo);
}
}  // namespace jni
}  // namespace odbc
}  // namespace ignite
