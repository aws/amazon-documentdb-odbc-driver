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

#ifndef _DOCUMENTDB_ODBC_CONNECTION
#define _DOCUMENTDB_ODBC_CONNECTION

#include <documentdb/odbc/common/concurrent.h>
#include <stdint.h>

#include <vector>

#include "documentdb/odbc/config/configuration.h"
#include "documentdb/odbc/config/connection_info.h"
#include "documentdb/odbc/diagnostic/diagnosable_adapter.h"
#include "documentdb/odbc/end_point.h"
#include "documentdb/odbc/jni/database_metadata.h"
#include "documentdb/odbc/jni/documentdb_connection.h"
#include "documentdb/odbc/jni/documentdb_connection_properties.h"
#include "documentdb/odbc/jni/documentdb_database_metadata.h"
#include "documentdb/odbc/jni/java.h"
#include "documentdb/odbc/log.h"
#include "documentdb/odbc/odbc_error.h"
#include "documentdb/odbc/parser.h"
#include "documentdb/odbc/streaming/streaming_context.h"
#include "mongocxx/client.hpp"

using documentdb::odbc::common::concurrent::SharedPointer;
using documentdb::odbc::jni::DatabaseMetaData;
using documentdb::odbc::jni::DocumentDbConnection;
using documentdb::odbc::jni::DocumentDbConnectionProperties;
using documentdb::odbc::jni::DocumentDbDatabaseMetadata;
using documentdb::odbc::jni::java::GlobalJObject;
using documentdb::odbc::jni::java::JniContext;

namespace documentdb {
namespace odbc {
class Environment;
class Statement;

/**
 * ODBC node connection.
 */
class Connection : public diagnostic::DiagnosableAdapter {
  friend class Environment;

 public:
  /**
   * Operation with timeout result.
   */
  struct OperationResult {
    enum T { SUCCESS, FAIL, TIMEOUT };
  };

  /** Default connection timeout in seconds. */
  enum { DEFAULT_CONNECT_TIMEOUT = 5 };

  /**
   * Destructor.
   */
  ~Connection();

  /**
   * Get connection info.
   *
   * @return Connection info.
   */
  const config::ConnectionInfo& GetInfo() const;

  /**
   * Get info of any type.
   *
   * @param type Info type.
   * @param buf Result buffer pointer.
   * @param buflen Result buffer length.
   * @param reslen Result value length pointer.
   */
  void GetInfo(config::ConnectionInfo::InfoType type, void* buf, short buflen,
               short* reslen);

  /**
   * Gets the native SQL statement.
   *
   * @param inQuery SQL query to translate to native SQL.
   * @param inQueryLen Length of the SQL query.
   * @param outQueryBuffer Buffer to place the native SQL.
   * @param outQueryBufferLen Length of the output buffer for the native SQL 
   * including the null terminating character.
   * @param outQueryLen Actual or required length of the output buffer for the native SQL
   * NOT including the null terminating character.
   */
  template < typename CharT >
  inline void NativeSql(const CharT* inQuery, int64_t inQueryLen,
                        CharT* outQueryBuffer, int64_t outQueryBufferLen,
                        int64_t* outQueryLen) {
    IGNITE_ODBC_API_CALL(InternalNativeSql(inQuery, inQueryLen, outQueryBuffer,
                                           outQueryBufferLen, outQueryLen));
  }

  /**
   * Establish connection to ODBC server.
   *
   * @param connectStr Connection string.
   * @param parentWindow Parent window pointer.
   */
  void Establish(const std::string& connectStr, void* parentWindow);

  /**
   * Establish connection to ODBC server.
   *
   * @param cfg Configuration.
   */
  void Establish(const config::Configuration cfg);

  /**
   * Release established connection.
   *
   * @return Operation result.
   */
  void Release();

  /**
   * Deregister self from the parent.
   */
  void Deregister();

  /**
   * Create statement associated with the connection.
   *
   * @return Pointer to valid instance on success and NULL on failure.
   */
  Statement* CreateStatement();

  /**
   * Gets the database metadata for the connection.
   *
   * @return SharedPointer to DatabaseMetaData.
   */
  SharedPointer< DatabaseMetaData > GetMetaData(DocumentDbError& err);

  /**
   * Gets the DocumentDB database metadata for the connection.
   *
   * @return SharedPointer to DocumentDbDatabaseMetadata.
   */
  SharedPointer< DocumentDbDatabaseMetadata > GetDatabaseMetadata(
      DocumentDbError& err);

  /**
   * Gets the DocumentDB connection properties.
   *
   * @return SharedPointer to DocumentDbConnectionProperties.
   */
  SharedPointer< DocumentDbConnectionProperties > GetConnectionProperties(
      DocumentDbError& err);

  /**
   * Get name of the assotiated schema.
   *
   * @return Schema name.
   */
  const std::string& GetSchema() const;

  /**
   * Get configuration.
   *
   * @return Connection configuration.
   */
  const config::Configuration& GetConfiguration() const;


