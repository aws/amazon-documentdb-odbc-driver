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
#include <Windows.h>
#endif

#include <ignite/odbc/common/common.h>
#include <ignite/odbc/common/concurrent.h>
#include <ignite/odbc/config/connection_string_parser.h>
#include <ignite/odbc/connection.h>
#include <ignite/odbc/dsn_config.h>
#include <ignite/odbc/ignite_error.h>
#include <ignite/odbc/jni/java.h>
#include <ignite/odbc/jni/utils.h>
#include <sql.h>
#include <sqlext.h>

#include <boost/test/unit_test.hpp>
#include <string>
#include <vector>

#include "odbc_test_suite.h"
#include "test_utils.h"

using ignite::odbc::OdbcTestSuite;
using namespace boost::unit_test;
using ignite::odbc::if_integration;

using ignite::odbc::common::ReleaseChars;
using ignite::odbc::config::ConnectionStringParser;
using ignite::odbc::jni::ResolveDocumentDbHome;
using ignite::odbc::jni::java::BuildJvmOptions;
using ignite::odbc::jni::java::JniErrorCode;
using ignite::odbc::jni::java::JniHandlers;

/**
 * Test setup fixture.
 */
struct JavaTestSuiteFixture : OdbcTestSuite {
  using OdbcTestSuite::OdbcTestSuite;

  const std::string DATABASE_NAME = "odbc-test";

  SharedPointer< JniContext > GetJniContext(std::vector< char* >& opts) const {
    JniErrorInfo errInfo;
    SharedPointer< JniContext > ctx(JniContext::Create(
        &opts[0], static_cast< int >(opts.size()), JniHandlers(), errInfo));
    BOOST_CHECK(ctx.Get() != nullptr);
    return ctx;
  }

  std::string GetJdbcConnectionString(bool isIntegration,
                                      bool isInternalSshTunnel) const {
    std::string dsnConnectionString;
    if (isIntegration) {
      CreateDsnConnectionStringForRemoteServer(dsnConnectionString, isInternalSshTunnel);
    } else {
      CreateDsnConnectionStringForLocalServer(dsnConnectionString);
    }

    Configuration config;
    ConnectionStringParser parser(config);
    parser.ParseConnectionString(dsnConnectionString, nullptr);
    std::string jdbcConnectionString = config.ToJdbcConnectionString();
    return jdbcConnectionString;
  }

  void PrepareContext(bool isIntegration = false, bool isInternalSshTunnel = false) {
    if (!_prepared) {
      _jdbcConnectionString = GetJdbcConnectionString(isIntegration, isInternalSshTunnel);
      std::string cp = ResolveDocumentDbHome();
      BuildJvmOptions(cp, _opts);
      _ctx = GetJniContext(_opts);
      _prepared = true;
    }
  }

  void CleanUpContext() {
    std::for_each(_opts.begin(), _opts.end(), ReleaseChars);
    _opts.clear();
    _ctx = nullptr;
    _prepared = false;
  }

  /**
   * Destructor.
   */
  ~JavaTestSuiteFixture() override {
    CleanUpContext();
  }

  bool _prepared = false;

  std::string _jdbcConnectionString;

  std::vector< char* > _opts;

  SharedPointer< JniContext > _ctx;

  JavaTestSuiteFixture& operator=(const JavaTestSuiteFixture& other) = delete;
};

struct AutoCloseConnection {
 public:
  AutoCloseConnection(SharedPointer< JniContext > ctx,
                      SharedPointer< GlobalJObject > connection)
      : _ctx(ctx), _connection(connection) {
  }

  ~AutoCloseConnection() {
    if (_ctx.Get() != nullptr && _connection.Get() != nullptr) {
      JniErrorInfo errInfo;
      _ctx.Get()->ConnectionClose(_connection, errInfo);
    }
    _connection = nullptr;
    _ctx = nullptr;
  }

 private:
  SharedPointer< JniContext > _ctx;
  SharedPointer< GlobalJObject > _connection;
  IGNITE_NO_COPY_ASSIGNMENT(AutoCloseConnection);
};

struct AutoCloseResultSet {
 public:
  AutoCloseResultSet(SharedPointer< JniContext > ctx,
                     SharedPointer< GlobalJObject > resultSet)
      : _ctx(ctx), _resultSet(resultSet) {
  }

