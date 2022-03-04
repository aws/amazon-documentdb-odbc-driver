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
#include "ignite/odbc/common/fixed_size_array.h"
#include "ignite/ignition.h"
#include "ignite/odbc/impl/binary/binary_utils.h"
#include "odbc_test_suite.h"
#include "test_type.h"
#include "test_utils.h"

using namespace ignite;
using namespace ignite::cache;
using namespace ignite::cache::query;
using namespace ignite::odbc::common;
using namespace ignite_test;
using namespace ignite::odbc::binary;
using namespace ignite::odbc::impl::binary;
using namespace ignite::odbc::impl::interop;

using namespace boost::unit_test;

/**
 * Test setup fixture.
 */
struct MetaQueriesTestSuiteFixture : public odbc::OdbcTestSuite {
  const static SQLLEN C_STR_LEN_DEFAULT = 1024;

  /**
   * Checks single row result set for correct work with SQLGetData.
   *
   * @param stmt Statement.
   */
  void CheckSingleRowResultSetWithGetData(
      SQLHSTMT stmt, SQLUSMALLINT columnIndex = 1,
      const char *expectedValue = nullptr) const {
    SQLRETURN ret = SQLFetch(stmt);

    if (!SQL_SUCCEEDED(ret)) {
      BOOST_CHECK(ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
      std::string sqlMessage = GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt);
      if (sqlMessage.empty()) {
        sqlMessage.append("SQLFetch returned: " + std::to_string(ret));
      }
      BOOST_FAIL(sqlMessage);
    }

    char buf[1024];
    SQLLEN bufLen = sizeof(buf);

    columnIndex = columnIndex >= 1 ? columnIndex : 1;
    for (int i = 1; i <= columnIndex; i++) {
      ret =
          SQLGetData(stmt, columnIndex, SQL_C_CHAR, buf, sizeof(buf), &bufLen);

      if (!SQL_SUCCEEDED(ret))
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

      if (i == columnIndex && expectedValue != nullptr) {
        std::string actualValueStr = buf;
        std::string expectedValueStr = expectedValue;
        BOOST_CHECK(expectedValueStr == actualValueStr);
      }
    }

    ret = SQLFetch(stmt);

    BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);

    ret = SQLGetData(stmt, 1, SQL_C_CHAR, buf, sizeof(buf), &bufLen);

    BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
    BOOST_CHECK_EQUAL(GetOdbcErrorState(SQL_HANDLE_STMT, stmt), "24000");
  }

  /**
   * Check string column.
   *
   * @param stmt Statement.
   * @param colId Column ID to check.
   * @param value Expected value.
   */
  void CheckStringColumn(SQLHSTMT stmt, int colId, const std::string &value) {
    char buf[1024];
    SQLLEN bufLen = sizeof(buf);

    SQLRETURN ret =
        SQLGetData(stmt, colId, SQL_C_CHAR, buf, sizeof(buf), &bufLen);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    if (bufLen <= 0)
      BOOST_CHECK(value.empty());
    else
      BOOST_CHECK_EQUAL(std::string(buf, static_cast< size_t >(bufLen)), value);
  }

  /**
   * Constructor.
   */
  MetaQueriesTestSuiteFixture() : grid(0), cache1(0), cache2(0) {
  }

  bool WasNull(const SQLLEN length) {
    return (length == SQL_NULL_DATA);
  }

  void SQLColumnsBindColumns(
      SQLHSTMT stmt, char table_cat[], SQLLEN &table_cat_len,
      char table_schem[], SQLLEN &table_schem_len, char table_name[],
      SQLLEN &table_name_len, char column_name[], SQLLEN &column_name_len,
      SQLSMALLINT &data_type, SQLLEN &data_type_len, char type_name[],
      SQLLEN &type_name_len, SQLINTEGER &column_size, SQLLEN &column_size_len,
      SQLINTEGER &buffer_length, SQLLEN &buffer_length_len,
      SQLSMALLINT &decimal_digits, SQLLEN &decimal_digits_len,
      SQLSMALLINT &num_prec_radix, SQLLEN &num_prec_radix_len,
      SQLSMALLINT &nullable, SQLLEN &nullable_len, char remarks[],
      SQLLEN &remarks_len, char column_def[], SQLLEN &column_def_len,
      SQLSMALLINT &sql_data_type, SQLLEN &sql_data_type_len,
      SQLSMALLINT &sql_datetime_sub, SQLLEN &sql_datetime_sub_len,
      SQLINTEGER &char_octet_length, SQLLEN &char_octet_length_len,
      SQLINTEGER &ordinal_position, SQLLEN &ordinal_position_len,
      char is_nullable[], SQLLEN &is_nullable_len) {
    SQLRETURN ret = SQL_SUCCESS;
    ret = SQLBindCol(stmt, 1, SQL_C_CHAR, table_cat, C_STR_LEN_DEFAULT,
                     &table_cat_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 2, SQL_C_CHAR, table_schem, C_STR_LEN_DEFAULT,
                     &table_schem_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 3, SQL_C_CHAR, table_name, C_STR_LEN_DEFAULT,
                     &table_name_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 4, SQL_C_CHAR, column_name, C_STR_LEN_DEFAULT,
                     &column_name_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 5, SQL_SMALLINT, &data_type, sizeof(data_type),
                     &data_type_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 6, SQL_C_CHAR, type_name, C_STR_LEN_DEFAULT,
                     &type_name_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 7, SQL_INTEGER, &column_size, sizeof(column_size),
                     &column_size_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 8, SQL_INTEGER, &buffer_length, sizeof(buffer_length),
                     &buffer_length_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 9, SQL_SMALLINT, &decimal_digits,
                     sizeof(decimal_digits), &decimal_digits_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 10, SQL_SMALLINT, &num_prec_radix,
                     sizeof(num_prec_radix), &num_prec_radix_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 11, SQL_SMALLINT, &nullable, sizeof(nullable),
                     &nullable_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 12, SQL_C_CHAR, remarks, C_STR_LEN_DEFAULT,
                     &remarks_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 13, SQL_C_CHAR, column_def, C_STR_LEN_DEFAULT,
                     &column_def_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 14, SQL_SMALLINT, &sql_data_type,
                     sizeof(sql_data_type), &sql_data_type_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 15, SQL_SMALLINT, &sql_datetime_sub,
                     sizeof(sql_datetime_sub), &sql_datetime_sub_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 16, SQL_INTEGER, &char_octet_length,
                     sizeof(char_octet_length), &char_octet_length_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 17, SQL_INTEGER, &ordinal_position,
                     sizeof(ordinal_position), &ordinal_position_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
    ret = SQLBindCol(stmt, 18, SQL_C_CHAR, is_nullable, C_STR_LEN_DEFAULT,
                     &is_nullable_len);
    BOOST_CHECK(SQL_SUCCEEDED(ret));
  }

  /**
   * Check result set column metadata using SQLDescribeCol.
   *
   * @param stmt Statement.
   * @param idx Index.
   * @param expName Expected name.
   * @param expDataType Expected data type.
   * @param expSize Expected column size.
   * @param expScale Expected column scale.
   * @param expNullability expected nullability.
   */
  void CheckColumnMetaWithSQLDescribeCol(SQLHSTMT stmt, SQLUSMALLINT idx,
                                         const std::string &expName,
                                         SQLSMALLINT expDataType,
                                         SQLULEN expSize, SQLSMALLINT expScale,
                                         SQLSMALLINT expNullability) {
    std::vector< SQLCHAR > name(ODBC_BUFFER_SIZE);
    SQLSMALLINT nameLen = 0;
    SQLSMALLINT dataType = 0;
    SQLULEN size;
    SQLSMALLINT scale;
    SQLSMALLINT nullability;

    SQLRETURN ret =
        SQLDescribeCol(stmt, idx, &name[0], (SQLSMALLINT)name.size(), &nameLen,
                       &dataType, &size, &scale, &nullability);
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    BOOST_CHECK_GE(nameLen, 0);
    BOOST_CHECK_LE(nameLen, static_cast< SQLSMALLINT >(ODBC_BUFFER_SIZE));

    std::string nameStr(name.begin(), name.begin() + nameLen);

    BOOST_CHECK_EQUAL(nameStr, expName);
    BOOST_CHECK_EQUAL(dataType, expDataType);
    BOOST_CHECK_EQUAL(size, expSize);
    BOOST_CHECK_EQUAL(scale, expScale);
    BOOST_CHECK_EQUAL(nullability, expNullability);
  }

  /**
   * @param func Function to call before tests. May be PrepareQuery or
   * ExecQuery.
   *
   * 1. Start node.
   * 2. Connect to node using ODBC.
   * 3. Create table with decimal and char columns with specified size and
   * scale.
   * 4. Execute or prepare statement.
   * 5. Check presicion and scale of every column using SQLDescribeCol.
   */
  template < typename F >
  void CheckSQLDescribeColPrecisionAndScale(F func) {
    Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=PUBLIC");

    SQLRETURN ret = ExecQuery(
        "create table TestScalePrecision("
        "   id int primary key,"
        "   dec1 decimal(3,0),"
        "   dec2 decimal(42,12),"
        "   dec3 decimal,"
        "   char1 char(3),"
        "   char2 char(42),"
        "   char3 char not null,"
        "   vchar varchar"
        ")");

    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    ret = SQLFreeStmt(stmt, SQL_CLOSE);
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    ret = ExecQuery(
        "insert into "
        "TestScalePrecision(id, dec1, dec2, dec3, char1, char2, char3, vchar) "
        "values (1, 12, 160.23, -1234.56789, 'TST', 'Lorem Ipsum', 'Some test "
        "value', 'Some test varchar')");

    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    ret = SQLFreeStmt(stmt, SQL_CLOSE);
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    ret = (this->*func)(
        "select id, dec1, dec2, dec3, char1, char2, char3, vchar from "
        "PUBLIC.TestScalePrecision");
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    SQLSMALLINT columnCount = 0;

    ret = SQLNumResultCols(stmt, &columnCount);
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    BOOST_CHECK_EQUAL(columnCount, 8);

    CheckColumnMetaWithSQLDescribeCol(stmt, 1, "ID", SQL_INTEGER, 10, 0,
                                      SQL_NULLABLE);
    CheckColumnMetaWithSQLDescribeCol(stmt, 2, "DEC1", SQL_DECIMAL, 3, 0,
                                      SQL_NULLABLE);
    CheckColumnMetaWithSQLDescribeCol(stmt, 3, "DEC2", SQL_DECIMAL, 42, 12,
                                      SQL_NULLABLE);
    CheckColumnMetaWithSQLDescribeCol(stmt, 4, "DEC3", SQL_DECIMAL, 65535,
                                      32767, SQL_NULLABLE);
    CheckColumnMetaWithSQLDescribeCol(stmt, 5, "CHAR1", SQL_VARCHAR, 3, 0,
                                      SQL_NULLABLE);
    CheckColumnMetaWithSQLDescribeCol(stmt, 6, "CHAR2", SQL_VARCHAR, 42, 0,
                                      SQL_NULLABLE);
    CheckColumnMetaWithSQLDescribeCol(stmt, 7, "CHAR3", SQL_VARCHAR, 2147483647,
                                      0, SQL_NO_NULLS);
    CheckColumnMetaWithSQLDescribeCol(stmt, 8, "VCHAR", SQL_VARCHAR, 2147483647,
                                      0, SQL_NULLABLE);
  }

  /**
   * Check result set column metadata using SQLColAttribute.
   *
   * @param stmt Statement.
   * @param idx Index.
   * @param expName Expected name.
   * @param expDataType Expected data type.
   * @param expSize Expected column size.
   * @param expScale Expected column scale.
   * @param expNullability expected nullability.
   */
  void CheckColumnMetaWithSQLColAttribute(SQLHSTMT stmt, SQLUSMALLINT idx,
                                          const std::string &expName,
                                          SQLLEN expDataType, SQLULEN expSize,
                                          SQLLEN expScale,
                                          SQLLEN expNullability) {
    std::vector< SQLCHAR > name(ODBC_BUFFER_SIZE);
    SQLSMALLINT nameLen = 0;
    SQLLEN dataType = 0;
    SQLLEN size;
    SQLLEN scale;
    SQLLEN nullability;

    SQLRETURN ret = SQLColAttribute(stmt, idx, SQL_DESC_NAME, &name[0],
                                    (SQLSMALLINT)name.size(), &nameLen, 0);
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    ret = SQLColAttribute(stmt, idx, SQL_DESC_TYPE, 0, 0, 0, &dataType);
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    ret = SQLColAttribute(stmt, idx, SQL_DESC_PRECISION, 0, 0, 0, &size);
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    ret = SQLColAttribute(stmt, idx, SQL_DESC_SCALE, 0, 0, 0, &scale);
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    ret = SQLColAttribute(stmt, idx, SQL_DESC_NULLABLE, 0, 0, 0, &nullability);
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    BOOST_CHECK_GE(nameLen, 0);
    BOOST_CHECK_LE(nameLen, static_cast< SQLSMALLINT >(ODBC_BUFFER_SIZE));

    std::string nameStr(name.begin(), name.begin() + nameLen);

    BOOST_CHECK_EQUAL(nameStr, expName);
    BOOST_CHECK_EQUAL(dataType, expDataType);
    BOOST_CHECK_EQUAL(size, expSize);
    BOOST_CHECK_EQUAL(scale, expScale);
    BOOST_CHECK_EQUAL(nullability, expNullability);
  }

  /**
   * @param func Function to call before tests. May be PrepareQuery or
   * ExecQuery.
   *
   * 1. Start node.
   * 2. Connect to node using ODBC.
   * 3. Create table with decimal and char columns with specified size and
   * scale.
   * 4. Execute or prepare statement.
   * 5. Check presicion and scale of every column using SQLColAttribute.
   */
  template < typename F >
  void CheckSQLColAttributePrecisionAndScale(F func) {
    Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=PUBLIC");

    SQLRETURN ret = ExecQuery(
        "create table TestScalePrecision("
        "   id int primary key,"
        "   dec1 decimal(3,0),"
        "   dec2 decimal(42,12),"
        "   dec3 decimal,"
        "   char1 char(3),"
        "   char2 char(42),"
        "   char3 char not null,"
        "   vchar varchar"
        ")");

    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    ret = SQLFreeStmt(stmt, SQL_CLOSE);
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    ret = ExecQuery(
        "insert into "
        "TestScalePrecision(id, dec1, dec2, dec3, char1, char2, char3, vchar) "
        "values (1, 12, 160.23, -1234.56789, 'TST', 'Lorem Ipsum', 'Some test "
        "value', 'Some test varchar')");

    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    ret = SQLFreeStmt(stmt, SQL_CLOSE);
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    ret = (this->*func)(
        "select id, dec1, dec2, dec3, char1, char2, char3, vchar from "
        "PUBLIC.TestScalePrecision");
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    SQLSMALLINT columnCount = 0;

    ret = SQLNumResultCols(stmt, &columnCount);
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    BOOST_CHECK_EQUAL(columnCount, 8);

    CheckColumnMetaWithSQLColAttribute(stmt, 1, "ID", SQL_INTEGER, 10, 0,
                                       SQL_NULLABLE);
    CheckColumnMetaWithSQLColAttribute(stmt, 2, "DEC1", SQL_DECIMAL, 3, 0,
                                       SQL_NULLABLE);
    CheckColumnMetaWithSQLColAttribute(stmt, 3, "DEC2", SQL_DECIMAL, 42, 12,
                                       SQL_NULLABLE);
    CheckColumnMetaWithSQLColAttribute(stmt, 4, "DEC3", SQL_DECIMAL, 65535,
                                       32767, SQL_NULLABLE);
    CheckColumnMetaWithSQLColAttribute(stmt, 5, "CHAR1", SQL_VARCHAR, 3, 0,
                                       SQL_NULLABLE);
    CheckColumnMetaWithSQLColAttribute(stmt, 6, "CHAR2", SQL_VARCHAR, 42, 0,
                                       SQL_NULLABLE);
    CheckColumnMetaWithSQLColAttribute(stmt, 7, "CHAR3", SQL_VARCHAR,
                                       2147483647, 0, SQL_NO_NULLS);
    CheckColumnMetaWithSQLColAttribute(stmt, 8, "VCHAR", SQL_VARCHAR,
                                       2147483647, 0, SQL_NULLABLE);
  }

  /**
   * Destructor.
   */
  ~MetaQueriesTestSuiteFixture() {
    // No-op.
  }

  /** Node started during the test. */
  Ignite grid;

  /** Frist cache instance. */
  Cache< int64_t, TestType > cache1;

  /** Second cache instance. */
  Cache< int64_t, ComplexType > cache2;
};

