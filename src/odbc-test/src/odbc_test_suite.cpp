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

#ifdef _WIN32
#include <windows.h>
#endif

#include <documentdb/odbc/common/fixed_size_array.h>
#include <sql.h>
#include <sqlext.h>
#include <fstream>

#include <boost/test/unit_test.hpp>

#include <documentdb/odbc/utility.h>
#include <documentdb/odbc/dsn_config.h>
#include <documentdb/odbc/config/configuration.h>

#include "odbc_test_suite.h"
#include "test_utils.h"

using namespace documentdb_test;
using namespace boost::unit_test;
using namespace documentdb::odbc::config;

/**
 * Test setup config for test results
 */
struct OdbcConfig {
  OdbcConfig() : test_log("odbc_test_result.xml") {
    unit_test_log.set_stream(test_log);
    unit_test_log.set_format(OF_JUNIT);
  }
  ~OdbcConfig() {
    unit_test_log.set_stream(std::cout);
  }

  std::ofstream test_log;
};

BOOST_GLOBAL_FIXTURE(OdbcConfig);

namespace documentdb {
namespace odbc {
void OdbcTestSuite::Prepare() {
  // Allocate an environment handle
  SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);

  BOOST_REQUIRE(env != NULL);

  // We want ODBC 3 support
  SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION,
                reinterpret_cast< void* >(SQL_OV_ODBC3), 0);

  // Allocate a connection handle
  SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);

  BOOST_REQUIRE(dbc != NULL);
}

void OdbcTestSuite::Connect(SQLHDBC& conn, SQLHSTMT& statement,
                            const std::string& connectStr) {
  // Allocate a connection handle
  SQLAllocHandle(SQL_HANDLE_DBC, env, &conn);

  BOOST_REQUIRE(conn != NULL);

  // Connect string
  std::vector< SQLWCHAR > connectStr0(connectStr.begin(), connectStr.end());

  SQLWCHAR outstr[ODBC_BUFFER_SIZE];
  SQLSMALLINT outstrlen;

  // Connecting to ODBC server.
  SQLRETURN ret =
      SQLDriverConnect(conn, NULL, &connectStr0[0],
                       static_cast< SQLSMALLINT >(connectStr0.size()), outstr,
                       ODBC_BUFFER_SIZE, &outstrlen, SQL_DRIVER_COMPLETE);

  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_DBC, conn));
  }

  // Allocate a statement handle
  SQLAllocHandle(SQL_HANDLE_STMT, conn, &statement);

  BOOST_REQUIRE(statement != NULL);
}

void OdbcTestSuite::Connect(const std::string& connectStr) {
  Prepare();

  // Connect string
  std::vector< SQLWCHAR > connectStr0(connectStr.begin(), connectStr.end());

  SQLWCHAR outstr[ODBC_BUFFER_SIZE];
  SQLSMALLINT outstrlen;

  // Connecting to ODBC server.
  SQLRETURN ret =
      SQLDriverConnect(dbc, NULL, &connectStr0[0],
                       static_cast< SQLSMALLINT >(connectStr0.size()), outstr,
                       ODBC_BUFFER_SIZE, &outstrlen, SQL_DRIVER_COMPLETE);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_DBC, dbc));

  // Allocate a statement handle
  SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

  BOOST_REQUIRE(stmt != NULL);
}

void OdbcTestSuite::Connect(const std::string& dsn, const std::string& username,
             const std::string& password) {
  Prepare();

  // Connect string
  std::vector< SQLWCHAR > wDsn(dsn.begin(), dsn.end());
  std::vector< SQLWCHAR > wUsername(username.begin(), username.end());
  std::vector< SQLWCHAR > wPassword(password.begin(), password.end());

  // Connecting to ODBC server.
  SQLRETURN ret = SQLConnect(
      dbc, wDsn.data(), static_cast< SQLSMALLINT >(wDsn.size()),
      wUsername.data(), static_cast< SQLSMALLINT >(wUsername.size()),
      wPassword.data(), static_cast< SQLSMALLINT >(wPassword.size()));
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_DBC, dbc));

  // Allocate a statement handle
  SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

  BOOST_REQUIRE(stmt != NULL);
}

void OdbcTestSuite::ParseConnectionString(const std::string& connectionString,
                                          Configuration& config) {
  ConnectionStringParser parser(config);
  parser.ParseConnectionString(connectionString, nullptr);
}

void OdbcTestSuite::WriteDsnConfiguration(const Configuration& config) {
  DocumentDbError error;
  if (!odbc::WriteDsnConfiguration(config, error)) {
    std::stringstream msg;
    msg << "Call to WriteDsnConfiguration failed: " << error.GetText()
        << ", code: " << error.GetCode();
    BOOST_FAIL(msg.str());
  }
}

