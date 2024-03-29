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

#ifndef ODBC_TEST_ODBC_TEST_SUITE
#define ODBC_TEST_ODBC_TEST_SUITE

#ifdef _WIN32
#include <windows.h>
#endif

#include <sql.h>
#include <sqlext.h>
#include <string>
#include <boost/test/unit_test.hpp>

#include <documentdb/odbc/common/platform_utils.h>
#include <documentdb/odbc/config/configuration.h>
#include <documentdb/odbc/config/connection_string_parser.h>

using namespace documentdb::odbc::config;

#ifndef BOOST_TEST_CONTEXT
#define BOOST_TEST_CONTEXT(...)
#endif

#ifndef BOOST_TEST_INFO
#define BOOST_TEST_INFO(...)
#endif

using boost::unit_test::test_unit_id;
using documentdb::odbc::common::GetEnv;

namespace documentdb {
namespace odbc {
/**
 * Test setup fixture.
 */
struct OdbcTestSuite {
  /**
   * Prepare environment.
   */
  void Prepare();

  /**
   * Establish connection to node using provided handles.
   *
   * @param conn Connection.
   * @param statement Statement to allocate.
   * @param connectStr Connection string.
   */
  void Connect(SQLHDBC& conn, SQLHSTMT& statement,
               const std::string& connectStr);

  /**
   * Establish connection to node using default handles.
   *
   * @param connectStr Connection string.
   */
  void Connect(const std::string& connectStr);

  /**
   * Establish connection to node using default handles.
   *
   * @param dsn The DSN of the connection
   * @param username user name
   * @param password password
   */
  void Connect(const std::string& dsn, const std::string& username,
               const std::string& password);

  /**
   * Parse the connection string into configuration.
   *
   * @param connectionString the connection string to parse.
   * @param config the configuration to update.
   */
  void ParseConnectionString(const std::string& connectionString,
                             Configuration& config);

  /**
   * Writes the DSN configuration
   *
   * @param dsn The DSN to write
   * @param config The configuration to write
   */
  void WriteDsnConfiguration(const Configuration& config);

  /**
   * Writes the DSN configuration for a local connection.
   * Ensures the username and password are not set in the DSN.
   * Instead sets the username and password parameters to be used
   * with SQLConnect.
   *
   * @param dsn the DSN to write configuration for.
   * @param username the updated username for the connection.
   * @param password the updated password for the connection.
   */
  void WriteDsnConfiguration(const std::string& dsn,
                             const std::string& connectionString,
                             std::string& username, std::string& password);

  /**
   * Removes the DSN configuration
   *
   * @param dsn The DSN to write
   */
  void DeleteDsnConfiguration(const std::string& dsn);

  /**
   * Expect connection to be rejected by the node.
   *
   * @param connectStr Connection string.
   * @return SQL State.
   */
  std::string ExpectConnectionReject(const std::string& connectStr,
                                     const std::string& expectedState,
                                     const std::string& expectedError);

  /**
   * Expect connection to be rejected by the node.
   *
   * @param dsn the DSN for the connection
   * @param username the username for the connection.
   * @param password the password for the connection.
   * @return SQL State.
   */
  std::string ExpectConnectionReject(const std::string& dsn,
                                     const std::string& username,
                                     const std::string& password,
                                     const std::string& expectedState,
                                     const std::string& expectedError);

  /**
   * Disconnect.
   */
  void Disconnect();

  /**
   * Clean up.
   */
  void CleanUp();

  /**
   * Constructor.
   */
  OdbcTestSuite();

  /**
   * Destructor.
   */
  virtual ~OdbcTestSuite();

  /**
   * Insert requested number of TestType values with all defaults except
   * for the strFields, which are generated using GetTestString().
   *
   * @param recordsNum Number of records to insert.
   * @param merge Set to true to use merge instead.
   */
  void InsertTestStrings(int recordsNum, bool merge = false);

