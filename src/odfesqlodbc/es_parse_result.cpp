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

#include "es_parse_result.h"

#include <unordered_map>

#include "es_helper.h"
#include "es_types.h"
#ifdef __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif  // __APPLE__
#include "rabbit.hpp"
#ifdef __APPLE__
#pragma clang diagnostic pop
#endif  // __APPLE__
#include "statement.h"

typedef std::vector< std::pair< std::string, OID > > schema_type;
typedef rabbit::array json_arr;
typedef json_arr::iterator::result_type json_arr_it;

bool _CC_from_ESResult(QResultClass *q_res, ConnectionClass *conn,
                       const char *cursor, ESResult &es_result);
bool _CC_Metadata_from_ESResult(QResultClass *q_res, ConnectionClass *conn,
                                const char *cursor, ESResult &es_result);
bool _CC_No_Metadata_from_ESResult(QResultClass *q_res, ConnectionClass *conn,
                                   const char *cursor, ESResult &es_result);
void GetSchemaInfo(schema_type &schema, json_doc &es_result_doc);
bool AssignColumnHeaders(QResultClass *q_res,
                         const ESResult &es_result);
bool AssignTableData(ESResult &es_result, QResultClass *q_res, ColumnInfoClass &fields);
bool AssignRowData(const Aws::TimestreamQuery::Model::Row &row,
                   QResultClass *q_res, ColumnInfoClass &fields,
                   const size_t &row_size);
void UpdateResultFields(QResultClass *q_res, const ConnectionClass *conn,
                        const SQLULEN starting_cached_rows, const char *cursor,
                        std::string &command_type);
bool QR_prepare_for_tupledata(QResultClass *q_res);
void SetError(const char *err);
void ClearError();

// clang-format off
// Not all of these are being used at the moment, but these are the keywords in the json
static const std::string JSON_KW_SCHEMA = "schema";
static const std::string JSON_KW_NAME = "name";
static const std::string JSON_KW_TYPE = "type";
static const std::string JSON_KW_TOTAL = "total";
static const std::string JSON_KW_SIZE = "size";
static const std::string JSON_KW_STATUS = "status";
static const std::string JSON_KW_DATAROWS = "datarows";
static const std::string JSON_KW_ERROR = "error";
static const std::string JSON_KW_CURSOR = "cursor";

// clang-format on
const std::unordered_map< std::string, OID > type_to_oid_map = {
    {ES_TYPE_NAME_BOOLEAN, ES_TYPE_BOOL},
    {ES_TYPE_NAME_BYTE, ES_TYPE_INT2},
    {ES_TYPE_NAME_SHORT, ES_TYPE_INT2},
    {ES_TYPE_NAME_INTEGER, ES_TYPE_INT4},
    {ES_TYPE_NAME_LONG, ES_TYPE_INT8},
    {ES_TYPE_NAME_HALF_FLOAT, ES_TYPE_FLOAT4},
    {ES_TYPE_NAME_FLOAT, ES_TYPE_FLOAT4},
    {ES_TYPE_NAME_DOUBLE, ES_TYPE_FLOAT8},
    {ES_TYPE_NAME_SCALED_FLOAT, ES_TYPE_FLOAT8},
    {ES_TYPE_NAME_KEYWORD, ES_TYPE_VARCHAR},
    {ES_TYPE_NAME_TEXT, ES_TYPE_VARCHAR},
    {ES_TYPE_NAME_DATE, ES_TYPE_TIMESTAMP},
    {ES_TYPE_NAME_OBJECT, ES_TYPE_VARCHAR},
    {ES_TYPE_NAME_VARCHAR, ES_TYPE_VARCHAR},
    {ES_TYPE_NAME_DATE, ES_TYPE_DATE},

    {TS_TYPE_NAME_VARCHAR, TS_TYPE_VARCHAR},
    {TS_TYPE_NAME_BOOLEAN, TS_TYPE_BOOLEAN},
    {TS_TYPE_NAME_BIGINT, TS_TYPE_BIGINT},
    {TS_TYPE_NAME_DOUBLE, TS_TYPE_DOUBLE},
    {TS_TYPE_NAME_TIMESTAMP, TS_TYPE_TIMESTAMP},
    {TS_TYPE_NAME_DATE, TS_TYPE_DATE},
    {TS_TYPE_NAME_TIME, TS_TYPE_TIME},
    {TS_TYPE_NAME_INTERVAL_DAY_TO_SECOND, TS_TYPE_INTERVAL_DAY_TO_SECOND},
    {TS_TYPE_NAME_INTERVAL_YEAR_TO_MONTH, TS_TYPE_INTERVAL_YEAR_TO_MONTH},
    {TS_TYPE_NAME_UNKNOWN, TS_TYPE_UNKNOWN},
    {TS_TYPE_NAME_INTEGER, TS_TYPE_INTEGER},
    {TS_TYPE_NAME_ROW, TS_TYPE_ROW},
    {TS_TYPE_NAME_ARRAY, TS_TYPE_ARRAY},
    {TS_TYPE_NAME_TIMESERIES, TS_TYPE_TIMESERIES}};

