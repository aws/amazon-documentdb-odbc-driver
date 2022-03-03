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

#include <algorithm>
#include <cstring>  // needed only on linux
#include <exception>
#include <stdexcept>
#include <string>
#include <vector>

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
  JavaVMInitArgs args;

  memset(&args, 0, sizeof(args));

  args.version = JNI_VERSION_9;

  return JNI_GetDefaultJavaVMInitArgs(&args) == JNI_OK;
}

void BuildJvmOptions(const std::string& cp, std::vector< char* >& opts, int xms,
                     int xmx) {
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
  if (this != &other) {
    // 1. Create new instance, exception could occur at this point.
    JniErrorInfo tmp(other);

    // 2. Swap with temp.
    std::swap(code, tmp.code);
    std::swap(errCls, tmp.errCls);
    std::swap(errMsg, tmp.errMsg);
  }

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
  if (!str) {
    *len = 0;
    return NULL;
  }

  const char* strChars = env->GetStringUTFChars(str, 0);
  const int strCharsLen = env->GetStringUTFLength(str);

  char* strChars0 = new char[strCharsLen + 1];
  std::strcpy(strChars0, strChars);
  *(strChars0 + strCharsLen) = 0;

  env->ReleaseStringUTFChars(str, strChars);

  if (len)
    *len = strCharsLen;

  return strChars0;
}

std::string JavaStringToCString(JNIEnv* env, jstring str, int* len) {
  char* resChars = StringToChars(env, str, len);

  if (resChars) {
    std::string res = std::string(resChars, *len);

    delete[] resChars;

    return res;
  } else
    return std::string();
}

jclass FindClass(JNIEnv* env, const char* name) {
  jclass res = env->FindClass(name);

  if (!res)
    throw JvmException();

  jclass res0 = static_cast< jclass >(env->NewGlobalRef(res));

  env->DeleteLocalRef(res);

  return res0;
}

void DeleteClass(JNIEnv* env, jclass cls) {
  if (cls)
    env->DeleteGlobalRef(cls);
}

void CheckClass(JNIEnv* env, const char* name) {
  jclass res = env->FindClass(name);

  if (!res)
    throw JvmException();
}

jmethodID FindMethod(JNIEnv* env, jclass cls, JniMethod mthd) {
  jmethodID mthd0 = mthd.isStatic
                        ? env->GetStaticMethodID(cls, mthd.name, mthd.sign)
                        : env->GetMethodID(cls, mthd.name, mthd.sign);

  if (!mthd0)
    throw JvmException();

  return mthd0;
}

void AddNativeMethod(JNINativeMethod* mthd, JniMethod jniMthd, void* fnPtr) {
  mthd->name = jniMthd.name;
  mthd->signature = jniMthd.sign;
  mthd->fnPtr = fnPtr;
}

void JniJavaMembers::Initialize(JNIEnv* env) {
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
}

void JniJavaMembers::Destroy(JNIEnv* env) {
  DeleteClass(env, c_Class);
  DeleteClass(env, c_Throwable);
  DeleteClass(env, c_PlatformUtils);
}

bool JniJavaMembers::WriteErrorInfo(JNIEnv* env, char** errClsName,
                                    int* errClsNameLen, char** errMsg,
                                    int* errMsgLen, char** stackTrace,
                                    int* stackTraceLen) {
  if (env && env->ExceptionCheck()) {
    if (m_Class_getName && m_Throwable_getMessage) {
      jthrowable err = env->ExceptionOccurred();

      env->ExceptionClear();

      jclass errCls = env->GetObjectClass(err);

      jstring clsName = static_cast< jstring >(
          env->CallObjectMethod(errCls, m_Class_getName));
      *errClsName = StringToChars(env, clsName, errClsNameLen);

      jstring msg = static_cast< jstring >(
          env->CallObjectMethod(err, m_Throwable_getMessage));
      *errMsg = StringToChars(env, msg, errMsgLen);

      jstring trace = NULL;

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

      return true;
    } else {
      env->ExceptionClear();
    }
  }

  return false;
}

void JniMembers::Initialize(JNIEnv* env) {
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
}

