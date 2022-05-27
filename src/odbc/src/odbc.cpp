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

#include "ignite/odbc.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "ignite/odbc/config/configuration.h"
#include "ignite/odbc/config/connection_string_parser.h"
#include "ignite/odbc/connection.h"
#include "ignite/odbc/dsn_config.h"
#include "ignite/odbc/environment.h"
#include "ignite/odbc/log.h"
#include "ignite/odbc/statement.h"
#include "ignite/odbc/system/odbc_constants.h"
#include "ignite/odbc/system/system_dsn.h"
#include "ignite/odbc/type_traits.h"
#include "ignite/odbc/utility.h"

/**
 * Handle window handle.
 * @param windowHandle Window handle.
 * @param config Configuration.
 * @return @c true on success and @c false otherwise.
 */
bool HandleParentWindow(SQLHWND windowHandle,
                        ignite::odbc::config::Configuration& config) {
#ifdef _WIN32
  if (windowHandle) {
    LOG_INFO_MSG("Parent window is passed. Creating configuration window.");
    return DisplayConnectionWindow(windowHandle, config);
  }
#else
  IGNITE_UNUSED(windowHandle);
  IGNITE_UNUSED(config);
#endif
  return true;
}

namespace ignite {
SQLRETURN SQLGetInfo(SQLHDBC conn, SQLUSMALLINT infoType, SQLPOINTER infoValue,
                     SQLSMALLINT infoValueMax, SQLSMALLINT* length) {
  using odbc::Connection;
  using odbc::config::ConnectionInfo;

  LOG_DEBUG_MSG("SQLGetInfo called: "
                << infoType << " ("
                << ConnectionInfo::InfoTypeToString(infoType) << "), "
                << std::hex << reinterpret_cast< size_t >(infoValue) << ", "
                << infoValueMax << ", " << std::hex
                << reinterpret_cast< size_t >(length));

  Connection* connection = reinterpret_cast< Connection* >(conn);

  if (!connection)
    return SQL_INVALID_HANDLE;

  connection->GetInfo(infoType, infoValue, infoValueMax, length);

  LOG_DEBUG_MSG("SQLGetInfo exiting");

  return connection->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLAllocHandle(SQLSMALLINT type, SQLHANDLE parent,
                         SQLHANDLE* result) {
  LOG_DEBUG_MSG("SQLAllocHandle called");
  switch (type) {
    case SQL_HANDLE_ENV:
      LOG_DEBUG_MSG("SQLAllocHandle exiting on case SQL_HANDLE_ENV");
      return SQLAllocEnv(result);

    case SQL_HANDLE_DBC:
      LOG_DEBUG_MSG("SQLAllocHandle exiting on case SQL_HANDLE_DBC");
      return SQLAllocConnect(parent, result);

    case SQL_HANDLE_STMT:
      LOG_DEBUG_MSG("SQLAllocHandle exiting on case SQL_HANDLE_STMT");
      return SQLAllocStmt(parent, result);

    case SQL_HANDLE_DESC: {
      using odbc::Connection;

      LOG_DEBUG_MSG("SQLAllocHandle on case SQL_HANDLE_DESC");

      Connection* connection = reinterpret_cast< Connection* >(parent);

      if (!connection) {
        LOG_ERROR_MSG("SQLAllocHandle exiting with SQL_INVALID_HANDLE");
        return SQL_INVALID_HANDLE;
      }

      if (result)
        *result = 0;

      connection->GetDiagnosticRecords().Reset();
      connection->AddStatusRecord(
          odbc::SqlState::SIM001_FUNCTION_NOT_SUPPORTED,
          "The HandleType argument was SQL_HANDLE_DESC, and "
          "the driver does not support allocating a descriptor handle");

      LOG_ERROR_MSG("SQLAllocHandle exiting with SQL_ERROR");
      return SQL_ERROR;
    }
    default:
      break;
  }

  *result = 0;

  LOG_ERROR_MSG("SQLAllocHandle exiting with SQL_ERROR");

  return SQL_ERROR;
}

SQLRETURN SQLAllocEnv(SQLHENV* env) {
  using odbc::Environment;

  LOG_DEBUG_MSG("SQLAllocEnv called");

  *env = reinterpret_cast< SQLHENV >(new Environment());

  LOG_DEBUG_MSG("SQLAllocEnv exiting");

  return SQL_SUCCESS;
}

SQLRETURN SQLAllocConnect(SQLHENV env, SQLHDBC* conn) {
  using odbc::Connection;
  using odbc::Environment;

  LOG_DEBUG_MSG("SQLAllocConnect called");

  *conn = SQL_NULL_HDBC;

  Environment* environment = reinterpret_cast< Environment* >(env);

  if (!environment) {
    LOG_ERROR_MSG("SQLAllocConnect exiting with SQL_INVALID_HANDLE");
    return SQL_INVALID_HANDLE;
  }

  Connection* connection = environment->CreateConnection();

  if (!connection) {
    LOG_ERROR_MSG("SQLAllocConnect exiting because connection object is null");
    return environment->GetDiagnosticRecords().GetReturnCode();
  }

  *conn = reinterpret_cast< SQLHDBC >(connection);

  LOG_DEBUG_MSG("SQLAllocConnect exiting");

  return SQL_SUCCESS;
}

SQLRETURN SQLAllocStmt(SQLHDBC conn, SQLHSTMT* stmt) {
  using odbc::Connection;
  using odbc::Statement;

  LOG_DEBUG_MSG("SQLAllocStmt called");

  *stmt = SQL_NULL_HDBC;

  auto connection = static_cast< Connection* >(conn);

  if (!connection) {
    LOG_ERROR_MSG(
        "SQLAllocStmt exiting with SQL_INVALID_HANDLE because connection "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  Statement* statement = connection->CreateStatement();

  *stmt = reinterpret_cast< SQLHSTMT >(statement);

  LOG_DEBUG_MSG("SQLAllocStmt exiting");

  return connection->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLFreeHandle(SQLSMALLINT type, SQLHANDLE handle) {
  LOG_DEBUG_MSG("SQLFreeHandle called");

  switch (type) {
    case SQL_HANDLE_ENV:
      LOG_DEBUG_MSG("SQLFreeHandle exiting on case SQL_HANDLE_ENV");
      return SQLFreeEnv(handle);

    case SQL_HANDLE_DBC:
      LOG_DEBUG_MSG("SQLFreeHandle exiting on case SQL_HANDLE_DBC");
      return SQLFreeConnect(handle);

    case SQL_HANDLE_STMT:
      LOG_DEBUG_MSG("SQLFreeHandle exiting on case SQL_HANDLE_STMT");
      return SQLFreeStmt(handle, SQL_DROP);

    case SQL_HANDLE_DESC:
      LOG_DEBUG_MSG("SQLFreeHandle is on case SQL_HANDLE_DESC");
    default:
      break;
  }

  LOG_ERROR_MSG("SQLFreeHandle exiting with SQL_ERROR");

  return SQL_ERROR;
}

SQLRETURN SQLFreeEnv(SQLHENV env) {
  using odbc::Environment;

  LOG_DEBUG_MSG("SQLFreeEnv called: " << env);

  Environment* environment = reinterpret_cast< Environment* >(env);

  if (!environment) {
    LOG_ERROR_MSG(
        "SQLFreeEnv exiting with SQL_INVALID_HANDLE because environment object "
        "is null");
    return SQL_INVALID_HANDLE;
  }

  delete environment;

  LOG_DEBUG_MSG("SQLFreeEnv exiting");

  return SQL_SUCCESS;
}

SQLRETURN SQLFreeConnect(SQLHDBC conn) {
  using odbc::Connection;

  LOG_DEBUG_MSG("SQLFreeConnect called");

  Connection* connection = reinterpret_cast< Connection* >(conn);

  if (!connection) {
    LOG_ERROR_MSG(
        "SQLFreeConnect exiting with SQL_INVALID_HANDLE because connection "
        "object "
        "is null");
    return SQL_INVALID_HANDLE;
  }

  connection->Deregister();

  delete connection;

  LOG_DEBUG_MSG("SQLFreeConnect exiting");

  return SQL_SUCCESS;
}

SQLRETURN SQLFreeStmt(SQLHSTMT stmt, SQLUSMALLINT option) {
  using odbc::Statement;

  LOG_DEBUG_MSG("SQLFreeStmt called [option=" << option << ']');

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLFreeStmt exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  if (option == SQL_DROP) {
    delete statement;
    LOG_DEBUG_MSG("SQLFreeConnect exiting because option is SQL_DROP");
    return SQL_SUCCESS;
  }

  statement->FreeResources(option);

  LOG_DEBUG_MSG("SQLFreeStmt exiting");

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLCloseCursor(SQLHSTMT stmt) {
  using odbc::Statement;

  LOG_DEBUG_MSG("SQLCloseCursor called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  statement->Close();

  LOG_DEBUG_MSG("SQLCloseCursor exiting");

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLDriverConnect(SQLHDBC conn, SQLHWND windowHandle,
                           SQLCHAR* inConnectionString,
                           SQLSMALLINT inConnectionStringLen,
                           SQLCHAR* outConnectionString,
                           SQLSMALLINT outConnectionStringBufferLen,
                           SQLSMALLINT* outConnectionStringLen,
                           SQLUSMALLINT driverCompletion) {
  IGNITE_UNUSED(driverCompletion);

  using odbc::Connection;
  using odbc::diagnostic::DiagnosticRecordStorage;
  using odbc::utility::CopyStringToBuffer;
  using odbc::utility::SqlStringToString;

  LOG_DEBUG_MSG("SQLDriverConnect called");

  // TODO enable logging connection string
  // https://bitquill.atlassian.net/browse/AD-702

  // if (inConnectionString)
  //   LOG_INFO_MSG("Connection String: [" << inConnectionString << "]");

  Connection* connection = reinterpret_cast< Connection* >(conn);

  if (!connection) {
    LOG_ERROR_MSG(
        "SQLFreeStmt exiting with SQL_INVALID_HANDLE because connection "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  std::string connectStr =
      SqlStringToString(inConnectionString, inConnectionStringLen);
  connection->Establish(connectStr, windowHandle);

  DiagnosticRecordStorage& diag = connection->GetDiagnosticRecords();
  if (!diag.IsSuccessful()) {
    LOG_INFO_MSG(
        "SQLFreeStmt exiting becase Diagnostic Record Storage shows operation "
        "is not successful");
    return diag.GetReturnCode();
  }

  size_t reslen = CopyStringToBuffer(
      connectStr, reinterpret_cast< char* >(outConnectionString),
      static_cast< size_t >(outConnectionStringBufferLen));

  if (outConnectionStringLen)
    *outConnectionStringLen = static_cast< SQLSMALLINT >(reslen);

  // if (outConnectionString)
  //   LOG_INFO_MSG(outConnectionString);

  LOG_DEBUG_MSG("SQLDriverConnect exiting");

  return diag.GetReturnCode();
}

SQLRETURN SQLConnect(SQLHDBC conn, SQLCHAR* serverName,
                     SQLSMALLINT serverNameLen, SQLCHAR* userName,
                     SQLSMALLINT userNameLen, SQLCHAR* auth,
                     SQLSMALLINT authLen) {
  IGNITE_UNUSED(userName);
  IGNITE_UNUSED(userNameLen);
  IGNITE_UNUSED(auth);
  IGNITE_UNUSED(authLen);

  using odbc::Connection;
  using odbc::config::Configuration;
  using odbc::utility::SqlStringToString;

  LOG_DEBUG_MSG("SQLConnect called\n");

  Connection* connection = reinterpret_cast< Connection* >(conn);

  if (!connection) {
    LOG_ERROR_MSG(
        "SQLConnect exiting with SQL_INVALID_HANDLE because connection "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  odbc::config::Configuration config;

  std::string dsn = SqlStringToString(serverName, serverNameLen);

  LOG_INFO_MSG("DSN: " << dsn);

  odbc::ReadDsnConfiguration(dsn.c_str(), config,
                             &connection->GetDiagnosticRecords());

  connection->Establish(config);

  LOG_DEBUG_MSG("SQLConnect exiting");

  return connection->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLDisconnect(SQLHDBC conn) {
  using odbc::Connection;

  LOG_DEBUG_MSG("SQLDisconnect called");

  Connection* connection = reinterpret_cast< Connection* >(conn);

  if (!connection) {
    LOG_ERROR_MSG(
        "SQLDisconnect exiting with SQL_INVALID_HANDLE because connection "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  connection->Release();

  LOG_DEBUG_MSG("SQLDisconnect exiting");

  return connection->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLPrepare(SQLHSTMT stmt, SQLCHAR* query, SQLINTEGER queryLen) {
  using odbc::Statement;
  using odbc::utility::SqlStringToString;

  LOG_DEBUG_MSG("SQLPrepare called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLPrepare exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  std::string sql = SqlStringToString(query, queryLen);

  LOG_INFO_MSG("SQL: " << sql);

  statement->PrepareSqlQuery(sql);

  LOG_DEBUG_MSG("SQLPrepare exiting");

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLExecute(SQLHSTMT stmt) {
  using odbc::Statement;

  LOG_DEBUG_MSG("SQLExecute called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLExecute exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  statement->ExecuteSqlQuery();

  LOG_DEBUG_MSG("SQLExecute exiting");

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLExecDirect(SQLHSTMT stmt, SQLCHAR* query, SQLINTEGER queryLen) {
  using odbc::Statement;
  using odbc::utility::SqlStringToString;

  LOG_DEBUG_MSG("SQLExecDirect called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLExecDirect exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  std::string sql = SqlStringToString(query, queryLen);

  LOG_INFO_MSG("SQL: " << sql);

  statement->ExecuteSqlQuery(sql);

  LOG_DEBUG_MSG("SQLExecDirect exiting");

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLBindCol(SQLHSTMT stmt, SQLUSMALLINT colNum, SQLSMALLINT targetType,
                     SQLPOINTER targetValue, SQLLEN bufferLength,
                     SQLLEN* strLengthOrIndicator) {
  using namespace odbc::type_traits;

  using odbc::Statement;
  using odbc::app::ApplicationDataBuffer;

  LOG_DEBUG_MSG("SQLBindCol called: index="
                << colNum << ", type=" << targetType
                << ", targetValue=" << reinterpret_cast< size_t >(targetValue)
                << ", bufferLength=" << bufferLength << ", lengthInd="
                << reinterpret_cast< size_t >(strLengthOrIndicator));

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLBindCol exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  statement->BindColumn(colNum, targetType, targetValue, bufferLength,
                        strLengthOrIndicator);

  LOG_DEBUG_MSG("SQLBindCol exiting");

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLFetch(SQLHSTMT stmt) {
  using odbc::Statement;

  LOG_DEBUG_MSG("SQLFetch called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLFetch exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  statement->FetchRow();

  LOG_DEBUG_MSG("SQLFetch exiting");

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLFetchScroll(SQLHSTMT stmt, SQLSMALLINT orientation,
                         SQLLEN offset) {
  using odbc::Statement;

  LOG_DEBUG_MSG("SQLFetchScroll called");
  LOG_INFO_MSG("Orientation: " << orientation << " Offset: " << offset);

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLFetchScroll exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  statement->FetchScroll(orientation, offset);

  LOG_DEBUG_MSG("SQLFetchScroll exiting");

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLExtendedFetch(SQLHSTMT stmt, SQLUSMALLINT orientation,
                           SQLLEN offset, SQLULEN* rowCount,
                           SQLUSMALLINT* rowStatusArray) {
  LOG_DEBUG_MSG("SQLExtendedFetch called");

  SQLRETURN res = SQLFetchScroll(stmt, orientation, offset);

  if (res == SQL_SUCCESS) {
    if (rowCount)
      *rowCount = 1;

    if (rowStatusArray)
      rowStatusArray[0] = SQL_ROW_SUCCESS;
  } else if (res == SQL_NO_DATA && rowCount)
    *rowCount = 0;

  LOG_DEBUG_MSG("SQLExtendedFetch exiting");

  return res;
}

SQLRETURN SQLNumResultCols(SQLHSTMT stmt, SQLSMALLINT* columnNum) {
  using odbc::Statement;
  using odbc::meta::ColumnMetaVector;

  LOG_DEBUG_MSG("SQLNumResultCols called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLNumResultCols exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  int32_t res = statement->GetColumnNumber();

  if (columnNum) {
    *columnNum = static_cast< SQLSMALLINT >(res);
    LOG_INFO_MSG("columnNum: " << *columnNum);
  }

  LOG_DEBUG_MSG("SQLNumResultCols exiting");

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLTables(SQLHSTMT stmt, SQLCHAR* catalogName,
                    SQLSMALLINT catalogNameLen, SQLCHAR* schemaName,
                    SQLSMALLINT schemaNameLen, SQLCHAR* tableName,
                    SQLSMALLINT tableNameLen, SQLCHAR* tableType,
                    SQLSMALLINT tableTypeLen) {
  using odbc::Statement;
  using odbc::utility::SqlStringToString;

  LOG_DEBUG_MSG("SQLTables called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLTables exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  std::string catalog = SqlStringToString(catalogName, catalogNameLen);
  std::string schema = SqlStringToString(schemaName, schemaNameLen);
  std::string table = SqlStringToString(tableName, tableNameLen);
  std::string tableTypeStr = SqlStringToString(tableType, tableTypeLen);

  LOG_INFO_MSG("catalog: " << catalog);
  LOG_INFO_MSG("schema: " << schema);
  LOG_INFO_MSG("table: " << table);
  LOG_INFO_MSG("tableType: " << tableTypeStr);

  statement->ExecuteGetTablesMetaQuery(catalog, schema, table, tableTypeStr);

  LOG_DEBUG_MSG("SQLTables exiting");

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLColumns(SQLHSTMT stmt, SQLCHAR* catalogName,
                     SQLSMALLINT catalogNameLen, SQLCHAR* schemaName,
                     SQLSMALLINT schemaNameLen, SQLCHAR* tableName,
                     SQLSMALLINT tableNameLen, SQLCHAR* columnName,
                     SQLSMALLINT columnNameLen) {
  using odbc::Statement;
  using odbc::utility::SqlStringToString;

  LOG_DEBUG_MSG("SQLColumns called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLColumns exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  std::string catalog = SqlStringToString(catalogName, catalogNameLen);
  std::string schema = SqlStringToString(schemaName, schemaNameLen);
  std::string table = SqlStringToString(tableName, tableNameLen);
  std::string column = SqlStringToString(columnName, columnNameLen);

  LOG_INFO_MSG("catalog: " << catalog);
  LOG_INFO_MSG("schema: " << schema);
  LOG_INFO_MSG("table: " << table);
  LOG_INFO_MSG("column: " << column);

  statement->ExecuteGetColumnsMetaQuery(catalog, schema, table, column);

  LOG_DEBUG_MSG("SQLColumns exiting");

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLMoreResults(SQLHSTMT stmt) {
  using odbc::Statement;

  LOG_DEBUG_MSG("SQLMoreResults called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLMoreResults exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  statement->MoreResults();

  LOG_DEBUG_MSG("SQLMoreResults exiting");

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLBindParameter(SQLHSTMT stmt, SQLUSMALLINT paramIdx,
                           SQLSMALLINT ioType, SQLSMALLINT bufferType,
                           SQLSMALLINT paramSqlType, SQLULEN columnSize,
                           SQLSMALLINT decDigits, SQLPOINTER buffer,
                           SQLLEN bufferLen, SQLLEN* resLen) {
  using odbc::Statement;

  LOG_DEBUG_MSG("SQLBindParameter called: " << paramIdx << ", " << bufferType
                                            << ", " << paramSqlType);

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLBindParameter exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  statement->BindParameter(paramIdx, ioType, bufferType, paramSqlType,
                           columnSize, decDigits, buffer, bufferLen, resLen);

  LOG_DEBUG_MSG("SQLBindParameter exiting");

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLNativeSql(SQLHDBC conn, SQLCHAR* inQuery, SQLINTEGER inQueryLen,
                       SQLCHAR* outQueryBuffer, SQLINTEGER outQueryBufferLen,
                       SQLINTEGER* outQueryLen) {
  IGNITE_UNUSED(conn);

  using namespace odbc::utility;

  LOG_DEBUG_MSG("SQLNativeSql called");

  std::string in = SqlStringToString(inQuery, inQueryLen);

  CopyStringToBuffer(in, reinterpret_cast< char* >(outQueryBuffer),
                     static_cast< size_t >(outQueryBufferLen));

  if (outQueryLen)
    *outQueryLen =
        std::min(outQueryBufferLen, static_cast< SQLINTEGER >(in.size()));

  LOG_DEBUG_MSG("SQLNativeSql exiting");

  return SQL_SUCCESS;
}

SQLRETURN SQLColAttribute(SQLHSTMT stmt, SQLUSMALLINT columnNum,
                          SQLUSMALLINT fieldId, SQLPOINTER strAttr,
                          SQLSMALLINT bufferLen, SQLSMALLINT* strAttrLen,
                          SQLLEN* numericAttr) {
  using odbc::Statement;
  using odbc::meta::ColumnMeta;
  using odbc::meta::ColumnMetaVector;

  LOG_DEBUG_MSG("SQLColAttribute called: "
                << fieldId << " (" << ColumnMeta::AttrIdToString(fieldId)
                << ")");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLColAttribute exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  // This is a special case
  if (fieldId == SQL_DESC_COUNT) {
    SQLSMALLINT val = 0;

    SQLRETURN res = SQLNumResultCols(stmt, &val);

    if (numericAttr && res == SQL_SUCCESS)
      *numericAttr = val;

    return res;
  }

  statement->GetColumnAttribute(columnNum, fieldId,
                                reinterpret_cast< char* >(strAttr), bufferLen,
                                strAttrLen, numericAttr);

  LOG_DEBUG_MSG("SQLColAttribute exiting");

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLDescribeCol(SQLHSTMT stmt, SQLUSMALLINT columnNum,
                         SQLCHAR* columnNameBuf, SQLSMALLINT columnNameBufLen,
                         SQLSMALLINT* columnNameLen, SQLSMALLINT* dataType,
                         SQLULEN* columnSize, SQLSMALLINT* decimalDigits,
                         SQLSMALLINT* nullable) {
  using odbc::SqlLen;
  using odbc::Statement;

  LOG_DEBUG_MSG("SQLDescribeCol called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLDescribeCol exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  statement->GetColumnAttribute(columnNum, SQL_DESC_NAME,
                                reinterpret_cast< char* >(columnNameBuf),
                                columnNameBufLen, columnNameLen, 0);

  SqlLen dataTypeRes;
  SqlLen columnSizeRes;
  SqlLen decimalDigitsRes;
  SqlLen nullableRes;

  statement->GetColumnAttribute(columnNum, SQL_DESC_TYPE, 0, 0, 0,
                                &dataTypeRes);
  statement->GetColumnAttribute(columnNum, SQL_DESC_PRECISION, 0, 0, 0,
                                &columnSizeRes);
  statement->GetColumnAttribute(columnNum, SQL_DESC_SCALE, 0, 0, 0,
                                &decimalDigitsRes);
  statement->GetColumnAttribute(columnNum, SQL_DESC_NULLABLE, 0, 0, 0,
                                &nullableRes);

  LOG_INFO_MSG("columnNum: " << columnNum);
  LOG_INFO_MSG("dataTypeRes: " << dataTypeRes);
  LOG_INFO_MSG("columnSizeRes: " << columnSizeRes);
  LOG_INFO_MSG("decimalDigitsRes: " << decimalDigitsRes);
  LOG_INFO_MSG("nullableRes: " << nullableRes);
  LOG_INFO_MSG(
      "columnNameBuf: " << (columnNameBuf
                                ? reinterpret_cast< const char* >(columnNameBuf)
                                : "<null>"));
  LOG_INFO_MSG("columnNameLen: " << (columnNameLen ? *columnNameLen : -1));

  if (dataType)
    *dataType = static_cast< SQLSMALLINT >(dataTypeRes);

  if (columnSize)
    *columnSize = static_cast< SQLULEN >(columnSizeRes);

  if (decimalDigits)
    *decimalDigits = static_cast< SQLSMALLINT >(decimalDigitsRes);

  if (nullable)
    *nullable = static_cast< SQLSMALLINT >(nullableRes);

  LOG_DEBUG_MSG("SQLDescribeCol exiting");

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLRowCount(SQLHSTMT stmt, SQLLEN* rowCnt) {
  using odbc::Statement;

  LOG_DEBUG_MSG("SQLRowCount called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLRowCount exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  int64_t res = statement->AffectedRows();

  LOG_INFO_MSG("Row count: " << res);

  if (rowCnt)
    *rowCnt = static_cast< SQLLEN >(res);

  LOG_DEBUG_MSG("SQLRowCount exiting");

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLForeignKeys(
    SQLHSTMT stmt, SQLCHAR* primaryCatalogName,
    SQLSMALLINT primaryCatalogNameLen, SQLCHAR* primarySchemaName,
    SQLSMALLINT primarySchemaNameLen, SQLCHAR* primaryTableName,
    SQLSMALLINT primaryTableNameLen, SQLCHAR* foreignCatalogName,
    SQLSMALLINT foreignCatalogNameLen, SQLCHAR* foreignSchemaName,
    SQLSMALLINT foreignSchemaNameLen, SQLCHAR* foreignTableName,
    SQLSMALLINT foreignTableNameLen) {
  using odbc::Statement;
  using odbc::utility::SqlStringToString;
  using odbc::utility::SqlStringToOptString;

  LOG_DEBUG_MSG("SQLForeignKeys called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLForeignKeys exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  std::string primaryCatalog =
      SqlStringToString(primaryCatalogName, primaryCatalogNameLen);
  std::string primarySchema =
      SqlStringToString(primarySchemaName, primarySchemaNameLen);
  std::string primaryTable =
      SqlStringToString(primaryTableName, primaryTableNameLen);
  const boost::optional< std::string > foreignCatalog =
      SqlStringToOptString(foreignCatalogName, foreignCatalogNameLen);
  const boost::optional< std::string > foreignSchema =
      SqlStringToOptString(foreignSchemaName, foreignSchemaNameLen);
  std::string foreignTable =
      SqlStringToString(foreignTableName, foreignTableNameLen);

  LOG_INFO_MSG("primaryCatalog: " << primaryCatalog);
  LOG_INFO_MSG("primarySchema: " << primarySchema);
  LOG_INFO_MSG("primaryTable: " << primaryTable);
  LOG_INFO_MSG("foreignCatalog: " << foreignCatalog.get_value_or(""));
  LOG_INFO_MSG("foreignSchema: " << foreignSchema.get_value_or(""));
  LOG_INFO_MSG("foreignTable: " << foreignTable);

  statement->ExecuteGetForeignKeysQuery(primaryCatalog, primarySchema,
                                        primaryTable, foreignCatalog,
                                        foreignSchema, foreignTable);

  LOG_DEBUG_MSG("SQLForeignKeys exiting");

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLGetStmtAttr(SQLHSTMT stmt, SQLINTEGER attr, SQLPOINTER valueBuf,
                         SQLINTEGER valueBufLen, SQLINTEGER* valueResLen) {
  using odbc::Statement;

  LOG_DEBUG_MSG("SQLGetStmtAttr called");

#ifdef _DEBUG
  using odbc::type_traits::StatementAttrIdToString;

  LOG_DEBUG_MSG("Attr: " << StatementAttrIdToString(attr) << " (" << attr
                         << ")");
#endif  //_DEBUG

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLGetStmtAttr exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  statement->GetAttribute(attr, valueBuf, valueBufLen, valueResLen);

  LOG_DEBUG_MSG("SQLGetStmtAttr exiting");

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLSetStmtAttr(SQLHSTMT stmt, SQLINTEGER attr, SQLPOINTER value,
                         SQLINTEGER valueLen) {
  using odbc::Statement;

  LOG_DEBUG_MSG("SQLSetStmtAttr called: " << attr);

#ifdef _DEBUG
  using odbc::type_traits::StatementAttrIdToString;

  LOG_DEBUG_MSG("Attr: " << StatementAttrIdToString(attr) << " (" << attr
                         << ")");
#endif  //_DEBUG

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLSetStmtAttr exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  statement->SetAttribute(attr, value, valueLen);

  LOG_DEBUG_MSG("SQLSetStmtAttr exiting");

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLPrimaryKeys(SQLHSTMT stmt, SQLCHAR* catalogName,
                         SQLSMALLINT catalogNameLen, SQLCHAR* schemaName,
                         SQLSMALLINT schemaNameLen, SQLCHAR* tableName,
                         SQLSMALLINT tableNameLen) {
  using odbc::Statement;
  using odbc::utility::SqlStringToOptString;

  LOG_DEBUG_MSG("SQLPrimaryKeys called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLPrimaryKeys exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  boost::optional< std::string > catalog =
      SqlStringToOptString(catalogName, catalogNameLen);
  boost::optional< std::string > schema =
      SqlStringToOptString(schemaName, schemaNameLen);
  boost::optional< std::string > table =
      SqlStringToOptString(tableName, tableNameLen);

  LOG_INFO_MSG("catalog: " << catalog.get_value_or(""));
  LOG_INFO_MSG("schema: " << schema.get_value_or(""));
  LOG_INFO_MSG("table: " << table.get_value_or(""));

  statement->ExecuteGetPrimaryKeysQuery(catalog, schema, table);

  LOG_DEBUG_MSG("SQLPrimaryKeys exiting");

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLNumParams(SQLHSTMT stmt, SQLSMALLINT* paramCnt) {
  using odbc::Statement;

  LOG_DEBUG_MSG("SQLNumParams called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLNumParams exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  if (paramCnt) {
    uint16_t paramNum = 0;
    statement->GetParametersNumber(paramNum);

    *paramCnt = static_cast< SQLSMALLINT >(paramNum);
  }

  LOG_DEBUG_MSG("SQLNumParams exiting");

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLGetDiagField(SQLSMALLINT handleType, SQLHANDLE handle,
                          SQLSMALLINT recNum, SQLSMALLINT diagId,
                          SQLPOINTER buffer, SQLSMALLINT bufferLen,
                          SQLSMALLINT* resLen) {
  using namespace odbc;
  using namespace odbc::diagnostic;
  using namespace odbc::type_traits;

  using odbc::app::ApplicationDataBuffer;

  LOG_DEBUG_MSG("SQLGetDiagField called: " << recNum);

  SqlLen outResLen;
  ApplicationDataBuffer outBuffer(OdbcNativeType::AI_DEFAULT, buffer, bufferLen,
                                  &outResLen);

  SqlResult::Type result;

  DiagnosticField::Type field = DiagnosticFieldToInternal(diagId);

  switch (handleType) {
    case SQL_HANDLE_ENV:
    case SQL_HANDLE_DBC:
    case SQL_HANDLE_STMT: {
      Diagnosable* diag = reinterpret_cast< Diagnosable* >(handle);

      result = diag->GetDiagnosticRecords().GetField(recNum, field, outBuffer);

      break;
    }

    default: {
      result = SqlResult::AI_NO_DATA;
      break;
    }
  }

  if (resLen && result == SqlResult::AI_SUCCESS)
    *resLen = static_cast< SQLSMALLINT >(outResLen);

  LOG_DEBUG_MSG("SQLGetDiagField exiting");

  return SqlResultToReturnCode(result);
}

SQLRETURN SQLGetDiagRec(SQLSMALLINT handleType, SQLHANDLE handle,
                        SQLSMALLINT recNum, SQLCHAR* sqlState,
                        SQLINTEGER* nativeError, SQLCHAR* msgBuffer,
                        SQLSMALLINT msgBufferLen, SQLSMALLINT* msgLen) {
  using namespace odbc::utility;
  using namespace odbc;
  using namespace odbc::diagnostic;
  using namespace odbc::type_traits;

  using odbc::app::ApplicationDataBuffer;

  LOG_DEBUG_MSG("SQLGetDiagRec called");

  const DiagnosticRecordStorage* records = 0;

  switch (handleType) {
    case SQL_HANDLE_ENV:
    case SQL_HANDLE_DBC:
    case SQL_HANDLE_STMT: {
      Diagnosable* diag = reinterpret_cast< Diagnosable* >(handle);

      if (!diag) {
        LOG_ERROR_MSG(
            "SQLGetDiagRec exiting with SQL_INVALID_HANDLE because diag "
            "object is null");
        return SQL_INVALID_HANDLE;
      }

      records = &diag->GetDiagnosticRecords();

      break;
    }

    default:
      LOG_ERROR_MSG(
          "SQLGetDiagRec exiting with SQL_INVALID_HANDLE on default case");
      return SQL_INVALID_HANDLE;
  }

  if (recNum < 1 || msgBufferLen < 0) {
    LOG_ERROR_MSG("SQLGetDiagRec exiting with SQL_ERROR. recNum: "
                  << recNum << ", msgBufferLen: " << msgBufferLen);
    return SQL_ERROR;
  }

  if (!records || recNum > records->GetStatusRecordsNumber()) {
    if (records) {
      LOG_ERROR_MSG("SQLGetDiagRec exiting with SQL_NO_DATA. recNum: "
                    << recNum << ", records: " << records
                    << ", records->GetStatusRecordsNumber(): "
                    << records->GetStatusRecordsNumber());
    } else {
      LOG_ERROR_MSG(
          "SQLGetDiagRec exiting with SQL_NO_DATA because records variable is "
          "null. recNum: "
          << recNum << ", records: " << records);
    }
    return SQL_NO_DATA;
  }

  const DiagnosticRecord& record = records->GetStatusRecord(recNum);

  if (sqlState)
    CopyStringToBuffer(record.GetSqlState(),
                       reinterpret_cast< char* >(sqlState), 6);

  if (nativeError)
    *nativeError = 0;

  const std::string& errMsg = record.GetMessageText();

  if (!msgBuffer
      || msgBufferLen < static_cast< SQLSMALLINT >(errMsg.size() + 1)) {
    if (!msgLen) {
      LOG_ERROR_MSG("SQLGetDiagRec exiting with SQL_ERROR. msgLen: " << msgLen);
      return SQL_ERROR;
    }

    CopyStringToBuffer(errMsg, reinterpret_cast< char* >(msgBuffer),
                       static_cast< size_t >(msgBufferLen));

    *msgLen = static_cast< SQLSMALLINT >(errMsg.size());

    LOG_DEBUG_MSG("SQLGetDiagRec exiting with SQL_SUCCESS_WITH_INFO");

    return SQL_SUCCESS_WITH_INFO;
  }

  CopyStringToBuffer(errMsg, reinterpret_cast< char* >(msgBuffer),
                     static_cast< size_t >(msgBufferLen));

  if (msgLen)
    *msgLen = static_cast< SQLSMALLINT >(errMsg.size());

  LOG_DEBUG_MSG("SQLGetDiagRec exiting with SQL_SUCCESS");

  return SQL_SUCCESS;
}

SQLRETURN SQLGetTypeInfo(SQLHSTMT stmt, SQLSMALLINT type) {
  using odbc::Statement;

  LOG_DEBUG_MSG("SQLGetTypeInfo called: [type=" << type << ']');

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLGetTypeInfo exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  statement->ExecuteGetTypeInfoQuery(static_cast< int16_t >(type));

  LOG_DEBUG_MSG("SQLGetTypeInfo exiting");

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLEndTran(SQLSMALLINT handleType, SQLHANDLE handle,
                     SQLSMALLINT completionType) {
  using namespace odbc;

  LOG_DEBUG_MSG("SQLEndTran called");

  SQLRETURN result;

  switch (handleType) {
    case SQL_HANDLE_ENV: {
      Environment* env = reinterpret_cast< Environment* >(handle);

      if (!env) {
        LOG_ERROR_MSG(
            "SQLEndTran exiting with SQL_INVALID_HANDLE because env "
            "object is null.");
        LOG_DEBUG_MSG("handletype is SQL_HANDLE_ENV");
        return SQL_INVALID_HANDLE;
      }

      if (completionType == SQL_COMMIT)
        env->TransactionCommit();
      else
        env->TransactionRollback();

      result = env->GetDiagnosticRecords().GetReturnCode();

      break;
    }

    case SQL_HANDLE_DBC: {
      Connection* conn = reinterpret_cast< Connection* >(handle);

      if (!conn) {
        LOG_ERROR_MSG(
            "SQLEndTran exiting with SQL_INVALID_HANDLE because conn "
            "object is null");
        LOG_DEBUG_MSG("handletype is SQL_HANDLE_DBC");
        return SQL_INVALID_HANDLE;
      }

      if (completionType == SQL_COMMIT)
        conn->TransactionCommit();
      else
        conn->TransactionRollback();

      result = conn->GetDiagnosticRecords().GetReturnCode();

      break;
    }

    default: {
      result = SQL_INVALID_HANDLE;

      break;
    }
  }

  LOG_DEBUG_MSG("SQLEndTran exiting");

  return result;
}

SQLRETURN SQLGetData(SQLHSTMT stmt, SQLUSMALLINT colNum, SQLSMALLINT targetType,
                     SQLPOINTER targetValue, SQLLEN bufferLength,
                     SQLLEN* strLengthOrIndicator) {
  using namespace odbc::type_traits;

  using odbc::Statement;
  using odbc::app::ApplicationDataBuffer;

  LOG_DEBUG_MSG("SQLGetData called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLGetData exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  OdbcNativeType::Type driverType = ToDriverType(targetType);

  ApplicationDataBuffer dataBuffer(driverType, targetValue, bufferLength,
                                   strLengthOrIndicator);

  statement->GetColumnData(colNum, dataBuffer);

  LOG_DEBUG_MSG("SQLGetData exiting");

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLSetEnvAttr(SQLHENV env, SQLINTEGER attr, SQLPOINTER value,
                        SQLINTEGER valueLen) {
  using odbc::Environment;

  LOG_DEBUG_MSG("SQLSetEnvAttr called");
  LOG_INFO_MSG("Attribute: " << attr << ", Value: " << (size_t)value);

  Environment* environment = reinterpret_cast< Environment* >(env);

  if (!environment) {
    LOG_ERROR_MSG(
        "SQLSetEnvAttr exiting with SQL_INVALID_HANDLE because environment "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  environment->SetAttribute(attr, value, valueLen);

  LOG_DEBUG_MSG("SQLSetEnvAttr exiting");

  return environment->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLGetEnvAttr(SQLHENV env, SQLINTEGER attr, SQLPOINTER valueBuf,
                        SQLINTEGER valueBufLen, SQLINTEGER* valueResLen) {
  using namespace odbc;
  using namespace type_traits;

  using app::ApplicationDataBuffer;

  LOG_DEBUG_MSG("SQLGetEnvAttr called");

  Environment* environment = reinterpret_cast< Environment* >(env);

  if (!environment)
    return SQL_INVALID_HANDLE;

  SqlLen outResLen;
  ApplicationDataBuffer outBuffer(OdbcNativeType::AI_SIGNED_LONG, valueBuf,
                                  static_cast< int32_t >(valueBufLen),
                                  &outResLen);

  environment->GetAttribute(attr, outBuffer);

  if (valueResLen)
    *valueResLen = static_cast< SQLSMALLINT >(outResLen);

  LOG_DEBUG_MSG("SQLGetEnvAttr exiting");

  return environment->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLSpecialColumns(SQLHSTMT stmt, SQLSMALLINT idType,
                            SQLCHAR* catalogName, SQLSMALLINT catalogNameLen,
                            SQLCHAR* schemaName, SQLSMALLINT schemaNameLen,
                            SQLCHAR* tableName, SQLSMALLINT tableNameLen,
                            SQLSMALLINT scope, SQLSMALLINT nullable) {
  using namespace odbc;

  using odbc::utility::SqlStringToString;

  LOG_DEBUG_MSG("SQLSpecialColumns called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLSpecialColumns exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  std::string catalog = SqlStringToString(catalogName, catalogNameLen);
  std::string schema = SqlStringToString(schemaName, schemaNameLen);
  std::string table = SqlStringToString(tableName, tableNameLen);

  LOG_INFO_MSG("catalog: " << catalog);
  LOG_INFO_MSG("schema: " << schema);
  LOG_INFO_MSG("table: " << table);

  statement->ExecuteSpecialColumnsQuery(idType, catalog, schema, table, scope,
                                        nullable);

  LOG_DEBUG_MSG("SQLSpecialColumns exiting");

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLParamData(SQLHSTMT stmt, SQLPOINTER* value) {
  using namespace ignite::odbc;

  LOG_DEBUG_MSG("SQLParamData called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLParamData exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  statement->SelectParam(value);

  LOG_DEBUG_MSG("SQLParamData exiting");

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLPutData(SQLHSTMT stmt, SQLPOINTER data,
                     SQLLEN strLengthOrIndicator) {
  using namespace ignite::odbc;

  LOG_DEBUG_MSG("SQLPutData called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLPutData exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  statement->PutData(data, strLengthOrIndicator);

  LOG_DEBUG_MSG("SQLPutData exiting");

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLDescribeParam(SQLHSTMT stmt, SQLUSMALLINT paramNum,
                           SQLSMALLINT* dataType, SQLULEN* paramSize,
                           SQLSMALLINT* decimalDigits, SQLSMALLINT* nullable) {
  using namespace ignite::odbc;

  LOG_DEBUG_MSG("SQLDescribeParam called");

  Statement* statement = reinterpret_cast< Statement* >(stmt);

  if (!statement) {
    LOG_ERROR_MSG(
        "SQLDescribeParam exiting with SQL_INVALID_HANDLE because statement "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  statement->DescribeParam(paramNum, dataType, paramSize, decimalDigits,
                           nullable);

  LOG_DEBUG_MSG("SQLDescribeParam exiting");

  return statement->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQLError(SQLHENV env, SQLHDBC conn, SQLHSTMT stmt, SQLCHAR* state,
                   SQLINTEGER* error, SQLCHAR* msgBuf, SQLSMALLINT msgBufLen,
                   SQLSMALLINT* msgResLen) {
  using namespace ignite::odbc::utility;
  using namespace ignite::odbc;
  using namespace ignite::odbc::diagnostic;
  using namespace ignite::odbc::type_traits;

  using ignite::odbc::app::ApplicationDataBuffer;

  LOG_DEBUG_MSG("SQLError called");

  SQLHANDLE handle = 0;

  if (env != 0)
    handle = static_cast< SQLHANDLE >(env);
  else if (conn != 0)
    handle = static_cast< SQLHANDLE >(conn);
  else if (stmt != 0)
    handle = static_cast< SQLHANDLE >(stmt);
  else {
    LOG_ERROR_MSG("SQLError exiting with SQL_INVALID_HANDLE");
    return SQL_INVALID_HANDLE;
  }

  Diagnosable* diag = reinterpret_cast< Diagnosable* >(handle);

  DiagnosticRecordStorage& records = diag->GetDiagnosticRecords();

  int32_t recNum = records.GetLastNonRetrieved();

  if (recNum < 1 || recNum > records.GetStatusRecordsNumber()) {
    LOG_INFO_MSG("SQLError exiting with SQL_NO_DATA");
    return SQL_NO_DATA;
  }

  DiagnosticRecord& record = records.GetStatusRecord(recNum);

  record.MarkRetrieved();

  if (state)
    CopyStringToBuffer(record.GetSqlState(), reinterpret_cast< char* >(state),
                       6);

  if (error)
    *error = 0;

  SqlLen outResLen;
  ApplicationDataBuffer outBuffer(OdbcNativeType::AI_CHAR, msgBuf, msgBufLen,
                                  &outResLen);

  outBuffer.PutString(record.GetMessageText());

  if (msgResLen)
    *msgResLen = static_cast< SQLSMALLINT >(outResLen);

  LOG_DEBUG_MSG("SQLError exiting");

  return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLGetConnectAttr(SQLHDBC conn, SQLINTEGER attr,
                                    SQLPOINTER valueBuf, SQLINTEGER valueBufLen,
                                    SQLINTEGER* valueResLen) {
  using namespace odbc;
  using namespace type_traits;

  using app::ApplicationDataBuffer;

  LOG_DEBUG_MSG("SQLGetConnectAttr called");

  Connection* connection = reinterpret_cast< Connection* >(conn);

  if (!connection) {
    LOG_ERROR_MSG(
        "SQLGetConnectAttr exiting with SQL_INVALID_HANDLE because connection "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  connection->GetAttribute(attr, valueBuf, valueBufLen, valueResLen);

  LOG_DEBUG_MSG("SQLGetConnectAttr exiting");

  return connection->GetDiagnosticRecords().GetReturnCode();
}

SQLRETURN SQL_API SQLSetConnectAttr(SQLHDBC conn, SQLINTEGER attr,
                                    SQLPOINTER value, SQLINTEGER valueLen) {
  using odbc::Connection;

  LOG_DEBUG_MSG("SQLSetConnectAttr called(" << attr << ", " << value << ")");

  Connection* connection = reinterpret_cast< Connection* >(conn);

  if (!connection) {
    LOG_ERROR_MSG(
        "SQLSetConnectAttr exiting with SQL_INVALID_HANDLE because connection "
        "object is null");
    return SQL_INVALID_HANDLE;
  }

  connection->SetAttribute(attr, value, valueLen);

  LOG_DEBUG_MSG("SQLSetConnectAttr exiting");

  return connection->GetDiagnosticRecords().GetReturnCode();
}

}  // namespace ignite
