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

#include <documentdb/odbc/binary/binary_raw_reader.h>
#include <documentdb/odbc/binary/binary_raw_writer.h>
#include <documentdb/odbc/impl/binary/binary_field_meta.h>

namespace documentdb {
namespace odbc {
namespace impl {
namespace binary {
DOCUMENTDB_IMPORT_EXPORT void BinaryFieldMeta::Write(
    BinaryRawWriter& writer) const {
  writer.WriteInt32(typeId);
  writer.WriteInt32(fieldId);
}

DOCUMENTDB_IMPORT_EXPORT void BinaryFieldMeta::Read(BinaryRawReader& reader) {
  typeId = reader.ReadInt32();
  fieldId = reader.ReadInt32();
}
}  // namespace binary
}  // namespace impl
}  // namespace odbc
}  // namespace documentdb