void JniMembers::Destroy(JNIEnv* env) {
  DeleteClass(env, c_IgniteException);
  DeleteClass(env, c_PlatformIgnition);
  DeleteClass(env, c_PlatformTarget);
  DeleteClass(env, c_PlatformUtils);
}

JniJvm::JniJvm()
    : jvm(NULL), javaMembers(JniJavaMembers()), members(JniMembers()) {
  // No-op.
}

JniJvm::JniJvm(JavaVM* jvm, JniJavaMembers javaMembers, JniMembers members)
    : jvm(jvm), javaMembers(javaMembers), members(members) {
  // No-op.
}

JavaVM* JniJvm::GetJvm() {
  return jvm;
}

JniJavaMembers& JniJvm::GetJavaMembers() {
  return javaMembers;
}

JniMembers& JniJvm::GetMembers() {
  return members;
}

GlobalJObject::GlobalJObject(JNIEnv* e, jobject obj) : env(e), ref(obj) {
  // No-op.
}

GlobalJObject::~GlobalJObject() {
  env->DeleteGlobalRef(ref);
}

jobject GlobalJObject::GetRef() const {
  return ref;
}

/**
 * Create JVM.
 */
jint GetOrCreateJvm(char** opts, int optsLen, JavaVM** jvm, JNIEnv** env) {
  // Check to see if a VM is already created
  const jsize nJvms = 1;
  jsize nJvmsAvailable = 0;
  JavaVM* availableJvms[nJvms]{};
  jint res = JNI_GetCreatedJavaVMs(&availableJvms[0], nJvms, &nJvmsAvailable);
  if (res == JNI_OK && nJvmsAvailable >= 1) {
    *jvm = availableJvms[0];
    res = (*jvm)->GetEnv(reinterpret_cast< void** >(env), JNI_VERSION_1_8);
    if (res == JNI_OK) {
      return res;
    }
  }

  // Otherwise, create a VM
  JavaVMOption* opts0 = new JavaVMOption[optsLen];

  for (int i = 0; i < optsLen; i++)
    opts0[i].optionString = *(opts + i);

  JavaVMInitArgs args{};
  args.version = JNI_VERSION_1_8;
  args.nOptions = optsLen;
  args.options = opts0;
  args.ignoreUnrecognized = 0;

  res = JNI_CreateJavaVM(jvm, reinterpret_cast< void** >(env), &args);

  delete[] opts0;

  return res;
}

void RegisterNatives(JNIEnv* env) {
  IGNITE_UNUSED(env);
  // TODO: Investigate registering callbacks to get console and logging streams.
}

JniContext::JniContext(JniJvm* jvm, JniHandlers hnds) : jvm(jvm), hnds(hnds) {
  // No-op.
}

JniContext* JniContext::Create(char** opts, int optsLen, JniHandlers hnds) {
  return Create(opts, optsLen, hnds, NULL);
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

JniContext* JniContext::Create(char** opts, int optsLen, JniHandlers hnds,
                               JniErrorInfo* errInfo) {
  // Acquire global lock to instantiate the JVM.
  JVM_LOCK.Enter();

  // Define local variables.
  JavaVM* jvm = NULL;
  JNIEnv* env = NULL;

  JniJavaMembers javaMembers;
  memset(&javaMembers, 0, sizeof(javaMembers));

  JniMembers members;
  memset(&members, 0, sizeof(members));

  JniContext* ctx = NULL;

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
    char* errClsNameChars = NULL;
    char* errMsgChars = NULL;
    char* stackTraceChars = NULL;

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
    if (errInfo) {
      JniErrorInfo errInfo0(JniErrorCode::IGNITE_JNI_ERR_JVM_INIT,
                            errClsName.c_str(), errMsg.c_str());

      *errInfo = errInfo0;
    }

    if (hnds.error)
      hnds.error(hnds.target, JniErrorCode::IGNITE_JNI_ERR_JVM_INIT,
                 errClsName.c_str(), errClsNameLen, errMsg.c_str(), errMsgLen,
                 stackTrace.c_str(), stackTraceLen, NULL, 0);
  }

  return ctx;
}

