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

#include "sql_test_suite_fixture.h"

#include "test_utils.h"

using namespace documentdb_test;

namespace documentdb {
SqlTestSuiteFixture::SqlTestSuiteFixture()
    : testCache(0), env(NULL), dbc(NULL), stmt(NULL) {
  grid = StartPlatformNode("queries-test.xml", "NodeMain");

  testCache = grid.GetCache< int64_t, TestType >("cache");

  // Allocate an environment handle
  SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);

  BOOST_REQUIRE(env != NULL);

  // We want ODBC 3 support
  SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION,
                reinterpret_cast< void* >(SQL_OV_ODBC3), 0);

  // Allocate a connection handle
  SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);

  BOOST_REQUIRE(dbc != NULL);

  // Connect string
  SQLCHAR connectStr[] =
      "DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache";

  SQLCHAR outstr[ODBC_BUFFER_SIZE];
  SQLSMALLINT outstrlen;

  // Connecting to ODBC server.
  SQLRETURN ret = SQLDriverConnect(
      dbc, NULL, connectStr, static_cast< SQLSMALLINT >(sizeof(connectStr)),
      outstr, sizeof(outstr), &outstrlen, SQL_DRIVER_COMPLETE);

  if (!SQL_SUCCEEDED(ret)) {
    Ignition::StopAll(true);

    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_DBC, dbc));
  }

  // Allocate a statement handle
  SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

  BOOST_REQUIRE(stmt != NULL);
}

SqlTestSuiteFixture::~SqlTestSuiteFixture() {
  // Releasing statement handle.
  SQLFreeHandle(SQL_HANDLE_STMT, stmt);

  // Disconneting from the server.
  SQLDisconnect(dbc);

  // Releasing allocated handles.
  SQLFreeHandle(SQL_HANDLE_DBC, dbc);
  SQLFreeHandle(SQL_HANDLE_ENV, env);

  documentdb::Ignition::StopAll(true);
}

