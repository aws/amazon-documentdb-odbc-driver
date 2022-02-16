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

namespace ignite
{
    namespace odbc
    {
        namespace meta
        {
            void TableMeta::Read(ignite::impl::binary::BinaryReaderImpl & reader)
            {
                utility::ReadString(reader, catalogName);
                utility::ReadString(reader, schemaName);
                utility::ReadString(reader, tableName);
                utility::ReadString(reader, tableType);
            }

            void ReadTableMetaVector(ignite::impl::binary::BinaryReaderImpl& reader, TableMetaVector& meta)
            {
                int32_t metaNum = reader.ReadInt32();

                meta.clear();
                meta.reserve(static_cast<size_t>(metaNum));

                for (int32_t i = 0; i < metaNum; ++i)
                {
                    meta.push_back(TableMeta());

                    meta.back().Read(reader);
                }
            }

            void TableMeta::Read(SharedPointer< ResultSet >& resultSet,
                                 JniErrorInfo& errInfo) {
                bool wasNull;
                catalogName = "";
                schemaName = "";
                tableName = "";
                tableType = "";
                resultSet.Get()->GetString("TABLE_CAT", catalogName, wasNull,
                                           errInfo);
                resultSet.Get()->GetString("TABLE_SCHEM", schemaName, wasNull,
                                           errInfo);
                resultSet.Get()->GetString("TABLE_NAME", tableName, wasNull,
                                           errInfo);
                resultSet.Get()->GetString("TABLE_TYPE", tableType, wasNull,
                                           errInfo);
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
                    if (!hasNext
                        || errCode != JniErrorCode::IGNITE_JNI_ERR_SUCCESS) {
                        break;
                    }

                    meta.emplace_back(TableMeta());
                    meta.back().Read(resultSet, errInfo);
                } while (hasNext);
            }
        }
    }
}