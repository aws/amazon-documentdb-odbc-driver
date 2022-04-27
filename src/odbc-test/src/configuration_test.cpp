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

#include <ignite/common/utils.h>
#include <ignite/odbc/config/config_tools.h>
#include <ignite/odbc/config/configuration.h>
#include <ignite/odbc/config/connection_string_parser.h>
#include <ignite/odbc/log.h>
#include <ignite/odbc/log_level.h>
#include <ignite/odbc/odbc_error.h>

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <set>

#include "ignite/odbc/diagnostic/diagnostic_record_storage.h"

using namespace ignite::odbc;
using namespace ignite::odbc::config;
using ignite::common::EncodeURIComponent;

namespace {
const std::string testDriverName = "Test Driver";
const std::string testDsn = "Test DSN";
const std::string testHostname = "testhost.com";
const uint16_t testServerPort = 27019;
const std::string testDatabaseName = "testDatabase";
const std::string testUsername = "testUser";
const std::string testPassword = "testPassword";
const std::string testAppName = "testAppName";
const int32_t testLoginTimeoutSec = 3000;
const ReadPreference::Type testReadPreference =
    ReadPreference::Type::PRIMARY_PREFERRED;
const std::string testReplicaSet = "rs0";
const bool testRetryReads = false;
const bool testTlsFlag = false;
const bool testTlsAllowInvalidHostnamesFlag = true;
const std::string testTlsCaFile = "/path/to/cafile";
const std::string testSshUser = "testEc2User";
const std::string testSshHost = "testsshhost.com";
const std::string testSshPrivateKeyFile = "/path/to/keyfile";
const std::string testSshPrivateKeyPassphrase = "testPassphrase";
const bool testSshStrictHostKeyCheckingFlag = false;
const std::string testLogPath = Logger::GetLoggerInstance()->GetLogPath();
const LogLevel::Type testLogLevel = Logger::GetLoggerInstance()->GetLogLevel();
const std::string testSshKnownHostsFile = "/path/to/knownhostsfile";
const ScanMethod::Type testScanMethod = ScanMethod::Type::ID_FORWARD;
const int32_t testScanLimit = 3000;
const std::string testSchemaName = "testSchemaName";
const bool testRefreshSchemaFlag = true;
const int32_t testDefaultFetchSize = 4321;

const EndPoint testAddress(testHostname, testServerPort, 10);
}  // namespace

const char* BoolToStr(bool val, bool lowerCase = true) {
  if (lowerCase)
    return val ? "true" : "false";

  return val ? "TRUE" : "FALSE";
}

void ParseValidDsnString(const std::string& dsnStr, Configuration& cfg) {
  ConnectionStringParser parser(cfg);

  diagnostic::DiagnosticRecordStorage diag;

  BOOST_CHECK_NO_THROW(parser.ParseConfigAttributes(dsnStr.c_str(), &diag));

  if (diag.GetStatusRecordsNumber() != 0)
    BOOST_FAIL(diag.GetStatusRecord(1).GetMessageText());
}

void ParseValidConnectString(const std::string& connectStr,
                             Configuration& cfg) {
  ConnectionStringParser parser(cfg);

  diagnostic::DiagnosticRecordStorage diag;

  BOOST_CHECK_NO_THROW(parser.ParseConnectionString(connectStr, &diag));

  if (diag.GetStatusRecordsNumber() != 0)
    BOOST_FAIL(diag.GetStatusRecord(1).GetMessageText());
}

void ParseConnectStringWithError(const std::string& connectStr,
                                 Configuration& cfg) {
  ConnectionStringParser parser(cfg);

  diagnostic::DiagnosticRecordStorage diag;

  BOOST_CHECK_NO_THROW(parser.ParseConnectionString(connectStr, &diag));

  BOOST_CHECK_NE(diag.GetStatusRecordsNumber(), 0);
}

void CheckValidAddress(const char* connectStr, const EndPoint& endPoint) {
  Configuration cfg;

  ParseValidConnectString(connectStr, cfg);

  BOOST_CHECK_EQUAL(cfg.GetHostname(), endPoint.host);
  BOOST_CHECK_EQUAL(cfg.GetPort(), endPoint.port);
}

void CheckValidLogLevel(const char* connectStr, LogLevel::Type loglevel) {
  Configuration cfg;

  ParseValidConnectString(connectStr, cfg);

  BOOST_CHECK(cfg.GetLogLevel() == loglevel);
}

void CheckInvalidLogLevel(const char* connectStr) {
  Configuration cfg;

  ParseConnectStringWithError(connectStr, cfg);

  BOOST_CHECK(cfg.GetLogLevel() == Configuration::DefaultValue::logLevel);
}

void CheckValidScanMethod(const char* connectStr, ScanMethod::Type scanMethod) {
  Configuration cfg;

  ParseValidConnectString(connectStr, cfg);

  BOOST_CHECK(cfg.GetScanMethod() == scanMethod);
}

