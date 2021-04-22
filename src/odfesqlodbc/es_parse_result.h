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

#ifndef _ES_PARSE_RESULT_H_
#define _ES_PARSE_RESULT_H_
#include "qresult.h"

#ifdef __cplusplus
std::string GetResultParserError();
extern "C" {
#endif
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include "es_helper.h"
#include <aws/timestream-query/model/Type.h>
#include <aws/timestream-query/model/ScalarType.h>
#include <aws/timestream-query/TimestreamQueryClient.h>
// const char* is used instead of string for the cursor, because a NULL cursor
// is sometimes used Cannot pass q_res as reference because it breaks qresult.h
// macros that expect to use -> operator
BOOL CC_from_TSResult(QResultClass *q_res, ConnectionClass *conn,
                      const char *next_token,
                      const Aws::TimestreamQuery::Model::QueryOutcome &ts_result);
BOOL CC_Metadata_from_TSResult(QResultClass *q_res, ConnectionClass *conn, const char *next_token,
    const Aws::TimestreamQuery::Model::QueryOutcome &ts_result);
BOOL CC_No_Metadata_from_TSResult(QResultClass *q_res, ConnectionClass *conn, const char *next_token,
    const Aws::TimestreamQuery::Model::QueryOutcome &ts_result);
BOOL CC_Append_Table_Data(const Aws::TimestreamQuery::Model::QueryOutcome &ts_result, QResultClass *q_res, ColumnInfoClass &fields);
#endif
#endif