#define ES_VARCHAR_SIZE (-2)
#define TS_ROW_SIZE (-3)
#define TS_ARRAY_SIZE (-4)
#define TS_TIMESERIES_SIZE (-5)

const std::unordered_map< OID, int16_t > oid_to_size_map = {
    {ES_TYPE_BOOL, (int16_t)1},
    {ES_TYPE_INT2, (int16_t)2},
    {ES_TYPE_INT4, (int16_t)4},
    {ES_TYPE_INT8, (int16_t)8},
    {ES_TYPE_FLOAT4, (int16_t)4},
    {ES_TYPE_FLOAT8, (int16_t)8},
    {ES_TYPE_VARCHAR, (int16_t)ES_VARCHAR_SIZE},
    {ES_TYPE_DATE, (int16_t)ES_VARCHAR_SIZE},
    {ES_TYPE_TIMESTAMP, (int16_t)1},

    {TS_TYPE_VARCHAR, (int16_t)ES_VARCHAR_SIZE},
    {TS_TYPE_BOOLEAN, (int16_t)1},
    {TS_TYPE_BIGINT, (int16_t)8},
    {TS_TYPE_DOUBLE, (int16_t)8},
    {TS_TYPE_TIMESTAMP, (int16_t)1},
    {TS_TYPE_DATE, (int16_t)ES_VARCHAR_SIZE},
    {TS_TYPE_TIME, (int16_t)ES_VARCHAR_SIZE},
    {TS_TYPE_INTERVAL_DAY_TO_SECOND, (int16_t)ES_VARCHAR_SIZE},
    {TS_TYPE_INTERVAL_YEAR_TO_MONTH, (int16_t)ES_VARCHAR_SIZE},
    {TS_TYPE_UNKNOWN, (int16_t)ES_VARCHAR_SIZE},
    {TS_TYPE_INTEGER, (int16_t)4},
    {TS_TYPE_ROW, (int16_t)TS_ROW_SIZE},
    {TS_ARRAY_SIZE, (int16_t)TS_ARRAY_SIZE},
    {TS_TIMESERIES_SIZE, (int16_t)TS_TIMESERIES_SIZE}};

// Using global variable here so that the error message can be propagated
// without going otu of scope
std::string error_msg;

void SetError(const char *err) {
    error_msg = err;
}
void ClearError() {
    error_msg = "";
}
std::string GetResultParserError() {
    return error_msg;
}

BOOL CC_from_ESResult(QResultClass *q_res, ConnectionClass *conn,
                      const char *cursor, ESResult &es_result) {
    ClearError();
    return _CC_from_ESResult(q_res, conn, cursor, es_result) ? TRUE : FALSE;
}

BOOL CC_Metadata_from_ESResult(QResultClass *q_res, ConnectionClass *conn,
                               const char *cursor, ESResult &es_result) {
    ClearError();
    return _CC_Metadata_from_ESResult(q_res, conn, cursor, es_result) ? TRUE : FALSE;
}