BOOST_FIXTURE_TEST_SUITE(MetaQueriesTestSuite, MetaQueriesTestSuiteFixture)

BOOST_AUTO_TEST_CASE(TestGetTypeInfoAllTypes, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLRETURN ret = SQLGetTypeInfo(stmt, SQL_ALL_TYPES);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
}

BOOST_AUTO_TEST_CASE(TestDateTypeColumnAttributeCurdate, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLCHAR req[] = "select CURDATE()";
  SQLExecDirect(stmt, req, SQL_NTS);

  SQLLEN intVal = 0;

  SQLRETURN ret = SQLColAttribute(stmt, 1, SQL_DESC_TYPE, 0, 0, 0, &intVal);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(intVal, SQL_TYPE_DATE);
}

BOOST_AUTO_TEST_CASE(TestDateTypeColumnAttributeLiteral, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLCHAR req[] = "select DATE '2020-10-25'";
  SQLExecDirect(stmt, req, SQL_NTS);

  SQLLEN intVal = 0;

  SQLRETURN ret = SQLColAttribute(stmt, 1, SQL_DESC_TYPE, 0, 0, 0, &intVal);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(intVal, SQL_TYPE_DATE);
}

BOOST_AUTO_TEST_CASE(TestDateTypeColumnAttributeField, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLCHAR req[] = "select CAST (dateField as DATE) from TestType";
  SQLExecDirect(stmt, req, SQL_NTS);

  SQLLEN intVal = 0;

  SQLRETURN ret = SQLColAttribute(stmt, 1, SQL_DESC_TYPE, 0, 0, 0, &intVal);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(intVal, SQL_TYPE_DATE);
}

