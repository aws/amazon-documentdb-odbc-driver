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

#include <bsoncxx/json.hpp>
#include <mongocxx/options/aggregate.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/exception/exception.hpp>
#include <mongocxx/pipeline.hpp>

#include <mongocxx/collection.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/options/aggregate.hpp>
#include <mongocxx/pipeline.hpp>

#include "ignite/odbc/connection.h"
#include "ignite/odbc/jni/documentdb_mql_query_context.h"
#include "ignite/odbc/jni/documentdb_query_mapping_service.h"
#include "ignite/odbc/log.h"
#include "ignite/odbc/message.h"
#include "ignite/odbc/mongo_cursor.h"
#include "ignite/odbc/odbc_error.h"
#include "ignite/odbc/query/batch_query.h"
#include "ignite/odbc/query/data_query.h"

using ignite::odbc::jni::DocumentDbConnectionProperties;
using ignite::odbc::jni::DocumentDbDatabaseMetadata;
using ignite::odbc::jni::DocumentDbMqlQueryContext;
using ignite::odbc::jni::DocumentDbQueryMappingService;
using ignite::odbc::jni::JdbcColumnMetadata;

namespace ignite {
namespace odbc {
namespace query {
DataQuery::DataQuery(diagnostic::DiagnosableAdapter& diag,
                     Connection& connection, const std::string& sql,
                     const app::ParameterSet& params, int32_t& timeout)
    : Query(diag, QueryType::DATA),
      _connection(connection),
      _sql(sql),
      _params(params),
      resultMetaAvailable(false),
      _resultMeta(),
      _cursor(),
      timeout(timeout) {
  // No-op.
}

DataQuery::~DataQuery() {
  InternalClose();
}

SqlResult::Type DataQuery::Execute() {
  if (_cursor.get())
    InternalClose();

  return MakeRequestExecute();
}

const meta::ColumnMetaVector* DataQuery::GetMeta() {
  if (!resultMetaAvailable) {
    MakeRequestResultsetMeta();

    if (!resultMetaAvailable)
      return nullptr;
  }

  return &_resultMeta;
}

SqlResult::Type DataQuery::FetchNextRow(app::ColumnBindingMap& columnBindings) {
  if (!_cursor.get()) {
    diag.AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR,
                         "Query was not executed.");

    return SqlResult::AI_ERROR;
  }

  if (!_cursor->HasData())
    return SqlResult::AI_NO_DATA;

  if (!_cursor->Increment())
    return SqlResult::AI_NO_DATA;

  MongoRow* row = _cursor->GetRow();

  if (!row) {
    diag.AddStatusRecord("Unknown error.");

    return SqlResult::AI_ERROR;
  }

  for (uint16_t i = 1; i < row->GetSize() + 1; ++i) {
    app::ColumnBindingMap::iterator it = columnBindings.find(i);

    if (it == columnBindings.end())
      continue;

    app::ConversionResult::Type convRes =
        row->ReadColumnToBuffer(i, it->second);

    SqlResult::Type result = ProcessConversionResult(convRes, 0, i);

    if (result == SqlResult::AI_ERROR)
      return SqlResult::AI_ERROR;
  }

  return SqlResult::AI_SUCCESS;
}

SqlResult::Type DataQuery::GetColumn(uint16_t columnIdx,
                                     app::ApplicationDataBuffer& buffer) {
  if (!_cursor.get()) {
    diag.AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR,
                         "Query was not executed.");

    return SqlResult::AI_ERROR;
  }

  MongoRow* row = _cursor->GetRow();

  if (!row) {
    diag.AddStatusRecord(SqlState::S24000_INVALID_CURSOR_STATE,
                         "Cursor has reached end of the result set.");

    return SqlResult::AI_ERROR;
  }

  app::ConversionResult::Type convRes =
      row->ReadColumnToBuffer(columnIdx, buffer);

  SqlResult::Type result = ProcessConversionResult(convRes, 0, columnIdx);