void CheckInvalidScanMethod(const char* connectStr) {
  Configuration cfg;

  ParseConnectStringWithError(connectStr, cfg);

  BOOST_CHECK(cfg.GetScanMethod() == Configuration::DefaultValue::scanMethod);
}

void CheckValidReadPreference(const char* connectStr,
                              ReadPreference::Type preference) {
  Configuration cfg;

  ParseValidConnectString(connectStr, cfg);

  BOOST_CHECK(cfg.GetReadPreference() == preference);
}

void CheckInvalidReadPreference(const char* connectStr) {
  Configuration cfg;

  ParseConnectStringWithError(connectStr, cfg);

  BOOST_CHECK(cfg.GetReadPreference()
              == Configuration::DefaultValue::readPreference);
}

void CheckValidBoolValue(const std::string& connectStr, const std::string& key,
                         bool val) {
  Configuration cfg;

  ParseValidConnectString(connectStr, cfg);

  Configuration::ArgumentMap map;
  cfg.ToMap(map);

  std::string expected = val ? "true" : "false";

  BOOST_CHECK_EQUAL(map[key], expected);
}

void CheckInvalidBoolValue(const std::string& connectStr,
                           const std::string& key) {
  Configuration cfg;

  ParseConnectStringWithError(connectStr, cfg);

  Configuration::ArgumentMap map;
  cfg.ToMap(map);

  BOOST_CHECK(map[key].empty());
}

