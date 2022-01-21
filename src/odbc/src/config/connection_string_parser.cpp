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

#include <vector>

#include "ignite/common/utils.h"

#include "ignite/odbc/utility.h"
//#include "ignite/odbc/ssl_mode.h"
#include "ignite/odbc/config/connection_string_parser.h"
#include "ignite/odbc/config/config_tools.h"
//#include "ignite/odbc/nested_tx_mode.h"

namespace ignite
{
    namespace odbc
    {
        namespace config
        {
            const std::string ConnectionStringParser::Key::dsn                      = "dsn";
            const std::string ConnectionStringParser::Key::driver                   = "driver";
            const std::string ConnectionStringParser::Key::database                 = "database";
            const std::string ConnectionStringParser::Key::hostname                 = "hostname";
            const std::string ConnectionStringParser::Key::port                     = "port";
            const std::string ConnectionStringParser::Key::user                     = "user";
            const std::string ConnectionStringParser::Key::password                 = "password";
            const std::string ConnectionStringParser::Key::appName                  = "app_name";
            const std::string ConnectionStringParser::Key::loginTimeoutSec          = "login_timeout_sec";
            const std::string ConnectionStringParser::Key::readPreference           = "read_preference";
            const std::string ConnectionStringParser::Key::replicaSet               = "replica_set";
            const std::string ConnectionStringParser::Key::retryReads               = "retry_reads";
            const std::string ConnectionStringParser::Key::tls                      = "tls";
            const std::string ConnectionStringParser::Key::tlsAllowInvalidHostnames = "tls_allow_invalid_hostnames";
            const std::string ConnectionStringParser::Key::tlsCaFile                = "tls_ca_file";
            const std::string ConnectionStringParser::Key::sshUser                  = "ssh_user";
            const std::string ConnectionStringParser::Key::sshHost                  = "ssh_host";
            const std::string ConnectionStringParser::Key::sshPrivateKeyFile        = "ssh_private_key_file";
            const std::string ConnectionStringParser::Key::sshPrivateKeyPassphrase  = "ssh_private_key_passphrase";
            const std::string ConnectionStringParser::Key::sshStrictHostKeyChecking = "ssh_strict_host_key_checking";
            const std::string ConnectionStringParser::Key::sshKnownHostsFile        = "ssh_known_hosts_file";
            const std::string ConnectionStringParser::Key::scanMethod               = "scan_method";
            const std::string ConnectionStringParser::Key::scanLimit                = "scan_limit";
            const std::string ConnectionStringParser::Key::schemaName               = "schema_name";
            const std::string ConnectionStringParser::Key::refreshSchema            = "refresh_schema";
            const std::string ConnectionStringParser::Key::defaultFetchSize         = "default_fetch_size";
            const std::string ConnectionStringParser::Key::uid                      = "uid";
            const std::string ConnectionStringParser::Key::pwd                      = "pwd";

            ConnectionStringParser::ConnectionStringParser(Configuration& cfg):
                cfg(cfg)
            {
                // No-op.
            }

            ConnectionStringParser::~ConnectionStringParser()
            {
                // No-op.
            }

            void ConnectionStringParser::ParseConnectionString(const char* str, size_t len, char delimiter,
                diagnostic::DiagnosticRecordStorage* diag)
            {
                std::string connect_str(str, len);

                while (connect_str.rbegin() != connect_str.rend() && *connect_str.rbegin() == 0)
                    connect_str.erase(connect_str.size() - 1);

                while (!connect_str.empty())
                {
                    size_t attr_begin = connect_str.rfind(delimiter);

                    if (attr_begin == std::string::npos)
                        attr_begin = 0;
                    else
                        ++attr_begin;

                    size_t attr_eq_pos = connect_str.rfind('=');

                    if (attr_eq_pos == std::string::npos)
                        attr_eq_pos = 0;

                    if (attr_begin < attr_eq_pos)
                    {
                        const char* key_begin = connect_str.data() + attr_begin;
                        const char* key_end = connect_str.data() + attr_eq_pos;

                        const char* value_begin = connect_str.data() + attr_eq_pos + 1;
                        const char* value_end = connect_str.data() + connect_str.size();

                        std::string key = common::StripSurroundingWhitespaces(key_begin, key_end);
                        std::string value = common::StripSurroundingWhitespaces(value_begin, value_end);

                        if (value[0] == '{' && value[value.size() - 1] == '}')
                            value = value.substr(1, value.size() - 2);

                        HandleAttributePair(key, value, diag);
                    }

                    if (!attr_begin)
                        break;

                    connect_str.erase(attr_begin - 1);
                }
            }

