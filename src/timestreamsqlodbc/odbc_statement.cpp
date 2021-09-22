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

#include "odbc_statement.h"

#include "apifunc.h"
#include "environ.h"  // Critical section for statment
#include "helper.h"
#include "misc.h"
#include "statement.h"
#include <aws/timestream-query/TimestreamQueryClient.h>

extern "C" void *common_cs;

RETCODE ExecuteStatement(StatementClass *stmt) {
    CSTR func = "ExecuteStatement";
    int func_cs_count = 0;
    ConnectionClass *conn = SC_get_conn(stmt);
    CONN_Status oldstatus = conn->status;

    auto CleanUp = [&]() -> RETCODE {
        SC_SetExecuting(stmt, FALSE);
        CLEANUP_FUNC_CONN_CS(func_cs_count, conn);
        if (conn->status != CONN_DOWN)
            conn->status = oldstatus;
        if (SC_get_errornumber(stmt) == STMT_OK)
            return SQL_SUCCESS;
        else if (SC_get_errornumber(stmt) < STMT_OK)
            return SQL_SUCCESS_WITH_INFO;
        else {
            if (!SC_get_errormsg(stmt) || !SC_get_errormsg(stmt)[0]) {
                if (STMT_NO_MEMORY_ERROR != SC_get_errornumber(stmt))
                    SC_set_errormsg(stmt,
                                    "Error while executing the query");
                SC_log_error(func, NULL, stmt);
            }
            return SQL_ERROR;
        }
    };

    ENTER_INNER_CONN_CS(conn, func_cs_count);

    if (conn->status == CONN_EXECUTING) {
        SC_set_error(stmt, STMT_SEQUENCE_ERROR,
                        "Connection is already in use.", func);
        return CleanUp();
    }

    if (!SC_SetExecuting(stmt, TRUE)) {
        SC_set_error(stmt, STMT_OPERATION_CANCELLED,
                        "Cancel Request Accepted", func);
        return CleanUp();
    }

    conn->status = CONN_EXECUTING;

    QResultClass *res = SendQueryGetResult(stmt);
    if (!res) {
        if (SC_get_errornumber(stmt) <= 0) {
            SC_set_error(stmt, STMT_NO_RESPONSE,
                         "Failed to retrieve error message from result. "
                         "Connection may be down.",
                         func);
        }
        return CleanUp();
    }

    if (CONN_DOWN != conn->status)
        conn->status = oldstatus;
    stmt->status = STMT_FINISHED;
    LEAVE_INNER_CONN_CS(func_cs_count, conn);

    // Check the status of the result
    if (SC_get_errornumber(stmt) < 0) {
        if (QR_command_successful(res))
            SC_set_errornumber(stmt, STMT_OK);
        else if (QR_command_nonfatal(res))
            SC_set_errornumber(stmt, STMT_INFO_ONLY);
        else
            SC_set_errorinfo(stmt, res, 0);
    }

    // Set cursor before the first tuple in the list
    stmt->currTuple = -1;
    SC_set_current_col(stmt, static_cast< int >(stmt->currTuple));
    SC_set_rowset_start(stmt, stmt->currTuple, FALSE);

    // Only perform if query was not aborted
    if (!QR_get_aborted(res)) {
        // Check if result columns were obtained from query
        for (QResultClass *tres = res; tres; tres = tres->next) {
            Int2 numcols = QR_NumResultCols(tres);
            if (numcols <= 0)
                continue;
            ARDFields *opts = SC_get_ARDF(stmt);
            extend_column_bindings(opts, numcols);
            if (opts->bindings)
                break;

            // Failed to allocate
            QR_Destructor(res);
            SC_set_error(stmt, STMT_NO_MEMORY_ERROR,
                            "Could not get enough free memory to store "
                            "the binding information",
                            func);
            return CleanUp();
        }
    }

    QResultClass *last = SC_get_Result(stmt);
    if (last) {
        // Statement already contains a result
        // Append to end if this hasn't happened
        while (last->next != NULL) {
            if (last == res)
                break;
            last = last->next;
        }
        if (last != res)
            last->next = res;
    } else {
        // Statement does not contain a result
        // Assign directly
        SC_set_Result(stmt, res);
    }

    stmt->diag_row_count = res->recent_processed_row_count;

    return CleanUp();
}

SQLRETURN GetNextResultSet(StatementClass *stmt) {
    CSTR func = "GetNextResultSet";
    ConnectionClass *cc = SC_get_conn(stmt);
    QResultClass *q_res = SC_get_Result(stmt);
    if ((q_res == NULL) && (cc == NULL)) {
        return SQL_ERROR;
    }
    
    PrefetchQueue *pPrefetchQueue = GetPrefetchQueue(cc->conn, stmt);
    if (pPrefetchQueue != nullptr && !pPrefetchQueue->IsEmpty()) {
        if (pPrefetchQueue->WaitForReadinessOfFront()) {
            auto outcome = pPrefetchQueue->Front();
            pPrefetchQueue->Pop();
            if (outcome.IsSuccess()) {
                if (!outcome.GetResult().GetNextToken().empty()
                    && pPrefetchQueue->IsRetrieving()) {
                    QR_set_next_token(
                        q_res, outcome.GetResult().GetNextToken().c_str());
                } else {
                    QR_set_next_token(q_res, NULL);
                }
                // Responsible for looping through rows, allocating tuples and
                // appending these rows in q_result
                CC_Append_Table_Data(outcome, q_res, *(q_res->fields));
                return SQL_SUCCESS;
            } else {
                QR_set_next_token(q_res, NULL);
                SC_set_error(stmt, STMT_EXEC_ERROR,
                             outcome.GetError().GetMessage().c_str(), func);
                return SQL_ERROR;
            }
        } else {
            QR_set_next_token(q_res, NULL);
            SC_set_error(stmt, STMT_OPERATION_CANCELLED, "Operation cancelled",
                         func);
            return SQL_ERROR;
        }
    } else {
        QR_set_next_token(q_res, NULL);
        if (pPrefetchQueue == nullptr) {
            SC_set_error(stmt, STMT_INTERNAL_ERROR,
                         "PrefetchQueue is not found", func);
        }
        return SQL_ERROR;
    }
}