int JniContext::Reallocate(int64_t memPtr, int cap) {
  JavaVM* jvm = JVM.GetJvm();

  JNIEnv* env;

  int attachRes =
      jvm->AttachCurrentThread(reinterpret_cast< void** >(&env), NULL);

  if (attachRes == JNI_OK)
    AttachHelper::OnThreadAttach();
  else
    return -1;

  env->CallStaticVoidMethod(JVM.GetMembers().c_PlatformUtils,
                            JVM.GetMembers().m_PlatformUtils_reallocate, memPtr,
                            cap);

  if (env->ExceptionCheck()) {
    env->ExceptionClear();

    return -1;
  }

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

JniErrorCode JniContext::DriverManagerGetConnection(
    const char* connectionString, SharedPointer< GlobalJObject >& connection,
    JniErrorInfo& errInfo) {
  JNIEnv* env = Attach();
  jstring jConnectionString = env->NewStringUTF(connectionString);
  jobject result = env->CallStaticObjectMethod(
      jvm->GetMembers().c_DriverManager,
      jvm->GetMembers().m_DriverManagerGetConnection, jConnectionString);
  ExceptionCheck(env, &errInfo);
  if (!result || errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    connection = nullptr;
    return errInfo.code;
  }
  connection = SharedPointer< GlobalJObject >(
      new GlobalJObject(env, env->NewGlobalRef(result)));
  return errInfo.code;
}

JniErrorCode JniContext::ConnectionClose(
    const SharedPointer< GlobalJObject >& connection, JniErrorInfo& errInfo) {
  if (connection.Get() == nullptr) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "Connection object must be set.";
    return errInfo.code;
  }
  JNIEnv* env = Attach();
  env->CallVoidMethod(connection.Get()->GetRef(),
                      jvm->GetMembers().m_ConnectionClose);
  ExceptionCheck(env, &errInfo);
  return errInfo.code;
}

JniErrorCode JniContext::DocumentDbConnectionIsSshTunnelActive(
    const SharedPointer< GlobalJObject >& connection, bool& isActive,
    JniErrorInfo& errInfo) {
  if (!connection.Get()) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "Connection object must be set.";
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
  return errInfo.code;
}

JniErrorCode JniContext::DocumentDbConnectionGetSshLocalPort(
    const SharedPointer< GlobalJObject >& connection, int32_t& result,
    JniErrorInfo& errInfo) {
  if (!connection.Get()) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "Connection object must be set.";
    return errInfo.code;
  }
  JNIEnv* env = Attach();
  result = env->CallIntMethod(
      connection.Get()->GetRef(),
      jvm->GetMembers().m_DocumentDbConnectionGetSshLocalPort);
  ExceptionCheck(env, &errInfo);
  return errInfo.code;
}

JniErrorCode JniContext::DocumentDbConnectionGetDatabaseMetadata(
    const SharedPointer< GlobalJObject >& connection,
    SharedPointer< GlobalJObject >& metadata, JniErrorInfo& errInfo) {
  if (!connection.Get()) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "Connection object must be set.";
    return errInfo.code;
  }
  JNIEnv* env = Attach();
  jobject result = env->CallObjectMethod(
      connection.Get()->GetRef(),
      jvm->GetMembers().m_DocumentDbConnectionGetDatabaseMetadata);
  ExceptionCheck(env, &errInfo);

  if (!result || errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    metadata = nullptr;
    return errInfo.code;
  }

  metadata = SharedPointer< GlobalJObject >(
      new GlobalJObject(env, env->NewGlobalRef(result)));
  return errInfo.code;
}

JniErrorCode JniContext::DocumentDbDatabaseSchemaMetadataGetSchemaName(
    const SharedPointer< GlobalJObject >& databaseMetadata, std::string& value,
    bool& wasNull, JniErrorInfo& errInfo) {
  if (!databaseMetadata.Get()) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "DatabaseMetadata object must be set.";
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

  return errInfo.code;
}

JniErrorCode JniContext::ConnectionGetMetaData(
    const SharedPointer< GlobalJObject >& connection,
    SharedPointer< GlobalJObject >& databaseMetaData, JniErrorInfo& errInfo) {
  if (connection.Get() == nullptr) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "Connection object must be set.";
    return errInfo.code;
  }

  JNIEnv* env = Attach();
  jobject result = env->CallObjectMethod(
      connection.Get()->GetRef(), jvm->GetMembers().m_ConnectionGetMetaData);
  ExceptionCheck(env, &errInfo);

  if (!result || errInfo.code != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    databaseMetaData = nullptr;
    return errInfo.code;
  }

  databaseMetaData = SharedPointer< GlobalJObject >(
      new GlobalJObject(env, env->NewGlobalRef(result)));
  return errInfo.code;
}

