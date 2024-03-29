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

#ifndef _DOCUMENTDB_ODBC_JNI_JAVA
#define _DOCUMENTDB_ODBC_JNI_JAVA

#include <documentdb/odbc/common/common.h>
#include <documentdb/odbc/common/concurrent.h>
#include <documentdb/odbc/documentdb_error.h>
#include <boost/optional.hpp>
#include <jni.h>
#include <stdint.h>

#include <boost/optional.hpp>
#include <vector>

using documentdb::odbc::common::concurrent::SharedPointer;

namespace documentdb {
namespace odbc {
namespace jni {
namespace java {

/* Handlers for callbacks from Java. */
typedef int64_t(JNICALL* CacheStoreCreateHandler)(void* target, int64_t memPtr);
typedef int(JNICALL* CacheStoreInvokeHandler)(void* target, int64_t objPtr,
                                              int64_t memPtr);
typedef void(JNICALL* CacheStoreDestroyHandler)(void* target, int64_t objPtr);
typedef int64_t(JNICALL* CacheStoreSessionCreateHandler)(void* target,
                                                         int64_t storePtr);

typedef int64_t(JNICALL* CacheEntryFilterCreateHandler)(void* target,
                                                        int64_t memPtr);
typedef int(JNICALL* CacheEntryFilterApplyHandler)(void* target, int64_t ptr,
                                                   int64_t memPtr);
typedef void(JNICALL* CacheEntryFilterDestroyHandler)(void* target,
                                                      int64_t ptr);

typedef void(JNICALL* CacheInvokeHandler)(void* target, int64_t inMemPtr,
                                          int64_t outMemPtr);

typedef void(JNICALL* ComputeTaskMapHandler)(void* target, int64_t taskPtr,
                                             int64_t inMemPtr,
                                             int64_t outMemPtr);
typedef int(JNICALL* ComputeTaskJobResultHandler)(void* target, int64_t taskPtr,
                                                  int64_t jobPtr,
                                                  int64_t memPtr);
typedef void(JNICALL* ComputeTaskReduceHandler)(void* target, int64_t taskPtr);
typedef void(JNICALL* ComputeTaskCompleteHandler)(void* target, int64_t taskPtr,
                                                  int64_t memPtr);
typedef int(JNICALL* ComputeJobSerializeHandler)(void* target, int64_t jobPtr,
                                                 int64_t memPtr);
typedef int64_t(JNICALL* ComputeJobCreateHandler)(void* target, int64_t memPtr);
typedef void(JNICALL* ComputeJobExecuteHandler)(void* target, int64_t jobPtr,
                                                int cancel, int64_t memPtr);
typedef void(JNICALL* ComputeJobCancelHandler)(void* target, int64_t jobPtr);
typedef void(JNICALL* ComputeJobDestroyHandler)(void* target, int64_t jobPtr);

typedef void(JNICALL* ContinuousQueryListenerApplyHandler)(void* target,
                                                           int64_t lsnrPtr,
                                                           int64_t memPtr);
typedef int64_t(JNICALL* ContinuousQueryFilterCreateHandler)(void* target,
                                                             int64_t memPtr);
typedef int(JNICALL* ContinuousQueryFilterApplyHandler)(void* target,
                                                        int64_t filterPtr,
                                                        int64_t memPtr);
typedef void(JNICALL* ContinuousQueryFilterReleaseHandler)(void* target,
                                                           int64_t filterPtr);

typedef void(JNICALL* DataStreamerTopologyUpdateHandler)(void* target,
                                                         int64_t ldrPtr,
                                                         int64_t topVer,
                                                         int topSize);
typedef void(JNICALL* DataStreamerStreamReceiverInvokeHandler)(
    void* target, int64_t ptr, void* cache, int64_t memPtr,
    unsigned char keepPortable);

typedef void(JNICALL* FutureByteResultHandler)(void* target, int64_t futAddr,
                                               int res);
typedef void(JNICALL* FutureBoolResultHandler)(void* target, int64_t futAddr,
                                               int res);
typedef void(JNICALL* FutureShortResultHandler)(void* target, int64_t futAddr,
                                                int res);
typedef void(JNICALL* FutureCharResultHandler)(void* target, int64_t futAddr,
                                               int res);
typedef void(JNICALL* FutureIntResultHandler)(void* target, int64_t futAddr,
                                              int res);
typedef void(JNICALL* FutureFloatResultHandler)(void* target, int64_t futAddr,
                                                float res);
typedef void(JNICALL* FutureLongResultHandler)(void* target, int64_t futAddr,
                                               int64_t res);
typedef void(JNICALL* FutureDoubleResultHandler)(void* target, int64_t futAddr,
                                                 double res);
typedef void(JNICALL* FutureObjectResultHandler)(void* target, int64_t futAddr,
                                                 int64_t memPtr);
typedef void(JNICALL* FutureNullResultHandler)(void* target, int64_t futAddr);
typedef void(JNICALL* FutureErrorHandler)(void* target, int64_t futAddr,
                                          int64_t memPtr);

typedef void(JNICALL* LifecycleEventHandler)(void* target, int64_t ptr,
                                             int evt);

typedef void(JNICALL* MemoryReallocateHandler)(void* target, int64_t memPtr,
                                               int cap);

typedef int64_t(JNICALL* MessagingFilterCreateHandler)(void* target,
                                                       int64_t memPtr);
typedef int(JNICALL* MessagingFilterApplyHandler)(void* target, int64_t ptr,
                                                  int64_t memPtr);
typedef void(JNICALL* MessagingFilterDestroyHandler)(void* target, int64_t ptr);

typedef int64_t(JNICALL* EventFilterCreateHandler)(void* target,
                                                   int64_t memPtr);
typedef int(JNICALL* EventFilterApplyHandler)(void* target, int64_t ptr,
                                              int64_t memPtr);
typedef void(JNICALL* EventFilterDestroyHandler)(void* target, int64_t ptr);

typedef int64_t(JNICALL* ServiceInitHandler)(void* target, int64_t memPtr);
typedef void(JNICALL* ServiceExecuteHandler)(void* target, int64_t svcPtr,
                                             int64_t memPtr);
typedef void(JNICALL* ServiceCancelHandler)(void* target, int64_t svcPtr,
                                            int64_t memPtr);
typedef void(JNICALL* ServiceInvokeMethodHandler)(void* target, int64_t svcPtr,
                                                  int64_t inMemPtr,
                                                  int64_t outMemPtr);
typedef int(JNICALL* ClusterNodeFilterApplyHandler)(void* target,
                                                    int64_t memPtr);

typedef int64_t(JNICALL* NodeInfoHandler)(void* target, int64_t memPtr);

typedef void(JNICALL* OnStartHandler)(void* target, void* proc, int64_t memPtr);
typedef void(JNICALL* OnStopHandler)(void* target);
typedef void(JNICALL* ErrorHandler)(void* target, JniErrorCode errCode,
                                    const char* errClsChars, int errClsCharsLen,
                                    const char* errMsgChars, int errMsgCharsLen,
                                    const char* stackTraceChars,
                                    int stackTraceCharsLen, void* errData,
                                    int errDataLen);

typedef int64_t(JNICALL* ExtensionCallbackInLongOutLongHandler)(void* target,
                                                                int typ,
                                                                int64_t arg1);
typedef int64_t(JNICALL* ExtensionCallbackInLongLongOutLongHandler)(
    void* target, int typ, int64_t arg1, int64_t arg2);

typedef void(JNICALL* OnClientDisconnectedHandler)(void* target);
typedef void(JNICALL* OnClientReconnectedHandler)(
    void* target, unsigned char clusterRestarted);

typedef int64_t(JNICALL* AffinityFunctionInitHandler)(void* target,
                                                      int64_t memPtr,
                                                      void* baseFunc);
typedef int(JNICALL* AffinityFunctionPartitionHandler)(void* target,
                                                       int64_t ptr,
                                                       int64_t memPtr);
typedef void(JNICALL* AffinityFunctionAssignPartitionsHandler)(
    void* target, int64_t ptr, int64_t inMemPtr, int64_t outMemPtr);
typedef void(JNICALL* AffinityFunctionRemoveNodeHandler)(void* target,
                                                         int64_t ptr,
                                                         int64_t memPtr);
typedef void(JNICALL* AffinityFunctionDestroyHandler)(void* target,
                                                      int64_t ptr);

typedef void(JNICALL* ConsoleWriteHandler)(const char* chars, int charsLen,
                                           unsigned char isErr);

typedef void(JNICALL* LoggerLogHandler)(
    void* target, int level, const char* messageChars, int messageCharsLen,
    const char* categoryChars, int categoryCharsLen, const char* errorInfoChars,
    int errorInfoCharsLen, int64_t memPtr);
typedef bool(JNICALL* LoggerIsLevelEnabledHandler)(void* target, int level);

typedef int64_t(JNICALL* InLongOutLongHandler)(void* target, int type,
                                               int64_t val);
typedef int64_t(JNICALL* InLongLongLongObjectOutLongHandler)(
    void* target, int type, int64_t val1, int64_t val2, int64_t val3,
    void* arg);

/**
 * Is Java 9 or later is used.
 *
 * @return true if the Java 9 or later is in use.
 */
bool DOCUMENTDB_IMPORT_EXPORT IsJava9OrLater();

/**
 * Builds the JVM options
 */
void BuildJvmOptions(const std::string& cp, std::vector< char* >& opts,
                     int xms = 256, int xmx = 1024);
/**
 * JNI handlers holder.
 */
struct JniHandlers {
  void* target;