  /**
   * Create diagnostic record associated with the Connection instance.
   *
   * @param sqlState SQL state.
   * @param message Message.
   * @param rowNum Associated row number.
   * @param columnNum Associated column number.
   * @return DiagnosticRecord associated with the instance.
   */
  static diagnostic::DiagnosticRecord CreateStatusRecord(
      SqlState::Type sqlState, const std::string& message, int32_t rowNum = 0,
      int32_t columnNum = 0);

  /**
   * Synchronously send request message and receive response.
   * Uses provided timeout.
   *
   * @param req Request message.
   * @param rsp Response message.
   * @param timeout Timeout.
   * @return @c true on success, @c false on timeout.
   * @throw OdbcError on error.
   */
  template < typename ReqT, typename RspT >
  bool SyncMessage(const ReqT& req, RspT& rsp, int32_t timeout) {
    // TODO: Remove when unnecessary.
    return true;
  }

  /**
   * Synchronously send request message and receive response.
   * Uses connection timeout.
   *
   * @param req Request message.
   * @param rsp Response message.
   * @throw OdbcError on error.
   */
  template < typename ReqT, typename RspT >
  void SyncMessage(const ReqT& req, RspT& rsp) {
    // TODO: Should be needed for TimeStream though not
    // for DocumentDB
  }

  /**
   * Send request message.
   * Uses connection timeout.
   *
   * @param req Request message.
   * @throw OdbcError on error.
   */
  template < typename ReqT >
  void SendRequest(const ReqT& req) {
    // TODO: Remove when unnecessary.
  }

  /**
   * Get connection attribute.
   *
   * @param attr Attribute type.
   * @param buf Buffer for value.
   * @param bufLen Buffer length.
   * @param valueLen Resulting value length.
   */
  void GetAttribute(int attr, void* buf, SQLINTEGER bufLen,
                    SQLINTEGER* valueLen);

  /**
   * Set connection attribute.
   *
   * @param attr Attribute type.
   * @param value Value pointer.
   * @param valueLen Value length.
   */
  void SetAttribute(int attr, void* value, SQLINTEGER valueLen);

  inline std::shared_ptr< mongocxx::client >& GetMongoClient() {
    return mongoClient_;
  }

 private:
  IGNITE_NO_COPY_ASSIGNMENT(Connection);

  /**
   * Init connection socket, using configuration.
   *
   * @return Operation result.
   */
  SqlResult::Type InitSocket();

  /**
   * Synchronously send request message and receive response.
   * Uses provided timeout. Does not try to restore connection on
   * fail.
   *
   * @param req Request message.
   * @param rsp Response message.
   * @param timeout Timeout.
   * @return @c true on success, @c false on timeout.
   * @throw OdbcError on error.
   */
  template < typename ReqT, typename RspT >
  bool InternalSyncMessage(const ReqT& req, RspT& rsp, int32_t timeout) {
    // TODO: Remove when unnecessary.
    return true;
  }

  /**
   * Establish connection to ODBC server.
   * Internal call.
   *
   * @param connectStr Connection string.
   * @param parentWindow Parent window.
   * @return Operation result.
   */
  SqlResult::Type InternalEstablish(const std::string& connectStr,
                                    void* parentWindow);

  /**
   * Establish connection to ODBC server.
   * Internal call.
   *
   * @param cfg Configuration.
   * @return Operation result.
   */
  SqlResult::Type InternalEstablish(const config::Configuration& cfg);

  /**
   * Release established connection.
   * Internal call.
   *
   * @return Operation result.
   */
  SqlResult::Type InternalRelease();

  /**
   * Close connection.
   */
  void Close();

  /**
   * Get info of any type.
   * Internal call.
   *
   * @param type Info type.
   * @param buf Result buffer pointer.
   * @param buflen Result buffer length.
   * @param reslen Result value length pointer.
   * @return Operation result.
   */
  SqlResult::Type InternalGetInfo(config::ConnectionInfo::InfoType type,
                                  void* buf, short buflen, short* reslen);

