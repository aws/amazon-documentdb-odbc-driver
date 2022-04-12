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

#include "ignite/odbc/query/data_query.h"

#include <bsoncxx/json.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/exception/exception.hpp>
#include <mongocxx/options/aggregate.hpp>
#include <mongocxx/pipeline.hpp>

#include "ignite/odbc/connection.h"
#include "ignite/odbc/documentdb_cursor.h"
#include "ignite/odbc/jni/documentdb_mql_query_context.h"
#include "ignite/odbc/jni/documentdb_query_mapping_service.h"
#include "ignite/odbc/log.h"
#include "ignite/odbc/message.h"
#include "ignite/odbc/odbc_error.h"
#include "ignite/odbc/query/batch_query.h"

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
      connection_(connection),
      sql_(sql),
      params_(params),
      timeout_(timeout) {
  // No-op.

  LOG_DEBUG_MSG("DataQuery constructor is called, and exiting");
}

DataQuery::~DataQuery() {
  LOG_DEBUG_MSG("~DataQuery is called");

  InternalClose();

  LOG_DEBUG_MSG("~DataQuery exiting");
}

SqlResult::Type DataQuery::Execute() {
  LOG_DEBUG_MSG("Execute is called");

  if (cursor_.get())
    InternalClose();

  LOG_DEBUG_MSG("Execute exiting");

  return MakeRequestExecute();
}

const meta::ColumnMetaVector* DataQuery::GetMeta() {
  LOG_DEBUG_MSG("GetMeta is called");

  if (!resultMetaAvailable_) {
    MakeRequestResultsetMeta();

    if (!resultMetaAvailable_) {
      LOG_ERROR_MSG("GetMeta exiting with error. Returning nullptr");
      LOG_DEBUG_MSG("reason: result meta is not available");

      return nullptr;
    }
  }

  LOG_DEBUG_MSG("GetMeta exiting");

  return &resultMeta_;
}

SqlResult::Type DataQuery::FetchNextRow(app::ColumnBindingMap& columnBindings) {
  LOG_DEBUG_MSG("FetchNextRow is called");

  if (!cursor_.get()) {
    diag.AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR,
                         "Query was not executed.");

    LOG_ERROR_MSG("FetchNextRow exiting with AI_ERROR");
    LOG_DEBUG_MSG("reason: query was not executed");

    return SqlResult::AI_ERROR;
  }

  if (!cursor_->HasData()) {
    LOG_INFO_MSG("FetchNextRow exiting with AI_NO_DATA");
    LOG_DEBUG_MSG("reason: cursor does not have data");

    return SqlResult::AI_NO_DATA;
  }

  if (!cursor_->Increment()) {
    LOG_INFO_MSG("FetchNextRow exiting with AI_NO_DATA");
    LOG_DEBUG_MSG(
        "reason: cursor cannot be moved to the next row; either data update is "
        "required or there is no more data");

    return SqlResult::AI_NO_DATA;
  }

  DocumentDbRow* row = cursor_->GetRow();

  if (!row) {
    diag.AddStatusRecord("Unknown error.");

    LOG_ERROR_MSG("FetchNextRow exiting with AI_ERROR");
    LOG_DEBUG_MSG("Error unknown. Getting row from cursor failed.");

    return SqlResult::AI_ERROR;
  }

  for (uint32_t i = 1; i < row->GetSize() + 1; ++i) {
    app::ColumnBindingMap::iterator it = columnBindings.find(i);

    if (it == columnBindings.end())
      continue;

    app::ConversionResult::Type convRes =
        row->ReadColumnToBuffer(i, it->second);

    SqlResult::Type result = ProcessConversionResult(convRes, 0, i);

    if (result == SqlResult::AI_ERROR) {
      LOG_ERROR_MSG("FetchNextRow exiting with AI_ERROR");
      LOG_DEBUG_MSG(
          "error occured during column conversion operation, inside the for "
          "loop");

      return SqlResult::AI_ERROR;
    }
  }

  LOG_DEBUG_MSG("FetchNextRow exiting with AI_SUCCESS");

  return SqlResult::AI_SUCCESS;
}

SqlResult::Type DataQuery::GetColumn(uint16_t columnIdx,
                                     app::ApplicationDataBuffer& buffer) {
  LOG_DEBUG_MSG("GetColumn is called");

  if (!cursor_.get()) {
    diag.AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR,
                         "Query was not executed.");

    LOG_ERROR_MSG("GetColumn exiting with error: Query was not executed");

    return SqlResult::AI_ERROR;
  }

  DocumentDbRow* row = cursor_->GetRow();

  if (!row) {
    diag.AddStatusRecord(SqlState::S24000_INVALID_CURSOR_STATE,
                         "Cursor has reached end of the result set.");

    LOG_ERROR_MSG(
        "GetColumn exiting with error: Cursor has reached end of the result "
        "set");

    return SqlResult::AI_ERROR;
  }

  app::ConversionResult::Type convRes =
      row->ReadColumnToBuffer(columnIdx, buffer);

  SqlResult::Type result = ProcessConversionResult(convRes, 0, columnIdx);

  LOG_DEBUG_MSG("GetColumn exiting");

  return result;
}

