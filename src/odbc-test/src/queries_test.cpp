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
#include "ignite/odbc/binary/binary_object.h"
#include "ignite/odbc/common/fixed_size_array.h"
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

  template < typename T >
  void CheckTwoRowsInt(SQLSMALLINT type) {
    connectToLocalServer("odbc-test");

    SQLRETURN ret;

    const SQLSMALLINT columnsCnt = 6;

    T columns[columnsCnt];

    std::memset(&columns, 0, sizeof(columns));

    // Binding columns.
    for (SQLSMALLINT i = 0; i < columnsCnt; ++i) {
      ret = SQLBindCol(stmt, i + 1, type, &columns[i], sizeof(columns[i]),
                       nullptr);

      if (!SQL_SUCCEEDED(ret))
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    }

    std::vector< SQLWCHAR > request = MakeSqlBuffer(
        "SELECT fieldInt, fieldLong, fieldDecimal128, fieldDouble,"
        "fieldString, fieldBoolean FROM queries_test_005 order by fieldInt");

    ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
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
    BOOST_CHECK_EQUAL(columns[5], 1);

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

    BOOST_CHECK_EQUAL(columns[0], 6);
    BOOST_CHECK_EQUAL(columns[1], 5);
    BOOST_CHECK_EQUAL(columns[2], 4);
    BOOST_CHECK_EQUAL(columns[3], 3);
    BOOST_CHECK_EQUAL(columns[4], 2);
    BOOST_CHECK_EQUAL(columns[5], 1);

    BOOST_CHECK_EQUAL(columnLens[0], static_cast< SQLLEN >(sizeof(T)));
    BOOST_CHECK_EQUAL(columnLens[1], static_cast< SQLLEN >(sizeof(T)));
    BOOST_CHECK_EQUAL(columnLens[2], static_cast< SQLLEN >(sizeof(T)));
    BOOST_CHECK_EQUAL(columnLens[3], static_cast< SQLLEN >(sizeof(T)));
    BOOST_CHECK_EQUAL(columnLens[4], static_cast< SQLLEN >(sizeof(T)));
    BOOST_CHECK_EQUAL(columnLens[5], static_cast< SQLLEN >(sizeof(T)));

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

BOOST_AUTO_TEST_CASE(TestSingleResultUsingGetData) {
  std::string dsnConnectionString;
  CreateDsnConnectionStringForLocalServer(dsnConnectionString);
  Connect(dsnConnectionString);
  SQLRETURN ret;
  std::vector< SQLWCHAR > request =
      MakeSqlBuffer("SELECT * FROM \"queries_test_001\"");

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
  ret = SQLGetData(stmt, 2, SQL_C_WCHAR, fieldDecimal128,
                   sizeof(fieldDecimal128), &fieldDecimal128_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLGetData(stmt, 3, SQL_C_DOUBLE, &fieldDouble, sizeof(fieldDouble),
                   &fieldDouble_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLGetData(stmt, 4, SQL_C_WCHAR, fieldString, sizeof(fieldString),
                   &fieldString_len);
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
  ret = SQLGetData(stmt, 12, SQL_C_WCHAR, fieldNull, sizeof(fieldNull),
                   &fieldNull_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLGetData(stmt, 13, SQL_C_BINARY, fieldBinary, sizeof(fieldBinary),
                   &fieldBinary_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_NE(SQL_NULL_DATA, id_len);
  BOOST_CHECK_EQUAL("62196dcc4d91892191475139",
                    utility::SqlWcharToString(id, id_len, true));
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldDecimal128_len);
  BOOST_CHECK_EQUAL(
      "340282350000000000000",
      utility::SqlWcharToString(fieldDecimal128, fieldDecimal128_len, true));
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldDouble_len);
  BOOST_CHECK_EQUAL(1.7976931348623157e308, fieldDouble);
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldString_len);
  BOOST_CHECK_EQUAL("some Text", utility::SqlWcharToString(
                                     fieldString, fieldString_len, true));
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldObjectId_len);
  BOOST_CHECK_EQUAL(
      "62196dcc4d9189219147513a",
      utility::SqlWcharToString(fieldObjectId, fieldObjectId_len, true));
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
  BOOST_CHECK_EQUAL(
      "MAXKEY", utility::SqlWcharToString(fieldMaxKey, fieldMaxKey_len, true));
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldMinKey_len);
  BOOST_CHECK_EQUAL(
      "MINKEY", utility::SqlWcharToString(fieldMinKey, fieldMinKey_len, true));
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
  std::vector< SQLWCHAR > request =
      MakeSqlBuffer("SELECT * FROM \"queries_test_001\"");

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

  ret = SQLBindCol(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLBindCol(stmt, 2, SQL_C_WCHAR, fieldDecimal128,
                   sizeof(fieldDecimal128), &fieldDecimal128_len);
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
  ret = SQLBindCol(stmt, 8, SQL_C_SLONG, &fieldInt, sizeof(fieldInt),
                   &fieldInt_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLBindCol(stmt, 9, SQL_C_SBIGINT, &fieldLong, sizeof(fieldLong),
                   &fieldLong_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLBindCol(stmt, 10, SQL_C_WCHAR, fieldMaxKey, sizeof(fieldMaxKey),
                   &fieldMaxKey_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLBindCol(stmt, 11, SQL_C_WCHAR, fieldMinKey, sizeof(fieldMinKey),
                   &fieldMinKey_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLBindCol(stmt, 12, SQL_C_WCHAR, fieldNull, sizeof(fieldNull),
                   &fieldNull_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLBindCol(stmt, 13, SQL_C_BINARY, fieldBinary, sizeof(fieldBinary),
                   &fieldBinary_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  // Fetch 1st row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_NE(SQL_NULL_DATA, id_len);
  BOOST_CHECK_EQUAL("62196dcc4d91892191475139",
                    utility::SqlWcharToString(id, id_len, true));
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldDecimal128_len);
  BOOST_CHECK_EQUAL(
      "340282350000000000000",
      utility::SqlWcharToString(fieldDecimal128, fieldDecimal128_len, true));
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldDouble_len);
  BOOST_CHECK_EQUAL(1.7976931348623157e308, fieldDouble);
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldString_len);
  BOOST_CHECK_EQUAL("some Text", utility::SqlWcharToString(
                                     fieldString, fieldString_len, true));
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldObjectId_len);
  BOOST_CHECK_EQUAL(
      "62196dcc4d9189219147513a",
      utility::SqlWcharToString(fieldObjectId, fieldObjectId_len, true));
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
  BOOST_CHECK_EQUAL(
      "MAXKEY", utility::SqlWcharToString(fieldMaxKey, fieldMaxKey_len, true));
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldMinKey_len);
  BOOST_CHECK_EQUAL(
      "MINKEY", utility::SqlWcharToString(fieldMinKey, fieldMinKey_len, true));
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
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "SELECT * FROM \"queries_test_002\" ORDER BY \"queries_test_002__id\"");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
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
  ret = SQLGetData(stmt, 2, SQL_C_WCHAR, fieldDecimal128,
                   sizeof(fieldDecimal128), &fieldDecimal128_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_NE(SQL_NULL_DATA, id_len);
  BOOST_CHECK_EQUAL("62196dcc4d91892191475139",
                    utility::SqlWcharToString(id, id_len, true));
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldDecimal128_len);
  BOOST_CHECK_EQUAL(
      "340282350000000000000",
      utility::SqlWcharToString(fieldDecimal128, fieldDecimal128_len, true));

  // Fetch 2nd row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLGetData(stmt, 2, SQL_C_WCHAR, fieldDecimal128,
                   sizeof(fieldDecimal128), &fieldDecimal128_len);

  BOOST_CHECK_NE(SQL_NULL_DATA, id_len);
  BOOST_CHECK_EQUAL("62196dcc4d9189219147513a",
                    utility::SqlWcharToString(id, id_len, true));
  BOOST_CHECK_EQUAL(SQL_NULL_DATA, fieldDecimal128_len);

  // Fetch 3rd row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, id, sizeof(id), &id_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLGetData(stmt, 2, SQL_C_WCHAR, fieldDecimal128,
                   sizeof(fieldDecimal128), &fieldDecimal128_len);

  BOOST_CHECK_NE(SQL_NULL_DATA, id_len);
  BOOST_CHECK_EQUAL("62196dcc4d9189219147513b",
                    utility::SqlWcharToString(id, id_len, true));
  BOOST_CHECK_NE(SQL_NULL_DATA, fieldDecimal128_len);
  BOOST_CHECK_EQUAL(
      "340282350000000000000",
      utility::SqlWcharToString(fieldDecimal128, fieldDecimal128_len, true));

  // Fetch 4th row - not exist
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
}

