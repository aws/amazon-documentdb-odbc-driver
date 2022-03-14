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
#include <ignite/odbc/jni/documentdb_connection_properties.h>
#include <ignite/odbc/jni/documentdb_database_metadata.h>
#include <ignite/odbc/jni/documentdb_mql_query_context.h>
#include <ignite/odbc/jni/documentdb_query_mapping_service.h>
#include <ignite/odbc/jni/java.h>
#include <ignite/odbc/jni/result_set.h>
#include <ignite/odbc/jni/utils.h>
#include <sql.h>
#include <sqlext.h>

#include <boost/test/unit_test.hpp>
#include <boost/optional.hpp>
#include <boost/algorithm/string.hpp>
#include <string>
#include <vector>
#include <regex>

#include "odbc_test_suite.h"
#include "test_utils.h"

using namespace boost::unit_test;
using namespace ignite::odbc::impl::binary;

using ignite::odbc::OdbcTestSuite;
using ignite_test::GetOdbcErrorMessage;

using ignite::odbc::Connection;
using ignite::odbc::common::ReleaseChars;
using ignite::odbc::common::concurrent::SharedPointer;
using ignite::odbc::config::Configuration;
using ignite::odbc::config::ConnectionStringParser;
using ignite::odbc::jni::DatabaseMetaData;
using ignite::odbc::jni::DocumentDbConnection;
using ignite::odbc::jni::DocumentDbConnectionProperties;
using ignite::odbc::jni::DocumentDbDatabaseMetadata;
using ignite::odbc::jni::DocumentDbMqlQueryContext;
using ignite::odbc::jni::DocumentDbQueryMappingService;
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

