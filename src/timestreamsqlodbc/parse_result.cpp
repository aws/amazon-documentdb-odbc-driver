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

#include "parse_result.h"

#include <map>

#include "helper.h"
#include "statement.h"
#include "types.h"

typedef std::vector< std::pair< std::string, OID > > schema_type;

bool _CC_from_TSResult(
    QResultClass *q_res, ConnectionClass *conn, StatementClass *stmt,
    const char *next_token,
    const Aws::TimestreamQuery::Model::QueryOutcome &ts_result);

/**
 * Responsible for looping through columns, allocating memory for column fields
 * and setting column info
 * @param q_res QResultClass to set column info
 * @param ts_result Timestream query outcome to get column info
 * @return true if successfully assigned
 */
bool AssignColumnHeaders(QResultClass *q_res,
    const Aws::TimestreamQuery::Model::QueryOutcome &ts_result);

/**
 * Responsible for looping through rows, allocating tuples and passing rows for
 * assignment
 * @param ts_result Timestream query outcome to get rows
 * @param q_res QResultClass to hold tuples
 * @param fields ColumnInfoClass to get information about columns
 * @return true if successfully assigned
 */
bool AssignTableData(const Aws::TimestreamQuery::Model::QueryOutcome &ts_result,
                     QResultClass *q_res, ColumnInfoClass &fields);

/**
 * Responsible for assigning the current row data to tuples
 * @param row Timestream row object
 * @param q_res QResultClass to hold tuples
 * @param fields ColumnInfoClass to get information about columns
 * @param col_size size of the column
 * @return true if successfully assigned
 */
bool AssignRowData(const Aws::TimestreamQuery::Model::Row &row,
                   QResultClass *q_res, ColumnInfoClass &fields,
                   const size_t &col_size);
void UpdateResultFields(QResultClass *q_res, const ConnectionClass *conn,
                        const SQLULEN starting_cached_rows, const char *next_token,
                        std::string &command_type);
bool QR_prepare_for_tupledata(QResultClass *q_res);
void SetError(const char *err);
void ClearError();

/**
 * Recursively parse the Timestream datum object to intermidiate string
 * representation for further conversion.
 * @param datum Timestream Datum object to be parsed
 * @param datum_value String representation of the datum to fill in recursively
 * @param column_attr_id Column attribute ID
 */
void ParseDatum(const Aws::TimestreamQuery::Model::Datum &datum,
                std::string &datum_value, OID column_attr_id);

/**
 * Parse Timestream array object to intermidiate string
 * representation for further conversion.
 * e.g. ARRAY[1.1, 2.3] -> [1.1, 2.3]
 * @param datums An array of Timestream Datum objects to be parsed
 * @param array_value String representation of the datum to fill in recursively
 * @param column_attr_id Column attribute ID
 */
void ParseArray(const Aws::Vector< Aws::TimestreamQuery::Model::Datum > &datums,
                std::string &array_value, OID column_attr_id);

/**
 * Parse Timestream row object to intermidiate string
 * representation for further conversion.
 * e.g. ROW(INTEGER '03', BIGINT '10', true) -> (3, 10, true)
 * @param datums An array of Timestream Datum objects to be parsed
 * @param row_value String representation of the datum to fill in recursively
 * @param column_attr_id Column attribute ID
 */
void ParseRow(const Aws::Vector< Aws::TimestreamQuery::Model::Datum > &datums,
              std::string &row_value, OID column_attr_id);

/**
 * Parse Timestream timeseries object to intermidiate string
 * representation for further conversion.
 * e.g. timeseries[row(timestamp, T,...)] -> [{time: 2021-03-05 14:18:30.123456789, value: [1, 2, 3]}]
 * @param series An array of Timestream TimeSeriesDataPoint objects to be parsed
 * @param timeseries_value String representation of the datum to fill in recursively
 * @param column_attr_id Column attribute ID
 */
