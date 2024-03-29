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

#include "documentdb/odbc/statement.h"

#include <boost/optional.hpp>
#include <limits>

#include "documentdb/odbc/connection.h"
#include "documentdb/odbc/log.h"
#include "documentdb/odbc/message.h"
#include "documentdb/odbc/odbc_error.h"
#include "documentdb/odbc/query/batch_query.h"
#include "documentdb/odbc/query/column_metadata_query.h"
#include "documentdb/odbc/query/data_query.h"
#include "documentdb/odbc/query/foreign_keys_query.h"
#include "documentdb/odbc/query/internal_query.h"
#include "documentdb/odbc/query/primary_keys_query.h"
#include "documentdb/odbc/query/special_columns_query.h"
#include "documentdb/odbc/query/table_metadata_query.h"
#include "documentdb/odbc/query/type_info_query.h"
#include "documentdb/odbc/sql/sql_parser.h"
#include "documentdb/odbc/sql/sql_set_streaming_command.h"
#include "documentdb/odbc/sql/sql_utils.h"
#include "documentdb/odbc/system/odbc_constants.h"
#include "documentdb/odbc/utility.h"

namespace documentdb {
namespace odbc {
Statement::Statement(Connection& parent)
    : connection(parent),
      columnBindings(),
      currentQuery(),
      rowsFetched(0),
      rowStatuses(0),
      columnBindOffset(0),
      rowArraySize(1),
      parameters(),
      timeout(0) {
  // No-op.
}

Statement::~Statement() {
  // No-op.
}

void Statement::BindColumn(uint16_t columnIdx, int16_t targetType,
                           void* targetValue, SqlLen bufferLength,
                           SqlLen* strLengthOrIndicator) {
  DOCUMENTDB_ODBC_API_CALL(InternalBindColumn(columnIdx, targetType, targetValue,
                                          bufferLength, strLengthOrIndicator));
}

SqlResult::Type Statement::InternalBindColumn(uint16_t columnIdx,
                                              int16_t targetType,
                                              void* targetValue,
                                              SqlLen bufferLength,
                                              SqlLen* strLengthOrIndicator) {
  using namespace type_traits;
  OdbcNativeType::Type driverType = ToDriverType(targetType);

  if (driverType == OdbcNativeType::AI_UNSUPPORTED) {
    AddStatusRecord(SqlState::SHY003_INVALID_APPLICATION_BUFFER_TYPE,
                    "The argument TargetType was not a valid data type.");

    return SqlResult::AI_ERROR;
  }

  if (bufferLength < 0) {
    AddStatusRecord(
        SqlState::SHY090_INVALID_STRING_OR_BUFFER_LENGTH,
        "The value specified for the argument BufferLength was less than 0.");

    return SqlResult::AI_ERROR;
  }

  if (targetValue || strLengthOrIndicator) {
    app::ApplicationDataBuffer dataBuffer(driverType, targetValue, bufferLength,
                                          strLengthOrIndicator);

    SafeBindColumn(columnIdx, dataBuffer);
  } else
    SafeUnbindColumn(columnIdx);

  return SqlResult::AI_SUCCESS;
}

void Statement::SafeBindColumn(uint16_t columnIdx,
                               const app::ApplicationDataBuffer& buffer) {
  columnBindings[columnIdx] = buffer;
}

void Statement::SafeUnbindColumn(uint16_t columnIdx) {
  columnBindings.erase(columnIdx);
}

void Statement::SafeUnbindAllColumns() {
  columnBindings.clear();
}

void Statement::SetColumnBindOffsetPtr(int* ptr) {
  columnBindOffset = ptr;
}

int* Statement::GetColumnBindOffsetPtr() {
  return columnBindOffset;
}

int32_t Statement::GetColumnNumber() {
  int32_t res;

  DOCUMENTDB_ODBC_API_CALL(InternalGetColumnNumber(res));

  return res;
}

SqlResult::Type Statement::InternalGetColumnNumber(int32_t& res) {
  const meta::ColumnMetaVector* meta = GetMeta();

  if (!meta)
    return SqlResult::AI_ERROR;

  res = static_cast< int32_t >(meta->size());

  return SqlResult::AI_SUCCESS;
}

void Statement::BindParameter(uint16_t paramIdx, int16_t ioType,
                              int16_t bufferType, int16_t paramSqlType,
                              SqlUlen columnSize, int16_t decDigits,
                              void* buffer, SqlLen bufferLen, SqlLen* resLen) {
  DOCUMENTDB_ODBC_API_CALL(
      InternalBindParameter(paramIdx, ioType, bufferType, paramSqlType,
                            columnSize, decDigits, buffer, bufferLen, resLen));
}

SqlResult::Type Statement::InternalBindParameter(
    uint16_t paramIdx, int16_t ioType, int16_t bufferType, int16_t paramSqlType,
    SqlUlen columnSize, int16_t decDigits, void* buffer, SqlLen bufferLen,
    SqlLen* resLen) {
  using namespace type_traits;
  using app::ApplicationDataBuffer;
  using app::Parameter;

  if (paramIdx == 0) {
    std::stringstream builder;
    builder << "The value specified for the argument ParameterNumber was less "
               "than 1. [ParameterNumber="
            << paramIdx << ']';

    AddStatusRecord(SqlState::S24000_INVALID_CURSOR_STATE, builder.str());

    return SqlResult::AI_ERROR;
  }

  if (ioType != SQL_PARAM_INPUT) {
    std::stringstream builder;
    builder << "The value specified for the argument InputOutputType was not "
               "SQL_PARAM_INPUT. [ioType="
            << ioType << ']';

    AddStatusRecord(SqlState::SHY105_INVALID_PARAMETER_TYPE, builder.str());

    return SqlResult::AI_ERROR;
  }

  if (!IsSqlTypeSupported(paramSqlType)) {
    std::stringstream builder;
    builder << "Data type is not supported. [typeId=" << paramSqlType << ']';

    AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                    builder.str());

    return SqlResult::AI_ERROR;
  }

