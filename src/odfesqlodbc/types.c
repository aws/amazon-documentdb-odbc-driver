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

#include "connection.h"
#include "dlg_specific.h"
#include "environ.h"
#include "qresult.h"
#include "statement.h"
#include "types.h"
#ifndef WIN32
#include <limits.h>
#endif

#define EXPERIMENTAL_CURRENTLY

SQLSMALLINT ansi_to_wtype(const ConnectionClass *self, SQLSMALLINT ansitype) {
#ifndef UNICODE_SUPPORT
    return ansitype;
#else
    if (!ALLOW_WCHAR(self))
        return ansitype;
    switch (ansitype) {
        case SQL_CHAR:
            return SQL_WCHAR;
        case SQL_VARCHAR:
            return SQL_WVARCHAR;
        case SQL_LONGVARCHAR:
            return SQL_WLONGVARCHAR;
    }
    return ansitype;
#endif /* UNICODE_SUPPORT */
}

#ifdef ODBCINT64
#define ALLOWED_C_BIGINT SQL_C_SBIGINT
/* #define	ALLOWED_C_BIGINT	SQL_C_CHAR */ /* Delphi should be either ? */
#else
#define ALLOWED_C_BIGINT SQL_C_CHAR
#endif

OID es_true_type(const ConnectionClass *conn, OID type, OID basetype) {
    if (0 == basetype)
        return type;
    else if (0 == type)
        return basetype;
    else if (type == (OID)conn->lobj_type)
        return type;
    return basetype;
}

#define MONTH_BIT (1 << 17)
#define YEAR_BIT (1 << 18)
#define DAY_BIT (1 << 19)
#define HOUR_BIT (1 << 26)
#define MINUTE_BIT (1 << 27)
#define SECOND_BIT (1 << 28)

static Int4 getCharColumnSizeX(const ConnectionClass *conn, OID type,
                               int atttypmod, int adtsize_or_longestlen,
                               int handle_unknown_size_as) {
    int p = -1, maxsize;
    MYLOG(LOG_TRACE,
          "entering type=%d, atttypmod=%d, adtsize_or=%d, unknown = %d\n", type,
          atttypmod, adtsize_or_longestlen, handle_unknown_size_as);

    maxsize = MAX_VARCHAR_SIZE;
#ifdef UNICODE_SUPPORT
    if (CC_is_in_unicode_driver(conn) && isSqlServr() && maxsize > 4000)
        maxsize = 4000;
#endif /* UNICODE_SUPPORT */

    if (maxsize == TEXT_FIELD_SIZE + 1) /* magic length for testing */
        maxsize = 0;

    /*
     * Static ColumnSize (i.e., the Maximum ColumnSize of the datatype) This
     * has nothing to do with a result set.
     */
    MYLOG(LOG_DEBUG, "!!! atttypmod  < 0 ?\n");
    if (atttypmod < 0 && adtsize_or_longestlen < 0)
        return maxsize;

    MYLOG(LOG_DEBUG, "!!! adtsize_or_logngest=%d\n", adtsize_or_longestlen);
    p = adtsize_or_longestlen; /* longest */
                               /*
                                * Catalog Result Sets -- use assigned column width (i.e., from
                                * set_tuplefield_string)
                                */
    MYLOG(LOG_DEBUG, "!!! catalog_result=%d\n", handle_unknown_size_as);
    if (UNKNOWNS_AS_LONGEST == handle_unknown_size_as) {
        MYLOG(LOG_DEBUG, "LONGEST: p = %d\n", p);
        if (p > 0 && (atttypmod < 0 || atttypmod > p))
            return p;
    }
    if (TYPE_MAY_BE_ARRAY(type)) {
        if (p > 0)
            return p;
        return maxsize;
    }

    /* Size is unknown -- handle according to parameter */
    if (atttypmod > 0) /* maybe the length is known */
    {
        return atttypmod;
    }

    /* The type is really unknown */
    switch (handle_unknown_size_as) {
        case UNKNOWNS_AS_DONTKNOW:
            return -1;
        case UNKNOWNS_AS_LONGEST:
        case UNKNOWNS_AS_MAX:
            break;
        default:
            return -1;
    }
    if (maxsize <= 0)
        return maxsize;
    switch (type) {
        case TS_TYPE_VARCHAR:
            return maxsize;
    }

    if (p > maxsize)
        maxsize = p;
    return maxsize;
}

/*
 *	Specify when handle_unknown_size_as parameter is unused
 */
#define UNUSED_HANDLE_UNKNOWN_SIZE_AS (-2)

static SQLSMALLINT getTimestampDecimalDigitsX(const ConnectionClass *conn,
                                              OID type, int atttypmod) {
    UNUSED(conn);
    MYLOG(LOG_DEBUG, "type=%d, atttypmod=%d\n", type, atttypmod);
    return (SQLSMALLINT)(atttypmod > -1 ? atttypmod : 6);
}

#ifdef TS_INTERVAL_AS_SQL_INTERVAL
static SQLSMALLINT getIntervalDecimalDigits(OID type, int atttypmod) {
    Int4 prec;

    MYLOG(LOG_TRACE, "entering type=%d, atttypmod=%d\n", type, atttypmod);

    if ((atttypmod & SECOND_BIT) == 0)
        return 0;
    return (SQLSMALLINT)((prec = atttypmod & 0xffff) == 0xffff ? 6 : prec);
}
#endif  // TS_INTERVAL_AS_SQL_INTERVAL