BOOST_AUTO_TEST_CASE(TestTimeTypeColumnAttributeLiteral, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLCHAR req[] = "select TIME '12:42:13'";
  SQLExecDirect(stmt, req, SQL_NTS);

  SQLLEN intVal = 0;

  SQLRETURN ret = SQLColAttribute(stmt, 1, SQL_DESC_TYPE, 0, 0, 0, &intVal);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(intVal, SQL_TYPE_TIME);
}

BOOST_AUTO_TEST_CASE(TestTimeTypeColumnAttributeField, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLCHAR req[] = "select timeField from TestType";
  SQLExecDirect(stmt, req, SQL_NTS);

  SQLLEN intVal = 0;

  SQLRETURN ret = SQLColAttribute(stmt, 1, SQL_DESC_TYPE, 0, 0, 0, &intVal);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(intVal, SQL_TYPE_TIME);
}

BOOST_AUTO_TEST_CASE(TestColAttributesColumnLength, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLCHAR req[] = "select strField from TestType";
  SQLExecDirect(stmt, req, SQL_NTS);

  SQLLEN intVal;
  SQLCHAR strBuf[1024];
  SQLSMALLINT strLen;

  SQLRETURN ret = SQLColAttribute(stmt, 1, SQL_COLUMN_LENGTH, strBuf,
                                  sizeof(strBuf), &strLen, &intVal);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(intVal, 60);
}

BOOST_AUTO_TEST_CASE(TestColAttributesColumnPresicion, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLCHAR req[] = "select strField from TestType";
  SQLExecDirect(stmt, req, SQL_NTS);

  SQLLEN intVal;
  SQLCHAR strBuf[1024];
  SQLSMALLINT strLen;

  SQLRETURN ret = SQLColAttribute(stmt, 1, SQL_COLUMN_PRECISION, strBuf,
                                  sizeof(strBuf), &strLen, &intVal);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(intVal, 60);
}

BOOST_AUTO_TEST_CASE(TestColAttributesColumnScale, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLCHAR req[] = "select strField from TestType";
  SQLExecDirect(stmt, req, SQL_NTS);

  SQLLEN intVal;
  SQLCHAR strBuf[1024];
  SQLSMALLINT strLen;

  SQLRETURN ret = SQLColAttribute(stmt, 1, SQL_COLUMN_SCALE, strBuf,
                                  sizeof(strBuf), &strLen, &intVal);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
}

