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

#ifndef TS_TYPES
#define TS_TYPES

#include "dlg_specific.h"
#include "odbc.h"
#ifdef __cplusplus
#include <aws/timestream-query/model/QueryResult.h>
extern "C" {
#endif
/* the type numbers are defined by the OID's of the types' rows */
/* in table ts_type */

#ifdef NOT_USED
#define TS_TYPE_LO ? ? ? ? /* waiting for permanent type */
#endif

#define TS_TYPE_NAME_BOOLEAN "boolean"
#define TS_TYPE_NAME_INTEGER "int"
#define TS_TYPE_NAME_BIGINT "bigint"
#define TS_TYPE_NAME_DOUBLE "double"
#define TS_TYPE_NAME_VARCHAR "varchar"
#define TS_TYPE_NAME_ARRAY "array[T,...]"
#define TS_TYPE_NAME_ROW "row(T,...)"
#define TS_TYPE_NAME_DATE "date"
#define TS_TYPE_NAME_TIME "time"
#define TS_TYPE_NAME_TIMESTAMP "timestamp"
#define TS_TYPE_NAME_INTERVAL_DAY_TO_SECOND "interval day to second"
#define TS_TYPE_NAME_INTERVAL_YEAR_TO_MONTH "interval year to month"
#define TS_TYPE_NAME_TIMESERIES "timeseries[row(timestamp, T,...)]"
#define TS_TYPE_NAME_UNKNOWN "unknown"

#define MS_ACCESS_SERIAL "int identity"
#define TS_TYPE_BOOLEAN 16
#define TS_TYPE_BIGINT 20
#define TS_TYPE_INT2 21
#define TS_TYPE_INTEGER 23
#define TS_TYPE_DOUBLE 701
#define TS_TYPE_ARRAY 1016
#define TS_TYPE_INTERVAL_DAY_TO_SECOND 1017
#define TS_TYPE_INTERVAL_YEAR_TO_MONTH 1018
#define TS_TYPE_ROW 1019
#define TS_TYPE_TIMESERIES 1020
#define TS_TYPE_VARCHAR 1043
#define TS_TYPE_UNKNOWN 1048
#define TS_TYPE_DATE 1082
#define TS_TYPE_TIME 1083
#define TS_TYPE_TIMESTAMP_NO_TMZONE 1114 /* since 7.2 */
#define TS_TYPE_TIMESTAMP 1296        /* deprecated since 7.0 */
#define INTERNAL_ASIS_TYPE (-9999)

#define TYPE_MAY_BE_ARRAY(type) \
    (type) >= 1000 && (type) <= 1041

#define TS_ATP_UNSET (-3) /* atttypmod */
#define TS_ADT_UNSET (-3) /* adtsize_or_longestlen */
#define TS_UNKNOWNS_UNSET 0 /* UNKNOWNS_AS_MAX */

OID es_true_type(const ConnectionClass *, OID, OID);
OID sqltype_to_estype(const ConnectionClass *conn, SQLSMALLINT fSqlType);
const char *sqltype_to_escast(const ConnectionClass *conn,
                              SQLSMALLINT fSqlType);

SQLSMALLINT estype_to_concise_type(const StatementClass *stmt, OID type,
                                   int col, int handle_unknown_size_as);
SQLSMALLINT estype_to_sqldesctype(const StatementClass *stmt, OID type, int col,
                                  int handle_unknown_size_as);
const char *estype_to_name(const StatementClass *stmt, OID type, int col,
                           BOOL auto_increment);

SQLSMALLINT estype_attr_to_concise_type(const ConnectionClass *conn, OID type,
                                        int typmod, int adtsize_or_longestlen,
                                        int handle_unknown_size_as);
SQLSMALLINT estype_attr_to_sqldesctype(const ConnectionClass *conn, OID type,
                                       int typmod, int adtsize_or_longestlen,
                                       int handle_unknown_size_as);
SQLSMALLINT tstype_attr_to_datetime_sub(const ConnectionClass *conn, OID type,
                                        int typmod);
SQLSMALLINT estype_attr_to_ctype(const ConnectionClass *conn, OID type,
                                 int typmod);
const char *tstype_attr_to_name(const ConnectionClass *conn, OID type,
                                int typmod, BOOL auto_increment);
Int4 tstype_attr_column_size(const ConnectionClass *conn, OID type,
                             int atttypmod, int adtsize_or_longest,
                             int handle_unknown_size_as);
Int4 estype_attr_buffer_length(const ConnectionClass *conn, OID type,
                               int atttypmod, int adtsize_or_longestlen,
                               int handle_unknown_size_as);
Int4 estype_attr_display_size(const ConnectionClass *conn, OID type,
                              int atttypmod, int adtsize_or_longestlen,
                              int handle_unknown_size_as);
Int2 estype_attr_decimal_digits(const ConnectionClass *conn, OID type,
                                int atttypmod);
Int4 estype_attr_transfer_octet_length(const ConnectionClass *conn, OID type,
                                       int atttypmod,
                                       int handle_unknown_size_as);
SQLSMALLINT estype_attr_precision(const ConnectionClass *conn, OID type,
                                  int atttypmod);
Int4 estype_attr_desclength(const ConnectionClass *conn, OID type,
                            int atttypmod, int adtsize_or_longestlen,
                            int handle_unknown_size_as);

/*	These functions can use static numbers or result sets(col parameter) */
Int4 estype_column_size(
    const StatementClass *stmt, OID type, int col,
    int handle_unknown_size_as); /* corresponds to "precision" in ODBC 2.x */
SQLSMALLINT estype_precision(
    const StatementClass *stmt, OID type, int col); /* "precsion in ODBC 3.x */
/* the following size/length are of Int4 due to ES restriction */
Int4 estype_display_size(const StatementClass *stmt, OID type, int col,
                         int handle_unknown_size_as);
Int4 estype_buffer_length(const StatementClass *stmt, OID type, int col,
                          int handle_unknown_size_as);
Int4 estype_desclength(const StatementClass *stmt, OID type, int col,
                       int handle_unknown_size_as);
// Int4		estype_transfer_octet_length(const ConnectionClass *conn, OID type,
// int column_size);

SQLSMALLINT estype_decimal_digits(
    const StatementClass *stmt, OID type,
    int col); /* corresponds to "scale" in ODBC 2.x */
SQLSMALLINT tstype_min_decimal_digits(
    const ConnectionClass *conn,
    OID type); /* corresponds to "min_scale" in ODBC 2.x */
SQLSMALLINT tstype_max_decimal_digits(
    const ConnectionClass *conn,
    OID type); /* corresponds to "max_scale" in ODBC 2.x */
Int2 tstype_radix(const ConnectionClass *conn, OID type);
Int2 tstype_nullable(const ConnectionClass *conn, OID type);
Int2 tstype_auto_increment(const ConnectionClass *conn, OID type);
Int2 tstype_case_sensitive(const ConnectionClass *conn, OID type);
Int2 estype_money(const ConnectionClass *conn, OID type);
Int2 tstype_searchable(const ConnectionClass *conn, OID type);
Int2 tstype_unsigned(const ConnectionClass *conn, OID type);
const char *tstype_literal_prefix(const ConnectionClass *conn, OID type);
const char *tstype_literal_suffix(const ConnectionClass *conn, OID type);

SQLSMALLINT sqltype_to_default_ctype(const ConnectionClass *stmt,
                                     SQLSMALLINT sqltype);
Int4 ctype_length(SQLSMALLINT ctype);

SQLSMALLINT ansi_to_wtype(const ConnectionClass *self, SQLSMALLINT ansitype);

#ifdef __cplusplus
}
#endif