  /**
   * Insert requested number of TestType values in a batch.
   *
   * @param from Index to start from.
   * @param to Index to stop.
   * @param expectedToAffect Expected number of affected records.
   * @param merge Set to true to use merge instead of insert.
   * @return Records inserted.
   */
  int InsertTestBatch(int from, int to, int expectedToAffect,
                      bool merge = false);

  /**
   * Insert requested number of TestType values in a batch,
   * select them and check all the values.
   *
   * @param recordsNum Number of records.
   */
  void InsertBatchSelect(int recordsNum);

  /**
   * Insert values in two batches, select them and check all the values.
   * @param recordsNum Number of records.
   * @param splitAt Point where two batches are separated.
   */
  void InsertNonFullBatchSelect(int recordsNum, int splitAt);

  /**
   * Get test i64Field.
   *
   * @param idx Index.
   * @return Corresponding i64Field value.
   */
  static int64_t GetTestI64Field(int idx);

  /**
   * Check i64Field test value.
   * @param idx Index.
   * @param value Value to test.
   */
  static void CheckTestI64Value(int idx, int64_t value);

  /**
   * Get test i32Field.
   *
   * @param idx Index.
   * @return Corresponding i32Field value.
   */
  static int32_t GetTestI32Field(int idx);

  /**
   * Check i32Field test value.
   * @param idx Index.
   * @param value Value to test.
   */
  static void CheckTestI32Value(int idx, int32_t value);

  /**
   * Get test _id string.
   *
   * @param idx Index.
   * @return Corresponding test string.
   */
  static std::string GetIdString(int idx);

  /**
   * Check _id test value.
   * @param idx Index.
   * @param value Value to test.
   */
  static void CheckTestIdValue(int idx, const std::string& value);

  /**
   * Get test Decimal 128 as string.
   *
   * @param idx Index.
   * @return Corresponding test string.
   */
  static std::string GetTestDec128String(int64_t idx);

  /**
   * Check fieldDecimal128 test value.
   * @param idx Index.
   * @param value Value to test.
   */
  static void CheckTestDec128Value(int idx, const std::string& value);

  /**
   * Get test string.
   *
   * @param idx Index.
   * @return Corresponding test string.
   */
  static std::string GetTestString(int64_t idx);

  /**
   * Check strField test value.
   * @param idx Index.
   * @param value Value to test.
   */
  static void CheckTestStringValue(int idx, const std::string& value);

  /**
   * Get test floatField.
   *
   * @param idx Index.
   * @return Corresponding floatField value.
   */
  static float GetTestFloatField(int64_t idx);

  /**
   * Check floatField test value.
   * @param idx Index.
   * @param value Value to test.
   */
  static void CheckTestFloatValue(int idx, float value);

  /**
   * Get test doubleField.
   *
   * @param idx Index.
   * @return Corresponding doubleField value.
   */
  static double GetTestDoubleField(int64_t idx);

  /**
   * Check doubleField test value.
   * @param idx Index.
   * @param value Value to test.
   */
  static void CheckTestDoubleValue(int idx, double value);

  /**
   * Get test boolField.
   *
   * @param idx Index.
   * @return Corresponding boolField value.
   */
  static bool GetTestBoolField(int64_t idx);

  /**
   * Check boolField test value.
   * @param idx Index.
   * @param value Value to test.
   */
  static void CheckTestBoolValue(int idx, bool value);

  /**
   * Get test dateField.
   *
   * @param idx Index.
   * @param val Output value.
   */
  static void GetTestDateField(int64_t idx, SQL_DATE_STRUCT& val);

  /**
   * Check dateField test value.
   *
   * @param idx Index.
   * @param val Value to test.
   */
  static void CheckTestDateValue(int idx, const SQL_DATE_STRUCT& val);

  /**
   * Get test timeField.
   *
   * @param idx Index.
   * @param val Output value.
   */
  static void GetTestTimeField(int64_t idx, SQL_TIME_STRUCT& val);

  /**
   * Check timeField test value.
   *
   * @param idx Index.
   * @param val Value to test.
   */
  static void CheckTestTimeValue(int idx, const SQL_TIME_STRUCT& val);

