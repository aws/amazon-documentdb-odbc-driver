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

#include "ignite/odbc/meta/foreign_key_meta.h"

#include "ignite/odbc/impl/binary/binary_common.h"
#include "ignite/odbc/common/utils.h"
#include "ignite/odbc/common_types.h"
#include "ignite/odbc/jni/java.h"
#include "ignite/odbc/log.h"
#include "ignite/odbc/system/odbc_constants.h"
#include "ignite/odbc/type_traits.h"

namespace ignite {
namespace odbc {
namespace meta {
const std::string PKTABLE_CAT = "PKTABLE_CAT";
const std::string PKTABLE_SCHEM = "PKTABLE_SCHEM";
const std::string PKTABLE_NAME = "PKTABLE_NAME";
const std::string PKCOLUMN_NAME = "PKCOLUMN_NAME";
const std::string FKTABLE_CAT = "FKTABLE_CAT";
const std::string FKTABLE_SCHEM = "FKTABLE_SCHEM";
const std::string FKTABLE_NAME = "FKTABLE_NAME";
const std::string FKCOLUMN_NAME = "FKCOLUMN_NAME";
const std::string KEY_SEQ = "KEY_SEQ";
const std::string UPDATE_RULE = "UPDATE_RULE";
const std::string DELETE_RULE = "DELETE_RULE";
const std::string FK_NAME = "FK_NAME";
const std::string PK_NAME = "PK_NAME";
const std::string DEFERRABILITY = "DEFERRABILITY";

void ForeignKeyMeta::Read(SharedPointer< ResultSet >& resultSet, JniErrorInfo& errInfo) {
  boost::optional< int > intDataType;
  resultSet.Get()->GetString(PKTABLE_CAT, PKCatalogName, errInfo);
  resultSet.Get()->GetString(PKTABLE_SCHEM, PKSchemaName, errInfo);
  resultSet.Get()->GetString(PKTABLE_NAME, PKTableName, errInfo);
  resultSet.Get()->GetString(PKCOLUMN_NAME, PKColumnName, errInfo);
  resultSet.Get()->GetString(FKTABLE_CAT, FKCatalogName, errInfo);
  resultSet.Get()->GetString(FKTABLE_SCHEM, FKSchemaName, errInfo);
  resultSet.Get()->GetString(FKTABLE_NAME, FKTableName, errInfo);
  resultSet.Get()->GetString(FKCOLUMN_NAME, FKColumnName, errInfo);
  resultSet.Get()->GetSmallInt(KEY_SEQ, keySeq, errInfo);
  resultSet.Get()->GetSmallInt(UPDATE_RULE, updateRule, errInfo);
  resultSet.Get()->GetSmallInt(DELETE_RULE, deleteRule, errInfo);
  resultSet.Get()->GetString(FK_NAME, FKName, errInfo);
  resultSet.Get()->GetString(PK_NAME, PKName, errInfo);
  resultSet.Get()->GetSmallInt(DEFERRABILITY, deferrability, errInfo);
}

void ReadForeignKeysColumnMetaVector(SharedPointer< ResultSet >& resultSet,
                          ForeignKeyMetaVector& meta) {
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

    meta.emplace_back(ForeignKeyMeta());
    meta.back().Read(resultSet, errInfo);
  } while (hasNext);
}

}  // namespace meta
}  // namespace odbc
}  // namespace ignite
