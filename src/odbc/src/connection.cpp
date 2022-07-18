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

#include "ignite/odbc/connection.h"

#include <ignite/odbc/ignite_error.h>

#include <algorithm>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/stdx/optional.hpp>
#include <bsoncxx/stdx/string_view.hpp>
#include <cstddef>
#include <cstring>
#include <mongocxx/client.hpp>
#include <mongocxx/exception/exception.hpp>
#include <mongocxx/uri.hpp>
#include <sstream>

#include "ignite/odbc/driver_instance.h"
#include "ignite/odbc/common/concurrent.h"
#include "ignite/odbc/common/utils.h"
#include "ignite/odbc/config/configuration.h"
#include "ignite/odbc/config/connection_string_parser.h"
#include "ignite/odbc/dsn_config.h"
#include "ignite/odbc/environment.h"
#include "ignite/odbc/jni/database_metadata.h"
#include "ignite/odbc/jni/documentdb_connection.h"
#include "ignite/odbc/jni/java.h"
#include "ignite/odbc/jni/utils.h"
#include "ignite/odbc/log.h"
#include "ignite/odbc/message.h"
#include "ignite/odbc/ssl_mode.h"
#include "ignite/odbc/statement.h"
#include "ignite/odbc/system/system_dsn.h"
#include "ignite/odbc/utility.h"

using namespace ignite::odbc::jni::java;
using namespace ignite::odbc::common;
using namespace ignite::odbc::common::concurrent;
using ignite::odbc::IgniteError;
using ignite::odbc::jni::DatabaseMetaData;
using ignite::odbc::jni::DocumentDbConnection;
using ignite::odbc::jni::java::BuildJvmOptions;
using ignite::odbc::jni::java::JniErrorCode;
using ignite::odbc::jni::java::JniHandlers;

// Uncomment for per-byte debug.
//#define PER_BYTE_DEBUG

namespace {
#pragma pack(push, 1)
struct OdbcProtocolHeader {
  int32_t len;
};
#pragma pack(pop)
}  // namespace

namespace ignite {
namespace odbc {
Connection::Connection(Environment* env) : env_(env), info_(config_) {
  // No-op
}

Connection::~Connection() {
  Close();
  jniContext_ = nullptr;
  Deinit();
}

const config::ConnectionInfo& Connection::GetInfo() const {
  return info_;
}

void Connection::GetInfo(config::ConnectionInfo::InfoType type, void* buf,
                         short buflen, short* reslen) {
  LOG_MSG("SQLGetInfo called: "
          << type << " (" << config::ConnectionInfo::InfoTypeToString(type)
          << "), " << std::hex << reinterpret_cast< size_t >(buf) << ", "
          << buflen << ", " << std::hex << reinterpret_cast< size_t >(reslen)
          << std::dec);

  IGNITE_ODBC_API_CALL(InternalGetInfo(type, buf, buflen, reslen));
}

SqlResult::Type Connection::InternalGetInfo(
    config::ConnectionInfo::InfoType type, void* buf, short buflen,
    short* reslen) {
  const config::ConnectionInfo& info = GetInfo();

  SqlResult::Type res = info.GetInfo(type, buf, buflen, reslen);

  if (res != SqlResult::AI_SUCCESS)
    AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                    "Not implemented.");

