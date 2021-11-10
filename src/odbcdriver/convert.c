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

#include "convert.h"

#include "misc.h"
#include "unicode_support.h"
#ifdef WIN32
#include <float.h>
#define HAVE_LOCALE_H
#endif /* WIN32 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "multibyte.h"
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <errno.h>

#include "apifunc.h"
#include "bind.h"
#include "catfunc.h"
#include "connection.h"
#include "statement.h"
#include "types.h"

CSTR INFINITY_STRING = "Infinity";
CSTR MINFINITY_STRING = "-Infinity";

#if defined(WIN32) || defined(__CYGWIN__)
#define TIMEZONE_GLOBAL _timezone
#define TZNAME_GLOBAL _tzname
#define DAYLIGHT_GLOBAL _daylight
#elif defined(HAVE_INT_TIMEZONE)
#define TIMEZONE_GLOBAL timezone
#define TZNAME_GLOBAL tzname
#define DAYLIGHT_GLOBAL daylight
#endif

typedef struct {
    int infinity;
    int m;
    int d;
    int y;
    int hh;
    int mm;
    int ss;
    int fr;
} SIMPLE_TIME;

size_t convert_linefeeds(const char *s, char *dst, size_t max, BOOL convlf,
                         BOOL *changed);
static size_t convert_from_esbinary(const char *value, char *rgbValue,
                                    SQLLEN cbValueMax);
static int convert_lo(StatementClass *stmt, const void *value,
                      SQLSMALLINT fCType, PTR rgbValue, SQLLEN cbValueMax,
                      SQLLEN *pcbValue);
static int conv_from_octal(const char *s);
static SQLLEN es_bin2hex(const char *src, char *dst, SQLLEN length);
#ifdef UNICODE_SUPPORT
static SQLLEN es_bin2whex(const char *src, SQLWCHAR *dst, SQLLEN length);
#endif /* UNICODE_SUPPORT */

/*---------
 *			A Guide for date/time/timestamp conversions
 *
 *			field_type		fCType				Output
 *			----------		------				----------
 *			DB_TYPE_DATE	SQL_C_DEFAULT		SQL_C_DATE
 *			DB_TYPE_DATE	SQL_C_DATE			SQL_C_DATE
 *			DB_TYPE_DATE	SQL_C_TIMESTAMP		SQL_C_TIMESTAMP		(time = 0 (midnight)) 
 *          DB_TYPE_TIME	SQL_C_DEFAULT		SQL_C_TIME 
 *          DB_TYPE_TIME    SQL_C_TIME			SQL_C_TIME
 *			DB_TYPE_TIME	SQL_C_TIMESTAMP		SQL_C_TIMESTAMP		(date = current date) 
 *---------
 */

static void parse_to_numeric_struct(const char *wv, SQL_NUMERIC_STRUCT *ns,
                                    BOOL *overflow);

/*
 *	TIMESTAMP <-----> SIMPLE_TIME
 *		precision support since 7.2.
 *		time zone support is unavailable(the stuff is unreliable)
 */
static BOOL timestamp2stime(const char *str, SIMPLE_TIME *st, BOOL *bZone,
                            int *zone) {
    char rest[64], bc[16], *ptr;
    int scnt, i;
    int y, m, d, hh, mm, ss;
#ifdef TIMEZONE_GLOBAL
    long timediff;
#endif
    BOOL withZone = *bZone;

    *bZone = FALSE;
    *zone = 0;
    st->fr = 0;
    st->infinity = 0;
    rest[0] = '\0';
    bc[0] = '\0';
    if ((scnt = sscanf(str, "%4d-%2d-%2d %2d:%2d:%2d%31s %15s", &y, &m, &d, &hh,
                       &mm, &ss, rest, bc))
        < 6) {
        if (scnt == 3) /* date */
        {
            st->y = y;
            st->m = m;
            st->d = d;
            st->hh = 0;
            st->mm = 0;
            st->ss = 0;
            return TRUE;
        }
        if ((scnt =
                 sscanf(str, "%2d:%2d:%2d%31s %15s", &hh, &mm, &ss, rest, bc))
            < 3)
            return FALSE;
        else {
            st->hh = hh;
            st->mm = mm;
            st->ss = ss;
            if (scnt == 3) /* time */
                return TRUE;
        }
    } else {
        st->y = y;
        st->m = m;
        st->d = d;
        st->hh = hh;
        st->mm = mm;
        st->ss = ss;
        if (scnt == 6)
            return TRUE;
    }
    switch (rest[0]) {
        case '+':
            *bZone = TRUE;
            *zone = atoi(&rest[1]);
            break;
        case '-':
            *bZone = TRUE;
            *zone = -atoi(&rest[1]);
            break;
        case '.':
            if ((ptr = strchr(rest, '+')) != NULL) {
                *bZone = TRUE;
                *zone = atoi(&ptr[1]);
                *ptr = '\0';
            } else if ((ptr = strchr(rest, '-')) != NULL) {
                *bZone = TRUE;
                *zone = -atoi(&ptr[1]);
                *ptr = '\0';
            }
            for (i = 1; i < 10; i++) {
                if (!isdigit((UCHAR)rest[i]))
                    break;
            }
            for (; i < 10; i++)
                rest[i] = '0';
            rest[i] = '\0';
            st->fr = atoi(&rest[1]);
            break;
        case 'B':
            if (stricmp(rest, "BC") == 0)
                st->y *= -1;
            return TRUE;
        default:
            return TRUE;
    }
    if (stricmp(bc, "BC") == 0) {
        st->y *= -1;
    }
    if (!withZone || !*bZone || st->y < 1970)
        return TRUE;
#ifdef TIMEZONE_GLOBAL
    if (!TZNAME_GLOBAL[0] || !TZNAME_GLOBAL[0][0]) {
        *bZone = FALSE;
        return TRUE;
    }
    timediff = TIMEZONE_GLOBAL + (*zone) * 3600;
    if (!DAYLIGHT_GLOBAL && timediff == 0) /* the same timezone */
        return TRUE;
    else {
        struct tm tm, *tm2;
        time_t time0;

        *bZone = FALSE;
        tm.tm_year = st->y - 1900;
        tm.tm_mon = st->m - 1;
        tm.tm_mday = st->d;
        tm.tm_hour = st->hh;
        tm.tm_min = st->mm;
        tm.tm_sec = st->ss;
        tm.tm_isdst = -1;
        time0 = mktime(&tm);
        if (time0 < 0)
            return TRUE;
        if (tm.tm_isdst > 0)
            timediff -= 3600;
        if (timediff == 0) /* the same time zone */
            return TRUE;
        time0 -= timediff;
#ifdef HAVE_LOCALTIME_R
        if (time0 >= 0 && (tm2 = localtime_r(&time0, &tm)) != NULL)
#else
        if (time0 >= 0 && (tm2 = localtime(&time0)) != NULL)
#endif /* HAVE_LOCALTIME_R */
        {
            st->y = tm2->tm_year + 1900;
            st->m = tm2->tm_mon + 1;
            st->d = tm2->tm_mday;
            st->hh = tm2->tm_hour;
            st->mm = tm2->tm_min;
            st->ss = tm2->tm_sec;
            *bZone = TRUE;
        }
    }
#endif /* TIMEZONE_GLOBAL */
    return TRUE;
}

BOOL valid_datetime_format(const char *str) {
    // yyyy-mm-dd hh:mm:ss.nnnnnnnnn
    int sz = (int)strlen(str);
    if (sz > 29)
        return FALSE;
    if (sz == 7 || sz == 10 || sz == 13 || sz == 16 || sz == 19 || sz > 20) {
        for (int i = 0; i < sz; i++) {
            if (i == 4 || i == 7) {
                if (str[i] != '-')
                    return FALSE;
            } else if (i == 10) {
                if (str[i] != ' ')
                    return FALSE;
            } else if (i == 13 || i == 16) {
                if (str[i] != ':')
                    return FALSE;
            } else if (i == 19) {
                if (str[i] != '.')
                    return FALSE;
            } else if (!isdigit(str[i])) {
                return FALSE;
            }
        }
        return TRUE;
    }
    return FALSE;
}

BOOL valid_time_format(const char *str) {
    // hh:mm:ss.nnnnnnnnn
    int sz = (int)strlen(str);
    if (sz > 18)
        return FALSE;
    if (sz == 5 || sz == 8 || sz > 9) {
        for (int i = 0; i < sz; i++) {
            if (i == 2 || i == 5) {
                if (str[i] != ':')
                    return FALSE;
            } else if (i == 8) {
                if (str[i] != '.')
                    return FALSE;
            } else if (!isdigit(str[i])) {
                return FALSE;
            }
        }
        return TRUE;
    }
    return FALSE;
}