  OdbcNativeType::Type driverType = ToDriverType(bufferType);

  if (driverType == OdbcNativeType::AI_UNSUPPORTED) {
    std::stringstream builder;
    builder << "The argument TargetType was not a valid data type. [TargetType="
            << bufferType << ']';

    AddStatusRecord(SqlState::SHY003_INVALID_APPLICATION_BUFFER_TYPE,
                    builder.str());

    return SqlResult::AI_ERROR;
  }

  if (!buffer && !resLen) {
    AddStatusRecord(
        SqlState::SHY009_INVALID_USE_OF_NULL_POINTER,
        "ParameterValuePtr and StrLen_or_IndPtr are both null pointers");

    return SqlResult::AI_ERROR;
  }

  ApplicationDataBuffer dataBuffer(driverType, buffer, bufferLen, resLen);

  Parameter param(dataBuffer, paramSqlType, columnSize, decDigits);

  parameters.BindParameter(paramIdx, param);

  return SqlResult::AI_SUCCESS;
}

void Statement::SetAttribute(int attr, void* value, SQLINTEGER valueLen) {
  DOCUMENTDB_ODBC_API_CALL(InternalSetAttribute(attr, value, valueLen));
}

SqlResult::Type Statement::InternalSetAttribute(int attr, void* value,
                                                SQLINTEGER) {
  switch (attr) {
    case SQL_ATTR_ROW_ARRAY_SIZE: {
      SqlUlen val = reinterpret_cast< SqlUlen >(value);
      LOG_DEBUG_MSG("SQL_ATTR_ROW_ARRAY_SIZE: " << val);

      if (val < 1) {
        AddStatusRecord(SqlState::SHY024_INVALID_ATTRIBUTE_VALUE,
                        "Array size value must be a positive integer value.");
        return SqlResult::AI_ERROR;
      }

      // Update row array size setting.
      rowArraySize = val;
      LOG_DEBUG_MSG("rowArraySize: " << rowArraySize);

      break;
    }

    case SQL_ATTR_ROW_BIND_TYPE: {
      SqlUlen rowBindType = reinterpret_cast< SqlUlen >(value);

      if (rowBindType != SQL_BIND_BY_COLUMN) {
        AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                        "Only binding by column is currently supported");

        return SqlResult::AI_ERROR;
      }

      break;
    }

    case SQL_ATTR_ROWS_FETCHED_PTR: {
      SetRowsFetchedPtr(reinterpret_cast< SQLINTEGER* >(value));

      break;
    }

    case SQL_ATTR_ROW_STATUS_PTR: {
      SetRowStatusesPtr(reinterpret_cast< SQLUSMALLINT* >(value));

      break;
    }

    case SQL_ATTR_PARAM_BIND_TYPE: {
      SqlUlen paramBindType = reinterpret_cast< SqlUlen >(value);

      if (paramBindType != SQL_PARAM_BIND_BY_COLUMN) {
        AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                        "Only binding by column is currently supported");

        return SqlResult::AI_ERROR;
      }

      break;
    }

    case SQL_ATTR_PARAM_BIND_OFFSET_PTR: {
      SetParamBindOffsetPtr(reinterpret_cast< int* >(value));

      break;
    }

    case SQL_ATTR_ROW_BIND_OFFSET_PTR: {
      SetColumnBindOffsetPtr(reinterpret_cast< int* >(value));

      break;
    }

    case SQL_ATTR_PARAMSET_SIZE: {
      SqlUlen size = reinterpret_cast< SqlUlen >(value);
      parameters.SetParamSetSize(size);

      break;
    }

    case SQL_ATTR_PARAMS_PROCESSED_PTR: {
      parameters.SetParamsProcessedPtr(reinterpret_cast< SqlUlen* >(value));

      break;
    }

    case SQL_ATTR_PARAM_STATUS_PTR: {
      parameters.SetParamsStatusPtr(reinterpret_cast< SQLUSMALLINT* >(value));
      break;
    }

