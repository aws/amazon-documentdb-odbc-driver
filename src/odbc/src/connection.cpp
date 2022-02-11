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
#include "ignite/odbc/jni/documentdb_connection.h"
#include "ignite/odbc/jni/database_metadata.h"

using namespace ignite::odbc::jni::java;
using namespace ignite::odbc::common;
using namespace ignite::odbc::common::concurrent;
using ignite::odbc::IgniteError;
using ignite::odbc::jni::DocumentDbConnection;
using ignite::odbc::jni::DatabaseMetaData;

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
            info(config)
        {
            // No-op
        }

        Connection::~Connection()
        {
            Close();
            _jniContext = nullptr;
            Deinit();
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

            if (_connection.IsValid()) {
                AddStatusRecord(SqlState::S08002_ALREADY_CONNECTED,
                                "Already connected.");

                return SqlResult::AI_ERROR;
            }

            if (!config.IsHostnameSet())
            {
                AddStatusRecord("No valid address to connect.");

                return SqlResult::AI_ERROR;
            }

            IgniteError err;
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
            if (!_connection.IsValid())
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
            if (_jniContext.IsValid()) {
                if (_connection.IsValid()) {
                    JniErrorInfo errInfo;
                    _connection.Get()->Close(errInfo);
                    if (errInfo.code != java::IGNITE_JNI_ERR_SUCCESS) {
                        // TODO: Determine if we need to error check the close.
                    }
                    _connection = nullptr;
                }
            }
        }

        Statement* Connection::CreateStatement()
        {
            Statement* statement;

            IGNITE_ODBC_API_CALL(InternalCreateStatement(statement));

            return statement;
        }

        SharedPointer< DatabaseMetaData > Connection::GetMetaData(
            IgniteError& err) {
            if (!_connection.IsValid()) {
                err = IgniteError(IgniteError::IGNITE_ERR_ILLEGAL_STATE,
                                  "Must be connected.");
                return nullptr;
            }
            JniErrorInfo errInfo;
            auto databaseMetaData = _connection.Get()->GetMetaData(errInfo);
            if (!databaseMetaData.IsValid()) {
                std::string message = errInfo.errMsg ? errInfo.errMsg : "";
                err = IgniteError(IgniteError::IGNITE_ERR_JNI_GET_DATABASE_METADATA,
                                  message.c_str());
                return nullptr;
            }
            return databaseMetaData;
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
            catch (const IgniteError& err)
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
            catch (const IgniteError& err)
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

                    *val = _connection.Get() ? SQL_CD_FALSE : SQL_CD_TRUE;

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
            catch (const IgniteError& err)
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
            if (_connection.Get() != nullptr)
                return;

            IgniteError err;
            bool success = TryRestoreConnection(err);

            if (!success) {
                std::string errMessage =
                    "Failed to establish connection with any provided hosts\n";
                errMessage.append(err.GetText());
                AddStatusRecord(SqlState::S08001_CANNOT_CONNECT, errMessage);
                throw OdbcError(SqlState::S08001_CANNOT_CONNECT, errMessage);
            }
        }

        bool Connection::TryRestoreConnection(IgniteError& err)
        {
            if (_connection.Get() != nullptr) {
                return true;
            }

            std::string connectionString = FormatJdbcConnectionString(config);
            JniErrorInfo errInfo;
            auto ctx = GetJniContext();
            SharedPointer< DocumentDbConnection > conn = new DocumentDbConnection(ctx);
            if (!conn.IsValid() || !conn.Get()->Open(config, errInfo)) {
                std::string message = errInfo.errMsg ? errInfo.errMsg : "Unable to open connection.";
                err = IgniteError(
                    IgniteError::IGNITE_ERR_SECURE_CONNECTION_FAILURE,
                    message.c_str());
            }
            _connection = conn;
            return _connection.IsValid() && _connection.Get()->IsOpen()
                   && errInfo.code == java::IGNITE_JNI_ERR_SUCCESS;
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
            if (indexOfAt != std::string::npos && sshUserAtHost.size() > (indexOfAt + 1)) {
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

        SharedPointer< JniContext > Connection::GetJniContext() {
            if (_jniContext.Get() == nullptr) {
                // Resolve DOCUMENTDB_HOME.
                std::string home = jni::ResolveDocumentDbHome();

                // Create classpath.
                std::string cp =
                    jni::CreateDocumentDbClasspath(std::string(), home);
                if (cp.empty()) {
                    return nullptr;
                }

                // Set the JVM options.
                SetJvmOptions(cp);

                // Create the context
                SharedPointer< JniContext > ctx(JniContext::Create(
                    &opts[0], static_cast< int >(opts.size()), JniHandlers()));
                _jniContext = ctx;
            }
            return _jniContext;
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
    }
}