BOOL CC_No_Metadata_from_ESResult(QResultClass *q_res, ConnectionClass *conn,
                                  const char *cursor, ESResult &es_result) {
    ClearError();
    return _CC_No_Metadata_from_ESResult(q_res, conn, cursor, es_result)
               ? TRUE
               : FALSE;
}

BOOL CC_Append_Table_Data(ESResult &es_result, QResultClass *q_res, ColumnInfoClass &fields) {
    ClearError();
    return AssignTableData(es_result, q_res, fields)
               ? TRUE
               : FALSE;
}

bool _CC_No_Metadata_from_ESResult(QResultClass *q_res, ConnectionClass *conn,
                                   const char *cursor, ESResult &es_result) {
    // Note - NULL conn and/or cursor is valid
    if (q_res == NULL)
        return false;

    try {
        schema_type doc_schema;
        GetSchemaInfo(doc_schema, es_result.es_result_doc);

        SQLULEN starting_cached_rows = q_res->num_cached_rows;

        // Assign table data and column headers
        if (!AssignTableData(es_result, q_res, *(q_res->fields)))
            return false;

        // Update fields of QResult to reflect data written
        UpdateResultFields(q_res, conn, starting_cached_rows, cursor,
                           es_result.command_type);

        // Return true (success)
        return true;
    } catch (const rabbit::type_mismatch &e) {
        SetError(e.what());
    } catch (const rabbit::parse_error &e) {
        SetError(e.what());
    } catch (const std::exception &e) {
        SetError(e.what());
    } catch (...) {
        SetError("Unknown exception thrown in _CC_No_Metadata_from_ESResult.");
    }

    // Exception occurred, return false (error)
    return false;
}

bool _CC_Metadata_from_ESResult(QResultClass *q_res, ConnectionClass *conn,
                                const char *cursor, ESResult &es_result) {
    // Note - NULL conn and/or cursor is valid
    if (q_res == NULL)
        return false;

    QR_set_conn(q_res, conn);
    try {
        schema_type doc_schema;
        GetSchemaInfo(doc_schema, es_result.es_result_doc);

        // Assign table data and column headers
        if (!AssignColumnHeaders(q_res, es_result))
            return false;

        // Set command type and cursor name
        QR_set_command(q_res, es_result.command_type.c_str());
        QR_set_cursor(q_res, cursor);
        if (cursor == NULL)
            QR_set_reached_eof(q_res);

        // Return true (success)
        return true;
    } catch (const rabbit::type_mismatch &e) {
        SetError(e.what());
    } catch (const rabbit::parse_error &e) {
        SetError(e.what());
    } catch (const std::exception &e) {
        SetError(e.what());
    } catch (...) {
        SetError("Unknown exception thrown in _CC_Metadata_from_ESResult.");
    }

    // Exception occurred, return false (error)
    return false;
}

void print_log(const std::string &s) {
#if WIN32
#pragma warning(push)
#pragma warning(disable : 4551)
#endif  // WIN32
        // cppcheck outputs an erroneous missing argument error which breaks
        // build. Disable for this function call
    MYLOG(DRV_ALL, "%s\n", s.c_str());
#if WIN32
#pragma warning(pop)
#endif  // WIN32
}

bool _CC_from_ESResult(QResultClass *q_res, ConnectionClass *conn,
                       const char *cursor, ESResult &es_result) {
    // Note - NULL conn and/or cursor is valid
    if (q_res == NULL)
        return false;

    QR_set_conn(q_res, conn);
    try {
        SQLULEN starting_cached_rows = q_res->num_cached_rows;

        // Assign table data and column headers
        if ((!AssignColumnHeaders(q_res, es_result))
            || (!AssignTableData(es_result, q_res, *(q_res->fields))))
            return false;

        // Update fields of QResult to reflect data written
        std::string command_type = "SELECT";
        UpdateResultFields(q_res, conn, starting_cached_rows, cursor,
                           command_type);

        // Return true (success)
        return true;
    } catch (const rabbit::type_mismatch &e) {
        SetError(e.what());
    } catch (const rabbit::parse_error &e) {
        SetError(e.what());
    } catch (const std::exception &e) {
        SetError(e.what());
    } catch (...) {
        SetError("Unknown exception thrown in CC_from_ESResult.");
    }

    // Exception occurred, return false (error)
    return false;
}