  ~AutoCloseResultSet() {
    if (_ctx.Get() != nullptr && _resultSet.Get() != nullptr) {
      JniErrorInfo errInfo;
      _ctx.Get()->ResultSetClose(_resultSet, errInfo);
    }
    _resultSet = nullptr;
    _ctx = nullptr;
  }

 private:
  SharedPointer< JniContext > _ctx;
  SharedPointer< GlobalJObject > _resultSet;
  IGNITE_NO_COPY_ASSIGNMENT(AutoCloseResultSet);
};

BOOST_FIXTURE_TEST_SUITE(JavaTestSuite, JavaTestSuiteFixture)

BOOST_AUTO_TEST_CASE(TestDriverManagerGetConnection) {
  PrepareContext();
  BOOST_REQUIRE(_ctx.Get() != nullptr);

  JniErrorInfo errInfo;
  SharedPointer< GlobalJObject > connection;
  JniErrorCode success = _ctx.Get()->DriverManagerGetConnection(
      _jdbcConnectionString.c_str(), connection, errInfo);
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }
  BOOST_REQUIRE(connection.Get() != nullptr);

  _ctx.Get()->ConnectionClose(connection, errInfo);
  connection = SharedPointer< GlobalJObject >(nullptr);
}

BOOST_AUTO_TEST_CASE(TestDocumentDbConnectionGetSshTunnelPort,
                     *precondition(if_integration())) {
  PrepareContext(true, true);  // remote, internal SSH tunnel
  BOOST_REQUIRE(_ctx.Get() != nullptr);

  // get Driver manager connection
  JniErrorInfo errInfo;
  SharedPointer< GlobalJObject > connection;
  JniErrorCode success = _ctx.Get()->DriverManagerGetConnection(
      _jdbcConnectionString.c_str(), connection, errInfo);
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }
  BOOST_REQUIRE(connection.Get());
  AutoCloseConnection autoCloseConnection(_ctx, connection);

  // see if SSH tunnel is active
  bool isActive;
  success = _ctx.Get()->DocumentDbConnectionIsSshTunnelActive(
      connection, isActive, errInfo);
  // if tunnel is not shown as active, or operation not successful, BOOST FAIL
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }
  BOOST_CHECK(isActive);

  // SSH tunnel confirmed to be active, get SSH tunnel local port
  int32_t port;
  success = _ctx.Get()->DocumentDbConnectionGetSshLocalPort(connection, port,
                                                            errInfo);
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    std::string errMsg = errInfo.errMsg;
    BOOST_FAIL(errMsg);
  }

  // if connection successful, port should be a positive number
  BOOST_CHECK(port > 0);
}

BOOST_AUTO_TEST_CASE(TestDocumentDbConnectionGetSshTunnelPortSshTunnelNotActive,
                     *precondition(if_integration())) {
  PrepareContext(true, false);  // remote, external SSH tunnel
  BOOST_REQUIRE(_ctx.Get() != nullptr);

  // get Driver manager connection
  JniErrorInfo errInfo;
  SharedPointer< GlobalJObject > connection;
  JniErrorCode success = _ctx.Get()->DriverManagerGetConnection(
      _jdbcConnectionString.c_str(), connection, errInfo);
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }
  BOOST_REQUIRE(connection.Get());
  AutoCloseConnection autoCloseConnection(_ctx, connection);

  // check if SSH tunnel is not active
  bool isActive;
  success = _ctx.Get()->DocumentDbConnectionIsSshTunnelActive(
      connection, isActive, errInfo);
  // if SSH tunnel is active, or operation not successful, BOOST FAIL
  if (isActive || success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }

  BOOST_CHECK(!isActive);

  // SSH tunnel confirmed to be not active, get SSH tunnel local port
  int32_t port;
  success = _ctx.Get()->DocumentDbConnectionGetSshLocalPort(connection, port,
                                                            errInfo);
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    std::string errMsg = errInfo.errMsg;
    BOOST_FAIL(errMsg);
  }

  // if SSH tunnel not active, SSH local port number should be 0
  BOOST_CHECK_EQUAL(port, 0);
}

