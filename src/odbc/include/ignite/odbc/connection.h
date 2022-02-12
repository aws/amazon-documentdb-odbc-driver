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

#ifndef _IGNITE_ODBC_CONNECTION
#define _IGNITE_ODBC_CONNECTION

#include <stdint.h>

#include <vector>

#include "ignite/odbc/parser.h"
#include "ignite/odbc/config/connection_info.h"
#include "ignite/odbc/config/configuration.h"
#include "ignite/odbc/diagnostic/diagnosable_adapter.h"
#include "ignite/odbc/streaming/streaming_context.h"
#include "ignite/odbc/odbc_error.h"
#include "ignite/odbc/jni/database_metadata.h"
#include "ignite/odbc/jni/documentdb_connection.h"
#include "ignite/odbc/jni/java.h"
#include <ignite/odbc/common/concurrent.h>
#include "ignite/odbc/end_point.h"

using ignite::odbc::common::concurrent::SharedPointer;
using ignite::odbc::jni::java::GlobalJObject;
using ignite::odbc::jni::java::JniContext;
using ignite::odbc::jni::DocumentDbConnection;
using ignite::odbc::jni::DatabaseMetaData;

namespace ignite
{
    namespace odbc
    {
        class Environment;
        class Statement;

        /**
         * ODBC node connection.
         */
        class Connection : public diagnostic::DiagnosableAdapter
        {
            friend class Environment;
        public:
            /**
             * Operation with timeout result.
             */
            struct OperationResult
            {
                enum T
                {
                    SUCCESS,
                    FAIL,
                    TIMEOUT
                };
            };

            /** Default connection timeout in seconds. */
            enum
            {
                DEFAULT_CONNECT_TIMEOUT = 5
            };

            /**
             * Destructor.
             */
            ~Connection();

            /**
             * Get connection info.
             *
             * @return Connection info.
             */
            const config::ConnectionInfo& GetInfo() const;

            /**
             * Get info of any type.
             *
             * @param type Info type.
             * @param buf Result buffer pointer.
             * @param buflen Result buffer length.
             * @param reslen Result value length pointer.
             */
            void GetInfo(config::ConnectionInfo::InfoType type, void* buf, short buflen, short* reslen);

            /**
             * Establish connection to ODBC server.
             *
             * @param connectStr Connection string.
             * @param parentWindow Parent window pointer.
             */
            void Establish(const std::string& connectStr, void* parentWindow);

            /**
             * Establish connection to ODBC server.
             *
             * @param cfg Configuration.
             */
            void Establish(const config::Configuration cfg);

            /**
             * Release established connection.
             *
             * @return Operation result.
             */
            void Release();

            /**
             * Deregister self from the parent.
             */
            void Deregister();

            /**
             * Create statement associated with the connection.
             *
             * @return Pointer to valid instance on success and NULL on failure.
             */
            Statement* CreateStatement();

            /** 
             * Gets the database metadata for the connection.
             * 
             * @return SharedPointer to DatabaseMetaData.
             */
            SharedPointer< DatabaseMetaData > GetMetaData(IgniteError& err);

            /**
             * Get name of the assotiated schema.
             *
             * @return Schema name.
             */
            const std::string& GetSchema() const;

            /**
             * Get configuration.
             *
             * @return Connection configuration.
             */
            const config::Configuration& GetConfiguration() const;

            /**
             * Is auto commit.
             *
             * @return @c true if the auto commit is enabled.
             */
            bool IsAutoCommit() const;

            /**
             * Get streaming context.
             *
             * @return Streaming context.
             */
            streaming::StreamingContext& GetStreamingContext()
            {
                return streamingContext;
            }

            /**
             * Create diagnostic record associated with the Connection instance.
             *
             * @param sqlState SQL state.
             * @param message Message.
             * @param rowNum Associated row number.
             * @param columnNum Associated column number.
             * @return DiagnosticRecord associated with the instance.
             */
            static diagnostic::DiagnosticRecord CreateStatusRecord(SqlState::Type sqlState,
                const std::string& message, int32_t rowNum = 0, int32_t columnNum = 0);

            /**
             * Synchronously send request message and receive response.
             * Uses provided timeout.
             *
             * @param req Request message.
             * @param rsp Response message.
             * @param timeout Timeout.
             * @return @c true on success, @c false on timeout.
             * @throw OdbcError on error.
             */
            template<typename ReqT, typename RspT>
            bool SyncMessage(const ReqT& req, RspT& rsp, int32_t timeout)
            {
                // TODO: Remove when unnecessary.
                return true;
            }

            /**
             * Synchronously send request message and receive response.
             * Uses connection timeout.
             *
             * @param req Request message.
             * @param rsp Response message.
             * @throw OdbcError on error.
             */
            template<typename ReqT, typename RspT>
            void SyncMessage(const ReqT& req, RspT& rsp)
            {
                // TODO: Remove when unnecessary.
            }

            /**
             * Send request message.
             * Uses connection timeout.
             *
             * @param req Request message.
             * @throw OdbcError on error.
             */
            template<typename ReqT>
            void SendRequest(const ReqT& req)
            {
                // TODO: Remove when unnecessary.
            }

            /**
             * Perform transaction commit.
             */
            void TransactionCommit();

            /**
             * Perform transaction rollback.
             */
            void TransactionRollback();