static BOOL char2stime(const char *str, SIMPLE_TIME *st, int *errcode,
                       const SQLSMALLINT fCType) {
    char rest[64];
    int scnt, i;
    int y, m, d, hh, mm, ss;
    st->fr = 0;
    st->infinity = 0;
    rest[0] = '\0';

    // time hh:mm:ss[.nnnnnnnnn]
    if (valid_time_format(str)) {
        if (fCType == SQL_C_DATE || fCType == SQL_C_TYPE_DATE) {
            *errcode = COPY_INVALID_STRING_CONVERSION;
            return FALSE;
        }
        scnt = sscanf(str, "%2d:%2d:%2d%10s", &hh, &mm, &ss, rest);
        if (scnt >= 3) {
            st->hh = hh;
            st->mm = mm;
            st->ss = ss;
            if (scnt == 3)
                return TRUE;
        } else if (scnt == 2) {
            st->hh = hh;
            st->mm = mm;
            st->ss = 0;
            return TRUE;
        } else {
            *errcode = COPY_INVALID_STRING_CONVERSION;
            return FALSE;
        }
        if (rest[0] != '.') {
            *errcode = COPY_INVALID_STRING_CONVERSION;
            return FALSE;
        } else {
            for (i = 1; i < 10; i++) {
                if (!isdigit((UCHAR)rest[i]))
                    break;
            }
            for (; i < 10; i++)
                rest[i] = '0';
            rest[i] = '\0';
            st->fr = atoi(&rest[1]);
            // Fractional seconds portion is nonzero
            if ((fCType == SQL_C_TIME || fCType == SQL_C_TYPE_TIME)
                && st->fr != 0) {
                *errcode = COPY_RESULT_FRACTIONAL_TRUNCATED;
                return TRUE;
            }
        }
        return TRUE;
    }

    // timestamp and date
    if (valid_datetime_format(str)) {
        scnt = sscanf(str, "%4d-%2d-%2d %2d:%2d:%2d%10s", &y, &m, &d, &hh, &mm,
                      &ss, rest);
        if (scnt >= 6) {
            st->y = y;
            st->m = m;
            st->d = d;
            st->hh = hh;
            st->mm = mm;
            st->ss = ss;
            // Time portion of timestamp is nonzero
            if ((fCType == SQL_C_DATE || fCType == SQL_C_TYPE_DATE)
                && (ss != 0 || mm != 0 || hh != 0)) {
                *errcode = COPY_RESULT_FRACTIONAL_TRUNCATED;
            }
            if (scnt == 6)
                return TRUE;
        } else if (scnt == 5) {
            st->y = y;
            st->m = m;
            st->d = d;
            st->hh = hh;
            st->mm = mm;
            st->ss = 0;
            // Time portion of timestamp is nonzero
            if ((fCType == SQL_C_DATE || fCType == SQL_C_TYPE_DATE)
                && (mm != 0 || hh != 0)) {
                *errcode = COPY_RESULT_FRACTIONAL_TRUNCATED;
            }
            return TRUE;
        } else if (scnt == 4) {
            st->y = y;
            st->m = m;
            st->d = d;
            st->hh = hh;
            st->mm = 0;
            st->ss = 0;
            // Time portion of timestamp is nonzero
            if ((fCType == SQL_C_DATE || fCType == SQL_C_TYPE_DATE)
                && hh != 0) {
                *errcode = COPY_RESULT_FRACTIONAL_TRUNCATED;
            }
            return TRUE;
        } else if (scnt == 3) {
            if (fCType == SQL_C_TIME || fCType == SQL_C_TYPE_TIME) {
                *errcode = COPY_INVALID_STRING_CONVERSION;
                return FALSE;
            }
            st->y = y;
            st->m = m;
            st->d = d;
            st->hh = 0;
            st->mm = 0;
            st->ss = 0;
            return TRUE;
        } else if (scnt == 2) {
            if (fCType == SQL_C_TIME || fCType == SQL_C_TYPE_TIME) {
                *errcode = COPY_INVALID_STRING_CONVERSION;
                return FALSE;
            }
            st->y = y;
            st->m = m;
            st->d = 1;
            st->hh = 0;
            st->mm = 0;
            st->ss = 0;
            return TRUE;
        } else {
            *errcode = COPY_INVALID_STRING_CONVERSION;
            return FALSE;
        }
        if (rest[0] != '.') {
            *errcode = COPY_INVALID_STRING_CONVERSION;
            return FALSE;
        } else {
            for (i = 1; i < 10; i++) {
                if (!isdigit((UCHAR)rest[i]))
                    break;
            }
            for (; i < 10; i++)
                rest[i] = '0';
            rest[i] = '\0';
            st->fr = atoi(&rest[1]);
            // Fractional seconds portion is nonzero
            if ((fCType == SQL_C_TIME || fCType == SQL_C_TYPE_TIME)
                && st->fr != 0) {
                *errcode = COPY_RESULT_FRACTIONAL_TRUNCATED;
                return TRUE;
            }
        }
        return TRUE;
    }

    // no match
    *errcode = COPY_INVALID_STRING_CONVERSION;
    return FALSE;
}

static int stime2timestamp(const SIMPLE_TIME *st, char *str, size_t bufsize,
                           BOOL bZone, int precision) {
    UNUSED(bZone);
    char precstr[16] = {0};
    char zonestr[16] = {0};

    precstr[0] = '\0';
    if (st->infinity > 0) {
        return snprintf(str, bufsize, "%s", INFINITY_STRING);
    } else if (st->infinity < 0) {
        return snprintf(str, bufsize, "%s", MINFINITY_STRING);
    }
    if (precision > 0) {
        SPRINTF_FIXED(precstr, ".%09d", st->fr);
        if (precision < 9)
            precstr[precision + 1] = '\0';
        else if (precision > 9)
            precision = 9;
    }
    zonestr[0] = '\0';
#ifdef TIMEZONE_GLOBAL
    if (bZone && TZNAME_GLOBAL[0] && TZNAME_GLOBAL[0][0] && st->y >= 1970) {
        long zoneint;
        struct tm tm;
        time_t time0;

        zoneint = TIMEZONE_GLOBAL;
        if (DAYLIGHT_GLOBAL && st->y >= 1900) {
            tm.tm_year = st->y - 1900;
            tm.tm_mon = st->m - 1;
            tm.tm_mday = st->d;
            tm.tm_hour = st->hh;
            tm.tm_min = st->mm;
            tm.tm_sec = st->ss;
            tm.tm_isdst = -1;
            time0 = mktime(&tm);
            if (time0 >= 0 && tm.tm_isdst > 0)
                zoneint -= 3600;
        }
        if (zoneint > 0)
            SPRINTF_FIXED(zonestr, "-%02d", (int)zoneint / 3600);
        else
            SPRINTF_FIXED(zonestr, "+%02d", -(int)zoneint / 3600);
    }
#endif /* TIMEZONE_GLOBAL */
    if (st->y < 0)
        return snprintf(str, bufsize, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d%s%s BC",
                        -st->y, st->m, st->d, st->hh, st->mm, st->ss, precstr,
                        zonestr);
    else
        return snprintf(str, bufsize, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d%s%s",
                        st->y, st->m, st->d, st->hh, st->mm, st->ss, precstr,
                        zonestr);
}

static SQLINTERVAL interval2itype(SQLSMALLINT ctype) {
    SQLINTERVAL sqlitv = 0;

    switch (ctype) {
        case SQL_C_INTERVAL_YEAR:
            sqlitv = SQL_IS_YEAR;
            break;
        case SQL_C_INTERVAL_MONTH:
            sqlitv = SQL_IS_MONTH;
            break;
        case SQL_C_INTERVAL_YEAR_TO_MONTH:
            sqlitv = SQL_IS_YEAR_TO_MONTH;
            break;
        case SQL_C_INTERVAL_DAY:
            sqlitv = SQL_IS_DAY;
            break;
        case SQL_C_INTERVAL_HOUR:
            sqlitv = SQL_IS_HOUR;
            break;
        case SQL_C_INTERVAL_DAY_TO_HOUR:
            sqlitv = SQL_IS_DAY_TO_HOUR;
            break;
        case SQL_C_INTERVAL_MINUTE:
            sqlitv = SQL_IS_MINUTE;
            break;
        case SQL_C_INTERVAL_DAY_TO_MINUTE:
            sqlitv = SQL_IS_DAY_TO_MINUTE;
            break;
        case SQL_C_INTERVAL_HOUR_TO_MINUTE:
            sqlitv = SQL_IS_HOUR_TO_MINUTE;
            break;
        case SQL_C_INTERVAL_SECOND:
            sqlitv = SQL_IS_SECOND;
            break;
        case SQL_C_INTERVAL_DAY_TO_SECOND:
            sqlitv = SQL_IS_DAY_TO_SECOND;
            break;
        case SQL_C_INTERVAL_HOUR_TO_SECOND:
            sqlitv = SQL_IS_HOUR_TO_SECOND;
            break;
        case SQL_C_INTERVAL_MINUTE_TO_SECOND:
            sqlitv = SQL_IS_MINUTE_TO_SECOND;
            break;
    }
    return sqlitv;
}

/*
 *	Interval data <-----> SQL_INTERVAL_STRUCT
 */

static int getPrecisionPart(int precision, const char *precPart) {
    char fraction[] = "000000000";
    size_t fracs = (size_t)(sizeof(fraction) - 1);
    size_t cpys;

    if (precision < 0)
        precision = 6; /* default */
    if (precision == 0)
        return 0;
    cpys = strlen(precPart);
    if (cpys > fracs)
        cpys = fracs;
    memcpy(fraction, precPart, cpys);
    fraction[precision] = '\0';

    return atoi(fraction);
}

static BOOL interval2istruct(SQLSMALLINT ctype, int precision, const char *str,
                             SQL_INTERVAL_STRUCT *st) {
    char lit1[64], lit2[64];
    int scnt, years, mons, days, hours, minutes, seconds;
    SQLSMALLINT sign;
    SQLINTERVAL itype = interval2itype(ctype);

    memset(st, 0, sizeof(SQL_INTERVAL_STRUCT));
    if ((scnt = sscanf(str, "%d-%d", &years, &mons)) >= 2) {
        if (SQL_IS_YEAR_TO_MONTH == itype) {
            sign = str[0] == '-' ? SQL_TRUE : SQL_FALSE;
            st->interval_type = itype;
            st->interval_sign = sign;
            st->intval.year_month.year = sign ? (-years) : years;
            st->intval.year_month.month = mons;
            return TRUE;
        }
        return FALSE;
    } else if (scnt = sscanf(str, "%d %02d:%02d:%02d.%09s", &days, &hours,
                             &minutes, &seconds, lit2),
               5 == scnt || 4 == scnt) {
        sign = str[0] == '-' ? SQL_TRUE : SQL_FALSE;
        st->interval_type = itype;
        st->interval_sign = sign;
        st->intval.day_second.day = sign ? (-days) : days;
        st->intval.day_second.hour = hours;
        st->intval.day_second.minute = minutes;
        st->intval.day_second.second = seconds;
        if (scnt > 4)
            st->intval.day_second.fraction = getPrecisionPart(precision, lit2);
        return TRUE;
    } else if ((scnt =
                    sscanf(str, "%d %10s %d %10s", &years, lit1, &mons, lit2))
               >= 4) {
        if (strnicmp(lit1, "year", 4) == 0 && strnicmp(lit2, "mon", 2) == 0
            && (SQL_IS_MONTH == itype || SQL_IS_YEAR_TO_MONTH == itype)) {
            sign = years < 0 ? SQL_TRUE : SQL_FALSE;
            st->interval_type = itype;
            st->interval_sign = sign;
            st->intval.year_month.year = sign ? (-years) : years;
            st->intval.year_month.month = sign ? (-mons) : mons;
            return TRUE;
        }
        return FALSE;
    }
    if ((scnt = sscanf(str, "%d %10s %d", &years, lit1, &days)) == 2) {
        sign = years < 0 ? SQL_TRUE : SQL_FALSE;
        if (SQL_IS_YEAR == itype
            && (stricmp(lit1, "year") == 0 || stricmp(lit1, "years") == 0)) {
            st->interval_type = itype;
            st->interval_sign = sign;
            st->intval.year_month.year = sign ? (-years) : years;
            return TRUE;
        }
        if (SQL_IS_MONTH == itype
            && (stricmp(lit1, "mon") == 0 || stricmp(lit1, "mons") == 0)) {
            st->interval_type = itype;
            st->interval_sign = sign;
            st->intval.year_month.month = sign ? (-years) : years;
            return TRUE;
        }
        if (SQL_IS_DAY == itype
            && (stricmp(lit1, "day") == 0 || stricmp(lit1, "days") == 0)) {
            st->interval_type = itype;
            st->interval_sign = sign;
            st->intval.day_second.day = sign ? (-years) : years;
            return TRUE;
        }
        return FALSE;
    }
    if (itype == SQL_IS_YEAR || itype == SQL_IS_MONTH
        || itype == SQL_IS_YEAR_TO_MONTH) {
        /* these formats should've been handled above already */
        return FALSE;
    }
    scnt = sscanf(str, "%d %10s %02d:%02d:%02d.%09s", &days, lit1, &hours,
                  &minutes, &seconds, lit2);
    if (scnt == 5 || scnt == 6) {
        if (strnicmp(lit1, "day", 3) != 0)
            return FALSE;
        sign = days < 0 ? SQL_TRUE : SQL_FALSE;

        st->interval_type = itype;
        st->interval_sign = sign;
        st->intval.day_second.day = sign ? (-days) : days;
        st->intval.day_second.hour = sign ? (-hours) : hours;
        st->intval.day_second.minute = minutes;
        st->intval.day_second.second = seconds;
        if (scnt > 5)
            st->intval.day_second.fraction = getPrecisionPart(precision, lit2);
        return TRUE;
    }
    scnt = sscanf(str, "%02d:%02d:%02d.%09s", &hours, &minutes, &seconds, lit2);
    if (scnt == 3 || scnt == 4) {
        sign = hours < 0 ? SQL_TRUE : SQL_FALSE;

        st->interval_type = itype;
        st->interval_sign = sign;
        st->intval.day_second.hour = sign ? (-hours) : hours;
        st->intval.day_second.minute = minutes;
        st->intval.day_second.second = seconds;
        if (scnt > 3)
            st->intval.day_second.fraction = getPrecisionPart(precision, lit2);
        return TRUE;
    }

    return FALSE;
}

