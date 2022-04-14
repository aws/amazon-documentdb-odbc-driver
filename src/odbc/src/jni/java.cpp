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

// ReSharper disable once CppUnusedIncludeDirective
#include <ignite/odbc/common/common.h>
#include <ignite/odbc/common/utils.h>
#include <ignite/odbc/ignite_error.h>
#include <ignite/odbc/jni/java.h>
#include <ignite/odbc/jni/utils.h>
#include <ignite/odbc/log.h>

#include <algorithm>
#include <cstring>  // needed only on linux
#include <exception>
#include <stdexcept>
#include <string>
#include <vector>

// Todo: Refactor boost::optional to std::optional after code base is migrated
// to C++17 https://bitquill.atlassian.net/browse/AD-631

// TODO Alina -AL-: add more debug logs to functions in here

using namespace ignite::odbc::common::concurrent;
using namespace ignite::odbc::jni::java;

#ifndef JNI_VERSION_9
#define JNI_VERSION_9 0x00090000
#endif  // JNI_VERSION_9

#define IGNITE_SAFE_PROC_NO_ARG(jniEnv, envPtr, type, field)      \
  {                                                               \
    JniHandlers* hnds = reinterpret_cast< JniHandlers* >(envPtr); \
    type hnd = hnds->field;                                       \
    if (hnd) {                                                    \
      try {                                                       \
        hnd(hnds->target);                                        \
      } catch (std::exception & err) {                            \
        ThrowToJava(jniEnv, err.what());                          \
      }                                                           \
    } else                                                        \
      ThrowOnMissingHandler(jniEnv);                              \
  }

#define IGNITE_SAFE_PROC(jniEnv, envPtr, type, field, ...)        \
  {                                                               \
    JniHandlers* hnds = reinterpret_cast< JniHandlers* >(envPtr); \
    type hnd = hnds->field;                                       \
    if (hnd) {                                                    \
      try {                                                       \
        hnd(hnds->target, __VA_ARGS__);                           \
      } catch (std::exception & err) {                            \
        ThrowToJava(jniEnv, err.what());                          \
      }                                                           \
    } else                                                        \
      ThrowOnMissingHandler(jniEnv);                              \
  }

#define IGNITE_SAFE_FUNC(jniEnv, envPtr, type, field, ...)        \
  {                                                               \
    JniHandlers* hnds = reinterpret_cast< JniHandlers* >(envPtr); \
    type hnd = hnds->field;                                       \
    if (hnd) {                                                    \
      try {                                                       \
        return hnd(hnds->target, __VA_ARGS__);                    \
      } catch (std::exception & err) {                            \
        ThrowToJava(jniEnv, err.what());                          \
        return 0;                                                 \
      }                                                           \
    } else {                                                      \
      ThrowOnMissingHandler(jniEnv);                              \
      return 0;                                                   \
    }                                                             \
  }

using namespace ignite::odbc::jni::java;

namespace ignite {
namespace odbc {
namespace jni {
namespace java {
namespace iocc = ignite::odbc::common::concurrent;

bool IGNITE_IMPORT_EXPORT IsJava9OrLater() {
  LOG_DEBUG_MSG("IsJava9OrLater called");

  JavaVMInitArgs args;

  memset(&args, 0, sizeof(args));

  args.version = JNI_VERSION_9;

  LOG_DEBUG_MSG("IsJava9OrLater exiting");

  return JNI_GetDefaultJavaVMInitArgs(&args) == JNI_OK;
}

void BuildJvmOptions(const std::string& cp, std::vector< char* >& opts, int xms,
                     int xmx) {
  LOG_DEBUG_MSG("BuildJvmOptions is called");

  using namespace common;

  const size_t REQ_OPTS_CNT = 4;
  const size_t JAVA9_OPTS_CNT = 6;

  opts.reserve(REQ_OPTS_CNT + JAVA9_OPTS_CNT);

  // 1. Set classpath.
  std::string cpFull = "-Djava.class.path=" + cp;

  opts.push_back(CopyChars(cpFull.c_str()));

  // 3. Set Xms, Xmx.
  std::string xmsStr = "-Xms" + std::to_string(xms) + "m";
  std::string xmxStr = "-Xmx" + std::to_string(xmx) + "m";

  opts.push_back(CopyChars(xmsStr.c_str()));
  opts.push_back(CopyChars(xmxStr.c_str()));

  // 4. Optional debug arguments
  // std::string debugStr =
  // "-agentlib:jdwp=transport=dt_socket,server=y,suspend=n,address=5005";
  // opts.push_back(CopyChars(debugStr.c_str()));

  // 5. Set file.encoding.
  std::string fileEncParam = "-Dfile.encoding=";
  std::string fileEncFull = fileEncParam + "UTF-8";
  opts.push_back(CopyChars(fileEncFull.c_str()));

  // Adding options for Java 9 or later
  if (jni::java::IsJava9OrLater()) {
    opts.push_back(
        CopyChars("--add-exports=java.base/"
                  "jdk.internal.misc=ALL-UNNAMED"));
    opts.push_back(CopyChars("--add-exports=java.base/sun.nio.ch=ALL-UNNAMED"));
    opts.push_back(
        CopyChars("--add-exports=java.management/"
                  "com.sun.jmx.mbeanserver=ALL-UNNAMED"));
    opts.push_back(
        CopyChars("--add-exports=jdk.internal.jvmstat/"
                  "sun.jvmstat.monitor=ALL-UNNAMED"));
    opts.push_back(
        CopyChars("--add-exports=java.base/"
                  "sun.reflect.generics.reflectiveObjects="
                  "ALL-UNNAMED"));
    opts.push_back(
        CopyChars("--add-opens=jdk.management/"
                  "com.sun.management.internal=ALL-UNNAMED"));
  }
  LOG_DEBUG_MSG("BuildJvmOptions exiting");
}

/* --- Startup exception. --- */
class JvmException : public std::exception {
  // No-op.
};

/* --- JNI method definitions. --- */
struct JniMethod {
  char* name;
  char* sign;
  bool isStatic;

