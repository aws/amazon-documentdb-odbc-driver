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

#ifdef WIN32

#include "apifunc.h"
#include "dlg_specific.h"
#include "loadlib.h"
#include "misc.h"  // strncpy_null
#include "win_setup.h"
#ifdef _HANDLE_ENLIST_IN_DTC_
#include "connexp.h"
#include "xalibname.h"
#endif /* _HANDLE_ENLIST_IN_DTC_ */
#include <ShlObj_core.h>

#define AUTHMODE_CNT 1  // TODO: update if new authentication method is added
#define LOGLEVEL_CNT 8
extern HINSTANCE s_hModule;

int loglevels[LOGLEVEL_CNT] = {{IDS_LOGTYPE_OFF},   {IDS_LOGTYPE_FATAL},
                               {IDS_LOGTYPE_ERROR}, {IDS_LOGTYPE_WARNING},
                               {IDS_LOGTYPE_INFO},  {IDS_LOGTYPE_DEBUG},
                               {IDS_LOGTYPE_TRACE}, {IDS_LOGTYPE_ALL}};

// TODO: If new authentication method added, update authmodes array below
// Create new macro "IDS_AUTHTYPE_NEWAUTHTYPE" for new auth method in resource.h
static const struct authmode authmodes[AUTHMODE_CNT] = {
    {IDS_AUTHTYPE_DEFAULT, AUTHTYPE_DEFAULT}};

const struct authmode *GetCurrentAuthMode(HWND hdlg) {
    unsigned int ams_cnt = 0;
    const struct authmode *ams = GetAuthModes(&ams_cnt);
    unsigned int authtype_selection_idx = (unsigned int)(DWORD)SendMessage(
        GetDlgItem(hdlg, IDC_AUTHTYPE), CB_GETCURSEL, 0L, 0L);
    if (authtype_selection_idx >= ams_cnt)
        authtype_selection_idx = 0;
    return &ams[authtype_selection_idx];
}

int *GetLogLevels(unsigned int *count) {
    *count = LOGLEVEL_CNT;
    return loglevels;
}

int GetCurrentLogLevel(HWND hdlg) {
    unsigned int log_cnt = 0;
    int *log = GetLogLevels(&log_cnt);
    unsigned int loglevel_selection_idx = (unsigned int)(DWORD)SendMessage(
        GetDlgItem(hdlg, IDC_LOG_LEVEL), CB_GETCURSEL, 0L, 0L);
    if (loglevel_selection_idx >= log_cnt)
        loglevel_selection_idx = 0;
    return log[loglevel_selection_idx];
}

void SetLoggingVisibility(HWND hdlg, int logLevel) {
    if (logLevel == IDS_LOGTYPE_OFF) {
        EnableWindow(GetDlgItem(hdlg, IDC_LOG_PATH), FALSE);
        EnableWindow(GetDlgItem(hdlg, IDC_BROWSE_BTN), FALSE);
        EnableWindow(GetDlgItem(hdlg, IDOK), TRUE);
    } else {
        EnableWindow(GetDlgItem(hdlg, IDC_LOG_PATH), TRUE);
        EnableWindow(GetDlgItem(hdlg, IDC_BROWSE_BTN), TRUE);

        char buf[LARGE_REGISTRY_LEN];
        GetDlgItemText(hdlg, IDC_LOG_PATH, buf, sizeof(buf));
        if (strcmp(buf, "") == 0) {
            EnableWindow(GetDlgItem(hdlg, IDOK), FALSE);
        } else {
            EnableWindow(GetDlgItem(hdlg, IDOK), TRUE);
        }
    }
}

// TODO: If connection string has been modified (e.g. new authentication method)
// Update SetAuthenticationVisibility accordingly (add else if statement)
void SetAuthenticationVisibility(HWND hdlg, const struct authmode *am) {
    if (strcmp(am->authtype_str, AUTHTYPE_DEFAULT) == 0) {
        ShowWindow(GetDlgItem(hdlg, IDC_USER), TRUE);
        ShowWindow(GetDlgItem(hdlg, IDC_PASSWORD), TRUE);
    }
}