SqlResult::Type DataQuery::Close() {
  LOG_DEBUG_MSG("Close is called, and exiting");

  return InternalClose();
}

SqlResult::Type DataQuery::InternalClose() {
  LOG_DEBUG_MSG("InternalClose is called");

  if (!cursor_.get()) {
    LOG_DEBUG_MSG("InternalClose exiting");

    return SqlResult::AI_SUCCESS;
  }

  SqlResult::Type result = MakeRequestClose();
  if (result == SqlResult::AI_SUCCESS) {
    cursor_.reset();
  }

  LOG_DEBUG_MSG("InternalClose exiting");

  return result;
}

bool DataQuery::DataAvailable() const {
  LOG_DEBUG_MSG("DataAvailable is called, and exiting");

  return cursor_.get() && cursor_->HasData();
}

int64_t DataQuery::AffectedRows() const {
  LOG_DEBUG_MSG("AffectedRows is called, and exiting");

  return 0;
}

SqlResult::Type DataQuery::NextResultSet() {
  LOG_DEBUG_MSG("NextResultSet is called");

  InternalClose();

  LOG_DEBUG_MSG("NextResultSet exiting");

  return SqlResult::AI_NO_DATA;
}

SqlResult::Type DataQuery::MakeRequestExecute() {
  LOG_DEBUG_MSG("MakeRequestExecute is called");

  cursor_.reset();

  LOG_DEBUG_MSG("MakeRequestExecute exiting");

  return MakeRequestFetch();
}

SqlResult::Type DataQuery::MakeRequestClose() {
  LOG_DEBUG_MSG("MakeRequestClose is called, and exiting");

  return SqlResult::AI_SUCCESS;
}

SqlResult::Type DataQuery::MakeRequestFetch() {
  LOG_DEBUG_MSG("MakeRequestFetch is called");

  try {
    SharedPointer< DocumentDbMqlQueryContext > mqlQueryContext;
    IgniteError error;

    SqlResult::Type result = GetMqlQueryContext(mqlQueryContext, error);
    if (result != SqlResult::AI_SUCCESS) {
      diag.AddStatusRecord(error.GetText());

      LOG_ERROR_MSG(
          "MakeRequestFetch exiting with error msg: " << error.GetText());

      return result;
    }

    std::vector< std::string > const& aggregateOperations =
        mqlQueryContext.Get()->GetAggregateOperations();
    std::vector< JdbcColumnMetadata >& columnMetadata =
        mqlQueryContext.Get()->GetColumnMetadata();
    std::vector< std::string >& paths = mqlQueryContext.Get()->GetPaths();

    if (!resultMetaAvailable_) {
      ReadJdbcColumnMetadataVector(columnMetadata);
    }

    const config::Configuration& config = connection_.GetConfiguration();
    std::string databaseName = config.GetDatabase();
    std::string collectionName = mqlQueryContext.Get()->GetCollectionName();

    std::shared_ptr< mongocxx::client > const& mongoClient =
        connection_.GetMongoClient();
    mongocxx::database database = mongoClient.get()->database(databaseName);
    mongocxx::collection collection = database[collectionName];
    auto pipeline = mongocxx::pipeline{};
    for (auto const& stage : aggregateOperations) {
      pipeline.append_stage(bsoncxx::from_json(stage));
    }
    mongocxx::cursor cursor = collection.aggregate(pipeline);

    this->cursor_.reset(new DocumentDbCursor(cursor, columnMetadata, paths));

    LOG_DEBUG_MSG("MakeRequestFetch exiting");

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

    LOG_ERROR_MSG(
        "MakeRequestFetch exiting with error msg: " << error.GetText());

    return SqlResult::AI_ERROR;
  }

  LOG_DEBUG_MSG("MakeRequestFetch exiting");
}

