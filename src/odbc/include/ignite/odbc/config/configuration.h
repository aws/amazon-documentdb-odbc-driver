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

#ifndef _IGNITE_ODBC_CONFIG_CONFIGURATION
#define _IGNITE_ODBC_CONFIG_CONFIGURATION

#include <stdint.h>
#include <string>
#include <map>

#include "ignite/odbc/protocol_version.h"
#include "ignite/odbc/config/settable_value.h"
#include "ignite/odbc/ssl_mode.h"
#include "ignite/odbc/end_point.h"
#include "ignite/odbc/nested_tx_mode.h"

namespace ignite
{
    namespace odbc
    {
        namespace config
        {
            /**
             * ODBC configuration abstraction.
             */
            class Configuration
            {
            public:
                /** Argument map type. */
                typedef std::map<std::string, std::string> ArgumentMap;

                /** Default values for configuration. */
                struct DefaultValue
                {
                    /** Default value for DSN attribute. */
                    static const std::string dsn;

                    /** Default value for Driver attribute. */
                    static const std::string driver;

                    /** Default value for schema attribute. */
                    static const std::string database;

                    /** Default value for address attribute. */
                    static const std::string address;

                    /** Default value for server attribute. */
                    static const std::string hostname;

                    /** Default value for port attribute. */
                    static const uint16_t port;

                    /** Default value for user attribute. */
                    static const std::string user;

                    /** Default value for password attribute. */
                    static const std::string password;

                    /** Default value for appName attribute. */
                    static const std::string appName;

                    /** Default value for loginTimeoutSec attribute. */
                    static const int32_t loginTimeoutSec;

                    /** Default value for readPreference attribute. */
                    static const std::string readPreference;

                    /** Default value for replicaSet attribute. */
                    static const std::string replicaSet;

                    /** Default value for retryReads attribute. */
                    static const bool retryReads;

                    /** Default value for sslMode attribute. */
                    static const ssl::SslMode::Type sslMode;

                    /** Default value for tls attribute. */
                    static const bool tls;

                    /** Default value for tlsAllowInvalidHostnames attribute. */
                    static const bool tlsAllowInvalidHostnames;

                    /** Default value for tlsCaFile attribute. */
                    static const std::string tlsCaFile;

                    /** Default value for sshUser attribute. */
                    static const std::string sshUser;

                    /** Default value for sshHost attribute. */
                    static const std::string sshHost;

                    /** Default value for sshPrivateKeyFile attribute. */
                    static const std::string sshPrivateKeyFile;

                    /** Default value for sshPrivateKeyPassphrase attribute. */
                    static const std::string sshPrivateKeyPassphrase;

                    /** Default value for sshStrictHostKeyChecking attribute. */
                    static const bool sshStrictHostKeyChecking;

                    /** Default value for sshKnownHostsFile attribute. */
                    static const std::string sshKnownHostsFile;

                    /** Default value for scanMethod attribute. */
                    static const std::string scanMethod;

                    /** Default value for scanLimit attribute. */
                    static const int32_t scanLimit;

                    /** Default value for schemaName attribute. */
                    static const std::string schemaName;

                    /** Default value for refreshSchema attribute. */
                    static const bool refreshSchema;

                    /** Default value for sslKeyFile attribute. */
                    static const std::string sslKeyFile;

                    /** Default value for sslCertFile attribute. */
                    static const std::string sslCertFile;

                    /** Default value for protocol version. */
                   // static const ProtocolVersion& protocolVersion;

                    /** Default value for fetch results page size attribute. */
                    static const int32_t defaultFetchSize;

                    /** Default value for nestedTxMode attribute. */
                    static const NestedTxMode::Type nestedTxMode;
                };

                /**
                 * Default constructor.
                 */
                Configuration();

                /**
                 * Destructor.
                 */
                ~Configuration();

                /**
                 * Convert configure to connect string.
                 *
                 * @return Connect string.
                 */
                std::string ToConnectString() const;

