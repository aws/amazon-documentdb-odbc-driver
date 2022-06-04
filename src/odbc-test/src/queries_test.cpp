﻿/*
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
#include "ignite/odbc/binary/binary_object.h"
#include "ignite/odbc/common/fixed_size_array.h"
#include "ignite/odbc/guid.h"
#include "ignite/odbc/impl/binary/binary_utils.h"
#include "ignite/odbc/utility.h"
#include "odbc_test_suite.h"
#include "test_type.h"
#include "test_utils.h"

using namespace ignite;
using namespace ignite::odbc::common;
using namespace ignite_test;
using namespace ignite::odbc::binary;
using namespace ignite::odbc::impl::binary;
using namespace ignite::odbc::impl::interop;

using namespace boost::unit_test;

using ignite::odbc::impl::binary::BinaryUtils;

/**
 * Test setup fixture.
 */
struct QueriesTestSuiteFixture : odbc::OdbcTestSuite {
  /**
   * Constructor.
   */
  QueriesTestSuiteFixture() {
      // No-op
  }

  /**
   * Destructor.
   */
  ~QueriesTestSuiteFixture() override = default;

  template < typename T >
  void CheckTwoRowsInt(SQLSMALLINT type) {
    Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

    SQLRETURN ret;

    TestType in1(1, 2, 3, 4, "5", 6.0f, 7.0, true, Guid(8, 9),
                 MakeDateGmt(1987, 6, 5), MakeTimeGmt(12, 48, 12),
                 MakeTimestampGmt(1998, 12, 27, 1, 2, 3, 456));

    TestType in2(8, 7, 6, 5, "4", 3.0f, 2.0, false, Guid(1, 0),
                 MakeDateGmt(1976, 1, 12), MakeTimeGmt(0, 8, 59),
                 MakeTimestampGmt(1978, 8, 21, 23, 13, 45, 456));

    const SQLSMALLINT columnsCnt = 12;

    T columns[columnsCnt];

    std::memset(&columns, 0, sizeof(columns));

    // Binding columns.
    for (SQLSMALLINT i = 0; i < columnsCnt; ++i) {
      ret = SQLBindCol(stmt, i + 1, type, &columns[i], sizeof(columns[i]), nullptr);

      if (!SQL_SUCCEEDED(ret))
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    wchar_t request[] =
        L"SELECT i8Field, i16Field, i32Field, i64Field, strField, floatField, "
        L"doubleField, boolField, guidField, dateField, timeField, "
        L"timestampField FROM TestType";

    ret = SQLExecDirect(stmt, reinterpret_cast< SQLWCHAR* >(request), SQL_NTS);
    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    ret = SQLFetch(stmt);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    BOOST_CHECK_EQUAL(columns[0], 1);
    BOOST_CHECK_EQUAL(columns[1], 2);
    BOOST_CHECK_EQUAL(columns[2], 3);
    BOOST_CHECK_EQUAL(columns[3], 4);
    BOOST_CHECK_EQUAL(columns[4], 5);
    BOOST_CHECK_EQUAL(columns[5], 6);
    BOOST_CHECK_EQUAL(columns[6], 7);
    BOOST_CHECK_EQUAL(columns[7], 1);
    BOOST_CHECK_EQUAL(columns[8], 0);
    BOOST_CHECK_EQUAL(columns[9], 0);
    BOOST_CHECK_EQUAL(columns[10], 0);
    BOOST_CHECK_EQUAL(columns[11], 0);

    SQLLEN columnLens[columnsCnt];

    // Binding columns.
    for (SQLSMALLINT i = 0; i < columnsCnt; ++i) {
      ret = SQLBindCol(stmt, i + 1, type, &columns[i], sizeof(columns[i]),
                       &columnLens[i]);

      if (!SQL_SUCCEEDED(ret))
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    ret = SQLFetch(stmt);
    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    BOOST_CHECK_EQUAL(columns[0], 8);
    BOOST_CHECK_EQUAL(columns[1], 7);
    BOOST_CHECK_EQUAL(columns[2], 6);
    BOOST_CHECK_EQUAL(columns[3], 5);
    BOOST_CHECK_EQUAL(columns[4], 4);
    BOOST_CHECK_EQUAL(columns[5], 3);
    BOOST_CHECK_EQUAL(columns[6], 2);
    BOOST_CHECK_EQUAL(columns[7], 0);
    BOOST_CHECK_EQUAL(columns[8], 0);
    BOOST_CHECK_EQUAL(columns[9], 0);
    BOOST_CHECK_EQUAL(columns[10], 0);
    BOOST_CHECK_EQUAL(columns[11], 0);

    BOOST_CHECK_EQUAL(columnLens[0], static_cast< SQLLEN >(sizeof(T)));
    BOOST_CHECK_EQUAL(columnLens[1], static_cast< SQLLEN >(sizeof(T)));
    BOOST_CHECK_EQUAL(columnLens[2], static_cast< SQLLEN >(sizeof(T)));
    BOOST_CHECK_EQUAL(columnLens[3], static_cast< SQLLEN >(sizeof(T)));
    BOOST_CHECK_EQUAL(columnLens[4], static_cast< SQLLEN >(sizeof(T)));
    BOOST_CHECK_EQUAL(columnLens[5], static_cast< SQLLEN >(sizeof(T)));
    BOOST_CHECK_EQUAL(columnLens[6], static_cast< SQLLEN >(sizeof(T)));
    BOOST_CHECK_EQUAL(columnLens[7], static_cast< SQLLEN >(sizeof(T)));

    ret = SQLFetch(stmt);
    BOOST_CHECK(ret == SQL_NO_DATA);
  }

  void CheckParamsNum(const std::string& req, SQLSMALLINT expectedParamsNum) {
    std::vector< SQLWCHAR > req0(req.begin(), req.end());

    SQLRETURN ret =
        SQLPrepare(stmt, &req0[0], static_cast< SQLINTEGER >(req0.size()));

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    SQLSMALLINT paramsNum = -1;

    ret = SQLNumParams(stmt, &paramsNum);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    BOOST_CHECK_EQUAL(paramsNum, expectedParamsNum);
  }

  int CountRows(SQLHSTMT stmt) {
    int res = 0;

    SQLRETURN ret = SQL_SUCCESS;

    while (ret == SQL_SUCCESS) {
      ret = SQLFetch(stmt);

      if (ret == SQL_NO_DATA)
        break;

      if (!SQL_SUCCEEDED(ret))
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

      ++res;
    }

    return res;
  }
};

BOOST_FIXTURE_TEST_SUITE(QueriesTestSuite, QueriesTestSuiteFixture)

// Re-enable to test large data set.
BOOST_AUTO_TEST_CASE(TestMoviesCast, *disabled()) {
  std::string dsnConnectionString;
  CreateDsnConnectionStringForRemoteServer(dsnConnectionString, true, "", "",
                                           "movies");
  Connect(dsnConnectionString);
  SQLRETURN ret;
  std::vector< SQLWCHAR > request =
      NewSqlWchar(L"SELECT * FROM \"movies_cast\"");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  const int32_t buf_size = 1024;
  SQLWCHAR id[buf_size]{};
  SQLLEN id_len = 0;
  SQLBIGINT index = 0;
  SQLLEN index_len = 0;
  SQLWCHAR value[buf_size]{};
  SQLLEN value_len = 0;

  // Fetch 1st row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  while (SQL_SUCCEEDED(ret)) {

    ret = SQLGetData(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
    BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
    ret = SQLGetData(stmt, 2, SQL_C_SBIGINT, &index, sizeof(index),
                     &index_len);
    BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
    ret = SQLGetData(stmt, 3, SQL_C_WCHAR, value, sizeof(value),
                     &value_len);
    BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

    BOOST_CHECK_NE(SQL_NULL_DATA, id_len);
    BOOST_CHECK_NE(SQL_NULL_DATA, index_len);

    ret = SQLFetch(stmt);
  }

  // Fetch 2nd row - not exist
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
}

BOOST_AUTO_TEST_CASE(TestSingleResultUsingGetData) {
  std::string dsnConnectionString;
  CreateDsnConnectionStringForLocalServer(dsnConnectionString);
  Connect(dsnConnectionString);
  SQLRETURN ret;
  std::vector< SQLWCHAR > request =
      NewSqlWchar(L"SELECT * FROM \"queries_test_001\"");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  const int32_t buf_size = 1024;
  SQLWCHAR id[buf_size]{};
  SQLLEN id_len = 0;
  SQLWCHAR fieldDecimal128[buf_size]{};
  SQLLEN fieldDecimal128_len = 0;
  double fieldDouble = 0;
  SQLLEN fieldDouble_len = 0;
  SQLWCHAR fieldString[buf_size]{};
  SQLLEN fieldString_len = 0;
  SQLWCHAR fieldObjectId[buf_size]{};
  SQLLEN fieldObjectId_len = 0;
  bool fieldBoolean = false;
  SQLLEN fieldBoolean_len = 0;
  DATE_STRUCT fieldDate{};
  SQLLEN fieldDate_len = 0;
  SQLINTEGER fieldInt;
  SQLLEN fieldInt_len = 0;
  SQLBIGINT fieldLong;
  SQLLEN fieldLong_len = 0;
  SQLWCHAR fieldMaxKey[buf_size];
  SQLLEN fieldMaxKey_len = 0;
  SQLWCHAR fieldMinKey[buf_size];
  SQLLEN fieldMinKey_len = 0;
  SQLWCHAR fieldNull[buf_size];
  SQLLEN fieldNull_len = 0;
  SQLCHAR fieldBinary[buf_size];
  SQLLEN fieldBinary_len = 0;

  // Fetch 1st row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLGetData(stmt, 2, SQL_C_WCHAR, fieldDecimal128, sizeof(fieldDecimal128),
                   &fieldDecimal128_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLGetData(stmt, 3, SQL_C_DOUBLE, &fieldDouble, sizeof(fieldDouble),
                   &fieldDouble_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret =
      SQLGetData(stmt, 4, SQL_C_WCHAR, fieldString, sizeof(fieldString), &fieldString_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLGetData(stmt, 5, SQL_C_WCHAR, fieldObjectId, sizeof(fieldObjectId),
                   &fieldObjectId_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLGetData(stmt, 6, SQL_C_BIT, &fieldBoolean, sizeof(fieldBoolean),
                   &fieldBoolean_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLGetData(stmt, 7, SQL_C_TYPE_DATE, &fieldDate, sizeof(fieldDate),
                   &fieldDate_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLGetData(stmt, 8, SQL_C_SLONG, &fieldInt, sizeof(fieldInt),
                   &fieldInt_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLGetData(stmt, 9, SQL_C_SBIGINT, &fieldLong, sizeof(fieldLong),
                   &fieldLong_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLGetData(stmt, 10, SQL_C_WCHAR, fieldMaxKey, sizeof(fieldMaxKey),
                   &fieldMaxKey_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLGetData(stmt, 11, SQL_C_WCHAR, fieldMinKey, sizeof(fieldMinKey),
                   &fieldMinKey_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLGetData(stmt, 12, SQL_C_WCHAR, fieldNull, sizeof(fieldNull), &fieldNull_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLGetData(stmt, 13, SQL_C_BINARY, fieldBinary, sizeof(fieldBinary),
                   &fieldBinary_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_NE(SQL_NULL_DATA, id_len);
  BOOST_CHECK_EQUAL(L"62196dcc4d91892191475139", (wchar_t*)id);
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldDecimal128_len);
  BOOST_CHECK_EQUAL(L"340282350000000000000", (wchar_t*)fieldDecimal128);
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldDouble_len);
  BOOST_CHECK_EQUAL(1.7976931348623157e308, fieldDouble);
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldString_len);
  BOOST_CHECK_EQUAL(L"some Text", (wchar_t*)fieldString);
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldObjectId_len);
  BOOST_CHECK_EQUAL(L"62196dcc4d9189219147513a", (wchar_t*)fieldObjectId);
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldBoolean_len);
  BOOST_CHECK_EQUAL(true, fieldBoolean);
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldDate_len);
  BOOST_CHECK_EQUAL(2020, fieldDate.year);
  BOOST_CHECK_EQUAL(1, fieldDate.month);
  BOOST_CHECK_EQUAL(1, fieldDate.day);
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldInt_len);
  BOOST_CHECK_EQUAL(2147483647, fieldInt);
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldLong_len);
  BOOST_CHECK_EQUAL(9223372036854775807, fieldLong);
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldMaxKey_len);
  BOOST_CHECK_EQUAL(L"MAXKEY", (wchar_t*)fieldMaxKey);
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldMinKey_len);
  BOOST_CHECK_EQUAL(L"MINKEY", (wchar_t*)fieldMinKey);
  BOOST_CHECK_EQUAL(SQL_NULL_DATA, fieldNull_len);
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldBinary_len);
  BOOST_CHECK_EQUAL(3, fieldBinary_len);
  BOOST_CHECK_EQUAL(fieldBinary[0], 0);
  BOOST_CHECK_EQUAL(fieldBinary[1], 1);
  BOOST_CHECK_EQUAL(fieldBinary[2], 2);

  // Fetch 2nd row - not exist
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
}

BOOST_AUTO_TEST_CASE(TestSingleResultUsingBindCol) {
  std::string dsnConnectionString;
  CreateDsnConnectionStringForLocalServer(dsnConnectionString);
  Connect(dsnConnectionString);
  SQLRETURN ret;
  wchar_t request[] = L"SELECT * FROM \"queries_test_001\"";

  ret = SQLExecDirect(stmt, reinterpret_cast< SQLWCHAR* >(request), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }
  const int32_t buf_size = 1024;
  SQLWCHAR id[buf_size]{};
  SQLLEN id_len = 0;
  SQLWCHAR fieldDecimal128[buf_size]{};
  SQLLEN fieldDecimal128_len = 0;
  double fieldDouble = 0;
  SQLLEN fieldDouble_len = 0;
  SQLWCHAR fieldString[buf_size]{};
  SQLLEN fieldString_len = 0;
  SQLWCHAR fieldObjectId[buf_size]{};
  SQLLEN fieldObjectId_len = 0;
  bool fieldBoolean = false;
  SQLLEN fieldBoolean_len = 0;
  DATE_STRUCT fieldDate{};
  SQLLEN fieldDate_len = 0;
  SQLINTEGER fieldInt;
  SQLLEN fieldInt_len = 0;
  SQLBIGINT fieldLong;
  SQLLEN fieldLong_len = 0;
  SQLWCHAR fieldMaxKey[buf_size];
  SQLLEN fieldMaxKey_len = 0;
  SQLWCHAR fieldMinKey[buf_size];
  SQLLEN fieldMinKey_len = 0;
  SQLWCHAR fieldNull[buf_size];
  SQLLEN fieldNull_len = 0;
  SQLCHAR fieldBinary[buf_size];
  SQLLEN fieldBinary_len = 0;

  ret = SQLBindCol(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLBindCol(stmt, 2, SQL_C_WCHAR, fieldDecimal128, sizeof(fieldDecimal128), &fieldDecimal128_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLBindCol(stmt, 3, SQL_C_DOUBLE, &fieldDouble, sizeof(fieldDouble),
                   &fieldDouble_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLBindCol(stmt, 4, SQL_C_WCHAR, fieldString, sizeof(fieldString),
                   &fieldString_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLBindCol(stmt, 5, SQL_C_WCHAR, fieldObjectId, sizeof(fieldObjectId),
                   &fieldObjectId_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLBindCol(stmt, 6, SQL_C_BIT, &fieldBoolean, sizeof(fieldBoolean),
                   &fieldBoolean_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLBindCol(stmt, 7, SQL_C_TYPE_DATE, &fieldDate, sizeof(fieldDate),
                   &fieldDate_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLBindCol(stmt, 8, SQL_C_SLONG, &fieldInt, sizeof(fieldInt), &fieldInt_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLBindCol(stmt, 9, SQL_C_SBIGINT, &fieldLong, sizeof(fieldLong),
                   &fieldLong_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret =
      SQLBindCol(stmt, 10, SQL_C_WCHAR, fieldMaxKey, sizeof(fieldMaxKey), &fieldMaxKey_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret =
      SQLBindCol(stmt, 11, SQL_C_WCHAR, fieldMinKey, sizeof(fieldMinKey), &fieldMinKey_len);
    BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLBindCol(stmt, 12, SQL_C_WCHAR, fieldNull, sizeof(fieldNull), &fieldNull_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLBindCol(stmt, 13, SQL_C_BINARY, fieldBinary, sizeof(fieldBinary), &fieldBinary_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  // Fetch 1st row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_NE(SQL_NULL_DATA, id_len);
  BOOST_CHECK_EQUAL(L"62196dcc4d91892191475139", (wchar_t*)id);
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldDecimal128_len);
  BOOST_CHECK_EQUAL(L"340282350000000000000", (wchar_t*)fieldDecimal128);
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldDouble_len);
  BOOST_CHECK_EQUAL(1.7976931348623157e308, fieldDouble);
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldString_len);
  BOOST_CHECK_EQUAL(L"some Text", (wchar_t*)fieldString);
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldObjectId_len);
  BOOST_CHECK_EQUAL(L"62196dcc4d9189219147513a", (wchar_t*)fieldObjectId);
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldBoolean_len);
  BOOST_CHECK_EQUAL(true, fieldBoolean);
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldDate_len);
  BOOST_CHECK_EQUAL(2020, fieldDate.year);
  BOOST_CHECK_EQUAL(1, fieldDate.month);
  BOOST_CHECK_EQUAL(1, fieldDate.day);
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldInt_len);
  BOOST_CHECK_EQUAL(2147483647, fieldInt);
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldLong_len);
  BOOST_CHECK_EQUAL(9223372036854775807, fieldLong);
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldMaxKey_len);
  BOOST_CHECK_EQUAL(L"MAXKEY", (wchar_t*)fieldMaxKey);
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldMinKey_len);
  BOOST_CHECK_EQUAL(L"MINKEY", (wchar_t*)fieldMinKey);
  BOOST_CHECK_EQUAL(SQL_NULL_DATA, fieldNull_len);
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldBinary_len);
  BOOST_CHECK_EQUAL(3, fieldBinary_len);
  BOOST_CHECK_EQUAL(fieldBinary[0], 0);
  BOOST_CHECK_EQUAL(fieldBinary[1], 1);
  BOOST_CHECK_EQUAL(fieldBinary[2], 2);

  // Fetch 2nd row - not exist
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
}

BOOST_AUTO_TEST_CASE(TestMultiLineResultUsingGetData) {
  std::string dsnConnectionString;
  CreateDsnConnectionStringForLocalServer(dsnConnectionString);
  Connect(dsnConnectionString);
  SQLRETURN ret;
  wchar_t request[] = L"SELECT * FROM \"queries_test_002\" ORDER BY \"queries_test_002__id\"";

  ret = SQLExecDirect(stmt, reinterpret_cast< SQLWCHAR* >(request), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  const int32_t buf_size = 1024;
  SQLWCHAR id[buf_size]{};
  SQLLEN id_len = 0;
  //"\"fieldDecimal128\": \"$fieldDecimal128\", "
  SQLWCHAR fieldDecimal128[buf_size]{};
  SQLLEN fieldDecimal128_len = 0;

  // Fetch 1st row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLGetData(stmt, 2, SQL_C_WCHAR, fieldDecimal128, sizeof(fieldDecimal128),
                   &fieldDecimal128_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_NE(SQL_NULL_DATA, id_len);
  BOOST_CHECK_EQUAL(L"62196dcc4d91892191475139", (wchar_t*)id);
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldDecimal128_len);
  BOOST_CHECK_EQUAL(L"340282350000000000000", (wchar_t*)fieldDecimal128);

  // Fetch 2nd row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLGetData(stmt, 2, SQL_C_WCHAR, fieldDecimal128, sizeof(fieldDecimal128),
                   &fieldDecimal128_len);

  BOOST_CHECK_NE(SQL_NULL_DATA, id_len);
  BOOST_CHECK_EQUAL(L"62196dcc4d9189219147513a", (wchar_t*)id);
  BOOST_CHECK_EQUAL(SQL_NULL_DATA, fieldDecimal128_len);

  // Fetch 3rd row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLGetData(stmt, 2, SQL_C_WCHAR, fieldDecimal128, sizeof(fieldDecimal128),
                   &fieldDecimal128_len);

  BOOST_CHECK_NE(SQL_NULL_DATA, id_len);
  BOOST_CHECK_EQUAL(L"62196dcc4d9189219147513b", (wchar_t*)id);
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldDecimal128_len);
  BOOST_CHECK_EQUAL(L"340282350000000000000", (wchar_t*)fieldDecimal128);

