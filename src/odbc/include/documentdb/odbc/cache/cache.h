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
 * Declares documentdb::odbc::cache::Cache class.
 */

#ifndef _DOCUMENTDB_ODBC_CACHE_CACHE
#define _DOCUMENTDB_ODBC_CACHE_CACHE

#include <map>
#include <set>

#include <documentdb/odbc/common/common.h>
#include <documentdb/odbc/common/concurrent.h>
#include <documentdb/odbc/documentdb_error.h>

#include <documentdb/odbc/cache/cache_peek_mode.h>
#include <documentdb/odbc/cache/query/query_cursor.h>
#include <documentdb/odbc/cache/query/query_fields_cursor.h>
#include <documentdb/odbc/cache/query/query_scan.h>
#include <documentdb/odbc/cache/query/query_sql.h>
#include <documentdb/odbc/cache/query/query_text.h>
#include <documentdb/odbc/cache/query/query_sql_fields.h>
#include <documentdb/odbc/cache/query/continuous/continuous_query_handle.h>
#include <documentdb/odbc/cache/query/continuous/continuous_query.h>
#include <documentdb/odbc/impl/cache/cache_impl.h>
#include <documentdb/odbc/impl/cache/cache_entry_processor_holder.h>
#include <documentdb/odbc/impl/operations.h>
#include <documentdb/odbc/impl/module_manager.h>
#include <documentdb/odbc/documentdb_error.h>

namespace documentdb {
namespace odbc {
namespace cache {
/**
 * Main entry point for all Data Grid APIs.
 *
 * Both key and value types should be default-constructable,
 * copy-constructable and assignable. Also BinaryType class
 * template should be specialized for both types.
 *
 * This class is implemented as a reference to an implementation so copying
 * of this class instance will only create another reference to the same
 * underlying object. Underlying object will be released automatically once all
 * the instances are destructed.
 *
 * @tparam K Cache key type.
 * @tparam V Cache value type.
 */
template < typename K, typename V >
class DOCUMENTDB_IMPORT_EXPORT Cache {
 public:
  /**
   * Constructor.
   *
   * Internal method. Should not be used by user.
   *
   * @param impl Implementation.
   */
  Cache(impl::cache::CacheImpl* impl) : impl(impl) {
    // No-op.
  }

  /**
   * Get name of this cache (null for default cache).
   *
   * This method should only be used on the valid instance.
   *
   * @return Name of this cache (null for default cache).
   */
  const char* GetName() const {
    return impl.Get()->GetName();
  }

  /**
   * Checks whether this cache contains no key-value mappings.
   * Semantically equals to Cache.Size(CachePeekMode::PRIMARY) == 0.
   *
   * This method should only be used on the valid instance.
   *
   * @return True if cache is empty.
   */
  bool IsEmpty() {
    DocumentDbError err;

    bool res = IsEmpty(err);

    DocumentDbError::ThrowIfNeeded(err);

    return res;
  }

  /**
   * Checks whether this cache contains no key-value mappings.
   * Semantically equals to Cache.Size(CachePeekMode::PRIMARY) == 0.
   *
   * This method should only be used on the valid instance.
   *
   * @param err Error.
   * @return True if cache is empty.
   */
  bool IsEmpty(DocumentDbError& err) {
    return Size(err) == 0;
  }

  /**
   * Check if cache contains mapping for this key.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key.
   * @return True if cache contains mapping for this key.
   */
  bool ContainsKey(const K& key) {
    DocumentDbError err;

    bool res = ContainsKey(key, err);

    DocumentDbError::ThrowIfNeeded(err);

    return res;
  }

  /**
   * Check if cache contains mapping for this key.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key.
   * @param err Error.
   * @return True if cache contains mapping for this key.
   */
  bool ContainsKey(const K& key, DocumentDbError& err) {
    impl::In1Operation< K > op(key);

    return impl.Get()->ContainsKey(op, err);
  }

  /**
   * Check if cache contains mapping for these keys.
   *
   * This method should only be used on the valid instance.
   *
   * @param keys Keys.
   * @return True if cache contains mapping for all these keys.
   */
  bool ContainsKeys(const std::set< K >& keys) {
    DocumentDbError err;

    bool res = ContainsKeys(keys, err);

    DocumentDbError::ThrowIfNeeded(err);

    return res;
  }

  /**
   * Check if cache contains mapping for these keys.
   *
   * This method should only be used on the valid instance.
   *
   * @param begin Iterator pointing to the beggining of the key sequence.
   * @param end Iterator pointing to the end of the key sequence.
   * @return True if cache contains mapping for all these keys.
   */
  template < typename InputIter >
  bool ContainsKeys(InputIter begin, InputIter end) {
    DocumentDbError err;

    impl::InIterOperation< K, V, InputIter > op(begin, end);

    bool res = impl.Get()->ContainsKeys(op, err);

    DocumentDbError::ThrowIfNeeded(err);

    return res;
  }

  /**
   * Check if cache contains mapping for these keys.
   *
   * This method should only be used on the valid instance.
   *
   * @param keys Keys.
   * @param err Error.
   * @return True if cache contains mapping for all these keys.
   */
  bool ContainsKeys(const std::set< K >& keys, DocumentDbError& err) {
    impl::InSetOperation< K > op(keys);

    return impl.Get()->ContainsKeys(op, err);
  }

  /**
   * Peeks at cached value using optional set of peek modes. This method will
   * sequentially iterate over given peek modes, and try to peek at value using
   * each peek mode. Once a non-null value is found, it will be immediately
   * returned. This method does not participate in any transactions, however, it
   * may peek at transactional value depending on the peek modes used.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key.
   * @param peekModes Peek modes.
   * @return Value.
   */
  V LocalPeek(const K& key, int32_t peekModes) {
    DocumentDbError err;

    V res = LocalPeek(key, peekModes, err);

    DocumentDbError::ThrowIfNeeded(err);

    return res;
  }

  /**
   * Peeks at cached value using optional set of peek modes. This method will
   * sequentially iterate over given peek modes, and try to peek at value using
   * each peek mode. Once a non-null value is found, it will be immediately
   * returned. This method does not participate in any transactions, however, it
   * may peek at transactional value depending on the peek modes used.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key.
   * @param peekModes Peek modes.
   * @param err Error.
   * @return Value.
   */
  V LocalPeek(const K& key, int32_t peekModes, DocumentDbError& err) {
    V val;

    impl::InCacheLocalPeekOperation< K > inOp(key, peekModes);
    impl::Out1Operation< V > outOp(val);

    impl.Get()->LocalPeek(inOp, outOp, err);

    return val;
  }

