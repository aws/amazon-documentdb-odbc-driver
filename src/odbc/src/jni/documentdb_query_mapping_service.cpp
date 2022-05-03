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
      connectionProperties.Get()->jniContext_;
  SharedPointer< GlobalJObject > queryMappingService;
  JniErrorCode success = jniContext.Get()->DocumentDbQueryMappingServiceCtor(
      connectionProperties.Get()->connectionProperties_,
      databaseMetadata.Get()->databaseMetadata_, queryMappingService, errInfo);
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
  success = jniContext_.Get()->ListSize(columnMetadata, listSize, errInfo);
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    return false;
  }

  for (int32_t index = 0; index < listSize; index++) {
    SharedPointer< GlobalJObject > jdbcColumnMetadata;
    success = jniContext_.Get()->ListGet(columnMetadata, index,
                                         jdbcColumnMetadata, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }

    int32_t ordinal;
    success = jniContext_.Get()->JdbcColumnMetadataGetOrdinal(
        jdbcColumnMetadata, ordinal, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    bool autoIncrement;
    success = jniContext_.Get()->JdbcColumnMetadataIsAutoIncrement(
        jdbcColumnMetadata, autoIncrement, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    bool caseSensitive;
    success = jniContext_.Get()->JdbcColumnMetadataIsCaseSensitive(
        jdbcColumnMetadata, caseSensitive, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    bool searchable;
    success = jniContext_.Get()->JdbcColumnMetadataIsSearchable(
        jdbcColumnMetadata, searchable, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    bool currency;
    success = jniContext_.Get()->JdbcColumnMetadataIsCurrency(
        jdbcColumnMetadata, currency, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    int32_t nullable;
    success = jniContext_.Get()->JdbcColumnMetadataGetNullable(
        jdbcColumnMetadata, nullable, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    bool isSigned;
    success = jniContext_.Get()->JdbcColumnMetadataIsSigned(jdbcColumnMetadata,
                                                            isSigned, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    int32_t columnDisplaySize;
    success = jniContext_.Get()->JdbcColumnMetadataGetColumnDisplaySize(
        jdbcColumnMetadata, columnDisplaySize, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    boost::optional< std::string > columnLabel;
    success = jniContext_.Get()->JdbcColumnMetadataGetColumnLabel(
        jdbcColumnMetadata, columnLabel, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    boost::optional< std::string > columnName;
    success = jniContext_.Get()->JdbcColumnMetadataGetColumnName(
        jdbcColumnMetadata, columnName, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    boost::optional< std::string > schemaName;
    success = jniContext_.Get()->JdbcColumnMetadataGetSchemaName(
        jdbcColumnMetadata, schemaName, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    int32_t precision;
    success = jniContext_.Get()->JdbcColumnMetadataGetPrecision(
        jdbcColumnMetadata, precision, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    int32_t scale;
    success = jniContext_.Get()->JdbcColumnMetadataGetScale(jdbcColumnMetadata,
                                                            scale, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    boost::optional< std::string > tableName;
    success = jniContext_.Get()->JdbcColumnMetadataGetTableName(
        jdbcColumnMetadata, tableName, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    boost::optional< std::string > catalogName;
    success = jniContext_.Get()->JdbcColumnMetadataGetCatalogName(
        jdbcColumnMetadata, catalogName, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    int32_t columnType;
    success = jniContext_.Get()->JdbcColumnMetadataGetColumnType(
        jdbcColumnMetadata, columnType, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    boost::optional< std::string > columnTypeName;
    success = jniContext_.Get()->JdbcColumnMetadataGetColumnTypeName(
        jdbcColumnMetadata, columnTypeName, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    bool readOnly;
    success = jniContext_.Get()->JdbcColumnMetadataIsReadOnly(
        jdbcColumnMetadata, readOnly, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    bool writable;
    success = jniContext_.Get()->JdbcColumnMetadataIsWritable(
        jdbcColumnMetadata, writable, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    bool definitelyWritable;
    success = jniContext_.Get()->JdbcColumnMetadataIsDefinitelyWritable(
        jdbcColumnMetadata, definitelyWritable, errInfo);
    if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      return false;
    }
    boost::optional< std::string > columnClassName;
    success = jniContext_.Get()->JdbcColumnMetadataGetColumnClassName(
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
  JniErrorCode success = jniContext_.Get()->DocumentDbQueryMappingServiceGet(
      queryMappingService_, sql, maxRowCount, mqlQueryContext, errInfo);
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    return nullptr;
  }

  std::string collectionName;
  success = jniContext_.Get()->DocumentdbMqlQueryContextGetCollectionName(
      mqlQueryContext, collectionName, errInfo);
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    return nullptr;
  }

  SharedPointer< DocumentDbMqlQueryContext > documentDbMqlQueryContext =
      new DocumentDbMqlQueryContext(collectionName);

  SharedPointer< GlobalJObject > aggregateOperations;
  success = jniContext_.Get()
                ->DocumentdbMqlQueryContextGetAggregateOperationsAsStrings(
                    mqlQueryContext, aggregateOperations, errInfo);
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    return nullptr;
  }
  if (!ReadListOfString(
          jniContext_, aggregateOperations,
          documentDbMqlQueryContext.Get()->GetAggregateOperations())) {
    return nullptr;
  }

  SharedPointer< GlobalJObject > paths;
  success = jniContext_.Get()->DocumentdbMqlQueryContextGetPaths(
      mqlQueryContext, paths, errInfo);
  if (success != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
    return nullptr;
  }
  if (!ReadListOfString(jniContext_, paths,
                        documentDbMqlQueryContext.Get()->GetPaths())) {
    return nullptr;
  }

  SharedPointer< GlobalJObject > columnMetadata;
  success = jniContext_.Get()->DocumentdbMqlQueryContextGetColumnMetadata(
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
