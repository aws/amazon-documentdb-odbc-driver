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
#include <documentdb/odbc/common/utils.h>
#include <documentdb/odbc/documentdb_error.h>

#include <utility>

using namespace documentdb::odbc::common;
using namespace documentdb::odbc::jni::java;

namespace documentdb {
namespace odbc {
void DocumentDbError::ThrowIfNeeded(const DocumentDbError& err) {
  if (err.code != DOCUMENTDB_SUCCESS)
    throw err;
}

DocumentDbError::DocumentDbError() : code(DOCUMENTDB_SUCCESS), msg(NULL) {
  // No-op.
}

DocumentDbError::DocumentDbError(int32_t code) : code(code), msg(NULL) {
}

DocumentDbError::DocumentDbError(int32_t code, const char* msg)
    : code(code), msg(CopyChars(msg)) {
  // No-op.
}

DocumentDbError::DocumentDbError(const DocumentDbError& other)
    : code(other.code), msg(CopyChars(other.msg)) {
  // No-op.
}

DocumentDbError& DocumentDbError::operator=(const DocumentDbError& other) {
  if (this != &other) {
    DocumentDbError tmp(other);

    std::swap(code, tmp.code);
    std::swap(msg, tmp.msg);
  }

  return *this;
}

DocumentDbError::~DocumentDbError() DOCUMENTDB_NO_THROW {
  ReleaseChars(msg);
}

int32_t DocumentDbError::GetCode() const {
  return code;
}

const char* DocumentDbError::GetText() const DOCUMENTDB_NO_THROW {
  if (code == DOCUMENTDB_SUCCESS)
    return "Operation completed successfully.";
  else if (msg)
    return msg;
  else
    return "No additional information available.";
}

const char* DocumentDbError::what() const DOCUMENTDB_NO_THROW {
  return GetText();
}

void DocumentDbError::SetError(const JniErrorCode jniCode, const char* jniCls,
                           const char* jniMsg, DocumentDbError& err) {
  if (jniCode == JniErrorCode::DOCUMENTDB_JNI_ERR_SUCCESS)
    err = DocumentDbError();
  else if (jniCode == JniErrorCode::DOCUMENTDB_JNI_ERR_GENERIC) {
    // The most common case when we have Java exception "in hands" and must map
    // it to respective code.
    if (jniCls) {
      std::string jniCls0 = jniCls;

      if (jniCls0.compare("java.lang.NoClassDefFoundError") == 0) {
        std::stringstream stream;

        stream << "Java class is not found (did you set DOCUMENTDB_HOME "
                  "environment variable?)";

        if (jniMsg)
          stream << ": " << jniMsg;

        err = DocumentDbError(DOCUMENTDB_ERR_JVM_NO_CLASS_DEF_FOUND,
                          stream.str().c_str());
      } else if (jniCls0.compare("java.lang.NoSuchMethodError") == 0) {
        std::stringstream stream;

        stream << "Java method is not found (did you set DOCUMENTDB_HOME "
                  "environment variable?)";

        if (jniMsg)
          stream << ": " << jniMsg;

        err = DocumentDbError(DOCUMENTDB_ERR_JVM_NO_SUCH_METHOD, stream.str().c_str());
      } else if (jniCls0.compare("java.lang.IllegalArgumentException") == 0)
        err = DocumentDbError(DOCUMENTDB_ERR_ILLEGAL_ARGUMENT, jniMsg);
      else if (jniCls0.compare("java.lang.IllegalStateException") == 0)
        err = DocumentDbError(DOCUMENTDB_ERR_ILLEGAL_STATE, jniMsg);
      else if (jniCls0.compare("java.lang.UnsupportedOperationException") == 0)
        err = DocumentDbError(DOCUMENTDB_ERR_UNSUPPORTED_OPERATION, jniMsg);
      else if (jniCls0.compare("java.lang.InterruptedException") == 0)
        err = DocumentDbError(DOCUMENTDB_ERR_INTERRUPTED, jniMsg);
      else if (jniCls0.compare("javax.cache.CacheException") == 0)
        err = DocumentDbError(DOCUMENTDB_ERR_CACHE, jniMsg);
      else if (jniCls0.compare("javax.cache.integration.CacheLoaderException")
               == 0)
        err = DocumentDbError(DOCUMENTDB_ERR_CACHE_LOADER, jniMsg);
      else if (jniCls0.compare("javax.cache.integration.CacheWriterException")
               == 0)
        err = DocumentDbError(DOCUMENTDB_ERR_CACHE_WRITER, jniMsg);
      else if (jniCls0.compare("javax.cache.processor.EntryProcessorException")
               == 0)
        err = DocumentDbError(DOCUMENTDB_ERR_ENTRY_PROCESSOR, jniMsg);
      else if (jniCls0.compare("java.sql.SQLException") == 0) {
        std::stringstream stream;
        stream << "SQL exception occurred [cls=" << jniCls0;
        if (jniMsg) {
          stream << ", msg=" << jniMsg;
        }
        stream << "]";

        err =
            DocumentDbError(DOCUMENTDB_ERR_SQL_EXCEPTION, stream.str().c_str());
      } else {
        std::stringstream stream;
        stream << "Java exception occurred [cls=" << jniCls0;
        if (jniMsg)
          stream << ", msg=" << jniMsg;
        stream << "]";

        err = DocumentDbError(DOCUMENTDB_ERR_UNKNOWN, stream.str().c_str());
      }
    } else {
      // JNI class name is not available. Something really weird.
      err = DocumentDbError(DOCUMENTDB_ERR_UNKNOWN);
    }
  } else if (jniCode == JniErrorCode::DOCUMENTDB_JNI_ERR_JVM_INIT) {
    std::stringstream stream;

    stream << "Failed to initialize JVM [errCls=";

    if (jniCls)
      stream << jniCls;
    else
      stream << "N/A";

    stream << ", errMsg=";

    if (jniMsg)
      stream << jniMsg;
    else
      stream << "N/A";

    stream << "]";

    err = DocumentDbError(DOCUMENTDB_ERR_JVM_INIT, stream.str().c_str());
  } else if (jniCode == JniErrorCode::DOCUMENTDB_JNI_ERR_JVM_ATTACH) {
    err = DocumentDbError(DOCUMENTDB_ERR_JVM_ATTACH, "Failed to attach to JVM.");
  }
}
}  // namespace odbc
}  // namespace documentdb