  return res;
}

void Connection::Establish(const std::string& connectStr, void* parentWindow) {
  IGNITE_ODBC_API_CALL(InternalEstablish(connectStr, parentWindow));
}

SqlResult::Type Connection::InternalEstablish(const std::string& connectStr,
                                              void* parentWindow) {
  config::ConnectionStringParser parser(config_);
  parser.ParseConnectionString(connectStr, &GetDiagnosticRecords());

  if (config_.IsDsnSet()) {
    std::string dsn = config_.GetDsn();

    ReadDsnConfiguration(dsn.c_str(), config_, &GetDiagnosticRecords());
  }

#ifdef _WIN32
  if (parentWindow) {
    LOG_MSG("Parent window is passed. Creating configuration window.");
    if (!DisplayConnectionWindow(parentWindow, config_)) {
      AddStatusRecord(odbc::SqlState::SHY008_OPERATION_CANCELED,
                      "Connection canceled by user");

      return SqlResult::AI_ERROR;
    }
  }
#endif  // _WIN32

  return InternalEstablish(config_);
}

void Connection::Establish(const config::Configuration cfg) {
  IGNITE_ODBC_API_CALL(InternalEstablish(cfg));
}

SqlResult::Type Connection::InternalEstablish(
    const config::Configuration& cfg) {
  using ssl::SslMode;

  config_ = cfg;

  if (connection_.IsValid()) {
    AddStatusRecord(SqlState::S08002_ALREADY_CONNECTED, "Already connected.");

    return SqlResult::AI_ERROR;
  }

  try {
    config_.Validate();
  } catch (const OdbcError& err) {
    AddStatusRecord(err);
    return SqlResult::AI_ERROR;
  }

  IgniteError err;
  bool connected = TryRestoreConnection(err);

  if (!connected) {
    std::string errMessage = "Failed to establish connection with the host.\n";
    errMessage.append(err.GetText());
    AddStatusRecord(SqlState::S08001_CANNOT_CONNECT, errMessage);

    return SqlResult::AI_ERROR;
  }

  bool errors = GetDiagnosticRecords().GetStatusRecordsNumber() > 0;

  return errors ? SqlResult::AI_SUCCESS_WITH_INFO : SqlResult::AI_SUCCESS;
}

void Connection::Release() {
  IGNITE_ODBC_API_CALL(InternalRelease());
}

void Connection::Deregister() {
  env_->DeregisterConnection(this);
}

SqlResult::Type Connection::InternalRelease() {
  if (!connection_.IsValid()) {
    AddStatusRecord(SqlState::S08003_NOT_CONNECTED, "Connection is not open.");

    // It is important to return SUCCESS_WITH_INFO and not ERROR here, as if we
    // return an error, Windows Driver Manager may decide that connection is not
    // valid anymore which results in memory leak.
    return SqlResult::AI_SUCCESS_WITH_INFO;
  }

  Close();

  return SqlResult::AI_SUCCESS;
}

void Connection::Close() {
  if (jniContext_.IsValid()) {
    if (connection_.IsValid()) {
      JniErrorInfo errInfo;
      connection_.Get()->Close(errInfo);
      if (errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
        // TODO: Determine if we need to error check the close.
      }
      connection_ = nullptr;
    }
  }
}

Statement* Connection::CreateStatement() {
  Statement* statement;

  IGNITE_ODBC_API_CALL(InternalCreateStatement(statement));

  return statement;
}

SharedPointer< DatabaseMetaData > Connection::GetMetaData(IgniteError& err) {
  if (!connection_.IsValid()) {
    err = IgniteError(IgniteError::IGNITE_ERR_ILLEGAL_STATE,
                      "Must be connected.");
    return nullptr;
  }
  JniErrorInfo errInfo;
  auto databaseMetaData = connection_.Get()->GetMetaData(errInfo);
  if (!databaseMetaData.IsValid()) {
    std::string message = errInfo.errMsg;
    err = IgniteError(IgniteError::IGNITE_ERR_JNI_GET_DATABASE_METADATA,
                      message.c_str());
    return nullptr;
  }
  return databaseMetaData;
}

SharedPointer< DocumentDbDatabaseMetadata > Connection::GetDatabaseMetadata(
    IgniteError& err) {
  if (!connection_.IsValid()) {
    err = IgniteError(IgniteError::IGNITE_ERR_ILLEGAL_STATE,
                      "Must be connected.");
    return nullptr;
  }
  JniErrorInfo errInfo;
  auto documentDbDatabaseMetaData =
      connection_.Get()->GetDatabaseMetadata(errInfo);
  if (!documentDbDatabaseMetaData.IsValid()) {
    std::string message = errInfo.errMsg;
    err = IgniteError(
        IgniteError::IGNITE_ERR_JNI_GET_DOCUMENTDB_DATABASE_METADATA,
        message.c_str());
    return nullptr;
  }
  return documentDbDatabaseMetaData;
}

SharedPointer< DocumentDbConnectionProperties >
Connection::GetConnectionProperties(IgniteError& err) {
  if (!connection_.IsValid()) {
    err = IgniteError(IgniteError::IGNITE_ERR_ILLEGAL_STATE,
                      "Must be connected.");
    return nullptr;
  }
  JniErrorInfo errInfo;
  auto connectionProperties =
      connection_.Get()->GetConnectionProperties(errInfo);
  if (!connectionProperties.IsValid()) {
    std::string message = errInfo.errMsg;
    err = IgniteError(
        IgniteError::IGNITE_ERR_JNI_GET_DOCUMENTDB_CONNECTION_PROPERTIES,
        message.c_str());
    return nullptr;
  }
  return connectionProperties;
}

SqlResult::Type Connection::InternalCreateStatement(Statement*& statement) {
  statement = new Statement(*this);

  if (!statement) {
    AddStatusRecord(SqlState::SHY001_MEMORY_ALLOCATION, "Not enough memory.");

    return SqlResult::AI_ERROR;
  }

  return SqlResult::AI_SUCCESS;
}

const std::string& Connection::GetSchema() const {
  return config_.GetDatabase();
}

const config::Configuration& Connection::GetConfiguration() const {
  return config_;
}

diagnostic::DiagnosticRecord Connection::CreateStatusRecord(
    SqlState::Type sqlState, const std::string& message, int32_t rowNum,
    int32_t columnNum) {
  return diagnostic::DiagnosticRecord(sqlState, message, "", "", rowNum,
                                      columnNum);
}

void Connection::GetAttribute(int attr, void* buf, SQLINTEGER bufLen,
                              SQLINTEGER* valueLen) {
  IGNITE_ODBC_API_CALL(InternalGetAttribute(attr, buf, bufLen, valueLen));
}

SqlResult::Type Connection::InternalGetAttribute(int attr, void* buf,
                                                 SQLINTEGER,
                                                 SQLINTEGER* valueLen) {
  if (!buf) {
    AddStatusRecord(SqlState::SHY009_INVALID_USE_OF_NULL_POINTER,
                    "Data buffer is null.");

    return SqlResult::AI_ERROR;
  }

  switch (attr) {
    case SQL_ATTR_PACKET_SIZE: {
      SQLUINTEGER* val = reinterpret_cast< SQLUINTEGER* >(buf);

      *val = static_cast< SQLUINTEGER >(config_.GetDefaultFetchSize());

      if (valueLen)
        *valueLen = SQL_IS_INTEGER;

      break;
    }

    case SQL_ATTR_CONNECTION_DEAD: {
      SQLUINTEGER* val = reinterpret_cast< SQLUINTEGER* >(buf);

      *val = connection_.Get() ? SQL_CD_FALSE : SQL_CD_TRUE;

      if (valueLen)
        *valueLen = SQL_IS_INTEGER;

      break;
    }

    case SQL_ATTR_LOGIN_TIMEOUT: {
      SQLUINTEGER* val = reinterpret_cast< SQLUINTEGER* >(buf);

      *val = static_cast< SQLUINTEGER >(config_.GetLoginTimeoutSeconds());

      if (valueLen)
        *valueLen = SQL_IS_INTEGER;

      break;
    }

    default: {
      AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                      "Specified attribute is not supported.");

      return SqlResult::AI_ERROR;
    }
  }

