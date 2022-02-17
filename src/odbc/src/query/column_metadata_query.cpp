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

#include <ignite/impl/binary/binary_common.h>

#include "ignite/odbc/jni/java.h"
#include "ignite/odbc/type_traits.h"
#include "ignite/odbc/connection.h"
#include "ignite/odbc/message.h"
#include "ignite/odbc/log.h"
#include "ignite/odbc/odbc_error.h"
#include "ignite/odbc/query/column_metadata_query.h"

namespace {
struct ResultColumn {
    enum Type {
        /** Catalog name. NULL if not applicable to the data source. */
        TABLE_CAT = 1,

        /** Schema name. NULL if not applicable to the data source. */
        TABLE_SCHEM,

        /** Table name. */
        TABLE_NAME,

        /** Column name. */
        COLUMN_NAME,

        /** SQL data type. */
        DATA_TYPE,

        /** Data source-dependent data type name. */
        TYPE_NAME,

        /** Column size. */
        COLUMN_SIZE,

        /** The length in bytes of data transferred on fetch. */
        BUFFER_LENGTH,

        /** The total number of significant digits to the right of the decimal
           point. */
        DECIMAL_DIGITS,

        /** Precision. */
        NUM_PREC_RADIX,

        /** Nullability of the data in column (int). */
        NULLABLE,

        /** A description of the column. */
        REMARKS,
        // the start of my added values -AL-
        /** Default value for the column. May be null. */
        COLUMN_DEF,

        /** SQL data type. */
        SQL_DATA_TYPE,

        /** Subtype code for datetime and interval data types. */
        SQL_DATETIME_SUB,

        /** Maximum length in bytes of a character or binary data type column.
           NULL for other data types. */
        CHAR_OCTET_LENGTH,

        /** Index of column in table (starting at 1) */
        ORDINAL_POSITION,

        /** Nullability of data in column (String). */
        IS_NULLABLE
    };
};
}  // namespace

namespace ignite {
namespace odbc {
namespace query {
ColumnMetadataQuery::ColumnMetadataQuery(
    diagnostic::DiagnosableAdapter& diag,
    SharedPointer< GlobalJObject > connection, const std::string& catalog,
    const std::string& schema, const std::string& table,
    const std::string& column)
    : Query(diag, QueryType::COLUMN_METADATA),
      connection(connection),
      catalog(catalog),
      schema(schema),
      table(table),
      column(column),
      executed(false),
      fetched(false),
      meta(),
      columnsMeta() {
  using namespace ignite::impl::binary;
  using namespace ignite::odbc::type_traits;

  using meta::ColumnMeta;

  columnsMeta.reserve(12);  // -AL- todo add to 24 or something

  const std::string sch;
  const std::string tbl;

  columnsMeta.push_back(ColumnMeta(sch, tbl, "TABLE_CAT", IGNITE_TYPE_STRING));
  columnsMeta.push_back(
      ColumnMeta(sch, tbl, "TABLE_SCHEM", IGNITE_TYPE_STRING));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "TABLE_NAME", IGNITE_TYPE_STRING));
  columnsMeta.push_back(
      ColumnMeta(sch, tbl, "COLUMN_NAME", IGNITE_TYPE_STRING));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "DATA_TYPE", IGNITE_TYPE_SHORT));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "TYPE_NAME", IGNITE_TYPE_STRING));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "COLUMN_SIZE", IGNITE_TYPE_INT));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "BUFFER_LENGTH", IGNITE_TYPE_INT));
  columnsMeta.push_back(
      ColumnMeta(sch, tbl, "DECIMAL_DIGITS", IGNITE_TYPE_SHORT));
  columnsMeta.push_back(
      ColumnMeta(sch, tbl, "NUM_PREC_RADIX", IGNITE_TYPE_SHORT));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "NULLABLE", IGNITE_TYPE_SHORT));
  columnsMeta.push_back(ColumnMeta(sch, tbl, "REMARKS", IGNITE_TYPE_STRING));
}

ColumnMetadataQuery::~ColumnMetadataQuery() {
  // No-op.
}

SqlResult::Type
ColumnMetadataQuery::Execute() {  // place to plug in new code -AL-
  if (executed)
    Close();

  SqlResult::Type result = MakeRequestGetColumnsMeta();

  if (result == SqlResult::AI_SUCCESS) {
    executed = true;
    fetched = false;

    cursor = meta.begin();
  }

  return result;
}

const meta::ColumnMetaVector* ColumnMetadataQuery::GetMeta() {
  return &columnsMeta;
}

