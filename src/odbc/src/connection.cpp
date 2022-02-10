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
#include "ignite/odbc/mongoInstance.h"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/stdx/make_unique.hpp>
#include <bsoncxx/stdx/optional.hpp>
#include <bsoncxx/stdx/string_view.hpp>

using namespace ignite::odbc::jni::java;
using namespace ignite::odbc::common;
using namespace ignite::odbc::common::concurrent;

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

            if (connection.Get() != nullptr) {
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
            if (connection.Get() == nullptr)
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
            if (connection.Get() != nullptr) {
                SharedPointer< JniContext > ctx(JniContext::Create(&opts[0], static_cast<int>(opts.size()), JniHandlers()));
                JniErrorInfo errInfo;
                ctx.Get()->ConnectionClose(connection, errInfo);
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

                    *val = connection.Get() ? SQL_CD_FALSE : SQL_CD_TRUE;

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
            if (connection.Get() != nullptr)
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

            if (connection.Get() != nullptr) {
                return true;
            }

            std::string connectionString = FormatJdbcConnectionString(config);
            JniErrorInfo errInfo;

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
            if (errInfo.code != java::IGNITE_JNI_ERR_SUCCESS) {
                IgniteError::SetError(errInfo.code, errInfo.errCls, errInfo.errMsg, err);
                return false;
            }
            if (ctx.Get() != nullptr) {
                SharedPointer< GlobalJObject > result;
                bool success = ctx.Get()->DriverManagerGetConnection(
                    connectionString.c_str(), result, errInfo);
                connected = (success && result.Get()
                             && errInfo.code == java::IGNITE_JNI_ERR_SUCCESS);
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

            if (!connected) {
                return connected;
            }
            
            
            bool isSSHTunnelActive;
            bool success = ctx.Get()->DocumentDbConnectionIsSshTunnelActive(
                connection, isSSHTunnelActive, errInfo);
            if (!success
                || errInfo.code != odbc::java::IGNITE_JNI_ERR_SUCCESS) {
                std::string errMsg = errInfo.errMsg;
                err = odbc::IgniteError(odbc::IgniteError::IGNITE_ERR_JVM_INIT,
                    errInfo.errMsg);
                return false;
            }
            int32_t localSSHTunnelPort = 0;
            if (isSSHTunnelActive) {
                bool success = ctx.Get()->DocumentDbConnectionGetSshLocalPort(
                    connection, localSSHTunnelPort, errInfo);  
                if (!success
                    || errInfo.code != odbc::java::IGNITE_JNI_ERR_SUCCESS) {
                    std::string errMsg = errInfo.errMsg;
                    err = odbc::IgniteError(
                        odbc::IgniteError::IGNITE_ERR_JVM_INIT,
                        errInfo.errMsg);
                    return false;
                }
            }
            connected = ConnectCPPDocumentDB(err, localSSHTunnelPort);

            return connected;
        }

        std::string Connection::FormatMongoCppConnectionString(
            int32_t localSSHTunnelPort) const {

            std::string host = "localhost";
            std::string port = std::to_string(localSSHTunnelPort);

            // localSSHTunnelPort == 0 means that internal SSH tunnel option was not set
            if (localSSHTunnelPort == 0) {
                host = config.GetHostname();
                port = config.GetPort();
            }
            std::string mongoConnectionString;

            mongoConnectionString = "mongodb:";
            mongoConnectionString.append("//" + config.GetUser());
            mongoConnectionString.append(":" + config.GetPassword());
            mongoConnectionString.append("@" + host);
            mongoConnectionString.append(":" + port);
            mongoConnectionString.append("/" + config.GetDatabase());
            mongoConnectionString.append("?tlsAllowInvalidHostnames=true");
            //mongoConnectionString.append("&tls=true");
  


            return mongoConnectionString;
        }

        std::string Connection::FormatJdbcConnectionString(
            const config::Configuration& config) {
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
            Deinit();
            BuildJvmOptions(cp, opts);
        }

        /**
         * Deallocates all allocated data.
         */
        void Connection::Deinit() {
            std::for_each(opts.begin(), opts.end(), ReleaseChars);
            opts.clear();
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

        bool Connection::ConnectCPPDocumentDB(odbc::IgniteError& err,
                                              int32_t localSSHTunnelPort) 
        {           
            using bsoncxx::builder::basic::kvp;
            using bsoncxx::builder::basic::make_document;

            MongoInstance::instance();
            try {
                std::string mongoCPPConnectionString =
                    FormatMongoCppConnectionString(localSSHTunnelPort);
                const auto uri = mongocxx::uri{mongoCPPConnectionString};
                mongocxx::options::client client_options;
                mongocxx::options::tls tls_options;

                //TO-DO Adapt to use certificates
                tls_options.allow_invalid_certificates(true);

                client_options.tls_opts(tls_options);
                auto client1 = mongocxx::client{
                    mongocxx::uri{mongoCPPConnectionString}, client_options};

                std::string database = "test";
                bsoncxx::builder::stream::document ping;
                ping << "ping" << 1;
                auto db = client1[database];
                auto result = db.run_command(ping.view());

                if (result.view()["ok"].get_double() != 1) {
                    err = odbc::IgniteError(
                        odbc::IgniteError::IGNITE_ERR_NETWORK_FAILURE,
                        "Unable to ping DocumentDB.");
                    return false;
                }

                return true;
            } catch (const std::exception& xcp) {
                err = odbc::IgniteError(
                    odbc::IgniteError::IGNITE_ERR_SECURE_CONNECTION_FAILURE,
                    "Unable to establish connection with DocumentDB.");
                return false;
            }
            
        
        }
    }
}
