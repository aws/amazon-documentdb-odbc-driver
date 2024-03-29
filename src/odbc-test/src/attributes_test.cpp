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
#include <sqlext.h>

#include <algorithm>
#include <boost/test/unit_test.hpp>
#include <string>
#include <vector>

#include "complex_type.h"
#include "documentdb/odbc/connection.h"
#include "documentdb/odbc/binary/binary_object.h"
#include "documentdb/odbc/common/fixed_size_array.h"
#include "documentdb/odbc/impl/binary/binary_utils.h"
#include "documentdb/odbc/utility.h"
#include "odbc_test_suite.h"
#include "test_type.h"
#include "test_utils.h"

using namespace documentdb;
using namespace documentdb::odbc::common;
using namespace documentdb_test;
using namespace documentdb::odbc::binary;
using namespace documentdb::odbc::impl::binary;
using namespace documentdb::odbc::impl::interop;

using namespace boost::unit_test;

using documentdb::odbc::impl::binary::BinaryUtils;

/**
 * Test setup fixture.
 */
struct AttributesTestSuiteFixture : odbc::OdbcTestSuite {
  /**
   * Constructor.
   */
  AttributesTestSuiteFixture() {
    // No-op
  }

  /**
   * Destructor.
   */
  ~AttributesTestSuiteFixture() override = default;

  /**
   * Connect to the local server with the database name
   *
   * @param databaseName Database Name
   */
  void connectToLocalServer(std::string databaseName) {
    std::string dsnConnectionString;
    CreateDsnConnectionStringForLocalServer(dsnConnectionString, databaseName);

    Connect(dsnConnectionString);
  }

};

BOOST_FIXTURE_TEST_SUITE(AttributesTestSuite, AttributesTestSuiteFixture)

BOOST_AUTO_TEST_CASE(ConnectionAttributeConnectionDeadGet) {
  connectToLocalServer("odbc-test");

  SQLUINTEGER dead = SQL_CD_TRUE;
  SQLRETURN ret;

  ret = SQLGetConnectAttr(dbc, SQL_ATTR_CONNECTION_DEAD, &dead, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  BOOST_REQUIRE_EQUAL(dead, SQL_CD_FALSE);
}

BOOST_AUTO_TEST_CASE(ConnectionAttributeConnectionDeadSet) {
  connectToLocalServer("odbc-test");

  SQLUINTEGER dead = SQL_CD_TRUE;
  SQLRETURN ret;

  ret = SQLSetConnectAttr(dbc, SQL_ATTR_CONNECTION_DEAD, &dead, 0);

  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);

  // According to
  // https://docs.microsoft.com/en-us/sql/odbc/reference/syntax/sqlsetconnectattr-function#diagnostics
  CheckSQLConnectionDiagnosticError("HY092");
}

BOOST_AUTO_TEST_CASE(StatementAttributeQueryTimeout) {
  connectToLocalServer("odbc-test");

  SQLULEN timeout = -1;
  SQLRETURN ret = SQLGetStmtAttr(stmt, SQL_ATTR_QUERY_TIMEOUT, &timeout, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  BOOST_REQUIRE_EQUAL(timeout, 0);

  ret = SQLSetStmtAttr(stmt, SQL_ATTR_QUERY_TIMEOUT,
                       reinterpret_cast< SQLPOINTER >(7), 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  timeout = -1;

  ret = SQLGetStmtAttr(stmt, SQL_ATTR_QUERY_TIMEOUT, &timeout, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
  BOOST_REQUIRE_EQUAL(timeout, 7);
}

BOOST_AUTO_TEST_CASE(ConnectionAttributeDefaultLoginTimeout) {
  Prepare();

  std::string dsnConnectionString;
  CreateDsnConnectionStringForLocalServer(dsnConnectionString, "odbc-test");

  std::vector< SQLWCHAR > connectStr(dsnConnectionString.begin(),
                                     dsnConnectionString.end());

  SQLWCHAR outstr[ODBC_BUFFER_SIZE];
  SQLSMALLINT outstrlen;

  // Connecting to ODBC server.
  SQLRETURN ret = SQLDriverConnect(
      dbc, NULL, &connectStr[0],
      static_cast< SQLSMALLINT >(connectStr.size()), outstr,
      sizeof(outstr), &outstrlen, SQL_DRIVER_COMPLETE);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  SQLUINTEGER timeout = -1;
  ret =
      SQLGetConnectAttr(dbc, SQL_ATTR_LOGIN_TIMEOUT, &timeout, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);
  BOOST_REQUIRE_EQUAL(
      timeout, (SQLUINTEGER)Configuration::DefaultValue::loginTimeoutSec);
}

// iODBC not working as expected 
// TODO investigate why the setConnectAttr are not being called before
// establishing connection
// enable test after fix
// https://github.com/aws/amazon-documentdb-odbc-driver/issues/186
#ifndef __APPLE__
BOOST_AUTO_TEST_CASE(ConnectionAttributeLoginTimeout) {
  Prepare();

  SQLRETURN ret = SQLSetConnectAttr(dbc, SQL_ATTR_LOGIN_TIMEOUT,
                          reinterpret_cast< SQLPOINTER >(42), 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  std::string dsnConnectionString;
  CreateDsnConnectionStringForLocalServer(dsnConnectionString, "odbc-test");

  std::vector< SQLWCHAR > connectStr(dsnConnectionString.begin(),
                                     dsnConnectionString.end());

  SQLWCHAR outstr[ODBC_BUFFER_SIZE];
  SQLSMALLINT outstrlen;

  // Connecting to ODBC server.
  ret = SQLDriverConnect(dbc, NULL, &connectStr[0],
                         static_cast< SQLSMALLINT >(connectStr.size()), outstr,
                         sizeof(outstr), &outstrlen, SQL_DRIVER_COMPLETE);

  SQLUINTEGER timeout = -1;

  ret = SQLGetConnectAttr(dbc, SQL_ATTR_LOGIN_TIMEOUT, &timeout, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);
  BOOST_REQUIRE_EQUAL(timeout, 42);
}
#endif

/**
 * Check that environment returns expected version of ODBC standard.
 *
 * 1. Start node.
 * 2. Establish connection using ODBC driver.
 * 3. Get current ODBC version from env handle.
 * 4. Check that version is of the expected value.
 */
BOOST_AUTO_TEST_CASE(TestSQLGetEnvAttrDriverVersion) {
  connectToLocalServer("odbc-test");

  SQLINTEGER version;
  SQLRETURN ret = SQLGetEnvAttr(env, SQL_ATTR_ODBC_VERSION, &version, 0, 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_ENV, env);

  BOOST_CHECK_EQUAL(version, SQL_OV_ODBC3);
}

BOOST_AUTO_TEST_SUITE_END()