JniErrorCode JniContext::DatabaseMetaDataGetTables(
    const SharedPointer< GlobalJObject >& databaseMetaData,
    const std::string& catalog, const std::string& schemaPattern,
    const std::string& tableNamePattern,
    const std::vector< std::string >& types,
    SharedPointer< GlobalJObject >& resultSet, JniErrorInfo& errInfo) {
  if (databaseMetaData.Get() == nullptr) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "DatabaseMetaData object must be set.";
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
    return errInfo.code;
  }

  resultSet = new GlobalJObject(env, env->NewGlobalRef(result));
  return errInfo.code;
}

JniErrorCode JniContext::DatabaseMetaDataGetColumns(
    const SharedPointer< GlobalJObject >& databaseMetaData,
    const std::string& catalog, const std::string& schemaPattern,
    const std::string& tableNamePattern, const std::string& columnNamePattern,
    SharedPointer< GlobalJObject >& resultSet, JniErrorInfo& errInfo) {
  if (databaseMetaData.Get() == nullptr) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "DatabaseMetaData object must be set.";
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
    return errInfo.code;
  }

  resultSet = new GlobalJObject(env, env->NewGlobalRef(result));
  return errInfo.code;
}

JniErrorCode JniContext::ResultSetClose(
    const SharedPointer< GlobalJObject >& resultSet, JniErrorInfo& errInfo) {
  if (resultSet.Get() == nullptr) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "ResultSet object must be set.";
    return errInfo.code;
  }

  JNIEnv* env = Attach();
  env->CallVoidMethod(resultSet.Get()->GetRef(),
                      jvm->GetMembers().m_ResultSetClose);
  ExceptionCheck(env, &errInfo);
  return errInfo.code;
}

JniErrorCode JniContext::ResultSetNext(
    const SharedPointer< GlobalJObject >& resultSet, bool& hasNext,
    JniErrorInfo& errInfo) {
  if (resultSet.Get() == nullptr) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "ResultSet object must be set.";
    return errInfo.code;
  }

  JNIEnv* env = Attach();
  jboolean res = env->CallBooleanMethod(resultSet.Get()->GetRef(),
                                        jvm->GetMembers().m_ResultSetNext);
  ExceptionCheck(env, &errInfo);
  if (errInfo.code == JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    hasNext = res != JNI_FALSE;
  }
  return errInfo.code;
}

JniErrorCode JniContext::ResultSetGetString(
    const SharedPointer< GlobalJObject >& resultSet, int columnIndex,
    std::string& value, bool& wasNull, JniErrorInfo& errInfo) {
  if (resultSet.Get() == nullptr) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "ResultSet object must be set.";
    return errInfo.code;
  }

  JNIEnv* env = Attach();
  jobject result = env->CallObjectMethod(
      resultSet.Get()->GetRef(), jvm->GetMembers().m_ResultSetGetStringByIndex,
      columnIndex);
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
  return errInfo.code;
}

JniErrorCode JniContext::ResultSetGetString(
    const SharedPointer< GlobalJObject >& resultSet,
    const std::string& columnName, std::string& value, bool& wasNull,
    JniErrorInfo& errInfo) {
  if (resultSet.Get() == nullptr) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "ResultSet object must be set.";
    return errInfo.code;
  }

  JNIEnv* env = Attach();
  jstring jColumnName = env->NewStringUTF(columnName.c_str());
  jobject result = env->CallObjectMethod(
      resultSet.Get()->GetRef(), jvm->GetMembers().m_ResultSetGetStringByName,
      jColumnName);
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

  return errInfo.code;
}

