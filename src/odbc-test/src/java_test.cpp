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
#include <ignite/odbc/common/common.h>
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
        BOOST_CHECK(ctx.Get() != nullptr);
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

struct AutoCloseConnection {
   public:
    AutoCloseConnection(SharedPointer< JniContext > ctx,
                        SharedPointer< GlobalJObject > connection)
        : _ctx(ctx), _connection(connection) {
    }

    ~AutoCloseConnection() {
        if (_ctx.Get() != nullptr && _connection.Get() != nullptr) {
            JniErrorInfo errInfo;
            _ctx.Get()->ConnectionClose(_connection, errInfo);
        }
        _connection = nullptr;
        _ctx = nullptr;
    }

   private:
    SharedPointer< JniContext > _ctx;
    SharedPointer< GlobalJObject > _connection;
    IGNITE_NO_COPY_ASSIGNMENT(AutoCloseConnection);
};

struct AutoCloseResultSet {
   public:
    AutoCloseResultSet(SharedPointer< JniContext > ctx,
                        SharedPointer< GlobalJObject > resultSet)
        : _ctx(ctx), _resultSet(resultSet) {
    }

    ~AutoCloseResultSet() {
        if (_ctx.Get() != nullptr && _resultSet.Get() != nullptr) {
            JniErrorInfo errInfo;
            _ctx.Get()->ResultSetClose(_resultSet, errInfo);
        }
        _resultSet = nullptr;
        _ctx = nullptr;
    }

   private:
    SharedPointer< JniContext > _ctx;
    SharedPointer< GlobalJObject > _resultSet;
    IGNITE_NO_COPY_ASSIGNMENT(AutoCloseResultSet);
};

BOOST_FIXTURE_TEST_SUITE(JavaTestSuite, JavaTestSuiteFixture)

BOOST_AUTO_TEST_CASE(TestDriverManagerGetConnection)
{
    PrepareContext();
    BOOST_REQUIRE(_ctx.Get() != nullptr);

    JniErrorInfo errInfo;
    SharedPointer< GlobalJObject > connection;
    bool success = _ctx.Get()->DriverManagerGetConnection(_jdbcConnectionString.c_str(), connection, errInfo);
    if (!success || errInfo.code != odbc::java::IGNITE_JNI_ERR_SUCCESS) {
        BOOST_FAIL(errInfo.errMsg);
    }
    BOOST_REQUIRE(connection.Get() != nullptr);

    _ctx.Get()->ConnectionClose(connection, errInfo);
    connection = SharedPointer< GlobalJObject >(nullptr);
}

BOOST_AUTO_TEST_CASE(TestDocumentDbConnectionGetSshTunnelPort) {
    PrepareContext();
    BOOST_REQUIRE(_ctx.Get() != nullptr);

    // get Driver manager connection
    JniErrorInfo errInfo;
    SharedPointer< GlobalJObject > connection;
    bool success = _ctx.Get()->DriverManagerGetConnection(_jdbcConnectionString.c_str(), connection, errInfo);
    if (!success || errInfo.code != odbc::java::IGNITE_JNI_ERR_SUCCESS) {
        BOOST_FAIL(errInfo.errMsg);
    }
    BOOST_REQUIRE(connection.Get());
    AutoCloseConnection autoCloseConnection(_ctx, connection);

    // see if SSH tunnel is active
    bool isActive;
    success = _ctx.Get()->DocumentDbConnectionIsSshTunnelActive(connection, isActive, errInfo);
    // if tunnel is not shown as active, or operation not successful, BOOST FAIL
    if (!success || errInfo.code != odbc::java::IGNITE_JNI_ERR_SUCCESS) {
        BOOST_FAIL(errInfo.errMsg);
    }
    BOOST_CHECK(isActive);

    // SSH tunnel confirmed to be active, get SSH tunnel local port
    int32_t port;
    success = _ctx.Get()->DocumentDbConnectionGetSshLocalPort(connection, port, errInfo);
    if (!success || errInfo.code != odbc::java::IGNITE_JNI_ERR_SUCCESS) {
        std::string errMsg = errInfo.errMsg;
        BOOST_FAIL(errMsg);
    }
    
    // if connection successful, port should be a positive number
    BOOST_CHECK(port > 0);
}