BOOST_AUTO_TEST_CASE(TestDocumentDbDatabaseMetaDataGetTables) {
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

    boost::optional<std::string> value;

    resultSet.Get()->GetString(1, value, errInfo);
    BOOST_CHECK(!value);
    resultSet.Get()->GetString("TABLE_CAT", value, errInfo);
    BOOST_CHECK(!value);

    resultSet.Get()->GetString(2, value, errInfo);
    BOOST_CHECK(value);
    resultSet.Get()->GetString("TABLE_SCHEM", value, errInfo);
    BOOST_CHECK(value);
    BOOST_CHECK("test" == *value);

    resultSet.Get()->GetString(3, value, errInfo);
    BOOST_CHECK(value);
    BOOST_CHECK(!value->empty());
    resultSet.Get()->GetString("TABLE_NAME", value, errInfo);
    BOOST_CHECK(value);
    BOOST_CHECK(!value->empty());

    resultSet.Get()->GetString(4, value, errInfo);
    BOOST_CHECK(value);
    BOOST_CHECK("TABLE" == *value);
    resultSet.Get()->GetString("TABLE_TYPE", value, errInfo);    BOOST_CHECK(value);
    BOOST_CHECK("TABLE" == *value);

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

BOOST_AUTO_TEST_CASE(TestDocumentDbDatabaseMetaDataGetColumns) {
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
    std::string schemaPattern = "test";
    std::string tablePattern = "test";
    std::string columnNamePattern;
    SharedPointer< ResultSet > resultSet = databaseMetaData.Get()->GetColumns(
        catalog, schemaPattern, tablePattern, columnNamePattern, errInfo);
    BOOST_CHECK(resultSet.IsValid());
    BOOST_CHECK(resultSet.Get()->IsOpen());

    int columnIndex = 0;
    // Loop the result set records
    bool hasNext = false;
    boost::optional<std::string> value;
    std::string columnName;
    boost::optional<int> intValue;
    do {
        resultSet.Get()->Next(hasNext, errInfo);
        if (!hasNext) {
            break;
        }
        columnIndex++;

        resultSet.Get()->GetString(1, value, errInfo);
        BOOST_CHECK(!value);

        resultSet.Get()->GetString("TABLE_CAT", value, errInfo);
        BOOST_CHECK(!value);

        resultSet.Get()->GetString(2, value, errInfo);
        
        BOOST_CHECK(value);
        BOOST_CHECK_EQUAL("test", *value);
        resultSet.Get()->GetString("TABLE_SCHEM", value, errInfo);
        
        BOOST_CHECK(value);
        BOOST_CHECK_EQUAL("test", *value);

        resultSet.Get()->GetString(3, value, errInfo);
        
        BOOST_CHECK(value);
        BOOST_CHECK_EQUAL("test", *value);
        resultSet.Get()->GetString("TABLE_NAME", value, errInfo);
        
        BOOST_CHECK(value);
        BOOST_CHECK_EQUAL("test", *value);

        resultSet.Get()->GetString(4, value, errInfo);
        
        BOOST_CHECK(value);
        BOOST_CHECK(!value->empty());
        resultSet.Get()->GetString("COLUMN_NAME", value, errInfo);
        
        BOOST_CHECK(value);
        BOOST_CHECK(!value->empty());
        switch (columnIndex) {
            case 1:
                BOOST_CHECK_EQUAL("test__id", *value);
                break;
            case 2:
                BOOST_CHECK_EQUAL("test", *value);
                break;
        }
        columnName = *value;

        resultSet.Get()->GetInt(5, intValue, errInfo);
        BOOST_CHECK(intValue);
        resultSet.Get()->GetInt("DATA_TYPE", intValue, errInfo);
        BOOST_CHECK(intValue);
        switch (columnIndex) {
            case 1:
                BOOST_CHECK_EQUAL(JDBC_TYPE_VARCHAR, *intValue);
                break;
            case 2:
                BOOST_CHECK_EQUAL(JDBC_TYPE_DOUBLE, *intValue);
                break;
            default:
                BOOST_FAIL("Unexpected column index.");
                break;
        }

        resultSet.Get()->GetString(6, value, errInfo);
        BOOST_CHECK(value);
        resultSet.Get()->GetString("TYPE_NAME",  value, errInfo);
        BOOST_CHECK(value);

        resultSet.Get()->GetInt(7, intValue, errInfo);
        resultSet.Get()->GetInt("COLUMN_SIZE", intValue, errInfo);
        BOOST_CHECK(intValue);
        switch (columnIndex) {
            case 1:
                BOOST_CHECK_EQUAL(65536, *intValue);
                break;
            case 2:
                BOOST_CHECK_EQUAL(23, *intValue);
                break;
        }

        resultSet.Get()->GetInt(8, intValue, errInfo);
        BOOST_CHECK(!intValue);
        resultSet.Get()->GetInt("BUFFER_LENGTH", intValue, errInfo);
        BOOST_CHECK(!intValue);

        resultSet.Get()->GetInt(9, intValue, errInfo);
        BOOST_CHECK(!intValue);
        resultSet.Get()->GetInt("DECIMAL_DIGITS", intValue, errInfo);
        BOOST_CHECK(!intValue);

        resultSet.Get()->GetInt(10, intValue, errInfo);
        BOOST_CHECK(!intValue);
        resultSet.Get()->GetInt("NUM_PREC_RADIX", intValue, errInfo);
        BOOST_CHECK(!intValue);

        resultSet.Get()->GetInt(11, intValue, errInfo);
        BOOST_CHECK(intValue);
        resultSet.Get()->GetInt("NULLABLE", intValue, errInfo);
        BOOST_CHECK(intValue);

        resultSet.Get()->GetString(12, value, errInfo);
        BOOST_CHECK(!value);
        resultSet.Get()->GetString("REMARKS", value, errInfo);
        BOOST_CHECK(!value);

        resultSet.Get()->GetString(13, value, errInfo);
        BOOST_CHECK(!value);
        resultSet.Get()->GetString("COLUMN_DEF", value, errInfo);
        BOOST_CHECK(!value);

        resultSet.Get()->GetInt(14, intValue, errInfo);
        BOOST_CHECK(!intValue);
        resultSet.Get()->GetInt("SQL_DATA_TYPE", intValue, errInfo);
        BOOST_CHECK(!intValue);

        resultSet.Get()->GetInt(15, intValue, errInfo);
        BOOST_CHECK(!intValue);
        resultSet.Get()->GetInt("SQL_DATETIME_SUB", intValue, errInfo);
        BOOST_CHECK(!intValue);

        resultSet.Get()->GetInt(16, intValue, errInfo);
        switch (columnIndex) {
          case 1:
            BOOST_CHECK(intValue);
            BOOST_CHECK_EQUAL(262144, intValue);
            break;
          case 2:
            BOOST_CHECK(!intValue);
            break;
        }
        resultSet.Get()->GetInt("CHAR_OCTET_LENGTH", intValue, errInfo);
        switch (columnIndex) {
            case 1:
                BOOST_CHECK(intValue);
                BOOST_CHECK_EQUAL(262144, intValue);
                break;
            case 2:
                BOOST_CHECK(!intValue);
                break;
        }

        resultSet.Get()->GetInt(17, intValue, errInfo);
        BOOST_CHECK(intValue);
        resultSet.Get()->GetInt("ORDINAL_POSITION", intValue,
                                errInfo);
        BOOST_CHECK(intValue);
        BOOST_CHECK_EQUAL(columnIndex, intValue);

        resultSet.Get()->GetString(18, value, errInfo);
        BOOST_CHECK(value);
        resultSet.Get()->GetString("IS_NULLABLE", value, errInfo);
        BOOST_CHECK(value);

        resultSet.Get()->GetString(19, value, errInfo);
        BOOST_CHECK(!value);
        resultSet.Get()->GetString("SCOPE_CATALOG", value, errInfo);
        BOOST_CHECK(!value);

        resultSet.Get()->GetString(20, value, errInfo);
        BOOST_CHECK(!value);
        resultSet.Get()->GetString("SCOPE_SCHEMA", value, errInfo);
        BOOST_CHECK(!value);

        resultSet.Get()->GetString(21, value, errInfo);
        BOOST_CHECK(!value);
        resultSet.Get()->GetString("SCOPE_TABLE", value, errInfo);
        BOOST_CHECK(!value);

        resultSet.Get()->GetInt(22, intValue, errInfo);
        BOOST_CHECK(!intValue);
        resultSet.Get()->GetInt("SOURCE_DATA_TYPE", intValue, errInfo);
        BOOST_CHECK(!intValue);

        resultSet.Get()->GetString(23, value, errInfo);
        BOOST_CHECK(value);
        resultSet.Get()->GetString("IS_AUTOINCREMENT", value, errInfo);
        BOOST_CHECK(value);

        resultSet.Get()->GetString(24, value, errInfo);
        BOOST_CHECK(value);
        resultSet.Get()->GetString("IS_GENERATEDCOLUMN", value, errInfo);
        BOOST_CHECK(value);
    } while (hasNext);
    BOOST_CHECK_EQUAL(2, columnIndex);

    if (resultSet.Get()->Close(errInfo)
        != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
        BOOST_FAIL(errInfo.errMsg);
    }
    BOOST_CHECK(!resultSet.Get()->IsOpen());

    if (dbConnection.Close(errInfo) != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
        BOOST_FAIL(errInfo.errMsg);
    }
    BOOST_CHECK(!dbConnection.IsOpen());
}

BOOST_AUTO_TEST_CASE(TestDocumentDbGetMqlQueryContext) {
  PrepareContext();
  BOOST_REQUIRE(_ctx.Get() != nullptr);

  std::string dsnConnectionString;
  CreateDsnConnectionStringForLocalServer(dsnConnectionString, "odbc-test");

  Configuration config;
  ConnectionStringParser parser(config);
  parser.ParseConnectionString(dsnConnectionString, nullptr);
  JniErrorInfo errInfo;
  DocumentDbConnection dbConnection(_ctx);
  dbConnection.Open(config, errInfo);
  BOOST_CHECK(JniErrorCode::IGNITE_JNI_ERR_SUCCESS == errInfo.code);

  SharedPointer< DocumentDbConnectionProperties > connectionProperties =
      dbConnection.GetConnectionProperties(errInfo);
  BOOST_CHECK(JniErrorCode::IGNITE_JNI_ERR_SUCCESS == errInfo.code);
  BOOST_CHECK(connectionProperties.IsValid());

  SharedPointer< DocumentDbDatabaseMetadata > databaseMetadata =
      dbConnection.GetDatabaseMetadata(errInfo);
  BOOST_CHECK(JniErrorCode::IGNITE_JNI_ERR_SUCCESS == errInfo.code);
  BOOST_CHECK(databaseMetadata.IsValid());

  SharedPointer< DocumentDbQueryMappingService > queryMappingService =
      DocumentDbQueryMappingService::Create(connectionProperties,
                                            databaseMetadata, errInfo);
  BOOST_CHECK(JniErrorCode::IGNITE_JNI_ERR_SUCCESS == errInfo.code);
  BOOST_CHECK(queryMappingService.IsValid());

  SharedPointer< DocumentDbMqlQueryContext > queryContext =
      queryMappingService.Get()->GetMqlQueryContext(
          "SELECT * FROM \"meta_queries_test_001\"", 0, errInfo);
  BOOST_CHECK(JniErrorCode::IGNITE_JNI_ERR_SUCCESS == errInfo.code);
  BOOST_REQUIRE(queryContext.IsValid());
  BOOST_CHECK_EQUAL(13, queryContext.Get()->GetColumnMetadata().size());
  BOOST_REQUIRE(queryContext.Get()->GetColumnMetadata()[0].GetColumnName());
  BOOST_CHECK_EQUAL(
      "meta_queries_test_001__id",
      *queryContext.Get()->GetColumnMetadata()[0].GetColumnName());

  // Test invalid table name will produce error.
  SharedPointer< DocumentDbMqlQueryContext > queryContext2 =
      queryMappingService.Get()->GetMqlQueryContext(
          "SELECT * FROM \"meta_queries_test_001_xxx\"", 0, errInfo);
  BOOST_CHECK(JniErrorCode::IGNITE_JNI_ERR_SUCCESS != errInfo.code);
  std::string modErrMsg = errInfo.errMsg;
  boost::erase_all(modErrMsg, "\r");
  boost::erase_all(modErrMsg, "\n");
  BOOST_CHECK(std::regex_match(
      modErrMsg, std::regex("^Unable to parse SQL.*"
          "Object 'meta_queries_test_001_xxx' not found'$")));
  BOOST_CHECK(!queryContext2.IsValid());
}

BOOST_AUTO_TEST_SUITE_END()