    case SQL_ATTR_QUERY_TIMEOUT: {
      SqlUlen uTimeout = reinterpret_cast< SqlUlen >(value);

      if (uTimeout > INT32_MAX) {
        timeout = INT32_MAX;

        std::stringstream ss;

        ss << "Value is too big: " << uTimeout << ", changing to " << timeout
           << ".";
        std::string msg = ss.str();

        AddStatusRecord(SqlState::S01S02_OPTION_VALUE_CHANGED, msg);

        return SqlResult::AI_SUCCESS_WITH_INFO;
      }

      timeout = static_cast< int32_t >(uTimeout);

      break;
    }

    default: {
      AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                      "Specified attribute is not supported.");

      return SqlResult::AI_ERROR;
    }
  }

  return SqlResult::AI_SUCCESS;
}

void Statement::GetAttribute(int attr, void* buf, SQLINTEGER bufLen,
                             SQLINTEGER* valueLen) {
  DOCUMENTDB_ODBC_API_CALL(InternalGetAttribute(attr, buf, bufLen, valueLen));
}

SqlResult::Type Statement::InternalGetAttribute(int attr, void* buf, SQLINTEGER,
                                                SQLINTEGER* valueLen) {
  if (!buf) {
    AddStatusRecord("Data buffer is NULL.");

    return SqlResult::AI_ERROR;
  }

  switch (attr) {
    case SQL_ATTR_APP_ROW_DESC:
    case SQL_ATTR_APP_PARAM_DESC:
    case SQL_ATTR_IMP_ROW_DESC:
    case SQL_ATTR_IMP_PARAM_DESC: {
      SQLPOINTER* val = reinterpret_cast< SQLPOINTER* >(buf);

      *val = static_cast< SQLPOINTER >(this);

      if (valueLen)
        *valueLen = SQL_IS_POINTER;

      break;
    }

    case SQL_ATTR_ROW_BIND_TYPE: {
      SqlUlen* val = reinterpret_cast< SqlUlen* >(buf);

      *val = SQL_BIND_BY_COLUMN;

      break;
    }

    case SQL_ATTR_ROW_ARRAY_SIZE: {
      SQLINTEGER* val = reinterpret_cast< SQLINTEGER* >(buf);

      *val = static_cast< SQLINTEGER >(rowArraySize);

      if (valueLen)
        *valueLen = SQL_IS_INTEGER;

      break;
    }

    case SQL_ATTR_ROWS_FETCHED_PTR: {
      SqlUlen** val = reinterpret_cast< SqlUlen** >(buf);

      *val = reinterpret_cast< SqlUlen* >(GetRowsFetchedPtr());

      if (valueLen)
        *valueLen = SQL_IS_POINTER;

      break;
    }

    case SQL_ATTR_ROW_STATUS_PTR: {
      SQLUSMALLINT** val = reinterpret_cast< SQLUSMALLINT** >(buf);

      *val = reinterpret_cast< SQLUSMALLINT* >(GetRowStatusesPtr());

      if (valueLen)
        *valueLen = SQL_IS_POINTER;

      break;
    }

    case SQL_ATTR_PARAM_BIND_TYPE: {
      SqlUlen* val = reinterpret_cast< SqlUlen* >(buf);

      *val = SQL_PARAM_BIND_BY_COLUMN;

      break;
    }

    case SQL_ATTR_PARAM_BIND_OFFSET_PTR: {
      SQLULEN** val = reinterpret_cast< SQLULEN** >(buf);

      *val = reinterpret_cast< SQLULEN* >(parameters.GetParamBindOffsetPtr());

      if (valueLen)
        *valueLen = SQL_IS_POINTER;

      break;
    }

    case SQL_ATTR_ROW_BIND_OFFSET_PTR: {
      SqlUlen** val = reinterpret_cast< SqlUlen** >(buf);

      *val = reinterpret_cast< SqlUlen* >(GetColumnBindOffsetPtr());

      if (valueLen)
        *valueLen = SQL_IS_POINTER;

      break;
    }

    case SQL_ATTR_PARAMSET_SIZE: {
      SqlUlen* val = reinterpret_cast< SqlUlen* >(buf);

      *val = static_cast< SqlUlen >(parameters.GetParamSetSize());

      if (valueLen)
        *valueLen = SQL_IS_UINTEGER;

      break;
    }

    case SQL_ATTR_PARAMS_PROCESSED_PTR: {
      SqlUlen** val = reinterpret_cast< SqlUlen** >(buf);

      *val = parameters.GetParamsProcessedPtr();

      if (valueLen)
        *valueLen = SQL_IS_POINTER;

      break;
    }

    case SQL_ATTR_PARAM_STATUS_PTR: {
      SQLUSMALLINT** val = reinterpret_cast< SQLUSMALLINT** >(buf);

      *val = parameters.GetParamsStatusPtr();

      if (valueLen)
        *valueLen = SQL_IS_POINTER;

      break;
    }

    case SQL_ATTR_QUERY_TIMEOUT: {
      SqlUlen* uTimeout = reinterpret_cast< SqlUlen* >(buf);

      *uTimeout = static_cast< SqlUlen >(timeout);

      break;
    }

    default: {
      AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                      "Specified attribute is not supported.");

      return SqlResult::AI_ERROR;
    }
  }

  return SqlResult::AI_SUCCESS;
}