  /**
   * Retrieves value mapped to the specified key from cache.
   * If the value is not present in cache, then it will be looked up from swap
   * storage. If it's not present in swap, or if swap is disabled, and if
   * read-through is allowed, value will be loaded from persistent store. This
   * method is transactional and will enlist the entry into ongoing transaction
   * if there is one.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key.
   * @return Value.
   */
  V Get(const K& key) {
    DocumentDbError err;

    V res = Get(key, err);

    DocumentDbError::ThrowIfNeeded(err);

    return res;
  }

  /**
   * Retrieves value mapped to the specified key from cache.
   * If the value is not present in cache, then it will be looked up from swap
   * storage. If it's not present in swap, or if swap is disabled, and if
   * read-through is allowed, value will be loaded from persistent store. This
   * method is transactional and will enlist the entry into ongoing transaction
   * if there is one.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key.
   * @param err Error.
   * @return Value.
   */
  V Get(const K& key, DocumentDbError& err) {
    V val;
    impl::In1Operation< K > inOp(key);
    impl::Out1Operation< V > outOp(val);

    impl.Get()->Get(inOp, outOp, err);

    return val;
  }

  /**
   * Retrieves values mapped to the specified keys from cache.
   * If some value is not present in cache, then it will be looked up from swap
   * storage. If it's not present in swap, or if swap is disabled, and if
   * read-through is allowed, value will be loaded from persistent store. This
   * method is transactional and will enlist the entry into ongoing transaction
   * if there is one.
   *
   * This method should only be used on the valid instance.
   *
   * @param keys Keys.
   * @return Map of key-value pairs.
   */
  std::map< K, V > GetAll(const std::set< K >& keys) {
    DocumentDbError err;

    std::map< K, V > res = GetAll(keys, err);

    DocumentDbError::ThrowIfNeeded(err);

    return res;
  }

  /**
   * Retrieves values mapped to the specified keys from cache.
   * If some value is not present in cache, then it will be looked up from swap
   * storage. If it's not present in swap, or if swap is disabled, and if
   * read-through is allowed, value will be loaded from persistent store. This
   * method is transactional and will enlist the entry into ongoing transaction
   * if there is one.
   *
   * This method should only be used on the valid instance.
   *
   * @param keys Keys.
   * @param err Error.
   * @return Map of key-value pairs.
   */
  std::map< K, V > GetAll(const std::set< K >& keys, DocumentDbError& err) {
    std::map< K, V > res;

    impl::InSetOperation< K > inOp(keys);
    impl::OutMapOperation< K, V > outOp(res);

    impl.Get()->GetAll(inOp, outOp, err);

    return res;
  }

  /**
   * Retrieves values mapped to the specified keys from cache.
   * If some value is not present in cache, then it will be looked up from swap
   * storage. If it's not present in swap, or if swap is disabled, and if
   * read-through is allowed, value will be loaded from persistent store. This
   * method is transactional and will enlist the entry into ongoing transaction
   * if there is one.
   *
   * This method should only be used on the valid instance.
   *
   * @param begin Iterator pointing to the beggining of the key sequence.
   * @param end Iterator pointing to the end of the key sequence.
   * @param dst Output iterator. Should dereference to std::pair or CacheEntry.
   */
  template < typename InIter, typename OutIter >
  void GetAll(InIter begin, InIter end, OutIter dst) {
    DocumentDbError err;

    impl::InIterOperation< K, V, InIter > inOp(begin, end);
    impl::OutMapIterOperation< K, V, OutIter > outOp(dst);

    impl.Get()->GetAll(inOp, outOp, err);

    DocumentDbError::ThrowIfNeeded(err);
  }

  /**
   * Associates the specified value with the specified key in the cache.
   * If the cache previously contained a mapping for the key,
   * the old value is replaced by the specified value.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key with which the specified value is to be associated.
   * @param val Value to be associated with the specified key.
   */
  void Put(const K& key, const V& val) {
    DocumentDbError err;

    Put(key, val, err);

    DocumentDbError::ThrowIfNeeded(err);
  }

  /**
   * Associates the specified value with the specified key in the cache.
   * If the cache previously contained a mapping for the key,
   * the old value is replaced by the specified value.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key with which the specified value is to be associated.
   * @param val Value to be associated with the specified key.
   * @param err Error.
   */
  void Put(const K& key, const V& val, DocumentDbError& err) {
    impl::In2Operation< K, V > op(key, val);

    impl.Get()->Put(op, err);
  }

  /**
   * Stores given key-value pairs in cache.
   * If write-through is enabled, the stored values will be persisted to store.
   * This method is transactional and will enlist the entry into ongoing
   * transaction if there is one.
   *
   * This method should only be used on the valid instance.
   *
   * @param vals Key-value pairs to store in cache.
   */
  void PutAll(const std::map< K, V >& vals) {
    DocumentDbError err;

    PutAll(vals, err);

    DocumentDbError::ThrowIfNeeded(err);
  }

  /**
   * Stores given key-value pairs in cache.
   * If write-through is enabled, the stored values will be persisted to store.
   * This method is transactional and will enlist the entry into ongoing
   * transaction if there is one.
   *
   * This method should only be used on the valid instance.
   *
   * @param vals Key-value pairs to store in cache.
   * @param err Error.
   */
  void PutAll(const std::map< K, V >& vals, DocumentDbError& err) {
    impl::InMapOperation< K, V > op(vals);

    impl.Get()->PutAll(op, err);
  }

  /**
   * Stores given key-value pairs in cache.
   * If write-through is enabled, the stored values will be persisted to store.
   * This method is transactional and will enlist the entry into ongoing
   * transaction if there is one.
   *
   * This method should only be used on the valid instance.
   *
   * Keys are locked in the order of iteration. It is caller's responsibility to
   * make sure keys always follow same order. If that is not observed, calling
   * this method in parallel <b>will lead to deadlock</b>.
   *
   * @param begin Iterator pointing to the beggining of the key-value pair
   * sequence.
   * @param end Iterator pointing to the end of the key-value pair sequence.
   */
  template < typename Iter >
  void PutAll(Iter begin, Iter end) {
    DocumentDbError err;

    impl::InIterOperation< K, V, Iter > op(begin, end);

    impl.Get()->PutAll(op, err);

    DocumentDbError::ThrowIfNeeded(err);
  }