JniErrorCode JniContext::ResultSetGetInt(
    const SharedPointer< GlobalJObject >& resultSet, int columnIndex,
    int& value, bool& wasNull, JniErrorInfo& errInfo) {
  if (resultSet.Get() == nullptr) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "ResultSet object must be set.";
    return errInfo.code;
  }

  JNIEnv* env = Attach();
  jint result = env->CallIntMethod(resultSet.Get()->GetRef(),
                                   jvm->GetMembers().m_ResultSetGetIntByIndex,
                                   columnIndex);
  ExceptionCheck(env, &errInfo);

  if (errInfo.code == JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    value = result;
    return ResultSetWasNull(resultSet, wasNull, errInfo);
  }

  return errInfo.code;
}

JniErrorCode JniContext::ResultSetGetInt(
    const SharedPointer< GlobalJObject >& resultSet,
    const std::string& columnName, int& value, bool& wasNull,
    JniErrorInfo& errInfo) {
  if (resultSet.Get() == nullptr) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "ResultSet object must be set.";
    return errInfo.code;
  }

  JNIEnv* env = Attach();
  jstring jColumnName = env->NewStringUTF(columnName.c_str());
  jint result = env->CallIntMethod(resultSet.Get()->GetRef(),
                                   jvm->GetMembers().m_ResultSetGetIntByName,
                                   jColumnName);
  ExceptionCheck(env, &errInfo);
  if (errInfo.code == JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    value = result;
    return ResultSetWasNull(resultSet, wasNull, errInfo);
  }
  return errInfo.code;
}

JniErrorCode JniContext::ResultSetGetRow(
    const SharedPointer< GlobalJObject >& resultSet, int& value, bool& wasNull,
    JniErrorInfo& errInfo) {
  if (resultSet.Get() == nullptr) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "ResultSet object must be set.";
    return errInfo.code;
  }

  JNIEnv* env = Attach();
  jint result = env->CallIntMethod(resultSet.Get()->GetRef(),
                                   jvm->GetMembers().m_ResultSetGetRow);
  ExceptionCheck(env, &errInfo);
  if (errInfo.code == JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    value = result;
    return ResultSetWasNull(resultSet, wasNull, errInfo);
  }
  return errInfo.code;
}

JniErrorCode JniContext::ResultSetWasNull(
    const SharedPointer< GlobalJObject >& resultSet, bool& value,
    JniErrorInfo& errInfo) {
  if (resultSet.Get() == nullptr) {
    errInfo.code = JniErrorCode::IGNITE_JNI_ERR_GENERIC;
    errInfo.errMsg = "ResultSet object must be set.";
    return errInfo.code;
  }

  JNIEnv* env = Attach();
  jboolean res = env->CallBooleanMethod(resultSet.Get()->GetRef(),
                                        jvm->GetMembers().m_ResultSetWasNull);
  ExceptionCheck(env, &errInfo);
  if (errInfo.code == JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    value = res != JNI_FALSE;
  }
  return errInfo.code;
}

int64_t JniContext::TargetInLongOutLong(jobject obj, int opType, int64_t val,
                                        JniErrorInfo* err) {
  JNIEnv* env = Attach();

  int64_t res = env->CallLongMethod(
      obj, jvm->GetMembers().m_PlatformTarget_inLongOutLong, opType, val);

  ExceptionCheck(env, err);

  return res;
}

int64_t JniContext::TargetInStreamOutLong(jobject obj, int opType,
                                          int64_t memPtr, JniErrorInfo* err) {
  JNIEnv* env = Attach();

  int64_t res = env->CallLongMethod(
      obj, jvm->GetMembers().m_PlatformTarget_inStreamOutLong, opType, memPtr);

  ExceptionCheck(env, err);

  return res;
}

void JniContext::TargetInStreamOutStream(jobject obj, int opType,
                                         int64_t inMemPtr, int64_t outMemPtr,
                                         JniErrorInfo* err) {
  JNIEnv* env = Attach();

  env->CallVoidMethod(obj, jvm->GetMembers().m_PlatformTarget_inStreamOutStream,
                      opType, inMemPtr, outMemPtr);

  ExceptionCheck(env, err);
}

jobject JniContext::TargetInStreamOutObject(jobject obj, int opType,
                                            int64_t memPtr, JniErrorInfo* err) {
  JNIEnv* env = Attach();

  jobject res = env->CallObjectMethod(
      obj, jvm->GetMembers().m_PlatformTarget_inStreamOutObject, opType,
      memPtr);

  ExceptionCheck(env, err);

  return LocalToGlobal(env, res);
}

