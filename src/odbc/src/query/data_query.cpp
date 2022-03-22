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

#include <mongocxx/collection.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/options/aggregate.hpp>
#include <mongocxx/pipeline.hpp>

#include "ignite/odbc/connection.h"
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
      connection(connection),
      sql(sql),
      params(params),
      resultMetaAvailable(false),
      resultMeta(),
      cursor(),
      rowsAffected(),
      rowsAffectedIdx(0),
      cachedNextPage(),
      timeout(timeout) {
  // No-op.
}

DataQuery::~DataQuery() {
  InternalClose();
}

SqlResult::Type DataQuery::Execute() {
  if (cursor.get())
    InternalClose();

  return MakeRequestExecute();
}

const meta::ColumnMetaVector* DataQuery::GetMeta() {
  if (!resultMetaAvailable) {
    MakeRequestResultsetMeta();

    if (!resultMetaAvailable)
      return nullptr;
  }

  return &resultMeta;
}

SqlResult::Type DataQuery::FetchNextRow(app::ColumnBindingMap& columnBindings) {
  // TODO: AD-604 - MakeRequestExecute
  // https://bitquill.atlassian.net/browse/AD-604
  return SqlResult::AI_NO_DATA;

  if (!cursor.get()) {
    diag.AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR,
                         "Query was not executed.");

    return SqlResult::AI_ERROR;
  }

  if (!cursor->HasData())
    return SqlResult::AI_NO_DATA;

  cursor->Increment();

  if (cursor->NeedDataUpdate()) {
    if (cachedNextPage.get())
      cursor->UpdateData(cachedNextPage);
    else {
      SqlResult::Type result = MakeRequestFetch();

      if (result != SqlResult::AI_SUCCESS)
        return result;
    }
  }

  if (!cursor->HasData())
    return SqlResult::AI_NO_DATA;

  Row* row = cursor->GetRow();

  if (!row) {
    diag.AddStatusRecord("Unknown error.");

    return SqlResult::AI_ERROR;
  }

  for (int32_t i = 1; i < row->GetSize() + 1; ++i) {
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
  if (!cursor.get()) {
    diag.AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR,
                         "Query was not executed.");

    return SqlResult::AI_ERROR;
  }

  Row* row = cursor->GetRow();

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
  if (!cursor.get())
    return SqlResult::AI_SUCCESS;

  SqlResult::Type result = SqlResult::AI_SUCCESS;

  if (!IsClosedRemotely())
    result = MakeRequestClose();

  if (result == SqlResult::AI_SUCCESS) {
    cursor.reset();

    rowsAffectedIdx = 0;

    rowsAffected.clear();
  }

  return result;
}

bool DataQuery::DataAvailable() const {
  return cursor.get() && cursor->HasData();
}

int64_t DataQuery::AffectedRows() const {
  int64_t affected =
      rowsAffectedIdx < rowsAffected.size() ? rowsAffected[rowsAffectedIdx] : 0;

  if (affected >= 0)
    return affected;

  return connection.GetConfiguration().GetDefaultFetchSize();
}

SqlResult::Type DataQuery::NextResultSet() {
  if (rowsAffectedIdx + 1 >= rowsAffected.size()) {
    InternalClose();

    return SqlResult::AI_NO_DATA;
  }

  SqlResult::Type res = MakeRequestMoreResults();

  if (res == SqlResult::AI_SUCCESS)
    ++rowsAffectedIdx;

  return res;
}

bool DataQuery::IsClosedRemotely() const {
  for (size_t i = 0; i < rowsAffected.size(); ++i) {
    if (rowsAffected[i] < 0)
      return false;
  }

  return true;
}

SqlResult::Type DataQuery::MakeRequestExecute() {
  // TODO: AD-604 - MakeRequestExecute
  // https://bitquill.atlassian.net/browse/AD-604
  cursor.reset(new Cursor(0L));
  rowsAffectedIdx = 0;
  MakeRequestResultsetMeta();
  // SetResultsetMeta(rsp.GetMeta());// original ignite code
  return SqlResult::AI_SUCCESS;

  // -AL- original ignite code
  const std::string& schema = connection.GetSchema();

  QueryExecuteRequest req(schema, sql, params, timeout,
                          connection.IsAutoCommit());
  QueryExecuteResponse rsp;

  try {
    // Setting connection timeout to 1 second more than query timeout itself.
    int32_t connectionTimeout = timeout ? timeout + 1 : 0;

    bool success = connection.SyncMessage(req, rsp, connectionTimeout);

    if (!success) {
      diag.AddStatusRecord(SqlState::SHYT00_TIMEOUT_EXPIRED,
                           "Query timeout expired");

      return SqlResult::AI_ERROR;
    }
  } catch (const OdbcError& err) {
    diag.AddStatusRecord(err);

    return SqlResult::AI_ERROR;
  } catch (const IgniteError& err) {
    diag.AddStatusRecord(err.GetText());

    return SqlResult::AI_ERROR;
  }

  if (rsp.GetStatus() != ResponseStatus::SUCCESS) {
    LOG_MSG("Error: " << rsp.GetError());

    diag.AddStatusRecord(ResponseStatusToSqlState(rsp.GetStatus()),
                         rsp.GetError());

    return SqlResult::AI_ERROR;
  }

  rowsAffected = rsp.GetAffectedRows();
  SetResultsetMeta(rsp.GetMeta());

  LOG_MSG("Query id: " << rsp.GetQueryId());
  LOG_MSG("Affected Rows list size: " << rowsAffected.size());

  cursor.reset(new Cursor(rsp.GetQueryId()));

  rowsAffectedIdx = 0;

  return SqlResult::AI_SUCCESS;
}

