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
#include "es_info.h"

#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// TODO #324 (SQL Plugin)- Update if Elasticsearch extends support for multiple
// tables
#define DEFAULT_TYPE_STR \
    { 'k', 'e', 'y', 'w', 'o', 'r', 'd', '\0' }
#define DEFAULT_TYPE_INT (SQL_WVARCHAR)
#define EMPTY_VARCHAR \
    { '\0' }
#define ES_UNINITIALIZED (-2)
#define COLUMN_TEMPLATE_COUNT 18
#define TABLE_TEMPLATE_COUNT 5

#define TABLE_CAT "TABLE_CAT"
#define TABLE_SCHEM "TABLE_SCHEM"
#define TABLE_NAME "TABLE_NAME"
#define COLUMN_NAME "COLUMN_NAME"
#define DATA_TYPE "DATA_TYPE"
#define TYPE_NAME "TYPE_NAME"
#define COLUMN_SIZE "COLUMN_SIZE"
#define BUFFER_LENGTH "BUFFER_LENGTH"
#define DECIMAL_DIGITS "DECIMAL_DIGITS"
#define NUM_PREC_RADIX "NUM_PREC_RADIX"
#define NULLABLE "NULLABLE"
#define REMARKS "REMARKS"
#define COLUMN_DEF "COLUMN_DEF"
#define SQL_DATA_TYPE "SQL_DATA_TYPE"
#define SQL_DATETIME_SUB "SQL_DATETIME_SUB"
#define CHAR_OCTET_LENGTH "CHAR_OCTET_LENGTH"
#define ORDINAL_POSITION "ORDINAL_POSITION"
#define IS_NULLABLE "IS_NULLABLE"
#define TABLE_QUALIFIER "TABLE_QUALIFIER"
#define TABLE_OWNER "TABLE_OWNER"
#define TABLE_TYPE "TABLE_TYPE"
#define PRECISION "PRECISION"
#define LITERAL_PREFIX "LITERAL_PREFIX"
#define LITERAL_SUFFIX "LITERAL_SUFFIX"
#define CREATE_PARAMS "CREATE_PARAMS"
#define CASE_SENSITIVE "CASE_SENSITIVE"
#define SEARCHABLE "SEARCHABLE"
#define UNSIGNED_ATTRIBUTE "UNSIGNED_ATTRIBUTE"
#define FIXED_PREC_SCALE "FIXED_PREC_SCALE"
#define AUTO_INCREMENT "AUTO_INCREMENT"
#define LOCAL_TYPE_NAME "LOCAL_TYPE_NAME"
#define MINIMUM_SCALE "MINIMUM_SCALE"
#define MAXIMUM_SCALE "MAXIMUM_SCALE"
#define INTERVAL_PRECISION "INTERVAL_PRECISION"

auto case_insensitive_compare = [](char &c1, char &c2) {
    return std::toupper(c1) == std::toupper(c2);
};

const std::unordered_map< int, std::vector< int > > sql_es_type_map = {
    {SQL_BIT, {TS_TYPE_BOOLEAN}},
    {SQL_TINYINT, {TS_TYPE_INTEGER}},
    {SQL_SMALLINT, {TS_TYPE_INTEGER}},
    {SQL_INTEGER, {TS_TYPE_INTEGER}},
    {SQL_BIGINT, {TS_TYPE_BIGINT}},
    {SQL_REAL, {ES_TYPE_HALF_FLOAT, ES_TYPE_FLOAT4}},
    {SQL_DOUBLE, {TS_TYPE_DOUBLE, ES_TYPE_SCALED_FLOAT}},
    {SQL_WVARCHAR,
     {ES_TYPE_KEYWORD, ES_TYPE_TEXT, ES_TYPE_NESTED, ES_TYPE_OBJECT}},
    {SQL_TYPE_TIMESTAMP, {ES_TYPE_DATETIME}}};

const std::unordered_map< std::string, int > data_name_data_type_map = {
    {TS_TYPE_NAME_DOUBLE, SQL_DOUBLE},
    {TS_TYPE_NAME_VARCHAR, SQL_VARCHAR},
    {TS_TYPE_NAME_TIMESTAMP, SQL_TYPE_TIMESTAMP},
    {TS_TYPE_NAME_BIGINT, SQL_BIGINT},
    {TS_TYPE_NAME_BOOLEAN, SQL_BIT}};

// Boilerplate code for easy column bind handling
class BindTemplate {
   public:
    BindTemplate(const bool can_be_null, const SQLUSMALLINT ordinal)
        : m_len(ES_UNINITIALIZED), m_ordinal(ordinal) {
        if (!can_be_null)
            throw std::runtime_error(
                "Do not use this constructor for values that can be NULL. A "
                "constructor with "
                "supplied default value must be used if value can be NULL.");
    }
    BindTemplate(const bool can_be_null, const SQLUSMALLINT ordinal, const Int2)
        : m_len(ES_UNINITIALIZED), m_ordinal(ordinal) {
        (void)(can_be_null);
    }
    BindTemplate(const bool can_be_null, const SQLUSMALLINT ordinal, const Int4)
        : m_len(ES_UNINITIALIZED), m_ordinal(ordinal) {
        (void)(can_be_null);
    }
    BindTemplate(const bool can_be_null, const SQLUSMALLINT ordinal,
                 const std::vector< SQLCHAR > &)
        : m_len(ES_UNINITIALIZED), m_ordinal(ordinal) {
        (void)(can_be_null);
    }
    virtual ~BindTemplate() {
    }

    SQLPOINTER GetData() {
        if (m_len == ES_UNINITIALIZED)
            throw std::runtime_error(
                "Length is uninitialized - Fetch must be executed before data "
                "is retreived.");
        return (m_len == SQL_NULL_DATA) ? NULL : GetDataForBind();
    }

    void BindColumn(StatementClass *stmt) {
        RETCODE err = API_BindCol(stmt, m_ordinal, GetType(),
                                    GetDataForBind(), GetSize(), &m_len);
        if (!SQL_SUCCEEDED(err)) {
            std::string error_msg =
                "Failed to bind column with ordinal "
                + std::to_string(m_ordinal)
                + ". SQL Error code: " + std::to_string(err);
            throw std::runtime_error(error_msg.c_str());
        }
    }
    void AssignData(TupleField *tuple) {
        SQLPOINTER data = GetData();
        if ((data == NULL) || (m_len == SQL_NULL_DATA)) {
            set_tuplefield_null(tuple);
            return;
        }
        switch (GetType()) {
            case SQL_C_LONG:
                set_tuplefield_int4(tuple, *static_cast< Int4 * >(data));
                break;
            case SQL_C_SHORT:
                set_tuplefield_int2(tuple, *static_cast< Int2 * >(data));
                break;
            case SQL_C_CHAR:
                set_tuplefield_string(tuple, static_cast< const char * >(data));
                break;
            default:
                throw std::runtime_error(
                    std::string(
                        "Cannot convert unknown data type to tuplefield: "
                        + std::to_string(GetType()))
                        .c_str());
        }
    }
    BindTemplate(const BindTemplate &) = default;
    BindTemplate &operator=(const BindTemplate &) = default;
    virtual std::string AsString() = 0;
    virtual void UpdateData(SQLPOINTER new_data, size_t size) = 0;

   private:
    SQLLEN m_len;
    SQLUSMALLINT m_ordinal;

   protected:
    virtual SQLSMALLINT GetType() = 0;
    virtual SQLLEN GetSize() = 0;
    virtual SQLPOINTER GetDataForBind() = 0;
};