  /**
   * Associates the specified value with the specified key in this cache,
   * returning an existing value if one existed.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key with which the specified value is to be associated.
   * @param val Value to be associated with the specified key.
   * @return The value associated with the key at the start of the
   *     operation or null if none was associated.
   */
  V GetAndPut(const K& key, const V& val) {
    DocumentDbError err;

    V res = GetAndPut(key, val, err);

    DocumentDbError::ThrowIfNeeded(err);

    return res;
  }

  /**
   * Associates the specified value with the specified key in this cache,
   * returning an existing value if one existed.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key with which the specified value is to be associated.
   * @param val Value to be associated with the specified key.
   * @param err Error.
   * @return The value associated with the key at the start of the
   *     operation or null if none was associated.
   */
  V GetAndPut(const K& key, const V& val, DocumentDbError& err) {
    V oldVal;

    impl::In2Operation< K, V > inOp(key, val);
    impl::Out1Operation< V > outOp(oldVal);

    impl.Get()->GetAndPut(inOp, outOp, err);

    return oldVal;
  }

  /**
   * Atomically replaces the value for a given key if and only if there is a
   * value currently mapped by the key.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key with which the specified value is to be associated.
   * @param val Value to be associated with the specified key.
   * @return The previous value associated with the specified key, or null if
   * there was no mapping for the key.
   */
  V GetAndReplace(const K& key, const V& val) {
    DocumentDbError err;

    V res = GetAndReplace(key, val, err);

    DocumentDbError::ThrowIfNeeded(err);

    return res;
  }

  /**
   * Atomically replaces the value for a given key if and only if there is a
   * value currently mapped by the key.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key with which the specified value is to be associated.
   * @param val Value to be associated with the specified key.
   * @param err Error.
   * @return The previous value associated with the specified key, or null if
   * there was no mapping for the key.
   */
  V GetAndReplace(const K& key, const V& val, DocumentDbError& err) {
    V oldVal;

    impl::In2Operation< K, V > inOp(key, val);
    impl::Out1Operation< V > outOp(oldVal);

    impl.Get()->GetAndReplace(inOp, outOp, err);

    return oldVal;
  }

  /**
   * Atomically removes the entry for a key only if currently mapped to some
   * value.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key with which the specified value is associated.
   * @return The value if one existed or null if no mapping existed for this
   * key.
   */
  V GetAndRemove(const K& key) {
    DocumentDbError err;

    V res = GetAndRemove(key, err);

    DocumentDbError::ThrowIfNeeded(err);

    return res;
  }

  /**
   * Atomically removes the entry for a key only if currently mapped to some
   * value.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key with which the specified value is associated.
   * @param err Error.
   * @return The value if one existed or null if no mapping existed for this
   * key.
   */
  V GetAndRemove(const K& key, DocumentDbError& err) {
    V oldVal;

    impl::In1Operation< K > inOp(key);
    impl::Out1Operation< V > outOp(oldVal);

    impl.Get()->GetAndRemove(inOp, outOp, err);

    return oldVal;
  }

  /**
   * Atomically associates the specified key with the given value if it is not
   * already associated with a value.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key with which the specified value is to be associated.
   * @param val Value to be associated with the specified key.
   * @return True if a value was set.
   */
  bool PutIfAbsent(const K& key, const V& val) {
    DocumentDbError err;

    bool res = PutIfAbsent(key, val, err);

    DocumentDbError::ThrowIfNeeded(err);

    return res;
  }

  /**
   * Atomically associates the specified key with the given value if it is not
   * already associated with a value.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key with which the specified value is to be associated.
   * @param val Value to be associated with the specified key.
   * @param err Error.
   * @return True if a value was set.
   */
  bool PutIfAbsent(const K& key, const V& val, DocumentDbError& err) {
    impl::In2Operation< K, V > op(key, val);

    return impl.Get()->PutIfAbsent(op, err);
  }

  /**
   * Stores given key-value pair in cache only if cache had no previous mapping
   * for it. If cache previously contained value for the given key, then this
   * value is returned. In case of PARTITIONED or REPLICATED caches, the value
   * will be loaded from the primary node, which in its turn may load the value
   * from the swap storage, and consecutively, if it's not in swap, from the
   * underlying persistent storage. If the returned value is not needed, method
   * putxIfAbsent() should be used instead of this one to avoid the overhead
   * associated with returning of the previous value. If write-through is
   * enabled, the stored value will be persisted to store. This method is
   * transactional and will enlist the entry into ongoing transaction if there
   * is one.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key to store in cache.
   * @param val Value to be associated with the given key.
   * @return Previously contained value regardless of whether put happened or
   * not (null if there was no previous value).
   */
  V GetAndPutIfAbsent(const K& key, const V& val) {
    DocumentDbError err;

    V res = GetAndPutIfAbsent(key, val, err);

    DocumentDbError::ThrowIfNeeded(err);

    return res;
  }

  /**
   * Stores given key-value pair in cache only if cache had no previous mapping
   * for it. If cache previously contained value for the given key, then this
   * value is returned. In case of PARTITIONED or REPLICATED caches, the value
   * will be loaded from the primary node, which in its turn may load the value
   * from the swap storage, and consecutively, if it's not in swap, from the
   * underlying persistent storage. If the returned value is not needed, method
   * putxIfAbsent() should be used instead of this one to avoid the overhead
   * associated with returning of the previous value. If write-through is
   * enabled, the stored value will be persisted to store. This method is
   * transactional and will enlist the entry into ongoing transaction if there
   * is one.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key to store in cache.
   * @param val Value to be associated with the given key.
   * @param err Error.
   * @return Previously contained value regardless of whether put happened or
   * not (null if there was no previous value).
   */
  V GetAndPutIfAbsent(const K& key, const V& val, DocumentDbError& err) {
    V oldVal;

    impl::In2Operation< K, V > inOp(key, val);
    impl::Out1Operation< V > outOp(oldVal);

    impl.Get()->GetAndPutIfAbsent(inOp, outOp, err);

    return oldVal;
  }

