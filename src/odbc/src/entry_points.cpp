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

// Mac/Linux empty definitions.
#ifndef _Out_
#define _Out_
#endif
#ifndef _Out_opt_
#define _Out_opt_
#endif
#ifndef _Out_writes_opt_
#defin _Out_writes_opt_(size)
#endif
#ifndef _Out_writes_bytes_opt_
#define _Out_writes_bytes_opt_(size)
#endif
#ifndef _In_
#define _In_
#endif
#ifndef _In_reads_
#defin _In_reads_(size)
#endif
#ifndef _In_reads_opt_
#define _In_reads_opt_(size)
#endif
#ifndef _In_reads_bytes_opt_
#define _In_reads_bytes_opt_(size)
#endif
#ifndef _Inout_opt_
#define _Inout_opt_
#endif
#ifndef _Inout_updates_opt_
#define _Inout_updates_opt_(size)
#endif
#ifndef _Inout_updates_bytes_opt_
#define _Inout_updates_bytes_opt_(len)
#endif

SQLRETURN SQL_API SQLGetInfo(SQLHDBC conn, SQLUSMALLINT infoType,
                             _Out_writes_bytes_opt_(infoValueMax)
                                 SQLPOINTER infoValue,
                             SQLSMALLINT infoValueMax,
                             _Out_opt_ SQLSMALLINT* length) {
  return documentdb::SQLGetInfo(conn, infoType, infoValue, infoValueMax,
                                length);
}

SQLRETURN SQL_API SQLAllocHandle(SQLSMALLINT type, SQLHANDLE parent,
                                 _Out_ SQLHANDLE* result) {
  return documentdb::SQLAllocHandle(type, parent, result);
}

SQLRETURN SQL_API SQLAllocEnv(_Out_ SQLHENV* env) {
  return documentdb::SQLAllocEnv(env);
}

SQLRETURN SQL_API SQLAllocConnect(SQLHENV env, _Out_ SQLHDBC* conn) {
  return documentdb::SQLAllocConnect(env, conn);
}

