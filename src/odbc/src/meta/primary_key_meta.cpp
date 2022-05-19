/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ignite/odbc/meta/primary_key_meta.h"

#include "ignite/odbc/impl/binary/binary_common.h"
#include "ignite/odbc/common/utils.h"
#include "ignite/odbc/common_types.h"
#include "ignite/odbc/jni/java.h"
#include "ignite/odbc/system/odbc_constants.h"
#include "ignite/odbc/type_traits.h"

namespace ignite {
namespace odbc {
namespace meta {

const std::string TABLE_CAT = "TABLE_CAT";
const std::string TABLE_SCHEM = "TABLE_SCHEM";
const std::string TABLE_NAME = "TABLE_NAME";
const std::string COLUMN_NAME = "COLUMN_NAME";
const std::string KEY_SEQ = "KEY_SEQ";
const std::string PK_NAME = "PK_NAME";


void PrimaryKeyMeta::Read(SharedPointer< ResultSet >& resultSet,
                          JniErrorInfo& errInfo) {
  resultSet.Get()->GetString(TABLE_CAT, catalog, errInfo);
  resultSet.Get()->GetString(TABLE_SCHEM, schema, errInfo);
  resultSet.Get()->GetString(TABLE_NAME, table, errInfo);
  resultSet.Get()->GetString(COLUMN_NAME, column, errInfo);
  resultSet.Get()->GetSmallInt(KEY_SEQ, keySeq, errInfo);
  resultSet.Get()->GetString(PK_NAME, keyName, errInfo);
}

void ReadPrimaryKeysColumnMetaVector(SharedPointer< ResultSet >& resultSet,
                                     PrimaryKeyMetaVector& meta) {
  meta.clear();

  if (!resultSet.IsValid()) {
    return;
  }

  JniErrorInfo errInfo;
  bool hasNext = false;
  int32_t prevPosition = 0;
  JniErrorCode errCode;
  do {
    errCode = resultSet.Get()->Next(hasNext, errInfo);
    if (!hasNext || errCode != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      break;
    }

    meta.emplace_back(PrimaryKeyMeta());
    meta.back().Read(resultSet, errInfo);
  } while (hasNext);
}

}  // namespace meta
}  // namespace odbc
}  // namespace ignite