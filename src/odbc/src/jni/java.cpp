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
#include <cstring>   // needed only on linux
#include <string>
#include <exception>
#include <vector>
#include <algorithm>
#include <stdexcept>

#include <ignite/odbc/ignite_error.h>
#include <ignite/odbc/jni/utils.h>
#include <ignite/odbc/jni/java.h>
#include <ignite/odbc/common/concurrent.h>
#include <ignite/odbc/common/utils.h>

#ifndef JNI_VERSION_9
#define JNI_VERSION_9 0x00090000
#endif // JNI_VERSION_9

#define IGNITE_SAFE_PROC_NO_ARG(jniEnv, envPtr, type, field) { \
    JniHandlers* hnds = reinterpret_cast<JniHandlers*>(envPtr); \
    type hnd = hnds->field; \
    if (hnd) \
    { \
        try \
        { \
            hnd(hnds->target); \
        } \
        catch (std::exception& err) \
        { \
            ThrowToJava(jniEnv, err.what()); \
        } \
    } \
    else \
        ThrowOnMissingHandler(jniEnv); \
}

#define IGNITE_SAFE_PROC(jniEnv, envPtr, type, field, ...) { \
    JniHandlers* hnds = reinterpret_cast<JniHandlers*>(envPtr); \
    type hnd = hnds->field; \
    if (hnd) \
    { \
        try \
        { \
            hnd(hnds->target, __VA_ARGS__); \
        } \
        catch (std::exception& err) \
        { \
            ThrowToJava(jniEnv, err.what()); \
        } \
    } \
    else \
        ThrowOnMissingHandler(jniEnv); \
}

#define IGNITE_SAFE_FUNC(jniEnv, envPtr, type, field, ...) { \
    JniHandlers* hnds = reinterpret_cast<JniHandlers*>(envPtr); \
    type hnd = hnds->field; \
    if (hnd) \
    { \
        try \
        { \
            return hnd(hnds->target, __VA_ARGS__); \
        } \
        catch (std::exception& err) \
        { \
            ThrowToJava(jniEnv, err.what()); \
            return 0; \
        } \
    } \
    else \
    { \
        ThrowOnMissingHandler(jniEnv); \
        return 0; \
    }\
}

using namespace ignite::odbc::java;

namespace ignite
{
    namespace odbc 
    {
        namespace jni
        {
            namespace java
            {
                namespace iocc = ignite::odbc::common::concurrent;