void SetDlgStuff(HWND hdlg, const ConnInfo *ci) {
    // Connection
    SetDlgItemText(hdlg, IDC_DRIVER_VERSION, "V." DATABASEDRIVERVERSION);
    SetDlgItemText(hdlg, IDC_DSNAME, ci->dsn);

    // Authentication
    int authtype_selection_idx = 0;
    unsigned int ams_cnt = 0;
    const struct authmode *ams = GetAuthModes(&ams_cnt);
    char buff[MEDIUM_REGISTRY_LEN + 1];
    for (unsigned int i = 0; i < ams_cnt; i++) {
        LoadString(GetWindowInstance(hdlg), ams[i].authtype_id, buff,
                   MEDIUM_REGISTRY_LEN);
        SendDlgItemMessage(hdlg, IDC_AUTHTYPE, CB_ADDSTRING, 0, (LPARAM)buff);
        if (stricmp(ci->authtype, ams[i].authtype_str) == 0) {
            authtype_selection_idx = i;
        }
    }
    SendDlgItemMessage(hdlg, IDC_AUTHTYPE, CB_SETCURSEL,
                       ams[authtype_selection_idx].authtype_id, (WPARAM)0);
    SetAuthenticationVisibility(hdlg, &ams[authtype_selection_idx]);

    // TODO: If connection string has been modified (e.g. new authentication
    // method) Update code below accordingly (add else if statement)
    if (strcmp(ci->authtype, AUTHTYPE_DEFAULT) == 0) {
        SetDlgItemText(hdlg, IDC_USER, ci->uid);
        SetDlgItemText(hdlg, IDC_PASSWORD, SAFE_NAME(ci->pwd));
    }
}

static void GetNameField(HWND hdlg, int item, esNAME *name) {
    char medium_buf[MEDIUM_REGISTRY_LEN + 1];
    GetDlgItemText(hdlg, item, medium_buf, sizeof(medium_buf));
    STR_TO_NAME((*name), medium_buf);
}

void GetDlgStuff(HWND hdlg, ConnInfo *ci) {
    // Connection

    // Authentication
    const struct authmode *am = GetCurrentAuthMode(hdlg);
    SetAuthenticationVisibility(hdlg, am);
    STRCPY_FIXED(ci->authtype, am->authtype_str);

    // TODO: If connection string has been modified (e.g. new authentication
    // method) Update code below accordingly (add else if statement)
    if (strcmp(ci->authtype, AUTHTYPE_DEFAULT) == 0) {
        GetDlgItemText(hdlg, IDC_USER, ci->uid, sizeof(ci->uid));
        GetNameField(hdlg, IDC_PASSWORD, &ci->pwd);
    }
}

const struct authmode *GetAuthModes(unsigned int *count) {
    *count = AUTHMODE_CNT;
    return authmodes;
}
static void getDriversDefaultsOfCi(const ConnInfo *ci, GLOBAL_VALUES *glbv) {
    const char *drivername = NULL;

    if (ci->drivername[0])
        drivername = ci->drivername;
    else if (NAME_IS_VALID(ci->drivers.drivername))
        drivername = SAFE_NAME(ci->drivers.drivername);
    if (drivername && drivername[0])
        getDriversDefaults(drivername, glbv);
    else
        getDriversDefaults(INVALID_DRIVER, glbv);
}

INT_PTR CALLBACK advancedOptionsProc(HWND hdlg, UINT wMsg, WPARAM wParam,
                                     LPARAM lParam) {
    switch (wMsg) {
        case WM_INITDIALOG: {
            SetWindowLongPtr(hdlg, DWLP_USER, lParam);
            ConnInfo *ci = (ConnInfo *)lParam;
            SetDlgItemText(hdlg, IDC_REQUEST_TIMEOUT, ci->request_timeout);
            SetDlgItemText(hdlg, IDC_CONNECTION_TIMEOUT,
                           ci->connection_timeout);
            SetDlgItemText(hdlg, IDC_MAX_RETRY_COUNT_CLIENT,
                           ci->max_retry_count_client);
            SetDlgItemText(hdlg, IDC_MAX_CONNECTIONS, ci->max_connections);
            break;
        }

        case WM_COMMAND: {
            ConnInfo *ci = (ConnInfo *)GetWindowLongPtr(hdlg, DWLP_USER);
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDOK:
                    // Get Dialog Values
                    GetDlgItemText(hdlg, IDC_REQUEST_TIMEOUT,
                                   ci->request_timeout,
                                   sizeof(ci->request_timeout));
                    GetDlgItemText(hdlg, IDC_CONNECTION_TIMEOUT,
                                   ci->connection_timeout,
                                   sizeof(ci->connection_timeout));
                    GetDlgItemText(hdlg, IDC_MAX_RETRY_COUNT_CLIENT,
                                   ci->max_retry_count_client,
                                   sizeof(ci->max_retry_count_client));
                    GetDlgItemText(hdlg, IDC_MAX_CONNECTIONS,
                                   ci->max_connections,
                                   sizeof(ci->max_connections));
                case IDCANCEL:
                    EndDialog(hdlg, FALSE);
                    return TRUE;
            }
        }
    }
    return FALSE;
}