void OdbcTestSuite::WriteDsnConfiguration(const std::string& dsn,
                                          const std::string& connectionString,
                                          std::string& username,
                                          std::string& password) {
  Configuration config;
  ParseConnectionString(connectionString, config);
  username = config.GetUser();
  password = config.GetPassword();

  // Update the DSN and clear the username and password from the DSN.
  config.SetDsn(dsn);
  config.SetUser("");
  config.SetPassword("");

  WriteDsnConfiguration(config);
}

void OdbcTestSuite::DeleteDsnConfiguration(const std::string& dsn) {
  DocumentDbError error;
  if (!odbc::DeleteDsnConfiguration(dsn, error)) {
    std::stringstream msg;
    msg << "Call to DeleteDsnConfiguration failed: " << error.GetText()
        << ", code: " << error.GetCode();
    BOOST_FAIL(msg.str());
  }
}


std::string OdbcTestSuite::ExpectConnectionReject(
    const std::string& connectStr, const std::string& expectedState,
    const std::string& expectedError) {
  Prepare();

  // Connect string
  std::vector< SQLWCHAR > connectStr0(connectStr.begin(), connectStr.end());

  SQLWCHAR outstr[ODBC_BUFFER_SIZE];
  SQLSMALLINT outstrlen;

  // Connecting to ODBC server.
  SQLRETURN ret =
      SQLDriverConnect(dbc, NULL, &connectStr0[0],
                       static_cast< SQLSMALLINT >(connectStr0.size()), outstr,
                       ODBC_BUFFER_SIZE, &outstrlen, SQL_DRIVER_COMPLETE);

  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
  OdbcClientError error = GetOdbcError(SQL_HANDLE_DBC, dbc);
  BOOST_CHECK_EQUAL(error.sqlstate, expectedState);
  size_t prefixLen = std::string("[unixODBC]").size();
  BOOST_REQUIRE(error.message.substr(0, expectedError.size()) == expectedError
                || error.message.substr(prefixLen, expectedError.size())
                       == expectedError);

  return GetOdbcErrorState(SQL_HANDLE_DBC, dbc);
}

std::string OdbcTestSuite::ExpectConnectionReject(
    const std::string& dsn, const std::string& username,
    const std::string& password, const std::string& expectedState,
    const std::string& expectedError) {
  Prepare();

  std::vector< SQLWCHAR > wDsn(dsn.begin(), dsn.end());
  std::vector< SQLWCHAR > wUsername(username.begin(), username.end());
  std::vector< SQLWCHAR > wPassword(password.begin(), password.end());

  SQLWCHAR outstr[ODBC_BUFFER_SIZE];
  SQLSMALLINT outstrlen;

  // Connecting to ODBC server.
  SQLRETURN ret = SQLConnect(
      dbc, wDsn.data(), static_cast< SQLSMALLINT >(wDsn.size()),
      wUsername.data(), static_cast< SQLSMALLINT >(wUsername.size()),
      wPassword.data(), static_cast< SQLSMALLINT >(wPassword.size()));

  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);

  OdbcClientError error = GetOdbcError(SQL_HANDLE_DBC, dbc);
  BOOST_CHECK_EQUAL(error.sqlstate, expectedState);
  size_t prefixLen = std::string("[unixODBC]").size();
  BOOST_REQUIRE(error.message.substr(0, expectedError.size())
          == expectedError
      || error.message.substr(prefixLen, expectedError.size()) == expectedError);

  return GetOdbcErrorState(SQL_HANDLE_DBC, dbc);
}

void OdbcTestSuite::Disconnect() {
  if (stmt) {
    // Releasing statement handle.
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    stmt = nullptr;
  }

  if (dbc) {
    // Disconneting from the server.
    SQLDisconnect(dbc);

    // Releasing allocated handles.
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    dbc = nullptr;
  }
}

void OdbcTestSuite::CleanUp() {
  Disconnect();

  if (env) {
    // Releasing allocated handles.
    SQLFreeHandle(SQL_HANDLE_ENV, env);
    env = NULL;
  }
}

OdbcTestSuite::OdbcTestSuite() : env(NULL), dbc(NULL), stmt(NULL) {
  // No-op.
}

OdbcTestSuite::~OdbcTestSuite() {
  CleanUp();
}

int64_t OdbcTestSuite::GetTestI64Field(int idx) {
  return static_cast< int16_t >(idx * 64);
}