  ErrorHandler error;

  LoggerLogHandler loggerLog;
  LoggerIsLevelEnabledHandler loggerIsLevelEnabled;

  InLongOutLongHandler inLongOutLong;
  InLongLongLongObjectOutLongHandler inLongLongLongObjectOutLong;
};

/**
 * JNI Java members.
 */
struct JniJavaMembers {
  jclass c_Class;
  jmethodID m_Class_getName;

  jclass c_Throwable;
  jmethodID m_Throwable_getMessage;
  jmethodID m_Throwable_printStackTrace;

  jclass c_String;

  jclass c_PlatformUtils;
  jmethodID m_PlatformUtils_getFullStackTrace;

  /**
   * Constructor.
   */
  void Initialize(JNIEnv* env);

  /**
   * Destroy members releasing all allocated classes.
   */
  void Destroy(JNIEnv* env);

  /**
   * Write error information.
   */
  bool WriteErrorInfo(JNIEnv* env, char** errClsName, int* errClsNameLen,
                      char** errMsg, int* errMsgLen, char** stackTrace,
                      int* stackTraceLen);
};

/**
 * JNI members.
 */
struct JniMembers {
  jclass c_DocumentDbConnectionProperties;
  jmethodID m_DocumentDbConnectionPropertiesGetPropertiesFromConnectionString;