void Statement::GetParametersNumber(uint16_t& paramNum) {
  DOCUMENTDB_ODBC_API_CALL(InternalGetParametersNumber(paramNum));
}

SqlResult::Type Statement::InternalGetParametersNumber(uint16_t& paramNum) {
  if (!currentQuery.get()) {
    AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR, "Query is not prepared.");

    return SqlResult::AI_ERROR;
  }

  if (currentQuery->GetType() != query::QueryType::DATA) {
    paramNum = 0;

    return SqlResult::AI_SUCCESS;
  }

  if (!parameters.IsMetadataSet()) {
    SqlResult::Type res = UpdateParamsMeta();

    if (res != SqlResult::AI_SUCCESS)
      return res;
  }

  paramNum = parameters.GetExpectedParamNum();

  return SqlResult::AI_SUCCESS;
}

void Statement::SetParamBindOffsetPtr(int* ptr) {
  DOCUMENTDB_ODBC_API_CALL_ALWAYS_SUCCESS;

  parameters.SetParamBindOffsetPtr(ptr);
}

void Statement::GetColumnData(uint16_t columnIdx,
                              app::ApplicationDataBuffer& buffer) {
  DOCUMENTDB_ODBC_API_CALL(InternalGetColumnData(columnIdx, buffer));
}

SqlResult::Type Statement::InternalGetColumnData(
    uint16_t columnIdx, app::ApplicationDataBuffer& buffer) {
  if (!currentQuery.get()) {
    AddStatusRecord(SqlState::S24000_INVALID_CURSOR_STATE,
                    "Cursor is not in the open state.");

    return SqlResult::AI_ERROR;
  }

  SqlResult::Type res = currentQuery->GetColumn(columnIdx, buffer);

  return res;
}

void Statement::PrepareSqlQuery(const std::string& query) {
  DOCUMENTDB_ODBC_API_CALL(InternalPrepareSqlQuery(query));
}

SqlResult::Type Statement::ProcessInternalCommand(const std::string& query) {
  try {
    SqlParser parser(query);

    std::shared_ptr< SqlCommand > cmd = parser.GetNextCommand();

    assert(cmd.get() != 0);

    parameters.Prepare();

    currentQuery.reset(new query::InternalQuery(*this, query, cmd));

    return SqlResult::AI_SUCCESS;
  } catch (const OdbcError& err) {
    AddStatusRecord(err);

    return SqlResult::AI_ERROR;
  }
}

SqlResult::Type Statement::InternalPrepareSqlQuery(const std::string& query) {
  if (sql_utils::IsInternalCommand(query))
    return ProcessInternalCommand(query);

  // Resetting parameters types as we are changing the query.
  parameters.Prepare();

  if (currentQuery.get())
    currentQuery->Close();

  currentQuery.reset(
      new query::DataQuery(*this, connection, query, parameters, timeout));

  return SqlResult::AI_SUCCESS;
}

void Statement::ExecuteSqlQuery(const std::string& query) {
  DOCUMENTDB_ODBC_API_CALL(InternalExecuteSqlQuery(query));
}

SqlResult::Type Statement::InternalExecuteSqlQuery(const std::string& query) {
  SqlResult::Type result = InternalPrepareSqlQuery(query);

  if (result != SqlResult::AI_SUCCESS)
    return result;

  return InternalExecuteSqlQuery();
}

void Statement::ExecuteSqlQuery() {
  DOCUMENTDB_ODBC_API_CALL(InternalExecuteSqlQuery());
}

SqlResult::Type Statement::InternalExecuteSqlQuery() {
  if (!currentQuery.get()) {
    AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR, "Query is not prepared.");

    return SqlResult::AI_ERROR;
  }

  if (parameters.GetParamSetSize() > 1
      && currentQuery->GetType() == query::QueryType::DATA) {
    query::DataQuery& qry = static_cast< query::DataQuery& >(*currentQuery);

    currentQuery.reset(new query::BatchQuery(*this, connection, qry.GetSql(),
                                             parameters, timeout));
  } else if (parameters.GetParamSetSize() == 1
             && currentQuery->GetType() == query::QueryType::BATCH) {
    query::BatchQuery& qry = static_cast< query::BatchQuery& >(*currentQuery);

    currentQuery.reset(new query::DataQuery(*this, connection, qry.GetSql(),
                                            parameters, timeout));
  }

  if (parameters.GetParamSetSize() > 1
      && currentQuery->GetType() == query::QueryType::STREAMING) {
    AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                    "Batching is not supported in streaming mode.");

    return SqlResult::AI_ERROR;
  }

  if (parameters.IsDataAtExecNeeded()) {
    if (currentQuery->GetType() == query::QueryType::BATCH
        || currentQuery->GetType() == query::QueryType::STREAMING) {
      AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                      "Data-at-execution is not supported with batching.");

      return SqlResult::AI_ERROR;
    }

    // In a normal case we should return SQL_NEED_DATA, but since we dont support
    // parameters in sql statemets we will return not support and SQL_ERROR
    AddStatusRecord(
        SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
        "Usage the value \"?\" as query parameters are not supported."
        "See ODBC limitations at "
        "https://github.com/aws/amazon-documentdb-odbc-driver/blob/develop/src/"
        "markdown/support/odbc-limitations.md");

    return SqlResult::AI_ERROR;
  }

  return currentQuery->Execute();
}

