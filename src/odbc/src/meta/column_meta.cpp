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

#include "ignite/common/utils.h"

#include "ignite/odbc/system/odbc_constants.h"
#include "ignite/odbc/meta/column_meta.h"
#include "ignite/odbc/type_traits.h"
#include "ignite/odbc/common_types.h"
#include "ignite/odbc/log.h"
#include "ignite/odbc/jni/java.h"

namespace ignite {
namespace odbc {
namespace meta {

#define DBG_STR_CASE(x) \
  case x:               \
    return #x

const char* ColumnMeta::AttrIdToString(uint16_t id) {
  switch (id) {
    DBG_STR_CASE(SQL_DESC_LABEL);
    DBG_STR_CASE(SQL_DESC_BASE_COLUMN_NAME);
    DBG_STR_CASE(SQL_DESC_NAME);
    DBG_STR_CASE(SQL_DESC_TABLE_NAME);
    DBG_STR_CASE(SQL_DESC_BASE_TABLE_NAME);
    DBG_STR_CASE(SQL_DESC_SCHEMA_NAME);
    DBG_STR_CASE(SQL_DESC_CATALOG_NAME);
    DBG_STR_CASE(SQL_DESC_LITERAL_PREFIX);
    DBG_STR_CASE(SQL_DESC_LITERAL_SUFFIX);
    DBG_STR_CASE(SQL_DESC_TYPE_NAME);
    DBG_STR_CASE(SQL_DESC_LOCAL_TYPE_NAME);
    DBG_STR_CASE(SQL_DESC_FIXED_PREC_SCALE);
    DBG_STR_CASE(SQL_DESC_AUTO_UNIQUE_VALUE);
    DBG_STR_CASE(SQL_DESC_CASE_SENSITIVE);
    DBG_STR_CASE(SQL_DESC_CONCISE_TYPE);
    DBG_STR_CASE(SQL_DESC_TYPE);
    DBG_STR_CASE(SQL_DESC_DISPLAY_SIZE);
    DBG_STR_CASE(SQL_DESC_LENGTH);
    DBG_STR_CASE(SQL_DESC_OCTET_LENGTH);
    DBG_STR_CASE(SQL_DESC_NULLABLE);
    DBG_STR_CASE(SQL_DESC_NUM_PREC_RADIX);
    DBG_STR_CASE(SQL_DESC_PRECISION);
    DBG_STR_CASE(SQL_DESC_SCALE);
    DBG_STR_CASE(SQL_DESC_SEARCHABLE);
    DBG_STR_CASE(SQL_DESC_UNNAMED);
    DBG_STR_CASE(SQL_DESC_UNSIGNED);
    DBG_STR_CASE(SQL_DESC_UPDATABLE);
    DBG_STR_CASE(SQL_COLUMN_LENGTH);
    DBG_STR_CASE(SQL_COLUMN_PRECISION);
    DBG_STR_CASE(SQL_COLUMN_SCALE);
    default:
      break;
  }
  return "<< UNKNOWN ID >>";
}

#undef DBG_STR_CASE

SqlLen Nullability::ToSql(int32_t nullability) {
  switch (nullability) {
    case Nullability::NO_NULL:
      return SQL_NO_NULLS;

    case Nullability::NULLABLE:
      return SQL_NULLABLE;

    case Nullability::NULLABILITY_UNKNOWN:
      return SQL_NULLABLE_UNKNOWN;

    default:
      break;
  }

  assert(false);
  return SQL_NULLABLE_UNKNOWN;
}

// -AL-: the original function called in column_vector
// should define this function to get values from the column
/*
void ColumnMeta::Read(
    ignite::impl::binary::BinaryReaderImpl& reader,
    const ProtocolVersion&
        ver) {  // taking the source and assigning to the variables.
  utility::ReadString(
      reader,
      schemaName);  // class variable, accessed by SQL columns over the fetch
  utility::ReadString(reader, tableName);
  utility::ReadString(reader, columnName);

  dataType = reader.ReadInt8();

  if (ver >= ProtocolVersion::VERSION_2_7_0) {
    precision = reader.ReadInt32();
    scale = reader.ReadInt32();
  }

  if (ver >= ProtocolVersion::VERSION_2_8_0)
    nullability = reader.ReadInt8();
}
*/

// newly added constants -AL-
// todo finish the rest of constants (no need to read for unused constants)
// todo check - how to determine which values to store? Can I go with the previous plan
// and read everything that is in both JDBC and SQLColumns?

// the constants should match JDBC column entry constants
const std::string TABLE_CAT = "TABLE_CAT";
const std::string TABLE_SCHEM = "TABLE_SCHEM";
const std::string TABLE_NAME = "TABLE_NAME";
const std::string COLUMN_NAME = "COLUMN_NAME";
const std::string DATA_TYPE = "DATA_TYPE";
const std::string REMARKS = "REMARKS";
const std::string COLUMN_DEF = "COLUMN_DEF";
const std::string NULLABLE = "NULLABLE";
const std::string ORDINAL_POSITION = "ORDINAL_POSITION";

// -AL- new read function
void ColumnMeta::Read(SharedPointer< ResultSet >& resultSet, int32_t& prevPosition,
                     JniErrorInfo& errInfo) {
    bool wasNull;
    catalogName = "";
    schemaName = "";
    tableName = "";
    columnName = "";
    remarks = "";
    columnDef = "";
    resultSet.Get()->GetString(TABLE_CAT, catalogName, wasNull, errInfo);
    resultSet.Get()->GetString(TABLE_SCHEM, schemaName, wasNull, errInfo);
    resultSet.Get()->GetString(TABLE_NAME, tableName, wasNull, errInfo);
    resultSet.Get()->GetString(COLUMN_NAME, columnName, wasNull, errInfo);
    resultSet.Get()->GetString(REMARKS, remarks, wasNull, errInfo);
    resultSet.Get()->GetString(COLUMN_DEF, columnDef, wasNull, errInfo);
    resultSet.Get()->GetInt(NULLABLE, nullability, wasNull, errInfo);
    resultSet.Get()->GetInt(ORDINAL_POSITION, ordinalPosition, wasNull, errInfo);
    if (wasNull) {
        ordinalPosition = ++prevPosition;
    } else {
        prevPosition = ordinalPosition;
    }
}

bool ColumnMeta::GetAttribute(uint16_t fieldId, std::string& value) const {
  using namespace ignite::impl::binary;

  switch (fieldId) {
    case SQL_DESC_LABEL:
    case SQL_DESC_BASE_COLUMN_NAME:
    case SQL_DESC_NAME: {
      value = columnName;

      return true;
    }

    case SQL_DESC_TABLE_NAME:
    case SQL_DESC_BASE_TABLE_NAME: {
      value = tableName;

      return true;
    }

    case SQL_DESC_SCHEMA_NAME: {
      value = schemaName;

      return true;
    }

    case SQL_DESC_CATALOG_NAME: {
      value.clear();

      return true;
    }

    case SQL_DESC_LITERAL_PREFIX:
    case SQL_DESC_LITERAL_SUFFIX: {
      if (dataType == IGNITE_TYPE_STRING)
        value = "'";
      else
        value.clear();

      return true;
    }

    case SQL_DESC_TYPE_NAME:
    case SQL_DESC_LOCAL_TYPE_NAME: {
      value = type_traits::BinaryTypeToSqlTypeName(dataType);

      return true;
    }

    case SQL_DESC_PRECISION:
    case SQL_COLUMN_LENGTH:
    case SQL_COLUMN_PRECISION: {
      if (precision == -1)
        return false;

      value = common::LexicalCast< std::string >(precision);

      return true;
    }

    case SQL_DESC_SCALE:
    case SQL_COLUMN_SCALE: {
      if (scale == -1)
        return false;

      value = common::LexicalCast< std::string >(scale);

      return true;
    }

    default:
      return false;
  }
}

bool ColumnMeta::GetAttribute(uint16_t fieldId, SqlLen& value) const {
  using namespace ignite::impl::binary;

  switch (fieldId) {
    case SQL_DESC_FIXED_PREC_SCALE: {
      if (scale == -1)
        value = SQL_FALSE;
      else
        value = SQL_TRUE;

      break;
    }

    case SQL_DESC_AUTO_UNIQUE_VALUE: {
      value = SQL_FALSE;

      break;
    }

    case SQL_DESC_CASE_SENSITIVE: {
      if (dataType == IGNITE_TYPE_STRING)
        value = SQL_TRUE;
      else
        value = SQL_FALSE;

      break;
    }

    case SQL_DESC_CONCISE_TYPE:
    case SQL_DESC_TYPE: {
      value = type_traits::BinaryToSqlType(dataType);

      break;
    }

    case SQL_DESC_DISPLAY_SIZE: {
      value = type_traits::BinaryTypeDisplaySize(dataType);

      break;
    }

    case SQL_DESC_LENGTH:
    case SQL_DESC_OCTET_LENGTH:
    case SQL_COLUMN_LENGTH: {
      if (precision == -1)
        value = type_traits::BinaryTypeTransferLength(dataType);
      else
        value = precision;

      break;
    }

    case SQL_DESC_NULLABLE: {
      value = Nullability::ToSql(nullability);

      break;
    }

    case SQL_DESC_NUM_PREC_RADIX: {
      value = type_traits::BinaryTypeNumPrecRadix(dataType);

      break;
    }

    case SQL_DESC_PRECISION:
    case SQL_COLUMN_PRECISION: {
      if (precision == -1)
        value = type_traits::BinaryTypeColumnSize(dataType);
      else
        value = precision;

      break;
    }

    case SQL_DESC_SCALE:
    case SQL_COLUMN_SCALE: {
      if (scale == -1) {
        value = type_traits::BinaryTypeDecimalDigits(dataType);

        if (value < 0)
          value = 0;
      } else
        value = scale;

      break;
    }

    case SQL_DESC_SEARCHABLE: {
      value = SQL_PRED_BASIC;

      break;
    }

    case SQL_DESC_UNNAMED: {
      value = columnName.empty() ? SQL_UNNAMED : SQL_NAMED;

      break;
    }

    case SQL_DESC_UNSIGNED: {
      value = type_traits::BinaryTypeUnsigned(dataType) ? SQL_TRUE : SQL_FALSE;

      break;
    }

    case SQL_DESC_UPDATABLE: {
      value = SQL_ATTR_READWRITE_UNKNOWN;

      break;
    }

    default:
      return false;
  }

  LOG_MSG("value: " << value);

  return true;
}

// where meta is set
// void ReadColumnMetaVector(ignite::impl::binary::BinaryReaderImpl& reader,
// ColumnMetaVector& meta,
//    const ProtocolVersion& ver)
void ReadColumnMetaVector(SharedPointer< ResultSet >& resultSet,
                          ColumnMetaVector& meta) {
  // int32_t metaNum = reader.ReadInt32(); // number of records

  /* // temporarily comment out -AL-
  // get to the last row of resultSet
  resultSet.absolute(-1);
  // get number of rows in the resultSet
  int32_t metaNum = resultSet.getRow();
  // get back to row 1
  resultSet.absolute(1);
  */

  meta.clear();

  if (!resultSet.IsValid()) {
      return;
  }

  JniErrorInfo errInfo;
  bool hasNext = false;
  int32_t prevPosition = 0;
  JniErrorCode errCode;
  do {
      errCode = resultSet.Get()->Next(hasNext, errInfo);
      if (!hasNext || errCode != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
          break;
      }

      meta.emplace_back(ColumnMeta());
      meta.back().Read(resultSet, prevPosition, errInfo);
  } while (hasNext);

  // old code, no longer used
  /*
  // for rows of each column
  for (int32_t i = 0; i < metaNum; ++i) {
    meta.push_back(ColumnMeta());  // create an empty instance

    // meta.back().Read(reader, ver); // reads from the reader, not needed
    // for us. temporarily comment out -AL- get column meta

    // ^ for each row in the result set (represent a column), it will have a
    // column, in each column, there is information need to map column
    // properties into column meta
    //
    //
    // resultSet has many columns, which have their own columns

    // TODO put in the copy from resultSet into meta
  }
  */
}

// original ignite code
/*
void ReadColumnMetaVector(ignite::impl::binary::BinaryReaderImpl&
 reader, ColumnMetaVector& meta,
    const ProtocolVersion& ver) {
    int32_t metaNum = reader.ReadInt32();  // number of records

    meta.clear();
    meta.reserve(static_cast< size_t >(metaNum));

    // for rows of each column
    for (int32_t i = 0; i < metaNum; ++i) {
        meta.push_back(ColumnMeta());  // create an empty instance

        meta.back().Read(
            reader,
            ver);  // reads from the reader, not needed for us.
        // get column meta

        // ^ for each row in the result set (represent a column), it
        // will have a column, in each column, there is information
        // need to map column properties into column meta
        //
        //
        // resultSet has many columns, which have their own columns

        // TODO put in the copy from resultSet into meta
    }

    default:
      return false;
  }
}
*/

/*
void ReadColumnMetaVector(ignite::impl::binary::BinaryReaderImpl& reader,
                          ColumnMetaVector& meta, const ProtocolVersion& ver) {
  int32_t metaNum = reader.ReadInt32();

  meta.clear();
  meta.reserve(static_cast< size_t >(metaNum));

  for (int32_t i = 0; i < metaNum; ++i) {
    meta.push_back(ColumnMeta());

    meta.back().Read(reader, ver);
  }
}
*/
}  // namespace meta
}  // namespace odbc
}  // namespace ignite
