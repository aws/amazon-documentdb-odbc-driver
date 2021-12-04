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

#include "dlg_specific.h"

#include <ctype.h>

#include "apifunc.h"
#include "misc.h"

#define NULL_IF_NULL(a) ((a) ? ((const char *)(a)) : "(null)")

#define OVR_EXTRA_BITS                                                      \
    (BIT_FORCEABBREVCONNSTR | BIT_FAKE_MSS | BIT_BDE_ENVIRONMENT            \
     | BIT_CVT_NULL_DATE | BIT_ACCESSIBLE_ONLY | BIT_IGNORE_ROUND_TRIP_TIME \
     | BIT_DISABLE_KEEPALIVE)

#define OPENING_BRACKET '{'
#define CLOSING_BRACKET '}'

#ifdef __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wembedded-directive"
#endif  // __APPLE__
void makeConnectString(char *connect_string, const ConnInfo *ci, UWORD len) {
    UNUSED(len);
    char got_dsn = (ci->dsn[0] != '\0');
    char *connsetStr = NULL;
    char *esoptStr = NULL;
#ifdef _HANDLE_ENLIST_IN_DTC_
    char xaOptStr[16];
#endif
    ssize_t hlen, nlen, olen = -1;

    /* fundamental info */
    nlen = MAX_CONNECT_STRING;

    // TODO: For each new authentication method, add else if statement for
    // AUTHTYPE and define respective connection string
    if (strcmp(ci->authtype, AUTHTYPE_DEFAULT) == 0) {
        olen = snprintf(
            connect_string, nlen,
            "%s=%s;"
            INI_AUTH_MODE "=%s;"
            INI_UID "=%s;"
            INI_PWD "=%s;" 
            INI_LOG_LEVEL "=%d;" 
            INI_LOG_OUTPUT "=%s;" 
            INI_REQUEST_TIMEOUT "=%s;" 
            INI_CONNECTION_TIMEOUT "=%s;" 
            INI_MAX_RETRY_COUNT_CLIENT "=%s" 
            INI_MAX_CONNECTIONS "=%s;",
            got_dsn ? "DSN" : INI_DRIVER, got_dsn ? ci->dsn : ci->drivername,
            ci->authtype,
            ci->uid, 
            SAFE_NAME(ci->pwd),
            (int)ci->drivers.loglevel,
            ci->drivers.output_dir,
            ci->request_timeout,
            ci->connection_timeout,
            ci->max_retry_count_client,
            ci->max_connections);
    }

    if (olen < 0 || olen >= nlen) {
        connect_string[0] = '\0';
        return;
    }

    /* extra info */
    hlen = strlen(connect_string);
    nlen = MAX_CONNECT_STRING - hlen;
    if (olen < 0 || olen >= nlen) /* failed */
        connect_string[0] = '\0';

    if (NULL != connsetStr)
        free(connsetStr);
    if (NULL != esoptStr)
        free(esoptStr);
}
#ifdef __APPLE__
#pragma clang diagnostic pop
#endif  // __APPLE__

BOOL get_DSN_or_Driver(ConnInfo *ci, const char *attribute, const char *value) {
    BOOL found = TRUE;

    if (stricmp(attribute, "DSN") == 0)
        STRCPY_FIXED(ci->dsn, value);
    else if (stricmp(attribute, INI_DRIVER) == 0)
        STRCPY_FIXED(ci->drivername, value);
    else
        found = FALSE;

    return found;
}

