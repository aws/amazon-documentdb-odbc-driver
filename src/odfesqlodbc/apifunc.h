/*
 * Copyright <2021> Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 */

#ifndef _TS_API_FUNC_H__
#define _TS_API_FUNC_H__

#include <stdio.h>
#include <string.h>
#include "odbc.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*	Internal flags for catalog functions */
#define PODBC_NOT_SEARCH_PATTERN 1L
#define PODBC_SEARCH_PUBLIC_SCHEMA (1L << 1)
#define PODBC_SEARCH_BY_IDS (1L << 2)
#define PODBC_SHOW_OID_COLUMN (1L << 3)
#define PODBC_ROW_VERSIONING (1L << 4)
/*	Internal flags for API_AllocStmt functions */
#define PODBC_EXTERNAL_STATEMENT 1L /* visible to the driver manager */
#define PODBC_INHERIT_CONNECT_OPTIONS (1L << 1)
/*	Internal flags for API_Exec... functions */
/*	Flags for the error handling */
#define PODBC_ALLOW_PARTIAL_EXTRACT 1L
/* #define	PODBC_ERROR_CLEAR		(1L << 1) 	no longer used */

RETCODE SQL_API API_AllocConnect(HENV EnvironmentHandle,
                                   HDBC *ConnectionHandle);
RETCODE SQL_API API_AllocEnv(HENV *EnvironmentHandle);
RETCODE SQL_API API_AllocStmt(HDBC ConnectionHandle, HSTMT *StatementHandle,
                                UDWORD flag);
RETCODE SQL_API API_BindCol(HSTMT StatementHandle, SQLUSMALLINT ColumnNumber,
                              SQLSMALLINT TargetType, PTR TargetValue,
                              SQLLEN BufferLength, SQLLEN *StrLen_or_Ind);
RETCODE SQL_API API_Connect(HDBC ConnectionHandle, const SQLCHAR *ServerName,
                              SQLSMALLINT NameLength1, const SQLCHAR *UserName,
                              SQLSMALLINT NameLength2,
                              const SQLCHAR *Authentication,
                              SQLSMALLINT NameLength3);
RETCODE SQL_API API_BrowseConnect(HDBC hdbc, const SQLCHAR *szConnStrIn,
                                    SQLSMALLINT cbConnStrIn,
                                    SQLCHAR *szConnStrOut,
                                    SQLSMALLINT cbConnStrOutMax,
                                    SQLSMALLINT *pcbConnStrOut);
RETCODE SQL_API API_DescribeCol(
    HSTMT StatementHandle, SQLUSMALLINT ColumnNumber, SQLCHAR *ColumnName,
    SQLSMALLINT BufferLength, SQLSMALLINT *NameLength, SQLSMALLINT *DataType,
    SQLULEN *ColumnSize, SQLSMALLINT *DecimalDigits, SQLSMALLINT *Nullable);
RETCODE SQL_API API_Disconnect(HDBC ConnectionHandle);
/* Helper functions for Error handling */
RETCODE SQL_API API_EnvError(HENV EnvironmentHandle, SQLSMALLINT RecNumber,
                               SQLCHAR *Sqlstate, SQLINTEGER *NativeError,
                               SQLCHAR *MessageText, SQLSMALLINT BufferLength,
                               SQLSMALLINT *TextLength, UWORD flag);
RETCODE SQL_API API_ConnectError(HDBC ConnectionHandle, SQLSMALLINT RecNumber,
                                   SQLCHAR *Sqlstate, SQLINTEGER *NativeError,
                                   SQLCHAR *MessageText,
                                   SQLSMALLINT BufferLength,
                                   SQLSMALLINT *TextLength, UWORD flag);
RETCODE SQL_API API_StmtError(HSTMT StatementHandle, SQLSMALLINT RecNumber,
                                SQLCHAR *Sqlstate, SQLINTEGER *NativeError,
                                SQLCHAR *MessageText, SQLSMALLINT BufferLength,
                                SQLSMALLINT *TextLength, UWORD flag);