                /**
                 * Get DSN.
                 *
                 * @return Data Source Name.
                 */
                const std::string& GetDsn(const std::string& dflt = DefaultValue::dsn) const;

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsDsnSet() const;

                /**
                 * Set DSN.
                 *
                 * @param dsn Data Source Name.
                 */
                void SetDsn(const std::string& dsn);

                /**
                 * Get Driver.
                 *
                 * @return Driver name.
                 */
                const std::string& GetDriver() const;

                /**
                 * Set driver.
                 *
                 * @param driver Driver.
                 */
                void SetDriver(const std::string& driver);

                /**
                 * Get server host.
                 *
                 * @return Server host.
                 */
                const std::string& GetHostname() const;

                /**
                 * Set server host.
                 *
                 * @param server Server host.
                 */
                void SetHostname(const std::string& host);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsHostnameSet() const;

                /**
                 * Get server port.
                 *
                 * @return Server port.
                 */
                uint16_t GetTcpPort() const;

                /**
                 * Set server port.
                 *
                 * @param port Server port.
                 */
                void SetTcpPort(uint16_t port);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsTcpPortSet() const;

                /**
                 * Get schema.
                 *
                 * @return Schema.
                 */
                const std::string& GetDatabase() const;

                /**
                 * Set schema.
                 *
                 * @param schema Schema name.
                 */
                void SetDatabase(const std::string& schema);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsDatabaseSet() const;

                /**
                 * Get user.
                 *
                 * @return User.
                 */
                const std::string& GetUser() const;

                /**
                 * Set user.
                 *
                 * @param user User.
                 */
                void SetUser(const std::string& user);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsUserSet() const;

                /**
                 * Get password.
                 *
                 * @return Password.
                 */
                const std::string& GetPassword() const;

                /**
                 * Set password.
                 *
                 * @param pass Password.
                 */
                void SetPassword(const std::string& pass);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsPasswordSet() const;

                /**
                 * Get password.
                 *
                 * @return Password.
                 */
                const std::string& GetApplicationName() const;

                /**
                 * Set password.
                 *
                 * @param pass Password.
                 */
                void SetApplicationName(const std::string& name);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsApplicationNameSet() const;

                /**
                 * Get fetch results page size. 
                 *
                 * @return Fetch results page size.
                 */
                int32_t GetLoginTimeoutSeconds() const;

                /**
                 * Set fetch results page size.
                 *
                 * @param size Fetch results page size.
                 */
                void SetLoginTimeoutSeconds(int32_t seconds);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsLoginTimeoutSecondsSet() const;

                /**
                 * Get password.
                 *
                 * @return Password.
                 */
                const std::string& GetReadPreference() const;

                /**
                 * Set password.
                 *
                 * @param pass Password.
                 */
                void SetReadPreference(const std::string& preference);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsReadPreferenceSet() const;
                /**
                 * Get password.
                 *
                 * @return Password.
                 */
                const std::string& GetReplicaSet() const;

                /**
                 * Set password.
                 *
                 * @param pass Password.
                 */
                void SetReplicaSet(const std::string& name);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsReplicaSetSet() const;

                /**
                 * Get password.
                 *
                 * @return Password.
                 */
                bool IsRetryReads() const;

                /**
                 * Set password.
                 *
                 * @param pass Password.
                 */
                void SetRetryReads(bool val);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsRetryReadsSet() const;

                /**
                 * Get password.
                 *
                 * @return Password.
                 */
                bool IsTls() const;

                /**
                 * Set password.
                 *
                 * @param pass Password.
                 */
                void SetTls(bool val);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsTlsSet() const;

                /**
                 * Get password.
                 *
                 * @return Password.
                 */
                bool IsTlsAllowInvalidHostnames() const;

                /**
                 * Set password.
                 *
                 * @param pass Password.
                 */
                void SetTlsAllowInvalidHostnames(bool val);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsTlsAllowInvalidHostnamesSet() const;
                
                /**
                 * Get password.
                 *
                 * @return Password.
                 */
                const std::string& GetTlsCaFile() const;