void CheckConnectionConfig(const Configuration& cfg) {
  BOOST_CHECK_EQUAL(cfg.GetDriver(), testDriverName);
  BOOST_CHECK_EQUAL(cfg.GetDatabase(), testDatabaseName);
  BOOST_CHECK_EQUAL(cfg.GetHostname(), testHostname);
  BOOST_CHECK_EQUAL(cfg.GetPort(), testServerPort);
  BOOST_CHECK_EQUAL(cfg.GetUser(), testUsername);
  BOOST_CHECK_EQUAL(cfg.GetPassword(), testPassword);
  BOOST_CHECK_EQUAL(cfg.GetApplicationName(), testAppName);
  BOOST_CHECK_EQUAL(cfg.GetLoginTimeoutSeconds(), testLoginTimeoutSec);
  BOOST_CHECK_EQUAL(cfg.GetReplicaSet(), testReplicaSet);
  BOOST_CHECK_EQUAL(cfg.IsRetryReads(), testRetryReads);
  BOOST_CHECK_EQUAL(cfg.IsTls(), testTlsFlag);
  BOOST_CHECK_EQUAL(cfg.IsTlsAllowInvalidHostnames(),
                    testTlsAllowInvalidHostnamesFlag);
  BOOST_CHECK_EQUAL(cfg.GetTlsCaFile(), testTlsCaFile);
  BOOST_CHECK_EQUAL(cfg.GetSshUser(), testSshUser);
  BOOST_CHECK_EQUAL(cfg.GetSshHost(), testSshHost);
  BOOST_CHECK_EQUAL(cfg.GetSshPrivateKeyFile(), testSshPrivateKeyFile);
  BOOST_CHECK_EQUAL(cfg.GetSshPrivateKeyPassphrase(),
                    testSshPrivateKeyPassphrase);
  BOOST_CHECK_EQUAL(cfg.IsSshStrictHostKeyChecking(),
                    testSshStrictHostKeyCheckingFlag);
  BOOST_CHECK_EQUAL(cfg.GetSshKnownHostsFile(), testSshKnownHostsFile);
  BOOST_CHECK(cfg.GetLogLevel() == testLogLevel);
  BOOST_CHECK(cfg.GetLogPath() == testLogPath);
  BOOST_CHECK_EQUAL(cfg.GetScanLimit(), testScanLimit);
  BOOST_CHECK_EQUAL(cfg.GetSchemaName(), testSchemaName);
  BOOST_CHECK_EQUAL(cfg.IsRefreshSchema(), testRefreshSchemaFlag);
  BOOST_CHECK_EQUAL(cfg.GetDefaultFetchSize(), testDefaultFetchSize);
  BOOST_CHECK(!cfg.IsDsnSet());
  BOOST_CHECK(cfg.GetReadPreference() == testReadPreference);
  BOOST_CHECK(cfg.GetScanMethod() == testScanMethod);

  std::stringstream constructor;
  constructor << "app_name=" << testAppName << ';'
              << "database=" << testDatabaseName << ';'
              << "default_fetch_size=" << testDefaultFetchSize << ';'
              << "driver={" << testDriverName << "};"
              << "hostname=" << testHostname << ';'
              << "log_level=" << LogLevel::ToString(testLogLevel) << ';'
              << "log_path=" << testLogPath << ';'
              << "login_timeout_sec=" << testLoginTimeoutSec << ';'
              << "password=" << testPassword << ';' << "port=" << testServerPort
              << ';' << "read_preference="
              << ReadPreference::ToString(testReadPreference) << ';'
              << "refresh_schema=" << BoolToStr(testRefreshSchemaFlag) << ';'
              << "replica_set=" << testReplicaSet << ';'
              << "retry_reads=" << BoolToStr(testRetryReads) << ';'
              << "scan_limit=" << testScanLimit << ';'
              << "scan_method=" << ScanMethod::ToString(testScanMethod) << ';'
              << "schema_name=" << testSchemaName << ';'
              << "ssh_host=" << testSshHost << ';'
              << "ssh_known_hosts_file=" << testSshKnownHostsFile << ';'
              << "ssh_private_key_file=" << testSshPrivateKeyFile << ';'
              << "ssh_private_key_passphrase=" << testSshPrivateKeyPassphrase
              << ';' << "ssh_strict_host_key_checking="
              << BoolToStr(testSshStrictHostKeyCheckingFlag) << ';'
              << "ssh_user=" << testSshUser << ';'
              << "tls=" << BoolToStr(testTlsFlag) << ';'
              << "tls_allow_invalid_hostnames="
              << BoolToStr(testTlsAllowInvalidHostnamesFlag) << ';'
              << "tls_ca_file=" << testTlsCaFile << ';'
              << "user=" << testUsername << ';';
  const std::string& expectedStr = constructor.str();

  std::stringstream jdbcConstructor;
  jdbcConstructor
      << "jdbc:documentdb://" << testUsername << ":" << testPassword << "@"
      << testHostname << ':' << testServerPort << "/" << testDatabaseName << '?'
      << "appName=" << testAppName
      << "&defaultFetchSize=" << testDefaultFetchSize
      << "&loginTimeoutSec=" << testLoginTimeoutSec
      << "&readPreference=" << ReadPreference::ToJdbcString(testReadPreference)
      << "&refreshSchema=" << BoolToStr(testRefreshSchemaFlag)
      << "&replicaSet=" << testReplicaSet
      << "&retryReads=" << BoolToStr(testRetryReads)
      << "&scanLimit=" << testScanLimit
      << "&scanMethod=" << ScanMethod::ToJdbcString(testScanMethod)
      << "&schemaName=" << testSchemaName << "&sshHost=" << testSshHost
      << "&sshKnownHostsFile=" << EncodeURIComponent(testSshKnownHostsFile)
      << "&sshPrivateKeyFile=" << EncodeURIComponent(testSshPrivateKeyFile)
      << "&sshPrivateKeyPassphrase=" << testSshPrivateKeyPassphrase
      << "&sshStrictHostKeyChecking="
      << BoolToStr(testSshStrictHostKeyCheckingFlag)
      << "&sshUser=" << testSshUser
      << "&tls=" << BoolToStr(testTlsFlag)
      << "&tlsAllowInvalidHostnames="
      << BoolToStr(testTlsAllowInvalidHostnamesFlag)
      << "&tlsCaFile=" << EncodeURIComponent(testTlsCaFile);
  const std::string& expectedJdbcStr = jdbcConstructor.str();

  BOOST_CHECK_EQUAL(ignite::common::ToLower(cfg.ToConnectString()),
                    ignite::common::ToLower(expectedStr));
  BOOST_CHECK_EQUAL(cfg.ToJdbcConnectionString(), expectedJdbcStr);
}