void ParseTimeSeries(const Aws::Vector< Aws::TimestreamQuery::Model::TimeSeriesDataPoint >  &series,
    std::string &timeseries_value, OID column_attr_id);

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

#define TS_VARCHAR_SIZE (-2)

const std::map< Aws::TimestreamQuery::Model::ScalarType,
                          std::pair< OID, int16_t > >
    scalar_type_to_oid_size_map = {
        {Aws::TimestreamQuery::Model::ScalarType::BIGINT,
         std::make_pair(TS_TYPE_BIGINT, (int16_t)8)},
        {Aws::TimestreamQuery::Model::ScalarType::BOOLEAN,
         std::make_pair(TS_TYPE_BOOLEAN, (int16_t)1)},
        {Aws::TimestreamQuery::Model::ScalarType::DATE,
         std::make_pair(TS_TYPE_DATE, (int16_t)6)},
        {Aws::TimestreamQuery::Model::ScalarType::DOUBLE,
         std::make_pair(TS_TYPE_DOUBLE, (int16_t)8)},
        {Aws::TimestreamQuery::Model::ScalarType::INTEGER,
         std::make_pair(TS_TYPE_INTEGER, (int16_t)4)},
        {Aws::TimestreamQuery::Model::ScalarType::INTERVAL_DAY_TO_SECOND,
         std::make_pair(TS_TYPE_VARCHAR, (int16_t)TS_VARCHAR_SIZE)},
        {Aws::TimestreamQuery::Model::ScalarType::INTERVAL_YEAR_TO_MONTH,
         std::make_pair(TS_TYPE_VARCHAR, (int16_t)TS_VARCHAR_SIZE)},
        {Aws::TimestreamQuery::Model::ScalarType::TIME,
         std::make_pair(TS_TYPE_TIME, (int16_t)6)},
        {Aws::TimestreamQuery::Model::ScalarType::TIMESTAMP,
         std::make_pair(TS_TYPE_TIMESTAMP, (int16_t)16)},
        {Aws::TimestreamQuery::Model::ScalarType::VARCHAR,
         std::make_pair(TS_TYPE_VARCHAR, (int16_t)TS_VARCHAR_SIZE)},
        {Aws::TimestreamQuery::Model::ScalarType::UNKNOWN,
         std::make_pair(TS_TYPE_VARCHAR, (int16_t)TS_VARCHAR_SIZE)},
};

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

BOOL CC_from_TSResult(
    QResultClass *q_res, ConnectionClass *conn, StatementClass *stmt,
    const char *next_token,
    const Aws::TimestreamQuery::Model::QueryOutcome &ts_result) {
    ClearError();
    return _CC_from_TSResult(q_res, conn, stmt, next_token, ts_result) ? TRUE
                                                                       : FALSE;
}

BOOL CC_Append_Table_Data(
    const Aws::TimestreamQuery::Model::QueryOutcome &ts_result,
    QResultClass *q_res, ColumnInfoClass &fields) {
    ClearError();
    return AssignTableData(ts_result, q_res, fields)
               ? TRUE
               : FALSE;
}

void print_log(const LogLevel &level, const std::string &s) {
#if WIN32
#pragma warning(push)
#pragma warning(disable : 4551)
#endif  // WIN32
        // cppcheck outputs an erroneous missing argument error which breaks
        // build. Disable for this function call
    MYLOG(level, "%s", s.c_str());
#if WIN32
#pragma warning(pop)
#endif  // WIN32
}