typedef enum {
    CONNECTION_OK,
    CONNECTION_BAD,
    /* Non-blocking mode only below here */

    /*
     * The existence of these should never be relied upon - they should only
     * be used for user feedback or similar purposes.
     */
    CONNECTION_STARTED,           /* Waiting for connection to be made.  */
    CONNECTION_MADE,              /* Connection OK; waiting to send.     */
    CONNECTION_AWAITING_RESPONSE, /* Waiting for a response from the postmaster.
                                   */
    CONNECTION_AUTH_OK, /* Received authentication; waiting for backend startup.
                         */
    CONNECTION_SETENV,  /* Negotiating environment. */
    CONNECTION_SSL_STARTUP,    /* Negotiating SSL. */
    CONNECTION_NEEDED,         /* Internal state: connect() needed */
    CONNECTION_CHECK_WRITABLE, /* Check if we could make a writable connection.
                                */
    CONNECTION_CONSUME,    /* Wait for any pending message and consume them. */
    CONNECTION_GSS_STARTUP /* Negotiating GSSAPI. */
} ConnStatusType;

// Only expose this to C++ code, this will be passed through the C interface as
// a void*
#ifdef __cplusplus
#include <stdint.h>

#include <string>
#include <vector>

typedef struct authentication_options {
    std::string auth_type;
    std::string profile_name;
    std::string uid;
    std::string pwd;
    std::string session_token;
    std::string region;
    std::string end_point_override;
    std::string idp_name;
    std::string idp_host;
    std::string okta_application_id;
    std::string role_arn;
    std::string aad_application_id;
    std::string aad_client_secret;
    std::string aad_tenant;
    std::string idp_arn;
} authentication_options;

typedef struct encryption_options {
    std::string certificate_type;
    std::string certificate;
    std::string key;
    std::string key_pw;
} encryption_options;

typedef struct connection_options {
    std::string timeout;
    std::string connection_timeout;
    std::string max_retry_count_client;
    std::string max_connections;
} connection_options;

typedef struct runtime_options {
    connection_options conn;
    authentication_options auth;
    encryption_options crypt;
} runtime_options;

#define INVALID_OID 0
#define KEYWORD_TYPE_OID 1043
#define KEYWORD_TYPE_SIZE 255
#define KEYWORD_DISPLAY_SIZE 255
#define KEYWORD_LENGTH_OF_STR 255

#endif
#endif