SQLSMALLINT
estype_attr_to_concise_type(const ConnectionClass *conn, OID type,
                            int atttypmod, int adtsize_or_longestlen,
                            int handle_unknown_size_as) {
    EnvironmentClass *env = (EnvironmentClass *)CC_get_env(conn);
#ifdef TS_INTERVAL_AS_SQL_INTERVAL
    SQLSMALLINT sqltype;
#endif /* TS_INTERVAL_AS_SQL_INTERVAL */
    BOOL bLongVarchar, bFixed = FALSE;

    switch (type) {
        case TS_TYPE_VARCHAR:
            if (getCharColumnSizeX(conn, type, atttypmod, adtsize_or_longestlen,
                                   handle_unknown_size_as)
                > MAX_VARCHAR_SIZE)
                bLongVarchar = TRUE;
            else
                bLongVarchar = FALSE;
            return ansi_to_wtype(conn, bLongVarchar
                                           ? SQL_LONGVARCHAR
                                           : (bFixed ? SQL_CHAR : SQL_VARCHAR));

        case TS_TYPE_INT2:
            return SQL_SMALLINT;

        case TS_TYPE_INTEGER:
            return SQL_INTEGER;

            /* Change this to SQL_BIGINT for ODBC v3 bjm 2001-01-23 */
        case TS_TYPE_BIGINT:
            if (conn->ms_jet)
                return SQL_NUMERIC; /* maybe a little better than SQL_VARCHAR */
            return SQL_BIGINT;

        case TS_TYPE_DOUBLE:
            return SQL_DOUBLE;
        case TS_TYPE_DATE:
            if (EN_is_odbc3(env))
                return SQL_TYPE_DATE;
            return SQL_DATE;
        case TS_TYPE_TIME:
            if (EN_is_odbc3(env))
                return SQL_TYPE_TIME;
            return SQL_TIME;
        case TS_TYPE_TIMESTAMP_NO_TMZONE:
        case TS_TYPE_TIMESTAMP:
            if (EN_is_odbc3(env))
                return SQL_TYPE_TIMESTAMP;
            return SQL_TIMESTAMP;
        case TS_TYPE_BOOLEAN:
            return SQL_BIT;

        default:

            /*
             * first, check to see if 'type' is in list.  If not, look up
             * with query. Add oid, name to list.  If it's already in
             * list, just return.
             */
            /* hack until permanent type is available */
            if (type == (OID)conn->lobj_type)
                return SQL_LONGVARBINARY;

            bLongVarchar = DEFAULT_UNKNOWNSASLONGVARCHAR;
            if (bLongVarchar) {
                int column_size = getCharColumnSizeX(conn, type, atttypmod,
                                                     adtsize_or_longestlen,
                                                     handle_unknown_size_as);
                if (column_size > 0 && column_size <= MAX_VARCHAR_SIZE)
                    bLongVarchar = FALSE;
            }
#ifdef EXPERIMENTAL_CURRENTLY
            return ansi_to_wtype(conn,
                                 bLongVarchar ? SQL_LONGVARCHAR : SQL_VARCHAR);
#else
            return bLongVarchar ? SQL_LONGVARCHAR : SQL_VARCHAR;
#endif /* EXPERIMENTAL_CURRENTLY */
    }
}

SQLSMALLINT
estype_attr_to_sqldesctype(const ConnectionClass *conn, OID type, int atttypmod,
                           int adtsize_or_longestlen,
                           int handle_unknown_size_as) {
    SQLSMALLINT rettype;

    switch (rettype = estype_attr_to_concise_type(conn, type, atttypmod,
                                                  adtsize_or_longestlen,
                                                  handle_unknown_size_as)) {
        case SQL_TYPE_DATE:
        case SQL_TYPE_TIME:
        case SQL_TYPE_TIMESTAMP:
            return SQL_DATETIME;
    }
    return rettype;
}

SQLSMALLINT
tstype_attr_to_datetime_sub(const ConnectionClass *conn, OID type,
                            int atttypmod) {
    UNUSED(conn, type, atttypmod);
    SQLSMALLINT rettype;
    switch (rettype = estype_attr_to_concise_type( conn, type, atttypmod, TS_ADT_UNSET, TS_UNKNOWNS_UNSET)) {
        case SQL_TYPE_DATE:
            return SQL_CODE_DATE;
        case SQL_TYPE_TIME:
            return SQL_CODE_TIME;
        case SQL_TYPE_TIMESTAMP:
            return SQL_CODE_TIMESTAMP;
        case SQL_INTERVAL_MONTH:
        case SQL_INTERVAL_YEAR:
        case SQL_INTERVAL_YEAR_TO_MONTH:
        case SQL_INTERVAL_DAY:
        case SQL_INTERVAL_HOUR:
        case SQL_INTERVAL_MINUTE:
        case SQL_INTERVAL_SECOND:
        case SQL_INTERVAL_DAY_TO_HOUR:
        case SQL_INTERVAL_DAY_TO_MINUTE:
        case SQL_INTERVAL_DAY_TO_SECOND:
        case SQL_INTERVAL_HOUR_TO_MINUTE:
        case SQL_INTERVAL_HOUR_TO_SECOND:
        case SQL_INTERVAL_MINUTE_TO_SECOND:
            return rettype - 100;
    }
    return -1;
}

