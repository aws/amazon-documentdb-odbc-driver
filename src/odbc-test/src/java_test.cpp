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
#   include <Windows.h>
#endif

#include <sql.h>
#include <sqlext.h>

#include <string>
#include <vector>

#include <boost/test/unit_test.hpp>
#include <ignite/odbc/jni/java.h>
#include <ignite/odbc/jni/utils.h>
#include <ignite/odbc/common/concurrent.h>
#include <ignite/odbc/config/connection_string_parser.h>
#include <ignite/odbc/connection.h>
#include <ignite/odbc/dsn_config.h>
#include <ignite/odbc/ignite_error.h>

#include "test_utils.h"
#include "odbc_test_suite.h"

using namespace ignite;
using namespace ignite_test;
using namespace boost::unit_test;
using namespace odbc;
using namespace odbc::config;
using namespace odbc::jni;
using namespace odbc::jni::java;
using namespace odbc::common::concurrent;

/**
 * Test setup fixture.
 */
struct JavaTestSuiteFixture: odbc::OdbcTestSuite
{
    using odbc::OdbcTestSuite::OdbcTestSuite;

    /**
     * Execute the query and return an error code.
     */
    std::string ExecQueryAndReturnError()
    {
        SQLCHAR selectReq[] = "select count(*) from TestType";

        SQLRETURN ret = SQLExecDirect(stmt, selectReq, sizeof(selectReq));

        std::string err;

        if (!SQL_SUCCEEDED(ret))
            err = ExtractErrorCode(GetOdbcErrorMessage(SQL_HANDLE_STMT, stmt));

        return err;
    }

    /**
     * Extract code from ODBC error message.
     *
     * @param err Error message.
     * @return Error code.
     */
    static std::string ExtractErrorCode(const std::string& err)
    {
        std::string code;

        size_t idx = err.find(':');

        if ((idx != std::string::npos) && (idx > 0))
            code = err.substr(0, idx);

        return code;
    }

    SharedPointer< JniContext > GetJniContext(std::vector< char* >& opts) const {

        SharedPointer< JniContext > ctx(JniContext::Create(
            &opts[0], static_cast< int >(opts.size()), JniHandlers()));
        BOOST_CHECK(ctx.Get());
        return ctx;
    }

    std::string GetJdbcConnectionString() const {
        std::string dsnConnectionString;
        CreateDsnConnectionString(dsnConnectionString);

        Configuration config;
        ConnectionStringParser parser(config);
        parser.ParseConnectionString(dsnConnectionString, nullptr);
        std::string jdbcConnectionString =
            Connection::FormatJdbcConnectionString(config);
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
        using namespace odbc::common;
        std::for_each(_opts.begin(), _opts.end(), ReleaseChars);
        _opts.clear();
        _ctx = nullptr;
        _prepared = false;
    }

    /**
     * Destructor.
     */
    ~JavaTestSuiteFixture() override {
        CleanUpContext();
    }

    bool _prepared = false;

    std::string _jdbcConnectionString;

    std::vector< char* > _opts;

    SharedPointer< JniContext > _ctx;
};


BOOST_FIXTURE_TEST_SUITE(JavaTestSuite, JavaTestSuiteFixture)

BOOST_AUTO_TEST_CASE(TestDriverManagerGetConnection)
{
    PrepareContext();

    JniErrorInfo errInfo;
    SharedPointer< GlobalJObject > connection;
    bool success = _ctx.Get()->DriverManagerGetConnection(_jdbcConnectionString.c_str(), connection, &errInfo);
    if (!success || errInfo.code != odbc::java::IGNITE_JNI_ERR_SUCCESS) {
        BOOST_FAIL(errInfo.errMsg);
    }
    BOOST_REQUIRE(connection.Get());

    _ctx.Get()->ConnectionClose(connection, &errInfo);
    connection = SharedPointer< GlobalJObject >(nullptr);
}

BOOST_AUTO_TEST_CASE(TestConnectionGetMetaData) {
    PrepareContext();

    JniErrorInfo errInfo;
    SharedPointer< GlobalJObject > connection;
    bool success = _ctx.Get()->DriverManagerGetConnection(
        _jdbcConnectionString.c_str(), connection, &errInfo);
    if (!success || errInfo.code != odbc::java::IGNITE_JNI_ERR_SUCCESS) {
        BOOST_FAIL(errInfo.errMsg);
    }
    BOOST_REQUIRE(connection.Get());

    SharedPointer< GlobalJObject > databaseMetaData;
    if (!_ctx.Get()->ConnectionGetMetaData(connection, databaseMetaData,
                                           &errInfo)) {
        std::string errMsg = errInfo.errMsg;
        _ctx.Get()->ConnectionClose(connection, &errInfo);
        BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(databaseMetaData.Get());

    _ctx.Get()->ConnectionClose(connection, &errInfo);
    connection = nullptr;
}

BOOST_AUTO_TEST_CASE(TestDatabaseMetaDataGetTables) {
    PrepareContext();

    JniErrorInfo errInfo;
    SharedPointer< GlobalJObject > connection;
    bool success = _ctx.Get()->DriverManagerGetConnection(
        _jdbcConnectionString.c_str(), connection, &errInfo);
    if (!success || errInfo.code != odbc::java::IGNITE_JNI_ERR_SUCCESS) {
        BOOST_FAIL(errInfo.errMsg);
    }
    BOOST_REQUIRE(connection.Get());

    SharedPointer< GlobalJObject > databaseMetaData;
    if (!_ctx.Get()->ConnectionGetMetaData(connection, databaseMetaData, &errInfo)) {
        std::string errMsg = errInfo.errMsg;
        _ctx.Get()->ConnectionClose(connection, &errInfo);
        BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(databaseMetaData.Get());

    SharedPointer< GlobalJObject > resultSet;
    if (!_ctx.Get()->DatabaseMetaDataGetTables(databaseMetaData, resultSet,
                                               &errInfo)) {
        std::string errMsg = errInfo.errMsg;
        _ctx.Get()->ConnectionClose(connection, &errInfo);
        BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(resultSet.Get());

    _ctx.Get()->ConnectionClose(connection, &errInfo);
    connection = nullptr;
}

BOOST_AUTO_TEST_SUITE_END()