void CheckDsnConfig(const Configuration& cfg) {
  // since setting logger path/level will change the logger settings,
  // we will not change the logger path/level in configuration_test,
  // which means it is possible for the logger path/level to be equivalent to 
  // the default values. Therefore, there will be no boost check for logger path/level.
  BOOST_CHECK_EQUAL(cfg.GetDriver(), testDriverName);
  BOOST_CHECK_EQUAL(cfg.GetDsn(), testDsn);
  BOOST_CHECK_EQUAL(cfg.GetDatabase(), Configuration::DefaultValue::database);
  BOOST_CHECK_EQUAL(cfg.GetHostname(), Configuration::DefaultValue::hostname);
  BOOST_CHECK_EQUAL(cfg.GetPort(), Configuration::DefaultValue::port);
  BOOST_CHECK_EQUAL(cfg.GetUser(), Configuration::DefaultValue::user);
  BOOST_CHECK_EQUAL(cfg.GetPassword(), Configuration::DefaultValue::password);
  BOOST_CHECK_EQUAL(cfg.GetApplicationName(),
                    Configuration::DefaultValue::appName);
  BOOST_CHECK_EQUAL(cfg.GetLoginTimeoutSeconds(),
                    Configuration::DefaultValue::loginTimeoutSec);
  BOOST_CHECK_EQUAL(cfg.GetReplicaSet(),
                    Configuration::DefaultValue::replicaSet);
  BOOST_CHECK_EQUAL(cfg.IsRetryReads(),
                    Configuration::DefaultValue::retryReads);
  BOOST_CHECK_EQUAL(cfg.IsTls(), Configuration::DefaultValue::tls);
  BOOST_CHECK_EQUAL(cfg.IsTlsAllowInvalidHostnames(),
                    Configuration::DefaultValue::tlsAllowInvalidHostnames);
  BOOST_CHECK_EQUAL(cfg.GetTlsCaFile(), Configuration::DefaultValue::tlsCaFile);
  BOOST_CHECK_EQUAL(cfg.GetSshUser(), Configuration::DefaultValue::sshUser);
  BOOST_CHECK_EQUAL(cfg.GetSshHost(), Configuration::DefaultValue::sshHost);
  BOOST_CHECK_EQUAL(cfg.GetSshPrivateKeyFile(),
                    Configuration::DefaultValue::sshPrivateKeyFile);
  BOOST_CHECK_EQUAL(cfg.GetSshPrivateKeyPassphrase(),
                    Configuration::DefaultValue::sshPrivateKeyPassphrase);
  BOOST_CHECK_EQUAL(cfg.IsSshStrictHostKeyChecking(),
                    Configuration::DefaultValue::sshStrictHostKeyChecking);
  BOOST_CHECK_EQUAL(cfg.GetSshKnownHostsFile(),
                    Configuration::DefaultValue::sshKnownHostsFile);
  BOOST_CHECK_EQUAL(cfg.GetScanLimit(), Configuration::DefaultValue::scanLimit);
  BOOST_CHECK_EQUAL(cfg.GetSchemaName(),
                    Configuration::DefaultValue::schemaName);
  BOOST_CHECK_EQUAL(cfg.IsRefreshSchema(),
                    Configuration::DefaultValue::refreshSchema);
  BOOST_CHECK_EQUAL(cfg.GetDefaultFetchSize(),
                    Configuration::DefaultValue::defaultFetchSize);
  BOOST_CHECK(cfg.GetReadPreference()
              == Configuration::DefaultValue::readPreference);
  BOOST_CHECK(cfg.GetScanMethod() == Configuration::DefaultValue::scanMethod);
}

BOOST_AUTO_TEST_SUITE(ConfigurationTestSuite)

BOOST_AUTO_TEST_CASE(CheckTestValuesNotEqualDefault) {
  BOOST_CHECK_NE(testDriverName, Configuration::DefaultValue::driver);
  BOOST_CHECK_NE(testDsn, Configuration::DefaultValue::dsn);
  BOOST_CHECK_NE(testDatabaseName, Configuration::DefaultValue::database);
  BOOST_CHECK_NE(testHostname, Configuration::DefaultValue::hostname);
  BOOST_CHECK_NE(testServerPort, Configuration::DefaultValue::port);
  BOOST_CHECK_NE(testUsername, Configuration::DefaultValue::user);
  BOOST_CHECK_NE(testPassword, Configuration::DefaultValue::password);
  BOOST_CHECK_NE(testAppName, Configuration::DefaultValue::appName);
  BOOST_CHECK_NE(testLoginTimeoutSec,
                 Configuration::DefaultValue::loginTimeoutSec);
  BOOST_CHECK_NE(testReplicaSet, Configuration::DefaultValue::replicaSet);
  BOOST_CHECK_NE(testRetryReads, Configuration::DefaultValue::retryReads);
  BOOST_CHECK_NE(testTlsFlag, Configuration::DefaultValue::tls);
  BOOST_CHECK_NE(testTlsAllowInvalidHostnamesFlag,
                 Configuration::DefaultValue::tlsAllowInvalidHostnames);
  BOOST_CHECK_NE(testTlsCaFile, Configuration::DefaultValue::tlsCaFile);
  BOOST_CHECK_NE(testSshUser, Configuration::DefaultValue::sshUser);
  BOOST_CHECK_NE(testSshHost, Configuration::DefaultValue::sshHost);
  BOOST_CHECK_NE(testSshPrivateKeyFile,
                 Configuration::DefaultValue::sshPrivateKeyFile);
  BOOST_CHECK_NE(testSshPrivateKeyPassphrase,
                 Configuration::DefaultValue::sshPrivateKeyPassphrase);
  BOOST_CHECK_NE(testSshStrictHostKeyCheckingFlag,
                 Configuration::DefaultValue::sshStrictHostKeyChecking);
  BOOST_CHECK_NE(testSshKnownHostsFile,
                 Configuration::DefaultValue::sshKnownHostsFile);
  BOOST_CHECK_NE(testScanLimit, Configuration::DefaultValue::scanLimit);
  BOOST_CHECK_NE(testSchemaName, Configuration::DefaultValue::schemaName);
  BOOST_CHECK_NE(testRefreshSchemaFlag,
                 Configuration::DefaultValue::refreshSchema);
  BOOST_CHECK_NE(testDefaultFetchSize,
                 Configuration::DefaultValue::defaultFetchSize);
  BOOST_CHECK(testReadPreference
              != Configuration::DefaultValue::readPreference);
  BOOST_CHECK(testScanMethod != Configuration::DefaultValue::scanMethod);
}