SqlResult::Type DataQuery::GetMqlQueryContext(
    SharedPointer< DocumentDbMqlQueryContext >& mqlQueryContext,
    IgniteError& error) {
  LOG_DEBUG_MSG("GetMqlQueryContext is called");

  SharedPointer< DocumentDbConnectionProperties > connectionProperties =
      connection_.GetConnectionProperties(error);
  if (error.GetCode() != IgniteError::IGNITE_SUCCESS) {
    LOG_ERROR_MSG(
        "GetMqlQueryContext exiting with error msg: " << error.GetText());

    return SqlResult::AI_ERROR;
  }
  SharedPointer< DocumentDbDatabaseMetadata > databaseMetadata =
      connection_.GetDatabaseMetadata(error);
  if (error.GetCode() != IgniteError::IGNITE_SUCCESS) {
    LOG_ERROR_MSG(
        "GetMqlQueryContext exiting with error msg: " << error.GetText());

    return SqlResult::AI_ERROR;
  }
  JniErrorInfo errInfo;
  SharedPointer< DocumentDbQueryMappingService > queryMappingService =
      DocumentDbQueryMappingService::Create(connectionProperties,
                                            databaseMetadata, errInfo);
  if (errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    IgniteError::SetError(errInfo.code, errInfo.errCls.c_str(),
                          errInfo.errMsg.c_str(), error);
    LOG_ERROR_MSG(
        "GetMqlQueryContext exiting with error msg: " << error.GetText());

    return SqlResult::AI_ERROR;
  }
  mqlQueryContext =
      queryMappingService.Get()->GetMqlQueryContext(sql_, 0, errInfo);
  if (errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    IgniteError::SetError(errInfo.code, errInfo.errCls.c_str(),
                          errInfo.errMsg.c_str(), error);

    LOG_ERROR_MSG(
        "GetMqlQueryContext exiting with error msg: " << error.GetText());

    return SqlResult::AI_ERROR;
  }
  LOG_DEBUG_MSG("GetMqlQueryContext exiting");

  return SqlResult::AI_SUCCESS;
}

SqlResult::Type DataQuery::MakeRequestMoreResults() {
  LOG_DEBUG_MSG("MakeRequestMoreResults is called, and exiting");

  return SqlResult::AI_SUCCESS;
}

SqlResult::Type DataQuery::MakeRequestResultsetMeta() {
  LOG_DEBUG_MSG("MakeRequestResultsetMeta is called");

  IgniteError error;
  SharedPointer< DocumentDbMqlQueryContext > mqlQueryContext;
  SqlResult::Type sqlRes = GetMqlQueryContext(mqlQueryContext, error);
  if (!mqlQueryContext.IsValid() || sqlRes != SqlResult::AI_SUCCESS) {
    diag.AddStatusRecord(error.GetText());
    LOG_ERROR_MSG(
        "MakeRequestResultsetMeta exiting with error msg: " << error.GetText());

    return SqlResult::AI_ERROR;
  }
  ReadJdbcColumnMetadataVector(mqlQueryContext.Get()->GetColumnMetadata());

  LOG_DEBUG_MSG("MakeRequestResultsetMeta exiting");

  return SqlResult::AI_SUCCESS;
}

void DataQuery::ReadJdbcColumnMetadataVector(
    std::vector< JdbcColumnMetadata > jdbcVector) {
  LOG_DEBUG_MSG("ReadJdbcColumnMetadataVector is called");

  using ignite::odbc::meta::ColumnMeta;
  resultMeta_.clear();

  if (jdbcVector.empty()) {
    LOG_ERROR_MSG(
        "ReadJdbcColumnMetadataVector exiting without reading jdbc vector");
    LOG_INFO_MSG("reason: jdbcVector is empty");

    return;
  }

  IgniteError error;
  int32_t prevPosition = 0;
  for (JdbcColumnMetadata jdbcMetadata : jdbcVector) {
    resultMeta_.emplace_back(ColumnMeta());
    resultMeta_.back().ReadJdbcMetadata(jdbcMetadata, prevPosition);
  }
  resultMetaAvailable_ = true;

  LOG_DEBUG_MSG("ReadJdbcColumnMetadataVector exiting");
}