BOOST_AUTO_TEST_CASE(TestArrayStructJoinUsingGetData) {
  std::string dsnConnectionString;
  CreateDsnConnectionStringForLocalServer(dsnConnectionString);
  Connect(dsnConnectionString);
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "SELECT q3.queries_test_003__id, \n"
      "  \"a1\".\"value\", \n"
      "  \"a2\".\"value\", \n"
      "  \"d1\".\"field1\", \n"
      "  \"d2\".\"field2\" \n"
      " FROM queries_test_003 AS q3 \n"
      " JOIN queries_test_003_fieldArrayOfInt AS a1 \n"
      "  ON q3.queries_test_003__id = a1.queries_test_003__id \n"
      " JOIN queries_test_003_fieldArrayOfString AS a2 \n"
      "  ON a1.queries_test_003__id = a2.queries_test_003__id \n"
      " JOIN queries_test_003_fieldDocument AS d1 \n"
      "  ON a2.queries_test_003__id = d1.queries_test_003__id \n"
      " JOIN queries_test_003_fieldDocument_subDoc AS d2 \n"
      "  ON d1.queries_test_003__id = d2.queries_test_003__id");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
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
  ret = SQLGetData(stmt, 2, SQL_C_SLONG, &a1_value, sizeof(a1_value),
                   &a1_value_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLGetData(stmt, 3, SQL_C_WCHAR, a2_value, sizeof(a2_value),
                   &a2_value_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLGetData(stmt, 4, SQL_C_WCHAR, d1_value, sizeof(d1_value),
                   &d1_value_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);
  ret = SQLGetData(stmt, 5, SQL_C_WCHAR, d2_value, sizeof(d2_value),
                   &d2_value_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  // Check the first row
  BOOST_CHECK_NE(SQL_NULL_DATA, id_len);
  BOOST_CHECK_EQUAL("62196dcc4d91892191475139",
                    utility::SqlWcharToString(id, id_len, true));
  BOOST_CHECK_NE(SQL_NULL_DATA, a1_value_len);
  BOOST_CHECK_EQUAL(1, a1_value);
  BOOST_CHECK_NE(SQL_NULL_DATA, a2_value_len);
  BOOST_CHECK_EQUAL("value1",
                    utility::SqlWcharToString(a2_value, a2_value_len, true));
  BOOST_CHECK_NE(SQL_NULL_DATA, d1_value_len);
  BOOST_CHECK_EQUAL("field1 Value",
                    utility::SqlWcharToString(d1_value, d1_value_len, true));
  BOOST_CHECK_NE(SQL_NULL_DATA, d2_value_len);
  BOOST_CHECK_EQUAL("field2 Value",
                    utility::SqlWcharToString(d2_value, d2_value_len, true));

  // Count the rows
  int32_t actual_rows = 0;
  while (SQL_SUCCEEDED(ret)) {
    actual_rows++;
    ret = SQLFetch(stmt);
  }
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
  BOOST_CHECK_EQUAL(9, actual_rows);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsInt8) {
  CheckTwoRowsInt< signed char >(SQL_C_STINYINT);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsUint8) {
  CheckTwoRowsInt< unsigned char >(SQL_C_UTINYINT);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsInt16) {
  CheckTwoRowsInt< signed short >(SQL_C_SSHORT);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsUint16) {
  CheckTwoRowsInt< unsigned short >(SQL_C_USHORT);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsInt32) {
  CheckTwoRowsInt< SQLINTEGER >(SQL_C_SLONG);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsUint32) {
  CheckTwoRowsInt< SQLUINTEGER >(SQL_C_ULONG);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsInt64) {
  CheckTwoRowsInt< int64_t >(SQL_C_SBIGINT);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsUint64) {
  CheckTwoRowsInt< uint64_t >(SQL_C_UBIGINT);
}

BOOST_AUTO_TEST_CASE(TestTwoRowsString) {
  connectToLocalServer("odbc-test");

  SQLRETURN ret;

  const SQLSMALLINT columnsCnt = 6;

  SQLWCHAR columns[columnsCnt][ODBC_BUFFER_SIZE];

  // Binding columns.
  for (SQLSMALLINT i = 0; i < columnsCnt; ++i) {
    ret = SQLBindCol(stmt, i + 1, SQL_C_WCHAR, &columns[i],
                     ODBC_BUFFER_SIZE * sizeof(SQLWCHAR), 0);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "SELECT fieldInt, fieldLong, fieldDecimal128, fieldDouble,"
      "fieldString, fieldBoolean FROM queries_test_005 order by fieldInt");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLFetch(stmt);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[0], SQL_NTS, true), "1");
  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[1], SQL_NTS, true), "2");
  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[2], SQL_NTS, true), "3");
  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[3], SQL_NTS, true), "4");
  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[4], SQL_NTS, true), "5");
  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[5], SQL_NTS, true), "1");
  
  SQLLEN columnLens[columnsCnt];

  // Binding columns.
  for (SQLSMALLINT i = 0; i < columnsCnt; ++i) {
    ret = SQLBindCol(stmt, i + 1, SQL_C_WCHAR, &columns[i],
                     ODBC_BUFFER_SIZE * sizeof(SQLWCHAR), &columnLens[i]);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[0], SQL_NTS, true), "6");
  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[1], SQL_NTS, true), "5");
  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[2], SQL_NTS, true), "4");
  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[3], SQL_NTS, true), "3");
  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[4], SQL_NTS, true), "2");
  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[5], SQL_NTS, true), "1");