SQLSMALLINT
estype_attr_to_ctype(const ConnectionClass *conn, OID type, int atttypmod) {
    UNUSED(atttypmod);
    EnvironmentClass *env = (EnvironmentClass *)CC_get_env(conn);
#ifdef TS_INTERVAL_AS_SQL_INTERVAL
    SQLSMALLINT ctype;
#endif /* TS_INTERVAL_A_SQL_INTERVAL */

    switch (type) {
        case TS_TYPE_BIGINT:
            if (!conn->ms_jet)
                return ALLOWED_C_BIGINT;
            return SQL_C_CHAR;
        case TS_TYPE_INT2:
            return SQL_C_SSHORT;
        case TS_TYPE_INTEGER:
            return SQL_C_SLONG;
        case TS_TYPE_DOUBLE:
            return SQL_C_DOUBLE;
        case TS_TYPE_DATE:
            if (EN_is_odbc3(env))
                return SQL_C_TYPE_DATE;
            return SQL_C_DATE;
        case TS_TYPE_TIME:
            if (EN_is_odbc3(env))
                return SQL_C_TYPE_TIME;
            return SQL_C_TIME;
        case TS_TYPE_TIMESTAMP_NO_TMZONE:
        case TS_TYPE_TIMESTAMP:
            if (EN_is_odbc3(env))
                return SQL_C_TYPE_TIMESTAMP;
            return SQL_C_TIMESTAMP;
        case TS_TYPE_BOOLEAN:
            return SQL_C_BIT;
        case TS_TYPE_VARCHAR:
            return ansi_to_wtype(conn, SQL_C_CHAR);
        default:
            /* hack until permanent type is available */
            if (type == (OID)conn->lobj_type)
                return SQL_C_BINARY;

                /* Experimental, Does this work ? */
#ifdef EXPERIMENTAL_CURRENTLY
            return ansi_to_wtype(conn, SQL_C_CHAR);
#else
            return SQL_C_CHAR;
#endif /* EXPERIMENTAL_CURRENTLY */
    }
}

const char *tstype_attr_to_name(const ConnectionClass *conn, OID type,
                                int typmod, BOOL auto_increment) {
    UNUSED(conn, typmod, conn, auto_increment);
    switch (type) {
        case TS_TYPE_BOOLEAN:
            return TS_TYPE_NAME_BOOLEAN;
        case TS_TYPE_INTEGER:
            return TS_TYPE_NAME_INTEGER;
        case TS_TYPE_BIGINT:
            return TS_TYPE_NAME_BIGINT;
        case TS_TYPE_DOUBLE:
            return TS_TYPE_NAME_DOUBLE;
        case TS_TYPE_VARCHAR:
            return TS_TYPE_NAME_VARCHAR;
        case TS_TYPE_ARRAY:
            return TS_TYPE_NAME_ARRAY;
        case TS_TYPE_ROW:
            return TS_TYPE_NAME_ROW;
        case TS_TYPE_DATE:
            return TS_TYPE_NAME_DATE;
        case TS_TYPE_TIME:
            return TS_TYPE_NAME_TIME;
        case TS_TYPE_TIMESTAMP:
            return TS_TYPE_NAME_TIMESTAMP;
        case TS_TYPE_INTERVAL_YEAR_TO_MONTH:
            return TS_TYPE_NAME_INTERVAL_YEAR_TO_MONTH;
        case TS_TYPE_INTERVAL_DAY_TO_SECOND:
            return TS_TYPE_NAME_INTERVAL_DAY_TO_SECOND;
        case TS_TYPE_TIMESERIES:
            return TS_TYPE_NAME_TIMESERIES;
        case TS_TYPE_UNKNOWN:
        default:
            return TS_TYPE_NAME_UNKNOWN;
    }
}

Int4 /* Amazon Timestream restriction */
tstype_attr_column_size(const ConnectionClass *conn, OID type, int atttypmod,
                        int adtsize_or_longest, int handle_unknown_size_as) {
    UNUSED(handle_unknown_size_as, adtsize_or_longest, atttypmod, conn);
    switch (type) {
        case TS_TYPE_BOOLEAN:
            return 1;
        case TS_TYPE_INT2:
            return 5;
        case TS_TYPE_INTEGER:
            return 11;
        case TS_TYPE_BIGINT:
            return 20;
        case TS_TYPE_DOUBLE:
            return 15;
        case TS_TYPE_DATE:
            return 10;
        case TS_TYPE_TIME:
            return 18;
        case TS_TYPE_TIMESTAMP:
            return 29;
        case TS_TYPE_INTERVAL_YEAR_TO_MONTH:
        case TS_TYPE_INTERVAL_DAY_TO_SECOND:
        case TS_TYPE_ARRAY:
        case TS_TYPE_ROW:
        case TS_TYPE_TIMESERIES:
        case TS_TYPE_VARCHAR:
        case TS_TYPE_UNKNOWN:
            return INT_MAX;
        default:
            return adtsize_or_longest;
    }
}