jobject JniContext::TargetInObjectStreamOutObjectStream(jobject obj, int opType,
                                                        void* arg,
                                                        int64_t inMemPtr,
                                                        int64_t outMemPtr,
                                                        JniErrorInfo* err) {
  JNIEnv* env = Attach();

  jobject res = env->CallObjectMethod(
      obj, jvm->GetMembers().m_PlatformTarget_inObjectStreamOutObjectStream,
      opType, arg, inMemPtr, outMemPtr);

  ExceptionCheck(env, err);

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
  JNIEnv* env = Attach();

  jobject res = env->CallObjectMethod(
      obj, jvm->GetMembers().m_PlatformTarget_outObject, opType);

  ExceptionCheck(env, err);

  return LocalToGlobal(env, res);
}

void JniContext::TargetInStreamAsync(jobject obj, int opType, int64_t memPtr,
                                     JniErrorInfo* err) {
  JNIEnv* env = Attach();

  env->CallVoidMethod(obj, jvm->GetMembers().m_PlatformTarget_inStreamAsync,
                      opType, memPtr);

  ExceptionCheck(env, err);
}

jobject JniContext::TargetInStreamOutObjectAsync(jobject obj, int opType,
                                                 int64_t memPtr,
                                                 JniErrorInfo* err) {
  JNIEnv* env = Attach();

  jobject res = env->CallObjectMethod(
      obj, jvm->GetMembers().m_PlatformTarget_inStreamOutObjectAsync, opType,
      memPtr);

  ExceptionCheck(env, err);

  return LocalToGlobal(env, res);
}

jobject JniContext::CacheOutOpQueryCursor(jobject obj, int type, int64_t memPtr,
                                          JniErrorInfo* err) {
  JNIEnv* env = Attach();

  jobject res = env->CallObjectMethod(
      obj, jvm->GetMembers().m_PlatformTarget_inStreamOutObject, type, memPtr);

  ExceptionCheck(env, err);

  return LocalToGlobal(env, res);
}

jobject JniContext::CacheOutOpContinuousQuery(jobject obj, int type,
                                              int64_t memPtr,
                                              JniErrorInfo* err) {
  JNIEnv* env = Attach();

  jobject res = env->CallObjectMethod(
      obj, jvm->GetMembers().m_PlatformTarget_inStreamOutObject, type, memPtr);

  ExceptionCheck(env, err);

  return LocalToGlobal(env, res);
}

jobject JniContext::Acquire(jobject obj) {
  if (obj) {
    JNIEnv* env = Attach();

    jobject obj0 = env->NewGlobalRef(obj);

    ExceptionCheck(env);

    return obj0;
  }

  return NULL;
}

void JniContext::Release(jobject obj) {
  if (obj) {
    JavaVM* jvm = JVM.GetJvm();

    if (jvm) {
      JNIEnv* env;

      jint attachRes =
          jvm->AttachCurrentThread(reinterpret_cast< void** >(&env), NULL);

      if (attachRes == JNI_OK) {
        AttachHelper::OnThreadAttach();

        env->DeleteGlobalRef(obj);
      }
    }
  }
}

void JniContext::SetConsoleHandler(ConsoleWriteHandler consoleHandler) {
  if (!consoleHandler)
    throw std::invalid_argument("consoleHandler can not be null");

  CONSOLE_LOCK.Enter();

  consoleWriteHandlers.push_back(consoleHandler);

  CONSOLE_LOCK.Leave();
}

int JniContext::RemoveConsoleHandler(ConsoleWriteHandler consoleHandler) {
  if (!consoleHandler)
    throw std::invalid_argument("consoleHandler can not be null");

  CONSOLE_LOCK.Enter();

  int oldSize = static_cast< int >(consoleWriteHandlers.size());

  consoleWriteHandlers.erase(remove(consoleWriteHandlers.begin(),
                                    consoleWriteHandlers.end(), consoleHandler),
                             consoleWriteHandlers.end());

  int removedCnt = oldSize - static_cast< int >(consoleWriteHandlers.size());

  CONSOLE_LOCK.Leave();

  return removedCnt;
}