  JniMethod(const char* name, const char* sign, bool isStatic)
      : name(const_cast< char* >(name)),
        sign(const_cast< char* >(sign)),
        isStatic(isStatic) {
  }
};

JniErrorInfo::JniErrorInfo() : code(JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
  // No-op.
}

JniErrorInfo::JniErrorInfo(JniErrorCode code, const char* errCls,
                           const char* errMsg)
    : code(code), errCls(errCls), errMsg(errMsg) {
}

JniErrorInfo::JniErrorInfo(const JniErrorInfo& other) = default;

JniErrorInfo& JniErrorInfo::operator=(const JniErrorInfo& other) {
  LOG_DEBUG_MSG("JniErrorInfo constructor is called");

  if (this != &other) {
    // 1. Create new instance, exception could occur at this point.
    JniErrorInfo tmp(other);

    // 2. Swap with temp.
    std::swap(code, tmp.code);
    std::swap(errCls, tmp.errCls);
    std::swap(errMsg, tmp.errMsg);
  }

  LOG_DEBUG_MSG("JniErrorInfo constructor exiting");

  return *this;
}

// Classes and method definitions.
const char* const C_THROWABLE = "java/lang/Throwable";
JniMethod const M_THROWABLE_GET_MESSAGE =
    JniMethod("getMessage", "()Ljava/lang/String;", false);
JniMethod const M_THROWABLE_PRINT_STACK_TRACE =
    JniMethod("printStackTrace", "()V", false);

const char* const C_CLASS = "java/lang/Class";
JniMethod const M_CLASS_GET_NAME =
    JniMethod("getName", "()Ljava/lang/String;", false);

const char* const C_STRING = "java/lang/String";

const char* const C_DOCUMENTDB_CONNECTION_PROPERTIES =
    "software/amazon/documentdb/jdbc/DocumentDbConnectionProperties";
JniMethod const
    M_DOCUMENTDB_CONNECTION_PROPERTIES_GET_PROPERTIES_FROM_CONNECTION_STRING =
        JniMethod("getPropertiesFromConnectionString",
                  "(Ljava/lang/String;)Lsoftware/amazon/documentdb/jdbc/"
                  "DocumentDbConnectionProperties;",
                  true);

const char* const C_RECORD_SET = "java/sql/ResultSet";
JniMethod const M_RECORD_SET_CLOSE = JniMethod("close", "()V", false);
JniMethod const M_RECORD_SET_NEXT = JniMethod("next", "()Z", false);
JniMethod const M_RECORD_SET_GET_STRING_BY_INDEX =
    JniMethod("getString", "(I)Ljava/lang/String;", false);
JniMethod const M_RECORD_SET_GET_STRING_BY_NAME =
    JniMethod("getString", "(Ljava/lang/String;)Ljava/lang/String;", false);
JniMethod const M_RECORD_SET_GET_INT_BY_INDEX =
    JniMethod("getInt", "(I)I", false);
JniMethod const M_RECORD_SET_GET_INT_BY_NAME =
    JniMethod("getInt", "(Ljava/lang/String;)I", false);
JniMethod const M_RECORD_SET_GET_ROW = JniMethod("getRow", "()I", false);
JniMethod const M_RECORD_SET_WAS_NULL = JniMethod("wasNull", "()Z", false);

const char* const C_DATABASE_META_DATA = "java/sql/DatabaseMetaData";
JniMethod const M_DATABASE_META_DATA_GET_TABLES =
    JniMethod("getTables",
              "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[Ljava/"
              "lang/String;)Ljava/sql/ResultSet;",
              false);
JniMethod const M_DATABASE_META_DATA_GET_COLUMNS =
    JniMethod("getColumns",
              "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/"
              "String;Ljava/lang/String;)Ljava/sql/ResultSet;",
              false);

const char* const C_DOCUMENTDB_CONNECTION =
    "software/amazon/documentdb/jdbc/DocumentDbConnection";
JniMethod const M_DOCUMENTDB_CONNECTION_GET_SSH_LOCAL_PORT =
    JniMethod("getSshLocalPort", "()I", false);
JniMethod const M_DOCUMENTDB_CONNECTION_IS_SSH_TUNNEL_ACTIVE =
    JniMethod("isSshTunnelActive", "()Z", false);
JniMethod const M_DOCUMENTDB_CONNECTION_GET_DATABASE_METADATA =
    JniMethod("getDatabaseMetadata",
              "()Lsoftware/amazon/documentdb/jdbc/metadata/"
              "DocumentDbDatabaseSchemaMetadata;",
              false);
JniMethod const M_DOCUMENTDB_CONNECTION_GET_CONNECTION_PROPERTIES =
    JniMethod("getConnectionProperties",
              "()Lsoftware/amazon/documentdb/jdbc/"
              "DocumentDbConnectionProperties;",
              false);

const char* const C_DOCUMENTDB_DATABASE_SCHEMA_METADATA =
    "software/amazon/documentdb/jdbc/metadata/DocumentDbDatabaseSchemaMetadata";
JniMethod const M_DOCUMENTDB_DATABASE_SCHEMA_METADATA_GET_SCHEMA_NAME =
    JniMethod("getSchemaName", "()Ljava/lang/String;", false);

const char* const C_DRIVERMANAGER = "java/sql/DriverManager";
JniMethod const M_DRIVERMANAGER_GET_CONNECTION = JniMethod(
    "getConnection", "(Ljava/lang/String;)Ljava/sql/Connection;", true);

const char* const C_JAVA_SQL_CONNECTION = "java/sql/Connection";
JniMethod const M_JAVA_SQL_CONNECTION_CLOSE = JniMethod("close", "()V", false);
JniMethod const M_JAVA_SQL_CONNECTION_GET_META_DATA =
    JniMethod("getMetaData", "()Ljava/sql/DatabaseMetaData;", false);

const char* const C_LIST = "java/util/List";
JniMethod const M_LIST_SIZE = JniMethod("size", "()I", false);
JniMethod const M_LIST_TO_ARRAY =
    JniMethod("toArray", "()[Ljava/lang/Object;", false);
JniMethod const M_LIST_GET = JniMethod("get", "(I)Ljava/lang/Object;", false);

const char* const C_ITERATOR = "java/util/Iterator";
JniMethod const M_ITERATOR_NEXT =
    JniMethod("next", "()Ljava/lang/Object;", false);
JniMethod const M_ITERATOR_HAS_NEXT = JniMethod("hasNext", "()Z", false);

const char* const C_DOCUMENTDB_MQL_QUERY_CONTEXT =
    "software/amazon/documentdb/jdbc/query/DocumentDbMqlQueryContext";
JniMethod const
    M_DOCUMENTDB_MQL_QUERY_CONTEXT_GET_AGGREGATE_OPERATIONS_AS_STRINGS =
        JniMethod("getAggregateOperationsAsStrings", "()Ljava/util/List;",
                  false);  // of String
JniMethod const M_DOCUMENTDB_MQL_QUERY_CONTEXT_GET_COLUMN_METADATA = JniMethod(
    "getColumnMetaData", "()Ljava/util/List;", false);  // of JdbcColumnMetaData
JniMethod const M_DOCUMENTDB_MQL_QUERY_CONTEXT_GET_COLLECTION_NAME =
    JniMethod("getCollectionName", "()Ljava/lang/String;", false);
JniMethod const M_DOCUMENTDB_MQL_QUERY_CONTEXT_GET_PATHS =
    JniMethod("getPaths", "()Ljava/util/List;", false);  // of String

const char* const C_JDBC_COLUMN_METADATA =
    "software/amazon/documentdb/jdbc/common/utilities/JdbcColumnMetaData";
JniMethod const M_JDBC_COLUMN_METADATA_GET_ORDINAL =
    JniMethod("getOrdinal", "()I", false);
JniMethod const M_JDBC_COLUMN_METADATA_IS_AUTO_INCREMENT =
    JniMethod("isAutoIncrement", "()Z", false);
JniMethod const M_JDBC_COLUMN_METADATA_IS_CASE_SENSITIVE =
    JniMethod("isCaseSensitive", "()Z", false);
JniMethod const M_JDBC_COLUMN_METADATA_IS_SEARCHABLE =
    JniMethod("isSearchable", "()Z", false);
JniMethod const M_JDBC_COLUMN_METADATA_IS_CURRENCY =
    JniMethod("isCurrency", "()Z", false);
JniMethod const M_JDBC_COLUMN_METADATA_GET_NULLABLE =
    JniMethod("getNullable", "()I", false);
JniMethod const M_JDBC_COLUMN_METADATA_IS_SIGNED =
    JniMethod("isSigned", "()Z", false);
JniMethod const M_JDBC_COLUMN_METADATA_GET_COLUMN_DISPLAY_SIZE =
    JniMethod("getColumnDisplaySize", "()I", false);
JniMethod const M_JDBC_COLUMN_METADATA_GET_COLUMN_LABEL =
    JniMethod("getColumnLabel", "()Ljava/lang/String;", false);
JniMethod const M_JDBC_COLUMN_METADATA_GET_COLUMN_NAME =
    JniMethod("getColumnName", "()Ljava/lang/String;", false);
JniMethod const M_JDBC_COLUMN_METADATA_GET_SCHEMA_NAME =
    JniMethod("getSchemaName", "()Ljava/lang/String;", false);
JniMethod const M_JDBC_COLUMN_METADATA_GET_PRECISION =
    JniMethod("getPrecision", "()I", false);
JniMethod const M_JDBC_COLUMN_METADATA_GET_SCALE =
    JniMethod("getScale", "()I", false);
JniMethod const M_JDBC_COLUMN_METADATA_GET_TABLE_NAME =
    JniMethod("getTableName", "()Ljava/lang/String;", false);
JniMethod const M_JDBC_COLUMN_METADATA_GET_CATALOG_NAME =
    JniMethod("getCatalogName", "()Ljava/lang/String;", false);
JniMethod const M_JDBC_COLUMN_METADATA_GET_COLUMN_TYPE =
    JniMethod("getColumnType", "()I", false);
JniMethod const M_JDBC_COLUMN_METADATA_GET_COLUMN_TYPE_NAME =
    JniMethod("getColumnTypeName", "()Ljava/lang/String;", false);
JniMethod const M_JDBC_COLUMN_METADATA_IS_READ_ONLY =
    JniMethod("isReadOnly", "()Z", false);
JniMethod const M_JDBC_COLUMN_METADATA_IS_WRITABLE =
    JniMethod("isWritable", "()Z", false);
JniMethod const M_JDBC_COLUMN_METADATA_IS_DEFINITELY_WRITABLE =
    JniMethod("isDefinitelyWritable", "()Z", false);
JniMethod const M_JDBC_COLUMN_METADATA_GET_COLUMN_CLASS_NAME =
    JniMethod("getColumnClassName", "()Ljava/lang/String;", false);

const char* const C_DOCUMENTDB_QUERY_MAPPING_SERVICE =
    "software/amazon/documentdb/jdbc/query/DocumentDbQueryMappingService";
JniMethod const M_DOCUMENTDB_QUERY_MAPPING_SERVICE_INIT =
    JniMethod("<init>",
              "("
              "Lsoftware/amazon/documentdb/jdbc/DocumentDbConnectionProperties;"
              "Lsoftware/amazon/documentdb/jdbc/metadata/"
              "DocumentDbDatabaseSchemaMetadata;"
              ")V",
              false);
JniMethod const M_DOCUMENTDB_QUERY_MAPPING_SERVICE_GET = JniMethod(
    "get",
    "(Ljava/lang/String;J)"
    "Lsoftware/amazon/documentdb/jdbc/query/DocumentDbMqlQueryContext;",
    false);

// TODO: Provide a "getFullStackTrace" from DocumentDB
// JniMethod M_PLATFORM_UTILS_GET_FULL_STACK_TRACE =
// JniMethod("getFullStackTrace", "(Ljava/lang/Throwable;)Ljava/lang/String;",
// true);

/* STATIC STATE. */
iocc::CriticalSection JVM_LOCK;
iocc::CriticalSection CONSOLE_LOCK;
JniJvm JVM;
bool PRINT_EXCEPTION = false;
std::vector< ConsoleWriteHandler > consoleWriteHandlers;

/* HELPER METHODS. */

/**
 * Throw exception to Java in case of missing callback pointer. It means that
 * callback is not implemented in native platform and Java -> platform operation
 * cannot proceede further. As JniContext is not available at this point, we
 * have to obtain exception details from scratch. This is not critical from
 * performance perspective because missing handler usually denotes fatal
 * condition.
 *
 * @param env JNI environment.
 */
int ThrowOnMissingHandler(JNIEnv* env) {
  // jclass cls = env->FindClass(C_PLATFORM_NO_CALLBACK_EXCEPTION);

  // env->ThrowNew(cls, "Callback handler is not set in native platform.");
  LOG_DEBUG_MSG("ThrowOnMissingHandler is called, and exiting");

  return 0;
}

/**
 * Throw generic exception to Java in case of native exception. As JniContext is
 * not available at this point, we have to obtain exception details from
 * scratch. This is not critical from performance perspective because such
 * exception is usually denotes fatal condition.
 *
 * @param env JNI environment.
 * @param msg Message.
 */
void ThrowToJava(JNIEnv* env, const char* msg) {
  // jclass cls = env->FindClass(C_IGNITE_EXCEPTION);

  // env->ThrowNew(cls, msg);
}

char* StringToChars(JNIEnv* env, jstring str, int* len) {
  LOG_DEBUG_MSG("StringToChars is called");

  if (!str) {
    if (len) {
      *len = 0;
    }
    LOG_DEBUG_MSG("StringToChars exiting with nullptr");

    return nullptr;
  }

  const char* strChars = env->GetStringUTFChars(str, nullptr);
  const int strCharsLen = env->GetStringUTFLength(str);

  auto* strChars0 = new char[strCharsLen + 1];
  std::strcpy(strChars0, strChars);
  *(strChars0 + strCharsLen) = 0;

  env->ReleaseStringUTFChars(str, strChars);
  if (len) {
    *len = strCharsLen;
  }
  LOG_DEBUG_MSG("StringToChars exiting");

  return strChars0;
}

std::string JavaStringToCString(JNIEnv* env, jstring str, int& len) {
  LOG_DEBUG_MSG("JavaStringToCString is called");

  char* resChars = StringToChars(env, str, &len);

  if (resChars) {
    std::string res(resChars, len);

    delete[] resChars;

    LOG_DEBUG_MSG("JavaStringToCString exiting");

    return res;
  } else {
    LOG_DEBUG_MSG("JavaStringToCString exiting with empty string");

    return std::string();
  }
}

jclass FindClass(JNIEnv* env, const char* name) {
  LOG_DEBUG_MSG("FindClass is called");

  jclass res = env->FindClass(name);

  if (!res)
    throw JvmException();

  auto res0 = static_cast< jclass >(env->NewGlobalRef(res));

  env->DeleteLocalRef(res);

  LOG_DEBUG_MSG("FindClass exiting");

  return res0;
}

void DeleteClass(JNIEnv* env, jclass cls) {
  LOG_DEBUG_MSG("DeleteClass is called");

  if (cls)
    env->DeleteGlobalRef(cls);

  LOG_DEBUG_MSG("DeleteClass is called");
}

void CheckClass(JNIEnv* env, const char* name) {
  LOG_DEBUG_MSG("CheckClass is called");

  jclass res = env->FindClass(name);

  if (!res)
    throw JvmException();

  LOG_DEBUG_MSG("CheckClass is called");
}

jmethodID FindMethod(JNIEnv* env, jclass cls, JniMethod mthd) {
  LOG_DEBUG_MSG("FindMethod is called");

  jmethodID mthd0 = mthd.isStatic
                        ? env->GetStaticMethodID(cls, mthd.name, mthd.sign)
                        : env->GetMethodID(cls, mthd.name, mthd.sign);

  if (!mthd0)
    throw JvmException();

  LOG_DEBUG_MSG("FindMethod is called");

  return mthd0;
}

void JniJavaMembers::Initialize(JNIEnv* env) {
  LOG_DEBUG_MSG("Initialize is called");

  c_Class = FindClass(env, C_CLASS);
  m_Class_getName = FindMethod(env, c_Class, M_CLASS_GET_NAME);

  c_Throwable = FindClass(env, C_THROWABLE);
  m_Throwable_getMessage =
      FindMethod(env, c_Throwable, M_THROWABLE_GET_MESSAGE);
  m_Throwable_printStackTrace =
      FindMethod(env, c_Throwable, M_THROWABLE_PRINT_STACK_TRACE);

  c_String = FindClass(env, C_STRING);

  // TODO: Provide "getFullStackTrace" in DocumentDB
  // m_PlatformUtils_getFullStackTrace = FindMethod(env, c_PlatformUtils,
  // M_PLATFORM_UTILS_GET_FULL_STACK_TRACE);

  LOG_DEBUG_MSG("Initialize exiting");
}

void JniJavaMembers::Destroy(JNIEnv* env) {
  LOG_DEBUG_MSG("Destroy is called");

  DeleteClass(env, c_Class);
  DeleteClass(env, c_Throwable);
  DeleteClass(env, c_PlatformUtils);

  LOG_DEBUG_MSG("Destroy exiting");
}

bool JniJavaMembers::WriteErrorInfo(JNIEnv* env, char** errClsName,
                                    int* errClsNameLen, char** errMsg,
                                    int* errMsgLen, char** stackTrace,
                                    int* stackTraceLen) {
  LOG_DEBUG_MSG("WriteErrorInfo is called");

  if (env && env->ExceptionCheck()) {
    if (m_Class_getName && m_Throwable_getMessage) {
      jthrowable err = env->ExceptionOccurred();

      env->ExceptionClear();

      jclass errCls = env->GetObjectClass(err);

      auto clsName = static_cast< jstring >(
          env->CallObjectMethod(errCls, m_Class_getName));
      *errClsName = StringToChars(env, clsName, errClsNameLen);

      auto msg = static_cast< jstring >(
          env->CallObjectMethod(err, m_Throwable_getMessage));
      *errMsg = StringToChars(env, msg, errMsgLen);

      jstring trace = nullptr;

      if (c_PlatformUtils && m_PlatformUtils_getFullStackTrace) {
        trace = static_cast< jstring >(env->CallStaticObjectMethod(
            c_PlatformUtils, m_PlatformUtils_getFullStackTrace, err));
        *stackTrace = StringToChars(env, trace, stackTraceLen);
      }

      if (errCls)
        env->DeleteLocalRef(errCls);

      if (clsName)
        env->DeleteLocalRef(clsName);

      if (msg)
        env->DeleteLocalRef(msg);

      if (trace)
        env->DeleteLocalRef(trace);

      LOG_DEBUG_MSG("WriteErrorInfo exiting with bool true");

      return true;
    } else {
      env->ExceptionClear();
    }
  }
  LOG_DEBUG_MSG("WriteErrorInfo exiting with bool false");

  return false;
}

void JniMembers::Initialize(JNIEnv* env) {
  LOG_DEBUG_MSG("Initialize is called");

  c_DocumentDbConnectionProperties =
      FindClass(env, C_DOCUMENTDB_CONNECTION_PROPERTIES);
  m_DocumentDbConnectionPropertiesGetPropertiesFromConnectionString = FindMethod(
      env, c_DocumentDbConnectionProperties,
      M_DOCUMENTDB_CONNECTION_PROPERTIES_GET_PROPERTIES_FROM_CONNECTION_STRING);

  c_DocumentDbConnection = FindClass(env, C_DOCUMENTDB_CONNECTION);
  // m_DocumentDbConnectionInit = FindMethod(env, c_DocumentDbConnection,
  // M_DOCUMENTDB_CONNECTION_PROPERTIES_INIT);
  m_DocumentDbConnectionGetSshLocalPort = FindMethod(
      env, c_DocumentDbConnection, M_DOCUMENTDB_CONNECTION_GET_SSH_LOCAL_PORT);
  m_DocumentDbConnectionIsSshTunnelActive =
      FindMethod(env, c_DocumentDbConnection,
                 M_DOCUMENTDB_CONNECTION_IS_SSH_TUNNEL_ACTIVE);
  m_DocumentDbConnectionGetDatabaseMetadata =
      FindMethod(env, c_DocumentDbConnection,
                 M_DOCUMENTDB_CONNECTION_GET_DATABASE_METADATA);
  m_DocumentDbConnectionGetConnectionProperties =
      FindMethod(env, c_DocumentDbConnection,
                 M_DOCUMENTDB_CONNECTION_GET_CONNECTION_PROPERTIES);

  c_DocumentDbDatabaseSchemaMetadata =
      FindClass(env, C_DOCUMENTDB_DATABASE_SCHEMA_METADATA);
  m_DocumentDbDatabaseSchemaMetadataGetSchemaName =
      FindMethod(env, c_DocumentDbDatabaseSchemaMetadata,
                 M_DOCUMENTDB_DATABASE_SCHEMA_METADATA_GET_SCHEMA_NAME);

  c_DriverManager = FindClass(env, C_DRIVERMANAGER);
  m_DriverManagerGetConnection =
      FindMethod(env, c_DriverManager, M_DRIVERMANAGER_GET_CONNECTION);

  c_ResultSet = FindClass(env, C_RECORD_SET);
  m_ResultSetClose = FindMethod(env, c_ResultSet, M_RECORD_SET_CLOSE);
  m_ResultSetNext = FindMethod(env, c_ResultSet, M_RECORD_SET_NEXT);
  m_ResultSetGetStringByIndex =
      FindMethod(env, c_ResultSet, M_RECORD_SET_GET_STRING_BY_INDEX);
  m_ResultSetGetStringByName =
      FindMethod(env, c_ResultSet, M_RECORD_SET_GET_STRING_BY_NAME);
  m_ResultSetGetIntByIndex =
      FindMethod(env, c_ResultSet, M_RECORD_SET_GET_INT_BY_INDEX);
  m_ResultSetGetIntByName =
      FindMethod(env, c_ResultSet, M_RECORD_SET_GET_INT_BY_NAME);
  m_ResultSetGetRow = FindMethod(env, c_ResultSet, M_RECORD_SET_GET_ROW);
  m_ResultSetWasNull = FindMethod(env, c_ResultSet, M_RECORD_SET_WAS_NULL);

  c_DatabaseMetaData = FindClass(env, C_DATABASE_META_DATA);
  m_DatabaseMetaDataGetTables =
      FindMethod(env, c_DatabaseMetaData, M_DATABASE_META_DATA_GET_TABLES);
  m_DatabaseMetaDataGetColumns =
      FindMethod(env, c_DatabaseMetaData, M_DATABASE_META_DATA_GET_COLUMNS);

  c_Connection = FindClass(env, C_JAVA_SQL_CONNECTION);
  m_ConnectionClose =
      FindMethod(env, c_Connection, M_JAVA_SQL_CONNECTION_CLOSE);
  m_ConnectionGetMetaData =
      FindMethod(env, c_Connection, M_JAVA_SQL_CONNECTION_GET_META_DATA);

  c_List = FindClass(env, C_LIST);
  m_ListSize = FindMethod(env, c_List, M_LIST_SIZE);
  m_ListGet = FindMethod(env, c_List, M_LIST_GET);

  c_DocumentDbMqlQueryContext = FindClass(env, C_DOCUMENTDB_MQL_QUERY_CONTEXT);
  m_DocumentDbMqlQueryContextGetAggregateOperationsAsStrings = FindMethod(
      env, c_DocumentDbMqlQueryContext,
      M_DOCUMENTDB_MQL_QUERY_CONTEXT_GET_AGGREGATE_OPERATIONS_AS_STRINGS);
  m_DocumentDbMqlQueryContextGetColumnMetadata =
      FindMethod(env, c_DocumentDbMqlQueryContext,
                 M_DOCUMENTDB_MQL_QUERY_CONTEXT_GET_COLUMN_METADATA);
  m_DocumentDbMqlQueryContextGetCollectionName =
      FindMethod(env, c_DocumentDbMqlQueryContext,
                 M_DOCUMENTDB_MQL_QUERY_CONTEXT_GET_COLLECTION_NAME);
  m_DocumentDbMqlQueryContextGetPaths =
      FindMethod(env, c_DocumentDbMqlQueryContext,
                 M_DOCUMENTDB_MQL_QUERY_CONTEXT_GET_PATHS);

  c_JdbcColumnMetadata = FindClass(env, C_JDBC_COLUMN_METADATA);
  m_JdbcColumnMetadataGetOrdinal =
      FindMethod(env, c_JdbcColumnMetadata, M_JDBC_COLUMN_METADATA_GET_ORDINAL);
  m_JdbcColumnMetadataIsAutoIncrement = FindMethod(
      env, c_JdbcColumnMetadata, M_JDBC_COLUMN_METADATA_IS_AUTO_INCREMENT);
  m_JdbcColumnMetadataIsCaseSensitive = FindMethod(
      env, c_JdbcColumnMetadata, M_JDBC_COLUMN_METADATA_IS_CASE_SENSITIVE);
  m_JdbcColumnMetadataIsSearchable = FindMethod(
      env, c_JdbcColumnMetadata, M_JDBC_COLUMN_METADATA_IS_SEARCHABLE);
  m_JdbcColumnMetadataIsCurrency =
      FindMethod(env, c_JdbcColumnMetadata, M_JDBC_COLUMN_METADATA_IS_CURRENCY);
  m_JdbcColumnMetadataGetNullable = FindMethod(
      env, c_JdbcColumnMetadata, M_JDBC_COLUMN_METADATA_GET_NULLABLE);
  m_JdbcColumnMetadataIsSigned =
      FindMethod(env, c_JdbcColumnMetadata, M_JDBC_COLUMN_METADATA_IS_SIGNED);
  m_JdbcColumnMetadataGetColumnDisplaySize =
      FindMethod(env, c_JdbcColumnMetadata,
                 M_JDBC_COLUMN_METADATA_GET_COLUMN_DISPLAY_SIZE);
  m_JdbcColumnMetadataGetColumnLabel = FindMethod(
      env, c_JdbcColumnMetadata, M_JDBC_COLUMN_METADATA_GET_COLUMN_LABEL);
  m_JdbcColumnMetadataGetColumnName = FindMethod(
      env, c_JdbcColumnMetadata, M_JDBC_COLUMN_METADATA_GET_COLUMN_NAME);
  m_JdbcColumnMetadataGetSchemaName = FindMethod(
      env, c_JdbcColumnMetadata, M_JDBC_COLUMN_METADATA_GET_SCHEMA_NAME);
  m_JdbcColumnMetadataGetPrecision = FindMethod(
      env, c_JdbcColumnMetadata, M_JDBC_COLUMN_METADATA_GET_PRECISION);
  m_JdbcColumnMetadataGetScale =
      FindMethod(env, c_JdbcColumnMetadata, M_JDBC_COLUMN_METADATA_GET_SCALE);
  m_JdbcColumnMetadataGetTableName = FindMethod(
      env, c_JdbcColumnMetadata, M_JDBC_COLUMN_METADATA_GET_TABLE_NAME);
  m_JdbcColumnMetadataGetCatalogName = FindMethod(
      env, c_JdbcColumnMetadata, M_JDBC_COLUMN_METADATA_GET_CATALOG_NAME);
  m_JdbcColumnMetadataGetColumnType = FindMethod(
      env, c_JdbcColumnMetadata, M_JDBC_COLUMN_METADATA_GET_COLUMN_TYPE);
  m_JdbcColumnMetadataGetColumnTypeName = FindMethod(
      env, c_JdbcColumnMetadata, M_JDBC_COLUMN_METADATA_GET_COLUMN_TYPE_NAME);
  m_JdbcColumnMetadataIsReadOnly = FindMethod(
      env, c_JdbcColumnMetadata, M_JDBC_COLUMN_METADATA_IS_READ_ONLY);
  m_JdbcColumnMetadataIsWritable =
      FindMethod(env, c_JdbcColumnMetadata, M_JDBC_COLUMN_METADATA_IS_WRITABLE);
  m_JdbcColumnMetadataIsDefinitelyWritable = FindMethod(
      env, c_JdbcColumnMetadata, M_JDBC_COLUMN_METADATA_IS_DEFINITELY_WRITABLE);
  m_JdbcColumnMetadataGetColumnClassName = FindMethod(
      env, c_JdbcColumnMetadata, M_JDBC_COLUMN_METADATA_GET_COLUMN_CLASS_NAME);

  c_DocumentDbQueryMappingService =
      FindClass(env, C_DOCUMENTDB_QUERY_MAPPING_SERVICE);
  m_DocumentDbQueryMappingServiceCtor =
      FindMethod(env, c_DocumentDbQueryMappingService,
                 M_DOCUMENTDB_QUERY_MAPPING_SERVICE_INIT);
  m_DocumentDbQueryMappingServiceGet =
      FindMethod(env, c_DocumentDbQueryMappingService,
                 M_DOCUMENTDB_QUERY_MAPPING_SERVICE_GET);

  LOG_DEBUG_MSG("Initialize exiting");
}

void JniMembers::Destroy(JNIEnv* env) {
  LOG_DEBUG_MSG("Destroy is called");

  DeleteClass(env, c_IgniteException);
  DeleteClass(env, c_PlatformIgnition);
  DeleteClass(env, c_PlatformTarget);
  DeleteClass(env, c_PlatformUtils);

  LOG_DEBUG_MSG("Destroy exiting");
}

JniJvm::JniJvm()
    : jvm(nullptr), javaMembers(JniJavaMembers()), members(JniMembers()) {
  // No-op.

  LOG_DEBUG_MSG("JniJvm() is called, and exiting");
}

JniJvm::JniJvm(JavaVM* jvm, JniJavaMembers const& javaMembers,
               JniMembers const& members)
    : jvm(jvm), javaMembers(javaMembers), members(members) {
  // No-op.

  LOG_DEBUG_MSG(
      "JniJvm(JavaVM* jvm, JniJavaMembers const& javaMembers,  JniMembers "
      "const& members) is called, and exiting ");
}

JavaVM* JniJvm::GetJvm() {
  LOG_DEBUG_MSG("GetJvm is called, and exiting");

  return jvm;
}

JniJavaMembers& JniJvm::GetJavaMembers() {
  LOG_DEBUG_MSG("GetJavaMembers is called, and exiting");

  return javaMembers;
}

JniMembers& JniJvm::GetMembers() {
  LOG_DEBUG_MSG("GetMembers is called, and exiting");

  return members;
}

GlobalJObject::GlobalJObject(JNIEnv* e, jobject obj) : env(e), ref(obj) {
  // No-op.

  LOG_DEBUG_MSG("GlobalJObject is called, and exiting");
}

GlobalJObject::~GlobalJObject() {
  LOG_DEBUG_MSG("~GlobalJObject is called");

  env->DeleteGlobalRef(ref);

  LOG_DEBUG_MSG("~GlobalJObject exiting");
}

jobject GlobalJObject::GetRef() const {
  LOG_DEBUG_MSG("GetRef is called, and exiting");

  return ref;
}

/**
 * Create JVM.
 */
jint GetOrCreateJvm(char** opts, int optsLen, JavaVM** jvm, JNIEnv** env) {
  // Check to see if a VM is already created
  LOG_DEBUG_MSG("GetOrCreateJvm is called");

  const jsize nJvms = 1;
  jsize nJvmsAvailable = 0;
  JavaVM* availableJvms[nJvms]{};
  jint res = JNI_GetCreatedJavaVMs(&availableJvms[0], nJvms, &nJvmsAvailable);
  if (res == JNI_OK && nJvmsAvailable >= 1) {
    *jvm = availableJvms[0];
    res = (*jvm)->GetEnv(reinterpret_cast< void** >(env), JNI_VERSION_1_8);
    if (res == JNI_OK) {
      LOG_INFO_MSG("Jvm already created. Existing Jvm is used.");
      LOG_DEBUG_MSG("GetOrCreateJvm exiting with JNI_OK");

      return res;
    }
  }

  // Otherwise, create a VM
  JavaVMOption* opts0 = new JavaVMOption[optsLen]{};

  for (int i = 0; i < optsLen; i++)
    opts0[i].optionString = *(opts + i);

  JavaVMInitArgs args{};
  args.version = JNI_VERSION_1_8;
  args.nOptions = optsLen;
  args.options = opts0;
  args.ignoreUnrecognized = 0;

  res = JNI_CreateJavaVM(jvm, reinterpret_cast< void** >(env), &args);

  delete[] opts0;

  LOG_INFO_MSG("There is no previous Jvm created. Created new Jvm.");
  LOG_DEBUG_MSG("GetOrCreateJvm exiting");

  return res;
}

void RegisterNatives(JNIEnv* env) {
  IGNITE_UNUSED(env);
  // TODO: Investigate registering callbacks to get console and logging streams.
}

JniContext::JniContext(JniJvm* jvm, JniHandlers const& hnds)
    : jvm(jvm), hnds(hnds) {
  // No-op.
  LOG_DEBUG_MSG("JniContext constructor called, and exiting");
}

void GetJniErrorMessage(std::string& errMsg, jint res) {
  switch (res) {
    case JNI_ERR:
      errMsg = "Unknown error (JNI_ERR).";
      break;

    case JNI_EDETACHED:
      errMsg = "Thread detached from the JVM.";
      break;

    case JNI_EVERSION:
      errMsg = "JNI version error.";
      break;

    case JNI_ENOMEM:
      errMsg =
          "Could not reserve enough space for object heap. Check Xmx option.";
      break;

    case JNI_EEXIST:
      errMsg = "JVM already created.";
      break;

    case JNI_EINVAL:
      errMsg = "Invalid JVM arguments.";
      break;

    default:
      errMsg = "Unexpected JNI_CreateJavaVM result.";
      break;
  }
}

JniContext* JniContext::Create(char** opts, int optsLen,
                               JniHandlers const& hnds, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("Create is is called");

  // Acquire global lock to instantiate the JVM.
  JVM_LOCK.Enter();

  // Define local variables.
  JavaVM* jvm = nullptr;
  JNIEnv* env = nullptr;

  JniJavaMembers javaMembers;
  memset(&javaMembers, 0, sizeof(javaMembers));

  JniMembers members;
  memset(&members, 0, sizeof(members));

  JniContext* ctx = nullptr;

  std::string errClsName;
  int errClsNameLen = 0;
  std::string errMsg;
  int errMsgLen = 0;
  std::string stackTrace;
  int stackTraceLen = 0;

  try {
    if (!JVM.GetJvm()) {
      // Create JVM itself.
      jint res = GetOrCreateJvm(opts, optsLen, &jvm, &env);

      if (res == JNI_OK) {
        // Populate members.
        javaMembers.Initialize(env);
        members.Initialize(env);

        // Register native functions.
        RegisterNatives(env);

        // Create JNI JVM.
        JVM = JniJvm(jvm, javaMembers, members);

        char* printStack = getenv("IGNITE_CPP_PRINT_STACK");
        PRINT_EXCEPTION = printStack && strcmp("true", printStack) == 0;
      } else {
        GetJniErrorMessage(errMsg, res);

        errMsgLen = static_cast< int >(errMsg.length());
      }
    }

    if (JVM.GetJvm())
      ctx = new JniContext(&JVM, hnds);
  } catch (const JvmException&) {
    char* errClsNameChars = nullptr;
    char* errMsgChars = nullptr;
    char* stackTraceChars = nullptr;

    // Read error info if possible.
    javaMembers.WriteErrorInfo(env, &errClsNameChars, &errClsNameLen,
                               &errMsgChars, &errMsgLen, &stackTraceChars,
                               &stackTraceLen);

    if (errClsNameChars) {
      errClsName = errClsNameChars;

      delete[] errClsNameChars;
    }

    if (errMsgChars) {
      errMsg = errMsgChars;

      delete[] errMsgChars;
    }

    if (stackTraceChars) {
      stackTrace = stackTraceChars;

      delete[] stackTraceChars;
    }

    // Destroy mmebers.
    if (env) {
      members.Destroy(env);
      javaMembers.Destroy(env);
    }

    // Destroy faulty JVM.
    if (jvm)
      jvm->DestroyJavaVM();
  }

  // It safe to release the lock at this point.
  JVM_LOCK.Leave();

  // Notify err callback if needed.
  if (!ctx) {
    errInfo = JniErrorInfo(JniErrorCode::IGNITE_JNI_ERR_JVM_INIT,
                           errClsName.c_str(), errMsg.c_str());

    if (hnds.error)
      hnds.error(hnds.target, JniErrorCode::IGNITE_JNI_ERR_JVM_INIT,
                 errClsName.c_str(), errClsNameLen, errMsg.c_str(), errMsgLen,
                 stackTrace.c_str(), stackTraceLen, nullptr, 0);
  }
  LOG_DEBUG_MSG("Create is exiting");

  return ctx;
}

int JniContext::Reallocate(int64_t memPtr, int cap) {
  LOG_DEBUG_MSG("Reallocate is called");

  JavaVM* jvm = JVM.GetJvm();

  JNIEnv* env;

  int attachRes =
      jvm->AttachCurrentThread(reinterpret_cast< void** >(&env), nullptr);

  if (attachRes == JNI_OK)
    AttachHelper::OnThreadAttach();
  else {
    LOG_ERROR_MSG("Reallocate exiting with -1");
    LOG_INFO_MSG("attachRes: " << attachRes);
    return -1;
  }

  env->CallStaticVoidMethod(JVM.GetMembers().c_PlatformUtils,
                            JVM.GetMembers().m_PlatformUtils_reallocate, memPtr,
                            cap);

  if (env->ExceptionCheck()) {
    env->ExceptionClear();
    LOG_ERROR_MSG("Reallocate exiting with -1 because JVM exception occured");
    return -1;
  }

  LOG_DEBUG_MSG("Reallocate exiting");

  return 0;
}

void JniContext::Detach() {
  iocc::Memory::Fence();

  if (JVM.GetJvm()) {
    JNIEnv* env;

    JVM.GetJvm()->GetEnv(reinterpret_cast< void** >(&env), JNI_VERSION_1_6);

    if (env)
      JVM.GetJvm()->DetachCurrentThread();
  }
}

std::string JniContext::JavaStringToCppString(
    const SharedPointer< GlobalJObject >& value) {
  LOG_DEBUG_MSG("JavaStringToCppString is called");
  int len;
  JNIEnv* env = Attach();
  LOG_DEBUG_MSG("JavaStringToCppString exiting");
  return JavaStringToCString(env, static_cast< jstring >(value.Get()->GetRef()),
                             len);
}

JniErrorCode JniContext::DriverManagerGetConnection(
    const char* connectionString, SharedPointer< GlobalJObject >& connection,
    JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("DriverManagerGetConnection is called");

  JNIEnv* env = Attach();

  LOG_INFO_MSG("Connection String: [" << connectionString << "]");

  jstring jConnectionString = env->NewStringUTF(connectionString);
  jobject result = env->CallStaticObjectMethod(
      jvm->GetMembers().c_DriverManager,
      jvm->GetMembers().m_DriverManagerGetConnection, jConnectionString);
  ExceptionCheck(env, &errInfo);
  if (!result || errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    connection = nullptr;
  } else {
    connection = new GlobalJObject(env, env->NewGlobalRef(result));
  }

  LOG_DEBUG_MSG("DriverManagerGetConnection exiting");

  return errInfo.code;
}

JniErrorCode JniContext::ConnectionClose(
    const SharedPointer< GlobalJObject >& connection, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("ConnectionClose is called");

  if (connection.Get() == nullptr) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "Connection object must be set.";

    LOG_ERROR_MSG("ConnectionClose exiting with error msg: " << errInfo.errMsg);

    return errInfo.code;
  }
  JNIEnv* env = Attach();
  env->CallVoidMethod(connection.Get()->GetRef(),
                      jvm->GetMembers().m_ConnectionClose);
  ExceptionCheck(env, &errInfo);

