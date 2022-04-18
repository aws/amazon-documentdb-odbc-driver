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

#define BOOST_TEST_MODULE DocumentDBTest
#ifdef _WIN32
#include <Windows.h>
#endif

#include <sql.h>
#include <sqlext.h>

#include <boost/test/unit_test.hpp>
#include <ignite/odbc/common/platform_utils.h>
#include <ignite/odbc/log_level.h>
#include <ignite/odbc/log.h>
#include <string>

#include "odbc_test_suite.h"
#include "test_utils.h"

using namespace ignite::odbc::common;
using namespace boost::unit_test;

using boost::unit_test::precondition;
using ignite::odbc::OdbcTestSuite;
using ignite_test::GetOdbcErrorMessage;
using ignite::odbc::if_integration;
using ignite::odbc::LogLevel;
using ignite::odbc::Logger;

/**
 * Test setup fixture.
 */
struct ConnectionTestSuiteFixture : OdbcTestSuite {
  using OdbcTestSuite::OdbcTestSuite;

  /**
   * Execute the query and return an error code.
   */
  std::string ExecQueryAndReturnError() {
    SQLCHAR selectReq[] = "select count(*) from TestType";

    SQLRETURN ret = SQLExecDirect(stmt, selectReq, sizeof(selectReq));

    std::string err;

    if (!SQL_SUCCEEDED(ret))
      err = ExtractErrorCode(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    return err;
  }

  /**
   * Extract code from ODBC error message.
   *
   * @param err Error message.
   * @return Error code.
   */
  static std::string ExtractErrorCode(const std::string& err) {
    std::string code;

    size_t idx = err.find(':');

    if ((idx != std::string::npos) && (idx > 0))
      code = err.substr(0, idx);

    return code;
  }

  /**
   * Destructor.
   */
  ~ConnectionTestSuiteFixture() override = default;
};

BOOST_FIXTURE_TEST_SUITE(ConnectionTestSuite, ConnectionTestSuiteFixture)

BOOST_AUTO_TEST_CASE(TestConnectionRestoreInternalSSHTunnel,
                     *precondition(if_integration())) {
  std::string connectionString;
  CreateDsnConnectionStringForRemoteServer(connectionString);
  Connect(connectionString);
  Disconnect();
}

BOOST_AUTO_TEST_CASE(TestConnectionRestoreExternalSSHTunnel,
                     *precondition(if_integration())) {
  std::string connectionString;
  CreateDsnConnectionStringForRemoteServer(connectionString, false);
  Connect(connectionString);
  Disconnect();
}

BOOST_AUTO_TEST_CASE(TestConnectionRestoreLocalServer) {
  std::string connectionString;
  CreateDsnConnectionStringForLocalServer(connectionString);
  Connect(connectionString);
  Disconnect();
}

// TODO enable the log file unit test after logging is properly 
// supported in test suites
// https://bitquill.atlassian.net/browse/AD-712
BOOST_AUTO_TEST_CASE(TestLogFileCreatedWithConnection, *disabled()) {
  std::string connectionString;
  CreateDsnConnectionStringForLocalServer(connectionString);
  Connect(connectionString);

  LOG_DEBUG_MSG("TestLogFileCreatedWithConnection: Connected");

  std::string logPath = GetEnv("DOC_DB_LOG_PATH", "");
  std::string logLevel = GetEnv("DOC_DB_LOG_LEVEL", "");
  LogLevel::Type level = LogLevel::FromString(logLevel);

  std::shared_ptr< Logger > logger = Logger::getLoggerInstance();

  // check log level
  LogLevel::Type loggerLogLevel = logger->getLogLevel();
  BOOST_CHECK_EQUAL(static_cast< int >(level),
                    static_cast< int >(loggerLogLevel));

  // check log path
  std::string loggerLogPath = logger->getLogPath();
  BOOST_CHECK_EQUAL(logPath,
                    loggerLogPath);

  // check that the file stream is open
  bool loggerEnabled = logger->IsEnabled();
  BOOST_CHECK(loggerEnabled);

  Disconnect();
}

BOOST_AUTO_TEST_CASE(TestConnectionRestoreMiscOptionsSet) {
  const std::string miscOptions =
      "APP_NAME=TestAppName;"
      "LOGIN_TIMEOUT_SEC=30;"
      "READ_PREFERENCE=primary_preferred;"
      "RETRY_READS=false;"
      "SCAN_METHOD=id_forward;"
      "SCAN_LIMIT=100;"
      "SCHEMA_NAME=test;"
      "REFRESH_SCHEMA=true;"
      "DEFAULT_FETCH_SIZE=1000;";
  std::string connectionString;
  CreateDsnConnectionStringForLocalServer(connectionString, "", "",
                                          miscOptions);

  Connect(connectionString);
  Disconnect();
}

BOOST_AUTO_TEST_CASE(TestConnectionIncompleteBasicProperties) {
  std::string connectionString =
      "DRIVER={Amazon DocumentDB};"
      "HOSTNAME=localhost;"
      "USER=user;"
      "PASSWORD=password;";

  ExpectConnectionReject(connectionString,
                         "01S00: Hostname, username, password, and database "
                         "are required to connect.");

  Disconnect();
}

BOOST_AUTO_TEST_CASE(TestConnectionIncompleteSSHTunnelProperties) {
  std::string connectionString =
      "DRIVER={Amazon DocumentDB};"
      "HOSTNAME=host.com;"
      "DATABASE=test;"
      "USER=user;"
      "PASSWORD=password;"
      "SSH_USER=sshUser;"
      "SSH_HOST=sshHost;";

  ExpectConnectionReject(
      connectionString,
      "01S00: If using an internal SSH tunnel, all of ssh_host, ssh_user, "
      "ssh_private_key_file are required to connect.");

  Disconnect();
}

BOOST_AUTO_TEST_CASE(TestConnectionInvalidUser) {
  std::string connectionString;
  CreateDsnConnectionStringForLocalServer(connectionString, "", "invaliduser");

  ExpectConnectionReject(
      connectionString,
      "08001: Failed to establish connection with the host.\n"
      "Invalid username or password or user is not authorized on database "
      "'odbc-test'. "
      "Please check your settings. Authorization failed for user 'invaliduser' "
      "on database 'admin' with mechanism");

  Disconnect();
}

BOOST_AUTO_TEST_SUITE_END()
