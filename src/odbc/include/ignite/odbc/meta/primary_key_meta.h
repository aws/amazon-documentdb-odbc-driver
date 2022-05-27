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

#ifndef _IGNITE_ODBC_META_PRIMARY_KEY_META
#define _IGNITE_ODBC_META_PRIMARY_KEY_META

#include <stdint.h>

#include <string>

#include "ignite/odbc/impl/binary/binary_reader_impl.h"
#include "ignite/odbc/jni/result_set.h"
#include "ignite/odbc/log.h"
#include "ignite/odbc/utility.h"

using ignite::odbc::jni::ResultSet;

namespace ignite {
namespace odbc {
namespace meta {
/**
 * Primary key metadata.
 */
class PrimaryKeyMeta {
 public:
  /**
   * Default constructor.
   */
  PrimaryKeyMeta() : keySeq(0) {
    // No-op.
  }

  /**
   * Constructor.
   *
   * @param catalog Catalog name.
   * @param schema Schema name.
   * @param table Table name.
   * @param column Column name.
   * @param keySeq Column sequence number in key (starting with 1).
   * @param keyName Key name.
   */
  PrimaryKeyMeta(const std::string& catalog, const std::string& schema,
                 const std::string& table, const std::string& column,
                 int16_t keySeq, const std::string& keyName)
      : catalog(catalog),
        schema(schema),
        table(table),
        column(column),
        keySeq(keySeq),
        keyName(keyName) {
    // No-op.
  }

  /**
   * Destructor.
   */
  ~PrimaryKeyMeta() {
    // No-op.
  }

  /**
   * Copy constructor.
   */
  PrimaryKeyMeta(const PrimaryKeyMeta& other)
      : catalog(other.catalog),
        schema(other.schema),
        table(other.table),
        column(other.column),
        keySeq(other.keySeq),
        keyName(other.keyName) {
    // No-op.
  }

  /**
   * Copy operator.
   */
  PrimaryKeyMeta& operator=(const PrimaryKeyMeta& other) {
    catalog = other.catalog;
    schema = other.schema;
    table = other.table;
    column = other.column;
    keySeq = other.keySeq;
    keyName = other.keyName;

    return *this;
  }

  /**
   * Read resultset item.
   * @param resultSet SharedPointer< ResultSet >.
   * @paran errInfo JniErrorInfo.
   */
  void Read(SharedPointer< ResultSet >& resultSet, JniErrorInfo& errInfo);

  /**
   * Get catalog name.
   * @return Catalog name.
   */
  const boost::optional< std::string >& GetCatalogName() const {
    return catalog;
  }

  /**
   * Get schema name.
   * @return Schema name.
   */
  const boost::optional< std::string >& GetSchemaName() const {
    return schema;
  }

  /**
   * Get table name.
   * @return Table name.
   */
  const boost::optional< std::string >& GetTableName() const {
    return table;
  }

  /**
   * Get column name.
   * @return Column name.
   */
  const boost::optional< std::string >& GetColumnName() const {
    return column;
  }

  /**
   * Get column sequence number in key.
   * @return Sequence number in key.
   */
  boost::optional< int16_t > GetKeySeq() const {
    return keySeq;
  }

  /**
   * Get key name.
   * @return Key name.
   */
  const boost::optional< std::string >& GetKeyName() const {
    return keyName;
  }

 private:
  /** Catalog name. */
  boost::optional< std::string > catalog;

  /** Schema name. */
  boost::optional< std::string > schema;

  /** Table name. */
  boost::optional< std::string > table;

  /** Column name. */
  boost::optional< std::string > column;

  /** Column sequence number in key. */
  boost::optional< int16_t > keySeq;

  /** Key name. */
  boost::optional< std::string > keyName;
};

/** Table metadata vector alias. */
typedef std::vector< PrimaryKeyMeta > PrimaryKeyMetaVector;

/**
 * Read primary keys metadata collection.
 * @param resultSet SharedPointer< ResultSet >.
 * @param meta Collection.
 */
void ReadPrimaryKeysColumnMetaVector(SharedPointer< ResultSet >& resultSet,
                                     PrimaryKeyMetaVector& meta);
}  // namespace meta
}  // namespace odbc
}  // namespace ignite

#endif  //_IGNITE_ODBC_META_PRIMARY_KEY_META
