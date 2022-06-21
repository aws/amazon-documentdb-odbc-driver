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

#include <ignite/odbc/common/utils.h>
#include <ignite/odbc/impl/binary/binary_writer_impl.h>
#include <ignite/odbc/utility.h>

#include <boost/test/unit_test.hpp>
#include <chrono>
#include <stdio.h>

using namespace ignite::odbc;
using namespace ignite::odbc::utility;
using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE(UtilityTestSuite)

BOOST_AUTO_TEST_CASE(TestUtilityRemoveSurroundingSpaces) {
  std::string inStr("   \r \n    \t  some meaningfull data   \n\n   \t  \r  ");
  std::string expectedOutStr("some meaningfull data");

  std::string realOutStr(
      common::StripSurroundingWhitespaces(inStr.begin(), inStr.end()));

  BOOST_REQUIRE(expectedOutStr == realOutStr);
}

BOOST_AUTO_TEST_CASE(TestUtilityCopyStringToBuffer) {
  SQLWCHAR buffer[1024];
  std::wstring wstr(L"你好 - Some data. And some more data here.");
  std::string str = ToUtf8(wstr);
  size_t bytesWrittenOrRequired = 0;

  // With length in character mode
  buffer[0] = 0;
  bytesWrittenOrRequired =
      CopyStringToBuffer(str, buffer, sizeof(buffer) / sizeof(SQLWCHAR));
  BOOST_REQUIRE_EQUAL(SqlWcharToString(buffer), str);
  BOOST_CHECK_EQUAL(wstr.size(), bytesWrittenOrRequired);

  // With length in byte mode
  buffer[0] = 0;
  bytesWrittenOrRequired =
      CopyStringToBuffer(str, buffer, sizeof(buffer), true);
  BOOST_REQUIRE_EQUAL(SqlWcharToString(buffer), str);
  BOOST_CHECK_EQUAL(wstr.size() * sizeof(SQLWCHAR), bytesWrittenOrRequired);

  // 10 characters plus 1 for null char.
  buffer[0] = 0;
  bytesWrittenOrRequired = CopyStringToBuffer(str, buffer, 11, false);
  BOOST_REQUIRE_EQUAL(SqlWcharToString(buffer), ToUtf8(wstr.substr(0, 10)));
  BOOST_CHECK_EQUAL(10, bytesWrittenOrRequired);

  // 10 characters plus 1 for null char, in bytes
  buffer[0] = 0;
  bytesWrittenOrRequired =
      CopyStringToBuffer(str, buffer, ((10 + 1) * sizeof(SQLWCHAR)), true);
  BOOST_REQUIRE_EQUAL(SqlWcharToString(buffer), ToUtf8(wstr.substr(0, 10)));
  BOOST_CHECK_EQUAL(10 * sizeof(SQLWCHAR), bytesWrittenOrRequired);

  // Zero length buffer in character mode.
  buffer[0] = 0;
  bytesWrittenOrRequired = CopyStringToBuffer(str, buffer, 0);
  BOOST_REQUIRE_EQUAL(SqlWcharToString(buffer), std::string());
  BOOST_CHECK_EQUAL(0, bytesWrittenOrRequired);

  // Zero length buffer in byte mode.
  buffer[0] = 0;
  bytesWrittenOrRequired = CopyStringToBuffer(str, buffer, 0, true);
  BOOST_REQUIRE_EQUAL(SqlWcharToString(buffer), std::string());
  BOOST_CHECK_EQUAL(0, bytesWrittenOrRequired);

  // nullptr buffer, zero length, in character mode.
  buffer[0] = 0;
  bytesWrittenOrRequired = CopyStringToBuffer(str, nullptr, 0);
  BOOST_CHECK_EQUAL(wstr.size(), bytesWrittenOrRequired);

  // nullptr buffer, zero length, in byte mode.
  buffer[0] = 0;
  bytesWrittenOrRequired = CopyStringToBuffer(str, nullptr, 0, true);
  BOOST_CHECK_EQUAL(wstr.size() * sizeof(SQLWCHAR), bytesWrittenOrRequired);

  // nullptr buffer, non-zero length, in character mode.
  buffer[0] = 0;
  bytesWrittenOrRequired =
      CopyStringToBuffer(str, nullptr, sizeof(buffer) / sizeof(SQLWCHAR));
  BOOST_CHECK_EQUAL(wstr.size(), bytesWrittenOrRequired);

  // nullptr buffer, non-zero length, in byte mode.
  buffer[0] = 0;
  bytesWrittenOrRequired =
      CopyStringToBuffer(str, nullptr, sizeof(buffer), true);
  BOOST_CHECK_EQUAL(wstr.size() * sizeof(SQLWCHAR), bytesWrittenOrRequired);
}

// Enable test to determine efficiency of conversion function.
BOOST_AUTO_TEST_CASE(TestUtilityCopyStringToBufferRepetative, *disabled()) {
  char cch;
  int strLen = 1024 * 1024;
  std::string str;
  std::vector< SQLWCHAR > buffer(strLen + 1);

  for (int i = 0; i < strLen; i++) {
    cch = 'a' + rand() % 26;
    str.push_back(cch);
  }

  auto t1 = std::chrono::high_resolution_clock::now();
  size_t bytesWrittenOrRequired = 0;
  for (int i = 0; i < 500; i++) {
    bytesWrittenOrRequired = CopyStringToBuffer(
        str, buffer.data(), ((strLen + 1) * sizeof(SQLWCHAR)));
    BOOST_CHECK_EQUAL(str.size(), bytesWrittenOrRequired);
  }
  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout << t2.time_since_epoch().count() - t1.time_since_epoch().count()
            << " nanoseconds\n";
}