SQLRETURN SQL_API SQLAllocStmt(SQLHDBC conn, _Out_ SQLHSTMT* stmt) {
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

SQLRETURN SQL_API
SQLDriverConnect(SQLHDBC conn, SQLHWND windowHandle,
                 _In_reads_(inConnectionStringLen) SQLWCHAR* inConnectionString,
                 SQLSMALLINT inConnectionStringLen,
                 _Out_writes_opt_(outConnectionStringBufferLen)
                     SQLWCHAR* outConnectionString,
                 SQLSMALLINT outConnectionStringBufferLen,
                 _Out_opt_ SQLSMALLINT* outConnectionStringLen,
                 SQLUSMALLINT driverCompletion) {
  return documentdb::SQLDriverConnect(
      conn, windowHandle, inConnectionString, inConnectionStringLen,
      outConnectionString, outConnectionStringBufferLen, outConnectionStringLen,
      driverCompletion);
}

SQLRETURN SQL_API SQLConnect(SQLHDBC conn,
                             _In_reads_(serverNameLen) SQLWCHAR* serverName,
                             SQLSMALLINT serverNameLen,
                             _In_reads_(userNameLen) SQLWCHAR* userName,
                             SQLSMALLINT userNameLen,
                             _In_reads_(authLen) SQLWCHAR* auth,
                             SQLSMALLINT authLen) {
  return documentdb::SQLConnect(conn, serverName, serverNameLen, userName,
                                userNameLen, auth, authLen);
}

SQLRETURN SQL_API SQLDisconnect(SQLHDBC conn) {
  return documentdb::SQLDisconnect(conn);
}

SQLRETURN SQL_API SQLPrepare(SQLHSTMT stmt,
                             _In_reads_(queryLen) SQLWCHAR* query,
                             SQLINTEGER queryLen) {
  return documentdb::SQLPrepare(stmt, query, queryLen);
}

SQLRETURN SQL_API SQLExecute(SQLHSTMT stmt) {
  return documentdb::SQLExecute(stmt);
}

SQLRETURN SQL_API SQLExecDirect(SQLHSTMT stmt,
                                _In_reads_opt_(queryLen) SQLWCHAR* query,
                                SQLINTEGER queryLen) {
  return documentdb::SQLExecDirect(stmt, query, queryLen);
}

#ifdef WIN32
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

SQLRETURN SQL_API SQLExtendedFetch(SQLHSTMT stmt, SQLUSMALLINT orientation,
                                   SQLLEN offset, _Out_opt_ SQLULEN* rowCount,
                                   _Out_opt_ SQLUSMALLINT* rowStatusArray) {
  return documentdb::SQLExtendedFetch(stmt, orientation, offset, rowCount,
                                      rowStatusArray);
}

SQLRETURN SQL_API SQLNumResultCols(SQLHSTMT stmt,
                                   _Out_ SQLSMALLINT* columnNum) {
  return documentdb::SQLNumResultCols(stmt, columnNum);
}

SQLRETURN SQL_API SQLTables(
    SQLHSTMT stmt, _In_reads_opt_(catalogNameLen) SQLWCHAR* catalogName,
    SQLSMALLINT catalogNameLen,
    _In_reads_opt_(schemaNameLen) SQLWCHAR* schemaName,
    SQLSMALLINT schemaNameLen, _In_reads_opt_(tableNameLen) SQLWCHAR* tableName,
    SQLSMALLINT tableNameLen, _In_reads_opt_(tableTypeLen) SQLWCHAR* tableType,
    SQLSMALLINT tableTypeLen) {
  return documentdb::SQLTables(stmt, catalogName, catalogNameLen, schemaName,
                               schemaNameLen, tableName, tableNameLen,
                               tableType, tableTypeLen);
}

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

SQLRETURN SQL_API SQLNativeSql(SQLHDBC conn,
                               _In_reads_(inQueryLen) SQLWCHAR* inQuery,
                               SQLINTEGER inQueryLen,
                               _Out_writes_opt_(outQueryBufferLen)
                                   SQLWCHAR* outQueryBuffer,
                               SQLINTEGER outQueryBufferLen,
                               SQLINTEGER* outQueryLen) {
  return documentdb::SQLNativeSql(conn, inQuery, inQueryLen, outQueryBuffer,
                                  outQueryBufferLen, outQueryLen);
}

SQLRETURN SQL_API SQLColAttribute(
    SQLHSTMT stmt, SQLUSMALLINT columnNum, SQLUSMALLINT fieldId,
    _Out_writes_bytes_opt_(bufferLen) SQLPOINTER strAttr, SQLSMALLINT bufferLen,
    _Out_opt_ SQLSMALLINT* strAttrLen, _Out_opt_ SQLLEN* numericAttr)
{
  return documentdb::SQLColAttribute(stmt, columnNum, fieldId, strAttr,
                                     bufferLen, strAttrLen,
                                     (SQLLEN*)numericAttr);
}

SQLRETURN SQL_API SQLDescribeCol(
    SQLHSTMT stmt, SQLUSMALLINT columnNum,
    _Out_writes_opt_(columnNameBufLen) SQLWCHAR* columnNameBuf,
    SQLSMALLINT columnNameBufLen, _Out_opt_ SQLSMALLINT* columnNameLen,
    _Out_opt_ SQLSMALLINT* dataType, _Out_opt_ SQLULEN* columnSize,
    _Out_opt_ SQLSMALLINT* decimalDigits, _Out_opt_ SQLSMALLINT* nullable) {
  return documentdb::SQLDescribeCol(stmt, columnNum, columnNameBuf,
                                    columnNameBufLen, columnNameLen, dataType,
                                    columnSize, decimalDigits, nullable);
}

SQLRETURN SQL_API SQLRowCount(_In_ SQLHSTMT stmt, _Out_ SQLLEN* rowCnt) {
  return documentdb::SQLRowCount(stmt, rowCnt);
}

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

SQLRETURN SQL_API SQLPrimaryKeys(
    SQLHSTMT stmt, _In_reads_opt_(catalogNameLen) SQLWCHAR* catalogName,
    SQLSMALLINT catalogNameLen,
    _In_reads_opt_(schemaNameLen) SQLWCHAR* schemaName,
    SQLSMALLINT schemaNameLen, _In_reads_opt_(tableNameLen) SQLWCHAR* tableName,
    SQLSMALLINT tableNameLen) {
  return documentdb::SQLPrimaryKeys(stmt, catalogName, catalogNameLen,
                                    schemaName, schemaNameLen, tableName,
                                    tableNameLen);
}

SQLRETURN SQL_API SQLNumParams(SQLHSTMT stmt, _Out_opt_ SQLSMALLINT* paramCnt) {
  return documentdb::SQLNumParams(stmt, paramCnt);
}

#ifdef WIN32
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

#endif  // !WIN32
  return documentdb::SQLGetDiagField(handleType, handle, recNum, diagId, buffer,
                                     bufferLen, resLen);
}

