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
#include "ignite/odbc/config/configuration.h"

#include <iterator>
#include <sstream>
#include <string>

#include "ignite/common/utils.h"
#include "ignite/odbc/config/config_tools.h"
#include "ignite/odbc/config/connection_string_parser.h"
#include "ignite/odbc/log.h"
#include "ignite/odbc/utility.h"

using ignite::common::EncodeURIComponent;

namespace ignite {
namespace odbc {
namespace config {
// Connection Settings
const std::string Configuration::DefaultValue::dsn = "DocumentDB DSN";
const std::string Configuration::DefaultValue::driver = "Amazon DocumentDB";
const std::string Configuration::DefaultValue::database = "";
const std::string Configuration::DefaultValue::hostname = "";
const uint16_t Configuration::DefaultValue::port = 27017;
const std::string Configuration::DefaultValue::user = "";
const std::string Configuration::DefaultValue::password = "";

// SSL/TLS options
const bool Configuration::DefaultValue::tls = true;
const bool Configuration::DefaultValue::tlsAllowInvalidHostnames = false;
const std::string Configuration::DefaultValue::tlsCaFile = "";

// Schema Generation and Discovery options
const ScanMethod::Type Configuration::DefaultValue::scanMethod =
    ScanMethod::Type::RANDOM;
const int32_t Configuration::DefaultValue::scanLimit = 1000;
const std::string Configuration::DefaultValue::schemaName = "_default";
const bool Configuration::DefaultValue::refreshSchema = false;

// Internal SSH Tunnel options
const bool Configuration::DefaultValue::sshEnable = false;
const std::string Configuration::DefaultValue::sshUser = "";
const std::string Configuration::DefaultValue::sshHost = "";
const std::string Configuration::DefaultValue::sshPrivateKeyFile = "";
const std::string Configuration::DefaultValue::sshPrivateKeyPassphrase = "";
const bool Configuration::DefaultValue::sshStrictHostKeyChecking = true;
const std::string Configuration::DefaultValue::sshKnownHostsFile = "";

// Logging Configuration options
const LogLevel::Type Configuration::DefaultValue::logLevel =
    LogLevel::Type::ERROR_LEVEL;
const std::string Configuration::DefaultValue::logPath = DEFAULT_LOG_PATH;

// Additional options
const std::string Configuration::DefaultValue::appName =
    "Amazon DocumentDB ODBC Driver";
const int32_t Configuration::DefaultValue::loginTimeoutSec = 0;
const ReadPreference::Type Configuration::DefaultValue::readPreference =
    ReadPreference::Type::PRIMARY;
const std::string Configuration::DefaultValue::replicaSet = "";
const bool Configuration::DefaultValue::retryReads = true;
const int32_t Configuration::DefaultValue::defaultFetchSize = 2000;

std::string Configuration::ToConnectString() const {
  ArgumentMap arguments;

  ToMap(arguments);

  std::stringstream connect_string_buffer;

  for (ArgumentMap::const_iterator it = arguments.begin();
       it != arguments.end(); ++it) {
    const std::string& key = it->first;
    const std::string& value = it->second;

    if (value.empty())
      continue;

    if (value.find(' ') == std::string::npos)
      connect_string_buffer << key << '=' << value << ';';
    else
      connect_string_buffer << key << "={" << value << "};";
  }

  return connect_string_buffer.str();
}

uint16_t Configuration::GetPort() const {
  return port.GetValue();
}

void Configuration::SetPort(uint16_t portNumber) {
  this->port.SetValue(portNumber);
}

bool Configuration::IsPortSet() const {
  return port.IsSet();
}

const std::string& Configuration::GetDsn(const std::string& dflt) const {
  if (!dsn.IsSet())
    return dflt;

  return dsn.GetValue();
}

bool Configuration::IsDsnSet() const {
  return dsn.IsSet();
}

void Configuration::SetDsn(const std::string& dsnName) {
  this->dsn.SetValue(dsnName);
}

const std::string& Configuration::GetDriver() const {
  return driver.GetValue();
}

void Configuration::SetDriver(const std::string& driverName) {
  this->driver.SetValue(driverName);
}

const std::string& Configuration::GetHostname() const {
  return hostname.GetValue();
}

void Configuration::SetHostname(const std::string& host) {
  this->hostname.SetValue(host);
}

bool Configuration::IsHostnameSet() const {
  return hostname.IsSet();
}

const std::string& Configuration::GetDatabase() const {
  return database.GetValue();
}

void Configuration::SetDatabase(const std::string& schema) {
  this->database.SetValue(schema);
}

bool Configuration::IsDatabaseSet() const {
  return database.IsSet();
}

const std::string& Configuration::GetApplicationName() const {
  return appName.GetValue();
}

void Configuration::SetApplicationName(const std::string& name) {
  this->appName.SetValue(name);
}

bool Configuration::IsApplicationNameSet() const {
  return appName.IsSet();
}

int32_t Configuration::GetLoginTimeoutSeconds() const {
  return loginTimeoutSec.GetValue();
}

void Configuration::SetLoginTimeoutSeconds(int32_t seconds) {
  this->loginTimeoutSec.SetValue(seconds);
}

bool Configuration::IsLoginTimeoutSecondsSet() const {
  return loginTimeoutSec.IsSet();
}

ReadPreference::Type Configuration::GetReadPreference() const {
  return readPreference.GetValue();
}

void Configuration::SetReadPreference(const ReadPreference::Type preference) {
  this->readPreference.SetValue(preference);
}

bool Configuration::IsReadPreferenceSet() const {
  return readPreference.IsSet();
}

const std::string& Configuration::GetReplicaSet() const {
  return replicaSet.GetValue();
}

void Configuration::SetReplicaSet(const std::string& name) {
  this->replicaSet.SetValue(name);
}

bool Configuration::IsReplicaSetSet() const {
  return replicaSet.IsSet();
}

bool Configuration::IsRetryReads() const {
  return retryReads.GetValue();
}

void Configuration::SetRetryReads(bool val) {
  this->retryReads.SetValue(val);
}

bool Configuration::IsRetryReadsSet() const {
  return retryReads.IsSet();
}

bool Configuration::IsTls() const {
  return tls.GetValue();
}

void Configuration::SetTls(bool val) {
  this->tls.SetValue(val);
}

bool Configuration::IsTlsSet() const {
  return tls.IsSet();
}

bool Configuration::IsTlsAllowInvalidHostnames() const {
  return tlsAllowInvalidHostnames.GetValue();
}

void Configuration::SetTlsAllowInvalidHostnames(bool val) {
  this->tlsAllowInvalidHostnames.SetValue(val);
}

bool Configuration::IsTlsAllowInvalidHostnamesSet() const {
  return tlsAllowInvalidHostnames.IsSet();
}

const std::string& Configuration::GetTlsCaFile() const {
  return tlsCaFile.GetValue();
}

void Configuration::SetTlsCaFile(const std::string& path) {
  this->tlsCaFile.SetValue(path);
}

bool Configuration::IsTlsCaFileSet() const {
  return tlsCaFile.IsSet();
}

bool Configuration::IsSshEnable() const {
  return sshEnable.GetValue();
}

void Configuration::SetSshEnable(bool val) {
  this->sshEnable.SetValue(val);
}

bool Configuration::IsSshEnableSet() const {
  return sshEnable.IsSet();
}

const std::string& Configuration::GetSshUser() const {
  return sshUser.GetValue();
}

void Configuration::SetSshUser(const std::string& username) {
  this->sshUser.SetValue(username);
}

bool Configuration::IsSshUserSet() const {
  return sshUser.IsSet();
}

const std::string& Configuration::GetSshHost() const {
  return sshHost.GetValue();
}

void Configuration::SetSshHost(const std::string& host) {
  this->sshHost.SetValue(host);
}

bool Configuration::IsSshHostSet() const {
  return sshHost.IsSet();
}

const std::string& Configuration::GetSshPrivateKeyFile() const {
  return sshPrivateKeyFile.GetValue();
}

void Configuration::SetSshPrivateKeyFile(const std::string& path) {
  this->sshPrivateKeyFile.SetValue(path);
}

bool Configuration::IsSshPrivateKeyFileSet() const {
  return sshPrivateKeyFile.IsSet();
}

const std::string& Configuration::GetSshPrivateKeyPassphrase() const {
  return sshPrivateKeyPassphrase.GetValue();
}

void Configuration::SetSshPrivateKeyPassphrase(const std::string& passphrase) {
  this->sshPrivateKeyPassphrase.SetValue(passphrase);
}

bool Configuration::IsSshPrivateKeyPassphraseSet() const {
  return sshPrivateKeyPassphrase.IsSet();
}

bool Configuration::IsSshStrictHostKeyChecking() const {
  return sshStrictHostKeyChecking.GetValue();
}

void Configuration::SetSshStrictHostKeyChecking(bool val) {
  this->sshStrictHostKeyChecking.SetValue(val);
}

bool Configuration::IsSshStrictHostKeyCheckingSet() const {
  return sshStrictHostKeyChecking.IsSet();
}

const std::string& Configuration::GetSshKnownHostsFile() const {
  return sshKnownHostsFile.GetValue();
}

void Configuration::SetSshKnownHostsFile(const std::string& path) {
  this->sshKnownHostsFile.SetValue(path);
}

bool Configuration::IsSshKnownHostsFileSet() const {
  return sshKnownHostsFile.IsSet();
}

LogLevel::Type Configuration::GetLogLevel() const {
  return logLevel.GetValue();
}

void Configuration::SetLogLevel(const LogLevel::Type level) {
  this->logLevel.SetValue(level);
  Logger::GetLoggerInstance()->SetLogLevel(level);
}

bool Configuration::IsLogLevelSet() const {
  return logLevel.IsSet();
}

const std::string& Configuration::GetLogPath() const {
  return logPath.GetValue();
}

void Configuration::SetLogPath(const std::string& path) {
  if (common::IsValidDirectory(path)) {
    this->logPath.SetValue(path);
    Logger::GetLoggerInstance()->SetLogPath(path);
  }
}

bool Configuration::IsLogPathSet() const {
  return logPath.IsSet();
}

ScanMethod::Type Configuration::GetScanMethod() const {
  return scanMethod.GetValue();
}

void Configuration::SetScanMethod(const ScanMethod::Type method) {
  this->scanMethod.SetValue(method);
}

bool Configuration::IsScanMethodSet() const {
  return scanMethod.IsSet();
}

int32_t Configuration::GetScanLimit() const {
  return scanLimit.GetValue();
}

void Configuration::SetScanLimit(int32_t limit) {
  this->scanLimit.SetValue(limit);
}

bool Configuration::IsScanLimitSet() const {
  return scanLimit.IsSet();
}

const std::string& Configuration::GetSchemaName() const {
  return schemaName.GetValue();
}

void Configuration::SetSchemaName(const std::string& name) {
  this->schemaName.SetValue(name);
}

bool Configuration::IsSchemaNameSet() const {
  return schemaName.IsSet();
}

bool Configuration::IsRefreshSchema() const {
  return refreshSchema.GetValue();
}

void Configuration::SetRefreshSchema(bool val) {
  this->refreshSchema.SetValue(val);
}

bool Configuration::IsRefreshSchemaSet() const {
  return refreshSchema.IsSet();
}

const std::string& Configuration::GetUser() const {
  return user.GetValue();
}

void Configuration::SetUser(const std::string& username) {
  this->user.SetValue(username);
}

bool Configuration::IsUserSet() const {
  return user.IsSet();
}

const std::string& Configuration::GetPassword() const {
  return password.GetValue();
}

void Configuration::SetPassword(const std::string& pass) {
  this->password.SetValue(pass);
}

bool Configuration::IsPasswordSet() const {
  return password.IsSet();
}

int32_t Configuration::GetDefaultFetchSize() const {
  return defaultFetchSize.GetValue();
}

void Configuration::SetDefaultFetchSize(int32_t size) {
  this->defaultFetchSize.SetValue(size);
}

bool Configuration::IsDefaultFetchSizeSet() const {
  return defaultFetchSize.IsSet();
}

void Configuration::ToMap(ArgumentMap& res) const {
  AddToMap(res, ConnectionStringParser::Key::dsn, dsn);
  AddToMap(res, ConnectionStringParser::Key::driver, driver);
  AddToMap(res, ConnectionStringParser::Key::database, database);
  AddToMap(res, ConnectionStringParser::Key::hostname, hostname);
  AddToMap(res, ConnectionStringParser::Key::port, port);
  AddToMap(res, ConnectionStringParser::Key::user, user);
  AddToMap(res, ConnectionStringParser::Key::password, password);
  AddToMap(res, ConnectionStringParser::Key::appName, appName);
  AddToMap(res, ConnectionStringParser::Key::loginTimeoutSec, loginTimeoutSec);
  AddToMap(res, ConnectionStringParser::Key::readPreference, readPreference,
           false);
  AddToMap(res, ConnectionStringParser::Key::replicaSet, replicaSet);
  AddToMap(res, ConnectionStringParser::Key::retryReads, retryReads);
  AddToMap(res, ConnectionStringParser::Key::tls, tls);
  AddToMap(res, ConnectionStringParser::Key::tlsAllowInvalidHostnames,
           tlsAllowInvalidHostnames);
  AddToMap(res, ConnectionStringParser::Key::tlsCaFile, tlsCaFile);
  AddToMap(res, ConnectionStringParser::Key::sshEnable, sshEnable);
  AddToMap(res, ConnectionStringParser::Key::sshUser, sshUser);
  AddToMap(res, ConnectionStringParser::Key::sshHost, sshHost);
  AddToMap(res, ConnectionStringParser::Key::sshPrivateKeyFile,
           sshPrivateKeyFile);
  AddToMap(res, ConnectionStringParser::Key::sshPrivateKeyPassphrase,
           sshPrivateKeyPassphrase);
  AddToMap(res, ConnectionStringParser::Key::sshStrictHostKeyChecking,
           sshStrictHostKeyChecking);
  AddToMap(res, ConnectionStringParser::Key::sshKnownHostsFile,
           sshKnownHostsFile);
  AddToMap(res, ConnectionStringParser::Key::logLevel, logLevel);
  AddToMap(res, ConnectionStringParser::Key::logPath, logPath);
  AddToMap(res, ConnectionStringParser::Key::scanMethod, scanMethod, false);
  AddToMap(res, ConnectionStringParser::Key::scanLimit, scanLimit);
  AddToMap(res, ConnectionStringParser::Key::schemaName, schemaName);
  AddToMap(res, ConnectionStringParser::Key::refreshSchema, refreshSchema);
  AddToMap(res, ConnectionStringParser::Key::defaultFetchSize,
           defaultFetchSize);
}

void Configuration::Validate() const {
  // Validate minimum required properties.
  if (!IsHostnameSet() || !IsUserSet() || !IsPasswordSet()
      || !IsDatabaseSet()) {
    throw OdbcError(
        SqlState::S01S00_INVALID_CONNECTION_STRING_ATTRIBUTE,
        "Hostname, username, password, and database are required to connect.");
  }

  // Validate required SSH tunnel properties if needed.
  bool sshTunnel = IsSshEnable() || IsSshHostSet() || IsSshUserSet()
                   || IsSshPrivateKeyFileSet();
  if (sshTunnel
      && (!IsSshHostSet() || !IsSshUserSet() || !IsSshPrivateKeyFileSet())) {
    throw OdbcError(SqlState::S01S00_INVALID_CONNECTION_STRING_ATTRIBUTE,
                    "If using an internal SSH tunnel, all of ssh_host, "
                    "ssh_user, ssh_private_key_file are required to connect.");
  }
}

std::string Configuration::ToJdbcConnectionString() const {
  std::string jdbcConnectionString;
  jdbcConnectionString = "jdbc:documentdb:";
  jdbcConnectionString.append("//" + EncodeURIComponent(GetUser()));
  jdbcConnectionString.append(":" + EncodeURIComponent(GetPassword()));
  jdbcConnectionString.append("@" + GetHostname());
  jdbcConnectionString.append(":"
                              + common::LexicalCast< std::string >(GetPort()));
  jdbcConnectionString.append("/" + EncodeURIComponent(GetDatabase()));
  // Always pass application name even when unset to override the JDBC default
  // application name.
  jdbcConnectionString.append("?appName="
                              + EncodeURIComponent(GetApplicationName()));

  config::Configuration::ArgumentMap arguments;
  ToJdbcOptionsMap(arguments);
  std::stringstream options;
  for (config::Configuration::ArgumentMap::const_iterator it =
           arguments.begin();
       it != arguments.end(); ++it) {
    const std::string& key = it->first;
    const std::string& value = it->second;
    if (key != "logLevel" && key != "logPath" && !value.empty())
      options << '&' << key << '=' << EncodeURIComponent(value);
  }
  jdbcConnectionString.append(options.str());

  return jdbcConnectionString;
}

void Configuration::ToJdbcOptionsMap(ArgumentMap& res) const {
  AddToMap(res, "loginTimeoutSec", loginTimeoutSec);
  AddToMap(res, "readPreference", readPreference, true);
  AddToMap(res, "replicaSet", replicaSet);
  AddToMap(res, "retryReads", retryReads);
  AddToMap(res, "tls", tls);
  AddToMap(res, "tlsAllowInvalidHostnames", tlsAllowInvalidHostnames);
  AddToMap(res, "tlsCaFile", tlsCaFile);
  AddToMap(res, "sshUser", sshUser);
  AddToMap(res, "sshHost", sshHost);
  AddToMap(res, "sshPrivateKeyFile", sshPrivateKeyFile);
  AddToMap(res, "sshPrivateKeyPassphrase", sshPrivateKeyPassphrase);
  AddToMap(res, "sshStrictHostKeyChecking", sshStrictHostKeyChecking);
  AddToMap(res, "sshKnownHostsFile", sshKnownHostsFile);
  AddToMap(res, "logLevel", logLevel);
  AddToMap(res, "logPath", logPath);
  AddToMap(res, "scanMethod", scanMethod, true);
  AddToMap(res, "scanLimit", scanLimit);
  AddToMap(res, "schemaName", schemaName);
  AddToMap(res, "refreshSchema", refreshSchema);
  AddToMap(res, "defaultFetchSize", defaultFetchSize);
}

template <>
void Configuration::AddToMap(ArgumentMap& map, const std::string& key,
                             const SettableValue< uint16_t >& value) {
  if (value.IsSet())
    map[key] = common::LexicalCast< std::string >(value.GetValue());
}

template <>
void Configuration::AddToMap(ArgumentMap& map, const std::string& key,
                             const SettableValue< int32_t >& value) {
  if (value.IsSet())
    map[key] = common::LexicalCast< std::string >(value.GetValue());
}

template <>
void Configuration::AddToMap(ArgumentMap& map, const std::string& key,
                             const SettableValue< std::string >& value) {
  if (value.IsSet())
    map[key] = value.GetValue();
}

template <>
void Configuration::AddToMap(ArgumentMap& map, const std::string& key,
                             const SettableValue< bool >& value) {
  if (value.IsSet())
    map[key] = value.GetValue() ? "true" : "false";
}

template <>
void Configuration::AddToMap(ArgumentMap& map, const std::string& key,
                             const SettableValue< ReadPreference::Type >& value,
                             bool isJdbcFormat) {
  if (value.IsSet())
    map[key] = isJdbcFormat ? ReadPreference::ToJdbcString(value.GetValue())
                            : ReadPreference::ToString(value.GetValue());
}

template <>
void Configuration::AddToMap(ArgumentMap& map, const std::string& key,
                             const SettableValue< LogLevel::Type >& value) {
  if (value.IsSet())
    map[key] = LogLevel::ToString(value.GetValue());
}

template <>
void Configuration::AddToMap(ArgumentMap& map, const std::string& key,
                             const SettableValue< ScanMethod::Type >& value,
                             bool isJdbcFormat) {
  if (value.IsSet())
    map[key] = isJdbcFormat ? ScanMethod::ToJdbcString(value.GetValue())
                            : ScanMethod::ToString(value.GetValue());
}

}  // namespace config
}  // namespace odbc
}  // namespace ignite