  LOG_DEBUG_MSG("ConnectionClose exiting");

  return errInfo.code;
}

JniErrorCode JniContext::DocumentDbConnectionIsSshTunnelActive(
    const SharedPointer< GlobalJObject >& connection, bool& isActive,
    JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("DocumentDbConnectionIsSshTunnelActive is called");

  if (!connection.Get()) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "Connection object must be set.";

    LOG_ERROR_MSG(
        "DocumentDbConnectionIsSshTunnelActive exiting with error msg: "
        << errInfo.errMsg);

    return errInfo.code;
  }
  JNIEnv* env = Attach();
  jboolean res = env->CallBooleanMethod(
      connection.Get()->GetRef(),
      jvm->GetMembers().m_DocumentDbConnectionIsSshTunnelActive);
  ExceptionCheck(env, &errInfo);
  if (errInfo.code == JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    isActive = res != JNI_FALSE;
  }

  LOG_DEBUG_MSG("DocumentDbConnectionIsSshTunnelActive exiting");

  return errInfo.code;
}

JniErrorCode JniContext::DocumentDbConnectionGetSshLocalPort(
    const SharedPointer< GlobalJObject >& connection, int32_t& result,
    JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("DocumentDbConnectionGetSshLocalPort is called");

  if (!connection.Get()) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "Connection object must be set.";

    LOG_ERROR_MSG("DocumentDbConnectionGetSshLocalPort exiting with error msg: "
                  << errInfo.errMsg);

    return errInfo.code;
  }
  JNIEnv* env = Attach();
  result = env->CallIntMethod(
      connection.Get()->GetRef(),
      jvm->GetMembers().m_DocumentDbConnectionGetSshLocalPort);
  ExceptionCheck(env, &errInfo);