#ifdef __APPLE__
  SQLLEN expectedLen = 4;
#else
  SQLLEN expectedLen = 2;
#endif

  BOOST_CHECK_EQUAL(columnLens[0], expectedLen);
  BOOST_CHECK_EQUAL(columnLens[1], expectedLen);
  BOOST_CHECK_EQUAL(columnLens[2], expectedLen);
  BOOST_CHECK_EQUAL(columnLens[3], expectedLen);
  BOOST_CHECK_EQUAL(columnLens[4], expectedLen);
  BOOST_CHECK_EQUAL(columnLens[5], expectedLen);

  ret = SQLFetch(stmt);
  BOOST_CHECK(ret == SQL_NO_DATA);
}

// TODO: Memory leak, traced by https://bitquill.atlassian.net/browse/AD-813
BOOST_AUTO_TEST_CASE(TestOneRowObject, *disabled()) {
  connectToLocalServer("odbc-test");

  SQLRETURN ret;

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

  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "SELECT fieldInt, fieldBinary, fieldString FROM queries_test_005 order "
      "by fieldInt");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLFetch(stmt);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(column1, 1);
  BOOST_CHECK_EQUAL(column2[0], 0);
  BOOST_CHECK_EQUAL(column2[1], 1);
  BOOST_CHECK_EQUAL(column2[2], 2);
  BOOST_CHECK_EQUAL(column3, "5");
}