void Statement::ExecuteGetColumnsMetaQuery(
    const boost::optional< std::string >& catalog,
    const boost::optional< std::string >& schema, const std::string& table,
    const std::string& column) {
  DOCUMENTDB_ODBC_API_CALL(
      InternalExecuteGetColumnsMetaQuery(catalog, schema, table, column));
}

SqlResult::Type Statement::InternalExecuteGetColumnsMetaQuery(
    const boost::optional< std::string >& catalog,
    const boost::optional< std::string >& schema, const std::string& table,
    const std::string& column) {
  if (currentQuery.get())
    currentQuery->Close();

  std::string schema0(schema.get_value_or(""));

  if (schema0.empty())
    schema0 = connection.GetSchema();

  currentQuery.reset(new query::ColumnMetadataQuery(*this, connection, catalog,
                                                    schema, table, column));

  return currentQuery->Execute();
}

void Statement::ExecuteGetTablesMetaQuery(
    const boost::optional< std::string >& catalog,
    const boost::optional< std::string >& schema, const std::string& table,
    const boost::optional< std::string >& tableType) {
  DOCUMENTDB_ODBC_API_CALL(
      InternalExecuteGetTablesMetaQuery(catalog, schema, table, tableType));
}

SqlResult::Type Statement::InternalExecuteGetTablesMetaQuery(
    const boost::optional< std::string >& catalog,
    const boost::optional< std::string >& schema, const std::string& table,
    const boost::optional< std::string >& tableType) {
  if (currentQuery.get())
    currentQuery->Close();

  currentQuery.reset(new query::TableMetadataQuery(*this, connection, catalog,
                                                   schema, table, tableType));

  return currentQuery->Execute();
}

void Statement::ExecuteGetForeignKeysQuery(
    const std::string& primaryCatalog, const std::string& primarySchema,
    const std::string& primaryTable,
    const boost::optional< std::string >& foreignCatalog,
    const boost::optional< std::string >& foreignSchema,
    const std::string& foreignTable) {
  DOCUMENTDB_ODBC_API_CALL(InternalExecuteGetForeignKeysQuery(
      primaryCatalog, primarySchema, primaryTable, foreignCatalog,
      foreignSchema, foreignTable));
}

SqlResult::Type Statement::InternalExecuteGetForeignKeysQuery(
    const std::string& primaryCatalog, const std::string& primarySchema,
    const std::string& primaryTable,
    const boost::optional< std::string >& foreignCatalog,
    const boost::optional< std::string >& foreignSchema,
    const std::string& foreignTable) {
  if (currentQuery.get())
    currentQuery->Close();

  currentQuery.reset(new query::ForeignKeysQuery(
      *this, connection, foreignCatalog, foreignSchema, foreignTable));

  return currentQuery->Execute();
}

void Statement::ExecuteGetPrimaryKeysQuery(
    const boost::optional< std::string >& catalog,
    const boost::optional< std::string >& schema,
    const boost::optional< std::string >& table) {
  DOCUMENTDB_ODBC_API_CALL(
      InternalExecuteGetPrimaryKeysQuery(catalog, schema, table));
}

SqlResult::Type Statement::InternalExecuteGetPrimaryKeysQuery(
    const boost::optional< std::string >& catalog,
    const boost::optional< std::string >& schema,
    const boost::optional< std::string >& table) {
  if (currentQuery.get())
    currentQuery->Close();

  currentQuery.reset(
      new query::PrimaryKeysQuery(*this, connection, catalog, schema, table));

  return currentQuery->Execute();
}

void Statement::ExecuteSpecialColumnsQuery(int16_t type,
                                           const std::string& catalog,
                                           const std::string& schema,
                                           const std::string& table,
                                           int16_t scope, int16_t nullable) {
  DOCUMENTDB_ODBC_API_CALL(InternalExecuteSpecialColumnsQuery(
      type, catalog, schema, table, scope, nullable));
}

SqlResult::Type Statement::InternalExecuteSpecialColumnsQuery(
    int16_t type, const std::string& catalog, const std::string& schema,
    const std::string& table, int16_t scope, int16_t nullable) {
  if (type != SQL_BEST_ROWID && type != SQL_ROWVER) {
    AddStatusRecord(SqlState::SHY097_COLUMN_TYPE_OUT_OF_RANGE,
                    "An invalid IdentifierType value was specified.");

    return SqlResult::AI_ERROR;
  }

  if (currentQuery.get())
    currentQuery->Close();

  currentQuery.reset(new query::SpecialColumnsQuery(
      *this, type, catalog, schema, table, scope, nullable));

  return currentQuery->Execute();
}