void OdbcTestSuite::CheckTestI64Value(int idx, int64_t value) {
  BOOST_TEST_INFO("Test index: " << idx);
  BOOST_CHECK_EQUAL(value, GetTestI64Field(idx));
}

int32_t OdbcTestSuite::GetTestI32Field(int idx) {
  return static_cast< int32_t >(idx * 32);
}

void OdbcTestSuite::CheckTestI32Value(int idx, int32_t value) {
  BOOST_TEST_INFO("Test index: " << idx);
  BOOST_CHECK_EQUAL(value, GetTestI32Field(idx));
}

std::string OdbcTestSuite::GetIdString(int idx) {
  std::stringstream builder;

  // Note: prefix used in queries_test_006.json input file.
  builder << "62196dcc4d9189219147513" << std::hex << idx;

  return builder.str();
}

void OdbcTestSuite::CheckTestIdValue(int idx, const std::string& value) {
  BOOST_TEST_INFO("Test index: " << idx);
  BOOST_CHECK_EQUAL(value, GetIdString(idx));
}

std::string OdbcTestSuite::GetTestDec128String(int64_t idx) {
  std::stringstream builder;

  builder << idx;

  return builder.str();
}

void OdbcTestSuite::CheckTestDec128Value(int idx, const std::string& value) {
  BOOST_TEST_INFO("Test index: " << idx);
  BOOST_CHECK_EQUAL(value, GetTestDec128String(idx));
}

std::string OdbcTestSuite::GetTestString(int64_t idx) {
  std::stringstream builder;

  builder << "String#" << idx;

  return builder.str();
}

void OdbcTestSuite::CheckTestStringValue(int idx, const std::string& value) {
  BOOST_TEST_INFO("Test index: " << idx);
  BOOST_CHECK_EQUAL(value, GetTestString(idx));
}

float OdbcTestSuite::GetTestFloatField(int64_t idx) {
  return static_cast< float >(idx) * 0.5f;
}

void OdbcTestSuite::CheckTestFloatValue(int idx, float value) {
  BOOST_TEST_INFO("Test index: " << idx);
  BOOST_CHECK_EQUAL(value, GetTestFloatField(idx));
}

double OdbcTestSuite::GetTestDoubleField(int64_t idx) {
  return static_cast< double >(idx) * 0.25f;
}

void OdbcTestSuite::CheckTestDoubleValue(int idx, double value) {
  BOOST_TEST_INFO("Test index: " << idx);
  BOOST_CHECK_EQUAL(value, GetTestDoubleField(idx));
}

bool OdbcTestSuite::GetTestBoolField(int64_t idx) {
  return ((idx % 2) == 0);
}

void OdbcTestSuite::CheckTestBoolValue(int idx, bool value) {
  BOOST_TEST_INFO("Test index: " << idx);
  BOOST_CHECK_EQUAL(value, GetTestBoolField(idx));
}

void OdbcTestSuite::GetTestDateField(int64_t idx, SQL_DATE_STRUCT& val) {
  val.year = static_cast< SQLSMALLINT >(2017 + idx / 365);
  val.month = static_cast< SQLUSMALLINT >(((idx / 28) % 12) + 1);
  val.day = static_cast< SQLUSMALLINT >((idx % 28) + 1);
}

void OdbcTestSuite::CheckTestDateValue(int idx, const SQL_DATE_STRUCT& val) {
  BOOST_TEST_CONTEXT("Test index: " << idx) {
    SQL_DATE_STRUCT expected;
    GetTestDateField(idx, expected);

    BOOST_CHECK_EQUAL(val.year, expected.year);
    BOOST_CHECK_EQUAL(val.month, expected.month);
    BOOST_CHECK_EQUAL(val.day, expected.day);
  }
}

void OdbcTestSuite::GetTestTimeField(int64_t idx, SQL_TIME_STRUCT& val) {
  val.hour = (idx / 3600) % 24;
  val.minute = (idx / 60) % 60;
  val.second = idx % 60;
}

void OdbcTestSuite::CheckTestTimeValue(int idx, const SQL_TIME_STRUCT& val) {
  BOOST_TEST_CONTEXT("Test index: " << idx) {
    SQL_TIME_STRUCT expected;
    GetTestTimeField(idx, expected);

    BOOST_CHECK_EQUAL(val.hour, expected.hour);
    BOOST_CHECK_EQUAL(val.minute, expected.minute);
    BOOST_CHECK_EQUAL(val.second, expected.second);
  }
}