  LOG_DEBUG_MSG("DocumentDbConnectionGetSshLocalPort exiting");

  return errInfo.code;
}

JniErrorCode JniContext::DocumentDbConnectionGetDatabaseMetadata(
    const SharedPointer< GlobalJObject >& connection,
    SharedPointer< GlobalJObject >& metadata, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("DocumentDbConnectionGetDatabaseMetadata is called");
  if (!connection.Get()) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "Connection object must be set.";

    LOG_ERROR_MSG(
        "DocumentDbConnectionGetDatabaseMetadata exiting with error msg: "
        << errInfo.errMsg);

    return errInfo.code;
  }
  JNIEnv* env = Attach();
  jobject result = env->CallObjectMethod(
      connection.Get()->GetRef(),
      jvm->GetMembers().m_DocumentDbConnectionGetDatabaseMetadata);
  ExceptionCheck(env, &errInfo);

  if (!result || errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    metadata = nullptr;

    LOG_ERROR_MSG(
        "DocumentDbConnectionGetDatabaseMetadata exiting with error. metadata "
        "will be null");

    return errInfo.code;
  }

  metadata = new GlobalJObject(env, env->NewGlobalRef(result));

  LOG_DEBUG_MSG("DocumentDbConnectionGetDatabaseMetadata exiting");
  return errInfo.code;
}

JniErrorCode JniContext::DocumentDbConnectionGetConnectionProperties(
    const SharedPointer< GlobalJObject >& connection,
    SharedPointer< GlobalJObject >& connectionProperties,
    JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("DocumentDbConnectionGetConnectionProperties is called");

  if (!connection.Get()) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "Connection object must be set.";

    LOG_ERROR_MSG(
        "DocumentDbConnectionGetConnectionProperties exiting with error msg: "
        << errInfo.errMsg);

    return errInfo.code;
  }
  JNIEnv* env = Attach();
  jobject result = env->CallObjectMethod(
      connection.Get()->GetRef(),
      jvm->GetMembers().m_DocumentDbConnectionGetConnectionProperties);
  ExceptionCheck(env, &errInfo);

  if (!result || errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    connectionProperties = nullptr;
    LOG_ERROR_MSG(
        "DocumentDbConnectionGetConnectionProperties exiting with error. "
        "connectionProperties will be null");
    return errInfo.code;
  }

  connectionProperties = new GlobalJObject(env, env->NewGlobalRef(result));

  LOG_DEBUG_MSG("DocumentDbConnectionGetConnectionProperties exiting");

  return errInfo.code;
}

JniErrorCode JniContext::DocumentDbDatabaseSchemaMetadataGetSchemaName(
    const SharedPointer< GlobalJObject >& databaseMetadata, std::string& value,
    bool& wasNull, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("DocumentDbDatabaseSchemaMetadataGetSchemaName is called");
  if (!databaseMetadata.Get()) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "DatabaseMetadata object must be set.";
    LOG_ERROR_MSG(
        "DocumentDbDatabaseSchemaMetadataGetSchemaName exiting with error msg: "
        << errInfo.errMsg);
    return errInfo.code;
  }

  JNIEnv* env = Attach();
  jobject result = env->CallObjectMethod(
      databaseMetadata.Get()->GetRef(),
      jvm->GetMembers().m_DocumentDbDatabaseSchemaMetadataGetSchemaName);
  ExceptionCheck(env, &errInfo);

  if (errInfo.code == JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    wasNull = !result;
    if (result != nullptr) {
      jboolean isCopy;
      const char* utfChars = env->GetStringUTFChars((jstring)result, &isCopy);
      value = std::string(utfChars);
      env->ReleaseStringUTFChars((jstring)result, utfChars);
    }
  }
  //-AL- todo? log the returned values ? If it is not parameters and not high
  // level function (not directly related
  // to entry_points functions), the values can be in debug_log.
  LOG_DEBUG_MSG("DocumentDbDatabaseSchemaMetadataGetSchemaName exiting");

  return errInfo.code;
}