SQLSMALLINT
estype_attr_precision(const ConnectionClass *conn, OID type, int atttypmod) {
    switch (type) {
        case TS_TYPE_TIME:
        case TS_TYPE_TIMESTAMP_NO_TMZONE:
            return getTimestampDecimalDigitsX(conn, type, atttypmod);
    }
    return -1;
}

Int4 estype_attr_display_size(const ConnectionClass *conn, OID type,
                              int atttypmod, int adtsize_or_longestlen,
                              int handle_unknown_size_as) {
    switch (type) {
        case TS_TYPE_INT2:
            return 6;

        case TS_TYPE_INTEGER:
            return 11;

        case TS_TYPE_BIGINT:
            return 20; /* signed: 19 digits + sign */

        case TS_TYPE_DOUBLE: /* a sign, TS_DOUBLE_DIGITS digits, a decimal
                                point, the letter E, a sign, and 3 digits */
            return (1 + TS_DOUBLE_DIGITS + 1 + 1 + 1 + 3);

            /* Character types use regular precision */
        default:
            return tstype_attr_column_size(conn, type, atttypmod,
                                           adtsize_or_longestlen,
                                           handle_unknown_size_as);
    }
}

Int4 estype_attr_buffer_length(const ConnectionClass *conn, OID type,
                               int atttypmod, int adtsize_or_longestlen,
                               int handle_unknown_size_as) {
    switch (type) {
        case TS_TYPE_INT2:
            return 2; /* sizeof(SQLSMALLINT) */

        case TS_TYPE_INTEGER:
            return 4; /* sizeof(SQLINTEGER) */

        case TS_TYPE_BIGINT:
            if (SQL_C_CHAR == estype_attr_to_ctype(conn, type, atttypmod))
                return 20; /* signed: 19 digits + sign */
            return 8;      /* sizeof(SQLSBININT) */

        case TS_TYPE_DOUBLE:
            return 8; /* sizeof(SQLFLOAT) */

        case TS_TYPE_DATE:
        case TS_TYPE_TIME:
            return 6; /* sizeof(DATE(TIME)_STRUCT) */

        case TS_TYPE_TIMESTAMP:
        case TS_TYPE_TIMESTAMP_NO_TMZONE:
            return 16; /* sizeof(TIMESTAMP_STRUCT) */

            /* Character types use the default precision */
        case TS_TYPE_VARCHAR: {
            int coef = 1;
            Int4 prec = tstype_attr_column_size(conn, type, atttypmod,
                                                adtsize_or_longestlen,
                                                handle_unknown_size_as),
                 maxvarc;
            if (SQL_NO_TOTAL == prec)
                return prec;
#ifdef UNICODE_SUPPORT
            if (CC_is_in_unicode_driver(conn))
                return prec * WCLEN;
#endif /* UNICODE_SUPPORT */
            coef = conn->mb_maxbyte_per_char;
            if (coef < 2)
                /* CR -> CR/LF */
                coef = 2;
            if (coef == 1)
                return prec;
            maxvarc = MAX_VARCHAR_SIZE;
            if (prec <= maxvarc && prec * coef > maxvarc)
                return maxvarc;
            return coef * prec;
        }
        default:
            return tstype_attr_column_size(conn, type, atttypmod,
                                           adtsize_or_longestlen,
                                           handle_unknown_size_as);
    }
}

/*
 */
Int4 estype_attr_desclength(const ConnectionClass *conn, OID type,
                            int atttypmod, int adtsize_or_longestlen,
                            int handle_unknown_size_as) {
    switch (type) {
        case TS_TYPE_INT2:
            return 2;

        case TS_TYPE_INTEGER:
            return 4;

        case TS_TYPE_BIGINT:
            return 20; /* signed: 19 digits + sign */

        case TS_TYPE_DOUBLE:
            return 8;

        case TS_TYPE_DATE:
        case TS_TYPE_TIME:
        case TS_TYPE_TIMESTAMP_NO_TMZONE:
        case TS_TYPE_TIMESTAMP:
        case TS_TYPE_VARCHAR:
        default:
            return tstype_attr_column_size(conn, type, atttypmod,
                                           adtsize_or_longestlen,
                                           handle_unknown_size_as);
    }
}

Int2 estype_attr_decimal_digits(const ConnectionClass *conn, OID type,
                                int atttypmod) {
    switch (type) {
        case TS_TYPE_INT2:
        case TS_TYPE_INTEGER:
        case TS_TYPE_BIGINT:
        case TS_TYPE_DOUBLE:
        case TS_TYPE_BOOLEAN:
        case TS_TYPE_TIMESTAMP:
            return 0;
        case TS_TYPE_TIME:
        case TS_TYPE_TIMESTAMP_NO_TMZONE:
            /* return 0; */
            return getTimestampDecimalDigitsX(conn, type, atttypmod);
        default:
            return -1;
    }
}