  return SqlResult::AI_SUCCESS;
}

void Connection::SetAttribute(int attr, void* value, SQLINTEGER valueLen) {
  IGNITE_ODBC_API_CALL(InternalSetAttribute(attr, value, valueLen));
}

SqlResult::Type Connection::InternalSetAttribute(int attr, void* value,
                                                 SQLINTEGER) {
  switch (attr) {
    case SQL_ATTR_CONNECTION_DEAD: {
      AddStatusRecord(SqlState::SHY092_OPTION_TYPE_OUT_OF_RANGE,
                      "Attribute is read only.");

      return SqlResult::AI_ERROR;
    }

    case SQL_ATTR_PACKET_SIZE: {
      SQLUINTEGER val =
          static_cast< SQLUINTEGER >(reinterpret_cast< ptrdiff_t >(value));
      config_.SetDefaultFetchSize(static_cast< int32_t >(val));
      break;
    }

    case SQL_ATTR_LOGIN_TIMEOUT: {
      config_.SetLoginTimeoutSeconds(RetrieveTimeout(value));

      if (GetDiagnosticRecords().GetStatusRecordsNumber() != 0)
        return SqlResult::AI_SUCCESS_WITH_INFO;

      break;
    }

    default: {
      AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                      "Specified attribute is not supported.");

      return SqlResult::AI_ERROR;
    }
  }

  return SqlResult::AI_SUCCESS;
}