SQLRETURN SQL_API SQLGetDiagRec(SQLSMALLINT handleType, SQLHANDLE handle,
                                SQLSMALLINT recNum,
                                _Out_writes_opt_(6) SQLWCHAR* sqlState,
                                SQLINTEGER* nativeError,
                                _Out_writes_opt_(msgBufferLen)
                                    SQLWCHAR* msgBuffer,
                                SQLSMALLINT msgBufferLen, SQLSMALLINT* msgLen) {
  return documentdb::SQLGetDiagRec(handleType, handle, recNum, sqlState,
                                   nativeError, msgBuffer, msgBufferLen,
                                   msgLen);
}

SQLRETURN SQL_API SQLGetTypeInfo(SQLHSTMT stmt, SQLSMALLINT type) {
  return documentdb::SQLGetTypeInfo(stmt, type);
}

#ifdef WIN32
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

SQLRETURN SQL_API SQLSetEnvAttr(SQLHENV env, SQLINTEGER attr,
                                _In_reads_bytes_opt_(valueLen) SQLPOINTER value,
                                SQLINTEGER valueLen) {
  return documentdb::SQLSetEnvAttr(env, attr, value, valueLen);
}

#ifdef WIN32
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

SQLRETURN SQL_API SQLSpecialColumns(
    SQLHSTMT stmt, SQLUSMALLINT idType,
    _In_reads_opt_(catalogNameLen) SQLWCHAR* catalogName,
    SQLSMALLINT catalogNameLen,
    _In_reads_opt_(schemaNameLen) SQLWCHAR* schemaName,
    SQLSMALLINT schemaNameLen, _In_reads_opt_(tableNameLen) SQLWCHAR* tableName,
    SQLSMALLINT tableNameLen, SQLUSMALLINT scope, SQLUSMALLINT nullable) {
  return documentdb::SQLSpecialColumns(
      stmt, idType, catalogName, catalogNameLen, schemaName, schemaNameLen,
      tableName, tableNameLen, scope, nullable);
}

SQLRETURN SQL_API SQLParamData(SQLHSTMT stmt, _Out_opt_ SQLPOINTER* value) {
  return documentdb::SQLParamData(stmt, value);
}

#ifdef WIN32
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

SQLRETURN SQL_API SQLDescribeParam(SQLHSTMT stmt, SQLUSMALLINT paramNum,
                                   _Out_opt_ SQLSMALLINT* dataType,
                                   _Out_opt_ SQLULEN* paramSize,
                                   _Out_opt_ SQLSMALLINT* decimalDigits,
                                   _Out_opt_ SQLSMALLINT* nullable) {
  return documentdb::SQLDescribeParam(stmt, paramNum, dataType, paramSize,
                                      decimalDigits, nullable);
}

SQLRETURN SQL_API SQLError(SQLHENV env, SQLHDBC conn, SQLHSTMT stmt,
                           _Out_writes_(6) SQLWCHAR* state,
                           _Out_opt_ SQLINTEGER* error,
                           _Out_writes_opt_(msgBufLen) SQLWCHAR* msgBuf,
                           SQLSMALLINT msgBufLen,
                           _Out_opt_ SQLSMALLINT* msgResLen) {
  return documentdb::SQLError(env, conn, stmt, state, error, msgBuf, msgBufLen,
                              msgResLen);
}

#ifdef WIN32
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

