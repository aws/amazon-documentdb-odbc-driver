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

#include <sql.h>
#include <sqltypes.h>
#include "documentdb/odbc/utility.h"
#include <sqlext.h>

#include <boost/test/unit_test.hpp>
#include <cstdio>
#include <string>
#include <vector>

#include "documentdb/odbc/impl/binary/binary_utils.h"
#include "odbc_test_suite.h"
#include "test_type.h"
#include "test_utils.h"

using namespace documentdb;
using namespace documentdb::odbc::common;
using namespace documentdb_test;

using namespace boost::unit_test;

using documentdb::odbc::impl::binary::BinaryUtils;

/**
 * Test setup fixture.
 */
struct ApiRobustnessTestSuiteFixture : public odbc::OdbcTestSuite {
  /**
   * Constructor.
   */
  ApiRobustnessTestSuiteFixture() {
    // No-op
  }

  /**
   * Check that SQLFetchScroll does not crash with unsupported orientation.
   *
   * @param orientation Fetch orientation.
   */
  void CheckFetchScrollUnsupportedOrientation(SQLUSMALLINT orientation) {
    connectToLocalServer("odbc-test");

    int32_t intField = -1;

    // Binding column.
    SQLRETURN ret = SQLBindCol(stmt, 1, SQL_C_SLONG, &intField, 0, 0);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    std::vector< SQLWCHAR > request =
        MakeSqlBuffer("SELECT fieldInt FROM \"api_robustness_test_001\"");

    ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    ret = SQLFetchScroll(stmt, SQL_FETCH_NEXT, 0);
    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    BOOST_CHECK_EQUAL(intField, 2147483647);

    ret = SQLFetchScroll(stmt, orientation, 0);

    // Operation is not supported. However, there should be no crash.
    BOOST_CHECK(ret == SQL_ERROR);

    CheckSQLStatementDiagnosticError("HYC00");
  }

  void connectToLocalServer(const std::string& databaseName) {
    std::string dsnConnectionString;
    CreateDsnConnectionStringForLocalServer(dsnConnectionString, databaseName);

    Connect(dsnConnectionString);
  }

  /**
   * Destructor.
   */
  virtual ~ApiRobustnessTestSuiteFixture() {
    // No-op.
  }
};

SQLSMALLINT unsupportedC[] = {SQL_C_INTERVAL_YEAR,
                              SQL_C_INTERVAL_MONTH,
                              SQL_C_INTERVAL_DAY,
                              SQL_C_INTERVAL_HOUR,
                              SQL_C_INTERVAL_MINUTE,
                              SQL_C_INTERVAL_SECOND,
                              SQL_C_INTERVAL_YEAR_TO_MONTH,
                              SQL_C_INTERVAL_DAY_TO_HOUR,
                              SQL_C_INTERVAL_DAY_TO_MINUTE,
                              SQL_C_INTERVAL_DAY_TO_SECOND,
                              SQL_C_INTERVAL_HOUR_TO_MINUTE,
                              SQL_C_INTERVAL_HOUR_TO_SECOND,
                              SQL_C_INTERVAL_MINUTE_TO_SECOND};

SQLSMALLINT unsupportedSql[] = {SQL_INTERVAL_MONTH,
                                SQL_INTERVAL_YEAR,
                                SQL_INTERVAL_YEAR_TO_MONTH,
                                SQL_INTERVAL_DAY,
                                SQL_INTERVAL_HOUR,
                                SQL_INTERVAL_MINUTE,
                                SQL_INTERVAL_SECOND,
                                SQL_INTERVAL_DAY_TO_HOUR,
                                SQL_INTERVAL_DAY_TO_MINUTE,
                                SQL_INTERVAL_DAY_TO_SECOND,
                                SQL_INTERVAL_HOUR_TO_MINUTE,
                                SQL_INTERVAL_HOUR_TO_SECOND,
                                SQL_INTERVAL_MINUTE_TO_SECOND};

BOOST_FIXTURE_TEST_SUITE(ApiRobustnessTestSuite, ApiRobustnessTestSuiteFixture)

BOOST_AUTO_TEST_CASE(TestSQLSetStmtAttrRowArraySize) {
  // check that statement array size cannot be set to values other than 1

  std::string dsnConnectionString;
  CreateDsnConnectionStringForLocalServer(dsnConnectionString);

  Connect(dsnConnectionString);

  SQLINTEGER prev_row_array_size = 1;
  SQLINTEGER actual_row_array_size;
  SQLINTEGER resLen = 0;

  // check that statement array size cannot be set to values not equal to 1
  // repeat test for different values
  SQLULEN valList[] = {6, 0, 2, 3, 4, 5, 1};
  for (SQLULEN val : valList) {
    SQLRETURN ret =
        SQLSetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE,
                       reinterpret_cast< SQLPOINTER >(val), sizeof(val));

    if (val >= 1) {
      BOOST_CHECK_EQUAL(ret, SQL_SUCCESS);
    } else {
      BOOST_CHECK_EQUAL(ret, SQL_ERROR);
      CheckSQLStatementDiagnosticError("HY024");
    }

    ret = SQLGetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE, &actual_row_array_size,
                         sizeof(actual_row_array_size), &resLen);
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    BOOST_CHECK_EQUAL(actual_row_array_size, val > 0 ? val : prev_row_array_size);
    prev_row_array_size = actual_row_array_size;
  }
}