SqlResult::Type DataQuery::MakeRequestClose() {
  // TODO: AD-604 - MakeRequestExecute
  // https://bitquill.atlassian.net/browse/AD-604
  return SqlResult::AI_SUCCESS;
}

SqlResult::Type DataQuery::MakeRequestFetch() {
  // TODO: AD-604 - MakeRequestExecute
  // https://bitquill.atlassian.net/browse/AD-604

  return SqlResult::AI_SUCCESS;
}

SqlResult::Type DataQuery::GetMqlQueryContext(
    SharedPointer< DocumentDbMqlQueryContext >& mqlQueryContext,
    IgniteError& error) {
  SharedPointer< DocumentDbConnectionProperties > connectionProperties =
      connection.GetConnectionProperties(error);
  if (error.GetCode() != IgniteError::IGNITE_SUCCESS) {
    return SqlResult::AI_ERROR;
  }
  SharedPointer< DocumentDbDatabaseMetadata > databaseMetadata =
      connection.GetDatabaseMetadata(error);
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
      queryMappingService.Get()->GetMqlQueryContext(sql, 0, errInfo);
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
  // TODO: AD-604 - MakeRequestExecute
  // https://bitquill.atlassian.net/browse/AD-604
  IgniteError error;
  SharedPointer< DocumentDbMqlQueryContext > mqlQueryContext;
  SqlResult::Type sqlRes = GetMqlQueryContext(mqlQueryContext, error);
  if (!mqlQueryContext.IsValid() || sqlRes != SqlResult::AI_SUCCESS) {
    // todo: do something with IgniteError
    diag.AddStatusRecord(error.GetText());
    return sqlRes;
  }

  ReadJdbcColumnMetadataVector(mqlQueryContext.Get()->GetColumnMetadata());
  // -AL-:
  // plan: implement/find jni call that receive sql query to output metadata
  // think I shouldn't need to implement more stuff:
  // m_DocumentDbMqlQueryContextGetColumnMetadata might be the one in the
  // current jni, we don't have sqlQuery but we do have mql query idea: call
  // DocumentDbMqlQueryContext(sql string, maxRowCount (an int)) to get
  // DocumentDbMqlQueryContext object Java doc: the DocumentDbMqlQueryContext
  // object includes the target collection, aggregation stages, and result set
  // metadata get resultset metadata from the DocumentDbMqlQueryContext object
  // [jni: DocumentdbMqlQueryContextGetColumnMetadata], implemented in [AD-545]
  // ^ .java: queryContext.getColumnMetaData() is enough.

  /*
  * old column_metadata code for reference
  *
  JniErrorInfo errInfo;
  SharedPointer< ResultSet > resultSet = databaseMetaData.Get()->GetColumns(
      catalog, schema, table, column, errInfo);
  if (!resultSet.IsValid()
      || errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    diag.AddStatusRecord(errInfo.errMsg);
    return SqlResult::AI_ERROR;
  }
  if (!resultSet.IsValid()
      || errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    diag.AddStatusRecord(errInfo.errMsg);
    return SqlResult::AI_ERROR;
  }

  meta::ReadColumnMetaVector(resultSet, resultMeta);
  */

  return SqlResult::AI_SUCCESS;

  // original ignite code -AL-
  const std::string& schema = connection.GetSchema();

  QueryGetResultsetMetaRequest req(schema, sql);
  QueryGetResultsetMetaResponse rsp;

  try {
    // Setting connection timeout to 1 second more than query timeout itself.
    int32_t connectionTimeout = timeout ? timeout + 1 : 0;
    bool success = connection.SyncMessage(req, rsp, connectionTimeout);

    if (!success) {
      diag.AddStatusRecord(SqlState::SHYT00_TIMEOUT_EXPIRED,
                           "Query timeout expired");

      return SqlResult::AI_ERROR;
    }
  } catch (const OdbcError& err) {
    diag.AddStatusRecord(err);

    return SqlResult::AI_ERROR;
  } catch (const IgniteError& err) {
    diag.AddStatusRecord(err.GetText());

    return SqlResult::AI_ERROR;
  }

  if (rsp.GetStatus() != ResponseStatus::SUCCESS) {
    LOG_MSG("Error: " << rsp.GetError());

    diag.AddStatusRecord(ResponseStatusToSqlState(rsp.GetStatus()),
                         rsp.GetError());

    return SqlResult::AI_ERROR;
  }

  SetResultsetMeta(rsp.GetMeta());

  return SqlResult::AI_SUCCESS;
}

void DataQuery::ReadJdbcColumnMetadataVector(
    std::vector< JdbcColumnMetadata > jdbcVector) {
  using ignite::odbc::meta::ColumnMeta;
  resultMeta.clear();

  if (jdbcVector.empty()) {
    return;
  }

  IgniteError error;
  int32_t prevPosition = 0;
  for (JdbcColumnMetadata jdbcMetadata : jdbcVector) {

    resultMeta.emplace_back(ColumnMeta());
    resultMeta.back().ReadJdbcMetadata(jdbcMetadata, prevPosition);
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
  resultMeta.assign(value.begin(), value.end());
  resultMetaAvailable = true;

  for (size_t i = 0; i < resultMeta.size(); ++i) {
    meta::ColumnMeta& meta = resultMeta.at(i);
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