void Statement::ExecuteGetTypeInfoQuery(int16_t sqlType) {
  DOCUMENTDB_ODBC_API_CALL(InternalExecuteGetTypeInfoQuery(sqlType));
}

SqlResult::Type Statement::InternalExecuteGetTypeInfoQuery(int16_t sqlType) {
  if (sqlType != SQL_ALL_TYPES && !type_traits::IsSqlTypeSupported(sqlType)) {
    std::stringstream builder;
    builder << "Data type is not supported. [typeId=" << sqlType << ']';

    AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                    builder.str());

    return SqlResult::AI_ERROR;
  }

  if (currentQuery.get())
    currentQuery->Close();

  currentQuery.reset(new query::TypeInfoQuery(*this, connection, sqlType));

  return currentQuery->Execute();
}

void Statement::FreeResources(int16_t option) {
  DOCUMENTDB_ODBC_API_CALL(InternalFreeResources(option));
}

SqlResult::Type Statement::InternalFreeResources(int16_t option) {
  switch (option) {
    case SQL_DROP: {
      AddStatusRecord("Deprecated, call SQLFreeHandle instead");

      return SqlResult::AI_ERROR;
    }

    case SQL_CLOSE: {
      return InternalClose();
    }

    case SQL_UNBIND: {
      SafeUnbindAllColumns();

      break;
    }

    case SQL_RESET_PARAMS: {
      parameters.UnbindAll();

      break;
    }

    default: {
      AddStatusRecord(
          SqlState::SHY092_OPTION_TYPE_OUT_OF_RANGE,
          "The value specified for the argument Option was invalid");
      return SqlResult::AI_ERROR;
    }
  }
  return SqlResult::AI_SUCCESS;
}

void Statement::Close() {
  DOCUMENTDB_ODBC_API_CALL(InternalClose());
}

SqlResult::Type Statement::InternalClose() {
  if (!currentQuery.get())
    return SqlResult::AI_SUCCESS;

  SqlResult::Type result = currentQuery->Close();

  return result;
}

void Statement::FetchScroll(int16_t orientation, int64_t offset) {
  DOCUMENTDB_ODBC_API_CALL(InternalFetchScroll(orientation, offset));
}

SqlResult::Type Statement::InternalFetchScroll(int16_t orientation,
                                               int64_t offset) {
  UNREFERENCED_PARAMETER(offset);

  if (orientation != SQL_FETCH_NEXT) {
    AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                    "Only SQL_FETCH_NEXT FetchOrientation type is supported");

    return SqlResult::AI_ERROR;
  }

  return InternalFetchRow();
}

void Statement::FetchRow() {
  DOCUMENTDB_ODBC_API_CALL(InternalFetchRow());
}

SqlResult::Type Statement::InternalFetchRow() {
  if (rowsFetched)
    *rowsFetched = 0;

  if (!currentQuery.get()) {
    AddStatusRecord(SqlState::S24000_INVALID_CURSOR_STATE,
                    "Cursor is not in the open state");

    return SqlResult::AI_ERROR;
  }

  if (columnBindOffset) {
    for (app::ColumnBindingMap::iterator it = columnBindings.begin();
         it != columnBindings.end(); ++it)
      it->second.SetByteOffset(*columnBindOffset);
  }

  SQLINTEGER fetched = 0;
  SQLINTEGER errors = 0;

  for (SqlUlen i = 0; i < rowArraySize; ++i) {
    for (app::ColumnBindingMap::iterator it = columnBindings.begin();
         it != columnBindings.end(); ++it)
      it->second.SetElementOffset(i);

    SqlResult::Type res = currentQuery->FetchNextRow(columnBindings);

    if (res == SqlResult::AI_SUCCESS || res == SqlResult::AI_SUCCESS_WITH_INFO)
      ++fetched;
    else if (res != SqlResult::AI_NO_DATA)
      ++errors;

    if (rowStatuses)
      rowStatuses[i] = SqlResultToRowResult(res);
  }

  if (rowsFetched)
    *rowsFetched =
        fetched < 0 ? static_cast< SQLINTEGER >(rowArraySize) : fetched;

  if (fetched > 0)
    return errors == 0 ? SqlResult::AI_SUCCESS
                       : SqlResult::AI_SUCCESS_WITH_INFO;

  return errors == 0 ? SqlResult::AI_NO_DATA : SqlResult::AI_ERROR;
}

const meta::ColumnMetaVector* Statement::GetMeta() {
  if (!currentQuery.get()) {
    AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR, "Query is not executed.");

    return 0;
  }

  return currentQuery->GetMeta();
}

bool Statement::DataAvailable() const {
  return currentQuery.get() && currentQuery->DataAvailable();
}

