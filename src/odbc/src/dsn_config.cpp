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

#include "ignite/odbc/dsn_config.h"

#include <ignite/odbc/common/fixed_size_array.h>

#include "ignite/odbc/config/config_tools.h"
#include "ignite/odbc/config/connection_string_parser.h"
#include "ignite/odbc/system/odbc_constants.h"
#include "ignite/odbc/utility.h"

using namespace ignite::odbc::config;

#define BUFFER_SIZE (1024 * 1024)
#define CONFIG_FILE u8"ODBC.INI"

namespace ignite {
namespace odbc {
void ThrowLastSetupError() {
  DWORD code;
  common::FixedSizeArray< SQLWCHAR > msg(BUFFER_SIZE);

  SQLInstallerError(1, &code, msg.GetData(),
                    msg.GetSize(), NULL);

  std::stringstream buf;

  buf << "Message: \"" << utility::SqlStringToString(msg.GetData(), msg.GetSize()) << "\", Code: " << code;

  throw IgniteError(IgniteError::IGNITE_ERR_GENERIC, buf.str().c_str());
}

void WriteDsnString(const char* dsn, const char* key, const char* value) {
  if (!SQLWritePrivateProfileString(utility::ToWCHARVector(dsn).data(),
                                    utility::ToWCHARVector(key).data(),
                                    utility::ToWCHARVector(value).data(),
                                    utility::ToWCHARVector(CONFIG_FILE).data()))
    ThrowLastSetupError();
}

SettableValue< std::string > ReadDsnString(const char* dsn,
                                           const std::string& key,
                                           const std::string& dflt = "") {
  static const char* unique = "35a920dd-8837-43d2-a846-e01a2e7b5f84";

  SettableValue< std::string > val(dflt);

  common::FixedSizeArray< SQLWCHAR > buf(BUFFER_SIZE);
  
  int ret = SQLGetPrivateProfileString(
      utility::ToWCHARVector(dsn).data(), utility::ToWCHARVector(key).data(),
      utility::ToWCHARVector(unique).data(),
      buf.GetData(), buf.GetSize(),
      utility::ToWCHARVector(CONFIG_FILE).data());

  if (ret > BUFFER_SIZE) {
    buf.Reset(ret + 1);

    ret = SQLGetPrivateProfileString(
        utility::ToWCHARVector(dsn).data(),
        utility::ToWCHARVector(key).data(),
        utility::ToWCHARVector(unique).data(),
        buf.GetData(), buf.GetSize(),
        utility::ToWCHARVector(CONFIG_FILE).data());
  }

  std::string res = utility::SqlStringToString(buf.GetData());

  if (res != unique)
    val.SetValue(res);

  return val;
}

SettableValue< int32_t > ReadDsnInt(const char* dsn, const std::string& key,
                                    int32_t dflt = 0) {
  SettableValue< std::string > str = ReadDsnString(dsn, key, "");

  SettableValue< int32_t > res(dflt);

  if (str.IsSet())
    res.SetValue(common::LexicalCast< int, std::string >(str.GetValue()));

  return res;
}

SettableValue< bool > ReadDsnBool(const char* dsn, const std::string& key,
                                  bool dflt = false) {
  SettableValue< std::string > str = ReadDsnString(dsn, key, "");

  SettableValue< bool > res(dflt);

  if (str.IsSet())
    res.SetValue(str.GetValue() == "true");

  return res;
}

void ReadDsnConfiguration(const char* dsn, Configuration& config,
                          diagnostic::DiagnosticRecordStorage* diag) {
  SettableValue< std::string > hostname =
      ReadDsnString(dsn, ConnectionStringParser::Key::hostname);

  if (hostname.IsSet() && !config.IsHostnameSet())
    config.SetHostname(hostname.GetValue());

  SettableValue< int32_t > port =
      ReadDsnInt(dsn, ConnectionStringParser::Key::port);

  if (port.IsSet() && !config.IsPortSet())
    config.SetPort(static_cast< uint16_t >(port.GetValue()));

  SettableValue< std::string > database =
      ReadDsnString(dsn, ConnectionStringParser::Key::database);

  if (database.IsSet() && !config.IsDatabaseSet())
    config.SetDatabase(database.GetValue());

  SettableValue< std::string > user =
      ReadDsnString(dsn, ConnectionStringParser::Key::user);

  if (user.IsSet() && !config.IsUserSet())
    config.SetUser(user.GetValue());

  SettableValue< std::string > password =
      ReadDsnString(dsn, ConnectionStringParser::Key::password);

  if (password.IsSet() && !config.IsPasswordSet())
    config.SetPassword(password.GetValue());

  SettableValue< std::string > appName =
      ReadDsnString(dsn, ConnectionStringParser::Key::appName);

  if (appName.IsSet() && !config.IsApplicationNameSet())
    config.SetApplicationName(appName.GetValue());

  SettableValue< int32_t > loginTimeoutSec =
      ReadDsnInt(dsn, ConnectionStringParser::Key::loginTimeoutSec);

  if (loginTimeoutSec.IsSet() && !config.IsLoginTimeoutSecondsSet())
    config.SetLoginTimeoutSeconds(loginTimeoutSec.GetValue());

  SettableValue< std::string > readPreference =
      ReadDsnString(dsn, ConnectionStringParser::Key::readPreference);

  if (readPreference.IsSet() && !config.IsReadPreferenceSet()) {
    ReadPreference::Type preference = ReadPreference::FromString(
        readPreference.GetValue(), ReadPreference::Type::PRIMARY);
    config.SetReadPreference(preference);
  }

  SettableValue< std::string > replicaSet =
      ReadDsnString(dsn, ConnectionStringParser::Key::replicaSet);

  if (replicaSet.IsSet() && !config.IsReplicaSetSet())
    config.SetReplicaSet(replicaSet.GetValue());

  SettableValue< bool > retryReads =
      ReadDsnBool(dsn, ConnectionStringParser::Key::retryReads);

  if (retryReads.IsSet() && !config.IsRetryReadsSet())
    config.SetRetryReads(retryReads.GetValue());

  SettableValue< bool > tls =
      ReadDsnBool(dsn, ConnectionStringParser::Key::tls);

  if (tls.IsSet() && !config.IsTlsSet())
    config.SetTls(tls.GetValue());

  SettableValue< bool > tlsAllowInvalidHostnames =
      ReadDsnBool(dsn, ConnectionStringParser::Key::tlsAllowInvalidHostnames);

  if (tlsAllowInvalidHostnames.IsSet()
      && !config.IsTlsAllowInvalidHostnamesSet())
    config.SetTlsAllowInvalidHostnames(tlsAllowInvalidHostnames.GetValue());

  SettableValue< std::string > tlsCaFile =
      ReadDsnString(dsn, ConnectionStringParser::Key::tlsCaFile);

  if (tlsCaFile.IsSet() && !config.IsTlsCaFileSet())
    config.SetTlsCaFile(tlsCaFile.GetValue());

  SettableValue< bool > sshEnable =
      ReadDsnBool(dsn, ConnectionStringParser::Key::sshEnable);

  if (sshEnable.IsSet() && !config.IsSshEnableSet())
    config.SetSshEnable(sshEnable.GetValue());

  SettableValue< std::string > sshUser =
      ReadDsnString(dsn, ConnectionStringParser::Key::sshUser);

  if (sshUser.IsSet() && !config.IsSshUserSet())
    config.SetSshUser(sshUser.GetValue());

  SettableValue< std::string > sshHost =
      ReadDsnString(dsn, ConnectionStringParser::Key::sshHost);

  if (sshHost.IsSet() && !config.IsSshHostSet())
    config.SetSshHost(sshHost.GetValue());

  SettableValue< std::string > sshPrivateKeyFile =
      ReadDsnString(dsn, ConnectionStringParser::Key::sshPrivateKeyFile);

  if (sshPrivateKeyFile.IsSet() && !config.IsSshPrivateKeyFileSet())
    config.SetSshPrivateKeyFile(sshPrivateKeyFile.GetValue());

  SettableValue< std::string > sshPrivateKeyPassphrase =
      ReadDsnString(dsn, ConnectionStringParser::Key::sshPrivateKeyPassphrase);

  if (sshPrivateKeyPassphrase.IsSet() && !config.IsSshPrivateKeyPassphraseSet())
    config.SetSshPrivateKeyPassphrase(sshPrivateKeyPassphrase.GetValue());

  SettableValue< bool > sshStrictHostKeyChecking =
      ReadDsnBool(dsn, ConnectionStringParser::Key::sshStrictHostKeyChecking);

  if (sshStrictHostKeyChecking.IsSet()
      && !config.IsSshStrictHostKeyCheckingSet())
    config.SetSshStrictHostKeyChecking(sshStrictHostKeyChecking.GetValue());

  SettableValue< std::string > sshKnownHostsFile =
      ReadDsnString(dsn, ConnectionStringParser::Key::sshKnownHostsFile);

  if (sshKnownHostsFile.IsSet() && !config.IsSshKnownHostsFileSet())
    config.SetSshKnownHostsFile(sshKnownHostsFile.GetValue());

  SettableValue< std::string > logLevel =
      ReadDsnString(dsn, ConnectionStringParser::Key::logLevel);

  if (logLevel.IsSet() && !config.IsLogLevelSet()) {
    LogLevel::Type level =
        LogLevel::FromString(logLevel.GetValue(), LogLevel::Type::ERROR_LEVEL);
    config.SetLogLevel(level);
  }

  SettableValue< std::string > logPath =
      ReadDsnString(dsn, ConnectionStringParser::Key::logPath);

  if (logPath.IsSet() && !config.IsLogPathSet())
    config.SetLogPath(logPath.GetValue());

  SettableValue< std::string > scanMethod =
      ReadDsnString(dsn, ConnectionStringParser::Key::scanMethod);

  if (scanMethod.IsSet() && !config.IsScanMethodSet()) {
    ScanMethod::Type method =
        ScanMethod::FromString(scanMethod.GetValue(), ScanMethod::Type::RANDOM);
    config.SetScanMethod(method);
  }

  SettableValue< int32_t > scanLimit =
      ReadDsnInt(dsn, ConnectionStringParser::Key::scanLimit);

  if (scanLimit.IsSet() && !config.IsScanLimitSet() && scanLimit.GetValue() > 0)
    config.SetScanLimit(scanLimit.GetValue());

  SettableValue< std::string > schemaName =
      ReadDsnString(dsn, ConnectionStringParser::Key::schemaName);

  if (schemaName.IsSet() && !config.IsSchemaNameSet())
    config.SetSchemaName(schemaName.GetValue());

  SettableValue< bool > refreshSchema =
      ReadDsnBool(dsn, ConnectionStringParser::Key::refreshSchema);

  if (refreshSchema.IsSet() && !config.IsRefreshSchemaSet())
    config.SetRefreshSchema(refreshSchema.GetValue());

  SettableValue< int32_t > defaultFetchSize =
      ReadDsnInt(dsn, ConnectionStringParser::Key::defaultFetchSize);

  if (defaultFetchSize.IsSet() && !config.IsDefaultFetchSizeSet()
      && defaultFetchSize.GetValue() > 0)
    config.SetDefaultFetchSize(defaultFetchSize.GetValue());
}
}  // namespace odbc
}  // namespace ignite
