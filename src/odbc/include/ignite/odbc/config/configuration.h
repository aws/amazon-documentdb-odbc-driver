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

#include "ignite/odbc/config/settable_value.h"
#include "ignite/odbc/diagnostic/diagnosable.h"
#include "ignite/odbc/read_preference.h"
#include "ignite/odbc/scan_method.h"

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

                    /** Default value for database attribute. */
                    static const std::string database;

                    /** Default value for hostname attribute. */
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
                    static const ReadPreference::Type readPreference;

                    /** Default value for replicaSet attribute. */
                    static const std::string replicaSet;

                    /** Default value for retryReads attribute. */
                    static const bool retryReads;

                    /** Default value for tls attribute. */
                    static const bool tls;

                    /** Default value for tlsAllowInvalidHostnames attribute. */
                    static const bool tlsAllowInvalidHostnames;

                    /** Default value for tlsCaFile attribute. */
                    static const std::string tlsCaFile;

                    /** Default value for sshEnable attribute. */
                    static const bool sshEnable;

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
                    static const ScanMethod::Type scanMethod;

                    /** Default value for scanLimit attribute. */
                    static const int32_t scanLimit;

                    /** Default value for schemaName attribute. */
                    static const std::string schemaName;

                    /** Default value for refreshSchema attribute. */
                    static const bool refreshSchema;

                    /** Default value for defaultFetchSize attribute. */
                    static const int32_t defaultFetchSize;
                };

                /**
                 * Default constructor.
                 */
                Configuration() = default;

                /**
                 * Destructor.
                 */
                ~Configuration() = default;

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
                void SetDsn(const std::string& dsnName);

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
                void SetDriver(const std::string& driverName);

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
                uint16_t GetPort() const;

                /**
                 * Set server port.
                 *
                 * @param portNumber Server port.
                 */
                void SetPort(uint16_t portNumber);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsPortSet() const;

                /**
                 * Get database.
                 *
                 * @return Database.
                 */
                const std::string& GetDatabase() const;

                /**
                 * Set schema.
                 *
                 * @param database Database name.
                 */
                void SetDatabase(const std::string& databaseName);

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
                void SetUser(const std::string& username);

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
                 * Get application name.
                 *
                 * @return Application name.
                 */
                const std::string& GetApplicationName() const;

                /**
                 * Set application name.
                 *
                 * @param name Application name.
                 */
                void SetApplicationName(const std::string& name);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsApplicationNameSet() const;

                /**
                 * Get login timeout in seconds. 
                 *
                 * @return Login timeout in seconds.
                 */
                int32_t GetLoginTimeoutSeconds() const;

                /**
                 * Set login timeout in seconds.
                 *
                 * @param seconds Login timeout in seconds.
                 */
                void SetLoginTimeoutSeconds(int32_t seconds);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsLoginTimeoutSecondsSet() const;

                /**
                 * Get read preference.
                 *
                 * @return Read preference.
                 */
                ReadPreference::Type GetReadPreference() const;

                /**
                 * Set read preference.
                 *
                 * @param preference Read preference.
                 */
                void SetReadPreference(const ReadPreference::Type preference);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsReadPreferenceSet() const;
                
                /**
                 * Get replica set name.
                 *
                 * @return Replica set name.
                 */
                const std::string& GetReplicaSet() const;

                /**
                 * Set replica set name.
                 *
                 * @param name Replica set name.
                 */
                void SetReplicaSet(const std::string& name);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsReplicaSetSet() const;

                /**
                 * Get retry reads flag.
                 *
                 * @return @true if retry reads enabled.
                 */
                bool IsRetryReads() const;

                /**
                 * Set retry reads.
                 *
                 * @param val Value to set.
                 */
                void SetRetryReads(bool val);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsRetryReadsSet() const;

                /**
                 * Get TLS flag.
                 *
                 * @return @true if TLS is enabled.
                 */
                bool IsTls() const;

                /**
                 * Set TLS flag.
                 *
                 * @param val Value to set.
                 */
                void SetTls(bool val);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsTlsSet() const;

                /**
                 * Get TLS allow invalid hostnames flag.
                 *
                 * @return @true if invalid hostnames are allowed with TLS.
                 */
                bool IsTlsAllowInvalidHostnames() const;

                /**
                 * Set TLS allow invaid hostnames flag.
                 *
                 * @param val Value to set.
                 */
                void SetTlsAllowInvalidHostnames(bool val);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsTlsAllowInvalidHostnamesSet() const;
                
                /**
                 * Get path to TLS CA file.
                 *
                 * @return path to TLS CA file.
                 */
                const std::string& GetTlsCaFile() const;

                /**
                 * Set path to TLS CA file.
                 *
                 * @param path Path to TLS CA file.
                 */
                void SetTlsCaFile(const std::string& path);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsTlsCaFileSet() const;

                /**
                 * Get SSH enable flag.
                 *
                 * @return @true if SSH is enabled.
                 */
                bool IsSshEnable() const;

                /**
                 * Set ssh enable.
                 *
                 * @param bool ssh enable.
                 */
                void SetSshEnable(bool val);

                /**
                 * Check if the ssh enable value set.
                 *
                 * @return @true if the ssh enable value set.
                 */
                bool IsSshEnableSet() const;

                /**
                 * Get password.
                 * Get username for SSH host.
                 *
                 * @return SSH username.
                 */
                const std::string& GetSshUser() const;

                /**
                 * Set username for SSH host.
                 *
                 * @param username SSH username.
                 */
                void SetSshUser(const std::string& username);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsSshUserSet() const;

                /**
                 * Get hostname for SSH host.
                 *
                 * @return SSH hostname.
                 */
                const std::string& GetSshHost() const;

                /**
                 * Set SSH hostname.
                 *
                 * @param host SSH hostname.
                 */
                void SetSshHost(const std::string& host);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsSshHostSet() const;

                /**
                 * Get path to private key file for SSH host.
                 *
                 * @return Path to private key file for SSH host.
                 */
                const std::string& GetSshPrivateKeyFile () const;

                /**
                 * Set path to private key file.
                 *
                 * @param path Path to private key file for SSH host.
                 */
                void SetSshPrivateKeyFile(const std::string& path);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsSshPrivateKeyFileSet() const;

                /**
                 * Get SSH private key passphrase.
                 *
                 * @return SSH private key passphrase.
                 */
                const std::string& GetSshPrivateKeyPassphrase() const;

                /**
                 * Set SSH private key file passphrase.
                 *
                 * @param passphrase SSH private key file passphrase.
                 */
                void SetSshPrivateKeyPassphrase(const std::string& passphrase);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsSshPrivateKeyPassphraseSet() const;

                /**
                 * Get SSH strict host key checking flag.
                 *
                 * @return @true if strict host key checking is enabled.
                 */
                bool IsSshStrictHostKeyChecking() const;

                /**
                 * Set strict host key checking flag.
                 *
                 * @param val Value to set.
                 */
                void SetSshStrictHostKeyChecking(bool val);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsSshStrictHostKeyCheckingSet() const;

                /**
                 * Get path to SSH known hosts file.
                 *
                 * @return Path to SSH known hosts file.
                 */
                const std::string& GetSshKnownHostsFile() const;

                /**
                 * Set path to SSH known hosts file.
                 *
                 * @param path Path to SSH known hosts file.
                 */
                void SetSshKnownHostsFile(const std::string& path);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsSshKnownHostsFileSet() const;

                /**
                  * Get scan method.
                  *
                  * @return Scan method.
                  */
                ScanMethod::Type GetScanMethod() const;

                /**
                 * Set scan method.
                 *
                 * @param method Scan method.
                 */
                void SetScanMethod(const ScanMethod::Type method);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsScanMethodSet() const;

                /**
                 * Get scan limit in # of documents.
                 *
                 * @return Scan limit.
                 */
                int32_t GetScanLimit() const;

                /**
                 * Set scan limit in # of documents.
                 *
                 * @param limit Scan limit in # of documents.
                 */
                void SetScanLimit(int32_t limit);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsScanLimitSet() const;

                /**
                  * Get schema name to save.
                  *
                  * @return Schema name.
                  */
                const std::string& GetSchemaName() const;

                /**
                 * Set schema name to save.
                 *
                 * @param name Schema name.
                 */
                void SetSchemaName(const std::string& name);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsSchemaNameSet() const;

                /**
                 * Get refresh schema flag.
                 *
                 * @return @true if refreshing schema is enabled.
                 */
                bool IsRefreshSchema() const;

                /**
                 * Set refresh schema flag.
                 *
                 * @return @true if the value set.
                 */
                void SetRefreshSchema(bool val);

                /**
                 * Check if the value set.
                 *
                 * @return @true if the value set.
                 */
                bool IsRefreshSchemaSet() const;

                /**
                 * Get default fetch size.
                 *
                 * @return Default fetch size.
                 */
                int32_t GetDefaultFetchSize() const;

                /**
                 * Set default fetch size.
                 *
                 * @param size Default fetch size.
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
                SettableValue<std::string> dsn = DefaultValue::dsn;

                /** Driver name. */
                SettableValue<std::string> driver = DefaultValue::driver;

                /** Schema. */
                SettableValue<std::string> database = DefaultValue::database;

                /** Hostname. */
                SettableValue<std::string> hostname = DefaultValue::hostname;

                /** Port. */
                SettableValue<uint16_t> port = DefaultValue::port;

                /** User. */
                SettableValue<std::string> user = DefaultValue::user;

                /** Password. */
                SettableValue<std::string> password = DefaultValue::password;

                /** Application name. */
                SettableValue<std::string> appName = DefaultValue::appName;

                /** Login timeout in seconds.  */
                SettableValue<int32_t> loginTimeoutSec = DefaultValue::loginTimeoutSec;

                /** Read pereference. */
                SettableValue<ReadPreference::Type> readPreference = DefaultValue::readPreference;

                /** Replica set name. */
                SettableValue<std::string> replicaSet = DefaultValue::replicaSet;

                /** Retry reads flag. */
                SettableValue<bool> retryReads = DefaultValue::retryReads;

                /** Enable SSL/TLS. */
                SettableValue<bool> tls = DefaultValue::tls;

                /** Flag for if invalid hostnames for the TLS certificate are allowed. */
                SettableValue<bool> tlsAllowInvalidHostnames = DefaultValue::tlsAllowInvalidHostnames;

                /** SSL/TLS certificate authority file path. */
                SettableValue<std::string> tlsCaFile = DefaultValue::tlsCaFile;

                /** The SSH enable option for the internal SSH tunnel. */
                SettableValue<bool> sshEnable = DefaultValue::sshEnable;

                /** The SSH host username for the internal SSH tunnel. */
                SettableValue<std::string> sshUser = DefaultValue::sshUser;

                /** The SSH host host name for the internal SSH tunnel. */
                SettableValue<std::string> sshHost = DefaultValue::sshHost;

                /** The SSH host private key file path for the internal SSH tunnel. */
                SettableValue<std::string> sshPrivateKeyFile = DefaultValue::sshPrivateKeyFile;

                /** The SSH host private key file passphrase for the internal SSH tunnel. */
                SettableValue<std::string> sshPrivateKeyPassphrase = DefaultValue::sshPrivateKeyPassphrase;

                /** Strict host key checking flag for the internal SSH tunnel. */
                SettableValue<bool> sshStrictHostKeyChecking = DefaultValue::sshStrictHostKeyChecking;

                /** The known hosts file path for the internal SSH tunnel. */
                SettableValue<std::string> sshKnownHostsFile = DefaultValue::sshKnownHostsFile;

                /** Scan method. */ 
                SettableValue<ScanMethod::Type> scanMethod = DefaultValue::scanMethod;

                /** Scan limit. */
                SettableValue<int32_t> scanLimit = DefaultValue::scanLimit;

                /** Schema name. */
                SettableValue<std::string> schemaName = DefaultValue::schemaName;

                /** Refresh schema flag. */
                SettableValue<bool> refreshSchema = DefaultValue::refreshSchema;

                /** Default fetch size. */
                SettableValue<int32_t> defaultFetchSize = DefaultValue::defaultFetchSize;
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
            void Configuration::AddToMap<ReadPreference::Type>(ArgumentMap& map, const std::string& key,
                const SettableValue<ReadPreference::Type>& value);

            template<>
            void Configuration::AddToMap<ScanMethod::Type>(ArgumentMap& map, const std::string& key,
                const SettableValue<ScanMethod::Type>& value);
        }
    }
}

#endif //_IGNITE_ODBC_CONFIG_CONFIGURATION