BOOST_AUTO_TEST_CASE(TestColAttributesColumnLengthPrepare, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  InsertTestStrings(1);

  SQLCHAR req[] = "select strField from TestType";
  SQLPrepare(stmt, req, SQL_NTS);

  SQLLEN intVal;
  SQLCHAR strBuf[1024];
  SQLSMALLINT strLen;

  SQLRETURN ret = SQLColAttribute(stmt, 1, SQL_COLUMN_LENGTH, strBuf,
                                  sizeof(strBuf), &strLen, &intVal);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(intVal, 60);

  ret = SQLExecute(stmt);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLColAttribute(stmt, 1, SQL_COLUMN_LENGTH, strBuf, sizeof(strBuf),
                        &strLen, &intVal);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(intVal, 60);
}

BOOST_AUTO_TEST_CASE(TestColAttributesColumnPresicionPrepare, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  InsertTestStrings(1);

  SQLCHAR req[] = "select strField from TestType";
  SQLPrepare(stmt, req, SQL_NTS);

  SQLLEN intVal;
  SQLCHAR strBuf[1024];
  SQLSMALLINT strLen;

  SQLRETURN ret = SQLColAttribute(stmt, 1, SQL_COLUMN_PRECISION, strBuf,
                                  sizeof(strBuf), &strLen, &intVal);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(intVal, 60);

  ret = SQLExecute(stmt);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLColAttribute(stmt, 1, SQL_COLUMN_PRECISION, strBuf, sizeof(strBuf),
                        &strLen, &intVal);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(intVal, 60);
}

BOOST_AUTO_TEST_CASE(TestColAttributesColumnScalePrepare, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  InsertTestStrings(1);

  SQLCHAR req[] = "select strField from TestType";
  SQLPrepare(stmt, req, SQL_NTS);

  SQLLEN intVal;
  SQLCHAR strBuf[1024];
  SQLSMALLINT strLen;

  SQLRETURN ret = SQLColAttribute(stmt, 1, SQL_COLUMN_SCALE, strBuf,
                                  sizeof(strBuf), &strLen, &intVal);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLExecute(stmt);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLColAttribute(stmt, 1, SQL_COLUMN_SCALE, strBuf, sizeof(strBuf),
                        &strLen, &intVal);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
}

BOOST_AUTO_TEST_CASE(TestGetDataWithGetTypeInfo, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLRETURN ret = SQLGetTypeInfo(stmt, SQL_VARCHAR);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  CheckSingleRowResultSetWithGetData(stmt);
}