void GetSchemaInfo(schema_type &schema, json_doc &es_result_doc) {
    json_arr schema_arr = es_result_doc[JSON_KW_SCHEMA];
    for (auto it : schema_arr) {
        auto mapped_oid = type_to_oid_map.find(it[JSON_KW_TYPE].as_string());
        OID type_oid = (mapped_oid == type_to_oid_map.end())
                           ? SQL_WVARCHAR
                           : mapped_oid->second;
        schema.push_back(
            std::make_pair(it[JSON_KW_NAME].as_string(), type_oid));
    }
}

bool AssignColumnHeaders(QResultClass *q_res,
                         const ESResult &es_result) {
    // Allocte memory for column fields
    const auto &column_info = es_result.ts_result.GetColumnInfo();
    QR_set_num_fields(q_res, (uint16_t)column_info.size());
    if (QR_get_fields(q_res)->coli_array == NULL)
        return false;
    
    for (size_t i = 0; i < column_info.size(); i++) {
        auto column = column_info[i];
        auto type =
            Aws::TimestreamQuery::Model::ScalarTypeMapper::GetNameForScalarType(
                column.GetType().GetScalarType());
        // TODO Some historic fields needs to be removed, set 0 fow now.
        CI_set_field_info(QR_get_fields(q_res), (int)i,
                          column.GetName().c_str(),
                          TS_TYPE_UNKNOWN, ES_ADT_UNSET, 0, 0, 0);
        QR_set_rstatus(q_res, PORES_FIELDS_OK);
    }
    q_res->num_fields = CI_get_num_fields(QR_get_fields(q_res));

    return true;
}

// Responsible for looping through rows, allocating tuples and passing rows for
// assignment
bool AssignTableData(ESResult &es_result, QResultClass *q_res, ColumnInfoClass &fields) {
    auto rows = es_result.ts_result.GetRows();
    for (const auto& row : rows) {
        // Setup memory to receive tuple
        if (!QR_prepare_for_tupledata(q_res))
            return false;

        // Assign row data
        if (!AssignRowData(row, q_res, fields, es_result.ts_result.GetColumnInfo().size()))
            return false;
    }

    return true;
}

// Responsible for assigning row data to tuples
bool AssignRowData(const Aws::TimestreamQuery::Model::Row &row,
                   QResultClass *q_res, ColumnInfoClass &fields,
                   const size_t &col_size) {
    TupleField *tuple =
        q_res->backend_tuples + (q_res->num_cached_rows * col_size);

    // Setup keyset if present
    KeySet *ks = NULL;
    if (QR_haskeyset(q_res)) {
        ks = q_res->keyset + q_res->num_cached_keys;
        ks->status = 0;
    }

    // Loop through and assign data
    for (size_t i = 0; i < row.GetData().size(); i++) {
        auto datum = row.GetData()[i];
        auto scalar_value = datum.GetScalarValue();
        if (scalar_value.empty()) {
            tuple[i].len = SQL_NULL_DATA;
            tuple[i].value = NULL;
        } else {
            // Copy string over to tuple
            const std::string data = scalar_value;
            tuple[i].len = static_cast< int >(data.length());
            QR_MALLOC_return_with_error(
                tuple[i].value, char, data.length() + 1, q_res,
                "Out of memory in allocating item buffer.", false);
            strcpy((char *)tuple[i].value, data.c_str());

            // If data length exceeds current display size, set display size
            if (fields.coli_array[i].display_size < tuple[i].len)
                fields.coli_array[i].display_size = tuple[i].len;
        }
    }

    // TODO Commented out for now, needs refactoring
    // If there are more rows than schema suggests, we have Keyset data
    /*if (row_size > row_schema_size) {
        if (ks == NULL) {
            QR_set_rstatus(q_res, PORES_INTERNAL_ERROR);
            QR_set_message(q_res,
                           "Keyset was NULL, but Keyset data was expected.");
            return false;
        }

        auto row_column = row.value_begin() + row_schema_size;
        if (sscanf(row_column->str().c_str(), "(%u,%hu)", &ks->blocknum,
                   &ks->offset)
            != 2) {
            QR_set_rstatus(q_res, PORES_INTERNAL_ERROR);
            QR_set_message(q_res, "Failed to assign Keyset.");
            return false;
        }
        row_column++;
        ks->oid = std::stoul(row_column->str(), nullptr, 10);
    }*/

    // Increment relevant data
    q_res->cursTuple++;
    if (q_res->num_fields > 0)
        QR_inc_num_cache(q_res);
    else if (QR_haskeyset(q_res))
        q_res->num_cached_keys++;

    if ((SQLULEN)q_res->cursTuple >= q_res->num_total_read)
        q_res->num_total_read = q_res->cursTuple + 1;
    return true;
}