RETCODE SQL_API API_ExecDirect(HSTMT StatementHandle,
                                 const SQLCHAR *StatementText,
                                 SQLINTEGER TextLength);
RETCODE SQL_API API_Execute(HSTMT StatementHandle);
RETCODE SQL_API API_Fetch(HSTMT StatementHandle);
RETCODE SQL_API API_FreeConnect(HDBC ConnectionHandle);
RETCODE SQL_API API_FreeEnv(HENV EnvironmentHandle);
RETCODE SQL_API API_FreeStmt(HSTMT StatementHandle, SQLUSMALLINT Option);
RETCODE SQL_API API_GetConnectOption(HDBC ConnectionHandle,
                                       SQLUSMALLINT Option, PTR Value,
                                       SQLINTEGER *StringLength,
                                       SQLINTEGER BufferLength);
RETCODE SQL_API API_GetCursorName(HSTMT StatementHandle, SQLCHAR *CursorName,
                                    SQLSMALLINT BufferLength,
                                    SQLSMALLINT *NameLength);
RETCODE SQL_API API_GetData(HSTMT StatementHandle, SQLUSMALLINT ColumnNumber,
                              SQLSMALLINT TargetType, PTR TargetValue,
                              SQLLEN BufferLength, SQLLEN *StrLen_or_Ind);
RETCODE SQL_API API_GetFunctions(HDBC ConnectionHandle,
                                   SQLUSMALLINT FunctionId,
                                   SQLUSMALLINT *Supported);
RETCODE SQL_API API_GetFunctions30(HDBC ConnectionHandle,
                                     SQLUSMALLINT FunctionId,
                                     SQLUSMALLINT *Supported);
RETCODE SQL_API API_GetInfo(HDBC ConnectionHandle, SQLUSMALLINT InfoType,
                              PTR InfoValue, SQLSMALLINT BufferLength,
                              SQLSMALLINT *StringLength);
RETCODE SQL_API API_GetStmtOption(HSTMT StatementHandle, SQLUSMALLINT Option,
                                    PTR Value, SQLINTEGER *StringLength,
                                    SQLINTEGER BufferLength);
RETCODE SQL_API API_NumResultCols(HSTMT StatementHandle,
                                    SQLSMALLINT *ColumnCount);
RETCODE SQL_API API_RowCount(HSTMT StatementHandle, SQLLEN *RowCount);
RETCODE SQL_API API_SetConnectOption(HDBC ConnectionHandle,
                                       SQLUSMALLINT Option, SQLULEN Value);
RETCODE SQL_API API_SetCursorName(HSTMT StatementHandle,
                                    const SQLCHAR *CursorName,
                                    SQLSMALLINT NameLength);
RETCODE SQL_API API_SetStmtOption(HSTMT StatementHandle, SQLUSMALLINT Option,
                                    SQLULEN Value);
RETCODE SQL_API
API_SpecialColumns(HSTMT StatementHandle, SQLUSMALLINT IdentifierType,
                     const SQLCHAR *CatalogName, SQLSMALLINT NameLength1,
                     const SQLCHAR *SchemaName, SQLSMALLINT NameLength2,
                     const SQLCHAR *TableName, SQLSMALLINT NameLength3,
                     SQLUSMALLINT Scope, SQLUSMALLINT Nullable);
RETCODE SQL_API API_Statistics(
    HSTMT StatementHandle, const SQLCHAR *CatalogName, SQLSMALLINT NameLength1,
    const SQLCHAR *SchemaName, SQLSMALLINT NameLength2,
    const SQLCHAR *TableName, SQLSMALLINT NameLength3, SQLUSMALLINT Unique,
    SQLUSMALLINT Reserved);
RETCODE SQL_API API_ColAttributes(HSTMT hstmt, SQLUSMALLINT icol,
                                    SQLUSMALLINT fDescType, PTR rgbDesc,
                                    SQLSMALLINT cbDescMax, SQLSMALLINT *pcbDesc,
                                    SQLLEN *pfDesc);