            void ConnectionStringParser::ParseConnectionString(const std::string& str,
                diagnostic::DiagnosticRecordStorage* diag)
            {
                ParseConnectionString(str.data(), str.size(), ';', diag);
            }

            void ConnectionStringParser::ParseConfigAttributes(const char* str,
                diagnostic::DiagnosticRecordStorage* diag)
            {
                size_t len = 0;

                // Getting list length. List is terminated by two '\0'.
                while (str[len] || str[len + 1])
                    ++len;

                ++len;

                ParseConnectionString(str, len, '\0', diag);
            }

            void ConnectionStringParser::HandleAttributePair(const std::string &key, const std::string &value,
                diagnostic::DiagnosticRecordStorage* diag)
            {
                std::string lKey = common::ToLower(key);

                if (lKey == Key::dsn)
                {
                    cfg.SetDsn(value);
                }
                else if (lKey == Key::database)
                {
                    cfg.SetDatabase(value);
                }
                else if (lKey == Key::hostname)
                {
                    EndPoint endpoint;

                    ParseSingleAddress(value, endpoint, diag);

                    cfg.SetHostname(endpoint.host);
                    
                    if (!cfg.IsTcpPortSet()) 
                        cfg.SetTcpPort(endpoint.port);
                }
                else if (lKey == Key::port)
                {
                    if (value.empty())
                    {
                        if (diag)
                        {
                            diag->AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED,
                                MakeErrorMessage("Port attribute value is empty. Using default value.", key, value));
                        }

                        return;
                    }

                    if (!common::AllDigits(value))
                    {
                        if (diag)
                        {
                            diag->AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED,
                                MakeErrorMessage("Port attribute value contains unexpected characters."
                                    " Using default value.", key, value));
                        }

                        return;
                    }

                    if (value.size() >= sizeof("65535"))
                    {
                        if (diag)
                        {
                            diag->AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED,
                                MakeErrorMessage("Port attribute value is too large. Using default value.", key, value));
                        }

                        return;
                    }

                    int32_t numValue = 0;
                    std::stringstream conv;

                    conv << value;
                    conv >> numValue;

                    if (numValue <= 0 || numValue > 0xFFFF)
                    {
                        if (diag)
                        {
                            diag->AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED,
                                MakeErrorMessage("Port attribute value is out of range. "
                                    "Using default value.", key, value));
                        }

                        return;
                    }

