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
#include <documentdb/odbc/config/configuration.h>
#include <documentdb/odbc/jni/database_metadata.h>
#include <documentdb/odbc/jni/documentdb_connection_properties.h>
#include <documentdb/odbc/jni/documentdb_database_metadata.h>
#include <documentdb/odbc/jni/java.h>

#ifndef _DOCUMENTDB_ODBC_JNI_DOCUMENTDB_CONNECTION
#define _DOCUMENTDB_ODBC_JNI_DOCUMENTDB_CONNECTION

using documentdb::odbc::common::concurrent::SharedPointer;
using documentdb::odbc::config::Configuration;
using documentdb::odbc::jni::java::GlobalJObject;
using documentdb::odbc::jni::java::JniContext;
using documentdb::odbc::jni::java::JniErrorInfo;

namespace documentdb {
namespace odbc {
namespace jni {
/**
 * A wrapper class for the DocumentDbConnection Java class
 */
class DocumentDbConnection {
 public:
  /**
   * Creates a new instance of the DocumentDbConnection class.
   */
  DocumentDbConnection(SharedPointer< JniContext > jniContext)
      : _jniContext(jniContext) {
  }

  /**
   * Destructs the current instance.
   */
  ~DocumentDbConnection();

  /**
   * Opens a DocumentDbConnection object given the configuration.
   *
   * @return a JniErrorCode indicating success or failure.
   */
  JniErrorCode Open(const Configuration& config, JniErrorInfo& errInfo);

  /**
   * Closes the current DocumentDbConnection object.
   *
   * @return a JniErrorCode indicating success or failure.
   */
  JniErrorCode Close(JniErrorInfo& errInfo);

  bool IsOpen() {
    return _connection.IsValid();
  }

  /**
   * Gets the DatbaseMetaData for this connection.
   *
   * @return a JniErrorCode indicating success or failure.
   */
  SharedPointer< DatabaseMetaData > GetMetaData(JniErrorInfo& errInfo);

  /**
   * Gets indicator of whether an SSH tunnel is active.
   *
   * @return true if active, false, otherwise.
   */
  JniErrorCode IsSshTunnelActive(bool& isActive, JniErrorInfo& errInfo);

  /**
   * Gets indicator of whether an SSH tunnel is active.
   *
   * @return If DOCUMENTDB_JNI_ERR_SUCCESS is returned, then localPort is updated
   * with the value of the local port of SSH tunnel.
   */
  JniErrorCode GetSshLocalPort(int32_t& localPort, JniErrorInfo& errInfo);

  /**
   * Gets the connection properties.
   *
   * @return the connection properties
   */
  SharedPointer< DocumentDbConnectionProperties > GetConnectionProperties(
      JniErrorInfo& errInfo);

  /**
   * Gets the DocumentDb Database Metadata (different from JDBC Database
   * MetaData)
   *
   * @return the DocumentDb database metadata.
   */
  SharedPointer< DocumentDbDatabaseMetadata > GetDatabaseMetadata(
      JniErrorInfo& errInfo);

 private:
  /** The JNI context */
  SharedPointer< JniContext > _jniContext;

  /** The DocumentDbConnection Java object */
  SharedPointer< GlobalJObject > _connection;

  DOCUMENTDB_NO_COPY_ASSIGNMENT(DocumentDbConnection);
};
}  // namespace jni
}  // namespace odbc
}  // namespace documentdb

#endif  // _DOCUMENTDB_ODBC_JNI_DOCUMENTDB_CONNECTION