#ifndef __APPLE__
// only enable for Windows and Linux as it crashes on Mac
// with iODBC, traced by AD-820
// https://bitquill.atlassian.net/browse/AD-820
BOOST_AUTO_TEST_CASE(TestSQLDriverConnect) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  Prepare();

  std::string dsnConnectionString;
  CreateDsnConnectionStringForLocalServer(dsnConnectionString);
  std::vector< SQLWCHAR > connectStr(dsnConnectionString.begin(),
                                     dsnConnectionString.end());
  SQLSMALLINT connectStrLen = static_cast< SQLSMALLINT >(connectStr.size());

  SQLWCHAR outStr[ODBC_BUFFER_SIZE];
  SQLSMALLINT outStrLen;

  // Normal connect.
  SQLRETURN ret = SQLDriverConnect(
      dbc, NULL, &connectStr[0], connectStrLen,
      outStr, ODBC_BUFFER_SIZE, &outStrLen, SQL_DRIVER_COMPLETE);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  SQLDisconnect(dbc);

  // Null out string resulting length.
  SQLDriverConnect(dbc, NULL, &connectStr[0],
                   connectStrLen, outStr,
                   ODBC_BUFFER_SIZE, 0, SQL_DRIVER_COMPLETE);
  SQLDisconnect(dbc);

  // Null out string buffer length.
  SQLDriverConnect(dbc, NULL, &connectStr[0],
                   connectStrLen, outStr, 0,
                   &outStrLen, SQL_DRIVER_COMPLETE);
  SQLDisconnect(dbc);

  // Null out string.
  SQLDriverConnect(dbc, NULL, &connectStr[0],
                   connectStrLen, 0,
                   ODBC_BUFFER_SIZE, &outStrLen, SQL_DRIVER_COMPLETE);
  SQLDisconnect(dbc);

  // Null all.
  SQLDriverConnect(dbc, NULL, &connectStr[0],
                   connectStrLen, 0, 0, 0,
                   SQL_DRIVER_COMPLETE);
  SQLDisconnect(dbc);

  // Reduced output buffer length. Test boundary condition of output buffer
  SQLSMALLINT reducedOutStrLen = 9;
  ret = SQLDriverConnect(dbc, NULL, &connectStr[0], connectStrLen, outStr, reducedOutStrLen + 1,
                         &outStrLen, SQL_DRIVER_COMPLETE);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  BOOST_REQUIRE_EQUAL(outStrLen, reducedOutStrLen);
  std::vector< SQLWCHAR > expectedOutStr(connectStr.begin(),
                                         connectStr.begin() + reducedOutStrLen);
  expectedOutStr.push_back(0);
  std::vector< SQLWCHAR > actualOutStr(outStrLen + 1);
  for (int i = 0; i <= outStrLen; i++) {
    actualOutStr[i] = outStr[i];
  }
  BOOST_CHECK_EQUAL_COLLECTIONS(actualOutStr.begin(), actualOutStr.end(),
                                expectedOutStr.begin(), expectedOutStr.end());
  SQLDisconnect(dbc);
}
#endif

BOOST_AUTO_TEST_CASE(TestSQLConnect) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  connectToLocalServer("odbc-test");

  SQLWCHAR buffer[ODBC_BUFFER_SIZE];
  SQLSMALLINT resLen = 0;

  // Everything is ok.
  SQLRETURN ret = SQLGetInfo(dbc, SQL_DRIVER_NAME, buffer, sizeof(buffer), &resLen);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  // Resulting length is null.
  SQLGetInfo(dbc, SQL_DRIVER_NAME, buffer, sizeof(buffer), 0);

  // Buffer length is null.
  SQLGetInfo(dbc, SQL_DRIVER_NAME, buffer, 0, &resLen);

  // Buffer is null.
  SQLGetInfo(dbc, SQL_DRIVER_NAME, 0, sizeof(buffer), &resLen);

  // Unknown info.
  SQLGetInfo(dbc, -1, buffer, sizeof(buffer), &resLen);

  // All nulls.
  SQLGetInfo(dbc, SQL_DRIVER_NAME, 0, 0, 0);
}

BOOST_AUTO_TEST_CASE(TestSQLPrepare) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  connectToLocalServer("odbc-test");

  std::vector< SQLWCHAR > sql =
      MakeSqlBuffer("SELECT * FROM \"api_robustness_test_001\"");

  // Everything is ok.
  SQLRETURN ret = SQLPrepare(stmt, sql.data(), SQL_NTS);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLCloseCursor(stmt);

  // Value length is null.
  SQLPrepare(stmt, sql.data(), 0);

  SQLCloseCursor(stmt);

  // Value is null.
  SQLPrepare(stmt, 0, SQL_NTS);

  SQLCloseCursor(stmt);

  // All nulls.
  SQLPrepare(stmt, 0, 0);

  SQLCloseCursor(stmt);
}

BOOST_AUTO_TEST_CASE(TestSQLExecDirect) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  connectToLocalServer("odbc-test");

  std::vector< SQLWCHAR > sql =
      MakeSqlBuffer("SELECT * FROM \"api_robustness_test_001\"");

  // Everything is ok.
  SQLRETURN ret = SQLExecDirect(stmt, sql.data(), SQL_NTS);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLCloseCursor(stmt);

  // Value length is null.
  SQLExecDirect(stmt, sql.data(), 0);

  SQLCloseCursor(stmt);

  // Value is null.
  SQLExecDirect(stmt, 0, SQL_NTS);

  SQLCloseCursor(stmt);

  // All nulls.
  SQLExecDirect(stmt, 0, 0);

  SQLCloseCursor(stmt);
}

BOOST_AUTO_TEST_CASE(TestSQLExtendedFetch) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.
  connectToLocalServer("odbc-test");

  std::vector< SQLWCHAR > sql =
      MakeSqlBuffer("SELECT * FROM \"api_robustness_test_001\"");

  SQLRETURN ret = SQLExecDirect(stmt, sql.data(), SQL_NTS);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLULEN rowCount;
  SQLUSMALLINT rowStatus[16];

  // Everything is ok.
  ret = SQLExtendedFetch(stmt, SQL_FETCH_NEXT, 0, &rowCount, rowStatus);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Row count is null.
  SQLExtendedFetch(stmt, SQL_FETCH_NEXT, 0, 0, rowStatus);

  // Row statuses is null.
  SQLExtendedFetch(stmt, SQL_FETCH_NEXT, 0, &rowCount, 0);

  // All nulls.
  SQLExtendedFetch(stmt, SQL_FETCH_NEXT, 0, 0, 0);
}

BOOST_AUTO_TEST_CASE(TestSQLNumResultCols) {
  connectToLocalServer("odbc-test");

  std::vector< SQLWCHAR > sql =
      MakeSqlBuffer("SELECT * FROM \"api_robustness_test_001\"");

  SQLRETURN ret = SQLExecDirect(stmt, sql.data(), SQL_NTS);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLSMALLINT columnCount;

  // Everything is ok.
  ret = SQLNumResultCols(stmt, &columnCount);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  BOOST_CHECK_EQUAL(13, columnCount);

  // Test with column count is null.
  ret = SQLNumResultCols(stmt, 0);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
}

