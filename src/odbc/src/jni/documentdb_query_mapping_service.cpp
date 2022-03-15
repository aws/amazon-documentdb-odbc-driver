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

#include <ignite/odbc/common/platform_utils.h>
#include <ignite/odbc/ignite_error.h>
#include <ignite/odbc/jni/documentdb_mql_query_context.h>
#include <ignite/odbc/jni/documentdb_query_mapping_service.h>
#include <ignite/odbc/jni/utils.h>
#include <ignite/odbc/log.h>

#include <vector>

using ignite::odbc::common::GetEnv;
using ignite::odbc::jni::java::JniErrorCode;
using ignite::odbc::jni::java::JniErrorInfo;

namespace ignite {
namespace odbc {
namespace jni {
SharedPointer< DocumentDbQueryMappingService >
DocumentDbQueryMappingService::Create(
    const SharedPointer< DocumentDbConnectionProperties >& connectionProperties,
    const SharedPointer< DocumentDbDatabaseMetadata >& databaseMetadata,
    JniErrorInfo& errInfo) {
  if (!connectionProperties.IsValid() || !databaseMetadata.IsValid()) {
    errInfo = JniErrorInfo(JniErrorCode::IGNITE_JNI_ERR_GENERIC, "",
                           "Connection and metadata must be set.");
    return nullptr;
  }

  SharedPointer< JniContext > jniContext =
      connectionProperties.Get()->_jniContext;
  SharedPointer< GlobalJObject > queryMappingService;
  JniErrorCode success = jniContext.Get()->DocumentDbQueryMappingServiceCtor(
      connectionProperties.Get()->_connectionProperties,
      databaseMetadata.Get()->_databaseMetadata, queryMappingService, errInfo);
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    return nullptr;
  }

  return new DocumentDbQueryMappingService(jniContext, queryMappingService);
}

bool ReadListOfString(SharedPointer< JniContext >& _jniContext,
                      const SharedPointer< GlobalJObject >& sourceList,
                      std::vector< std::string >& targetList) {
  JniErrorCode success;
  JniErrorInfo errInfo;

  int32_t listSize;
  success = _jniContext.Get()->ListSize(sourceList, listSize, errInfo);
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    return false;
  }

  for (int32_t index = 0; index < listSize; index++) {
    SharedPointer< GlobalJObject > operation;
    success = _jniContext.Get()->ListGet(sourceList, index, operation, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    std::string value = _jniContext.Get()->JavaStringToCppString(operation);
    targetList.push_back(value);
  }

  return true;
}

bool DocumentDbQueryMappingService::ReadJdbcColumnMetadata(
    SharedPointer< GlobalJObject > const& columnMetadata,
    std::vector< JdbcColumnMetadata >& columnMetadataList,
    JniErrorInfo& errInfo) {
  JniErrorCode success;
  int32_t listSize;
  success = _jniContext.Get()->ListSize(columnMetadata, listSize, errInfo);
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    return false;
  }

