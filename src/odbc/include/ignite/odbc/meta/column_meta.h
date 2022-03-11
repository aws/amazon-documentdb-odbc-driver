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

#ifndef _IGNITE_ODBC_META_COLUMN_META
#define _IGNITE_ODBC_META_COLUMN_META

#include <stdint.h>

#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <string>

#include "ignite/odbc/impl/binary/binary_reader_impl.h"
#include "ignite/odbc/common_types.h"
#include "ignite/odbc/jni/result_set.h"
#include "ignite/odbc/protocol_version.h"
#include "ignite/odbc/utility.h"

using ignite::odbc::jni::ResultSet;
using ignite::odbc::jni::java::JniErrorInfo;

namespace ignite {
namespace odbc {
namespace meta {
/**
 * Nullability type.
 */
struct Nullability {
  enum Type {
    NO_NULL = 0,

    NULLABLE = 1,

    NULLABILITY_UNKNOWN = 2
  };

  /**
   * Convert to SQL constant.
   *
   * @param nullability Nullability.
   * @return SQL constant.
   */
  static SqlLen ToSql(boost::optional< int32_t > nullability);
};

using namespace ignite::odbc;

/**
 * Column metadata.
 */
class ColumnMeta {
 public:
  /**
   * Convert attribute ID to string containing its name.
   * Debug function.
   * @param type Attribute ID.
   * @return Null-terminated string containing attribute name.
   */
  static const char* AttrIdToString(uint16_t id);

  /**
   * Default constructor.
   */
  ColumnMeta()
      : dataType(), nullability(), precision(), scale(), ordinalPosition() {
    // No-op.
  }

  /**
   * Constructor.
   *
   * @param schemaName Schema name.
   * @param tableName Table name.
   * @param columnName Column name.
   * @param typeName Type name.
   * @param dataType Data type.
   */
  ColumnMeta(const std::string& schemaName, const std::string& tableName,
             const std::string& columnName, int16_t dataType,
             Nullability::Type nullability)
      : schemaName(schemaName),
        tableName(tableName),
        columnName(columnName),
        dataType(dataType),
        precision(-1),
        scale(-1),
        nullability(nullability),
        ordinalPosition(-1) {
    // No-op.
  }

  /**
   * Destructor.
   */
  ~ColumnMeta() {
    // No-op.
  }

  /**
   * Copy constructor.
   */
  ColumnMeta(const ColumnMeta& other)
      : catalogName(other.catalogName),
        schemaName(other.schemaName),
        tableName(other.tableName),
        columnName(other.columnName),
        remarks(other.remarks),
        columnDef(other.columnDef),
        dataType(other.dataType),
        precision(other.precision),
        scale(other.scale),
        nullability(other.nullability),
        ordinalPosition(other.ordinalPosition) {
    // No-op.
  }

  /**
   * Copy operator.
   */
  ColumnMeta& operator=(const ColumnMeta& other) {
    catalogName = other.catalogName;
    schemaName = other.schemaName;
    tableName = other.tableName;
    columnName = other.columnName;
    remarks = other.remarks;
    columnDef = other.columnDef;
    dataType = other.dataType;
    precision = other.precision;
    scale = other.scale;
    nullability = other.nullability;
    ordinalPosition = other.ordinalPosition;

    return *this;
  }

  /**
   * Read using reader.
   * @param resultSet SharedPointer< ResultSet >.
   * @param prevPosition the ordinal position of the previous column.
   * @paran errInfo JniErrorInfo.
   */
  void Read(SharedPointer< ResultSet >& resultSet, int32_t& prevPosition,
            JniErrorInfo& errInfo);

  /**
   * Get catalog name.
   * @return Catalog name.
   */
  const boost::optional< std::string >& GetCatalogName() const {
    return catalogName;
  }

  /**
   * Get schema name.
   * @return Schema name.
   */
  const boost::optional< std::string >& GetSchemaName() const {
    return schemaName;
  }

  /**
   * Get table name.
   * @return Table name.
   */
  const boost::optional< std::string >& GetTableName() const {
    return tableName;
  }

  /**
   * Get column name.
   * @return Column name.
   */
  const boost::optional< std::string >& GetColumnName() const {
    return columnName;
  }

  /**
   * Get the remarks.
   * @return Remarks.
   */
  const boost::optional< std::string >& GetRemarks() const {
    return remarks;
  }

  /**
   * Get the column default value.
   * @return Column default value.
   */
  const boost::optional< std::string >& GetColumnDef() const {
    return columnDef;
  }

  /**
   * Get data type.
   * @return Data type.
   */
  boost::optional< int16_t > GetDataType() const {
    return dataType;
  }

  /**
   * Get column precision.
   * @return Column precision.
   */
  boost::optional< int32_t > GetPrecision() const {
    return precision;
  }

  /**
   * Get column scale.
   * @return Column scale.
   */
  boost::optional< int32_t > GetScale() const {
    return scale;
  }

  /**
   * Get column nullability.
   * @return Column nullability.
   */
  boost::optional< int32_t > GetNullability() const {
    return nullability;
  }

  /**
   * Get column ordinal position.
   * @return Column ordinal position.
   */
  boost::optional< int32_t > GetOrdinalPosition() const {
    return ordinalPosition;
  }

  /**
   * Try to get attribute of a string type.
   *
   * @param fieldId Field ID.
   * @param value Output attribute value.
   * @return True if the attribute supported and false otherwise.
   */
  bool GetAttribute(uint16_t fieldId, std::string& value) const;

  /**
   * Try to get attribute of a integer type.
   *
   * @param fieldId Field ID.
   * @param value Output attribute value.
   * @return True if the attribute supported and false otherwise.
   */
  bool GetAttribute(uint16_t fieldId, SqlLen& value) const;

 private:
  /** Catalog name. */
  boost::optional< std::string > catalogName;

  /** Schema name. */
  boost::optional< std::string > schemaName;

  /** Table name. */
  boost::optional< std::string > tableName;

  /** Column name. */
  boost::optional< std::string > columnName;

  /** Remarks */
  boost::optional< std::string > remarks;

  /** Column default value */
  boost::optional< std::string > columnDef;

  /** Data type. */
  boost::optional< int16_t > dataType;

  /** Column precision. */
  boost::optional< int32_t > precision;

  /** Column scale. */
  boost::optional< int32_t > scale;

  /** Column nullability. */
  boost::optional< int32_t > nullability;

  /** Column ordinal position. */
  boost::optional< int32_t > ordinalPosition;
};

/** Column metadata vector alias. */
typedef std::vector< ColumnMeta > ColumnMetaVector;

/**
 * Read columns metadata collection.
 * @param resultSet SharedPointer< ResultSet >.
 * @param meta Collection.
 */
void ReadColumnMetaVector(SharedPointer< ResultSet >& resultSet,
                          ColumnMetaVector& meta);

}  // namespace meta
}  // namespace odbc
}  // namespace ignite
#endif  //_IGNITE_ODBC_META_COLUMN_META
