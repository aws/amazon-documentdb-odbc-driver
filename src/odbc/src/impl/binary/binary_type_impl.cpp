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

#include <documentdb/odbc/binary/binary_reader.h>
#include <documentdb/odbc/binary/binary_writer.h>
#include <documentdb/odbc/impl/binary/binary_type_impl.h>

#include <cstring>

namespace documentdb {
namespace odbc {
namespace binary {
int32_t BinaryType< DocumentDbError >::GetTypeId() {
  return GetBinaryStringHashCode("DocumentDbError");
}

int32_t BinaryType< DocumentDbError >::GetFieldId(const char* name) {
  return GetBinaryStringHashCode(name);
}

void BinaryType< DocumentDbError >::GetNull(DocumentDbError& dst) {
  dst = DocumentDbError(0, 0);
}

void BinaryType< DocumentDbError >::Write(BinaryWriter& writer,
                                      const DocumentDbError& obj) {
  BinaryRawWriter raw = writer.RawWriter();

  raw.WriteInt32(obj.GetCode());
  raw.WriteString(obj.GetText(), static_cast< int32_t >(strlen(obj.GetText())));
}

void BinaryType< DocumentDbError >::Read(BinaryReader& reader, DocumentDbError& dst) {
  BinaryRawReader raw = reader.RawReader();

  int32_t code = raw.ReadInt32();
  std::string msg = raw.ReadObject< std::string >();

  dst = DocumentDbError(code, msg.c_str());
}
}  // namespace binary
}  // namespace odbc
}  // namespace documentdb
