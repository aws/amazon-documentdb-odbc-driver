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
 * Declares documentdb::odbc::DocumentDbError class.
 */

#ifndef _DOCUMENTDB_ODBC_DOCUMENTDB_ERROR
#define _DOCUMENTDB_ODBC_DOCUMENTDB_ERROR

#include <documentdb/odbc/common/common.h>
#include <stdint.h>

#include <exception>
#include <sstream>

// Define can be removed once the duplicated code was removed
#ifndef _DOCUMENTDB_ERROR_MACRO
#define _DOCUMENTDB_ERROR_MACRO

#define DOCUMENTDB_ERROR_1(code, part1)                \
  {                                                \
    std::stringstream stream;                      \
    stream << (part1);                             \
    throw DocumentDbError(code, stream.str().c_str()); \
  }

#define DOCUMENTDB_ERROR_2(code, part1, part2)         \
  {                                                \
    std::stringstream stream;                      \
    stream << (part1) << (part2);                  \
    throw DocumentDbError(code, stream.str().c_str()); \
  }

#define DOCUMENTDB_ERROR_3(code, part1, part2, part3)  \
  {                                                \
    std::stringstream stream;                      \
    stream << (part1) << (part2) << (part3);       \
    throw DocumentDbError(code, stream.str().c_str()); \
  }

#define DOCUMENTDB_ERROR_FORMATTED_1(code, msg, key1, val1)    \
  {                                                        \
    std::stringstream stream;                              \
    stream << msg << " [" << key1 << "=" << (val1) << "]"; \
    throw DocumentDbError(code, stream.str().c_str());         \
  }

#define DOCUMENTDB_ERROR_FORMATTED_2(code, msg, key1, val1, key2, val2)       \
  {                                                                       \
    std::stringstream stream;                                             \
    stream << msg << " [" << key1 << "=" << (val1) << ", " << key2 << "=" \
           << (val2) << "]";                                              \
    throw DocumentDbError(code, stream.str().c_str());                        \
  }

#define DOCUMENTDB_ERROR_FORMATTED_3(code, msg, key1, val1, key2, val2, key3, \
                                 val3)                                    \
  {                                                                       \
    std::stringstream stream;                                             \
    stream << msg << " [" << key1 << "=" << (val1) << ", " << key2 << "=" \
           << (val2) << ", " << key3 << "=" << (val3) << "]";             \
    throw DocumentDbError(code, stream.str().c_str());                        \
  }

#define DOCUMENTDB_ERROR_FORMATTED_4(code, msg, key1, val1, key2, val2, key3,    \
                                 val3, key4, val4)                           \
  {                                                                          \
    std::stringstream stream;                                                \
    stream << msg << " [" << key1 << "=" << (val1) << ", " << key2 << "="    \
           << (val2) << ", " << key3 << "=" << (val3) << ", " << key4 << "=" \
           << (val4) << "]";                                                 \
    throw DocumentDbError(code, stream.str().c_str());                           \
  }

#endif  //_DOCUMENTDB_ERROR_MACRO

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4275)
#endif  //_MSC_VER

namespace documentdb {
namespace odbc {
namespace jni {
namespace java {
enum class JniErrorCode {
  /* JNI error constants. */
  DOCUMENTDB_JNI_ERR_SUCCESS = 0,
  DOCUMENTDB_JNI_ERR_GENERIC = 1,
  DOCUMENTDB_JNI_ERR_JVM_INIT = 2,
  DOCUMENTDB_JNI_ERR_JVM_ATTACH = 3
};
}  // namespace java
}  // namespace jni

/**
 * %DocumentDB error information.
 */
class DOCUMENTDB_IMPORT_EXPORT DocumentDbError : public std::exception {
 public:
  /** Success. */
  static const int DOCUMENTDB_SUCCESS = 0;

  /** Failed to initialize JVM. */
  static const int DOCUMENTDB_ERR_JVM_INIT = 1;

