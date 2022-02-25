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
#include <ignite/odbc/jni/database_metadata.h>
#include <ignite/odbc/jni/documentdb_connection.h>
#include <ignite/odbc/jni/java.h>
#include <ignite/odbc/jni/result_set.h>
#include <ignite/odbc/jni/utils.h>
#include <sql.h>
#include <sqlext.h>

#include <boost/test/unit_test.hpp>
#include <string>
#include <vector>

#include "odbc_test_suite.h"
#include "test_utils.h"

using namespace boost::unit_test;

using ignite::odbc::OdbcTestSuite;
using ignite_test::GetOdbcErrorMessage;

using ignite::odbc::Connection;
using ignite::odbc::common::ReleaseChars;
using ignite::odbc::common::concurrent::SharedPointer;
using ignite::odbc::config::Configuration;
using ignite::odbc::config::ConnectionStringParser;
using ignite::odbc::jni::DatabaseMetaData;
using ignite::odbc::jni::DocumentDbConnection;
using ignite::odbc::jni::ResolveDocumentDbHome;
using ignite::odbc::jni::ResultSet;
using ignite::odbc::jni::java::BuildJvmOptions;
using ignite::odbc::jni::java::JniErrorCode;
using ignite::odbc::jni::java::JniErrorInfo;
using ignite::odbc::jni::java::JniHandlers;

/**
 * Test setup fixture.
 */
struct JniTestSuiteFixture : OdbcTestSuite {
  using OdbcTestSuite::OdbcTestSuite;

  SharedPointer< JniContext > GetJniContext(std::vector< char* >& opts) const {
    SharedPointer< JniContext > ctx(JniContext::Create(
        &opts[0], static_cast< int >(opts.size()), JniHandlers()));
    BOOST_CHECK(ctx.Get() != nullptr);
    return ctx;
  }

  std::string GetJdbcConnectionString() const {
    std::string dsnConnectionString;
    CreateDsnConnectionString(dsnConnectionString);

    Configuration config;
    ConnectionStringParser parser(config);
    parser.ParseConnectionString(dsnConnectionString, nullptr);
    std::string jdbcConnectionString = config.ToJdbcConnectionString();
    return jdbcConnectionString;
  }

  void PrepareContext() {
    if (!_prepared) {
      _jdbcConnectionString = GetJdbcConnectionString();
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
  ~JniTestSuiteFixture() override {
    CleanUpContext();
  }

  bool _prepared = false;

  std::string _jdbcConnectionString;

  std::vector< char* > _opts;

  SharedPointer< JniContext > _ctx;
};

BOOST_FIXTURE_TEST_SUITE(JniTestSuite, JniTestSuiteFixture)

BOOST_AUTO_TEST_CASE(TestDocumentDbConnectionOpen) {
  PrepareContext();
  BOOST_REQUIRE(_ctx.Get() != nullptr);

  std::string dsnConnectionString;
  CreateDsnConnectionString(dsnConnectionString);

  Configuration config;
  ConnectionStringParser parser(config);
  parser.ParseConnectionString(dsnConnectionString, nullptr);
  JniErrorInfo errInfo;

  DocumentDbConnection dbConnection(_ctx);
  BOOST_CHECK(!dbConnection.IsOpen());
  if (dbConnection.Open(config, errInfo)
      != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }
  BOOST_CHECK(dbConnection.IsOpen());
}

BOOST_AUTO_TEST_CASE(TestDocumentDbConnectionClose) {
  PrepareContext();
  BOOST_REQUIRE(_ctx.Get() != nullptr);

  std::string dsnConnectionString;
  CreateDsnConnectionString(dsnConnectionString);

  Configuration config;
  ConnectionStringParser parser(config);
  parser.ParseConnectionString(dsnConnectionString, nullptr);
  JniErrorInfo errInfo;
  DocumentDbConnection dbConnection(_ctx);

  BOOST_CHECK(!dbConnection.IsOpen());
  if (dbConnection.Open(config, errInfo)
      != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }
  BOOST_CHECK(dbConnection.IsOpen());

  if (dbConnection.Close(errInfo) != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }
  BOOST_CHECK(!dbConnection.IsOpen());
}

BOOST_AUTO_TEST_CASE(TestDocumentDatabaseDbMetaDataGetTables) {
  PrepareContext();
  BOOST_REQUIRE(_ctx.Get() != nullptr);

  std::string dsnConnectionString;
  CreateDsnConnectionString(dsnConnectionString);

  Configuration config;
  ConnectionStringParser parser(config);
  parser.ParseConnectionString(dsnConnectionString, nullptr);
  JniErrorInfo errInfo;
  DocumentDbConnection dbConnection(_ctx);

  BOOST_CHECK(!dbConnection.IsOpen());
  if (dbConnection.Open(config, errInfo)
      != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }
  BOOST_CHECK(dbConnection.IsOpen());

  SharedPointer< DatabaseMetaData > databaseMetaData =
      dbConnection.GetMetaData(errInfo);
  BOOST_REQUIRE(databaseMetaData.IsValid());

  std::string catalog;
  std::string schemaPattern;
  std::string tablePattern;
  std::vector< std::string > types = {"TABLE"};
  SharedPointer< ResultSet > resultSet = databaseMetaData.Get()->GetTables(
      catalog, schemaPattern, tablePattern, types, errInfo);
  BOOST_CHECK(resultSet.IsValid());
  BOOST_CHECK(resultSet.Get()->IsOpen());

  // Loop the result set records
  bool hasNext = false;
  do {
    resultSet.Get()->Next(hasNext, errInfo);
    if (!hasNext) {
      break;
    }

    std::string value;
    bool wasNull;

    resultSet.Get()->GetString(1, value, wasNull, errInfo);
    BOOST_CHECK(wasNull);
    resultSet.Get()->GetString("TABLE_CAT", value, wasNull, errInfo);
    BOOST_CHECK(wasNull);

    resultSet.Get()->GetString(2, value, wasNull, errInfo);
    BOOST_CHECK(!wasNull);
    resultSet.Get()->GetString("TABLE_SCHEM", value, wasNull, errInfo);
    BOOST_CHECK(!wasNull);
    BOOST_CHECK("test" == value);

    resultSet.Get()->GetString(3, value, wasNull, errInfo);
    BOOST_CHECK(!wasNull);
    BOOST_CHECK(!value.empty());
    resultSet.Get()->GetString("TABLE_NAME", value, wasNull, errInfo);
    BOOST_CHECK(!wasNull);
    BOOST_CHECK(!value.empty());

    resultSet.Get()->GetString(4, value, wasNull, errInfo);
    BOOST_CHECK(!wasNull);
    BOOST_CHECK("TABLE" == value);
    resultSet.Get()->GetString("TABLE_TYPE", value, wasNull, errInfo);
    BOOST_CHECK(!wasNull);
    BOOST_CHECK("TABLE" == value);

  } while (hasNext);

  if (resultSet.Get()->Close(errInfo) != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }
  BOOST_CHECK(!resultSet.Get()->IsOpen());

  if (dbConnection.Close(errInfo) != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }
  BOOST_CHECK(!dbConnection.IsOpen());
}

BOOST_AUTO_TEST_SUITE_END()