BOOST_AUTO_TEST_CASE(TestSQLForeignKeys) {
  connectToLocalServer("odbc-test");

  std::vector< SQLWCHAR > fkTableName = MakeSqlBuffer("jni_test_001_sub_doc");

  SQLRETURN ret =
      SQLForeignKeys(stmt, NULL, 0,                /* Primary catalog */
                     NULL, 0,                      /* Primary schema */
                     NULL, 0,                      /* Primary table */
                     NULL, 0,                      /* Foreign catalog */
                     NULL, 0,                      /* Foreign schema */
                     fkTableName.data(), SQL_NTS); /* Foreign table */

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
}

BOOST_AUTO_TEST_CASE(TestSQLTables) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  connectToLocalServer("odbc-test");

  std::vector< SQLWCHAR > catalogName = {0};
  std::vector< SQLWCHAR > schemaName = {0};
  std::vector< SQLWCHAR > tableName = {0};
  std::vector< SQLWCHAR > tableType = {0};

  // Everything is ok.
  SQLRETURN ret =
      SQLTables(stmt, catalogName.data(), SQL_NTS, schemaName.data(), SQL_NTS,
                tableName.data(), SQL_NTS, tableType.data(), SQL_NTS);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Sizes are nulls.
  SQLTables(dbc, catalogName.data(), 0, schemaName.data(), 0, tableName.data(),
            0, tableType.data(), 0);

  // Values are nulls.
  SQLTables(dbc, 0, SQL_NTS, 0, SQL_NTS, 0, SQL_NTS, 0, SQL_NTS);

  // All nulls.
  SQLTables(dbc, 0, 0, 0, 0, 0, 0, 0, 0);
}

BOOST_AUTO_TEST_CASE(TestSQLColumns) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  connectToLocalServer("odbc-test");

  std::vector< SQLWCHAR > catalogName = {0};
  std::vector< SQLWCHAR > schemaName = {0};
  std::vector< SQLWCHAR > tableName = {0};
  std::vector< SQLWCHAR > columnName = {0};

  // Everything is ok.
  SQLRETURN ret =
      SQLColumns(stmt, catalogName.data(), SQL_NTS, schemaName.data(), SQL_NTS,
                 tableName.data(), SQL_NTS, columnName.data(), SQL_NTS);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Sizes are nulls.
  SQLColumns(dbc, catalogName.data(), 0, schemaName.data(), 0, tableName.data(),
             0, columnName.data(), 0);

  // Values are nulls.
  SQLColumns(dbc, 0, SQL_NTS, 0, SQL_NTS, 0, SQL_NTS, 0, SQL_NTS);

  // All nulls.
  SQLColumns(dbc, 0, 0, 0, 0, 0, 0, 0, 0);
}

BOOST_AUTO_TEST_CASE(TestSQLPrimaryKeys) {
  connectToLocalServer("odbc-test");

  std::vector< SQLWCHAR > catalogName = {0};
  std::vector< SQLWCHAR > schemaName = MakeSqlBuffer("odbc-test");
  std::vector< SQLWCHAR > tableName = MakeSqlBuffer("jni_test_001");

  // Everything is ok.
  SQLRETURN ret =
      SQLPrimaryKeys(stmt, catalogName.data(), SQL_NTS, schemaName.data(),
                     SQL_NTS, tableName.data(), SQL_NTS);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  SQLPrimaryKeys(stmt, 0, SQL_NTS, schemaName.data(), SQL_NTS, tableName.data(),
                 SQL_NTS);
  SQLPrimaryKeys(stmt, catalogName.data(), 0, schemaName.data(), SQL_NTS,
                 tableName.data(), SQL_NTS);
  SQLPrimaryKeys(stmt, catalogName.data(), SQL_NTS, 0, SQL_NTS,
                 tableName.data(), SQL_NTS);
  SQLPrimaryKeys(stmt, catalogName.data(), SQL_NTS, schemaName.data(), 0,
                 tableName.data(), SQL_NTS);
  SQLPrimaryKeys(stmt, catalogName.data(), SQL_NTS, schemaName.data(), SQL_NTS,
                 0, SQL_NTS);
  SQLPrimaryKeys(stmt, catalogName.data(), SQL_NTS, schemaName.data(), SQL_NTS,
                 tableName.data(), 0);
  SQLPrimaryKeys(stmt, 0, 0, 0, 0, 0, 0);
}

BOOST_AUTO_TEST_CASE(TestSQLBindCol) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  connectToLocalServer("odbc-test");

  SQLINTEGER ind1;
  SQLLEN len1 = 0;

  // Everything is ok.
  SQLRETURN ret = SQLBindCol(stmt, 1, SQL_C_SLONG, &ind1, sizeof(ind1), &len1);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Unsupported data types
  for (size_t i = 0; i < sizeof(unsupportedC) / sizeof(unsupportedC[0]); ++i) {
    ret = SQLBindCol(stmt, 1, unsupportedC[i], &ind1, sizeof(ind1), &len1);
    BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
    CheckSQLStatementDiagnosticError("HY003");
  }

  // Size is negative.
  ret = SQLBindCol(stmt, 1, SQL_C_SLONG, &ind1, -1, &len1);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
#ifdef __APPLE__
  CheckSQLStatementDiagnosticError("S1090");
#else
  CheckSQLStatementDiagnosticError("HY090");
#endif

  // Size is null.
  SQLBindCol(stmt, 1, SQL_C_SLONG, &ind1, 0, &len1);

  // Res size is null.
  SQLBindCol(stmt, 2, SQL_C_SLONG, &ind1, sizeof(ind1), 0);

  // Value is null.
  SQLBindCol(stmt, 3, SQL_C_SLONG, 0, sizeof(ind1), &len1);

  // All nulls.
  SQLBindCol(stmt, 4, SQL_C_SLONG, 0, 0, 0);
}

BOOST_AUTO_TEST_CASE(TestSQLBindParameter) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  connectToLocalServer("odbc-test");

  SQLINTEGER ind1;
  SQLLEN len1 = 0;

  // Everything is ok.
  SQLRETURN ret =
      SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 100,
                       100, &ind1, sizeof(ind1), &len1);

  std::string expectedSqlState;