// TODO: If connection string has been modified (e.g. new authentication method)
// Update copyConnAttributes accordingly
BOOL copyConnAttributes(ConnInfo *ci, const char *attribute,
                        const char *value) {
    BOOL found = TRUE, printed = FALSE;
    if (stricmp(attribute, "DSN") == 0) {
        STRCPY_FIXED(ci->dsn, value);
    } else if (stricmp(attribute, INI_DRIVER) == 0) {
        STRCPY_FIXED(ci->drivername, value);
    } else if (stricmp(attribute, INI_UID) == 0) {
        STRCPY_FIXED(ci->uid, value);
    } else if (stricmp(attribute, INI_PWD) == 0) {
        STR_TO_NAME(ci->pwd, value);
        MYLOG(LOG_DEBUG, "key='%s' value='xxxxxxxx'", attribute);
        printed = TRUE;
    } else if (stricmp(attribute, INI_AUTH_MODE) == 0) {
        STRCPY_FIXED(ci->authtype, value);
    } else if (stricmp(attribute, INI_LOG_LEVEL) == 0) {
        ci->drivers.loglevel = (char)atoi(value);
    } else if (stricmp(attribute, INI_LOG_OUTPUT) == 0) {
        STRCPY_FIXED(ci->drivers.output_dir, value);
    } else if (stricmp(attribute, INI_REQUEST_TIMEOUT) == 0) {
        STRCPY_FIXED(ci->request_timeout, value);
    } else if (stricmp(attribute, INI_CONNECTION_TIMEOUT) == 0) {
        STRCPY_FIXED(ci->connection_timeout, value);
    } else if (stricmp(attribute, INI_MAX_RETRY_COUNT_CLIENT) == 0) {
        STRCPY_FIXED(ci->max_retry_count_client, value);
    } else if (stricmp(attribute, INI_MAX_CONNECTIONS) == 0) {
        STRCPY_FIXED(ci->max_connections, value);
    } else {
        found = FALSE;
    }

    if (!printed)
        MYLOG(LOG_DEBUG, "key='%s' value='%s'%s", attribute, value,
              found ? NULL_STRING : " not found");

    return found;
}

// TODO: If connection string has been modified (e.g. new authentication method)
// Update getCiDefaults accordingly
static void getCiDefaults(ConnInfo *ci) {
    strncpy(ci->drivername, DEFAULT_DRIVERNAME, MEDIUM_REGISTRY_LEN);
    strncpy(ci->request_timeout, DEFAULT_REQUEST_TIMEOUT_STR,
            SMALL_REGISTRY_LEN);
    strncpy(ci->connection_timeout, DEFAULT_CONNECTION_TIMEOUT_STR,
            SMALL_REGISTRY_LEN);
    strncpy(ci->max_retry_count_client, DEFAULT_NONE, SMALL_REGISTRY_LEN);
    strncpy(ci->max_connections, DEFAULT_MAX_CONNECTIONS_STR,
            SMALL_REGISTRY_LEN);
    strncpy(ci->authtype, DEFAULT_AUTHTYPE, MEDIUM_REGISTRY_LEN);
    if (ci->pwd.name != NULL)
        free(ci->pwd.name);
    ci->pwd.name = NULL;
    strncpy(ci->uid, DEFAULT_NONE, MEDIUM_REGISTRY_LEN);
#if defined(__APPLE__) || defined(__linux__)
    strcpy(ci->drivers.output_dir, "/tmp");
#else
    strcpy(ci->drivers.output_dir, "");
#endif
}

#ifdef __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wembedded-directive"
#endif  // __APPLE__
int getDriverNameFromDSN(const char *dsn, char *driver_name, int namelen) {
#ifdef WIN32
    return SQLGetPrivateProfileString(ODBC_DATASOURCES, dsn, NULL_STRING,
                                      driver_name, namelen, ODBC_INI);
#else  /* WIN32 */
    int cnt;

    cnt = SQLGetPrivateProfileString(dsn, INI_DRIVER, NULL_STRING, driver_name,
                                     namelen, ODBC_INI);
    if (!driver_name[0])
        return cnt;
    if (strchr(driver_name, '/') || /* path to the driver */
        strchr(driver_name, '.')) {
        driver_name[0] = '\0';
        return 0;
    }
    return cnt;
#endif /* WIN32 */
#ifdef __APPLE__
#pragma clang diagnostic pop
#endif  // __APPLE__
}

void getDriversDefaults(const char *drivername, GLOBAL_VALUES *comval) {
    if (NULL != drivername)
        STR_TO_NAME(comval->drivername, drivername);
}