  return result;
}

SqlResult::Type DataQuery::Close() {
  return InternalClose();
}

SqlResult::Type DataQuery::InternalClose() {
  if (!_cursor.get())
    return SqlResult::AI_SUCCESS;

  SqlResult::Type result = MakeRequestClose();
  if (result == SqlResult::AI_SUCCESS) {
    _cursor.reset();
  }

  return result;
}

bool DataQuery::DataAvailable() const {
  return _cursor.get() && _cursor->HasData();
}

int64_t DataQuery::AffectedRows() const {
  return 0;
}

SqlResult::Type DataQuery::NextResultSet() {
    InternalClose();
    return SqlResult::AI_NO_DATA;
}

SqlResult::Type DataQuery::MakeRequestExecute() {
  _cursor.reset();
  return MakeRequestFetch();
}

SqlResult::Type DataQuery::MakeRequestClose() {
  // TODO: AD-604 - MakeRequestExecute
  // https://bitquill.atlassian.net/browse/AD-604
  return SqlResult::AI_SUCCESS;
}

SqlResult::Type DataQuery::MakeRequestFetch() {
  try {
    SharedPointer< DocumentDbMqlQueryContext > mqlQueryContext;
    IgniteError error;

    SqlResult::Type result = GetMqlQueryContext(mqlQueryContext, error);
    if (result != SqlResult::AI_SUCCESS) {
      diag.AddStatusRecord(error.GetText());
      return result;
    }

    std::vector< std::string > const& aggregateOperations =
        mqlQueryContext.Get()->GetAggregateOperations();
    std::vector< JdbcColumnMetadata >& columnMetadata =
        mqlQueryContext.Get()->GetColumnMetadata();
    std::vector< std::string >& paths = mqlQueryContext.Get()->GetPaths();

    ReadJdbcColumnMetadataVector(columnMetadata);

    const config::Configuration& config = _connection.GetConfiguration();
    std::shared_ptr< mongocxx::client > const& mongoClient = _connection.GetMongoClient();
    mongocxx::database database = (*mongoClient.get())[config.GetDatabase()];
    mongocxx::collection collection =
        database[mqlQueryContext.Get()->GetCollectionName()];
    auto pipeline = mongocxx::pipeline{};
    for (auto const& stage : aggregateOperations) {
      pipeline.append_stage(bsoncxx::from_json(stage));
    }
    mongocxx::cursor cursor1 = collection.aggregate(pipeline);
    this->_cursor = std::make_unique< MongoCursor >(cursor1, columnMetadata, paths);

  return SqlResult::AI_SUCCESS;
  } catch (mongocxx::exception const& xcp) {
    std::stringstream message;
    message << "Unable to establish connection with DocumentDB."
            << " code: " << xcp.code().value()
            << " messagge: " << xcp.code().message()
            << " cause: " << xcp.what();
    odbc::IgniteError error(
        odbc::IgniteError::IGNITE_ERR_SECURE_CONNECTION_FAILURE,
        message.str().c_str());
    diag.AddStatusRecord(error.GetText());
    return SqlResult::AI_ERROR;
  }
}

SqlResult::Type DataQuery::GetMqlQueryContext(
    SharedPointer< DocumentDbMqlQueryContext >& mqlQueryContext,
    IgniteError& error) {
  SharedPointer< DocumentDbConnectionProperties > connectionProperties =
      _connection.GetConnectionProperties(error);
  if (error.GetCode() != IgniteError::IGNITE_SUCCESS) {
    return SqlResult::AI_ERROR;
  }
  SharedPointer< DocumentDbDatabaseMetadata > databaseMetadata =
      _connection.GetDatabaseMetadata(error);
  if (error.GetCode() != IgniteError::IGNITE_SUCCESS) {
    return SqlResult::AI_ERROR;
  }
  JniErrorInfo errInfo;
  SharedPointer< DocumentDbQueryMappingService > queryMappingService =
      DocumentDbQueryMappingService::Create(connectionProperties,
                                            databaseMetadata, errInfo);
  if (errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    IgniteError::SetError(errInfo.code, errInfo.errCls.c_str(),
                          errInfo.errMsg.c_str(), error);
    return SqlResult::AI_ERROR;
  }
  mqlQueryContext =
      queryMappingService.Get()->GetMqlQueryContext(_sql, 0, errInfo);
  if (errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    IgniteError::SetError(errInfo.code, errInfo.errCls.c_str(),
                          errInfo.errMsg.c_str(), error);
    return SqlResult::AI_ERROR;
  }
  return SqlResult::AI_SUCCESS;
}

SqlResult::Type DataQuery::MakeRequestMoreResults() {
  // TODO: AD-604 - MakeRequestExecute
  // https://bitquill.atlassian.net/browse/AD-604
  return SqlResult::AI_SUCCESS;
}

SqlResult::Type DataQuery::MakeRequestResultsetMeta() {
  IgniteError error;
  SharedPointer< DocumentDbMqlQueryContext > mqlQueryContext;
  SqlResult::Type sqlRes = GetMqlQueryContext(mqlQueryContext, error);
  if (!mqlQueryContext.IsValid() || sqlRes != SqlResult::AI_SUCCESS) {
    diag.AddStatusRecord(error.GetText());
    return SqlResult::AI_ERROR;
  }
  ReadJdbcColumnMetadataVector(mqlQueryContext.Get()->GetColumnMetadata());
  return SqlResult::AI_SUCCESS;
}

void DataQuery::ReadJdbcColumnMetadataVector(
    std::vector< JdbcColumnMetadata > jdbcVector) {
  using ignite::odbc::meta::ColumnMeta;
  _resultMeta.clear();

  if (jdbcVector.empty()) {
    return;
  }

  IgniteError error;
  int32_t prevPosition = 0;
  for (JdbcColumnMetadata jdbcMetadata : jdbcVector) {

    _resultMeta.emplace_back(ColumnMeta());
    _resultMeta.back().ReadJdbcMetadata(jdbcMetadata, prevPosition);
  }
  resultMetaAvailable = true;
}

SqlResult::Type DataQuery::ProcessConversionResult(
    app::ConversionResult::Type convRes, int32_t rowIdx, int32_t columnIdx) {
  switch (convRes) {
    case app::ConversionResult::AI_SUCCESS: {
      return SqlResult::AI_SUCCESS;
    }

    case app::ConversionResult::AI_NO_DATA: {
      return SqlResult::AI_NO_DATA;
    }

    case app::ConversionResult::AI_VARLEN_DATA_TRUNCATED: {
      diag.AddStatusRecord(
          SqlState::S01004_DATA_TRUNCATED,
          "Buffer is too small for the column data. Truncated from the right.",
          rowIdx, columnIdx);

      return SqlResult::AI_SUCCESS_WITH_INFO;
    }

    case app::ConversionResult::AI_FRACTIONAL_TRUNCATED: {
      diag.AddStatusRecord(
          SqlState::S01S07_FRACTIONAL_TRUNCATION,
          "Buffer is too small for the column data. Fraction truncated.",
          rowIdx, columnIdx);

      return SqlResult::AI_SUCCESS_WITH_INFO;
    }

    case app::ConversionResult::AI_INDICATOR_NEEDED: {
      diag.AddStatusRecord(
          SqlState::S22002_INDICATOR_NEEDED,
          "Indicator is needed but not suplied for the column buffer.", rowIdx,
          columnIdx);

      return SqlResult::AI_SUCCESS_WITH_INFO;
    }

    case app::ConversionResult::AI_UNSUPPORTED_CONVERSION: {
      diag.AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                           "Data conversion is not supported.", rowIdx,
                           columnIdx);

      return SqlResult::AI_SUCCESS_WITH_INFO;
    }

    case app::ConversionResult::AI_FAILURE:
    default: {
      diag.AddStatusRecord(SqlState::S01S01_ERROR_IN_ROW,
                           "Can not retrieve row column.", rowIdx, columnIdx);

      break;
    }
  }