JniErrorCode JniContext::ConnectionGetMetaData(
    const SharedPointer< GlobalJObject >& connection,
    SharedPointer< GlobalJObject >& databaseMetaData, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("ConnectionGetMetaData is called");

  if (connection.Get() == nullptr) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "Connection object must be set.";

    LOG_ERROR_MSG(
        "ConnectionGetMetaData exiting with error msg: " << errInfo.errMsg);

    return errInfo.code;
  }

  JNIEnv* env = Attach();
  jobject result = env->CallObjectMethod(
      connection.Get()->GetRef(), jvm->GetMembers().m_ConnectionGetMetaData);
  ExceptionCheck(env, &errInfo);

  if (!result || errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    databaseMetaData = nullptr;

    LOG_ERROR_MSG(
        "ConnectionGetMetaData exiting with error. databaseMetaData will be "
        "null");

    return errInfo.code;
  }

  databaseMetaData = new GlobalJObject(env, env->NewGlobalRef(result));

  LOG_DEBUG_MSG("ConnectionGetMetaData exiting");

  return errInfo.code;
}

JniErrorCode JniContext::DatabaseMetaDataGetTables(
    const SharedPointer< GlobalJObject >& databaseMetaData,
    const std::string& catalog, const std::string& schemaPattern,
    const std::string& tableNamePattern,
    const std::vector< std::string >& types,
    SharedPointer< GlobalJObject >& resultSet, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("DatabaseMetaDataGetTables is called");

  if (databaseMetaData.Get() == nullptr) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "DatabaseMetaData object must be set.";

    LOG_ERROR_MSG(
        "DatabaseMetaDataGetTables exiting with error msg: " << errInfo.errMsg);

    return errInfo.code;
  }

  JNIEnv* env = Attach();
  jstring jCatalog = env->NewStringUTF(catalog.c_str());
  jstring jSchemaPattern = env->NewStringUTF(schemaPattern.c_str());
  jstring jTableNamePattern = env->NewStringUTF(tableNamePattern.c_str());
  jobjectArray jTypes =
      env->NewObjectArray(static_cast< jsize >(types.size()),
                          jvm->GetJavaMembers().c_String, nullptr);
  for (int i = 0; i < types.size(); i++) {
    env->SetObjectArrayElement(jTypes, i, env->NewStringUTF(types[i].c_str()));
  }

  jobject result = env->CallObjectMethod(
      databaseMetaData.Get()->GetRef(),
      jvm->GetMembers().m_DatabaseMetaDataGetTables, jCatalog, jSchemaPattern,
      jTableNamePattern, jTypes);
  ExceptionCheck(env, &errInfo);

  if (!result || errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    resultSet = nullptr;

    LOG_ERROR_MSG(
        "DatabaseMetaDataGetTables exiting with error. resultSet will be null");

    return errInfo.code;
  }

  resultSet = new GlobalJObject(env, env->NewGlobalRef(result));

  LOG_DEBUG_MSG("DatabaseMetaDataGetTables exiting");

  return errInfo.code;
}

JniErrorCode JniContext::DatabaseMetaDataGetColumns(
    const SharedPointer< GlobalJObject >& databaseMetaData,
    const std::string& catalog, const std::string& schemaPattern,
    const std::string& tableNamePattern, const std::string& columnNamePattern,
    SharedPointer< GlobalJObject >& resultSet, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("DatabaseMetaDataGetColumns is called");

  if (databaseMetaData.Get() == nullptr) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "DatabaseMetaData object must be set.";

    LOG_ERROR_MSG("DatabaseMetaDataGetColumns exiting with error msg: "
                  << errInfo.errMsg);

    return errInfo.code;
  }

  JNIEnv* env = Attach();
  jstring jCatalog = env->NewStringUTF(catalog.c_str());
  jstring jSchemaPattern = env->NewStringUTF(schemaPattern.c_str());
  jstring jTableNamePattern = env->NewStringUTF(tableNamePattern.c_str());
  jstring jColumnNamePattern = env->NewStringUTF(columnNamePattern.c_str());

  jobject result = env->CallObjectMethod(
      databaseMetaData.Get()->GetRef(),
      jvm->GetMembers().m_DatabaseMetaDataGetColumns, jCatalog, jSchemaPattern,
      jTableNamePattern, jColumnNamePattern);
  ExceptionCheck(env, &errInfo);

  if (!result || errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    resultSet = nullptr;
    LOG_ERROR_MSG(
        "DatabaseMetaDataGetColumns exiting with error. resultSet will be "
        "null");
    return errInfo.code;
  }

  resultSet = new GlobalJObject(env, env->NewGlobalRef(result));

  LOG_DEBUG_MSG("DatabaseMetaDataGetColumns exiting");

  return errInfo.code;
}

JniErrorCode JniContext::ResultSetClose(
    const SharedPointer< GlobalJObject >& resultSet, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("ResultSetClose is called");

  if (resultSet.Get() == nullptr) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "ResultSet object must be set.";

    LOG_ERROR_MSG("ResultSetClose exiting with error msg: " << errInfo.errMsg);

    return errInfo.code;
  }

  JNIEnv* env = Attach();
  env->CallVoidMethod(resultSet.Get()->GetRef(),
                      jvm->GetMembers().m_ResultSetClose);
  ExceptionCheck(env, &errInfo);

  LOG_DEBUG_MSG("ResultSetClose exiting");

  return errInfo.code;
}

JniErrorCode JniContext::ResultSetNext(
    const SharedPointer< GlobalJObject >& resultSet, bool& hasNext,
    JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("ResultSetNext is called");

  if (resultSet.Get() == nullptr) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "ResultSet object must be set.";

    LOG_ERROR_MSG("ResultSetNext exiting with error msg: " << errInfo.errMsg);

    return errInfo.code;
  }

  JNIEnv* env = Attach();
  jboolean res = env->CallBooleanMethod(resultSet.Get()->GetRef(),
                                        jvm->GetMembers().m_ResultSetNext);
  ExceptionCheck(env, &errInfo);
  if (errInfo.code == JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    hasNext = res != JNI_FALSE;
  }

  LOG_DEBUG_MSG("ResultSetNext exiting");

  return errInfo.code;
}

JniErrorCode JniContext::ResultSetGetString(
    const SharedPointer< GlobalJObject >& resultSet, int columnIndex,
    boost::optional< std::string >& value, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("ResultSetGetString is called");

  if (resultSet.Get() == nullptr) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "ResultSet object must be set.";

    LOG_ERROR_MSG(
        "ResultSetGetString exiting with error msg: " << errInfo.errMsg);

    return errInfo.code;
  }

  value = boost::none;
  JNIEnv* env = Attach();
  jobject result = env->CallObjectMethod(
      resultSet.Get()->GetRef(), jvm->GetMembers().m_ResultSetGetStringByIndex,
      columnIndex);
  ExceptionCheck(env, &errInfo);

  if (errInfo.code == JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    if (result != nullptr) {
      jboolean isCopy;
      const char* utfChars = env->GetStringUTFChars((jstring)result, &isCopy);
      value = std::string(utfChars);
      env->ReleaseStringUTFChars((jstring)result, utfChars);
    }
  }

  LOG_DEBUG_MSG("ResultSetGetString exiting");

  return errInfo.code;
}

JniErrorCode JniContext::ResultSetGetString(
    const SharedPointer< GlobalJObject >& resultSet,
    const std::string& columnName, boost::optional< std::string >& value,
    JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("ResultSetGetString is called");

  if (resultSet.Get() == nullptr) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "ResultSet object must be set.";

    LOG_ERROR_MSG(
        "ResultSetGetString exiting with error msg: " << errInfo.errMsg);

    return errInfo.code;
  }
  value = boost::none;
  JNIEnv* env = Attach();
  jstring jColumnName = env->NewStringUTF(columnName.c_str());
  jobject result = env->CallObjectMethod(
      resultSet.Get()->GetRef(), jvm->GetMembers().m_ResultSetGetStringByName,
      jColumnName);
  ExceptionCheck(env, &errInfo);

  if (errInfo.code == JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    if (result != nullptr) {
      jboolean isCopy;
      const char* utfChars = env->GetStringUTFChars((jstring)result, &isCopy);
      value = std::string(utfChars);
      env->ReleaseStringUTFChars((jstring)result, utfChars);
    }
  }

  LOG_DEBUG_MSG("ResultSetGetString exiting");

  return errInfo.code;
}

JniErrorCode JniContext::ResultSetGetInt(
    const SharedPointer< GlobalJObject >& resultSet, int columnIndex,
    boost::optional< int >& value, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("ResultSetGetInt is called");

  if (resultSet.Get() == nullptr) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "ResultSet object must be set.";

    LOG_ERROR_MSG("function exiting with error msg: " << errInfo.errMsg);

    return errInfo.code;
  }

  value = boost::none;
  JNIEnv* env = Attach();
  jint result = env->CallIntMethod(resultSet.Get()->GetRef(),
                                   jvm->GetMembers().m_ResultSetGetIntByIndex,
                                   columnIndex);
  ExceptionCheck(env, &errInfo);

  if (errInfo.code == JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    bool wasNull;
    errInfo.code = ResultSetWasNull(resultSet, wasNull, errInfo);
    if (!wasNull)
      value = result;
  }

  LOG_DEBUG_MSG("ResultSetGetInt exiting");

  return errInfo.code;
}

JniErrorCode JniContext::ResultSetGetInt(
    const SharedPointer< GlobalJObject >& resultSet,
    const std::string& columnName, boost::optional< int >& value,
    JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("ResultSetGetInt is called");

  if (resultSet.Get() == nullptr) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "ResultSet object must be set.";

    LOG_ERROR_MSG("ResultSetGetInt exiting with error msg: " << errInfo.errMsg);

    return errInfo.code;
  }

  value = boost::none;
  JNIEnv* env = Attach();
  jstring jColumnName = env->NewStringUTF(columnName.c_str());
  jint result = env->CallIntMethod(resultSet.Get()->GetRef(),
                                   jvm->GetMembers().m_ResultSetGetIntByName,
                                   jColumnName);
  ExceptionCheck(env, &errInfo);
  if (errInfo.code == JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    bool wasNull;
    errInfo.code = ResultSetWasNull(resultSet, wasNull, errInfo);
    if (!wasNull)
      value = result;
  }

  LOG_DEBUG_MSG("ResultSetGetInt exiting");

  return errInfo.code;
}

JniErrorCode JniContext::ResultSetGetRow(
    const SharedPointer< GlobalJObject >& resultSet,
    boost::optional< int >& value, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("ResultSetGetRow is called");

  if (resultSet.Get() == nullptr) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "ResultSet object must be set.";

    LOG_ERROR_MSG("ResultSetGetRow exiting with error msg: " << errInfo.errMsg);

    return errInfo.code;
  }

  value = boost::none;
  JNIEnv* env = Attach();
  jint result = env->CallIntMethod(resultSet.Get()->GetRef(),
                                   jvm->GetMembers().m_ResultSetGetRow);
  ExceptionCheck(env, &errInfo);
  if (errInfo.code == JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    value = result;
    bool wasNull;
    errInfo.code = ResultSetWasNull(resultSet, wasNull, errInfo);
    if (!wasNull)
      value = result;
  }

  LOG_DEBUG_MSG("ResultSetGetRow exiting");

  return errInfo.code;
}

JniErrorCode JniContext::ResultSetWasNull(
    const SharedPointer< GlobalJObject >& resultSet, bool& value,
    JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("ResultSetWasNull is called");

  if (resultSet.Get() == nullptr) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "ResultSet object must be set.";

    LOG_ERROR_MSG(
        "ResultSetWasNull exiting with error msg: " << errInfo.errMsg);

    return errInfo.code;
  }

  JNIEnv* env = Attach();
  jboolean res = env->CallBooleanMethod(resultSet.Get()->GetRef(),
                                        jvm->GetMembers().m_ResultSetWasNull);
  ExceptionCheck(env, &errInfo);
  if (errInfo.code == JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    value = res != JNI_FALSE;
  }

  LOG_DEBUG_MSG("ResultSetWasNull exiting");

  return errInfo.code;
}