void OdbcTestSuite::GetTestTimestampField(int64_t idx,
                                          SQL_TIMESTAMP_STRUCT& val) {
  SQL_DATE_STRUCT date;
  GetTestDateField(idx, date);

  SQL_TIME_STRUCT time;
  GetTestTimeField(idx, time);

  val.year = date.year;
  val.month = date.month;
  val.day = date.day;
  val.hour = time.hour;
  val.minute = time.minute;
  val.second = time.second;
  val.fraction = static_cast< uint64_t >(std::abs(idx * 914873)) % 1000000000;
}

void OdbcTestSuite::CheckTestTimestampValue(int idx,
                                            const SQL_TIMESTAMP_STRUCT& val) {
  BOOST_TEST_CONTEXT("Test index: " << idx) {
    SQL_TIMESTAMP_STRUCT expected;
    GetTestTimestampField(idx, expected);

    BOOST_CHECK_EQUAL(val.year, expected.year);
    BOOST_CHECK_EQUAL(val.month, expected.month);
    BOOST_CHECK_EQUAL(val.day, expected.day);
    BOOST_CHECK_EQUAL(val.hour, expected.hour);
    BOOST_CHECK_EQUAL(val.minute, expected.minute);
    BOOST_CHECK_EQUAL(val.second, expected.second);
    BOOST_CHECK_EQUAL(val.fraction, expected.fraction);
  }
}

void OdbcTestSuite::GetTestI8ArrayField(int64_t idx, int8_t* val,
                                        size_t valLen) {
  for (size_t j = 0; j < valLen; ++j)
    val[j] = static_cast< int8_t >(j);
}

void OdbcTestSuite::CheckTestI8ArrayValue(int idx, const int8_t* val,
                                          size_t valLen) {
  BOOST_TEST_CONTEXT("Test index: " << idx) {
    std::vector< int8_t > expected;
    expected.resize(valLen);
    GetTestI8ArrayField(idx, expected.data(), expected.size());

    for (size_t j = 0; j < valLen; ++j) {
      BOOST_TEST_INFO("Byte index: " << j);
      BOOST_CHECK_EQUAL(val[j], expected[j]);
    }
  }
}

void OdbcTestSuite::CheckSQLDiagnosticError(
    int16_t handleType, SQLHANDLE handle,
    const std::string& expectedSqlStateStr) {
  SQLWCHAR state[ODBC_BUFFER_SIZE];
  SQLINTEGER nativeError = 0;
  SQLWCHAR message[ODBC_BUFFER_SIZE];
  SQLSMALLINT messageLen = 0;

  SQLRETURN ret = SQLGetDiagRec(handleType, handle, 1, state, &nativeError,
                                message, ODBC_BUFFER_SIZE, &messageLen);

  const std::string sqlState = utility::SqlWcharToString(state);
  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);
  BOOST_REQUIRE_EQUAL(sqlState, expectedSqlStateStr);
  BOOST_REQUIRE(messageLen > 0);
}

void OdbcTestSuite::CheckSQLStatementDiagnosticError(
    const std::string& expectSqlState) {
  CheckSQLDiagnosticError(SQL_HANDLE_STMT, stmt, expectSqlState);
}

void OdbcTestSuite::CheckSQLConnectionDiagnosticError(
    const std::string& expectSqlState) {
  CheckSQLDiagnosticError(SQL_HANDLE_DBC, dbc, expectSqlState);
}

std::vector< SQLWCHAR > OdbcTestSuite::MakeSqlBuffer(const std::string& value) {
  return utility::ToWCHARVector(value);
}

SQLRETURN OdbcTestSuite::ExecQuery(const std::string& qry) {
  std::vector< SQLWCHAR > sql = MakeSqlBuffer(qry);

  return SQLExecDirect(stmt, sql.data(), static_cast< SQLINTEGER >(sql.size()));
}

SQLRETURN OdbcTestSuite::PrepareQuery(const std::string& qry) {
  std::vector< SQLWCHAR > sql = MakeSqlBuffer(qry);

  return SQLPrepare(stmt, sql.data(), static_cast< SQLINTEGER >(sql.size()));
}

