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

#include <cstring>
#include <cstddef>

#include <sstream>
#include <algorithm>

#include <ignite/odbc/ignite_error.h>
#include <ignite/network/network.h>

#include "ignite/odbc/log.h"
#include "ignite/odbc/utility.h"
#include "ignite/odbc/environment.h"
#include "ignite/odbc/statement.h"
#include "ignite/odbc/connection.h"
#include "ignite/odbc/message.h"
#include "ignite/odbc/ssl_mode.h"
#include "ignite/odbc/dsn_config.h"
#include "ignite/odbc/config/configuration.h"
#include "ignite/odbc/config/connection_string_parser.h"
#include "ignite/odbc/system/system_dsn.h"
#include "ignite/odbc/jni/java.h"
#include "ignite/odbc/jni/utils.h"
#include "ignite/odbc/common/concurrent.h"
#include "ignite/odbc/common/utils.h"

// Uncomment for per-byte debug.
//#define PER_BYTE_DEBUG

namespace
{
#pragma pack(push, 1)
    struct OdbcProtocolHeader
    {
        int32_t len;
    };
#pragma pack(pop)
}


namespace ignite
{
    namespace odbc
    {
        Connection::Connection(Environment* env) :
            env(env),
            timeout(0),
            loginTimeout(DEFAULT_CONNECT_TIMEOUT),
            autoCommit(true),
            parser(),
            config(),
            info(config),
            connection(),
            opts()
        {
            // No-op
        }

        Connection::~Connection()
        {
            // No-op.
        }

        const config::ConnectionInfo& Connection::GetInfo() const
        {
            return info;
        }

        void Connection::GetInfo(config::ConnectionInfo::InfoType type, void* buf, short buflen, short* reslen)
        {
            LOG_MSG("SQLGetInfo called: "
                << type << " ("
                << config::ConnectionInfo::InfoTypeToString(type) << "), "
                << std::hex << reinterpret_cast<size_t>(buf) << ", "
                << buflen << ", "
                << std::hex << reinterpret_cast<size_t>(reslen)
                << std::dec);

            IGNITE_ODBC_API_CALL(InternalGetInfo(type, buf, buflen, reslen));
        }

        SqlResult::Type Connection::InternalGetInfo(config::ConnectionInfo::InfoType type, void* buf, short buflen, short* reslen)
        {
            const config::ConnectionInfo& info = GetInfo();

            SqlResult::Type res = info.GetInfo(type, buf, buflen, reslen);

            if (res != SqlResult::AI_SUCCESS)
                AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED, "Not implemented.");

            return res;
        }

        void Connection::Establish(const std::string& connectStr, void* parentWindow)
        {
            IGNITE_ODBC_API_CALL(InternalEstablish(connectStr, parentWindow));
        }

        SqlResult::Type Connection::InternalEstablish(const std::string& connectStr, void* parentWindow)
        {
            config::Configuration config;
            config::ConnectionStringParser parser(config);
            parser.ParseConnectionString(connectStr, &GetDiagnosticRecords());

#ifdef _WIN32
            if (parentWindow)
            {
                LOG_MSG("Parent window is passed. Creating configuration window.");
                if (!DisplayConnectionWindow(parentWindow, config))
                {
                    AddStatusRecord(odbc::SqlState::SHY008_OPERATION_CANCELED, "Connection canceled by user");

                    return SqlResult::AI_ERROR;
                }
            }
#endif  // _WIN32

            if (config.IsDsnSet())
            {
                std::string dsn = config.GetDsn();

                ReadDsnConfiguration(dsn.c_str(), config, &GetDiagnosticRecords());
            }

            return InternalEstablish(config);
        }

        void Connection::Establish(const config::Configuration cfg)
        {
            IGNITE_ODBC_API_CALL(InternalEstablish(cfg));
        }

