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

#include "ignite/odbc/meta/table_meta.h"

namespace ignite {
namespace odbc {
namespace meta {
const std::string TABLE_CAT = "TABLE_CAT";
const std::string TABLE_SCHEM = "TABLE_SCHEM";
const std::string TABLE_NAME = "TABLE_NAME";
const std::string TABLE_TYPE = "TABLE_TYPE";
const std::string REMARKS = "REMARKS";

void TableMeta::Read(SharedPointer< ResultSet >& resultSet,
                     JniErrorInfo& errInfo) {
  bool wasNull;
  resultSet.Get()->GetString(TABLE_CAT, catalogName, wasNull, errInfo);
  resultSet.Get()->GetString(TABLE_SCHEM, schemaName, wasNull, errInfo);
  resultSet.Get()->GetString(TABLE_NAME, tableName, wasNull, errInfo);
  resultSet.Get()->GetString(TABLE_TYPE, tableType, wasNull, errInfo);
  resultSet.Get()->GetString(REMARKS, remarks, wasNull, errInfo);
}

void ReadTableMetaVector(SharedPointer< ResultSet >& resultSet,
                         TableMetaVector& meta) {
  meta.clear();
  if (!resultSet.IsValid()) {
    return;
  }

  JniErrorInfo errInfo;
  bool hasNext = false;
  JniErrorCode errCode;
  do {
    errCode = resultSet.Get()->Next(hasNext, errInfo);
    if (!hasNext || errCode != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
      break;
    }

    meta.emplace_back(TableMeta());
    meta.back().Read(resultSet, errInfo);
  } while (hasNext);
}
}  // namespace meta
}  // namespace odbc
}  // namespace ignite
