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

#include <ignite/odbc/common/platform_utils.h>
#include <ignite/odbc/ignite_error.h>
#include <ignite/odbc/jni/documentdb_connection.h>
#include <ignite/odbc/jni/utils.h>
#include <ignite/odbc/log.h>

using ignite::odbc::common::GetEnv;
using ignite::odbc::jni::java::JniErrorCode;
using ignite::odbc::jni::java::JniErrorInfo;

namespace ignite {
namespace odbc {
namespace jni {
JniErrorCode DocumentDbConnection::Open(const Configuration& config,
                                        JniErrorInfo& errInfo) {
  bool connected = false;

  if (!_jniContext.IsValid()) {
    errInfo.errMsg = new char[]{"Unable to get initialized JVM."};
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_JVM_INIT;
    return errInfo.code;
  }
  if (_connection.IsValid()) {
    return JniErrorCode::IGNITE_JNI_ERR_SUCCESS;
  }

  std::string connectionString = config.ToJdbcConnectionString();

  SharedPointer< GlobalJObject > result;
  JniErrorCode success = _jniContext.Get()->DriverManagerGetConnection(
      connectionString.c_str(), result, errInfo);
  connected = (success == JniErrorCode::IGNITE_JNI_ERR_SUCCESS);
  if (!connected) {
    JniErrorInfo closeErrInfo;
    Close(closeErrInfo);
    return success;
  }
  _connection = result;

  return success;
}

JniErrorCode DocumentDbConnection::Close(JniErrorInfo& errInfo) {
  if (_connection.Get() != nullptr) {
    _jniContext.Get()->ConnectionClose(_connection, errInfo);
    if (errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      LOG_MSG(errInfo.errMsg);
    }
    _connection = nullptr;
    return errInfo.code;
  }
  return JniErrorCode::IGNITE_JNI_ERR_SUCCESS;
}

DocumentDbConnection::~DocumentDbConnection() {
  if (_jniContext.Get() != nullptr) {
    JniErrorInfo errInfo;
    Close(errInfo);
    _jniContext = nullptr;
  }
}

SharedPointer< DatabaseMetaData > DocumentDbConnection::GetMetaData(
    JniErrorInfo& errInfo) {
  if (!_connection.IsValid()) {
    return nullptr;
  }
  SharedPointer< GlobalJObject > databaseMetaData;
  JniErrorCode success = _jniContext.Get()->ConnectionGetMetaData(
      _connection, databaseMetaData, errInfo);
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    return nullptr;
  }
  return new DatabaseMetaData(_jniContext, databaseMetaData);
}

JniErrorCode DocumentDbConnection::IsSshTunnelActive(bool& isActive,
                                                     JniErrorInfo& errInfo) {
  JniErrorCode success =
      _jniContext.Get()->DocumentDbConnectionIsSshTunnelActive(
          _connection, isActive, errInfo);
  return success;
}

JniErrorCode DocumentDbConnection::GetSshLocalPort(int32_t& localPort,
                                                   JniErrorInfo& errInfo) {
  JniErrorCode success = _jniContext.Get()->DocumentDbConnectionGetSshLocalPort(
      _connection, localPort, errInfo);
  return success;
}
}  // namespace jni
}  // namespace odbc
}  // namespace ignite