        SqlResult::Type Connection::InternalEstablish(const config::Configuration& cfg)
        {
            using ssl::SslMode;

            config = cfg;

            if (connection) {
                AddStatusRecord(SqlState::S08002_ALREADY_CONNECTED,
                                "Already connected.");

                return SqlResult::AI_ERROR;
            }

            if (!config.IsHostnameSet())
            {
                AddStatusRecord("No valid address to connect.");

                return SqlResult::AI_ERROR;
            }

            odbc::IgniteError err;
            bool connected = TryRestoreConnection(err);

            if (!connected)
            {
                std::string errMessage = "Failed to establish connection with the host.\n";
                errMessage.append(err.GetText());
                AddStatusRecord(
                    SqlState::S08001_CANNOT_CONNECT, errMessage);

                return SqlResult::AI_ERROR;
            }

            bool errors = GetDiagnosticRecords().GetStatusRecordsNumber() > 0;

            return errors ? SqlResult::AI_SUCCESS_WITH_INFO : SqlResult::AI_SUCCESS;
        }

        void Connection::Release()
        {
            IGNITE_ODBC_API_CALL(InternalRelease());
        }

        void Connection::Deregister()
        {
            env->DeregisterConnection(this);
        }

        SqlResult::Type Connection::InternalRelease()
        {
            if (!connection)
            {
                AddStatusRecord(SqlState::S08003_NOT_CONNECTED, "Connection is not open.");

                // It is important to return SUCCESS_WITH_INFO and not ERROR here, as if we return an error, Windows
                // Driver Manager may decide that connection is not valid anymore which results in memory leak.
                return SqlResult::AI_SUCCESS_WITH_INFO;
            }

            Close();

            return SqlResult::AI_SUCCESS;
        }

        void Connection::Close()
        {
            if (connection) {
                using namespace jni::java;
                using namespace common::concurrent;
                SharedPointer< JniContext > ctx(JniContext::Create(&opts[0], static_cast<int>(opts.size()), JniHandlers()));
                JniErrorInfo errInfo;
                // NOTE: DocumentDbDisconnect will notify JNI connection is no longer used - must set to nullptr.
                ctx.Get()->DocumentDbDisconnect(connection, &errInfo);
                if (errInfo.code != java::IGNITE_JNI_ERR_SUCCESS) {
                    // TODO: Determine if we need to error check the close.
                }
                connection = nullptr;
            }
            Deinit();
        }

        Statement* Connection::CreateStatement()
        {
            Statement* statement;

            IGNITE_ODBC_API_CALL(InternalCreateStatement(statement));

            return statement;
        }

        SqlResult::Type Connection::InternalCreateStatement(Statement*& statement)
        {
            statement = new Statement(*this);

            if (!statement)
            {
                AddStatusRecord(SqlState::SHY001_MEMORY_ALLOCATION, "Not enough memory.");

                return SqlResult::AI_ERROR;
            }

            return SqlResult::AI_SUCCESS;
        }

        bool Connection::Send(const int8_t* data, size_t len, int32_t timeout)
        {
            // TODO: Remove if unnecessary
            return true;
        }

        Connection::OperationResult::T Connection::SendAll(const int8_t* data, size_t len, int32_t timeout)
        {
            // TODO: Remove if unnecessary.
            // No-op
            return OperationResult::SUCCESS;
        }

        bool Connection::Receive(std::vector<int8_t>& msg, int32_t timeout)
        {
            // TODO: Remove if unnecessary.
            return true;
        }

        Connection::OperationResult::T Connection::ReceiveAll(void* dst, size_t len, int32_t timeout)
        {
            // TODO: Remove if unnecessary.
            return OperationResult::SUCCESS;
        }

        const std::string& Connection::GetSchema() const
        {
            return config.GetDatabase();
        }

        const config::Configuration& Connection::GetConfiguration() const
        {
            return config;
        }

        bool Connection::IsAutoCommit() const
        {
            return autoCommit;
        }

