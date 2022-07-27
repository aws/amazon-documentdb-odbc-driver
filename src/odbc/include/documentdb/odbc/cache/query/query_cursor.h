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

/**
 * @file
 * Declares documentdb::odbc::cache::query::QueryCursor class template.
 */

#ifndef _DOCUMENTDB_ODBC_CACHE_QUERY_QUERY_CURSOR
#define _DOCUMENTDB_ODBC_CACHE_QUERY_QUERY_CURSOR

#include <vector>

#include <documentdb/odbc/common/concurrent.h>
#include <documentdb/odbc/documentdb_error.h>

#include "documentdb/odbc/cache/cache_entry.h"
#include "documentdb/odbc/impl/cache/query/query_impl.h"
#include "documentdb/odbc/impl/operations.h"

namespace documentdb {
namespace odbc {
namespace cache {
namespace query {
/**
 * Query cursor class template.
 *
 * Both key and value types should be default-constructable,
 * copy-constructable and assignable. Also BinaryType class
 * template should be specialized for both types.
 *
 * This class is implemented as a reference to an implementation so copying
 * of this class instance will only create another reference to the same
 * underlying object. Underlying object will be released automatically once all
 * the instances are destructed.
 */
template < typename K, typename V >
class QueryCursor {
 public:
  /**
   * Default constructor.
   *
   * Constructed instance is not valid and thus can not be used
   * as a cursor.
   */
  QueryCursor() : impl(0) {
    // No-op.
  }

  /**
   * Constructor.
   *
   * Internal method. Should not be used by user.
   *
   * @param impl Implementation.
   */
  QueryCursor(impl::cache::query::QueryCursorImpl* impl) : impl(impl) {
    // No-op.
  }

  /**
   * Check whether next entry exists.
   *
   * This method should only be used on the valid instance.
   *
   * @return True if next entry exists.
   *
   * @throw DocumentDbError class instance in case of failure.
   */
  bool HasNext() {
    DocumentDbError err;

    bool res = HasNext(err);

    DocumentDbError::ThrowIfNeeded(err);

    return res;
  }

  /**
   * Check whether next entry exists.
   * Properly sets error param in case of failure.
   *
   * This method should only be used on the valid instance.
   *
   * @param err Used to set operation result.
   * @return True if next entry exists and operation resulted in
   * success. Returns false on failure.
   */
  bool HasNext(DocumentDbError& err) {
    impl::cache::query::QueryCursorImpl* impl0 = impl.Get();

    if (impl0)
      return impl0->HasNext(err);
    else {
      err = DocumentDbError(DocumentDbError::DOCUMENTDB_ERR_GENERIC,
                        "Instance is not usable (did you check for error?).");

      return false;
    }
  }

  /**
   * Get next entry.
   *
   * This method should only be used on the valid instance.
   *
   * @return Next entry.
   *
   * @throw DocumentDbError class instance in case of failure.
   */
  CacheEntry< K, V > GetNext() {
    DocumentDbError err;

    CacheEntry< K, V > res = GetNext(err);

    DocumentDbError::ThrowIfNeeded(err);

    return res;
  }

  /**
   * Get next entry.
   * Properly sets error param in case of failure.
   *
   * This method should only be used on the valid instance.
   *
   * @param err Used to set operation result.
   * @return Next entry on success and default-constructed
   * entry on failure. Default-constructed entry contains
   * default-constructed instances of both key and value types.
   */
  CacheEntry< K, V > GetNext(DocumentDbError& err) {
    impl::cache::query::QueryCursorImpl* impl0 = impl.Get();

    if (impl0) {
      K key;
      V val;

      impl::Out2Operation< K, V > outOp(key, val);

      impl0->GetNext(outOp, err);

      return CacheEntry< K, V >(key, val);
    } else {
      err = DocumentDbError(DocumentDbError::DOCUMENTDB_ERR_GENERIC,
                        "Instance is not usable (did you check for error?).");

      return CacheEntry< K, V >();
    }
  }

  /**
   * Get all entries.
   *
   * This method should only be used on the valid instance.
   *
   * @param res Vector where query entries will be stored.
   *
   * @throw DocumentDbError class instance in case of failure.
   */
  void GetAll(std::vector< CacheEntry< K, V > >& res) {
    DocumentDbError err;

    GetAll(res, err);

    DocumentDbError::ThrowIfNeeded(err);
  }

  /**
   * Get all entries.
   * Properly sets error param in case of failure.
   *
   * This method should only be used on the valid instance.
   *
   * @param res Vector where query entries will be stored.
   * @param err Used to set operation result.
   */
  void GetAll(std::vector< CacheEntry< K, V > >& res, DocumentDbError& err) {
    impl::cache::query::QueryCursorImpl* impl0 = impl.Get();

    if (impl0) {
      impl::OutQueryGetAllOperation< K, V > outOp(res);

      impl0->GetAll(outOp, err);
    } else
      err = DocumentDbError(DocumentDbError::DOCUMENTDB_ERR_GENERIC,
                        "Instance is not usable (did you check for error?).");
  }

  /**
   * Get all entries.
   *
   * This method should only be used on the valid instance.
   *
   * @param iter Output iterator.
   */
  template < typename OutIter >
  void GetAll(OutIter iter) {
    impl::cache::query::QueryCursorImpl* impl0 = impl.Get();

    if (impl0) {
      impl::OutQueryGetAllOperationIter< K, V, OutIter > outOp(iter);

      impl0->GetAll(outOp);
    } else {
      throw DocumentDbError(DocumentDbError::DOCUMENTDB_ERR_GENERIC,
                        "Instance is not usable (did you check for error?).");
    }
  }

  /**
   * Check if the instance is valid.
   *
   * Invalid instance can be returned if some of the previous
   * operations have resulted in a failure. For example invalid
   * instance can be returned by not-throwing version of method
   * in case of error. Invalid instances also often can be
   * created using default constructor.
   *
   * @return True if the instance is valid and can be used.
   */
  bool IsValid() const {
    return impl.IsValid();
  }

 private:
  /** Implementation delegate. */
  documentdb::odbc::common::concurrent::SharedPointer<
      impl::cache::query::QueryCursorImpl >
      impl;
};
}  // namespace query
}  // namespace cache
}  // namespace odbc
}  // namespace documentdb

#endif  //_DOCUMENTDB_ODBC_CACHE_QUERY_QUERY_CURSOR