SQLRETURN SQL_API SQLSetConnectAttr(SQLHDBC conn, SQLINTEGER attr,
                                    _In_reads_bytes_opt_(valueLen)
                                        SQLPOINTER value,
                                    SQLINTEGER valueLen) {
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

SQLRETURN SQL_API SQLColAttributes(SQLHSTMT stmt, SQLUSMALLINT colNum,
                                   SQLUSMALLINT fieldId,
                                   _Out_writes_bytes_opt_(strAttrBufLen)
                                       SQLPOINTER strAttrBuf,
                                   SQLSMALLINT strAttrBufLen,
                                   _Out_opt_ SQLSMALLINT* strAttrResLen,
                                   _Out_opt_ SQLLEN* numAttrBuf) {
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

SQLRETURN SQL_API SQLGetCursorName(SQLHSTMT stmt,
                                   _Out_writes_opt_(nameBufLen)
                                       SQLWCHAR* nameBuf,
                                   SQLSMALLINT nameBufLen,
                                   _Out_opt_ SQLSMALLINT* nameResLen) {
  DOCUMENTDB_UNUSED(stmt);
  DOCUMENTDB_UNUSED(nameBuf);
  DOCUMENTDB_UNUSED(nameBufLen);
  DOCUMENTDB_UNUSED(nameResLen);
  if (nameResLen)
    nameResLen = 0;

  LOG_DEBUG_MSG("SQLGetCursorName called");
  return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLSetCursorName(SQLHSTMT stmt,
                                   _In_reads_(nameLen) SQLWCHAR* name,
                                   SQLSMALLINT nameLen) {
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

SQLRETURN SQL_API SQLStatistics(
    SQLHSTMT stmt, _In_reads_opt_(catalogNameLen) SQLWCHAR* catalogName,
    SQLSMALLINT catalogNameLen,
    _In_reads_opt_(schemaNameLen) SQLWCHAR* schemaName,
    SQLSMALLINT schemaNameLen, _In_reads_opt_(tableNameLen) SQLWCHAR* tableName,
    SQLSMALLINT tableNameLen, SQLUSMALLINT unique, SQLUSMALLINT reserved) {
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

SQLRETURN SQL_API SQLBrowseConnect(
    SQLHDBC conn, _In_reads_(inConnectionStrLen) SQLWCHAR* inConnectionStr,
    SQLSMALLINT inConnectionStrLen,
    _Out_writes_opt_(outConnectionStrBufLen) SQLWCHAR* outConnectionStrBuf,
    SQLSMALLINT outConnectionStrBufLen,
    _Out_opt_ SQLSMALLINT* outConnectionStrResLen) {
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

SQLRETURN SQL_API SQLProcedureColumns(
    SQLHSTMT stmt, _In_reads_opt_(catalogNameLen) SQLWCHAR* catalogName,
    SQLSMALLINT catalogNameLen,
    _In_reads_opt_(schemaNameLen) SQLWCHAR* schemaName,
    SQLSMALLINT schemaNameLen, _In_reads_opt_(procNameLen) SQLWCHAR* procName,
    SQLSMALLINT procNameLen, _In_reads_opt_(columnNameLen) SQLWCHAR* columnName,
    SQLSMALLINT columnNameLen) {
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

SQLRETURN SQL_API SQLTablePrivileges(
    SQLHSTMT stmt, _In_reads_opt_(catalogNameLen) SQLWCHAR* catalogName,
    SQLSMALLINT catalogNameLen,
    _In_reads_opt_(schemaNameLen) SQLWCHAR* schemaName,
    SQLSMALLINT schemaNameLen, _In_reads_opt_(tableNameLen) SQLWCHAR* tableName,
    SQLSMALLINT tableNameLen) {
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

#ifdef WIN32
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

SQLRETURN SQL_API
SQLGetDescRec(SQLHDESC DescriptorHandle, SQLSMALLINT RecNumber,
              _Out_writes_opt_(nameBufferLen) SQLWCHAR* nameBuffer,
              SQLSMALLINT nameBufferLen, _Out_opt_ SQLSMALLINT* strLen,
              _Out_opt_ SQLSMALLINT* type, _Out_opt_ SQLSMALLINT* subType,
              _Out_opt_ SQLLEN* len, _Out_opt_ SQLSMALLINT* precision,
              _Out_opt_ SQLSMALLINT* scale, _Out_opt_ SQLSMALLINT* nullable) {
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

SQLRETURN SQL_API SQLSetDescRec(
    SQLHDESC descr, SQLSMALLINT recNum, SQLSMALLINT type, SQLSMALLINT subType,
    SQLLEN len, SQLSMALLINT precision, SQLSMALLINT scale,
    _Inout_updates_bytes_opt_(len) SQLPOINTER buffer,
    _Inout_opt_ SQLLEN* resLen, _Inout_opt_ SQLLEN* id) {
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

SQLRETURN SQL_API SQLColumnPrivileges(
    SQLHSTMT stmt, _In_reads_opt_(catalogNameLen) SQLWCHAR* catalogName,
    SQLSMALLINT catalogNameLen,
    _In_reads_opt_(schemaNameLen) SQLWCHAR* schemaName,
    SQLSMALLINT schemaNameLen, _In_reads_opt_(tableNameLen) SQLWCHAR* tableName,
    SQLSMALLINT tableNameLen,
    _In_reads_opt_(columnNameLen) SQLWCHAR* columnName,
    SQLSMALLINT columnNameLen) {
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

SQLRETURN SQL_API SQLProcedures(
    SQLHSTMT stmt, _In_reads_opt_(catalogNameLen) SQLWCHAR* catalogName,
    SQLSMALLINT catalogNameLen,
    _In_reads_opt_(schemaNameLen) SQLWCHAR* schemaName,
    SQLSMALLINT schemaNameLen, _In_reads_opt_(tableNameLen) SQLWCHAR* tableName,
    SQLSMALLINT tableNameLen) {
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