  jclass c_DocumentDbConnection;
  jmethodID m_DocumentDbConnectionGetSshLocalPort;
  jmethodID m_DocumentDbConnectionIsSshTunnelActive;
  jmethodID m_DocumentDbConnectionGetDatabaseMetadata;
  jmethodID m_DocumentDbConnectionGetConnectionProperties;
  jmethodID m_DocumentDbClose;

  jclass c_DocumentDbDatabaseSchemaMetadata;
  jmethodID m_DocumentDbDatabaseSchemaMetadataGetSchemaName;

  jclass c_DriverManager;
  jmethodID m_DriverManagerGetConnection;

  jclass c_Connection;
  jmethodID m_ConnectionClose;
  jmethodID m_ConnectionGetMetaData;

  jclass c_ResultSet;
  jmethodID m_ResultSetClose;
  jmethodID m_ResultSetNext;
  jmethodID m_ResultSetGetStringByIndex;
  jmethodID m_ResultSetGetIntByIndex;
  jmethodID m_ResultSetGetStringByName;
  jmethodID m_ResultSetGetIntByName;
  jmethodID m_ResultSetGetRow;
  jmethodID m_ResultSetWasNull;

  jclass c_DatabaseMetaData;
  jmethodID m_DatabaseMetaDataGetTables;
  jmethodID m_DatabaseMetaDataGetColumns;
  jmethodID m_DatabaseMetaDataGetPrimaryKeys;
  jmethodID m_DatabaseMetaDataGetImportedKeys;
  jmethodID m_DatabaseMetaDataGetTypeInfo;

  jclass c_List;
  jmethodID m_ListSize;
  jmethodID m_ListGet;

  jclass c_DocumentDbMqlQueryContext;
  jmethodID m_DocumentDbMqlQueryContextGetAggregateOperationsAsStrings;
  jmethodID m_DocumentDbMqlQueryContextGetColumnMetadata;
  jmethodID m_DocumentDbMqlQueryContextGetCollectionName;
  jmethodID m_DocumentDbMqlQueryContextGetPaths;

  jclass c_JdbcColumnMetadata;
  jmethodID m_JdbcColumnMetadataGetOrdinal;
  jmethodID m_JdbcColumnMetadataIsAutoIncrement;
  jmethodID m_JdbcColumnMetadataIsCaseSensitive;
  jmethodID m_JdbcColumnMetadataIsSearchable;
  jmethodID m_JdbcColumnMetadataIsCurrency;
  jmethodID m_JdbcColumnMetadataGetNullable;
  jmethodID m_JdbcColumnMetadataIsSigned;
  jmethodID m_JdbcColumnMetadataGetColumnDisplaySize;
  jmethodID m_JdbcColumnMetadataGetColumnLabel;
  jmethodID m_JdbcColumnMetadataGetColumnName;
  jmethodID m_JdbcColumnMetadataGetSchemaName;
  jmethodID m_JdbcColumnMetadataGetPrecision;
  jmethodID m_JdbcColumnMetadataGetScale;
  jmethodID m_JdbcColumnMetadataGetTableName;
  jmethodID m_JdbcColumnMetadataGetCatalogName;
  jmethodID m_JdbcColumnMetadataGetColumnType;
  jmethodID m_JdbcColumnMetadataGetColumnTypeName;
  jmethodID m_JdbcColumnMetadataIsReadOnly;
  jmethodID m_JdbcColumnMetadataIsWritable;
  jmethodID m_JdbcColumnMetadataIsDefinitelyWritable;
  jmethodID m_JdbcColumnMetadataGetColumnClassName;

  jclass c_DocumentDbQueryMappingService;
  jmethodID m_DocumentDbQueryMappingServiceCtor;
  jmethodID m_DocumentDbQueryMappingServiceGet;

  jclass c_IgniteException;

  jclass c_PlatformIgnition;
  jmethodID m_PlatformIgnition_start;
  jmethodID m_PlatformIgnition_instance;
  jmethodID m_PlatformIgnition_environmentPointer;
  jmethodID m_PlatformIgnition_stop;
  jmethodID m_PlatformIgnition_stopAll;

  jclass c_PlatformTarget;
  jmethodID m_PlatformTarget_inLongOutLong;
  jmethodID m_PlatformTarget_inStreamOutLong;
  jmethodID m_PlatformTarget_inStreamOutObject;
  jmethodID m_PlatformTarget_outStream;
  jmethodID m_PlatformTarget_outObject;
  jmethodID m_PlatformTarget_inStreamAsync;
  jmethodID m_PlatformTarget_inStreamOutObjectAsync;
  jmethodID m_PlatformTarget_inStreamOutStream;
  jmethodID m_PlatformTarget_inObjectStreamOutObjectStream;

