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

bool _CC_from_TSResult(QResultClass *q_res, ConnectionClass *conn, const char *next_token,
    const Aws::TimestreamQuery::Model::QueryOutcome &ts_result);
bool _CC_Metadata_from_TSResult(QResultClass *q_res, ConnectionClass *conn, const char *next_token,
    const Aws::TimestreamQuery::Model::QueryOutcome &ts_result);
bool _CC_No_Metadata_from_TSResult(QResultClass *q_res, ConnectionClass *conn, const char *next_token,
    const Aws::TimestreamQuery::Model::QueryOutcome &ts_result);
bool AssignColumnHeaders(QResultClass *q_res,
    const Aws::TimestreamQuery::Model::QueryOutcome &ts_result);
bool AssignTableData(const Aws::TimestreamQuery::Model::QueryOutcome &ts_result,
                     QResultClass *q_res, ColumnInfoClass &fields);
bool AssignRowData(const Aws::TimestreamQuery::Model::Row &row,
                   QResultClass *q_res, ColumnInfoClass &fields,
                   const size_t &row_size);
void UpdateResultFields(QResultClass *q_res, const ConnectionClass *conn,
                        const SQLULEN starting_cached_rows, const char *next_token,
                        std::string &command_type);
bool QR_prepare_for_tupledata(QResultClass *q_res);
void SetError(const char *err);
void ClearError();

/**
 * Parse datum
 * @param datum const Aws::TimestreamQuery::Model::Datum > &
 * @param datum_value std::string &
 * @param column_attr_id OID
 */
void ParseDatum(const Aws::TimestreamQuery::Model::Datum &datum,
                std::string &datum_value, OID column_attr_id);

/**
 * Parse array
 * @param datums const Aws::Vector< Aws::TimestreamQuery::Model::Datum > &
 * @param array_value std::string &
 * @param column_attr_id OID
 */
void ParseArray(const Aws::Vector< Aws::TimestreamQuery::Model::Datum > &datums,
                std::string &array_value, OID column_attr_id);
/**
 * Parse row
 * @param datums const Aws::Vector< Aws::TimestreamQuery::Model::Datum > &
 * @param row_value std::string &
 * @param column_attr_id OID
 */
void ParseRow(const Aws::Vector< Aws::TimestreamQuery::Model::Datum > &datums,
              std::string &row_value, OID column_attr_id);