BOOST_AUTO_TEST_CASE(TestDocumentDbConnectionGetDatabaseMetadata) {
  PrepareContext();
  BOOST_REQUIRE(_ctx.Get() != nullptr);

  // get Driver manager connection
  JniErrorInfo errInfo;
  SharedPointer< GlobalJObject > connection;
  JniErrorCode success = _ctx.Get()->DriverManagerGetConnection(
      _jdbcConnectionString.c_str(), connection, errInfo);
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }
  BOOST_REQUIRE(connection.Get());
  AutoCloseConnection autoCloseConnection(_ctx, connection);

  // get metadata
  SharedPointer< GlobalJObject > databaseMetadata;
  if (_ctx.Get()->DocumentDbConnectionGetDatabaseMetadata(
          connection, databaseMetadata, errInfo)
      != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }
  BOOST_REQUIRE(databaseMetadata.Get());
}

BOOST_AUTO_TEST_CASE(TestDocumentDbDatabaseSchemaMetadataGetSchemaName) {
  PrepareContext();
  BOOST_REQUIRE(_ctx.Get() != nullptr);

  // get Driver manager connection
  JniErrorInfo errInfo;
  SharedPointer< GlobalJObject > connection;
  JniErrorCode success = _ctx.Get()->DriverManagerGetConnection(
      _jdbcConnectionString.c_str(), connection, errInfo);
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }
  BOOST_REQUIRE(connection.Get());
  AutoCloseConnection autoCloseConnection(_ctx, connection);

  // get metadata
  SharedPointer< GlobalJObject > databaseMetadata;
  if (_ctx.Get()->DocumentDbConnectionGetDatabaseMetadata(
          connection, databaseMetadata, errInfo)
      != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }
  BOOST_REQUIRE(databaseMetadata.Get());

  std::string schemaName;
  bool wasNull;
  success = _ctx.Get()->DocumentDbDatabaseSchemaMetadataGetSchemaName(
      databaseMetadata, schemaName, wasNull, errInfo);
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }

  BOOST_CHECK(!wasNull);
  BOOST_CHECK_EQUAL(schemaName, "_default");
}

BOOST_AUTO_TEST_CASE(TestConnectionGetMetaData) {
  PrepareContext();
  BOOST_REQUIRE(_ctx.Get() != nullptr);

  JniErrorInfo errInfo;
  SharedPointer< GlobalJObject > connection;
  JniErrorCode success = _ctx.Get()->DriverManagerGetConnection(
      _jdbcConnectionString.c_str(), connection, errInfo);
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }
  BOOST_REQUIRE(connection.Get() != nullptr);
  AutoCloseConnection autoCloseConnection(_ctx, connection);

  SharedPointer< GlobalJObject > databaseMetaData;
  if (_ctx.Get()->ConnectionGetMetaData(connection, databaseMetaData, errInfo)
      != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    std::string errMsg = errInfo.errMsg;
    BOOST_FAIL(errMsg);
  }
  BOOST_REQUIRE(databaseMetaData.Get() != nullptr);
}