void OdbcTestSuite::InsertTestStrings(int recordsNum, bool merge) {
  std::vector< SQLWCHAR > insertReq =
      MakeSqlBuffer("INSERT INTO TestType(_key, strField) VALUES(?, ?)");
  std::vector< SQLWCHAR > mergeReq =
      MakeSqlBuffer("MERGE INTO TestType(_key, strField) VALUES(?, ?)");

  SQLRETURN ret =
      SQLPrepare(stmt, merge ? mergeReq.data() : insertReq.data(), SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  int64_t key = 0;
  char strField[1024] = {0};
  SQLLEN strFieldLen = 0;

  // Binding parameters.
  ret = SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_BIGINT, 0,
                         0, &key, 0, 0);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_WVARCHAR,
                         sizeof(strField), sizeof(strField), &strField,
                         sizeof(strField), &strFieldLen);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  // Inserting values.
  for (SQLSMALLINT i = 0; i < recordsNum; ++i) {
    key = static_cast< int64_t >(i) + 1;
    std::string val = GetTestString(i);

    CopyStringToBuffer(strField, val, sizeof(strField));
    strFieldLen = SQL_NTS;

    ret = SQLExecute(stmt);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    SQLLEN affected = 0;
    ret = SQLRowCount(stmt, &affected);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    BOOST_CHECK_EQUAL(affected, 1);

    ret = SQLMoreResults(stmt);

    if (ret != SQL_NO_DATA)
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  // Resetting parameters.
  ret = SQLFreeStmt(stmt, SQL_RESET_PARAMS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
}

void OdbcTestSuite::CreateDsnConnectionStringForRemoteServer(
    std::string& connectionString, bool sshTunnel, const std::string& username,
    const std::string& miscOptions, const std::string databasename) const {
  std::string user = common::GetEnv("DOC_DB_USER_NAME", "documentdb");
  std::string password = common::GetEnv("DOC_DB_PASSWORD", "");
  std::string host =
      sshTunnel ? common::GetEnv("DOC_DB_HOST", "") : "localhost";
  std::string sshUserAtHost = common::GetEnv("DOC_DB_USER", "");
  std::string sshPrivKeyFile = common::GetEnv("DOC_DB_PRIV_KEY_FILE", "");
  std::string port = sshTunnel ? common::GetEnv("DOC_DB_REMOTE_PORT", "27017")
                               : common::GetEnv("DOC_DB_LOCAL_PORT", "27019");

  if (!username.empty()) {
    user = username;
  }
  std::string database = "test";
  if (!databasename.empty()) {
    database = databasename;
  }

  std::string sshUser;
  std::string sshTunnelHost;
  size_t indexOfAt = sshUserAtHost.find_first_of('@');
  if (indexOfAt != std::string::npos
      && sshUserAtHost.size() > (indexOfAt + 1)) {
    sshUser = sshUserAtHost.substr(0, indexOfAt);
    sshTunnelHost = sshUserAtHost.substr(indexOfAt + 1);
  }

  connectionString =
            "DRIVER={Amazon DocumentDB};"
            "DSN=" + Configuration::DefaultValue::dsn + ";"
            "HOSTNAME=" + host + ":" + port + ";"
            "DATABASE=" + database + ";"
            "USER=" + user + ";"
            "PASSWORD=" + password + ";"
            "TLS=true;"
            "TLS_ALLOW_INVALID_HOSTNAMES=true;";

  if (sshTunnel && !sshUserAtHost.empty() && !sshPrivKeyFile.empty()
      && !sshUser.empty() && !sshTunnelHost.empty()) {
    connectionString.append("SSH_USER=" + sshUser + ';');
    connectionString.append("SSH_HOST=" + sshTunnelHost + ';');
    connectionString.append("SSH_PRIVATE_KEY_FILE=" + sshPrivKeyFile + ';');
    connectionString.append("SSH_STRICT_HOST_KEY_CHECKING=false;");
  }

  if (!miscOptions.empty())
    connectionString.append(miscOptions);
}

void OdbcTestSuite::CreateDsnConnectionStringForLocalServer(
    std::string& connectionString, const std::string& databaseName,
    const std::string& userName, const std::string& miscOptions,
    const std::string& portNum) const {
  std::string user = userName.size() > 0
                         ? userName
                         : common::GetEnv("DOC_DB_USER_NAME", "documentdb");
  std::string password = common::GetEnv("DOC_DB_PASSWORD", "");
  std::string host = common::GetEnv("LOCAL_DATABASE_HOST", "localhost");
  std::string port = portNum;
  std::string database = databaseName.size() > 0 ? databaseName : "odbc-test";
  std::string logPath = common::GetEnv("DOC_DB_LOG_PATH", "");
  std::string logLevel = common::GetEnv("DOC_DB_LOG_LEVEL", "Error");

  connectionString =
    "DRIVER={Amazon DocumentDB};"
    "DSN=" + Configuration::DefaultValue::dsn + ";"
    "HOSTNAME=" + host + ":" + port + ";"
    "DATABASE=" + database + ";"
    "USER=" + user + ";"
    "PASSWORD=" + password + ";"
    "TLS=false;"
    "LOG_PATH=" + logPath + ";"
    "LOG_LEVEL=" + logLevel + ";";

  if (miscOptions.size() > 0) {
    connectionString.append(miscOptions);
  }
}
}  // namespace odbc
}  // namespace documentdb