SqlResult::Type ColumnMetadataQuery::FetchNextRow(
    app::ColumnBindingMap& columnBindings) {
  if (!executed) {
    diag.AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR,
                         "Query was not executed.");

    return SqlResult::AI_ERROR;
  }

  if (!fetched)
    fetched = true;
  else
    ++cursor;

  if (cursor == meta.end())
    return SqlResult::AI_NO_DATA;

  app::ColumnBindingMap::iterator it;

  for (it = columnBindings.begin(); it != columnBindings.end(); ++it)
    GetColumn(it->first, it->second);

  return SqlResult::AI_SUCCESS;
}

SqlResult::Type ColumnMetadataQuery::GetColumn(
    uint16_t columnIdx,
    app::ApplicationDataBuffer&
        buffer) {  // -AL- I need to change this code to match resultSet return
                   // values. todo
  // this place has BinaryTypeToSqlTypeName, and other mentioned method names to
  // adapt
  if (!executed) {
    diag.AddStatusRecord(SqlState::SHY010_SEQUENCE_ERROR,
                         "Query was not executed.");

    return SqlResult::AI_ERROR;
  }

  if (cursor == meta.end()) {
    diag.AddStatusRecord(SqlState::S24000_INVALID_CURSOR_STATE,
                         "Cursor has reached end of the result set.");

    return SqlResult::AI_ERROR;
  }

  const meta::ColumnMeta& currentColumn = *cursor;
  uint8_t columnType = currentColumn.GetDataType();

  switch (columnIdx) {
    case ResultColumn::TABLE_CAT: {
      buffer.PutNull();
      break;
    }

    case ResultColumn::TABLE_SCHEM: {
      buffer.PutString(currentColumn.GetSchemaName());
      break;
    }

    case ResultColumn::TABLE_NAME: {
      buffer.PutString(currentColumn.GetTableName());
      break;
    }

    case ResultColumn::COLUMN_NAME: {
      buffer.PutString(currentColumn.GetColumnName());
      break;
    }

    case ResultColumn::DATA_TYPE: {
      buffer.PutInt16(type_traits::BinaryToSqlType(columnType));
      break;
    }

    case ResultColumn::TYPE_NAME: {
      buffer.PutString(
          type_traits::BinaryTypeToSqlTypeName(currentColumn.GetDataType()));
      break;
    }

    case ResultColumn::COLUMN_SIZE: {
      buffer.PutInt32(type_traits::BinaryTypeColumnSize(columnType));
      break;
    }

    case ResultColumn::BUFFER_LENGTH: {
      buffer.PutInt32(type_traits::BinaryTypeTransferLength(columnType));
      break;
    }

    case ResultColumn::DECIMAL_DIGITS: {
      int32_t decDigits = type_traits::BinaryTypeDecimalDigits(columnType);
      if (decDigits < 0)
        buffer.PutNull();
      else
        buffer.PutInt16(static_cast< int16_t >(decDigits));
      break;
    }

    case ResultColumn::NUM_PREC_RADIX: {
      buffer.PutInt16(type_traits::BinaryTypeNumPrecRadix(columnType));
      break;
    }

    case ResultColumn::NULLABLE: {
      buffer.PutInt16(type_traits::BinaryTypeNullability(columnType));
      break;
    }

    case ResultColumn::REMARKS: {
      buffer.PutNull();
      // buffer.PutString(); // todo, it should be string not null. 
      // It seems that I might need to create a currentColumn.GetRemarks() function
      // -AL-
      break;
    }
    // -AL- start of my added values

    case ResultColumn::COLUMN_DEF: {
      buffer.PutNull();
      // buffer.PutString(); // todo, it should be string not null.
      // It seems that I might need to create a currentColumn.getColumnDef()
      // function -AL-
      break;
    } 

    case ResultColumn::SQL_DATA_TYPE: {
      buffer.PutInt16(type_traits::BinaryToSqlType(columnType));
      break;
    }

    case ResultColumn::SQL_DATETIME_SUB: {
      buffer.PutNull(); // note: this is okay since JDBC does not use this value. -AL- 
      break;
    }

    case ResultColumn::CHAR_OCTET_LENGTH: {
      buffer.PutInt32(type_traits::BinaryToSqlType(columnType));
      break;
    }
 
    case ResultColumn::IS_NULLABLE: {
      buffer.PutNull(); 
      // buffer.PutString(); // todo, it should be string not null.
      // It seems that I might need to create a currentColumn.get isNullable()
      // function -AL-
      break;
    }

    default:
      break;
  }

  return SqlResult::AI_SUCCESS;
}