#ifdef __APPLE__
  expectedSqlState = "HYC00";
  BOOST_REQUIRE_EQUAL(ret, SQL_INVALID_HANDLE);
#else
  expectedSqlState = "HY105";
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
#endif

  // Unsupported parameter type : output
  SQLBindParameter(stmt, 2, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 100,
                   100, &ind1, sizeof(ind1), &len1);
  CheckSQLStatementDiagnosticError(expectedSqlState);

  // Unsupported parameter type : input/output
  SQLBindParameter(stmt, 2, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG, SQL_INTEGER,
                   100, 100, &ind1, sizeof(ind1), &len1);
  CheckSQLStatementDiagnosticError(expectedSqlState);

  // Unsupported data types
  for (size_t i = 0; i < sizeof(unsupportedSql) / sizeof(unsupportedSql[0]);
       ++i) {
    ret = SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG,
                           unsupportedSql[i], 100, 100, &ind1, sizeof(ind1),
                           &len1);

#ifdef __APPLE__
    expectedSqlState = "HY105";
    BOOST_REQUIRE_EQUAL(ret, SQL_INVALID_HANDLE);
#else
    expectedSqlState = "HYC00";
    BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
#endif
    CheckSQLStatementDiagnosticError(expectedSqlState);
  }

  // Size is null.
  SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 100, 100,
                   &ind1, 0, &len1);

  // Res size is null.
  SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 100, 100,
                   &ind1, sizeof(ind1), 0);

  // Value is null.
  SQLBindParameter(stmt, 4, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 100, 100,
                   0, sizeof(ind1), &len1);

  // All nulls.
  SQLBindParameter(stmt, 5, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 100, 100,
                   0, 0, 0);
}

BOOST_AUTO_TEST_CASE(TestSQLNativeSql) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  connectToLocalServer("odbc-test");

  std::vector< SQLWCHAR > sql =
      MakeSqlBuffer("SELECT * FROM \"api_robustness_test_001\"");
  SQLWCHAR buffer[ODBC_BUFFER_SIZE];
  SQLINTEGER resLen = 0;

  // Everything is ok.
  SQLRETURN ret =
      SQLNativeSql(dbc, sql.data(), SQL_NTS, buffer, ODBC_BUFFER_SIZE, &resLen);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Confirm boundary condition.
  SQLINTEGER reducedLength = 8;
  ret = SQLNativeSql(dbc, sql.data(), SQL_NTS, buffer, reducedLength + 1, &resLen);
  BOOST_CHECK_EQUAL(SQL_SUCCESS_WITH_INFO, ret);
  BOOST_CHECK_EQUAL(reducedLength, resLen);

  // Value size is null.
  ret = SQLNativeSql(dbc, sql.data(), 0, buffer, ODBC_BUFFER_SIZE, &resLen);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(0, resLen);

  // Buffer size is null.
  ret = SQLNativeSql(dbc, sql.data(), SQL_NTS, buffer, 0, &resLen);
  BOOST_CHECK_EQUAL(SQL_ERROR, ret);

  // Res size is null.
  ret = SQLNativeSql(dbc, sql.data(), SQL_NTS, buffer, ODBC_BUFFER_SIZE, nullptr);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  // Value is null.
  ret = SQLNativeSql(dbc, nullptr, SQL_NTS, buffer, ODBC_BUFFER_SIZE, &resLen);
  BOOST_CHECK_EQUAL(SQL_ERROR, ret);

  // Buffer is null.
  ret = SQLNativeSql(dbc, sql.data(), SQL_NTS, nullptr, ODBC_BUFFER_SIZE, &resLen);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(sql.size() - 1, resLen);

  // All nulls.
  ret = SQLNativeSql(dbc, nullptr, 0, nullptr, 0, nullptr);
  BOOST_CHECK_EQUAL(SQL_ERROR, ret);
}

BOOST_AUTO_TEST_CASE(TestSQLColAttribute) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  connectToLocalServer("odbc-test");

  std::vector< SQLWCHAR > sql =
      MakeSqlBuffer("SELECT * FROM \"api_robustness_test_001\"");

  SQLRETURN ret = SQLExecDirect(stmt, sql.data(), SQL_NTS);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLWCHAR buffer[ODBC_BUFFER_SIZE];
  SQLSMALLINT resLen = 0;
  SQLLEN numericAttr = 0;

  // Everything is ok. Character attribute.
  ret = SQLColAttribute(stmt, 1, SQL_COLUMN_TABLE_NAME, buffer, sizeof(buffer),
                        &resLen, &numericAttr);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Everything is ok. Numeric attribute.
  ret = SQLColAttribute(stmt, 1, SQL_DESC_COUNT, buffer, sizeof(buffer),
                        &resLen, &numericAttr);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLColAttribute(stmt, 1, SQL_COLUMN_TABLE_NAME, buffer, sizeof(buffer),
                  &resLen, 0);
  SQLColAttribute(stmt, 1, SQL_COLUMN_TABLE_NAME, buffer, sizeof(buffer), 0,
                  &numericAttr);
  SQLColAttribute(stmt, 1, SQL_COLUMN_TABLE_NAME, buffer, 0, &resLen,
                  &numericAttr);
  SQLColAttribute(stmt, 1, SQL_COLUMN_TABLE_NAME, 0, sizeof(buffer), &resLen,
                  &numericAttr);
  SQLColAttribute(stmt, 1, SQL_COLUMN_TABLE_NAME, 0, 0, 0, 0);

  SQLColAttribute(stmt, 1, SQL_DESC_COUNT, buffer, sizeof(buffer), &resLen, 0);
  SQLColAttribute(stmt, 1, SQL_DESC_COUNT, buffer, sizeof(buffer), 0,
                  &numericAttr);
  SQLColAttribute(stmt, 1, SQL_DESC_COUNT, buffer, 0, &resLen, &numericAttr);
  SQLColAttribute(stmt, 1, SQL_DESC_COUNT, 0, sizeof(buffer), &resLen,
                  &numericAttr);
  SQLColAttribute(stmt, 1, SQL_DESC_COUNT, 0, 0, 0, 0);
}

