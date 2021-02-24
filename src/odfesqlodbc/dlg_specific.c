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

#include "es_apifunc.h"
#include "misc.h"

#define NULL_IF_NULL(a) ((a) ? ((const char *)(a)) : "(null)")

static void encode(const esNAME, char *out, int outlen);
static esNAME decode(const char *in);
static esNAME decode_or_remove_braces(const char *in);

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
    char encoded_item[LARGE_REGISTRY_LEN];
    char *connsetStr = NULL;
    char *esoptStr = NULL;
#ifdef _HANDLE_ENLIST_IN_DTC_
    char xaOptStr[16];
#endif
    ssize_t hlen, nlen, olen=-1;

    encode(ci->pwd, encoded_item, sizeof(encoded_item));
    /* fundamental info */
    nlen = MAX_CONNECT_STRING;
    if (strcmp(ci->authtype, AUTHTYPE_AWS_PROFILE) == 0) {
        olen = snprintf(
            connect_string, nlen,
            "%s=%s;"
            INI_AUTH_MODE "=%s;"
            INI_PROFILE_NAME "=%s;"
            INI_REGION "=%s;"
            INI_END_POINT_OVERRIDE "=%s;"
            INI_LOG_LEVEL "=%d;"
            INI_LOG_OUTPUT "=%s;"
            INI_REQUEST_TIMEOUT "=%s;"
            INI_CONNECTION_TIMEOUT "=%s;"
            INI_MAX_RETRY_COUNT_CLIENT "=%s;"
            INI_MAX_CONNECTIONS "=%s;",
            got_dsn ? "DSN" : INI_DRIVER, got_dsn ? ci->dsn : ci->drivername,
            ci->authtype,
            ci->profile_name,
            ci->region,
            ci->end_point_override,
            (int)ci->drivers.loglevel,
            ci->drivers.output_dir,
            ci->request_timeout,
            ci->connection_timeout,
            ci->max_retry_count_client,
            ci->max_connections);
    } else if (strcmp(ci->authtype, AUTHTYPE_IAM) == 0) {
        olen = snprintf(
            connect_string, nlen,
            "%s=%s;"
            INI_AUTH_MODE "=%s;"
            INI_UID "=%s;"
            INI_PWD "=%s;"
            INI_SESSION_TOKEN "=%s;"
            INI_REGION "=%s;"
            INI_END_POINT_OVERRIDE "=%s;"
            INI_LOG_LEVEL "=%d;"
            INI_LOG_OUTPUT "=%s;"
            INI_REQUEST_TIMEOUT "=%s;"
            INI_CONNECTION_TIMEOUT "=%s;"
            INI_MAX_RETRY_COUNT_CLIENT "=%s;"
            INI_MAX_CONNECTIONS "=%s;",
            got_dsn ? "DSN" : INI_DRIVER, got_dsn ? ci->dsn : ci->drivername,
            ci->authtype,
            ci->uid, 
            encoded_item, 
            ci->session_token,
            ci->region,
            ci->end_point_override,
            (int)ci->drivers.loglevel,
            ci->drivers.output_dir,
            ci->request_timeout,
            ci->connection_timeout,
            ci->max_retry_count_client,
            ci->max_connections);
    } else if (strcmp(ci->authtype, AUTHTYPE_AAD) == 0) {
        olen = snprintf(
            connect_string, nlen,
            "%s=%s;"
            INI_AUTH_MODE "=%s;"
            INI_UID "=%s;"
            INI_PWD "=%s;"
            INI_IDP_NAME "=%s;"
            INI_AAD_APPLICATION_ID "=%s;"
            INI_AAD_CLIENT_SECRET "=%s;"
            INI_AAD_TENANT "=%s;"
            INI_IDP_ARN "=%s;"
            INI_REGION "=%s;"
            INI_END_POINT_OVERRIDE "=%s;"
            INI_LOG_LEVEL "=%d;"
            INI_LOG_OUTPUT "=%s;"
            INI_REQUEST_TIMEOUT "=%s;"
            INI_CONNECTION_TIMEOUT "=%s;"
            INI_MAX_RETRY_COUNT_CLIENT "=%s;"
            INI_MAX_CONNECTIONS "=%s;",
            got_dsn ? "DSN" : INI_DRIVER, got_dsn ? ci->dsn : ci->drivername,
            ci->authtype,
            ci->uid, 
            encoded_item, 
            ci->idp_name,
            ci->aad_application_id,
            ci->aad_client_secret,
            ci->aad_tenant,
            ci->idp_arn,
            ci->region,
            ci->end_point_override,
            (int)ci->drivers.loglevel,
            ci->drivers.output_dir,
            ci->request_timeout,
            ci->connection_timeout,
            ci->max_retry_count_client,
            ci->max_connections);
    } else if (strcmp(ci->authtype, AUTHTYPE_OKTA) == 0) {
        olen = snprintf(
            connect_string, nlen,
            "%s=%s;"
            INI_AUTH_MODE "=%s;"
            INI_UID "=%s;"
            INI_PWD "=%s;"
            INI_IDP_NAME "=%s;"
            INI_IDP_HOST "=%s;"
            INI_OKTA_APPLICATION_ID "=%s;"
            INI_ROLE_ARN "=%s;"
            INI_IDP_ARN "=%s;"
            INI_REGION "=%s;"
            INI_END_POINT_OVERRIDE "=%s;"
            INI_LOG_LEVEL "=%d;"
            INI_LOG_OUTPUT "=%s;"
            INI_REQUEST_TIMEOUT "=%s;"
            INI_CONNECTION_TIMEOUT "=%s;"
            INI_MAX_RETRY_COUNT_CLIENT "=%s;"
            INI_MAX_CONNECTIONS "=%s;",
            got_dsn ? "DSN" : INI_DRIVER, got_dsn ? ci->dsn : ci->drivername,
            ci->authtype,
            ci->uid, 
            encoded_item, 
            ci->idp_name,
            ci->idp_host,
            ci->okta_application_id,
            ci->role_arn,
            ci->idp_arn,
            ci->region,
            ci->end_point_override,
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

BOOL copyConnAttributes(ConnInfo *ci, const char *attribute,
                        const char *value) {
    BOOL found = TRUE, printed = FALSE;
    if (stricmp(attribute, "DSN") == 0)
        STRCPY_FIXED(ci->dsn, value);
    else if (stricmp(attribute, INI_DRIVER) == 0)
        STRCPY_FIXED(ci->drivername, value);
    else if (stricmp(attribute, INI_ACCESS_KEY_ID) == 0
             || stricmp(attribute, INI_UID) == 0
             || stricmp(attribute, INI_IDP_USERNAME) == 0)
        STRCPY_FIXED(ci->uid, value);
    else if (stricmp(attribute, INI_SECRET_ACCESS_KEY) == 0
             || stricmp(attribute, INI_PWD) == 0
             || stricmp(attribute, INI_IDP_PASSWORD) == 0) {
        ci->pwd = decode_or_remove_braces(value);
#ifndef FORCE_PASSWORDE_DISPLAY
        MYLOG(LOG_DEBUG, "key='%s' value='xxxxxxxx'\n", attribute);
        printed = TRUE;
#endif
    } else if (stricmp(attribute, INI_SESSION_TOKEN) == 0)
        STRCPY_FIXED(ci->session_token, value);
    else if (stricmp(attribute, INI_AUTH_MODE) == 0)
        STRCPY_FIXED(ci->authtype, value);
    else if (stricmp(attribute, INI_PROFILE_NAME) == 0)
        STRCPY_FIXED(ci->profile_name, value);
    else if (stricmp(attribute, INI_REGION) == 0)
        STRCPY_FIXED(ci->region, value);
    else if (stricmp(attribute, INI_END_POINT_OVERRIDE) == 0)
        STRCPY_FIXED(ci->end_point_override, value);
    else if (stricmp(attribute, INI_LOG_LEVEL) == 0)
        ci->drivers.loglevel = (char)atoi(value);
    else if (stricmp(attribute, INI_LOG_OUTPUT) == 0)
        STRCPY_FIXED(ci->drivers.output_dir, value);
    else if (stricmp(attribute, INI_REQUEST_TIMEOUT) == 0)
        STRCPY_FIXED(ci->request_timeout, value);
    else if (stricmp(attribute, INI_CONNECTION_TIMEOUT) == 0)
        STRCPY_FIXED(ci->connection_timeout, value);
    else if (stricmp(attribute, INI_MAX_RETRY_COUNT_CLIENT) == 0)
        STRCPY_FIXED(ci->max_retry_count_client, value);
    else if (stricmp(attribute, INI_MAX_CONNECTIONS) == 0)
        STRCPY_FIXED(ci->max_connections, value);
    else if (stricmp(attribute, INI_IDP_NAME) == 0)
        STRCPY_FIXED(ci->idp_name, value);
    else if (stricmp(attribute, INI_IDP_HOST) == 0)
        STRCPY_FIXED(ci->idp_host, value);
    else if (stricmp(attribute, INI_OKTA_APPLICATION_ID) == 0)
        STRCPY_FIXED(ci->okta_application_id, value);
    else if (stricmp(attribute, INI_ROLE_ARN) == 0)
        STRCPY_FIXED(ci->role_arn, value);
    else if (stricmp(attribute, INI_AAD_APPLICATION_ID) == 0)
        STRCPY_FIXED(ci->aad_application_id, value);
    else if (stricmp(attribute, INI_AAD_CLIENT_SECRET) == 0)
        STRCPY_FIXED(ci->aad_client_secret, value);
    else if (stricmp(attribute, INI_AAD_TENANT) == 0)
        STRCPY_FIXED(ci->aad_tenant, value);
    else if (stricmp(attribute, INI_IDP_ARN) == 0)
        STRCPY_FIXED(ci->idp_arn, value);
    else
        found = FALSE;

    if (!printed)
        MYLOG(LOG_DEBUG, "key='%s' value='%s'%s\n", attribute, value,
              found ? NULL_STRING : " not found");

    return found;
}

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
    strncpy(ci->profile_name, DEFAULT_NONE, MEDIUM_REGISTRY_LEN);
    if (ci->pwd.name != NULL)
        free(ci->pwd.name);
    ci->pwd.name = NULL;
    strncpy(ci->uid, DEFAULT_NONE, MEDIUM_REGISTRY_LEN);
    strncpy(ci->session_token, DEFAULT_NONE, LARGE_REGISTRY_LEN);
    strncpy(ci->region, DEFAULT_REGION, MEDIUM_REGISTRY_LEN);
    strncpy(ci->end_point_override, DEFAULT_NONE, MEDIUM_REGISTRY_LEN);
    strncpy(ci->idp_name, DEFAULT_NONE, MEDIUM_REGISTRY_LEN);
    strncpy(ci->idp_host, DEFAULT_NONE, MEDIUM_REGISTRY_LEN);
    strncpy(ci->okta_application_id, DEFAULT_NONE, MEDIUM_REGISTRY_LEN);
    strncpy(ci->role_arn, DEFAULT_NONE, MEDIUM_REGISTRY_LEN);
    strncpy(ci->idp_arn, DEFAULT_NONE, MEDIUM_REGISTRY_LEN);
    strncpy(ci->aad_application_id, DEFAULT_NONE, MEDIUM_REGISTRY_LEN);
    strncpy(ci->aad_client_secret, DEFAULT_NONE, MEDIUM_REGISTRY_LEN);
    strncpy(ci->aad_tenant, DEFAULT_NONE, MEDIUM_REGISTRY_LEN);
#ifdef __APPLE__
    strcpy(ci->drivers.output_dir, "/tmp/");
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
    MYLOG(LOG_DEBUG, "drivername=%s\n", drivername);
    if (!drivername[0])
        drivername = INVALID_DRIVER;
    getDriversDefaults(drivername, &(ci->drivers));

    if (DSN[0] == '\0')
        return;

    /* Proceed with getting info for the given DSN. */
    if (SQLGetPrivateProfileString(DSN, INI_AUTH_MODE, NULL_STRING, temp,
                                   sizeof(temp), ODBC_INI)
        > 0)
        STRCPY_FIXED(ci->authtype, temp);
    if (SQLGetPrivateProfileString(DSN, INI_PROFILE_NAME, NULL_STRING, temp,
                                   sizeof(temp), ODBC_INI)
        > 0)
        STRCPY_FIXED(ci->profile_name, temp);
    if (strcmp(ci->authtype, AUTHTYPE_IAM) == 0) {
        if (SQLGetPrivateProfileString(DSN, INI_ACCESS_KEY_ID, NULL_STRING,
                                       temp, sizeof(temp), ODBC_INI)
            > 0)
            STRCPY_FIXED(ci->uid, temp);
        if (SQLGetPrivateProfileString(DSN, INI_SECRET_ACCESS_KEY, NULL_STRING,
                                       temp, sizeof(temp), ODBC_INI)
            > 0)
            ci->pwd = decode(temp);
    } else if (strcmp(ci->authtype, AUTHTYPE_AAD) == 0
               || strcmp(ci->authtype, AUTHTYPE_OKTA) == 0) {
        if (SQLGetPrivateProfileString(DSN, INI_IDP_USERNAME, NULL_STRING, temp,
                                       sizeof(temp), ODBC_INI)
            > 0)
            STRCPY_FIXED(ci->uid, temp);
        if (SQLGetPrivateProfileString(DSN, INI_IDP_PASSWORD, NULL_STRING, temp,
                                       sizeof(temp), ODBC_INI)
            > 0)
            ci->pwd = decode(temp);
    }
    if (strcmp(ci->authtype, AUTHTYPE_AWS_PROFILE) != 0) {
        if (SQLGetPrivateProfileString(DSN, INI_UID, NULL_STRING, temp,
                                       sizeof(temp), ODBC_INI)
            > 0)
            STRCPY_FIXED(ci->uid, temp);
        if (SQLGetPrivateProfileString(DSN, INI_PWD, NULL_STRING, temp,
                                       sizeof(temp), ODBC_INI)
            > 0)
            ci->pwd = decode(temp);
    }
    if (SQLGetPrivateProfileString(DSN, INI_SESSION_TOKEN, NULL_STRING, temp,
                                   sizeof(temp), ODBC_INI)
        > 0)
        STRCPY_FIXED(ci->session_token, temp);
    if (SQLGetPrivateProfileString(DSN, INI_REGION, NULL_STRING, temp,
                                   sizeof(temp), ODBC_INI)
        > 0)
        STRCPY_FIXED(ci->region, temp);
    if (SQLGetPrivateProfileString(DSN, INI_END_POINT_OVERRIDE, NULL_STRING, temp,
                                   sizeof(temp), ODBC_INI)
        > 0)
        STRCPY_FIXED(ci->end_point_override, temp);
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
    if (SQLGetPrivateProfileString(DSN, INI_CONNECTION_TIMEOUT, NULL_STRING, temp,
                                   sizeof(temp), ODBC_INI)
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
    if (SQLGetPrivateProfileString(DSN, INI_IDP_NAME, NULL_STRING, temp,
                                   sizeof(temp), ODBC_INI)
        > 0)
        STRCPY_FIXED(ci->idp_name, temp);
    if (SQLGetPrivateProfileString(DSN, INI_IDP_HOST, NULL_STRING, temp,
                                   sizeof(temp), ODBC_INI)
        > 0)
        STRCPY_FIXED(ci->idp_host, temp);
    if (SQLGetPrivateProfileString(DSN, INI_OKTA_APPLICATION_ID, NULL_STRING,
                                   temp,
                                   sizeof(temp), ODBC_INI)
        > 0)
        STRCPY_FIXED(ci->okta_application_id, temp);
    if (SQLGetPrivateProfileString(DSN, INI_ROLE_ARN, NULL_STRING, temp,
                                   sizeof(temp), ODBC_INI)
        > 0)
        STRCPY_FIXED(ci->role_arn, temp);
    if (SQLGetPrivateProfileString(DSN, INI_AAD_APPLICATION_ID, NULL_STRING,
                                   temp,
                                   sizeof(temp), ODBC_INI)
        > 0)
        STRCPY_FIXED(ci->aad_application_id, temp);
    if (SQLGetPrivateProfileString(DSN, INI_AAD_CLIENT_SECRET, NULL_STRING,
                                   temp,
                                   sizeof(temp), ODBC_INI)
        > 0)
        STRCPY_FIXED(ci->aad_client_secret, temp);
    if (SQLGetPrivateProfileString(DSN, INI_AAD_TENANT, NULL_STRING,
                                   temp, sizeof(temp), ODBC_INI)
        > 0)
        STRCPY_FIXED(ci->aad_tenant, temp);
    if (SQLGetPrivateProfileString(DSN, INI_IDP_ARN, NULL_STRING,
                                   temp, sizeof(temp), ODBC_INI)
        > 0)
        STRCPY_FIXED(ci->idp_arn, temp);

    STR_TO_NAME(ci->drivers.drivername, drivername);
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
void writeDSNinfo(const ConnInfo *ci) {
    const char *DSN = ci->dsn;
    char encoded_item[MEDIUM_REGISTRY_LEN], temp[SMALL_REGISTRY_LEN];

    SQLWritePrivateProfileString(DSN, INI_UID, ci->uid, ODBC_INI);
    encode(ci->pwd, encoded_item, sizeof(encoded_item));
    SQLWritePrivateProfileString(DSN, INI_PWD, encoded_item, ODBC_INI);
    SQLWritePrivateProfileString(DSN, INI_SESSION_TOKEN, ci->session_token, ODBC_INI);
    SQLWritePrivateProfileString(DSN, INI_AUTH_MODE, ci->authtype, ODBC_INI);
    SQLWritePrivateProfileString(DSN, INI_PROFILE_NAME, ci->profile_name, ODBC_INI);
    SQLWritePrivateProfileString(DSN, INI_REGION, ci->region, ODBC_INI);
    SQLWritePrivateProfileString(DSN, INI_END_POINT_OVERRIDE, ci->end_point_override, ODBC_INI);
    ITOA_FIXED(temp, ci->drivers.loglevel);
    SQLWritePrivateProfileString(DSN, INI_LOG_LEVEL, temp, ODBC_INI);
    SQLWritePrivateProfileString(DSN, INI_LOG_OUTPUT, ci->drivers.output_dir,
                                 ODBC_INI);
    SQLWritePrivateProfileString(DSN, INI_REQUEST_TIMEOUT, ci->request_timeout,
                                 ODBC_INI);
    SQLWritePrivateProfileString(DSN, INI_CONNECTION_TIMEOUT, ci->connection_timeout,
                                 ODBC_INI);
    SQLWritePrivateProfileString(DSN, INI_MAX_RETRY_COUNT_CLIENT, ci->max_retry_count_client,
                                 ODBC_INI);
    SQLWritePrivateProfileString(DSN, INI_MAX_CONNECTIONS, ci->max_connections,
                                 ODBC_INI);
    SQLWritePrivateProfileString(DSN, INI_IDP_NAME, ci->idp_name,
                                 ODBC_INI);
    SQLWritePrivateProfileString(DSN, INI_IDP_HOST, ci->idp_host,
                                 ODBC_INI);
    SQLWritePrivateProfileString(DSN, INI_OKTA_APPLICATION_ID,
                                 ci->okta_application_id,
                                 ODBC_INI);
    SQLWritePrivateProfileString(DSN, INI_ROLE_ARN, ci->role_arn,
                                 ODBC_INI);
    SQLWritePrivateProfileString(DSN, INI_AAD_APPLICATION_ID,
                                 ci->aad_application_id,
                                 ODBC_INI);
    SQLWritePrivateProfileString(DSN, INI_AAD_CLIENT_SECRET,
                                 ci->aad_client_secret,
                                 ODBC_INI);
    SQLWritePrivateProfileString(DSN, INI_AAD_TENANT, ci->aad_tenant,
                                 ODBC_INI);
    SQLWritePrivateProfileString(DSN, INI_IDP_ARN, ci->idp_arn,
                                 ODBC_INI);
}

static void encode(const esNAME in, char *out, int outlen) {
    size_t i, ilen = 0;
    int o = 0;
    char inc, *ins;

    if (NAME_IS_NULL(in)) {
        out[0] = '\0';
        return;
    }
    ins = GET_NAME(in);
    ilen = strlen(ins);
    for (i = 0; i < ilen && o < outlen - 1; i++) {
        inc = ins[i];
        if (inc == '+') {
            if (o + 2 >= outlen)
                break;
            snprintf(&out[o], outlen - o, "%%2B");
            o += 3;
        } else if (isspace((unsigned char)inc))
            out[o++] = '+';
        else if (!isalnum((unsigned char)inc)) {
            if (o + 2 >= outlen)
                break;
            snprintf(&out[o], outlen - o, "%%%02x", inc);
            o += 3;
        } else
            out[o++] = inc;
    }
    out[o++] = '\0';
}

static unsigned int conv_from_hex(const char *s) {
    int i, y = 0, val;

    for (i = 1; i <= 2; i++) {
        if (s[i] >= 'a' && s[i] <= 'f')
            val = s[i] - 'a' + 10;
        else if (s[i] >= 'A' && s[i] <= 'F')
            val = s[i] - 'A' + 10;
        else
            val = s[i] - '0';

        y += val << (4 * (2 - i));
    }

    return y;
}

static esNAME decode(const char *in) {
    size_t i, ilen = strlen(in), o = 0;
    char inc, *outs;
    esNAME out;

    INIT_NAME(out);
    if (0 == ilen) {
        return out;
    }
    outs = (char *)malloc(ilen + 1);
    if (!outs)
        return out;
    for (i = 0; i < ilen; i++) {
        inc = in[i];
        if (inc == '+')
            outs[o++] = ' ';
        else if (inc == '%') {
            snprintf(&outs[o], ilen + 1 - o, "%c", conv_from_hex(&in[i]));
            o++;
            i += 2;
        } else
            outs[o++] = inc;
    }
    outs[o++] = '\0';
    STR_TO_NAME(out, outs);
    free(outs);
    return out;
}

/*
 *	Remove braces if the input value is enclosed by braces({}).
 *	Othewise decode the input value.
 */
static esNAME decode_or_remove_braces(const char *in) {
    if (OPENING_BRACKET == in[0]) {
        size_t inlen = strlen(in);
        if (CLOSING_BRACKET == in[inlen - 1]) /* enclosed with braces */
        {
            int i;
            const char *istr, *eptr;
            char *ostr;
            esNAME out;

            INIT_NAME(out);
            if (NULL == (ostr = (char *)malloc(inlen)))
                return out;
            eptr = in + inlen - 1;
            for (istr = in + 1, i = 0; *istr && istr < eptr; i++) {
                if (CLOSING_BRACKET == istr[0] && CLOSING_BRACKET == istr[1])
                    istr++;
                ostr[i] = *(istr++);
            }
            ostr[i] = '\0';
            SET_NAME_DIRECTLY(out, ostr);
            return out;
        }
    }
    return decode(in);
}

void CC_conninfo_release(ConnInfo *conninfo) {
    NULL_THE_NAME(conninfo->pwd);
    finalize_globals(&conninfo->drivers);
}

void CC_conninfo_init(ConnInfo *conninfo, UInt4 option) {
    MYLOG(LOG_TRACE, "entering opt=%d\n", option);

    if (0 != (CLEANUP_FOR_REUSE & option))
        CC_conninfo_release(conninfo);
    memset(conninfo, 0, sizeof(ConnInfo));

    strncpy(conninfo->dsn, DEFAULT_NONE, MEDIUM_REGISTRY_LEN);
    strncpy(conninfo->drivername, DEFAULT_DRIVERNAME, MEDIUM_REGISTRY_LEN);
    strncpy(conninfo->request_timeout, DEFAULT_REQUEST_TIMEOUT_STR,
            SMALL_REGISTRY_LEN);
    strncpy(conninfo->connection_timeout, DEFAULT_CONNECTION_TIMEOUT_STR,
            SMALL_REGISTRY_LEN);
    strncpy(conninfo->max_retry_count_client, DEFAULT_NONE,
            SMALL_REGISTRY_LEN);
    strncpy(conninfo->max_connections, DEFAULT_MAX_CONNECTIONS_STR,
            SMALL_REGISTRY_LEN);
    strncpy(conninfo->authtype, DEFAULT_AUTHTYPE, MEDIUM_REGISTRY_LEN);
    strncpy(conninfo->profile_name, DEFAULT_NONE, MEDIUM_REGISTRY_LEN);
    if (conninfo->pwd.name != NULL)
        free(conninfo->pwd.name);
    conninfo->pwd.name = NULL;
    strncpy(conninfo->uid, DEFAULT_NONE, MEDIUM_REGISTRY_LEN);
    strncpy(conninfo->session_token, DEFAULT_NONE, LARGE_REGISTRY_LEN);
    strncpy(conninfo->region, DEFAULT_REGION, MEDIUM_REGISTRY_LEN);
    strncpy(conninfo->end_point_override, DEFAULT_NONE, MEDIUM_REGISTRY_LEN);
    strncpy(conninfo->idp_name, DEFAULT_NONE, MEDIUM_REGISTRY_LEN);
    strncpy(conninfo->idp_host, DEFAULT_NONE, MEDIUM_REGISTRY_LEN);
    strncpy(conninfo->okta_application_id, DEFAULT_NONE, MEDIUM_REGISTRY_LEN);
    strncpy(conninfo->role_arn, DEFAULT_NONE, MEDIUM_REGISTRY_LEN);
    strncpy(conninfo->aad_application_id, DEFAULT_NONE, MEDIUM_REGISTRY_LEN);
    strncpy(conninfo->aad_client_secret, DEFAULT_NONE, MEDIUM_REGISTRY_LEN);
    strncpy(conninfo->aad_tenant, DEFAULT_NONE, MEDIUM_REGISTRY_LEN);
    strncpy(conninfo->idp_arn, DEFAULT_NONE, MEDIUM_REGISTRY_LEN);

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

void CC_copy_conninfo(ConnInfo *ci, const ConnInfo *sci) {
    memset(ci, 0, sizeof(ConnInfo));
    CORR_STRCPY(dsn);
    CORR_STRCPY(drivername);
    CORR_STRCPY(uid);
    CORR_STRCPY(authtype);
    CORR_STRCPY(profile_name);
    CORR_STRCPY(region);
    CORR_STRCPY(end_point_override);
    NAME_TO_NAME(ci->pwd, sci->pwd);
    CORR_STRCPY(session_token);
    CORR_STRCPY(request_timeout);
    CORR_STRCPY(connection_timeout);
    CORR_STRCPY(max_retry_count_client);
    CORR_STRCPY(max_connections);
    CORR_STRCPY(idp_name);
    CORR_STRCPY(idp_host);
    CORR_STRCPY(okta_application_id);
    CORR_STRCPY(role_arn);
    CORR_STRCPY(aad_application_id);
    CORR_STRCPY(aad_client_secret);
    CORR_STRCPY(aad_tenant);
    CORR_STRCPY(idp_arn);
    copy_globals(&(ci->drivers), &(sci->drivers));
}
#undef CORR_STRCPY
#undef CORR_VALCPY