// TODO Enable when we can get external SSH tunnel working
BOOST_AUTO_TEST_CASE(TestDocumentDbConnectionGetSshTunnelPortSshTunnelNotActive,  * disabled()) {
//BOOST_AUTO_TEST_CASE(TestDocumentDbConnectionGetSshTunnelPortSshTunnelNotActive) {
    // test when SSH tunnel is not active, the SSH tunnel port should be 0
    // TODO do things so SSH tunnel is not active, but connection is open
    PrepareContext();
    BOOST_REQUIRE(_ctx.Get() != nullptr);

    // get Driver manager connection
    JniErrorInfo errInfo;
    SharedPointer< GlobalJObject > connection;
    bool success = _ctx.Get()->DriverManagerGetConnection(
        _jdbcConnectionString.c_str(), connection, errInfo);
    if (!success || errInfo.code != odbc::java::IGNITE_JNI_ERR_SUCCESS) {
        BOOST_FAIL(errInfo.errMsg);
    }
    BOOST_REQUIRE(connection.Get());
    AutoCloseConnection autoCloseConnection(_ctx, connection);

    // check if SSH tunnel is not active
    bool isActive;
    success = _ctx.Get()->DocumentDbConnectionIsSshTunnelActive(connection, isActive, errInfo);
    // if SSH tunnel is active, or operation not successful, BOOST FAIL
    if (isActive || !success || errInfo.code != odbc::java::IGNITE_JNI_ERR_SUCCESS) {
        BOOST_FAIL(errInfo.errMsg);
    }

    BOOST_CHECK(!isActive);

    // SSH tunnel confirmed to be not active, get SSH tunnel local port
    int32_t port;
    success = _ctx.Get()->DocumentDbConnectionGetSshLocalPort(connection, port, errInfo);
    if (errInfo.code != odbc::java::IGNITE_JNI_ERR_SUCCESS) {
        std::string errMsg = errInfo.errMsg;
        BOOST_FAIL(errMsg);
    }

    // if SSH tunnel not active, SSH local port number should be 0
    BOOST_CHECK_EQUAL(port, 0);
}

BOOST_AUTO_TEST_CASE(TestDocumentDbConnectionGetDatabaseMetadata) {
    PrepareContext();
    BOOST_REQUIRE(_ctx.Get() != nullptr);

    // get Driver manager connection
    JniErrorInfo errInfo;
    SharedPointer< GlobalJObject > connection;
    bool success = _ctx.Get()->DriverManagerGetConnection(
        _jdbcConnectionString.c_str(), connection, errInfo);
    if (!success || errInfo.code != odbc::java::IGNITE_JNI_ERR_SUCCESS) {
        BOOST_FAIL(errInfo.errMsg);
    }
    BOOST_REQUIRE(connection.Get());
    AutoCloseConnection autoCloseConnection(_ctx, connection);

    // get metadata
    SharedPointer< GlobalJObject > databaseMetadata;
    if (!_ctx.Get()->DocumentDbConnectionGetDatabaseMetadata(
            connection, databaseMetadata,
                                           errInfo)) {
        BOOST_FAIL(errInfo.errMsg);
    }
    BOOST_REQUIRE(databaseMetadata.Get());
}

BOOST_AUTO_TEST_CASE(TestDocumentDbDatabaseSchemaMetadataGetSchemaName) { 
    PrepareContext();
    BOOST_REQUIRE(_ctx.Get() != nullptr);

    // get Driver manager connection
    JniErrorInfo errInfo;
    SharedPointer< GlobalJObject > connection;
    bool success = _ctx.Get()->DriverManagerGetConnection(
        _jdbcConnectionString.c_str(), connection, errInfo);
    if (!success || errInfo.code != odbc::java::IGNITE_JNI_ERR_SUCCESS) {
        BOOST_FAIL(errInfo.errMsg);
    }
    BOOST_REQUIRE(connection.Get());
    AutoCloseConnection autoCloseConnection(_ctx, connection);

    // get metadata
    SharedPointer< GlobalJObject > databaseMetadata;
    if (!_ctx.Get()->DocumentDbConnectionGetDatabaseMetadata(
            connection, databaseMetadata, errInfo)) {
        BOOST_FAIL(errInfo.errMsg);
    }
    BOOST_REQUIRE(databaseMetadata.Get());

    std::string schemaName;
    bool wasNull;
    success = _ctx.Get()->DocumentDbDatabaseSchemaMetadataGetSchemaName(
        databaseMetadata, schemaName, wasNull, errInfo); 
    if (!success || errInfo.code != odbc::java::IGNITE_JNI_ERR_SUCCESS) {
        BOOST_FAIL(errInfo.errMsg);
    }

    BOOST_CHECK(!wasNull);
    BOOST_CHECK_EQUAL(schemaName, "_default");
}