        diagnostic::DiagnosticRecord Connection::CreateStatusRecord(SqlState::Type sqlState,
            const std::string& message, int32_t rowNum, int32_t columnNum)
        {
            return diagnostic::DiagnosticRecord(sqlState, message, "", "", rowNum, columnNum);
        }

        void Connection::TransactionCommit()
        {
            IGNITE_ODBC_API_CALL(InternalTransactionCommit());
        }

        SqlResult::Type Connection::InternalTransactionCommit()
        {
            std::string schema = config.GetDatabase();

            app::ParameterSet empty;

            QueryExecuteRequest req(schema, "COMMIT", empty, timeout, autoCommit);
            QueryExecuteResponse rsp;

            try
            {
                bool sent = SyncMessage(req, rsp, timeout);

                if (!sent)
                {
                    AddStatusRecord(SqlState::S08S01_LINK_FAILURE, "Failed to send commit request.");

                    return SqlResult::AI_ERROR;
                }
            }
            catch (const OdbcError& err)
            {
                AddStatusRecord(err);

                return SqlResult::AI_ERROR;
            }
            catch (const odbc::IgniteError& err)
            {
                AddStatusRecord(err.GetText());

                return SqlResult::AI_ERROR;
            }

            return SqlResult::AI_SUCCESS;
        }

        void Connection::TransactionRollback()
        {
            IGNITE_ODBC_API_CALL(InternalTransactionRollback());
        }

        SqlResult::Type Connection::InternalTransactionRollback()
        {
            std::string schema = config.GetDatabase();

            app::ParameterSet empty;

            QueryExecuteRequest req(schema, "ROLLBACK", empty, timeout, autoCommit);
            QueryExecuteResponse rsp;

            try
            {
                bool sent = SyncMessage(req, rsp, timeout);

                if (!sent)
                {
                    AddStatusRecord(SqlState::S08S01_LINK_FAILURE, "Failed to send rollback request.");

                    return SqlResult::AI_ERROR;
                }
            }
            catch (const OdbcError& err)
            {
                AddStatusRecord(err);

                return SqlResult::AI_ERROR;
            }
            catch (const odbc::IgniteError& err)
            {
                AddStatusRecord(err.GetText());

                return SqlResult::AI_ERROR;
            }

            return SqlResult::AI_SUCCESS;
        }

        void Connection::GetAttribute(int attr, void* buf, SQLINTEGER bufLen, SQLINTEGER* valueLen)
        {
            IGNITE_ODBC_API_CALL(InternalGetAttribute(attr, buf, bufLen, valueLen));
        }

        SqlResult::Type Connection::InternalGetAttribute(int attr, void* buf, SQLINTEGER, SQLINTEGER* valueLen)
        {
            if (!buf)
            {
                AddStatusRecord(SqlState::SHY009_INVALID_USE_OF_NULL_POINTER, "Data buffer is null.");

                return SqlResult::AI_ERROR;
            }

            switch (attr)
            {
                case SQL_ATTR_CONNECTION_DEAD:
                {
                    SQLUINTEGER *val = reinterpret_cast<SQLUINTEGER*>(buf);

                    *val = connection ? SQL_CD_FALSE : SQL_CD_TRUE;

                    if (valueLen)
                        *valueLen = SQL_IS_INTEGER;

                    break;
                }

                case SQL_ATTR_CONNECTION_TIMEOUT:
                {
                    SQLUINTEGER *val = reinterpret_cast<SQLUINTEGER*>(buf);

                    *val = static_cast<SQLUINTEGER>(timeout);

                    if (valueLen)
                        *valueLen = SQL_IS_INTEGER;

                    break;
                }

                case SQL_ATTR_LOGIN_TIMEOUT:
                {
                    SQLUINTEGER *val = reinterpret_cast<SQLUINTEGER*>(buf);

                    *val = static_cast<SQLUINTEGER>(loginTimeout);

                    if (valueLen)
                        *valueLen = SQL_IS_INTEGER;

                    break;
                }

                case SQL_ATTR_AUTOCOMMIT:
                {
                    SQLUINTEGER *val = reinterpret_cast<SQLUINTEGER*>(buf);

                    *val = autoCommit ? SQL_AUTOCOMMIT_ON : SQL_AUTOCOMMIT_OFF;

                    if (valueLen)
                        *valueLen = SQL_IS_INTEGER;

                    break;
                }

                default:
                {
                    AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                        "Specified attribute is not supported.");

                    return SqlResult::AI_ERROR;
                }
            }