Int4 estype_attr_transfer_octet_length(const ConnectionClass *conn, OID type,
                                       int atttypmod,
                                       int handle_unknown_size_as) {
    int coef = 1;
    Int4 maxvarc, column_size;

    switch (type) {
        case TS_TYPE_VARCHAR:
        case TS_TYPE_UNKNOWN:
            column_size = tstype_attr_column_size(
                conn, type, atttypmod, TS_ADT_UNSET, handle_unknown_size_as);
            if (SQL_NO_TOTAL == column_size)
                return column_size;
#ifdef UNICODE_SUPPORT
            if (CC_is_in_unicode_driver(conn))
                return column_size * WCLEN;
#endif /* UNICODE_SUPPORT */
            coef = conn->mb_maxbyte_per_char;
            if (coef < 2)
                /* CR -> CR/LF */
                coef = 2;
            if (coef == 1)
                return column_size;
            maxvarc = MAX_VARCHAR_SIZE;
            if (column_size <= maxvarc && column_size * coef > maxvarc)
                return maxvarc;
            return coef * column_size;
        default:
            if (type == (OID)conn->lobj_type)
                return tstype_attr_column_size(conn, type, atttypmod,
                                               TS_ADT_UNSET,
                                               handle_unknown_size_as);
    }
    return -1;
}

/*
 * Casting parameters e.g. ?::timestamp is much more flexible
 * than specifying parameter datatype oids determined by
 * sqltype_to_bind_estype() via parse message.
 */
const char *sqltype_to_escast(const ConnectionClass *conn,
                              SQLSMALLINT fSqlType) {
    const char *esCast = NULL_STRING;

    switch (fSqlType) {
        case SQL_BINARY:
        case SQL_VARBINARY:
            esCast = "::bytea";
            break;
        case SQL_TYPE_DATE:
        case SQL_DATE:
            esCast = "::date";
            break;
        case SQL_DECIMAL:
        case SQL_NUMERIC:
            esCast = "::numeric";
            break;
        case SQL_BIGINT:
            esCast = "::int8";
            break;
        case SQL_INTEGER:
            esCast = "::int4";
            break;
        case SQL_REAL:
            esCast = "::float4";
            break;
        case SQL_SMALLINT:
        case SQL_TINYINT:
            esCast = "::int2";
            break;
        case SQL_TIME:
        case SQL_TYPE_TIME:
            esCast = "::time";
            break;
        case SQL_TIMESTAMP:
        case SQL_TYPE_TIMESTAMP:
            esCast = "::timestamp";
            break;
        case SQL_GUID:
            if (VERSION_GE(conn, 8.3))
                esCast = "::uuid";
            break;
        case SQL_INTERVAL_MONTH:
        case SQL_INTERVAL_YEAR:
        case SQL_INTERVAL_YEAR_TO_MONTH:
        case SQL_INTERVAL_DAY:
        case SQL_INTERVAL_HOUR:
        case SQL_INTERVAL_MINUTE:
        case SQL_INTERVAL_SECOND:
        case SQL_INTERVAL_DAY_TO_HOUR:
        case SQL_INTERVAL_DAY_TO_MINUTE:
        case SQL_INTERVAL_DAY_TO_SECOND:
        case SQL_INTERVAL_HOUR_TO_MINUTE:
        case SQL_INTERVAL_HOUR_TO_SECOND:
        case SQL_INTERVAL_MINUTE_TO_SECOND:
            esCast = "::interval";
            break;
    }

    return esCast;
}

OID sqltype_to_estype(const ConnectionClass *conn, SQLSMALLINT fSqlType) {
    OID esType = 0;
    switch (fSqlType) {
        case SQL_BIT:
            esType = TS_TYPE_BOOLEAN;
            break;

        case SQL_TYPE_DATE:
        case SQL_DATE:
            esType = TS_TYPE_DATE;
            break;

        case SQL_DOUBLE:
        case SQL_FLOAT:
            esType = TS_TYPE_DOUBLE;
            break;

        case SQL_BIGINT:
            esType = TS_TYPE_BIGINT;
            break;

        case SQL_INTEGER:
            esType = TS_TYPE_INTEGER;
            break;

        case SQL_LONGVARBINARY:
            esType = conn->lobj_type;
            break;

        case SQL_LONGVARCHAR:
            esType = TS_TYPE_VARCHAR;
            break;

#ifdef UNICODE_SUPPORT
        case SQL_WLONGVARCHAR:
            esType = TS_TYPE_VARCHAR;
            break;
#endif /* UNICODE_SUPPORT */

        case SQL_SMALLINT:
        case SQL_TINYINT:
            esType = TS_TYPE_INT2;
            break;

        case SQL_TIME:
        case SQL_TYPE_TIME:
            esType = TS_TYPE_TIME;
            break;

        case SQL_VARCHAR:
            esType = TS_TYPE_VARCHAR;
            break;

#ifdef UNICODE_SUPPORT
        case SQL_WVARCHAR:
            esType = TS_TYPE_VARCHAR;
            break;
#endif /* UNICODE_SUPPORT */

    }

    return esType;
}