  /**
   * Gets the native SQL statement.
   *
   * @param inQuery SQL query to translate to native SQL.
   * @param inQueryLen Length of the SQL query.
   * @param outQueryBuffer Buffer to place the native SQL.
   * @param outQueryBufferLen Length of the output buffer for the native SQL
   * including the null terminating character.
   * @param outQueryLen Actual or required length of the output buffer for the
   * native SQL NOT including the null terminating character.
   * @return Operation result.
   */
  template < typename CharT >
  inline SqlResult::Type InternalNativeSql(const CharT* inQuery,
                                           int64_t inQueryLen,
                                           CharT* outQueryBuffer,
                                           int64_t outQueryBufferLen,
                                           int64_t* outQueryLen) {
    bool isTruncated = false;
    if (!inQuery) {
      AddStatusRecord(SqlState::SHY009_INVALID_USE_OF_NULL_POINTER,
                      "The InStatementText argument must not NULL");
      return SqlResult::AI_ERROR;
    }

    SQLINTEGER actualBufferLen = 0;
    if (outQueryBuffer) {
      if (outQueryBufferLen <= 0) {
        AddStatusRecord(SqlState::SHY090_INVALID_STRING_OR_BUFFER_LENGTH,
                        "The BufferLength argument must be greater than zero");
        return SqlResult::AI_ERROR;
      }

      if (inQueryLen == SQL_NTS) {
        for (; inQuery[actualBufferLen] != 0; actualBufferLen++) {
          if ((actualBufferLen + 1) >= outQueryBufferLen) {
            isTruncated = true;
            break;
          }
          outQueryBuffer[actualBufferLen] = inQuery[actualBufferLen];
        }
      } else if (inQueryLen >= 0) {
        for (; actualBufferLen < inQueryLen; actualBufferLen++) {
          if ((actualBufferLen + 1) >= outQueryBufferLen) {
            isTruncated = true;
            break;
          }
          outQueryBuffer[actualBufferLen] = inQuery[actualBufferLen];
        }
      } else {
        AddStatusRecord(
            SqlState::SHY090_INVALID_STRING_OR_BUFFER_LENGTH,
            "The argument TextLength1 was less than 0, but not equal to "
            "SQL_NTS");
        return SqlResult::AI_ERROR;
      }
      outQueryBuffer[actualBufferLen] = 0;
    } else {
      // Get the required length
      if (inQueryLen == SQL_NTS) {
        for (; inQuery[actualBufferLen] != 0; actualBufferLen++) {
        }
      } else if (inQueryLen > 0) {
        actualBufferLen = inQueryLen;
      }
    }

    if (outQueryLen)
      *outQueryLen = actualBufferLen;

    LOG_DEBUG_MSG("SQLNativeSql exiting");

    if (isTruncated) {
      AddStatusRecord(
          SqlState::S01004_DATA_TRUNCATED,
          "Buffer is too small for the data. Truncated from the right.");
      return SqlResult::AI_SUCCESS_WITH_INFO;
    }

    return SqlResult::AI_SUCCESS;
  }

  /**
   * Create statement associated with the connection.
   * Internal call.
   *
   * @param statement Pointer to valid instance on success and NULL on failure.
   * @return Operation result.
   */
  SqlResult::Type InternalCreateStatement(Statement*& statement);

  /**
   * Get connection attribute.
   * Internal call.
   *
   * @param attr Attribute type.
   * @param buf Buffer for value.
   * @param bufLen Buffer length.
   * @param valueLen Resulting value length.
   * @return Operation result.
   */
  SqlResult::Type InternalGetAttribute(int attr, void* buf, SQLINTEGER bufLen,
                                       SQLINTEGER* valueLen);

  /**
   * Set connection attribute.
   * Internal call.
   *
   * @param attr Attribute type.
   * @param value Value pointer.
   * @param valueLen Value length.
   * @return Operation result.
   */
  SqlResult::Type InternalSetAttribute(int attr, void* value,
                                       SQLINTEGER valueLen);

  /**
   * Ensure there is a connection to the cluster.
   *
   * @throw OdbcError on failure.
   */
  void EnsureConnected();

  /**
   * Try to restore connection to the cluster.
   *
   * @throw DocumentDbError on failure.
   * @return @c true on success and @c false otherwise.
   */
  bool TryRestoreConnection(DocumentDbError& err);

  /**
   * Connect to DocumentDB using Mongo cxx driver
   *
   * @param localSSHTunnelPort internal SSH tunnel port
   * @param err
   * @return @c true on success and @c false otherwise.
   */
  bool ConnectCPPDocumentDB(int32_t localSSHTunnelPort, DocumentDbError& err);

  /**
   * Formats the Mongo connection string from configuration values.
   *
   *  @return the JDBC connection string.
   */
  std::string FormatMongoCppConnectionString(int32_t localSSHTunnelPort) const;

  /**
   * Helper function to get internall SSH tunnel Port
   *
   * @param localSSHTunnelPort internal SSH tunnel port
   * @param ctx java context
   * @param err
   * @return bool
   */
  bool GetInternalSSHTunnelPort(int32_t& localSSHTunnelPort,
                                SharedPointer< jni::java::JniContext > ctx,
                                DocumentDbError& err);

  /**
   * Creates JVM options
   */
  void SetJvmOptions(const std::string& cp);

  /**
   * Get the singleton instance of the JNI context for the connection.
   *
   * @return JNI context.
   */
  SharedPointer< JniContext > GetJniContext(JniErrorInfo& errInfo);

  /**
   * De-initializes the JVM options
   */
  void Deinit();

  /**
   * Retrieve timeout from parameter.
   *
   * @param value Parameter.
   * @return Timeout.
   */
  int32_t RetrieveTimeout(void* value);

  /**
   * Constructor.
   */
  Connection(Environment* env);

  /** Parent. */
  Environment* env_;

  /** Connection timeout in seconds. */
  int32_t timeout_ = 0;

  /** Configuration. */
  config::Configuration config_;

  /** Connection info. */
  config::ConnectionInfo info_;

  /** Java connection object */
  SharedPointer< DocumentDbConnection > connection_;

  SharedPointer< JniContext > jniContext_;

  std::shared_ptr< mongocxx::client > mongoClient_;

  /** JVM options */
  std::vector< char* > opts_;
};
}  // namespace odbc
}  // namespace documentdb

#endif  //_DOCUMENTDB_ODBC_CONNECTION
