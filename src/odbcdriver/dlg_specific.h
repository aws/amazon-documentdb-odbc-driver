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

#ifndef __DLG_SPECIFIC_H__
#define __DLG_SPECIFIC_H__

#include "odbc.h"

#ifdef WIN32
#include <windowsx.h>

#include "resource.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/*	Unknown data type sizes */
#define UNKNOWNS_AS_MAX 0
#define UNKNOWNS_AS_DONTKNOW 1
#define UNKNOWNS_AS_LONGEST 2

/* ODBC initialization files */
#ifdef WIN32
#define ODBC_INI "ODBC.INI"
#define ODBCINST_INI "ODBCINST.INI"
#elif __APPLE__
#define ODBC_INI ".odbc.ini"
#define ODBCINST_INI "odbcinst.ini"
#elif __linux__
#define ODBC_INI "odbc.ini"
#define ODBCINST_INI "odbcinst.ini"
#endif

#define ODBC_DATASOURCES "ODBC Data Sources"
#define INVALID_DRIVER " @@driver not exist@@ "

#ifdef UNICODE_SUPPORT
#define INI_DSN "Timestream35W"
#else
#define INI_DSN "Timestream30"
#endif /* UNICODE_SUPPORT */

#define INI_DRIVER "Driver"
#define INI_UID "UID"
#define INI_PWD "PWD"
#define INI_AUTH_MODE "Auth"
// Logging
#define INI_LOG_LEVEL "LogLevel"
#define INI_LOG_OUTPUT "LogOutput"
// Advanced
#define INI_REQUEST_TIMEOUT "RequestTimeout"
#define INI_CONNECTION_TIMEOUT "ConnectionTimeout"
#define INI_MAX_RETRY_COUNT_CLIENT "MaxRetryCountClient"
#define INI_MAX_CONNECTIONS "MaxConnections"

#define AUTHTYPE_DEFAULT "DEFAULT"

#define DEFAULT_REQUEST_TIMEOUT 3000
#define DEFAULT_REQUEST_TIMEOUT_STR "3000"
#define DEFAULT_CONNECTION_TIMEOUT 1000
#define DEFAULT_CONNECTION_TIMEOUT_STR "1000"
#define DEFAULT_MAX_CONNECTIONS 25
#define DEFAULT_MAX_CONNECTIONS_STR "25"
#define DEFAULT_AUTHTYPE AUTHTYPE_DEFAULT
#define DEFAULT_DRIVERNAME "timestreamodbc"
#define DEFAULT_NONE ""



#ifdef _HANDLE_ENLIST_IN_DTC_
#define INI_XAOPT "XaOpt"
#endif /* _HANDLE_ENLIST_IN_DTC_ */
/* Bit representation for abbreviated connection strings */
#define BIT_LFCONVERSION (1L)
#define BIT_UPDATABLECURSORS (1L << 1)
/* #define BIT_DISALLOWPREMATURE                  (1L<<2) */
#define BIT_UNIQUEINDEX (1L << 3)
#define BIT_UNKNOWN_DONTKNOW (1L << 6)
#define BIT_UNKNOWN_ASMAX (1L << 7)
#define BIT_COMMLOG (1L << 10)
#define BIT_DEBUG (1L << 11)
#define BIT_PARSE (1L << 12)
#define BIT_CANCELASFREESTMT (1L << 13)
#define BIT_USEDECLAREFETCH (1L << 14)
#define BIT_READONLY (1L << 15)
#define BIT_TEXTASLONGVARCHAR (1L << 16)
#define BIT_UNKNOWNSASLONGVARCHAR (1L << 17)
#define BIT_BOOLSASCHAR (1L << 18)
#define BIT_ROWVERSIONING (1L << 19)
#define BIT_SHOWSYSTEMTABLES (1L << 20)
#define BIT_SHOWOIDCOLUMN (1L << 21)
#define BIT_FAKEOIDINDEX (1L << 22)
#define BIT_TRUEISMINUS1 (1L << 23)
#define BIT_BYTEAASLONGVARBINARY (1L << 24)
#define BIT_USESERVERSIDEPREPARE (1L << 25)
#define BIT_LOWERCASEIDENTIFIER (1L << 26)

#define EFFECTIVE_BIT_COUNT 28