void Connection::EnsureConnected() {
  if (connection_.IsValid())
    return;

  IgniteError err;
  bool success = TryRestoreConnection(err);

  if (!success) {
    std::string errMessage =
        "Failed to establish connection with any provided hosts\n";
    errMessage.append(err.GetText());
    AddStatusRecord(SqlState::S08001_CANNOT_CONNECT, errMessage);
    throw OdbcError(SqlState::S08001_CANNOT_CONNECT, errMessage);
  }
}

bool Connection::TryRestoreConnection(IgniteError& err) {
  if (connection_.IsValid()) {
    return true;
  }

  JniErrorInfo errInfo;
  auto ctx = GetJniContext(errInfo);
  if (errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    err = IgniteError(static_cast< int32_t >(errInfo.code),
                      std::string(errInfo.errCls)
                          .append(": ")
                          .append(errInfo.errMsg)
                          .c_str());
    return false;
  }
  SharedPointer< DocumentDbConnection > conn = new DocumentDbConnection(ctx);
  if (!conn.IsValid()
      || conn.Get()->Open(config_, errInfo)
             != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    std::string message = errInfo.errMsg;
    err = IgniteError(IgniteError::IGNITE_ERR_SECURE_CONNECTION_FAILURE,
                      message.c_str());
  }
  connection_ = conn;
  bool connected = connection_.IsValid() && connection_.Get()->IsOpen()
                   && errInfo.code == JniErrorCode::IGNITE_JNI_ERR_SUCCESS;

  if (!connected) {
    return connected;
  }

  int32_t localSSHTunnelPort = 0;
  if (!GetInternalSSHTunnelPort(localSSHTunnelPort, ctx, err)) {
    return false;
  }

  connected = ConnectCPPDocumentDB(localSSHTunnelPort, err);

  return connected;
}

bool Connection::GetInternalSSHTunnelPort(int32_t& localSSHTunnelPort,
                                          SharedPointer< JniContext > ctx,
                                          odbc::IgniteError& err) {
  bool isSSHTunnelActive;
  JniErrorInfo errInfo;
  JniErrorCode success =
      connection_.Get()->IsSshTunnelActive(isSSHTunnelActive, errInfo);

  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    err = IgniteError(odbc::IgniteError::IGNITE_ERR_JVM_INIT,
                      errInfo.errMsg.c_str());
    return false;
  }

  if (isSSHTunnelActive) {
    JniErrorCode success =
        connection_.Get()->GetSshLocalPort(localSSHTunnelPort, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      err = IgniteError(odbc::IgniteError::IGNITE_ERR_JVM_INIT,
                        errInfo.errMsg.c_str());
      return false;
    }
  }

  return true;
}

std::string Connection::FormatMongoCppConnectionString(
    int32_t localSSHTunnelPort) const {
  std::string host = "localhost";
  std::string port = std::to_string(localSSHTunnelPort);

  // localSSHTunnelPort == 0 means that internal SSH tunnel option was not set
  if (localSSHTunnelPort == 0) {
    host = config_.GetHostname();
    port = common::LexicalCast< std::string >(config_.GetPort());
  }
  std::string mongoConnectionString;

  mongoConnectionString = "mongodb:";
  mongoConnectionString.append("//" + config_.GetUser());
  mongoConnectionString.append(":" + config_.GetPassword());
  mongoConnectionString.append("@" + host);
  mongoConnectionString.append(":" + port);
  mongoConnectionString.append("/admin");
  mongoConnectionString.append("?authMechanism=SCRAM-SHA-1");
  if (config_.IsTls()) {
    mongoConnectionString.append("&tlsAllowInvalidHostnames=true");
  }
  if (config_.GetLoginTimeoutSeconds()) {
    std::chrono::milliseconds connectionTimeoutMS = std::chrono::seconds(config_.GetLoginTimeoutSeconds());
    mongoConnectionString.append("&socketTimeoutMS="
                                 + std::to_string(connectionTimeoutMS.count()));
  }

  // tls configuration is handled using tls_options in connectionCPP
  // TODO handle the other DSN configuration
  // https://bitquill.atlassian.net/browse/AD-599

  return mongoConnectionString;
}