void Statement::MoreResults() {
  DOCUMENTDB_ODBC_API_CALL(InternalMoreResults());
}

SqlResult::Type Statement::InternalMoreResults() {
  if (!currentQuery.get()) {
    AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR, "Query is not executed.");

    return SqlResult::AI_ERROR;
  }

  return currentQuery->NextResultSet();
}

void Statement::GetColumnAttribute(uint16_t colIdx, uint16_t attrId,
                                   SQLWCHAR* strbuf, int16_t buflen,
                                   int16_t* reslen, SqlLen* numbuf) {
  DOCUMENTDB_ODBC_API_CALL(InternalGetColumnAttribute(colIdx, attrId, strbuf,
                                                  buflen, reslen, numbuf));
}

SqlResult::Type Statement::InternalGetColumnAttribute(
    uint16_t colIdx, uint16_t attrId, SQLWCHAR* strbuf, int16_t buflen,
    int16_t* reslen, SqlLen* numbuf) {
  const meta::ColumnMetaVector* meta = GetMeta();

  LOG_MSG("Collumn ID: " << colIdx << ", Attribute ID: " << attrId);

  if (!meta)
    return SqlResult::AI_ERROR;

  if (colIdx > meta->size() || colIdx < 1) {
    AddStatusRecord(SqlState::SHY000_GENERAL_ERROR,
                    "Column index is out of range.", 0, colIdx);

    return SqlResult::AI_ERROR;
  }

  const meta::ColumnMeta& columnMeta = meta->at(colIdx - 1);

  bool found = false;

  LOG_DEBUG_MSG("calling GetAttribute, numbuf: " << numbuf);

  // NumericAttributePtr field is used.
  if (numbuf) {
    found = columnMeta.GetAttribute(attrId, *numbuf);
    LOG_DEBUG_MSG("numbuf found: " << numbuf);
  }

  // NumericAttributePtr field is unused.
  if (!found) {
    std::string out;

    found = columnMeta.GetAttribute(attrId, out);

    size_t outSize = out.size();

    if (found) {
      LOG_DEBUG_MSG("out found: " << out);
      bool isTruncated = false;
      if (strbuf)
        // Length is given in bytes
        outSize =
            utility::CopyStringToBuffer(out, strbuf, buflen, isTruncated, true);
      if (reslen)
        *reslen = static_cast< int16_t >(outSize);
      if (isTruncated)
        return SqlResult::AI_SUCCESS_WITH_INFO;
    }
  }

  if (!found) {
    AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                    "Unknown attribute.");

    return SqlResult::AI_ERROR;
  }

  return SqlResult::AI_SUCCESS;
}

int64_t Statement::AffectedRows() {
  int64_t rowCnt = 0;

  DOCUMENTDB_ODBC_API_CALL(InternalAffectedRows(rowCnt));

  return rowCnt;
}

SqlResult::Type Statement::InternalAffectedRows(int64_t& rowCnt) {
  if (!currentQuery.get()) {
    AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR, "Query is not executed.");

    return SqlResult::AI_ERROR;
  }

  rowCnt = currentQuery->AffectedRows();

  return SqlResult::AI_SUCCESS;
}

void Statement::SetRowsFetchedPtr(SQLINTEGER* ptr) {
  rowsFetched = ptr;
}

SQLINTEGER* Statement::GetRowsFetchedPtr() {
  return rowsFetched;
}

void Statement::SetRowStatusesPtr(SQLUSMALLINT* ptr) {
  rowStatuses = ptr;
}

SQLUSMALLINT* Statement::GetRowStatusesPtr() {
  return rowStatuses;
}

void Statement::SelectParam(void** paramPtr) {
  DOCUMENTDB_ODBC_API_CALL(InternalSelectParam(paramPtr));
}

SqlResult::Type Statement::InternalSelectParam(void** paramPtr) {
  if (!paramPtr) {
    AddStatusRecord(SqlState::SHY000_GENERAL_ERROR,
                    "Invalid parameter: ValuePtrPtr is null.");

    return SqlResult::AI_ERROR;
  }

  if (!currentQuery.get()) {
    AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR, "Query is not prepared.");

    return SqlResult::AI_ERROR;
  }

  app::Parameter* selected = parameters.GetSelectedParameter();

  if (selected && !selected->IsDataReady()) {
    AddStatusRecord(
        SqlState::S22026_DATA_LENGTH_MISMATCH,
        "Less data was sent for a parameter than was specified with "
        "the StrLen_or_IndPtr argument in SQLBindParameter.");

    return SqlResult::AI_ERROR;
  }

  selected = parameters.SelectNextParameter();

  if (selected) {
    *paramPtr = selected->GetBuffer().GetData();

    return SqlResult::AI_NEED_DATA;
  }

  SqlResult::Type res = currentQuery->Execute();

  if (res != SqlResult::AI_SUCCESS)
    res = SqlResult::AI_SUCCESS_WITH_INFO;

  return res;
}