  jclass c_PlatformUtils;
  jmethodID m_PlatformUtils_reallocate;
  jmethodID m_PlatformUtils_errData;

  /**
   * Constructor.
   */
  void Initialize(JNIEnv* env);

  /**
   * Destroy members releasing all allocated classes.
   */
  void Destroy(JNIEnv* env);
};

/**
 * Guard to ensure global reference cleanup.
 */
class GlobalJObject {
 public:
  GlobalJObject(JNIEnv* e, jobject obj);

  ~GlobalJObject();

  jobject GetRef() const;

 private:
  /** Environment. */
  JNIEnv* env;

  /** Target reference. */
  jobject ref;

  DOCUMENTDB_NO_COPY_ASSIGNMENT(GlobalJObject);
};

/**
 * JNI JVM wrapper.
 */
class DOCUMENTDB_IMPORT_EXPORT JniJvm {
 public:
  /**
   * Default constructor for uninitialized JVM.
   */
  JniJvm();

  /**
   * Constructor.
   *
   * @param jvm JVM.
   * @param javaMembers Java members.
   * @param members Members.
   */
  JniJvm(JavaVM* jvm, JniJavaMembers const& javaMembers,
         JniMembers const& members);

  /**
   * Get JVM.
   *
   * @param JVM.
   */
  JavaVM* GetJvm();

  /**
   * Get Java members.
   *
   * @param Java members.
   */
  JniJavaMembers& GetJavaMembers();

  /**
   * Get members.
   *
   * @param Members.
   */
  JniMembers& GetMembers();

 private:
  /** JVM. */
  JavaVM* jvm;

  /** Java members. */
  JniJavaMembers javaMembers;

  /** Members. */
  JniMembers members;
};

/**
 * JNI error information.
 */
struct DOCUMENTDB_IMPORT_EXPORT JniErrorInfo {
  JniErrorCode code;
  std::string errCls;
  std::string errMsg;

  /**
   * Default constructor. Creates empty error info.
   */
  JniErrorInfo();

  /**
   * Constructor.
   *
   * @param code Code.
   * @param errCls Error class.
   * @param errMsg Error message.
   */
  JniErrorInfo(JniErrorCode code, const char* errCls, const char* errMsg);

  /**
   * Copy constructor.
   *
   * @param other Other instance.
   */
  JniErrorInfo(const JniErrorInfo& other);

  /**
   * Assignment operator overload.
   *
   * @param other Other instance.
   * @return This instance.
   */
  JniErrorInfo& operator=(const JniErrorInfo& other);

  /**
   * Destructor.
   */
  ~JniErrorInfo() = default;
};

/**
 * Unmanaged context.
 */
class DOCUMENTDB_IMPORT_EXPORT JniContext {
 public:
  static JniContext* Create(char** opts, int optsLen, JniHandlers const& hnds,
                            JniErrorInfo& errInfo);
  static int Reallocate(int64_t memPtr, int cap);
  static void Detach();
  static void Release(jobject obj);

  std::string JavaStringToCppString(
      const SharedPointer< GlobalJObject >& jstring, JniErrorInfo& errInfo);

  int64_t TargetInLongOutLong(jobject obj, int type, int64_t memPtr,
                              JniErrorInfo* errInfo);
  int64_t TargetInStreamOutLong(jobject obj, int type, int64_t memPtr,
                                JniErrorInfo* errInfo);
  jobject TargetOutObject(jobject obj, int opType, JniErrorInfo* errInfo);
  void TargetInStreamOutStream(jobject obj, int opType, int64_t inMemPtr,
                               int64_t outMemPtr, JniErrorInfo* errInfo);
  jobject TargetInStreamOutObject(jobject obj, int type, int64_t memPtr,
                                  JniErrorInfo* errInfo);
  void TargetOutStream(jobject obj, int opType, int64_t memPtr,
                       JniErrorInfo* errInfo);

  JniErrorCode DriverManagerGetConnection(
      const char* connectionString, SharedPointer< GlobalJObject >& connection,
      JniErrorInfo& errInfo);
  JniErrorCode ConnectionClose(const SharedPointer< GlobalJObject >& connection,
                               JniErrorInfo& errInfo);
  JniErrorCode ConnectionGetMetaData(
      const SharedPointer< GlobalJObject >& connection,
      SharedPointer< GlobalJObject >& databaseMetaData, JniErrorInfo& errInfo);

  JniErrorCode DocumentDbConnectionIsSshTunnelActive(
      const SharedPointer< GlobalJObject >& connection, bool& isActive,
      JniErrorInfo& errInfo);
  JniErrorCode DocumentDbConnectionGetSshLocalPort(
      const SharedPointer< GlobalJObject >& connection, int32_t& result,
      JniErrorInfo& errInfo);
  JniErrorCode DocumentDbConnectionGetDatabaseMetadata(
      const SharedPointer< GlobalJObject >& connection,
      SharedPointer< GlobalJObject >& metadata, JniErrorInfo& errInfo);
  JniErrorCode DocumentDbConnectionGetConnectionProperties(
      const SharedPointer< GlobalJObject >& connection,
      SharedPointer< GlobalJObject >& connectionProperties,
      JniErrorInfo& errInfo);

