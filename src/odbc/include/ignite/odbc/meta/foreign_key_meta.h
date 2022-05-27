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

#ifndef _IGNITE_ODBC_META_FOREIGN_KEY_META
#define _IGNITE_ODBC_META_FOREIGN_KEY_META

#include <stdint.h>

#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <string>

#include "ignite/odbc/impl/binary/binary_reader_impl.h"
#include "ignite/odbc/utility.h"
#include "ignite/odbc/jni/result_set.h"

using ignite::odbc::jni::ResultSet;

namespace ignite {
namespace odbc {
namespace meta {
/**
 * Foreign key metadata.
 */
class ForeignKeyMeta {
 public:
  /**
   * Default constructor.
   */
  ForeignKeyMeta() : keySeq(0) {
    // No-op.
  }

  /**
   * Constructor.
   *
   * @param PKCatalogName Primary key table catalog name
   * @param PKSchemaName Primary key table schema name.
   * @param PKTableName Primary key table name.
   * @param PKColumnName Primary key column name.
   * @param FKCatalogName Foreign key table catalog name
   * @param FKSchemaName Foreign key table schema name.
   * @param FKTableName Foreign key table name.
   * @param FKColumnName Foreign key column name.
   * @param keySeq Column sequence number in key (starting with 1).
   * @param updateRule Update rule.
   * @param deleteRule Delete rule.
   * @param PKName Primary key name.
   * @param FKName Foreign key name.
   * @param deferrability Deferrability.
   */
  ForeignKeyMeta(
      const std::string& PKCatalogName, const std::string& PKSchemaName,
      const std::string& PKTableName, const std::string& PKColumnName,
      const std::string& FKCatalogName, const std::string& FKSchemaName,
      const std::string& FKTableName, const std::string& FKColumnName,
      int16_t keySeq, int16_t updateRule, int16_t deleteRule,
      const std::string& PKName, const std::string& FKName,
      int16_t deferrability)
      : PKCatalogName(PKCatalogName),
        PKSchemaName(PKSchemaName),
        PKTableName(PKTableName),
        PKColumnName(PKColumnName),
        FKCatalogName(FKCatalogName),
        FKSchemaName(FKSchemaName),
        FKTableName(FKTableName),
        FKColumnName(FKColumnName),
        keySeq(keySeq),
        updateRule(updateRule),
        deleteRule(deleteRule),
        PKName(PKName),
        FKName(FKName),
        deferrability(deferrability) {
    // No-op.
  }

  /**
   * Destructor.
   */
  ~ForeignKeyMeta() {
    // No-op.
  }

  /**
   * Copy constructor.
   */
  ForeignKeyMeta(const ForeignKeyMeta& other)
      : PKCatalogName(other.PKCatalogName),
        PKSchemaName(other.PKSchemaName),
        PKTableName(other.PKTableName),
        PKColumnName(other.PKColumnName),
        FKCatalogName(other.FKCatalogName),
        FKSchemaName(other.FKSchemaName),
        FKTableName(other.FKTableName),
        FKColumnName(other.FKColumnName),
        keySeq(other.keySeq),
        updateRule(other.updateRule),
        deleteRule(other.deleteRule),
        PKName(other.PKName),
        FKName(other.FKName),
        deferrability(other.deferrability) {
    // No-op.
  }

  /**
   * Copy operator.
   */
  ForeignKeyMeta& operator=(const ForeignKeyMeta& other) {
    PKCatalogName = other.PKCatalogName;
    PKSchemaName = other.PKSchemaName;
    PKTableName = other.PKTableName;
    PKColumnName = other.PKColumnName;
    FKCatalogName = other.FKCatalogName;
    FKSchemaName = other.FKSchemaName;
    FKTableName = other.FKTableName;
    FKColumnName = other.FKColumnName;
    keySeq = other.keySeq;
    updateRule = other.updateRule;
    deleteRule = other.deleteRule;
    PKName = other.PKName;
    FKName = other.FKName;
    deferrability = other.deferrability;

    return *this;
  }

  /**
   * Read resultset item.
   * @param resultSet SharedPointer< ResultSet >.
   * @paran errInfo JniErrorInfo.
   */
  void Read(SharedPointer< ResultSet >& resultSet, JniErrorInfo& errInfo);