static int getAtttypmodEtc(const StatementClass *stmt, int col,
                           int *adtsize_or_longestlen) {
    int atttypmod = -1;

    if (NULL != adtsize_or_longestlen)
        *adtsize_or_longestlen = TS_ADT_UNSET;
    if (col >= 0) {
        const QResultClass *res;

        if (res = SC_get_Curres(stmt), NULL != res) {
            atttypmod = QR_get_atttypmod(res, col);
            if (NULL != adtsize_or_longestlen) {
                if (stmt->catalog_result)
                    *adtsize_or_longestlen = QR_get_fieldsize(res, col);
                else {
                    *adtsize_or_longestlen = QR_get_display_size(res, col);
                }
            }
        }
    }
    return atttypmod;
}

/*
 *	There are two ways of calling this function:
 *
 *	1.	When going through the supported TS types (SQLGetTypeInfo)
 *
 *	2.	When taking any type id (SQLColumns, SQLGetData)
 *
 *	The first type will always work because all the types defined are returned
 *here. The second type will return a default based on global parameter when it
 *does not know.	This allows for supporting types that are unknown.  All
 *other ts routines in here return a suitable default.
 */
SQLSMALLINT
estype_to_concise_type(const StatementClass *stmt, OID type, int col,
                       int handle_unknown_size_as) {
    int atttypmod, adtsize_or_longestlen;

    atttypmod = getAtttypmodEtc(stmt, col, &adtsize_or_longestlen);
    return estype_attr_to_concise_type(SC_get_conn(stmt), type, atttypmod,
                                       adtsize_or_longestlen,
                                       handle_unknown_size_as);
}

SQLSMALLINT
estype_to_sqldesctype(const StatementClass *stmt, OID type, int col,
                      int handle_unknown_size_as) {
    int adtsize_or_longestlen;
    int atttypmod = getAtttypmodEtc(stmt, col, &adtsize_or_longestlen);

    return estype_attr_to_sqldesctype(SC_get_conn(stmt), type, atttypmod,
                                      adtsize_or_longestlen,
                                      handle_unknown_size_as);
}

const char *estype_to_name(const StatementClass *stmt, OID type, int col,
                           BOOL auto_increment) {
    int atttypmod = getAtttypmodEtc(stmt, col, NULL);

    return tstype_attr_to_name(SC_get_conn(stmt), type, atttypmod,
                               auto_increment);
}

/*
 *	This corresponds to "precision" in ODBC 2.x.
 *
 *	For TS_TYPE_VARCHAR, SQLColumns will
 *	override this length with the atttypmod length from attribute .
 *
 *	If col >= 0, then will attempt to get the info from the result set.
 *	This is used for functions SQLDescribeCol and SQLColAttributes.
 */
Int4 /* Timestream restriction */
estype_column_size(const StatementClass *stmt, OID type, int col,
                   int handle_unknown_size_as) {
    int atttypmod, adtsize_or_longestlen;

    atttypmod = getAtttypmodEtc(stmt, col, &adtsize_or_longestlen);
    return tstype_attr_column_size(
        SC_get_conn(stmt), type, atttypmod, adtsize_or_longestlen,
        stmt->catalog_result ? UNKNOWNS_AS_LONGEST : handle_unknown_size_as);
}

/*
 *	precision in ODBC 3.x.
 */
SQLSMALLINT
estype_precision(const StatementClass *stmt, OID type, int col) {
    int atttypmod, adtsize_or_longestlen;

    atttypmod = getAtttypmodEtc(stmt, col, &adtsize_or_longestlen);
    return estype_attr_precision(
        SC_get_conn(stmt), type, atttypmod);
}

Int4 estype_display_size(const StatementClass *stmt, OID type, int col,
                         int handle_unknown_size_as) {
    int atttypmod, adtsize_or_longestlen;

    atttypmod = getAtttypmodEtc(stmt, col, &adtsize_or_longestlen);
    return estype_attr_display_size(
        SC_get_conn(stmt), type, atttypmod, adtsize_or_longestlen,
        stmt->catalog_result ? UNKNOWNS_AS_LONGEST : handle_unknown_size_as);
}

/*
 *	The length in bytes of data transferred on an SQLGetData, SQLFetch,
 *	or SQLFetchScroll operation if SQL_C_DEFAULT is specified.
 */
Int4 estype_buffer_length(const StatementClass *stmt, OID type, int col,
                          int handle_unknown_size_as) {
    int atttypmod, adtsize_or_longestlen;

    atttypmod = getAtttypmodEtc(stmt, col, &adtsize_or_longestlen);
    return estype_attr_buffer_length(
        SC_get_conn(stmt), type, atttypmod, adtsize_or_longestlen,
        stmt->catalog_result ? UNKNOWNS_AS_LONGEST : handle_unknown_size_as);
}

/*
 */
Int4 estype_desclength(const StatementClass *stmt, OID type, int col,
                       int handle_unknown_size_as) {
    int atttypmod, adtsize_or_longestlen;

    atttypmod = getAtttypmodEtc(stmt, col, &adtsize_or_longestlen);
    return estype_attr_desclength(
        SC_get_conn(stmt), type, atttypmod, adtsize_or_longestlen,
        stmt->catalog_result ? UNKNOWNS_AS_LONGEST : handle_unknown_size_as);
}

#ifdef NOT_USED
/*
 *	Transfer octet length.
 */
