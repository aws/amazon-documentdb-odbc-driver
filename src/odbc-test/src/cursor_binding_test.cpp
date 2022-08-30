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

#include <boost/test/unit_test.hpp>
#include <cstdio>
#include <string>
#include <vector>

#include "documentdb/odbc/utility.h"
#include "odbc_test_suite.h"
#include "test_type.h"
#include "test_utils.h"

using namespace documentdb;
using namespace documentdb_test;

using namespace boost::unit_test;

/**
 * Test setup fixture.
 */
struct CursorBindingTestSuiteFixture : public odbc::OdbcTestSuite {
  /**
   * Constructor.
   */
  CursorBindingTestSuiteFixture() {
  }

  /**
   * Destructor.
   */
  virtual ~CursorBindingTestSuiteFixture() {
    // No-op.
  }
};

BOOST_FIXTURE_TEST_SUITE(CursorBindingTestSuite, CursorBindingTestSuiteFixture)

#define CHECK_TEST_VALUES(idx, testIdx)                                     \
  do {                                                                      \
    BOOST_TEST_CONTEXT("Test idx: " << testIdx) {                           \
      BOOST_CHECK(RowStatus[idx] == SQL_ROW_SUCCESS                         \
                  || RowStatus[idx] == SQL_ROW_SUCCESS_WITH_INFO);          \
                                                                            \
      BOOST_CHECK(idFieldsLen[idx] != SQL_NULL_DATA);                       \
      BOOST_CHECK(i32FieldsInd[idx] != SQL_NULL_DATA);                      \
      BOOST_CHECK(i64FieldsInd[idx] != SQL_NULL_DATA);                      \
      BOOST_CHECK(dec128FieldsLen[idx] != SQL_NULL_DATA);                   \
      BOOST_CHECK(doubleFieldsInd[idx] != SQL_NULL_DATA);                   \
      BOOST_CHECK(strFieldsLen[idx] != SQL_NULL_DATA);                      \
      BOOST_CHECK(boolFieldsInd[idx] != SQL_NULL_DATA);                     \
      BOOST_CHECK(dateFieldsInd[idx] != SQL_NULL_DATA);                     \
      BOOST_CHECK(nullFieldsLen[idx] == SQL_NULL_DATA);                     \
      BOOST_CHECK(binaryFieldsLen[idx] != SQL_NULL_DATA);                   \
                                                                            \
      std::string idField = utility::SqlWcharToString(&idFields[idx][0]);   \
      int32_t i32Field = static_cast< int32_t >(i32Fields[idx]);            \
      int32_t i64Field = static_cast< int32_t >(i64Fields[idx]);            \
      std::string dec128Field =                                             \
          utility::SqlWcharToString(&dec128Fields[idx][0]);                 \
      double doubleField = static_cast< double >(doubleFields[idx]);        \
      std::string strField = utility::SqlWcharToString(&strFields[idx][0]); \
      bool boolField = boolFields[idx] != 0;                                \
                                                                            \
      CheckTestIdValue(testIdx, idField);                                   \
      CheckTestI32Value(testIdx, i32Field);                                 \
      CheckTestI64Value(testIdx, i64Field);                                 \
      CheckTestDec128Value(testIdx, dec128Field);                           \
      CheckTestDoubleValue(testIdx, doubleField);                           \
      CheckTestStringValue(testIdx, strField);                              \
      CheckTestBoolValue(testIdx, boolField);                               \
      CheckTestDateValue(testIdx, dateFields[idx]);                         \
      CheckTestI8ArrayValue(testIdx,                                        \
                            reinterpret_cast< int8_t* >(binaryFields[idx]), \
                            static_cast< SQLLEN >(binaryFieldsLen[idx]));   \
    }                                                                       \
  } while (false)