SqlResult::Type DataQuery::ProcessConversionResult(
    app::ConversionResult::Type convRes, int32_t rowIdx, int32_t columnIdx) {
  LOG_DEBUG_MSG("ProcessConversionResult is called");

  switch (convRes) {
    case app::ConversionResult::Type::AI_SUCCESS: {
      LOG_DEBUG_MSG("parameter: convRes: AI_SUCCESS");
      LOG_DEBUG_MSG("ProcessConversionResult exiting");

      return SqlResult::AI_SUCCESS;
    }

    case app::ConversionResult::Type::AI_NO_DATA: {
      LOG_DEBUG_MSG("parameter: convRes: AI_NO_DATA");
      LOG_DEBUG_MSG("ProcessConversionResult exiting");

      return SqlResult::AI_NO_DATA;
    }

    case app::ConversionResult::Type::AI_VARLEN_DATA_TRUNCATED: {
      diag.AddStatusRecord(
          SqlState::S01004_DATA_TRUNCATED,
          "Buffer is too small for the column data. Truncated from the right.",
          rowIdx, columnIdx);
      LOG_DEBUG_MSG("parameter: convRes: AI_VARLEN_DATA_TRUNCATED");
      LOG_DEBUG_MSG(
          "ProcessConversionResult exiting with AI_SUCCESS_WITH_INFO: Buffer "
          "is too small for the column data. Truncated from the right.");

      return SqlResult::AI_SUCCESS_WITH_INFO;
    }

    case app::ConversionResult::Type::AI_FRACTIONAL_TRUNCATED: {
      diag.AddStatusRecord(
          SqlState::S01S07_FRACTIONAL_TRUNCATION,
          "Buffer is too small for the column data. Fraction truncated.",
          rowIdx, columnIdx);

      LOG_DEBUG_MSG("parameter: convRes: AI_FRACTIONAL_TRUNCATED");
      LOG_DEBUG_MSG(
          "ProcessConversionResult exiting with AI_SUCCESS_WITH_INFO: Buffer "
          "is too small for the column data. "
          "Fraction truncated.");

      return SqlResult::AI_SUCCESS_WITH_INFO;
    }

    case app::ConversionResult::Type::AI_INDICATOR_NEEDED: {
      diag.AddStatusRecord(
          SqlState::S22002_INDICATOR_NEEDED,
          "Indicator is needed but not suplied for the column buffer.", rowIdx,
          columnIdx);

      LOG_DEBUG_MSG("parameter: convRes: AI_INDICATOR_NEEDED");
      LOG_DEBUG_MSG(
          "ProcessConversionResult exiting with AI_SUCCESS_WITH_INFO: "
          "Indicator is needed but not suplied for the column buffer.");

      return SqlResult::AI_SUCCESS_WITH_INFO;
    }

    case app::ConversionResult::Type::AI_UNSUPPORTED_CONVERSION: {
      diag.AddStatusRecord(SqlState::SHYC00_OPTIONAL_FEATURE_NOT_IMPLEMENTED,
                           "Data conversion is not supported.", rowIdx,
                           columnIdx);

      LOG_DEBUG_MSG("parameter: convRes: AI_UNSUPPORTED_CONVERSION");
      LOG_DEBUG_MSG(
          "ProcessConversionResult exiting with AI_SUCCESS_WITH_INFO: Data "
          "conversion is not supported.");

      return SqlResult::AI_SUCCESS_WITH_INFO;
    }

    case app::ConversionResult::Type::AI_FAILURE:
      LOG_DEBUG_MSG("parameter: convRes: AI_FAILURE");
    default: {
      diag.AddStatusRecord(SqlState::S01S01_ERROR_IN_ROW,
                           "Can not retrieve row column.", rowIdx, columnIdx);
      LOG_ERROR_MSG(
          "Default case: ProcessConversionResult exiting. msg: Can not "
          "retrieve row column.");
      break;
    }
  }
  LOG_ERROR_MSG("ProcessConversionResult exiting with error");

  return SqlResult::AI_ERROR;
}

void DataQuery::SetResultsetMeta(const meta::ColumnMetaVector& value) {
  LOG_DEBUG_MSG("SetResultsetMeta is called");

  resultMeta_.assign(value.begin(), value.end());
  resultMetaAvailable_ = true;

  for (size_t i = 0; i < resultMeta_.size(); ++i) {
    meta::ColumnMeta& meta = resultMeta_.at(i);
    if (meta.GetDataType()) {
      LOG_DEBUG_MSG(
          "\n[" << i << "] SchemaName:     "
                << meta.GetSchemaName().get_value_or("") << "\n[" << i
                << "] TypeName:       " << meta.GetTableName().get_value_or("")
                << "\n[" << i
                << "] ColumnName:     " << meta.GetColumnName().get_value_or("")
                << "\n[" << i << "] ColumnType:     "
                << static_cast< int32_t >(*meta.GetDataType()));
    } else {
      LOG_DEBUG_MSG(
          "\n[" << i << "] SchemaName:     "
                << meta.GetSchemaName().get_value_or("") << "\n[" << i
                << "] TypeName:       " << meta.GetTableName().get_value_or("")
                << "\n[" << i
                << "] ColumnName:     " << meta.GetColumnName().get_value_or("")
                << "\n[" << i << "] ColumnType: not available");
    }
  }
  LOG_DEBUG_MSG("SetResultsetMeta exiting");
}
}  // namespace query
}  // namespace odbc
}  // namespace ignite