  /**
   * Stores given key-value pair in cache only if there is a previous mapping
   * for it. If cache previously contained value for the given key, then this
   * value is returned. In case of PARTITIONED or REPLICATED caches, the value
   * will be loaded from the primary node, which in its turn may load the value
   * from the swap storage, and consecutively, if it's not in swap, rom the
   * underlying persistent storage. If write-through is enabled, the stored
   * value will be persisted to store. This method is transactional and will
   * enlist the entry into ongoing transaction if there is one.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key to store in cache.
   * @param val Value to be associated with the given key.
   * @return True if the value was replaced.
   */
  bool Replace(const K& key, const V& val) {
    DocumentDbError err;

    bool res = Replace(key, val, err);

    DocumentDbError::ThrowIfNeeded(err);

    return res;
  }

  /**
   * Stores given key-value pair in cache only if there is a previous mapping
   * for it. If cache previously contained value for the given key, then this
   * value is returned. In case of PARTITIONED or REPLICATED caches, the value
   * will be loaded from the primary node, which in its turn may load the value
   * from the swap storage, and consecutively, if it's not in swap, rom the
   * underlying persistent storage. If write-through is enabled, the stored
   * value will be persisted to store. This method is transactional and will
   * enlist the entry into ongoing transaction if there is one.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key to store in cache.
   * @param val Value to be associated with the given key.
   * @param err Error.
   * @return True if the value was replaced.
   */
  bool Replace(const K& key, const V& val, DocumentDbError& err) {
    impl::In2Operation< K, V > op(key, val);

    return impl.Get()->Replace(op, err);
  }

  /**
   * Stores given key-value pair in cache only if the previous value is equal to
   * the old value passed as argument. This method is transactional and will
   * enlist the entry into ongoing transaction if there is one.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key to store in cache.
   * @param oldVal Old value to match.
   * @param newVal Value to be associated with the given key.
   * @return True if replace happened, false otherwise.
   */
  bool Replace(const K& key, const V& oldVal, const V& newVal) {
    DocumentDbError err;

    bool res = Replace(key, oldVal, newVal, err);

    DocumentDbError::ThrowIfNeeded(err);

    return res;
  }

  /**
   * Stores given key-value pair in cache only if the previous value is equal to
   * the old value passed as argument. This method is transactional and will
   * enlist the entry into ongoing transaction if there is one.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key to store in cache.
   * @param oldVal Old value to match.
   * @param newVal Value to be associated with the given key.
   * @param err Error.
   * @return True if replace happened, false otherwise.
   */
  bool Replace(const K& key, const V& oldVal, const V& newVal,
               DocumentDbError& err) {
    impl::In3Operation< K, V, V > op(key, oldVal, newVal);

    return impl.Get()->ReplaceIfEqual(op, err);
  }

  /**
   * Attempts to evict all entries associated with keys.
   *
   * @note Entry will be evicted only if it's not used (not
   * participating in any locks or transactions).
   *
   * This method should only be used on the valid instance.
   *
   * @param keys Keys to evict from cache.
   */
  void LocalEvict(const std::set< K >& keys) {
    DocumentDbError err;

    LocalEvict(keys, err);

    DocumentDbError::ThrowIfNeeded(err);
  }

  /**
   * Attempts to evict all entries associated with keys.
   *
   * @note Entry will be evicted only if it's not used (not
   * participating in any locks or transactions).
   *
   * This method should only be used on the valid instance.
   *
   * @param keys Keys to evict from cache.
   * @param err Error.
   */
  void LocalEvict(const std::set< K >& keys, DocumentDbError& err) {
    impl::InSetOperation< K > op(keys);

    impl.Get()->LocalEvict(op, err);
  }

  /**
   * Attempts to evict all entries associated with keys.
   *
   * @note Entry will be evicted only if it's not used (not
   * participating in any locks or transactions).
   *
   * This method should only be used on the valid instance.
   *
   * @param begin Iterator pointing to the beggining of the key sequence.
   * @param end Iterator pointing to the end of the key sequence.
   */
  template < typename Iter >
  void LocalEvict(Iter begin, Iter end) {
    DocumentDbError err;

    impl::InIterOperation< K, V, Iter > op(begin, end);

    impl.Get()->LocalEvict(op, err);

    DocumentDbError::ThrowIfNeeded(err);
  }

  /**
   * Clear cache.
   *
   * This method should only be used on the valid instance.
   */
  void Clear() {
    DocumentDbError err;

    Clear(err);

    DocumentDbError::ThrowIfNeeded(err);
  }

  /**
   * Clear cache.
   *
   * This method should only be used on the valid instance.
   *
   * @param err Error.
   */
  void Clear(DocumentDbError& err) {
    impl.Get()->Clear(err);
  }

  /**
   * Clear entry from the cache and swap storage, without notifying listeners or
   * CacheWriters. Entry is cleared only if it is not currently locked, and is
   * not participating in a transaction.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key to clear.
   */
  void Clear(const K& key) {
    DocumentDbError err;

    Clear(key, err);

    DocumentDbError::ThrowIfNeeded(err);
  }

  /**
   * Clear entry from the cache and swap storage, without notifying listeners or
   * CacheWriters. Entry is cleared only if it is not currently locked, and is
   * not participating in a transaction.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key to clear.
   * @param err Error.
   */
  void Clear(const K& key, DocumentDbError& err) {
    impl::In1Operation< K > op(key);

    impl.Get()->Clear(op, err);
  }

  /**
   * Clear entries from the cache and swap storage, without notifying listeners
   * or CacheWriters. Entry is cleared only if it is not currently locked, and
   * is not participating in a transaction.
   *
   * This method should only be used on the valid instance.
   *
   * @param keys Keys to clear.
   */
  void ClearAll(const std::set< K >& keys) {
    DocumentDbError err;

    ClearAll(keys, err);

    DocumentDbError::ThrowIfNeeded(err);
  }

  /**
   * Clear entries from the cache and swap storage, without notifying listeners
   * or CacheWriters. Entry is cleared only if it is not currently locked, and
   * is not participating in a transaction.
   *
   * This method should only be used on the valid instance.
   *
   * @param keys Keys to clear.
   * @param err Error.
   */
  void ClearAll(const std::set< K >& keys, DocumentDbError& err) {
    impl::InSetOperation< K > op(keys);

    impl.Get()->ClearAll(op, err);
  }

  /**
   * Clear entries from the cache and swap storage, without notifying listeners
   * or CacheWriters. Entry is cleared only if it is not currently locked, and
   * is not participating in a transaction.
   *
   * This method should only be used on the valid instance.
   *
   * @param begin Iterator pointing to the beggining of the key sequence.
   * @param end Iterator pointing to the end of the key sequence.
   */
  template < typename Iter >
  void ClearAll(Iter begin, Iter end) {
    DocumentDbError err;

    impl::InIterOperation< K, V, Iter > op(begin, end);

    impl.Get()->ClearAll(op, err);

    DocumentDbError::ThrowIfNeeded(err);
  }