SharedPointer< JniContext > Connection::GetJniContext(JniErrorInfo& errInfo) {
  if (!jniContext_.IsValid()) {
    // Resolve DOCUMENTDB_HOME.
    std::string home = jni::ResolveDocumentDbHome();

    // Create classpath.
    std::string cp = jni::CreateDocumentDbClasspath(std::string(), home);
    if (cp.empty()) {
      return nullptr;
    }

    // Set the JVM options.
    SetJvmOptions(cp);

    // Create the context
    SharedPointer< JniContext > ctx(JniContext::Create(
        &opts_[0], static_cast< int >(opts_.size()), JniHandlers(), errInfo));
    jniContext_ = ctx;
  }
  return jniContext_;
}

/**
 * Create JVM options from configuration.
 *
 * @param cfg Configuration.
 * @param home Optional GG home.
 * @param cp Classpath.
 */
void Connection::SetJvmOptions(const std::string& cp) {
  Deinit();
  BuildJvmOptions(cp, opts_);
}

/**
 * Deallocates all allocated data.
 */
void Connection::Deinit() {
  std::for_each(opts_.begin(), opts_.end(), ReleaseChars);
  opts_.clear();
}

int32_t Connection::RetrieveTimeout(void* value) {
  SQLUINTEGER uTimeout =
      static_cast< SQLUINTEGER >(reinterpret_cast< ptrdiff_t >(value));

  if (uTimeout > INT32_MAX) {
    std::stringstream ss;

    ss << "Value is too big: " << uTimeout << ", changing to " << timeout_
       << ".";

    AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED, ss.str());

    return INT32_MAX;
  }

  return static_cast< int32_t >(uTimeout);
}

bool Connection::ConnectCPPDocumentDB(int32_t localSSHTunnelPort,
                                      odbc::IgniteError& err) {
  using bsoncxx::builder::basic::kvp;
  using bsoncxx::builder::basic::make_document;

  // Make sure that the DriverInstance is initialize
  DriverInstance::getInstance().initialize();
  try {
    std::string mongoCPPConnectionString =
        FormatMongoCppConnectionString(localSSHTunnelPort);
    mongocxx::options::client client_options;
    mongocxx::options::tls tls_options;
    if (config_.IsTls()) {
      // TO-DO Adapt to use certificates
      // https://bitquill.atlassian.net/browse/AD-598
      tls_options.allow_invalid_certificates(true);
      client_options.tls_opts(tls_options);
    }

    mongoClient_ = std::make_shared< mongocxx::client >(
        mongocxx::uri(mongoCPPConnectionString), client_options);
    std::string database = config_.GetDatabase();
    bsoncxx::builder::stream::document ping;
    ping << "ping" << 1;
    auto db = (*mongoClient_.get())[database];
    auto result = db.run_command(ping.view());

    if (result.view()["ok"].get_double() != 1) {
      err = odbc::IgniteError(odbc::IgniteError::IGNITE_ERR_NETWORK_FAILURE,
                              "Unable to ping DocumentDB.");
      return false;
    }

    return true;
  } catch (const mongocxx::exception& xcp) {
    std::stringstream message;
    message << "Unable to establish connection with DocumentDB."
            << " code: " << xcp.code().value()
            << " messagge: " << xcp.code().message()
            << " cause: " << xcp.what();
    err = odbc::IgniteError(
        odbc::IgniteError::IGNITE_ERR_SECURE_CONNECTION_FAILURE,
        message.str().c_str());
    return false;
  }
}
}  // namespace odbc
}  // namespace ignite