SqlResult::Type ColumnMetadataQuery::Close() {
  meta.clear();

  executed = false;

  return SqlResult::AI_SUCCESS;
}

bool ColumnMetadataQuery::DataAvailable() const {
  return cursor != meta.end();
}

int64_t ColumnMetadataQuery::AffectedRows() const {
  return 0;
}

SqlResult::Type ColumnMetadataQuery::NextResultSet() {
  return SqlResult::AI_NO_DATA;
}

SqlResult::Type ColumnMetadataQuery::MakeRequestGetColumnsMeta() {

  // replace req, rsp with JDBC call. catalog = null is to be passed
  // QueryGetColumnsMetaRequest req(schema, table, column);
  //QueryGetColumnsMetaResponse rsp;  // has the result of the req. Our result
                                    // is the result set from getColumns.

  // TODO update after Bruce's connection code is done

  IgniteError error;
  SharedPointer< DatabaseMetaData > databaseMetaData =
      connection.GetMetaData(error);
  if (!databaseMetaData.IsValid()
      || error.GetCode() != IgniteError::IGNITE_SUCCESS) {
      diag.AddStatusRecord(error.GetText());
      return SqlResult::AI_ERROR;
  }

  JniErrorInfo errInfo;
  SharedPointer< ResultSet > resultSet =
      databaseMetaData.Get()->GetColumns(catalog, schema, table, column, errInfo);
  if (!resultSet.IsValid()
      || errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      diag.AddStatusRecord(errInfo.errMsg);
      return SqlResult::AI_ERROR;
  }

  meta::ReadTableMetaVector(resultSet, meta);
  /*// temp comment out -AL-
  bool success;
  JniErrorInfo errInfo;
  SharedPointer< GlobalJObject > databaseMetadata;
  _ctx.Get()->ConnectionGetMetaData(connection, databaseMetadata, errInfo);
  */

  //SharedPointer< GlobalJObject > resultSet;

  // option 1 // this one makes more sense since the function doesn't throw
  // exceptions
  /*// temp comment out -AL-
  if (!_ctx.Get()->DatabaseMetaDataGetColumns(
          databaseMetadata, catalog, schema,
          table, column, resultSet,
          errInfo)) {
      LOG_MSG("Error: " << errInfo.errMsg);
      return SqlResult::AI_ERROR;
  }
  */

  // option 2
  //try {
  //  /* // temp comment out -AL-
  //  success = _ctx.Get()->DatabaseMetaDataGetColumns(
  //      databaseMetadata, catalog, schema,
  //      table, column, resultSet,
  //      errInfo);
  //      */
  //  // connection.SyncMessage(req, rsp);
  //  // what happens on Ignite [I might be wrong]: when syncMessage is
  //  // succssessful, call ReadOnSuccess, which then calls
  //  // meta::ReadColumnMetaVector(reader, meta, ver);.

  //  // getTables here -AL- Actually nvm, I think getColumns make more sense
  //  // here... dunno why I wrote getTables
  //} catch (const OdbcError& err) {
  //  diag.AddStatusRecord(err);

  //  return SqlResult::AI_ERROR;
  //} catch (const IgniteError& err) {
  //  diag.AddStatusRecord(err.GetText());

  //  return SqlResult::AI_ERROR;
  //}

  // orig ignite code
  /*
  if (rsp.GetStatus() != ResponseStatus::SUCCESS)
  {
      LOG_MSG("Error: " << rsp.GetError());
      diag.AddStatusRecord(ResponseStatusToSqlState(rsp.GetStatus()),
  rsp.GetError());

      return SqlResult::AI_ERROR;
  }
  */
  // ----- code before Bruce's branch is merged
  //meta::ColumnMetaVector meta;
  //ReadColumnMetaVector(resultSet, meta);
  //-----
  // meta = rsp.GetMeta();  // ignite uses reponse object to reap the results
  // into a meta object
  // ignite sets meta object in rsp, and also calls the construct vector
  // method in rsp.

  // us: write a function that takes the resultSet from getTables and produce
  // a meta vector
  for (size_t i = 0; i < meta.size(); ++i) {
    LOG_MSG("\n[" << i << "] SchemaName:     " << meta[i].GetSchemaName()
                  << "\n[" << i
                  << "] TableName:      " << meta[i].GetTableName() << "\n["
                  << i << "] ColumnName:     " << meta[i].GetColumnName()
                  << "\n[" << i << "] ColumnType:     "
                  << static_cast< int32_t >(meta[i].GetDataType()));
  }

  return SqlResult::AI_SUCCESS;
}
}  // namespace query
}  // namespace odbc
}