BOOST_AUTO_TEST_CASE(TestSQLDescribeCol) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  connectToLocalServer("odbc-test");

  std::vector< SQLWCHAR > sql =
      MakeSqlBuffer("SELECT * FROM \"api_robustness_test_001\"");

  SQLRETURN ret = SQLExecDirect(stmt, sql.data(), SQL_NTS);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLWCHAR columnName[ODBC_BUFFER_SIZE];
  SQLSMALLINT columnNameLen = 0;
  SQLSMALLINT dataType = 0;
  SQLULEN columnSize = 0;
  SQLSMALLINT decimalDigits = 0;
  SQLSMALLINT nullable = 0;

  // Everything is ok.
  ret = SQLDescribeCol(stmt, 1, columnName, ODBC_BUFFER_SIZE, &columnNameLen,
                       &dataType, &columnSize, &decimalDigits, &nullable);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Confirm boundary condition.
  SQLSMALLINT reducedNameLen = 4;
  ret = SQLDescribeCol(stmt, 1, columnName, reducedNameLen + 1, &columnNameLen,
                       &dataType, &columnSize, &decimalDigits, &nullable);
  BOOST_CHECK_EQUAL(ret, SQL_SUCCESS_WITH_INFO);
  BOOST_CHECK_EQUAL(columnNameLen, reducedNameLen);

  ret = SQLDescribeCol(stmt, 1, 0, ODBC_BUFFER_SIZE, &columnNameLen, &dataType,
                       &columnSize, &decimalDigits, &nullable);
  BOOST_CHECK_EQUAL(ret, SQL_SUCCESS);

  ret = SQLDescribeCol(stmt, 1, columnName, 0, &columnNameLen, &dataType,
                       &columnSize, &decimalDigits, &nullable);
  BOOST_CHECK_EQUAL(ret, SQL_SUCCESS);
  BOOST_CHECK_EQUAL(columnNameLen, 0);

  ret = SQLDescribeCol(stmt, 1, columnName, ODBC_BUFFER_SIZE, 0, &dataType,
                       &columnSize, &decimalDigits, &nullable);
  BOOST_CHECK_EQUAL(ret, SQL_SUCCESS);

  ret = SQLDescribeCol(stmt, 1, columnName, ODBC_BUFFER_SIZE, &columnNameLen, 0,
                       &columnSize, &decimalDigits, &nullable);
  BOOST_CHECK_EQUAL(ret, SQL_SUCCESS);

  ret = SQLDescribeCol(stmt, 1, columnName, ODBC_BUFFER_SIZE, &columnNameLen,
                       &dataType, 0, &decimalDigits, &nullable);
  BOOST_CHECK_EQUAL(ret, SQL_SUCCESS);

  ret = SQLDescribeCol(stmt, 1, columnName, ODBC_BUFFER_SIZE, &columnNameLen,
                       &dataType, &columnSize, 0, &nullable);
  BOOST_CHECK_EQUAL(ret, SQL_SUCCESS);

  ret = SQLDescribeCol(stmt, 1, columnName, ODBC_BUFFER_SIZE, &columnNameLen,
                       &dataType, &columnSize, &decimalDigits, 0);
  BOOST_CHECK_EQUAL(ret, SQL_SUCCESS);

  ret = SQLDescribeCol(stmt, 1, 0, 0, 0, 0, 0, 0, 0);
  BOOST_CHECK_EQUAL(ret, SQL_SUCCESS);

  ret = SQLDescribeCol(stmt, -1, 0, 0, 0, 0, 0, 0, 0);
  BOOST_CHECK_EQUAL(ret, SQL_ERROR);

  ret = SQLDescribeCol(nullptr, 1, 0, 0, 0, 0, 0, 0, 0);
  BOOST_CHECK_EQUAL(ret, SQL_INVALID_HANDLE);
}

BOOST_AUTO_TEST_CASE(TestSQLRowCount) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  connectToLocalServer("odbc-test");

  std::vector< SQLWCHAR > sql =
      MakeSqlBuffer("SELECT * FROM \"api_robustness_test_001\"");

  SQLRETURN ret = SQLExecDirect(stmt, sql.data(), SQL_NTS);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLLEN rows = 0;

  // Everything is ok.
  ret = SQLRowCount(stmt, &rows);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLRowCount(stmt, nullptr);
}