void UpdateResultFields(QResultClass *q_res, const ConnectionClass *conn,
                        const SQLULEN starting_cached_rows, const char *cursor,
                        std::string &command_type) {
    // Adjust total read
    if (!QR_once_reached_eof(q_res)
        && q_res->cursTuple >= (Int4)q_res->num_total_read)
        q_res->num_total_read = q_res->cursTuple + 1;

    // Adjust eof and tuple cursor
    if (q_res->num_cached_rows - starting_cached_rows < q_res->cmd_fetch_size) {
        QR_set_reached_eof(q_res);
        if (q_res->cursTuple < (Int4)q_res->num_total_read)
            q_res->cursTuple = q_res->num_total_read;
    }

    // Handle NULL connection
    if (conn != NULL) {
        q_res->fetch_number = static_cast< SQLLEN >(0);
        QR_set_rowstart_in_cache(q_res, 0);
        q_res->key_base = 0;
    }

    // Set command type and cursor name
    QR_set_command(q_res, command_type.c_str());
    QR_set_cursor(q_res, cursor);
    if (cursor == NULL)
        QR_set_reached_eof(q_res);

    // Set flags, adjust pointers, and return true (success)
    q_res->dataFilled = true;
    q_res->tupleField =
        q_res->backend_tuples + (q_res->fetch_number * q_res->num_fields);
    QR_set_rstatus(q_res, PORES_TUPLES_OK);
}

bool QR_prepare_for_tupledata(QResultClass *q_res) {
    if (QR_get_cursor(q_res)) {
        return true;
    }

    // If total tuples > allocated tuples, need to reallocate
    if (q_res->num_fields > 0
        && QR_get_num_total_tuples(q_res) >= q_res->count_backend_allocated) {
        SQLLEN tuple_size = (q_res->count_backend_allocated < 1)
                                ? TUPLE_MALLOC_INC
                                : q_res->count_backend_allocated * 2;

        // Will return false if allocation fails
        QR_REALLOC_return_with_error(
            q_res->backend_tuples, TupleField,
            tuple_size * q_res->num_fields * sizeof(TupleField), q_res,
            "Out of memory while reading tuples.", false);
        q_res->count_backend_allocated = tuple_size;
    }

    // If total keyset > allocated keyset, need to reallocate
    if (QR_haskeyset(q_res)
        && q_res->num_cached_keys >= q_res->count_keyset_allocated) {
        SQLLEN keyset_size = (q_res->count_keyset_allocated < 1)
                                 ? TUPLE_MALLOC_INC
                                 : q_res->count_keyset_allocated * 2;

        // Will return false if macro fails
        QR_REALLOC_return_with_error(
            q_res->keyset, KeySet, sizeof(KeySet) * keyset_size, q_res,
            "Out of memory while allocating keyset", false);
        memset(&q_res->keyset[q_res->count_keyset_allocated], 0,
               (keyset_size - q_res->count_keyset_allocated) * sizeof(KeySet));
        q_res->count_keyset_allocated = keyset_size;
    }

    return true;
}