void getDSNinfo(ConnInfo *ci, const char *configDrvrname) {
    char *DSN = ci->dsn;
    char temp[LARGE_REGISTRY_LEN];
    const char *drivername;
    getCiDefaults(ci);
    drivername = ci->drivername;
    if (DSN[0] == '\0') {
        if (drivername[0] == '\0') /* adding new DSN via configDSN */
        {
            if (configDrvrname)
                drivername = configDrvrname;
            strncpy_null(DSN, "DSN", sizeof(ci->dsn));
        }
        /* else dns-less connections */
    }

    /* brute-force chop off trailing blanks... */
    while (*(DSN + strlen(DSN) - 1) == ' ')
        *(DSN + strlen(DSN) - 1) = '\0';

    if (!drivername[0] && DSN[0])
        getDriverNameFromDSN(DSN, (char *)drivername, sizeof(ci->drivername));
    MYLOG(LOG_DEBUG, "drivername=%s", drivername);
    if (!drivername[0])
        drivername = INVALID_DRIVER;
    getDriversDefaults(drivername, &(ci->drivers));

    if (DSN[0] == '\0')
        return;

        /* Proceed with getting info for the given DSN. */
#ifdef __linux
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress"
#endif
    // TODO: If connection string has been modified (e.g. new authentication
    // method) Update code below accordingly
    if (SQLGetPrivateProfileString(DSN, INI_AUTH_MODE, NULL_STRING, temp,
                                   sizeof(temp), ODBC_INI)
        > 0)
        STRCPY_FIXED(ci->authtype, temp);
    if (strcmp(ci->authtype, AUTHTYPE_DEFAULT) == 0) {
        if (SQLGetPrivateProfileString(DSN, INI_UID, NULL_STRING, temp,
                                       sizeof(temp), ODBC_INI)
            > 0)
            STRCPY_FIXED(ci->uid, temp);
        if (SQLGetPrivateProfileString(DSN, INI_PWD, NULL_STRING, temp,
                                       sizeof(temp), ODBC_INI)

            > 0)
            STR_TO_NAME(ci->pwd, temp);
    }
    if (SQLGetPrivateProfileString(DSN, INI_LOG_LEVEL, NULL_STRING, temp,
                                   sizeof(temp), ODBC_INI)
        > 0)
        ci->drivers.loglevel = (char)atoi(temp);
    if (SQLGetPrivateProfileString(DSN, INI_LOG_OUTPUT, NULL_STRING, temp,
                                   sizeof(temp), ODBC_INI)
        > 0)
        STRCPY_FIXED(ci->drivers.output_dir, temp);
    if (SQLGetPrivateProfileString(DSN, INI_REQUEST_TIMEOUT, NULL_STRING, temp,
                                   sizeof(temp), ODBC_INI)
        > 0)
        STRCPY_FIXED(ci->request_timeout, temp);
    if (SQLGetPrivateProfileString(DSN, INI_CONNECTION_TIMEOUT, NULL_STRING,
                                   temp, sizeof(temp), ODBC_INI)
        > 0)
        STRCPY_FIXED(ci->connection_timeout, temp);
    if (SQLGetPrivateProfileString(DSN, INI_MAX_RETRY_COUNT_CLIENT, NULL_STRING,
                                   temp, sizeof(temp), ODBC_INI)
        > 0)
        STRCPY_FIXED(ci->max_retry_count_client, temp);
    if (SQLGetPrivateProfileString(DSN, INI_MAX_CONNECTIONS, NULL_STRING, temp,
                                   sizeof(temp), ODBC_INI)
        > 0)
        STRCPY_FIXED(ci->max_connections, temp);

    STR_TO_NAME(ci->drivers.drivername, drivername);

#ifdef __linux
#pragma GCC diagnostic pop
#endif
}
/*
 *	This function writes any global parameters (that can be manipulated)
 *	to the ODBCINST.INI portion of the registry
 */
int write_Ci_Drivers(const char *fileName, const char *sectionName,
                     const GLOBAL_VALUES *comval) {
    UNUSED(comval, fileName, sectionName);

    // We don't need anything here
    return 0;
}

int writeDriversDefaults(const char *drivername, const GLOBAL_VALUES *comval) {
    return write_Ci_Drivers(ODBCINST_INI, drivername, comval);
}

/*	This is for datasource based options only */
// TODO: If connection string has been modified (e.g. new authentication method)
// Update writeDSNinfo accordingly
void writeDSNinfo(const ConnInfo *ci) {
    const char *DSN = ci->dsn;
    char temp[SMALL_REGISTRY_LEN];

    SQLWritePrivateProfileString(DSN, INI_UID, ci->uid, ODBC_INI);
    SQLWritePrivateProfileString(DSN, INI_PWD, SAFE_NAME(ci->pwd), ODBC_INI);
    SQLWritePrivateProfileString(DSN, INI_AUTH_MODE, ci->authtype, ODBC_INI);
    ITOA_FIXED(temp, ci->drivers.loglevel);
    SQLWritePrivateProfileString(DSN, INI_LOG_LEVEL, temp, ODBC_INI);
    SQLWritePrivateProfileString(DSN, INI_LOG_OUTPUT, ci->drivers.output_dir,
                                 ODBC_INI);
    SQLWritePrivateProfileString(DSN, INI_REQUEST_TIMEOUT, ci->request_timeout,
                                 ODBC_INI);
    SQLWritePrivateProfileString(DSN, INI_CONNECTION_TIMEOUT,
                                 ci->connection_timeout, ODBC_INI);
    SQLWritePrivateProfileString(DSN, INI_MAX_RETRY_COUNT_CLIENT,
                                 ci->max_retry_count_client, ODBC_INI);
    SQLWritePrivateProfileString(DSN, INI_MAX_CONNECTIONS, ci->max_connections,
                                 ODBC_INI);
}