BOOST_AUTO_TEST_CASE(TestCursorBindingColumnWise) {
  enum { ROWS_COUNT = 16 };
  enum { ROW_ARRAY_SIZE = 10 };
  enum { BUFFER_SIZE = 64 };

  std::string connectionStr;
  CreateDsnConnectionStringForLocalServer(connectionStr);
  Connect(connectionStr);

  // Setting attributes.

  SQLUSMALLINT RowStatus[ROW_ARRAY_SIZE];
  SQLUINTEGER NumRowsFetched;

  SQLRETURN ret;

  ret = SQLSetStmtAttr(stmt, SQL_ATTR_ROW_BIND_TYPE, SQL_BIND_BY_COLUMN, 0);
  ODBC_THROW_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLSetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE,
                       reinterpret_cast< SQLPOINTER* >(ROW_ARRAY_SIZE), 0);
  ODBC_THROW_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLSetStmtAttr(stmt, SQL_ATTR_ROW_STATUS_PTR, RowStatus, 0);
  ODBC_THROW_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLSetStmtAttr(stmt, SQL_ATTR_ROWS_FETCHED_PTR, &NumRowsFetched, 0);
  ODBC_THROW_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  // Binding collumns.

  SQLWCHAR idFields[ROW_ARRAY_SIZE][BUFFER_SIZE];
  SQLLEN idFieldsLen[ROW_ARRAY_SIZE];

  SQLINTEGER i32Fields[ROW_ARRAY_SIZE] = {0};
  SQLLEN i32FieldsInd[ROW_ARRAY_SIZE];

  SQLBIGINT i64Fields[ROW_ARRAY_SIZE] = {0};
  SQLLEN i64FieldsInd[ROW_ARRAY_SIZE];

  SQLWCHAR dec128Fields[ROW_ARRAY_SIZE][BUFFER_SIZE];
  SQLLEN dec128FieldsLen[ROW_ARRAY_SIZE];

  SQLDOUBLE doubleFields[ROW_ARRAY_SIZE];
  SQLLEN doubleFieldsInd[ROW_ARRAY_SIZE];

  SQLWCHAR strFields[ROW_ARRAY_SIZE][BUFFER_SIZE];
  SQLLEN strFieldsLen[ROW_ARRAY_SIZE];

  bool boolFields[ROW_ARRAY_SIZE];
  SQLLEN boolFieldsInd[ROW_ARRAY_SIZE];

  SQL_DATE_STRUCT dateFields[ROW_ARRAY_SIZE];
  SQLLEN dateFieldsInd[ROW_ARRAY_SIZE];

  SQLWCHAR nullFields[ROW_ARRAY_SIZE][BUFFER_SIZE];
  SQLLEN nullFieldsLen[ROW_ARRAY_SIZE];

  SQLSCHAR binaryFields[ROW_ARRAY_SIZE][BUFFER_SIZE];
  SQLLEN binaryFieldsLen[ROW_ARRAY_SIZE];

  ret = SQLBindCol(stmt, 1, SQL_C_WCHAR, idFields,
                   BUFFER_SIZE * sizeof(SQLWCHAR), idFieldsLen);
  ODBC_THROW_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLBindCol(stmt, 2, SQL_C_LONG, i32Fields, 0, i32FieldsInd);
  ODBC_THROW_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLBindCol(stmt, 3, SQL_C_SBIGINT, i64Fields, 0, i64FieldsInd);
  ODBC_THROW_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLBindCol(stmt, 4, SQL_C_WCHAR, dec128Fields,
                   BUFFER_SIZE * sizeof(SQLWCHAR), dec128FieldsLen);
  ODBC_THROW_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLBindCol(stmt, 5, SQL_C_DOUBLE, doubleFields, 0, doubleFieldsInd);
  ODBC_THROW_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLBindCol(stmt, 6, SQL_C_WCHAR, strFields,
                   BUFFER_SIZE * sizeof(SQLWCHAR), strFieldsLen);
  ODBC_THROW_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLBindCol(stmt, 7, SQL_C_BIT, boolFields, 0, boolFieldsInd);
  ODBC_THROW_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLBindCol(stmt, 8, SQL_C_TYPE_DATE, dateFields, 0, dateFieldsInd);
  ODBC_THROW_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLBindCol(stmt, 9, SQL_C_TYPE_TIME, nullFields, 0, nullFieldsLen);
  ODBC_THROW_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLBindCol(stmt, 10, SQL_C_BINARY, binaryFields, BUFFER_SIZE,
                   binaryFieldsLen);
  ODBC_THROW_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  std::vector< SQLWCHAR > sql = utility::ToWCHARVector(
      "SELECT "
      "  queries_test_006__id, fieldInt, fieldLong, fieldDecimal128, "
      "  fieldDouble, fieldString, fieldBoolean, fieldDate, fieldNull, "
      "  fieldBinary "
      " FROM queries_test_006 "
      " ORDER BY queries_test_006__id");

  // Execute a statement to retrieve rows from the Orders table.
  ret = SQLExecDirect(stmt, sql.data(), SQL_NTS);
  ODBC_THROW_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLFetchScroll(stmt, SQL_FETCH_NEXT, 0);
  ODBC_THROW_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  BOOST_CHECK_EQUAL(NumRowsFetched, (SQLUINTEGER)ROW_ARRAY_SIZE);

  for (int64_t i = 0; i < NumRowsFetched; i++) {
    CHECK_TEST_VALUES(i, static_cast< int >(i));
  }

  ret = SQLFetchScroll(stmt, SQL_FETCH_NEXT, 0);
  ODBC_THROW_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  BOOST_CHECK_EQUAL(NumRowsFetched, ROWS_COUNT - ROW_ARRAY_SIZE);

  for (int64_t i = 0; i < NumRowsFetched; i++) {
    int64_t testIdx = i + ROW_ARRAY_SIZE;
    CHECK_TEST_VALUES(i, static_cast< int >(testIdx));
  }

  for (int64_t i = NumRowsFetched; i < ROW_ARRAY_SIZE; i++) {
    BOOST_TEST_INFO("Checking row status for row: " << i);
    BOOST_CHECK(RowStatus[i] == SQL_ROW_NOROW);
  }

  ret = SQLFetchScroll(stmt, SQL_FETCH_NEXT, 0);
  BOOST_CHECK_EQUAL(ret, SQL_NO_DATA);

  // Close the cursor.
  ret = SQLCloseCursor(stmt);
  ODBC_THROW_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);
}

BOOST_AUTO_TEST_CASE(TestCursorBindingRowWise) {
  std::string connectionStr;
  CreateDsnConnectionStringForLocalServer(connectionStr);
  Connect(connectionStr);

  SQLRETURN ret = SQLSetStmtAttr(stmt, SQL_ATTR_ROW_BIND_TYPE,
                                 reinterpret_cast< SQLPOINTER* >(42), 0);

  BOOST_CHECK_EQUAL(ret, SQL_ERROR);

  CheckSQLStatementDiagnosticError("HYC00");
}

BOOST_AUTO_TEST_SUITE_END()