BOOST_AUTO_TEST_CASE(TestConnectStringUppercase) {
  Configuration cfg;

  std::stringstream constructor;

  constructor << "HOSTNAME=" << testHostname << ';' << "PORT=" << testServerPort
              << ';' << "DATABASE=" << testDatabaseName << ';'
              << "USER=" << testUsername << ';' << "PASSWORD=" << testPassword
              << ';' << "APP_NAME=" << testAppName << ';'
              << "LOG_LEVEL=" << LogLevel::ToString(testLogLevel) << ';'
              << "LOG_PATH=" << testLogPath << ';'
              << "LOGIN_TIMEOUT_SEC=" << testLoginTimeoutSec << ';'
              << "READ_PREFERENCE="
              << ReadPreference::ToString(testReadPreference) << ';'
              << "REPLICA_SET=" << testReplicaSet << ';'
              << "RETRY_READS=" << BoolToStr(testRetryReads) << ';'
              << "TLS=" << BoolToStr(testTlsFlag) << ';'
              << "TLS_ALLOW_INVALID_HOSTNAMES="
              << BoolToStr(testTlsAllowInvalidHostnamesFlag) << ';'
              << "TLS_CA_FILE=" << testTlsCaFile << ';'
              << "SSH_USER=" << testSshUser << ';' << "SSH_HOST=" << testSshHost
              << ';' << "SSH_PRIVATE_KEY_FILE=" << testSshPrivateKeyFile << ';'
              << "SSH_PRIVATE_KEY_PASSPHRASE=" << testSshPrivateKeyPassphrase
              << ';' << "SSH_STRICT_HOST_KEY_CHECKING="
              << BoolToStr(testSshStrictHostKeyCheckingFlag) << ';'
              << "SSH_KNOWN_HOSTS_FILE=" << testSshKnownHostsFile << ';'
              << "SCAN_METHOD=" << ScanMethod::ToString(testScanMethod) << ';'
              << "SCAN_LIMIT=" << testScanLimit << ';'
              << "SCHEMA_NAME=" << testSchemaName << ';'
              << "REFRESH_SCHEMA=" << BoolToStr(testRefreshSchemaFlag) << ';'
              << "DEFAULT_FETCH_SIZE=" << testDefaultFetchSize << ';'
              << "DRIVER={" << testDriverName << "};";

  const std::string& connectStr = constructor.str();

  ParseValidConnectString(connectStr, cfg);

  CheckConnectionConfig(cfg);
}

BOOST_AUTO_TEST_CASE(TestConnectStringLowercase) {
  Configuration cfg;

  std::stringstream constructor;

  constructor << "hostname=" << testHostname << ';' << "port=" << testServerPort
              << ';' << "database=" << testDatabaseName << ';'
              << "user=" << testUsername << ';' << "password=" << testPassword
              << ';' << "app_name=" << testAppName << ';'
              << "log_level=" << LogLevel::ToString(testLogLevel) << ';'
              << "log_path=" << testLogPath << ';'
              << "login_timeout_sec=" << testLoginTimeoutSec << ';'
              << "read_preference="
              << ReadPreference::ToString(testReadPreference) << ';'
              << "replica_set=" << testReplicaSet << ';'
              << "retry_reads=" << BoolToStr(testRetryReads) << ';'
              << "tls=" << BoolToStr(testTlsFlag) << ';'
              << "tls_allow_invalid_hostnames="
              << BoolToStr(testTlsAllowInvalidHostnamesFlag) << ';'
              << "tls_ca_file=" << testTlsCaFile << ';'
              << "ssh_user=" << testSshUser << ';' << "ssh_host=" << testSshHost
              << ';' << "ssh_private_key_file=" << testSshPrivateKeyFile << ';'
              << "ssh_private_key_passphrase=" << testSshPrivateKeyPassphrase
              << ';' << "ssh_strict_host_key_checking="
              << BoolToStr(testSshStrictHostKeyCheckingFlag) << ';'
              << "ssh_known_hosts_file=" << testSshKnownHostsFile << ';'
              << "scan_method=" << ScanMethod::ToString(testScanMethod) << ';'
              << "scan_limit=" << testScanLimit << ';'
              << "schema_name=" << testSchemaName << ';'
              << "refresh_schema=" << BoolToStr(testRefreshSchemaFlag) << ';'
              << "default_fetch_size=" << testDefaultFetchSize << ';'
              << "driver={" << testDriverName << "};";

  const std::string& connectStr = constructor.str();

  ParseValidConnectString(connectStr, cfg);

  CheckConnectionConfig(cfg);
}