                /**
                 * Set password.
                 *
                 * @param pass Password.
                 */
                void SetTlsCaFile(const std::string& path);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsTlsCaFileSet() const;

                /**
                 * Get password.
                 *
                 * @return Password.
                 */
                const std::string& GetSshUser() const;

                /**
                 * Set password.
                 *
                 * @param pass Password.
                 */
                void SetSshUser(const std::string& user);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsSshUserSet() const;

                /**
                 * Get password.
                 *
                 * @return Password.
                 */
                const std::string& GetSshHost() const;

                /**
                 * Set password.
                 *
                 * @param pass Password.
                 */
                void SetSshHost(const std::string& host);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsSshHostSet() const;

                /**
                 * Get password.
                 *
                 * @return Password.
                 */
                const std::string& GetSshPrivateKeyFile () const;

                /**
                 * Set password.
                 *
                 * @param pass Password.
                 */
                void SetSshPrivateKeyFile(const std::string& path);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsSshPrivateKeyFileSet() const;

                /**
                 * Get password.
                 *
                 * @return Password.
                 */
                const std::string& GetSshPrivateKeyPassphrase() const;

                /**
                 * Set password.
                 *
                 * @param pass Password.
                 */
                void SetSshPrivateKeyPassphrase(const std::string& passphrase);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsSshPrivateKeyPassphraseSet() const;

                /**
                 * Get password.
                 *
                 * @return Password.
                 */
                bool IsSshStrictHostKeyChecking() const;

                /**
                 * Set password.
                 *
                 * @param pass Password.
                 */
                void SetSshStrictHostKeyChecking(bool val);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsSshStrictHostKeyCheckingSet() const;

                /**
                 * Get password.
                 *
                 * @return Password.
                 */
                const std::string& GetSshKnownHostsFile() const;

                /**
                 * Set password.
                 *
                 * @param pass Password.
                 */
                void SetSshKnownHostsFile(const std::string& path);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsSshKnownHostsFileSet() const;

                /**
                  * Get password.
                  *
                  * @return Password.
                  */
                const std::string& GetScanMethod() const;

                /**
                 * Set password.
                 *
                 * @param pass Password.
                 */
                void SetScanMethod(const std::string& method);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsScanMethodSet() const;

                /**
                 * Get fetch results page size.
                 *
                 * @return Fetch results page size.
                 */
                int32_t GetScanLimit() const;

                /**
                 * Set fetch results page size.
                 *
                 * @param size Fetch results page size.
                 */
                void SetScanLimit(int32_t limit);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsScanLimitSet() const;

                /**
                  * Get password.
                  *
                  * @return Password.
                  */
                const std::string& GetSchemaName() const;

                /**
                 * Set password.
                 *
                 * @param pass Password.
                 */
                void SetSchemaName(const std::string& method);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsSchemaNameSet() const;

                /**
                 * Get password.
                 *
                 * @return Password.
                 */
                bool IsSchemaRefresh() const;

                /**
                 * Set password.
                 *
                 * @param pass Password.
                 */
                void SetSchemaRefresh(bool val);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsSchemaRefreshSet() const;

                /**
                 * Get addresses.
                 *
                 * @return Addresses.
                 */
                const std::vector<EndPoint>& GetAddresses() const;

                /**
                 * Set addresses to connect to.
                 *
                 * @param endPoints Addresses.
                 */
                void SetAddresses(const std::vector<EndPoint>& endPoints);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsAddressesSet() const;

                /**
                 * Get fetch results page size.
                 *
                 * @return Fetch results page size.
                 */
                int32_t GetDefaultFetchSize() const;

                /**
                 * Set fetch results page size.
                 *
                 * @param size Fetch results page size.
                 */
                void SetDefaultFetchSize(int32_t size);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsDefaultFetchSizeSet() const;

                /**
                 * Get argument map.
                 *
                 * @param res Resulting argument map.
                 */
                void ToMap(ArgumentMap& res) const;