  /**
   * Get test timestampField.
   *
   * @param idx Index.
   * @param val Output value.
   */
  static void GetTestTimestampField(int64_t idx, SQL_TIMESTAMP_STRUCT& val);

  /**
   * Check timestampField test value.
   *
   * @param idx Index.
   * @param val Value to test.
   */
  static void CheckTestTimestampValue(int idx, const SQL_TIMESTAMP_STRUCT& val);

  /**
   * Get test i8ArrayField.
   *
   * @param idx Index.
   * @param val Output value.
   * @param valLen Value length.
   */
  static void GetTestI8ArrayField(int64_t idx, int8_t* val, size_t valLen);

  /**
   * Check i8ArrayField test value.
   *
   * @param idx Index.
   * @param val Value to test.
   * @param valLen Value length.
   */
  static void CheckTestI8ArrayValue(int idx, const int8_t* val, size_t valLen);

  /**
   * Check that SQL error has expected SQL state.
   *
   * @param handleType Handle type.
   * @param handle Handle.
   * @param expectSqlState Expected state.
   */
  void CheckSQLDiagnosticError(int16_t handleType, SQLHANDLE handle,
                               const std::string& expectSqlState);

  /**
   * Check that statement SQL error has expected SQL state.
   *
   * @param expectSqlState Expected state.
   */
  void CheckSQLStatementDiagnosticError(const std::string& expectSqlState);

  /**
   * Check that connection SQL error has expected SQL state.
   *
   * @param expectSqlState Expected state.
   */
  void CheckSQLConnectionDiagnosticError(const std::string& expectSqlState);

  /**
   * Convert string to vector of SQLWCHARs.
   *
   * @param value Query.
   * @return Corresponding vector.
   */
  static std::vector< SQLWCHAR > MakeSqlBuffer(const std::string& value);

  /**
   * Performs SQL query.
   *
   * @param qry Query.
   * @return Result.
   */
  SQLRETURN ExecQuery(const std::string& qry);

  /**
   * Prepares SQL query.
   *
   * @param qry Query.
   * @return Result.
   */
  SQLRETURN PrepareQuery(const std::string& qry);

  /**
   * Creates the standard DSN connection string.
   */
  void CreateDsnConnectionStringForRemoteServer(
      std::string& connectionString, bool sshTunnel = true,
      const std::string& username = std::string(),
      const std::string& miscOptions = std::string(),
      const std::string databasename = std::string()) const;

  /**
   * Creates the standard DSN connection string for use with local instance.
   */
  void CreateDsnConnectionStringForLocalServer(
      std::string& connectionString, const std::string& databaseName = "",
      const std::string& userName = "", const std::string& miscOptions = "",
      const std::string& portNum = "27017") const;

  /** ODBC Environment. */
  SQLHENV env;

  /** ODBC Connect. */
  SQLHDBC dbc;

  /** ODBC Statement. */
  SQLHSTMT stmt;
};

struct if_integration {
  const std::string DOC_DB_ODBC_INTEGRATION_TEST =
      "DOC_DB_ODBC_INTEGRATION_TEST";
  const std::string DOCUMENTDB_ODBC_TEST_INTEGRATION_OFF = "0";
  const std::string DOCUMENTDB_ODBC_TEST_INTEGRATION_FALSE = "false";

  boost::test_tools::assertion_result operator()(test_unit_id) const {
    std::string runIntegrationTests = GetEnv(
        DOC_DB_ODBC_INTEGRATION_TEST, DOCUMENTDB_ODBC_TEST_INTEGRATION_OFF);
    return runIntegrationTests != DOCUMENTDB_ODBC_TEST_INTEGRATION_OFF
           && runIntegrationTests != DOCUMENTDB_ODBC_TEST_INTEGRATION_FALSE;
  }
};
}  // namespace odbc
}  // namespace documentdb

#endif  // ODBC_TEST_ODBC_TEST_SUITE