// 4 byte integer column
class BindTemplateInt4 : public BindTemplate {
   public:
    BindTemplateInt4(const bool nullable, const SQLUSMALLINT ordinal)
        : BindTemplate(nullable, ordinal), m_data(0) {
    }
    BindTemplateInt4(const bool nullable, const SQLUSMALLINT ordinal,
                     const Int4 data)
        : BindTemplate(nullable, ordinal, data), m_data(data) {
    }
    ~BindTemplateInt4() {
    }
    std::string AsString() {
        return std::to_string(*static_cast< Int4 * >(GetData()));
    }
    void UpdateData(SQLPOINTER new_data, size_t size) {
        (void)size;
        m_data = *(Int4 *)new_data;
    }

   private:
    Int4 m_data;

   protected:
    SQLPOINTER GetDataForBind() {
        return &m_data;
    }
    SQLSMALLINT GetType() {
        return SQL_C_LONG;
    }
    SQLLEN GetSize() {
        return static_cast< SQLLEN >(sizeof(Int4));
    }
};

// 2 byte integer column
class BindTemplateInt2 : public BindTemplate {
   public:
    BindTemplateInt2(const bool nullable, const SQLUSMALLINT ordinal)
        : BindTemplate(nullable, ordinal), m_data(0) {
    }
    BindTemplateInt2(const bool nullable, const SQLUSMALLINT ordinal,
                     const Int2 data)
        : BindTemplate(nullable, ordinal, data), m_data(data) {
    }
    ~BindTemplateInt2() {
    }
    std::string AsString() {
        return std::to_string(*static_cast< Int2 * >(GetData()));
    }
    void UpdateData(SQLPOINTER new_data, size_t size) {
        (void)size;
        m_data = *(Int2 *)new_data;
    }

   private:
    Int2 m_data;

   protected:
    SQLPOINTER GetDataForBind() {
        return &m_data;
    }
    SQLSMALLINT GetType() {
        return SQL_C_SHORT;
    }
    SQLLEN GetSize() {
        return static_cast< SQLLEN >(sizeof(Int2));
    }
};

// Varchar data
class BindTemplateSQLCHAR : public BindTemplate {
   public:
    BindTemplateSQLCHAR(const bool nullable, const SQLUSMALLINT ordinal)
        : BindTemplate(nullable, ordinal), m_data(MAX_INFO_STRING, '\0') {
    }
    BindTemplateSQLCHAR(const bool nullable, const SQLUSMALLINT ordinal,
                        const std::vector< SQLCHAR > &data)
        : BindTemplate(nullable, ordinal, data), m_data(MAX_INFO_STRING, '\0') {
        if (data.size() >= m_data.size()) {
            throw std::runtime_error(
                "Default data size exceeds max info string size.");
        } else {
            m_data.insert(m_data.begin(), data.begin(), data.end());
        }
    }
    ~BindTemplateSQLCHAR() {
    }
    std::string AsString() {
        char *bind_tbl_data_char = static_cast< char * >(GetData());
        return (bind_tbl_data_char == NULL) ? "" : bind_tbl_data_char;
    }
    void UpdateData(SQLPOINTER new_data, size_t size) {
        m_data.clear();
        SQLCHAR *data = reinterpret_cast< SQLCHAR * >(new_data);
        for (size_t i = 0; i < size; i++) {
            m_data.push_back(*data++);
        }
        m_data.push_back(0);
    }

   private:
    std::vector< SQLCHAR > m_data;

   protected:
    SQLPOINTER GetDataForBind() {
        return m_data.data();
    }
    SQLSMALLINT GetType() {
        return SQL_C_CHAR;
    }
    SQLLEN GetSize() {
        return static_cast< SQLLEN >(m_data.size());
    }
};

// Typedefs and macros to ease creation of BindTemplates
typedef std::unique_ptr< BindTemplate > bind_ptr;
typedef std::vector< bind_ptr > bind_vector;
#define _SQLCHAR_(...) \
    (std::make_unique< BindTemplateSQLCHAR >(BindTemplateSQLCHAR(__VA_ARGS__)))
#define _SQLINT2_(...) \
    (std::make_unique< BindTemplateInt2 >(BindTemplateInt2(__VA_ARGS__)))
#define _SQLINT4_(...) \
    (std::make_unique< BindTemplateInt4 >(BindTemplateInt4(__VA_ARGS__)))

// Common function declarations
enum class TableResultSet { Catalog, Schema, TableTypes, TableLookUp, All };
void ConvertToString(std::string &out, bool &valid, const SQLCHAR *sql_char,
                     const SQLSMALLINT sz);
QResultClass *SetupQResult(StatementClass *stmt, const int col_cnt);
void CleanUp(StatementClass *stmt, StatementClass *sub_stmt, const RETCODE ret);
void ExecuteQuery(ConnectionClass *conn, HSTMT *stmt, const std::string &query);
std::regex ConvertPattern(const std::string &pattern);

// Common function definitions
void ConvertToString(std::string &out, bool &valid, const SQLCHAR *sql_char,
                     const SQLSMALLINT sz) {
    valid = (sql_char != NULL);
    if (!valid) {
        out = "%";
    } else if (sz == SQL_NTS) {
        out.assign(reinterpret_cast< const char * >(sql_char));
    } else if (sz <= 0) {
        out = "";
    } else {
        out.assign(reinterpret_cast< const char * >(sql_char),
                   static_cast< size_t >(sz));
    }
}

QResultClass *SetupQResult(StatementClass *stmt, const int col_cnt) {
    // Initialize memory for data retreival
    QResultClass *res = NULL;
    if ((res = QR_Constructor()) == NULL) {
        SC_set_error(stmt, STMT_NO_MEMORY_ERROR,
                     "Couldn't allocate memory for Tables or Columns result.",
                     "FetchResults");
        throw std::runtime_error(
            "Couldn't allocate memory for Tables or Columns result.");
    }
    SC_set_Result(stmt, res);

    // The binding structure for a statement is not set up until a statement is
    // actually executed, so we'll have to do this ourselves
    extend_column_bindings(SC_get_ARDF(stmt),
                           static_cast< SQLSMALLINT >(col_cnt));
    QR_set_num_fields(res, col_cnt);

    return res;
}

std::regex ConvertPattern(const std::string &sql_pattern) {
    std::string regex_pattern;
    for (std::string::size_type i = 0; i < sql_pattern.size(); i++) {
        if (i == 0) {
            regex_pattern.push_back('^');
        }
        switch (sql_pattern[i]) {
            case '_': {
                regex_pattern += ".";
                break;
            }
            case '%': {
                regex_pattern += ".*";
                break;
            }
            case '[':
            case ']':
            case '(':
            case ')':
            case '\\': {
                regex_pattern += "\\";
                [[fallthrough]];
            }
            default: {
                regex_pattern.push_back(sql_pattern.at(i));
                break;
            }
        }
        if (i == sql_pattern.size() - 1) {
            regex_pattern.push_back('$');
        }
    }
    return std::regex(regex_pattern);
}

void CleanUp(StatementClass *stmt, StatementClass *sub_stmt,
             const RETCODE ret = SQL_ERROR) {
    stmt->status = STMT_FINISHED;
    stmt->catalog_result = TRUE;

    if (!SQL_SUCCEEDED(ret) && 0 >= SC_get_errornumber(stmt))
        SC_error_copy(stmt, sub_stmt, TRUE);

    // set up the current tuple pointer for
    stmt->currTuple = -1;
    SC_set_rowset_start(stmt, -1, FALSE);
    SC_set_current_col(stmt, -1);

    if (sub_stmt)
        ESAPI_FreeStmt(sub_stmt, SQL_DROP);
}

