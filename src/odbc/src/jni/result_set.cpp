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

#include <documentdb/odbc/common/concurrent.h>
#include <documentdb/odbc/jni/java.h>
#include <documentdb/odbc/jni/result_set.h>
#include <documentdb/odbc/log.h>

using documentdb::odbc::common::concurrent::SharedPointer;
using documentdb::odbc::jni::java::GlobalJObject;
using documentdb::odbc::jni::java::JniContext;
using documentdb::odbc::jni::java::JniErrorInfo;

namespace documentdb {
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
    if (errInfo.code != JniErrorCode::DOCUMENTDB_JNI_ERR_SUCCESS) {
      LOG_MSG(errInfo.errMsg);
    }
    _resultSet = nullptr;
    return success;
  }
  return JniErrorCode::DOCUMENTDB_JNI_ERR_SUCCESS;
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

JniErrorCode ResultSet::GetSmallInt(const int columnIndex,
                                    boost::optional< int16_t >& value,
                                    JniErrorInfo& errInfo) {
  boost::optional< int > val;
  JniErrorCode err =
      _jniContext.Get()->ResultSetGetInt(_resultSet, columnIndex, val, errInfo);
  if (val)
    value = static_cast< int16_t >(*val);
  return err;
}

JniErrorCode ResultSet::GetSmallInt(const std::string& columnName,
                                    boost::optional< int16_t >& value,
                                    JniErrorInfo& errInfo) {
  boost::optional< int > val;
  JniErrorCode err =
      _jniContext.Get()->ResultSetGetInt(_resultSet, columnName, val, errInfo);
  if (val)
    value = static_cast< int16_t >(*val);
  return err;
}
}  // namespace jni
}  // namespace odbc
}  // namespace documentdb