  /** Failed to attach to JVM. */
  static const int DOCUMENTDB_ERR_JVM_ATTACH = 2;

  /** JVM library is not found. */
  static const int DOCUMENTDB_ERR_JVM_LIB_NOT_FOUND = 3;

  /** Failed to load JVM library. */
  static const int DOCUMENTDB_ERR_JVM_LIB_LOAD_FAILED = 4;

  /** JVM classpath is not provided. */
  static const int DOCUMENTDB_ERR_JVM_NO_CLASSPATH = 5;

  /** JVM error: no class definition found. */
  static const int DOCUMENTDB_ERR_JVM_NO_CLASS_DEF_FOUND = 6;

  /** JVM error: no such method. */
  static const int DOCUMENTDB_ERR_JVM_NO_SUCH_METHOD = 7;

  /** JNI error: getting database metadata */
  static const int DOCUMENTDB_ERR_JNI_GET_DATABASE_METADATA = 101;

  /** JNI error: getting DocumentDB database metadata */
  static const int DOCUMENTDB_ERR_JNI_GET_DOCUMENTDB_DATABASE_METADATA = 102;

  /** JNI error: getting DocumentDB connection properties */
  static const int DOCUMENTDB_ERR_JNI_GET_DOCUMENTDB_CONNECTION_PROPERTIES = 103;

  /** JNI error: creating DocumentDB query mapping service */
  static const int DOCUMENTDB_ERR_JNI_GET_DOCUMENTDB_QUERY_MAPPING_SERVICE = 104;

  /** Memory operation error. */
  static const int DOCUMENTDB_ERR_MEMORY = 1001;

  /** Binary error. */
  static const int DOCUMENTDB_ERR_BINARY = 1002;

  /** Standard library exception. */
  static const int DOCUMENTDB_ERR_STD = 1003;

  /** Generic %Ignite error. */
  static const int DOCUMENTDB_ERR_GENERIC = 2000;

  /** Illegal argument passed. */
  static const int DOCUMENTDB_ERR_ILLEGAL_ARGUMENT = 2001;

  /** Illegal state. */
  static const int DOCUMENTDB_ERR_ILLEGAL_STATE = 2002;

  /** Unsupported operation. */
  static const int DOCUMENTDB_ERR_UNSUPPORTED_OPERATION = 2003;

  /** Thread has been interrup. */
  static const int DOCUMENTDB_ERR_INTERRUPTED = 2004;

  /** Cluster group is empty. */
  static const int DOCUMENTDB_ERR_CLUSTER_GROUP_EMPTY = 2005;

  /** Cluster topology problem. */
  static const int DOCUMENTDB_ERR_CLUSTER_TOPOLOGY = 2006;

  /** Compute execution rejected. */
  static const int DOCUMENTDB_ERR_COMPUTE_EXECUTION_REJECTED = 2007;

  /** Compute job failover. */
  static const int DOCUMENTDB_ERR_COMPUTE_JOB_FAILOVER = 2008;

  /** Compute task cancelled. */
  static const int DOCUMENTDB_ERR_COMPUTE_TASK_CANCELLED = 2009;

  /** Compute task timeout. */
  static const int DOCUMENTDB_ERR_COMPUTE_TASK_TIMEOUT = 2010;

  /** Compute user undeclared exception. */
  static const int DOCUMENTDB_ERR_COMPUTE_USER_UNDECLARED_EXCEPTION = 2011;

  /** Generic cache error. */
  static const int DOCUMENTDB_ERR_CACHE = 2012;

  /** Generic cache loader error. */
  static const int DOCUMENTDB_ERR_CACHE_LOADER = 2013;

  /** Generic cache writer error. */
  static const int DOCUMENTDB_ERR_CACHE_WRITER = 2014;

  /** Generic cache entry processor error. */
  static const int DOCUMENTDB_ERR_ENTRY_PROCESSOR = 2015;