BOOST_AUTO_TEST_CASE(TestConnectStringZeroTerminated) {
  Configuration cfg;

  std::stringstream constructor;

  constructor << "hostname=" << testHostname << ';' << "port=" << testServerPort
              << ';' << "database=" << testDatabaseName << ';'
              << "user=" << testUsername << ';' << "password=" << testPassword
              << ';' << "app_name=" << testAppName << ';'
              << "log_level=" << LogLevel::ToString(testLogLevel) << ';'
              << "log_path=" << testLogPath << ';'
              << "login_timeout_sec=" << testLoginTimeoutSec << ';'
              << "read_preference="
              << ReadPreference::ToString(testReadPreference) << ';'
              << "replica_set=" << testReplicaSet << ';'
              << "retry_reads=" << BoolToStr(testRetryReads) << ';'
              << "tls=" << BoolToStr(testTlsFlag) << ';'
              << "tls_allow_invalid_hostnames="
              << BoolToStr(testTlsAllowInvalidHostnamesFlag) << ';'
              << "tls_ca_file=" << testTlsCaFile << ';'
              << "ssh_user=" << testSshUser << ';' << "ssh_host=" << testSshHost
              << ';' << "ssh_private_key_file=" << testSshPrivateKeyFile << ';'
              << "ssh_private_key_passphrase=" << testSshPrivateKeyPassphrase
              << ';' << "ssh_strict_host_key_checking="
              << BoolToStr(testSshStrictHostKeyCheckingFlag) << ';'
              << "ssh_known_hosts_file=" << testSshKnownHostsFile << ';'
              << "scan_method=" << ScanMethod::ToString(testScanMethod) << ';'
              << "scan_limit=" << testScanLimit << ';'
              << "schema_name=" << testSchemaName << ';'
              << "refresh_schema=" << BoolToStr(testRefreshSchemaFlag) << ';'
              << "default_fetch_size=" << testDefaultFetchSize << ';'
              << "driver={" << testDriverName << "};";

  std::string connectStr = constructor.str();

  connectStr.push_back(0);

  ParseValidConnectString(connectStr, cfg);

  CheckConnectionConfig(cfg);
}

BOOST_AUTO_TEST_CASE(TestConnectStringMixed) {
  Configuration cfg;

  std::stringstream constructor;

  constructor << "Hostname=" << testHostname << ';' << "Port=" << testServerPort
              << ';' << "Database=" << testDatabaseName << ';'
              << "User=" << testUsername << ';' << "Password=" << testPassword
              << ';' << "App_Name=" << testAppName << ';'
              << "Log_Level=" << LogLevel::ToString(testLogLevel) << ';'
              << "Log_Path=" << testLogPath << ';'
              << "Login_Timeout_Sec=" << testLoginTimeoutSec << ';'
              << "Read_Preference="
              << ReadPreference::ToString(testReadPreference) << ';'
              << "Replica_Set=" << testReplicaSet << ';'
              << "Retry_Reads=" << BoolToStr(testRetryReads) << ';'
              << "Tls=" << BoolToStr(testTlsFlag) << ';'
              << "Tls_Allow_Invalid_Hostnames="
              << BoolToStr(testTlsAllowInvalidHostnamesFlag) << ';'
              << "Tls_Ca_File=" << testTlsCaFile << ';'
              << "Ssh_User=" << testSshUser << ';' << "Ssh_Host=" << testSshHost
              << ';' << "Ssh_Private_Key_File=" << testSshPrivateKeyFile << ';'
              << "Ssh_Private_Key_Passphrase=" << testSshPrivateKeyPassphrase
              << ';' << "Ssh_Strict_Host_Key_Checking="
              << BoolToStr(testSshStrictHostKeyCheckingFlag) << ';'
              << "Ssh_Known_Hosts_File=" << testSshKnownHostsFile << ';'
              << "Scan_Method=" << ScanMethod::ToString(testScanMethod) << ';'
              << "Scan_Limit=" << testScanLimit << ';'
              << "Schema_Name=" << testSchemaName << ';'
              << "Refresh_Schema=" << BoolToStr(testRefreshSchemaFlag) << ';'
              << "Default_Fetch_Size=" << testDefaultFetchSize << ';'
              << "Driver={" << testDriverName << "};";

  const std::string& connectStr = constructor.str();

  ParseValidConnectString(connectStr, cfg);

  CheckConnectionConfig(cfg);
}

