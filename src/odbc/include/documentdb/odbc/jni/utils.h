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
#ifndef _DOCUMENTDB_ODBC_JNI_UTILS
#define _DOCUMENTDB_ODBC_JNI_UTILS

#include <documentdb/odbc/common/common.h>
#include <documentdb/odbc/common/concurrent.h>
#include <documentdb/odbc/common/platform_utils.h>
#include <documentdb/odbc/config/configuration.h>
#include <documentdb/odbc/jni/java.h>

#include <string>

namespace documentdb {
namespace odbc {
namespace jni {
/**
 * Helper class to manage attached threads.
 */
class AttachHelper {
 public:
  /**
   * Destructor.
   */
  ~AttachHelper();

  /**
   * Callback invoked on successful thread attach ot JVM.
   */
  static void OnThreadAttach();
};

/**
 * Represents global reference to Java object.
 */
class DOCUMENTDB_IMPORT_EXPORT JavaGlobalRef {
 public:
  /**
   * Default constructor
   */
  JavaGlobalRef() : obj(NULL) {
    // No-op.
  }

  /**
   * Constructor
   *
   * @param ctx JNI context.
   * @param obj Java object.
   */
  JavaGlobalRef(common::concurrent::SharedPointer< java::JniContext >& ctx,
                jobject obj)
      : obj(NULL) {
    Init(ctx, obj);
  }

  /**
   * Copy constructor
   *
   * @param other Other instance.
   */
  JavaGlobalRef(const JavaGlobalRef& other) : obj(NULL) {
    Init(other.ctx, other.obj);
  }

  /**
   * Assignment operator.
   *
   * @param other Other instance.
   * @return *this.
   */
  JavaGlobalRef& operator=(const JavaGlobalRef& other) {
    if (this != &other) {
      java::JniContext::Release(obj);

      Init(other.ctx, other.obj);
    }

    return *this;
  }

  /**
   * Destructor.
   */
  ~JavaGlobalRef() {
    java::JniContext::Release(obj);
  }

  /**
   * Get object.
   *
   * @return Object.
   */
  jobject Get() {
    return obj;
  }

 private:
  /** Initializer */
  void Init(const common::concurrent::SharedPointer< java::JniContext >& ctx0,
            jobject obj0) {
    ctx = ctx0;

    if (ctx.IsValid())
      this->obj = ctx.Get()->Acquire(obj0);
  }

  /** Context. */
  common::concurrent::SharedPointer< java::JniContext > ctx;

  /** Object. */
  jobject obj;
};

/**
 * Attempts to find JVM library to load it into the process later.
 * First search is performed using the passed path argument (is not NULL).
 * Then JRE_HOME is evaluated. Last, JAVA_HOME is evaluated.
 *
 * @param path Explicitly defined path (optional).
 * @return Path to the file. Empty string if the library was not found.
 */
DOCUMENTDB_IMPORT_EXPORT std::string FindJvmLibrary(const std::string& path);

/**
 * Helper function to create classpath based on DocumentDB home directory.
 *
 * @param home Home directory; expected to be valid.
 * @param forceTest Force test classpath.
 * @return Classpath.
 */
DOCUMENTDB_IMPORT_EXPORT std::string CreateDocumentDbHomeClasspath(
    const std::string& home, bool forceTest);

/**
 * Create DocumentDB classpath based on user input and home directory.
 *
 * @param usrCp User's classpath.
 * @param home DocumentDB home directory.
 * @return Classpath.
 */
DOCUMENTDB_IMPORT_EXPORT std::string CreateDocumentDbClasspath(
    const std::string& usrCp, const std::string& home);

/**
 * Resolve DOCUMENTDB_HOME directory. Resolution is performed in several
 * steps:
 * 1) Check for path provided as argument.
 * 2) Check for environment variable.
 * 3) Check for current working directory.
 * Result of these checks are evaluated based on existence of certain
 * predefined folders inside possible DocumentDB home. If they are found,
 * DOCUMENTDB_HOME is considered resolved.
 *
 * @param path Optional path to evaluate.
 * @return Resolved DocumentDB home.
 */
DOCUMENTDB_IMPORT_EXPORT std::string ResolveDocumentDbHome(
    const std::string& path = "");
}  // namespace jni
}  // namespace odbc
}  // namespace documentdb

#endif  //_DOCUMENTDB_ODBC_JNI_UTILS