BOOST_AUTO_TEST_CASE(TestGetDataWithTablesReturnsOne) {
  std::string dsnConnectionString;
  CreateDsnConnectionString(dsnConnectionString);

  Connect(dsnConnectionString);

  SQLCHAR empty[] = "";
  SQLCHAR table[] = "test";

  SQLRETURN ret = SQLTables(stmt, empty, SQL_NTS, empty, SQL_NTS, table,
                            SQL_NTS, empty, SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  CheckSingleRowResultSetWithGetData(stmt, 2, "test");
}

BOOST_AUTO_TEST_CASE(TestGetDataWithTablesReturnsOneFromDocker) {
  SQLCHAR empty[] = "";
  SQLCHAR table[] = "meta_queries_test_001";

  std::string dsnConnectionString;
  std::string databaseName("odbc-test");
  CreateDsnConnectionStringForDocker(dsnConnectionString, databaseName);

  Connect(dsnConnectionString);

  SQLRETURN ret = SQLTables(stmt, empty, SQL_NTS, empty, SQL_NTS, table,
                            SQL_NTS, empty, SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  CheckSingleRowResultSetWithGetData(stmt, 3, "meta_queries_test_001");
}

BOOST_AUTO_TEST_CASE(TestGetDataWithTablesReturnsNone) {
  std::string dsnConnectionString;
  CreateDsnConnectionString(dsnConnectionString);

  Connect(dsnConnectionString);

  SQLCHAR empty[] = "";
  SQLCHAR table[] = "nonexistent";

  SQLRETURN ret = SQLTables(stmt, empty, SQL_NTS, empty, SQL_NTS, table,
                            SQL_NTS, empty, SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLFetch(stmt);

  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestGetDataWithTablesReturnsMany) {
  std::string dsnConnectionString;
  CreateDsnConnectionString(dsnConnectionString);

  Connect(dsnConnectionString);

  SQLCHAR empty[] = "";

  SQLRETURN ret = SQLTables(stmt, empty, SQL_NTS, empty, SQL_NTS, empty,
                            SQL_NTS, empty, SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  int count = 0;
  do {
    ret = SQLFetch(stmt);
    count++;
  } while (SQL_SUCCEEDED(ret));
  BOOST_CHECK(count > 1);

  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestGetDataWithColumnsReturnsNone) {
  std::string dsnConnectionString;
  CreateDsnConnectionString(dsnConnectionString);

  Connect(dsnConnectionString);

  SQLCHAR empty[] = "";
  SQLCHAR table[] = "nonexistent";
  SQLCHAR column[] = "nonexistent_column";

  SQLRETURN ret = SQLColumns(stmt, empty, SQL_NTS, empty, SQL_NTS, table,
                             SQL_NTS, column, SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLFetch(stmt);

  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestGetDataWithColumnsReturnsMany) {
  std::string dsnConnectionString;
  CreateDsnConnectionString(dsnConnectionString);

  Connect(dsnConnectionString);

  SQLCHAR empty[] = "";

  SQLRETURN ret = SQLColumns(stmt, empty, SQL_NTS, empty, SQL_NTS, empty,
                             SQL_NTS, empty, SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  int count = 0;
  do {
    ret = SQLFetch(stmt);
    count++;
  } while (SQL_SUCCEEDED(ret));
  BOOST_CHECK(count > 1);

  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestSQLColumnWithSQLBindCols) {
  std::string dsnConnectionString;
  CreateDsnConnectionString(dsnConnectionString);
  Connect(dsnConnectionString);

  SQLCHAR empty[] = "";
  SQLCHAR table[] = "test";
  SQLCHAR column[] = "test__id";

  SQLRETURN ret = SQL_SUCCESS;

  char table_cat[C_STR_LEN_DEFAULT]{};
  SQLLEN table_cat_len = sizeof(table_cat);
  char table_schem[C_STR_LEN_DEFAULT]{};
  SQLLEN table_schem_len = sizeof(table_schem);
  char table_name[C_STR_LEN_DEFAULT]{};
  SQLLEN table_name_len = sizeof(table_name);
  char column_name[C_STR_LEN_DEFAULT]{};
  SQLLEN column_name_len = sizeof(column_name);
  SQLSMALLINT data_type = 0;
  SQLLEN data_type_len = sizeof(data_type);
  char type_name[C_STR_LEN_DEFAULT]{};
  SQLLEN type_name_len = sizeof(type_name);
  SQLINTEGER column_size = 0;
  SQLLEN column_size_len = sizeof(column_size);
  SQLINTEGER buffer_length = 0;
  SQLLEN buffer_length_len = sizeof(buffer_length);
  SQLSMALLINT decimal_digits = 0;
  SQLLEN decimal_digits_len = sizeof(decimal_digits);
  SQLSMALLINT num_prec_radix = 0;
  SQLLEN num_prec_radix_len = sizeof(num_prec_radix);
  SQLSMALLINT nullable = 0;
  SQLLEN nullable_len = sizeof(nullable);
  char remarks[C_STR_LEN_DEFAULT]{};
  SQLLEN remarks_len = sizeof(remarks);
  char column_def[C_STR_LEN_DEFAULT]{};
  SQLLEN column_def_len = sizeof(column_def);
  SQLSMALLINT sql_data_type = 0;
  SQLLEN sql_data_type_len = sizeof(sql_data_type);
  SQLSMALLINT sql_datetime_sub = 0;
  SQLLEN sql_datetime_sub_len = sizeof(sql_datetime_sub);
  SQLINTEGER char_octet_length = 0;
  SQLLEN char_octet_length_len = sizeof(char_octet_length);
  SQLINTEGER ordinal_position = 0;
  SQLLEN ordinal_position_len = sizeof(ordinal_position);
  char is_nullable[C_STR_LEN_DEFAULT]{};
  SQLLEN is_nullable_len = sizeof(is_nullable);

  SQLColumnsBindColumns(
      stmt, table_cat, table_cat_len, table_schem, table_schem_len, table_name,
      table_name_len, column_name, column_name_len, data_type, data_type_len,
      type_name, type_name_len, column_size, column_size_len, buffer_length,
      buffer_length_len, decimal_digits, decimal_digits_len, num_prec_radix,
      num_prec_radix_len, nullable, nullable_len, remarks, remarks_len,
      column_def, column_def_len, sql_data_type, sql_data_type_len,
      sql_datetime_sub, sql_datetime_sub_len, char_octet_length,
      char_octet_length_len, ordinal_position, ordinal_position_len,
      is_nullable, is_nullable_len);

  ret = SQLColumns(stmt, empty, SQL_NTS, empty, SQL_NTS, table, SQL_NTS, column,
                   SQL_NTS);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  SQLSMALLINT numResultCols = 0;
  ret = SQLNumResultCols(stmt, &numResultCols);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  BOOST_CHECK_EQUAL(18, numResultCols);

  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  bool errorExpected = false;
  BOOST_CHECK_EQUAL(
      false, WasNull(table_cat_len));  // TODO: change to true when nullable
  BOOST_CHECK_EQUAL("", table_cat);    // TABLE_CAT
  BOOST_CHECK_EQUAL(false, WasNull(table_schem_len));
  BOOST_CHECK_EQUAL("test", table_schem);  // TABLE_SCHEM
  BOOST_CHECK_EQUAL(false, WasNull(table_name_len));
  BOOST_CHECK_EQUAL("test", table_name);  // TABLE_NAME
  BOOST_CHECK_EQUAL(false, WasNull(column_name_len));
  BOOST_CHECK_EQUAL("test__id", column_name);  // COLUMN_NAME
  BOOST_CHECK_EQUAL(false, WasNull(data_type_len));
  BOOST_CHECK_EQUAL(SQL_VARCHAR, data_type);  // DATA_TYPE
  BOOST_CHECK_EQUAL(false, WasNull(type_name_len));
  BOOST_CHECK_EQUAL("VARCHAR", type_name);  // TYPE_NAME
  BOOST_CHECK_EQUAL(false, WasNull(column_size_len));
  BOOST_CHECK_EQUAL(SQL_NO_TOTAL, column_size);  // COLUMN_SIZE
  BOOST_CHECK_EQUAL(
      false,
      WasNull(
          buffer_length_len));  // TODO: change to true whan nullable supported
  BOOST_CHECK_EQUAL(SQL_NO_TOTAL, buffer_length);  // BUFFER_LENGTH
  BOOST_CHECK_EQUAL(true, WasNull(decimal_digits_len));
  BOOST_CHECK_EQUAL(0, decimal_digits);  // DECIMAL_DIGITS
  BOOST_CHECK_EQUAL(true, WasNull(num_prec_radix_len));
  BOOST_CHECK_EQUAL(0, num_prec_radix);  // NUM_PREC_RADIX
  BOOST_CHECK_EQUAL(false, WasNull(nullable_len));
  BOOST_CHECK_EQUAL(SQL_NO_NULLS, nullable);  // NULLABLE
  BOOST_CHECK_EQUAL(
      false,
      WasNull(remarks_len));  // TODO: change to true whan nullable supported
  BOOST_CHECK_EQUAL("", remarks);  // REMARKS
  BOOST_CHECK_EQUAL(
      false,
      WasNull(column_def_len));  // TODO: change to true whan nullable supported
  BOOST_CHECK_EQUAL("", column_def);  // COLUMN_DEF
  BOOST_CHECK_EQUAL(false, WasNull(sql_data_type_len));
  BOOST_CHECK_EQUAL(SQL_VARCHAR, sql_data_type);  // SQL_DATA_TYPE
  BOOST_CHECK_EQUAL(true, WasNull(sql_datetime_sub_len));
  BOOST_CHECK_EQUAL(0, sql_datetime_sub);  // SQL_DATETIME_SUB
  BOOST_CHECK_EQUAL(false, WasNull(char_octet_length_len));
  BOOST_CHECK_EQUAL(SQL_NO_TOTAL, char_octet_length);  // CHAR_OCTET_LENGTH
  BOOST_CHECK_EQUAL(false, WasNull(ordinal_position_len));
  BOOST_CHECK_EQUAL(1, ordinal_position);  // ORDINAL_POSITION
  BOOST_CHECK_EQUAL(false, WasNull(is_nullable_len));
  BOOST_CHECK_EQUAL("NO", is_nullable);  // IS_NULLABLE

  // Check that we can get an attribute on the columns metadata.
  char attrColumnName[C_STR_LEN_DEFAULT];
  SQLSMALLINT attrColumnNameLen = 0;
  ret = SQLColAttribute(stmt, 2, SQL_DESC_NAME, attrColumnName,
                        sizeof(attrColumnName), &attrColumnNameLen,
                        nullptr);  // COLUMN_NAME, NOT NULL
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_ERROR(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }
  BOOST_CHECK_EQUAL("TABLE_SCHEM", attrColumnName);

  // Test that the next fetch will have no data.
  ret = SQLFetch(stmt);
  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestGetDataWithSelectQuery, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLCHAR insertReq[] =
      "insert into TestType(_key, strField) VALUES(1, 'Lorem ipsum')";
  SQLRETURN ret = SQLExecDirect(stmt, insertReq, SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  SQLCHAR selectReq[] = "select strField from TestType";
  ret = SQLExecDirect(stmt, selectReq, SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  CheckSingleRowResultSetWithGetData(stmt);
}

BOOST_AUTO_TEST_CASE(TestInsertTooLongValueFail, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLCHAR insertReq[] =
      "insert into TestType(_key, strField) VALUES(42, "
      "'0123456789012345678901234567890123456789012345678901234567891')";

  SQLRETURN ret = SQLExecDirect(stmt, insertReq, SQL_NTS);

  if (SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
}

BOOST_AUTO_TEST_CASE(TestGetInfoScrollOptions, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLUINTEGER val = 0;
  SQLRETURN ret = SQLGetInfo(dbc, SQL_SCROLL_OPTIONS, &val, 0, 0);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_DBC, dbc));

  BOOST_CHECK_NE(val, 0);
}

BOOST_AUTO_TEST_CASE(TestDdlTablesMeta, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=PUBLIC");

  SQLCHAR createTable[] =
      "create table TestTable(id int primary key, testColumn varchar)";
  SQLRETURN ret = SQLExecDirect(stmt, createTable, SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  SQLCHAR empty[] = "";
  SQLCHAR table[] = "TestTable";

  ret = SQLTables(stmt, empty, SQL_NTS, empty, SQL_NTS, table, SQL_NTS, empty,
                  SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLFetch(stmt);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  CheckStringColumn(stmt, 1, "");
  CheckStringColumn(stmt, 2, "\"PUBLIC\"");
  CheckStringColumn(stmt, 3, "TESTTABLE");
  CheckStringColumn(stmt, 4, "TABLE");

  ret = SQLFetch(stmt);

  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestDdlTablesMetaTableTypeList, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=PUBLIC");

  SQLCHAR createTable[] =
      "create table TestTable(id int primary key, testColumn varchar)";
  SQLRETURN ret = SQLExecDirect(stmt, createTable, SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  SQLCHAR empty[] = "";
  SQLCHAR table[] = "TestTable";
  SQLCHAR typeList[] = "TABLE,VIEW";

  ret = SQLTables(stmt, empty, SQL_NTS, empty, SQL_NTS, table, SQL_NTS,
                  typeList, SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLFetch(stmt);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  CheckStringColumn(stmt, 1, "");
  CheckStringColumn(stmt, 2, "\"PUBLIC\"");
  CheckStringColumn(stmt, 3, "TESTTABLE");
  CheckStringColumn(stmt, 4, "TABLE");

  ret = SQLFetch(stmt);

  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestDdlColumnsMeta, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=PUBLIC");

  SQLCHAR createTable[] =
      "create table TestTable(id int primary key, testColumn varchar)";
  SQLRETURN ret = SQLExecDirect(stmt, createTable, SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  SQLCHAR empty[] = "";
  SQLCHAR table[] = "TestTable";

  ret = SQLColumns(stmt, empty, SQL_NTS, empty, SQL_NTS, table, SQL_NTS, empty,
                   SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLFetch(stmt);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  CheckStringColumn(stmt, 1, "");
  CheckStringColumn(stmt, 2, "\"PUBLIC\"");
  CheckStringColumn(stmt, 3, "TESTTABLE");
  CheckStringColumn(stmt, 4, "ID");

  ret = SQLFetch(stmt);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  CheckStringColumn(stmt, 1, "");
  CheckStringColumn(stmt, 2, "\"PUBLIC\"");
  CheckStringColumn(stmt, 3, "TESTTABLE");
  CheckStringColumn(stmt, 4, "TESTCOLUMN");

  ret = SQLFetch(stmt);

  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestDdlColumnsMetaEscaped, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=PUBLIC");

  SQLCHAR createTable[] =
      "create table ESG_FOCUS(id int primary key, TEST_COLUMN varchar)";
  SQLRETURN ret = SQLExecDirect(stmt, createTable, SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  SQLCHAR empty[] = "";
  SQLCHAR table[] = "ESG\\_FOCUS";

  ret = SQLColumns(stmt, empty, SQL_NTS, empty, SQL_NTS, table, SQL_NTS, empty,
                   SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLFetch(stmt);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  CheckStringColumn(stmt, 1, "");
  CheckStringColumn(stmt, 2, "\"PUBLIC\"");
  CheckStringColumn(stmt, 3, "ESG_FOCUS");
  CheckStringColumn(stmt, 4, "ID");

  ret = SQLFetch(stmt);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  CheckStringColumn(stmt, 1, "");
  CheckStringColumn(stmt, 2, "\"PUBLIC\"");
  CheckStringColumn(stmt, 3, "ESG_FOCUS");
  CheckStringColumn(stmt, 4, "TEST_COLUMN");

  ret = SQLFetch(stmt);

  BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestSQLNumResultColsAfterSQLPrepare, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=PUBLIC");

  SQLRETURN ret = ExecQuery(
      "create table TestSqlPrepare(id int primary key, test1 varchar, test2 "
      "long, test3 varchar)");
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLFreeStmt(stmt, SQL_CLOSE);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = PrepareQuery("select * from PUBLIC.TestSqlPrepare");
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  SQLSMALLINT columnCount = 0;

  ret = SQLNumResultCols(stmt, &columnCount);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  BOOST_CHECK_EQUAL(columnCount, 4);

  ret = SQLExecute(stmt);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  columnCount = 0;

  ret = SQLNumResultCols(stmt, &columnCount);
  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  BOOST_CHECK_EQUAL(columnCount, 4);
}

/**
 * Check that SQLDescribeCol return valid scale and precision for columns of
 * different type after Prepare.
 *
 * 1. Start node.
 * 2. Connect to node using ODBC.
 * 3. Create table with decimal and char columns with specified size and scale.
 * 4. Prepare statement.
 * 5. Check precision and scale of every column using SQLDescribeCol.
 */
BOOST_AUTO_TEST_CASE(TestSQLDescribeColPrecisionAndScaleAfterPrepare,
                     *disabled()) {
  CheckSQLDescribeColPrecisionAndScale(&OdbcTestSuite::PrepareQuery);
}

/**
 * Check that SQLDescribeCol return valid scale and precision for columns of
 * different type after Execute.
 *
 * 1. Start node.
 * 2. Connect to node using ODBC.
 * 3. Create table with decimal and char columns with specified size and scale.
 * 4. Execute statement.
 * 5. Check precision and scale of every column using SQLDescribeCol. */
BOOST_AUTO_TEST_CASE(TestSQLDescribeColPrecisionAndScaleAfterExec,
                     *disabled()) {
  CheckSQLDescribeColPrecisionAndScale(&OdbcTestSuite::ExecQuery);
}

/**
 * Check that SQLColAttribute return valid scale and precision for columns of
 * different type after Prepare.
 *
 * 1. Start node.
 * 2. Connect to node using ODBC.
 * 3. Create table with decimal and char columns with specified size and scale.
 * 4. Prepare statement.
 * 5. Check precision and scale of every column using SQLColAttribute.
 */
BOOST_AUTO_TEST_CASE(TestSQLColAttributePrecisionAndScaleAfterPrepare,
                     *disabled()) {
  CheckSQLColAttributePrecisionAndScale(&OdbcTestSuite::PrepareQuery);
}

/**
 * Check that SQLColAttribute return valid scale and precision for columns of
 * different type after Execute.
 *
 * 1. Start node.
 * 2. Connect to node using ODBC.
 * 3. Create table with decimal and char columns with specified size and scale.
 * 4. Execute statement.
 * 5. Check precision and scale of every column using SQLColAttribute. */
BOOST_AUTO_TEST_CASE(TestSQLColAttributePrecisionAndScaleAfterExec,
                     *disabled()) {
  CheckSQLColAttributePrecisionAndScale(&OdbcTestSuite::ExecQuery);
}

BOOST_AUTO_TEST_SUITE_END()