  for (int32_t index = 0; index < listSize; index++) {
    SharedPointer< GlobalJObject > jdbcColumnMetadata;
    success = _jniContext.Get()->ListGet(columnMetadata, index,
                                         jdbcColumnMetadata, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }

    int32_t ordinal;
    success = _jniContext.Get()->JdbcColumnMetadataGetOrdinal(
        jdbcColumnMetadata, ordinal, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    bool autoIncrement;
    success = _jniContext.Get()->JdbcColumnMetadataIsAutoIncrement(
        jdbcColumnMetadata, autoIncrement, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    bool caseSensitive;
    success = _jniContext.Get()->JdbcColumnMetadataIsCaseSensitive(
        jdbcColumnMetadata, caseSensitive, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    bool searchable;
    success = _jniContext.Get()->JdbcColumnMetadataIsSearchable(
        jdbcColumnMetadata, searchable, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    bool currency;
    success = _jniContext.Get()->JdbcColumnMetadataIsCurrency(
        jdbcColumnMetadata, currency, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    int32_t nullable;
    success = _jniContext.Get()->JdbcColumnMetadataGetNullable(
        jdbcColumnMetadata, nullable, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    bool isSigned;
    success = _jniContext.Get()->JdbcColumnMetadataIsSigned(jdbcColumnMetadata,
                                                            isSigned, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    int32_t columnDisplaySize;
    success = _jniContext.Get()->JdbcColumnMetadataGetColumnDisplaySize(
        jdbcColumnMetadata, columnDisplaySize, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    boost::optional< std::string > columnLabel;
    success = _jniContext.Get()->JdbcColumnMetadataGetColumnLabel(
        jdbcColumnMetadata, columnLabel, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    boost::optional< std::string > columnName;
    success = _jniContext.Get()->JdbcColumnMetadataGetColumnName(
        jdbcColumnMetadata, columnName, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    boost::optional< std::string > schemaName;
    success = _jniContext.Get()->JdbcColumnMetadataGetSchemaName(
        jdbcColumnMetadata, schemaName, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    int32_t precision;
    success = _jniContext.Get()->JdbcColumnMetadataGetPrecision(
        jdbcColumnMetadata, precision, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    int32_t scale;
    success = _jniContext.Get()->JdbcColumnMetadataGetScale(jdbcColumnMetadata,
                                                            scale, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    boost::optional< std::string > tableName;
    success = _jniContext.Get()->JdbcColumnMetadataGetTableName(
        jdbcColumnMetadata, tableName, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    boost::optional< std::string > catalogName;
    success = _jniContext.Get()->JdbcColumnMetadataGetCatalogName(
        jdbcColumnMetadata, catalogName, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    int32_t columnType;
    success = _jniContext.Get()->JdbcColumnMetadataGetColumnType(
        jdbcColumnMetadata, columnType, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    boost::optional< std::string > columnTypeName;
    success = _jniContext.Get()->JdbcColumnMetadataGetColumnTypeName(
        jdbcColumnMetadata, columnTypeName, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    bool readOnly;
    success = _jniContext.Get()->JdbcColumnMetadataIsReadOnly(
        jdbcColumnMetadata, readOnly, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    bool writable;
    success = _jniContext.Get()->JdbcColumnMetadataIsWritable(
        jdbcColumnMetadata, writable, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    bool definitelyWritable;
    success = _jniContext.Get()->JdbcColumnMetadataIsDefinitelyWritable(
        jdbcColumnMetadata, definitelyWritable, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    boost::optional< std::string > columnClassName;
    success = _jniContext.Get()->JdbcColumnMetadataGetColumnClassName(
        jdbcColumnMetadata, columnClassName, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }

    JdbcColumnMetadata columnMetadataNode(
        ordinal, autoIncrement, caseSensitive, searchable, currency, nullable,
        isSigned, columnDisplaySize, columnLabel, columnName, schemaName,
        precision, scale, tableName, catalogName, columnType, columnTypeName,
        readOnly, writable, definitelyWritable, columnClassName);
    columnMetadataList.push_back(columnMetadataNode);
  }

  return true;
}

SharedPointer< DocumentDbMqlQueryContext >
DocumentDbQueryMappingService::GetMqlQueryContext(const std::string& sql,
                                                  int maxRowCount,
                                                  JniErrorInfo& errInfo) {
  SharedPointer< GlobalJObject > mqlQueryContext;
  JniErrorCode success = _jniContext.Get()->DocumentDbQueryMappingServiceGet(
      _queryMappingService, sql, maxRowCount, mqlQueryContext, errInfo);
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    return nullptr;
  }

  std::string collectionName;
  success = _jniContext.Get()->DocumentdbMqlQueryContextGetCollectionName(
      mqlQueryContext, collectionName, errInfo);
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    return nullptr;
  }

  SharedPointer< DocumentDbMqlQueryContext > documentDbMqlQueryContext =
      new DocumentDbMqlQueryContext(collectionName);

  SharedPointer< GlobalJObject > aggregateOperations;
  success = _jniContext.Get()
                ->DocumentdbMqlQueryContextGetAggregateOperationsAsStrings(
                    mqlQueryContext, aggregateOperations, errInfo);
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    return nullptr;
  }
  if (!ReadListOfString(
          _jniContext, aggregateOperations,
          documentDbMqlQueryContext.Get()->GetAggregateOperations())) {
    return nullptr;
  }

  SharedPointer< GlobalJObject > paths;
  success = _jniContext.Get()->DocumentdbMqlQueryContextGetPaths(
      mqlQueryContext, paths, errInfo);
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    return nullptr;
  }
  if (!ReadListOfString(_jniContext, paths,
                        documentDbMqlQueryContext.Get()->GetPaths())) {
    return nullptr;
  }

  SharedPointer< GlobalJObject > columnMetadata;
  success = _jniContext.Get()->DocumentdbMqlQueryContextGetColumnMetadata(
      mqlQueryContext, columnMetadata, errInfo);
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    return nullptr;
  }
  ReadJdbcColumnMetadata(columnMetadata,
                         documentDbMqlQueryContext.Get()->GetColumnMetadata(),
                         errInfo);

  return documentDbMqlQueryContext;
}
}  // namespace jni
}  // namespace odbc
}  // namespace ignite
