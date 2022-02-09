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
    /**
     * Constructor.
     */
    ConnectionTestSuiteFixture() :
        OdbcTestSuite()
    {
        // No-op.
    }

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

    static void SetConnectionString(std::string& connectionString,
                                    const std::string& username = std::string(), 
                                    boolean sshTunnel = true) {
        std::string user = common::GetEnv("DOC_DB_USER_NAME", "documentdb");
        std::string password = common::GetEnv("DOC_DB_PASSWORD", "");
        std::string host = common::GetEnv("DOC_DB_HOST", "");
        std::string sshUserAtHost = common::GetEnv("DOC_DB_USER", "");
        std::string sshRemoteHost = common::GetEnv("DOC_DB_HOST", "");
        std::string sshPrivKeyFile = common::GetEnv("DOC_DB_PRIV_KEY_FILE", "");
        std::string port = "27017";
        
        if (!username.empty()) {
            user = username;
        }
        
        std::string sshUser;
        std::string sshTunnelHost;
        size_t indexOfAt = sshUserAtHost.find_first_of('@');
        if (indexOfAt != std::string::npos && sshUserAtHost.size() > (indexOfAt + 1)) {
            sshUser = sshUserAtHost.substr(0, indexOfAt);
            sshTunnelHost = sshUserAtHost.substr(indexOfAt + 1);
        }

        connectionString =
            "DRIVER={Apache Ignite};"
            "HOSTNAME=" + host + ":" + port + ";"
            "DATABASE=test;"
            "USER=" + user + ";"
            "PASSWORD=" + password + ";"
            "TLS_ALLOW_INVALID_HOSTNAMES=true;";
        
        if (sshTunnel 
            && !sshUserAtHost.empty()
            && !sshRemoteHost.empty()
            && !sshPrivKeyFile.empty()
            && !sshUser.empty()
            && !sshTunnelHost.empty()) {
            connectionString.append("SSH_USER=" + sshUser + ';');
            connectionString.append("SSH_HOST=" + sshTunnelHost + ';');
            connectionString.append("SSH_PRIVATE_KEY_FILE=" + sshPrivKeyFile + ';');
            connectionString.append("SSH_STRICT_HOST_KEY_CHECKING=false;");
        }
    }

    /**
     * Destructor.
     */
    ~ConnectionTestSuiteFixture()
    {
        // No-op.
    }
};


BOOST_FIXTURE_TEST_SUITE(ConnectionTestSuite, ConnectionTestSuiteFixture)

BOOST_AUTO_TEST_CASE(TestConnectionRestore)
{
    std::string connectionString;
    SetConnectionString(connectionString);

    Connect(connectionString);
    Disconnect();

    // TODO: [AD-507] Re-enable when querying is supported.
    // https://bitquill.atlassian.net/browse/AD-507

    //// Check that query was successfully executed.
    //BOOST_CHECK_EQUAL(ExecQueryAndReturnError(), "");

    //// Query execution should throw ODBC error.
    //BOOST_CHECK_EQUAL(ExecQueryAndReturnError(), "08S01");

    //// Reusing a closed connection should not crash an application.
    //BOOST_CHECK_EQUAL(ExecQueryAndReturnError(), "08001");

    //// Check that connection was restored.
    //BOOST_CHECK_EQUAL(ExecQueryAndReturnError(), "");

}

BOOST_AUTO_TEST_CASE(TestConnectionMemoryLeak)
{
    std::string connectionString;
    SetConnectionString(connectionString);

    Connect(connectionString);

    // TODO: [AD-507] Re-enable when querying is supported.
    // https://bitquill.atlassian.net/browse/AD-507
    // ExecQuery("Select * from Test");

    Disconnect();
}

BOOST_AUTO_TEST_CASE(TestConnectionInvalidUser) {
    std::string connectionString;
    SetConnectionString(connectionString, "invaliduser");

    ExpectConnectionReject(connectionString, "08001: Failed to establish connection with the host.\n"
      "Invalid username or password or user is not authorized on database 'test'. "
      "Please check your settings. Authorization failed for user 'invaliduser' on database 'admin' with mechanism");

    // TODO: [AD-507] Re-enable when querying is supported.
    // https://bitquill.atlassian.net/browse/AD-507
    // ExecQuery("Select * from Test");

    Disconnect();
}

BOOST_AUTO_TEST_SUITE_END()