#ifdef HAVE_LOCALE_H
/*
 * Get the decimal point of the current locale.
 *
 * XXX: This isn't thread-safe, if another thread changes the locale with
 * setlocale() concurrently. There are two problems with that:
 *
 * 1. The pointer returned by localeconv(), or the lc->decimal_point string,
 * might be invalidated by calls in other threads. Until someone comes up
 * with a thread-safe version of localeconv(), there isn't much we can do
 * about that. (libc implementations that return a static buffer (like glibc)
 * happen to be safe from the lconv struct being invalidated, but the
 * decimal_point string might still not point to a static buffer).
 *
 * 2. The between the call to sprintf() and get_current_decimal_point(), the
 * decimal point might change. That would cause set_server_decimal_point()
 * to fail to recognize a decimal separator, and we might send a numeric
 * string to the server that the server won't recognize. This would cause
 * the query to fail in the server.
 *
 * XXX: we only take into account the first byte of the decimal separator.
 */
static char get_current_decimal_point(void) {
    struct lconv *lc = localeconv();

    return lc->decimal_point[0];
}

/*
 * Inverse of set_server_decimal_point.
 */
static void set_client_decimal_point(char *num) {
    char current_decimal_point = get_current_decimal_point();
    char *str;

    if ('.' == current_decimal_point)
        return;
    for (str = num; '\0' != *str; str++) {
        if (*str == '.') {
            *str = current_decimal_point;
            break;
        }
    }
}
#else
static void set_client_decimal_point(char *num) {
    UNUSED(num);
}
#endif /* HAVE_LOCALE_H */

/*	This is called by SQLFetch() */
int copy_and_convert_field_bindinfo(StatementClass *stmt, OID field_type,
                                    int atttypmod, void *value, int col) {
    ARDFields *opts = SC_get_ARDF(stmt);
    BindInfoClass *bic;
    SQLULEN offset = opts->row_offset_ptr ? *opts->row_offset_ptr : 0;

    if (opts->allocated <= col)
        extend_column_bindings(opts, (SQLSMALLINT)(col + 1));
    bic = &(opts->bindings[col]);
    SC_set_current_col(stmt, -1);
    return copy_and_convert_field(stmt, field_type, atttypmod, value,
                                  bic->returntype, bic->precision,
                                  (PTR)(bic->buffer + offset), bic->buflen,
                                  LENADDR_SHIFT(bic->used, offset),
                                  LENADDR_SHIFT(bic->indicator, offset));
}

static int char2guid(const char *str, SQLGUID *g) {
    /*
     * SQLGUID.Data1 is an "unsigned long" on some platforms, and
     * "unsigned int" on others. For format "%08X", it should be an
     * "unsigned int", so use a temporary variable for it.
     */
    unsigned int Data1;
    if (sscanf(str,
               "%08X-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%"
               "02hhX",
               &Data1, &g->Data2, &g->Data3, &g->Data4[0], &g->Data4[1],
               &g->Data4[2], &g->Data4[3], &g->Data4[4], &g->Data4[5],
               &g->Data4[6], &g->Data4[7])
        < 11)
        return COPY_GENERAL_ERROR;
    g->Data1 = Data1;
    return COPY_OK;
}

static int get_terminator_len(SQLSMALLINT fCType) {
    switch (fCType) {
#ifdef UNICODE_SUPPORT
        case SQL_C_WCHAR:
            return WCLEN;
#endif /* UNICODE_SUPPORT */
        case SQL_C_BINARY:
            return 0;
    }

    /* SQL_C_CHAR or INTERNAL_ASIS_TYPE */
    return 1;
}

static SQLLEN get_adjust_len(SQLSMALLINT fCType, SQLLEN len) {
    switch (fCType) {
#ifdef UNICODE_SUPPORT
        case SQL_C_WCHAR:
            return (len / WCLEN) * WCLEN;
#endif /* UNICODE_SUPPORT */
    }

    return len;
}

#define BYTEA_PROCESS_ESCAPE 1
#define BYTEA_PROCESS_BINARY 2

static int setup_getdataclass(SQLLEN *const length_return,
                              const char **const ptr_return,
                              int *needbuflen_return, GetDataClass *const esdc,
                              const char *neut_str, const OID field_type,
                              const SQLSMALLINT fCType, const SQLLEN cbValueMax,
                              const ConnectionClass *const conn) {
    SQLLEN len = (-2);
    const char *ptr = NULL;
    int needbuflen = 0;
    int result = COPY_OK;

    BOOL lf_conv = 0;
    int bytea_process_kind = 0;
    BOOL already_processed = FALSE;
    BOOL changed = FALSE;
    int len_for_wcs_term = 0;

#ifdef UNICODE_SUPPORT
    char *allocbuf = NULL;
    int unicode_count = -1;
    BOOL localize_needed = FALSE;
    BOOL hybrid = FALSE;
#endif /* UNICODE_SUPPORT */

#ifdef UNICODE_SUPPORT
    if (0 == bytea_process_kind) {
        if (get_convtype()
            > 0) /* coversion between the current locale is available */
        {
            BOOL wcs_debug = 0;
            BOOL same_encoding =
                (conn->ccsc == es_CS_code(conn->locale_encoding));
            BOOL is_utf8 = (UTF8 == conn->ccsc);

            switch (field_type) {
                case DB_TYPE_UNKNOWN:
                case DB_TYPE_VARCHAR:
                    if (SQL_C_CHAR == fCType || SQL_C_BINARY == fCType)
                        localize_needed = (!same_encoding || wcs_debug);
                    if (SQL_C_WCHAR == fCType)
                        hybrid = (!is_utf8 || (same_encoding && wcs_debug));
            }
            MYLOG(LOG_DEBUG,
                  "localize=%d hybrid=%d is_utf8=%d same_encoding=%d "
                  "wcs_debug=%d",
                  localize_needed, hybrid, is_utf8, same_encoding, wcs_debug);
        }
    }
    if (fCType == SQL_C_WCHAR) {
        if (BYTEA_PROCESS_ESCAPE == bytea_process_kind)
            unicode_count = (int)convert_from_esbinary(neut_str, NULL, 0) * 2;
        else if (hybrid) {
            MYLOG(LOG_DEBUG, "hybrid estimate");
            if ((unicode_count =
                     (int)bindcol_hybrid_estimate(neut_str, lf_conv, &allocbuf))
                < 0) {
                result = COPY_INVALID_STRING_CONVERSION;
                goto cleanup;
            }
        } else /* normally */
        {
            unicode_count = (int)utf8_to_ucs2_lf(neut_str, SQL_NTS, lf_conv,
                                                 NULL, 0, FALSE);
        }
        len = WCLEN * unicode_count;
        already_processed = changed = TRUE;
    } else if (localize_needed) {
        if ((len = bindcol_localize_estimate(neut_str, lf_conv, &allocbuf))
            < 0) {
            result = COPY_INVALID_STRING_CONVERSION;
            goto cleanup;
        }
        already_processed = changed = TRUE;
    }
#endif /* UNICODE_SUPPORT */

    if (already_processed) /* skip */
        ;
    else if (0 != bytea_process_kind) {
        len = convert_from_esbinary(neut_str, NULL, 0);
        if (BYTEA_PROCESS_BINARY != bytea_process_kind)
            len *= 2;
        changed = TRUE;
    } else
        /* convert linefeeds to carriage-return/linefeed */
        len = convert_linefeeds(neut_str, NULL, 0, lf_conv, &changed);

    /* just returns length info */
    if (cbValueMax == 0) {
        result = COPY_RESULT_STRING_TRUNCATED;
        goto cleanup;
    }

    if (!esdc->ttlbuf)
        esdc->ttlbuflen = 0;
    needbuflen = (int)len + get_terminator_len(fCType);
    if (SQL_C_BINARY == fCType) {
        /*
         * Though Binary doesn't have NULL terminator,
         * bindcol_localize_exec() needs output buffer
         * for NULL terminator.
         */
        len_for_wcs_term = 1;
    }
    if (changed || needbuflen > cbValueMax) {
        if (needbuflen > (SQLLEN)esdc->ttlbuflen) {
            esdc->ttlbuf = realloc(esdc->ttlbuf, needbuflen + len_for_wcs_term);
            esdc->ttlbuflen = needbuflen;
        }

        already_processed = FALSE;
#ifdef UNICODE_SUPPORT
        if (fCType == SQL_C_WCHAR) {
            if (BYTEA_PROCESS_ESCAPE == bytea_process_kind) {
                len = convert_from_esbinary(neut_str, esdc->ttlbuf,
                                            esdc->ttlbuflen);
                len = es_bin2whex(esdc->ttlbuf, (SQLWCHAR *)esdc->ttlbuf, len);
            } else {
                if (!hybrid) /* normally */
                    utf8_to_ucs2_lf(neut_str, SQL_NTS, lf_conv,
                                    (SQLWCHAR *)esdc->ttlbuf, unicode_count,
                                    FALSE);
                else /* hybrid */
                {
                    MYLOG(LOG_DEBUG, "hybrid convert");
                    if (bindcol_hybrid_exec((SQLWCHAR *)esdc->ttlbuf, neut_str,
                                            unicode_count + 1, lf_conv,
                                            &allocbuf)
                        < 0) {
                        result = COPY_INVALID_STRING_CONVERSION;
                        goto cleanup;
                    }
                }
            }
            already_processed = TRUE;
        } else if (localize_needed) {
            if (bindcol_localize_exec(esdc->ttlbuf, len + 1, lf_conv, &allocbuf)
                < 0) {
                result = COPY_INVALID_STRING_CONVERSION;
                goto cleanup;
            }
            already_processed = TRUE;
        }
#endif /* UNICODE_SUPPORT */

        if (already_processed)
            ;
        else if (0 != bytea_process_kind) {
            len =
                convert_from_esbinary(neut_str, esdc->ttlbuf, esdc->ttlbuflen);
            if (BYTEA_PROCESS_ESCAPE == bytea_process_kind)
                len = es_bin2hex(esdc->ttlbuf, esdc->ttlbuf, len);
        } else
            convert_linefeeds(neut_str, esdc->ttlbuf, esdc->ttlbuflen, lf_conv,
                              &changed);
        ptr = esdc->ttlbuf;
        esdc->ttlbufused = len;
    } else {
        if (esdc->ttlbuf) {
            free(esdc->ttlbuf);
            esdc->ttlbuf = NULL;
        }
        ptr = neut_str;
    }
cleanup:
#ifdef UNICODE_SUPPORT
    if (allocbuf)
        free(allocbuf);
#endif /* UNICODE_SUPPORT */

    *length_return = len;
    *ptr_return = ptr;
    *needbuflen_return = needbuflen;

    return result;
}