bool _CC_from_TSResult(
    QResultClass *q_res, ConnectionClass *conn, StatementClass *stmt,
    const char *next_token,
    const Aws::TimestreamQuery::Model::QueryOutcome &ts_result) {
    CSTR func = "_CC_from_TSResult";
    // Note - NULL conn and/or cursor is valid
    if (q_res == NULL)
        return false;

    QR_set_conn(q_res, conn);
    try {
        SQLULEN starting_cached_rows = q_res->num_cached_rows;

        // Assign table data and column headers
        if ((!AssignColumnHeaders(q_res, ts_result))
            || (!AssignTableData(ts_result, q_res, *(q_res->fields))))
            return false;

        // Update fields of QResult to reflect data written
        std::string command = "SELECT";
        UpdateResultFields(q_res, conn, starting_cached_rows, next_token,
                           command);

        // Return true (success)
        return true;
    } catch (const std::exception &e) {
        SC_set_error(stmt, STMT_EXEC_ERROR, e.what(), func);
    } catch (...) {
        SC_set_error(stmt, STMT_EXEC_ERROR, "Unknown exception thrown", func);
    }

    // Exception occurred, return false (error)
    return false;
}

bool AssignColumnHeaders(QResultClass *q_res,
    const Aws::TimestreamQuery::Model::QueryOutcome &outcome) {
    // Allocte memory for column fields
    const auto &column_info = outcome.GetResult().GetColumnInfo();
    QR_set_num_fields(q_res, (uint16_t)column_info.size());
    if (QR_get_fields(q_res)->coli_array == NULL)
        return false;
    
    for (size_t i = 0; i < column_info.size(); i++) {
        auto column = column_info[i];
        std::string column_name;
        if (column.NameHasBeenSet()) {
            column_name = column.GetName();
        }
        OID column_type_id = TS_TYPE_UNKNOWN;
        int16_t column_size = 0;
        if (column.TypeHasBeenSet()) {
            auto type = column.GetType();
            if (type.ScalarTypeHasBeenSet()) {
                auto oid_size_it =
                    scalar_type_to_oid_size_map.find(type.GetScalarType());
                if (oid_size_it == scalar_type_to_oid_size_map.end()) {
                    // NOT_SET and unsupported
                    throw std::runtime_error(
                        "Timestream scalar type is not set or unsupported "
                        "scalar type.");
                }
                column_type_id = oid_size_it->second.first;
                column_size = oid_size_it->second.second;
            } else if (type.ArrayColumnInfoHasBeenSet()) {
                column_type_id = TS_TYPE_VARCHAR;
                column_size = TS_VARCHAR_SIZE;
            } else if (type.RowColumnInfoHasBeenSet()) {
                column_type_id = TS_TYPE_VARCHAR;
                column_size = TS_VARCHAR_SIZE;
            } else if (type.TimeSeriesMeasureValueColumnInfoHasBeenSet()) {
                column_type_id = TS_TYPE_VARCHAR;
                column_size = TS_VARCHAR_SIZE;
            } else {
                throw std::runtime_error("Unsupported Timestream type.");
            }
        }
        // TODO Some historic fields needs to be removed, set 0 for now.
        CI_set_field_info(QR_get_fields(q_res), (int)i,
                          column.GetName().c_str(), column_type_id, column_size,
                          0, 0, 0);
        QR_set_rstatus(q_res, PORES_FIELDS_OK);
    }
    q_res->num_fields = CI_get_num_fields(QR_get_fields(q_res));

    return true;
}

bool AssignTableData(const Aws::TimestreamQuery::Model::QueryOutcome &outcome,
                     QResultClass *q_res, ColumnInfoClass &fields) {
    auto rows = outcome.GetResult().GetRows();
    auto col_size = outcome.GetResult().GetColumnInfo().size();
    for (const auto &row : rows) {
        // Setup memory to receive tuple
        if (!QR_prepare_for_tupledata(q_res))
            return false;

        // Assign row data
        if (!AssignRowData(row, q_res, fields, col_size))
            return false;
    }
    return true;
}