BOOST_AUTO_TEST_CASE(TestConnectStringWhitepaces) {
  Configuration cfg;

  std::stringstream constructor;

  constructor << "  HOSTNAME=" << testHostname << ';'
              << "PORT  =" << testServerPort << ';'
              << "  DATABASE=" << testDatabaseName << ';'
              << "USER =" << testUsername << ';' << "PASSWORD=" << testPassword
              << ';' << "APP_NAME=" << testAppName << ';'
              << "  LOG_LEVEL =" << LogLevel::ToString(testLogLevel) << ';'
              << "LOG_PATH=  " << testLogPath << ';'
              << "LOGIN_TIMEOUT_SEC=" << testLoginTimeoutSec << ';'
              << "READ_PREFERENCE="
              << ReadPreference::ToString(testReadPreference) << ';'
              << " REPLICA_SET=" << testReplicaSet << ';'
              << "RETRY_READS=" << BoolToStr(testRetryReads) << ';'
              << "TLS =" << BoolToStr(testTlsFlag) << ';'
              << "  TLS_ALLOW_INVALID_HOSTNAMES="
              << BoolToStr(testTlsAllowInvalidHostnamesFlag) << ';'
              << "TLS_CA_FILE=" << testTlsCaFile << ';'
              << "   SSH_USER=" << testSshUser << ';'
              << "SSH_HOST=" << testSshHost << ';'
              << "SSH_PRIVATE_KEY_FILE=  " << testSshPrivateKeyFile << ';'
              << " SSH_PRIVATE_KEY_PASSPHRASE= " << testSshPrivateKeyPassphrase
              << ';' << "  SSH_STRICT_HOST_KEY_CHECKING= "
              << BoolToStr(testSshStrictHostKeyCheckingFlag) << ';'
              << " SSH_KNOWN_HOSTS_FILE = " << testSshKnownHostsFile << ';'
              << "  SCAN_METHOD  =  " << ScanMethod::ToString(testScanMethod)
              << ';' << " SCAN_LIMIT= " << testScanLimit << ';'
              << "SCHEMA_NAME=" << testSchemaName << " ;              "
              << " REFRESH_SCHEMA=" << BoolToStr(testRefreshSchemaFlag) << ';'
              << " DEFAULT_FETCH_SIZE = " << testDefaultFetchSize << "   ;  "
              << "DRIVER = {" << testDriverName << "};";

  const std::string& connectStr = constructor.str();

  ParseValidConnectString(connectStr, cfg);

  CheckConnectionConfig(cfg);
}

BOOST_AUTO_TEST_CASE(TestConnectStringInvalidAddress) {
  Configuration cfg;

  ParseConnectStringWithError("hostname=example.com:0;", cfg);
  ParseConnectStringWithError("hostname=example.com:00000;", cfg);
  ParseConnectStringWithError("hostname=example.com:fdsf;", cfg);
  ParseConnectStringWithError("hostname=example.com:123:1;", cfg);
  ParseConnectStringWithError("hostname=example.com:12322221;", cfg);
  ParseConnectStringWithError("hostname=example.com:12322a;", cfg);
  ParseConnectStringWithError("hostname=example.com:;", cfg);
}

BOOST_AUTO_TEST_CASE(TestConnectStringValidAddress) {
  CheckValidAddress("hostname=example.com:1;", EndPoint("example.com", 1));
  CheckValidAddress("hostname=example.com:31242;",
                    EndPoint("example.com", 31242));
  CheckValidAddress("hostname=example.com:55555;",
                    EndPoint("example.com", 55555));
  CheckValidAddress("hostname=example.com:110;port=27019;",
                    EndPoint("example.com", 27019));
  CheckValidAddress("hostname=example.com;",
                    EndPoint("example.com", Configuration::DefaultValue::port));
  CheckValidAddress("hostname=example.com:1000..1010;",
                    EndPoint("example.com", 1000, 10));
}

BOOST_AUTO_TEST_CASE(TestConnectStringInvalidLogLevel) {
  CheckInvalidLogLevel("log_level=debug_level;");
  CheckInvalidLogLevel("log_level=off_level;");
}

BOOST_AUTO_TEST_CASE(TestConnectStringValidLogLevel) {
  CheckValidLogLevel("log_level=debug;", LogLevel::Type::DEBUG_LEVEL);
  CheckValidLogLevel("log_level=info;", LogLevel::Type::INFO_LEVEL);
  CheckValidLogLevel("log_level=error;", LogLevel::Type::ERROR_LEVEL);
  CheckValidLogLevel("log_level=off;", LogLevel::Type::OFF);
}

BOOST_AUTO_TEST_CASE(TestConnectStringInvalidScanMethod) {
  CheckInvalidScanMethod("scan_method=forward;");
  CheckInvalidScanMethod("scan_method=id_random;");
}