  /**
   * Clear entry from the cache and swap storage, without notifying listeners or
   * CacheWriters. Entry is cleared only if it is not currently locked, and is
   * not participating in a transaction.
   *
   * @note This operation is local as it merely clears an entry from local
   * cache, it does not remove entries from remote caches.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key to clear.
   */
  void LocalClear(const K& key) {
    DocumentDbError err;

    LocalClear(key, err);

    DocumentDbError::ThrowIfNeeded(err);
  }

  /**
   * Clear entry from the cache and swap storage, without notifying listeners or
   * CacheWriters. Entry is cleared only if it is not currently locked, and is
   * not participating in a transaction.
   *
   * @note This operation is local as it merely clears an entry from local
   * cache, it does not remove entries from remote caches.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key to clear.
   * @param err Error.
   */
  void LocalClear(const K& key, DocumentDbError& err) {
    impl::In1Operation< K > op(key);

    impl.Get()->LocalClear(op, err);
  }

  /**
   * Clear entries from the cache and swap storage, without notifying listeners
   * or CacheWriters. Entry is cleared only if it is not currently locked, and
   * is not participating in a transaction.
   *
   * @note This operation is local as it merely clears entries from local cache,
   * it does not remove entries from remote caches.
   *
   * This method should only be used on the valid instance.
   *
   * @param keys Keys to clear.
   */
  void LocalClearAll(const std::set< K >& keys) {
    DocumentDbError err;

    LocalClearAll(keys, err);

    DocumentDbError::ThrowIfNeeded(err);
  }

  /**
   * Clear entries from the cache and swap storage, without notifying listeners
   * or CacheWriters. Entry is cleared only if it is not currently locked, and
   * is not participating in a transaction.
   *
   * @note This operation is local as it merely clears entries from local cache,
   * it does not remove entries from remote caches.
   *
   * This method should only be used on the valid instance.
   *
   * @param keys Keys to clear.
   * @param err Error.
   */
  void LocalClearAll(const std::set< K >& keys, DocumentDbError& err) {
    impl::InSetOperation< K > op(keys);

    impl.Get()->LocalClearAll(op, err);
  }

  /**
   * Clear entries from the cache and swap storage, without notifying listeners
   * or CacheWriters. Entry is cleared only if it is not currently locked, and
   * is not participating in a transaction.
   *
   * @note This operation is local as it merely clears entries from local cache,
   * it does not remove entries from remote caches.
   *
   * This method should only be used on the valid instance.
   *
   * @param begin Iterator pointing to the beggining of the key sequence.
   * @param end Iterator pointing to the end of the key sequence.
   */
  template < typename Iter >
  void LocalClearAll(Iter begin, Iter end) {
    DocumentDbError err;

    impl::InIterOperation< K, V, Iter > op(begin, end);

    impl.Get()->LocalClearAll(op, err);

    DocumentDbError::ThrowIfNeeded(err);
  }

  /**
   * Removes given key mapping from cache. If cache previously contained value
   * for the given key, then this value is returned. In case of PARTITIONED or
   * REPLICATED caches, the value will be loaded from the primary node, which in
   * its turn may load the value from the disk-based swap storage, and
   * consecutively, if it's not in swap, from the underlying persistent storage.
   * If the returned value is not needed, method removex() should always be used
   * instead of this one to avoid the overhead associated with returning of the
   * previous value. If write-through is enabled, the value will be removed from
   * store. This method is transactional and will enlist the entry into ongoing
   * transaction if there is one.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key whose mapping is to be removed from cache.
   * @return False if there was no matching key.
   */
  bool Remove(const K& key) {
    DocumentDbError err;

    bool res = Remove(key, err);

    DocumentDbError::ThrowIfNeeded(err);

    return res;
  }

  /**
   * Removes given key mapping from cache. If cache previously contained value
   * for the given key, then this value is returned. In case of PARTITIONED or
   * REPLICATED caches, the value will be loaded from the primary node, which in
   * its turn may load the value from the disk-based swap storage, and
   * consecutively, if it's not in swap, from the underlying persistent storage.
   * If the returned value is not needed, method removex() should always be used
   * instead of this one to avoid the overhead associated with returning of the
   * previous value. If write-through is enabled, the value will be removed from
   * store. This method is transactional and will enlist the entry into ongoing
   * transaction if there is one.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key whose mapping is to be removed from cache.
   * @param err Error.
   * @return False if there was no matching key.
   */
  bool Remove(const K& key, DocumentDbError& err) {
    impl::In1Operation< K > op(key);

    return impl.Get()->Remove(op, err);
  }

  /**
   * Removes given key mapping from cache if one exists and value is equal to
   * the passed in value. If write-through is enabled, the value will be removed
   * from store. This method is transactional and will enlist the entry into
   * ongoing transaction if there is one.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key whose mapping is to be removed from cache.
   * @param val Value to match against currently cached value.
   * @return True if entry was removed, false otherwise.
   */
  bool Remove(const K& key, const V& val) {
    DocumentDbError err;

    bool res = Remove(key, val, err);

    DocumentDbError::ThrowIfNeeded(err);

    return res;
  }

  /**
   * Removes given key mapping from cache if one exists and value is equal to
   * the passed in value. If write-through is enabled, the value will be removed
   * from store. This method is transactional and will enlist the entry into
   * ongoing transaction if there is one.
   *
   * This method should only be used on the valid instance.
   *
   * @param key Key whose mapping is to be removed from cache.
   * @param val Value to match against currently cached value.
   * @param err Error.
   * @return True if entry was removed, false otherwise.
   */
  bool Remove(const K& key, const V& val, DocumentDbError& err) {
    impl::In2Operation< K, V > op(key, val);

    return impl.Get()->RemoveIfEqual(op, err);
  }

  /**
   * Removes given key mappings from cache.
   * If write-through is enabled, the value will be removed from store.
   * This method is transactional and will enlist the entry into ongoing
   * transaction if there is one.
   *
   * This method should only be used on the valid instance.
   *
   * @param keys Keys whose mappings are to be removed from cache.
   */
  void RemoveAll(const std::set< K >& keys) {
    DocumentDbError err;

    RemoveAll(keys, err);

    DocumentDbError::ThrowIfNeeded(err);
  }