  JniErrorCode DocumentDbDatabaseSchemaMetadataGetSchemaName(
      const SharedPointer< GlobalJObject >& databaseMetadata,
      std::string& value, bool& wasNull, JniErrorInfo& errInfo);

  JniErrorCode DatabaseMetaDataGetTables(
      const SharedPointer< GlobalJObject >& databaseMetaData,
      const boost::optional< std::string >& catalog,
      const boost::optional< std::string >& schemaPattern,
      const std::string& tableNamePattern,
      const boost::optional< std::vector< std::string > >& types,
      SharedPointer< GlobalJObject >& resultSet, JniErrorInfo& errInfo);

  JniErrorCode DatabaseMetaDataGetColumns(
      const SharedPointer< GlobalJObject >& databaseMetaData,
      const boost::optional< std::string >& catalog,
      const boost::optional< std::string >& schemaPattern,
      const std::string& tableNamePattern, const std::string& columnNamePattern,
      SharedPointer< GlobalJObject >& resultSet, JniErrorInfo& errInfo);

  JniErrorCode DatabaseMetaDataGetPrimaryKeys(
      const SharedPointer< GlobalJObject >& databaseMetaData,
      const boost::optional< std::string >& catalog,
      const boost::optional< std::string >& schema,
      const boost::optional< std::string >& table,
      SharedPointer< GlobalJObject >& resultSet, JniErrorInfo& errInfo);

  JniErrorCode DatabaseMetaDataGetImportedKeys(
      const SharedPointer< GlobalJObject >& databaseMetaData,
      const boost::optional< std::string >& catalog,
      const boost::optional< std::string >& schema, const std::string& table,
      SharedPointer< GlobalJObject >& resultSet, JniErrorInfo& errInfo);

  JniErrorCode DatabaseMetaDataGetTypeInfo(
      const SharedPointer< GlobalJObject >& databaseMetaData,
      SharedPointer< GlobalJObject >& resultSet, JniErrorInfo& errInfo);

  JniErrorCode ResultSetClose(const SharedPointer< GlobalJObject >& resultSet,
                              JniErrorInfo& errInfo);
  JniErrorCode ResultSetNext(const SharedPointer< GlobalJObject >& resultSet,
                             bool& hasNext, JniErrorInfo& errInfo);
  JniErrorCode ResultSetGetString(
      const SharedPointer< GlobalJObject >& resultSet, int columnIndex,
      boost::optional< std::string >& value, JniErrorInfo& errInfo);
  JniErrorCode ResultSetGetString(
      const SharedPointer< GlobalJObject >& resultSet,
      const std::string& columnName, boost::optional< std::string >& value,
      JniErrorInfo& errInfo);
  JniErrorCode ResultSetGetInt(const SharedPointer< GlobalJObject >& resultSet,
                               int columnIndex, boost::optional< int >& value,

                               JniErrorInfo& errInfo);
  JniErrorCode ResultSetGetInt(const SharedPointer< GlobalJObject >& resultSet,
                               const std::string& columnName,
                               boost::optional< int >& value,
                               JniErrorInfo& errInfo);
  JniErrorCode ResultSetGetRow(const SharedPointer< GlobalJObject >& resultSet,
                               boost::optional< int >& value,
                               JniErrorInfo& errInfo);
  JniErrorCode ResultSetWasNull(const SharedPointer< GlobalJObject >& resultSet,
                                bool& value, JniErrorInfo& errInfo);

  JniErrorCode ListSize(const SharedPointer< GlobalJObject >& list,
                        int32_t& size, JniErrorInfo& errInfo);
  JniErrorCode ListGet(const SharedPointer< GlobalJObject >& list,
                       int32_t index, SharedPointer< GlobalJObject >& array,
                       JniErrorInfo& errInfo);

  JniErrorCode DocumentdbMqlQueryContextGetAggregateOperationsAsStrings(
      const SharedPointer< GlobalJObject >& mqlQueryContext,
      SharedPointer< GlobalJObject >& list, JniErrorInfo& errInfo);
  JniErrorCode DocumentdbMqlQueryContextGetColumnMetadata(
      const SharedPointer< GlobalJObject >& mqlQueryContext,
      SharedPointer< GlobalJObject >& columnMetadata, JniErrorInfo& errInfo);
  JniErrorCode DocumentdbMqlQueryContextGetCollectionName(
      const SharedPointer< GlobalJObject >& mqlQueryContext,
      std::string& collectionName, JniErrorInfo& errInfo);
  JniErrorCode DocumentdbMqlQueryContextGetPaths(
      const SharedPointer< GlobalJObject >& mqlQueryContext,
      SharedPointer< GlobalJObject >& list, JniErrorInfo& errInfo);

  JniErrorCode DocumentDbQueryMappingServiceCtor(
      const SharedPointer< GlobalJObject >& connectionProperties,
      const SharedPointer< GlobalJObject >& databaseMetadata,
      SharedPointer< GlobalJObject >& queryMappingService,
      JniErrorInfo& errInfo);
  JniErrorCode DocumentDbQueryMappingServiceGet(
      const SharedPointer< GlobalJObject >& queryMappingService,
      const std::string sql, int64_t maxRowCount,
      SharedPointer< GlobalJObject >& mqlQueryContext, JniErrorInfo& errInfo);