static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam,
                                       LPARAM lpData) {
    if (uMsg == BFFM_SETSELECTION) {
        UNUSED(lParam);
        LPCTSTR path = (LPCTSTR)lpData;
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)path);
    }
    return 0;
}

INT_PTR CALLBACK logOptionsProc(HWND hdlg, UINT wMsg, WPARAM wParam,
                                LPARAM lParam) {
    switch (wMsg) {
        case WM_INITDIALOG: {
            ConnInfo *ci = (ConnInfo *)lParam;
            SetWindowLongPtr(hdlg, DWLP_USER, lParam);

            // Logging
            int loglevel_selection_idx = 0;
            unsigned int log_cnt = 0;
            int *log = GetLogLevels(&log_cnt);
            char buff[MEDIUM_REGISTRY_LEN + 1];
            for (unsigned int i = 0; i < log_cnt; i++) {
                LoadString(GetWindowInstance(hdlg), log[i], buff,
                           MEDIUM_REGISTRY_LEN);
                SendDlgItemMessage(hdlg, IDC_LOG_LEVEL, CB_ADDSTRING, 0,
                                   (WPARAM)buff);
                if ((unsigned int)ci->drivers.loglevel == i) {
                    loglevel_selection_idx = i;
                }
            }
            SendDlgItemMessage(hdlg, IDC_LOG_LEVEL, CB_SETCURSEL,
                               loglevel_selection_idx, (WPARAM)0);
            SetDlgItemText(hdlg, IDC_LOG_PATH, ci->drivers.output_dir);
            break;
        }

        case WM_COMMAND: {
            ConnInfo *ci = (ConnInfo *)GetWindowLongPtr(hdlg, DWLP_USER);
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDC_BROWSE_BTN: {
                    BROWSEINFO bi = {0};
                    bi.lpszTitle = ("Choose log file target directory:");
                    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
                    bi.lpfn = BrowseCallbackProc;
                    bi.lParam = (LPARAM)ci->drivers.output_dir;

                    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

                    if (pidl != NULL) {
                        // get the name of the folder and put it in path
                        SHGetPathFromIDList(pidl, ci->drivers.output_dir);
                        SetDlgItemText(hdlg, IDC_LOG_PATH,
                                       ci->drivers.output_dir);
                        // free memory used
                        CoTaskMemFree(pidl);
                    }
                    break;
                }
                case IDC_LOG_LEVEL: {
                    SetLoggingVisibility(hdlg, GetCurrentLogLevel(hdlg));
                    break;
                }
                case IDOK: {
                    // Get Dialog Values
                    int log = GetCurrentLogLevel(hdlg);
                    switch (log) {
                        case IDS_LOGTYPE_OFF:
                            ci->drivers.loglevel = (char)LOG_OFF;
                            break;
                        case IDS_LOGTYPE_FATAL:
                            ci->drivers.loglevel = (char)LOG_FATAL;
                            break;
                        case IDS_LOGTYPE_ERROR:
                            ci->drivers.loglevel = (char)LOG_ERROR;
                            break;
                        case IDS_LOGTYPE_WARNING:
                            ci->drivers.loglevel = (char)LOG_WARNING;
                            break;
                        case IDS_LOGTYPE_INFO:
                            ci->drivers.loglevel = (char)LOG_INFO;
                            break;
                        case IDS_LOGTYPE_DEBUG:
                            ci->drivers.loglevel = (char)LOG_DEBUG;
                            break;
                        case IDS_LOGTYPE_TRACE:
                            ci->drivers.loglevel = (char)LOG_TRACE;
                            break;
                        case IDS_LOGTYPE_ALL:
                            ci->drivers.loglevel = (char)LOG_ALL;
                            break;
                        default:
                            ci->drivers.loglevel = (char)LOG_OFF;
                            break;
                    }
                    setGlobalCommlog(ci->drivers.loglevel);
                    setGlobalDebug(ci->drivers.loglevel);
                    writeGlobalLogs();
                    GetDlgItemText(hdlg, IDC_LOG_PATH, ci->drivers.output_dir,
                                   sizeof(ci->drivers.output_dir));
                    setLogDir(ci->drivers.output_dir);
                }

                case IDCANCEL: {
                    EndDialog(hdlg, FALSE);
                    return TRUE;
                }
            }
        }
        case WM_ENTERIDLE: {
            SetLoggingVisibility(hdlg, GetCurrentLogLevel(hdlg));
            break;
        }
    }
    return FALSE;
}

#endif /* WIN32 */
