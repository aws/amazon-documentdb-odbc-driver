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

#include "documentdb/odbc/environment.h"

#include <cstdlib>

#include "documentdb/odbc/connection.h"
#include "documentdb/odbc/system/odbc_constants.h"

namespace documentdb {
namespace odbc {
Environment::Environment()
    : connections(), odbcVersion(SQL_OV_ODBC3), odbcNts(SQL_TRUE) {
  srand(common::GetRandSeed());
}

Environment::~Environment() {
  // No-op.
}

Connection* Environment::CreateConnection() {
  Connection* connection;

  DOCUMENTDB_ODBC_API_CALL(InternalCreateConnection(connection));

  return connection;
}

void Environment::DeregisterConnection(Connection* conn) {
  connections.erase(conn);
}

SqlResult::Type Environment::InternalCreateConnection(Connection*& connection) {
  connection = new Connection(this);

  if (!connection) {
    AddStatusRecord(SqlState::SHY001_MEMORY_ALLOCATION, "Not enough memory.");

    return SqlResult::AI_ERROR;
  }

  connections.insert(connection);

  return SqlResult::AI_SUCCESS;
}

void Environment::SetAttribute(int32_t attr, void* value, int32_t len) {
  DOCUMENTDB_ODBC_API_CALL(InternalSetAttribute(attr, value, len));
}

SqlResult::Type Environment::InternalSetAttribute(int32_t attr, void* value,
                                                  int32_t len) {
  DOCUMENTDB_UNUSED(len);

  EnvironmentAttribute::Type attribute = EnvironmentAttributeToInternal(attr);

  switch (attribute) {
    case EnvironmentAttribute::ODBC_VERSION: {
      int32_t version =
          static_cast< int32_t >(reinterpret_cast< intptr_t >(value));

      if (version != odbcVersion) {
        AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED,
                        "ODBC version is not supported.");

        return SqlResult::AI_SUCCESS_WITH_INFO;
      }

      return SqlResult::AI_SUCCESS;
    }

    case EnvironmentAttribute::OUTPUT_NTS: {
      int32_t nts = static_cast< int32_t >(reinterpret_cast< intptr_t >(value));

      if (nts != odbcNts) {
        AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED,
                        "Only null-termination of strings is supported.");

        return SqlResult::AI_SUCCESS_WITH_INFO;
      }

      return SqlResult::AI_SUCCESS;
    }

    case EnvironmentAttribute::UNKNOWN:
    default:
      break;
  }

  AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                  "Attribute is not supported.");

  return SqlResult::AI_ERROR;
}

void Environment::GetAttribute(int32_t attr,
                               app::ApplicationDataBuffer& buffer) {
  DOCUMENTDB_ODBC_API_CALL(InternalGetAttribute(attr, buffer));
}

SqlResult::Type Environment::InternalGetAttribute(
    int32_t attr, app::ApplicationDataBuffer& buffer) {
  EnvironmentAttribute::Type attribute = EnvironmentAttributeToInternal(attr);

  switch (attribute) {
    case EnvironmentAttribute::ODBC_VERSION: {
      buffer.PutInt32(odbcVersion);

      return SqlResult::AI_SUCCESS;
    }

    case EnvironmentAttribute::OUTPUT_NTS: {
      buffer.PutInt32(odbcNts);

      return SqlResult::AI_SUCCESS;
    }

    case EnvironmentAttribute::UNKNOWN:
    default:
      break;
  }

  AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                  "Attribute is not supported.");

  return SqlResult::AI_ERROR;
}
}  // namespace odbc
}  // namespace documentdb