JniErrorCode JniContext::ListSize(const SharedPointer< GlobalJObject >& list,
                                  int32_t& size, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("ListSize is called");

  if (!list.IsValid()) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "List object must be set.";

    LOG_ERROR_MSG("ListSize exiting with error msg: " << errInfo.errMsg);

    return errInfo.code;
  }

  JNIEnv* env = Attach();
  jint res =
      env->CallIntMethod(list.Get()->GetRef(), jvm->GetMembers().m_ListSize);
  ExceptionCheck(env, &errInfo);
  if (errInfo.code == JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    size = res;
  }

  LOG_DEBUG_MSG("ListSize exiting");

  return errInfo.code;
}

JniErrorCode JniContext::ListGet(const SharedPointer< GlobalJObject >& list,
                                 int32_t index,
                                 SharedPointer< GlobalJObject >& value,
                                 JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("ListGet is called");

  if (!list.IsValid()) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "List object must be set.";

    LOG_ERROR_MSG("ListGet exiting with error msg: " << errInfo.errMsg);

    return errInfo.code;
  }

  JNIEnv* env = Attach();
  jobject result = env->CallObjectMethod(list.Get()->GetRef(),
                                         jvm->GetMembers().m_ListGet, index);
  ExceptionCheck(env, &errInfo);
  if (!result || errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    value = nullptr;

    LOG_ERROR_MSG("ListGet exiting with error. value variable will be null");

    return errInfo.code;
  }
  value = new GlobalJObject(env, env->NewGlobalRef(result));

  LOG_DEBUG_MSG("ListGet exiting");

  return errInfo.code;
}

JniErrorCode
JniContext::DocumentdbMqlQueryContextGetAggregateOperationsAsStrings(
    const SharedPointer< GlobalJObject >& mqlQueryContext,
    SharedPointer< GlobalJObject >& list, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG(
      "DocumentdbMqlQueryContextGetAggregateOperationsAsStrings is called");

  if (!mqlQueryContext.IsValid()) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "MQL Query Context object must be set.";

    LOG_ERROR_MSG(
        "DocumentdbMqlQueryContextGetAggregateOperationsAsStrings exiting with "
        "error msg: "
        << errInfo.errMsg);

    return errInfo.code;
  }

  JNIEnv* env = Attach();
  jobject result = env->CallObjectMethod(
      mqlQueryContext.Get()->GetRef(),
      jvm->GetMembers()
          .m_DocumentDbMqlQueryContextGetAggregateOperationsAsStrings);
  ExceptionCheck(env, &errInfo);
  if (!result || errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    list = nullptr;

    LOG_ERROR_MSG(
        "DocumentdbMqlQueryContextGetAggregateOperationsAsStrings exiting with "
        "error. list variable will be null");

    return errInfo.code;
  }
  list = new GlobalJObject(env, env->NewGlobalRef(result));

  LOG_DEBUG_MSG(
      "DocumentdbMqlQueryContextGetAggregateOperationsAsStrings exiting");

  return errInfo.code;
}

JniErrorCode JniContext::DocumentdbMqlQueryContextGetColumnMetadata(
    const SharedPointer< GlobalJObject >& mqlQueryContext,
    SharedPointer< GlobalJObject >& columnMetadata, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("DocumentdbMqlQueryContextGetColumnMetadata is called");

  if (!mqlQueryContext.IsValid()) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "MQL Query Context object must be set.";

    LOG_ERROR_MSG(
        "DocumentdbMqlQueryContextGetColumnMetadata exiting with error msg: "
        << errInfo.errMsg);

    return errInfo.code;
  }

  JNIEnv* env = Attach();
  jobject result = env->CallObjectMethod(
      mqlQueryContext.Get()->GetRef(),
      jvm->GetMembers().m_DocumentDbMqlQueryContextGetColumnMetadata);
  ExceptionCheck(env, &errInfo);
  if (!result || errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    columnMetadata = nullptr;

    LOG_ERROR_MSG(
        "DocumentdbMqlQueryContextGetColumnMetadata exiting with error. "
        "columnMetadata variable will be null");

    return errInfo.code;
  }
  columnMetadata = new GlobalJObject(env, env->NewGlobalRef(result));

  LOG_DEBUG_MSG("DocumentdbMqlQueryContextGetColumnMetadata exiting");

  return errInfo.code;
}

JniErrorCode JniContext::DocumentdbMqlQueryContextGetCollectionName(
    const SharedPointer< GlobalJObject >& mqlQueryContext,
    std::string& collectionName, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("DocumentdbMqlQueryContextGetCollectionName is called");

  if (!mqlQueryContext.IsValid()) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "MQL Query Context object must be set.";

    LOG_ERROR_MSG(
        "DocumentdbMqlQueryContextGetCollectionName exiting with error msg: "
        << errInfo.errMsg);

    return errInfo.code;
  }

  JNIEnv* env = Attach();
  jstring result = static_cast< jstring >(env->CallObjectMethod(
      mqlQueryContext.Get()->GetRef(),
      jvm->GetMembers().m_DocumentDbMqlQueryContextGetCollectionName));
  ExceptionCheck(env, &errInfo);
  if (!result || errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    collectionName = "";

    LOG_ERROR_MSG(
        "DocumentdbMqlQueryContextGetCollectionName exiting with error. "
        "collectionName variable will be null");

    return errInfo.code;
  }
  int len;
  collectionName = JavaStringToCString(env, result, len);

  LOG_DEBUG_MSG("DocumentdbMqlQueryContextGetCollectionName exiting");

  return errInfo.code;
}

JniErrorCode JniContext::DocumentdbMqlQueryContextGetPaths(
    const SharedPointer< GlobalJObject >& mqlQueryContext,
    SharedPointer< GlobalJObject >& list, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("DocumentdbMqlQueryContextGetPaths is called");

  if (!mqlQueryContext.IsValid()) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "MQL Query Context object must be set.";

    LOG_ERROR_MSG("DocumentdbMqlQueryContextGetPaths exiting with error msg: "
                  << errInfo.errMsg);

    return errInfo.code;
  }

  JNIEnv* env = Attach();
  jobject result = env->CallObjectMethod(
      mqlQueryContext.Get()->GetRef(),
      jvm->GetMembers().m_DocumentDbMqlQueryContextGetPaths);
  ExceptionCheck(env, &errInfo);
  if (!result || errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    list = nullptr;

    LOG_ERROR_MSG(
        "DocumentdbMqlQueryContextGetPaths exiting with error. list variable "
        "will be null");

    return errInfo.code;
  }
  list = new GlobalJObject(env, env->NewGlobalRef(result));

  LOG_DEBUG_MSG("DocumentdbMqlQueryContextGetPaths exiting");

  return errInfo.code;
}

JniErrorCode JniContext::DocumentDbQueryMappingServiceCtor(
    const SharedPointer< GlobalJObject >& connectionProperties,
    const SharedPointer< GlobalJObject >& databaseMetadata,
    SharedPointer< GlobalJObject >& queryMappingService,
    JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("DocumentDbQueryMappingServiceCtor is called");

  if (!connectionProperties.IsValid() || !databaseMetadata.IsValid()) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg =
        "Connection Properties and Database Metadata objects must be set.";

    LOG_ERROR_MSG("DocumentDbQueryMappingServiceCtor exiting with error msg: "
                  << errInfo.errMsg);

    return errInfo.code;
  }

  JNIEnv* env = Attach();
  jobject result = env->NewObject(
      jvm->GetMembers().c_DocumentDbQueryMappingService,
      jvm->GetMembers().m_DocumentDbQueryMappingServiceCtor,
      connectionProperties.Get()->GetRef(), databaseMetadata.Get()->GetRef());
  ExceptionCheck(env, &errInfo);
  if (!result || errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    queryMappingService = nullptr;

    LOG_ERROR_MSG(
        "DocumentDbQueryMappingServiceCtor exiting with error. "
        "queryMappingService variable will be null");

    return errInfo.code;
  }
  queryMappingService = new GlobalJObject(env, env->NewGlobalRef(result));

  LOG_DEBUG_MSG("DocumentDbQueryMappingServiceCtor exiting");

  return errInfo.code;
}

JniErrorCode JniContext::DocumentDbQueryMappingServiceGet(
    const SharedPointer< GlobalJObject >& queryMappingService,
    const std::string sql, int64_t maxRowCount,
    SharedPointer< GlobalJObject >& mqlQueryContext, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("DocumentDbQueryMappingServiceGet is called");

  if (!queryMappingService.IsValid()) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "Query Mapping Service object must be set.";

    LOG_ERROR_MSG("DocumentDbQueryMappingServiceGet exiting with error msg: "
                  << errInfo.errMsg);

    return errInfo.code;
  }

  JNIEnv* env = Attach();
  jstring sqlString = env->NewStringUTF(sql.c_str());
  jlong maxRowCountLong = maxRowCount;
  jobject result = env->CallObjectMethod(
      queryMappingService.Get()->GetRef(),
      jvm->GetMembers().m_DocumentDbQueryMappingServiceGet, sqlString,
      maxRowCountLong);
  env->DeleteLocalRef(sqlString);
  ExceptionCheck(env, &errInfo);
  if (!result || errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    mqlQueryContext = nullptr;

    LOG_ERROR_MSG(
        "DocumentDbQueryMappingServiceGet exiting with error. mqlQueryContext "
        "variable will be null");

    return errInfo.code;
  }
  mqlQueryContext = new GlobalJObject(env, env->NewGlobalRef(result));

  LOG_DEBUG_MSG("DocumentDbQueryMappingServiceGet exiting");

  return errInfo.code;
}

JniErrorCode JniContext::JdbcColumnMetadataGetOrdinal(
    const SharedPointer< GlobalJObject >& jdbcColumnMetadata, int32_t& ordinal,
    JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("JdbcColumnMetadataGetOrdinal is called");

  if (!jdbcColumnMetadata.IsValid()) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "JDBC Column Metaata object must be set.";

    LOG_ERROR_MSG("JdbcColumnMetadataGetOrdinal exiting with error msg: "
                  << errInfo.errMsg);

    return errInfo.code;
  }

  JNIEnv* env = Attach();
  jint result =
      env->CallIntMethod(jdbcColumnMetadata.Get()->GetRef(),
                         jvm->GetMembers().m_JdbcColumnMetadataGetOrdinal);
  ExceptionCheck(env, &errInfo);
  if (errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    LOG_ERROR_MSG(
        "JdbcColumnMetadataGetOrdinal exiting with error. ordinal variable "
        "will not be set");
    return errInfo.code;
  }

  ordinal = result;

  LOG_DEBUG_MSG("JdbcColumnMetadataGetOrdinal exiting");

  return errInfo.code;
}

JniErrorCode JniContext::CallBooleanMethod(
    const SharedPointer< GlobalJObject >& object, const jmethodID& method,
    bool& value, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("CallBooleanMethod is called");

  if (!object.IsValid()) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "JDBC Column Metadata object must be set.";

    LOG_ERROR_MSG(
        "CallBooleanMethod exiting with error msg: " << errInfo.errMsg);

    return errInfo.code;
  }

  JNIEnv* env = Attach();
  jboolean result = env->CallBooleanMethod(object.Get()->GetRef(), method);
  ExceptionCheck(env, &errInfo);
  if (errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    LOG_ERROR_MSG(
        "CallBooleanMethod exiting with error. value variable will not be set");
    return errInfo.code;
  }

  value = result;

  LOG_DEBUG_MSG("CallBooleanMethod exiting");

  return errInfo.code;
}

JniErrorCode JniContext::CallIntMethod(
    const SharedPointer< GlobalJObject >& object, const jmethodID& method,
    int32_t& value, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("CallIntMethod is called");

  if (!object.IsValid()) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "JDBC Column Metadata object must be set.";

    LOG_ERROR_MSG("CallIntMethod exiting with error msg: " << errInfo.errMsg);

    return errInfo.code;
  }

  JNIEnv* env = Attach();
  jint result = env->CallIntMethod(object.Get()->GetRef(), method);
  ExceptionCheck(env, &errInfo);
  if (errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    LOG_ERROR_MSG(
        "CallIntMethod exiting with error. value variable will not be set");

    return errInfo.code;
  }

  value = result;

  LOG_DEBUG_MSG("function exiting");

  return errInfo.code;
}

JniErrorCode JniContext::CallStringMethod(
    const SharedPointer< GlobalJObject >& object, const jmethodID& method,
    boost::optional< std::string >& value, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("CallStringMethod is called");

  if (!object.IsValid()) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "JDBC Column Metadata object must be set.";

    LOG_ERROR_MSG(
        "CallStringMethod exiting with error msg: " << errInfo.errMsg);

    return errInfo.code;
  }

  JNIEnv* env = Attach();
  auto result = static_cast< jstring >(
      env->CallObjectMethod(object.Get()->GetRef(), method));
  ExceptionCheck(env, &errInfo);
  if (errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    LOG_ERROR_MSG(
        "CallStringMethod exiting with error. value variable will not be set");

    return errInfo.code;
  }

  if (result == nullptr) {
    value = boost::none;
  } else {
    int len;
    value = JavaStringToCString(env, result, len);
  }

  LOG_DEBUG_MSG("CallStringMethod exiting");

  return errInfo.code;
}

