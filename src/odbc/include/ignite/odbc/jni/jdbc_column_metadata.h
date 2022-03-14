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
 * Wrapper for the the JDBC DatabaseMetaData.
 */
class JdbcColumnMetadata {
  friend class DocumentDbConnection;
  friend class DocumentDbQueryMappingService;

 public:
  ~JdbcColumnMetadata() = default;

  int32_t GetOrdinal() {
    return _ordinal;
  }

  bool IsAutoIncrement() {
    return _autoIncrement;
  }

  bool IsCaseSensitive() {
    return _caseSensitive;
  }

  bool IsSearchable() {
    return _searchable;
  }

  bool IsCurrency() {
    return _currency;
  }

  int32_t GetNullable() {
    return _nullable;
  }

  bool IsSigned() {
    return _signed;
  }

  int32_t GetColumnDisplaySize() {
    return _columnDisplaySize;
  }

  boost::optional< std::string > GetColumnLabel() {
    return _columnLabel;
  }

  boost::optional< std::string > GetColumnName() {
    return _columnName;
  }

  boost::optional< std::string > GetSchemaName() {
    return _schemaName;
  }

  int32_t GetPrecision() {
    return _precision;
  }

  int32_t GetScale() {
    return _scale;
  }

  boost::optional< std::string > GetTableName() {
    return _tableName;
  }

  boost::optional< std::string > GetCatalogName() {
    return _catalogName;
  }

  int32_t GetColumnType() {
    return _columnType;
  }

  boost::optional< std::string > GetColumnTypeName() {
    return _columnTypeName;
  }

  bool IsReadOnly() {
    return _readOnly;
  }
  
  bool IsWritable() {
    return _writable;
  }

  bool IsDefinitelyWritable() {
    return _definitelyWritable;
  }

  boost::optional< std::string > GetColumnClassName() {
    return _columnClassName;
  }


 private:
  /**
   * Constructs an instance of the DatabaseMetaData class.
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

  int32_t _ordinal = 0;
  bool _autoIncrement = false;
  bool _caseSensitive = false;
  bool _searchable = false;
  bool _currency = false;
  int32_t _nullable = 0;
  bool _signed = false;
  int32_t _columnDisplaySize = 0;
  boost::optional< std::string > _columnLabel{};
  boost::optional< std::string > _columnName{};
  boost::optional< std::string > _schemaName{};
  int32_t _precision = 0;
  int32_t _scale = 0;
  boost::optional< std::string > _tableName{};
  boost::optional< std::string > _catalogName{};
  int32_t _columnType = 0;
  boost::optional< std::string > _columnTypeName{};
  bool _readOnly = false;
  bool _writable = false;
  bool _definitelyWritable = false;
  boost::optional< std::string > _columnClassName{};
};
}  // namespace jni
}  // namespace odbc
}  // namespace ignite

#endif  // _IGNITE_ODBC_JNI_JDBC_COLUMN_METADATA