void SqlTestSuiteFixture::CheckSingleResult0(const char* request,
                                             SQLSMALLINT type, void* column,
                                             SQLLEN bufSize,
                                             SQLLEN* resSize) const {
  SQLRETURN ret;

  ret = SQLBindCol(stmt, 1, type, column, bufSize, resSize);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLExecDirect(
      stmt, reinterpret_cast< SQLCHAR* >(const_cast< char* >(request)),
      SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLFetch(stmt);

  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

  ret = SQLFetch(stmt);
  BOOST_CHECK(ret == SQL_NO_DATA);

  ret = SQLFreeStmt(stmt, SQL_CLOSE);
  if (!SQL_SUCCEEDED(ret))
    BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
}

template <>
void SqlTestSuiteFixture::CheckSingleResult< std::string >(
    const char* request, const std::string& expected) {
  SQLCHAR res[ODBC_BUFFER_SIZE] = {0};
  SQLLEN resLen = 0;

  CheckSingleResult0(request, SQL_C_CHAR, res, ODBC_BUFFER_SIZE, &resLen);

  std::string actual;

  if (resLen > 0)
    actual.assign(reinterpret_cast< char* >(res),
                  static_cast< size_t >(resLen));

  BOOST_CHECK_EQUAL(actual, expected);
}

template <>
void SqlTestSuiteFixture::CheckSingleResult< SQLBIGINT >(
    const char* request, const SQLBIGINT& expected) {
  CheckSingleResultNum0< SQLBIGINT >(request, expected, SQL_C_SBIGINT);
}

template <>
void SqlTestSuiteFixture::CheckSingleResult< SQLINTEGER >(
    const char* request, const SQLINTEGER& expected) {
  CheckSingleResultNum0< SQLINTEGER >(request, expected, SQL_C_SLONG);
}

template <>
void SqlTestSuiteFixture::CheckSingleResult< SQLSMALLINT >(
    const char* request, const SQLSMALLINT& expected) {
  CheckSingleResultNum0< SQLSMALLINT >(request, expected, SQL_C_SSHORT);
}

template <>
void SqlTestSuiteFixture::CheckSingleResult< int8_t >(const char* request,
                                                      const int8_t& expected) {
  CheckSingleResultNum0< int8_t >(request, expected, SQL_C_STINYINT);
}

template <>
void SqlTestSuiteFixture::CheckSingleResult< float >(const char* request,
                                                     const float& expected) {
  SQLREAL res = 0;

  CheckSingleResult0(request, SQL_C_FLOAT, &res, 0, 0);

  BOOST_CHECK_CLOSE(static_cast< float >(res), expected, 1E-6f);
}

template <>
void SqlTestSuiteFixture::CheckSingleResult< double >(const char* request,
                                                      const double& expected) {
  SQLDOUBLE res = 0;

  CheckSingleResult0(request, SQL_C_DOUBLE, &res, 0, 0);

  BOOST_CHECK_CLOSE(static_cast< double >(res), expected, 1E-6);
}

template <>
void SqlTestSuiteFixture::CheckSingleResult< bool >(const char* request,
                                                    const bool& expected) {
  SQLCHAR res = 0;

  CheckSingleResult0(request, SQL_C_BIT, &res, 0, 0);

  BOOST_CHECK_EQUAL((res != 0), expected);
}

template <>
void SqlTestSuiteFixture::CheckSingleResult< documentdb::Guid >(
    const char* request, const documentdb::Guid& expected) {
  SQLGUID res;

  memset(&res, 0, sizeof(res));

  CheckSingleResult0(request, SQL_C_GUID, &res, 0, 0);

  BOOST_CHECK_EQUAL(res.Data1, expected.GetMostSignificantBits()
                                   & 0xFFFFFFFF00000000UL >> 32);
  BOOST_CHECK_EQUAL(res.Data2, expected.GetMostSignificantBits()
                                   & 0x00000000FFFF0000UL >> 16);
  BOOST_CHECK_EQUAL(res.Data3,
                    expected.GetMostSignificantBits() & 0x000000000000FFFFUL);

  for (size_t i = 0; i < sizeof(res.Data4); ++i)
    BOOST_CHECK_EQUAL(
        res.Data4[i],
        (expected.GetLeastSignificantBits() & (0xFFUL << (8 * i))) >> (8 * i));
}

template <>
void SqlTestSuiteFixture::CheckSingleResult< std::string >(
    const char* request) {
  SQLCHAR res[ODBC_BUFFER_SIZE] = {0};
  SQLLEN resLen = 0;

  CheckSingleResult0(request, SQL_C_CHAR, res, ODBC_BUFFER_SIZE, &resLen);
}

template <>
void SqlTestSuiteFixture::CheckSingleResult< SQLBIGINT >(const char* request) {
  CheckSingleResultNum0< SQLBIGINT >(request, SQL_C_SBIGINT);
}

template <>
void SqlTestSuiteFixture::CheckSingleResult< SQLINTEGER >(const char* request) {
  CheckSingleResultNum0< SQLINTEGER >(request, SQL_C_SLONG);
}

template <>
void SqlTestSuiteFixture::CheckSingleResult< SQLSMALLINT >(
    const char* request) {
  CheckSingleResultNum0< SQLSMALLINT >(request, SQL_C_SSHORT);
}

template <>
void SqlTestSuiteFixture::CheckSingleResult< int8_t >(const char* request) {
  CheckSingleResultNum0< int8_t >(request, SQL_C_STINYINT);
}

template <>
void SqlTestSuiteFixture::CheckSingleResult< float >(const char* request) {
  SQLREAL res = 0;

  CheckSingleResult0(request, SQL_C_FLOAT, &res, 0, 0);
}

template <>
void SqlTestSuiteFixture::CheckSingleResult< double >(const char* request) {
  SQLDOUBLE res = 0;

  CheckSingleResult0(request, SQL_C_DOUBLE, &res, 0, 0);
}

template <>
void SqlTestSuiteFixture::CheckSingleResult< Date >(const char* request) {
  SQL_DATE_STRUCT res;

  CheckSingleResult0(request, SQL_C_TYPE_DATE, &res, 0, 0);
}

template <>
void SqlTestSuiteFixture::CheckSingleResult< Timestamp >(const char* request) {
  SQL_TIMESTAMP_STRUCT res;

  CheckSingleResult0(request, SQL_C_TYPE_TIMESTAMP, &res, 0, 0);
}

template <>
void SqlTestSuiteFixture::CheckSingleResult< Time >(const char* request) {
  SQL_TIME_STRUCT res;

  CheckSingleResult0(request, SQL_C_TYPE_TIME, &res, 0, 0);
}

template <>
void SqlTestSuiteFixture::CheckSingleResult< std::vector< int8_t > >(
    const char* request, const std::vector< int8_t >& expected) {
  SQLCHAR res[ODBC_BUFFER_SIZE] = {0};
  SQLLEN resLen = 0;

  CheckSingleResult0(request, SQL_C_BINARY, res, ODBC_BUFFER_SIZE, &resLen);

  BOOST_REQUIRE_EQUAL(resLen, expected.size());

  if (resLen > 0) {
    std::vector< int8_t > actual(res, res + resLen);
    BOOST_REQUIRE_EQUAL_COLLECTIONS(expected.begin(), expected.end(),
                                    actual.begin(), actual.end());
  }
}

template <>
void SqlTestSuiteFixture::CheckSingleResult< documentdb::common::Decimal >(
    const char* request, const documentdb::common::Decimal& expected) {
  SQLCHAR res[ODBC_BUFFER_SIZE] = {0};
  SQLLEN resLen = 0;

  CheckSingleResult0(request, SQL_C_CHAR, res, ODBC_BUFFER_SIZE, &resLen);
  documentdb::common::Decimal actual(std::string(res, res + resLen));
  BOOST_REQUIRE_EQUAL(actual, expected);
}

template <>
void SqlTestSuiteFixture::CheckSingleResult< Date >(const char* request,
                                                    const Date& expected) {
  SQL_DATE_STRUCT res;

  CheckSingleResult0(request, SQL_C_TYPE_DATE, &res, 0, 0);

  using documentdb::impl::binary::BinaryUtils;
  Date actual = common::MakeDateGmt(res.year, res.month, res.day);
  BOOST_REQUIRE_EQUAL(actual.GetSeconds(), expected.GetSeconds());
}

template <>
void SqlTestSuiteFixture::CheckSingleResult< Timestamp >(
    const char* request, const Timestamp& expected) {
  SQL_TIMESTAMP_STRUCT res;

  CheckSingleResult0(request, SQL_C_TYPE_TIMESTAMP, &res, 0, 0);

  using documentdb::impl::binary::BinaryUtils;
  Timestamp actual =
      common::MakeTimestampGmt(res.year, res.month, res.day, res.hour,
                               res.minute, res.second, res.fraction);

  BOOST_REQUIRE_EQUAL(actual.GetSeconds(), expected.GetSeconds());
  BOOST_REQUIRE_EQUAL(actual.GetSecondFraction(), expected.GetSecondFraction());
}

template <>
void SqlTestSuiteFixture::CheckSingleResult< Time >(const char* request,
                                                    const Time& expected) {
  SQL_TIME_STRUCT res;

  CheckSingleResult0(request, SQL_C_TYPE_TIME, &res, 0, 0);

  using documentdb::impl::binary::BinaryUtils;
  Time actual = common::MakeTimeGmt(res.hour, res.minute, res.second);

  BOOST_REQUIRE_EQUAL(actual.GetSeconds(), expected.GetSeconds());
}
}  // namespace documentdb
