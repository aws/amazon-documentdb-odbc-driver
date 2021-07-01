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

#ifndef __INFO_H__
#define __INFO_H__
#include "helper.h"
#include "odbc.h"
#include "unicode_support.h"

#ifndef WIN32
#include <ctype.h>
#endif

#include "apifunc.h"
#include "bind.h"
#include "catfunc.h"
#include "connection.h"
#include "dlg_specific.h"
#include "environ.h"
#include "misc.h"
#include "multibyte.h"
#include "statement.h"
#include "tuple.h"
#include "types.h"

// C Interface
#ifdef __cplusplus
extern "C" {
#endif
RETCODE SQL_API API_Tables(HSTMT hstmt, const SQLCHAR* catalog_name_sql,
                             const SQLSMALLINT catalog_name_sz,
                             const SQLCHAR* schema_name_sql,
                             const SQLSMALLINT schema_name_sz,
                             const SQLCHAR* table_name_sql,
                             const SQLSMALLINT table_name_sz,
                             const SQLCHAR* table_type_sql,
                             const SQLSMALLINT table_type_sz, const UWORD flag);
RETCODE SQL_API
API_Columns(HSTMT hstmt, const SQLCHAR* catalog_name_sql,
              const SQLSMALLINT catalog_name_sz, const SQLCHAR* schema_name_sql,
              const SQLSMALLINT schema_name_sz, const SQLCHAR* table_name_sql,
              const SQLSMALLINT table_name_sz, const SQLCHAR* column_name_sql,
              const SQLSMALLINT column_name_sz, const UWORD flag,
              const OID reloid, const Int2 attnum);

RETCODE SQL_API API_GetTypeInfo(HSTMT hstmt, SQLSMALLINT fSqlType);
#ifdef __cplusplus
}
#endif

#endif /* __INFO_H__ */
