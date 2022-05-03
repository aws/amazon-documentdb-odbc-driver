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
  /** Constructs a default instance */
  JdbcColumnMetadata() = default;

  /**
   * Destructs the JdbcColumnMetadata object.
   */
  ~JdbcColumnMetadata() = default;

  /**
   * Gets the (zero-indexed) ordinal of the column in the table.
   */
  int32_t GetOrdinal() const {
    return ordinal_;
  }

  /**
   * Gets the indicator of whether the column is auto incremented.
   */
  bool IsAutoIncrement() const {
    return autoIncrement_;
  }

  /**
   * Gets the indicator of whether the column is case sensitive.
   */
  bool IsCaseSensitive() const {
    return caseSensitive_;
  }

  /**
   * Gets the indicator of whether the column is searchable.
   */
  bool IsSearchable() const {
    return searchable_;
  }

  /**
   * Gets the indicator of whether the column is a currency.
   */
  bool IsCurrency() const {
    return currency_;
  }

  /**
   * Gets the indicator of whether the column is nullable, non-nullable or unknown-nullable.
   */
  int32_t GetNullable() const {
    return nullable_;
  }

  /**
   * Gets the indicator of whether the column is signed numeric value.
   */
  bool IsSigned() const {
    return signed_;
  }

  /**
   * Gets the display size for the column.
   */
  int32_t GetColumnDisplaySize() const {
    return columnDisplaySize_;
  }

  /**
   * Gets the label for the column.
   */
  boost::optional< std::string > GetColumnLabel() const {
    return columnLabel_;
  }

  /**
   * Gets the name of the column.
   */
  boost::optional< std::string > GetColumnName() const {
    return columnName_;
  }

  /**
   * Gets the schema name (if any) for the column.
   */
  boost::optional< std::string > GetSchemaName() const {
    return schemaName_;
  }

  /**
   * Gets the precision (i.e. length) of the value.
   */
  int32_t GetPrecision() const {
    return precision_;
  }

  /**
   * Gets the scale of the numeric value.
   */
  int32_t GetScale() const {
    return scale_;
  }

  /**
   * Gets the name of the table the column belongs.
   */
  boost::optional< std::string > GetTableName() const {
    return tableName_;
  }

  /**
   * Gets the name of the catalog (if any) the column belongs.
   */
  boost::optional< std::string > GetCatalogName() const {
    return catalogName_;
  }

  /**
   * Gets the data type of the column. See JDBC_TYPE_*
   */
  int32_t GetColumnType() const {
    return columnType_;
  }

  /**
   * Gets the type name of the column.
   */
  boost::optional< std::string > GetColumnTypeName() const {
    return columnTypeName_;
  }

  /**
   * Gets the indicator of whether the column is read only.
   */
  bool IsReadOnly() const {
    return readOnly_;
  }
  
  /**
   * Gets the indicator of whether the column is writable.
   */
  bool IsWritable() const {
    return writable_;
  }

  /**
   * Gets the indicator of whether the column is definitely writable.
   */
  bool IsDefinitelyWritable() const {
    return definitelyWritable_;
  }

  /**
   * Gets the Java class name for the column.
   */
  boost::optional< std::string > GetColumnClassName() const {
    return columnClassName_;
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
      : ordinal_(ordinal),
        autoIncrement_(autoIncrement),
        caseSensitive_(caseSensitive),
        searchable_(searchable),
        currency_(currency),
        nullable_(nullable),
        signed_(isSigned),
        columnDisplaySize_(columnDisplaySize),
        columnLabel_(columnLabel),
        columnName_(columnName),
        schemaName_(schemaName),
        precision_(precision),
        scale_(scale),
        tableName_(tableName),
        catalogName_(catalogName),
        columnType_(columnType),
        columnTypeName_(columnTypeName),
        readOnly_(readOnly),
        writable_(writable),
        definitelyWritable_(definitelyWritable),
        columnClassName_(columnClassName) {
      // No-op
  }

  int32_t ordinal_;
  bool autoIncrement_;
  bool caseSensitive_;
  bool searchable_;
  bool currency_;
  int32_t nullable_;
  bool signed_;
  int32_t columnDisplaySize_;
  boost::optional< std::string > columnLabel_;
  boost::optional< std::string > columnName_;
  boost::optional< std::string > schemaName_;
  int32_t precision_;
  int32_t scale_;
  boost::optional< std::string > tableName_;
  boost::optional< std::string > catalogName_;
  int32_t columnType_;
  boost::optional< std::string > columnTypeName_;
  bool readOnly_;
  bool writable_;
  bool definitelyWritable_;
  boost::optional< std::string > columnClassName_;
};
}  // namespace jni
}  // namespace odbc
}  // namespace ignite

#endif  // _IGNITE_ODBC_JNI_JDBC_COLUMN_METADATA