void CC_conninfo_release(ConnInfo *conninfo) {
    NULL_THE_NAME(conninfo->pwd);
    finalize_globals(&conninfo->drivers);
}

// TODO: If connection string has been modified (e.g. new authentication method)
// Update CC_conninfo_init accordingly
void CC_conninfo_init(ConnInfo *conninfo, UInt4 option) {
    MYLOG(LOG_TRACE, "entering opt=%d", option);

    if (0 != (CLEANUP_FOR_REUSE & option))
        CC_conninfo_release(conninfo);
    memset(conninfo, 0, sizeof(ConnInfo));

    strncpy(conninfo->dsn, DEFAULT_NONE, MEDIUM_REGISTRY_LEN);
    strncpy(conninfo->drivername, DEFAULT_DRIVERNAME, MEDIUM_REGISTRY_LEN);
    strncpy(conninfo->request_timeout, DEFAULT_REQUEST_TIMEOUT_STR,
            SMALL_REGISTRY_LEN);
    strncpy(conninfo->connection_timeout, DEFAULT_CONNECTION_TIMEOUT_STR,
            SMALL_REGISTRY_LEN);
    strncpy(conninfo->max_retry_count_client, DEFAULT_NONE, SMALL_REGISTRY_LEN);
    strncpy(conninfo->max_connections, DEFAULT_MAX_CONNECTIONS_STR,
            SMALL_REGISTRY_LEN);
    strncpy(conninfo->authtype, DEFAULT_AUTHTYPE, MEDIUM_REGISTRY_LEN);
    if (conninfo->pwd.name != NULL)
        free(conninfo->pwd.name);
    conninfo->pwd.name = NULL;
    strncpy(conninfo->uid, DEFAULT_NONE, MEDIUM_REGISTRY_LEN);

    if (0 != (INIT_GLOBALS & option))
        init_globals(&(conninfo->drivers));
}

void init_globals(GLOBAL_VALUES *glbv) {
    memset(glbv, 0, sizeof(*glbv));
    glbv->loglevel = DEFAULT_LOGLEVEL;
    glbv->output_dir[0] = '\0';
}

#define CORR_STRCPY(item) strncpy_null(to->item, from->item, sizeof(to->item))
#define CORR_VALCPY(item) (to->item = from->item)

void copy_globals(GLOBAL_VALUES *to, const GLOBAL_VALUES *from) {
    memset(to, 0, sizeof(*to));
    NAME_TO_NAME(to->drivername, from->drivername);
    CORR_VALCPY(loglevel);
}

void finalize_globals(GLOBAL_VALUES *glbv) {
    NULL_THE_NAME(glbv->drivername);
}

#undef CORR_STRCPY
#undef CORR_VALCPY
#define CORR_STRCPY(item) strncpy_null(ci->item, sci->item, sizeof(ci->item))
#define CORR_VALCPY(item) (ci->item = sci->item)

// TODO: If connection string has been modified (e.g. new authentication method)
// Update CC_copy_conninfo accordingly
void CC_copy_conninfo(ConnInfo *ci, const ConnInfo *sci) {
    memset(ci, 0, sizeof(ConnInfo));
    CORR_STRCPY(dsn);
    CORR_STRCPY(drivername);
    CORR_STRCPY(uid);
    CORR_STRCPY(authtype);
    NAME_TO_NAME(ci->pwd, sci->pwd);
    CORR_STRCPY(request_timeout);
    CORR_STRCPY(connection_timeout);
    CORR_STRCPY(max_retry_count_client);
    CORR_STRCPY(max_connections);
    copy_globals(&(ci->drivers), &(sci->drivers));
}
#undef CORR_STRCPY
#undef CORR_VALCPY