Int4 estype_transfer_octet_length(const StatementClass *stmt, OID type,
                                  int column_size) {
    ConnectionClass *conn = SC_get_conn(stmt);

    int coef = 1;
    Int4 maxvarc;
    switch (type) {
        case TS_TYPE_VARCHAR:
            if (SQL_NO_TOTAL == column_size)
                return column_size;
#ifdef UNICODE_SUPPORT
            if (CC_is_in_unicode_driver(conn))
                return column_size * WCLEN;
#endif /* UNICODE_SUPPORT */
            coef = conn->mb_maxbyte_per_char;
            if (coef < 2 && (conn->connInfo).lf_conversion)
                /* CR -> CR/LF */
                coef = 2;
            if (coef == 1)
                return column_size;
            maxvarc = conn->connInfo.drivers.max_varchar_size;
            if (column_size <= maxvarc && column_size * coef > maxvarc)
                return maxvarc;
            return coef * column_size;
        default:
            if (type == conn->lobj_type)
                return column_size;
    }
    return -1;
}
#endif /* NOT_USED */

/*
 *	corrsponds to "min_scale" in ODBC 2.x.
 */
Int2 tstype_min_decimal_digits(const ConnectionClass *conn, OID type) {
    UNUSED(conn, type);
    switch (type) {
        case TS_TYPE_TIME:
        case TS_TYPE_TIMESTAMP:
            return 9;
        default:
            return -1;
    }
}

/*
 *	corrsponds to "max_scale" in ODBC 2.x.
 */
Int2 tstype_max_decimal_digits(const ConnectionClass *conn, OID type) {
    UNUSED(conn, type);
    switch (type) {
        case TS_TYPE_TIME:
        case TS_TYPE_TIMESTAMP:
            return 9;
        default:
            return -1;
    }
}

/*
 *	corrsponds to "scale" in ODBC 2.x.
 */
Int2 estype_decimal_digits(const StatementClass *stmt, OID type, int col) {
    int atttypmod, adtsize_or_longestlen;

    atttypmod = getAtttypmodEtc(stmt, col, &adtsize_or_longestlen);
    return estype_attr_decimal_digits(SC_get_conn(stmt), type, atttypmod);
}

Int2 tstype_radix(const ConnectionClass *conn, OID type) {
    UNUSED(conn, type);
    switch (type) {
        case TS_TYPE_INTEGER:
        case TS_TYPE_BIGINT:
        case TS_TYPE_DOUBLE:
            return 10;
        case TS_TYPE_VARCHAR:
        case TS_TYPE_DATE:
        case TS_TYPE_TIME:
        case TS_TYPE_TIMESTAMP:
        case TS_TYPE_UNKNOWN:
        default:
            return -1;
    }
}

Int2 tstype_nullable(const ConnectionClass *conn, OID type) {
    UNUSED(conn, type);
    return SQL_NULLABLE; /* everything should be nullable */
}

Int2 tstype_auto_increment(const ConnectionClass *conn, OID type) {
    UNUSED(conn, type);
    return SQL_FALSE;
}

Int2 tstype_case_sensitive(const ConnectionClass *conn, OID type) {
    UNUSED(conn, type);
    return (TS_TYPE_VARCHAR == type) ? SQL_TRUE : SQL_FALSE;
}

Int2 estype_money(const ConnectionClass *conn, OID type) {
    UNUSED(conn, type);
    return SQL_FALSE;
}

Int2 tstype_searchable(const ConnectionClass *conn, OID type) {
    UNUSED(conn, type);
    return SQL_SEARCHABLE;
}

Int2 tstype_unsigned(const ConnectionClass *conn, OID type) {
    UNUSED(conn);
    switch (type) {
        case TS_TYPE_BOOLEAN:
            return SQL_TRUE;
        case TS_TYPE_INT2:
        case TS_TYPE_INTEGER:
        case TS_TYPE_BIGINT:
        case TS_TYPE_DOUBLE:
            return SQL_FALSE;
        default:
            // This is NULL representation in TupleInt2
            return -1;
    }
}

const char *tstype_literal_prefix(const ConnectionClass *conn, OID type) {
    UNUSED(conn);
    switch (type) {
        case TS_TYPE_VARCHAR:
            return "VARCHAR '";
        case TS_TYPE_BIGINT:
            return "BIGINT '";
        case TS_TYPE_DOUBLE:
            return "DOUBLE '";
        case TS_TYPE_INTEGER:
            return "INTEGER '";
        case TS_TYPE_DATE:
            return "DATE '";
        case TS_TYPE_TIME:
            return "TIME '";
        case TS_TYPE_TIMESTAMP:
            return "TIMESTAMP '";
        case TS_TYPE_BOOLEAN:
            return "BOOLEAN '";
        case TS_TYPE_ARRAY:
            return "ARRAY [";
        case TS_TYPE_ROW:
            return "ROW (";
        default:
            return NULL;
    }
}

