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
#include <ignite/odbc/jni/jdbc_column_metadata.h>
#include <ignite/odbc/jni/result_set.h>
#include <ignite/odbc/jni/utils.h>
#include <sql.h>
#include <sqlext.h>

#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>
#include <boost/test/unit_test.hpp>
#include <regex>
#include <string>
#include <vector>

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
using ignite::odbc::jni::JdbcColumnMetadata;
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

  const std::string DATABASE_NAME = "odbc-test";
  const std::string TABLE_NAME = "jni_test_001";
  const std::string TABLE_ID_COLUMN_NAME = "jni_test_001__id";

  SharedPointer< JniContext > GetJniContext(std::vector< char* >& opts) const {
    JniErrorInfo errInfo;
    SharedPointer< JniContext > ctx(JniContext::Create(
        &opts[0], static_cast< int >(opts.size()), JniHandlers(), errInfo));
    BOOST_CHECK(ctx.Get() != nullptr);
    return ctx;
  }

  std::string GetJdbcConnectionString() const {
    std::string dsnConnectionString;
    CreateDsnConnectionStringForLocalServer(dsnConnectionString);

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

  void ValidateJdbcColumnMetadata(
      std::vector< JdbcColumnMetadata >& columnMetadata) const {
    BOOST_CHECK_EQUAL(13, columnMetadata.size());
    int32_t index = 0;
    JdbcColumnMetadata columnMetadataItem = columnMetadata[index];
    BOOST_CHECK_EQUAL(index, columnMetadataItem.GetOrdinal());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsAutoIncrement());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsCaseSensitive());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsSearchable());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsCurrency());
    BOOST_CHECK_EQUAL(SQL_NO_NULLS, columnMetadataItem.GetNullable());
    // 'signed' is set to 'true', by default, in Calcite
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsSigned());
    BOOST_CHECK_EQUAL(65536, columnMetadataItem.GetColumnDisplaySize());
    BOOST_CHECK_EQUAL(TABLE_ID_COLUMN_NAME,
                      *columnMetadataItem.GetColumnLabel());
    BOOST_CHECK_EQUAL(TABLE_ID_COLUMN_NAME,
                      *columnMetadataItem.GetColumnName());
    BOOST_CHECK_EQUAL(DATABASE_NAME, *columnMetadataItem.GetSchemaName());
    BOOST_CHECK_EQUAL(65536, columnMetadataItem.GetPrecision());
    BOOST_CHECK_EQUAL(0, columnMetadataItem.GetScale());
    BOOST_CHECK_EQUAL(TABLE_NAME, *columnMetadataItem.GetTableName());
    BOOST_CHECK(!columnMetadataItem.GetCatalogName());
    BOOST_CHECK_EQUAL(JDBC_TYPE_VARCHAR, columnMetadataItem.GetColumnType());
    BOOST_CHECK_EQUAL("VARCHAR", *columnMetadataItem.GetColumnTypeName());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsReadOnly());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsWritable());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsDefinitelyWritable());
    BOOST_CHECK_EQUAL("java.lang.String",
                      *columnMetadataItem.GetColumnClassName());

    index = 1;
    columnMetadataItem = columnMetadata[index];
    BOOST_CHECK_EQUAL(index, columnMetadataItem.GetOrdinal());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsAutoIncrement());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsCaseSensitive());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsSearchable());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsCurrency());
    BOOST_CHECK_EQUAL(SQL_NULLABLE, columnMetadataItem.GetNullable());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsSigned());
    BOOST_CHECK_EQUAL(19, columnMetadataItem.GetColumnDisplaySize());
    BOOST_CHECK_EQUAL("fieldDecimal128", *columnMetadataItem.GetColumnLabel());
    BOOST_CHECK_EQUAL("fieldDecimal128", *columnMetadataItem.GetColumnName());
    BOOST_CHECK_EQUAL(DATABASE_NAME, *columnMetadataItem.GetSchemaName());
    BOOST_CHECK_EQUAL(19, columnMetadataItem.GetPrecision());
    BOOST_CHECK_EQUAL(19, columnMetadataItem.GetScale());
    BOOST_CHECK_EQUAL(TABLE_NAME, *columnMetadataItem.GetTableName());
    BOOST_CHECK(!columnMetadataItem.GetCatalogName());
    BOOST_CHECK_EQUAL(JDBC_TYPE_DECIMAL, columnMetadataItem.GetColumnType());
    BOOST_CHECK_EQUAL("DECIMAL", *columnMetadataItem.GetColumnTypeName());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsReadOnly());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsWritable());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsDefinitelyWritable());
    BOOST_CHECK_EQUAL("java.math.BigDecimal",
                      *columnMetadataItem.GetColumnClassName());

    index = 2;
    columnMetadataItem = columnMetadata[index];
    BOOST_CHECK_EQUAL(index, columnMetadataItem.GetOrdinal());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsAutoIncrement());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsCaseSensitive());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsSearchable());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsCurrency());
    BOOST_CHECK_EQUAL(SQL_NULLABLE, columnMetadataItem.GetNullable());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsSigned());
    BOOST_CHECK_EQUAL(15, columnMetadataItem.GetColumnDisplaySize());
    BOOST_CHECK_EQUAL("fieldDouble", *columnMetadataItem.GetColumnLabel());
    BOOST_CHECK_EQUAL("fieldDouble", *columnMetadataItem.GetColumnName());
    BOOST_CHECK_EQUAL(DATABASE_NAME, *columnMetadataItem.GetSchemaName());
    BOOST_CHECK_EQUAL(15, columnMetadataItem.GetPrecision());
    BOOST_CHECK_EQUAL(0, columnMetadataItem.GetScale());
    BOOST_CHECK_EQUAL(TABLE_NAME, *columnMetadataItem.GetTableName());
    BOOST_CHECK(!columnMetadataItem.GetCatalogName());
    BOOST_CHECK_EQUAL(JDBC_TYPE_DOUBLE, columnMetadataItem.GetColumnType());
    BOOST_CHECK_EQUAL("DOUBLE", *columnMetadataItem.GetColumnTypeName());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsReadOnly());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsWritable());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsDefinitelyWritable());
    BOOST_CHECK_EQUAL("java.lang.Double",
                      *columnMetadataItem.GetColumnClassName());

    index = 3;
    columnMetadataItem = columnMetadata[index];
    BOOST_CHECK_EQUAL(index, columnMetadataItem.GetOrdinal());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsAutoIncrement());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsCaseSensitive());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsSearchable());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsCurrency());
    BOOST_CHECK_EQUAL(SQL_NULLABLE, columnMetadataItem.GetNullable());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsSigned());
    BOOST_CHECK_EQUAL(65536, columnMetadataItem.GetColumnDisplaySize());
    BOOST_CHECK_EQUAL("fieldString", *columnMetadataItem.GetColumnLabel());
    BOOST_CHECK_EQUAL("fieldString", *columnMetadataItem.GetColumnName());
    BOOST_CHECK_EQUAL(DATABASE_NAME, *columnMetadataItem.GetSchemaName());
    BOOST_CHECK_EQUAL(65536, columnMetadataItem.GetPrecision());
    BOOST_CHECK_EQUAL(0, columnMetadataItem.GetScale());
    BOOST_CHECK_EQUAL(TABLE_NAME, *columnMetadataItem.GetTableName());
    BOOST_CHECK(!columnMetadataItem.GetCatalogName());
    BOOST_CHECK_EQUAL(JDBC_TYPE_VARCHAR, columnMetadataItem.GetColumnType());
    BOOST_CHECK_EQUAL("VARCHAR", *columnMetadataItem.GetColumnTypeName());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsReadOnly());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsWritable());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsDefinitelyWritable());
    BOOST_CHECK_EQUAL("java.lang.String",
                      *columnMetadataItem.GetColumnClassName());

    index = 4;
    columnMetadataItem = columnMetadata[index];
    BOOST_CHECK_EQUAL(index, columnMetadataItem.GetOrdinal());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsAutoIncrement());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsCaseSensitive());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsSearchable());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsCurrency());
    BOOST_CHECK_EQUAL(SQL_NULLABLE, columnMetadataItem.GetNullable());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsSigned());
    BOOST_CHECK_EQUAL(65536, columnMetadataItem.GetColumnDisplaySize());
    BOOST_CHECK_EQUAL("fieldObjectId", *columnMetadataItem.GetColumnLabel());
    BOOST_CHECK_EQUAL("fieldObjectId", *columnMetadataItem.GetColumnName());
    BOOST_CHECK_EQUAL(DATABASE_NAME, *columnMetadataItem.GetSchemaName());
    BOOST_CHECK_EQUAL(65536, columnMetadataItem.GetPrecision());
    BOOST_CHECK_EQUAL(0, columnMetadataItem.GetScale());
    BOOST_CHECK_EQUAL(TABLE_NAME, *columnMetadataItem.GetTableName());
    BOOST_CHECK(!columnMetadataItem.GetCatalogName());
    BOOST_CHECK_EQUAL(JDBC_TYPE_VARCHAR, columnMetadataItem.GetColumnType());
    BOOST_CHECK_EQUAL("VARCHAR", *columnMetadataItem.GetColumnTypeName());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsReadOnly());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsWritable());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsDefinitelyWritable());
    BOOST_CHECK_EQUAL("java.lang.String",
                      *columnMetadataItem.GetColumnClassName());

    index = 5;
    columnMetadataItem = columnMetadata[index];
    BOOST_CHECK_EQUAL(index, columnMetadataItem.GetOrdinal());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsAutoIncrement());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsCaseSensitive());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsSearchable());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsCurrency());
    BOOST_CHECK_EQUAL(SQL_NULLABLE, columnMetadataItem.GetNullable());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsSigned());
    BOOST_CHECK_EQUAL(1, columnMetadataItem.GetColumnDisplaySize());
    BOOST_CHECK_EQUAL("fieldBoolean", *columnMetadataItem.GetColumnLabel());
    BOOST_CHECK_EQUAL("fieldBoolean", *columnMetadataItem.GetColumnName());
    BOOST_CHECK_EQUAL(DATABASE_NAME, *columnMetadataItem.GetSchemaName());
    BOOST_CHECK_EQUAL(1, columnMetadataItem.GetPrecision());
    BOOST_CHECK_EQUAL(0, columnMetadataItem.GetScale());
    BOOST_CHECK_EQUAL(TABLE_NAME, *columnMetadataItem.GetTableName());
    BOOST_CHECK(!columnMetadataItem.GetCatalogName());
    BOOST_CHECK_EQUAL(JDBC_TYPE_BOOLEAN, columnMetadataItem.GetColumnType());
    BOOST_CHECK_EQUAL("BOOLEAN", *columnMetadataItem.GetColumnTypeName());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsReadOnly());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsWritable());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsDefinitelyWritable());
    BOOST_CHECK_EQUAL("java.lang.Boolean",
                      *columnMetadataItem.GetColumnClassName());

    index = 6;
    columnMetadataItem = columnMetadata[index];
    BOOST_CHECK_EQUAL(index, columnMetadataItem.GetOrdinal());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsAutoIncrement());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsCaseSensitive());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsSearchable());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsCurrency());
    BOOST_CHECK_EQUAL(SQL_NULLABLE, columnMetadataItem.GetNullable());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsSigned());
    BOOST_CHECK_EQUAL(0, columnMetadataItem.GetColumnDisplaySize());
    BOOST_CHECK_EQUAL("fieldDate", *columnMetadataItem.GetColumnLabel());
    BOOST_CHECK_EQUAL("fieldDate", *columnMetadataItem.GetColumnName());
    BOOST_CHECK_EQUAL(DATABASE_NAME, *columnMetadataItem.GetSchemaName());
    BOOST_CHECK_EQUAL(0, columnMetadataItem.GetPrecision());
    BOOST_CHECK_EQUAL(0, columnMetadataItem.GetScale());
    BOOST_CHECK_EQUAL(TABLE_NAME, *columnMetadataItem.GetTableName());
    BOOST_CHECK(!columnMetadataItem.GetCatalogName());
    BOOST_CHECK_EQUAL(JDBC_TYPE_TIMESTAMP, columnMetadataItem.GetColumnType());
    BOOST_CHECK_EQUAL("TIMESTAMP", *columnMetadataItem.GetColumnTypeName());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsReadOnly());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsWritable());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsDefinitelyWritable());
    BOOST_CHECK_EQUAL("java.sql.Timestamp",
                      *columnMetadataItem.GetColumnClassName());

    index = 7;
    columnMetadataItem = columnMetadata[index];
    BOOST_CHECK_EQUAL(index, columnMetadataItem.GetOrdinal());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsAutoIncrement());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsCaseSensitive());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsSearchable());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsCurrency());
    BOOST_CHECK_EQUAL(SQL_NULLABLE, columnMetadataItem.GetNullable());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsSigned());
    BOOST_CHECK_EQUAL(10, columnMetadataItem.GetColumnDisplaySize());
    BOOST_CHECK_EQUAL("fieldInt", *columnMetadataItem.GetColumnLabel());
    BOOST_CHECK_EQUAL("fieldInt", *columnMetadataItem.GetColumnName());
    BOOST_CHECK_EQUAL(DATABASE_NAME, *columnMetadataItem.GetSchemaName());
    BOOST_CHECK_EQUAL(10, columnMetadataItem.GetPrecision());
    BOOST_CHECK_EQUAL(0, columnMetadataItem.GetScale());
    BOOST_CHECK_EQUAL(TABLE_NAME, *columnMetadataItem.GetTableName());
    BOOST_CHECK(!columnMetadataItem.GetCatalogName());
    BOOST_CHECK_EQUAL(JDBC_TYPE_INTEGER, columnMetadataItem.GetColumnType());
    BOOST_CHECK_EQUAL("INTEGER", *columnMetadataItem.GetColumnTypeName());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsReadOnly());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsWritable());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsDefinitelyWritable());
    BOOST_CHECK_EQUAL("java.lang.Integer",
                      *columnMetadataItem.GetColumnClassName());

    index = 8;
    columnMetadataItem = columnMetadata[index];
    BOOST_CHECK_EQUAL(index, columnMetadataItem.GetOrdinal());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsAutoIncrement());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsCaseSensitive());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsSearchable());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsCurrency());
    BOOST_CHECK_EQUAL(SQL_NULLABLE, columnMetadataItem.GetNullable());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsSigned());
    BOOST_CHECK_EQUAL(19, columnMetadataItem.GetColumnDisplaySize());
    BOOST_CHECK_EQUAL("fieldLong", *columnMetadataItem.GetColumnLabel());
    BOOST_CHECK_EQUAL("fieldLong", *columnMetadataItem.GetColumnName());
    BOOST_CHECK_EQUAL(DATABASE_NAME, *columnMetadataItem.GetSchemaName());
    BOOST_CHECK_EQUAL(19, columnMetadataItem.GetPrecision());
    BOOST_CHECK_EQUAL(0, columnMetadataItem.GetScale());
    BOOST_CHECK_EQUAL(TABLE_NAME, *columnMetadataItem.GetTableName());
    BOOST_CHECK(!columnMetadataItem.GetCatalogName());
    BOOST_CHECK_EQUAL(JDBC_TYPE_BIGINT, columnMetadataItem.GetColumnType());
    BOOST_CHECK_EQUAL("BIGINT", *columnMetadataItem.GetColumnTypeName());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsReadOnly());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsWritable());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsDefinitelyWritable());
    BOOST_CHECK_EQUAL("java.lang.Long",
                      *columnMetadataItem.GetColumnClassName());

    index = 9;
    columnMetadataItem = columnMetadata[index];
    BOOST_CHECK_EQUAL(index, columnMetadataItem.GetOrdinal());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsAutoIncrement());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsCaseSensitive());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsSearchable());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsCurrency());
    BOOST_CHECK_EQUAL(SQL_NULLABLE, columnMetadataItem.GetNullable());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsSigned());
    BOOST_CHECK_EQUAL(65536, columnMetadataItem.GetColumnDisplaySize());
    BOOST_CHECK_EQUAL("fieldMaxKey", *columnMetadataItem.GetColumnLabel());
    BOOST_CHECK_EQUAL("fieldMaxKey", *columnMetadataItem.GetColumnName());
    BOOST_CHECK_EQUAL(DATABASE_NAME, *columnMetadataItem.GetSchemaName());
    BOOST_CHECK_EQUAL(65536, columnMetadataItem.GetPrecision());
    BOOST_CHECK_EQUAL(0, columnMetadataItem.GetScale());
    BOOST_CHECK_EQUAL(TABLE_NAME, *columnMetadataItem.GetTableName());
    BOOST_CHECK(!columnMetadataItem.GetCatalogName());
    BOOST_CHECK_EQUAL(JDBC_TYPE_VARCHAR, columnMetadataItem.GetColumnType());
    BOOST_CHECK_EQUAL("VARCHAR", *columnMetadataItem.GetColumnTypeName());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsReadOnly());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsWritable());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsDefinitelyWritable());
    BOOST_CHECK_EQUAL("java.lang.String",
                      *columnMetadataItem.GetColumnClassName());

    index = 10;
    columnMetadataItem = columnMetadata[index];
    BOOST_CHECK_EQUAL(index, columnMetadataItem.GetOrdinal());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsAutoIncrement());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsCaseSensitive());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsSearchable());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsCurrency());
    BOOST_CHECK_EQUAL(SQL_NULLABLE, columnMetadataItem.GetNullable());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsSigned());
    BOOST_CHECK_EQUAL(65536, columnMetadataItem.GetColumnDisplaySize());
    BOOST_CHECK_EQUAL("fieldMinKey", *columnMetadataItem.GetColumnLabel());
    BOOST_CHECK_EQUAL("fieldMinKey", *columnMetadataItem.GetColumnName());
    BOOST_CHECK_EQUAL(DATABASE_NAME, *columnMetadataItem.GetSchemaName());
    BOOST_CHECK_EQUAL(65536, columnMetadataItem.GetPrecision());
    BOOST_CHECK_EQUAL(0, columnMetadataItem.GetScale());
    BOOST_CHECK_EQUAL(TABLE_NAME, *columnMetadataItem.GetTableName());
    BOOST_CHECK(!columnMetadataItem.GetCatalogName());
    BOOST_CHECK_EQUAL(JDBC_TYPE_VARCHAR, columnMetadataItem.GetColumnType());
    BOOST_CHECK_EQUAL("VARCHAR", *columnMetadataItem.GetColumnTypeName());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsReadOnly());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsWritable());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsDefinitelyWritable());
    BOOST_CHECK_EQUAL("java.lang.String",
                      *columnMetadataItem.GetColumnClassName());

    index = 11;
    columnMetadataItem = columnMetadata[index];
    BOOST_CHECK_EQUAL(index, columnMetadataItem.GetOrdinal());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsAutoIncrement());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsCaseSensitive());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsSearchable());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsCurrency());
    BOOST_CHECK_EQUAL(SQL_NULLABLE, columnMetadataItem.GetNullable());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsSigned());
    // This seems odd
    BOOST_CHECK_EQUAL(-1, columnMetadataItem.GetColumnDisplaySize());
    BOOST_CHECK_EQUAL("fieldNull", *columnMetadataItem.GetColumnLabel());
    BOOST_CHECK_EQUAL("fieldNull", *columnMetadataItem.GetColumnName());
    BOOST_CHECK_EQUAL(DATABASE_NAME, *columnMetadataItem.GetSchemaName());
    BOOST_CHECK_EQUAL(0, columnMetadataItem.GetPrecision());
    BOOST_CHECK_EQUAL(0, columnMetadataItem.GetScale());
    BOOST_CHECK_EQUAL(TABLE_NAME, *columnMetadataItem.GetTableName());
    BOOST_CHECK(!columnMetadataItem.GetCatalogName());
    BOOST_CHECK_EQUAL(JDBC_TYPE_VARCHAR, columnMetadataItem.GetColumnType());
    BOOST_CHECK_EQUAL("VARCHAR", *columnMetadataItem.GetColumnTypeName());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsReadOnly());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsWritable());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsDefinitelyWritable());
    BOOST_CHECK_EQUAL("java.lang.String",
                      *columnMetadataItem.GetColumnClassName());

    index = 12;
    columnMetadataItem = columnMetadata[index];
    BOOST_CHECK_EQUAL(index, columnMetadataItem.GetOrdinal());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsAutoIncrement());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsCaseSensitive());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsSearchable());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsCurrency());
    BOOST_CHECK_EQUAL(SQL_NULLABLE, columnMetadataItem.GetNullable());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsSigned());
    // This seems odd
    BOOST_CHECK_EQUAL(65536, columnMetadataItem.GetColumnDisplaySize());
    BOOST_CHECK_EQUAL("fieldBinary", *columnMetadataItem.GetColumnLabel());
    BOOST_CHECK_EQUAL("fieldBinary", *columnMetadataItem.GetColumnName());
    BOOST_CHECK_EQUAL(DATABASE_NAME, *columnMetadataItem.GetSchemaName());
    BOOST_CHECK_EQUAL(65536, columnMetadataItem.GetPrecision());
    BOOST_CHECK_EQUAL(0, columnMetadataItem.GetScale());
    BOOST_CHECK_EQUAL(TABLE_NAME, *columnMetadataItem.GetTableName());
    BOOST_CHECK(!columnMetadataItem.GetCatalogName());
    BOOST_CHECK_EQUAL(JDBC_TYPE_VARBINARY, columnMetadataItem.GetColumnType());
    BOOST_CHECK_EQUAL("VARBINARY", *columnMetadataItem.GetColumnTypeName());
    BOOST_CHECK_EQUAL(true, columnMetadataItem.IsReadOnly());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsWritable());
    BOOST_CHECK_EQUAL(false, columnMetadataItem.IsDefinitelyWritable());
    BOOST_CHECK_EQUAL("[B", *columnMetadataItem.GetColumnClassName());
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
  CreateDsnConnectionStringForLocalServer(dsnConnectionString);

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
  CreateDsnConnectionStringForLocalServer(dsnConnectionString);

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
  CreateDsnConnectionStringForLocalServer(dsnConnectionString);

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

  boost::optional< std::string > catalog;
  boost::optional< std::string > schemaPattern;
  std::string tablePattern;
  boost::optional< std::vector< std::string > > types(
      std::vector< std::string >({"TABLE"}));
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

    boost::optional< std::string > value;

    resultSet.Get()->GetString(1, value, errInfo);
    BOOST_CHECK(!value);
    resultSet.Get()->GetString("TABLE_CAT", value, errInfo);
    BOOST_CHECK(!value);

    resultSet.Get()->GetString(2, value, errInfo);
    BOOST_CHECK(value);
    resultSet.Get()->GetString("TABLE_SCHEM", value, errInfo);
    BOOST_CHECK(value);
    BOOST_CHECK_EQUAL(DATABASE_NAME, *value);

    resultSet.Get()->GetString(3, value, errInfo);
    BOOST_CHECK(value);
    BOOST_CHECK(!value->empty());
    resultSet.Get()->GetString("TABLE_NAME", value, errInfo);
    BOOST_CHECK(value);
    BOOST_CHECK(!value->empty());

    resultSet.Get()->GetString(4, value, errInfo);
    BOOST_CHECK(value);
    BOOST_CHECK("TABLE" == *value);
    resultSet.Get()->GetString("TABLE_TYPE", value, errInfo);
    BOOST_CHECK(value);
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
  CreateDsnConnectionStringForLocalServer(dsnConnectionString);

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

  boost::optional< std::string > catalog;
  boost::optional< std::string > schemaPattern = DATABASE_NAME;
  std::string tablePattern = "jni_test_001";
  std::string columnNamePattern = "%";
  SharedPointer< ResultSet > resultSet = databaseMetaData.Get()->GetColumns(
      catalog, schemaPattern, tablePattern, columnNamePattern, errInfo);
  BOOST_CHECK(resultSet.IsValid());
  BOOST_CHECK(resultSet.Get()->IsOpen());

  int columnIndex = 0;
  // Loop the result set records
  bool hasNext = false;
  boost::optional< std::string > value;
  std::string columnName;
  boost::optional< int > intValue;
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
    BOOST_CHECK_EQUAL(DATABASE_NAME, *value);
    resultSet.Get()->GetString("TABLE_SCHEM", value, errInfo);

    BOOST_CHECK(value);
    BOOST_CHECK_EQUAL(DATABASE_NAME, *value);

    resultSet.Get()->GetString(3, value, errInfo);

    BOOST_CHECK(value);
    BOOST_CHECK_EQUAL(TABLE_NAME, *value);
    resultSet.Get()->GetString("TABLE_NAME", value, errInfo);

    BOOST_CHECK(value);
    BOOST_CHECK_EQUAL(TABLE_NAME, *value);

    resultSet.Get()->GetString(4, value, errInfo);

    BOOST_CHECK(value);
    BOOST_CHECK(!value->empty());
    resultSet.Get()->GetString("COLUMN_NAME", value, errInfo);

    BOOST_CHECK(value);
    BOOST_CHECK(!value->empty());
    switch (columnIndex) {
      case 1:
        BOOST_CHECK_EQUAL(TABLE_ID_COLUMN_NAME, *value);
        break;
      case 2:
        BOOST_CHECK_EQUAL("fieldDecimal128", *value);
        break;
      case 3:
        BOOST_CHECK_EQUAL("fieldDouble", *value);
        break;
      case 4:
        BOOST_CHECK_EQUAL("fieldString", *value);
        break;
      case 5:
        BOOST_CHECK_EQUAL("fieldObjectId", *value);
        break;
      case 6:
        BOOST_CHECK_EQUAL("fieldBoolean", *value);
        break;
      case 7:
        BOOST_CHECK_EQUAL("fieldDate", *value);
        break;
      case 8:
        BOOST_CHECK_EQUAL("fieldInt", *value);
        break;
      case 9:
        BOOST_CHECK_EQUAL("fieldLong", *value);
        break;
      case 10:
        BOOST_CHECK_EQUAL("fieldMaxKey", *value);
        break;
      case 11:
        BOOST_CHECK_EQUAL("fieldMinKey", *value);
        break;
      case 12:
        BOOST_CHECK_EQUAL("fieldNull", *value);
        break;
      case 13:
        BOOST_CHECK_EQUAL("fieldBinary", *value);
        break;
      default:
        BOOST_FAIL("Unexpected column index.");
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
        BOOST_CHECK_EQUAL(JDBC_TYPE_DECIMAL, *intValue);
        break;
      case 3:
        BOOST_CHECK_EQUAL(JDBC_TYPE_DOUBLE, *intValue);
        break;
      case 4:
        BOOST_CHECK_EQUAL(JDBC_TYPE_VARCHAR, *intValue);
        break;
      case 5:
        BOOST_CHECK_EQUAL(JDBC_TYPE_VARCHAR, *intValue);
        break;
      case 6:
        BOOST_CHECK_EQUAL(JDBC_TYPE_BOOLEAN, *intValue);
        break;
      case 7:
        BOOST_CHECK_EQUAL(JDBC_TYPE_TIMESTAMP, *intValue);
        break;
      case 8:
        BOOST_CHECK_EQUAL(JDBC_TYPE_INTEGER, *intValue);
        break;
      case 9:
        BOOST_CHECK_EQUAL(JDBC_TYPE_BIGINT, *intValue);
        break;
      case 10:
        BOOST_CHECK_EQUAL(JDBC_TYPE_VARCHAR, *intValue);
        break;
      case 11:
        BOOST_CHECK_EQUAL(JDBC_TYPE_VARCHAR, *intValue);
        break;
      case 12:
        BOOST_CHECK_EQUAL(JDBC_TYPE_NULL, *intValue);
        break;
      case 13:
        BOOST_CHECK_EQUAL(JDBC_TYPE_VARBINARY, *intValue);
        break;
      default:
        BOOST_FAIL("Unexpected column index.");
        break;
    }

    resultSet.Get()->GetString(6, value, errInfo);
    BOOST_CHECK(value);
    resultSet.Get()->GetString("TYPE_NAME", value, errInfo);
    BOOST_CHECK(value);

    resultSet.Get()->GetInt(7, intValue, errInfo);
    resultSet.Get()->GetInt("COLUMN_SIZE", intValue, errInfo);
    BOOST_CHECK(intValue);
    switch (columnIndex) {
      case 1:
        BOOST_CHECK_EQUAL(65536, *intValue);
        break;
      case 2:
        BOOST_CHECK_EQUAL(646456995, *intValue);
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
    resultSet.Get()->GetInt("ORDINAL_POSITION", intValue, errInfo);
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
  BOOST_CHECK_EQUAL(13, columnIndex);

  if (resultSet.Get()->Close(errInfo) != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }
  BOOST_CHECK(!resultSet.Get()->IsOpen());

  if (dbConnection.Close(errInfo) != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }
  BOOST_CHECK(!dbConnection.IsOpen());
}