// Enable this testcase when AD-549 is finished
BOOST_AUTO_TEST_CASE(TestDataAtExecution, *disabled()) {
  connectToLocalServer("odbc-test");

  SQLRETURN ret;

  const SQLSMALLINT columnsCnt = 6;

  SQLLEN columnLens[columnsCnt];
  SQLWCHAR columns[columnsCnt][ODBC_BUFFER_SIZE];

  // Binding columns.
  for (SQLSMALLINT i = 0; i < columnsCnt; ++i) {
    ret = SQLBindCol(stmt, i + 1, SQL_C_WCHAR, &columns[i], ODBC_BUFFER_SIZE,
                     &columnLens[i]);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "SELECT fieldInt, fieldLong, fieldDecimal128, fieldDouble,"
      "fieldString, fieldBoolean FROM queries_test_005 "
      "where fieldInt = ?");

  ret = SQLPrepare(stmt, request.data(), SQL_NTS);

  SQLLEN ind1 = 1;

  SQLLEN len1 = SQL_DATA_AT_EXEC;

  ret = SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
                         100, 100, &ind1, sizeof(ind1), &len1);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLExecute(stmt);

  BOOST_REQUIRE_EQUAL(ret, SQL_NEED_DATA);

  void* oind;

  ret = SQLParamData(stmt, &oind);

  BOOST_REQUIRE_EQUAL(ret, SQL_NEED_DATA);

  int value = 1;
  if (oind == &ind1)
    ret = SQLPutData(stmt, &value, 0);
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

  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[0], SQL_NTS, true), "1");
  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[1], SQL_NTS, true), "2");
  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[2], SQL_NTS, true), "3");
  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[3], SQL_NTS, true), "4");
  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[4], SQL_NTS, true), "5");
  BOOST_CHECK_EQUAL(utility::SqlWcharToString(columns[5], SQL_NTS, true), "6");

  BOOST_CHECK_EQUAL(columnLens[0], 1);
  BOOST_CHECK_EQUAL(columnLens[1], 1);
  BOOST_CHECK_EQUAL(columnLens[2], 1);
  BOOST_CHECK_EQUAL(columnLens[3], 1);
  BOOST_CHECK_EQUAL(columnLens[4], 1);
  BOOST_CHECK_EQUAL(columnLens[5], 1);

  ret = SQLFetch(stmt);
  BOOST_CHECK(ret == SQL_NO_DATA);
}

