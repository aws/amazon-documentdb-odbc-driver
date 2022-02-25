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

#ifndef _IGNITE_ODBC_CONFIG_CONNECTION_STRING_PARSER
#define _IGNITE_ODBC_CONFIG_CONNECTION_STRING_PARSER

#include <string>

#include "ignite/odbc/config/configuration.h"
#include "ignite/odbc/diagnostic/diagnostic_record_storage.h"

namespace ignite {
namespace odbc {
namespace config {
/**
 * ODBC configuration parser abstraction.
 */
class ConnectionStringParser {
 public:
  /** Connection attribute keywords. */
  struct Key {
    /** Connection attribute keyword for DSN attribute. */
    static const std::string dsn;

    /** Connection attribute keyword for Driver attribute. */
    static const std::string driver;

    /** Connection attribute keyword for database attribute. */
    static const std::string database;

    /** Connection attribute keyword for hostname attribute. */
    static const std::string hostname;

    /** Connection attribute keyword for port attribute. */
    static const std::string port;

    /** Connection attribute keyword for username attribute. */
    static const std::string user;

    /** Connection attribute keyword for password attribute. */
    static const std::string password;

    /** Connection attribute keyword for appName attribute. */
    static const std::string appName;

    /** Connection attribute keyword for loginTimeoutSec attribute. */
    static const std::string loginTimeoutSec;

    /** Connection attribute keyword for readPreference attribute. */
    static const std::string readPreference;

    /** Connection attribute keyword for replicaSet attribute. */
    static const std::string replicaSet;

    /** Connection attribute keyword for retryReads attribute. */
    static const std::string retryReads;

    /** Connection attribute keyword for tls attribute. */
    static const std::string tls;

    /** Connection attribute keyword for tlsAllowInvalidHostnames attribute. */
    static const std::string tlsAllowInvalidHostnames;

    /** Connection attribute keyword for tlsCaFile attribute. */
    static const std::string tlsCaFile;

    /** Connection attribute keyword for sshEnable attribute. */
    static const std::string sshEnable;

    /** Connection attribute keyword for sshUser attribute. */
    static const std::string sshUser;

    /** Connection attribute keyword for sshHost attribute. */
    static const std::string sshHost;

    /** Connection attribute keyword for sshPrivateKeyFile attribute. */
    static const std::string sshPrivateKeyFile;

    /** Connection attribute keyword for sshPrivateKeyPassphrase attribute. */
    static const std::string sshPrivateKeyPassphrase;

    /** Connection attribute keyword for sshStrictHostKeyChecking attribute. */
    static const std::string sshStrictHostKeyChecking;

    /** Connection attribute keyword for sshKnownHostsFile attribute. */
    static const std::string sshKnownHostsFile;

    /** Connection attribute keyword for scanMethod attribute. */
    static const std::string scanMethod;

    /** Connection attribute keyword for scanLimit attribute. */
    static const std::string scanLimit;

    /** Connection attribute keyword for schemaName attribute. */
    static const std::string schemaName;

    /** Connection attribute keyword for refreshSchema attribute. */
    static const std::string refreshSchema;

    /** Connection attribute keyword for defaultFetchSize attribute. */
    static const std::string defaultFetchSize;

    /** Connection attribute keyword for sslMode attribute. */
    static const std::string sslMode;

    /** Connection attribute keyword for sslKeyFile attribute. */
    static const std::string sslKeyFile;

    /** Connection attribute keyword for sslCertFile attribute. */
    static const std::string sslCertFile;

    /** Connection attribute keyword for sslCaFile attribute. */
    static const std::string sslCaFile;

    /** Connection attribute keyword for username attribute. */
    static const std::string uid;

    /** Connection attribute keyword for password attribute. */
    static const std::string pwd;
  };

  /**
   * Constructor.
   *
   * @param cfg Configuration.
   */
  ConnectionStringParser(Configuration& cfg);

  /**
   * Destructor.
   */
  ~ConnectionStringParser() = default;

  /**
   * Parse connect string.
   *
   * @param str String to parse.
   * @param len String length.
   * @param delimiter delimiter.
   * @param diag Diagnostics collector.
   */
  void ParseConnectionString(const char* str, size_t len, char delimiter,
                             diagnostic::DiagnosticRecordStorage* diag);

  /**
   * Parse connect string.
   *
   * @param str String to parse.
   * @param diag Diagnostics collector.
   */
  void ParseConnectionString(const std::string& str,
                             diagnostic::DiagnosticRecordStorage* diag);

  /**
   * Parse config attributes.
   *
   * @param str String to parse.
   * @param diag Diagnostics collector.
   */
  void ParseConfigAttributes(const char* str,
                             diagnostic::DiagnosticRecordStorage* diag);

 private:
  /**
   * Result of parsing string value to bool.
   */
  struct BoolParseResult {
    enum class Type { AI_FALSE, AI_TRUE, AI_UNRECOGNIZED };
  };

  /**
   * Handle new attribute pair callback.
   *
   * @param key Key.
   * @param value Value.
   * @param diag Diagnostics collector.
   */
  void HandleAttributePair(const std::string& key, const std::string& value,
                           diagnostic::DiagnosticRecordStorage* diag);

  /**
   * Convert string to boolean value.
   *
   * @param value Value to convert to bool.
   * @return Result.
   */
  static BoolParseResult::Type StringToBool(const std::string& value);

  /**
   * Convert string to boolean value.
   *
   * @param msg Error message.
   * @param key Key.
   * @param value Value.
   * @return Resulting error message.
   */
  static std::string MakeErrorMessage(const std::string& msg,
                                      const std::string& key,
                                      const std::string& value);

  /** Configuration. */
  Configuration& cfg;
};
}  // namespace config

}  // namespace odbc
}  // namespace ignite

#endif  //_IGNITE_ODBC_CONFIG_CONNECTION_STRING_PARSER