void Statement::PutData(void* data, SqlLen len) {
  DOCUMENTDB_ODBC_API_CALL(InternalPutData(data, len));
}

SqlResult::Type Statement::InternalPutData(void* data, SqlLen len) {
  if (!data && len != 0 && len != SQL_DEFAULT_PARAM && len != SQL_NULL_DATA) {
    AddStatusRecord(
        SqlState::SHY009_INVALID_USE_OF_NULL_POINTER,
        "Invalid parameter: DataPtr is null StrLen_or_Ind is not 0, "
        "SQL_DEFAULT_PARAM, or SQL_NULL_DATA.");

    return SqlResult::AI_ERROR;
  }

  if (!parameters.IsParameterSelected()) {
    AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR,
                    "Parameter is not selected with the SQLParamData.");

    return SqlResult::AI_ERROR;
  }

  app::Parameter* param = parameters.GetSelectedParameter();

  if (!param) {
    AddStatusRecord(SqlState::SHY000_GENERAL_ERROR,
                    "Selected parameter has been unbound.");

    return SqlResult::AI_ERROR;
  }

  param->PutData(data, len);

  return SqlResult::AI_SUCCESS;
}

void Statement::DescribeParam(int16_t paramNum, int16_t* dataType,
                              SqlUlen* paramSize, int16_t* decimalDigits,
                              int16_t* nullable) {
  DOCUMENTDB_ODBC_API_CALL(InternalDescribeParam(paramNum, dataType, paramSize,
                                             decimalDigits, nullable));
}

SqlResult::Type Statement::InternalDescribeParam(int16_t paramNum,
                                                 int16_t* dataType,
                                                 SqlUlen* paramSize,
                                                 int16_t* decimalDigits,
                                                 int16_t* nullable) {
  query::Query* qry = currentQuery.get();
  if (!qry) {
    AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR, "Query is not prepared.");

    return SqlResult::AI_ERROR;
  }

  if (qry->GetType() != query::QueryType::DATA) {
    AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR,
                    "Query is not SQL data query.");

    return SqlResult::AI_ERROR;
  }

  int8_t type = parameters.GetParamType(paramNum, 0);

  LOG_MSG("Type: " << type);

  if (!type) {
    SqlResult::Type res = UpdateParamsMeta();

    if (res != SqlResult::AI_SUCCESS)
      return res;

    type = parameters.GetParamType(paramNum, impl::binary::DOCUMENTDB_HDR_NULL);
  }

  boost::optional< int16_t > sqlType = type_traits::BinaryToSqlType(type);
  if (dataType && sqlType)
    *dataType = *sqlType;

  boost::optional< int32_t > colSize = type_traits::BinaryTypeColumnSize(type);
  if (paramSize && colSize)
    *paramSize = *colSize;

  boost::optional< int16_t > decDigits =
      type_traits::BinaryTypeDecimalDigits(type);
  if (decimalDigits && decDigits)
    *decimalDigits = *decDigits;

  if (nullable)
    *nullable = type_traits::BinaryTypeNullability(type);

  return SqlResult::AI_SUCCESS;
}

SqlResult::Type Statement::UpdateParamsMeta() {
  query::Query* qry0 = currentQuery.get();

  assert(qry0 != 0);
  assert(qry0->GetType() == query::QueryType::DATA);

  query::DataQuery* qry = static_cast< query::DataQuery* >(qry0);

  const std::string& schema = connection.GetSchema();
  const std::string& sql = qry->GetSql();

  QueryGetParamsMetaRequest req(schema, sql);
  QueryGetParamsMetaResponse rsp;

  try {
    connection.SyncMessage(req, rsp);
  } catch (const OdbcError& err) {
    AddStatusRecord(err);

    return SqlResult::AI_ERROR;
  } catch (const DocumentDbError& err) {
    AddStatusRecord(err.GetText());

    return SqlResult::AI_ERROR;
  }

  if (rsp.GetStatus() != ResponseStatus::SUCCESS) {
    LOG_MSG("Error: " << rsp.GetError());

    AddStatusRecord(ResponseStatusToSqlState(rsp.GetStatus()), rsp.GetError());

    return SqlResult::AI_ERROR;
  }

  parameters.UpdateParamsTypes(rsp.GetTypeIds());

  for (size_t i = 0; i < rsp.GetTypeIds().size(); ++i) {
    LOG_MSG("[" << i << "] Parameter type: " << rsp.GetTypeIds()[i]);
  }

  return SqlResult::AI_SUCCESS;
}

uint16_t Statement::SqlResultToRowResult(SqlResult::Type value) {
  switch (value) {
    case SqlResult::AI_NO_DATA:
      return SQL_ROW_NOROW;

    case SqlResult::AI_SUCCESS:
      return SQL_ROW_SUCCESS;

    case SqlResult::AI_SUCCESS_WITH_INFO:
      return SQL_ROW_SUCCESS_WITH_INFO;

    default:
      return SQL_ROW_ERROR;
  }
}
}  // namespace odbc
}  // namespace documentdb