/*	Mask for extra options	*/
#define BIT_FORCEABBREVCONNSTR 1L
#define BIT_FAKE_MSS (1L << 1)
#define BIT_BDE_ENVIRONMENT (1L << 2)
#define BIT_CVT_NULL_DATE (1L << 3)
#define BIT_ACCESSIBLE_ONLY (1L << 4)
#define BIT_IGNORE_ROUND_TRIP_TIME (1L << 5)
#define BIT_DISABLE_KEEPALIVE (1L << 6)

/*	Connection Defaults */
#define DEFAULT_READONLY 1
#define DEFAULT_PROTOCOL              \
    "7.4" /* the latest protocol is \ \
           * the default */
#define DEFAULT_USEDECLAREFETCH 0
#define DEFAULT_TEXTASLONGVARCHAR 0
#define DEFAULT_UNKNOWNSASLONGVARCHAR 0
#define DEFAULT_BOOLSASCHAR 0
#define DEFAULT_UNIQUEINDEX 1 /* dont recognize */
#define DEFAULT_LOGLEVEL LOG_OFF
#define DEFAULT_TRUST_SELF_SIGNED 0
#define DEFAULT_AUTH_MODE "IAM"
#define DEFAULT_CERTIFICATE ""
#define DEFAULT_KEY ""
#define DEFAULT_UNKNOWNSIZES UNKNOWNS_AS_MAX

#define DEFAULT_FAKEOIDINDEX 0
#define DEFAULT_SHOWOIDCOLUMN 0
#define DEFAULT_ROWVERSIONING 0
#define DEFAULT_SHOWSYSTEMTABLES 0 /* dont show system tables */
#define DEFAULT_LIE 0
#define DEFAULT_PARSE 0

#define DEFAULT_CANCELASFREESTMT 0

#define DEFAULT_EXTRASYSTABLEPREFIXES ""

#define DEFAULT_TRUEISMINUS1 0
#define DEFAULT_UPDATABLECURSORS 1
#ifdef WIN32
#define DEFAULT_LFCONVERSION 1
#else
#define DEFAULT_LFCONVERSION 0
#endif /* WIN32 */
#define DEFAULT_INT8AS 0
#define DEFAULT_BYTEAASLONGVARBINARY 0
#define DEFAULT_USESERVERSIDEPREPARE 1
#define DEFAULT_LOWERCASEIDENTIFIER 0
#define DEFAULT_NUMERIC_AS (-101)

#ifdef _HANDLE_ENLIST_IN_DTC_
#define DEFAULT_XAOPT 1
#endif /* _HANDLE_ENLIST_IN_DTC_ */

/*	for CC_DSN_info */
#define CONN_DONT_OVERWRITE 0
#define CONN_OVERWRITE 1

struct authmode {
    int authtype_id;
    const char *authtype_str;
};
const struct authmode *GetAuthModes();

/*	prototypes */

#ifdef WIN32
void SetDlgStuff(HWND hdlg, const ConnInfo *ci);
void GetDlgStuff(HWND hdlg, ConnInfo *ci);
INT_PTR CALLBACK advancedOptionsProc(HWND hdlg, UINT wMsg, WPARAM wParam,
                                 LPARAM lParam);
INT_PTR CALLBACK logOptionsProc(HWND hdlg, UINT wMsg, WPARAM wParam, LPARAM lParam);
#endif /* WIN32 */

int write_Ci_Drivers(const char *fileName, const char *sectionName,
                     const GLOBAL_VALUES *);
int writeDriversDefaults(const char *drivername, const GLOBAL_VALUES *);
void writeDSNinfo(const ConnInfo *ci);
void getDriversDefaults(const char *drivername, GLOBAL_VALUES *);
void getDSNinfo(ConnInfo *ci, const char *configDrvrname);
void makeConnectString(char *connect_string, const ConnInfo *ci, UWORD);
BOOL get_DSN_or_Driver(ConnInfo *ci, const char *attribute, const char *value);
BOOL copyConnAttributes(ConnInfo *ci, const char *attribute, const char *value);
int getDriverNameFromDSN(const char *dsn, char *driver_name, int namelen);
UInt4 getExtraOptions(const ConnInfo *);
void SetAuthenticationVisibility(HWND hdlg, const struct authmode *am);
const struct authmode *GetCurrentAuthMode(HWND hdlg);
int *GetLogLevels();
int GetCurrentLogLevel(HWND hdlg);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __DLG_SPECIFIC_H__ */