void ExecuteQuery(ConnectionClass *conn, HSTMT *stmt,
                  const std::string &query) {
    // Prepare statement
    if (!SQL_SUCCEEDED(API_AllocStmt(conn, stmt, 0))) {
        throw std::runtime_error("Failed to allocate memory for statement.");
    }

    // Execute query
    if (!SQL_SUCCEEDED(API_ExecDirect(
            *stmt, reinterpret_cast< const SQLCHAR * >(query.c_str()), SQL_NTS,
            1))) {
        std::string error_msg = "Failed to execute query '" + query + "'.";
        throw std::runtime_error(error_msg.c_str());
    }
}

// Table specific function definitions
void split(const std::string &input, const std::string &delim,
           std::vector< std::string > &output);
void SetupTableQResInfo(QResultClass *res, EnvironmentClass *env);
void SetTableTuples(QResultClass *res, const TableResultSet res_type,
                    const bind_vector &bind_tbl, std::string &table_type,
                    StatementClass *stmt, StatementClass *tbl_stmt,
                    std::vector< std::string > *list_of_columns = NULL);

// Table specific function declarations
void split(const std::string &input, const std::string &delim,
           std::vector< std::string > &output) {
    size_t start = 0;
    size_t end = input.find(delim);
    while (end != std::string::npos) {
        output.push_back(input.substr(start, end - start));
        start = end + delim.length();
        end = input.find(delim, start);
    }
    output.push_back(input.substr(start, end));
}

void SetupTableQResInfo(QResultClass *res, EnvironmentClass *env) {
    if (EN_is_odbc3(env)) {
        QR_set_field_info_v(res, TABLES_CATALOG_NAME, TABLE_CAT,
                            TS_TYPE_VARCHAR, MAX_INFO_STRING);
        QR_set_field_info_v(res, TABLES_SCHEMA_NAME, TABLE_SCHEM,
                            TS_TYPE_VARCHAR, MAX_INFO_STRING);
    } else {
        QR_set_field_info_v(res, TABLES_CATALOG_NAME, TABLE_QUALIFIER,
                            TS_TYPE_VARCHAR, MAX_INFO_STRING);
        QR_set_field_info_v(res, TABLES_SCHEMA_NAME, TABLE_OWNER,
                            TS_TYPE_VARCHAR, MAX_INFO_STRING);
    }
    QR_set_field_info_v(res, TABLES_TABLE_NAME, TABLE_NAME, TS_TYPE_VARCHAR,
                        MAX_INFO_STRING);
    QR_set_field_info_v(res, TABLES_TABLE_TYPE, TABLE_TYPE, TS_TYPE_VARCHAR,
                        MAX_INFO_STRING);
    QR_set_field_info_v(res, TABLES_REMARKS, REMARKS, TS_TYPE_VARCHAR,
                        INFO_VARCHAR_SIZE);
}

void SetTableTuples(QResultClass *res, const TableResultSet res_type,
                    const bind_vector &bind_tbl, std::string &table_type,
                    StatementClass *stmt, StatementClass *tbl_stmt,
                    std::vector< std::string > *list_of_columns) {
    auto CheckResult = [&](const auto &res) {
        if (res != SQL_NO_DATA_FOUND) {
            SC_full_error_copy(stmt, tbl_stmt, FALSE);
            throw std::runtime_error(
                std::string("Failed to fetch data after query. Error code :"
                            + std::to_string(res))
                    .c_str());
        }
    };
    auto AssignData = [&](auto *res, const auto &binds) {
        TupleField *tuple = QR_AddNew(res);
        // Since we do not support catalogs, we will return an empty string for
        // catalog names. This is required for Excel for Mac, which uses this
        // information for its Data Preview window.
        std::string catalog("");
        bind_tbl[TABLES_CATALOG_NAME]->UpdateData((void *)catalog.c_str(), 0);

        // TODO #630 - Revisit logic of adding tuples for SQLTables & SQLColumns
        for (size_t i = 0; i < binds.size(); i++) {
            // Add tuples for SQLColumns
            if (binds.size() > COLUMNS_SQL_DATA_TYPE) {
                // Add data type for data loading issue in Power BI Desktop
                auto data_type = data_name_data_type_map
                        .find(bind_tbl[COLUMNS_TYPE_NAME]->AsString())->second;
                if (i == COLUMNS_DATA_TYPE) {
                    set_tuplefield_int2(&tuple[COLUMNS_DATA_TYPE],
                                        static_cast< short >(data_type));
                } else if (i == COLUMNS_SQL_DATA_TYPE) {
                    set_tuplefield_int2(&tuple[COLUMNS_SQL_DATA_TYPE],
                                        static_cast< short >(data_type));
                } else {
                    binds[i]->AssignData(&tuple[i]);
                }
            }
            // Add tuples for SQLTables
            else {
                binds[i]->AssignData(&tuple[i]);
            }
        }
    };

    // General case
    if (res_type == TableResultSet::All) {
        RETCODE result = SQL_NO_DATA_FOUND;
        int ordinal_position = 0;
        while (SQL_SUCCEEDED(result = ESAPI_Fetch(tbl_stmt))) {
            if (bind_tbl[TABLES_TABLE_TYPE]->AsString() == "BASE TABLE") {
                std::string table("TABLE");
                bind_tbl[TABLES_TABLE_TYPE]->UpdateData((void *)table.c_str(),
                                                        table.length());
            }
            if (list_of_columns != NULL && !list_of_columns->empty()) {
                if (std::find(list_of_columns->begin(), list_of_columns->end(),
                              bind_tbl[COLUMNS_COLUMN_NAME]->AsString())
                    != list_of_columns->end()) {
                    ordinal_position++;
                    bind_tbl[COLUMNS_ORDINAL_POSITION]->UpdateData(
                        &ordinal_position, 0);
                    AssignData(res, bind_tbl);
                }
            } else {
                AssignData(res, bind_tbl);
            }
        }
        CheckResult(result);
    } else if (res_type == TableResultSet::TableLookUp) {
        // Get accepted table types
        std::vector< std::string > table_types;
        table_type.erase(
            std::remove(table_type.begin(), table_type.end(), '\''),
            table_type.end());
        split(table_type, ",", table_types);

        // Loop through all data
        RETCODE result = SQL_NO_DATA_FOUND;
        while (SQL_SUCCEEDED(result = ESAPI_Fetch(tbl_stmt))) {
            // Replace BASE TABLE with TABLE for Excel & Power BI SQLTables call
            if (bind_tbl[TABLES_TABLE_TYPE]->AsString() == "BASE TABLE") {
                std::string table("TABLE");
                bind_tbl[TABLES_TABLE_TYPE]->UpdateData((void *)table.c_str(),
                                                        table.length());
            }
            if (std::find(table_types.begin(), table_types.end(),
                          bind_tbl[TABLES_TABLE_TYPE]->AsString())
                != table_types.end()) {
                AssignData(res, bind_tbl);
            }
        }

        CheckResult(result);

    }
    // Special cases - only need single grab for this one
    else {
        RETCODE result;
        if (!SQL_SUCCEEDED(result = ESAPI_Fetch(tbl_stmt))) {
            SC_full_error_copy(stmt, tbl_stmt, FALSE);
            throw std::runtime_error(
                std::string("Failed to fetch data after query. Error code :"
                            + std::to_string(result))
                    .c_str());
        }

        // Get index of result type of interest
        size_t idx = NUM_OF_TABLES_FIELDS;
        switch (res_type) {
            case TableResultSet::TableTypes:
                idx = TABLES_TABLE_TYPE;
                break;
            default:
                // This should not be possible, handle it anyway
                throw std::runtime_error(
                    "Result type is not an expected type.");
        }

        // Get new tuple and assign index of interest (NULL others)
        // TODO #324 (SQL Plugin)- Should these be unique?
        TupleField *tuple = QR_AddNew(res);
        for (size_t i = 0; i < bind_tbl.size(); i++) {
            if (i == idx)
                bind_tbl[i]->AssignData(&tuple[i]);
            else
                set_tuplefield_string(&tuple[i], NULL_STRING);
        }
    }
}