BOOST_AUTO_TEST_CASE(TestDocumentDbDatabaseMetaDataGetPrimaryKeys) {
  PrepareContext();
  BOOST_REQUIRE(_ctx.Get() != nullptr);

  std::string dsnConnectionString;
  CreateDsnConnectionStringForLocalServer(dsnConnectionString);

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

  boost::optional< std::string > catalog = boost::none;
  boost::optional< std::string > schema = boost::none;
  boost::optional< std::string > table = TABLE_NAME;
  SharedPointer< ResultSet > resultSet =
      databaseMetaData.Get()->GetPrimaryKeys(catalog, schema, table, errInfo);
  BOOST_CHECK(resultSet.IsValid());
  BOOST_CHECK(resultSet.Get()->IsOpen());

  int columnIndex = 0;
  // Loop the result set records
  bool hasNext = false;
  boost::optional< std::string > value;
  std::string columnName;
  boost::optional< int16_t > smallIntValue;
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
    resultSet.Get()->GetString("TABLE_SCHEM", value, errInfo);

    BOOST_CHECK(value);

    resultSet.Get()->GetString(3, value, errInfo);
    BOOST_CHECK(value);
    BOOST_CHECK_EQUAL(TABLE_NAME, *value);

    resultSet.Get()->GetString("TABLE_NAME", value, errInfo);
    BOOST_CHECK(value);
    BOOST_CHECK_EQUAL(TABLE_NAME, *value);

    resultSet.Get()->GetString(4, value, errInfo);

    BOOST_CHECK(value);
    BOOST_CHECK(!value->empty());
    resultSet.Get()->GetString("COLUMN_NAME", value, errInfo);

    BOOST_CHECK(value);
    BOOST_CHECK_EQUAL(TABLE_ID_COLUMN_NAME, *value);

    resultSet.Get()->GetSmallInt(5, smallIntValue, errInfo);
    BOOST_CHECK(smallIntValue);

    resultSet.Get()->GetSmallInt("KEY_SEQ", smallIntValue, errInfo);
    BOOST_CHECK(smallIntValue);

    resultSet.Get()->GetString(6, value, errInfo);
    BOOST_CHECK(!value);

    resultSet.Get()->GetString("PK_NAME", value, errInfo);
    BOOST_CHECK(!value);

  } while (hasNext);
  BOOST_CHECK_EQUAL(1, columnIndex);

  if (resultSet.Get()->Close(errInfo) != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }
  BOOST_CHECK(!resultSet.Get()->IsOpen());

  if (dbConnection.Close(errInfo) != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }
  BOOST_CHECK(!dbConnection.IsOpen());
}

