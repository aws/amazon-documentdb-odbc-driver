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

#ifndef _DOCUMENTDB_ODBC_IMPL_BINARY_BINARY_ID_RESOLVER
#define _DOCUMENTDB_ODBC_IMPL_BINARY_BINARY_ID_RESOLVER

#include <map>

#include "documentdb/odbc/binary/binary_type.h"
#include "documentdb/odbc/common/concurrent.h"
#include "documentdb/odbc/impl/binary/binary_type_handler.h"

using documentdb::odbc::DocumentDbError;

namespace documentdb {
namespace odbc {
namespace impl {
namespace binary {
/**
 * Binary type id resolver.
 */
class BinaryIdResolver {
 public:
  /**
   * Destructor.
   */
  virtual ~BinaryIdResolver() {
    // No-op.
  }

  /**
   * Get binary object type ID.
   *
   * @return Type ID.
   */
  virtual int32_t GetTypeId() = 0;

  /**
   * Get binary object field ID.
   *
   * @param typeId Type ID.
   * @param name Field name.
   * @return Field ID.
   */
  virtual int32_t GetFieldId(const int32_t typeId, const char* name) = 0;

  /**
   * Get copy of the instance.
   *
   * @return Copy of the instance.
   */
  virtual BinaryIdResolver* Clone() const = 0;
};

/**
 * Templated binary type resolver.
 */
template < typename T >
class TemplatedBinaryIdResolver : public BinaryIdResolver {
 public:
  /**
   * Constructor.
   */
  TemplatedBinaryIdResolver() {
    // No-op.
  }

  virtual int32_t GetTypeId() {
    return documentdb::odbc::binary::BinaryType< T >::GetTypeId();
  }

  virtual int32_t GetFieldId(const int32_t typeId, const char* name) {
    if (name)
      return documentdb::odbc::binary::BinaryType< T >::GetFieldId(name);

    DOCUMENTDB_ERROR_FORMATTED_1(DocumentDbError::DOCUMENTDB_ERR_BINARY,
                             "Field name cannot be NULL.", "typeId", typeId);
  }

  virtual BinaryIdResolver* Clone() const {
    return new TemplatedBinaryIdResolver< T >(*this);
  }
};

/**
 * Metadata binary type resolver.
 */
class MetadataBinaryIdResolver : public BinaryIdResolver {
 public:
  /**
   * Constructor.
   */
  MetadataBinaryIdResolver() : meta() {
    // No-op.
  }

  /**
   * Constructor.
   *
   * @param meta Binary type metadata snapshot.
   */
  MetadataBinaryIdResolver(SPSnap meta) : meta(meta) {
    // No-op.
  }

  virtual int32_t GetTypeId() {
    return meta.Get()->GetTypeId();
  }

  virtual int32_t GetFieldId(const int32_t typeId, const char* name) {
    if (!name) {
      DOCUMENTDB_ERROR_FORMATTED_1(DocumentDbError::DOCUMENTDB_ERR_BINARY,
                               "Field name cannot be NULL.", "typeId", typeId);
    }

    int32_t res = meta.Get()->GetFieldId(name);

    if (res == 0)
      res = documentdb::odbc::binary::GetBinaryStringHashCode(name);

    if (res == 0) {
      DOCUMENTDB_ERROR_FORMATTED_2(
          DocumentDbError::DOCUMENTDB_ERR_BINARY,
          "Field ID for the field name is zero. Please, redefine GetFieldId()"
          " method for the type or change field name",
          "typeId", typeId, "fieldName", name);
    }

    return res;
  }

  virtual BinaryIdResolver* Clone() const {
    return new MetadataBinaryIdResolver(*this);
  }

 private:
  /** Metadata snapshot. */
  SPSnap meta;
};
}  // namespace binary
}  // namespace impl
}  // namespace odbc
}  // namespace documentdb

#endif  //_DOCUMENTDB_ODBC_IMPL_BINARY_BINARY_ID_RESOLVER
