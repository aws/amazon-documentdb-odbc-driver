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

#include "ignite/odbc/meta/column_meta.h"

#include "ignite/odbc/common/utils.h"
#include "ignite/odbc/common_types.h"
#include "ignite/odbc/impl/binary/binary_common.h"
#include "ignite/odbc/jni/java.h"
#include "ignite/odbc/log.h"
#include "ignite/odbc/system/odbc_constants.h"
#include "ignite/odbc/type_traits.h"

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

SqlLen Nullability::ToSql(boost::optional< int32_t > nullability) {
  if (!nullability) {
    assert(false);
    return SQL_NULLABLE_UNKNOWN;
  }
  switch (*nullability) {
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

const std::string TABLE_CAT = "TABLE_CAT";
const std::string TABLE_SCHEM = "TABLE_SCHEM";
const std::string TABLE_NAME = "TABLE_NAME";
const std::string COLUMN_NAME = "COLUMN_NAME";
const std::string DATA_TYPE = "DATA_TYPE";
const std::string DECIMAL_DIGITS = "DECIMAL_DIGITS";
const std::string REMARKS = "REMARKS";
const std::string COLUMN_DEF = "COLUMN_DEF";
const std::string NULLABLE = "NULLABLE";
const std::string ORDINAL_POSITION = "ORDINAL_POSITION";
const std::string IS_AUTOINCREMENT = "IS_AUTOINCREMENT";

void ColumnMeta::Read(SharedPointer< ResultSet >& resultSet,
                      int32_t& prevPosition, JniErrorInfo& errInfo) {
  resultSet.Get()->GetString(TABLE_CAT, catalogName, errInfo);
  resultSet.Get()->GetString(TABLE_SCHEM, schemaName, errInfo);
  resultSet.Get()->GetString(TABLE_NAME, tableName, errInfo);
  resultSet.Get()->GetString(COLUMN_NAME, columnName, errInfo);
  resultSet.Get()->GetSmallInt(DATA_TYPE, dataType, errInfo);
  resultSet.Get()->GetInt(DECIMAL_DIGITS, decimalDigits, errInfo);
  resultSet.Get()->GetString(REMARKS, remarks, errInfo);
  resultSet.Get()->GetString(COLUMN_DEF, columnDef, errInfo);
  resultSet.Get()->GetInt(NULLABLE, nullability, errInfo);
  resultSet.Get()->GetInt(ORDINAL_POSITION, ordinalPosition, errInfo);
  if (!ordinalPosition) {
    ordinalPosition = ++prevPosition;
  } else {
    prevPosition = *ordinalPosition;
  }
  resultSet.Get()->GetString(IS_AUTOINCREMENT, isAutoIncrement, errInfo);
}

void ColumnMeta::ReadJdbcMetadata(JdbcColumnMetadata& jdbcMetadata,
                                  int32_t& prevPosition) {
  catalogName = jdbcMetadata.GetCatalogName();
  schemaName = jdbcMetadata.GetSchemaName();
  tableName = jdbcMetadata.GetTableName();
  columnName = jdbcMetadata.GetColumnName();
  dataType = jdbcMetadata.GetColumnType();
  precision = jdbcMetadata.GetPrecision();
  scale = jdbcMetadata.GetScale();
  nullability = jdbcMetadata.GetNullable();
  ordinalPosition = jdbcMetadata.GetOrdinal();
  if (!ordinalPosition) {
    ordinalPosition = ++prevPosition;
  } else {
    prevPosition = *ordinalPosition;
  }
  isAutoIncrement = jdbcMetadata.IsAutoIncrement() ? "YES" : "NO";
}

bool ColumnMeta::GetAttribute(uint16_t fieldId, std::string& value) const {
  using namespace ignite::odbc::impl::binary;

  // an empty string is returned if the column does not have the requested field
  value = "";

  switch (fieldId) {
    case SQL_DESC_LABEL:
    case SQL_DESC_BASE_COLUMN_NAME:
    case SQL_DESC_NAME: {
      if (columnName)
        value = *columnName;

      return true;
    }

    case SQL_DESC_TABLE_NAME:
    case SQL_DESC_BASE_TABLE_NAME: {
      if (tableName)
        value = *tableName;

      return true;
    }

    case SQL_DESC_SCHEMA_NAME: {
      if (schemaName)
        value = *schemaName;

      return true;
    }

    case SQL_DESC_CATALOG_NAME: {
      value.clear();

      return true;
    }

    case SQL_DESC_LITERAL_PREFIX: {
      if (dataType) {
        if ((*dataType == JDBC_TYPE_VARCHAR) || (*dataType == JDBC_TYPE_CHAR)
            || (*dataType == JDBC_TYPE_NCHAR)
            || (*dataType == JDBC_TYPE_NVARCHAR)
            || (*dataType == JDBC_TYPE_LONGVARCHAR)
            || (*dataType == JDBC_TYPE_LONGNVARCHAR))
          value = "'";
        else if ((*dataType == JDBC_TYPE_BINARY)
                 || (*dataType == JDBC_TYPE_VARBINARY)
                 || (*dataType == JDBC_TYPE_LONGVARBINARY))
          value = "0x";
      } else
        value.clear();

      return true;
    }

    case SQL_DESC_LITERAL_SUFFIX: {
      if (dataType && (*dataType == JDBC_TYPE_VARCHAR)
          || (*dataType == JDBC_TYPE_CHAR) || (*dataType == JDBC_TYPE_NCHAR)
          || (*dataType == JDBC_TYPE_NVARCHAR)
          || (*dataType == JDBC_TYPE_LONGVARCHAR)
          || (*dataType == JDBC_TYPE_LONGNVARCHAR))
        value = "'";
      else
        value.clear();

      return true;
    }

    case SQL_DESC_TYPE_NAME:
    case SQL_DESC_LOCAL_TYPE_NAME: {
      if (boost::optional< std::string > val =
              type_traits::BinaryTypeToSqlTypeName(dataType))
        value = *val;
      else
        value.clear();

      return true;
    }

    case SQL_DESC_PRECISION:
    case SQL_COLUMN_LENGTH:
    case SQL_COLUMN_PRECISION: {
      if (!precision || *precision == -1)
        return false;

      value = common::LexicalCast< std::string >(*precision);

      return true;
    }

    case SQL_DESC_SCALE:
    case SQL_COLUMN_SCALE: {
      if (!scale || *scale == -1)
        return false;

      value = common::LexicalCast< std::string >(*scale);

      return true;
    }

    default:
      return false;
  }
}

bool ColumnMeta::GetAttribute(uint16_t fieldId, SqlLen& value) const {
  using namespace ignite::odbc::impl::binary;

  // value equals -1 by default.
  value = -1;
  switch (fieldId) {
    case SQL_DESC_FIXED_PREC_SCALE: {
      if ((!precision || *precision == -1)
          || (!scale || *scale == -1 || *scale == 0))
        value = SQL_FALSE;
      else
        value = SQL_TRUE;

      break;
    }

    case SQL_DESC_AUTO_UNIQUE_VALUE: {
      if (isAutoIncrement && (*isAutoIncrement == "YES"))
        value = SQL_TRUE;
      else
        value = SQL_FALSE;

      break;
    }

    case SQL_DESC_CASE_SENSITIVE: {
      if (dataType
          && ((*dataType == JDBC_TYPE_VARCHAR) || (*dataType == JDBC_TYPE_CHAR)
              || (*dataType == JDBC_TYPE_NCHAR)
              || (*dataType == JDBC_TYPE_NVARCHAR)
              || (*dataType == JDBC_TYPE_LONGVARCHAR)
              || (*dataType == JDBC_TYPE_LONGNVARCHAR)))
        value = SQL_TRUE;
      else
        value = SQL_FALSE;

      break;
    }

    case SQL_DESC_CONCISE_TYPE:
    case SQL_DESC_TYPE: {
      if (boost::optional< int16_t > val =
              type_traits::BinaryToSqlType(dataType))
        value = *val;

      break;
    }

    case SQL_DESC_DISPLAY_SIZE: {
      if (boost::optional< int > val =
              type_traits::BinaryTypeDisplaySize(dataType))
        value = *val;

      break;
    }

    case SQL_DESC_LENGTH:
      // SQL_DESC_LENGTH is either the maximum or actual character length of a
      // character string or binary data type
    case SQL_COLUMN_LENGTH: {
      if (dataType) {
        if (boost::optional< int > val =
                type_traits::BinaryTypeTransferLength(dataType))
          value = *val;
      }

      break;
    }

    case SQL_DESC_OCTET_LENGTH: {
      // SQL_DESC_OCTET_LENGTH is SQL_DESC_LENGTH in bytes
      if (dataType) {
        if (boost::optional< int > val =
                type_traits::BinaryTypeTransferLength(dataType)) {
          // multiply SQL_DESC_LENGTH by bytes per char if needed
          if (*val != SQL_NO_TOTAL)
            *val *= sizeof(char);
          value = *val;
        }
      }

      break;
    }

    case SQL_DESC_NULLABLE: {
      value = Nullability::ToSql(nullability);

      break;
    }

    case SQL_DESC_NUM_PREC_RADIX: {
      if (boost::optional< int > val =
              type_traits::BinaryTypeNumPrecRadix(dataType))
        value = *val;

      break;
    }

    case SQL_DESC_PRECISION:
    case SQL_COLUMN_PRECISION: {
      if (dataType) {
        if (decimalDigits
            && ((*dataType == JDBC_TYPE_TIME) || (*dataType == JDBC_TYPE_DATE)
                || (*dataType == JDBC_TYPE_TIMESTAMP))) {
          // return decimal digits for all datetime types and all interval
          // types with a seconds component
          value = *decimalDigits;
        } else if (!precision || *precision == -1) {
          if (boost::optional< int > val =
                  type_traits::BinaryTypeColumnSize(dataType))
            value = *val;
        }
      } else if (precision)
        value = *precision;

      break;
    }

    case SQL_DESC_SCALE:
    case SQL_COLUMN_SCALE: {
      // scale value of -1 means value not availabe.
      if (dataType
          && (decimalDigits
              && ((*dataType == JDBC_TYPE_DECIMAL)
                  || (*dataType == JDBC_TYPE_NUMERIC)))) {
        // return decimal digits for all decimal and numeric types
        value = *decimalDigits;
      } else if (dataType && (!scale || *scale == -1)) {
        if (boost::optional< int16_t > val =
                type_traits::BinaryTypeDecimalDigits(dataType))
          value = *val;
      } else if (scale)
        value = *scale;

      break;
    }

    case SQL_DESC_SEARCHABLE: {
      value = SQL_PRED_BASIC;

      break;
    }

    case SQL_DESC_UNNAMED: {
      value = columnName ? SQL_NAMED : SQL_UNNAMED;

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

void ReadColumnMetaVector(SharedPointer< ResultSet >& resultSet,
                          ColumnMetaVector& meta) {
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
}

}  // namespace meta
}  // namespace odbc
}  // namespace ignite