  /**
   * Removes given key mappings from cache.
   * If write-through is enabled, the value will be removed from store.
   * This method is transactional and will enlist the entry into ongoing
   * transaction if there is one.
   *
   * This method should only be used on the valid instance.
   *
   * @param keys Keys whose mappings are to be removed from cache.
   * @param err Error.
   */
  void RemoveAll(const std::set< K >& keys, DocumentDbError& err) {
    impl::InSetOperation< K > op(keys);

    impl.Get()->RemoveAll(op, err);
  }

  /**
   * Removes given key mappings from cache.
   * If write-through is enabled, the value will be removed from store.
   * This method is transactional and will enlist the entry into ongoing
   * transaction if there is one.
   *
   * This method should only be used on the valid instance.
   *
   * Keys are locked in the order of iteration. It is caller's responsibility to
   * make sure keys always follow same order. If that is not observed, calling
   * this method in parallel <b>will lead to deadlock</b>.
   *
   * @param begin Iterator pointing to the beggining of the key sequence.
   * @param end Iterator pointing to the end of the key sequence.
   */
  template < typename Iter >
  void RemoveAll(Iter begin, Iter end) {
    DocumentDbError err;

    impl::InIterOperation< K, V, Iter > op(begin, end);

    impl.Get()->RemoveAll(op, err);

    DocumentDbError::ThrowIfNeeded(err);
  }

  /**
   * Removes all mappings from cache.
   * If write-through is enabled, the value will be removed from store.
   * This method is transactional and will enlist the entry into ongoing
   * transaction if there is one.
   *
   * This method should only be used on the valid instance.
   *
   */
  void RemoveAll() {
    DocumentDbError err;

    RemoveAll(err);

    DocumentDbError::ThrowIfNeeded(err);
  }

  /**
   * Removes all mappings from cache.
   * If write-through is enabled, the value will be removed from store.
   * This method is transactional and will enlist the entry into ongoing
   * transaction if there is one.
   *
   * This method should only be used on the valid instance.
   *
   * @param err Error.
   */
  void RemoveAll(DocumentDbError& err) {
    return impl.Get()->RemoveAll(err);
  }

  /**
   * Gets the number of all entries cached on this node.
   *
   * This method should only be used on the valid instance.
   *
   * @return Cache size on this node.
   */
  int32_t LocalSize() {
    return LocalSize(CachePeekMode::ALL);
  }

  /**
   * Gets the number of all entries cached on this node.
   *
   * This method should only be used on the valid instance.
   *
   * @param err Error.
   * @return Cache size on this node.
   */
  int32_t LocalSize(DocumentDbError& err) {
    return LocalSize(CachePeekMode::ALL, err);
  }

  /**
   * Gets the number of all entries cached on this node.
   *
   * This method should only be used on the valid instance.
   *
   * @param peekModes Peek modes.
   * @return Cache size on this node.
   */
  int32_t LocalSize(int32_t peekModes) {
    DocumentDbError err;

    int32_t res = LocalSize(peekModes, err);

    DocumentDbError::ThrowIfNeeded(err);

    return res;
  }

  /**
   * Gets the number of all entries cached on this node.
   *
   * This method should only be used on the valid instance.
   *
   * @param peekModes Peek modes.
   * @param err Error.
   * @return Cache size on this node.
   */
  int32_t LocalSize(int32_t peekModes, DocumentDbError& err) {
    return impl.Get()->Size(peekModes, true, err);
  }

  /**
   * Gets the number of all entries cached across all nodes.
   * @note this operation is distributed and will query all participating nodes
   * for their cache sizes.
   *
   * This method should only be used on the valid instance.
   *
   * @return Cache size across all nodes.
   */
  int32_t Size() {
    return Size(documentdb::odbc::cache::CachePeekMode::ALL);
  }

  /**
   * Gets the number of all entries cached across all nodes.
   * @note This operation is distributed and will query all participating nodes
   * for their cache sizes.
   *
   * This method should only be used on the valid instance.
   *
   * @param err Error.
   * @return Cache size across all nodes.
   */
  int32_t Size(DocumentDbError& err) {
    return Size(documentdb::odbc::cache::CachePeekMode::ALL, err);
  }

  /**
   * Gets the number of all entries cached across all nodes.
   * @note This operation is distributed and will query all participating nodes
   * for their cache sizes.
   *
   * This method should only be used on the valid instance.
   *
   * @param peekModes Peek modes.
   * @return Cache size across all nodes.
   */
  int32_t Size(int32_t peekModes) {
    DocumentDbError err;

    int32_t res = Size(peekModes, err);

    DocumentDbError::ThrowIfNeeded(err);

    return res;
  }

  /**
   * Gets the number of all entries cached across all nodes.
   * @note This operation is distributed and will query all participating nodes
   * for their cache sizes.
   *
   * This method should only be used on the valid instance.
   *
   * @param peekModes Peek modes.
   * @param err Error.
   * @return Cache size across all nodes.
   */
  int32_t Size(int32_t peekModes, DocumentDbError& err) {
    return impl.Get()->Size(peekModes, false, err);
  }

  /**
   * Perform SQL query.
   * @deprecated Will be removed in future releases. Use SqlFieldsQuery instead.
   *
   * This method should only be used on the valid instance.
   *
   * @param qry Query.
   * @return Query cursor.
   */
  query::QueryCursor< K, V > Query(const query::SqlQuery& qry) {
    DocumentDbError err;

    query::QueryCursor< K, V > res = Query(qry, err);

    DocumentDbError::ThrowIfNeeded(err);

    return res;
  }

  /**
   * Perform SQL query.
   *
   * This method should only be used on the valid instance.
   *
   * @param qry Query.
   * @param err Error.
   * @return Query cursor.
   */
  query::QueryCursor< K, V > Query(const query::SqlQuery& qry,
                                   DocumentDbError& err) {
    impl::cache::query::QueryCursorImpl* cursorImpl =
        impl.Get()->QuerySql(qry, err);

    return query::QueryCursor< K, V >(cursorImpl);
  }

  /**
   * Perform text query.
   *
   * This method should only be used on the valid instance.
   *
   * @param qry Query.
   * @return Query cursor.
   */
  query::QueryCursor< K, V > Query(const query::TextQuery& qry) {
    DocumentDbError err;

    query::QueryCursor< K, V > res = Query(qry, err);

    DocumentDbError::ThrowIfNeeded(err);

    return res;
  }