// Column specific function definitions
void SetupColumnQResInfo(QResultClass *res, EnvironmentClass *unused);
void GenerateColumnQuery(std::string &query, const std::string &table_name,
                         const std::string &column_name, const bool table_valid,
                         const bool column_valid, const UWORD flag);
int GetColumnSize(const std::string &type_name);
int GetBufferLength(const std::string &type_name);

// Column Specific function declarations
void SetupColumnQResInfo(QResultClass *res, EnvironmentClass *unused) {
    (void)(unused);

    QR_set_field_info_v(res, COLUMNS_CATALOG_NAME, TABLE_CAT, TS_TYPE_VARCHAR,
                        MAX_INFO_STRING);
    QR_set_field_info_v(res, COLUMNS_SCHEMA_NAME, TABLE_SCHEM, TS_TYPE_VARCHAR,
                        MAX_INFO_STRING);
    QR_set_field_info_v(res, COLUMNS_TABLE_NAME, TABLE_NAME, TS_TYPE_VARCHAR,
                        MAX_INFO_STRING);
    QR_set_field_info_v(res, COLUMNS_COLUMN_NAME, COLUMN_NAME, TS_TYPE_VARCHAR,
                        MAX_INFO_STRING);
    QR_set_field_info_v(res, COLUMNS_DATA_TYPE, DATA_TYPE, TS_TYPE_INT2, 2);
    QR_set_field_info_v(res, COLUMNS_TYPE_NAME, TYPE_NAME, TS_TYPE_VARCHAR,
                        MAX_INFO_STRING);
    QR_set_field_info_v(res, COLUMNS_PRECISION, COLUMN_SIZE, TS_TYPE_INTEGER, 4);
    QR_set_field_info_v(res, COLUMNS_LENGTH, BUFFER_LENGTH, TS_TYPE_INTEGER, 4);
    QR_set_field_info_v(res, COLUMNS_SCALE, DECIMAL_DIGITS, TS_TYPE_INT2, 2);
    QR_set_field_info_v(res, COLUMNS_RADIX, NUM_PREC_RADIX, TS_TYPE_INT2, 2);
    QR_set_field_info_v(res, COLUMNS_NULLABLE, NULLABLE, TS_TYPE_INT2, 2);
    QR_set_field_info_v(res, COLUMNS_REMARKS, REMARKS, TS_TYPE_VARCHAR,
                        INFO_VARCHAR_SIZE);
    QR_set_field_info_v(res, COLUMNS_COLUMN_DEF, COLUMN_DEF, TS_TYPE_VARCHAR,
                        INFO_VARCHAR_SIZE);
    QR_set_field_info_v(res, COLUMNS_SQL_DATA_TYPE, SQL_DATA_TYPE, TS_TYPE_INT2,
                        2);
    QR_set_field_info_v(res, COLUMNS_SQL_DATETIME_SUB, SQL_DATETIME_SUB,
                        TS_TYPE_INT2, 2);
    QR_set_field_info_v(res, COLUMNS_CHAR_OCTET_LENGTH, CHAR_OCTET_LENGTH,
                        TS_TYPE_INTEGER, 4);
    QR_set_field_info_v(res, COLUMNS_ORDINAL_POSITION, ORDINAL_POSITION,
                        TS_TYPE_INTEGER, 4);
    QR_set_field_info_v(res, COLUMNS_IS_NULLABLE, IS_NULLABLE, TS_TYPE_VARCHAR,
                        INFO_VARCHAR_SIZE);
}

// TODO #325 (SQL Plugin)- Fix patterns and escape characters for this
void GenerateColumnQuery(std::string &query, const std::string &table_name,
                         const std::string &column_name, const bool table_valid,
                         const bool column_valid, const UWORD flag) {
    bool search_pattern = (~flag & PODBC_NOT_SEARCH_PATTERN);
    query = "DESCRIBE TABLES LIKE ";
    query += table_valid
                 ? (search_pattern ? table_name : "^" + table_name + "$")
                 : "%";
    if (column_valid)
        query += " COLUMNS LIKE " + column_name;
}

int GetColumnSize(const std::string &type_name) {
    switch (data_name_data_type_map.find(type_name)->second) {
        case SQL_VARCHAR:
            return INT_MAX;
        case SQL_DOUBLE:
            return 15;
        case SQL_TYPE_TIMESTAMP:
            return 29;
        case SQL_BIGINT:
            return 20;
        case SQL_BIT:
            return 5;
        default:
            return 0;
    }
}

int GetBufferLength(const std::string &type_name) {
    switch (data_name_data_type_map.find(type_name)->second) {
        case SQL_VARCHAR:
            return 256;
        case SQL_DOUBLE:
            return sizeof(SQLDOUBLE);
        case SQL_TYPE_TIMESTAMP:
            return sizeof(TIMESTAMP_STRUCT);
        case SQL_BIGINT:
            return sizeof(SQLBIGINT);
        case SQL_BIT:
            return 1;
        default:
            return 0;
    }
}