            /**
             * Get connection attribute.
             *
             * @param attr Attribute type.
             * @param buf Buffer for value.
             * @param bufLen Buffer length.
             * @param valueLen Resulting value length.
             */
            void GetAttribute(int attr, void* buf, SQLINTEGER bufLen, SQLINTEGER *valueLen);

            /**
             * Set connection attribute.
             *
             * @param attr Attribute type.
             * @param value Value pointer.
             * @param valueLen Value length.
             */
            void SetAttribute(int attr, void* value, SQLINTEGER valueLen);

           private:
            IGNITE_NO_COPY_ASSIGNMENT(Connection);

            /**
             * Init connection socket, using configuration.
             *
             * @return Operation result.
             */
            SqlResult::Type InitSocket();

            /**
             * Synchronously send request message and receive response.
             * Uses provided timeout. Does not try to restore connection on
             * fail.
             *
             * @param req Request message.
             * @param rsp Response message.
             * @param timeout Timeout.
             * @return @c true on success, @c false on timeout.
             * @throw OdbcError on error.
             */
            template<typename ReqT, typename RspT>
            bool InternalSyncMessage(const ReqT& req, RspT& rsp, int32_t timeout)
            {
                // TODO: Remove when unnecessary.
                return true;
            }

            /**
             * Establish connection to ODBC server.
             * Internal call.
             *
             * @param connectStr Connection string.
             * @param parentWindow Parent window.
             * @return Operation result.
             */
            SqlResult::Type InternalEstablish(const std::string& connectStr, void* parentWindow);

            /**
             * Establish connection to ODBC server.
             * Internal call.
             *
             * @param cfg Configuration.
             * @return Operation result.
             */
            SqlResult::Type InternalEstablish(const config::Configuration& cfg);

            /**
             * Release established connection.
             * Internal call.
             *
             * @return Operation result.
             */
            SqlResult::Type InternalRelease();

            /**
             * Close connection.
             */
            void Close();

            /**
             * Get info of any type.
             * Internal call.
             *
             * @param type Info type.
             * @param buf Result buffer pointer.
             * @param buflen Result buffer length.
             * @param reslen Result value length pointer.
             * @return Operation result.
             */
            SqlResult::Type InternalGetInfo(config::ConnectionInfo::InfoType type, void* buf, short buflen, short* reslen);

            /**
             * Create statement associated with the connection.
             * Internal call.
             *
             * @param statement Pointer to valid instance on success and NULL on failure.
             * @return Operation result.
             */
            SqlResult::Type InternalCreateStatement(Statement*& statement);

            /**
             * Perform transaction commit on all the associated connections.
             * Internal call.
             *
             * @return Operation result.
             */
            SqlResult::Type InternalTransactionCommit();

            /**
             * Perform transaction rollback on all the associated connections.
             * Internal call.
             *
             * @return Operation result.
             */
            SqlResult::Type InternalTransactionRollback();

            /**
             * Get connection attribute.
             * Internal call.
             *
             * @param attr Attribute type.
             * @param buf Buffer for value.
             * @param bufLen Buffer length.
             * @param valueLen Resulting value length.
             * @return Operation result.
             */
            SqlResult::Type InternalGetAttribute(int attr, void* buf, SQLINTEGER bufLen, SQLINTEGER* valueLen);

            /**
             * Set connection attribute.
             * Internal call.
             *
             * @param attr Attribute type.
             * @param value Value pointer.
             * @param valueLen Value length.
             * @return Operation result.
             */
            SqlResult::Type InternalSetAttribute(int attr, void* value, SQLINTEGER valueLen);

            /**
             * Perform handshake request.
             *
             * @return Operation result.
             */
            SqlResult::Type MakeRequestHandshake();

            /**
             * Ensure there is a connection to the cluster.
             *
             * @throw OdbcError on failure.
             */
            void EnsureConnected();

            /**
             * Try to restore connection to the cluster.
             *
             * @throw IgniteError on failure.
             * @return @c true on success and @c false otherwise.
             */
            bool TryRestoreConnection(IgniteError& err);

            /** 
             * Creates JVM options
             */
            void SetJvmOptions(const std::string& cp);

            /**
             * Get the singleton instance of the JNI context for the connection.
             *
             * @return JNI context.
             */
            SharedPointer< JniContext > GetJniContext();

            /**
             * De-initializes the JVM options
             */
            void Deinit();

            /**
             * Retrieve timeout from parameter.
             *
             * @param value Parameter.
             * @return Timeout.
             */
            int32_t RetrieveTimeout(void* value);

            /**
             * Constructor.
             */
            Connection(Environment* env);

            /** Parent. */
            Environment* env;

            /** Connection timeout in seconds. */
            int32_t timeout = 0;

            /** Login timeout in seconds. */
            int32_t loginTimeout = DEFAULT_CONNECT_TIMEOUT;

            /** Autocommit flag. */
            bool autoCommit = true;

            /** Configuration. */
            config::Configuration config;

            /** Connection info. */
            config::ConnectionInfo info;

            /** Java connection object */
            SharedPointer< DocumentDbConnection > _connection;

            SharedPointer< JniContext > _jniContext;

            /** JVM options */
            std::vector< char* > opts;

            /** Streaming context. */
            streaming::StreamingContext streamingContext;
        };
    }
}

#endif //_IGNITE_ODBC_CONNECTION