BOOST_AUTO_TEST_CASE(TestSQLForeignKeysSegFault) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  connectToLocalServer("odbc-test");

  std::vector< SQLWCHAR > catalogName = {0};
  std::vector< SQLWCHAR > schemaName = MakeSqlBuffer("cache");
  std::vector< SQLWCHAR > tableName = MakeSqlBuffer("TestType");

  // Everything is ok.
  SQLRETURN ret = SQLForeignKeys(
      stmt, catalogName.data(), SQL_NTS, schemaName.data(), SQL_NTS,
      tableName.data(), SQL_NTS, catalogName.data(), SQL_NTS, schemaName.data(),
      SQL_NTS, tableName.data(), SQL_NTS);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLCloseCursor(stmt);

  SQLForeignKeys(stmt, 0, SQL_NTS, schemaName.data(), SQL_NTS, tableName.data(),
                 SQL_NTS, catalogName.data(), SQL_NTS, schemaName.data(),
                 SQL_NTS, tableName.data(), SQL_NTS);

  SQLCloseCursor(stmt);

  SQLForeignKeys(stmt, catalogName.data(), 0, schemaName.data(),
                 sizeof(schemaName), tableName.data(), SQL_NTS,
                 catalogName.data(), SQL_NTS, schemaName.data(), SQL_NTS,
                 tableName.data(), SQL_NTS);

  SQLCloseCursor(stmt);

  SQLForeignKeys(stmt, catalogName.data(), SQL_NTS, 0, SQL_NTS,
                 tableName.data(), SQL_NTS, catalogName.data(), SQL_NTS,
                 schemaName.data(), SQL_NTS, tableName.data(), SQL_NTS);

  SQLCloseCursor(stmt);

  SQLForeignKeys(stmt, catalogName.data(), SQL_NTS, schemaName.data(), 0,
                 tableName.data(), SQL_NTS, catalogName.data(), SQL_NTS,
                 schemaName.data(), SQL_NTS, tableName.data(), SQL_NTS);

  SQLCloseCursor(stmt);

  SQLForeignKeys(stmt, catalogName.data(), SQL_NTS, schemaName.data(), SQL_NTS,
                 0, SQL_NTS, catalogName.data(), SQL_NTS, schemaName.data(),
                 SQL_NTS, tableName.data(), SQL_NTS);

  SQLCloseCursor(stmt);

  SQLForeignKeys(stmt, catalogName.data(), SQL_NTS, schemaName.data(), SQL_NTS,
                 tableName.data(), 0, catalogName.data(), SQL_NTS,
                 schemaName.data(), SQL_NTS, tableName.data(), SQL_NTS);

  SQLCloseCursor(stmt);

  SQLForeignKeys(stmt, catalogName.data(), SQL_NTS, schemaName.data(), SQL_NTS,
                 tableName.data(), SQL_NTS, 0, SQL_NTS, schemaName.data(),
                 SQL_NTS, tableName.data(), SQL_NTS);

  SQLCloseCursor(stmt);

  SQLForeignKeys(stmt, catalogName.data(), SQL_NTS, schemaName.data(), SQL_NTS,
                 tableName.data(), SQL_NTS, catalogName.data(), 0,
                 schemaName.data(), SQL_NTS, tableName.data(), SQL_NTS);

  SQLCloseCursor(stmt);

  SQLForeignKeys(stmt, catalogName.data(), SQL_NTS, schemaName.data(), SQL_NTS,
                 tableName.data(), SQL_NTS, catalogName.data(), SQL_NTS, 0,
                 SQL_NTS, tableName.data(), SQL_NTS);

  SQLCloseCursor(stmt);

  SQLForeignKeys(stmt, catalogName.data(), SQL_NTS, schemaName.data(), SQL_NTS,
                 tableName.data(), SQL_NTS, catalogName.data(), SQL_NTS,
                 schemaName.data(), 0, tableName.data(), SQL_NTS);

  SQLCloseCursor(stmt);

  SQLForeignKeys(stmt, catalogName.data(), SQL_NTS, schemaName.data(), SQL_NTS,
                 tableName.data(), SQL_NTS, catalogName.data(), SQL_NTS,
                 schemaName.data(), SQL_NTS, 0, SQL_NTS);

  SQLCloseCursor(stmt);

  SQLForeignKeys(stmt, catalogName.data(), SQL_NTS, schemaName.data(), SQL_NTS,
                 tableName.data(), SQL_NTS, catalogName.data(), SQL_NTS,
                 schemaName.data(), SQL_NTS, tableName.data(), 0);

  SQLCloseCursor(stmt);

  SQLForeignKeys(stmt, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

  SQLCloseCursor(stmt);
}

BOOST_AUTO_TEST_CASE(TestSQLGetStmtAttr) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  connectToLocalServer("odbc-test");

  SQLWCHAR buffer[ODBC_BUFFER_SIZE];
  SQLINTEGER resLen = 0;

  // Everything is ok.
  SQLRETURN ret = SQLGetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE, buffer,
                                 sizeof(buffer), &resLen);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLGetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE, 0, sizeof(buffer), &resLen);
  SQLGetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE, buffer, 0, &resLen);
  SQLGetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE, buffer, sizeof(buffer), 0);
  SQLGetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE, 0, 0, 0);
}

BOOST_AUTO_TEST_CASE(TestSQLSetStmtAttr) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  connectToLocalServer("odbc-test");

  SQLULEN val = 1;

  // Everything is ok.
  SQLRETURN ret =
      SQLSetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE,
                     reinterpret_cast< SQLPOINTER >(val), sizeof(val));

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLSetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE, 0, sizeof(val));
  SQLSetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE,
                 reinterpret_cast< SQLPOINTER >(val), 0);
  SQLSetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE, 0, 0);
}

BOOST_AUTO_TEST_CASE(TestSQLGetDiagField) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.
  connectToLocalServer("odbc-test");

  // Should fail.
  SQLRETURN ret = SQLGetTypeInfo(stmt, SQL_INTERVAL_MONTH);

  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);

  SQLWCHAR buffer[ODBC_BUFFER_SIZE];
  SQLSMALLINT resLen = 0;

  // Everything is ok
  ret = SQLGetDiagField(SQL_HANDLE_STMT, stmt, 1, SQL_DIAG_MESSAGE_TEXT, buffer,
                        sizeof(buffer), &resLen);

  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);

  SQLGetDiagField(SQL_HANDLE_STMT, stmt, 1, SQL_DIAG_MESSAGE_TEXT, 0,
                  sizeof(buffer), &resLen);
  SQLGetDiagField(SQL_HANDLE_STMT, stmt, 1, SQL_DIAG_MESSAGE_TEXT, buffer, 0,
                  &resLen);
  SQLGetDiagField(SQL_HANDLE_STMT, stmt, 1, SQL_DIAG_MESSAGE_TEXT, buffer,
                  sizeof(buffer), 0);
  SQLGetDiagField(SQL_HANDLE_STMT, stmt, 1, SQL_DIAG_MESSAGE_TEXT, 0, 0, 0);
}