  JniErrorCode JdbcColumnMetadataGetOrdinal(
      const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
      int32_t& ordinal, JniErrorInfo& errInfo);
  JniErrorCode JdbcColumnMetadataIsAutoIncrement(
      const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
      bool& autoIncrement, JniErrorInfo& errInfo);
  JniErrorCode JdbcColumnMetadataIsCaseSensitive(
      const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
      bool& caseSensitive, JniErrorInfo& errInfo);
  JniErrorCode JdbcColumnMetadataIsSearchable(
      const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
      bool& searchable, JniErrorInfo& errInfo);
  JniErrorCode JdbcColumnMetadataIsCurrency(
      const SharedPointer< GlobalJObject >& jdbcColumnMetadata, bool& currency,
      JniErrorInfo& errInfo);
  JniErrorCode JdbcColumnMetadataGetNullable(
      const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
      int32_t& nullable, JniErrorInfo& errInfo);
  JniErrorCode JdbcColumnMetadataIsSigned(
      const SharedPointer< GlobalJObject >& jdbcColumnMetadata, bool& isSigned,
      JniErrorInfo& errInfo);
  JniErrorCode JdbcColumnMetadataGetColumnDisplaySize(
      const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
      int32_t& columnDisplaySize, JniErrorInfo& errInfo);
  JniErrorCode JdbcColumnMetadataGetColumnLabel(
      const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
      boost::optional< std::string >& columnLabel, JniErrorInfo& errInfo);
  JniErrorCode JdbcColumnMetadataGetColumnName(
      const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
      boost::optional< std::string >& columnName, JniErrorInfo& errInfo);
  JniErrorCode JdbcColumnMetadataGetSchemaName(
      const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
      boost::optional< std::string >& schemaName, JniErrorInfo& errInfo);
  JniErrorCode JdbcColumnMetadataGetPrecision(
      const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
      int32_t& precision, JniErrorInfo& errInfo);
  JniErrorCode JdbcColumnMetadataGetScale(
      const SharedPointer< GlobalJObject >& jdbcColumnMetadata, int32_t& scale,
      JniErrorInfo& errInfo);
  JniErrorCode JdbcColumnMetadataGetTableName(
      const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
      boost::optional< std::string >& tableName, JniErrorInfo& errInfo);
  JniErrorCode JdbcColumnMetadataGetCatalogName(
      const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
      boost::optional< std::string >& catalogName, JniErrorInfo& errInfo);
  JniErrorCode JdbcColumnMetadataGetColumnType(
      const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
      int32_t& columnType, JniErrorInfo& errInfo);
  JniErrorCode JdbcColumnMetadataGetColumnTypeName(
      const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
      boost::optional< std::string >& columnTypeName, JniErrorInfo& errInfo);
  JniErrorCode JdbcColumnMetadataIsReadOnly(
      const SharedPointer< GlobalJObject >& jdbcColumnMetadata, bool& readOnly,
      JniErrorInfo& errInfo);
  JniErrorCode JdbcColumnMetadataIsWritable(
      const SharedPointer< GlobalJObject >& jdbcColumnMetadata, bool& writable,
      JniErrorInfo& errInfo);
  JniErrorCode JdbcColumnMetadataIsDefinitelyWritable(
      const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
      bool& definitelyWritable, JniErrorInfo& errInfo);
  JniErrorCode JdbcColumnMetadataGetColumnClassName(
      const SharedPointer< GlobalJObject >& jdbcColumnMetadata,
      boost::optional< std::string >& columnClassName, JniErrorInfo& errInfo);

  jobject CacheOutOpQueryCursor(jobject obj, int type, int64_t memPtr,
                                JniErrorInfo* errInfo);
  jobject CacheOutOpContinuousQuery(jobject obj, int type, int64_t memPtr,
                                    JniErrorInfo* errInfo);

  jobject Acquire(jobject obj);

 private:
  JniJvm* jvm;
  JniHandlers hnds;

  JniContext(JniJvm* jvm, JniHandlers const& hnds);

  JNIEnv* Attach(JniErrorInfo& errInfo);
  void ExceptionCheck(JNIEnv* env);
  void ExceptionCheck(JNIEnv* env, JniErrorInfo* errInfo);