BOOST_AUTO_TEST_CASE(TestNullFields) {
  connectToLocalServer("odbc-test");

  SQLRETURN ret;

  const SQLSMALLINT columnsCnt = 7;

  SQLLEN columnLens[columnsCnt];

  int32_t intColumn;
  int64_t longColumn;
  double decimalColumn;
  double doubleColumn;
  char idColumn[ODBC_BUFFER_SIZE];
  char strColumn[ODBC_BUFFER_SIZE];
  bool boolColumn;

  // Binding columns.
  ret = SQLBindCol(stmt, 1, SQL_C_CHAR, &idColumn, ODBC_BUFFER_SIZE,
                   &columnLens[0]);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindCol(stmt, 2, SQL_C_SLONG, &intColumn, 0, &columnLens[1]);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindCol(stmt, 3, SQL_C_SBIGINT, &longColumn, 0, &columnLens[2]);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindCol(stmt, 4, SQL_C_DOUBLE, &decimalColumn, 0, &columnLens[3]);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindCol(stmt, 5, SQL_C_DOUBLE, &doubleColumn, 0, &columnLens[4]);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindCol(stmt, 6, SQL_C_CHAR, &strColumn, ODBC_BUFFER_SIZE,
                   &columnLens[5]);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLBindCol(stmt, 7, SQL_C_BIT, &boolColumn, 0, &columnLens[6]);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      "SELECT queries_test_002__id, fieldInt, fieldLong, fieldDecimal128, "
      "fieldDouble,"
      "fieldString, fieldBoolean FROM queries_test_002 order by "
      "queries_test_002__id");

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
  for (SQLSMALLINT i = 1; i < columnsCnt; ++i)
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

// Maybe needed for TS
BOOST_AUTO_TEST_CASE(TestParamsNum, *disabled()) {
  connectToLocalServer("odbc-test");

  CheckParamsNum("SELECT * FROM TestType", 0);
  CheckParamsNum("SELECT * FROM TestType WHERE _key=?", 1);
  CheckParamsNum("SELECT * FROM TestType WHERE _key=? AND _val=?", 2);
}

BOOST_AUTO_TEST_CASE(TestExecuteAfterCursorClose) {
  connectToLocalServer("odbc-test");

  int64_t key = 0;
  SQLWCHAR strField[1024];
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
  std::vector< SQLWCHAR > selectReq = MakeSqlBuffer(
      "SELECT fieldInt, fieldString FROM queries_test_005 where fieldInt=1");

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

  BOOST_CHECK_EQUAL(utility::SqlWcharToString(strField, strFieldLen, true),
                    "5");

  ret = SQLFetch(stmt);

  BOOST_CHECK_EQUAL(ret, SQL_NO_DATA);
}

// TODO: Memory leak, traced by https://bitquill.atlassian.net/browse/AD-813
BOOST_AUTO_TEST_CASE(TestCloseNonFullFetch, *disabled()) {
  connectToLocalServer("odbc-test");

  int64_t key = 0;
  SQLWCHAR strField[1024];
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
  std::vector< SQLWCHAR > selectReq = MakeSqlBuffer(
      "SELECT fieldInt, fieldString FROM queries_test_005 where fieldInt=1");

  ret = SQLExecDirect(stmt, selectReq.data(), SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLFetch(stmt);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  BOOST_CHECK_EQUAL(key, 1);

  BOOST_CHECK_EQUAL(utility::SqlWcharToString(strField, strFieldLen, true),
                    "5");
  ret = SQLFreeStmt(stmt, SQL_CLOSE);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
}

BOOST_AUTO_TEST_CASE(TestErrorMessage) {
  connectToLocalServer("odbc-test");

  // Just selecting everything to make sure everything is OK
  std::vector< SQLWCHAR > selectReq = MakeSqlBuffer("SELECT A FROM B");

  SQLRETURN ret = SQLExecDirect(stmt, selectReq.data(), SQL_NTS);

  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);

  std::string error = GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt);
  std::string pattern = "Object \"B\" not found";

  if (error.find(pattern) != std::string::npos)
    BOOST_FAIL("'" + error + "' does not match '" + pattern + "'");
}