            private:
                /**
                 * Add key and value to the argument map.
                 *
                 * @param map Map.
                 * @param key Key.
                 * @param value Value.
                 */
                template<typename T>
                static void AddToMap(ArgumentMap& map, const std::string& key, const SettableValue<T>& value);

                /** DSN. */
                SettableValue<std::string> dsn;

                /** Driver name. */
                SettableValue<std::string> driver;

                /** Schema. */
                SettableValue<std::string> database;

                /** Server. Deprecated. */
                SettableValue<std::string> hostname;

                /** TCP port. Deprecated. */
                SettableValue<uint16_t> port;

                /** User. */
                SettableValue<std::string> user;

                /** Password. */
                SettableValue<std::string> password;

                /** Application name. */
                SettableValue<std::string> appName;

                /** Login timeout in seconds.  */
                SettableValue<int32_t> loginTimeoutSec;

                /** Read pereference. */
                SettableValue<std::string> readPreference;

                /** Replica set name. */
                SettableValue<std::string> replicaSet;

                /** Retry reads flag. */
                SettableValue<bool> retryReads;

                /** Connection end-points. */
                SettableValue< std::vector<EndPoint> > endPoints; // Remove in favor of deprecated one

                /** Enable SSL/TLS. */
                SettableValue<bool> tls;

                /** Flag for if invalid hostnames for the TLS certificate are allowed. */
                SettableValue<bool> tlsAllowInvalidHostnames;

                /** SSL/TLS certificate authority file path. */
                SettableValue<std::string> tlsCaFile;

                /** The SSH host username for the internal SSH tunnel. */
                SettableValue<std::string> sshUser;

                /** The SSH host host name for the internal SSH tunnel. */
                SettableValue<std::string> sshHost;

                /** The SSH host private key file path for the internal SSH tunnel. */
                SettableValue<std::string> sshPrivateKeyFile;

                /** The SSH host private key file passphrase for the internal SSH tunnel. */
                SettableValue<std::string> sshPrivateKeyPassphrase;

                /** Strict host key checking flag for the internal SSH tunnel. */
                SettableValue<bool> sshStrictHostKeyChecking;

                /** The known hosts file path for the internal SSH tunnel. */
                SettableValue<std::string> sshKnownHostsFile;

                /** Scan method. */ 
                SettableValue<std::string> scanMethod;

                /** Scan limit. */
                SettableValue<int32_t> scanLimit;

                /** Schema name. */
                SettableValue<std::string> schemaName;

                /** Refresh schema flag. */
                SettableValue<bool> refreshSchema;

                /** Request and response page size. */
                SettableValue<int32_t> defaultFetchSize;
            };

            template<>
            void Configuration::AddToMap<std::string>(ArgumentMap& map, const std::string& key,
                const SettableValue<std::string>& value);

            template<>
            void Configuration::AddToMap<uint16_t>(ArgumentMap& map, const std::string& key,
                const SettableValue<uint16_t>& value);

            template<>
            void Configuration::AddToMap<int32_t>(ArgumentMap& map, const std::string& key,
                const SettableValue<int32_t>& value);

            template<>
            void Configuration::AddToMap<bool>(ArgumentMap& map, const std::string& key,
                const SettableValue<bool>& value);

            template<>
            void Configuration::AddToMap<ProtocolVersion>(ArgumentMap& map, const std::string& key,
                const SettableValue<ProtocolVersion>& value);

            template<>
            void Configuration::AddToMap< std::vector<EndPoint> >(ArgumentMap& map, const std::string& key,
                const SettableValue< std::vector<EndPoint> >& value);

            template<>
            void Configuration::AddToMap<ssl::SslMode::Type>(ArgumentMap& map, const std::string& key,
                const SettableValue<ssl::SslMode::Type>& value);

            template<>
            void Configuration::AddToMap<NestedTxMode::Type>(ArgumentMap& map, const std::string& key,
                const SettableValue<NestedTxMode::Type>& value);
        }
    }
}

#endif //_IGNITE_ODBC_CONFIG_CONFIGURATION
