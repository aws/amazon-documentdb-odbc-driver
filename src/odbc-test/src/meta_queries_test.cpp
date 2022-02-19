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
#   include <windows.h>
#endif

#include <sql.h>
#include <sqlext.h>

#include <vector>
#include <string>
#include <algorithm>

#include <boost/test/unit_test.hpp>

#include "ignite/ignition.h"

#include "ignite/common/fixed_size_array.h"
#include "ignite/impl/binary/binary_utils.h"

#include "test_type.h"
#include "complex_type.h"
#include "test_utils.h"
#include "odbc_test_suite.h"

using namespace ignite;
using namespace ignite::cache;
using namespace ignite::cache::query;
using namespace ignite::common;
using namespace ignite_test;
using namespace ignite::binary;
using namespace ignite::impl::binary;
using namespace ignite::impl::interop;

using namespace boost::unit_test;

/**
 * Test setup fixture.
 */
struct MetaQueriesTestSuiteFixture : public odbc::OdbcTestSuite
{
    /**
     * Checks single row result set for correct work with SQLGetData.
     *
     * @param stmt Statement.
     */
    void CheckSingleRowResultSetWithGetData(SQLHSTMT stmt, SQLUSMALLINT columnIndex = 1, const char* expectedValue = nullptr) const
    {
        SQLRETURN ret = SQLFetch(stmt);

        if (!SQL_SUCCEEDED(ret))
            BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

        char buf[1024];
        SQLLEN bufLen = sizeof(buf);

        columnIndex = columnIndex >= 1 ? columnIndex : 1;
        for (int i = 1; i <= columnIndex; i++) {
            ret = SQLGetData(stmt, columnIndex, SQL_C_CHAR, buf, sizeof(buf),
                             &bufLen);

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
    void CheckStringColumn(SQLHSTMT stmt, int colId, const std::string& value)
    {
        char buf[1024];
        SQLLEN bufLen = sizeof(buf);

        SQLRETURN ret = SQLGetData(stmt, colId, SQL_C_CHAR, buf, sizeof(buf), &bufLen);

        if (!SQL_SUCCEEDED(ret))
            BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

        if (bufLen <= 0)
            BOOST_CHECK(value.empty());
        else
            BOOST_CHECK_EQUAL(std::string(buf, static_cast<size_t>(bufLen)), value);
    }

    /**
     * Constructor.
     */
    MetaQueriesTestSuiteFixture() :
        grid(0),
        cache1(0),
        cache2(0)
    {
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
    void CheckColumnMetaWithSQLDescribeCol(SQLHSTMT stmt, SQLUSMALLINT idx, const std::string& expName,
        SQLSMALLINT expDataType, SQLULEN expSize, SQLSMALLINT expScale, SQLSMALLINT expNullability)
    {
        std::vector<SQLCHAR> name(ODBC_BUFFER_SIZE);
        SQLSMALLINT nameLen = 0;
        SQLSMALLINT dataType = 0;
        SQLULEN size;
        SQLSMALLINT scale;
        SQLSMALLINT nullability;

        SQLRETURN ret = SQLDescribeCol(stmt, idx, &name[0], (SQLSMALLINT)name.size(), &nameLen, &dataType, &size, &scale, &nullability);
        ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

        BOOST_CHECK_GE(nameLen, 0);
        BOOST_CHECK_LE(nameLen, static_cast<SQLSMALLINT>(ODBC_BUFFER_SIZE));

        std::string nameStr(name.begin(), name.begin() + nameLen);

        BOOST_CHECK_EQUAL(nameStr, expName);
        BOOST_CHECK_EQUAL(dataType, expDataType);
        BOOST_CHECK_EQUAL(size, expSize);
        BOOST_CHECK_EQUAL(scale, expScale);
        BOOST_CHECK_EQUAL(nullability, expNullability);
    }

    /**
     * @param func Function to call before tests. May be PrepareQuery or ExecQuery.
     *
     * 1. Start node.
     * 2. Connect to node using ODBC.
     * 3. Create table with decimal and char columns with specified size and scale.
     * 4. Execute or prepare statement.
     * 5. Check presicion and scale of every column using SQLDescribeCol.
     */
    template<typename F>
    void CheckSQLDescribeColPrecisionAndScale(F func)
    {
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
            "values (1, 12, 160.23, -1234.56789, 'TST', 'Lorem Ipsum', 'Some test value', 'Some test varchar')");

        ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

        ret = SQLFreeStmt(stmt, SQL_CLOSE);
        ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

        ret = (this->*func)("select id, dec1, dec2, dec3, char1, char2, char3, vchar from PUBLIC.TestScalePrecision");
        ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

        SQLSMALLINT columnCount = 0;

        ret = SQLNumResultCols(stmt, &columnCount);
        ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

        BOOST_CHECK_EQUAL(columnCount, 8);

        CheckColumnMetaWithSQLDescribeCol(stmt, 1, "ID", SQL_INTEGER, 10, 0, SQL_NULLABLE);
        CheckColumnMetaWithSQLDescribeCol(stmt, 2, "DEC1", SQL_DECIMAL, 3, 0, SQL_NULLABLE);
        CheckColumnMetaWithSQLDescribeCol(stmt, 3, "DEC2", SQL_DECIMAL, 42, 12, SQL_NULLABLE);
        CheckColumnMetaWithSQLDescribeCol(stmt, 4, "DEC3", SQL_DECIMAL, 65535, 32767, SQL_NULLABLE);
        CheckColumnMetaWithSQLDescribeCol(stmt, 5, "CHAR1", SQL_VARCHAR, 3, 0, SQL_NULLABLE);
        CheckColumnMetaWithSQLDescribeCol(stmt, 6, "CHAR2", SQL_VARCHAR, 42, 0, SQL_NULLABLE);
        CheckColumnMetaWithSQLDescribeCol(stmt, 7, "CHAR3", SQL_VARCHAR, 2147483647, 0, SQL_NO_NULLS);
        CheckColumnMetaWithSQLDescribeCol(stmt, 8, "VCHAR", SQL_VARCHAR, 2147483647, 0, SQL_NULLABLE);
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
    void CheckColumnMetaWithSQLColAttribute(SQLHSTMT stmt, SQLUSMALLINT idx, const std::string& expName,
        SQLLEN expDataType, SQLULEN expSize, SQLLEN expScale, SQLLEN expNullability)
    {
        std::vector<SQLCHAR> name(ODBC_BUFFER_SIZE);
        SQLSMALLINT nameLen = 0;
        SQLLEN dataType = 0;
        SQLLEN size;
        SQLLEN scale;
        SQLLEN nullability;

        SQLRETURN ret = SQLColAttribute(stmt, idx, SQL_DESC_NAME, &name[0], (SQLSMALLINT)name.size(), &nameLen, 0);
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
        BOOST_CHECK_LE(nameLen, static_cast<SQLSMALLINT>(ODBC_BUFFER_SIZE));

        std::string nameStr(name.begin(), name.begin() + nameLen);

        BOOST_CHECK_EQUAL(nameStr, expName);
        BOOST_CHECK_EQUAL(dataType, expDataType);
        BOOST_CHECK_EQUAL(size, expSize);
        BOOST_CHECK_EQUAL(scale, expScale);
        BOOST_CHECK_EQUAL(nullability, expNullability);
    }

    /**
     * @param func Function to call before tests. May be PrepareQuery or ExecQuery.
     *
     * 1. Start node.
     * 2. Connect to node using ODBC.
     * 3. Create table with decimal and char columns with specified size and scale.
     * 4. Execute or prepare statement.
     * 5. Check presicion and scale of every column using SQLColAttribute.
     */
    template<typename F>
    void CheckSQLColAttributePrecisionAndScale(F func)
    {
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
            "values (1, 12, 160.23, -1234.56789, 'TST', 'Lorem Ipsum', 'Some test value', 'Some test varchar')");

        ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

        ret = SQLFreeStmt(stmt, SQL_CLOSE);
        ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

        ret = (this->*func)("select id, dec1, dec2, dec3, char1, char2, char3, vchar from PUBLIC.TestScalePrecision");
        ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

        SQLSMALLINT columnCount = 0;

        ret = SQLNumResultCols(stmt, &columnCount);
        ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

        BOOST_CHECK_EQUAL(columnCount, 8);

        CheckColumnMetaWithSQLColAttribute(stmt, 1, "ID", SQL_INTEGER, 10, 0, SQL_NULLABLE);
        CheckColumnMetaWithSQLColAttribute(stmt, 2, "DEC1", SQL_DECIMAL, 3, 0, SQL_NULLABLE);
        CheckColumnMetaWithSQLColAttribute(stmt, 3, "DEC2", SQL_DECIMAL, 42, 12, SQL_NULLABLE);
        CheckColumnMetaWithSQLColAttribute(stmt, 4, "DEC3", SQL_DECIMAL, 65535, 32767, SQL_NULLABLE);
        CheckColumnMetaWithSQLColAttribute(stmt, 5, "CHAR1", SQL_VARCHAR, 3, 0, SQL_NULLABLE);
        CheckColumnMetaWithSQLColAttribute(stmt, 6, "CHAR2", SQL_VARCHAR, 42, 0, SQL_NULLABLE);
        CheckColumnMetaWithSQLColAttribute(stmt, 7, "CHAR3", SQL_VARCHAR, 2147483647, 0, SQL_NO_NULLS);
        CheckColumnMetaWithSQLColAttribute(stmt, 8, "VCHAR", SQL_VARCHAR, 2147483647, 0, SQL_NULLABLE);
    }

    /**
     * Destructor.
     */
    ~MetaQueriesTestSuiteFixture()
    {
        // No-op.
    }

    /** Node started during the test. */
    Ignite grid;

    /** Frist cache instance. */
    Cache<int64_t, TestType> cache1;

    /** Second cache instance. */
    Cache<int64_t, ComplexType> cache2;
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

    SQLRETURN ret = SQLColAttribute(stmt, 1, SQL_COLUMN_LENGTH, strBuf, sizeof(strBuf), &strLen, &intVal);

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

    SQLRETURN ret = SQLColAttribute(stmt, 1, SQL_COLUMN_PRECISION, strBuf, sizeof(strBuf), &strLen, &intVal);

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

    SQLRETURN ret = SQLColAttribute(stmt, 1, SQL_COLUMN_SCALE, strBuf, sizeof(strBuf), &strLen, &intVal);

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

    SQLRETURN ret = SQLColAttribute(stmt, 1, SQL_COLUMN_LENGTH, strBuf, sizeof(strBuf), &strLen, &intVal);

    if (!SQL_SUCCEEDED(ret))
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    BOOST_CHECK_EQUAL(intVal, 60);

    ret = SQLExecute(stmt);
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    ret = SQLColAttribute(stmt, 1, SQL_COLUMN_LENGTH, strBuf, sizeof(strBuf), &strLen, &intVal);

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

    SQLRETURN ret = SQLColAttribute(stmt, 1, SQL_COLUMN_PRECISION, strBuf, sizeof(strBuf), &strLen, &intVal);

    if (!SQL_SUCCEEDED(ret))
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    BOOST_CHECK_EQUAL(intVal, 60);

    ret = SQLExecute(stmt);
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    ret = SQLColAttribute(stmt, 1, SQL_COLUMN_PRECISION, strBuf, sizeof(strBuf), &strLen, &intVal);

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

    SQLRETURN ret = SQLColAttribute(stmt, 1, SQL_COLUMN_SCALE, strBuf, sizeof(strBuf), &strLen, &intVal);

    if (!SQL_SUCCEEDED(ret))
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    ret = SQLExecute(stmt);
    ODBC_FAIL_ON_ERROR(ret, SQL_HANDLE_STMT, stmt);

    ret = SQLColAttribute(stmt, 1, SQL_COLUMN_SCALE, strBuf, sizeof(strBuf), &strLen, &intVal);

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

BOOST_AUTO_TEST_CASE(TestGetDataWithTablesReturnsOne)
{
    std::string dsnConnectionString;
    CreateDsnConnectionString(dsnConnectionString);

    Connect(dsnConnectionString);

    SQLCHAR empty[] = "";
    SQLCHAR table[] = "test";

    SQLRETURN ret = SQLTables(stmt, empty, SQL_NTS, empty, SQL_NTS, table, SQL_NTS, empty, SQL_NTS);

    if (!SQL_SUCCEEDED(ret))
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    CheckSingleRowResultSetWithGetData(stmt, 2, "test");
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

BOOST_AUTO_TEST_CASE(TestGetDataWithColumns) {
    std::string dsnConnectionString;
    CreateDsnConnectionString(dsnConnectionString);
    Connect(dsnConnectionString);

    SQLCHAR empty[] = "";
    SQLCHAR table[] = "test";
    SQLCHAR column[] = "test__id";

    SQLRETURN ret = SQLColumns(stmt, empty, SQL_NTS, empty, SQL_NTS, table, SQL_NTS, column, SQL_NTS);
    if (!SQL_SUCCEEDED(ret))
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));
    
    ret = SQLFetch(stmt);
    if (!SQL_SUCCEEDED(ret))
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    char buf[1024]{};
    SQLLEN bufLen = sizeof(buf);
    SQLSMALLINT bufSmallInt;
    SQLINTEGER bufInt;
    for (int columnIndex = 1; columnIndex <= 18; columnIndex++) {
        switch (columnIndex) {
            case 1:
            case 2:
            case 3:
            case 4:
            case 6:
            case 12:
            case 13:
            case 18:
                ret = SQLGetData(stmt, columnIndex, SQL_C_CHAR, buf,
                                 sizeof(buf), &bufLen);
                break;
            case 5:
            case 9:
            case 10:
            case 11:
            case 14:
            case 15:
                ret = SQLGetData(stmt, columnIndex, SQL_C_SHORT, &bufSmallInt,
                                 sizeof(bufSmallInt), &bufLen);
                break;
            case 7:
            case 8:
            case 16:
            case 17:
                ret = SQLGetData(stmt, columnIndex, SQL_C_LONG, &bufInt,
                                 sizeof(bufInt), &bufLen);
                break;
            default:
                BOOST_FAIL("Unexpected column index encountered.");
                break;
        }
        if (ret == SQL_NO_DATA) {
            break;
        }

        if (!SQL_SUCCEEDED(ret))
            BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

        bool wasNull = bufLen == SQL_NULL_DATA;

        switch (columnIndex) {
            case 1:
                BOOST_CHECK_EQUAL("", buf);  // TABLE_CAT
                break;
            case 2:
                BOOST_CHECK_EQUAL("test", buf);  // TABLE_SCHEM
                break;
            case 3:
                BOOST_CHECK_EQUAL("test", buf);  // TABLE_NAME
                break;
            case 4:
                BOOST_CHECK_EQUAL("test__id", buf);  // COLUMN_NAME
                break;
            case 5:
                BOOST_CHECK_EQUAL(0, bufSmallInt);  // DATA_TYPE
                break;
            case 6:
                BOOST_CHECK_EQUAL("NULL", buf);  // TYPE_NAME
                break;
            case 7:
                BOOST_CHECK_EQUAL(1, bufInt);  // COLUMN_SIZE
                break;
            case 8:
                BOOST_CHECK_EQUAL(1, bufInt);  // BUFFER_LENGTH
                break;
            case 9:
                BOOST_CHECK_EQUAL(0, bufSmallInt);  // DECIMAL_DIGITS
                break;
            case 10:
                BOOST_CHECK_EQUAL(0, bufSmallInt);  // NUM_PREC_RADIX
                break;
            case 11:
                BOOST_CHECK_EQUAL(0, bufSmallInt);  // NULLABLE
                break;
            case 12:
                BOOST_CHECK_EQUAL("", buf);  // REMARKS
                break;
            case 13:
                BOOST_CHECK_EQUAL("", buf);  // COLUMN_DEF
                break;
            case 14:
                BOOST_CHECK_EQUAL(0, bufSmallInt);  // SQL_DATA_TYPE
                break;
            case 15:
                BOOST_CHECK_EQUAL(0, bufSmallInt);  // SQL_DATETIME_SUB
                break;
            case 16:
                BOOST_CHECK_EQUAL(0, bufInt);  // CHAR_OCTET_LENGTH
                break;
            case 17:
                BOOST_CHECK_EQUAL(1, bufInt);  // ORDINAL_POSITION
                break;
            case 18:
                BOOST_CHECK_EQUAL("NO" , buf);  // IS_NULLABLE
                break;
            default:
                BOOST_FAIL("Unexpected column index encountered.");
                break;
        }
    }

    ret = SQLFetch(stmt);
    BOOST_REQUIRE_EQUAL(ret, SQL_NO_DATA);

    ret = SQLGetData(stmt, 1, SQL_C_CHAR, buf, sizeof(buf), &bufLen);
    BOOST_REQUIRE_EQUAL(ret, SQL_ERROR);
    BOOST_CHECK_EQUAL(GetOdbcErrorState(SQL_HANDLE_STMT, stmt), "24000");
}

BOOST_AUTO_TEST_CASE(TestGetDataWithSelectQuery, *disabled()) {
    Connect("DRIVER={Apache Ignite};ADDRESS=127.0.0.1:11110;SCHEMA=cache");

    SQLCHAR insertReq[] = "insert into TestType(_key, strField) VALUES(1, 'Lorem ipsum')";
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
        "insert into TestType(_key, strField) VALUES(42, '0123456789012345678901234567890123456789012345678901234567891')";

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

    SQLCHAR createTable[] = "create table TestTable(id int primary key, testColumn varchar)";
    SQLRETURN ret = SQLExecDirect(stmt, createTable, SQL_NTS);

    if (!SQL_SUCCEEDED(ret))
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    SQLCHAR empty[] = "";
    SQLCHAR table[] = "TestTable";

    ret = SQLTables(stmt, empty, SQL_NTS, empty, SQL_NTS, table, SQL_NTS, empty, SQL_NTS);

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

    SQLCHAR createTable[] = "create table TestTable(id int primary key, testColumn varchar)";
    SQLRETURN ret = SQLExecDirect(stmt, createTable, SQL_NTS);

    if (!SQL_SUCCEEDED(ret))
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    SQLCHAR empty[] = "";
    SQLCHAR table[] = "TestTable";
    SQLCHAR typeList[] = "TABLE,VIEW";

    ret = SQLTables(stmt, empty, SQL_NTS, empty, SQL_NTS, table, SQL_NTS, typeList, SQL_NTS);

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

    SQLCHAR createTable[] = "create table TestTable(id int primary key, testColumn varchar)";
    SQLRETURN ret = SQLExecDirect(stmt, createTable, SQL_NTS);

    if (!SQL_SUCCEEDED(ret))
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    SQLCHAR empty[] = "";
    SQLCHAR table[] = "TestTable";

    ret = SQLColumns(stmt, empty, SQL_NTS, empty, SQL_NTS, table, SQL_NTS, empty, SQL_NTS);

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

    SQLCHAR createTable[] = "create table ESG_FOCUS(id int primary key, TEST_COLUMN varchar)";
    SQLRETURN ret = SQLExecDirect(stmt, createTable, SQL_NTS);

    if (!SQL_SUCCEEDED(ret))
        BOOST_FAIL(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

    SQLCHAR empty[] = "";
    SQLCHAR table[] = "ESG\\_FOCUS";

    ret = SQLColumns(stmt, empty, SQL_NTS, empty, SQL_NTS, table, SQL_NTS, empty, SQL_NTS);

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

    SQLRETURN ret = ExecQuery("create table TestSqlPrepare(id int primary key, test1 varchar, test2 long, test3 varchar)");
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
 * Check that SQLDescribeCol return valid scale and precision for columns of different type after Prepare.
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
 * Check that SQLDescribeCol return valid scale and precision for columns of different type after Execute.
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
 * Check that SQLColAttribute return valid scale and precision for columns of different type after Prepare.
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
 * Check that SQLColAttribute return valid scale and precision for columns of different type after Execute.
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
