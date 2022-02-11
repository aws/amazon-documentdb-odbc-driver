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

#include <ignite/odbc/common/platform_utils.h>
#include <ignite/odbc/ignite_error.h>
#include <ignite/odbc/jni/documentdb_connection.h>

using ignite::odbc::common::GetEnv;
using ignite::odbc::java::IGNITE_JNI_ERR_SUCCESS;
using ignite::odbc::java::IGNITE_JNI_ERR_JVM_INIT;
using ignite::odbc::jni::java::JniErrorInfo;

namespace ignite {
    namespace odbc {
        namespace jni
        {
            bool DocumentDbConnection::Open(const Configuration& config,
                                            JniErrorInfo& errInfo) {
                bool connected = false;

                if (_connection.Get() != nullptr) {
                    return true;
                }

                std::string connectionString = FormatJdbcConnectionString(config);

                auto ctx = GetJniContext();
                if (ctx.Get() == nullptr) {
                    errInfo.errMsg = new char[]{"Unable to get initialized JVM."};
                    errInfo.code = IGNITE_JNI_ERR_JVM_INIT;
                    return false;
                }
     
                SharedPointer< GlobalJObject > result;
                bool success = ctx.Get()->DriverManagerGetConnection(
                    connectionString.c_str(), result, errInfo);
                connected =
                    (success && result.Get() && errInfo.code == IGNITE_JNI_ERR_SUCCESS);
                if (!connected) {
                    JniErrorInfo closeErrInfo;
                    Close(closeErrInfo);
                    return false;
                }
                _connection = result;

                return connected;
            }

            bool DocumentDbConnection::Close(JniErrorInfo& errInfo) {
                if (_connection.Get() != nullptr) {
                    _jniContext.Get()->ConnectionClose(_connection, errInfo);
                    _connection = nullptr;
                    return errInfo.code == IGNITE_JNI_ERR_SUCCESS;
                }
                return true;
            }

            DocumentDbConnection::~DocumentDbConnection() {
                if (_jniContext.Get() != nullptr) {
                    JniErrorInfo errInfo;
                    Close(errInfo);
                    _jniContext = nullptr;
                }
            }

            SharedPointer< DatabaseMetaData > DocumentDbConnection::GetMetaData(
                JniErrorInfo& errInfo){
                auto jniContext = GetJniContext();
                if (!_connection.IsValid()) {
                    return nullptr;
                }
                SharedPointer< GlobalJObject > databaseMetaData;
                bool success = jniContext.Get()->ConnectionGetMetaData(
                    _connection, databaseMetaData, errInfo);
                if (!success) {
                    return nullptr;
                }
                return new DatabaseMetaData(jniContext, databaseMetaData);
            }

            std::string DocumentDbConnection::FormatJdbcConnectionString(
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
                // TODO: Remove use of environment variables and use DSN
                // properties
                std::string sshUserAtHost = GetEnv("DOC_DB_USER", "");
                std::string sshRemoteHost = GetEnv("DOC_DB_HOST", "");
                std::string sshPrivKeyFile = GetEnv("DOC_DB_PRIV_KEY_FILE", "");
                std::string sshUser;
                std::string sshTunnelHost;
                size_t indexOfAt = sshUserAtHost.find_first_of('@');
                if (indexOfAt != std::string::npos && sshUserAtHost.size() > (indexOfAt + 1)) {
                    sshUser = sshUserAtHost.substr(0, indexOfAt);
                    sshTunnelHost = sshUserAtHost.substr(indexOfAt + 1);
                }
                if (!sshUserAtHost.empty() && !sshRemoteHost.empty()
                    && !sshPrivKeyFile.empty() && !sshUser.empty()
                    && !sshTunnelHost.empty()) {
                    jdbConnectionString.append("&sshUser=" + sshUser);
                    jdbConnectionString.append("&sshHost=" + sshTunnelHost);
                    jdbConnectionString.append("&sshPrivateKeyFile="
                                               + sshPrivKeyFile);
                    jdbConnectionString.append(
                        "&sshStrictHostKeyChecking=false");
                }

                return jdbConnectionString;
            }
        }  // namespace jni
    }  // namespace odbc
}  // namespace ignite