RETCODE SQL_API
API_Tables(HSTMT hstmt, const SQLCHAR *catalog_name_sql,
             const SQLSMALLINT catalog_name_sz, const SQLCHAR *schema_name_sql,
             const SQLSMALLINT schema_name_sz, const SQLCHAR *table_name_sql,
             const SQLSMALLINT table_name_sz, const SQLCHAR *table_type_sql,
             const SQLSMALLINT table_type_sz, const UWORD flag) {
    CSTR func = "API_Tables";
    StatementClass *stmt = (StatementClass *)hstmt;
    bool is_search_pattern = (~flag & PODBC_NOT_SEARCH_PATTERN);
    if (!is_search_pattern
        && (catalog_name_sql == nullptr || schema_name_sql == nullptr
            || table_name_sql == nullptr)) {
        SC_set_error(stmt, STMT_INVALID_NULL_ARG, "Invalid use of null pointer", func);
        return SQL_ERROR;
    }
    RETCODE result = SQL_ERROR;
    if ((result = SC_initialize_and_recycle(stmt)) != SQL_SUCCESS)
        return result;

    try {
        // Convert const SQLCHAR*'s to c++ strings
        std::string catalog_name, schema_name, table_name, table_type;
        bool catalog_valid, schema_valid, table_valid, table_type_valid;
        ConvertToString(catalog_name, catalog_valid, catalog_name_sql,
                        catalog_name_sz);
        ConvertToString(schema_name, schema_valid, schema_name_sql,
                        schema_name_sz);
        ConvertToString(table_name, table_valid, table_name_sql, table_name_sz);
        ConvertToString(table_type, table_type_valid, table_type_sql,
                        table_type_sz);

        // Setup QResultClass
        QResultClass *res = SetupQResult(stmt, TABLE_TEMPLATE_COUNT);
        SetupTableQResInfo(res, static_cast< EnvironmentClass * >(
                                    CC_get_env(SC_get_conn(stmt))));

        if (catalog_name == SQL_ALL_CATALOGS && schema_name.empty()
            && table_name.empty() && table_type.empty()) {
            /**
             * If CatalogName is SQL_ALL_CATALOGS and SchemaName and TableName
             * are empty strings, the result set contains a list of valid
             * catalogs for the data source. (All columns except the TABLE_CAT
             * column contain NULLs.)
             */
            std::string query = "SHOW DATABASES";
            if (is_search_pattern && !catalog_name.empty()) {
                query += " LIKE \'" + catalog_name + "\'";
            }
            StatementClass *database_stmt = NULL;
            // Execute query
            ExecuteQuery(SC_get_conn(stmt),
                         reinterpret_cast< HSTMT * >(&database_stmt), query);
            RETCODE ret = SQL_NO_DATA;
            while ((ret = ESAPI_Fetch(database_stmt)) != SQL_NO_DATA
                   && SQL_SUCCEEDED(ret)) {
                SQLCHAR data[256] = {0};
                SQLLEN indicator = 0;
                ret = ESAPI_GetData(database_stmt, 1, SQL_C_CHAR, data, 256,
                                    &indicator);
                if (SQL_SUCCEEDED(ret)) {
                    std::string database_name;
                    database_name.assign(reinterpret_cast< const char * >(data),
                                         static_cast< size_t >(indicator));
                    if (is_search_pattern
                        || std::equal(database_name.begin(),
                                      database_name.end(), catalog_name.begin(),
                                      case_insensitive_compare)) {
                        TupleField *tuple = QR_AddNew(res);
                        tuple[TABLES_CATALOG_NAME].value =
                            strdup(database_name.c_str());
                        tuple[TABLES_CATALOG_NAME].len =
                            (int)database_name.size();
                        tuple[TABLES_SCHEMA_NAME].value = NULL;
                        tuple[TABLES_SCHEMA_NAME].len = 0;
                        tuple[TABLES_TABLE_NAME].value = NULL;
                        tuple[TABLES_TABLE_NAME].len = 0;
                        tuple[TABLES_TABLE_TYPE].value = NULL;
                        tuple[TABLES_TABLE_TYPE].len = 0;
                        tuple[TABLES_REMARKS].value = NULL;
                        tuple[TABLES_REMARKS].len = 0;
                    }
                }
            }
            CleanUp(stmt, database_stmt, SQL_SUCCESS);
        } else if (schema_valid && !schema_name.empty() && catalog_name.empty()
                   && table_name.empty()) {
            /**
             * If SchemaName is SQL_ALL_SCHEMAS and CatalogName and TableName
             * are empty strings, the result set contains a list of valid
             * schemas for the data source. (All columns except the TABLE_SCHEM
             * column contain NULLs.) Note: We don't have schema in Amazon
             * Timestream, empty list returns
             */
            CleanUp(stmt, nullptr, SQL_SUCCESS);
        } else if ( catalog_valid && catalog_name.empty() && 
                    schema_valid && schema_name.empty() && 
                    table_valid && table_name.empty() && 
                    table_type == SQL_ALL_TABLE_TYPES) {
            /**
             * If TableType is SQL_ALL_TABLE_TYPES and CatalogName, SchemaName, and TableName
             * are empty strings, the result set contains a list of valid table types for the 
             * data source. (All columns except the TABLE_TYPE column contain NULLs.)
             */
            TupleField *tuple = QR_AddNew(res);
            tuple[TABLES_CATALOG_NAME].value = NULL;
            tuple[TABLES_CATALOG_NAME].len = 0;
            tuple[TABLES_SCHEMA_NAME].value = NULL;
            tuple[TABLES_SCHEMA_NAME].len = 0;
            tuple[TABLES_TABLE_NAME].value = NULL;
            tuple[TABLES_TABLE_NAME].len = 0;
            std::string table_type_return = "TABLE";
            tuple[TABLES_TABLE_TYPE].value = strdup(table_type_return.c_str());
            tuple[TABLES_TABLE_TYPE].len = (int)table_type_return.size();
            tuple[TABLES_REMARKS].value = NULL;
            tuple[TABLES_REMARKS].len = 0;
            CleanUp(stmt, nullptr, SQL_SUCCESS);
        } else {
            // Parse table types
            bool table_type_filter = true;
            if (table_type_valid && !table_type.empty()) {
                if (table_type == SQL_ALL_TABLE_TYPES) {
                    table_type_filter = false;
                } else {
                    table_type.erase(
                        std::remove(table_type.begin(), table_type.end(), '\''),
                        table_type.end());
                    std::vector< std::string > table_types;
                    split(table_type, ",", table_types);
                    for (auto &t : table_types) {
                        std::transform(
                            t.begin(), t.end(), t.begin(),
                            [](char c) { return (char)std::toupper(c); });
                        if (t == "TABLE") {
                            // We support "TABLE" only
                            table_type_filter = false;
                        }
                    }
                }
            } else {
                // No filter
                table_type_filter = false;
            }
            // Get all databases
            std::string database_query = "SHOW DATABASES";
            if (is_search_pattern && !catalog_name.empty()) {
                database_query += " LIKE \'" + catalog_name + "\'";
            }
            StatementClass *database_stmt = NULL;
            ExecuteQuery(SC_get_conn(stmt),
                         reinterpret_cast< HSTMT * >(&database_stmt),
                         database_query);
            RETCODE ret = SQL_NO_DATA;
            std::vector< std::string > databases;
            while ((ret = ESAPI_Fetch(database_stmt)) != SQL_NO_DATA
                   && SQL_SUCCEEDED(ret)) {
                SQLCHAR data[256] = {0};
                SQLLEN indicator = 0;
                ret = ESAPI_GetData(database_stmt, 1, SQL_C_CHAR, data, 256,
                                    &indicator);
                std::string database_name;
                if (SQL_SUCCEEDED(ret)) {
                    database_name.assign(reinterpret_cast< const char * >(data),
                                         static_cast< size_t >(indicator));
                    if (is_search_pattern
                        || std::equal(database_name.begin(),
                                      database_name.end(), catalog_name.begin(),
                                      case_insensitive_compare)) {
                        databases.push_back(database_name);
                    }
                }
            }
            ESAPI_FreeStmt(database_stmt, SQL_DROP);
            // Get all tables per database
            for (auto database : databases) {
                std::string table_query =
                    "SHOW TABLES FROM \"" + database + "\"";
                if (is_search_pattern && !table_name.empty()) {
                    table_query += " LIKE \'" + table_name + "\'";
                }
                StatementClass *table_stmt = NULL;
                ExecuteQuery(SC_get_conn(stmt),
                             reinterpret_cast< HSTMT * >(&table_stmt),
                             table_query);
                while ((ret = ESAPI_Fetch(table_stmt)) != SQL_NO_DATA
                       && SQL_SUCCEEDED(ret)) {
                    SQLCHAR data[256] = {0};
                    SQLLEN indicator = 0;
                    ret = ESAPI_GetData(table_stmt, 1, SQL_C_CHAR, data, 256,
                                        &indicator);
                    std::string table_name_return;
                    if (SQL_SUCCEEDED(ret)) {
                        table_name_return.assign(
                            reinterpret_cast< const char * >(data),
                            static_cast< size_t >(indicator));
                        if (!table_type_filter) {
                            if (is_search_pattern
                                || std::equal(table_name_return.begin(),
                                              table_name_return.end(),
                                              table_name.begin(),
                                              case_insensitive_compare)) {
                                TupleField *tuple = QR_AddNew(res);
                                tuple[TABLES_CATALOG_NAME].value =
                                    strdup(database.c_str());
                                tuple[TABLES_CATALOG_NAME].len =
                                    (int)database.size();
                                tuple[TABLES_SCHEMA_NAME].value = NULL;
                                tuple[TABLES_SCHEMA_NAME].len = 0;
                                tuple[TABLES_TABLE_NAME].value =
                                    strdup(table_name_return.c_str());
                                tuple[TABLES_TABLE_NAME].len =
                                    (int)table_name_return.size();
                                std::string table_type_return = "TABLE";
                                tuple[TABLES_TABLE_TYPE].value =
                                    strdup(table_type_return.c_str());
                                tuple[TABLES_TABLE_TYPE].len =
                                    (int)table_type_return.size();
                                tuple[TABLES_REMARKS].value = NULL;
                                tuple[TABLES_REMARKS].len = 0;
                            }
                        }
                    }
                }
                ESAPI_FreeStmt(table_stmt, SQL_DROP);
            }
            CleanUp(stmt, nullptr, SQL_SUCCESS);
        }
        return SQL_SUCCESS;
    } catch (std::bad_alloc &e) {
        std::string error_msg = std::string("Bad allocation exception: '")
                                + e.what() + std::string("'.");
        SC_set_error(stmt, STMT_NO_MEMORY_ERROR, error_msg.c_str(), func);
    } catch (std::exception &e) {
        std::string error_msg =
            std::string("Generic exception: '") + e.what() + std::string("'.");
        SC_set_error(stmt, STMT_INTERNAL_ERROR, error_msg.c_str(), func);
    } catch (...) {
        std::string error_msg = std::string("Unknown exception raised.");
        SC_set_error(stmt, STMT_INTERNAL_ERROR, error_msg.c_str(), func);
    }
    CleanUp(stmt, nullptr);
    return SQL_ERROR;
}