JniErrorCode JniContext::JdbcColumnMetadataIsAutoIncrement(
    const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
    bool& autoIncrement, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("JdbcColumnMetadataIsAutoIncrement is called, and exiting");

  return CallBooleanMethod(
      jdbcColumnMetadata, jvm->GetMembers().m_JdbcColumnMetadataIsAutoIncrement,
      autoIncrement, errInfo);
}

JniErrorCode JniContext::JdbcColumnMetadataIsCaseSensitive(
    const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
    bool& caseSensitive, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("JdbcColumnMetadataIsCaseSensitive is called, and exiting");

  return CallBooleanMethod(
      jdbcColumnMetadata, jvm->GetMembers().m_JdbcColumnMetadataIsCaseSensitive,
      caseSensitive, errInfo);
}

JniErrorCode JniContext::JdbcColumnMetadataIsSearchable(
    const SharedPointer< GlobalJObject >& jdbcColumnMetadata, bool& searchable,
    JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("JdbcColumnMetadataIsSearchable is called, and exiting");

  return CallBooleanMethod(jdbcColumnMetadata,
                           jvm->GetMembers().m_JdbcColumnMetadataIsSearchable,
                           searchable, errInfo);
}

JniErrorCode JniContext::JdbcColumnMetadataIsCurrency(
    const SharedPointer< GlobalJObject >& jdbcColumnMetadata, bool& currency,
    JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("JdbcColumnMetadataIsCurrency is called, and exiting");

  return CallBooleanMethod(jdbcColumnMetadata,
                           jvm->GetMembers().m_JdbcColumnMetadataIsCurrency,
                           currency, errInfo);
}

JniErrorCode JniContext::JdbcColumnMetadataGetNullable(
    const SharedPointer< GlobalJObject >& jdbcColumnMetadata, int32_t& nullable,
    JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("JdbcColumnMetadataGetNullable is called, and exiting");

  return CallIntMethod(jdbcColumnMetadata,
                       jvm->GetMembers().m_JdbcColumnMetadataGetNullable,
                       nullable, errInfo);
}

JniErrorCode JniContext::JdbcColumnMetadataIsSigned(
    const SharedPointer< GlobalJObject >& jdbcColumnMetadata, bool& isSigned,
    JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("JdbcColumnMetadataIsSigned is called, and exiting");

  return CallBooleanMethod(jdbcColumnMetadata,
                           jvm->GetMembers().m_JdbcColumnMetadataIsSigned,
                           isSigned, errInfo);
}

JniErrorCode JniContext::JdbcColumnMetadataGetColumnDisplaySize(
    const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
    int32_t& columnDisplaySize, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG(
      "JdbcColumnMetadataGetColumnDisplaySize is called, and exiting");

  return CallIntMethod(
      jdbcColumnMetadata,
      jvm->GetMembers().m_JdbcColumnMetadataGetColumnDisplaySize,
      columnDisplaySize, errInfo);
}

JniErrorCode JniContext::JdbcColumnMetadataGetColumnLabel(
    const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
    boost::optional< std::string >& columnLabel, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("JdbcColumnMetadataGetColumnLabel is called, and exiting");

  return CallStringMethod(jdbcColumnMetadata,
                          jvm->GetMembers().m_JdbcColumnMetadataGetColumnLabel,
                          columnLabel, errInfo);
}

JniErrorCode JniContext::JdbcColumnMetadataGetColumnName(
    const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
    boost::optional< std::string >& columnName, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("JdbcColumnMetadataGetColumnName is called, and exiting");

  return CallStringMethod(jdbcColumnMetadata,
                          jvm->GetMembers().m_JdbcColumnMetadataGetColumnName,
                          columnName, errInfo);
}

JniErrorCode JniContext::JdbcColumnMetadataGetSchemaName(
    const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
    boost::optional< std::string >& schemaName, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("JdbcColumnMetadataGetSchemaName is called, and exiting");

  return CallStringMethod(jdbcColumnMetadata,
                          jvm->GetMembers().m_JdbcColumnMetadataGetSchemaName,
                          schemaName, errInfo);
}

JniErrorCode JniContext::JdbcColumnMetadataGetPrecision(
    const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
    int32_t& precision, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("JdbcColumnMetadataGetPrecision is called, and exiting");

  return CallIntMethod(jdbcColumnMetadata,
                       jvm->GetMembers().m_JdbcColumnMetadataGetPrecision,
                       precision, errInfo);
}

JniErrorCode JniContext::JdbcColumnMetadataGetScale(
    const SharedPointer< GlobalJObject >& jdbcColumnMetadata, int32_t& scale,
    JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("JdbcColumnMetadataGetScale is called, and exiting");

  return CallIntMethod(jdbcColumnMetadata,
                       jvm->GetMembers().m_JdbcColumnMetadataGetScale, scale,
                       errInfo);
}

JniErrorCode JniContext::JdbcColumnMetadataGetTableName(
    const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
    boost::optional< std::string >& tableName, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("JdbcColumnMetadataGetTableName is called, and exiting");

  return CallStringMethod(jdbcColumnMetadata,
                          jvm->GetMembers().m_JdbcColumnMetadataGetTableName,
                          tableName, errInfo);
}

JniErrorCode JniContext::JdbcColumnMetadataGetCatalogName(
    const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
    boost::optional< std::string >& catalogName, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("JdbcColumnMetadataGetCatalogName is called, and exiting");

  return CallStringMethod(jdbcColumnMetadata,
                          jvm->GetMembers().m_JdbcColumnMetadataGetCatalogName,
                          catalogName, errInfo);
}

JniErrorCode JniContext::JdbcColumnMetadataGetColumnType(
    const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
    int32_t& columnType, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("JdbcColumnMetadataGetColumnType is called, and exiting");

  return CallIntMethod(jdbcColumnMetadata,
                       jvm->GetMembers().m_JdbcColumnMetadataGetColumnType,
                       columnType, errInfo);
}

JniErrorCode JniContext::JdbcColumnMetadataGetColumnTypeName(
    const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
    boost::optional< std::string >& columnTypeName, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("JdbcColumnMetadataGetColumnTypeName is called, and exiting");

  return CallStringMethod(
      jdbcColumnMetadata,
      jvm->GetMembers().m_JdbcColumnMetadataGetColumnTypeName, columnTypeName,
      errInfo);
}

JniErrorCode JniContext::JdbcColumnMetadataIsReadOnly(
    const SharedPointer< GlobalJObject >& jdbcColumnMetadata, bool& readOnly,
    JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("JdbcColumnMetadataIsReadOnly is called, and exiting");

  return CallBooleanMethod(jdbcColumnMetadata,
                           jvm->GetMembers().m_JdbcColumnMetadataIsReadOnly,
                           readOnly, errInfo);
}

JniErrorCode JniContext::JdbcColumnMetadataIsWritable(
    const SharedPointer< GlobalJObject >& jdbcColumnMetadata, bool& writable,
    JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("JdbcColumnMetadataIsWritable is called, and exiting");

  return CallBooleanMethod(jdbcColumnMetadata,
                           jvm->GetMembers().m_JdbcColumnMetadataIsWritable,
                           writable, errInfo);
}

JniErrorCode JniContext::JdbcColumnMetadataIsDefinitelyWritable(
    const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
    bool& definitelyWritable, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG(
      "JdbcColumnMetadataIsDefinitelyWritable is called, and exiting");

  return CallBooleanMethod(
      jdbcColumnMetadata,
      jvm->GetMembers().m_JdbcColumnMetadataIsDefinitelyWritable,
      definitelyWritable, errInfo);
}

JniErrorCode JniContext::JdbcColumnMetadataGetColumnClassName(
    const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
    boost::optional< std::string >& columnClassName, JniErrorInfo& errInfo) {
  LOG_DEBUG_MSG("JdbcColumnMetadataGetColumnClassName is called, and exiting");

  return CallStringMethod(
      jdbcColumnMetadata,
      jvm->GetMembers().m_JdbcColumnMetadataGetColumnClassName, columnClassName,
      errInfo);
}

int64_t JniContext::TargetInLongOutLong(jobject obj, int opType, int64_t val,
                                        JniErrorInfo* err) {
  LOG_DEBUG_MSG("TargetInLongOutLong is called");

  JNIEnv* env = Attach();

  int64_t res = env->CallLongMethod(
      obj, jvm->GetMembers().m_PlatformTarget_inLongOutLong, opType, val);

  ExceptionCheck(env, err);

  LOG_DEBUG_MSG("TargetInLongOutLong exiting");

  return res;
}

int64_t JniContext::TargetInStreamOutLong(jobject obj, int opType,
                                          int64_t memPtr, JniErrorInfo* err) {
  LOG_DEBUG_MSG("TargetInStreamOutLong is called");

  JNIEnv* env = Attach();

  int64_t res = env->CallLongMethod(
      obj, jvm->GetMembers().m_PlatformTarget_inStreamOutLong, opType, memPtr);

  ExceptionCheck(env, err);

  LOG_DEBUG_MSG("TargetInStreamOutLong exiting");

  return res;
}

void JniContext::TargetInStreamOutStream(jobject obj, int opType,
                                         int64_t inMemPtr, int64_t outMemPtr,
                                         JniErrorInfo* err) {
  LOG_DEBUG_MSG("TargetInStreamOutStream is called");

  JNIEnv* env = Attach();

  env->CallVoidMethod(obj, jvm->GetMembers().m_PlatformTarget_inStreamOutStream,
                      opType, inMemPtr, outMemPtr);

  ExceptionCheck(env, err);

  LOG_DEBUG_MSG("TargetInStreamOutStream exiting");
}

jobject JniContext::TargetInStreamOutObject(jobject obj, int opType,
                                            int64_t memPtr, JniErrorInfo* err) {
  LOG_DEBUG_MSG("TargetInStreamOutObject is called");

  JNIEnv* env = Attach();

  jobject res = env->CallObjectMethod(
      obj, jvm->GetMembers().m_PlatformTarget_inStreamOutObject, opType,
      memPtr);

  ExceptionCheck(env, err);

  LOG_DEBUG_MSG("TargetInStreamOutObject exiting");

  return LocalToGlobal(env, res);
}

jobject JniContext::TargetInObjectStreamOutObjectStream(jobject obj, int opType,
                                                        void* arg,
                                                        int64_t inMemPtr,
                                                        int64_t outMemPtr,
                                                        JniErrorInfo* err) {
  LOG_DEBUG_MSG("TargetInObjectStreamOutObjectStream is called");

  JNIEnv* env = Attach();

  jobject res = env->CallObjectMethod(
      obj, jvm->GetMembers().m_PlatformTarget_inObjectStreamOutObjectStream,
      opType, arg, inMemPtr, outMemPtr);

  ExceptionCheck(env, err);

  LOG_DEBUG_MSG("TargetInObjectStreamOutObjectStream exiting");

  return LocalToGlobal(env, res);
}

void JniContext::TargetOutStream(jobject obj, int opType, int64_t memPtr,
                                 JniErrorInfo* err) {
  JNIEnv* env = Attach();

  env->CallVoidMethod(obj, jvm->GetMembers().m_PlatformTarget_outStream, opType,
                      memPtr);

  ExceptionCheck(env, err);
}

jobject JniContext::TargetOutObject(jobject obj, int opType,
                                    JniErrorInfo* err) {
  LOG_DEBUG_MSG("TargetOutObject is called");

  JNIEnv* env = Attach();

  jobject res = env->CallObjectMethod(
      obj, jvm->GetMembers().m_PlatformTarget_outObject, opType);

  ExceptionCheck(env, err);

  LOG_DEBUG_MSG("TargetOutObject exiting");

  return LocalToGlobal(env, res);
}

void JniContext::TargetInStreamAsync(jobject obj, int opType, int64_t memPtr,
                                     JniErrorInfo* err) {
  LOG_DEBUG_MSG("TargetInStreamAsync is called");

  JNIEnv* env = Attach();

  env->CallVoidMethod(obj, jvm->GetMembers().m_PlatformTarget_inStreamAsync,
                      opType, memPtr);

  ExceptionCheck(env, err);

  LOG_DEBUG_MSG("TargetInStreamAsync exiting");
}