void ParseDatum(const Aws::TimestreamQuery::Model::Datum &datum,
                std::string &datum_value, OID column_attr_id) {
    if (datum.ScalarValueHasBeenSet()) {
        auto scalar_value = datum.GetScalarValue();
        if (column_attr_id == TS_TYPE_DOUBLE) {
            auto d = atof(scalar_value.c_str());
            scalar_value = std::to_string(d);
        }
        datum_value += scalar_value;
    } else if (datum.ArrayValueHasBeenSet()) {
        ParseArray(datum.GetArrayValue(), datum_value, column_attr_id);
    } else if (datum.RowValueHasBeenSet()) {
        if (datum.GetRowValue().DataHasBeenSet()) {
            ParseRow(datum.GetRowValue().GetData(), datum_value, column_attr_id);
        }
    } else if (datum.TimeSeriesValueHasBeenSet()) {
        ParseTimeSeries(datum.GetTimeSeriesValue(), datum_value, column_attr_id);
    } else if (datum.NullValueHasBeenSet()) {
        if (datum.GetNullValue()) {
            datum_value += "null";
        }
    } else {
        // Empty
        datum_value += "-";
    }
}

void ParseArray(const Aws::Vector< Aws::TimestreamQuery::Model::Datum > & datums, std::string& array_value, OID column_attr_id) {
    if (datums.size() == 0) {
        array_value += "-";
    } else {
        array_value += "[";
        for (auto &datum : datums) {
            ParseDatum(datum, array_value, column_attr_id);
            array_value += ", ";
        }
        // remove the last ',' & ' '
        if (array_value.size() > 1 && array_value[array_value.size() - 1] == ' '
            && array_value[array_value.size() - 2] == ',') {
            array_value.pop_back();
            array_value.pop_back();
        }
        array_value += "]";
    }
}

void ParseRow(const Aws::Vector< Aws::TimestreamQuery::Model::Datum > &datums,
              std::string &row_value, OID column_attr_id) {
    row_value += "(";
    for (auto &datum : datums) {
        ParseDatum(datum, row_value, column_attr_id);
        row_value += ", ";
    }
    // remove the last ',' & ' '
    if (row_value.size() > 1 && row_value[row_value.size() - 1] == ' '
        && row_value[row_value.size() - 2] == ',') {
        row_value.pop_back();
        row_value.pop_back();
    }
    row_value += ")";
}

void ParseTimeSeries(const Aws::Vector< Aws::TimestreamQuery::Model::TimeSeriesDataPoint > &series,
    std::string &timeseries_value, OID column_attr_id) {
    timeseries_value += "[";
    for (auto &s : series) {
        timeseries_value += "{";
        timeseries_value += "time: ";
        if (s.TimeHasBeenSet()) {
            timeseries_value += s.GetTime();
        }
        timeseries_value += ", value: ";
        if (s.ValueHasBeenSet()) {
            std::string value;
            ParseDatum(s.GetValue(), value, column_attr_id);
            timeseries_value += value;
        }
        timeseries_value += "}, ";
    }
    // remove the last ',' & ' '
    if (timeseries_value.size() > 1 && timeseries_value[timeseries_value.size() - 1] == ' '
        && timeseries_value[timeseries_value.size() - 2] == ',') {
        timeseries_value.pop_back();
        timeseries_value.pop_back();
    }
    timeseries_value += "]";
}

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
        if (row.DataHasBeenSet()) {
            auto datum = row.GetData()[i];
            if (datum.NullValueHasBeenSet()) {
                tuple[i].len = SQL_NULL_DATA;
                tuple[i].value = NULL;
            } else {
                std::string datum_value;
                ParseDatum(datum, datum_value, fields.coli_array[i].adtid);
                tuple[i].len = static_cast< int >(datum_value.length());
                QR_MALLOC_return_with_error(
                    tuple[i].value, char, tuple[i].len + 1, q_res,
                    "Out of memory in allocating item buffer.", false);
                strncpy((char *)tuple[i].value, datum_value.c_str(), datum_value.size() + 1);
                // If data length exceeds current display size, set display size
                if (fields.coli_array[i].display_size < tuple[i].len)
                    fields.coli_array[i].display_size = tuple[i].len;
            }
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
                        const SQLULEN starting_cached_rows, const char *next_token,
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
    QR_set_cursor(q_res, next_token);
    if (next_token == NULL)
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
