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

#include <documentdb/odbc/common/common.h>

#include "documentdb/odbc.h"
#include "documentdb/odbc/log.h"
#include "documentdb/odbc/utility.h"

#if defined _WIN64 || defined _WIN32
SQLRETURN SQL_API SQLGetInfo(SQLHDBC conn, SQLUSMALLINT infoType,
                             _Out_writes_bytes_opt_(infoValueMax)
                                 SQLPOINTER infoValue,
                             SQLSMALLINT infoValueMax,
                             _Out_opt_ SQLSMALLINT* length) {
#else
SQLRETURN SQL_API SQLGetInfo(SQLHDBC conn, SQLUSMALLINT infoType,
                             SQLPOINTER infoValue, SQLSMALLINT infoValueMax,
                             SQLSMALLINT* length) {
#endif
  return documentdb::SQLGetInfo(conn, infoType, infoValue, infoValueMax,
                                length);
}

#if defined _WIN64 || defined _WIN32
SQLRETURN SQL_API SQLAllocHandle(SQLSMALLINT type, SQLHANDLE parent,
                                 _Out_ SQLHANDLE* result) {
#else
SQLRETURN SQL_API SQLAllocHandle(SQLSMALLINT type, SQLHANDLE parent,
                                 SQLHANDLE* result) {
#endif
  return documentdb::SQLAllocHandle(type, parent, result);
}

#if defined _WIN64 || defined _WIN32
SQLRETURN SQL_API SQLAllocEnv(_Out_ SQLHENV* env) {
#else
SQLRETURN SQL_API SQLAllocEnv(SQLHENV* env) {
#endif
  return documentdb::SQLAllocEnv(env);
}

#if defined _WIN64 || defined _WIN32
SQLRETURN SQL_API SQLAllocConnect(SQLHENV env, _Out_ SQLHDBC* conn) {
#else
SQLRETURN SQL_API SQLAllocConnect(SQLHENV env, SQLHDBC* conn) {
#endif
  return documentdb::SQLAllocConnect(env, conn);
}

#if defined _WIN64 || defined _WIN32
SQLRETURN SQL_API SQLAllocStmt(SQLHDBC conn, _Out_ SQLHSTMT* stmt) {
#else
SQLRETURN SQL_API SQLAllocStmt(SQLHDBC conn, SQLHSTMT* stmt) {
#endif
  return documentdb::SQLAllocStmt(conn, stmt);
}

SQLRETURN SQL_API SQLFreeHandle(SQLSMALLINT type, SQLHANDLE handle) {
  return documentdb::SQLFreeHandle(type, handle);
}

SQLRETURN SQL_API SQLFreeEnv(SQLHENV env) {
  return documentdb::SQLFreeEnv(env);
}

SQLRETURN SQL_API SQLFreeConnect(SQLHDBC conn) {
  return documentdb::SQLFreeConnect(conn);
}

SQLRETURN SQL_API SQLFreeStmt(SQLHSTMT stmt, SQLUSMALLINT option) {
  return documentdb::SQLFreeStmt(stmt, option);
}

SQLRETURN SQL_API SQLCloseCursor(SQLHSTMT stmt) {
  return documentdb::SQLCloseCursor(stmt);
}

#if defined _WIN64 || defined _WIN32
SQLRETURN SQL_API
SQLDriverConnect(SQLHDBC conn, SQLHWND windowHandle,
                 _In_reads_(inConnectionStringLen) SQLWCHAR* inConnectionString,
                 SQLSMALLINT inConnectionStringLen,
                 _Out_writes_opt_(outConnectionStringBufferLen)
                     SQLWCHAR* outConnectionString,
                 SQLSMALLINT outConnectionStringBufferLen,
                 _Out_opt_ SQLSMALLINT* outConnectionStringLen,
                 SQLUSMALLINT driverCompletion) {
#else
SQLRETURN SQL_API SQLDriverConnect(SQLHDBC conn, SQLHWND windowHandle,
                                   SQLWCHAR* inConnectionString,
                                   SQLSMALLINT inConnectionStringLen,
                                   SQLWCHAR* outConnectionString,
                                   SQLSMALLINT outConnectionStringBufferLen,
                                   SQLSMALLINT* outConnectionStringLen,
                                   SQLUSMALLINT driverCompletion) {
#endif
  return documentdb::SQLDriverConnect(
      conn, windowHandle, inConnectionString, inConnectionStringLen,
      outConnectionString, outConnectionStringBufferLen, outConnectionStringLen,
      driverCompletion);
}

#if defined _WIN64 || defined _WIN32
SQLRETURN SQL_API SQLConnect(SQLHDBC conn,
                             _In_reads_(serverNameLen) SQLWCHAR* serverName,
                             SQLSMALLINT serverNameLen,
                             _In_reads_(userNameLen) SQLWCHAR* userName,
                             SQLSMALLINT userNameLen,
                             _In_reads_(authLen) SQLWCHAR* auth,
                             SQLSMALLINT authLen) {
#else
SQLRETURN SQL_API SQLConnect(SQLHDBC conn, SQLWCHAR* serverName,
                             SQLSMALLINT serverNameLen, SQLWCHAR* userName,
                             SQLSMALLINT userNameLen, SQLWCHAR* auth,
                             SQLSMALLINT authLen) {
#endif
  return documentdb::SQLConnect(conn, serverName, serverNameLen, userName,
                                userNameLen, auth, authLen);
}

SQLRETURN SQL_API SQLDisconnect(SQLHDBC conn) {
  return documentdb::SQLDisconnect(conn);
}

#if defined _WIN64 || defined _WIN32
SQLRETURN SQL_API SQLPrepare(SQLHSTMT stmt,
                             _In_reads_(queryLen) SQLWCHAR* query,
                             SQLINTEGER queryLen) {
#else
SQLRETURN SQL_API SQLPrepare(SQLHSTMT stmt, SQLWCHAR* query,
                             SQLINTEGER queryLen) {
#endif
  return documentdb::SQLPrepare(stmt, query, queryLen);
}

SQLRETURN SQL_API SQLExecute(SQLHSTMT stmt) {
  return documentdb::SQLExecute(stmt);
}

#if defined _WIN64 || defined _WIN32
SQLRETURN SQL_API SQLExecDirect(SQLHSTMT stmt,
                                _In_reads_opt_(queryLen) SQLWCHAR* query,
                                SQLINTEGER queryLen) {
#else
SQLRETURN SQL_API SQLExecDirect(SQLHSTMT stmt, SQLWCHAR* query,
                                SQLINTEGER queryLen) {
#endif
  return documentdb::SQLExecDirect(stmt, query, queryLen);
}

#if defined _WIN64 || defined _WIN32
SQLRETURN SQL_API SQLBindCol(SQLHSTMT stmt, SQLUSMALLINT colNum,
                             SQLSMALLINT targetType,
                             _Inout_updates_opt_(_Inexpressible_(bufferLength))
                                 SQLPOINTER targetValue,
                             SQLLEN bufferLength,
                             _Inout_opt_ SQLLEN* strLengthOrIndicator) {
#else
SQLRETURN SQL_API SQLBindCol(SQLHSTMT stmt, SQLUSMALLINT colNum,
                             SQLSMALLINT targetType, SQLPOINTER targetValue,
                             SQLLEN bufferLength,
                             SQLLEN* strLengthOrIndicator) {
#endif
  return documentdb::SQLBindCol(stmt, colNum, targetType, targetValue,
                                bufferLength, strLengthOrIndicator);
}

SQLRETURN SQL_API SQLFetch(SQLHSTMT stmt) {
  return documentdb::SQLFetch(stmt);
}

SQLRETURN SQL_API SQLFetchScroll(SQLHSTMT stmt, SQLSMALLINT orientation,
                                 SQLLEN offset) {
  return documentdb::SQLFetchScroll(stmt, orientation, offset);
}

#if defined _WIN64 || defined _WIN32
SQLRETURN SQL_API SQLExtendedFetch(SQLHSTMT stmt, SQLUSMALLINT orientation,
                                   SQLLEN offset, _Out_opt_ SQLULEN* rowCount,
                                   _Out_opt_ SQLUSMALLINT* rowStatusArray) {
#else
SQLRETURN SQL_API SQLExtendedFetch(SQLHSTMT stmt, SQLUSMALLINT orientation,
                                   SQLLEN offset, SQLULEN* rowCount,
                                   SQLUSMALLINT* rowStatusArray) {
#endif
  return documentdb::SQLExtendedFetch(stmt, orientation, offset, rowCount,
                                      rowStatusArray);
}

#if defined _WIN64 || defined _WIN32
SQLRETURN SQL_API SQLNumResultCols(SQLHSTMT stmt,
                                   _Out_ SQLSMALLINT* columnNum) {
#else
SQLRETURN SQL_API SQLNumResultCols(SQLHSTMT stmt, SQLSMALLINT* columnNum) {
#endif
  return documentdb::SQLNumResultCols(stmt, columnNum);
}

#if defined _WIN64 || defined _WIN32
SQLRETURN SQL_API SQLTables(
    SQLHSTMT stmt, _In_reads_opt_(catalogNameLen) SQLWCHAR* catalogName,
    SQLSMALLINT catalogNameLen,
    _In_reads_opt_(schemaNameLen) SQLWCHAR* schemaName,
    SQLSMALLINT schemaNameLen, _In_reads_opt_(tableNameLen) SQLWCHAR* tableName,
    SQLSMALLINT tableNameLen, _In_reads_opt_(tableTypeLen) SQLWCHAR* tableType,
    SQLSMALLINT tableTypeLen) {
#else
SQLRETURN SQL_API SQLTables(SQLHSTMT stmt, SQLWCHAR* catalogName,
                            SQLSMALLINT catalogNameLen, SQLWCHAR* schemaName,
                            SQLSMALLINT schemaNameLen, SQLWCHAR* tableName,
                            SQLSMALLINT tableNameLen, SQLWCHAR* tableType,
                            SQLSMALLINT tableTypeLen) {
#endif
  return documentdb::SQLTables(stmt, catalogName, catalogNameLen, schemaName,
                               schemaNameLen, tableName, tableNameLen,
                               tableType, tableTypeLen);
}

#if defined _WIN64 || defined _WIN32
SQLRETURN SQL_API SQLColumns(SQLHSTMT stmt,
                             _In_reads_opt_(catalogNameLen)
                                 SQLWCHAR* catalogName,
                             SQLSMALLINT catalogNameLen,
                             _In_reads_opt_(schemaNameLen) SQLWCHAR* schemaName,
                             SQLSMALLINT schemaNameLen,
                             _In_reads_opt_(tableNameLen) SQLWCHAR* tableName,
                             SQLSMALLINT tableNameLen,
                             _In_reads_opt_(columnNameLen) SQLWCHAR* columnName,
                             SQLSMALLINT columnNameLen) {
#else
SQLRETURN SQL_API SQLColumns(SQLHSTMT stmt, SQLWCHAR* catalogName,
                             SQLSMALLINT catalogNameLen, SQLWCHAR* schemaName,
                             SQLSMALLINT schemaNameLen, SQLWCHAR* tableName,
                             SQLSMALLINT tableNameLen, SQLWCHAR* columnName,
                             SQLSMALLINT columnNameLen) {
#endif
  return documentdb::SQLColumns(stmt, catalogName, catalogNameLen, schemaName,
                                schemaNameLen, tableName, tableNameLen,
                                columnName, columnNameLen);
}

SQLRETURN SQL_API SQLMoreResults(SQLHSTMT stmt) {
  return documentdb::SQLMoreResults(stmt);
}

SQLRETURN SQL_API SQLBindParameter(SQLHSTMT stmt, SQLUSMALLINT paramIdx,
                                   SQLSMALLINT ioType, SQLSMALLINT bufferType,
                                   SQLSMALLINT paramSqlType, SQLULEN columnSize,
                                   SQLSMALLINT decDigits, SQLPOINTER buffer,
                                   SQLLEN bufferLen, SQLLEN* resLen) {
  return documentdb::SQLBindParameter(stmt, paramIdx, ioType, bufferType,
                                  paramSqlType, columnSize, decDigits, buffer,
                                  bufferLen, resLen);
}

#if defined _WIN64 || defined _WIN32
SQLRETURN SQL_API SQLNativeSql(SQLHDBC conn,
                               _In_reads_(inQueryLen) SQLWCHAR* inQuery,
                               SQLINTEGER inQueryLen,
                               _Out_writes_opt_(outQueryBufferLen)
                                   SQLWCHAR* outQueryBuffer,
                               SQLINTEGER outQueryBufferLen,
                               SQLINTEGER* outQueryLen) {
#else
SQLRETURN SQL_API SQLNativeSql(SQLHDBC conn, SQLWCHAR* inQuery,
                               SQLINTEGER inQueryLen, SQLWCHAR* outQueryBuffer,
                               SQLINTEGER outQueryBufferLen,
                               SQLINTEGER* outQueryLen) {
#endif
  return documentdb::SQLNativeSql(conn, inQuery, inQueryLen, outQueryBuffer,
                                  outQueryBufferLen, outQueryLen);
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLColAttribute(SQLHSTMT stmt, SQLUSMALLINT columnNum,
                                  SQLUSMALLINT fieldId,
                                  _Out_writes_bytes_opt_(bufferLen)
                                      SQLPOINTER strAttr,
                                  SQLSMALLINT bufferLen,
    _Out_opt_ SQLSMALLINT* strAttrLen, _Out_opt_ SQLLEN* numericAttr)
#else
SQLRETURN SQL_API SQLColAttribute(SQLHSTMT stmt, SQLUSMALLINT columnNum,
                                  SQLUSMALLINT fieldId, SQLPOINTER strAttr,
                                  SQLSMALLINT bufferLen,
                                  SQLSMALLINT* strAttrLen,
                                  SQLPOINTER numericAttr)
#endif
{
  return documentdb::SQLColAttribute(stmt, columnNum, fieldId, strAttr,
                                     bufferLen, strAttrLen,
                                     (SQLLEN*)numericAttr);
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLDescribeCol(
    SQLHSTMT stmt, SQLUSMALLINT columnNum,
    _Out_writes_opt_(columnNameBufLen) SQLWCHAR* columnNameBuf,
    SQLSMALLINT columnNameBufLen, _Out_opt_ SQLSMALLINT* columnNameLen,
    _Out_opt_ SQLSMALLINT* dataType, _Out_opt_ SQLULEN* columnSize,
    _Out_opt_ SQLSMALLINT* decimalDigits, _Out_opt_ SQLSMALLINT* nullable) {
#else
SQLRETURN SQL_API SQLDescribeCol(SQLHSTMT stmt, SQLUSMALLINT columnNum,
                                 SQLWCHAR* columnNameBuf,
                                 SQLSMALLINT columnNameBufLen,
                                 SQLSMALLINT* columnNameLen,
                                 SQLSMALLINT* dataType, SQLULEN* columnSize,
                                 SQLSMALLINT* decimalDigits,
                                 SQLSMALLINT* nullable) {
#endif
  return documentdb::SQLDescribeCol(stmt, columnNum, columnNameBuf,
                                    columnNameBufLen, columnNameLen, dataType,
                                    columnSize, decimalDigits, nullable);
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLRowCount(_In_ SQLHSTMT stmt, _Out_ SQLLEN* rowCnt) {
#else
SQLRETURN SQL_API SQLRowCount(SQLHSTMT stmt, SQLLEN* rowCnt) {
#endif
  return documentdb::SQLRowCount(stmt, rowCnt);
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLForeignKeys(
    SQLHSTMT stmt,
    _In_reads_opt_(primaryCatalogNameLen) SQLWCHAR* primaryCatalogName,
    SQLSMALLINT primaryCatalogNameLen,
    _In_reads_opt_(primarySchemaNameLen) SQLWCHAR* primarySchemaName,
    SQLSMALLINT primarySchemaNameLen,
    _In_reads_opt_(primaryTableNameLen) SQLWCHAR* primaryTableName,
    SQLSMALLINT primaryTableNameLen,
    _In_reads_opt_(foreignCatalogNameLen) SQLWCHAR* foreignCatalogName,
    SQLSMALLINT foreignCatalogNameLen,
    _In_reads_opt_(foreignSchemaNameLen) SQLWCHAR* foreignSchemaName,
    SQLSMALLINT foreignSchemaNameLen,
    _In_reads_opt_(foreignTableNameLen) SQLWCHAR* foreignTableName,
    SQLSMALLINT foreignTableNameLen) {
#else
SQLRETURN SQL_API
SQLForeignKeys(SQLHSTMT stmt, SQLWCHAR* primaryCatalogName,
               SQLSMALLINT primaryCatalogNameLen, SQLWCHAR* primarySchemaName,
               SQLSMALLINT primarySchemaNameLen, SQLWCHAR* primaryTableName,
               SQLSMALLINT primaryTableNameLen, SQLWCHAR* foreignCatalogName,
               SQLSMALLINT foreignCatalogNameLen, SQLWCHAR* foreignSchemaName,
               SQLSMALLINT foreignSchemaNameLen, SQLWCHAR* foreignTableName,
               SQLSMALLINT foreignTableNameLen) {
#endif
  return documentdb::SQLForeignKeys(
      stmt, primaryCatalogName, primaryCatalogNameLen, primarySchemaName,
      primarySchemaNameLen, primaryTableName, primaryTableNameLen,
      foreignCatalogName, foreignCatalogNameLen, foreignSchemaName,
      foreignSchemaNameLen, foreignTableName, foreignTableNameLen);
}

SQLRETURN SQL_API SQLGetStmtAttr(SQLHSTMT stmt, SQLINTEGER attr,
                                 SQLPOINTER valueBuf, SQLINTEGER valueBufLen,
                                 SQLINTEGER* valueResLen) {
  return documentdb::SQLGetStmtAttr(stmt, attr, valueBuf, valueBufLen, valueResLen);
}

SQLRETURN SQL_API SQLSetStmtAttr(SQLHSTMT stmt, SQLINTEGER attr,
                                 SQLPOINTER value, SQLINTEGER valueLen) {
  return documentdb::SQLSetStmtAttr(stmt, attr, value, valueLen);
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLPrimaryKeys(
    SQLHSTMT stmt, _In_reads_opt_(catalogNameLen) SQLWCHAR* catalogName,
    SQLSMALLINT catalogNameLen,
    _In_reads_opt_(schemaNameLen) SQLWCHAR* schemaName,
    SQLSMALLINT schemaNameLen, _In_reads_opt_(tableNameLen) SQLWCHAR* tableName,
    SQLSMALLINT tableNameLen) {
#else
SQLRETURN SQL_API SQLPrimaryKeys(SQLHSTMT stmt, SQLWCHAR* catalogName,
                                 SQLSMALLINT catalogNameLen,
                                 SQLWCHAR* schemaName,
                                 SQLSMALLINT schemaNameLen, SQLWCHAR* tableName,
                                 SQLSMALLINT tableNameLen) {
#endif
  return documentdb::SQLPrimaryKeys(stmt, catalogName, catalogNameLen,
                                    schemaName, schemaNameLen, tableName,
                                    tableNameLen);
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLNumParams(SQLHSTMT stmt, _Out_opt_ SQLSMALLINT* paramCnt) {
#else
SQLRETURN SQL_API SQLNumParams(SQLHSTMT stmt, SQLSMALLINT* paramCnt) {
#endif
  return documentdb::SQLNumParams(stmt, paramCnt);
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLGetDiagField(SQLSMALLINT handleType, SQLHANDLE handle,
                                  SQLSMALLINT recNum, SQLSMALLINT diagId,
                                  _Out_writes_opt_(_Inexpressible_(bufferLen))
                                      SQLPOINTER buffer,
                                  SQLSMALLINT bufferLen,
                                  _Out_opt_ SQLSMALLINT* resLen) {
#else
SQLRETURN SQL_API SQLGetDiagField(SQLSMALLINT handleType, SQLHANDLE handle,
                                  SQLSMALLINT recNum, SQLSMALLINT diagId,
                                  SQLPOINTER buffer, SQLSMALLINT bufferLen,
                                  SQLSMALLINT* resLen) {
#endif
  return documentdb::SQLGetDiagField(handleType, handle, recNum, diagId, buffer,
                                     bufferLen, resLen);
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLGetDiagRec(SQLSMALLINT handleType, SQLHANDLE handle,
                                SQLSMALLINT recNum,
                                _Out_writes_opt_(6) SQLWCHAR* sqlState,
                                SQLINTEGER* nativeError,
                                _Out_writes_opt_(msgBufferLen)
                                    SQLWCHAR* msgBuffer,
                                SQLSMALLINT msgBufferLen, SQLSMALLINT* msgLen) {
#else
SQLRETURN SQL_API SQLGetDiagRec(SQLSMALLINT handleType, SQLHANDLE handle,
                                SQLSMALLINT recNum, SQLWCHAR* sqlState,
                                SQLINTEGER* nativeError, SQLWCHAR* msgBuffer,
                                SQLSMALLINT msgBufferLen, SQLSMALLINT* msgLen) {
#endif
  return documentdb::SQLGetDiagRec(handleType, handle, recNum, sqlState,
                                   nativeError, msgBuffer, msgBufferLen,
                                   msgLen);
}

SQLRETURN SQL_API SQLGetTypeInfo(SQLHSTMT stmt, SQLSMALLINT type) {
  return documentdb::SQLGetTypeInfo(stmt, type);
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLGetData(SQLHSTMT stmt, SQLUSMALLINT colNum,
                             SQLSMALLINT targetType,
                             _Out_writes_opt_(_Inexpressible_(bufferLength))
                                 SQLPOINTER targetValue,
                             SQLLEN bufferLength,
                             _Out_opt_ SQLLEN* strLengthOrIndicator) {
#else
SQLRETURN SQL_API SQLGetData(SQLHSTMT stmt, SQLUSMALLINT colNum,
                             SQLSMALLINT targetType, SQLPOINTER targetValue,
                             SQLLEN bufferLength,
                             SQLLEN* strLengthOrIndicator) {
#endif
  return documentdb::SQLGetData(stmt, colNum, targetType, targetValue,
                                bufferLength, strLengthOrIndicator);
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLSetEnvAttr(SQLHENV env, SQLINTEGER attr,
                                _In_reads_bytes_opt_(valueLen) SQLPOINTER value,
                                SQLINTEGER valueLen) {
#else
SQLRETURN SQL_API SQLSetEnvAttr(SQLHENV env, SQLINTEGER attr, SQLPOINTER value,
                                SQLINTEGER valueLen) {
#endif
  return documentdb::SQLSetEnvAttr(env, attr, value, valueLen);
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLGetEnvAttr(SQLHENV env, SQLINTEGER attr,
                                _Out_writes_(_Inexpressible_(valueBufLen))
                                    SQLPOINTER valueBuf,
                                SQLINTEGER valueBufLen,
                                _Out_opt_ SQLINTEGER* valueResLen) {
#else
SQLRETURN SQL_API SQLGetEnvAttr(SQLHENV env, SQLINTEGER attr,
                                SQLPOINTER valueBuf, SQLINTEGER valueBufLen,
                                SQLINTEGER* valueResLen) {
#endif
  return documentdb::SQLGetEnvAttr(env, attr, valueBuf, valueBufLen,
                                   valueResLen);
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLSpecialColumns(
    SQLHSTMT stmt, SQLUSMALLINT idType,
    _In_reads_opt_(catalogNameLen) SQLWCHAR* catalogName,
    SQLSMALLINT catalogNameLen,
    _In_reads_opt_(schemaNameLen) SQLWCHAR* schemaName,
    SQLSMALLINT schemaNameLen, _In_reads_opt_(tableNameLen) SQLWCHAR* tableName,
    SQLSMALLINT tableNameLen, SQLUSMALLINT scope, SQLUSMALLINT nullable) {
#else
SQLRETURN SQL_API SQLSpecialColumns(
    SQLHSTMT stmt, SQLUSMALLINT idType, SQLWCHAR* catalogName,
    SQLSMALLINT catalogNameLen, SQLWCHAR* schemaName, SQLSMALLINT schemaNameLen,
    SQLWCHAR* tableName, SQLSMALLINT tableNameLen, SQLUSMALLINT scope,
    SQLUSMALLINT nullable) {
#endif
  return documentdb::SQLSpecialColumns(
      stmt, idType, catalogName, catalogNameLen, schemaName, schemaNameLen,
      tableName, tableNameLen, scope, nullable);
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLParamData(SQLHSTMT stmt, _Out_opt_ SQLPOINTER* value) {
#else
SQLRETURN SQL_API SQLParamData(SQLHSTMT stmt, SQLPOINTER* value) {
#endif
  return documentdb::SQLParamData(stmt, value);
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLPutData(SQLHSTMT stmt,
                             _In_reads_(_Inexpressible_(strLengthOrIndicator))
                                 SQLPOINTER data,
                             SQLLEN strLengthOrIndicator) {
#else
SQLRETURN SQL_API SQLPutData(SQLHSTMT stmt, SQLPOINTER data,
                             SQLLEN strLengthOrIndicator) {
#endif
  return documentdb::SQLPutData(stmt, data, strLengthOrIndicator);
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLDescribeParam(SQLHSTMT stmt, SQLUSMALLINT paramNum,
                                   _Out_opt_ SQLSMALLINT* dataType,
                                   _Out_opt_ SQLULEN* paramSize,
                                   _Out_opt_ SQLSMALLINT* decimalDigits,
                                   _Out_opt_ SQLSMALLINT* nullable) {
#else
SQLRETURN SQL_API SQLDescribeParam(SQLHSTMT stmt, SQLUSMALLINT paramNum,
                                   SQLSMALLINT* dataType, SQLULEN* paramSize,
                                   SQLSMALLINT* decimalDigits,
                                   SQLSMALLINT* nullable) {
#endif
  return documentdb::SQLDescribeParam(stmt, paramNum, dataType, paramSize,
                                      decimalDigits, nullable);
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLError(SQLHENV env, SQLHDBC conn, SQLHSTMT stmt,
                           _Out_writes_(6) SQLWCHAR* state,
                           _Out_opt_ SQLINTEGER* error,
                           _Out_writes_opt_(msgBufLen) SQLWCHAR* msgBuf,
                           SQLSMALLINT msgBufLen,
                           _Out_opt_ SQLSMALLINT* msgResLen) {
#else
SQLRETURN SQL_API SQLError(SQLHENV env, SQLHDBC conn, SQLHSTMT stmt,
                           SQLWCHAR* state, SQLINTEGER* error, SQLWCHAR* msgBuf,
                           SQLSMALLINT msgBufLen, SQLSMALLINT* msgResLen) {
#endif
  return documentdb::SQLError(env, conn, stmt, state, error, msgBuf, msgBufLen,
                              msgResLen);
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLGetConnectAttr(
    SQLHDBC conn, SQLINTEGER attr,
    _Out_writes_opt_(_Inexpressible_(valueBufLen)) SQLPOINTER valueBuf,
    SQLINTEGER valueBufLen, _Out_opt_ SQLINTEGER* valueResLen) {
#else
SQLRETURN SQL_API SQLGetConnectAttr(SQLHDBC conn, SQLINTEGER attr,
                                    SQLPOINTER valueBuf, SQLINTEGER valueBufLen,
                                    SQLINTEGER* valueResLen) {
#endif
  return documentdb::SQLGetConnectAttr(conn, attr, valueBuf, valueBufLen,
                                       valueResLen);
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLSetConnectAttr(SQLHDBC conn, SQLINTEGER attr,
                                    _In_reads_bytes_opt_(valueLen)
                                        SQLPOINTER value,
                                    SQLINTEGER valueLen) {
#else
SQLRETURN SQL_API SQLSetConnectAttr(SQLHDBC conn, SQLINTEGER attr,
                                    SQLPOINTER value, SQLINTEGER valueLen) {
#endif
  return documentdb::SQLSetConnectAttr(conn, attr, value, valueLen);
}

//
// ==== Not Supported ====
//

SQLRETURN SQL_API SQLEndTran(SQLSMALLINT handleType, SQLHANDLE handle,
                             SQLSMALLINT completionType) {
                               
  DOCUMENTDB_UNUSED(handleType);
  DOCUMENTDB_UNUSED(handle);
  DOCUMENTDB_UNUSED(completionType);

  LOG_DEBUG_MSG("SQLEndTran called");
  return SQL_SUCCESS;
}
//
// ==== Not implemented ====
//

SQLRETURN SQL_API SQLCancel(SQLHSTMT stmt) {
  DOCUMENTDB_UNUSED(stmt);

  LOG_DEBUG_MSG("SQLCancel called");
  return SQL_SUCCESS;
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLColAttributes(SQLHSTMT stmt, SQLUSMALLINT colNum,
                                   SQLUSMALLINT fieldId,
                                   _Out_writes_bytes_opt_(strAttrBufLen)
                                       SQLPOINTER strAttrBuf,
                                   SQLSMALLINT strAttrBufLen,
                                   _Out_opt_ SQLSMALLINT* strAttrResLen,
                                   _Out_opt_ SQLLEN* numAttrBuf) {
#else
SQLRETURN SQL_API SQLColAttributes(SQLHSTMT stmt, SQLUSMALLINT colNum,
                                   SQLUSMALLINT fieldId, SQLPOINTER strAttrBuf,
                                   SQLSMALLINT strAttrBufLen,
                                   SQLSMALLINT* strAttrResLen,
                                   SQLLEN* numAttrBuf) {
#endif
  DOCUMENTDB_UNUSED(stmt);
  DOCUMENTDB_UNUSED(colNum);
  DOCUMENTDB_UNUSED(fieldId);
  DOCUMENTDB_UNUSED(strAttrBuf);
  DOCUMENTDB_UNUSED(strAttrBufLen);
  DOCUMENTDB_UNUSED(strAttrResLen);
  DOCUMENTDB_UNUSED(numAttrBuf);
  if (strAttrResLen)
    *strAttrResLen = 0;
  if (numAttrBuf)
    numAttrBuf = 0;

  LOG_DEBUG_MSG("SQLColAttributes called");
  return SQL_SUCCESS;
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLGetCursorName(SQLHSTMT stmt,
                                   _Out_writes_opt_(nameBufLen)
                                       SQLWCHAR* nameBuf,
                                   SQLSMALLINT nameBufLen,
                                   _Out_opt_ SQLSMALLINT* nameResLen) {
#else
SQLRETURN SQL_API SQLGetCursorName(SQLHSTMT stmt, SQLWCHAR* nameBuf,
                                   SQLSMALLINT nameBufLen,
                                   SQLSMALLINT* nameResLen) {
#endif
  DOCUMENTDB_UNUSED(stmt);
  DOCUMENTDB_UNUSED(nameBuf);
  DOCUMENTDB_UNUSED(nameBufLen);
  DOCUMENTDB_UNUSED(nameResLen);
  if (nameResLen)
    nameResLen = 0;

  LOG_DEBUG_MSG("SQLGetCursorName called");
  return SQL_SUCCESS;
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLSetCursorName(SQLHSTMT stmt,
                                   _In_reads_(nameLen) SQLWCHAR* name,
                                   SQLSMALLINT nameLen) {
#else
SQLRETURN SQL_API SQLSetCursorName(SQLHSTMT stmt, SQLWCHAR* name,
                                   SQLSMALLINT nameLen) {
#endif
  DOCUMENTDB_UNUSED(stmt);
  DOCUMENTDB_UNUSED(name);
  DOCUMENTDB_UNUSED(nameLen);

  LOG_DEBUG_MSG("SQLSetCursorName called");
  return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLGetConnectOption(SQLHDBC conn, SQLUSMALLINT option,
                                      SQLPOINTER value) {
  DOCUMENTDB_UNUSED(conn);
  DOCUMENTDB_UNUSED(option);
  DOCUMENTDB_UNUSED(value);

  LOG_DEBUG_MSG("SQLGetConnectOption called");
  return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLGetStmtOption(SQLHSTMT stmt, SQLUSMALLINT option,
                                   SQLPOINTER value) {
  DOCUMENTDB_UNUSED(stmt);
  DOCUMENTDB_UNUSED(option);
  DOCUMENTDB_UNUSED(value);

  LOG_DEBUG_MSG("SQLGetStmtOption called");
  return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLSetConnectOption(SQLHDBC conn, SQLUSMALLINT option,
                                      SQLULEN value) {
  DOCUMENTDB_UNUSED(conn);
  DOCUMENTDB_UNUSED(option);
  DOCUMENTDB_UNUSED(value);

  LOG_DEBUG_MSG("SQLSetConnectOption called");
  return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLSetStmtOption(SQLHSTMT stmt, SQLUSMALLINT option,
                                   SQLULEN value) {
  DOCUMENTDB_UNUSED(stmt);
  DOCUMENTDB_UNUSED(option);
  DOCUMENTDB_UNUSED(value);

  LOG_DEBUG_MSG("SQLSetStmtOption called");
  return SQL_SUCCESS;
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLStatistics(
    SQLHSTMT stmt, _In_reads_opt_(catalogNameLen) SQLWCHAR* catalogName,
    SQLSMALLINT catalogNameLen,
    _In_reads_opt_(schemaNameLen) SQLWCHAR* schemaName,
    SQLSMALLINT schemaNameLen, _In_reads_opt_(tableNameLen) SQLWCHAR* tableName,
    SQLSMALLINT tableNameLen, SQLUSMALLINT unique, SQLUSMALLINT reserved) {
#else
SQLRETURN SQL_API SQLStatistics(SQLHSTMT stmt, SQLWCHAR* catalogName,
                                SQLSMALLINT catalogNameLen,
                                SQLWCHAR* schemaName, SQLSMALLINT schemaNameLen,
                                SQLWCHAR* tableName, SQLSMALLINT tableNameLen,
                                SQLUSMALLINT unique, SQLUSMALLINT reserved) {
#endif
  DOCUMENTDB_UNUSED(stmt);
  DOCUMENTDB_UNUSED(catalogName);
  DOCUMENTDB_UNUSED(catalogNameLen);
  DOCUMENTDB_UNUSED(schemaName);
  DOCUMENTDB_UNUSED(schemaNameLen);
  DOCUMENTDB_UNUSED(tableName);
  DOCUMENTDB_UNUSED(tableNameLen);
  DOCUMENTDB_UNUSED(unique);
  DOCUMENTDB_UNUSED(reserved);

  LOG_DEBUG_MSG("SQLStatistics called");
  return SQL_SUCCESS;
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLBrowseConnect(
    SQLHDBC conn, _In_reads_(inConnectionStrLen) SQLWCHAR* inConnectionStr,
    SQLSMALLINT inConnectionStrLen,
    _Out_writes_opt_(outConnectionStrBufLen) SQLWCHAR* outConnectionStrBuf,
    SQLSMALLINT outConnectionStrBufLen,
    _Out_opt_ SQLSMALLINT* outConnectionStrResLen) {
#else
SQLRETURN SQL_API SQLBrowseConnect(SQLHDBC conn, SQLWCHAR* inConnectionStr,
                                   SQLSMALLINT inConnectionStrLen,
                                   SQLWCHAR* outConnectionStrBuf,
                                   SQLSMALLINT outConnectionStrBufLen,
                                   SQLSMALLINT* outConnectionStrResLen) {
#endif
  DOCUMENTDB_UNUSED(conn);
  DOCUMENTDB_UNUSED(inConnectionStr);
  DOCUMENTDB_UNUSED(inConnectionStrLen);
  DOCUMENTDB_UNUSED(outConnectionStrBuf);
  DOCUMENTDB_UNUSED(outConnectionStrBufLen);
  DOCUMENTDB_UNUSED(outConnectionStrResLen);
  if (outConnectionStrResLen)
    outConnectionStrResLen = 0;

  LOG_DEBUG_MSG("SQLBrowseConnect called");
  return SQL_SUCCESS;
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLProcedureColumns(
    SQLHSTMT stmt, _In_reads_opt_(catalogNameLen) SQLWCHAR* catalogName,
    SQLSMALLINT catalogNameLen,
    _In_reads_opt_(schemaNameLen) SQLWCHAR* schemaName,
    SQLSMALLINT schemaNameLen, _In_reads_opt_(procNameLen) SQLWCHAR* procName,
    SQLSMALLINT procNameLen, _In_reads_opt_(columnNameLen) SQLWCHAR* columnName,
    SQLSMALLINT columnNameLen) {
#else
SQLRETURN SQL_API SQLProcedureColumns(
    SQLHSTMT stmt, SQLWCHAR* catalogName, SQLSMALLINT catalogNameLen,
    SQLWCHAR* schemaName, SQLSMALLINT schemaNameLen, SQLWCHAR* procName,
    SQLSMALLINT procNameLen, SQLWCHAR* columnName, SQLSMALLINT columnNameLen) {
#endif
  DOCUMENTDB_UNUSED(stmt);
  DOCUMENTDB_UNUSED(catalogName);
  DOCUMENTDB_UNUSED(catalogNameLen);
  DOCUMENTDB_UNUSED(schemaName);
  DOCUMENTDB_UNUSED(schemaNameLen);
  DOCUMENTDB_UNUSED(procName);
  DOCUMENTDB_UNUSED(procNameLen);
  DOCUMENTDB_UNUSED(columnName);
  DOCUMENTDB_UNUSED(columnNameLen);

  LOG_DEBUG_MSG("SQLProcedureColumns called");
  return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLSetPos(SQLHSTMT stmt, SQLSETPOSIROW rowNum,
                            SQLUSMALLINT operation, SQLUSMALLINT lockType) {
  DOCUMENTDB_UNUSED(stmt);
  DOCUMENTDB_UNUSED(rowNum);
  DOCUMENTDB_UNUSED(operation);
  DOCUMENTDB_UNUSED(lockType);

  LOG_DEBUG_MSG("SQLSetPos called");
  return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLSetScrollOptions(SQLHSTMT stmt, SQLUSMALLINT concurrency,
                                      SQLLEN crowKeyset,
                                      SQLUSMALLINT crowRowset) {
  DOCUMENTDB_UNUSED(stmt);
  DOCUMENTDB_UNUSED(concurrency);
  DOCUMENTDB_UNUSED(crowKeyset);
  DOCUMENTDB_UNUSED(crowRowset);

  LOG_DEBUG_MSG("SQLSetScrollOptions called");
  return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLBulkOperations(SQLHSTMT stmt, SQLUSMALLINT operation) {
  DOCUMENTDB_UNUSED(stmt);
  DOCUMENTDB_UNUSED(operation);

  LOG_DEBUG_MSG("SQLBulkOperations called");
  return SQL_SUCCESS;
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLTablePrivileges(
    SQLHSTMT stmt, _In_reads_opt_(catalogNameLen) SQLWCHAR* catalogName,
    SQLSMALLINT catalogNameLen,
    _In_reads_opt_(schemaNameLen) SQLWCHAR* schemaName,
    SQLSMALLINT schemaNameLen, _In_reads_opt_(tableNameLen) SQLWCHAR* tableName,
    SQLSMALLINT tableNameLen) {
#else
SQLRETURN SQL_API SQLTablePrivileges(SQLHSTMT stmt, SQLWCHAR* catalogName,
                                     SQLSMALLINT catalogNameLen,
                                     SQLWCHAR* schemaName,
                                     SQLSMALLINT schemaNameLen,
                                     SQLWCHAR* tableName,
                                     SQLSMALLINT tableNameLen) {
#endif
  DOCUMENTDB_UNUSED(stmt);
  DOCUMENTDB_UNUSED(catalogName);
  DOCUMENTDB_UNUSED(catalogNameLen);
  DOCUMENTDB_UNUSED(schemaName);
  DOCUMENTDB_UNUSED(schemaNameLen);
  DOCUMENTDB_UNUSED(tableName);
  DOCUMENTDB_UNUSED(tableNameLen);

  LOG_DEBUG_MSG("SQLTablePrivileges called");
  return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLCopyDesc(SQLHDESC src, SQLHDESC dst) {
  DOCUMENTDB_UNUSED(src);
  DOCUMENTDB_UNUSED(dst);

  LOG_DEBUG_MSG("SQLCopyDesc called");
  return SQL_SUCCESS;
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLGetDescField(SQLHDESC descr, SQLSMALLINT recNum,
                                  SQLSMALLINT fieldId,
                                  _Out_writes_opt_(_Inexpressible_(bufferLen))
                                      SQLPOINTER buffer,
                                  SQLINTEGER bufferLen,
                                  _Out_opt_ SQLINTEGER* resLen) {
#else
SQLRETURN SQL_API SQLGetDescField(SQLHDESC descr, SQLSMALLINT recNum,
                                  SQLSMALLINT fieldId, SQLPOINTER buffer,
                                  SQLINTEGER bufferLen, SQLINTEGER* resLen) {
#endif
  DOCUMENTDB_UNUSED(descr);
  DOCUMENTDB_UNUSED(recNum);
  DOCUMENTDB_UNUSED(fieldId);
  DOCUMENTDB_UNUSED(buffer);
  DOCUMENTDB_UNUSED(bufferLen);
  DOCUMENTDB_UNUSED(resLen);
  if (buffer) {
    if (bufferLen > 0) {
      *(reinterpret_cast< SQLWCHAR* >(buffer)) = 0;
    } else {
      *(reinterpret_cast< SQLULEN* >(buffer)) = 0;
    }
  }
  if (resLen)
    *resLen = 0;

  LOG_DEBUG_MSG("SQLGetDescField called");
  return SQL_SUCCESS;
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API
SQLGetDescRec(SQLHDESC DescriptorHandle, SQLSMALLINT RecNumber,
              _Out_writes_opt_(nameBufferLen) SQLWCHAR* nameBuffer,
              SQLSMALLINT nameBufferLen, _Out_opt_ SQLSMALLINT* strLen,
              _Out_opt_ SQLSMALLINT* type, _Out_opt_ SQLSMALLINT* subType,
              _Out_opt_ SQLLEN* len, _Out_opt_ SQLSMALLINT* precision,
              _Out_opt_ SQLSMALLINT* scale, _Out_opt_ SQLSMALLINT* nullable) {
#else
SQLRETURN SQL_API SQLGetDescRec(SQLHDESC DescriptorHandle,
                                SQLSMALLINT RecNumber, SQLWCHAR* nameBuffer,
                                SQLSMALLINT nameBufferLen, SQLSMALLINT* strLen,
                                SQLSMALLINT* type, SQLSMALLINT* subType,
                                SQLLEN* len, SQLSMALLINT* precision,
                                SQLSMALLINT* scale, SQLSMALLINT* nullable) {
#endif
  DOCUMENTDB_UNUSED(DescriptorHandle);
  DOCUMENTDB_UNUSED(RecNumber);
  DOCUMENTDB_UNUSED(nameBuffer);
  DOCUMENTDB_UNUSED(nameBufferLen);
  DOCUMENTDB_UNUSED(strLen);
  DOCUMENTDB_UNUSED(type);
  DOCUMENTDB_UNUSED(subType);
  DOCUMENTDB_UNUSED(len);
  DOCUMENTDB_UNUSED(precision);
  DOCUMENTDB_UNUSED(scale);
  DOCUMENTDB_UNUSED(nullable);
  if (strLen)
    *strLen = 0;
  if (type)
    *type = 0;
  if (subType)
    *subType = 0;
  if (len)
    *len = 0;
  if (precision)
    *precision = 0;
  if (scale)
    *scale = 0;
  if (nullable)
    *nullable = 0;

  LOG_DEBUG_MSG("SQLGetDescRec called");
  return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLSetDescField(SQLHDESC descr, SQLSMALLINT recNum,
                                  SQLSMALLINT fieldId, SQLPOINTER buffer,
                                  SQLINTEGER bufferLen) {
  DOCUMENTDB_UNUSED(descr);
  DOCUMENTDB_UNUSED(recNum);
  DOCUMENTDB_UNUSED(fieldId);
  DOCUMENTDB_UNUSED(buffer);
  DOCUMENTDB_UNUSED(bufferLen);

  LOG_DEBUG_MSG("SQLSetDescField called");
  return SQL_SUCCESS;
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLSetDescRec(
    SQLHDESC descr, SQLSMALLINT recNum, SQLSMALLINT type, SQLSMALLINT subType,
    SQLLEN len, SQLSMALLINT precision, SQLSMALLINT scale,
    _Inout_updates_bytes_opt_(len) SQLPOINTER buffer,
    _Inout_opt_ SQLLEN* resLen, _Inout_opt_ SQLLEN* id) {
#else
SQLRETURN SQL_API SQLSetDescRec(SQLHDESC descr, SQLSMALLINT recNum,
                                SQLSMALLINT type, SQLSMALLINT subType,
                                SQLLEN len, SQLSMALLINT precision,
                                SQLSMALLINT scale, SQLPOINTER buffer,
                                SQLLEN* resLen, SQLLEN* id) {
#endif
  DOCUMENTDB_UNUSED(descr);
  DOCUMENTDB_UNUSED(recNum);
  DOCUMENTDB_UNUSED(type);
  DOCUMENTDB_UNUSED(subType);
  DOCUMENTDB_UNUSED(len);
  DOCUMENTDB_UNUSED(precision);
  DOCUMENTDB_UNUSED(scale);
  DOCUMENTDB_UNUSED(buffer);
  DOCUMENTDB_UNUSED(resLen);
  DOCUMENTDB_UNUSED(id);

  LOG_DEBUG_MSG("SQLSetDescRec called");
  return SQL_SUCCESS;
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLColumnPrivileges(
    SQLHSTMT stmt, _In_reads_opt_(catalogNameLen) SQLWCHAR* catalogName,
    SQLSMALLINT catalogNameLen,
    _In_reads_opt_(schemaNameLen) SQLWCHAR* schemaName,
    SQLSMALLINT schemaNameLen, _In_reads_opt_(tableNameLen) SQLWCHAR* tableName,
    SQLSMALLINT tableNameLen,
    _In_reads_opt_(columnNameLen) SQLWCHAR* columnName,
    SQLSMALLINT columnNameLen) {
#else
SQLRETURN SQL_API SQLColumnPrivileges(
    SQLHSTMT stmt, SQLWCHAR* catalogName, SQLSMALLINT catalogNameLen,
    SQLWCHAR* schemaName, SQLSMALLINT schemaNameLen, SQLWCHAR* tableName,
    SQLSMALLINT tableNameLen, SQLWCHAR* columnName, SQLSMALLINT columnNameLen) {
#endif
  DOCUMENTDB_UNUSED(stmt);
  DOCUMENTDB_UNUSED(catalogName);
  DOCUMENTDB_UNUSED(catalogNameLen);
  DOCUMENTDB_UNUSED(schemaName);
  DOCUMENTDB_UNUSED(schemaNameLen);
  DOCUMENTDB_UNUSED(tableName);
  DOCUMENTDB_UNUSED(tableNameLen);
  DOCUMENTDB_UNUSED(columnName);
  DOCUMENTDB_UNUSED(columnNameLen);

  LOG_DEBUG_MSG("SQLColumnPrivileges called");
  return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLParamOptions(SQLHSTMT stmt, SQLULEN paramSetSize,
                                  SQLULEN* paramsProcessed) {
  DOCUMENTDB_UNUSED(stmt);
  DOCUMENTDB_UNUSED(paramSetSize);
  DOCUMENTDB_UNUSED(paramsProcessed);

  LOG_DEBUG_MSG("SQLParamOptions called");
  return SQL_SUCCESS;
}

#if defined _WIN64 || !defined _WIN32
SQLRETURN SQL_API SQLProcedures(
    SQLHSTMT stmt, _In_reads_opt_(catalogNameLen) SQLWCHAR* catalogName,
    SQLSMALLINT catalogNameLen,
    _In_reads_opt_(schemaNameLen) SQLWCHAR* schemaName,
    SQLSMALLINT schemaNameLen, _In_reads_opt_(tableNameLen) SQLWCHAR* tableName,
    SQLSMALLINT tableNameLen) {
#else
SQLRETURN SQL_API SQLProcedures(SQLHSTMT stmt, SQLWCHAR* catalogName,
                                SQLSMALLINT catalogNameLen,
                                SQLWCHAR* schemaName, SQLSMALLINT schemaNameLen,
                                SQLWCHAR* tableName, SQLSMALLINT tableNameLen) {
#endif
  DOCUMENTDB_UNUSED(stmt);
  DOCUMENTDB_UNUSED(catalogName);
  DOCUMENTDB_UNUSED(catalogNameLen);
  DOCUMENTDB_UNUSED(schemaName);
  DOCUMENTDB_UNUSED(schemaNameLen);
  DOCUMENTDB_UNUSED(tableName);
  DOCUMENTDB_UNUSED(tableNameLen);

  LOG_DEBUG_MSG("SQLProcedures called");
  return SQL_SUCCESS;
}