RETCODE SQL_API API_Prepare(HSTMT hstmt, const SQLCHAR *szSqlStr,
                              SQLINTEGER cbSqlStr);
RETCODE SQL_API API_ColumnPrivileges(
    HSTMT hstmt, const SQLCHAR *szCatalogName, SQLSMALLINT cbCatalogName,
    const SQLCHAR *szSchemaName, SQLSMALLINT cbSchemaName,
    const SQLCHAR *szTableName, SQLSMALLINT cbTableName,
    const SQLCHAR *szColumnName, SQLSMALLINT cbColumnName, UWORD flag);
RETCODE SQL_API API_ExtendedFetch(HSTMT hstmt, SQLUSMALLINT fFetchType,
                                    SQLLEN irow, SQLULEN *pcrow,
                                    SQLUSMALLINT *rgfRowStatus,
                                    SQLLEN FetchOffset, SQLLEN rowsetSize);
RETCODE SQL_API API_ForeignKeys(
    HSTMT hstmt, const SQLCHAR *szPkCatalogName, SQLSMALLINT cbPkCatalogName,
    const SQLCHAR *szPkSchemaName, SQLSMALLINT cbPkSchemaName,
    const SQLCHAR *szPkTableName, SQLSMALLINT cbPkTableName,
    const SQLCHAR *szFkCatalogName, SQLSMALLINT cbFkCatalogName,
    const SQLCHAR *szFkSchemaName, SQLSMALLINT cbFkSchemaName,
    const SQLCHAR *szFkTableName, SQLSMALLINT cbFkTableName);
RETCODE SQL_API API_MoreResults(HSTMT hstmt);
RETCODE SQL_API API_NativeSql(HDBC hdbc, const SQLCHAR *szSqlStrIn,
                                SQLINTEGER cbSqlStrIn, SQLCHAR *szSqlStr,
                                SQLINTEGER cbSqlStrMax, SQLINTEGER *pcbSqlStr);
RETCODE SQL_API API_NumParams(HSTMT hstmt, SQLSMALLINT *pcpar);
RETCODE SQL_API API_PrimaryKeys(HSTMT hstmt, const SQLCHAR *szCatalogName,
                                  SQLSMALLINT cbCatalogName,
                                  const SQLCHAR *szSchemaName,
                                  SQLSMALLINT cbSchemaName,
                                  const SQLCHAR *szTableName,
                                  SQLSMALLINT cbTableName, OID reloid);
RETCODE SQL_API API_ProcedureColumns(
    HSTMT hstmt, const SQLCHAR *szCatalogName, SQLSMALLINT cbCatalogName,
    const SQLCHAR *szSchemaName, SQLSMALLINT cbSchemaName,
    const SQLCHAR *szProcName, SQLSMALLINT cbProcName,
    const SQLCHAR *szColumnName, SQLSMALLINT cbColumnName, UWORD flag);
RETCODE SQL_API API_Procedures(HSTMT hstmt, const SQLCHAR *szCatalogName,
                                 SQLSMALLINT cbCatalogName,
                                 const SQLCHAR *szSchemaName,
                                 SQLSMALLINT cbSchemaName,
                                 const SQLCHAR *szProcName,
                                 SQLSMALLINT cbProcName, UWORD flag);
RETCODE SQL_API API_TablePrivileges(HSTMT hstmt, const SQLCHAR *szCatalogName,
                                      SQLSMALLINT cbCatalogName,
                                      const SQLCHAR *szSchemaName,
                                      SQLSMALLINT cbSchemaName,
                                      const SQLCHAR *szTableName,
                                      SQLSMALLINT cbTableName, UWORD flag);