  /**
   * Get primary key table catalog name.
   * @return Primary key table catalog name.
   */
  const boost::optional< std::string >& GetPKCatalogName() const {
    return PKCatalogName;
  }

  /**
   * Get primary key table schema name.
   * @return Primary key table schema name.
   */
  const boost::optional< std::string >& GetPKSchemaName() const {
    return PKSchemaName;
  }

  /**
   * Get primary key table name.
   * @return Primary key table name.
   */
  const boost::optional< std::string >& GetPKTableName() const {
    return PKTableName;
  }

  /**
   * Get foreign key column name.
   * @return Primary key column name.
   */
  const boost::optional< std::string >& GetPKColumnName() const {
    return PKColumnName;
  }

  /**
   * Get foreign key table catalog name.
   * @return Foreign key table catalog name.
   */
  const boost::optional< std::string >& GetFKCatalogName() const {
    return FKCatalogName;
  }

  /**
   * Get foreign key table schema name.
   * @return Foreign key table schema name.
   */
  const boost::optional< std::string >& GetFKSchemaName() const {
    return FKSchemaName;
  }

  /**
   * Get foreign key table name.
   * @return Foreign key table name.
   */
  const boost::optional< std::string >& GetFKTableName() const {
    return FKTableName;
  }

  /**
   * Get foreign key column name.
   * @return Foreign key column name.
   */
  const boost::optional< std::string >& GetFKColumnName() const {
    return FKColumnName;
  }

  /**
   * Get column sequence number in key.
   * @return Sequence number in key.
   */
  boost::optional< int16_t > GetKeySeq() const {
    return keySeq;
  }

  /**
   * Get update rule.
   * @return Update rule.
   */
  boost::optional< int16_t > GetUpdateRule() const {
    return updateRule;
  }

  /**
   * Get delete rule.
   * @return Delete rule.
   */
  boost::optional< int16_t > GetDeleteRule() const {
    return deleteRule;
  }

  /**
   * Get foreign key name.
   * @return Foreign Key name.
   */
  const boost::optional< std::string >& GetFKName() const {
    return FKName;
  }

  /**
   * Get primary key name.
   * @return Primary Key name.
   */
  const boost::optional< std::string >& GetPKName() const {
    return PKName;
  }

  /**
   * Get deferrability.
   * @return Deferrability.
   */
  boost::optional< int16_t > GetDeferrability() const {
    return deferrability;
  }

 private:
  /** Primary key table catalog name. */
  boost::optional< std::string > PKCatalogName;

  /** Primary key table schema name. */
  boost::optional< std::string > PKSchemaName;

  /** Primary key table name. */
  boost::optional< std::string > PKTableName;

  /** Primary key column name. */
  boost::optional< std::string > PKColumnName;

  /** Foreign key table catalog name. */
  boost::optional< std::string > FKCatalogName;

  /** Foreign key table schema name. */
  boost::optional< std::string > FKSchemaName;

  /** Foreign key table name. */
  boost::optional< std::string > FKTableName;

  /** Foreign key column name. */
  boost::optional< std::string > FKColumnName;

  /** Column sequence number in key. */
  boost::optional< int16_t > keySeq;

  /** Update rule. */
  boost::optional< int16_t > updateRule;

  /** Delete rule. */
  boost::optional< int16_t > deleteRule;

  /** Foreign key name. */
  boost::optional< std::string > FKName;

  /** Primary key name. */
  boost::optional< std::string > PKName;

  /** Deferrability. */
  boost::optional< int16_t > deferrability;
};

/** Table metadata vector alias. */
typedef std::vector< ForeignKeyMeta > ForeignKeyMetaVector;

/**
 * Read foreign keys metadata collection.
 * @param resultSet SharedPointer< ResultSet >.
 * @param meta Collection.
 */
void ReadForeignKeysColumnMetaVector(SharedPointer< ResultSet >& resultSet,
                                     ForeignKeyMetaVector& meta);
}  // namespace meta
}  // namespace odbc
}  // namespace ignite

#endif  //_IGNITE_ODBC_META_FOREIGN_KEY_META