BOOST_AUTO_TEST_CASE(TestDatabaseMetaDataGetTables) {
  PrepareContext();
  BOOST_REQUIRE(_ctx.Get() != nullptr);

  JniErrorInfo errInfo;
  SharedPointer< GlobalJObject > connection;
  JniErrorCode success = _ctx.Get()->DriverManagerGetConnection(
      _jdbcConnectionString.c_str(), connection, errInfo);
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }
  BOOST_REQUIRE(connection.Get());
  AutoCloseConnection autoCloseConnection(_ctx, connection);

  SharedPointer< GlobalJObject > databaseMetaData;
  if (_ctx.Get()->ConnectionGetMetaData(connection, databaseMetaData, errInfo)
      != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    std::string errMsg = errInfo.errMsg;
    BOOST_FAIL(errMsg);
  }
  BOOST_REQUIRE(databaseMetaData.Get());

  boost::optional< std::string > catalog = boost::none;
  boost::optional< std::string > schemaPattern = boost::none;
  std::string tableNamePattern = "%";
  boost::optional < std::vector< std::string > > types(
      {"TABLE"});  // Need to specify this to get result.
  SharedPointer< GlobalJObject > resultSet;
  if (_ctx.Get()->DatabaseMetaDataGetTables(databaseMetaData, catalog,
                                            schemaPattern, tableNamePattern,
                                            types, resultSet, errInfo)
      != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    std::string errMsg = errInfo.errMsg;
    BOOST_FAIL(errMsg);
  }
  BOOST_REQUIRE(resultSet.Get());
  AutoCloseResultSet autoCloseResultSet(_ctx, resultSet);

  // Get first
  bool hasNext;
  if (_ctx.Get()->ResultSetNext(resultSet, hasNext, errInfo)
      != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    std::string errMsg = errInfo.errMsg;
    BOOST_FAIL(errMsg);
  }
  BOOST_REQUIRE(hasNext);

  int i = 1;
  while (hasNext) {
    boost::optional<std::string> value;
    // TABLE_CAT (i.e., catalog - always NULL in our case)
    if (_ctx.Get()->ResultSetGetString(resultSet, 1, value,  errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(!value);

    // TABLE_CAT (i.e., catalog - always NULL in our case)
    if (_ctx.Get()->ResultSetGetString(resultSet, "TABLE_CAT", value, 
                                       errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(!value);

    // TABLE_SCHEM (i.e., database)
    if (_ctx.Get()->ResultSetGetString(resultSet, 2, value,  errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(value);
    BOOST_REQUIRE(*value == DATABASE_NAME);

    // TABLE_SCHEM (i.e., database)
    if (_ctx.Get()->ResultSetGetString(resultSet, "TABLE_SCHEM", value, 
                                       errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(value);
    BOOST_REQUIRE(*value == DATABASE_NAME);

    // TABLE_NAME
    if (_ctx.Get()->ResultSetGetString(resultSet, 3, value,  errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(value);
    BOOST_REQUIRE(value->size() > 0);

    // TABLE_NAME
    if (_ctx.Get()->ResultSetGetString(resultSet, "TABLE_NAME", value, 
                                       errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(value);
    BOOST_REQUIRE(value->size() > 0);

    // TABLE_TYPE
    if (_ctx.Get()->ResultSetGetString(resultSet, 4, value,  errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(value);
    BOOST_REQUIRE(*value == "TABLE");

    // TABLE_TYPE
    if (_ctx.Get()->ResultSetGetString(resultSet, "TABLE_TYPE", value, 
                                       errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(value);
    BOOST_REQUIRE(*value == "TABLE");

    // check getRow
    boost::optional<int> val;
    if (_ctx.Get()->ResultSetGetRow(resultSet, val,  errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(val);
    BOOST_CHECK_EQUAL(*val, i);

    // Get next
    if (_ctx.Get()->ResultSetNext(resultSet, hasNext, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }

    i++;
  }
}

BOOST_AUTO_TEST_CASE(TestDatabaseMetaDataGetColumns) {
  PrepareContext();
  BOOST_REQUIRE(_ctx.Get() != nullptr);

  JniErrorInfo errInfo;
  SharedPointer< GlobalJObject > connection;
  // get connection
  JniErrorCode success = _ctx.Get()->DriverManagerGetConnection(
      _jdbcConnectionString.c_str(), connection, errInfo);
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }
  BOOST_REQUIRE(connection.Get());
  AutoCloseConnection autoCloseConnection(_ctx, connection);

  // get databaseMetaData object
  SharedPointer< GlobalJObject > databaseMetaData;
  if (_ctx.Get()->ConnectionGetMetaData(connection, databaseMetaData, errInfo)
      != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    std::string errMsg = errInfo.errMsg;
    BOOST_FAIL(errMsg);
  }
  BOOST_REQUIRE(databaseMetaData.Get());

  boost::optional< std::string > catalog;
  boost::optional< std::string > schemaPattern;
  std::string tableNamePattern = "%";
  std::string columnNamePattern = "%";
  SharedPointer< GlobalJObject > resultSet;
  if (_ctx.Get()->DatabaseMetaDataGetColumns(
          databaseMetaData, catalog, schemaPattern, tableNamePattern,
          columnNamePattern, resultSet, errInfo)
      != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    std::string errMsg = errInfo.errMsg;
    BOOST_FAIL(errMsg);
  }
  BOOST_REQUIRE(resultSet.Get());
  AutoCloseResultSet autoCloseResultSet(_ctx, resultSet);

  // Get first
  bool hasNext;
  if (_ctx.Get()->ResultSetNext(resultSet, hasNext, errInfo)
      != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    std::string errMsg = errInfo.errMsg;
    BOOST_FAIL(errMsg);
  }
  BOOST_REQUIRE(hasNext);

  int i = 1;
  while (hasNext) {
    boost::optional<std::string> value;
    // TABLE_CAT (i.e., catalog - always NULL in our case)
    if (_ctx.Get()->ResultSetGetString(resultSet, 1, value,  errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      // grab string from first column
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(!value);

    // TABLE_CAT (i.e., catalog - always NULL in our case)
    if (_ctx.Get()->ResultSetGetString(resultSet, "TABLE_CAT", value, 
                                       errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {  // grab string from first
                                                    // column by name
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(!value);

    // TABLE_SCHEM (i.e., database)
    if (_ctx.Get()->ResultSetGetString(resultSet, 2, value,  errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(value);
    BOOST_CHECK_EQUAL(DATABASE_NAME, *value);

    // TABLE_SCHEM (i.e., database)
    if (_ctx.Get()->ResultSetGetString(resultSet, "TABLE_SCHEM", value, 
                                       errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(value);
    BOOST_CHECK_EQUAL(DATABASE_NAME, *value);

    // TABLE_NAME
    if (_ctx.Get()->ResultSetGetString(resultSet, 3, value,  errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(value);
    BOOST_REQUIRE(value->size() > 0);

    // TABLE_NAME
    if (_ctx.Get()->ResultSetGetString(resultSet, "TABLE_NAME", value, 
                                       errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(value);
    BOOST_REQUIRE(value->size() > 0);

    // COLUMN_NAME
    if (_ctx.Get()->ResultSetGetString(resultSet, 4, value,  errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(value);
    BOOST_REQUIRE(value->size() > 0);

    // COLUMN_NAME
    if (_ctx.Get()->ResultSetGetString(resultSet, "COLUMN_NAME", value, 
                                       errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(value);
    BOOST_REQUIRE(value->size() > 0);

    // ORDINAL_POSITION
    boost::optional< int > val;
    if (_ctx.Get()->ResultSetGetInt(resultSet, 17, val,  errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(val);
    BOOST_REQUIRE(*val > 0);

    // ORDINAL_POSITION
    if (_ctx.Get()->ResultSetGetInt(resultSet, "ORDINAL_POSITION", val, 
                                    errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(val);
    BOOST_REQUIRE(*val > 0);

    // check getRow
    if (_ctx.Get()->ResultSetGetRow(resultSet, val,  errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(val);
    BOOST_CHECK_EQUAL(*val, i);

    // Get next
    if (_ctx.Get()->ResultSetNext(resultSet, hasNext, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }

    i++;
  }
}

BOOST_AUTO_TEST_CASE(TestDatabaseMetaDataGetPrimaryKeys) {
  PrepareContext();
  BOOST_REQUIRE(_ctx.Get() != nullptr);

  JniErrorInfo errInfo;
  SharedPointer< GlobalJObject > connection;
  // get connection
  JniErrorCode success = _ctx.Get()->DriverManagerGetConnection(
      _jdbcConnectionString.c_str(), connection, errInfo);
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }
  BOOST_REQUIRE(connection.Get());
  AutoCloseConnection autoCloseConnection(_ctx, connection);

  // get databaseMetaData object
  SharedPointer< GlobalJObject > databaseMetaData;
  if (_ctx.Get()->ConnectionGetMetaData(connection, databaseMetaData, errInfo)
      != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    std::string errMsg = errInfo.errMsg;
    BOOST_FAIL(errMsg);
  }
  BOOST_REQUIRE(databaseMetaData.Get());

  boost::optional< std::string > catalog = boost::none;
  boost::optional< std::string > schema = boost::none;
  boost::optional< std::string > table(std::string("jni_test_001"));
  std::string pkColumn = "jni_test_001__id";
  SharedPointer< GlobalJObject > resultSet;
  if (_ctx.Get()->DatabaseMetaDataGetPrimaryKeys(
          databaseMetaData, catalog, schema, table, resultSet, errInfo)
      != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    std::string errMsg = errInfo.errMsg;
    BOOST_FAIL(errMsg);
  }
  BOOST_REQUIRE(resultSet.Get());
  AutoCloseResultSet autoCloseResultSet(_ctx, resultSet);

  // Get first
  bool hasNext;
  if (_ctx.Get()->ResultSetNext(resultSet, hasNext, errInfo)
      != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    std::string errMsg = errInfo.errMsg;
    BOOST_FAIL(errMsg);
  }
  BOOST_REQUIRE(hasNext);

  int i = 0;
  while (hasNext) {
    boost::optional< std::string > value;
    // TABLE_CAT (i.e., catalog - always NULL in our case)
    if (_ctx.Get()->ResultSetGetString(resultSet, 1, value, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      // grab string from first column
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(!value);

    // TABLE_CAT (i.e., catalog - always NULL in our case)
    if (_ctx.Get()->ResultSetGetString(resultSet, "TABLE_CAT", value,
                                       errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {  // grab string from first
                                                    // column by name
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(!value);

    // TABLE_SCHEM (i.e., database)
    if (_ctx.Get()->ResultSetGetString(resultSet, 2, value, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(value);
    BOOST_CHECK_EQUAL(DATABASE_NAME, *value);

    // TABLE_SCHEM (i.e., database)
    if (_ctx.Get()->ResultSetGetString(resultSet, "TABLE_SCHEM", value,
                                       errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(value);
    BOOST_CHECK_EQUAL(DATABASE_NAME, *value);

    // TABLE_NAME
    if (_ctx.Get()->ResultSetGetString(resultSet, 3, value, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(value);
    BOOST_CHECK_EQUAL(table, *value);

    // TABLE_NAME
    if (_ctx.Get()->ResultSetGetString(resultSet, "TABLE_NAME", value, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(value);
    BOOST_CHECK_EQUAL(table, *value);

    // COLUMN_NAME
    if (_ctx.Get()->ResultSetGetString(resultSet, 4, value, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(value);
    BOOST_CHECK_EQUAL(pkColumn, *value);

    // COLUMN_NAME
    if (_ctx.Get()->ResultSetGetString(resultSet, "COLUMN_NAME", value, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(value);
    BOOST_CHECK_EQUAL(pkColumn, *value);

    // KEY_SEQ
    boost::optional< int > intVal;
    if (_ctx.Get()->ResultSetGetInt(resultSet, 5, intVal, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(intVal);
    BOOST_CHECK_EQUAL(1, *intVal);

    // KEY_SEQ
    if (_ctx.Get()->ResultSetGetInt(resultSet, "KEY_SEQ", intVal, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(intVal);
    BOOST_CHECK_EQUAL(1, *intVal);

    // PK_NAME
    if (_ctx.Get()->ResultSetGetString(resultSet, 6, value, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(!value);

    // PK_NAME
    if (_ctx.Get()->ResultSetGetString(resultSet, "PK_NAME", value,
                                       errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(!value);

    // Get next
    if (_ctx.Get()->ResultSetNext(resultSet, hasNext, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }

    i++;
  }
  BOOST_CHECK_EQUAL(1, i);
}

BOOST_AUTO_TEST_CASE(TestDatabaseMetaDataGetImportedKeys) {
  PrepareContext();
  BOOST_REQUIRE(_ctx.Get() != nullptr);

  JniErrorInfo errInfo;
  SharedPointer< GlobalJObject > connection;
  // get connection
  JniErrorCode success = _ctx.Get()->DriverManagerGetConnection(
      _jdbcConnectionString.c_str(), connection, errInfo);
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }
  BOOST_REQUIRE(connection.Get());
  AutoCloseConnection autoCloseConnection(_ctx, connection);

  // get databaseMetaData object
  SharedPointer< GlobalJObject > databaseMetaData;
  if (_ctx.Get()->ConnectionGetMetaData(connection, databaseMetaData, errInfo)
      != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    std::string errMsg = errInfo.errMsg;
    BOOST_FAIL(errMsg);
  }
  BOOST_REQUIRE(databaseMetaData.Get());

  const boost::optional< std::string > catalog = boost::none;
  const boost::optional< std::string > schema = boost::none;
  std::string fkTableName = "jni_test_001_sub_doc";
  SharedPointer< GlobalJObject > resultSet;
  if (_ctx.Get()->DatabaseMetaDataGetImportedKeys(
          databaseMetaData, catalog, schema, fkTableName, resultSet, errInfo)
      != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    std::string errMsg = errInfo.errMsg;
    BOOST_FAIL(errMsg);
  }
  BOOST_REQUIRE(resultSet.Get());
  AutoCloseResultSet autoCloseResultSet(_ctx, resultSet);

  // Get first
  bool hasNext;
  if (_ctx.Get()->ResultSetNext(resultSet, hasNext, errInfo)
      != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    std::string errMsg = errInfo.errMsg;
    BOOST_FAIL(errMsg);
  }
  BOOST_REQUIRE(hasNext);

  std::string table = "jni_test_001_sub";
  std::string fkColumn = "jni_test_001_sub__id";

  int i = 1;
  while (hasNext) {
    boost::optional< std::string > value;
    // TABLE_CAT (i.e., catalog - always NULL in our case)
    if (_ctx.Get()->ResultSetGetString(resultSet, 1, value, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      // grab string from first column
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(!value);

    // PKTABLE_CAT (i.e., catalog - always NULL in our case)
    if (_ctx.Get()->ResultSetGetString(resultSet, "PKTABLE_CAT", value,
                                       errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {  // grab string from first
                                                    // column by name
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(!value);

    // PKTABLE_SCHEM (i.e., database)
    if (_ctx.Get()->ResultSetGetString(resultSet, 2, value, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(value);
    BOOST_CHECK_EQUAL(DATABASE_NAME, *value);

    // PKTABLE_SCHEM (i.e., database)
    if (_ctx.Get()->ResultSetGetString(resultSet, "PKTABLE_SCHEM", value, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(value);
    BOOST_CHECK_EQUAL(DATABASE_NAME, *value);

    // PKTABLE_NAME
    if (_ctx.Get()->ResultSetGetString(resultSet, 3, value, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(value);
    BOOST_CHECK_EQUAL(table, *value);

    // PKTABLE_NAME
    if (_ctx.Get()->ResultSetGetString(resultSet, "PKTABLE_NAME", value, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(value);
    BOOST_REQUIRE(value->size() > 0);
    BOOST_CHECK_EQUAL(table, *value);

    // PKCOLUMN_NAME
    if (_ctx.Get()->ResultSetGetString(resultSet, 4, value, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(value);
    BOOST_REQUIRE(value->size() > 0);
    BOOST_CHECK_EQUAL(fkColumn, *value);

    // PKCOLUMN_NAME
    if (_ctx.Get()->ResultSetGetString(resultSet, "PKCOLUMN_NAME", value, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(value);
    BOOST_REQUIRE(value->size() > 0);
    BOOST_CHECK_EQUAL(fkColumn, *value);

    // FKTABLE_CAT (always null)
    if (_ctx.Get()->ResultSetGetString(resultSet, 5, value, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(!value);

    // FKTABLE_CAT
    if (_ctx.Get()->ResultSetGetString(resultSet, "FKTABLE_CAT", value,
                                       errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(!value);

    // FKTABLE_SCHEM (i.e., database)
    if (_ctx.Get()->ResultSetGetString(resultSet, 6, value, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(value);
    BOOST_CHECK_EQUAL(DATABASE_NAME, *value);

    // FKTABLE_SCHEM (i.e., database)
    if (_ctx.Get()->ResultSetGetString(resultSet, "FKTABLE_SCHEM", value,
                                       errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(value);
    BOOST_CHECK_EQUAL(DATABASE_NAME, *value);

    // FKTABLE_NAME
    if (_ctx.Get()->ResultSetGetString(resultSet, 7, value, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(value);
    BOOST_CHECK_EQUAL(fkTableName, *value);

    // FKTABLE_NAME
    if (_ctx.Get()->ResultSetGetString(resultSet, "FKTABLE_NAME", value,
                                       errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(value);
    BOOST_REQUIRE(value->size() > 0);
    BOOST_CHECK_EQUAL(fkTableName, *value);

    // FKCOLUMN_NAME
    if (_ctx.Get()->ResultSetGetString(resultSet, 8, value, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }

    BOOST_REQUIRE(value);
    BOOST_REQUIRE(value->size() > 0);
    BOOST_CHECK_EQUAL(fkColumn, *value);

    // FKCOLUMN_NAME
    if (_ctx.Get()->ResultSetGetString(resultSet, "FKCOLUMN_NAME", value,
                                       errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(value);
    BOOST_REQUIRE(value->size() > 0);
    BOOST_CHECK_EQUAL(fkColumn, *value);

    // KEY_SEQ
    boost::optional< int > val;
    if (_ctx.Get()->ResultSetGetInt(resultSet, 9, val, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(val);
    BOOST_CHECK_EQUAL(1, *val);

    // KEY_SEQ
    if (_ctx.Get()->ResultSetGetInt(resultSet, "KEY_SEQ", val, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(val);
    BOOST_CHECK_EQUAL(1, *val);

    // UPDATE_RULE
    if (_ctx.Get()->ResultSetGetInt(resultSet, 10, val, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(val);

    // UPDATE_RULE
    if (_ctx.Get()->ResultSetGetInt(resultSet, "UPDATE_RULE", val, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(val);

    // DELETE_RULE
    if (_ctx.Get()->ResultSetGetInt(resultSet, 11, val, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(val);

    // DELETE_RULE
    if (_ctx.Get()->ResultSetGetInt(resultSet, "DELETE_RULE", val, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(val);

    // FK_NAME
    if (_ctx.Get()->ResultSetGetString(resultSet, 12, value, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(!value);

    // FK_NAME
    if (_ctx.Get()->ResultSetGetString(resultSet, "FK_NAME", value, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(!value);

    // PK_NAME
    if (_ctx.Get()->ResultSetGetString(resultSet, 13, value, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(!value);

    // PK_NAME
    if (_ctx.Get()->ResultSetGetString(resultSet, "PK_NAME", value, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(!value);

    // DEFERRABILITY
    if (_ctx.Get()->ResultSetGetInt(resultSet, 14, val, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(val);

    // DEFERRABILITY
    if (_ctx.Get()->ResultSetGetInt(resultSet, "DEFERRABILITY", val, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }
    BOOST_CHECK(val);

    // Get next
    if (_ctx.Get()->ResultSetNext(resultSet, hasNext, errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      std::string errMsg = errInfo.errMsg;
      BOOST_FAIL(errMsg);
    }

    i++;
  }
  BOOST_CHECK_EQUAL(2, i);
}

BOOST_AUTO_TEST_CASE(TestDatabaseMetaDataGetImportedKeysReturnsNone) {
  PrepareContext();
  BOOST_REQUIRE(_ctx.Get() != nullptr);

  JniErrorInfo errInfo;
  SharedPointer< GlobalJObject > connection;
  // get connection
  JniErrorCode success = _ctx.Get()->DriverManagerGetConnection(
      _jdbcConnectionString.c_str(), connection, errInfo);
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }
  BOOST_REQUIRE(connection.Get());
  AutoCloseConnection autoCloseConnection(_ctx, connection);

  // get databaseMetaData object
  SharedPointer< GlobalJObject > databaseMetaData;
  if (_ctx.Get()->ConnectionGetMetaData(connection, databaseMetaData, errInfo)
      != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    std::string errMsg = errInfo.errMsg;
    BOOST_FAIL(errMsg);
  }
  BOOST_REQUIRE(databaseMetaData.Get());

  const boost::optional< std::string > catalog(std::string(""));
  const boost::optional< std::string > schema(std::string(""));
  std::string fkTableName("");
  SharedPointer< GlobalJObject > resultSet;
  if (_ctx.Get()->DatabaseMetaDataGetImportedKeys(
          databaseMetaData, catalog, schema, fkTableName, resultSet, errInfo)
      != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    std::string errMsg = errInfo.errMsg;
    BOOST_FAIL(errMsg);
  }
  BOOST_REQUIRE(resultSet.Get());
  AutoCloseResultSet autoCloseResultSet(_ctx, resultSet);

  bool hasNext;
  if (_ctx.Get()->ResultSetNext(resultSet, hasNext, errInfo)
      != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    std::string errMsg = errInfo.errMsg;
    BOOST_FAIL(errMsg);
  }
  BOOST_CHECK(!hasNext);
}

BOOST_AUTO_TEST_SUITE_END()