RETCODE SQL_API
API_Columns(HSTMT hstmt, const SQLCHAR *catalog_name_sql,
              const SQLSMALLINT catalog_name_sz, const SQLCHAR *schema_name_sql,
              const SQLSMALLINT schema_name_sz, const SQLCHAR *table_name_sql,
              const SQLSMALLINT table_name_sz, const SQLCHAR *column_name_sql,
              const SQLSMALLINT column_name_sz, const UWORD flag,
              const OID reloid, const Int2 attnum) {
    (void)(reloid);
    (void)(attnum);

    CSTR func = "API_Columns";

    // Declare outside of try so we can clean them up properly if an exception
    // occurs
    StatementClass *stmt = (StatementClass *)hstmt;
    StatementClass *table_stmt = NULL;

    bool is_search_pattern = (~flag & PODBC_NOT_SEARCH_PATTERN);
    if (!is_search_pattern
        && (catalog_name_sql == nullptr || schema_name_sql == nullptr
            || table_name_sql == nullptr)) {
        SC_set_error(stmt, STMT_INVALID_NULL_ARG, "Invalid use of null pointer",
                     func);
        return SQL_ERROR;
    }
    
    RETCODE result = SQL_ERROR;
    if ((result = SC_initialize_and_recycle(stmt)) != SQL_SUCCESS)
        return result;

    try {
        // Prepare statement
        if (!SQL_SUCCEEDED(
                API_AllocStmt(SC_get_conn(stmt),
                                reinterpret_cast< HSTMT * >(&table_stmt), 0))) {
            throw std::runtime_error(
                "Failed to allocate memory for statement.");
        }

        RETCODE ret;
        ret = API_Tables(table_stmt, catalog_name_sql, catalog_name_sz,
                         schema_name_sql, schema_name_sz, table_name_sql,
                         table_name_sz, (SQLCHAR *)SQL_ALL_TABLE_TYPES, SQL_NTS,
                         flag);
        if (!SQL_SUCCEEDED(ret)) {
            SC_set_error(stmt, SC_get_errornumber(table_stmt),
                         SC_get_errormsg(table_stmt), func);
            ESAPI_FreeStmt(table_stmt, SQL_DROP);
            CleanUp(stmt, nullptr);
            return ret;
        }

        std::vector< std::pair< std::string, std::string > > tables;
        while ((ret = ESAPI_Fetch(table_stmt)) != SQL_NO_DATA
               && SQL_SUCCEEDED(ret)) {
            // Get database name
            SQLCHAR database[256] = {0};
            SQLLEN db_strlen_or_ind = 0;
            ret = ESAPI_GetData(table_stmt, 1,
                              SQL_C_CHAR, database, 256, &db_strlen_or_ind);
            std::string database_name;
            if (SQL_SUCCEEDED(ret)) {
                database_name.assign(reinterpret_cast< const char * >(database),
                                     static_cast< size_t >(db_strlen_or_ind));
            }
            
            std::string catalog_name_sql_str;
            if (catalog_name_sz == SQL_NTS) {
                catalog_name_sql_str.assign(
                    reinterpret_cast< const char * >(catalog_name_sql));
            } else if (catalog_name_sz <= 0) {
                catalog_name_sql_str = "";
            } else {
                catalog_name_sql_str.assign(
                    reinterpret_cast< const char * >(catalog_name_sql),
                    static_cast< size_t >(catalog_name_sz));
            }
            // Catalog name is an ordinary argument when SQL_ATTR_METADATA_ID is
            // SQL_FALSE
            if (is_search_pattern && database_name != catalog_name_sql_str) {
                continue;
            }

            // Get table name
            SQLCHAR table[256] = {0};
            SQLLEN tb_strlen_or_ind = 0;
            ret = ESAPI_GetData(table_stmt, 3,
                                SQL_C_CHAR, table, 256, &tb_strlen_or_ind);
            std::string table_name;
            if (SQL_SUCCEEDED(ret)) {
                table_name.assign(reinterpret_cast< const char * >(table),
                                  static_cast< size_t >(tb_strlen_or_ind));
            }
            tables.push_back(std::make_pair(database_name, table_name));
        }
        ESAPI_FreeStmt(table_stmt, SQL_DROP);

        // Setup QResultClass
        QResultClass *res = SetupQResult(stmt, COLUMN_TEMPLATE_COUNT);
        SetupColumnQResInfo(res, static_cast< EnvironmentClass * >(
                                     CC_get_env(SC_get_conn(stmt))));

        for (const std::pair< std::string, std::string > &table : tables) {
            // Generate query
            std::string query =
                "DESCRIBE \"" + table.first + "\".\"" + table.second + "\"";

            StatementClass *col_stmt = NULL;
            ExecuteQuery(SC_get_conn(stmt),
                         reinterpret_cast< HSTMT * >(&col_stmt), query);
            int col_num = 0;
            while ((ret = ESAPI_Fetch(col_stmt)) != SQL_NO_DATA
                   && SQL_SUCCEEDED(ret)) {
                SQLCHAR column[256] = {0};
                SQLLEN col_strlen_or_ind = 0;
                ret = ESAPI_GetData(col_stmt, 1, SQL_C_CHAR, column, 256,
                                    &col_strlen_or_ind);
                col_num++;

                std::string column_name_return;
                if (SQL_SUCCEEDED(ret)) {
                    column_name_return.assign(
                        reinterpret_cast< const char * >(column),
                        static_cast< size_t >(col_strlen_or_ind));
                }

                std::string column_name;
                bool column_valid;
                ConvertToString(column_name, column_valid, column_name_sql,
                                column_name_sz);

                // Filter through search pattern
                std::regex regex_pattern = ConvertPattern(column_name);
                if (is_search_pattern
                    && !std::regex_match(column_name_return, regex_pattern)) {
                    continue;
                } else if (!is_search_pattern
                           && !std::equal(column_name_return.begin(),
                                          column_name_return.end(),
                                          column_name.begin(),
                                          case_insensitive_compare)) {
                    continue;
                }

                SQLCHAR type[256] = {0};
                SQLLEN type_strlen_or_ind = 0;
                ret = ESAPI_GetData(col_stmt, 2, SQL_C_CHAR, type, 256,
                                    &type_strlen_or_ind);
                std::string type_name_return;
                if (SQL_SUCCEEDED(ret)) {
                    type_name_return.assign(
                        reinterpret_cast< const char * >(type),
                        static_cast< size_t >(type_strlen_or_ind));
                }

                TupleField *tuple = QR_AddNew(res);
                tuple[COLUMNS_CATALOG_NAME].value = strdup(table.first.c_str());
                tuple[COLUMNS_CATALOG_NAME].len = (int)table.first.size();
                tuple[COLUMNS_SCHEMA_NAME].value = NULL;
                tuple[COLUMNS_SCHEMA_NAME].len = 0;
                tuple[COLUMNS_TABLE_NAME].value = strdup(table.second.c_str());
                tuple[COLUMNS_TABLE_NAME].len = (int)table.second.size();
                tuple[COLUMNS_COLUMN_NAME].value =
                    strdup(column_name_return.c_str());
                tuple[COLUMNS_COLUMN_NAME].len = (int)column_name_return.size();
                std::string col_data_type = std::to_string(
                    data_name_data_type_map.find(type_name_return)->second);
                tuple[COLUMNS_DATA_TYPE].value = strdup(col_data_type.c_str());
                tuple[COLUMNS_DATA_TYPE].len = (int)col_data_type.size();
                tuple[COLUMNS_TYPE_NAME].value =
                    strdup(type_name_return.c_str());
                tuple[COLUMNS_TYPE_NAME].len = (int)type_name_return.size();
                std::string col_size =
                    std::to_string(GetColumnSize(type_name_return));
                tuple[COLUMNS_PRECISION].value = strdup(col_size.c_str());
                tuple[COLUMNS_PRECISION].len = (int)col_size.size();
                std::string buf_len =
                    std::to_string(GetBufferLength(type_name_return));
                tuple[COLUMNS_LENGTH].value = strdup(buf_len.c_str());
                tuple[COLUMNS_LENGTH].len = (int)buf_len.size();
                std::string decimal_digits = std::to_string(9);
                tuple[COLUMNS_SCALE].value =
                    (type_name_return == "timestamp")
                        ? strdup(decimal_digits.c_str())
                        : NULL;
                tuple[COLUMNS_SCALE].len = (type_name_return == "timestamp")
                                               ? (int)decimal_digits.size()
                                               : 0;
                std::string col_radix = std::to_string(10);
                tuple[COLUMNS_RADIX].value = (type_name_return == "double")
                                                 ? strdup(col_radix.c_str())
                                                 : NULL;
                tuple[COLUMNS_RADIX].len =
                    (type_name_return == "double") ? (int)col_radix.size() : 0;
                std::string col_nullable = std::to_string(SQL_NULLABLE);
                tuple[COLUMNS_NULLABLE].value = strdup(col_nullable.c_str());
                tuple[COLUMNS_NULLABLE].len = (int)col_nullable.size();
                tuple[COLUMNS_REMARKS].value = NULL;
                tuple[COLUMNS_REMARKS].len = 0;
                tuple[COLUMNS_COLUMN_DEF].value = NULL;
                tuple[COLUMNS_COLUMN_DEF].len = 0;
                std::string sql_data_type =
                    (col_data_type == std::to_string(SQL_TYPE_TIMESTAMP))
                        ? std::to_string(SQL_DATETIME)
                        : col_data_type;
                tuple[COLUMNS_SQL_DATA_TYPE].value =
                    strdup(sql_data_type.c_str());
                tuple[COLUMNS_SQL_DATA_TYPE].len = (int)sql_data_type.size();
                std::string datetime_sub = std::to_string(SQL_CODE_TIMESTAMP);
                tuple[COLUMNS_SQL_DATETIME_SUB].value =
                    (col_data_type == std::to_string(SQL_TYPE_TIMESTAMP))
                        ? strdup(datetime_sub.c_str())
                        : NULL;
                tuple[COLUMNS_SQL_DATETIME_SUB].len =
                    (col_data_type == std::to_string(SQL_TYPE_TIMESTAMP))
                        ? (int)datetime_sub.size()
                        : 0;
                std::string col_char_octet_len = std::to_string(INT_MAX);
                tuple[COLUMNS_CHAR_OCTET_LENGTH].value =
                    (type_name_return == "varchar")
                        ? strdup(col_char_octet_len.c_str())
                        : NULL;
                tuple[COLUMNS_CHAR_OCTET_LENGTH].len =
                    (type_name_return == "varchar")
                        ? (int)col_char_octet_len.size()
                        : 0;
                std::string col_number = std::to_string(col_num);
                tuple[COLUMNS_ORDINAL_POSITION].value =
                    strdup(col_number.c_str());
                tuple[COLUMNS_ORDINAL_POSITION].len = (int)col_number.size();
                std::string col_is_nullable = "YES";
                tuple[COLUMNS_IS_NULLABLE].value =
                    strdup(col_is_nullable.c_str());
                tuple[COLUMNS_IS_NULLABLE].len = (int)col_is_nullable.size();
            }
            ESAPI_FreeStmt(col_stmt, SQL_DROP);
        }
        CleanUp(stmt, nullptr, SQL_SUCCESS);
        return SQL_SUCCESS;
    } catch (std::bad_alloc &e) {
        std::string error_msg = std::string("Bad allocation exception: '")
                                + e.what() + std::string("'.");
        SC_set_error(stmt, STMT_NO_MEMORY_ERROR, error_msg.c_str(), func);
    } catch (std::exception &e) {
        std::string error_msg =
            std::string("Generic exception: '") + e.what() + std::string("'.");
        SC_set_error(stmt, STMT_INTERNAL_ERROR, error_msg.c_str(), func);
    } catch (...) {
        std::string error_msg("Unknown exception raised.");
        SC_set_error(stmt, STMT_INTERNAL_ERROR, error_msg.c_str(), func);
    }
    CleanUp(stmt, nullptr);
    return SQL_ERROR;
}
void CleanUp_GetTypeInfo(StatementClass *stmt, const RETCODE ret = SQL_ERROR) {
    stmt->status = STMT_FINISHED;
    stmt->currTuple = -1;
    if (SQL_SUCCEEDED(ret))
        SC_set_rowset_start(stmt, -1, FALSE);
    else
        SC_set_Result(stmt, NULL);
    SC_set_current_col(stmt, -1);
}

