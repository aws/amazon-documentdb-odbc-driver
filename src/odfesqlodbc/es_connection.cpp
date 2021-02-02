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

/*	TryEnterCriticalSection needs the following #define */
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif /* _WIN32_WINNT */

#include "es_connection.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "misc.h"

/* for htonl */
#ifdef WIN32
#include <Winsock2.h>
#else
#include <arpa/inet.h>
#endif

#include <map>
#include <string>
#include <stdexcept>
#include "dlg_specific.h"
#include "environ.h"
#include "es_apifunc.h"
#include "es_helper.h"
#include "loadlib.h"
#include "multibyte.h"
#include "qresult.h"
#include "statement.h"

#define ERROR_BUFF_SIZE 200

void CC_determine_locale_encoding(ConnectionClass *self);

void* LIB_connect(ConnectionClass *self) {
    if (self == nullptr) {
        throw std::invalid_argument("ConnectionClass is nullptr.");
    }
    // Setup options
    runtime_options rt_opts;
    // Connection
    rt_opts.conn.timeout.assign(self->connInfo.request_timeout);
    rt_opts.conn.connection_timeout.assign(self->connInfo.connection_timeout);
    rt_opts.conn.max_connections.assign(self->connInfo.max_connections);
    // Authentication
    rt_opts.auth.auth_type.assign(self->connInfo.authtype);
    rt_opts.auth.uid.assign(self->connInfo.uid);
    rt_opts.auth.pwd.assign(SAFE_NAME(self->connInfo.pwd));
    rt_opts.auth.session_token.assign(self->connInfo.session_token);
    rt_opts.auth.region.assign(self->connInfo.region);
    rt_opts.auth.end_point_override.assign(self->connInfo.end_point_override);
    rt_opts.auth.idp_name.assign(self->connInfo.idp_name);
    rt_opts.auth.idp_host.assign(self->connInfo.idp_host);
    rt_opts.auth.okta_application_id.assign(self->connInfo.okta_application_id);
    rt_opts.auth.role_arn.assign(self->connInfo.role_arn);
    rt_opts.auth.aad_application_id.assign(self->connInfo.aad_application_id);
    rt_opts.auth.aad_client_secret.assign(self->connInfo.aad_client_secret);
    rt_opts.auth.aad_tenant.assign(self->connInfo.aad_tenant);
    rt_opts.auth.idp_arn.assign(self->connInfo.idp_arn);

    auto conn = static_cast< void * >(ConnectDBParams(rt_opts));
    if (conn == nullptr) {
        throw std::runtime_error("Communication is nullptr.");
    }
    // Set sdk version
    std::string version = GetVersion(conn);
    STRCPY_FIXED(self->version, version.c_str());

    return conn;
}

char CC_connect(ConnectionClass *self) {
    try {
        if (self == nullptr)
            throw std::invalid_argument("ConnectionClass is nullptr");

        // Attempt to connect
        self->conn = LIB_connect(self);

        // Set encodings
        CC_determine_locale_encoding(self);
#ifdef UNICODE_SUPPORT
        if (CC_is_in_unicode_driver(self)) {
            if (!SQL_SUCCEEDED(CC_send_client_encoding(self, "UTF8"))) {
                throw std::runtime_error("Cannot set UTF8 as client encoding");
            }
        } else
#endif
        {
            if (!SQL_SUCCEEDED(
                    CC_send_client_encoding(self, self->locale_encoding))) {
                throw std::runtime_error("Cannot set " + std::string(self->locale_encoding)
                                         + " as client encoding");
            }
        }

        // Set cursor parameters based on connection info
        self->status = CONN_CONNECTED;
        if ((CC_is_in_unicode_driver(self)) && (CC_is_in_ansi_app(self)))
            self->unicode |= CONN_DISALLOW_WCHAR;

        // 1 is SQL_SUCCESS and 2 is SQL_SCCUESS_WITH_INFO
        return 1;
    } catch (const std::exception& e) {
        CC_set_error(self, CONN_OPENDB_ERROR, e.what(), "CC_connect");
        return 0;
    }
}

// TODO #36 - When we fix encoding, we should look into returning a code here.
// This is called in connection.c and the return code isn't checked
void CC_set_locale_encoding(ConnectionClass *self, const char *encoding) {
    if (self == NULL)
        return;

    // Set encoding
    char *prev_encoding = self->locale_encoding;
    self->locale_encoding = (encoding == NULL) ? NULL : strdup(encoding);
    if (prev_encoding)
        free(prev_encoding);
}

// TODO #36 - Add return code - see above function comment
void CC_determine_locale_encoding(ConnectionClass *self) {
    // Don't update if it's already set
    if ((self == NULL) || (self->locale_encoding != NULL))
        return;

    // Get current db encoding and derive the locale encoding
    // TODO #34 - Investigate locale
    CC_set_locale_encoding(self, "SQL_ASCII");
}

int CC_send_client_encoding(ConnectionClass *self, const char *encoding) {
    if ((self == NULL) || (encoding == NULL))
        return SQL_ERROR;

    // Update client encoding
    std::string des_db_encoding(encoding);
    std::string cur_db_encoding = GetClientEncoding(self->conn);
    if (des_db_encoding != cur_db_encoding) {
        if (!SetClientEncoding(self->conn, des_db_encoding)) {
            return SQL_ERROR;
        }
    }

    // Update connection class to reflect updated client encoding
    char *prev_encoding = self->original_client_encoding;
    self->original_client_encoding = strdup(des_db_encoding.c_str());
    self->ccsc = static_cast< short >(es_CS_code(des_db_encoding.c_str()));
    self->mb_maxbyte_per_char = static_cast< short >(es_mb_maxlen(self->ccsc));
    if (prev_encoding != NULL)
        free(prev_encoding);

    return SQL_SUCCESS;
}

void CC_initialize_version(ConnectionClass *self) {
    STRCPY_FIXED(self->version, "1.7.329");
    self->version_major = 1;
    self->version_minor = 7;
    self->version_patch = 329;
}

void LIB_disconnect(void *conn) {
    Disconnect(conn);
}