BOOST_AUTO_TEST_CASE(TestConnectStringValidScanMethod) {
  CheckValidScanMethod("scan_method=all;", ScanMethod::Type::ALL);
  CheckValidScanMethod("scan_method=id_forward;", ScanMethod::Type::ID_FORWARD);
  CheckValidScanMethod("scan_method=id_reverse;", ScanMethod::Type::ID_REVERSE);
  CheckValidScanMethod("scan_method=random;", ScanMethod::Type::RANDOM);
}

BOOST_AUTO_TEST_CASE(TestConnectStringInvalidReadPreference) {
  CheckInvalidReadPreference("read_preference=primary_nearest;");
  CheckInvalidReadPreference("read_preference=nearest_preferred;");
}

BOOST_AUTO_TEST_CASE(TestConnectStringValidReadPreference) {
  CheckValidReadPreference("read_preference=primary;",
                           ReadPreference::Type::PRIMARY);
  CheckValidReadPreference("read_preference=primary_preferred;",
                           ReadPreference::Type::PRIMARY_PREFERRED);
  CheckValidReadPreference("read_preference=secondary;",
                           ReadPreference::Type::SECONDARY);
  CheckValidReadPreference("read_preference=secondary_preferred;",
                           ReadPreference::Type::SECONDARY_PREFERRED);
  CheckValidReadPreference("read_preference=nearest;",
                           ReadPreference::Type::NEAREST);
}

BOOST_AUTO_TEST_CASE(TestConnectStringInvalidBoolKeys) {
  typedef std::set< std::string > Set;

  Set keys;

  keys.emplace("retry_reads");
  keys.emplace("tls");
  keys.emplace("tls_allow_invalid_hostnames");
  keys.emplace("ssh_strict_host_key_checking");
  keys.emplace("refresh_schema");

  for (auto it = keys.begin(); it != keys.end(); ++it) {
    const std::string& key = *it;

    CheckInvalidBoolValue(key + "=1;", key);
    CheckInvalidBoolValue(key + "=0;", key);
    CheckInvalidBoolValue(key + "=42;", key);
    CheckInvalidBoolValue(key + "=truee;", key);
    CheckInvalidBoolValue(key + "=flase;", key);
    CheckInvalidBoolValue(key + "=falsee;", key);
    CheckInvalidBoolValue(key + "=yes;", key);
    CheckInvalidBoolValue(key + "=no;", key);
  }
}

BOOST_AUTO_TEST_CASE(TestConnectStringValidBoolKeys) {
  typedef std::set< std::string > Set;

  Set keys;

  keys.emplace("retry_reads");
  keys.emplace("tls");
  keys.emplace("tls_allow_invalid_hostnames");
  keys.emplace("ssh_strict_host_key_checking");
  keys.emplace("refresh_schema");

  for (auto it = keys.begin(); it != keys.end(); ++it) {
    const std::string& key = *it;

    CheckValidBoolValue(key + "=true;", key, true);
    CheckValidBoolValue(key + "=True;", key, true);
    CheckValidBoolValue(key + "=TRUE;", key, true);

    CheckValidBoolValue(key + "=false;", key, false);
    CheckValidBoolValue(key + "=False;", key, false);
    CheckValidBoolValue(key + "=FALSE;", key, false);
  }
}

BOOST_AUTO_TEST_CASE(TestDsnStringUppercase) {
  Configuration cfg;

  std::stringstream constructor;

  constructor << "DRIVER=" << testDriverName << '\0' << "DSN={" << testDsn
              << "}" << '\0' << '\0';

  const std::string& configStr = constructor.str();

  ParseValidDsnString(configStr, cfg);

  CheckDsnConfig(cfg);
}

BOOST_AUTO_TEST_CASE(TestDsnStringLowercase) {
  Configuration cfg;

  std::stringstream constructor;

  constructor << "driver=" << testDriverName << '\0' << "dsn={" << testDsn
              << "}" << '\0' << '\0';

  const std::string& configStr = constructor.str();

  ParseValidDsnString(configStr, cfg);

  CheckDsnConfig(cfg);
}

BOOST_AUTO_TEST_CASE(TestDsnStringMixed) {
  Configuration cfg;

  std::stringstream constructor;

  constructor << "Driver=" << testDriverName << '\0' << "Dsn={" << testDsn
              << "}" << '\0' << '\0';

  const std::string& configStr = constructor.str();

  ParseValidDsnString(configStr, cfg);

  CheckDsnConfig(cfg);
}

BOOST_AUTO_TEST_CASE(TestDsnStringWhitespaces) {
  Configuration cfg;

  std::stringstream constructor;

  constructor << " DRIVER =  " << testDriverName << "\r\n"
              << '\0' << "DSN= {" << testDsn << "} \n"
              << '\0' << '\0';

  const std::string& configStr = constructor.str();

  ParseValidDsnString(configStr, cfg);

  CheckDsnConfig(cfg);
}

BOOST_AUTO_TEST_SUITE_END()