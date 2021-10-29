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

#include <stdio.h>
#include <string.h>

#include "odbc.h"
#include "misc.h"

#ifndef WIN32
#include <ctype.h>
#endif /* WIN32 */

#include "apifunc.h"
#include "bind.h"
#include "connection.h"
#include "convert.h"
#include "environ.h"
#include "odbc_statement.h"
#include "qresult.h"
#include "statement.h"
#include "types.h"

RETCODE SQL_API API_Prepare(HSTMT hstmt, const SQLCHAR *stmt_str,
                            SQLINTEGER stmt_sz) {
    CSTR func = "API_Prepare";
    if (hstmt == NULL)
        return SQL_ERROR;

    // We know cursor is not open at this point
    StatementClass *stmt = (StatementClass *)hstmt;
    RETCODE ret;
    if (!stmt_str) {
        SC_initialize_and_recycle(stmt);
        SC_set_error(stmt, STMT_INVALID_NULL_ARG, "Invalid use of null pointer",
                     func);
        return SQL_ERROR;
    }

    // Get statement size and type
    size_t original_stmt_len;
    int query_type = STMT_TYPE_UNKNOWN;
    if (stmt_sz >= 0) {
        original_stmt_len = stmt_sz;
        char *temp_stmt;
        temp_stmt = calloc(original_stmt_len + 1, sizeof(char));
        if (!temp_stmt) {
            SC_initialize_and_recycle(stmt);
            SC_set_error(stmt, STMT_NO_MEMORY_ERROR,
                         "No memory available to store statement", func);
            return SQL_ERROR;
        }
        strncpy(temp_stmt, (char *)stmt_str, original_stmt_len);
        query_type = statement_type(temp_stmt);
        free(temp_stmt);
    } else if (SQL_NTS == stmt_sz) {
        original_stmt_len = strlen((char *)stmt_str);
        query_type = statement_type((char *)stmt_str);
    } else {
        MYLOG(LOG_DEBUG, "invalid length=" FORMAT_INTEGER, stmt_sz);
        SC_initialize_and_recycle(stmt);
        SC_set_error(stmt, STMT_INVALID_NULL_ARG, "Invalid buffer length",
                     func);
        return SQL_ERROR;
    }

    // Only run metadata statement on SELECT statements
    if (query_type == STMT_TYPE_SELECT || query_type == STMT_TYPE_WITH) {
        // Construct the metadata statement
        const char *metadata_stmt_prefix = "SELECT * FROM (";
        const size_t metadata_stmt_prefix_len = 15;
        const char *metadata_stmt_suffix = ") AS original LIMIT 0";
        const size_t metadata_stmt_suffix_len = 21;
        const size_t metadata_stmt_affix_len = 36;
        char *metadata_stmt;
        metadata_stmt = calloc(metadata_stmt_affix_len + original_stmt_len + 1,
                               sizeof(char));
        if (!metadata_stmt) {
            SC_initialize_and_recycle(stmt);
            SC_set_error(
                stmt, STMT_NO_MEMORY_ERROR,
                "No memory available to store statement to get metadata", func);
            return SQL_ERROR;
        }
        strncpy(metadata_stmt, metadata_stmt_prefix,
                metadata_stmt_prefix_len + 1);
        strncat(metadata_stmt, (char *)stmt_str, original_stmt_len);
        strncat(metadata_stmt, metadata_stmt_suffix, metadata_stmt_suffix_len);

        // Prepare metadata statement to get metadata
        // PrepareStatement deallocates memory if necessary
        ret = PrepareStatement(
            stmt, (SQLCHAR *)metadata_stmt,
            (SQLINTEGER)(metadata_stmt_affix_len + original_stmt_len));

        // Not used afterwards
        free(metadata_stmt);

        if (ret != SQL_SUCCESS) {
            return ret;
        }

        // Execute the metadata statement
        ret = ExecuteStatement(stmt);
        if (ret == SQL_SUCCESS)
            stmt->prepared = PREPARED;

        // Restore to original statement
        strncpy_null(stmt->statement, (char *)stmt_str, original_stmt_len + 1);
    } else {
        ret = PrepareStatement(stmt, stmt_str, stmt_sz);
    }
    return ret;
}

RETCODE SQL_API API_Execute(HSTMT hstmt) {
    CSTR func = "API_Execute";
    if (hstmt == NULL)
        return SQL_ERROR;

    // We know cursor is not open at this point
    StatementClass *stmt = (StatementClass *)hstmt;
    RETCODE ret = SQL_ERROR;
    switch (stmt->prepared) {
        case PREPARED:
            SC_reset_result_for_rerun(stmt);
            ret = ExecuteStatement(stmt);
            stmt->prepared = EXECUTED;
            break;
        case EXECUTED:
            ret = RePrepareStatement(stmt);
            if (ret != SQL_SUCCESS)
                break;
            ret = ExecuteStatement(stmt);
            if (ret != SQL_SUCCESS)
                break;
            stmt->prepared = EXECUTED;
            break;
        case NOT_PREPARED:
            SC_set_error(stmt, STMT_SEQUENCE_ERROR, "No prepared statement", func);
            ret = SQL_ERROR;
            break;
        default:
            break;
    }
    return ret;
}

RETCODE SQL_API API_ExecDirect(HSTMT hstmt, const SQLCHAR *stmt_str,
                               SQLINTEGER stmt_sz) {
    if (hstmt == NULL)
        return SQL_ERROR;

    // We know cursor is not open at this point
    StatementClass *stmt = (StatementClass *)hstmt;
    RETCODE ret = PrepareStatement(stmt, stmt_str, stmt_sz);
    if (ret != SQL_SUCCESS)
        return ret;

    // Execute statement
    ret = ExecuteStatement(hstmt);
    if (ret != SQL_SUCCESS)
        return ret;
    stmt->prepared = NOT_PREPARED;
    return ret;
}

/*
 *	Returns the SQL string as modified by the driver.
 *	Currently, just copy the input string without modification
 *	observing buffer limits and truncation.
 */
RETCODE SQL_API API_NativeSql(HDBC hdbc, const SQLCHAR *szSqlStrIn,
                                SQLINTEGER cbSqlStrIn, SQLCHAR *szSqlStr,
                                SQLINTEGER cbSqlStrMax, SQLINTEGER *pcbSqlStr) {
    CSTR func = "API_NativeSql";
    size_t len = 0;
    char *ptr;
    ConnectionClass *conn = (ConnectionClass *)hdbc;
    RETCODE result;

    MYLOG(LOG_TRACE, "entering...cbSqlStrIn=" FORMAT_INTEGER, cbSqlStrIn);

    ptr = (cbSqlStrIn == 0) ? "" : make_string(szSqlStrIn, cbSqlStrIn, NULL, 0);
    if (!ptr) {
        CC_set_error(conn, CONN_NO_MEMORY_ERROR,
                     "No memory available to store native sql string", func);
        return SQL_ERROR;
    }

    result = SQL_SUCCESS;
    len = strlen(ptr);

    if (szSqlStr) {
        strncpy_null((char *)szSqlStr, ptr, cbSqlStrMax);

        if (len >= (size_t)cbSqlStrMax) {
            result = SQL_SUCCESS_WITH_INFO;
            CC_set_error(conn, CONN_TRUNCATED,
                         "The buffer was too small for the NativeSQL.", func);
        }
    }

    if (pcbSqlStr)
        *pcbSqlStr = (SQLINTEGER)len;

    if (cbSqlStrIn)
        free(ptr);

    return result;
}