                    cfg.SetTcpPort(static_cast<uint16_t>(numValue));
                }
                else if (lKey == Key::appName)
                {
                    cfg.SetApplicationName(value);
                }
                else if (lKey == Key::loginTimeoutSec)
                {
                    if (!common::AllDigits(value))
                    {
                        if (diag)
                        {
                            diag->AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED,
                                MakeErrorMessage("Login timeout seconds attribute value contains unexpected characters."
                                    " Using default value.", key, value));
                        }

                        return;
                    }

                    int64_t numValue = 0;
                    std::stringstream conv;

                    conv << value;
                    conv >> numValue;

                    if (numValue <= 0 || numValue > 0xFFFFFFFFL)
                    {
                        if (diag)
                        {
                            diag->AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED,
                                MakeErrorMessage("Login timeout seconds attribute value is out of range."
                                    " Using default value.", key, value));
                        }

                        return;
                    }

                    cfg.SetLoginTimeoutSeconds(static_cast<int32_t>(numValue));
                }
                else if (lKey == Key::readPreference)
                {
                    ReadPreference::Type preference = ReadPreference::FromString(value);

                    if (preference == ReadPreference::Type::UNKNOWN)
                    {
                        if (diag)
                        {
                            diag->AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED,
                                "Specified read preference is not supported. Default value used ('primary').");
                        }

                        return;
                    }

                    cfg.SetReadPreference(preference);
                }
                else if (lKey == Key::replicaSet)
                {
                    cfg.SetReplicaSet(value);
                }
                else if (lKey == Key::retryReads)
                {
                    BoolParseResult::Type res = StringToBool(value);

                    if (res == BoolParseResult::AI_UNRECOGNIZED)
                    {
                        if (diag)
                        {
                            diag->AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED,
                                MakeErrorMessage("Unrecognized bool value. Using default value.", key, value));
                        }

                        return;
                    }

                    cfg.SetRetryReads(res == BoolParseResult::AI_TRUE);
                }
                else if (lKey == Key::tls)
                {
                    BoolParseResult::Type res = StringToBool(value);

                    if (res == BoolParseResult::AI_UNRECOGNIZED)
                    {
                        if (diag)
                        {
                            diag->AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED,
                                MakeErrorMessage("Unrecognized bool value. Using default value.", key, value));
                        }

                        return;
                    }

                    cfg.SetTls(res == BoolParseResult::AI_TRUE);
                }
                else if (lKey == Key::tlsAllowInvalidHostnames)
                {
                    BoolParseResult::Type res = StringToBool(value);

                    if (res == BoolParseResult::AI_UNRECOGNIZED)
                    {
                        if (diag)
                        {
                            diag->AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED,
                                MakeErrorMessage("Unrecognized bool value. Using default value.", key, value));
                        }

                        return;
                    }

                    cfg.SetTlsAllowInvalidHostnames(res == BoolParseResult::AI_TRUE);
                }
                else if (lKey == Key::tlsCaFile)
                {
                    cfg.SetTlsCaFile(value);
                }
                else if (lKey == Key::sshUser)
                {
                    cfg.SetSshUser(value);
                }
                else if (lKey == Key::sshHost)
                {
                    cfg.SetSshHost(value);
                }
                else if (lKey == Key::sshPrivateKeyFile)
                {
                    cfg.SetSshPrivateKeyFile(value);
                }
                else if (lKey == Key::sshPrivateKeyPassphrase)
                {
                    cfg.SetSshPrivateKeyPassphrase(value);
                }
                else if (lKey == Key::sshStrictHostKeyChecking)
                {
                    BoolParseResult::Type res = StringToBool(value);

                    if (res == BoolParseResult::AI_UNRECOGNIZED)
                    {
                        if (diag)
                        {
                            diag->AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED,
                                MakeErrorMessage("Unrecognized bool value. Using default value.", key, value));
                        }

                        return;
                    }

                    cfg.SetSshStrictHostKeyChecking(res == BoolParseResult::AI_TRUE);
                }
                else if (lKey == Key::sshKnownHostsFile)
                {
                    cfg.SetSshKnownHostsFile(value);
                }
                else if (lKey == Key::scanMethod)
                {
                    ScanMethod::Type method = ScanMethod::FromString(value);
                    
                    if (method == ScanMethod::Type::UNKNOWN)
                    {
                        if (diag)
                        {
                            diag->AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED,
                                "Specified scan method is not supported. Default value used ('random').");
                        }

                        return;
                    }

                    cfg.SetScanMethod(method);
                }
                else if (lKey == Key::scanLimit)
                {
                    if (!common::AllDigits(value))
                    {
                        if (diag)
                        {
                            diag->AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED,
                                MakeErrorMessage("Scan limit attribute value contains unexpected characters."
                                    " Using default value.", key, value));
                        }

                        return;
                    }

                    int64_t numValue = 0;
                    std::stringstream conv;

                    conv << value;
                    conv >> numValue;

                    if (numValue <= 0 || numValue > 0xFFFFFFFFL)
                    {
                        if (diag)
                        {
                            diag->AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED,
                                MakeErrorMessage("Scan limit attribute value is out of range."
                                    " Using default value.", key, value));
                        }

                        return;
                    }

                    cfg.SetScanLimit(static_cast<int32_t>(numValue));
                }
                else if (lKey == Key::schemaName)
                {
                    cfg.SetSchemaName(value);
                }
                else if (lKey == Key::refreshSchema)
                {
                    BoolParseResult::Type res = StringToBool(value);

                    if (res == BoolParseResult::AI_UNRECOGNIZED)
                    {
                        if (diag)
                        {
                            diag->AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED,
                                MakeErrorMessage("Unrecognized bool value. Using default value.", key, value));
                        }

                        return;
                    }

                    cfg.SetRefreshSchema(res == BoolParseResult::AI_TRUE);
                }
                else if (lKey == Key::defaultFetchSize)
                {
                    if (!common::AllDigits(value))
                    {
                        if (diag)
                        {
                            diag->AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED,
                                MakeErrorMessage("Default fetch size attribute value contains unexpected characters."
                                    " Using default value.", key, value));
                        }

                        return;
                    }

                    if (value.size() >= sizeof("4294967295"))
                    {
                        if (diag)
                        {
                            diag->AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED,
                                MakeErrorMessage("Page size attribute value is too large."
                                    " Using default value.", key, value));
                        }

                        return;
                    }

                    int64_t numValue = 0;
                    std::stringstream conv;

                    conv << value;
                    conv >> numValue;

                    if (numValue <= 0 || numValue > 0xFFFFFFFFL)
                    {
                        if (diag)
                        {
                            diag->AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED,
                                MakeErrorMessage("Page size attribute value is out of range."
                                    " Using default value.", key, value));
                        }

                        return;
                    }

                    cfg.SetDefaultFetchSize(static_cast<int32_t>(numValue));
                } 
                else if (lKey == Key::tlsCaFile)
                {
                    cfg.SetTlsCaFile(value);
                }
                else if (lKey == Key::driver)
                {
                    cfg.SetDriver(value);
                }
                else if (lKey == Key::user || lKey == Key::uid)
                {
                    if (!cfg.GetUser().empty() && diag)
                    {
                        diag->AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED,
                            "Re-writing USER (have you specified it several times?");
                    }

                    cfg.SetUser(value);
                }
                else if (lKey == Key::password || lKey == Key::pwd)
                {
                    if (!cfg.GetPassword().empty() && diag)
                    {
                        diag->AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED,
                            "Re-writing PASSWORD (have you specified it several times?");
                    }

                    cfg.SetPassword(value);
                }
                else if (diag)
                {
                    std::stringstream stream;

                    stream << "Unknown attribute: '" << key << "'. Ignoring.";

                    diag->AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED, stream.str());
                }
            }

            ConnectionStringParser::BoolParseResult::Type ConnectionStringParser::StringToBool(const std::string& value)
            {
                std::string lower = common::ToLower(value);

                if (lower == "true")
                    return BoolParseResult::AI_TRUE;

                if (lower == "false")
                    return BoolParseResult::AI_FALSE;

                return BoolParseResult::AI_UNRECOGNIZED;
            }

            std::string ConnectionStringParser::MakeErrorMessage(const std::string& msg, const std::string& key,
                const std::string& value)
            {
                std::stringstream stream;

                stream << msg << " [key='" << key << "', value='" << value << "']";

                return stream.str();
            }
        }
    }
}