BOOST_AUTO_TEST_CASE(TestSQLGetDiagRec) {
  connectToLocalServer("odbc-test");

  SQLWCHAR state[ODBC_BUFFER_SIZE];
  SQLINTEGER nativeError = 0;
  SQLWCHAR message[ODBC_BUFFER_SIZE];
  SQLSMALLINT messageLen = 0;

  // Generating error.
  SQLRETURN ret = SQLGetTypeInfo(stmt, SQL_INTERVAL_MONTH);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);

  // Everything is ok.
  ret = SQLGetDiagRec(SQL_HANDLE_STMT, stmt, 1, state, &nativeError, message,
                      ODBC_BUFFER_SIZE, &messageLen);
  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS);
  std::vector< SQLWCHAR > actualMessage;
  actualMessage.insert(actualMessage.end(), &message[0],
                       &message[messageLen + 1]);

  // Should return error.
  ret = SQLGetDiagRec(SQL_HANDLE_STMT, stmt, 1, state, &nativeError, message,
                      -1, &messageLen);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);

  // Should return message length.
  ret = SQLGetDiagRec(SQL_HANDLE_STMT, stmt, 1, state, &nativeError, message,
                      0, &messageLen);
  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS_WITH_INFO);

  // Check boundary condition on reduced output buffer.
  SQLSMALLINT reducedMessageLen = 8;
  ret = SQLGetDiagRec(SQL_HANDLE_STMT, stmt, 1, state, &nativeError, message,
                      reducedMessageLen + 1, &messageLen);
  BOOST_REQUIRE_EQUAL(ret, SQL_SUCCESS_WITH_INFO);
  BOOST_REQUIRE_EQUAL(messageLen, reducedMessageLen);
  std::vector< SQLWCHAR > reducedExpectedMessage(
      actualMessage.begin(), actualMessage.begin() + reducedMessageLen);
  reducedExpectedMessage.push_back(0);
  std::vector< SQLWCHAR > reducedMessage;
  reducedMessage.insert(reducedMessage.end(), &message[0],
                        &message[messageLen + 1]);
  BOOST_CHECK_EQUAL_COLLECTIONS(reducedMessage.begin(), reducedMessage.end(),
                                reducedExpectedMessage.begin(),
                                reducedExpectedMessage.end());

  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.
  SQLGetDiagRec(SQL_HANDLE_STMT, stmt, 1, 0, &nativeError, message,
                ODBC_BUFFER_SIZE, &messageLen);
  SQLGetDiagRec(SQL_HANDLE_STMT, stmt, 1, state, 0, message, ODBC_BUFFER_SIZE,
                &messageLen);
  SQLGetDiagRec(SQL_HANDLE_STMT, stmt, 1, state, &nativeError, 0,
                ODBC_BUFFER_SIZE, &messageLen);
  SQLGetDiagRec(SQL_HANDLE_STMT, stmt, 1, state, &nativeError, message, 0,
                &messageLen);
  SQLGetDiagRec(SQL_HANDLE_STMT, stmt, 1, state, &nativeError, message,
                ODBC_BUFFER_SIZE, 0);
  SQLGetDiagRec(SQL_HANDLE_STMT, stmt, 1, 0, 0, 0, 0, 0);
}

BOOST_AUTO_TEST_CASE(TestSQLGetData) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  connectToLocalServer("odbc-test");

  std::vector< SQLWCHAR > sql =
      MakeSqlBuffer("SELECT * FROM \"api_robustness_test_001\"");

  SQLRETURN ret = SQLExecDirect(stmt, sql.data(), SQL_NTS);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLFetch(stmt);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLWCHAR buffer[ODBC_BUFFER_SIZE];
  SQLLEN resLen = 0;

  // Everything is ok.
  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, buffer, sizeof(buffer), &resLen);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLFetch(stmt);

  SQLGetData(stmt, 1, SQL_C_WCHAR, 0, sizeof(buffer), &resLen);

  SQLFetch(stmt);

  SQLGetData(stmt, 1, SQL_C_WCHAR, buffer, 0, &resLen);

  SQLFetch(stmt);

  SQLGetData(stmt, 1, SQL_C_WCHAR, buffer, sizeof(buffer), 0);

  SQLFetch(stmt);

  SQLGetData(stmt, 1, SQL_C_WCHAR, 0, 0, 0);

  SQLFetch(stmt);
}

BOOST_AUTO_TEST_CASE(TestSQLGetEnvAttr) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  connectToLocalServer("odbc-test");

  SQLWCHAR buffer[ODBC_BUFFER_SIZE];
  SQLINTEGER resLen = 0;

  // Everything is ok.
  SQLRETURN ret = SQLGetEnvAttr(env, SQL_ATTR_ODBC_VERSION, buffer,
                                sizeof(buffer), &resLen);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_ENV, env);

  SQLGetEnvAttr(env, SQL_ATTR_ODBC_VERSION, nullptr, sizeof(buffer), &resLen);
  SQLGetEnvAttr(env, SQL_ATTR_ODBC_VERSION, buffer, 0, &resLen);
  SQLGetEnvAttr(env, SQL_ATTR_ODBC_VERSION, buffer, sizeof(buffer), 0);
  SQLGetEnvAttr(env, SQL_ATTR_ODBC_VERSION, nullptr, 0, nullptr);
}

BOOST_AUTO_TEST_CASE(TestSQLSpecialColumns) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  connectToLocalServer("odbc-test");

  std::vector< SQLWCHAR > catalogName = {0};
  std::vector< SQLWCHAR > schemaName = MakeSqlBuffer("cache");
  std::vector< SQLWCHAR > tableName = MakeSqlBuffer("TestType");

  // Everything is ok.
  SQLRETURN ret = SQLSpecialColumns(
      stmt, SQL_BEST_ROWID, catalogName.data(), SQL_NTS, schemaName.data(),
      SQL_NTS, tableName.data(), SQL_NTS, SQL_SCOPE_CURROW, SQL_NO_NULLS);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLCloseCursor(stmt);

  SQLSpecialColumns(stmt, SQL_BEST_ROWID, 0, SQL_NTS, schemaName.data(),
                    SQL_NTS, tableName.data(), SQL_NTS, SQL_SCOPE_CURROW,
                    SQL_NO_NULLS);

  SQLCloseCursor(stmt);

  SQLSpecialColumns(stmt, SQL_BEST_ROWID, catalogName.data(), 0,
                    schemaName.data(), SQL_NTS, tableName.data(), SQL_NTS,
                    SQL_SCOPE_CURROW, SQL_NO_NULLS);

  SQLCloseCursor(stmt);

  SQLSpecialColumns(stmt, SQL_BEST_ROWID, catalogName.data(), SQL_NTS, 0,
                    SQL_NTS, tableName.data(), SQL_NTS, SQL_SCOPE_CURROW,
                    SQL_NO_NULLS);

  SQLCloseCursor(stmt);

  SQLSpecialColumns(stmt, SQL_BEST_ROWID, catalogName.data(), SQL_NTS,
                    schemaName.data(), 0, tableName.data(), SQL_NTS,
                    SQL_SCOPE_CURROW, SQL_NO_NULLS);

  SQLCloseCursor(stmt);

  SQLSpecialColumns(stmt, SQL_BEST_ROWID, catalogName.data(), SQL_NTS,
                    schemaName.data(), SQL_NTS, 0, SQL_NTS, SQL_SCOPE_CURROW,
                    SQL_NO_NULLS);

  SQLCloseCursor(stmt);

  SQLSpecialColumns(stmt, SQL_BEST_ROWID, catalogName.data(), SQL_NTS,
                    schemaName.data(), SQL_NTS, tableName.data(), 0,
                    SQL_SCOPE_CURROW, SQL_NO_NULLS);

  SQLCloseCursor(stmt);

  SQLSpecialColumns(stmt, SQL_BEST_ROWID, 0, 0, 0, 0, 0, 0, SQL_SCOPE_CURROW,
                    SQL_NO_NULLS);

  SQLCloseCursor(stmt);
}