jobject JniContext::TargetInStreamOutObjectAsync(jobject obj, int opType,
                                                 int64_t memPtr,
                                                 JniErrorInfo* err) {
  LOG_DEBUG_MSG("TargetInStreamOutObjectAsync is called");

  JNIEnv* env = Attach();

  jobject res = env->CallObjectMethod(
      obj, jvm->GetMembers().m_PlatformTarget_inStreamOutObjectAsync, opType,
      memPtr);

  ExceptionCheck(env, err);

  LOG_DEBUG_MSG("TargetInStreamOutObjectAsync exiting");

  return LocalToGlobal(env, res);
}

jobject JniContext::CacheOutOpQueryCursor(jobject obj, int type, int64_t memPtr,
                                          JniErrorInfo* err) {
  LOG_DEBUG_MSG("CacheOutOpQueryCursor is called");

  JNIEnv* env = Attach();

  jobject res = env->CallObjectMethod(
      obj, jvm->GetMembers().m_PlatformTarget_inStreamOutObject, type, memPtr);

  ExceptionCheck(env, err);

  LOG_DEBUG_MSG("CacheOutOpQueryCursor exiting");

  return LocalToGlobal(env, res);
}

jobject JniContext::CacheOutOpContinuousQuery(jobject obj, int type,
                                              int64_t memPtr,
                                              JniErrorInfo* err) {
  LOG_DEBUG_MSG("CacheOutOpContinuousQuery is called");

  JNIEnv* env = Attach();

  jobject res = env->CallObjectMethod(
      obj, jvm->GetMembers().m_PlatformTarget_inStreamOutObject, type, memPtr);

  ExceptionCheck(env, err);

  LOG_DEBUG_MSG("CacheOutOpContinuousQuery exiting");

  return LocalToGlobal(env, res);
}

jobject JniContext::Acquire(jobject obj) {
  LOG_DEBUG_MSG("Acquire is called");

  if (obj) {
    JNIEnv* env = Attach();

    jobject obj0 = env->NewGlobalRef(obj);

    ExceptionCheck(env);

    LOG_DEBUG_MSG("Acquire exiting with obj0 variable.");

    return obj0;
  }

  LOG_ERROR_MSG("Acquire exiting with nullptr.");

  return nullptr;
}

void JniContext::Release(jobject obj) {
  LOG_DEBUG_MSG("Release is called");

  if (obj) {
    JavaVM* jvm = JVM.GetJvm();

    if (jvm) {
      JNIEnv* env;

      jint attachRes =
          jvm->AttachCurrentThread(reinterpret_cast< void** >(&env), nullptr);

      if (attachRes == JNI_OK) {
        AttachHelper::OnThreadAttach();

        env->DeleteGlobalRef(obj);
      }
    }
  }
  LOG_DEBUG_MSG("Release exiting");
}

void JniContext::SetConsoleHandler(ConsoleWriteHandler consoleHandler) {
  LOG_DEBUG_MSG("SetConsoleHandler is called");

  if (!consoleHandler)
    throw std::invalid_argument("consoleHandler can not be null");

  CONSOLE_LOCK.Enter();

  consoleWriteHandlers.push_back(consoleHandler);

  CONSOLE_LOCK.Leave();

  LOG_DEBUG_MSG("SetConsoleHandler exiting");
}

int JniContext::RemoveConsoleHandler(ConsoleWriteHandler consoleHandler) {
  LOG_DEBUG_MSG("RemoveConsoleHandler is called");

  if (!consoleHandler)
    throw std::invalid_argument("consoleHandler can not be null");

  CONSOLE_LOCK.Enter();

  int oldSize = static_cast< int >(consoleWriteHandlers.size());

  consoleWriteHandlers.erase(remove(consoleWriteHandlers.begin(),
                                    consoleWriteHandlers.end(), consoleHandler),
                             consoleWriteHandlers.end());

  int removedCnt = oldSize - static_cast< int >(consoleWriteHandlers.size());

  CONSOLE_LOCK.Leave();

  LOG_DEBUG_MSG("RemoveConsoleHandler exiting");

  return removedCnt;
}

void JniContext::ThrowToJava(char* msg) {
  LOG_DEBUG_MSG("ThrowToJava is called");

  JNIEnv* env = Attach();

  env->ThrowNew(jvm->GetMembers().c_IgniteException, msg);

  LOG_DEBUG_MSG("ThrowToJava exiting");
}

void JniContext::DestroyJvm() {
  LOG_DEBUG_MSG("DestroyJvm is called");

  jvm->GetJvm()->DestroyJavaVM();

  LOG_DEBUG_MSG("DestroyJvm exiting");
}

/**
 * Attach thread to JVM.
 */
JNIEnv* JniContext::Attach() {
  LOG_DEBUG_MSG("Attach is called");

  JNIEnv* env;

  jint attachRes = jvm->GetJvm()->AttachCurrentThread(
      reinterpret_cast< void** >(&env), nullptr);

  if (attachRes == JNI_OK)
    AttachHelper::OnThreadAttach();
  else {
    if (hnds.error)
      hnds.error(hnds.target, JniErrorCode::IGNITE_JNI_ERR_JVM_ATTACH, nullptr,
                 0, nullptr, 0, nullptr, 0, nullptr, 0);
  }
  LOG_DEBUG_MSG("Attach exiting");

  return env;
}

void JniContext::ExceptionCheck(JNIEnv* env) {
  LOG_DEBUG_MSG("ExceptionCheck(JNIEnv* env) is called");

  ExceptionCheck(env, nullptr);

  LOG_DEBUG_MSG("ExceptionCheck(JNIEnv* env) exiting");
}

void JniContext::ExceptionCheck(JNIEnv* env, JniErrorInfo* errInfo) {
  LOG_DEBUG_MSG("ExceptionCheck(JNIEnv* env, JniErrorInfo* errInfo) is called");

  if (env->ExceptionCheck()) {
    jthrowable err = env->ExceptionOccurred();

    if (PRINT_EXCEPTION)
      env->CallVoidMethod(err,
                          jvm->GetJavaMembers().m_Throwable_printStackTrace);

    env->ExceptionClear();

    // Get error class name and message.
    jclass cls = env->GetObjectClass(err);

    jstring clsName = static_cast< jstring >(
        env->CallObjectMethod(cls, jvm->GetJavaMembers().m_Class_getName));
    jstring msg = static_cast< jstring >(env->CallObjectMethod(
        err, jvm->GetJavaMembers().m_Throwable_getMessage));
    int traceLen = 0;
    std::string trace0 = "";
    if (jvm->GetJavaMembers().c_PlatformUtils
        && jvm->GetJavaMembers().m_PlatformUtils_getFullStackTrace) {
      jstring trace = static_cast< jstring >(env->CallStaticObjectMethod(
          jvm->GetJavaMembers().c_PlatformUtils,
          jvm->GetJavaMembers().m_PlatformUtils_getFullStackTrace, err));

      trace0 = JavaStringToCString(env, trace, traceLen);
    }

    env->DeleteLocalRef(cls);

    int clsNameLen;
    std::string clsName0 = JavaStringToCString(env, clsName, clsNameLen);

    int msgLen;
    std::string msg0 = JavaStringToCString(env, msg, msgLen);

    if (errInfo) {
      JniErrorInfo errInfo0(JniErrorCode::IGNITE_JNI_ERR_GENERIC,
                            clsName0.c_str(), msg0.c_str());

      *errInfo = errInfo0;
    }

    // Get error additional data (if any).
    jbyteArray errData = nullptr;
    if (jvm->GetMembers().c_PlatformUtils
        && jvm->GetMembers().m_PlatformUtils_errData) {
      errData = static_cast< jbyteArray >(env->CallStaticObjectMethod(
          jvm->GetMembers().c_PlatformUtils,
          jvm->GetMembers().m_PlatformUtils_errData, err));
    }

    if (errData) {
      jbyte* errBytesNative = env->GetByteArrayElements(errData, nullptr);

      int errBytesLen = env->GetArrayLength(errData);

      if (hnds.error)
        hnds.error(hnds.target, JniErrorCode::IGNITE_JNI_ERR_GENERIC,
                   clsName0.c_str(), clsNameLen, msg0.c_str(), msgLen,
                   trace0.c_str(), traceLen, errBytesNative, errBytesLen);

      env->ReleaseByteArrayElements(errData, errBytesNative, JNI_ABORT);
    } else {
      if (hnds.error)
        hnds.error(hnds.target, JniErrorCode::IGNITE_JNI_ERR_GENERIC,
                   clsName0.c_str(), clsNameLen, msg0.c_str(), msgLen,
                   trace0.c_str(), traceLen, nullptr, 0);
    }

    env->DeleteLocalRef(err);
  } else if (errInfo) {
    JniErrorInfo errInfo0(JniErrorCode::IGNITE_JNI_ERR_SUCCESS, "", "");
    *errInfo = errInfo0;
  }
  LOG_DEBUG_MSG("ExceptionCheck(JNIEnv* env, JniErrorInfo* errInfo) exiting");
}

/**
 * Convert local reference to global.
 */
jobject JniContext::LocalToGlobal(JNIEnv* env, jobject localRef) {
  LOG_DEBUG_MSG("LocalToGlobal is called");

  if (localRef) {
    jobject globalRef = env->NewGlobalRef(localRef);

    env->DeleteLocalRef(localRef);  // Clear local ref irrespective of result.

    if (!globalRef)
      ExceptionCheck(env);

    LOG_DEBUG_MSG("LocalToGlobal exiting with globalRef variable");

    return globalRef;
  } else {
    LOG_DEBUG_MSG(
        "LocalToGlobal exiting with nullptr because localRef variable is null");

    return nullptr;
  }
}

JNIEXPORT void JNICALL JniConsoleWrite(JNIEnv* env, jclass, jstring str,
                                       jboolean isErr) {
  LOG_DEBUG_MSG("JniConsoleWrite is called");

  CONSOLE_LOCK.Enter();

  if (consoleWriteHandlers.size() > 0) {
    ConsoleWriteHandler consoleWrite = consoleWriteHandlers.at(0);

    const char* strChars = env->GetStringUTFChars(str, 0);
    const int strCharsLen = env->GetStringUTFLength(str);

    consoleWrite(strChars, strCharsLen, isErr);

    env->ReleaseStringUTFChars(str, strChars);
  }

  CONSOLE_LOCK.Leave();

  LOG_DEBUG_MSG("JniConsoleWrite exiting");
}

JNIEXPORT void JNICALL JniLoggerLog(JNIEnv* env, jclass, jlong envPtr,
                                    jint level, jstring message,
                                    jstring category, jstring errorInfo,
                                    jlong memPtr) {
  LOG_DEBUG_MSG("JniLoggerLog is called");

  int messageLen;
  char* messageChars = StringToChars(env, message, &messageLen);

  int categoryLen;
  char* categoryChars = StringToChars(env, category, &categoryLen);

  int errorInfoLen;
  char* errorInfoChars = StringToChars(env, errorInfo, &errorInfoLen);

  IGNITE_SAFE_PROC(env, envPtr, LoggerLogHandler, loggerLog, level,
                   messageChars, messageLen, categoryChars, categoryLen,
                   errorInfoChars, errorInfoLen, memPtr);

  if (messageChars)
    delete[] messageChars;

  if (categoryChars)
    delete[] categoryChars;

  if (errorInfoChars)
    delete[] errorInfoChars;

  LOG_DEBUG_MSG("JniLoggerLog exiting");
}

JNIEXPORT jboolean JNICALL JniLoggerIsLevelEnabled(JNIEnv* env, jclass,
                                                   jlong envPtr, jint level) {
  LOG_DEBUG_MSG("JniLoggerIsLevelEnabled is called");

  IGNITE_SAFE_FUNC(env, envPtr, LoggerIsLevelEnabledHandler,
                   loggerIsLevelEnabled, level);

  LOG_DEBUG_MSG("JniLoggerIsLevelEnabled exiting");
}

JNIEXPORT jlong JNICALL JniInLongOutLong(JNIEnv* env, jclass, jlong envPtr,
                                         jint type, jlong val) {
  LOG_DEBUG_MSG("JniInLongOutLong is called");

  IGNITE_SAFE_FUNC(env, envPtr, InLongOutLongHandler, inLongOutLong, type, val);

  LOG_DEBUG_MSG("JniInLongOutLong exiting");
}

JNIEXPORT jlong JNICALL JniInLongLongLongObjectOutLong(JNIEnv* env, jclass,
                                                       jlong envPtr, jint type,
                                                       jlong val1, jlong val2,
                                                       jlong val3,
                                                       jobject arg) {
  LOG_DEBUG_MSG("JniInLongLongLongObjectOutLong is called");

  IGNITE_SAFE_FUNC(env, envPtr, InLongLongLongObjectOutLongHandler,
                   inLongLongLongObjectOutLong, type, val1, val2, val3, arg);

  LOG_DEBUG_MSG("JniInLongLongLongObjectOutLong exiting");
}
}  //  namespace java
}  //  namespace jni
}  //  namespace odbc
}  //  namespace ignite