BOOST_AUTO_TEST_CASE(TestDocumentDbDatabaseMetaDataGetImportedKeys) {
  PrepareContext();
  BOOST_REQUIRE(_ctx.Get() != nullptr);

  std::string dsnConnectionString;
  CreateDsnConnectionStringForLocalServer(dsnConnectionString);

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

  const boost::optional< std::string > fkCatalog = boost::none;
  const boost::optional< std::string > fkSchema = boost::none;
  std::string fkTable = "jni_test_001_sub_doc";
  SharedPointer< ResultSet > resultSet =
      databaseMetaData.Get()->GetImportedKeys(fkCatalog, fkSchema, fkTable,
                                              errInfo);
  BOOST_CHECK(resultSet.IsValid());
  BOOST_CHECK(resultSet.Get()->IsOpen());

  int columnIndex = 0;
  // Loop the result set records
  bool hasNext = false;
  boost::optional< std::string > value;
  std::string table = "jni_test_001_sub";
  std::string fkColumn = "jni_test_001_sub__id";
  boost::optional< int16_t > smallIntValue;
  do {
    resultSet.Get()->Next(hasNext, errInfo);
    if (!hasNext) {
      break;
    }
    columnIndex++;

    resultSet.Get()->GetString(1, value, errInfo);
    BOOST_CHECK(!value);

    resultSet.Get()->GetString("PKTABLE_CAT", value, errInfo);
    BOOST_CHECK(!value);

    resultSet.Get()->GetString(2, value, errInfo);
    BOOST_CHECK(value);
    BOOST_CHECK_EQUAL(DATABASE_NAME, *value);

    resultSet.Get()->GetString("PKTABLE_SCHEM", value, errInfo);
    BOOST_CHECK(value);
    BOOST_CHECK_EQUAL(DATABASE_NAME, *value);

    resultSet.Get()->GetString(3, value, errInfo);
    BOOST_CHECK(value);
    BOOST_CHECK_EQUAL(table, *value);

    resultSet.Get()->GetString("PKTABLE_NAME", value, errInfo);
    BOOST_CHECK(value);
    BOOST_CHECK_EQUAL(table, *value);

    resultSet.Get()->GetString(4, value, errInfo);
    BOOST_CHECK(value);
    BOOST_CHECK(!value->empty());
    BOOST_CHECK_EQUAL(fkColumn, *value);

    resultSet.Get()->GetString("PKCOLUMN_NAME", value, errInfo);
    BOOST_CHECK(value);
    BOOST_CHECK(!value->empty());
    BOOST_CHECK_EQUAL(fkColumn, *value);

    resultSet.Get()->GetString(5, value, errInfo);
    BOOST_CHECK(!value);

    resultSet.Get()->GetString("FKTABLE_CAT", value, errInfo);
    BOOST_CHECK(!value);

    resultSet.Get()->GetString(6, value, errInfo);
    BOOST_CHECK(value);
    BOOST_CHECK_EQUAL(DATABASE_NAME, *value);

    resultSet.Get()->GetString("FKTABLE_SCHEM", value, errInfo);
    BOOST_CHECK(value);
    BOOST_CHECK_EQUAL(DATABASE_NAME, *value);

    resultSet.Get()->GetString(7, value, errInfo);
    BOOST_CHECK(value);
    BOOST_CHECK_EQUAL(fkTable, *value);

    resultSet.Get()->GetString("FKTABLE_NAME", value, errInfo);
    BOOST_CHECK(value);
    BOOST_CHECK_EQUAL(fkTable, *value);

    resultSet.Get()->GetString(8, value, errInfo);
    BOOST_CHECK(value);
    BOOST_CHECK(!value->empty());
    BOOST_CHECK_EQUAL(fkColumn, *value);

    resultSet.Get()->GetString("FKCOLUMN_NAME", value, errInfo);
    BOOST_CHECK(value);
    BOOST_CHECK(!value->empty());
    BOOST_CHECK_EQUAL(fkColumn, *value);

    resultSet.Get()->GetSmallInt(9, smallIntValue, errInfo);
    BOOST_CHECK(smallIntValue);
    BOOST_CHECK_EQUAL(1, *smallIntValue);

    resultSet.Get()->GetSmallInt("KEY_SEQ", smallIntValue, errInfo);
    BOOST_CHECK(smallIntValue);
    BOOST_CHECK_EQUAL(1, *smallIntValue);

    resultSet.Get()->GetSmallInt(10, smallIntValue, errInfo);
    BOOST_CHECK(smallIntValue);

    resultSet.Get()->GetSmallInt("UPDATE_RULE", smallIntValue, errInfo);
    BOOST_CHECK(smallIntValue);

    resultSet.Get()->GetSmallInt(11, smallIntValue, errInfo);
    BOOST_CHECK(smallIntValue);
    resultSet.Get()->GetSmallInt("DELETE_RULE", smallIntValue, errInfo);
    BOOST_CHECK(smallIntValue);

    resultSet.Get()->GetString(12, value, errInfo);
    BOOST_CHECK(!value);
    resultSet.Get()->GetString("FK_NAME", value, errInfo);
    BOOST_CHECK(!value);

    resultSet.Get()->GetString(13, value, errInfo);
    BOOST_CHECK(!value);
    resultSet.Get()->GetString("PK_NAME", value, errInfo);
    BOOST_CHECK(!value);

    resultSet.Get()->GetSmallInt(14, smallIntValue, errInfo);
    BOOST_CHECK(smallIntValue);
    resultSet.Get()->GetSmallInt("DEFERRABILITY", smallIntValue, errInfo);
    BOOST_CHECK(smallIntValue);
  } while (hasNext);
  BOOST_CHECK_EQUAL(1, columnIndex);

  if (resultSet.Get()->Close(errInfo) != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }
  BOOST_CHECK(!resultSet.Get()->IsOpen());

  if (dbConnection.Close(errInfo) != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }
  BOOST_CHECK(!dbConnection.IsOpen());
}