RETCODE SQL_API API_GetDiagRec(SQLSMALLINT HandleType, SQLHANDLE Handle,
                                 SQLSMALLINT RecNumber, SQLCHAR *Sqlstate,
                                 SQLINTEGER *NativeError, SQLCHAR *MessageText,
                                 SQLSMALLINT BufferLength,
                                 SQLSMALLINT *TextLength);
RETCODE SQL_API API_GetDiagField(SQLSMALLINT HandleType, SQLHANDLE Handle,
                                   SQLSMALLINT RecNumber,
                                   SQLSMALLINT DiagIdentifier, PTR DiagInfoPtr,
                                   SQLSMALLINT BufferLength,
                                   SQLSMALLINT *StringLengthPtr);
RETCODE SQL_API API_GetConnectAttr(HDBC ConnectionHandle,
                                     SQLINTEGER Attribute, PTR Value,
                                     SQLINTEGER BufferLength,
                                     SQLINTEGER *StringLength);
RETCODE SQL_API API_GetStmtAttr(HSTMT StatementHandle, SQLINTEGER Attribute,
                                  PTR Value, SQLINTEGER BufferLength,
                                  SQLINTEGER *StringLength);

/* Driver-specific connection attributes, for SQLSet/GetConnectAttr() */
enum {
    SQL_ATTR_ESOPT_DEBUG = 65536,
    SQL_ATTR_ESOPT_COMMLOG = 65537,
    SQL_ATTR_ESOPT_PARSE = 65538,
    SQL_ATTR_ESOPT_USE_DECLAREFETCH = 65539,
    SQL_ATTR_ESOPT_SERVER_SIDE_PREPARE = 65540,
    SQL_ATTR_ESOPT_FETCH = 65541,
    SQL_ATTR_ESOPT_UNKNOWNSIZES = 65542,
    SQL_ATTR_ESOPT_TEXTASLONGVARCHAR = 65543,
    SQL_ATTR_ESOPT_UNKNOWNSASLONGVARCHAR = 65544,
    SQL_ATTR_ESOPT_BOOLSASCHAR = 65545,
    SQL_ATTR_ESOPT_MAXVARCHARSIZE = 65546,
    SQL_ATTR_ESOPT_MAXLONGVARCHARSIZE = 65547,
    SQL_ATTR_ESOPT_WCSDEBUG = 65548,
    SQL_ATTR_ESOPT_MSJET = 65549
};
RETCODE SQL_API API_SetConnectAttr(HDBC ConnectionHandle,
                                     SQLINTEGER Attribute, PTR Value,
                                     SQLINTEGER StringLength);
RETCODE SQL_API API_SetStmtAttr(HSTMT StatementHandle, SQLINTEGER Attribute,
                                  PTR Value, SQLINTEGER StringLength);
RETCODE SQL_API API_AllocDesc(HDBC ConnectionHandle,
                                SQLHDESC *DescriptorHandle);
RETCODE SQL_API API_FreeDesc(SQLHDESC DescriptorHandle);
RETCODE SQL_API API_CopyDesc(SQLHDESC SourceDescHandle,
                               SQLHDESC TargetDescHandle);
RETCODE SQL_API API_SetDescField(SQLHDESC DescriptorHandle,
                                   SQLSMALLINT RecNumber,
                                   SQLSMALLINT FieldIdentifier, PTR Value,
                                   SQLINTEGER BufferLength);
RETCODE SQL_API API_GetDescField(SQLHDESC DescriptorHandle,
                                   SQLSMALLINT RecNumber,
                                   SQLSMALLINT FieldIdentifier, PTR Value,
                                   SQLINTEGER BufferLength,
                                   SQLINTEGER *StringLength);
RETCODE SQL_API API_DescError(SQLHDESC DescriptorHandle,
                                SQLSMALLINT RecNumber, SQLCHAR *Sqlstate,
                                SQLINTEGER *NativeError, SQLCHAR *MessageText,
                                SQLSMALLINT BufferLength,
                                SQLSMALLINT *TextLength, UWORD flag);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* define_TS_API_FUNC_H__ */