  /**
   * Perform text query.
   *
   * This method should only be used on the valid instance.
   *
   * @param qry Query.
   * @param err Error.
   * @return Query cursor.
   */
  query::QueryCursor< K, V > Query(const query::TextQuery& qry,
                                   DocumentDbError& err) {
    impl::cache::query::QueryCursorImpl* cursorImpl =
        impl.Get()->QueryText(qry, err);

    return query::QueryCursor< K, V >(cursorImpl);
  }

  /**
   * Perform scan query.
   *
   * This method should only be used on the valid instance.
   *
   * @param qry Query.
   * @return Query cursor.
   */
  query::QueryCursor< K, V > Query(const query::ScanQuery& qry) {
    DocumentDbError err;

    query::QueryCursor< K, V > res = Query(qry, err);

    DocumentDbError::ThrowIfNeeded(err);

    return res;
  }

  /**
   * Perform scan query.
   *
   * This method should only be used on the valid instance.
   *
   * @param qry Query.
   * @param err Error.
   * @return Query cursor.
   */
  query::QueryCursor< K, V > Query(const query::ScanQuery& qry,
                                   DocumentDbError& err) {
    impl::cache::query::QueryCursorImpl* cursorImpl =
        impl.Get()->QueryScan(qry, err);

    return query::QueryCursor< K, V >(cursorImpl);
  }

  /**
   * Perform sql fields query.
   *
   * This method should only be used on the valid instance.
   *
   * @param qry Query.
   * @return Query cursor.
   */
  query::QueryFieldsCursor Query(const query::SqlFieldsQuery& qry) {
    DocumentDbError err;

    query::QueryFieldsCursor res = Query(qry, err);

    DocumentDbError::ThrowIfNeeded(err);

    return res;
  }

  /**
   * Perform sql fields query.
   *
   * This method should only be used on the valid instance.
   *
   * @param qry Query.
   * @param err Error.
   * @return Query cursor.
   */
  query::QueryFieldsCursor Query(const query::SqlFieldsQuery& qry,
                                 DocumentDbError& err) {
    impl::cache::query::QueryCursorImpl* cursorImpl =
        impl.Get()->QuerySqlFields(qry, err);

    return query::QueryFieldsCursor(cursorImpl);
  }

  /**
   * Invokes an CacheEntryProcessor against the MutableCacheEntry
   * specified by the provided key. If an entry does not exist for the
   * specified key, an attempt is made to load it (if a loader is
   * configured) or a surrogate entry, consisting of the key with a
   * null value is used instead.
   *
   * Return value, processor and argument classes should all be
   * default-constructable, copy-constructable and assignable. Also,
   * BinaryType class template should be specialized for every custom
   * class.
   *
   * Processor class should be registered as a cache entry processor
   * using IgniteBinding::RegisterCacheEntryProcessor() method. You
   * can declare #IgniteModuleInit() function to register your cache
   * processors upon module loading. There should be at most one
   * instance of such function per module.
   *
   * See the example below for details:
   * @code{.cpp}
   * DOCUMENTDB_EXPORTED_CALL void
   * IgniteModuleInit(documentdb::odbc::IgniteBindingContext& context)
   * {
   *     IgniteBinding binding = context.GetBinding();
   *
   *     binding.RegisterCacheEntryProcessor<MyProcessor1>();
   *     binding.RegisterCacheEntryProcessor<MyProcessor2>();
   *     // ...
   *     binding.RegisterCacheEntryProcessor<MyProcessorN>();
   * }
   * @endcode
   *
   * Additionally, processor class should be derived from the
   * documentdb::odbc::cacheEntryProcessor class.
   *
   * @throw DocumentDbError on fail.
   *
   * @param key The key.
   * @param processor The processor.
   * @param arg The argument.
   * @return Result of the processing.
   */
  template < typename R, typename P, typename A >
  R Invoke(const K& key, const P& processor, const A& arg) {
    DocumentDbError err;

    R res = Invoke< R >(key, processor, arg, err);

    DocumentDbError::ThrowIfNeeded(err);

    return res;
  }

  /**
   * Invokes an CacheEntryProcessor against the MutableCacheEntry
   * specified by the provided key. If an entry does not exist for the
   * specified key, an attempt is made to load it (if a loader is
   * configured) or a surrogate entry, consisting of the key with a
   * null value is used instead.
   *
   * Return value, processor and argument classes should all be
   * default-constructable, copy-constructable and assignable. Also,
   * BinaryType class template should be specialized for every custom
   * class.
   *
   * Processor class should be registered as a cache entry processor
   * using IgniteBinding::RegisterCacheEntryProcessor() method. You
   * can declare #IgniteModuleInit() function to register your cache
   * processors upon module loading. There should be at most one
   * instance of such function per module.
   *
   * See the example below for details:
   * @code{.cpp}
   * DOCUMENTDB_EXPORTED_CALL void
   * IgniteModuleInit(documentdb::odbc::IgniteBindingContext& context)
   * {
   *     IgniteBinding binding = context.GetBinding();
   *
   *     binding.RegisterCacheEntryProcessor<MyProcessor1>();
   *     binding.RegisterCacheEntryProcessor<MyProcessor2>();
   *     // ...
   *     binding.RegisterCacheEntryProcessor<MyProcessorN>();
   * }
   * @endcode
   *
   * Additionally, processor class should be derived from the
   * documentdb::odbc::cacheEntryProcessor class.
   *
   * Sets err param which should be checked for the operation result.
   *
   * @param key The key.
   * @param processor The processor.
   * @param arg The argument.
   * @param err Error.
   * @return Result of the processing. Default-constructed value on
   *   error.
   */
  template < typename R, typename P, typename A >
  R Invoke(const K& key, const P& processor, const A& arg, DocumentDbError& err) {
    typedef impl::cache::CacheEntryProcessorHolder< P, A > ProcessorHolder;

    R res;
    ProcessorHolder procHolder(processor, arg);

    impl::InCacheInvokeOperation< K, ProcessorHolder > inOp(key, procHolder);
    impl::Out1Operation< R > outOp(res);

    impl.Get()->Invoke(inOp, outOp, err);

    return res;
  }