/**
 * Parse time series
 * @param series const Aws::Vector< Aws::TimestreamQuery::Model::TimeSeriesDataPoint > &
 * @param timeseries_value std::string &
 * @param column_attr_id OID
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

// clang-format on
//const std::unordered_map< std::string, OID > type_to_oid_map = {
//    {ES_TYPE_NAME_BOOLEAN, TS_TYPE_BOOLEAN},
//    {ES_TYPE_NAME_BYTE, TS_TYPE_INT2},
//    {ES_TYPE_NAME_SHORT, TS_TYPE_INT2},
//    {TS_TYPE_NAME_INTEGER, TS_TYPE_INTEGER},
//    {TS_TYPE_NAME_BIGINT, TS_TYPE_BIGINT},
//    {ES_TYPE_NAME_HALF_FLOAT, ES_TYPE_FLOAT4},
//    {ES_TYPE_NAME_FLOAT, ES_TYPE_FLOAT4},
//    {TS_TYPE_NAME_DOUBLE, TS_TYPE_DOUBLE},
//    {ES_TYPE_NAME_SCALED_FLOAT, TS_TYPE_DOUBLE},
//    {ES_TYPE_NAME_KEYWORD, TS_TYPE_VARCHAR},
//    {ES_TYPE_NAME_TEXT, TS_TYPE_VARCHAR},
//    {ES_TYPE_NAME_DATE, TS_TYPE_TIMESTAMP},
//    {ES_TYPE_NAME_OBJECT, TS_TYPE_VARCHAR},
//    {TS_TYPE_NAME_VARCHAR, TS_TYPE_VARCHAR},
//    {ES_TYPE_NAME_DATE, TS_TYPE_DATE},
//
//    {TS_TYPE_NAME_VARCHAR, TS_TYPE_VARCHAR},
//    {TS_TYPE_NAME_BOOLEAN, TS_TYPE_BOOLEAN},
//    {TS_TYPE_NAME_BIGINT, TS_TYPE_BIGINT},
//    {TS_TYPE_NAME_DOUBLE, TS_TYPE_DOUBLE},
//    {TS_TYPE_NAME_TIMESTAMP, TS_TYPE_TIMESTAMP},
//    {TS_TYPE_NAME_DATE, TS_TYPE_DATE},
//    {TS_TYPE_NAME_TIME, TS_TYPE_TIME},
//    {TS_TYPE_NAME_INTERVAL_DAY_TO_SECOND, TS_TYPE_INTERVAL_DAY_TO_SECOND},
//    {TS_TYPE_NAME_INTERVAL_YEAR_TO_MONTH, TS_TYPE_INTERVAL_YEAR_TO_MONTH},
//    {TS_TYPE_NAME_UNKNOWN, TS_TYPE_UNKNOWN},
//    {TS_TYPE_NAME_INTEGER, TS_TYPE_INTEGER},
//    {TS_TYPE_NAME_ROW, TS_TYPE_ROW},
//    {TS_TYPE_NAME_ARRAY, TS_TYPE_ARRAY},
//    {TS_TYPE_NAME_TIMESERIES, TS_TYPE_TIMESERIES}};

#define TS_VARCHAR_SIZE (-2)
//#define TS_ROW_SIZE (-3)
//#define TS_ARRAY_SIZE (-4)
//#define TS_TIMESERIES_SIZE (-5)

//const std::unordered_map< OID, int16_t > oid_to_size_map = {
//    {TS_TYPE_BOOLEAN, (int16_t)1},
//    {TS_TYPE_INT2, (int16_t)2},
//    {TS_TYPE_INTEGER, (int16_t)4},
//    {TS_TYPE_BIGINT, (int16_t)8},
//    {ES_TYPE_FLOAT4, (int16_t)4},
//    {TS_TYPE_DOUBLE, (int16_t)8},
//    {TS_TYPE_VARCHAR, (int16_t)TS_VARCHAR_SIZE},
//    {TS_TYPE_DATE, (int16_t)TS_VARCHAR_SIZE},
//    {TS_TYPE_TIMESTAMP, (int16_t)1},
//
//    {TS_TYPE_VARCHAR, (int16_t)TS_VARCHAR_SIZE},
//    {TS_TYPE_BOOLEAN, (int16_t)1},
//    {TS_TYPE_BIGINT, (int16_t)8},
//    {TS_TYPE_DOUBLE, (int16_t)8},
//    {TS_TYPE_TIMESTAMP, (int16_t)1},
//    {TS_TYPE_DATE, (int16_t)TS_VARCHAR_SIZE},
//    {TS_TYPE_TIME, (int16_t)TS_VARCHAR_SIZE},
//    {TS_TYPE_INTERVAL_DAY_TO_SECOND, (int16_t)TS_VARCHAR_SIZE},
//    {TS_TYPE_INTERVAL_YEAR_TO_MONTH, (int16_t)TS_VARCHAR_SIZE},
//    {TS_TYPE_UNKNOWN, (int16_t)TS_VARCHAR_SIZE},
//    {TS_TYPE_INTEGER, (int16_t)4},
//    {TS_TYPE_ROW, (int16_t)TS_ROW_SIZE},
//    {TS_ARRAY_SIZE, (int16_t)TS_ARRAY_SIZE},
//    {TS_TIMESERIES_SIZE, (int16_t)TS_TIMESERIES_SIZE}};

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

BOOL CC_from_TSResult(QResultClass *q_res, ConnectionClass *conn,
                      const char *next_token, const Aws::TimestreamQuery::Model::QueryOutcome &ts_result) {
    ClearError();
    return _CC_from_TSResult(q_res, conn, next_token, ts_result) ? TRUE : FALSE;
}

BOOL CC_Metadata_from_TSResult(QResultClass *q_res, ConnectionClass *conn, const char *next_token,
    const Aws::TimestreamQuery::Model::QueryOutcome &ts_result) {
    ClearError();
    return _CC_Metadata_from_TSResult(q_res, conn, next_token, ts_result) ? TRUE : FALSE;
}

BOOL CC_No_Metadata_from_TSResult(QResultClass *q_res, ConnectionClass *conn, const char *next_token,
    const Aws::TimestreamQuery::Model::QueryOutcome &ts_result) {
    ClearError();
    return _CC_No_Metadata_from_TSResult(q_res, conn, next_token, ts_result)
               ? TRUE
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

bool _CC_No_Metadata_from_TSResult(QResultClass *q_res, ConnectionClass *conn, const char *next_token,
    const Aws::TimestreamQuery::Model::QueryOutcome &ts_result) {
    // Note - NULL conn and/or cursor is valid
    if (q_res == NULL)
        return false;

    try {

        SQLULEN starting_cached_rows = q_res->num_cached_rows;

        // Assign table data and column headers
        if (!AssignTableData(ts_result, q_res, *(q_res->fields)))
            return false;
        std::string command_type = "SELECT";
        // Update fields of QResult to reflect data written
        UpdateResultFields(q_res, conn, starting_cached_rows, next_token,
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
        SetError("Unknown exception thrown in _CC_No_Metadata_from_TSResult.");
    }

    // Exception occurred, return false (error)
    return false;
}

bool _CC_Metadata_from_TSResult(QResultClass *q_res, ConnectionClass *conn, const char *next_token,
    const Aws::TimestreamQuery::Model::QueryOutcome &ts_result) {
    // Note - NULL conn and/or cursor is valid
    if (q_res == NULL)
        return false;

    QR_set_conn(q_res, conn);
    try {
        // Assign table data and column headers
        if (!AssignColumnHeaders(q_res, ts_result))
            return false;

        // Set command type and cursor name
        std::string command = "SELECT";
        QR_set_command(q_res, command.c_str());
        QR_set_cursor(q_res, next_token);
        if (next_token == NULL)
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
        SetError("Unknown exception thrown in _CC_Metadata_from_TSResult.");
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
    MYLOG(LOG_ALL, "%s\n", s.c_str());
#if WIN32
#pragma warning(pop)
#endif  // WIN32
}

bool _CC_from_TSResult(QResultClass *q_res, ConnectionClass *conn, const char *next_token,
    const Aws::TimestreamQuery::Model::QueryOutcome &ts_result) {
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
    } catch (const rabbit::type_mismatch &e) {
        SetError(e.what());
    } catch (const rabbit::parse_error &e) {
        SetError(e.what());
    } catch (const std::exception &e) {
        SetError(e.what());
    } catch (...) {
        SetError("Unknown exception thrown in CC_from_TSResult.");
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
                switch (type.GetScalarType()) { 
                    case Aws::TimestreamQuery::Model::ScalarType::BIGINT:
                        column_type_id = TS_TYPE_BIGINT;
                        column_size = 8;
                        break;
                    case Aws::TimestreamQuery::Model::ScalarType::BOOLEAN:
                        column_type_id = TS_TYPE_BOOLEAN;
                        column_size = 1;
                        break;
                    case Aws::TimestreamQuery::Model::ScalarType::DATE:
                        column_type_id = TS_TYPE_DATE;
                        column_size = 6;
                        break;
                    case Aws::TimestreamQuery::Model::ScalarType::DOUBLE:
                        column_type_id = TS_TYPE_DOUBLE;
                        column_size = 8;
                        break;
                    case Aws::TimestreamQuery::Model::ScalarType::INTEGER:
                        column_type_id = TS_TYPE_INTEGER;
                        column_size = 4;
                        break;
                    case Aws::TimestreamQuery::Model::ScalarType::INTERVAL_DAY_TO_SECOND:
                        column_type_id = TS_TYPE_VARCHAR;
                        column_size = TS_VARCHAR_SIZE;
                        break;
                    case Aws::TimestreamQuery::Model::ScalarType::INTERVAL_YEAR_TO_MONTH:
                        column_type_id = TS_TYPE_VARCHAR;
                        column_size = TS_VARCHAR_SIZE;
                        break;
                    case Aws::TimestreamQuery::Model::ScalarType::TIME:
                        column_type_id = TS_TYPE_TIME;
                        column_size = 6;
                        break;
                    case Aws::TimestreamQuery::Model::ScalarType::TIMESTAMP:
                        column_type_id = TS_TYPE_TIMESTAMP;
                        column_size = 16;
                        break;
                    case Aws::TimestreamQuery::Model::ScalarType::VARCHAR:
                        column_type_id = TS_TYPE_VARCHAR;
                        column_size = TS_VARCHAR_SIZE;
                        break;
                    case Aws::TimestreamQuery::Model::ScalarType::UNKNOWN:
                        column_type_id = TS_TYPE_VARCHAR;
                        column_size = TS_VARCHAR_SIZE;
                        break;
                    case Aws::TimestreamQuery::Model::ScalarType::NOT_SET:
                    default:
                        // NOT_SET & default
                        break;
                }
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
                // Empty
            }
        }
        //// TODO Some historic fields needs to be removed, set 0 fow now.
        CI_set_field_info(QR_get_fields(q_res), (int)i,
                          column.GetName().c_str(), column_type_id, column_size,
                          0, 0, 0);
        QR_set_rstatus(q_res, PORES_FIELDS_OK);
    }
    q_res->num_fields = CI_get_num_fields(QR_get_fields(q_res));

    return true;
}

// Responsible for looping through rows, allocating tuples and passing rows for
// assignment
bool AssignTableData(const Aws::TimestreamQuery::Model::QueryOutcome &outcome,
                     QResultClass *q_res, ColumnInfoClass &fields) {
    auto rows = outcome.GetResult().GetRows();
    for (const auto& row : rows) {
        // Setup memory to receive tuple
        if (!QR_prepare_for_tupledata(q_res))
            return false;

        // Assign row data
        if (!AssignRowData(row, q_res, fields, outcome.GetResult().GetColumnInfo().size()))
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
                strcpy((char *)tuple[i].value, datum_value.c_str());
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