BOOST_AUTO_TEST_CASE(TestDocumentDbDatabaseMetaDataGetPrimaryKeysReturnsNone) {
  PrepareContext();
  BOOST_REQUIRE(_ctx.Get() != nullptr);

  std::string dsnConnectionString;
  CreateDsnConnectionStringForLocalServer(dsnConnectionString);

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

  boost::optional< std::string > catalog("");
  boost::optional< std::string > schema("");
  boost::optional< std::string > table("");
  SharedPointer< ResultSet > resultSet =
      databaseMetaData.Get()->GetPrimaryKeys(catalog, schema, table, errInfo);
  BOOST_CHECK(resultSet.IsValid());
  BOOST_CHECK(resultSet.Get()->IsOpen());

  bool hasNext = false;
  resultSet.Get()->Next(hasNext, errInfo);
  BOOST_CHECK(!hasNext);

  if (resultSet.Get()->Close(errInfo) != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }
  BOOST_CHECK(!resultSet.Get()->IsOpen());

  if (dbConnection.Close(errInfo) != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    BOOST_FAIL(errInfo.errMsg);
  }
  BOOST_CHECK(!dbConnection.IsOpen());
}

BOOST_AUTO_TEST_CASE(TestDocumentDbDatabaseMetaDataGetImportedKeysReturnsNone) {
  PrepareContext();
  BOOST_REQUIRE(_ctx.Get() != nullptr);

  std::string dsnConnectionString;
  CreateDsnConnectionStringForLocalServer(dsnConnectionString);

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

  const boost::optional< std::string > fkCatalog("");
  const boost::optional< std::string > fkSchema("");
  std::string fkTable("");
  SharedPointer< ResultSet > resultSet =
      databaseMetaData.Get()->GetImportedKeys(fkCatalog, fkSchema, fkTable,
                                              errInfo);
  BOOST_CHECK(resultSet.IsValid());
  BOOST_CHECK(resultSet.Get()->IsOpen());

  bool hasNext = false;
  resultSet.Get()->Next(hasNext, errInfo);
  BOOST_CHECK(!hasNext);

  if (resultSet.Get()->Close(errInfo) != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
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
  CreateDsnConnectionStringForLocalServer(dsnConnectionString);

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
          "SELECT * FROM \"" + TABLE_NAME + "\"", 0, errInfo);
  BOOST_CHECK(JniErrorCode::IGNITE_JNI_ERR_SUCCESS == errInfo.code);
  BOOST_REQUIRE(queryContext.IsValid());
  std::vector< JdbcColumnMetadata > columnMetadata =
      queryContext.Get()->GetColumnMetadata();
  ValidateJdbcColumnMetadata(columnMetadata);
  BOOST_CHECK_EQUAL(TABLE_NAME, queryContext.Get()->GetCollectionName());
  BOOST_CHECK_EQUAL(13, queryContext.Get()->GetPaths().size());
  BOOST_CHECK_EQUAL(1, queryContext.Get()->GetAggregateOperations().size());
  BOOST_CHECK_EQUAL(
      "{\"$project\": {"
      "\"" +  TABLE_ID_COLUMN_NAME + "\": \"$_id\", "
      "\"fieldDecimal128\": \"$fieldDecimal128\", "
      "\"fieldDouble\": \"$fieldDouble\", "
      "\"fieldString\": \"$fieldString\", "
      "\"fieldObjectId\": \"$fieldObjectId\", "
      "\"fieldBoolean\": \"$fieldBoolean\", "
      "\"fieldDate\": \"$fieldDate\", "
      "\"fieldInt\": \"$fieldInt\", "
      "\"fieldLong\": \"$fieldLong\", "
      "\"fieldMaxKey\": \"$fieldMaxKey\", "
      "\"fieldMinKey\": \"$fieldMinKey\", "
      "\"fieldNull\": \"$fieldNull\", "
      "\"fieldBinary\": \"$fieldBinary\", "
      "\"_id\": {\"$numberInt\": \"0\"}}}",
      queryContext.Get()->GetAggregateOperations()[0]);

  // Test invalid table name will produce error.
  SharedPointer< DocumentDbMqlQueryContext > queryContext2 =
      queryMappingService.Get()->GetMqlQueryContext(
          "SELECT * FROM \"jni_test_001_xxx\"", 0, errInfo);
  BOOST_CHECK(JniErrorCode::IGNITE_JNI_ERR_SUCCESS != errInfo.code);
  std::string modErrMsg = errInfo.errMsg;
  boost::erase_all(modErrMsg, "\r");
  boost::erase_all(modErrMsg, "\n");
  BOOST_CHECK(std::regex_match(
      modErrMsg, std::regex("^Unable to parse SQL.*"
                            "Object 'jni_test_001_xxx' not found'$")));
  BOOST_CHECK(!queryContext2.IsValid());
}

BOOST_AUTO_TEST_SUITE_END()