BOOST_AUTO_TEST_CASE(TestConnectionGetMetaData) {
    PrepareContext();
    BOOST_REQUIRE(_ctx.Get() != nullptr);

    JniErrorInfo errInfo;
    SharedPointer< GlobalJObject > connection;
    bool success = _ctx.Get()->DriverManagerGetConnection(
        _jdbcConnectionString.c_str(), connection, errInfo);
    if (!success || errInfo.code != odbc::java::IGNITE_JNI_ERR_SUCCESS) {
        BOOST_FAIL(errInfo.errMsg);
    }
    BOOST_REQUIRE(connection.Get() != nullptr);
    AutoCloseConnection autoCloseConnection(_ctx, connection);

    SharedPointer< GlobalJObject > databaseMetaData;
    if (!_ctx.Get()->ConnectionGetMetaData(connection, databaseMetaData,
                                           errInfo)) {
        std::string errMsg = errInfo.errMsg;
        BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(databaseMetaData.Get() != nullptr);
}

BOOST_AUTO_TEST_CASE(TestDatabaseMetaDataGetTables) {
    PrepareContext();
    BOOST_REQUIRE(_ctx.Get() != nullptr);

    JniErrorInfo errInfo;
    SharedPointer< GlobalJObject > connection;
    bool success = _ctx.Get()->DriverManagerGetConnection(
        _jdbcConnectionString.c_str(), connection, errInfo);
    if (!success || errInfo.code != odbc::java::IGNITE_JNI_ERR_SUCCESS) {
        BOOST_FAIL(errInfo.errMsg);
    }
    BOOST_REQUIRE(connection.Get());
    AutoCloseConnection autoCloseConnection(_ctx, connection);

    SharedPointer< GlobalJObject > databaseMetaData;
    if (!_ctx.Get()->ConnectionGetMetaData(connection, databaseMetaData, errInfo)) {
        std::string errMsg = errInfo.errMsg;
        BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(databaseMetaData.Get());

    std::string catalog;
    std::string schemaPattern;
    std::string tableNamePattern;
    std::vector< std::string > types({"TABLE"}); // Need to specify this to get result.
    SharedPointer< GlobalJObject > resultSet;
    if (!_ctx.Get()->DatabaseMetaDataGetTables(databaseMetaData, catalog,
                                               schemaPattern, tableNamePattern,
                                               types, resultSet, errInfo)) {
        std::string errMsg = errInfo.errMsg;
        BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(resultSet.Get());
    AutoCloseResultSet autoCloseResultSet(_ctx, resultSet);

    // Get first
    bool hasNext;
    if (!_ctx.Get()->ResultSetNext(resultSet, hasNext, errInfo)) {
        std::string errMsg = errInfo.errMsg;
        BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(hasNext);

    int i = 1;
    while (hasNext) {

        bool wasNull;
        std::string value;
        // TABLE_CAT (i.e., catalog - always NULL in our case)
        if (!_ctx.Get()->ResultSetGetString(resultSet, 1, value, wasNull,
                                            errInfo)) {
            std::string errMsg = errInfo.errMsg;
            BOOST_FAIL(errMsg);
        }
        BOOST_REQUIRE(wasNull);

        // TABLE_CAT (i.e., catalog - always NULL in our case)
        if (!_ctx.Get()->ResultSetGetString(resultSet, "TABLE_CAT", value, wasNull,
                                            errInfo)) {
            std::string errMsg = errInfo.errMsg;
            BOOST_FAIL(errMsg);
        }
        BOOST_REQUIRE(wasNull);

        // TABLE_SCHEM (i.e., database)
        if (!_ctx.Get()->ResultSetGetString(resultSet, 2, value, wasNull,
                                            errInfo)) {
            std::string errMsg = errInfo.errMsg;
            BOOST_FAIL(errMsg);
        }
        BOOST_REQUIRE(!wasNull);
        BOOST_REQUIRE(value == "test");

        // TABLE_SCHEM (i.e., database)
        if (!_ctx.Get()->ResultSetGetString(resultSet, "TABLE_SCHEM", value, wasNull,
                                            errInfo)) {
            std::string errMsg = errInfo.errMsg;
            BOOST_FAIL(errMsg);
        }
        BOOST_REQUIRE(!wasNull);
        BOOST_REQUIRE(value == "test");

        // TABLE_NAME
        if (!_ctx.Get()->ResultSetGetString(resultSet, 3, value, wasNull,
                                            errInfo)) {
            std::string errMsg = errInfo.errMsg;
            BOOST_FAIL(errMsg);
        }
        BOOST_REQUIRE(!wasNull);
        BOOST_REQUIRE(value.size() > 0);

        // TABLE_NAME
        if (!_ctx.Get()->ResultSetGetString(resultSet, "TABLE_NAME", value, wasNull,
                                            errInfo)) {
            std::string errMsg = errInfo.errMsg;
            BOOST_FAIL(errMsg);
        }
        BOOST_REQUIRE(!wasNull);
        BOOST_REQUIRE(value.size() > 0);

        // TABLE_TYPE
        if (!_ctx.Get()->ResultSetGetString(resultSet, 4, value, wasNull,
                                            errInfo)) {
            std::string errMsg = errInfo.errMsg;
            BOOST_FAIL(errMsg);
        }
        BOOST_REQUIRE(!wasNull);
        BOOST_REQUIRE(value == "TABLE");

        // TABLE_TYPE
        if (!_ctx.Get()->ResultSetGetString(resultSet, "TABLE_TYPE", value, wasNull,
                                            errInfo)) {
            std::string errMsg = errInfo.errMsg;
            BOOST_FAIL(errMsg);
        }
        BOOST_REQUIRE(!wasNull);
        BOOST_REQUIRE(value == "TABLE");

        // check getRow
        int val;
        if (!_ctx.Get()->ResultSetGetRow(resultSet, val, wasNull, errInfo)) {
            std::string errMsg = errInfo.errMsg;
            BOOST_FAIL(errMsg);
        }
        BOOST_CHECK_EQUAL(val, i);

        // Get next
        if (!_ctx.Get()->ResultSetNext(resultSet, hasNext, errInfo)) {
            std::string errMsg = errInfo.errMsg;
            BOOST_FAIL(errMsg);
        }

        i++;
    }
}

// note: might need to add some values to test in the future?
BOOST_AUTO_TEST_CASE(TestDatabaseMetaDataGetColumns) {
    PrepareContext();
    BOOST_REQUIRE(_ctx.Get() != nullptr);

    JniErrorInfo errInfo;
    SharedPointer< GlobalJObject > connection; 
    // get connection
    bool success = _ctx.Get()->DriverManagerGetConnection(
        _jdbcConnectionString.c_str(), connection, errInfo);
    if (!success || errInfo.code != odbc::java::IGNITE_JNI_ERR_SUCCESS) {
        BOOST_FAIL(errInfo.errMsg);
    }
    BOOST_REQUIRE(connection.Get());
    AutoCloseConnection autoCloseConnection(_ctx, connection);

    // get databaseMetaData object
    SharedPointer< GlobalJObject > databaseMetaData;
    if (!_ctx.Get()->ConnectionGetMetaData(connection, databaseMetaData,
                                           errInfo)) {
        std::string errMsg = errInfo.errMsg;
        BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(databaseMetaData.Get());

    std::string catalog;
    std::string schemaPattern;
    std::string tableNamePattern;
    std::string columnNamePattern;
    SharedPointer< GlobalJObject > resultSet;
    if (!_ctx.Get()->DatabaseMetaDataGetColumns(databaseMetaData, catalog,
                                               schemaPattern, tableNamePattern,
                                               columnNamePattern, resultSet, errInfo)) {
        std::string errMsg = errInfo.errMsg;
        BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(resultSet.Get());
    AutoCloseResultSet autoCloseResultSet(_ctx, resultSet);

    // Get first
    bool hasNext;
    if (!_ctx.Get()->ResultSetNext(resultSet, hasNext, errInfo)) {
        std::string errMsg = errInfo.errMsg;
        BOOST_FAIL(errMsg);
    }
    BOOST_REQUIRE(hasNext);

    int i = 1;
    while (hasNext) {
        bool wasNull;
        std::string value;
        // TABLE_CAT (i.e., catalog - always NULL in our case)
        if (!_ctx.Get()->ResultSetGetString(
                resultSet, 1, value, wasNull,
                errInfo)) {  // grab string from first column
            std::string errMsg = errInfo.errMsg;
            BOOST_FAIL(errMsg);
        }
        BOOST_REQUIRE(wasNull);

        // TABLE_CAT (i.e., catalog - always NULL in our case)
        if (!_ctx.Get()->ResultSetGetString(
                resultSet, "TABLE_CAT", value, wasNull,
                errInfo)) {  // grab string from first column by name
            std::string errMsg = errInfo.errMsg;
            BOOST_FAIL(errMsg);
        }
        BOOST_REQUIRE(wasNull);

        // TABLE_SCHEM (i.e., database)
        if (!_ctx.Get()->ResultSetGetString(resultSet, 2, value, wasNull,
                                            errInfo)) {
            std::string errMsg = errInfo.errMsg;
            BOOST_FAIL(errMsg);
        }
        BOOST_REQUIRE(!wasNull);
        BOOST_REQUIRE(value == "test");

        // TABLE_SCHEM (i.e., database)
        if (!_ctx.Get()->ResultSetGetString(resultSet, "TABLE_SCHEM", value,
                                            wasNull, errInfo)) {
            std::string errMsg = errInfo.errMsg;
            BOOST_FAIL(errMsg);
        }
        BOOST_REQUIRE(!wasNull);
        BOOST_REQUIRE(value == "test");

        // TABLE_NAME
        if (!_ctx.Get()->ResultSetGetString(resultSet, 3, value, wasNull,
                                            errInfo)) {
            std::string errMsg = errInfo.errMsg;
            BOOST_FAIL(errMsg);
        }
        BOOST_REQUIRE(!wasNull);
        BOOST_REQUIRE(value.size() > 0);

        // TABLE_NAME
        if (!_ctx.Get()->ResultSetGetString(resultSet, "TABLE_NAME", value,
                                            wasNull, errInfo)) {
            std::string errMsg = errInfo.errMsg;
            BOOST_FAIL(errMsg);
        }
        BOOST_REQUIRE(!wasNull);
        BOOST_REQUIRE(value.size() > 0);

        // COLUMN_NAME
        if (!_ctx.Get()->ResultSetGetString(resultSet, 4, value, wasNull,
                                            errInfo)) {
            std::string errMsg = errInfo.errMsg;
            BOOST_FAIL(errMsg);
        }
        BOOST_REQUIRE(!wasNull);
        BOOST_REQUIRE(value.size() > 0);

        // COLUMN_NAME
        if (!_ctx.Get()->ResultSetGetString(resultSet, "COLUMN_NAME", value,
                                            wasNull, errInfo)) {
            std::string errMsg = errInfo.errMsg;
            BOOST_FAIL(errMsg);
        }
        BOOST_REQUIRE(!wasNull);
        BOOST_REQUIRE(value.size() > 0);

        // ORDINAL_POSITION
        int val;
        if (!_ctx.Get()->ResultSetGetInteger(resultSet, 17, val, wasNull,
                                            errInfo)) {
            std::string errMsg = errInfo.errMsg;
            BOOST_FAIL(errMsg);
        }
        BOOST_REQUIRE(!wasNull);
        BOOST_REQUIRE(val > 0);

        // ORDINAL_POSITION
        if (!_ctx.Get()->ResultSetGetInteger(resultSet, "ORDINAL_POSITION", val,
                                            wasNull, errInfo)) {
            std::string errMsg = errInfo.errMsg;
            BOOST_FAIL(errMsg);
        }
        BOOST_REQUIRE(!wasNull);
        BOOST_REQUIRE(val > 0);

        
        // todo test getShort, just check for errors because it could be null -AL-
        // SOURCE_DATA_TYPE
        short valShort;
        if (!_ctx.Get()->ResultSetGetShort(resultSet, 22, valShort, wasNull,
                                             errInfo)) {
            std::string errMsg = errInfo.errMsg;
            BOOST_FAIL(errMsg);
        }

        // SOURCE_DATA_TYPE
        if (!_ctx.Get()->ResultSetGetShort(resultSet, "SOURCE_DATA_TYPE",
                                           valShort, wasNull, errInfo)) {
            std::string errMsg = errInfo.errMsg;
            BOOST_FAIL(errMsg);
        }

        // check getRow
        if (!_ctx.Get()->ResultSetGetRow(resultSet, val, wasNull, errInfo)) {
            std::string errMsg = errInfo.errMsg;
            BOOST_FAIL(errMsg);
        }
        BOOST_CHECK_EQUAL(val, i);

        // Get next
        if (!_ctx.Get()->ResultSetNext(resultSet, hasNext, errInfo)) {
            std::string errMsg = errInfo.errMsg;
            BOOST_FAIL(errMsg);
        }

        i++;
    }
}

BOOST_AUTO_TEST_SUITE_END()