  return SqlResult::AI_ERROR;
}

void DataQuery::SetResultsetMeta(const meta::ColumnMetaVector& value) {
  _resultMeta.assign(value.begin(), value.end());
  resultMetaAvailable = true;

  for (size_t i = 0; i < _resultMeta.size(); ++i) {
    meta::ColumnMeta& meta = _resultMeta.at(i);
    if (meta.GetDataType()) {
      LOG_MSG(
          "\n[" << i << "] SchemaName:     "
                << meta.GetSchemaName().get_value_or("") << "\n[" << i
                << "] TypeName:       " << meta.GetTableName().get_value_or("")
                << "\n[" << i
                << "] ColumnName:     " << meta.GetColumnName().get_value_or("")
                << "\n[" << i << "] ColumnType:     "
                << static_cast< int32_t >(*meta.GetDataType()));
    } else {
      LOG_MSG(
          "\n[" << i << "] SchemaName:     "
                << meta.GetSchemaName().get_value_or("") << "\n[" << i
                << "] TypeName:       " << meta.GetTableName().get_value_or("")
                << "\n[" << i
                << "] ColumnName:     " << meta.GetColumnName().get_value_or("")
                << "\n[" << i << "] ColumnType: not available");
    }
  }
}
}  // namespace query
}  // namespace odbc
}  // namespace ignite