  /**
   * Invokes an instance of Java class CacheEntryProcessor against the
   * entry specified by the provided key. If an entry does not exist
   * for the specified key, an attempt is made to load it (if a loader
   * is configured) or a surrogate entry, consisting of the key with a
   * null value is used instead.
   *
   * If task for given name has not been deployed yet, then
   * @c processorName will be used as task class name to auto-deploy
   * the task.
   *
   * Return value and argument classes should all be
   * default-constructable, copy-constructable and assignable. Also,
   * BinaryType class template should be specialized for every custom
   * class.
   *
   * For additional information on CacheEntryProcessor class and
   * details of its invocation refer to Java API documentation for
   * method org.apache.ignite.IgniteCache#invoke(...).
   *
   * @throw DocumentDbError on fail.
   *
   * @param key The key.
   * @param arg The argument.
   * @return Result of the processing.
   */
  template < typename R, typename A >
  R InvokeJava(const K& key, const std::string& processorName, const A& arg) {
    DocumentDbError err;

    R res = InvokeJava< R >(key, processorName, arg, err);

    DocumentDbError::ThrowIfNeeded(err);

    return res;
  }

  /**
   * Invokes an instance of Java class CacheEntryProcessor against the
   * entry specified by the provided key. If an entry does not exist
   * for the specified key, an attempt is made to load it (if a loader
   * is configured) or a surrogate entry, consisting of the key with a
   * null value is used instead.
   *
   * If task for given name has not been deployed yet, then
   * @c processorName will be used as task class name to auto-deploy
   * the task.
   *
   * Return value and argument classes should all be
   * default-constructable, copy-constructable and assignable. Also,
   * BinaryType class template should be specialized for every custom
   * class.
   *
   * For additional information on CacheEntryProcessor class and
   * details of its invocation refer to Java API documentation for
   * method org.apache.ignite.IgniteCache#invoke(...).
   *
   * @throw DocumentDbError on fail.
   *
   * @param key The key.
   * @param arg The argument.
   * @return Result of the processing.
   */
  template < typename R, typename A >
  R InvokeJava(const K& key, const std::string& processorName, const A& arg,
               DocumentDbError& err) {
    R res;

    impl::In3Operation< std::string, K, A > inOp(processorName, key, arg);
    impl::Out1Operation< R > outOp(res);

    impl.Get()->InvokeJava(inOp, outOp, err);

    return res;
  }

  /**
   * Start continuous query execution.
   *
   * @param qry Continuous query.
   * @return Continuous query handle.
   */
  query::continuous::ContinuousQueryHandle< K, V > QueryContinuous(
      const query::continuous::ContinuousQuery< K, V >& qry) {
    DocumentDbError err;

    query::continuous::ContinuousQueryHandle< K, V > res =
        QueryContinuous(qry, err);

    DocumentDbError::ThrowIfNeeded(err);

    return res;
  }

  /**
   * Start continuous query execution.
   *
   * @param qry Continuous query.
   * @param err Error.
   * @return Continuous query handle.
   */
  query::continuous::ContinuousQueryHandle< K, V > QueryContinuous(
      const query::continuous::ContinuousQuery< K, V >& qry, DocumentDbError& err) {
    using namespace impl::cache::query::continuous;
    using namespace common::concurrent;

    const SharedPointer< ContinuousQueryImpl< K, V > >& qryImpl = qry.impl;

    if (!qryImpl.IsValid() || !qryImpl.Get()->HasListener()) {
      err =
          DocumentDbError(DocumentDbError::DOCUMENTDB_ERR_GENERIC,
                      "Event listener is not set for ContinuousQuery instance");

      return query::continuous::ContinuousQueryHandle< K, V >();
    }

    ContinuousQueryHandleImpl* cqImpl =
        impl.Get()->QueryContinuous(qryImpl, err);

    return query::continuous::ContinuousQueryHandle< K, V >(cqImpl);
  }

  /**
   * Start continuous query execution with the initial query.
   *
   * @param qry Continuous query.
   * @param initialQry Initial query to be executed.
   * @return Continuous query handle.
   */
  template < typename Q >
  query::continuous::ContinuousQueryHandle< K, V > QueryContinuous(
      const query::continuous::ContinuousQuery< K, V >& qry,
      const Q& initialQry) {
    DocumentDbError err;

    query::continuous::ContinuousQueryHandle< K, V > res =
        QueryContinuous(qry, initialQry, err);

    DocumentDbError::ThrowIfNeeded(err);

    return res;
  }

  /**
   * Start continuous query execution with the initial query.
   *
   * @param qry Continuous query.
   * @param initialQry Initial query to be executed.
   * @param err Error.
   * @return Continuous query handle.
   */
  template < typename Q >
  query::continuous::ContinuousQueryHandle< K, V > QueryContinuous(
      const query::continuous::ContinuousQuery< K, V >& qry,
      const Q& initialQry, DocumentDbError& err) {
    using namespace impl::cache::query::continuous;
    using namespace common::concurrent;

    const SharedPointer< ContinuousQueryImpl< K, V > >& qryImpl = qry.impl;

    if (!qryImpl.IsValid() || !qryImpl.Get()->HasListener()) {
      err =
          DocumentDbError(DocumentDbError::DOCUMENTDB_ERR_GENERIC,
                      "Event listener is not set for ContinuousQuery instance");

      return query::continuous::ContinuousQueryHandle< K, V >();
    }

    ContinuousQueryHandleImpl* cqImpl =
        impl.Get()->QueryContinuous(qryImpl, initialQry, err);

    return query::continuous::ContinuousQueryHandle< K, V >(cqImpl);
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

  /**
   * Executes LocalLoadCache on all cache nodes.
   */
  void LoadCache() {
    DocumentDbError err;

    impl.Get()->LoadCache(err);

    DocumentDbError::ThrowIfNeeded(err);
  }

  /**
   * Loads state from the underlying persistent storage.
   *
   * This method is not transactional and may end up loading a stale value into
   * cache if another thread has updated the value immediately after it has been
   * loaded. It is mostly useful when pre-loading the cache from underlying
   * data store before start, or for read-only caches.
   */
  void LocalLoadCache() {
    DocumentDbError err;

    impl.Get()->LocalLoadCache(err);

    DocumentDbError::ThrowIfNeeded(err);
  }

 private:
  /** Implementation delegate. */
  common::concurrent::SharedPointer< impl::cache::CacheImpl > impl;
};
}  // namespace cache
}  // namespace odbc
}  // namespace documentdb

#endif  //_DOCUMENTDB_ODBC_CACHE_CACHE