  // Fetch 4th row - not exist
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
}

BOOST_AUTO_TEST_CASE(TestArrayStructJoinUsingGetData) {
  std::string dsnConnectionString;
  CreateDsnConnectionStringForLocalServer(dsnConnectionString);
  Connect(dsnConnectionString);
  SQLRETURN ret;
  wchar_t request[] =
      L"SELECT q3.queries_test_003__id, \n"
      L"  \"a1\".\"value\", \n"
      L"  \"a2\".\"value\", \n"
      L"  \"d1\".\"field1\", \n"
      L"  \"d2\".\"field2\" \n"
      L" FROM queries_test_003 AS q3 \n"
      L" JOIN queries_test_003_fieldArrayOfInt AS a1 \n"
      L"  ON q3.queries_test_003__id = a1.queries_test_003__id \n"
      L" JOIN queries_test_003_fieldArrayOfString AS a2 \n"
      L"  ON a1.queries_test_003__id = a2.queries_test_003__id \n"
      L" JOIN queries_test_003_fieldDocument AS d1 \n"
      L"  ON a2.queries_test_003__id = d1.queries_test_003__id \n"
      L" JOIN queries_test_003_fieldDocument_subDoc AS d2 \n"
      L"  ON d1.queries_test_003__id = d2.queries_test_003__id";

  ret = SQLExecDirect(stmt, reinterpret_cast< SQLWCHAR* >(request), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  const int32_t buf_size = 1024;
  SQLWCHAR id[buf_size]{};
  SQLLEN id_len = 0;
  SQLINTEGER a1_value = 0;
  SQLLEN a1_value_len = 0;
  SQLWCHAR a2_value[buf_size]{};
  SQLLEN a2_value_len = 0;
  SQLWCHAR d1_value[buf_size]{};
  SQLLEN d1_value_len = 0;
  SQLWCHAR d2_value[buf_size]{};
  SQLLEN d2_value_len = 0;

  // Fetch 1st row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLGetData(stmt, 2, SQL_C_SLONG, &a1_value, sizeof(a1_value), &a1_value_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLGetData(stmt, 3, SQL_C_WCHAR, a2_value, sizeof(a2_value), &a2_value_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLGetData(stmt, 4, SQL_C_WCHAR, d1_value, sizeof(d1_value), &d1_value_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLGetData(stmt, 5, SQL_C_WCHAR, d2_value, sizeof(d2_value), &d2_value_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  // Check the first row
  BOOST_CHECK_NE(SQL_NULL_DATA, id_len);
  BOOST_CHECK_EQUAL(L"62196dcc4d91892191475139", (wchar_t*)id);
  BOOST_CHECK_NE(SQL_NULL_DATA, a1_value_len);
  BOOST_CHECK_EQUAL(1, a1_value);
  BOOST_CHECK_NE(SQL_NULL_DATA, a2_value_len);
  BOOST_CHECK_EQUAL(L"value1", (wchar_t*)a2_value);
  BOOST_CHECK_NE(SQL_NULL_DATA, d1_value_len);
  BOOST_CHECK_EQUAL(L"field1 Value", (wchar_t*)d1_value);
  BOOST_CHECK_NE(SQL_NULL_DATA, d2_value_len);
  BOOST_CHECK_EQUAL(L"field2 Value", (wchar_t*)d2_value);

  // Count the rows
  int32_t actual_rows = 0;
  while (SQL_SUCCEEDED(ret)) {
    actual_rows++;
    ret = SQLFetch(stmt);
  }
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
  BOOST_CHECK_EQUAL(9, actual_rows);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsInt8, *disabled()) {
  CheckTwoRowsInt< signed char >(SQL_C_STINYINT);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsUint8, *disabled()) {
  CheckTwoRowsInt< unsigned char >(SQL_C_UTINYINT);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsInt16, *disabled()) {
  CheckTwoRowsInt< signed short >(SQL_C_SSHORT);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsUint16, *disabled()) {
  CheckTwoRowsInt< unsigned short >(SQL_C_USHORT);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsInt32, *disabled()) {
  CheckTwoRowsInt< SQLINTEGER >(SQL_C_SLONG);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsUint32, *disabled()) {
  CheckTwoRowsInt< SQLUINTEGER >(SQL_C_ULONG);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsInt64, *disabled()) {
  CheckTwoRowsInt< int64_t >(SQL_C_SBIGINT);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsUint64, *disabled()) {
  CheckTwoRowsInt< uint64_t >(SQL_C_UBIGINT);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsString, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLRETURN ret;

  TestType in1(1, 2, 3, 4, "5", 6.0f, 7.0, true, Guid(8, 9),
               MakeDateGmt(1987, 6, 5), MakeTimeGmt(12, 48, 12),
               MakeTimestampGmt(1998, 12, 27, 1, 2, 3, 456));

  TestType in2(8, 7, 6, 5, "4", 3.0f, 2.0, false, Guid(1, 0),
               MakeDateGmt(1976, 1, 12), MakeTimeGmt(0, 8, 59),
               MakeTimestampGmt(1978, 8, 21, 23, 13, 45, 999999999));

  const SQLSMALLINT columnsCnt = 12;

  SQLWCHAR columns[columnsCnt][ODBC_BUFFER_SIZE];

  // Binding columns.
  for (SQLSMALLINT i = 0; i < columnsCnt; ++i) {
    ret = SQLBindCol(stmt, i + 1, SQL_C_WCHAR, &columns[i],
                     ODBC_BUFFER_SIZE * sizeof(SQLWCHAR), 0);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  std::vector< SQLWCHAR > request = NewSqlWchar(
      L"SELECT i8Field, i16Field, i32Field, i64Field, strField, floatField, "
      L"doubleField, boolField, guidField, dateField, timeField, "
      L"timestampField FROM TestType");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLFetch(stmt);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[0], SQL_NTS), "1");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[1], SQL_NTS), "2");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[2], SQL_NTS), "3");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[3], SQL_NTS), "4");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[4], SQL_NTS), "5");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[5], SQL_NTS), "6");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[6], SQL_NTS), "7");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[7], SQL_NTS), "1");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[8], SQL_NTS),
                    "00000000-0000-0008-0000-000000000009");
  // Such format is used because Date returned as Timestamp.
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[9], SQL_NTS),
                    "1987-06-05 00:00:00");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[10], SQL_NTS),
                    "12:48:12");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[11], SQL_NTS),
                    "1998-12-27 01:02:03");

  SQLLEN columnLens[columnsCnt];

  // Binding columns.
  for (SQLSMALLINT i = 0; i < columnsCnt; ++i) {
    ret = SQLBindCol(stmt, i + 1, SQL_C_WCHAR, &columns[i], ODBC_BUFFER_SIZE,
                     &columnLens[i]);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[0], SQL_NTS), "8");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[1], SQL_NTS), "7");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[2], SQL_NTS), "6");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[3], SQL_NTS), "5");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[4], SQL_NTS), "4");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[5], SQL_NTS), "3");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[6], SQL_NTS), "2");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[7], SQL_NTS), "0");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[8], SQL_NTS),
                    "00000000-0000-0001-0000-000000000000");
  // Such format is used because Date returned as Timestamp.
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[9], SQL_NTS),
                    "1976-01-12 00:00:00");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[10], SQL_NTS),
                    "00:08:59");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[11], SQL_NTS),
                    "1978-08-21 23:13:45");

  BOOST_CHECK_EQUAL(columnLens[0], 1);
  BOOST_CHECK_EQUAL(columnLens[1], 1);
  BOOST_CHECK_EQUAL(columnLens[2], 1);
  BOOST_CHECK_EQUAL(columnLens[3], 1);
  BOOST_CHECK_EQUAL(columnLens[4], 1);
  BOOST_CHECK_EQUAL(columnLens[5], 1);
  BOOST_CHECK_EQUAL(columnLens[6], 1);
  BOOST_CHECK_EQUAL(columnLens[7], 1);
  BOOST_CHECK_EQUAL(columnLens[8], 36);
  BOOST_CHECK_EQUAL(columnLens[9], 19);
  BOOST_CHECK_EQUAL(columnLens[10], 8);
  BOOST_CHECK_EQUAL(columnLens[11], 19);

  ret = SQLFetch(stmt);
  BOOST_CHECK(ret == SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestOneRowString, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLRETURN ret;

  TestType in(1, 2, 3, 4, "5", 6.0f, 7.0, true, Guid(8, 9),
              MakeDateGmt(1987, 6, 5), MakeTimeGmt(12, 48, 12),
              MakeTimestampGmt(1998, 12, 27, 1, 2, 3, 456));

  const SQLSMALLINT columnsCnt = 12;

  SQLWCHAR columns[columnsCnt][ODBC_BUFFER_SIZE];

  SQLLEN columnLens[columnsCnt];

  // Binding columns.
  for (SQLSMALLINT i = 0; i < columnsCnt; ++i) {
    ret = SQLBindCol(stmt, i + 1, SQL_C_WCHAR, &columns[i], ODBC_BUFFER_SIZE,
                     &columnLens[i]);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  std::vector< SQLWCHAR > request = NewSqlWchar(
      L"SELECT i8Field, i16Field, i32Field, i64Field, strField, floatField, "
      L"doubleField, boolField, guidField, dateField, CAST('12:48:12' AS "
      L"TIME), timestampField FROM TestType");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[0], SQL_NTS),
                    "1");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[1], SQL_NTS),
                    "2");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[2], SQL_NTS),
                    "3");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[3], SQL_NTS),
                    "4");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[4], SQL_NTS),
                    "5");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[5], SQL_NTS),
                    "6");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[6], SQL_NTS),
                    "7");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[7], SQL_NTS),
                    "1");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[8], SQL_NTS),
                    "00000000-0000-0008-0000-000000000009");
  // Such format is used because Date returned as Timestamp.
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[9], SQL_NTS),
                    "1987-06-05 00:00:00");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[10], SQL_NTS),
                    "12:48:12");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[11], SQL_NTS),
                    "1998-12-27 01:02:03");

  BOOST_CHECK_EQUAL(columnLens[0], 1);
  BOOST_CHECK_EQUAL(columnLens[1], 1);
  BOOST_CHECK_EQUAL(columnLens[2], 1);
  BOOST_CHECK_EQUAL(columnLens[3], 1);
  BOOST_CHECK_EQUAL(columnLens[4], 1);
  BOOST_CHECK_EQUAL(columnLens[5], 1);
  BOOST_CHECK_EQUAL(columnLens[6], 1);
  BOOST_CHECK_EQUAL(columnLens[7], 1);
  BOOST_CHECK_EQUAL(columnLens[8], 36);
  BOOST_CHECK_EQUAL(columnLens[9], 19);
  BOOST_CHECK_EQUAL(columnLens[10], 8);
  BOOST_CHECK_EQUAL(columnLens[11], 19);

  ret = SQLFetch(stmt);
  BOOST_CHECK(ret == SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestOneRowStringLen, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLRETURN ret;

  TestType in(1, 2, 3, 4, "5", 6.0f, 7.0, true, Guid(8, 9),
              MakeDateGmt(1987, 6, 5), MakeTimeGmt(12, 48, 12),
              MakeTimestampGmt(1998, 12, 27, 1, 2, 3, 456));

  const SQLSMALLINT columnsCnt = 12;

  SQLLEN columnLens[columnsCnt];

  // Binding columns.
  for (SQLSMALLINT i = 0; i < columnsCnt; ++i) {
    ret = SQLBindCol(stmt, i + 1, SQL_C_WCHAR, 0, 0, &columnLens[i]);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  std::vector< SQLWCHAR > request = NewSqlWchar(
      L"SELECT i8Field, i16Field, i32Field, i64Field, strField, floatField, "
      L"doubleField, boolField, guidField, dateField, timeField, "
      L"timestampField FROM TestType");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(columnLens[0], 1);
  BOOST_CHECK_EQUAL(columnLens[1], 1);
  BOOST_CHECK_EQUAL(columnLens[2], 1);
  BOOST_CHECK_EQUAL(columnLens[3], 1);
  BOOST_CHECK_EQUAL(columnLens[4], 1);
  BOOST_CHECK_EQUAL(columnLens[5], 1);
  BOOST_CHECK_EQUAL(columnLens[6], 1);
  BOOST_CHECK_EQUAL(columnLens[7], 1);
  BOOST_CHECK_EQUAL(columnLens[8], 36);
  BOOST_CHECK_EQUAL(columnLens[9], 19);
  BOOST_CHECK_EQUAL(columnLens[10], 8);
  BOOST_CHECK_EQUAL(columnLens[11], 19);

  ret = SQLFetch(stmt);
  BOOST_CHECK(ret == SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestOneRowObject, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache2");

  SQLRETURN ret;

  ComplexType obj;

  obj.i32Field = 123;
  obj.strField = "Some string";

  obj.objField.f1 = 54321;
  obj.objField.f2 = "Hello Ignite";

  int64_t column1 = 0;
  int8_t column2[ODBC_BUFFER_SIZE];
  char column3[ODBC_BUFFER_SIZE];

  SQLLEN column1Len = sizeof(column1);
  SQLLEN column2Len = sizeof(column2);
  SQLLEN column3Len = sizeof(column3);

  // Binding columns.
  ret = SQLBindCol(stmt, 1, SQL_C_SLONG, &column1, column1Len, &column1Len);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindCol(stmt, 2, SQL_C_BINARY, &column2, column2Len, &column2Len);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindCol(stmt, 3, SQL_C_CHAR, &column3, column3Len, &column3Len);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  std::vector< SQLWCHAR > request =
      NewSqlWchar(L"SELECT i32Field, objField, strField FROM ComplexType");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(column1, obj.i32Field);
  BOOST_CHECK_EQUAL(column3, obj.strField.c_str());

  ret = SQLFetch(stmt);
  BOOST_CHECK(ret == SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestDataAtExecution, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLRETURN ret;

  TestType in1(1, 2, 3, 4, "5", 6.0f, 7.0, true, Guid(8, 9),
               MakeDateGmt(1987, 6, 5), MakeTimeGmt(12, 48, 12),
               MakeTimestampGmt(1998, 12, 27, 1, 2, 3, 456));

  TestType in2(8, 7, 6, 5, "4", 3.0f, 2.0, false, Guid(1, 0),
               MakeDateGmt(1976, 1, 12), MakeTimeGmt(0, 8, 59),
               MakeTimestampGmt(1978, 8, 21, 23, 13, 45, 999999999));

  const SQLSMALLINT columnsCnt = 12;

  SQLLEN columnLens[columnsCnt];
  SQLWCHAR columns[columnsCnt][ODBC_BUFFER_SIZE];

  // Binding columns.
  for (SQLSMALLINT i = 0; i < columnsCnt; ++i) {
    ret = SQLBindCol(stmt, i + 1, SQL_C_WCHAR, &columns[i], ODBC_BUFFER_SIZE,
                     &columnLens[i]);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  std::vector< SQLWCHAR > request = NewSqlWchar(
      L"SELECT i8Field, i16Field, i32Field, i64Field, strField, floatField, "
      L"doubleField, boolField, guidField, dateField, timeField, "
      L"timestampField FROM TestType "
      L"WHERE i32Field = ? AND strField = ?");

  ret = SQLPrepare(stmt, request.data(), SQL_NTS);

  SQLLEN ind1 = 1;
  SQLLEN ind2 = 2;

  SQLLEN len1 = SQL_DATA_AT_EXEC;
  SQLLEN len2 =
      SQL_LEN_DATA_AT_EXEC(static_cast< SQLLEN >(in1.strField.size()));

  ret = SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
                         100, 100, &ind1, sizeof(ind1), &len1);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR, 100,
                         100, &ind2, sizeof(ind2), &len2);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLExecute(stmt);

  BOOST_REQUIRE_EQUAL(ret, SQL_NEED_DATA);

  void* oind;

  ret = SQLParamData(stmt, &oind);

  BOOST_REQUIRE_EQUAL(ret, SQL_NEED_DATA);

  if (oind == &ind1)
    ret = SQLPutData(stmt, &in1.i32Field, 0);
  else if (oind == &ind2)
    ret = SQLPutData(stmt, (SQLPOINTER)in1.strField.c_str(),
                     (SQLLEN)in1.strField.size());
  else
    BOOST_FAIL("Unknown indicator value");

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLParamData(stmt, &oind);

  BOOST_REQUIRE_EQUAL(ret, SQL_NEED_DATA);

  if (oind == &ind1)
    ret = SQLPutData(stmt, &in1.i32Field, 0);
  else if (oind == &ind2)
    ret = SQLPutData(stmt, (SQLPOINTER)in1.strField.c_str(),
                     (SQLLEN)in1.strField.size());
  else
    BOOST_FAIL("Unknown indicator value");

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLParamData(stmt, &oind);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[0], SQL_NTS),
                    "1");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[1], SQL_NTS),
                    "2");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[2], SQL_NTS),
                    "3");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[3], SQL_NTS),
                    "4");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[4], SQL_NTS),
                    "5");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[5], SQL_NTS),
                    "6");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[6], SQL_NTS),
                    "7");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[7], SQL_NTS),
                    "1");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[8], SQL_NTS),
                    "00000000-0000-0008-0000-000000000009");
  // Such format is used because Date returned as Timestamp.
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[9], SQL_NTS),
                    "1987-06-05 00:00:00");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[10], SQL_NTS),
                    "12:48:12");
  BOOST_CHECK_EQUAL(utility::SqlStringToString(columns[11], SQL_NTS),
                    "1998-12-27 01:02:03");

  BOOST_CHECK_EQUAL(columnLens[0], 1);
  BOOST_CHECK_EQUAL(columnLens[1], 1);
  BOOST_CHECK_EQUAL(columnLens[2], 1);
  BOOST_CHECK_EQUAL(columnLens[3], 1);
  BOOST_CHECK_EQUAL(columnLens[4], 1);
  BOOST_CHECK_EQUAL(columnLens[5], 1);
  BOOST_CHECK_EQUAL(columnLens[6], 1);
  BOOST_CHECK_EQUAL(columnLens[7], 1);
  BOOST_CHECK_EQUAL(columnLens[8], 36);
  BOOST_CHECK_EQUAL(columnLens[9], 19);
  BOOST_CHECK_EQUAL(columnLens[10], 8);
  BOOST_CHECK_EQUAL(columnLens[11], 19);

  ret = SQLFetch(stmt);
  BOOST_CHECK(ret == SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestNullFields, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLRETURN ret;

  TestType in(1, 2, 3, 4, "5", 6.0f, 7.0, true, Guid(8, 9),
              MakeDateGmt(1987, 6, 5), MakeTimeGmt(12, 48, 12),
              MakeTimestampGmt(1998, 12, 27, 1, 2, 3, 456));

  TestType inNull;

  inNull.allNulls = true;

  const SQLSMALLINT columnsCnt = 11;

  SQLLEN columnLens[columnsCnt];

  int8_t i8Column;
  int16_t i16Column;
  int32_t i32Column;
  int64_t i64Column;
  wchar_t strColumn[ODBC_BUFFER_SIZE];
  float floatColumn;
  double doubleColumn;
  bool boolColumn;
  SQL_DATE_STRUCT dateColumn;
  SQL_TIME_STRUCT timeColumn;
  SQL_TIMESTAMP_STRUCT timestampColumn;

  // Binding columns.
  ret = SQLBindCol(stmt, 1, SQL_C_STINYINT, &i8Column, 0, &columnLens[0]);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindCol(stmt, 2, SQL_C_SSHORT, &i16Column, 0, &columnLens[1]);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindCol(stmt, 3, SQL_C_SLONG, &i32Column, 0, &columnLens[2]);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindCol(stmt, 4, SQL_C_SBIGINT, &i64Column, 0, &columnLens[3]);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindCol(stmt, 5, SQL_C_WCHAR, &strColumn, ODBC_BUFFER_SIZE,
                   &columnLens[4]);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindCol(stmt, 6, SQL_C_FLOAT, &floatColumn, 0, &columnLens[5]);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindCol(stmt, 7, SQL_C_DOUBLE, &doubleColumn, 0, &columnLens[6]);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindCol(stmt, 8, SQL_C_BIT, &boolColumn, 0, &columnLens[7]);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindCol(stmt, 9, SQL_C_TYPE_DATE, &dateColumn, 0, &columnLens[8]);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindCol(stmt, 10, SQL_C_TYPE_TIME, &timeColumn, 0, &columnLens[9]);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindCol(stmt, 11, SQL_C_TYPE_TIMESTAMP, &timestampColumn, 0,
                   &columnLens[10]);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  std::vector< SQLWCHAR > request = NewSqlWchar(
      L"SELECT i8Field, i16Field, i32Field, i64Field, strField, floatField, "
      L"doubleField, boolField, dateField, timeField, timestampField FROM "
      L"TestType "
      L"ORDER BY _key");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  // Fetching the first non-null row.
  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  // Checking that columns are not null.
  for (SQLSMALLINT i = 0; i < columnsCnt; ++i)
    BOOST_CHECK_NE(columnLens[i], SQL_NULL_DATA);

  // Fetching null row.
  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  // Checking that columns are null.
  for (SQLSMALLINT i = 0; i < columnsCnt; ++i)
    BOOST_CHECK_EQUAL(columnLens[i], SQL_NULL_DATA);

  // Fetching the last non-null row.
  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  // Checking that columns are not null.
  for (SQLSMALLINT i = 0; i < columnsCnt; ++i)
    BOOST_CHECK_NE(columnLens[i], SQL_NULL_DATA);

  ret = SQLFetch(stmt);
  BOOST_CHECK(ret == SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestDistributedJoins, *disabled()) {
  const int entriesNum = 1000;

  // Filling cache with data.
  for (int i = 0; i < entriesNum; ++i) {
    TestType entry;

    entry.i32Field = i;
    entry.i64Field = entriesNum - i - 1;
  }

  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLRETURN ret;

  const SQLSMALLINT columnsCnt = 2;

  SQLBIGINT columns[columnsCnt];

  // Binding colums.
  for (SQLSMALLINT i = 0; i < columnsCnt; ++i) {
    ret = SQLBindCol(stmt, i + 1, SQL_C_SLONG, &columns[i], 0, 0);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  std::vector< SQLWCHAR > request = NewSqlWchar(
      L"SELECT T0.i32Field, T1.i64Field FROM TestType AS T0 "
      L"INNER JOIN TestType AS T1 "
      L"ON (T0.i32Field = T1.i64Field)");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  int rowsNum = CountRows(stmt);

  BOOST_CHECK_GT(rowsNum, 0);
  BOOST_CHECK_LT(rowsNum, entriesNum);

  Disconnect();

  Connect(
      "DRIVER={Apache "
      "Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache;DISTRIBUTED_JOINS=true;");

  // Binding colums.
  for (SQLSMALLINT i = 0; i < columnsCnt; ++i) {
    ret = SQLBindCol(stmt, i + 1, SQL_C_SLONG, &columns[i], 0, 0);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  rowsNum = CountRows(stmt);

  BOOST_CHECK_EQUAL(rowsNum, entriesNum);
}

template < typename T >
void CheckObjectData(int8_t* data, int32_t len, T const& value) {
  InteropUnpooledMemory mem(len);
  mem.Length(len);
  memcpy(mem.Data(), data, len);

  BinaryObject obj(BinaryObjectImpl::FromMemory(mem, 0, 0));

  T actual = obj.Deserialize< T >();

  BOOST_CHECK_EQUAL(value, actual);
}

BOOST_AUTO_TEST_CASE(TestKeyVal, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache2");

  SQLRETURN ret;

  ComplexType obj;

  obj.i32Field = 123;
  obj.strField = "Some string";

  obj.objField.f1 = 54321;
  obj.objField.f2 = "Hello Ignite";

  //_key
  int64_t column1 = 0;
  //_val
  int8_t column2[ODBC_BUFFER_SIZE];
  // k
  int64_t column3 = 0;
  // v
  int8_t column4[ODBC_BUFFER_SIZE];
  // i32Field
  int64_t column5 = 0;
  // objField
  int8_t column6[ODBC_BUFFER_SIZE];
  // strField
  char column7[ODBC_BUFFER_SIZE];

  SQLLEN column1Len = sizeof(column1);
  SQLLEN column2Len = sizeof(column2);
  SQLLEN column3Len = sizeof(column3);
  SQLLEN column4Len = sizeof(column4);
  SQLLEN column5Len = sizeof(column5);
  SQLLEN column6Len = sizeof(column6);
  SQLLEN column7Len = sizeof(column7);

  // Binding columns.
  ret = SQLBindCol(stmt, 1, SQL_C_SLONG, &column1, column1Len, &column1Len);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindCol(stmt, 2, SQL_C_BINARY, &column2, column2Len, &column2Len);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindCol(stmt, 3, SQL_C_SLONG, &column3, column3Len, &column3Len);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindCol(stmt, 4, SQL_C_BINARY, &column4, column4Len, &column4Len);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindCol(stmt, 5, SQL_C_SLONG, &column5, column5Len, &column5Len);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindCol(stmt, 6, SQL_C_BINARY, &column6, column6Len, &column6Len);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindCol(stmt, 7, SQL_C_CHAR, &column7, column7Len, &column7Len);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  std::vector< SQLWCHAR > request = NewSqlWchar(
      L"SELECT _key, _val, k, v, i32Field, objField, strField FROM ComplexType");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(column1, 1);

  CheckObjectData(column2, static_cast< int32_t >(column2Len), obj);

  BOOST_CHECK_EQUAL(column3, 1);

  CheckObjectData(column4, static_cast< int32_t >(column4Len), obj);

  BOOST_CHECK_EQUAL(column5, obj.i32Field);

  CheckObjectData(column6, static_cast< int32_t >(column6Len), obj.objField);

  BOOST_CHECK_EQUAL(column7, obj.strField.c_str());

  ret = SQLFetch(stmt);
  BOOST_CHECK(ret == SQL_NO_DATA);

  ret = SQLFreeStmt(stmt, SQL_CLOSE);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  std::vector< SQLWCHAR > requestStar = NewSqlWchar(L"SELECT _key, _val, * FROM ComplexType");

  ret = SQLExecDirect(stmt, requestStar.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(column1, 1);

  CheckObjectData(column2, static_cast< int32_t >(column2Len), obj);

  BOOST_CHECK_EQUAL(column3, 1);

  CheckObjectData(column4, static_cast< int32_t >(column4Len), obj);

  BOOST_CHECK_EQUAL(column5, obj.i32Field);

  CheckObjectData(column6, static_cast< int32_t >(column6Len), obj.objField);

  BOOST_CHECK_EQUAL(column7, obj.strField.c_str());

  ret = SQLFetch(stmt);
  BOOST_CHECK(ret == SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestParamsNum, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  CheckParamsNum("SELECT * FROM TestType", 0);
  CheckParamsNum("SELECT * FROM TestType WHERE _key=?", 1);
  CheckParamsNum("SELECT * FROM TestType WHERE _key=? AND _val=?", 2);
  CheckParamsNum("INSERT INTO TestType(_key, strField) VALUES(1, 'some')", 0);
  CheckParamsNum("INSERT INTO TestType(_key, strField) VALUES(?, ?)", 2);
}

BOOST_AUTO_TEST_CASE(TestExecuteAfterCursorClose, *disabled()) {
  TestType in(1, 2, 3, 4, "5", 6.0f, 7.0, true, Guid(8, 9),
              MakeDateGmt(1987, 6, 5), MakeTimeGmt(12, 48, 12),
              MakeTimestampGmt(1998, 12, 27, 1, 2, 3, 456));

  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  int64_t key = 0;
  wchar_t strField[1024];
  SQLLEN strFieldLen = 0;

  // Binding columns.
  SQLRETURN ret = SQLBindCol(stmt, 1, SQL_C_SLONG, &key, 0, 0);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  // Binding columns.
  ret = SQLBindCol(stmt, 2, SQL_C_WCHAR, &strField, sizeof(strField),
                   &strFieldLen);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  // Just selecting everything to make sure everything is OK
  std::vector< SQLWCHAR > selectReq =
      NewSqlWchar(L"SELECT _key, strField FROM TestType");

  ret = SQLPrepare(stmt, selectReq.data(), SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLExecute(stmt);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLFreeStmt(stmt, SQL_CLOSE);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLExecute(stmt);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLFetch(stmt);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(key, 1);

  BOOST_CHECK_EQUAL(std::wstring(strField, strFieldLen), L"5");

  ret = SQLFetch(stmt);

  BOOST_CHECK_EQUAL(ret, SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestCloseNonFullFetch, *disabled()) {
  TestType in1;
  TestType in2;

  in1.strField = "str1";
  in2.strField = "str2";

  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  int64_t key = 0;
  wchar_t strField[1024];
  SQLLEN strFieldLen = 0;

  // Binding columns.
  SQLRETURN ret = SQLBindCol(stmt, 1, SQL_C_SLONG, &key, 0, 0);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  // Binding columns.
  ret = SQLBindCol(stmt, 2, SQL_C_WCHAR, &strField, sizeof(strField),
                   &strFieldLen);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  // Just selecting everything to make sure everything is OK
  std::vector< SQLWCHAR > selectReq =
      NewSqlWchar(L"SELECT _key, strField FROM TestType ORDER BY _key");

  ret = SQLExecDirect(stmt, selectReq.data(), SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLFetch(stmt);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(key, 1);
  BOOST_CHECK_EQUAL(std::wstring(strField, strFieldLen), L"str1");

  ret = SQLFreeStmt(stmt, SQL_CLOSE);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
}

BOOST_AUTO_TEST_CASE(TestBindNullParameter, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLLEN paramInd = SQL_NULL_DATA;

  // Binding NULL parameter.
  SQLRETURN ret = SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR,
                                   SQL_CHAR, 100, 100, 0, 0, &paramInd);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  // Just selecting everything to make sure everything is OK
  std::vector< SQLWCHAR > insertReq =
      NewSqlWchar(L"INSERT INTO TestType(_key, strField) VALUES(1, ?)");

  ret = SQLExecDirect(stmt, insertReq.data(), SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  // Unbindning parameter.
  ret = SQLFreeStmt(stmt, SQL_RESET_PARAMS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  // Selecting inserted column to make sure that everything is OK
  std::vector< SQLWCHAR > selectReq =
      NewSqlWchar(L"SELECT strField FROM TestType");

  wchar_t strField[1024];
  SQLLEN strFieldLen = 0;

  // Binding column.
  ret = SQLBindCol(stmt, 1, SQL_C_WCHAR, &strField, sizeof(strField),
                   &strFieldLen);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLExecDirect(stmt, selectReq.data(), SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLFetch(stmt);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(strFieldLen, SQL_NULL_DATA);
}

BOOST_AUTO_TEST_CASE(TestErrorMessage, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  // Just selecting everything to make sure everything is OK
  std::vector< SQLWCHAR > selectReq = NewSqlWchar(L"SELECT a FROM B");

  SQLRETURN ret = SQLExecDirect(stmt, selectReq.data(), SQL_NTS);

  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);

  std::string error = GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt);
  std::string pattern =
      "42000: Table \"B\" not found; SQL statement:\nSELECT a FROM B";

  if (error.substr(0, pattern.size()) != pattern)
    BOOST_FAIL("'" + error + "' does not match '" + pattern + "'");
}

BOOST_AUTO_TEST_CASE(TestLoginTimeout, *disabled()) {
  Prepare();

  SQLRETURN ret = SQLSetConnectAttr(dbc, SQL_ATTR_LOGIN_TIMEOUT,
                                    reinterpret_cast< SQLPOINTER >(1), 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  std::vector< SQLWCHAR > connectStr = NewSqlWchar(
      L"DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLWCHAR outstr[ODBC_BUFFER_SIZE];
  SQLSMALLINT outstrlen;

  // Connecting to ODBC server.
  ret = SQLDriverConnect(dbc, NULL, connectStr.data(), SQL_NTS, outstr,
                         sizeof(outstr), &outstrlen, SQL_DRIVER_COMPLETE);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_DBC, dbc));
}

BOOST_AUTO_TEST_CASE(TestLoginTimeoutFail, *disabled()) {
  Prepare();

  SQLRETURN ret = SQLSetConnectAttr(dbc, SQL_ATTR_LOGIN_TIMEOUT,
                                    reinterpret_cast< SQLPOINTER >(5), 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  std::vector< SQLWCHAR > connectStr = NewSqlWchar(
      L"DRIVER={Apache Ignite};ADDRESS=192.168.0.1:11120;SCHEMA=cache");

  SQLWCHAR outstr[ODBC_BUFFER_SIZE];
  SQLSMALLINT outstrlen;

  // Connecting to ODBC server.
  ret = SQLDriverConnect(dbc, NULL, connectStr.data(), SQL_NTS, outstr,
                         sizeof(outstr), &outstrlen, SQL_DRIVER_COMPLETE);

  if (SQL_SUCCEEDED(ret))
    BOOST_FAIL("Should timeout");
}

BOOST_AUTO_TEST_CASE(TestConnectionTimeoutQuery, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLRETURN ret = SQLSetConnectAttr(dbc, SQL_ATTR_CONNECTION_TIMEOUT,
                                    reinterpret_cast< SQLPOINTER >(5), 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  InsertTestStrings(10, false);
}

BOOST_AUTO_TEST_CASE(TestConnectionTimeoutBatch, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLRETURN ret = SQLSetConnectAttr(dbc, SQL_ATTR_CONNECTION_TIMEOUT,
                                    reinterpret_cast< SQLPOINTER >(5), 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  InsertTestBatch(11, 20, 9);
}

BOOST_AUTO_TEST_CASE(TestConnectionTimeoutBoth, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLRETURN ret = SQLSetConnectAttr(dbc, SQL_ATTR_CONNECTION_TIMEOUT,
                                    reinterpret_cast< SQLPOINTER >(5), 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  InsertTestStrings(10, false);
  InsertTestBatch(11, 20, 9);
}

BOOST_AUTO_TEST_CASE(TestQueryTimeoutQuery, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLRETURN ret = SQLSetStmtAttr(stmt, SQL_ATTR_QUERY_TIMEOUT,
                                 reinterpret_cast< SQLPOINTER >(5), 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  InsertTestStrings(10, false);
}

BOOST_AUTO_TEST_CASE(TestQueryTimeoutBatch, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLRETURN ret = SQLSetStmtAttr(stmt, SQL_ATTR_QUERY_TIMEOUT,
                                 reinterpret_cast< SQLPOINTER >(5), 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  InsertTestBatch(11, 20, 9);
}

BOOST_AUTO_TEST_CASE(TestQueryTimeoutBoth, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLRETURN ret = SQLSetStmtAttr(stmt, SQL_ATTR_QUERY_TIMEOUT,
                                 reinterpret_cast< SQLPOINTER >(5), 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  InsertTestStrings(10, false);
  InsertTestBatch(11, 20, 9);
}

BOOST_AUTO_TEST_CASE(TestQueryAndConnectionTimeoutQuery, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLRETURN ret = SQLSetStmtAttr(stmt, SQL_ATTR_QUERY_TIMEOUT,
                                 reinterpret_cast< SQLPOINTER >(5), 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLSetConnectAttr(dbc, SQL_ATTR_CONNECTION_TIMEOUT,
                          reinterpret_cast< SQLPOINTER >(3), 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  InsertTestStrings(10, false);
}

BOOST_AUTO_TEST_CASE(TestQueryAndConnectionTimeoutBatch, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLRETURN ret = SQLSetStmtAttr(stmt, SQL_ATTR_QUERY_TIMEOUT,
                                 reinterpret_cast< SQLPOINTER >(5), 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLSetConnectAttr(dbc, SQL_ATTR_CONNECTION_TIMEOUT,
                          reinterpret_cast< SQLPOINTER >(3), 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  InsertTestBatch(11, 20, 9);
}

BOOST_AUTO_TEST_CASE(TestQueryAndConnectionTimeoutBoth, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLRETURN ret = SQLSetStmtAttr(stmt, SQL_ATTR_QUERY_TIMEOUT,
                                 reinterpret_cast< SQLPOINTER >(5), 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLSetConnectAttr(dbc, SQL_ATTR_CONNECTION_TIMEOUT,
                          reinterpret_cast< SQLPOINTER >(3), 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  InsertTestStrings(10, false);
  InsertTestBatch(11, 20, 9);
}

BOOST_AUTO_TEST_CASE(TestSeveralInsertsWithoutClosing, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  std::vector< SQLWCHAR > request =
      NewSqlWchar(L"INSERT INTO TestType(_key, i32Field) VALUES(?, ?)");

  SQLRETURN ret = SQLPrepare(stmt, request.data(), SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  int64_t key = 0;
  ret = SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_SBIGINT, SQL_BIGINT, 0,
                         0, &key, 0, 0);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  int32_t data = 0;
  ret = SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0,
                         0, &data, 0, 0);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  for (int32_t i = 0; i < 10; ++i) {
    key = i;
    data = i * 10;

    ret = SQLExecute(stmt);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }
}

BOOST_AUTO_TEST_CASE(TestManyCursors, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  for (int32_t i = 0; i < 1000; ++i) {
    std::vector< SQLWCHAR > req = NewSqlWchar(L"SELECT 1");

    SQLRETURN ret = SQLExecDirect(stmt, req.data(), SQL_NTS);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    ret = SQLFreeStmt(stmt, SQL_CLOSE);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }
}

BOOST_AUTO_TEST_CASE(TestManyCursors2, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  SQLRETURN ret = SQLFreeHandle(SQL_HANDLE_STMT, stmt);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  for (int32_t i = 0; i < 1000; ++i) {
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    std::vector< SQLWCHAR > req = NewSqlWchar(L"SELECT 1");

    ret = SQLExecDirect(stmt, req.data(), SQL_NTS);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    int32_t res = 0;
    SQLLEN resLen = 0;
    ret = SQLBindCol(stmt, 1, SQL_INTEGER, &res, 0, &resLen);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    ret = SQLFetch(stmt);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    BOOST_REQUIRE_EQUAL(res, 1);

    ret = SQLFreeHandle(SQL_HANDLE_STMT, stmt);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    stmt = NULL;
  }
}

BOOST_AUTO_TEST_CASE(TestManyCursorsTwoSelects1, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  for (int32_t i = 0; i < 1000; ++i) {
    std::vector< SQLWCHAR > req = NewSqlWchar(L"SELECT 1; SELECT 2");

    SQLRETURN ret = SQLExecDirect(stmt, req.data(), SQL_NTS);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    ret = SQLFreeStmt(stmt, SQL_CLOSE);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }
}

BOOST_AUTO_TEST_CASE(TestManyCursorsTwoSelects2, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  for (int32_t i = 0; i < 1000; ++i) {
    std::vector< SQLWCHAR > req = NewSqlWchar(L"SELECT 1; SELECT 2;");

    SQLRETURN ret = SQLExecDirect(stmt, req.data(), SQL_NTS);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    ret = SQLMoreResults(stmt);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    ret = SQLFreeStmt(stmt, SQL_CLOSE);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }
}

BOOST_AUTO_TEST_CASE(TestManyCursorsSelectMerge1, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  for (int32_t i = 0; i < 1000; ++i) {
    std::vector< SQLWCHAR > req =
        NewSqlWchar(L"SELECT 1; MERGE into TestType(_key) values(2)");

    SQLRETURN ret = SQLExecDirect(stmt, req.data(), SQL_NTS);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    ret = SQLFreeStmt(stmt, SQL_CLOSE);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }
}

BOOST_AUTO_TEST_CASE(TestManyCursorsSelectMerge2, *disabled()) {
  Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

  for (int32_t i = 0; i < 1000; ++i) {
    std::vector< SQLWCHAR > req =
        NewSqlWchar(L"SELECT 1; MERGE into TestType(_key) values(2)");

    SQLRETURN ret = SQLExecDirect(stmt, req.data(), SQL_NTS);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    ret = SQLMoreResults(stmt);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    ret = SQLFreeStmt(stmt, SQL_CLOSE);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }
}

BOOST_AUTO_TEST_CASE(TestSingleResultUsingGetDataWideChar) {
  std::string dsnConnectionString;
  CreateDsnConnectionStringForLocalServer(dsnConnectionString);
  Connect(dsnConnectionString);
  SQLRETURN ret;
  std::vector< SQLWCHAR > request =
      NewSqlWchar(L"SELECT fieldString FROM \"queries_test_004\"");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  const int32_t buf_size = 1024;
  SQLWCHAR fieldString[buf_size]{};
  SQLLEN fieldString_len = 0;

  // Fetch 1st row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, fieldString, sizeof(fieldString),
                   &fieldString_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_NE(SQL_NULL_DATA, fieldString_len);
  BOOST_CHECK_EQUAL(std::wstring(L"你好"), std::wstring((wchar_t*)fieldString));

  // Fetch 2nd row - not exist
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
}

BOOST_AUTO_TEST_CASE(TestSingleResultSelectWideCharUsingGetDataWideChar) {
  std::string dsnConnectionString;
  CreateDsnConnectionStringForLocalServer(dsnConnectionString);
  Connect(dsnConnectionString);
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = NewSqlWchar(
      L"SELECT fieldString FROM \"queries_test_004\" WHERE fieldString = "
      L"'你好'");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  const int32_t buf_size = 1024;
  SQLWCHAR fieldString[buf_size]{};
  SQLLEN fieldString_len = 0;

  // Fetch 1st row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, fieldString, sizeof(fieldString),
                   &fieldString_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_NE(SQL_NULL_DATA, fieldString_len);
  BOOST_CHECK_EQUAL(std::wstring(L"你好"), std::wstring((wchar_t*)fieldString));

  // Fetch 2nd row - not exist
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
}

BOOST_AUTO_TEST_CASE(TestSingleResultSelectWideCharUsingGetDataNarrowChar) {
  std::string dsnConnectionString;
  CreateDsnConnectionStringForLocalServer(dsnConnectionString);
  Connect(dsnConnectionString);
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = NewSqlWchar(
      L"SELECT fieldString FROM \"queries_test_004\" WHERE fieldString = "
      L"'你好'");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  const int32_t buf_size = 1024;
  SQLCHAR fieldString[buf_size]{};
  SQLLEN fieldString_len = 0;

  // Fetch 1st row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetData(stmt, 1, SQL_C_CHAR, fieldString, sizeof(fieldString),
                   &fieldString_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_NE(SQL_NULL_DATA, fieldString_len);
  BOOST_CHECK_EQUAL(std::string("??"), std::string((char*)fieldString));

  // Fetch 2nd row - not exist
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
}

BOOST_AUTO_TEST_SUITE_END()
