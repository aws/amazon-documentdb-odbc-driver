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

#include <ignite/odbc/common/concurrent.h>
#include <ignite/odbc/jni/java.h>
#include <ignite/odbc/jni/result_set.h>
#include <boost/optional.hpp>

#include <string>
#include <list>
#include <map>

#ifndef _IGNITE_ODBC_JNI_JDBC_COLUMN_METADATA
#define _IGNITE_ODBC_JNI_JDBC_COLUMN_METADATA

using ignite::odbc::common::concurrent::SharedPointer;
using ignite::odbc::jni::ResultSet;
using ignite::odbc::jni::java::GlobalJObject;
using ignite::odbc::jni::java::JniContext;
using ignite::odbc::jni::java::JniErrorInfo;

namespace ignite {
namespace odbc {
namespace jni {
/**
 * Wrapper for the the JDBC column metadata.
 */
class JdbcColumnMetadata {
  friend class DocumentDbConnection;
  friend class DocumentDbQueryMappingService;

 public:
  /**
   * Destructs the JdbcColumnMetadata object.
   */
  ~JdbcColumnMetadata() = default;

  /**
   * Gets the (zero-indexed) ordinal of the column in the table.
   */
  int32_t GetOrdinal() {
    return _ordinal;
  }

  /**
   * Gets the indicator of whether the column is auto incremented.
   */
  bool IsAutoIncrement() {
    return _autoIncrement;
  }

  /**
   * Gets the indicator of whether the column is case sensitive.
   */
  bool IsCaseSensitive() {
    return _caseSensitive;
  }

  /**
   * Gets the indicator of whether the column is searchable.
   */
  bool IsSearchable() {
    return _searchable;
  }

  /**
   * Gets the indicator of whether the column is a currency.
   */
  bool IsCurrency() {
    return _currency;
  }

  /**
   * Gets the indicator of whether the column is nullable, non-nullable or unknown-nullable.
   */
  int32_t GetNullable() {
    return _nullable;
  }

  /**
   * Gets the indicator of whether the column is signed numeric value.
   */
  bool IsSigned() {
    return _signed;
  }

  /**
   * Gets the display size for the column.
   */
  int32_t GetColumnDisplaySize() {
    return _columnDisplaySize;
  }

  /**
   * Gets the label for the column.
   */
  boost::optional< std::string > GetColumnLabel() {
    return _columnLabel;
  }

  /**
   * Gets the name of the column.
   */
  boost::optional< std::string > GetColumnName() {
    return _columnName;
  }

  /**
   * Gets the schema name (if any) for the column.
   */
  boost::optional< std::string > GetSchemaName() {
    return _schemaName;
  }

  /**
   * Gets the precision (i.e. length) of the value.
   */
  int32_t GetPrecision() {
    return _precision;
  }

  /**
   * Gets the scale of the numeric value.
   */
  int32_t GetScale() {
    return _scale;
  }

  /**
   * Gets the name of the table the column belongs.
   */
  boost::optional< std::string > GetTableName() {
    return _tableName;
  }

  /**
   * Gets the name of the catalog (if any) the column belongs.
   */
  boost::optional< std::string > GetCatalogName() {
    return _catalogName;
  }

  /**
   * Gets the data type of the column. See JDBC_TYPE_*
   */
  int32_t GetColumnType() {
    return _columnType;
  }

  /**
   * Gets the type name of the column.
   */
  boost::optional< std::string > GetColumnTypeName() {
    return _columnTypeName;
  }

  /**
   * Gets the indicator of whether the column is read only.
   */
  bool IsReadOnly() {
    return _readOnly;
  }
  
  /**
   * Gets the indicator of whether the column is writable.
   */
  bool IsWritable() {
    return _writable;
  }

  /**
   * Gets the indicator of whether the column is definitely writable.
   */
  bool IsDefinitelyWritable() {
    return _definitelyWritable;
  }

  /**
   * Gets the Java class name for the column.
   */
  boost::optional< std::string > GetColumnClassName() {
    return _columnClassName;
  }


 private:
  /**
   * Constructs an instance of the JdbcColumnMetadata class.
   */
  JdbcColumnMetadata(int32_t ordinal, bool autoIncrement, bool caseSensitive,
                     bool searchable, bool currency, int32_t nullable,
                     bool isSigned, int32_t columnDisplaySize,
                     boost::optional< std::string > columnLabel,
                     boost::optional< std::string > columnName,
                     boost::optional< std::string > schemaName,
                     int32_t precision, int32_t scale,
                     boost::optional< std::string > tableName,
                     boost::optional< std::string > catalogName,
                     int32_t columnType,
                     boost::optional< std::string > columnTypeName,
                     bool readOnly, bool writable, bool definitelyWritable,
                     boost::optional< std::string > columnClassName)
      : _ordinal(ordinal),
        _autoIncrement(autoIncrement),
        _caseSensitive(caseSensitive),
        _searchable(searchable),
        _currency(currency),
        _nullable(nullable),
        _signed(isSigned),
        _columnDisplaySize(columnDisplaySize),
        _columnLabel(columnLabel),
        _columnName(columnName),
        _schemaName(schemaName),
        _precision(precision),
        _scale(scale),
        _tableName(tableName),
        _catalogName(catalogName),
        _columnType(columnType),
        _columnTypeName(columnTypeName),
        _readOnly(readOnly),
        _writable(writable),
        _definitelyWritable(definitelyWritable),
        _columnClassName(columnClassName) {
      // No-op
  }

  int32_t _ordinal;
  bool _autoIncrement;
  bool _caseSensitive;
  bool _searchable;
  bool _currency;
  int32_t _nullable;
  bool _signed;
  int32_t _columnDisplaySize;
  boost::optional< std::string > _columnLabel;
  boost::optional< std::string > _columnName;
  boost::optional< std::string > _schemaName;
  int32_t _precision;
  int32_t _scale;
  boost::optional< std::string > _tableName;
  boost::optional< std::string > _catalogName;
  int32_t _columnType;
  boost::optional< std::string > _columnTypeName;
  bool _readOnly;
  bool _writable;
  bool _definitelyWritable;
  boost::optional< std::string > _columnClassName;
};
}  // namespace jni
}  // namespace odbc
}  // namespace ignite

#endif  // _IGNITE_ODBC_JNI_JDBC_COLUMN_METADATA