void SetupTypeQResInfo(QResultClass *res) {
    QR_set_field_info_v(res, GETTYPE_TYPE_NAME, TYPE_NAME, TS_TYPE_VARCHAR,
                        MAX_INFO_STRING);
    QR_set_field_info_v(res, GETTYPE_DATA_TYPE, DATA_TYPE, TS_TYPE_INT2, 2);
    QR_set_field_info_v(res, GETTYPE_COLUMN_SIZE, PRECISION, TS_TYPE_INTEGER, 4);
    QR_set_field_info_v(res, GETTYPE_LITERAL_PREFIX, LITERAL_PREFIX,
                        TS_TYPE_VARCHAR, MAX_INFO_STRING);
    QR_set_field_info_v(res, GETTYPE_LITERAL_SUFFIX, LITERAL_SUFFIX,
                        TS_TYPE_VARCHAR, MAX_INFO_STRING);
    QR_set_field_info_v(res, GETTYPE_CREATE_PARAMS, CREATE_PARAMS,
                        TS_TYPE_VARCHAR, MAX_INFO_STRING);
    QR_set_field_info_v(res, GETTYPE_NULLABLE, NULLABLE, TS_TYPE_INT2, 2);
    QR_set_field_info_v(res, GETTYPE_CASE_SENSITIVE, CASE_SENSITIVE,
                        TS_TYPE_INT2, 2);
    QR_set_field_info_v(res, GETTYPE_SEARCHABLE, SEARCHABLE, TS_TYPE_INT2, 2);
    QR_set_field_info_v(res, GETTYPE_UNSIGNED_ATTRIBUTE, UNSIGNED_ATTRIBUTE,
                        TS_TYPE_INT2, 2);
    QR_set_field_info_v(res, GETTYPE_FIXED_PREC_SCALE, FIXED_PREC_SCALE,
                        TS_TYPE_INT2, 2);
    QR_set_field_info_v(res, GETTYPE_AUTO_UNIQUE_VALUE, AUTO_INCREMENT,
                        TS_TYPE_INT2, 2);
    QR_set_field_info_v(res, GETTYPE_LOCAL_TYPE_NAME, LOCAL_TYPE_NAME,
                        TS_TYPE_VARCHAR, MAX_INFO_STRING);
    QR_set_field_info_v(res, GETTYPE_MINIMUM_SCALE, MINIMUM_SCALE, TS_TYPE_INT2,
                        2);
    QR_set_field_info_v(res, GETTYPE_MAXIMUM_SCALE, MAXIMUM_SCALE, TS_TYPE_INT2,
                        2);
    QR_set_field_info_v(res, GETTYPE_SQL_DATA_TYPE, SQL_DATA_TYPE, TS_TYPE_INT2,
                        2);
    QR_set_field_info_v(res, GETTYPE_SQL_DATETIME_SUB, SQL_DATETIME_SUB,
                        TS_TYPE_INT2, 2);
    QR_set_field_info_v(res, GETTYPE_NUM_PREC_RADIX, NUM_PREC_RADIX,
                        TS_TYPE_INTEGER, 4);
    QR_set_field_info_v(res, GETTYPE_INTERVAL_PRECISION, INTERVAL_PRECISION,
                        TS_TYPE_INT2, 2);
}