            return SqlResult::AI_SUCCESS;
        }

        void Connection::SetAttribute(int attr, void* value, SQLINTEGER valueLen)
        {
            IGNITE_ODBC_API_CALL(InternalSetAttribute(attr, value, valueLen));
        }

        SqlResult::Type Connection::InternalSetAttribute(int attr, void* value, SQLINTEGER)
        {
            switch (attr)
            {
                case SQL_ATTR_CONNECTION_DEAD:
                {
                    AddStatusRecord(SqlState::SHY092_OPTION_TYPE_OUT_OF_RANGE, "Attribute is read only.");

                    return SqlResult::AI_ERROR;
                }

                case SQL_ATTR_CONNECTION_TIMEOUT:
                {
                    timeout = RetrieveTimeout(value);

                    if (GetDiagnosticRecords().GetStatusRecordsNumber() != 0)
                        return SqlResult::AI_SUCCESS_WITH_INFO;

                    break;
                }

                case SQL_ATTR_LOGIN_TIMEOUT:
                {
                    loginTimeout = RetrieveTimeout(value);

                    if (GetDiagnosticRecords().GetStatusRecordsNumber() != 0)
                        return SqlResult::AI_SUCCESS_WITH_INFO;

                    break;
                }

                case SQL_ATTR_AUTOCOMMIT:
                {
                    SQLUINTEGER mode = static_cast<SQLUINTEGER>(reinterpret_cast<ptrdiff_t>(value));

                    if (mode != SQL_AUTOCOMMIT_ON && mode != SQL_AUTOCOMMIT_OFF)
                    {
                        AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                            "Specified attribute is not supported.");

                        return SqlResult::AI_ERROR;
                    }

                    autoCommit = mode == SQL_AUTOCOMMIT_ON;

                    break;
                }

                default:
                {
                    AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                        "Specified attribute is not supported.");

                    return SqlResult::AI_ERROR;
                }
            }

            return SqlResult::AI_SUCCESS;
        }

        SqlResult::Type Connection::MakeRequestHandshake()
        {
            HandshakeRequest req(config);
            HandshakeResponse rsp;

            try
            {
                // Workaround for some Linux systems that report connection on non-blocking
                // sockets as successful but fail to establish real connection.
                bool sent = InternalSyncMessage(req, rsp, loginTimeout);

                if (!sent)
                {
                    AddStatusRecord(SqlState::S08001_CANNOT_CONNECT,
                        "Failed to get handshake response (Did you forget to enable SSL?).");

                    return SqlResult::AI_ERROR;
                }
            }
            catch (const OdbcError& err)
            {
                AddStatusRecord(err);

                return SqlResult::AI_ERROR;
            }
            catch (const odbc::IgniteError& err)
            {
                AddStatusRecord(SqlState::S08004_CONNECTION_REJECTED, err.GetText());

                return SqlResult::AI_ERROR;
            }

            if (!rsp.IsAccepted())
            {
                LOG_MSG("Handshake message has been rejected.");

                std::stringstream constructor;

                constructor << "Node rejected handshake message. ";

                if (!rsp.GetError().empty())
                    constructor << "Additional info: " << rsp.GetError() << " ";

                AddStatusRecord(SqlState::S08004_CONNECTION_REJECTED, constructor.str());

                return SqlResult::AI_ERROR;
            }

            return SqlResult::AI_SUCCESS;
        }

        void Connection::EnsureConnected()
        {
            if (connection)
                return;

            odbc::IgniteError err;
            bool success = TryRestoreConnection(err);

            if (!success)
                throw OdbcError(SqlState::S08001_CANNOT_CONNECT,
                    "Failed to establish connection with any provided hosts");
        }

        bool Connection::TryRestoreConnection(odbc::IgniteError& err)
        {
            bool connected = false;

            using namespace jni::java;
            using namespace common::concurrent;

            if (connection) {
                return true;
            }

            std::string connectionString = FormatJdbcConnectionString();
            JniErrorInfo errInfo;

            std::string docdb_home = common::GetEnv("DOCUMENTDB_HOME")
                                     + "\\documentdb-jdbc-1.1.0-all.jar";
            
            // 2. Resolve DOCUMENTDB_HOME.
            std::string home = jni::ResolveDocumentDbHome();

            // 3. Create classpath.
            std::string cp = jni::CreateDocumentDbClasspath(std::string(), home);

            if (cp.empty()) {
                err =
                    odbc::IgniteError(odbc::IgniteError::IGNITE_ERR_JVM_NO_CLASSPATH,
                                  "Java classpath is empty (did you set "
                                  "DOCUMENTDB_HOME environment variable?)");

                return false;
            }

            SetJvmOptions(cp);
            
            SharedPointer< JniContext > ctx(JniContext::Create(&opts[0], static_cast<int>(opts.size()), JniHandlers(), &errInfo));
            if (ctx.Get()) {
                jobject result = ctx.Get()->DocumentDbConnect(
                    connectionString.c_str(), &errInfo);
                connected = (result && errInfo.code == java::IGNITE_JNI_ERR_SUCCESS);
                if (!connected) {
                    err = odbc::IgniteError(
                        odbc::IgniteError::IGNITE_ERR_SECURE_CONNECTION_FAILURE,
                        errInfo.errMsg);
                    Close();
                }
                connection = result;
            } else {
                err = odbc::IgniteError(odbc::IgniteError::IGNITE_ERR_JVM_INIT, "Unable to get initialized JVM.");
                connection = nullptr;
            }

            return connected;
        }

        std::string Connection::FormatJdbcConnectionString() const {
            std::string host = "localhost";
            std::string port = "27017";
            std::string jdbConnectionString;

            if (config.IsHostnameSet()) {
                host = config.GetHostname();
            }

            if (config.IsPortSet()) {
                port = std::to_string(config.GetPort());
            }

            jdbConnectionString = "jdbc:documentdb:";
            jdbConnectionString.append("//" + config.GetUser());
            jdbConnectionString.append(":" + config.GetPassword());
            jdbConnectionString.append("@" + host);
            jdbConnectionString.append(":" + port);
            jdbConnectionString.append("/" + config.GetDatabase());
            jdbConnectionString.append("?tlsAllowInvalidHostnames=true");

            // Check if internal SSH tunnel should be enabled.
            // TODO: Remove use of environment variables and use DSN properties
            std::string sshUserAtHost = common::GetEnv("DOC_DB_USER", "");
            std::string sshRemoteHost = common::GetEnv("DOC_DB_HOST", "");
            std::string sshPrivKeyFile = common::GetEnv("DOC_DB_PRIV_KEY_FILE", "");
            std::string sshUser;
            std::string sshTunnelHost;
            size_t indexOfAt = sshUserAtHost.find_first_of('@');
            if (indexOfAt >= 0 && sshUserAtHost.size() > (indexOfAt + 1)) {
                sshUser = sshUserAtHost.substr(0, indexOfAt);
                sshTunnelHost = sshUserAtHost.substr(indexOfAt + 1);
            }
            if (!sshUserAtHost.empty()
                && !sshRemoteHost.empty()
                && !sshPrivKeyFile.empty()
                && !sshUser.empty()
                && !sshTunnelHost.empty()) {
                jdbConnectionString.append("&sshUser=" + sshUser);
                jdbConnectionString.append("&sshHost=" + sshTunnelHost);
                jdbConnectionString.append("&sshPrivateKeyFile=" + sshPrivKeyFile);
                jdbConnectionString.append("&sshStrictHostKeyChecking=false");
            }

            return jdbConnectionString;
        }

         /**
         * Create JVM options from configuration.
         *
         * @param cfg Configuration.
         * @param home Optional GG home.
         * @param cp Classpath.
         */
        void Connection::SetJvmOptions(const std::string& cp) {
            using namespace common;
            Deinit();

            const size_t REQ_OPTS_CNT = 4;
            const size_t JAVA9_OPTS_CNT = 6;

            opts.reserve(REQ_OPTS_CNT + JAVA9_OPTS_CNT);

            // 1. Set classpath.
            std::string cpFull = "-Djava.class.path=" + cp;

            opts.push_back(CopyChars(cpFull.c_str()));

            // 3. Set Xms, Xmx.
            std::string xmsStr = "-Xms" + std::to_string(256) + "m";
            std::string xmxStr = "-Xmx" + std::to_string(1024) + "m";

            opts.push_back(CopyChars(xmsStr.c_str()));
            opts.push_back(CopyChars(xmxStr.c_str()));

            // 4. Optional debug arguments
            //std::string debugStr = "-agentlib:jdwp=transport=dt_socket,server=y,suspend=n,address=5005";
            //opts.push_back(CopyChars(debugStr.c_str()));

            // 5. Set file.encoding.
            std::string fileEncParam = "-Dfile.encoding=";
            std::string fileEncFull = fileEncParam + "UTF-8";
            opts.push_back(CopyChars(fileEncFull.c_str()));

            // Adding options for Java 9 or later
            if (jni::java::IsJava9OrLater()) {
                opts.push_back(CopyChars(
                    "--add-exports=java.base/jdk.internal.misc=ALL-UNNAMED"));
                opts.push_back(CopyChars(
                    "--add-exports=java.base/sun.nio.ch=ALL-UNNAMED"));
                opts.push_back(CopyChars(
                  "--add-exports=java.management/com.sun.jmx.mbeanserver=ALL-UNNAMED"));
                opts.push_back(CopyChars(
                  "--add-exports=jdk.internal.jvmstat/sun.jvmstat.monitor=ALL-UNNAMED"));
                opts.push_back(CopyChars(
                    "--add-exports=java.base/sun.reflect.generics.reflectiveObjects=ALL-UNNAMED"));
                opts.push_back(CopyChars(
                  "--add-opens=jdk.management/com.sun.management.internal=ALL-UNNAMED"));
            }
        }

        /**
         * Deallocates all allocated data.
         */
        void Connection::Deinit() {
            using namespace common;
            std::for_each(opts.begin(), opts.end(), ReleaseChars);
            opts.clear();
        }
        
        void Connection::CollectAddresses(const config::Configuration& cfg, std::vector<EndPoint>& endPoints)
        {
            endPoints.clear();

            // DocumentDB driver is currently not supporting list of addresses. Return vector with the one address for now.
            LOG_MSG("'Address' is not set. Using legacy connection method.");
            endPoints.push_back(EndPoint(cfg.GetHostname(), cfg.GetPort()));
            return;
        }

        int32_t Connection::RetrieveTimeout(void* value)
        {
            SQLUINTEGER uTimeout = static_cast<SQLUINTEGER>(reinterpret_cast<ptrdiff_t>(value));

            if (uTimeout > INT32_MAX)
            {
                std::stringstream ss;

                ss << "Value is too big: " << uTimeout << ", changing to " << timeout << ".";

                AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED, ss.str());

                return INT32_MAX;
            }

            return static_cast<int32_t>(uTimeout);
        }
    }
}