/*
    gdata		SC_get_GDTI(stmt)
    current_col	stmt->current_col
 */

/*
 *	fCType treated in the following function is
 *
 *	SQL_C_CHAR, SQL_C_BINARY, SQL_C_WCHAR or INTERNAL_ASIS_TYPE
 */
static int convert_text_field_to_sql_c(
    GetDataInfo *const gdata, const int current_col, const char *const neut_str,
    const OID field_type, const SQLSMALLINT fCType, char *const rgbValueBindRow,
    const SQLLEN cbValueMax, const ConnectionClass *const conn,
    SQLLEN *const length_return) {
    int result = COPY_OK;
    SQLLEN len = (-2);
    GetDataClass *esdc;
    int copy_len = 0, needbuflen = 0, i;
    const char *ptr;

    MYLOG(LOG_DEBUG, "field_type=%u type=%d", field_type, fCType);

    switch (field_type) {
        case DB_TYPE_DOUBLE:
            set_client_decimal_point((char *)neut_str);
            break;
    }

    if (current_col < 0) {
        esdc = &(gdata->fdata);
        esdc->data_left = -1;
    } else
        esdc = &gdata->gdata[current_col];
    if (esdc->data_left < 0) {
        if (COPY_OK
            != (result =
                    setup_getdataclass(&len, &ptr, &needbuflen, esdc, neut_str,
                                       field_type, fCType, cbValueMax, conn)))
            goto cleanup;
    } else {
        ptr = esdc->ttlbuf;
        len = esdc->ttlbufused;
    }

    MYLOG(LOG_DEBUG, "DEFAULT: len = " FORMAT_LEN ", ptr = '%.*s'", len,
          (int)len, ptr);

    if (current_col >= 0) {
        if (esdc->data_left > 0) {
            ptr += (len - esdc->data_left);
            len = esdc->data_left;
            needbuflen = (int)len + (int)(esdc->ttlbuflen - esdc->ttlbufused);
        } else
            esdc->data_left = len;
    }

    if (cbValueMax > 0) {
        BOOL already_copied = FALSE;
        int terminatorlen;
        terminatorlen = get_terminator_len(fCType);
        if (terminatorlen >= cbValueMax)
            copy_len = 0;
        else if (len + terminatorlen > cbValueMax)
            copy_len = (int)get_adjust_len(fCType, cbValueMax - terminatorlen);
        else
            copy_len = (int)len;

        if (!already_copied) {
            /* Copy the data */
            if (copy_len > 0)
                memcpy(rgbValueBindRow, ptr, copy_len);
            /* Add null terminator */
            for (i = 0; i < terminatorlen && copy_len + i < cbValueMax; i++)
                rgbValueBindRow[copy_len + i] = '\0';
        }
        /* Adjust data_left for next time */
        if (current_col >= 0)
            esdc->data_left -= copy_len;
    }

    /*
     * Finally, check for truncation so that proper status can
     * be returned
     */
    if (cbValueMax > 0 && needbuflen > cbValueMax)
        result = COPY_RESULT_STRING_TRUNCATED;
    else {
        if (esdc->ttlbuf != NULL) {
            free(esdc->ttlbuf);
            esdc->ttlbuf = NULL;
        }
    }

#ifdef UNICODE_SUPPORT
    if (SQL_C_WCHAR == fCType)
        MYLOG(LOG_DEBUG,
              "    SQL_C_WCHAR, default: len = " FORMAT_LEN
              ", cbValueMax = " FORMAT_LEN ", rgbValueBindRow = '%s'",
              len, cbValueMax, rgbValueBindRow);
    else
#endif /* UNICODE_SUPPORT */
        if (SQL_C_BINARY == fCType)
        MYLOG(LOG_DEBUG,
              "    SQL_C_BINARY, default: len = " FORMAT_LEN
              ", cbValueMax = " FORMAT_LEN ", rgbValueBindRow = '%.*s'",
              len, cbValueMax, copy_len, rgbValueBindRow);
    else
        MYLOG(LOG_DEBUG,
              "    SQL_C_CHAR, default: len = " FORMAT_LEN
              ", cbValueMax = " FORMAT_LEN ", rgbValueBindRow = '%s'",
              len, cbValueMax, rgbValueBindRow);

cleanup:
    *length_return = len;

    return result;
}

static void check_truncation(char *p_end, int *errcode) {
    if (*p_end == '\0') {
        return;
    }
    if (*p_end != '.') {
        *errcode = COPY_INVALID_STRING_CONVERSION;
        return;
    }
    for (char *copy = p_end + 1; *copy != '\0'; copy++) {
        if (*copy != '0') {
            *errcode = COPY_RESULT_FRACTIONAL_TRUNCATED;
            return;
        }
    }
}

static int is_numeric(const char *s) {
    if (s == NULL || *s == '\0' || isspace(*s))
        return 0;
    char *p;
    strtod(s, &p);
    return *p == '\0';
}

