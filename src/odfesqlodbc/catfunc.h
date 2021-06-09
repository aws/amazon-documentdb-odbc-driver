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

#ifndef __CATFUNC_H__
#define __CATFUNC_H__

#include "odbc.h"

/*	SQLTables field position	*/
enum {
    TABLES_CATALOG_NAME = 0,
    TABLES_SCHEMA_NAME,
    TABLES_TABLE_NAME,
    TABLES_TABLE_TYPE,
    TABLES_REMARKS,
    NUM_OF_TABLES_FIELDS
};

/*	SQLColumns field position	*/
enum {
    COLUMNS_CATALOG_NAME = 0,
    COLUMNS_SCHEMA_NAME,
    COLUMNS_TABLE_NAME,
    COLUMNS_COLUMN_NAME,
    COLUMNS_DATA_TYPE,
    COLUMNS_TYPE_NAME,
    COLUMNS_PRECISION,
    COLUMNS_LENGTH,
    COLUMNS_SCALE,
    COLUMNS_RADIX,
    COLUMNS_NULLABLE,
    COLUMNS_REMARKS,
    COLUMNS_COLUMN_DEF /* ODBC 3.0 but always use it */
    ,
    COLUMNS_SQL_DATA_TYPE,
    COLUMNS_SQL_DATETIME_SUB,
    COLUMNS_CHAR_OCTET_LENGTH,
    COLUMNS_ORDINAL_POSITION,
    COLUMNS_IS_NULLABLE,
    COLUMNS_DISPLAY_SIZE,
    COLUMNS_FIELD_TYPE,
    COLUMNS_AUTO_INCREMENT,
    COLUMNS_PHYSICAL_NUMBER,
    COLUMNS_TABLE_OID,
    COLUMNS_BASE_TYPEID,
    COLUMNS_ATTTYPMOD,
    COLUMNS_TABLE_INFO,
    NUM_OF_COLUMNS_FIELDS
};
/*	SQLPrimaryKeys field position	*/
enum {
    PKS_TABLE_CAT = 0,
    PKS_TABLE_SCHEM,
    PKS_TABLE_NAME,
    PKS_COLUMN_NAME,
    PKS_KEY_SQ,
    PKS_PK_NAME,
    NUM_OF_PKS_FIELDS
};
/*	SQLForeignKeys field position	*/
enum {
    FKS_PKTABLE_CAT = 0,
    FKS_PKTABLE_SCHEM,
    FKS_PKTABLE_NAME,
    FKS_PKCOLUMN_NAME,
    FKS_FKTABLE_CAT,
    FKS_FKTABLE_SCHEM,
    FKS_FKTABLE_NAME,
    FKS_FKCOLUMN_NAME,
    FKS_KEY_SEQ,
    FKS_UPDATE_RULE,
    FKS_DELETE_RULE,
    FKS_FK_NAME,
    FKS_PK_NAME,
    FKS_DEFERRABILITY,
    FKS_TRIGGER_NAME,
    NUM_OF_FKS_FIELDS
};
/* SQLColAttribute */
enum {
    COLATTR_DESC_COUNT = -1,
    COLATTR_DESC_AUTO_UNIQUE_VALUE = 0,
    COLATTR_DESC_BASE_COLUMN_NAME,
    COLATTR_DESC_BASE_TABLE_NAME,
    COLATTR_DESC_CASE_SENSITIVE,
    COLATTR_DESC_CATALOG_NAME,
    COLATTR_DESC_CONCISE_TYPE,
    COLATTR_DESC_DISPLAY_SIZE,
    COLATTR_DESC_FIXED_PREC_SCALE,
    COLATTR_DESC_LABEL,
    COLATTR_DESC_LENGTH,
    COLATTR_DESC_LITERAL_PREFIX,
    COLATTR_DESC_LITERAL_SUFFIX,
    COLATTR_DESC_LOCAL_TYPE_NAME,
    COLATTR_DESC_NAME,
    COLATTR_DESC_NULLABLE,
    COLATTR_DESC_NUM_PREX_RADIX,
    COLATTR_DESC_OCTET_LENGTH,
    COLATTR_DESC_PRECISION,
    COLATTR_DESC_SCALE,
    COLATTR_DESC_SCHEMA_NAME,
    COLATTR_DESC_SEARCHABLE,
    COLATTR_DESC_TABLE_NAME,
    COLATTR_DESC_TYPE,
    COLATTR_DESC_TYPE_NAME,
    COLATTR_DESC_UNNAMED,
    COLATTR_DESC_UNSIGNED,
    COLATTR_DESC_UPDATABLE
};