  JniErrorCode CallBooleanMethod(const SharedPointer< GlobalJObject >& object,
                                 const jmethodID& method, bool& value,
                                 JniErrorInfo& errInfo);
  JniErrorCode CallIntMethod(const SharedPointer< GlobalJObject >& object,
                             const jmethodID& method, int32_t& value,
                             JniErrorInfo& errInfo);
  JniErrorCode CallStringMethod(const SharedPointer< GlobalJObject >& object,
                                const jmethodID& method,
                                boost::optional< std::string >& value,
                                JniErrorInfo& errInfo);
  jobject LocalToGlobal(JNIEnv* env, jobject obj);
};

JNIEXPORT jlong JNICALL JniCacheStoreCreate(JNIEnv* env, jclass cls,
                                            jlong envPtr, jlong memPtr);
JNIEXPORT jint JNICALL JniCacheStoreInvoke(JNIEnv* env, jclass cls,
                                           jlong envPtr, jlong objPtr,
                                           jlong memPtr);
JNIEXPORT void JNICALL JniCacheStoreDestroy(JNIEnv* env, jclass cls,
                                            jlong envPtr, jlong objPtr);
JNIEXPORT jlong JNICALL JniCacheStoreSessionCreate(JNIEnv* env, jclass cls,
                                                   jlong envPtr,
                                                   jlong storePtr);

JNIEXPORT jlong JNICALL JniCacheEntryFilterCreate(JNIEnv* env, jclass cls,
                                                  jlong envPtr, jlong memPtr);
JNIEXPORT jint JNICALL JniCacheEntryFilterApply(JNIEnv* env, jclass cls,
                                                jlong envPtr, jlong objPtr,
                                                jlong memPtr);
JNIEXPORT void JNICALL JniCacheEntryFilterDestroy(JNIEnv* env, jclass cls,
                                                  jlong envPtr, jlong objPtr);

JNIEXPORT void JNICALL JniCacheInvoke(JNIEnv* env, jclass cls, jlong envPtr,
                                      jlong inMemPtr, jlong outMemPtr);

JNIEXPORT void JNICALL JniComputeTaskMap(JNIEnv* env, jclass cls, jlong envPtr,
                                         jlong taskPtr, jlong inMemPtr,
                                         jlong outMemPtr);
JNIEXPORT jint JNICALL JniComputeTaskJobResult(JNIEnv* env, jclass cls,
                                               jlong envPtr, jlong taskPtr,
                                               jlong jobPtr, jlong memPtr);
JNIEXPORT void JNICALL JniComputeTaskReduce(JNIEnv* env, jclass cls,
                                            jlong envPtr, jlong taskPtr);
JNIEXPORT void JNICALL JniComputeTaskComplete(JNIEnv* env, jclass cls,
                                              jlong envPtr, jlong taskPtr,
                                              jlong memPtr);
JNIEXPORT jint JNICALL JniComputeJobSerialize(JNIEnv* env, jclass cls,
                                              jlong envPtr, jlong jobPtr,
                                              jlong memPtr);
JNIEXPORT jlong JNICALL JniComputeJobCreate(JNIEnv* env, jclass cls,
                                            jlong envPtr, jlong memPtr);
JNIEXPORT void JNICALL JniComputeJobExecute(JNIEnv* env, jclass cls,
                                            jlong envPtr, jlong jobPtr,
                                            jint cancel, jlong memPtr);
JNIEXPORT void JNICALL JniComputeJobCancel(JNIEnv* env, jclass cls,
                                           jlong envPtr, jlong jobPtr);
JNIEXPORT void JNICALL JniComputeJobDestroy(JNIEnv* env, jclass cls,
                                            jlong envPtr, jlong jobPtr);

JNIEXPORT void JNICALL JniContinuousQueryListenerApply(JNIEnv* env, jclass cls,
                                                       jlong envPtr,
                                                       jlong cbPtr,
                                                       jlong memPtr);
JNIEXPORT jlong JNICALL JniContinuousQueryFilterCreate(JNIEnv* env, jclass cls,
                                                       jlong envPtr,
                                                       jlong memPtr);
JNIEXPORT jint JNICALL JniContinuousQueryFilterApply(JNIEnv* env, jclass cls,
                                                     jlong envPtr,
                                                     jlong filterPtr,
                                                     jlong memPtr);
JNIEXPORT void JNICALL JniContinuousQueryFilterRelease(JNIEnv* env, jclass cls,
                                                       jlong envPtr,
                                                       jlong filterPtr);

JNIEXPORT void JNICALL JniDataStreamerTopologyUpdate(JNIEnv* env, jclass cls,
                                                     jlong envPtr, jlong ldrPtr,
                                                     jlong topVer,
                                                     jint topSize);
JNIEXPORT void JNICALL JniDataStreamerStreamReceiverInvoke(
    JNIEnv* env, jclass cls, jlong envPtr, jlong ptr, jobject cache,
    jlong memPtr, jboolean keepPortable);

JNIEXPORT void JNICALL JniFutureByteResult(JNIEnv* env, jclass cls,
                                           jlong envPtr, jlong futPtr,
                                           jint res);
JNIEXPORT void JNICALL JniFutureBoolResult(JNIEnv* env, jclass cls,
                                           jlong envPtr, jlong futPtr,
                                           jint res);
JNIEXPORT void JNICALL JniFutureShortResult(JNIEnv* env, jclass cls,
                                            jlong envPtr, jlong futPtr,
                                            jint res);
JNIEXPORT void JNICALL JniFutureCharResult(JNIEnv* env, jclass cls,
                                           jlong envPtr, jlong futPtr,
                                           jint res);
JNIEXPORT void JNICALL JniFutureIntResult(JNIEnv* env, jclass cls, jlong envPtr,
                                          jlong futPtr, jint res);
JNIEXPORT void JNICALL JniFutureFloatResult(JNIEnv* env, jclass cls,
                                            jlong envPtr, jlong futPtr,
                                            jfloat res);
JNIEXPORT void JNICALL JniFutureLongResult(JNIEnv* env, jclass cls,
                                           jlong envPtr, jlong futPtr,
                                           jlong res);
JNIEXPORT void JNICALL JniFutureDoubleResult(JNIEnv* env, jclass cls,
                                             jlong envPtr, jlong futPtr,
                                             jdouble res);
JNIEXPORT void JNICALL JniFutureObjectResult(JNIEnv* env, jclass cls,
                                             jlong envPtr, jlong futPtr,
                                             jlong memPtr);
JNIEXPORT void JNICALL JniFutureNullResult(JNIEnv* env, jclass cls,
                                           jlong envPtr, jlong futPtr);
JNIEXPORT void JNICALL JniFutureError(JNIEnv* env, jclass cls, jlong envPtr,
                                      jlong futPtr, jlong memPtr);

JNIEXPORT void JNICALL JniLifecycleEvent(JNIEnv* env, jclass cls, jlong envPtr,
                                         jlong ptr, jint evt);

JNIEXPORT void JNICALL JniMemoryReallocate(JNIEnv* env, jclass cls,
                                           jlong envPtr, jlong memPtr,
                                           jint cap);

JNIEXPORT jlong JNICALL JniMessagingFilterCreate(JNIEnv* env, jclass cls,
                                                 jlong envPtr, jlong memPtr);
JNIEXPORT jint JNICALL JniMessagingFilterApply(JNIEnv* env, jclass cls,
                                               jlong envPtr, jlong ptr,
                                               jlong memPtr);
JNIEXPORT void JNICALL JniMessagingFilterDestroy(JNIEnv* env, jclass cls,
                                                 jlong envPtr, jlong ptr);

JNIEXPORT jlong JNICALL JniEventFilterCreate(JNIEnv* env, jclass cls,
                                             jlong envPtr, jlong memPtr);
JNIEXPORT jint JNICALL JniEventFilterApply(JNIEnv* env, jclass cls,
                                           jlong envPtr, jlong ptr,
                                           jlong memPtr);
JNIEXPORT void JNICALL JniEventFilterDestroy(JNIEnv* env, jclass cls,
                                             jlong envPtr, jlong ptr);

JNIEXPORT jlong JNICALL JniServiceInit(JNIEnv* env, jclass cls, jlong envPtr,
                                       jlong memPtr);
JNIEXPORT void JNICALL JniServiceExecute(JNIEnv* env, jclass cls, jlong envPtr,
                                         jlong svcPtr, jlong memPtr);
JNIEXPORT void JNICALL JniServiceCancel(JNIEnv* env, jclass cls, jlong envPtr,
                                        jlong svcPtr, jlong memPtr);
JNIEXPORT void JNICALL JniServiceInvokeMethod(JNIEnv* env, jclass cls,
                                              jlong envPtr, jlong svcPtr,
                                              jlong inMemPtr, jlong outMemPtr);
JNIEXPORT jint JNICALL JniClusterNodeFilterApply(JNIEnv* env, jclass cls,
                                                 jlong envPtr, jlong memPtr);

JNIEXPORT jlong JNICALL JniNodeInfo(JNIEnv* env, jclass cls, jlong envPtr,
                                    jlong memPtr);

JNIEXPORT void JNICALL JniOnStart(JNIEnv* env, jclass cls, jlong envPtr,
                                  jobject proc, jlong memPtr);
JNIEXPORT void JNICALL JniOnStop(JNIEnv* env, jclass cls, jlong envPtr);

JNIEXPORT jlong JNICALL JniExtensionCallbackInLongOutLong(JNIEnv* env,
                                                          jclass cls,
                                                          jlong envPtr,
                                                          jint typ, jlong arg1);
JNIEXPORT jlong JNICALL JniExtensionCallbackInLongLongOutLong(
    JNIEnv* env, jclass cls, jlong envPtr, jint typ, jlong arg1, jlong arg2);

JNIEXPORT void JNICALL JniOnClientDisconnected(JNIEnv* env, jclass cls,
                                               jlong envPtr);
JNIEXPORT void JNICALL JniOnClientReconnected(JNIEnv* env, jclass cls,
                                              jlong envPtr,
                                              jboolean clusterRestarted);

JNIEXPORT jlong JNICALL JniAffinityFunctionInit(JNIEnv* env, jclass cls,
                                                jlong envPtr, jlong memPtr,
                                                jobject baseFunc);
JNIEXPORT jint JNICALL JniAffinityFunctionPartition(JNIEnv* env, jclass cls,
                                                    jlong envPtr, jlong ptr,
                                                    jlong memPtr);
JNIEXPORT void JNICALL
JniAffinityFunctionAssignPartitions(JNIEnv* env, jclass cls, jlong envPtr,
                                    jlong ptr, jlong inMemPtr, jlong outMemPtr);
JNIEXPORT void JNICALL JniAffinityFunctionRemoveNode(JNIEnv* env, jclass cls,
                                                     jlong envPtr, jlong ptr,
                                                     jlong memPtr);
JNIEXPORT void JNICALL JniAffinityFunctionDestroy(JNIEnv* env, jclass cls,
                                                  jlong envPtr, jlong ptr);

}  // namespace java
}  // namespace jni
}  // namespace odbc
}  // namespace documentdb

#endif  //_DOCUMENTDB_ODBC_JNI_JAVA