static char *trim_white_space(char *str) {
    char *end;

    // Trim leading spaces
    while (isspace((unsigned char)*str))
        str++;

    if (*str == 0)  // All spaces?
        return str;

    // Trim trailing spaces
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

/*	This is called by SQLGetData() */
int copy_and_convert_field(StatementClass *stmt, OID field_type, int atttypmod,
                           void *valuei, SQLSMALLINT fCType, int precision,
                           PTR rgbValue, SQLLEN cbValueMax, SQLLEN *pcbValue,
                           SQLLEN *pIndicator) {
    CSTR func = "copy_and_convert_field";
    const char *value = valuei;
    ARDFields *opts = SC_get_ARDF(stmt);
    GetDataInfo *gdata = SC_get_GDTI(stmt);
    SQLLEN len = 0;
    SIMPLE_TIME std_time;
#ifdef HAVE_LOCALTIME_R
    struct tm tm;
#endif /* HAVE_LOCALTIME_R */
    SQLLEN pcbValueOffset, rgbValueOffset;
    char *rgbValueBindRow = NULL;
    SQLLEN *pcbValueBindRow = NULL, *pIndicatorBindRow = NULL;
    SQLSETPOSIROW bind_row = stmt->bind_row;
    int bind_size = opts->bind_size;
    int result = COPY_OK;
    const ConnectionClass *conn = SC_get_conn(stmt);
    BOOL text_bin_handling;
    const char *neut_str = value;
    char booltemp[3] = {0};
    char midtemp[64] = {0};
    GetDataClass *esdc;
    long slong;
    char *p_end;

    if (stmt->current_col >= 0) {
        if (stmt->current_col >= opts->allocated) {
            return SQL_ERROR;
        }
        if (gdata->allocated != opts->allocated)
            extend_getdata_info(gdata, opts->allocated, TRUE);
        esdc = &gdata->gdata[stmt->current_col];
        if (esdc->data_left == -2)
            esdc->data_left = (cbValueMax > 0) ? 0 : -1; /* This seems to be *
                                                          * needed by ADO ? */
        if (esdc->data_left == 0) {
            if (esdc->ttlbuf != NULL) {
                free(esdc->ttlbuf);
                esdc->ttlbuf = NULL;
                esdc->ttlbuflen = 0;
            }
            esdc->data_left = -2; /* needed by ADO ? */
            return COPY_NO_DATA_FOUND;
        }
    }
    /*---------
     *	rgbValueOffset is *ONLY* for character and binary data.
     *	pcbValueOffset is for computing any pcbValue location
     *---------
     */

    if (bind_size > 0)
        pcbValueOffset = rgbValueOffset = (bind_size * bind_row);
    else {
        pcbValueOffset = bind_row * sizeof(SQLLEN);
        rgbValueOffset = bind_row * cbValueMax;
    }
    /*
     *	The following is applicable in case bind_size > 0
     *	or the fCType is of variable length.
     */
    if (rgbValue)
        rgbValueBindRow = (char *)rgbValue + rgbValueOffset;
    if (pcbValue)
        pcbValueBindRow = LENADDR_SHIFT(pcbValue, pcbValueOffset);
    if (pIndicator) {
        pIndicatorBindRow = (SQLLEN *)((char *)pIndicator + pcbValueOffset);
        *pIndicatorBindRow = 0;
    }

    memset(&std_time, 0, sizeof(SIMPLE_TIME));

    MYLOG(LOG_DEBUG,
          "field_type = %d, fctype = %d, value = '%s', cbValueMax=" FORMAT_LEN,
          field_type, fCType, (value == NULL) ? "<NULL>" : value, cbValueMax);

    if (!value) {
        /*
         * handle a null just by returning SQL_NULL_DATA in pcbValue, and
         * doing nothing to the buffer.
         */
        if (pIndicator) {
            *pIndicatorBindRow = SQL_NULL_DATA;
            return COPY_OK;
        } else {
            SC_set_error(stmt, STMT_RETURN_NULL_WITHOUT_INDICATOR,
                         "StrLen_or_IndPtr was a null pointer and NULL data "
                         "was retrieved",
                         func);
            return SQL_ERROR;
        }
    }

    if (stmt->hdbc->DataSourceToDriver != NULL) {
        size_t length = strlen(value);

        stmt->hdbc->DataSourceToDriver(stmt->hdbc->translation_option, SQL_CHAR,
                                       valuei, (SDWORD)length, valuei,
                                       (SDWORD)length, NULL, NULL, 0, NULL);
    }

    /*
     * First convert any specific Amazon Timestream types into more useable data.
     *
     * NOTE: Conversions from ES char/varchar of a date/time/timestamp value
     * to SQL_C_DATE,SQL_C_TIME, SQL_C_TIMESTAMP not supported
     */
    switch (field_type) {
        case DB_TYPE_DATE:
            sscanf(value, "%4d-%2d-%2d", &std_time.y, &std_time.m, &std_time.d);
            break;

        case DB_TYPE_TIME: {
            BOOL bZone = FALSE; /* time zone stuff is unreliable */
            int zone;
            timestamp2stime(value, &std_time, &bZone, &zone);
        } break;

        case DB_TYPE_TIMESTAMP_NO_TMZONE:
        case DB_TYPE_TIMESTAMP:
            std_time.fr = 0;
            std_time.infinity = 0;
            if (strnicmp(value, INFINITY_STRING, 8) == 0) {
                std_time.infinity = 1;
                std_time.m = 12;
                std_time.d = 31;
                std_time.y = 9999;
                std_time.hh = 23;
                std_time.mm = 59;
                std_time.ss = 59;
            }
            if (strnicmp(value, MINFINITY_STRING, 9) == 0) {
                std_time.infinity = -1;
                std_time.m = 1;
                std_time.d = 1;
                std_time.y = -9999;
                std_time.hh = 0;
                std_time.mm = 0;
                std_time.ss = 0;
            }
            if (strnicmp(value, "invalid", 7) != 0) {
                char2stime(value, &std_time, &result, fCType);
                MYLOG(LOG_ALL, "2stime fr=%d", std_time.fr);
            } else {
                /*
                 * The timestamp is invalid so set something conspicuous,
                 * like the epoch
                 */
                struct tm *tim;
                time_t t = 0;
#ifdef HAVE_LOCALTIME_R
                tim = localtime_r(&t, &tm);
#else
                tim = localtime(&t);
#endif /* HAVE_LOCALTIME_R */
                std_time.m = tim->tm_mon + 1;
                std_time.d = tim->tm_mday;
                std_time.y = tim->tm_year + 1900;
                std_time.hh = tim->tm_hour;
                std_time.mm = tim->tm_min;
                std_time.ss = tim->tm_sec;
            }
            break;

        case DB_TYPE_VARCHAR: {
            char *copy = strdup(value);
            char *trimmed = trim_white_space(copy);
            if (fCType == SQL_C_DATE || fCType == SQL_C_TYPE_DATE
                || fCType == SQL_C_TIME || fCType == SQL_C_TYPE_TIME) {
                char2stime(trimmed, &std_time, &result, fCType);
            } else if (fCType == SQL_C_TIMESTAMP
                       || fCType == SQL_C_TYPE_TIMESTAMP) {
                std_time.fr = 0;
                std_time.infinity = 0;
                if (strnicmp(trimmed, INFINITY_STRING, 8) == 0) {
                    std_time.infinity = 1;
                    std_time.m = 12;
                    std_time.d = 31;
                    std_time.y = 9999;
                    std_time.hh = 23;
                    std_time.mm = 59;
                    std_time.ss = 59;
                }
                if (strnicmp(trimmed, MINFINITY_STRING, 9) == 0) {
                    std_time.infinity = -1;
                    std_time.m = 1;
                    std_time.d = 1;
                    // std_time.y = -4713;
                    std_time.y = -9999;
                    std_time.hh = 0;
                    std_time.mm = 0;
                    std_time.ss = 0;
                }
                if (strnicmp(trimmed, "invalid", 7) != 0) {
                    char2stime(trimmed, &std_time, &result, fCType);
                    MYLOG(LOG_ALL, "2stime fr=%d", std_time.fr);
                } else {
                    /*
                     * The timestamp is invalid so set something conspicuous,
                     * like the epoch
                     */
                    struct tm *tim;
                    time_t t = 0;
#ifdef HAVE_LOCALTIME_R
                    tim = localtime_r(&t, &tm);
#else
                    tim = localtime(&t);
#endif /* HAVE_LOCALTIME_R */
                    std_time.m = tim->tm_mon + 1;
                    std_time.d = tim->tm_mday;
                    std_time.y = tim->tm_year + 1900;
                    std_time.hh = tim->tm_hour;
                    std_time.mm = tim->tm_min;
                    std_time.ss = tim->tm_sec;
                }
            }
            free(copy);
            break;
        }
        case DB_TYPE_BOOLEAN: { /* change T/F to 1/0 */
            switch (((char *)value)[0]) {
                case 'f':
                case 'F':
                case 'n':
                case 'N':
                case '0':
                    STRCPY_FIXED(booltemp, "0");
                    break;
                default:
                    STRCPY_FIXED(booltemp, "1");
            }
            neut_str = booltemp;
        } break;

        case 0:
            break;

        default:
            if (field_type
                    == (OID)stmt->hdbc
                           ->lobj_type /* hack until permanent type available */
                )
                return convert_lo(stmt, value, fCType, rgbValueBindRow,
                                  cbValueMax, pcbValueBindRow);
    }

    /* Change default into something useable */
    if (fCType == SQL_C_DEFAULT) {
        fCType = estype_attr_to_ctype(conn, field_type, atttypmod);
#ifdef UNICODE_SUPPORT
        if (fCType == SQL_C_WCHAR && CC_default_is_c(conn))
            fCType = SQL_C_CHAR;
#endif

        MYLOG(LOG_DEBUG, ", SQL_C_DEFAULT: fCType = %d", fCType);
    }

    text_bin_handling = FALSE;
    switch (fCType) {
        case INTERNAL_ASIS_TYPE:
#ifdef UNICODE_SUPPORT
        case SQL_C_WCHAR:
#endif /* UNICODE_SUPPORT */
        case SQL_C_CHAR:
            text_bin_handling = TRUE;
            break;
        case SQL_C_BINARY:
            switch (field_type) {
                case DB_TYPE_UNKNOWN:
                case DB_TYPE_VARCHAR:
                    text_bin_handling = TRUE;
                    break;
            }
            break;
    }

    if (text_bin_handling) {
        BOOL pre_convert = TRUE;
        int midsize = sizeof(midtemp);

        /* Special character formatting as required */

        /*
         * These really should return error if cbValueMax is not big
         * enough.
         */
        switch (field_type) {
            case DB_TYPE_DATE:
                len = SPRINTF_FIXED(midtemp, "%.4d-%.2d-%.2d", std_time.y,
                                    std_time.m, std_time.d);
                break;

            case DB_TYPE_TIME:
                len = SPRINTF_FIXED(midtemp, "%.2d:%.2d:%.2d.%09d", std_time.hh,
                                    std_time.mm, std_time.ss, std_time.fr);
                break;

            case DB_TYPE_TIMESTAMP_NO_TMZONE:
            case DB_TYPE_TIMESTAMP:
                len = stime2timestamp(&std_time, midtemp, midsize, FALSE,
                                      (int)(midsize - 19 - 2));
                break;

            default:
                pre_convert = FALSE;
        }
        if (pre_convert)
            neut_str = midtemp;
        result = convert_text_field_to_sql_c(
            gdata, stmt->current_col, neut_str, field_type, fCType,
            rgbValueBindRow, cbValueMax, conn, &len);
    } else {
        SQLGUID g;

        switch (fCType) {
            case SQL_C_DATE:
            case SQL_C_TYPE_DATE: /* 91 */
                if (field_type == DB_TYPE_TIME) {
                    result = COPY_UNSUPPORTED_CONVERSION;
                } else {
                    len = 6;
                    {
                        DATE_STRUCT *ds;
                        struct tm *tim;

                        if (bind_size > 0)
                            ds = (DATE_STRUCT *)rgbValueBindRow;
                        else
                            ds = (DATE_STRUCT *)rgbValue + bind_row;

                        /*
                         * Initialize date in case conversion destination
                         * expects date part from this source time data.
                         * A value may be partially set here, so do some
                         * sanity checks on the existing values before
                         * setting them.
                         */
                        tim = SC_get_localtime(stmt);
                        if (std_time.m == 0)
                            std_time.m = tim->tm_mon + 1;
                        if (std_time.d == 0)
                            std_time.d = tim->tm_mday;
                        if (std_time.y == 0)
                            std_time.y = tim->tm_year + 1900;
                        ds->year = (SQLSMALLINT)std_time.y;
                        ds->month = (SQLUSMALLINT)std_time.m;
                        ds->day = (SQLUSMALLINT)std_time.d;
                    }
                }
                break;

            case SQL_C_TIME:
            case SQL_C_TYPE_TIME: /* 92 */
                if (field_type == DB_TYPE_DATE) {
                    result = COPY_UNSUPPORTED_CONVERSION;
                } else {
                    len = 6;
                    {
                        TIME_STRUCT *ts;

                        if (bind_size > 0)
                            ts = (TIME_STRUCT *)rgbValueBindRow;
                        else
                            ts = (TIME_STRUCT *)rgbValue + bind_row;
                        ts->hour = (SQLUSMALLINT)std_time.hh;
                        ts->minute = (SQLUSMALLINT)std_time.mm;
                        ts->second = (SQLUSMALLINT)std_time.ss;
                    }
                }
                break;

            case SQL_C_TIMESTAMP:
            case SQL_C_TYPE_TIMESTAMP: /* 93 */
                len = 16;
                {
                    struct tm *tim;
                    TIMESTAMP_STRUCT *ts;

                    if (bind_size > 0)
                        ts = (TIMESTAMP_STRUCT *)rgbValueBindRow;
                    else
                        ts = (TIMESTAMP_STRUCT *)rgbValue + bind_row;

                    /*
                     * Initialize date in case conversion destination
                     * expects date part from this source time data.
                     * A value may be partially set here, so do some
                     * sanity checks on the existing values before
                     * setting them.
                     */
                    tim = SC_get_localtime(stmt);
                    if (std_time.m == 0)
                        std_time.m = tim->tm_mon + 1;
                    if (std_time.d == 0)
                        std_time.d = tim->tm_mday;
                    if (std_time.y == 0)
                        std_time.y = tim->tm_year + 1900;

                    ts->year = (SQLSMALLINT)std_time.y;
                    ts->month = (SQLUSMALLINT)std_time.m;
                    ts->day = (SQLUSMALLINT)std_time.d;
                    ts->hour = (SQLUSMALLINT)std_time.hh;
                    ts->minute = (SQLUSMALLINT)std_time.mm;
                    ts->second = (SQLUSMALLINT)std_time.ss;
                    ts->fraction = (SQLUINTEGER)std_time.fr;
                }
                break;

            case SQL_C_BIT: {
                len = 1;
                char *copy = strdup(neut_str);
                char *trimmed = trim_white_space(copy);
                if (!is_numeric(trimmed)) {
                    result = COPY_INVALID_STRING_CONVERSION;
                    free(copy);
                    break;
                }
                slong = strtol(trimmed, &p_end, 10);
                check_truncation(p_end, &result);
                if (slong < 0 || slong >= 2) {
                    result = COPY_RESULT_OVERFLOW_UNDERFLOW;
                } else {
                    if (bind_size > 0)
                        *((UCHAR *)rgbValueBindRow) = (UCHAR)slong;
                    else
                        *((UCHAR *)rgbValue + bind_row) = (UCHAR)slong;
                }
                free(copy);

                MYLOG(99,
                      "SQL_C_BIT: bind_row = " FORMAT_POSIROW
                      " val = %ld, cb = " FORMAT_LEN ", rgb=%d",
                      bind_row, slong, cbValueMax, *((UCHAR *)rgbValue));
                break;
            }

            case SQL_C_STINYINT:
            case SQL_C_TINYINT: {
                len = 1;
                char *copy = strdup(neut_str);
                char *trimmed = trim_white_space(copy);
                if (!is_numeric(trimmed)) {
                    result = COPY_INVALID_STRING_CONVERSION;
                    free(copy);
                    break;
                }
                slong = strtol(trimmed, &p_end, 10);
                check_truncation(p_end, &result);
                if (slong < SCHAR_MIN || slong > SCHAR_MAX) {
                    result = COPY_RESULT_OVERFLOW_UNDERFLOW;
                } else {
                    if (bind_size > 0)
                        *((SCHAR *)rgbValueBindRow) = (SCHAR)slong;
                    else
                        *((SCHAR *)rgbValue + bind_row) = (SCHAR)slong;
                }
                free(copy);
                break;
            }

            case SQL_C_UTINYINT: {
                len = 1;
                char *copy = strdup(neut_str);
                char *trimmed = trim_white_space(copy);
                if (!is_numeric(trimmed)) {
                    result = COPY_INVALID_STRING_CONVERSION;
                    free(copy);
                    break;
                }
                slong = strtol(trimmed, &p_end, 10);
                check_truncation(p_end, &result);
                if (slong < 0 || slong > UCHAR_MAX) {
                    result = COPY_RESULT_OVERFLOW_UNDERFLOW;
                } else {
                    if (bind_size > 0)
                        *((UCHAR *)rgbValueBindRow) = (UCHAR)slong;
                    else
                        *((UCHAR *)rgbValue + bind_row) = (UCHAR)slong;
                }
                free(copy);
                break;
            }
            case SQL_C_FLOAT: {
                char *copy = strdup(neut_str);
                char *trimmed = trim_white_space(copy);
                if (!is_numeric(trimmed)) {
                    result = COPY_INVALID_STRING_CONVERSION;
                    free(copy);
                    break;
                }
                set_client_decimal_point((char *)trimmed);
                len = 4;
                errno = 0;
                float f = strtof(trimmed, 0);
                if (errno == ERANGE) {
                    result = COPY_RESULT_OVERFLOW_UNDERFLOW;
                } else {
                    if (bind_size > 0)
                        *((SFLOAT *)rgbValueBindRow) = f;
                    else
                        *((SFLOAT *)rgbValue + bind_row) = f;
                }
                free(copy);
                break;
            }

            case SQL_C_DOUBLE: {
                char *copy = strdup(neut_str);
                char *trimmed = trim_white_space(copy);
                if (!is_numeric(trimmed)) {
                    result = COPY_INVALID_STRING_CONVERSION;
                    free(copy);
                    break;
                }
                set_client_decimal_point((char *)trimmed);
                len = 8;
                errno = 0;
                double d = strtod(trimmed, 0);
                if ((d == -HUGE_VAL || d == HUGE_VAL)
                    && errno == ERANGE) {
                    result = COPY_RESULT_OVERFLOW_UNDERFLOW;
                } else {
                    if (bind_size > 0)
                        *((SDOUBLE *)rgbValueBindRow) = d;
                    else
                        *((SDOUBLE *)rgbValue + bind_row) = d;
                }
                free(copy);
                break;
            }

            case SQL_C_NUMERIC: {
                SQL_NUMERIC_STRUCT *ns;
                BOOL overflowed;

                if (bind_size > 0)
                    ns = (SQL_NUMERIC_STRUCT *)rgbValueBindRow;
                else
                    ns = (SQL_NUMERIC_STRUCT *)rgbValue + bind_row;

                parse_to_numeric_struct(neut_str, ns, &overflowed);
                if (overflowed)
                    result = COPY_RESULT_STRING_TRUNCATED;
            } break;

            case SQL_C_SSHORT:
            case SQL_C_SHORT: {
                len = 2;
                char *copy = strdup(neut_str);
                char *trimmed = trim_white_space(copy);
                if (!is_numeric(trimmed)) {
                    result = COPY_INVALID_STRING_CONVERSION;
                    free(copy);
                    break;
                }
                slong = strtol(trimmed, &p_end, 10);
                check_truncation(p_end, &result);
                if (slong < SHRT_MIN || slong > SHRT_MAX) {
                    result = COPY_RESULT_OVERFLOW_UNDERFLOW;
                } else {
                    if (bind_size > 0)
                        *((SQLSMALLINT *)rgbValueBindRow) = (SQLSMALLINT)slong;
                    else
                        *((SQLSMALLINT *)rgbValue + bind_row) =
                            (SQLSMALLINT)slong;
                }
                free(copy);
                break;
            }

            case SQL_C_USHORT: {
                len = 2;
                char *copy = strdup(neut_str);
                char *trimmed = trim_white_space(copy);
                if (!is_numeric(trimmed)) {
                    result = COPY_INVALID_STRING_CONVERSION;
                    free(copy);
                    break;
                }
                slong = strtol(trimmed, &p_end, 10);
                check_truncation(p_end, &result);
                if (slong < 0 || slong > USHRT_MAX) {
                    result = COPY_RESULT_OVERFLOW_UNDERFLOW;
                } else {
                    if (bind_size > 0)
                        *((SQLUSMALLINT *)rgbValueBindRow) =
                            (SQLUSMALLINT)slong;
                    else
                        *((SQLUSMALLINT *)rgbValue + bind_row) =
                            (SQLUSMALLINT)slong;
                }
                free(copy);
                break;
            }

            case SQL_C_SLONG:
            case SQL_C_LONG: {
                len = 4;
                char *copy = strdup(neut_str);
                char *trimmed = trim_white_space(copy);
                if (!is_numeric(trimmed)) {
                    result = COPY_INVALID_STRING_CONVERSION;
                    free(copy);
                    break;
                }
                errno = 0;
                signed long long sllong = strtoll(trimmed, &p_end, 10);
                check_truncation(p_end, &result);
                if (errno == ERANGE || sllong < INT32_MIN || sllong > INT32_MAX) {
                    result = COPY_RESULT_OVERFLOW_UNDERFLOW;
                } else {
                    if (bind_size > 0)
                        *((SQLINTEGER *)rgbValueBindRow) = (SQLINTEGER)sllong;
                    else
                        *((SQLINTEGER *)rgbValue + bind_row) = (SQLINTEGER)sllong;
                }
                free(copy);
                break;
            }

            case SQL_C_ULONG: {
                len = 4;
                char *copy = strdup(neut_str);
                char *trimmed = trim_white_space(copy);
                if (!is_numeric(trimmed)) {
                    result = COPY_INVALID_STRING_CONVERSION;
                    free(copy);
                    break;
                }
                // negative case
                if (strtol(trimmed, 0, 10) < 0) {
                    result = COPY_RESULT_OVERFLOW_UNDERFLOW;
                    free(copy);
                    break;
                }
                errno = 0;
                unsigned long long ullong = strtoull(trimmed, &p_end, 10);
                check_truncation(p_end, &result);
                if (errno == ERANGE || ullong > UINT32_MAX) {
                    result = COPY_RESULT_OVERFLOW_UNDERFLOW;
                } else {
                    if (bind_size > 0)
                        *((SQLUINTEGER *)rgbValueBindRow) = (SQLUINTEGER)ullong;
                    else
                        *((SQLUINTEGER *)rgbValue + bind_row) = (SQLUINTEGER)ullong;
                }
                free(copy);
                break;
            }

#ifdef ODBCINT64
            case SQL_C_SBIGINT: {
                len = 8;
                char *copy = strdup(neut_str);
                char *trimmed = trim_white_space(copy);
                if (!is_numeric(trimmed)) {
                    result = COPY_INVALID_STRING_CONVERSION;
                    free(copy);
                    break;
                }
                errno = 0;
                signed long long sllong = strtoll(trimmed, &p_end, 10);
                check_truncation(p_end, &result);
                if ((sllong == LLONG_MIN || sllong == LLONG_MAX)
                    && errno == ERANGE) {
                    result = COPY_RESULT_OVERFLOW_UNDERFLOW;
                } else {
                    if (bind_size > 0)
                        *((SQLBIGINT *)rgbValueBindRow) = sllong;
                    else
                        *((SQLBIGINT *)rgbValue + bind_row) = sllong;
                }
                free(copy);
                break;
            }

            case SQL_C_UBIGINT: {
                len = 8;
                char *copy = strdup(neut_str);
                char *trimmed = trim_white_space(copy);
                if (!is_numeric(trimmed)) {
                    result = COPY_INVALID_STRING_CONVERSION;
                    free(copy);
                    break;
                }
                // negative case
                if (strtol(trimmed, 0, 10) < 0) {
                    result = COPY_RESULT_OVERFLOW_UNDERFLOW;
                    free(copy);
                    break;
                }
                errno = 0;
                unsigned long long ullong = strtoull(trimmed, &p_end, 10);
                check_truncation(p_end, &result);
                if (ullong == ULLONG_MAX && errno == ERANGE) {
                    result = COPY_RESULT_OVERFLOW_UNDERFLOW;
                } else {
                    if (bind_size > 0)
                        *((SQLUBIGINT *)rgbValueBindRow) = ullong;
                    else
                        *((SQLUBIGINT *)rgbValue + bind_row) = ullong;
                }
                free(copy);
                break;
            }

#endif /* ODBCINT64 */
            case SQL_C_BINARY:
                /* The following is for SQL_C_VARBOOKMARK */
                if (DB_TYPE_INTEGER == field_type) {
                    UInt4 ival = strtoul(neut_str, 0, 10);

                    MYLOG(LOG_ALL, "SQL_C_VARBOOKMARK value=%d", ival);
                    if (pcbValue)
                        *pcbValueBindRow = sizeof(ival);
                    if (cbValueMax >= (SQLLEN)sizeof(ival)) {
                        memcpy(rgbValueBindRow, &ival, sizeof(ival));
                        return COPY_OK;
                    } else
                        return COPY_RESULT_STRING_TRUNCATED;
                } else {
                    MYLOG(LOG_DEBUG,
                          "couldn't convert the type %d to SQL_C_BINARY",
                          field_type);
                    return COPY_UNSUPPORTED_TYPE;
                }
                break;
            case SQL_C_GUID:

                result = char2guid(neut_str, &g);
                if (COPY_OK != result) {
                    MYLOG(LOG_DEBUG, "Could not convert to SQL_C_GUID");
                    return COPY_UNSUPPORTED_TYPE;
                }
                len = sizeof(g);
                if (bind_size > 0)
                    *((SQLGUID *)rgbValueBindRow) = g;
                else
                    *((SQLGUID *)rgbValue + bind_row) = g;
                break;
            case SQL_C_INTERVAL_YEAR:
            case SQL_C_INTERVAL_MONTH:
            case SQL_C_INTERVAL_YEAR_TO_MONTH:
            case SQL_C_INTERVAL_DAY:
            case SQL_C_INTERVAL_HOUR:
            case SQL_C_INTERVAL_DAY_TO_HOUR:
            case SQL_C_INTERVAL_MINUTE:
            case SQL_C_INTERVAL_HOUR_TO_MINUTE:
            case SQL_C_INTERVAL_SECOND:
            case SQL_C_INTERVAL_DAY_TO_SECOND:
            case SQL_C_INTERVAL_HOUR_TO_SECOND:
            case SQL_C_INTERVAL_MINUTE_TO_SECOND:
                len = sizeof(SQL_INTERVAL_STRUCT);
                UNUSED(precision);
                char *copy = strdup(neut_str);
                char *trimmed = trim_white_space(copy);
                result = interval2istruct(
                             fCType, 9, trimmed,
                             bind_size > 0
                                 ? (SQL_INTERVAL_STRUCT *)rgbValueBindRow
                                 : (SQL_INTERVAL_STRUCT *)rgbValue + bind_row)
                             ? result
                             : COPY_INVALID_STRING_CONVERSION;
                free(copy);
                break;

            default:
                MYLOG(LOG_DEBUG, "conversion to the type %d isn't supported",
                      fCType);
                return COPY_UNSUPPORTED_TYPE;
        }
    }

    /* store the length of what was copied, if there's a place for it */
    if (pcbValue)
        *pcbValueBindRow = len;

    if (result == COPY_OK && stmt->current_col >= 0)
        gdata->gdata[stmt->current_col].data_left = 0;
    return result;
}

/*--------------------------------------------------------------------
 *	Functions/Macros to get rid of query size limit.
 *
 *	I always used the follwoing macros to convert from
 *	old_statement to new_statement.  Please improve it
 *	if you have a better way.	Hiroshi 2001/05/22
 *--------------------------------------------------------------------
 */

#define FLGP_USING_CURSOR (1L << 1)
#define FLGP_SELECT_INTO (1L << 2)
#define FLGP_SELECT_FOR_UPDATE_OR_SHARE (1L << 3)
#define FLGP_MULTIPLE_STATEMENT (1L << 5)
#define FLGP_SELECT_FOR_READONLY (1L << 6)
typedef struct _QueryParse {
    const char *statement;
    int statement_type;
    size_t opos;
    ssize_t from_pos;
    ssize_t where_pos;
    ssize_t stmt_len;
    int in_status;
    char escape_in_literal, prev_token_end;
    const char *dollar_tag;
    ssize_t taglen;
    char token_curr[64];
    int token_len;
    size_t declare_pos;
    UInt4 flags, comment_level;
    encoded_str encstr;
} QueryParse;

enum {
    QP_IN_IDENT_KEYWORD = 1L /* identifier or keyword */
    ,
    QP_IN_DQUOTE_IDENTIFIER = (1L << 1) /* "" */
    ,
    QP_IN_LITERAL = (1L << 2) /* '' */
    ,
    QP_IN_ESCAPE = (1L << 3) /* \ in literal */
    ,
    QP_IN_DOLLAR_QUOTE = (1L << 4) /* $...$    $...$ */
    ,
    QP_IN_COMMENT_BLOCK = (1L << 5) /* slash asterisk */
    ,
    QP_IN_LINE_COMMENT = (1L << 6) /* -- */
};

#define QP_in_idle_status(qp) ((qp)->in_status == 0)

#define QP_is_in(qp, status) (((qp)->in_status & status) != 0)
#define QP_enter(qp, status) ((qp)->in_status |= status)
#define QP_exit(qp, status) ((qp)->in_status &= (~status))

typedef enum {
    RPM_REPLACE_PARAMS,
    RPM_FAKE_PARAMS,
    RPM_BUILDING_PREPARE_STATEMENT,
    RPM_BUILDING_BIND_REQUEST
} ResolveParamMode;

#define FLGB_INACCURATE_RESULT (1L << 4)
#define FLGB_CREATE_KEYSET (1L << 5)
#define FLGB_KEYSET_DRIVEN (1L << 6)
#define FLGB_CONVERT_LF (1L << 7)
#define FLGB_DISCARD_OUTPUT (1L << 8)
#define FLGB_BINARY_AS_POSSIBLE (1L << 9)
#define FLGB_LITERAL_EXTENSION (1L << 10)
#define FLGB_HEX_BIN_FORMAT (1L << 11)
#define FLGB_PARAM_CAST (1L << 12)
typedef struct _QueryBuild {
    char *query_statement;
    size_t str_alsize;
    size_t npos;
    SQLLEN current_row;
    Int2 param_number;
    Int2 dollar_number;
    Int2 num_io_params;
    Int2 num_output_params;
    Int2 num_discard_params;
    Int2 proc_return;
    Int2 brace_level;
    char parenthesize_the_first;
    APDFields *apdopts;
    IPDFields *ipdopts;
    PutDataInfo *pdata;
    size_t load_stmt_len;
    size_t load_from_pos;
    ResolveParamMode param_mode;
    UInt4 flags;
    int ccsc;
    int errornumber;
    const char *errormsg;

    ConnectionClass *conn; /* mainly needed for LO handling */
    StatementClass *stmt;  /* needed to set error info in ENLARGE_.. */
} QueryBuild;

#define INIT_MIN_ALLOC 4096

/*
 * New macros (Aceto)
 *--------------------
 */

#define F_OldChar(qp) ((qp)->statement[(qp)->opos])

#define F_OldPtr(qp) ((qp)->statement + (qp)->opos)

#define F_OldNext(qp) (++(qp)->opos)

#define F_OldPrior(qp) (--(qp)->opos)

#define F_OldPos(qp) (qp)->opos

#define F_ExtractOldTo(qp, buf, ch, maxsize)         \
    do {                                             \
        size_t c = 0;                                \
        while ((qp)->statement[qp->opos] != '\0'     \
               && (qp)->statement[qp->opos] != ch) { \
            if (c >= maxsize)                        \
                break;                               \
            buf[c++] = (qp)->statement[qp->opos++];  \
        }                                            \
        if ((qp)->statement[qp->opos] == '\0') {     \
            retval = SQL_ERROR;                      \
            goto cleanup;                            \
        }                                            \
        buf[c] = '\0';                               \
    } while (0)

#define F_NewChar(qb) (qb->query_statement[(qb)->npos])

#define F_NewPtr(qb) ((qb)->query_statement + (qb)->npos)

#define F_NewNext(qb) (++(qb)->npos)

#define F_NewPos(qb) ((qb)->npos)

/*----------
 *	Terminate the stmt_with_params string with NULL.
 *----------
 */
#define CVT_TERMINATE(qb)                         \
    do {                                          \
        if (NULL == (qb)->query_statement) {      \
            retval = SQL_ERROR;                   \
            goto cleanup;                         \
        }                                         \
        (qb)->query_statement[(qb)->npos] = '\0'; \
    } while (0)

/*----------
 *	Append a data.
 *----------
 */
#define CVT_APPEND_DATA(qb, s, len)                         \
    do {                                                    \
        size_t newpos = (qb)->npos + len;                   \
        ENLARGE_NEWSTATEMENT((qb), newpos);                 \
        memcpy(&(qb)->query_statement[(qb)->npos], s, len); \
        (qb)->npos = newpos;                                \
        (qb)->query_statement[newpos] = '\0';               \
    } while (0)

/*----------
 *	Append a string.
 *----------
 */
#define CVT_APPEND_STR(qb, s)        \
    do {                             \
        size_t len = strlen(s);      \
        CVT_APPEND_DATA(qb, s, len); \
    } while (0)

/*----------
 *	Append a char.
 *----------
 */
#define CVT_APPEND_CHAR(qb, c)                    \
    do {                                          \
        ENLARGE_NEWSTATEMENT(qb, (qb)->npos + 1); \
        (qb)->query_statement[(qb)->npos++] = c;  \
    } while (0)

int findIdentifier(const UCHAR *str, int ccsc, const UCHAR **next_token) {
    int slen = -1;
    encoded_str encstr;
    UCHAR tchar;
    BOOL dquote = FALSE;

    *next_token = NULL;
    encoded_str_constr(&encstr, ccsc, (const char *)str);
    for (tchar = (UCHAR)encoded_nextchar(&encstr); tchar;
         tchar = (UCHAR)encoded_nextchar(&encstr)) {
        if (MBCS_NON_ASCII(encstr))
            continue;
        if (encstr.pos == 0) /* the first character */
        {
            if (dquote = (IDENTIFIER_QUOTE == tchar), dquote)
                continue;
            if (!isalpha(tchar)) {
                slen = 0;
                if (IS_NOT_SPACE(tchar))
                    *next_token = ENCODE_PTR(encstr);
                break;
            }
        }
        if (dquote) {
            if (IDENTIFIER_QUOTE == tchar) {
                tchar = (UCHAR)encoded_nextchar(&encstr);
                if (IDENTIFIER_QUOTE == tchar)
                    continue;
                slen = (int)encstr.pos;
                break;
            }
        } else {
            if (isalnum(tchar))
                continue;
            switch (tchar) {
                case '_':
                case DOLLAR_QUOTE:
                    continue;
            }
            slen = (int)encstr.pos;
            if (IS_NOT_SPACE(tchar))
                *next_token = ENCODE_PTR(encstr);
            break;
        }
    }
    if (slen < 0 && !dquote)
        slen = (int)encstr.pos;
    if (NULL == *next_token) {
        for (; tchar; tchar = (UCHAR)encoded_nextchar(&encstr)) {
            if (IS_NOT_SPACE((UCHAR)tchar)) {
                *next_token = ENCODE_PTR(encstr);
                break;
            }
        }
    }
    return slen;
}

static esNAME lower_or_remove_dquote(esNAME nm, const UCHAR *src, int srclen,
                                     int ccsc) {
    int i, outlen;
    char *tc;
    UCHAR tchar;
    BOOL idQuote;
    encoded_str encstr;

    if (nm.name)
        tc = realloc(nm.name, srclen + 1);
    else
        tc = malloc(srclen + 1);
    if (!tc) {
        NULL_THE_NAME(nm);
        return nm;
    }
    nm.name = tc;
    idQuote = (src[0] == IDENTIFIER_QUOTE);
    encoded_str_constr(&encstr, ccsc, (const char *)src);
    for (i = 0, tchar = (UCHAR)encoded_nextchar(&encstr), outlen = 0;
         i < srclen; i++, tchar = (UCHAR)encoded_nextchar(&encstr)) {
        if (MBCS_NON_ASCII(encstr)) {
            tc[outlen++] = tchar;
            continue;
        }
        if (idQuote) {
            if (IDENTIFIER_QUOTE == tchar) {
                if (0 == i)
                    continue;
                if (i == srclen - 1)
                    continue;
                i++;
                tchar = (UCHAR)encoded_nextchar(&encstr);
            }
            tc[outlen++] = tchar;
        } else {
            tc[outlen++] = (char)tolower(tchar);
        }
    }
    tc[outlen] = '\0';
    return nm;
}

int eatTableIdentifiers(const UCHAR *str, int ccsc, esNAME *table,
                        esNAME *schema) {
    int len;
    const UCHAR *next_token;
    const UCHAR *tstr = str;

    while (isspace(*tstr))
        tstr++;

    if ((len = findIdentifier(tstr, ccsc, &next_token)) <= 0)
        return len; /* table name doesn't exist */
    if (table) {
        if (IDENTIFIER_QUOTE == *tstr)
            *table = lower_or_remove_dquote(*table, tstr, len, ccsc);
        else
            STRN_TO_NAME(*table, tstr, len);
    }
    if (!next_token || '.' != *next_token || (int)(next_token - tstr) != len)
        return (int)(next_token - str); /* table only */
    tstr = next_token + 1;
    if ((len = findIdentifier(tstr, ccsc, &next_token)) <= 0)
        return -1;
    if (table) {
        if (schema)
            MOVE_NAME(*schema, *table);
        *table = lower_or_remove_dquote(*table, tstr, len, ccsc);
    }
    if (!next_token || '.' != *next_token || (int)(next_token - tstr) != len)
        return (int)(next_token - str); /* schema.table */
    tstr = next_token + 1;
    if ((len = findIdentifier(tstr, ccsc, &next_token)) <= 0)
        return -1;
    if (table) {
        if (schema)
            MOVE_NAME(*schema, *table);
        *table = lower_or_remove_dquote(*table, tstr, len, ccsc);
    }
    return (int)(next_token - str); /* catalog.schema.table */
}

#define PT_TOKEN_IGNORE(pt) ((pt)->curchar_processed = TRUE)

#define MIN_ALC_SIZE 128

/*
 * With SQL_MAX_NUMERIC_LEN = 16, the highest representable number is
 * 2^128 - 1, which fits in 39 digits.
 */
#define MAX_NUMERIC_DIGITS 39

/*
 * Convert a string representation of a numeric into SQL_NUMERIC_STRUCT.
 */
static void parse_to_numeric_struct(const char *wv, SQL_NUMERIC_STRUCT *ns,
                                    BOOL *overflow) {
    int i, nlen, dig;
    char calv[SQL_MAX_NUMERIC_LEN * 3];
    BOOL dot_exist;

    *overflow = FALSE;

    /* skip leading space */
    while (*wv && isspace((unsigned char)*wv))
        wv++;

    /* sign */
    ns->sign = 1;
    if (*wv == '-') {
        ns->sign = 0;
        wv++;
    } else if (*wv == '+')
        wv++;

    /* skip leading zeros */
    while (*wv == '0')
        wv++;

    /* read the digits into calv */
    ns->precision = 0;
    ns->scale = 0;
    for (nlen = 0, dot_exist = FALSE;; wv++) {
        if (*wv == '.') {
            if (dot_exist)
                break;
            dot_exist = TRUE;
        } else if (*wv == '\0' || !isdigit((unsigned char)*wv))
            break;
        else {
            if (nlen >= (int)sizeof(calv)) {
                if (dot_exist)
                    break;
                else {
                    ns->scale--;
                    *overflow = TRUE;
                    continue;
                }
            }
            if (dot_exist)
                ns->scale++;
            calv[nlen++] = *wv;
        }
    }
    ns->precision = (SQLCHAR)nlen;

    /* Convert the decimal digits to binary */
    memset(ns->val, 0, sizeof(ns->val));
    for (dig = 0; dig < nlen; dig++) {
        UInt4 carry;

        /* multiply the current value by 10, and add the next digit */
        carry = calv[dig] - '0';
        for (i = 0; i < (int)sizeof(ns->val); i++) {
            UInt4 t;

            t = ((UInt4)ns->val[i]) * 10 + carry;
            ns->val[i] = (unsigned char)(t & 0xFF);
            carry = (t >> 8);
        }

        if (carry != 0)
            *overflow = TRUE;
    }
}

/*	Change linefeed to carriage-return/linefeed */
size_t convert_linefeeds(const char *si, char *dst, size_t max, BOOL convlf,
                         BOOL *changed) {
    size_t i = 0, out = 0;

    if (max == 0)
        max = 0xffffffff;
    *changed = FALSE;
    for (i = 0; si[i] && out < max - 1; i++) {
        if (convlf && si[i] == '\n') {
            /* Only add the carriage-return if needed */
            if (i > 0 && ES_CARRIAGE_RETURN == si[i - 1]) {
                if (dst)
                    dst[out++] = si[i];
                else
                    out++;
                continue;
            }
            *changed = TRUE;

            if (dst) {
                dst[out++] = ES_CARRIAGE_RETURN;
                dst[out++] = '\n';
            } else
                out += 2;
        } else {
            if (dst)
                dst[out++] = si[i];
            else
                out++;
        }
    }
    if (dst)
        dst[out] = '\0';
    return out;
}

static int conv_from_octal(const char *s) {
    ssize_t i;
    int y = 0;

    for (i = 1; i <= 3; i++)
        y += (s[i] - '0') << (3 * (3 - i));

    return y;
}

/*	convert octal escapes to bytes */
static size_t convert_from_esbinary(const char *value, char *rgbValue,
                                    SQLLEN cbValueMax) {
    UNUSED(cbValueMax);
    size_t i, ilen = strlen(value);
    size_t o = 0;

    for (i = 0; i < ilen;) {
        if (value[i] == BYTEA_ESCAPE_CHAR) {
            if (value[i + 1] == BYTEA_ESCAPE_CHAR) {
                if (rgbValue)
                    rgbValue[o] = value[i];
                o++;
                i += 2;
            } else if (value[i + 1] == 'x') {
                i += 2;
                if (i < ilen) {
                    ilen -= i;
                    if (rgbValue)
                        es_hex2bin(value + i, rgbValue + o, ilen);
                    o += ilen / 2;
                }
                break;
            } else {
                if (rgbValue)
                    rgbValue[o] = (char)conv_from_octal(&value[i]);
                o++;
                i += 4;
            }
        } else {
            if (rgbValue)
                rgbValue[o] = value[i];
            o++;
            i++;
        }
        /** if (rgbValue)
            MYLOG(LOG_DEBUG, "i=%d, rgbValue[%d] = %d, %c", i, o, rgbValue[o],
           rgbValue[o]); ***/
    }

    if (rgbValue)
        rgbValue[o] = '\0'; /* extra protection */

    MYLOG(LOG_DEBUG, "in=" FORMAT_SIZE_T ", out = " FORMAT_SIZE_T, ilen, o);

    return o;
}

static const char *hextbl = "0123456789ABCDEF";

#define def_bin2hex(type)                                            \
    (const char *src, type *dst, SQLLEN length) {                    \
        const char *src_wk;                                          \
        UCHAR chr;                                                   \
        type *dst_wk;                                                \
        BOOL backwards;                                              \
        int i;                                                       \
                                                                     \
        backwards = FALSE;                                           \
        if ((char *)dst < src) {                                     \
            if ((char *)(dst + 2 * (length - 1)) > src + length - 1) \
                return -1;                                           \
        } else if ((char *)dst < src + length)                       \
            backwards = TRUE;                                        \
        if (backwards) {                                             \
            for (i = 0, src_wk = src + length - 1,                   \
                dst_wk = dst + 2 * length - 1;                       \
                 i < length; i++, src_wk--) {                        \
                chr = *src_wk;                                       \
                *dst_wk-- = hextbl[chr % 16];                        \
                *dst_wk-- = hextbl[chr >> 4];                        \
            }                                                        \
        } else {                                                     \
            for (i = 0, src_wk = src, dst_wk = dst; i < length;      \
                 i++, src_wk++) {                                    \
                chr = *src_wk;                                       \
                *dst_wk++ = hextbl[chr >> 4];                        \
                *dst_wk++ = hextbl[chr % 16];                        \
            }                                                        \
        }                                                            \
        dst[2 * length] = '\0';                                      \
        return 2 * length * sizeof(type);                            \
    }
#ifdef UNICODE_SUPPORT
static SQLLEN es_bin2whex def_bin2hex(SQLWCHAR)
#endif /* UNICODE_SUPPORT */

    static SQLLEN es_bin2hex def_bin2hex(char)

        SQLLEN es_hex2bin(const char *src, char *dst, SQLLEN length) {
    UCHAR chr;
    const char *src_wk;
    char *dst_wk;
    SQLLEN i;
    int val;
    BOOL HByte = TRUE;

    for (i = 0, src_wk = src, dst_wk = dst; i < length; i++, src_wk++) {
        chr = *src_wk;
        if (!chr)
            break;
        if (chr >= 'a' && chr <= 'f')
            val = chr - 'a' + 10;
        else if (chr >= 'A' && chr <= 'F')
            val = chr - 'A' + 10;
        else
            val = chr - '0';
        if (HByte)
            *dst_wk = (char)(val << 4);
        else {
            *dst_wk += (char)val;
            dst_wk++;
        }
        HByte = !HByte;
    }
    *dst_wk = '\0';
    return length;
}

static int convert_lo(StatementClass *stmt, const void *value,
                      SQLSMALLINT fCType, PTR rgbValue, SQLLEN cbValueMax,
                      SQLLEN *pcbValue) {
    UNUSED(cbValueMax, pcbValue, rgbValue, fCType, value);
    SC_set_error(stmt, STMT_EXEC_ERROR,
                 "Could not convert large object to c-type (large objects are "
                 "not supported).",
                 "convert_lo");
    return COPY_GENERAL_ERROR;
}