RETCODE SetTypeResult(ConnectionClass *conn, StatementClass *stmt,
                      QResultClass *res, int esType, int sqlType) {
    TupleField *tuple;

    if (tuple = QR_AddNew(res), NULL == tuple) {
        SC_set_error(stmt, STMT_NO_MEMORY_ERROR, "Couldn't QR_AddNew.",
                     "SetTypeResult");
        CleanUp_GetTypeInfo(stmt, SQL_ERROR);
        return SQL_ERROR;
    }

    set_tuplefield_string(&tuple[GETTYPE_TYPE_NAME],
                          estype_attr_to_name(conn, esType, -1, FALSE));
    set_tuplefield_int2(&tuple[GETTYPE_NULLABLE],
                        estype_nullable(conn, esType));

    set_tuplefield_int2(&tuple[GETTYPE_DATA_TYPE],
                        static_cast< short >(sqlType));
    set_tuplefield_int2(&tuple[GETTYPE_CASE_SENSITIVE],
                        tstype_case_sensitive(conn, esType));
    set_tuplefield_int2(&tuple[GETTYPE_SEARCHABLE],
                        estype_searchable(conn, esType));
    set_tuplefield_int2(&tuple[GETTYPE_FIXED_PREC_SCALE],
                        estype_money(conn, esType));

    //  Localized data-source dependent data type name (always NULL)
    set_tuplefield_null(&tuple[GETTYPE_LOCAL_TYPE_NAME]);

    // These values can be NULL
    set_nullfield_int4(
        &tuple[GETTYPE_COLUMN_SIZE],
        estype_attr_column_size(conn, esType, ES_ATP_UNSET, ES_ADT_UNSET,
                                ES_UNKNOWNS_UNSET));
    set_nullfield_string(&tuple[GETTYPE_LITERAL_PREFIX],
                         estype_literal_prefix(conn, esType));
    set_nullfield_string(&tuple[GETTYPE_LITERAL_SUFFIX],
                         estype_literal_suffix(conn, esType));
    set_nullfield_string(&tuple[GETTYPE_CREATE_PARAMS],
                         estype_create_params(conn, esType));
    set_nullfield_int2(&tuple[GETTYPE_UNSIGNED_ATTRIBUTE],
                       estype_unsigned(conn, esType));
    set_nullfield_int2(&tuple[GETTYPE_AUTO_UNIQUE_VALUE],
                       tstype_auto_increment(conn, esType));
    set_nullfield_int2(&tuple[GETTYPE_MINIMUM_SCALE],
                       estype_min_decimal_digits(conn, esType));
    set_nullfield_int2(&tuple[GETTYPE_MAXIMUM_SCALE],
                       estype_max_decimal_digits(conn, esType));
    set_tuplefield_int2(&tuple[GETTYPE_SQL_DATA_TYPE],
                        static_cast< short >(sqlType));
    set_nullfield_int2(&tuple[GETTYPE_SQL_DATETIME_SUB],
                       estype_attr_to_datetime_sub(conn, esType, ES_ATP_UNSET));
    set_nullfield_int4(&tuple[GETTYPE_NUM_PREC_RADIX],
                       estype_radix(conn, esType));
    set_nullfield_int4(&tuple[GETTYPE_INTERVAL_PRECISION], 0);

    return SQL_SUCCESS;
}

RETCODE SQL_API ESAPI_GetTypeInfo(HSTMT hstmt, SQLSMALLINT fSqlType) {
    CSTR func = "ESAPI_GetTypeInfo";
    StatementClass *stmt = (StatementClass *)hstmt;
    ConnectionClass *conn;
    conn = SC_get_conn(stmt);
    QResultClass *res = NULL;

    int result_cols;
    RETCODE result = SQL_ERROR;

    if (result = SC_initialize_and_recycle(stmt), SQL_SUCCESS != result)
        return result;

    try {
        if (res = QR_Constructor(), !res) {
            SC_set_error(stmt, STMT_INTERNAL_ERROR, "Error creating result.",
                         func);
            return SQL_ERROR;
        }
        SC_set_Result(stmt, res);

        result_cols = NUM_OF_GETTYPE_FIELDS;
        extend_column_bindings(SC_get_ARDF(stmt),
                               static_cast< SQLSMALLINT >(result_cols));

        stmt->catalog_result = TRUE;
        QR_set_num_fields(res, result_cols);
        SetupTypeQResInfo(res);

        if (fSqlType == SQL_ALL_TYPES) {
            for (std::pair< int, std::vector< int > > sqlType :
                 sql_es_type_map) {
                for (auto const &esType : sqlType.second) {
                    result =
                        SetTypeResult(conn, stmt, res, esType, sqlType.first);
                }
            }
        } else {
            if (sql_es_type_map.count(fSqlType) > 0) {
                for (auto esType : sql_es_type_map.at(fSqlType)) {
                    result = SetTypeResult(conn, stmt, res, esType, fSqlType);
                }
            }
        }
        result = SQL_SUCCESS;

    } catch (std::bad_alloc &e) {
        std::string error_msg = std::string("Bad allocation exception: '")
                                + e.what() + std::string("'.");
        SC_set_error(stmt, STMT_NO_MEMORY_ERROR, error_msg.c_str(), func);
    } catch (std::exception &e) {
        std::string error_msg =
            std::string("Generic exception: '") + e.what() + std::string("'.");
        SC_set_error(stmt, STMT_INTERNAL_ERROR, error_msg.c_str(), func);
    } catch (...) {
        std::string error_msg("Unknown exception raised.");
        SC_set_error(stmt, STMT_INTERNAL_ERROR, error_msg.c_str(), func);
    }

    CleanUp_GetTypeInfo(stmt, result);
    return result;
}