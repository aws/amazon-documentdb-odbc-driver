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

#include <boost/test/unit_test.hpp>

#include "test_utils.h"
#include "odbc_test_suite.h"

using namespace ignite;
using namespace ignite_test;

using namespace boost::unit_test;

/**
 * Test setup fixture.
 */
struct ConnectionTestSuiteFixture: odbc::OdbcTestSuite
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

    /**
     * Destructor.
     */
    ~ConnectionTestSuiteFixture() override = default;
};


BOOST_FIXTURE_TEST_SUITE(ConnectionTestSuite, ConnectionTestSuiteFixture)

BOOST_AUTO_TEST_CASE(TestConnectionRestoreInternalSSHTunnel)
{
    std::string connectionString;
    CreateDsnConnectionString(connectionString);

    Connect(connectionString);
    Disconnect();
}

BOOST_AUTO_TEST_CASE(TestConnectionRestoreExternalSSHTunnel)
{
    std::string connectionString;
    CreateDsnConnectionString(connectionString, std::string(), false);

    Connect(connectionString);
    Disconnect();
}

BOOST_AUTO_TEST_CASE(TestConnectionInvalidUser) {
    std::string connectionString;
    CreateDsnConnectionString(connectionString, "invaliduser");

    ExpectConnectionReject(connectionString, "08001: Failed to establish connection with the host.\n"
      "Invalid username or password or user is not authorized on database 'test'. "
      "Please check your settings. Authorization failed for user 'invaliduser' on database 'admin' with mechanism");

    Disconnect();
}

BOOST_AUTO_TEST_SUITE_END()