                bool IGNITE_IMPORT_EXPORT IsJava9OrLater()
                {
                    JavaVMInitArgs args;

                    memset(&args, 0, sizeof(args));

                    args.version = JNI_VERSION_9;

                    return JNI_GetDefaultJavaVMInitArgs(&args) == JNI_OK;
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

                JniErrorInfo::JniErrorInfo() : code(IGNITE_JNI_ERR_SUCCESS)
                {
                    // No-op.
                }

                JniErrorInfo::JniErrorInfo(int code, const char* errCls, const char* errMsg) : code(code)
                {
                    this->errCls = common::CopyChars(errCls);
                    this->errMsg = common::CopyChars(errMsg);
                }

                JniErrorInfo::JniErrorInfo(const JniErrorInfo& other) : code(other.code)
                {
                    this->errCls = common::CopyChars(other.errCls);
                    this->errMsg = common::CopyChars(other.errMsg);
                }

                JniErrorInfo& JniErrorInfo::operator=(const JniErrorInfo& other)
                {
                    if (this != &other)
                    {
                        // 1. Create new instance, exception could occur at this point.
                        JniErrorInfo tmp(other);

                        // 2. Swap with temp.
                        std::swap(code, tmp.code);
                        std::swap(errCls, tmp.errCls);
                        std::swap(errMsg, tmp.errMsg);
                    }

                    return *this;
                }

                JniErrorInfo::~JniErrorInfo()
                {
                    delete[] errCls;
                    delete[] errMsg;
                }

                /**
                  * Guard to ensure global reference cleanup.
                  */
                class JniGlobalRefGuard
                {
                public:
                    JniGlobalRefGuard(JNIEnv *e, jobject obj) : env(e), ref(obj)
                    {
                        // No-op.
                    }

                    ~JniGlobalRefGuard()
                    {
                        env->DeleteGlobalRef(ref);
                    }

                private:
                    /** Environment. */
                    JNIEnv* env;

                    /** Target reference. */
                    jobject ref;

                    IGNITE_NO_COPY_ASSIGNMENT(JniGlobalRefGuard);
                };

                const char* C_THROWABLE = "java/lang/Throwable";
                JniMethod M_THROWABLE_GET_MESSAGE = JniMethod("getMessage", "()Ljava/lang/String;", false);
                JniMethod M_THROWABLE_PRINT_STACK_TRACE = JniMethod("printStackTrace", "()V", false);

                const char* C_CLASS = "java/lang/Class";
                JniMethod M_CLASS_GET_NAME = JniMethod("getName", "()Ljava/lang/String;", false);

                const char* C_DOCUMENTDB_CONNECTION_PROPERTIES =
                    "software/amazon/documentdb/jdbc/DocumentDbConnectionProperties";
                JniMethod M_DOCUMENTDB_CONNECTION_PROPERTIES_GET_PROPERTIES_FROM_CONNECTION_STRING =
                        JniMethod(
                            "getPropertiesFromConnectionString",
                            "(Ljava/lang/String;)Lsoftware/amazon/documentdb/jdbc/DocumentDbConnectionProperties;",
                            true);

                const char* C_DOCUMENTDB_CONNECTION = "software/amazon/documentdb/jdbc/DocumentDbConnectionProperties";

                const char* C_DRIVERMANAGER = "java/sql/DriverManager";
                JniMethod M_DRIVERMANAGER_GET_CONNECTION = 
                  JniMethod("getConnection", "(Ljava/lang/String;)Ljava/sql/Connection;", true);

                const char* C_JAVA_SQL_CONNECTION = "java/sql/Connection";
                JniMethod M_JAVA_SQL_CONNECTION_CLOSE = JniMethod("close", "()V", false);

                // TODO: Provide a "getFullStackTrace" from DocumentDB
                //JniMethod M_PLATFORM_UTILS_GET_FULL_STACK_TRACE = JniMethod("getFullStackTrace", "(Ljava/lang/Throwable;)Ljava/lang/String;", true);


                /* STATIC STATE. */
                iocc::CriticalSection JVM_LOCK;
                iocc::CriticalSection CONSOLE_LOCK;
                JniJvm JVM;
                bool PRINT_EXCEPTION = false;
                std::vector<ConsoleWriteHandler> consoleWriteHandlers;

                /* HELPER METHODS. */

                /**
                  * Throw exception to Java in case of missing callback pointer. It means that callback is not implemented in
                  * native platform and Java -> platform operation cannot proceede further. As JniContext is not available at
                  * this point, we have to obtain exception details from scratch. This is not critical from performance
                  * perspective because missing handler usually denotes fatal condition.
                  *
                  * @param env JNI environment.
                  */
                int ThrowOnMissingHandler(JNIEnv* env)
                {
                    //jclass cls = env->FindClass(C_PLATFORM_NO_CALLBACK_EXCEPTION);

                    //env->ThrowNew(cls, "Callback handler is not set in native platform.");

                    return 0;
                }

                /**
                  * Throw generic exception to Java in case of native exception. As JniContext is not available at
                  * this point, we have to obtain exception details from scratch. This is not critical from performance
                  * perspective because such exception is usually denotes fatal condition.
                  *
                  * @param env JNI environment.
                  * @param msg Message.
                  */
                void ThrowToJava(JNIEnv* env, const char* msg)
                {
                    //jclass cls = env->FindClass(C_IGNITE_EXCEPTION);

                    //env->ThrowNew(cls, msg);
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

                std::string JavaStringToCString(JNIEnv* env, jstring str, int* len)
                {
                    char* resChars = StringToChars(env, str, len);

                    if (resChars)
                    {
                        std::string res = std::string(resChars, *len);

                        delete[] resChars;

                        return res;
                    }
                    else
                        return std::string();
                }

                jclass FindClass(JNIEnv* env, const char *name) {
                    jclass res = env->FindClass(name);

                    if (!res)
                        throw JvmException();

                    jclass res0 = static_cast<jclass>(env->NewGlobalRef(res));

                    env->DeleteLocalRef(res);

                    return res0;
                }

                void DeleteClass(JNIEnv* env, jclass cls) {
                    if (cls)
                        env->DeleteGlobalRef(cls);
                }

                void CheckClass(JNIEnv* env, const char *name)
                {
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

                jobject NewObject(JNIEnv* env, jclass clazz, jmethodID constructor, ...) {
                    va_list args;
                    va_start(args, constructor);
                    jobject result = env->NewObject(clazz, constructor, args);
                    va_end(args);

                    if (!result)
                        throw JvmException();

                    return result;
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
                    m_Throwable_getMessage = FindMethod(env, c_Throwable, M_THROWABLE_GET_MESSAGE);
                    m_Throwable_printStackTrace = FindMethod(env, c_Throwable, M_THROWABLE_PRINT_STACK_TRACE);

                    // TODO: Provide "getFullStackTrace" in DocumentDB
                    //m_PlatformUtils_getFullStackTrace = FindMethod(env, c_PlatformUtils, M_PLATFORM_UTILS_GET_FULL_STACK_TRACE);
                }

                void JniJavaMembers::Destroy(JNIEnv* env) {
                    DeleteClass(env, c_Class);
                    DeleteClass(env, c_Throwable);
                    DeleteClass(env, c_PlatformUtils);
                }

                bool JniJavaMembers::WriteErrorInfo(JNIEnv* env, char** errClsName, int* errClsNameLen, char** errMsg,
                    int* errMsgLen, char** stackTrace, int* stackTraceLen) {
                    if (env && env->ExceptionCheck()) {
                        if (m_Class_getName && m_Throwable_getMessage) {
                            jthrowable err = env->ExceptionOccurred();

                            env->ExceptionClear();

                            jclass errCls = env->GetObjectClass(err);

                            jstring clsName = static_cast<jstring>(env->CallObjectMethod(errCls, m_Class_getName));
                            *errClsName = StringToChars(env, clsName, errClsNameLen);

                            jstring msg = static_cast<jstring>(env->CallObjectMethod(err, m_Throwable_getMessage));
                            *errMsg = StringToChars(env, msg, errMsgLen);

                            jstring trace = NULL;

                            if (c_PlatformUtils && m_PlatformUtils_getFullStackTrace) {
                                trace = static_cast<jstring>(env->CallStaticObjectMethod(c_PlatformUtils, m_PlatformUtils_getFullStackTrace, err));
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
                        }
                        else {
                            env->ExceptionClear();
                        }
                    }

                    return false;
                }

                void JniMembers::Initialize(JNIEnv* env) {
                    c_DocumentDbConnectionProperties = FindClass(env, C_DOCUMENTDB_CONNECTION_PROPERTIES);
                    m_DocumentDbConnectionPropertiesGetPropertiesFromConnectionString =
                        FindMethod(env, c_DocumentDbConnectionProperties, M_DOCUMENTDB_CONNECTION_PROPERTIES_GET_PROPERTIES_FROM_CONNECTION_STRING);

                    c_DocumentDbConnection = FindClass(env, C_DOCUMENTDB_CONNECTION);
                    //m_DocumentDbConnectionInit = FindMethod(env, c_DocumentDbConnection, M_DOCUMENTDB_CONNECTION_PROPERTIES_INIT);

                    c_DriverManager = FindClass(env, C_DRIVERMANAGER);
                    m_DriverManagerGetConnection = FindMethod(env, c_DriverManager, M_DRIVERMANAGER_GET_CONNECTION);

                    c_JavaSqlConnection = FindClass(env, C_JAVA_SQL_CONNECTION);
                    m_JavaSqlConnectionClose = FindMethod(env, c_JavaSqlConnection, M_JAVA_SQL_CONNECTION_CLOSE);
                }

                void JniMembers::Destroy(JNIEnv* env) {
                    DeleteClass(env, c_IgniteException);
                    DeleteClass(env, c_PlatformIgnition);
                    DeleteClass(env, c_PlatformTarget);
                    DeleteClass(env, c_PlatformUtils);
                }

                JniJvm::JniJvm() : jvm(NULL), javaMembers(JniJavaMembers()), members(JniMembers())
                {
                    // No-op.
                }

                JniJvm::JniJvm(JavaVM* jvm, JniJavaMembers javaMembers, JniMembers members) :
                    jvm(jvm), javaMembers(javaMembers), members(members)
                {
                    // No-op.
                }

                JavaVM* JniJvm::GetJvm()
                {
                    return jvm;
                }

                JniJavaMembers& JniJvm::GetJavaMembers()
                {
                    return javaMembers;
                }

                JniMembers& JniJvm::GetMembers()
                {
                    return members;
                }

                /**
                  * Create JVM.
                  */
                jint GetOrCreateJvm(char** opts, int optsLen, JavaVM** jvm, JNIEnv** env) {
                    // Check to see if a VM is already created
                    const jsize nJvms = 1;
                    jsize nJvmsAvailable = 0;
                    JavaVM* availableJvms[nJvms]{};
                    jint res = JNI_GetCreatedJavaVMs(&availableJvms[0], nJvms,
                                                     &nJvmsAvailable);
                    if (res == JNI_OK && nJvmsAvailable >= 1) {
                        *jvm = availableJvms[0];
                        res = (*jvm)->GetEnv(reinterpret_cast< void** >(env),
                                             JNI_VERSION_1_8);
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

                    res = JNI_CreateJavaVM(jvm, reinterpret_cast<void**>(env), &args);

                    delete[] opts0;

                    return res;
                }

                void RegisterNatives(JNIEnv* env) {
                    {
                        JNINativeMethod methods[5];

                        int idx = 0;

                        // TODO: Investigate registering callbacks to get console and logging streams.

                        //AddNativeMethod(methods + idx++, M_PLATFORM_CALLBACK_UTILS_CONSOLE_WRITE, reinterpret_cast<void*>(JniConsoleWrite));

                        //AddNativeMethod(methods + idx++, M_PLATFORM_CALLBACK_UTILS_LOGGER_LOG, reinterpret_cast<void*>(JniLoggerLog));
                        //AddNativeMethod(methods + idx++, M_PLATFORM_CALLBACK_UTILS_LOGGER_IS_LEVEL_ENABLED, reinterpret_cast<void*>(JniLoggerIsLevelEnabled));

                        //jint res = env->RegisterNatives(FindClass(env, C_PLATFORM_CALLBACK_UTILS), methods, idx);

                        //if (res != JNI_OK)
                        //    throw JvmException();
                    }
                }

                JniContext::JniContext(JniJvm* jvm, JniHandlers hnds) : jvm(jvm), hnds(hnds) {
                    // No-op.
                }

                JniContext* JniContext::Create(char** opts, int optsLen, JniHandlers hnds) {
                    return Create(opts, optsLen, hnds, NULL);
                }

                void GetJniErrorMessage(std::string& errMsg, jint res)
                {
                    switch (res)
                    {
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
                            errMsg = "Could not reserve enough space for object heap. Check Xmx option.";
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

                JniContext* JniContext::Create(char** opts, int optsLen, JniHandlers hnds, JniErrorInfo* errInfo)
                {
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
                        if (!JVM.GetJvm())
                        {
                            // 1. Create JVM itself.
                            jint res = GetOrCreateJvm(opts, optsLen, &jvm, &env);

                            if (res == JNI_OK)
                            {
                                // 2. Populate members;
                                javaMembers.Initialize(env);
                                members.Initialize(env);

                                // 3. Register native functions.
                                RegisterNatives(env);

                                // 4. Create JNI JVM.
                                JVM = JniJvm(jvm, javaMembers, members);

                                char* printStack = getenv("IGNITE_CPP_PRINT_STACK");
                                PRINT_EXCEPTION = printStack && strcmp("true", printStack) == 0;
                            }
                            else
                            {
                                GetJniErrorMessage(errMsg, res);

                                errMsgLen = static_cast<int>(errMsg.length());
                            }
                        }

                        if (JVM.GetJvm())
                            ctx = new JniContext(&JVM, hnds);
                    }
                    catch (const JvmException&)
                    {
                        char* errClsNameChars = NULL;
                        char* errMsgChars = NULL;
                        char* stackTraceChars = NULL;

                        // Read error info if possible.
                        javaMembers.WriteErrorInfo(env, &errClsNameChars, &errClsNameLen, &errMsgChars, &errMsgLen,
                            &stackTraceChars, &stackTraceLen);

                        if (errClsNameChars) {
                            errClsName = errClsNameChars;

                            delete[] errClsNameChars;
                        }

                        if (errMsgChars)
                        {
                            errMsg = errMsgChars;

                            delete[] errMsgChars;
                        }

                        if (stackTraceChars)
                        {
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
                            JniErrorInfo errInfo0(IGNITE_JNI_ERR_JVM_INIT, errClsName.c_str(), errMsg.c_str());

                            *errInfo = errInfo0;
                        }

                        if (hnds.error)
                            hnds.error(hnds.target, IGNITE_JNI_ERR_JVM_INIT, errClsName.c_str(), errClsNameLen,
                                errMsg.c_str(), errMsgLen, stackTrace.c_str(), stackTraceLen, NULL, 0);
                    }

                    return ctx;
                }

                int JniContext::Reallocate(int64_t memPtr, int cap) {
                    JavaVM* jvm = JVM.GetJvm();

                    JNIEnv* env;

                    int attachRes = jvm->AttachCurrentThread(reinterpret_cast<void**>(&env), NULL);

                    if (attachRes == JNI_OK)
                        AttachHelper::OnThreadAttach();
                    else
                        return -1;

                    env->CallStaticVoidMethod(JVM.GetMembers().c_PlatformUtils, JVM.GetMembers().m_PlatformUtils_reallocate, memPtr, cap);

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

                        JVM.GetJvm()->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);

                        if (env)
                            JVM.GetJvm()->DetachCurrentThread();
                    }
                }

                jobject JniContext::DocumentDbConnect(const char* connectionString,
                                                   JniErrorInfo* errInfo) {
                    JNIEnv* env = Attach();
                    jstring jConnectionString =
                        env->NewStringUTF(connectionString);
                    jobject connection = env->CallStaticObjectMethod(
                        jvm->GetMembers().c_DriverManager,
                        jvm->GetMembers().m_DriverManagerGetConnection, jConnectionString);
                    ExceptionCheck(env, errInfo);
                    return connection;
                }

                void JniContext::DocumentDbDisconnect(const jobject connection, JniErrorInfo* errInfo) {
                    if (!connection) {
                        return;
                    }
                    JNIEnv* env = Attach();
                    env->CallVoidMethod(connection, jvm->GetMembers().m_JavaSqlConnectionClose);
                    ExceptionCheck(env, errInfo);
                    env->DeleteLocalRef(connection);
                }

                int64_t JniContext::TargetInLongOutLong(jobject obj, int opType, int64_t val, JniErrorInfo* err) {
                    JNIEnv* env = Attach();

                    int64_t res = env->CallLongMethod(obj, jvm->GetMembers().m_PlatformTarget_inLongOutLong, opType, val);

                    ExceptionCheck(env, err);

                    return res;
                }

                int64_t JniContext::TargetInStreamOutLong(jobject obj, int opType, int64_t memPtr, JniErrorInfo* err) {
                    JNIEnv* env = Attach();

                    int64_t res = env->CallLongMethod(obj, jvm->GetMembers().m_PlatformTarget_inStreamOutLong, opType, memPtr);

                    ExceptionCheck(env, err);

                    return res;
                }

                void JniContext::TargetInStreamOutStream(jobject obj, int opType, int64_t inMemPtr, int64_t outMemPtr, JniErrorInfo* err) {
                    JNIEnv* env = Attach();

                    env->CallVoidMethod(obj, jvm->GetMembers().m_PlatformTarget_inStreamOutStream, opType, inMemPtr, outMemPtr);

                    ExceptionCheck(env, err);
                }

                jobject JniContext::TargetInStreamOutObject(jobject obj, int opType, int64_t memPtr, JniErrorInfo* err) {
                    JNIEnv* env = Attach();

                    jobject res = env->CallObjectMethod(obj, jvm->GetMembers().m_PlatformTarget_inStreamOutObject, opType, memPtr);

                    ExceptionCheck(env, err);

                    return LocalToGlobal(env, res);
                }

                jobject JniContext::TargetInObjectStreamOutObjectStream(jobject obj, int opType, void* arg, int64_t inMemPtr, int64_t outMemPtr, JniErrorInfo* err) {
                    JNIEnv* env = Attach();

                    jobject res = env->CallObjectMethod(obj, jvm->GetMembers().m_PlatformTarget_inObjectStreamOutObjectStream, opType, arg, inMemPtr, outMemPtr);

                    ExceptionCheck(env, err);

                    return LocalToGlobal(env, res);
                }

                void JniContext::TargetOutStream(jobject obj, int opType, int64_t memPtr, JniErrorInfo* err) {
                    JNIEnv* env = Attach();

                    env->CallVoidMethod(obj, jvm->GetMembers().m_PlatformTarget_outStream, opType, memPtr);

                    ExceptionCheck(env, err);
                }

                jobject JniContext::TargetOutObject(jobject obj, int opType, JniErrorInfo* err)
                {
                    JNIEnv* env = Attach();

                    jobject res = env->CallObjectMethod(obj, jvm->GetMembers().m_PlatformTarget_outObject, opType);

                    ExceptionCheck(env, err);

                    return LocalToGlobal(env, res);
                }

                void JniContext::TargetInStreamAsync(jobject obj, int opType, int64_t memPtr, JniErrorInfo* err) {
                    JNIEnv* env = Attach();

                    env->CallVoidMethod(obj, jvm->GetMembers().m_PlatformTarget_inStreamAsync, opType, memPtr);

                    ExceptionCheck(env, err);
                }

                jobject JniContext::TargetInStreamOutObjectAsync(jobject obj, int opType, int64_t memPtr, JniErrorInfo* err) {
                    JNIEnv* env = Attach();

                    jobject res = env->CallObjectMethod(obj, jvm->GetMembers().m_PlatformTarget_inStreamOutObjectAsync, opType, memPtr);

                    ExceptionCheck(env, err);

                    return LocalToGlobal(env, res);
                }

                jobject JniContext::CacheOutOpQueryCursor(jobject obj, int type, int64_t memPtr, JniErrorInfo* err) {
                    JNIEnv* env = Attach();

                    jobject res = env->CallObjectMethod(
                        obj, jvm->GetMembers().m_PlatformTarget_inStreamOutObject, type, memPtr);

                    ExceptionCheck(env, err);

                    return LocalToGlobal(env, res);
                }

                jobject JniContext::CacheOutOpContinuousQuery(jobject obj, int type, int64_t memPtr, JniErrorInfo* err) {
                    JNIEnv* env = Attach();

                    jobject res = env->CallObjectMethod(
                        obj, jvm->GetMembers().m_PlatformTarget_inStreamOutObject, type, memPtr);

                    ExceptionCheck(env, err);

                    return LocalToGlobal(env, res);
                }

                jobject JniContext::Acquire(jobject obj)
                {
                    if (obj) {

                        JNIEnv* env = Attach();

                        jobject obj0 = env->NewGlobalRef(obj);

                        ExceptionCheck(env);

                        return obj0;
                    }

                    return NULL;
                }

                void JniContext::Release(jobject obj) {
                    if (obj)
                    {
                        JavaVM* jvm = JVM.GetJvm();

                        if (jvm)
                        {
                            JNIEnv* env;

                            jint attachRes = jvm->AttachCurrentThread(reinterpret_cast<void**>(&env), NULL);

                            if (attachRes == JNI_OK)
                            {
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

                    int oldSize = static_cast<int>(consoleWriteHandlers.size());

                    consoleWriteHandlers.erase(remove(consoleWriteHandlers.begin(), consoleWriteHandlers.end(),
                        consoleHandler), consoleWriteHandlers.end());

                    int removedCnt = oldSize - static_cast<int>(consoleWriteHandlers.size());

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

                    jint attachRes = jvm->GetJvm()->AttachCurrentThread(reinterpret_cast<void**>(&env), NULL);

                    if (attachRes == JNI_OK)
                        AttachHelper::OnThreadAttach();
                    else {
                        if (hnds.error)
                            hnds.error(hnds.target, IGNITE_JNI_ERR_JVM_ATTACH, NULL, 0, NULL, 0, NULL, 0, NULL, 0);
                    }

                    return env;
                }

                void JniContext::ExceptionCheck(JNIEnv* env) {
                    ExceptionCheck(env, NULL);
                }

                void JniContext::ExceptionCheck(JNIEnv* env, JniErrorInfo* errInfo)
                {
                    if (env->ExceptionCheck()) {
                        jthrowable err = env->ExceptionOccurred();

                        if (PRINT_EXCEPTION)
                            env->CallVoidMethod(err, jvm->GetJavaMembers().m_Throwable_printStackTrace);

                        env->ExceptionClear();

                        // Get error class name and message.
                        jclass cls = env->GetObjectClass(err);

                        jstring clsName = static_cast<jstring>(env->CallObjectMethod(cls, jvm->GetJavaMembers().m_Class_getName));
                        jstring msg = static_cast<jstring>(env->CallObjectMethod(err, jvm->GetJavaMembers().m_Throwable_getMessage));
                        int traceLen = 0;
                        std::string trace0 = ""; 
                        if (jvm->GetJavaMembers().c_PlatformUtils
                            && jvm->GetJavaMembers()
                                   .m_PlatformUtils_getFullStackTrace) {
                            jstring trace = static_cast< jstring >(
                                env->CallStaticObjectMethod(
                                    jvm->GetJavaMembers().c_PlatformUtils,
                                    jvm->GetJavaMembers()
                                        .m_PlatformUtils_getFullStackTrace,
                                    err));
 
                            trace0 =
                                JavaStringToCString(env, trace, &traceLen);
                        }

                        env->DeleteLocalRef(cls);

                        int clsNameLen;
                        std::string clsName0 = JavaStringToCString(env, clsName, &clsNameLen);

                        int msgLen;
                        std::string msg0 = JavaStringToCString(env, msg, &msgLen);

                        if (errInfo)
                        {
                            JniErrorInfo errInfo0(IGNITE_JNI_ERR_GENERIC, clsName0.c_str(), msg0.c_str());

                            *errInfo = errInfo0;
                        }

                        // Get error additional data (if any).
                        jbyteArray errData = nullptr;
                        if (jvm->GetMembers().c_PlatformUtils
                            && jvm->GetMembers().m_PlatformUtils_errData) {
                            errData = static_cast< jbyteArray >(
                                env->CallStaticObjectMethod(
                                    jvm->GetMembers().c_PlatformUtils,
                                    jvm->GetMembers().m_PlatformUtils_errData,
                                    err));
                        }

                        if (errData)
                        {
                            jbyte* errBytesNative = env->GetByteArrayElements(errData, NULL);

                            int errBytesLen = env->GetArrayLength(errData);

                            if (hnds.error)
                                hnds.error(hnds.target, IGNITE_JNI_ERR_GENERIC, clsName0.c_str(), clsNameLen, msg0.c_str(),
                                    msgLen, trace0.c_str(), traceLen, errBytesNative, errBytesLen);

                            env->ReleaseByteArrayElements(errData, errBytesNative, JNI_ABORT);
                        }
                        else
                        {
                            if (hnds.error)
                                hnds.error(hnds.target, IGNITE_JNI_ERR_GENERIC, clsName0.c_str(), clsNameLen, msg0.c_str(),
                                    msgLen, trace0.c_str(), traceLen, NULL, 0);
                        }

                        env->DeleteLocalRef(err);
                    }
                }

                /**
                  * Convert local reference to global.
                  */
                jobject JniContext::LocalToGlobal(JNIEnv* env, jobject localRef) {
                    if (localRef) {
                        jobject globalRef = env->NewGlobalRef(localRef);

                        env->DeleteLocalRef(localRef); // Clear local ref irrespective of result.

                        if (!globalRef)
                            ExceptionCheck(env);

                        return globalRef;
                    }
                    else
                        return NULL;
                }

                JNIEXPORT void JNICALL JniConsoleWrite(JNIEnv *env, jclass, jstring str, jboolean isErr) {
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

                JNIEXPORT void JNICALL JniLoggerLog(JNIEnv *env, jclass, jlong envPtr, jint level, jstring message, jstring category, jstring errorInfo, jlong memPtr) {
                    int messageLen;
                    char* messageChars = StringToChars(env, message, &messageLen);

                    int categoryLen;
                    char* categoryChars = StringToChars(env, category, &categoryLen);

                    int errorInfoLen;
                    char* errorInfoChars = StringToChars(env, errorInfo, &errorInfoLen);

                    IGNITE_SAFE_PROC(env, envPtr, LoggerLogHandler, loggerLog, level, messageChars, messageLen, categoryChars, categoryLen, errorInfoChars, errorInfoLen, memPtr);

                    if (messageChars)
                        delete[] messageChars;

                    if (categoryChars)
                        delete[] categoryChars;

                    if (errorInfoChars)
                        delete[] errorInfoChars;
                }

                JNIEXPORT jboolean JNICALL JniLoggerIsLevelEnabled(JNIEnv *env, jclass, jlong envPtr, jint level) {
                    IGNITE_SAFE_FUNC(env, envPtr, LoggerIsLevelEnabledHandler, loggerIsLevelEnabled, level);
                }

                JNIEXPORT jlong JNICALL JniInLongOutLong(JNIEnv *env, jclass, jlong envPtr, jint type, jlong val) {
                    IGNITE_SAFE_FUNC(env, envPtr, InLongOutLongHandler, inLongOutLong, type, val);
                }

                JNIEXPORT jlong JNICALL JniInLongLongLongObjectOutLong(JNIEnv *env, jclass, jlong envPtr, jint type, jlong val1, jlong val2, jlong val3, jobject arg) {
                    IGNITE_SAFE_FUNC(env, envPtr, InLongLongLongObjectOutLongHandler, inLongLongLongObjectOutLong, type, val1, val2, val3, arg);
                }
            } //  namespace java
        }  //  namespace jni
    }  //  namespace odbc
}  //  namespace ignite