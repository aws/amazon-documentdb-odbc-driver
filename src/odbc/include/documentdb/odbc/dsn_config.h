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

#ifndef _DOCUMENTDB_ODBC_DSN_CONFIG
#define _DOCUMENTDB_ODBC_DSN_CONFIG

#include "documentdb/odbc/config/configuration.h"
#include "documentdb/odbc/documentdb_error.h"
#include "sqltypes.h"

using namespace documentdb::odbc::config;
using namespace documentdb::odbc;

namespace documentdb {
namespace odbc {
/**
 * Extract last setup error and throw it like DocumentDbError.
 */
void ThrowLastSetupError();

/**
 * Add new string to the DSN file.
 *
 * @param dsn DSN name.
 * @param key Key.
 * @param value Value.
 */
bool WriteDsnString(const char* dsn, const char* key, const char* value, DocumentDbError& error);

/**
 * Get string from the DSN file.
 *
 * @param dsn DSN name.
 * @param key Key.
 * @param dflt Default value.
 * @return Value.
 */
std::string ReadDsnString(const char* dsn, const char* key, const char* dflt);

/**
 * Read DSN to fill the configuration.
 *
 * @param dsn DSN name.
 * @param config Configuration.
 * @param diag Diagnostic collector.
 */
void ReadDsnConfiguration(const char* dsn, Configuration& config,
                          diagnostic::DiagnosticRecordStorage* diag);

/**
 * Write DSN from the configuration.
 *
 * @param config Configuration.
 * @param diag Diagnostic collector.
 */
bool WriteDsnConfiguration(const Configuration& config, DocumentDbError& error);

/**
 * Deletes a DSN from the system.
 *
 * @param dsn The DSN name to remove.
 * @param diag Diagnostic collector.
 */
bool DeleteDsnConfiguration(const std::string dsn, DocumentDbError& error);

/**
 * Register DSN with specified configuration.
 *
 * @param config Configuration.
 * @param driver Driver.
 * @return True on success and false on fail.
 */
bool RegisterDsn(const Configuration& config, const LPCSTR driver, DocumentDbError& error);

/**
 * Unregister specified DSN.
 *
 * @param dsn DSN name.
 * @return True on success and false on fail.
 */
bool UnregisterDsn(const std::string& dsn, DocumentDbError& error);
}  // namespace odbc
}  // namespace documentdb

#endif  //_DOCUMENTDB_ODBC_DSN_CONFIG
