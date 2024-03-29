﻿/*
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

#ifndef _DOCUMENTDB_ODBC_TEST_COMPLEX_TYPE
#define _DOCUMENTDB_ODBC_TEST_COMPLEX_TYPE

#include <string>

#include "documentdb/odbc/ignite.h"

namespace documentdb {
struct TestObject {
  TestObject() : f1(412), f2("Lorem ipsum") {
    // No-op.
  }

  friend bool operator==(TestObject const& lhs, TestObject const& rhs) {
    return lhs.f1 == rhs.f1 && lhs.f2 == rhs.f2;
  }

  friend std::ostream& operator<<(std::ostream& str, TestObject const& obj) {
    str << "TestObject::f1: " << obj.f1 << "TestObject::f2: " << obj.f2;
    return str;
  }

  int32_t f1;
  std::string f2;
};

struct ComplexType {
  ComplexType() : i32Field(0) {
    // No-op.
  }

  friend bool operator==(ComplexType const& lhs, ComplexType const& rhs) {
    return lhs.i32Field == rhs.i32Field && lhs.objField == rhs.objField
           && lhs.strField == rhs.strField;
  }

  friend std::ostream& operator<<(std::ostream& str, ComplexType const& obj) {
    str << "ComplexType::i32Field: " << obj.i32Field
        << "ComplexType::objField: " << obj.objField
        << "ComplexType::strField: " << obj.strField;
    return str;
  }

  int32_t i32Field;
  TestObject objField;
  std::string strField;
};
}  // namespace documentdb

namespace documentdb {
namespace odbc {
namespace binary {

DOCUMENTDB_BINARY_TYPE_START(documentdb::TestObject)

typedef documentdb::TestObject TestObject;

DOCUMENTDB_BINARY_GET_TYPE_ID_AS_HASH(TestObject)
DOCUMENTDB_BINARY_GET_TYPE_NAME_AS_IS(TestObject)
DOCUMENTDB_BINARY_GET_FIELD_ID_AS_HASH
DOCUMENTDB_BINARY_IS_NULL_FALSE(TestObject)
DOCUMENTDB_BINARY_GET_NULL_DEFAULT_CTOR(TestObject)

static void Write(BinaryWriter& writer, const TestObject& obj) {
  writer.WriteInt32("f1", obj.f1);
  writer.WriteString("f2", obj.f2);
}

static void Read(BinaryReader& reader, TestObject& dst) {
  dst.f1 = reader.ReadInt32("f1");
  dst.f2 = reader.ReadString("f2");
}

DOCUMENTDB_BINARY_TYPE_END

DOCUMENTDB_BINARY_TYPE_START(documentdb::ComplexType)

typedef documentdb::ComplexType ComplexType;

DOCUMENTDB_BINARY_GET_TYPE_ID_AS_HASH(ComplexType)
DOCUMENTDB_BINARY_GET_TYPE_NAME_AS_IS(ComplexType)
DOCUMENTDB_BINARY_GET_FIELD_ID_AS_HASH
DOCUMENTDB_BINARY_IS_NULL_FALSE(ComplexType)
DOCUMENTDB_BINARY_GET_NULL_DEFAULT_CTOR(ComplexType)

static void Write(BinaryWriter& writer, const ComplexType& obj) {
  writer.WriteInt32("i32Field", obj.i32Field);
  writer.WriteObject("objField", obj.objField);
  writer.WriteString("strField", obj.strField);
}

static void Read(BinaryReader& reader, ComplexType& dst) {
  dst.i32Field = reader.ReadInt32("i32Field");
  dst.objField = reader.ReadObject< TestObject >("objField");
  dst.strField = reader.ReadString("strField");
}

DOCUMENTDB_BINARY_TYPE_END
}  // namespace binary
}  // namespace odbc
}  // namespace documentdb

#endif  // _DOCUMENTDB_ODBC_TEST_COMPLEX_TYPE