  /** Cache atomic update timeout. */
  static const int DOCUMENTDB_ERR_CACHE_ATOMIC_UPDATE_TIMEOUT = 2016;

  /** Cache partial update. */
  static const int DOCUMENTDB_ERR_CACHE_PARTIAL_UPDATE = 2017;

  /** Transaction optimisitc exception. */
  static const int DOCUMENTDB_ERR_TX_OPTIMISTIC = 2018;

  /** Transaction timeout. */
  static const int DOCUMENTDB_ERR_TX_TIMEOUT = 2019;

  /** Transaction rollback. */
  static const int DOCUMENTDB_ERR_TX_ROLLBACK = 2020;

  /** Transaction heuristic exception. */
  static const int DOCUMENTDB_ERR_TX_HEURISTIC = 2021;

  /** Authentication error. */
  static const int DOCUMENTDB_ERR_AUTHENTICATION = 2022;

  /** Security error. */
  static const int DOCUMENTDB_ERR_SECURITY = 2023;

  /** Future state error. */
  static const int DOCUMENTDB_ERR_FUTURE_STATE = 2024;

  /** Networking error. */
  static const int DOCUMENTDB_ERR_NETWORK_FAILURE = 2025;

  /** SSL/TLS error. */
  static const int DOCUMENTDB_ERR_SECURE_CONNECTION_FAILURE = 2026;

  /** Transaction already started by current thread. */
  static const int DOCUMENTDB_ERR_TX_THIS_THREAD = 2027;

  /** Generic transaction error. */
  static const int DOCUMENTDB_ERR_TX = 2028;

  /** Unknown error. */
  static const int DOCUMENTDB_ERR_UNKNOWN = -1;

  /**
   * Throw an error if code is not DOCUMENTDB_SUCCESS.
   *
   * @param err Error.
   */
  static void ThrowIfNeeded(const DocumentDbError& err);

  /**
   * Default constructor.
   * Creates empty error. Code is DOCUMENTDB_SUCCESS and message is NULL.
   */
  DocumentDbError();

  /**
   * Create error with specific code. Message is set to NULL.
   *
   * @param code Error code.
   */
  DocumentDbError(const int32_t code);

  /**
   * Create error with specific code and message.
   *
   * @param code Error code.
   * @param msg Message.
   */
  DocumentDbError(const int32_t code, const char* msg);

  /**
   * Copy constructor.
   *
   * @param other Other instance.
   */
  DocumentDbError(const DocumentDbError& other);

  /**
   * Assignment operator.
   *
   * @param other Other instance.
   * @return *this.
   */
  DocumentDbError& operator=(const DocumentDbError& other);

  /**
   * Destructor.
   */
  ~DocumentDbError() DOCUMENTDB_NO_THROW;

  /**
   * Get error code.
   *
   * @return Error code.
   */
  int32_t GetCode() const;

  /**
   * Get error message.
   *
   * @return Error message. Can be NULL.
   */
  const char* GetText() const DOCUMENTDB_NO_THROW;

  /**
   * Implementation of the standard std::exception::what() method.
   * Synonym for GetText() method.
   *
   * @return Error message string.
   */
  virtual const char* what() const DOCUMENTDB_NO_THROW;

  /**
   * Initializes DocumentDbError instance from the JNI error.
   *
   * @param jniCode Error code.
   * @param jniCls Error class.
   * @param jniMsg Error message.
   * @param err Error. Can not be NULL.
   */
  static void SetError(const jni::java::JniErrorCode jniCode,
                       const char* jniCls, const char* jniMsg,
                       DocumentDbError& err);

 private:
  /** Error code. */
  int32_t code;

  /** Error message. */
  char* msg;
};
}  // namespace odbc
}  // namespace documentdb

#ifdef _MSC_VER
#pragma warning(pop)
#endif  //_MSC_VER

#endif  //_DOCUMENTDB_ODBC_DOCUMENTDB_ERROR