BOOST_AUTO_TEST_CASE(TestUtilitySqlStringToString) {
  std::string utf8String = u8"你好 - Some data. And some more data here.";
  std::vector< SQLWCHAR > buffer = ToWCHARVector(utf8String);
  std::string utf8StringShortened = u8"你好 - Some da";

  std::string result = SqlWcharToString(buffer.data());
  BOOST_CHECK_EQUAL(utf8String, result);

  result = SqlWcharToString(buffer.data(), buffer.size());
  BOOST_CHECK_EQUAL(utf8String, result);

  result = SqlWcharToString(buffer.data(), buffer.size(), false);
  BOOST_CHECK_EQUAL(utf8String, result);

  result =
      SqlWcharToString(buffer.data(), buffer.size() * sizeof(SQLWCHAR), true);
  BOOST_CHECK_EQUAL(utf8String, result);

  result = SqlWcharToString(nullptr, buffer.size());
  BOOST_CHECK_EQUAL(std::string(), result);

  result = SqlWcharToString(nullptr, buffer.size() * sizeof(SQLWCHAR), true);
  BOOST_CHECK_EQUAL(std::string(), result);

  result = SqlWcharToString(buffer.data(), 0);
  BOOST_CHECK_EQUAL(std::string(), result);

  result = SqlWcharToString(buffer.data(), 0, true);
  BOOST_CHECK_EQUAL(std::string(), result);

  result = SqlWcharToString(buffer.data(), 12);
  BOOST_CHECK_EQUAL(utf8StringShortened, result);

  result = SqlWcharToString(buffer.data(), 12 * sizeof(SQLWCHAR), true);
  BOOST_CHECK_EQUAL(utf8StringShortened, result);
}

BOOST_AUTO_TEST_CASE(TestUtilityWriteReadString) {
  using namespace impl::binary;
  using namespace impl::interop;

  std::string inStr1("Hello World!");
  std::string inStr2;
  std::string inStr3("Lorem ipsum");

  std::string outStr1;
  std::string outStr2;
  std::string outStr3;
  std::string outStr4;

  impl::interop::InteropUnpooledMemory mem(1024);
  InteropOutputStream outStream(&mem);
  BinaryWriterImpl writer(&outStream, 0);

  WriteString(writer, inStr1);
  WriteString(writer, inStr2);
  WriteString(writer, inStr3);
  writer.WriteNull();

  outStream.Synchronize();

  InteropInputStream inStream(&mem);
  BinaryReaderImpl reader(&inStream);

  ReadString(reader, outStr1);
  ReadString(reader, outStr2);
  ReadString(reader, outStr3);
  ReadString(reader, outStr4);

  BOOST_REQUIRE(inStr1 == outStr1);
  BOOST_REQUIRE(inStr2 == outStr2);
  BOOST_REQUIRE(inStr3 == outStr3);
  BOOST_REQUIRE(outStr4.empty());
}

void CheckDecimalWriteRead(const std::string& val) {
  using namespace impl::binary;
  using namespace impl::interop;
  using namespace common;
  using namespace utility;

  InteropUnpooledMemory mem(1024);
  InteropOutputStream outStream(&mem);
  BinaryWriterImpl writer(&outStream, 0);

  Decimal decimal(val);

  WriteDecimal(writer, decimal);

  outStream.Synchronize();

  InteropInputStream inStream(&mem);
  BinaryReaderImpl reader(&inStream);

  Decimal out;
  ReadDecimal(reader, out);

  std::stringstream converter;
  converter << out;

  std::string res = converter.str();

  BOOST_CHECK_EQUAL(res, val);
}

/**
 * Check that Decimal writing and reading works as expected.
 *
 * 1. Create Decimal value.
 * 2. Write using standard serialization algorithm.
 * 3. Read using standard de-serialization algorithm.
 * 4. Check that initial and read value are equal.
 *
 * Repeat with the following values: 0, 1, -1, 0.1, -0.1, 42, -42, 160, -160,
 * 34729864879625196, -34729864879625196, 3472986487.9625196,
 * -3472986487.9625196, 3472.9864879625196, -3472.9864879625196,
 * 0.34729864879625196, -0.34729864879625196
 */
BOOST_AUTO_TEST_CASE(TestUtilityWriteReadDecimal) {
  CheckDecimalWriteRead("0");
  CheckDecimalWriteRead("1");
  CheckDecimalWriteRead("-1");
  CheckDecimalWriteRead("0.1");
  CheckDecimalWriteRead("-0.1");
  CheckDecimalWriteRead("42");
  CheckDecimalWriteRead("-42");
  CheckDecimalWriteRead("160");
  CheckDecimalWriteRead("-160");
  CheckDecimalWriteRead("34729864879625196");
  CheckDecimalWriteRead("-34729864879625196");
  CheckDecimalWriteRead("3472986487.9625196");
  CheckDecimalWriteRead("-3472986487.9625196");
  CheckDecimalWriteRead("3472.9864879625196");
  CheckDecimalWriteRead("-3472.9864879625196");
  CheckDecimalWriteRead("0.34729864879625196");
  CheckDecimalWriteRead("-0.34729864879625196");
}

BOOST_AUTO_TEST_SUITE_END()
