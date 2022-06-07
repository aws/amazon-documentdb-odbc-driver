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

#include <sqlext.h>

#include <boost/test/unit_test.hpp>
#include <utility>

#include "ignite/odbc/impl/binary/binary_common.h"
#include "ignite/odbc/meta/column_meta.h"
#include "ignite/odbc/type_traits.h"
#include "odbc_test_suite.h"

using namespace ignite::odbc::impl::binary;
using ignite::odbc::OdbcTestSuite;
using ignite::odbc::meta::ColumnMeta;
using ignite::odbc::meta::Nullability;

BOOST_AUTO_TEST_CASE(TestGetAttribute) {
  // Only SQL_DESC_* fields are tested in this test.
  // This is because those are the fields that would be passed to
  // SQLColAttribute function.
  using namespace ignite::odbc::type_traits;

  std::string schema("database");
  std::string table("table");
  std::string column("column");

  ColumnMeta columnMeta(schema, table, column, JDBC_TYPE_VARCHAR,
                        Nullability::NULLABLE);

  SQLLEN intVal;
  std::string resVal;
  bool found;

  // test retrieving std::string value

  // test SQL_DESC_LABEL
  found = columnMeta.GetAttribute(SQL_DESC_LABEL, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, column);

  // test SQL_DESC_BASE_COLUMN_NAME
  found = columnMeta.GetAttribute(SQL_DESC_BASE_COLUMN_NAME, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, column);

  // test SQL_DESC_NAME
  found = columnMeta.GetAttribute(SQL_DESC_NAME, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, column);

  // test SQL_DESC_TABLE_NAME
  found = columnMeta.GetAttribute(SQL_DESC_TABLE_NAME, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, table);

  // test SQL_DESC_BASE_TABLE_NAME
  found = columnMeta.GetAttribute(SQL_DESC_BASE_TABLE_NAME, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, table);

  // test SQL_DESC_SCHEMA_NAME
  found = columnMeta.GetAttribute(SQL_DESC_SCHEMA_NAME, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, schema);

  // test SQL_DESC_CATALOG_NAME
  found = columnMeta.GetAttribute(SQL_DESC_CATALOG_NAME, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, "");

  // test SQL_DESC_LITERAL_PREFIX
  found = columnMeta.GetAttribute(SQL_DESC_LITERAL_PREFIX, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, "'");

  // test SQL_DESC_LITERAL_SUFFIX
  found = columnMeta.GetAttribute(SQL_DESC_LITERAL_SUFFIX, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, "'");

  // test SQL_DESC_TYPE_NAME
  found = columnMeta.GetAttribute(SQL_DESC_TYPE_NAME, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, SqlTypeName::VARCHAR);

  // test SQL_DESC_LOCAL_TYPE_NAME
  found = columnMeta.GetAttribute(SQL_DESC_LOCAL_TYPE_NAME, resVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(resVal, SqlTypeName::VARCHAR);

  // fields SQL_COLUMN_PRECISION and SQL_DESC_SCALE are not tested
  // for retrieving string values

  // test retrieving SQLLEN value

  // test SQL_DESC_FIXED_PREC_SCALE
  found = columnMeta.GetAttribute(SQL_DESC_FIXED_PREC_SCALE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_FALSE);

  // test SQL_DESC_AUTO_UNIQUE_VALUE
  found = columnMeta.GetAttribute(SQL_DESC_AUTO_UNIQUE_VALUE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_FALSE);

  // test SQL_DESC_CASE_SENSITIVE
  found = columnMeta.GetAttribute(SQL_DESC_CASE_SENSITIVE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_TRUE);

  // test SQL_DESC_CONCISE_TYPE
  found = columnMeta.GetAttribute(SQL_DESC_CONCISE_TYPE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_VARCHAR);

  // test SQL_DESC_TYPE
  found = columnMeta.GetAttribute(SQL_DESC_TYPE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_VARCHAR);

  // test SQL_DESC_DISPLAY_SIZE
  found = columnMeta.GetAttribute(SQL_DESC_DISPLAY_SIZE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_NO_TOTAL);

  // test SQL_DESC_LENGTH
  found = columnMeta.GetAttribute(SQL_DESC_LENGTH, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_NO_TOTAL);

  // test SQL_DESC_OCTET_LENGTH
  found = columnMeta.GetAttribute(SQL_DESC_OCTET_LENGTH, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_NO_TOTAL);

  // test SQL_DESC_NULLABLE
  found = columnMeta.GetAttribute(SQL_DESC_NULLABLE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_NULLABLE);

  // test SQL_DESC_NUM_PREC_RADIX
  found = columnMeta.GetAttribute(SQL_DESC_NUM_PREC_RADIX, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, 0);

  // test SQL_DESC_PRECISION
  found = columnMeta.GetAttribute(SQL_DESC_PRECISION, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_NO_TOTAL);

  // test SQL_DESC_SCALE
  found = columnMeta.GetAttribute(SQL_DESC_SCALE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, -1);

  // test SQL_DESC_SEARCHABLE
  found = columnMeta.GetAttribute(SQL_DESC_SEARCHABLE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_PRED_BASIC);

  // test SQL_DESC_UNNAMED
  found = columnMeta.GetAttribute(SQL_DESC_UNNAMED, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_NAMED);

  // test SQL_DESC_UNSIGNED
  found = columnMeta.GetAttribute(SQL_DESC_UNSIGNED, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_TRUE);

  // test SQL_DESC_UPDATABLE
  found = columnMeta.GetAttribute(SQL_DESC_UPDATABLE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_ATTR_READWRITE_UNKNOWN);
}

BOOST_AUTO_TEST_CASE(TestGetAttributeLiteralPrefix) {
  std::string schema("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::pair< int16_t, std::string > tests[] = {
      std::make_pair(JDBC_TYPE_VARCHAR, std::string("'")),
      std::make_pair(JDBC_TYPE_CHAR, std::string("'")),
      std::make_pair(JDBC_TYPE_NCHAR, std::string("'")),
      std::make_pair(JDBC_TYPE_NVARCHAR, std::string("'")),
      std::make_pair(JDBC_TYPE_LONGVARCHAR, std::string("'")),
      std::make_pair(JDBC_TYPE_LONGNVARCHAR, std::string("'")),
      std::make_pair(JDBC_TYPE_BINARY, std::string("0x")),
      std::make_pair(JDBC_TYPE_VARBINARY, std::string("0x")),
      std::make_pair(JDBC_TYPE_LONGVARBINARY, std::string("0x")),
      std::make_pair(JDBC_TYPE_BIGINT, std::string("")),
      std::make_pair(JDBC_TYPE_BOOLEAN, std::string("")),
      std::make_pair(JDBC_TYPE_FLOAT, std::string(""))};

  int numTests = sizeof(tests) / sizeof(std::pair< int16_t, std::string >);

  for (int i = 0; i < numTests; i++) {
    ColumnMeta columnMeta(schema, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving std::string value

    // test SQL_DESC_LITERAL_PREFIX
    found = columnMeta.GetAttribute(SQL_DESC_LITERAL_PREFIX, resVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(resVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeLiteralSuffix) {
  std::string schema("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::pair< int16_t, std::string > tests[] = {
      std::make_pair(JDBC_TYPE_VARCHAR, std::string("'")),
      std::make_pair(JDBC_TYPE_CHAR, std::string("'")),
      std::make_pair(JDBC_TYPE_NCHAR, std::string("'")),
      std::make_pair(JDBC_TYPE_NVARCHAR, std::string("'")),
      std::make_pair(JDBC_TYPE_LONGVARCHAR, std::string("'")),
      std::make_pair(JDBC_TYPE_LONGNVARCHAR, std::string("'")),
      std::make_pair(JDBC_TYPE_BINARY, std::string("")),
      std::make_pair(JDBC_TYPE_VARBINARY, std::string("")),
      std::make_pair(JDBC_TYPE_LONGVARBINARY, std::string("")),
      std::make_pair(JDBC_TYPE_BIGINT, std::string("")),
      std::make_pair(JDBC_TYPE_BOOLEAN, std::string("")),
      std::make_pair(JDBC_TYPE_FLOAT, std::string(""))};

  int numTests = sizeof(tests) / sizeof(std::pair< int16_t, std::string >);

  for (int i = 0; i < numTests; i++) {
    ColumnMeta columnMeta(schema, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving std::string value

    // test SQL_DESC_LITERAL_SUFFIX
    found = columnMeta.GetAttribute(SQL_DESC_LITERAL_SUFFIX, resVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(resVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeLocalTypeName) {
  using namespace ignite::odbc::type_traits;

  std::string schema("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::pair< int16_t, std::string > tests[] = {
      std::make_pair(JDBC_TYPE_BOOLEAN, SqlTypeName::BIT),
      std::make_pair(JDBC_TYPE_SMALLINT, SqlTypeName::SMALLINT),
      std::make_pair(JDBC_TYPE_TINYINT, SqlTypeName::TINYINT),
      std::make_pair(JDBC_TYPE_INTEGER, SqlTypeName::INTEGER),
      std::make_pair(JDBC_TYPE_BIGINT, SqlTypeName::BIGINT),
      std::make_pair(JDBC_TYPE_FLOAT, SqlTypeName::FLOAT),
      std::make_pair(JDBC_TYPE_REAL, SqlTypeName::REAL),
      std::make_pair(JDBC_TYPE_DOUBLE, SqlTypeName::DOUBLE),
      std::make_pair(JDBC_TYPE_VARCHAR, SqlTypeName::VARCHAR),
      std::make_pair(JDBC_TYPE_BINARY, SqlTypeName::BINARY),
      std::make_pair(JDBC_TYPE_VARBINARY, SqlTypeName::VARBINARY),
      std::make_pair(JDBC_TYPE_DATE, SqlTypeName::DATE),
      std::make_pair(JDBC_TYPE_TIME, SqlTypeName::TIME),
      std::make_pair(JDBC_TYPE_TIMESTAMP, SqlTypeName::TIMESTAMP)};

  int numTests = sizeof(tests) / sizeof(std::pair< int16_t, std::string >);

  for (int i = 0; i < numTests; i++) {
    ColumnMeta columnMeta(schema, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving std::string value

    // test SQL_DESC_LOCAL_TYPE_NAME
    found = columnMeta.GetAttribute(SQL_DESC_LOCAL_TYPE_NAME, resVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(resVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeCaseSensitive) {
  std::string schema("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::pair< int16_t, SQLLEN > tests[] = {
      std::make_pair(JDBC_TYPE_VARCHAR, SQL_TRUE),
      std::make_pair(JDBC_TYPE_CHAR, SQL_TRUE),
      std::make_pair(JDBC_TYPE_NCHAR, SQL_TRUE),
      std::make_pair(JDBC_TYPE_NVARCHAR, SQL_TRUE),
      std::make_pair(JDBC_TYPE_LONGVARCHAR, SQL_TRUE),
      std::make_pair(JDBC_TYPE_LONGNVARCHAR, SQL_TRUE),
      std::make_pair(JDBC_TYPE_BOOLEAN, SQL_FALSE),
      std::make_pair(JDBC_TYPE_SMALLINT, SQL_FALSE),
      std::make_pair(JDBC_TYPE_TINYINT, SQL_FALSE),
      std::make_pair(JDBC_TYPE_INTEGER, SQL_FALSE),
      std::make_pair(JDBC_TYPE_BIGINT, SQL_FALSE),
      std::make_pair(JDBC_TYPE_FLOAT, SQL_FALSE),
      std::make_pair(JDBC_TYPE_REAL, SQL_FALSE),
      std::make_pair(JDBC_TYPE_DOUBLE, SQL_FALSE),
      std::make_pair(JDBC_TYPE_BINARY, SQL_FALSE),
      std::make_pair(JDBC_TYPE_VARBINARY, SQL_FALSE),
      std::make_pair(JDBC_TYPE_DATE, SQL_FALSE),
      std::make_pair(JDBC_TYPE_TIME, SQL_FALSE),
      std::make_pair(JDBC_TYPE_TIMESTAMP, SQL_FALSE)};

  int numTests = sizeof(tests) / sizeof(std::pair< int16_t, SQLLEN >);

  for (int i = 0; i < numTests; i++) {
    ColumnMeta columnMeta(schema, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving SQLLEN value

    // test SQL_DESC_CASE_SENSITIVE
    found = columnMeta.GetAttribute(SQL_DESC_CASE_SENSITIVE, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeConciseTypeAndType) {
  std::string schema("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::pair< int16_t, SQLLEN > tests[] = {
      std::make_pair(JDBC_TYPE_VARCHAR, SQL_VARCHAR),
      std::make_pair(JDBC_TYPE_CHAR, SQL_CHAR),
      std::make_pair(JDBC_TYPE_LONGVARCHAR, SQL_LONGVARCHAR),
      std::make_pair(JDBC_TYPE_BOOLEAN, SQL_BIT),
      std::make_pair(JDBC_TYPE_SMALLINT, SQL_SMALLINT),
      std::make_pair(JDBC_TYPE_TINYINT, SQL_TINYINT),
      std::make_pair(JDBC_TYPE_INTEGER, SQL_INTEGER),
      std::make_pair(JDBC_TYPE_BIGINT, SQL_BIGINT),
      std::make_pair(JDBC_TYPE_FLOAT, SQL_FLOAT),
      std::make_pair(JDBC_TYPE_REAL, SQL_REAL),
      std::make_pair(JDBC_TYPE_DOUBLE, SQL_DOUBLE),
      std::make_pair(JDBC_TYPE_BINARY, SQL_BINARY),
      std::make_pair(JDBC_TYPE_VARBINARY, SQL_VARBINARY),
      std::make_pair(JDBC_TYPE_DATE, SQL_TYPE_DATE),
      std::make_pair(JDBC_TYPE_TIME, SQL_TYPE_TIME),
      std::make_pair(JDBC_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP)};

  int numTests = sizeof(tests) / sizeof(std::pair< int16_t, SQLLEN >);

  for (int i = 0; i < numTests; i++) {
    ColumnMeta columnMeta(schema, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving SQLLEN value

    // test SQL_DESC_CONCISE_TYPE
    found = columnMeta.GetAttribute(SQL_DESC_CONCISE_TYPE, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);

    // test SQL_DESC_TYPE
    found = columnMeta.GetAttribute(SQL_DESC_TYPE, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeDisplaySize) {
  std::string schema("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::pair< int16_t, SQLLEN > tests[] = {
      std::make_pair(JDBC_TYPE_VARCHAR, SQL_NO_TOTAL),
      std::make_pair(JDBC_TYPE_CHAR, SQL_NO_TOTAL),
      std::make_pair(JDBC_TYPE_LONGVARCHAR, SQL_NO_TOTAL),
      std::make_pair(JDBC_TYPE_BOOLEAN, 1),
      std::make_pair(JDBC_TYPE_SMALLINT, 6),
      std::make_pair(JDBC_TYPE_TINYINT, 4),
      std::make_pair(JDBC_TYPE_INTEGER, 11),
      std::make_pair(JDBC_TYPE_BIGINT, 20),
      std::make_pair(JDBC_TYPE_FLOAT, 24),
      std::make_pair(JDBC_TYPE_REAL, 14),
      std::make_pair(JDBC_TYPE_DOUBLE, 24),
      std::make_pair(JDBC_TYPE_BINARY, SQL_NO_TOTAL),
      std::make_pair(JDBC_TYPE_VARBINARY, SQL_NO_TOTAL),
      std::make_pair(JDBC_TYPE_DATE, 10),
      std::make_pair(JDBC_TYPE_TIME, 8),
      std::make_pair(JDBC_TYPE_TIMESTAMP, 19)};

  int numTests = sizeof(tests) / sizeof(std::pair< int16_t, SQLLEN >);

  for (int i = 0; i < numTests; i++) {
    ColumnMeta columnMeta(schema, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving SQLLEN value

    // test SQL_DESC_DISPLAY_SIZE
    found = columnMeta.GetAttribute(SQL_DESC_DISPLAY_SIZE, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeLength) {
  std::string schema("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::pair< int16_t, SQLLEN > tests[] = {
      std::make_pair(JDBC_TYPE_VARCHAR, SQL_NO_TOTAL),
      std::make_pair(JDBC_TYPE_CHAR, SQL_NO_TOTAL),
      std::make_pair(JDBC_TYPE_LONGVARCHAR, SQL_NO_TOTAL),
      std::make_pair(JDBC_TYPE_BOOLEAN, 1),
      std::make_pair(JDBC_TYPE_SMALLINT, 2),
      std::make_pair(JDBC_TYPE_TINYINT, 1),
      std::make_pair(JDBC_TYPE_INTEGER, 4),
      std::make_pair(JDBC_TYPE_BIGINT, 8),
      std::make_pair(JDBC_TYPE_FLOAT, 4),
      std::make_pair(JDBC_TYPE_REAL, 4),
      std::make_pair(JDBC_TYPE_DOUBLE, 8),
      std::make_pair(JDBC_TYPE_BINARY, SQL_NO_TOTAL),
      std::make_pair(JDBC_TYPE_VARBINARY, SQL_NO_TOTAL),
      std::make_pair(JDBC_TYPE_DATE, 6),
      std::make_pair(JDBC_TYPE_TIME, 6),
      std::make_pair(JDBC_TYPE_TIMESTAMP, 16)};

  int numTests = sizeof(tests) / sizeof(std::pair< int16_t, SQLLEN >);

  for (int i = 0; i < numTests; i++) {
    ColumnMeta columnMeta(schema, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving SQLLEN value

    // test SQL_DESC_LENGTH
    found = columnMeta.GetAttribute(SQL_DESC_LENGTH, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeOctetLength) {
  std::string schema("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::pair< int16_t, SQLLEN > tests[] = {
      std::make_pair(JDBC_TYPE_VARCHAR, SQL_NO_TOTAL),
      std::make_pair(JDBC_TYPE_CHAR, SQL_NO_TOTAL),
      std::make_pair(JDBC_TYPE_LONGVARCHAR, SQL_NO_TOTAL),
      std::make_pair(JDBC_TYPE_BOOLEAN, 1 * sizeof(char)),
      std::make_pair(JDBC_TYPE_SMALLINT, 2 * sizeof(char)),
      std::make_pair(JDBC_TYPE_TINYINT, 1 * sizeof(char)),
      std::make_pair(JDBC_TYPE_INTEGER, 4 * sizeof(char)),
      std::make_pair(JDBC_TYPE_BIGINT, 8 * sizeof(char)),
      std::make_pair(JDBC_TYPE_FLOAT, 4 * sizeof(char)),
      std::make_pair(JDBC_TYPE_REAL, 4 * sizeof(char)),
      std::make_pair(JDBC_TYPE_DOUBLE, 8 * sizeof(char)),
      std::make_pair(JDBC_TYPE_BINARY, SQL_NO_TOTAL),
      std::make_pair(JDBC_TYPE_VARBINARY, SQL_NO_TOTAL),
      std::make_pair(JDBC_TYPE_DATE, 6),
      std::make_pair(JDBC_TYPE_TIME, 6),
      std::make_pair(JDBC_TYPE_TIMESTAMP, 16)};

  int numTests = sizeof(tests) / sizeof(std::pair< int16_t, SQLLEN >);

  for (int i = 0; i < numTests; i++) {
    ColumnMeta columnMeta(schema, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving SQLLEN value

    // test SQL_DESC_OCTET_LENGTH
    found = columnMeta.GetAttribute(SQL_DESC_OCTET_LENGTH, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeNullable) {
  std::string schema("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;
  ColumnMeta columnMetaNullable(schema, table, column, JDBC_TYPE_NULL,
                               Nullability::NULLABLE);

  // test SQL_DESC_NULLABLE
  found = columnMetaNullable.GetAttribute(SQL_DESC_NULLABLE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_NULLABLE);

  ColumnMeta columnMetaNoNulls(schema, table, column, JDBC_TYPE_VARCHAR,
                             Nullability::NO_NULL);

  // test SQL_DESC_NULLABLE
  found = columnMetaNoNulls.GetAttribute(SQL_DESC_NULLABLE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_NO_NULLS);

  ColumnMeta columnMetaUnknown(schema, table, column, JDBC_TYPE_BOOLEAN,
                               Nullability::NULLABILITY_UNKNOWN);

  // test SQL_DESC_NULLABLE
  found = columnMetaUnknown.GetAttribute(SQL_DESC_NULLABLE, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_NULLABLE_UNKNOWN);
}

BOOST_AUTO_TEST_CASE(TestGetAttributeNumPrecRadix) {
  std::string schema("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::pair< int16_t, SQLLEN > tests[] = {
      std::make_pair(JDBC_TYPE_VARCHAR, 0), std::make_pair(JDBC_TYPE_CHAR, 0),
      std::make_pair(JDBC_TYPE_LONGVARCHAR, 0),
      std::make_pair(JDBC_TYPE_BOOLEAN, 10),
      // JDBC_TYPE_BOOLEAN corresponds to SQL_BIT, which gives radix 10
      std::make_pair(JDBC_TYPE_SMALLINT, 10),
      std::make_pair(JDBC_TYPE_TINYINT, 10),
      std::make_pair(JDBC_TYPE_INTEGER, 10),
      std::make_pair(JDBC_TYPE_BIGINT, 10), std::make_pair(JDBC_TYPE_FLOAT, 2),
      std::make_pair(JDBC_TYPE_REAL, 2), std::make_pair(JDBC_TYPE_DOUBLE, 2),
      std::make_pair(JDBC_TYPE_BINARY, 0),
      std::make_pair(JDBC_TYPE_VARBINARY, 0), std::make_pair(JDBC_TYPE_DATE, 0),
      std::make_pair(JDBC_TYPE_TIME, 0),
      std::make_pair(JDBC_TYPE_TIMESTAMP, 0)};

  int numTests = sizeof(tests) / sizeof(std::pair< int16_t, SQLLEN >);

  for (int i = 0; i < numTests; i++) {
    ColumnMeta columnMeta(schema, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving SQLLEN value

    // test SQL_DESC_NUM_PREC_RADIX
    found = columnMeta.GetAttribute(SQL_DESC_NUM_PREC_RADIX, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributePrecision) {
  std::string schema("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::pair< int16_t, SQLLEN > tests[] = {
      std::make_pair(JDBC_TYPE_VARCHAR, SQL_NO_TOTAL),
      std::make_pair(JDBC_TYPE_CHAR, SQL_NO_TOTAL),
      std::make_pair(JDBC_TYPE_LONGVARCHAR, SQL_NO_TOTAL),
      std::make_pair(JDBC_TYPE_BOOLEAN, 1),
      std::make_pair(JDBC_TYPE_SMALLINT, 5),
      std::make_pair(JDBC_TYPE_TINYINT, 3),
      std::make_pair(JDBC_TYPE_INTEGER, 10),
      std::make_pair(JDBC_TYPE_BIGINT, 19),
      std::make_pair(JDBC_TYPE_FLOAT, 15),
      std::make_pair(JDBC_TYPE_REAL, 7),
      std::make_pair(JDBC_TYPE_DOUBLE, 15),
      std::make_pair(JDBC_TYPE_BINARY, SQL_NO_TOTAL),
      std::make_pair(JDBC_TYPE_VARBINARY, SQL_NO_TOTAL),
      std::make_pair(JDBC_TYPE_DATE, 10),
      std::make_pair(JDBC_TYPE_TIME, 8),
      std::make_pair(JDBC_TYPE_TIMESTAMP, 19)};

  int numTests = sizeof(tests) / sizeof(std::pair< int16_t, SQLLEN >);

  for (int i = 0; i < numTests; i++) {
    ColumnMeta columnMeta(schema, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving SQLLEN value

    // test SQL_DESC_PRECISION
    found = columnMeta.GetAttribute(SQL_DESC_PRECISION, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeScale) {
  std::string schema("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::pair< int16_t, SQLLEN > tests[] = {
      std::make_pair(JDBC_TYPE_VARCHAR, -1),
      std::make_pair(JDBC_TYPE_CHAR, -1),
      std::make_pair(JDBC_TYPE_LONGVARCHAR, -1),
      std::make_pair(JDBC_TYPE_BOOLEAN, -1),
      std::make_pair(JDBC_TYPE_SMALLINT, 0),
      std::make_pair(JDBC_TYPE_TINYINT, 0),
      std::make_pair(JDBC_TYPE_INTEGER, 0),
      std::make_pair(JDBC_TYPE_BIGINT, 0),
      std::make_pair(JDBC_TYPE_FLOAT, -1),
      std::make_pair(JDBC_TYPE_REAL, -1),
      std::make_pair(JDBC_TYPE_DOUBLE, -1),
      std::make_pair(JDBC_TYPE_BINARY, -1),
      std::make_pair(JDBC_TYPE_VARBINARY, -1),
      std::make_pair(JDBC_TYPE_DATE, -1),
      std::make_pair(JDBC_TYPE_TIME, -1),
      std::make_pair(JDBC_TYPE_TIMESTAMP, -1)};

  int numTests = sizeof(tests) / sizeof(std::pair< int16_t, SQLLEN >);

  for (int i = 0; i < numTests; i++) {
    ColumnMeta columnMeta(schema, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving SQLLEN value

    // test SQL_DESC_SCALE
    found = columnMeta.GetAttribute(SQL_DESC_SCALE, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);
  }
}

BOOST_AUTO_TEST_CASE(TestGetAttributeUnnamed) {
  std::string schema("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;
  ColumnMeta columnMetaUnnamed(schema, table, std::string(""), JDBC_TYPE_NULL,
                               Nullability::NULLABLE);

  // test SQL_DESC_UNNAMED
  found = columnMetaUnnamed.GetAttribute(SQL_DESC_UNNAMED, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_UNNAMED);

  ColumnMeta columnMetaNamed(schema, table, column, JDBC_TYPE_NULL,
                             Nullability::NULLABLE);

  // test SQL_DESC_UNNAMED
  found = columnMetaNamed.GetAttribute(SQL_DESC_UNNAMED, intVal);
  BOOST_CHECK(found);
  BOOST_CHECK_EQUAL(intVal, SQL_NAMED);
}

BOOST_AUTO_TEST_CASE(TestGetAttributeUnsigned) {
  std::string schema("database");
  std::string table("table");
  std::string column("column");

  SQLLEN intVal;
  std::string resVal;
  bool found;

  std::pair< int16_t, SQLLEN > tests[] = {
      std::make_pair(JDBC_TYPE_VARCHAR, SQL_TRUE),
      std::make_pair(JDBC_TYPE_CHAR, SQL_TRUE),
      std::make_pair(JDBC_TYPE_NCHAR, SQL_TRUE),
      std::make_pair(JDBC_TYPE_NVARCHAR, SQL_TRUE),
      std::make_pair(JDBC_TYPE_LONGVARCHAR, SQL_TRUE),
      std::make_pair(JDBC_TYPE_LONGNVARCHAR, SQL_TRUE),
      std::make_pair(JDBC_TYPE_BOOLEAN, SQL_FALSE),
      // JDBC_TYPE_BOOLEAN corresponds to SQL_BIT, which is signed
      std::make_pair(JDBC_TYPE_SMALLINT, SQL_FALSE),
      std::make_pair(JDBC_TYPE_TINYINT, SQL_FALSE),
      std::make_pair(JDBC_TYPE_INTEGER, SQL_FALSE),
      std::make_pair(JDBC_TYPE_BIGINT, SQL_FALSE),
      std::make_pair(JDBC_TYPE_FLOAT, SQL_FALSE),
      std::make_pair(JDBC_TYPE_REAL, SQL_FALSE),
      std::make_pair(JDBC_TYPE_DOUBLE, SQL_FALSE),
      std::make_pair(JDBC_TYPE_BINARY, SQL_TRUE),
      std::make_pair(JDBC_TYPE_VARBINARY, SQL_TRUE),
      std::make_pair(JDBC_TYPE_DATE, SQL_TRUE),
      std::make_pair(JDBC_TYPE_TIME, SQL_TRUE),
      std::make_pair(JDBC_TYPE_TIMESTAMP, SQL_TRUE)};

  int numTests = sizeof(tests) / sizeof(std::pair< int16_t, SQLLEN >);

  for (int i = 0; i < numTests; i++) {
    ColumnMeta columnMeta(schema, table, column, tests[i].first,
                          Nullability::NULLABLE);
    // test retrieving SQLLEN value

    // test SQL_DESC_UNSIGNED
    found = columnMeta.GetAttribute(SQL_DESC_UNSIGNED, intVal);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(intVal, tests[i].second);
  }
}