const char *tstype_literal_suffix(const ConnectionClass *conn, OID type) {
    UNUSED(conn);
    switch (type) {
        case TS_TYPE_VARCHAR:
        case TS_TYPE_BIGINT:
        case TS_TYPE_DOUBLE:
        case TS_TYPE_INTEGER:
        case TS_TYPE_DATE:
        case TS_TYPE_TIME:
        case TS_TYPE_TIMESTAMP:
        case TS_TYPE_BOOLEAN:
            return "'";
        case TS_TYPE_ARRAY:
            return "]";
        case TS_TYPE_ROW:
            return ")";
        default:
            return NULL;
    }
}

SQLSMALLINT
sqltype_to_default_ctype(const ConnectionClass *conn, SQLSMALLINT sqltype) {
    /*
     * from the table on page 623 of ODBC 2.0 Programmer's Reference
     * (Appendix D)
     */
    switch (sqltype) {
        case SQL_CHAR:
        case SQL_VARCHAR:
        case SQL_LONGVARCHAR:
        case SQL_DECIMAL:
        case SQL_NUMERIC:
            return SQL_C_CHAR;
        case SQL_BIGINT:
            return ALLOWED_C_BIGINT;

#ifdef UNICODE_SUPPORT
        case SQL_WCHAR:
        case SQL_WVARCHAR:
        case SQL_WLONGVARCHAR:
            return ansi_to_wtype(conn, SQL_C_CHAR);
#endif /* UNICODE_SUPPORT */

        case SQL_BIT:
            return SQL_C_BIT;

        case SQL_TINYINT:
            return SQL_C_STINYINT;

        case SQL_SMALLINT:
            return SQL_C_SSHORT;

        case SQL_INTEGER:
            return SQL_C_SLONG;

        case SQL_REAL:
            return SQL_C_FLOAT;

        case SQL_FLOAT:
        case SQL_DOUBLE:
            return SQL_C_DOUBLE;

        case SQL_BINARY:
        case SQL_VARBINARY:
        case SQL_LONGVARBINARY:
            return SQL_C_BINARY;

        case SQL_DATE:
            return SQL_C_DATE;

        case SQL_TIME:
            return SQL_C_TIME;

        case SQL_TIMESTAMP:
            return SQL_C_TIMESTAMP;

        case SQL_TYPE_DATE:
            return SQL_C_TYPE_DATE;

        case SQL_TYPE_TIME:
            return SQL_C_TYPE_TIME;

        case SQL_TYPE_TIMESTAMP:
            return SQL_C_TYPE_TIMESTAMP;

        case SQL_GUID:
            if (conn->ms_jet)
                return SQL_C_CHAR;
            else
                return SQL_C_GUID;

        default:
            /* should never happen */
            return SQL_C_CHAR;
    }
}

Int4 ctype_length(SQLSMALLINT ctype) {
    switch (ctype) {
        case SQL_C_SSHORT:
        case SQL_C_SHORT:
            return sizeof(SWORD);

        case SQL_C_USHORT:
            return sizeof(UWORD);

        case SQL_C_SLONG:
        case SQL_C_LONG:
            return sizeof(SDWORD);

        case SQL_C_ULONG:
            return sizeof(UDWORD);

        case SQL_C_FLOAT:
            return sizeof(SFLOAT);

        case SQL_C_DOUBLE:
            return sizeof(SDOUBLE);

        case SQL_C_BIT:
            return sizeof(UCHAR);

        case SQL_C_STINYINT:
        case SQL_C_TINYINT:
            return sizeof(SCHAR);

        case SQL_C_UTINYINT:
            return sizeof(UCHAR);

        case SQL_C_DATE:
        case SQL_C_TYPE_DATE:
            return sizeof(DATE_STRUCT);

        case SQL_C_TIME:
        case SQL_C_TYPE_TIME:
            return sizeof(TIME_STRUCT);

        case SQL_C_TIMESTAMP:
        case SQL_C_TYPE_TIMESTAMP:
            return sizeof(TIMESTAMP_STRUCT);

        case SQL_C_GUID:
            return sizeof(SQLGUID);
        case SQL_C_INTERVAL_YEAR:
        case SQL_C_INTERVAL_MONTH:
        case SQL_C_INTERVAL_YEAR_TO_MONTH:
        case SQL_C_INTERVAL_DAY:
        case SQL_C_INTERVAL_HOUR:
        case SQL_C_INTERVAL_DAY_TO_HOUR:
        case SQL_C_INTERVAL_MINUTE:
        case SQL_C_INTERVAL_DAY_TO_MINUTE:
        case SQL_C_INTERVAL_HOUR_TO_MINUTE:
        case SQL_C_INTERVAL_SECOND:
        case SQL_C_INTERVAL_DAY_TO_SECOND:
        case SQL_C_INTERVAL_HOUR_TO_SECOND:
        case SQL_C_INTERVAL_MINUTE_TO_SECOND:
            return sizeof(SQL_INTERVAL_STRUCT);
        case SQL_C_NUMERIC:
            return sizeof(SQL_NUMERIC_STRUCT);
        case SQL_C_SBIGINT:
        case SQL_C_UBIGINT:
            return sizeof(SQLBIGINT);

        case SQL_C_BINARY:
        case SQL_C_CHAR:
#ifdef UNICODE_SUPPORT
        case SQL_C_WCHAR:
#endif /* UNICODE_SUPPORT */
            return 0;

        default: /* should never happen */
            return 0;
    }
}