BOOST_AUTO_TEST_CASE(TestLoginTimeout) {
  Prepare();

  std::string dsnConnectionString;
  CreateDsnConnectionStringForLocalServer(dsnConnectionString, "odbc-test");

  SQLRETURN ret = SQLSetConnectAttr(dbc, SQL_ATTR_LOGIN_TIMEOUT,
                                    reinterpret_cast< SQLPOINTER >(1), 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  std::vector< SQLWCHAR > connectStr(dsnConnectionString.begin(),
                                     dsnConnectionString.end());

  SQLWCHAR outstr[ODBC_BUFFER_SIZE];
  SQLSMALLINT outstrlen;

  // Connecting to ODBC server.
  ret = SQLDriverConnect(dbc, NULL, &connectStr[0],
                         static_cast< SQLSMALLINT >(connectStr.size()), outstr,
                         sizeof(outstr), &outstrlen, SQL_DRIVER_COMPLETE);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_DBC, dbc));
}

// Enable this test after https://bitquill.atlassian.net/browse/AD-599 is
// finished
BOOST_AUTO_TEST_CASE(TestConnectionTimeoutFail, *disabled()) {
  Prepare();

  std::string dsnConnectionString;
  // pass a wrong port number to make the connect action timeout
  CreateDsnConnectionStringForLocalServer(dsnConnectionString, "", "", "",
                                          "27018");
  // The connection timeout value is set but not applied when driver connects
  SQLRETURN ret = SQLSetConnectAttr(dbc, SQL_ATTR_CONNECTION_TIMEOUT,
                                    reinterpret_cast< SQLPOINTER >(5), 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  std::vector< SQLWCHAR > connectStr(dsnConnectionString.begin(),
                                     dsnConnectionString.end());

  SQLWCHAR outstr[ODBC_BUFFER_SIZE];
  SQLSMALLINT outstrlen;

  // Connecting to ODBC server.
  ret = SQLDriverConnect(dbc, NULL, &connectStr[0],
                         static_cast< SQLSMALLINT >(connectStr.size()), outstr,
                         sizeof(outstr), &outstrlen, SQL_DRIVER_COMPLETE);

  BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);

  std::string error = GetOdbcErrorMessage(SQL_HANDLE_DBC, dbc);
  std::string pattern = "Timed out after 5000 ms while waiting to connect";

  if (error.find(pattern) == std::string::npos)
    BOOST_FAIL("'" + error + "' does not match '" + pattern + "'");
}