// TODO: Memory leak, traced by https://bitquill.atlassian.net/browse/AD-813
BOOST_AUTO_TEST_CASE(TestFetchScrollLast, *disabled()) {
  CheckFetchScrollUnsupportedOrientation(SQL_FETCH_LAST);
}

BOOST_AUTO_TEST_CASE(TestFetchScrollPrior, *disabled()) {
  CheckFetchScrollUnsupportedOrientation(SQL_FETCH_PRIOR);
}

BOOST_AUTO_TEST_CASE(TestFetchScrollFirst, *disabled()) {
  CheckFetchScrollUnsupportedOrientation(SQL_FETCH_FIRST);
}

#ifndef __APPLE__
// only enable for Windows and Linux as it crashes on Mac
// with iODBC, traced by AD-820
// https://bitquill.atlassian.net/browse/AD-820
BOOST_AUTO_TEST_CASE(TestSQLError) {
  // There are no checks because we do not really care what is the result of
  // these calls as long as they do not cause segmentation fault.

  connectToLocalServer("odbc-test");

  SQLWCHAR state[6] = {0};
  SQLINTEGER nativeCode = 0;
  SQLWCHAR message[ODBC_BUFFER_SIZE] = {0};
  SQLSMALLINT messageLen = 0;

  // Generating error by passing unsupported SQL Type (SQL_INTERVAL_MONTH).
  SQLRETURN ret = SQLGetTypeInfo(stmt, SQL_INTERVAL_MONTH);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);

  // Everything is ok.
  ret = SQLError(env, dbc, stmt, state, &nativeCode, message, ODBC_BUFFER_SIZE,
                 &messageLen);
  BOOST_REQUIRE_EQUAL(SQL_SUCCESS, ret);
  BOOST_CHECK_EQUAL(message[messageLen], 0);
  std::vector< SQLWCHAR > actualMessage;
  actualMessage.insert(actualMessage.end(), &message[0],
                       &message[messageLen + 1]);
  // variable actualMessage is to be used in AD-841

  #if 0
  // TODO: [AD-841](https://bitquill.atlassian.net/browse/AD-841)
  // Check boundary condition with reduced buffer size.
  ret = SQLGetTypeInfo(stmt, SQL_INTERVAL_MONTH);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
  SQLSMALLINT reducedMessageLen = 8;
  ret = SQLError(env, dbc, stmt, state, &nativeCode, message,
                 reducedMessageLen + 1, &messageLen);
  BOOST_REQUIRE_EQUAL(SQL_SUCCESS_WITH_INFO, ret);
  BOOST_CHECK_EQUAL(messageLen, reducedMessageLen);
  BOOST_CHECK_EQUAL(message[messageLen], 0);
  std::vector< SQLWCHAR > reducedActualMessage;
  reducedActualMessage.insert(reducedActualMessage.end(), &message[0],
                       &message[messageLen + 1]);
  std::vector< SQLWCHAR > reducedExpectedMessage(
      actualMessage.begin(), actualMessage.begin() + reducedMessageLen);
  reducedExpectedMessage.push_back(0);
  BOOST_CHECK_EQUAL_COLLECTIONS(
      reducedActualMessage.begin(), reducedActualMessage.end(),
      reducedExpectedMessage.begin(), reducedExpectedMessage.end());
#endif

  ret = SQLError(0, dbc, 0, state, &nativeCode, message, ODBC_BUFFER_SIZE,
                 &messageLen);
  if (ret != SQL_SUCCESS && ret != SQL_NO_DATA)
    BOOST_FAIL("Unexpected error");

  ret = SQLError(0, 0, stmt, state, &nativeCode, message, ODBC_BUFFER_SIZE,
                 &messageLen);
  if (ret != SQL_SUCCESS && ret != SQL_NO_DATA)
    BOOST_FAIL("Unexpected error");

  SQLError(0, 0, 0, state, &nativeCode, message, ODBC_BUFFER_SIZE, &messageLen);

  SQLError(0, 0, stmt, 0, &nativeCode, message, ODBC_BUFFER_SIZE, &messageLen);

  SQLError(0, 0, stmt, state, 0, message, ODBC_BUFFER_SIZE, &messageLen);

  SQLError(0, 0, stmt, state, &nativeCode, 0, ODBC_BUFFER_SIZE, &messageLen);

  SQLError(0, 0, stmt, state, &nativeCode, message, 0, &messageLen);

  SQLError(0, 0, stmt, state, &nativeCode, message, ODBC_BUFFER_SIZE, 0);

  SQLError(0, 0, stmt, 0, 0, 0, 0, 0);

  SQLError(0, 0, 0, 0, 0, 0, 0, 0);
}
#endif

BOOST_AUTO_TEST_CASE(TestSQLDiagnosticRecords) {
  connectToLocalServer("odbc-test");

  SQLHANDLE hnd;

  SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_DESC, dbc, &hnd);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
  CheckSQLConnectionDiagnosticError("IM001");

  ret = SQLFreeStmt(stmt, 4);
  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
#ifdef __APPLE__
  CheckSQLStatementDiagnosticError("S1092");
#else
  CheckSQLStatementDiagnosticError("HY092");
#endif
}

BOOST_AUTO_TEST_CASE(TestManyFds) {
  enum { FDS_NUM = 2000 };

  std::FILE* fds[FDS_NUM];

  for (int i = 0; i < FDS_NUM; ++i)
    fds[i] = tmpfile();

  connectToLocalServer("odbc-test");

  for (int i = 0; i < FDS_NUM; ++i) {
    if (fds[i])
      fclose(fds[i]);
  }
}

BOOST_AUTO_TEST_SUITE_END()