void JniContext::ThrowToJava(char* msg) {
  JNIEnv* env = Attach();

  env->ThrowNew(jvm->GetMembers().c_IgniteException, msg);
}

void JniContext::DestroyJvm() {
  jvm->GetJvm()->DestroyJavaVM();
}

/**
 * Attach thread to JVM.
 */
JNIEnv* JniContext::Attach() {
  JNIEnv* env;

  jint attachRes = jvm->GetJvm()->AttachCurrentThread(
      reinterpret_cast< void** >(&env), NULL);

  if (attachRes == JNI_OK)
    AttachHelper::OnThreadAttach();
  else {
    if (hnds.error)
      hnds.error(hnds.target, JniErrorCode::IGNITE_JNI_ERR_JVM_ATTACH, NULL, 0,
                 NULL, 0, NULL, 0, NULL, 0);
  }

  return env;
}

void JniContext::ExceptionCheck(JNIEnv* env) {
  ExceptionCheck(env, NULL);
}

void JniContext::ExceptionCheck(JNIEnv* env, JniErrorInfo* errInfo) {
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

      trace0 = JavaStringToCString(env, trace, &traceLen);
    }

    env->DeleteLocalRef(cls);

    int clsNameLen;
    std::string clsName0 = JavaStringToCString(env, clsName, &clsNameLen);

    int msgLen;
    std::string msg0 = JavaStringToCString(env, msg, &msgLen);

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
      jbyte* errBytesNative = env->GetByteArrayElements(errData, NULL);

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
                   trace0.c_str(), traceLen, NULL, 0);
    }

    env->DeleteLocalRef(err);
  } else if (errInfo) {
    JniErrorInfo errInfo0(JniErrorCode::IGNITE_JNI_ERR_SUCCESS, "", "");
    *errInfo = errInfo0;
  }
}

/**
 * Convert local reference to global.
 */
jobject JniContext::LocalToGlobal(JNIEnv* env, jobject localRef) {
  if (localRef) {
    jobject globalRef = env->NewGlobalRef(localRef);

    env->DeleteLocalRef(localRef);  // Clear local ref irrespective of result.

    if (!globalRef)
      ExceptionCheck(env);

    return globalRef;
  } else
    return NULL;
}

JNIEXPORT void JNICALL JniConsoleWrite(JNIEnv* env, jclass, jstring str,
                                       jboolean isErr) {
  CONSOLE_LOCK.Enter();

  if (consoleWriteHandlers.size() > 0) {
    ConsoleWriteHandler consoleWrite = consoleWriteHandlers.at(0);

    const char* strChars = env->GetStringUTFChars(str, 0);
    const int strCharsLen = env->GetStringUTFLength(str);

    consoleWrite(strChars, strCharsLen, isErr);

    env->ReleaseStringUTFChars(str, strChars);
  }

  CONSOLE_LOCK.Leave();
}

JNIEXPORT void JNICALL JniLoggerLog(JNIEnv* env, jclass, jlong envPtr,
                                    jint level, jstring message,
                                    jstring category, jstring errorInfo,
                                    jlong memPtr) {
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
}

JNIEXPORT jboolean JNICALL JniLoggerIsLevelEnabled(JNIEnv* env, jclass,
                                                   jlong envPtr, jint level) {
  IGNITE_SAFE_FUNC(env, envPtr, LoggerIsLevelEnabledHandler,
                   loggerIsLevelEnabled, level);
}

JNIEXPORT jlong JNICALL JniInLongOutLong(JNIEnv* env, jclass, jlong envPtr,
                                         jint type, jlong val) {
  IGNITE_SAFE_FUNC(env, envPtr, InLongOutLongHandler, inLongOutLong, type, val);
}

JNIEXPORT jlong JNICALL JniInLongLongLongObjectOutLong(JNIEnv* env, jclass,
                                                       jlong envPtr, jint type,
                                                       jlong val1, jlong val2,
                                                       jlong val3,
                                                       jobject arg) {
  IGNITE_SAFE_FUNC(env, envPtr, InLongLongLongObjectOutLongHandler,
                   inLongLongLongObjectOutLong, type, val1, val2, val3, arg);
}
}  //  namespace java
}  //  namespace jni
}  //  namespace odbc
}  //  namespace ignite