RETCODE RePrepareStatement(StatementClass *stmt) {
    CSTR func = "RePrepareStatement";
    SC_reset_result_for_rerun(stmt);
    if (!stmt->statement) {
        SC_set_error(stmt, STMT_NO_MEMORY_ERROR,
                     "Expected statement to be allocated.", func);
        return SQL_ERROR;
    }

    // If an SQLPrepare was performed prior to this, but was left in the
    // described state because an error prior to SQLExecute then set the
    // statement to finished so it can be recycled.
    if (stmt->status == STMT_DESCRIBED)
        stmt->status = STMT_FINISHED;

    return SQL_SUCCESS;
}

RETCODE PrepareStatement(StatementClass *stmt, const SQLCHAR *stmt_str,
                         SQLINTEGER stmt_sz) {
    CSTR func = "PrepareStatement";
    RETCODE result = SC_initialize_and_recycle(stmt);
    if (result != SQL_SUCCESS)
        return result;

    stmt->statement = make_string(stmt_str, stmt_sz, NULL, 0);
    if (!stmt->statement) {
        SC_set_error(stmt, STMT_NO_MEMORY_ERROR,
                     "No memory available to store statement", func);
        return SQL_ERROR;
    }

    // If an SQLPrepare was performed prior to this, but was left in the
    // described state because an error prior to SQLExecute then set the
    // statement to finished so it can be recycled.
    if (stmt->status == STMT_DESCRIBED)
        stmt->status = STMT_FINISHED;
    stmt->statement_type = (short)statement_type(stmt->statement);

    return SQL_SUCCESS;
}

QResultClass *SendQueryGetResult(StatementClass *stmt) {
    CSTR func = "SendQueryGetResult";
    if (stmt == NULL)
        return NULL;

    // Execute the query
    ConnectionClass *cc = SC_get_conn(stmt);
    if (!ExecDirect(cc->conn, stmt, stmt->statement)) {
        return NULL;
    }

    // Allocate QResultClass
    QResultClass *res = QR_Constructor();
    if (res == NULL)
        return NULL;

    res->rstatus = PORES_COMMAND_OK;

    PrefetchQueue *pPrefetchQueue = GetPrefetchQueue(cc->conn, stmt);
    if (pPrefetchQueue != nullptr && !pPrefetchQueue->IsEmpty()) {
        if (pPrefetchQueue->WaitForReadinessOfFront()) {
            bool success = false;
            auto outcome = pPrefetchQueue->Front();
            pPrefetchQueue->Pop();
            if (outcome.IsSuccess()) {
                success =
                    CC_from_TSResult(res, cc, stmt, res->cursor_name, outcome);

                // Check for pagination
                if (!outcome.GetResult().GetNextToken().empty()
                    && pPrefetchQueue->IsRetrieving()) {
                    QR_set_next_token(
                        res, outcome.GetResult().GetNextToken().c_str());
                } else {
                    QR_set_next_token(res, NULL);
                }
            } else {
                success = false;
                SC_set_error(stmt, STMT_EXEC_ERROR,
                             outcome.GetError().GetMessage().c_str(), func);
            }
            if (!success) {
                QR_Destructor(res);
                res = NULL;
            }
        } else {
            SC_set_error(stmt, STMT_OPERATION_CANCELLED,
                         "Operation cancelled", func);
            QR_Destructor(res);
            res = NULL;
        }
    } else {
        if (pPrefetchQueue == nullptr) {
            SC_set_error(stmt, STMT_INTERNAL_ERROR,
                         "PrefetchQueue is not found", func);
        }
        QR_Destructor(res);
        res = NULL;
    }
    return res;
}

SQLRETURN API_Cancel(HSTMT hstmt) {
    // Verify pointer validity and convert to StatementClass
    if (hstmt == NULL)
        return SQL_INVALID_HANDLE;
    StatementClass *stmt = (StatementClass *)hstmt;

    // Get execution delegate (if applicable) and initialize return code
    StatementClass *estmt =
        (stmt->execute_delegate == NULL) ? stmt : stmt->execute_delegate;
    SQLRETURN ret = SQL_SUCCESS;

    // Entry common critical section
    ENTER_COMMON_CS;

    // Waiting for more data from SQLParamData/SQLPutData - cancel statement
    if (estmt->data_at_exec >= 0) {
        // Enter statement critical section
        ENTER_STMT_CS(stmt);

        // Clear info and cancel need data
        SC_clear_error(stmt);
        estmt->data_at_exec = -1;
        estmt->put_data = FALSE;
        cancelNeedDataState(estmt);

        // Leave statement critical section
        LEAVE_STMT_CS(stmt);
    } else {
        CancelQuery(stmt);
    }

    // Leave common critical section
    LEAVE_COMMON_CS;

    return ret;
}