/*	SQLStatistics field position	*/
enum {
    STATS_CATALOG_NAME = 0,
    STATS_SCHEMA_NAME,
    STATS_TABLE_NAME,
    STATS_NON_UNIQUE,
    STATS_INDEX_QUALIFIER,
    STATS_INDEX_NAME,
    STATS_TYPE,
    STATS_SEQ_IN_INDEX,
    STATS_COLUMN_NAME,
    STATS_COLLATION,
    STATS_CARDINALITY,
    STATS_PAGES,
    STATS_FILTER_CONDITION,
    NUM_OF_STATS_FIELDS
};

/*	SQLProcedure field position	*/
enum {
    PRO_PROCEDURE_CAT = 0,
    PRO_PROCEDURE_SCHEM,
    PRO_PROCEDURE_NAME,
    PRO_NUM_INPUT_PARAMS,
    PRO_NUM_OUTPUT_PARAMS,
    PRO_RESULT_SETS,
    PRO_REMARKS,
    PRO_PROCEDURE_TYPE,
    NUM_OF_PRO_FIELDS
};

/*	SQLProcedureColumns field position	*/
enum {
    PROCOLS_PROCEDURE_CAT = 0,
    PROCOLS_PROCEDURE_SCHEM,
    PROCOLS_PROCEDURE_NAME,
    PROCOLS_COLUMN_NAME,
    PROCOLS_COLUMN_TYPE,
    PROCOLS_DATA_TYPE,
    PROCOLS_TYPE_NAME,
    PROCOLS_COLUMN_SIZE,
    PROCOLS_BUFFER_LENGTH,
    PROCOLS_DECIMAL_DIGITS,
    PROCOLS_NUM_PREC_RADIX,
    PROCOLS_NULLABLE,
    PROCOLS_REMARKS,
    PROCOLS_COLUMN_DEF,
    PROCOLS_SQL_DATA_TYPE,
    PROCOLS_SQL_DATETIME_SUB,
    PROCOLS_CHAR_OCTET_LENGTH,
    PROCOLS_ORDINAL_POSITION,
    PROCOLS_IS_NULLABLE,
    NUM_OF_PROCOLS_FIELDS
};
/*	SQLGetTypeInfo field position	*/
enum {
    GETTYPE_TYPE_NAME = 0,
    GETTYPE_DATA_TYPE,
    GETTYPE_COLUMN_SIZE,
    GETTYPE_LITERAL_PREFIX,
    GETTYPE_LITERAL_SUFFIX,
    GETTYPE_CREATE_PARAMS,
    GETTYPE_NULLABLE,
    GETTYPE_CASE_SENSITIVE,
    GETTYPE_SEARCHABLE,
    GETTYPE_UNSIGNED_ATTRIBUTE,
    GETTYPE_FIXED_PREC_SCALE,
    GETTYPE_AUTO_UNIQUE_VALUE,
    GETTYPE_LOCAL_TYPE_NAME,
    GETTYPE_MINIMUM_SCALE,
    GETTYPE_MAXIMUM_SCALE,
    GETTYPE_SQL_DATA_TYPE,
    GETTYPE_SQL_DATETIME_SUB,
    GETTYPE_NUM_PREC_RADIX,
    GETTYPE_INTERVAL_PRECISION,
    NUM_OF_GETTYPE_FIELDS
};
/*	SQLSpecialColumns field position	*/
enum {
    SPECOLS_SCOPE = 0,
    SPECOLS_COLUMN_NAME,
    SPECOLS_DATA_TYPE,
    SPECOLS_TYPE_NAME,
    SPECOLS_COLUMN_SIZE,
    SPECOLS_BUFFER_LENGTH,
    SPECOLS_DECIMAL_DIGITS,
    SPECOLS_PSEUDO_COLUMN,
    NUM_OF_SPECOLS_FIELDS
};
/*	SQLColumnPrivileges field position	*/
enum {
    COLPRIV_TABLE_CAT = 0,
    COLPRIV_TABLE_SCHEM,
    COLPRIV_TABLE_NAME,
    COLPRIV_COLUMN_NAME,
    COLPRIV_GRANTOR,
    COLPRIV_GRANTEE,
    COLPRIV_PRIVILEGE,
    COLPRIV_IS_GRANTABLE,
    NUM_OF_COLPRIV_FIELDS
};
/*	SQLTablePrivileges field position	*/
enum {
    TABPRIV_TABLE_CAT = 0,
    TABPRIV_TABLE_SCHEM,
    TABPRIV_TABLE_NAME,
    TABPRIV_GRANTOR,
    TABPRIV_GRANTEE,
    TABPRIV_PRIVILEGE,
    TABPRIV_IS_GRANTABLE,
    NUM_OF_TABPRIV_FIELDS
};
#endif /* __CARFUNC_H__ */