BOOST_AUTO_TEST_CASE(TestConnectionTimeoutQuery) {
  connectToLocalServer("odbc-test");

  SQLRETURN ret = SQLSetConnectAttr(dbc, SQL_ATTR_CONNECTION_TIMEOUT,
                                    reinterpret_cast< SQLPOINTER >(5), 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  std::vector< SQLWCHAR > selectReq =
      MakeSqlBuffer("SELECT * FROM queries_test_005");

  ret = SQLExecDirect(stmt, selectReq.data(), selectReq.size());

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
}

BOOST_AUTO_TEST_CASE(TestQueryTimeoutQuery) {
  connectToLocalServer("odbc-test");

  SQLRETURN ret = SQLSetStmtAttr(stmt, SQL_ATTR_QUERY_TIMEOUT,
                                 reinterpret_cast< SQLPOINTER >(5), 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  std::vector< SQLWCHAR > selectReq =
      MakeSqlBuffer("SELECT * FROM queries_test_005");

  ret = SQLExecDirect(stmt, selectReq.data(), selectReq.size());

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
}

BOOST_AUTO_TEST_CASE(TestQueryAndConnectionTimeoutQuery) {
  connectToLocalServer("odbc-test");

  SQLRETURN ret = SQLSetStmtAttr(stmt, SQL_ATTR_QUERY_TIMEOUT,
                                 reinterpret_cast< SQLPOINTER >(5), 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

  ret = SQLSetConnectAttr(dbc, SQL_ATTR_CONNECTION_TIMEOUT,
                          reinterpret_cast< SQLPOINTER >(3), 0);

  ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_DBC, dbc);

  std::vector< SQLWCHAR > selectReq =
      MakeSqlBuffer("SELECT * FROM queries_test_005");

  ret = SQLExecDirect(stmt, selectReq.data(), selectReq.size());

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
}

BOOST_AUTO_TEST_CASE(TestManyCursors) {
  connectToLocalServer("odbc-test");

  for (int32_t i = 0; i < 1000; ++i) {
    std::vector< SQLWCHAR > req =
        MakeSqlBuffer("SELECT * FROM queries_test_005");

    SQLRETURN ret = SQLExecDirect(stmt, req.data(), SQL_NTS);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    ret = SQLFreeStmt(stmt, SQL_CLOSE);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }
}

// TODO: Memory leak, traced by https://bitquill.atlassian.net/browse/AD-813
BOOST_AUTO_TEST_CASE(TestManyCursors2, *disabled()) {
  connectToLocalServer("odbc-test");

  SQLRETURN ret = SQLFreeHandle(SQL_HANDLE_STMT, stmt);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  for (int32_t i = 0; i < 1000; ++i) {
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    std::vector< SQLWCHAR > req =
        MakeSqlBuffer("SELECT fieldInt, fieldString FROM queries_test_005");

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

// Needed for TS?
BOOST_AUTO_TEST_CASE(TestManyCursorsTwoSelects1, *disabled()) {
  connectToLocalServer("odbc-test");

  for (int32_t i = 0; i < 1000; ++i) {
    std::vector< SQLWCHAR > req = MakeSqlBuffer(
        "SELECT * FROM queries_test_005; SELECT * FROM queries_test_004");

    SQLRETURN ret = SQLExecDirect(stmt, req.data(), SQL_NTS);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    ret = SQLFreeStmt(stmt, SQL_CLOSE);

    if (!SQL_SUCCEEDED(ret))
      BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }
}

// Needed for TS?
BOOST_AUTO_TEST_CASE(TestManyCursorsTwoSelects2, *disabled()) {
  connectToLocalServer("odbc-test");

  for (int32_t i = 0; i < 1000; ++i) {
    std::vector< SQLWCHAR > req = MakeSqlBuffer(
        "SELECT * FROM queries_test_005; SELECT * FROM queries_test_004");

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
      MakeSqlBuffer("SELECT fieldString FROM \"queries_test_004\"");

  ret = SQLExecDirect(stmt, request.data(), SQL_NTS);
  if (!SQL_SUCCEEDED(ret)) {
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
  }

  SQLWCHAR fieldString[1024]{};
  SQLLEN fieldString_len = 0;

  // Fetch 1st row
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  ret = SQLGetData(stmt, 1, SQL_C_WCHAR, fieldString, sizeof(fieldString),
                   &fieldString_len);
  BOOST_CHECK_EQUAL(SQL_SUCCESS, ret);

  BOOST_CHECK_NE(SQL_NULL_DATA, fieldString_len);
  BOOST_CHECK_EQUAL(
      u8"你好", utility::SqlWcharToString(fieldString, fieldString_len, true));

  // Fetch 2nd row - not exist
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
}

BOOST_AUTO_TEST_CASE(TestSingleResultSelectWideCharUsingGetDataWideChar) {
  std::string dsnConnectionString;
  CreateDsnConnectionStringForLocalServer(dsnConnectionString);
  Connect(dsnConnectionString);
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      u8"SELECT fieldString FROM \"queries_test_004\" WHERE fieldString = "
      u8"'你好'");

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
  BOOST_CHECK_EQUAL(
      u8"你好", utility::SqlWcharToString(fieldString, fieldString_len, true));

  // Fetch 2nd row - does not exist
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
}

BOOST_AUTO_TEST_CASE(TestSingleResultSelectWideCharUsingGetDataNarrowChar) {
  std::string dsnConnectionString;
  CreateDsnConnectionStringForLocalServer(dsnConnectionString);
  Connect(dsnConnectionString);
  SQLRETURN ret;
  std::vector< SQLWCHAR > request = MakeSqlBuffer(
      u8"SELECT fieldString FROM \"queries_test_004\" WHERE fieldString = "
      u8"'你好'");

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

  // Fetch 2nd row - does not exist
  ret = SQLFetch(stmt);
  BOOST_CHECK_EQUAL(SQL_NO_DATA, ret);
}

BOOST_AUTO_TEST_SUITE_END()
