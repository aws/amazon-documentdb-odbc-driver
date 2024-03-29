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

#include <documentdb/odbc/common/decimal.h>
#include <documentdb/odbc/guid.h>
#include <documentdb/odbc/app/application_data_buffer.h>
#include <documentdb/odbc/system/odbc_constants.h>
#include <documentdb/odbc/utility.h>

#include <boost/test/unit_test.hpp>

#define FLOAT_PRECISION 0.0000001f

using namespace boost::unit_test;
using namespace documentdb;
using namespace documentdb::odbc;
using namespace documentdb::odbc::app;
using namespace documentdb::odbc::type_traits;

BOOST_AUTO_TEST_SUITE(ApplicationDataBufferTestSuite)

BOOST_AUTO_TEST_CASE(TestPutIntToString) {
  char buffer[1024];
  SqlLen reslen = 0;

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_CHAR, buffer, sizeof(buffer),
                               &reslen);

  appBuf.PutInt8(12);
  BOOST_CHECK(!strcmp(buffer, "12"));
  BOOST_CHECK(reslen == strlen("12"));

  appBuf.PutInt8(-12);
  BOOST_CHECK(!strcmp(buffer, "-12"));
  BOOST_CHECK(reslen == strlen("-12"));

  appBuf.PutInt16(9876);
  BOOST_CHECK(!strcmp(buffer, "9876"));
  BOOST_CHECK(reslen == strlen("9876"));

  appBuf.PutInt16(-9876);
  BOOST_CHECK(!strcmp(buffer, "-9876"));
  BOOST_CHECK(reslen == strlen("-9876"));

  appBuf.PutInt32(1234567);
  BOOST_CHECK(!strcmp(buffer, "1234567"));
  BOOST_CHECK(reslen == strlen("1234567"));

  appBuf.PutInt32(-1234567);
  BOOST_CHECK(!strcmp(buffer, "-1234567"));
  BOOST_CHECK(reslen == strlen("-1234567"));

  std::string intMaxStr = std::to_string(INT64_MAX);
  appBuf.PutInt64(INT64_MAX);
  BOOST_CHECK(!strcmp(buffer, intMaxStr.c_str()));
  BOOST_CHECK(reslen == intMaxStr.size());

  std::string intMinStr = std::to_string(INT64_MIN);
  appBuf.PutInt64(INT64_MIN);
  BOOST_CHECK(!strcmp(buffer, intMinStr.c_str()));
  BOOST_CHECK(reslen == intMinStr.size());
}

BOOST_AUTO_TEST_CASE(TestPutIntToWString) {
  SQLWCHAR buffer[1024];
  SqlLen reslen = 0;

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_WCHAR, buffer, sizeof(buffer),
                               &reslen);

  appBuf.PutInt8(12);
  BOOST_CHECK(utility::SqlWcharToString(buffer) == "12");
  BOOST_CHECK((reslen / sizeof(SQLWCHAR)) == strlen("12"));

  appBuf.PutInt8(-12);
  BOOST_CHECK(utility::SqlWcharToString(buffer) == "-12");
  BOOST_CHECK((reslen / sizeof(SQLWCHAR)) == strlen("-12"));

  appBuf.PutInt16(9876);
  BOOST_CHECK(utility::SqlWcharToString(buffer) == "9876");
  BOOST_CHECK((reslen / sizeof(SQLWCHAR)) == strlen("9876"));

  appBuf.PutInt16(-9876);
  BOOST_CHECK(utility::SqlWcharToString(buffer) == "-9876");
  BOOST_CHECK((reslen / sizeof(SQLWCHAR)) == strlen("-9876"));

  appBuf.PutInt32(1234567);
  BOOST_CHECK(utility::SqlWcharToString(buffer) == "1234567");
  BOOST_CHECK((reslen / sizeof(SQLWCHAR)) == strlen("1234567"));

  appBuf.PutInt32(-1234567);
  BOOST_CHECK(utility::SqlWcharToString(buffer) == "-1234567");
  BOOST_CHECK((reslen / sizeof(SQLWCHAR)) == strlen("-1234567"));

  std::string intMaxStr = std::to_string(INT64_MAX);
  appBuf.PutInt64(INT64_MAX);
  BOOST_CHECK(utility::SqlWcharToString(buffer) == intMaxStr);
  BOOST_CHECK((reslen / sizeof(SQLWCHAR)) == intMaxStr.size());

  std::string intMinStr = std::to_string(INT64_MIN);
  appBuf.PutInt64(INT64_MIN);
  BOOST_CHECK(utility::SqlWcharToString(buffer) == intMinStr);
  BOOST_CHECK((reslen / sizeof(SQLWCHAR)) == intMinStr.size());
}

BOOST_AUTO_TEST_CASE(TestPutFloatToString) {
  char buffer[1024];
  SqlLen reslen = 0;

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_CHAR, buffer, sizeof(buffer),
                               &reslen);

  appBuf.PutFloat(12.42f);
  BOOST_CHECK(!strcmp(buffer, "12.42"));
  BOOST_CHECK(reslen == strlen("12.42"));

  appBuf.PutFloat(-12.42f);
  BOOST_CHECK(!strcmp(buffer, "-12.42"));
  BOOST_CHECK(reslen == strlen("-12.42"));

  appBuf.PutDouble(1000.21);
  BOOST_CHECK(!strcmp(buffer, "1000.21"));
  BOOST_CHECK(reslen == strlen("1000.21"));

  appBuf.PutDouble(-1000.21);
  BOOST_CHECK(!strcmp(buffer, "-1000.21"));
  BOOST_CHECK(reslen == strlen("-1000.21"));
}

BOOST_AUTO_TEST_CASE(TestPutFloatToWString) {
  SQLWCHAR buffer[1024];
  SqlLen reslen = 0;

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_WCHAR, buffer, sizeof(buffer),
                               &reslen);

  appBuf.PutFloat(12.42f);
  BOOST_CHECK(utility::SqlWcharToString(buffer) == "12.42");
  BOOST_CHECK((reslen / sizeof(SQLWCHAR)) == strlen("12.42"));

  appBuf.PutFloat(-12.42f);
  BOOST_CHECK(utility::SqlWcharToString(buffer) == "-12.42");
  BOOST_CHECK((reslen / sizeof(SQLWCHAR)) == strlen("-12.42"));

  appBuf.PutDouble(1000.21);
  BOOST_CHECK(utility::SqlWcharToString(buffer) == "1000.21");
  BOOST_CHECK((reslen / sizeof(SQLWCHAR)) == strlen("1000.21"));

  appBuf.PutDouble(-1000.21);
  BOOST_CHECK(utility::SqlWcharToString(buffer) == "-1000.21");
  BOOST_CHECK((reslen / sizeof(SQLWCHAR)) == strlen("-1000.21"));
}

BOOST_AUTO_TEST_CASE(TestPutGuidToString) {
  char buffer[1024];
  SqlLen reslen = 0;

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_CHAR, buffer, sizeof(buffer),
                               &reslen);

  Guid guid(0x1da1ef8f39ff4d62UL, 0x8b72e8e9f3371801UL);

  appBuf.PutGuid(guid);

  BOOST_CHECK(!strcmp(buffer, "1da1ef8f-39ff-4d62-8b72-e8e9f3371801"));
  BOOST_CHECK(reslen == strlen("1da1ef8f-39ff-4d62-8b72-e8e9f3371801"));
}

BOOST_AUTO_TEST_CASE(TestPutGuidToWString) {
  SQLWCHAR buffer[1024];
  SqlLen reslen = 0;

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_WCHAR, buffer, sizeof(buffer),
                               &reslen);

  Guid guid(0x1da1ef8f39ff4d62UL, 0x8b72e8e9f3371801UL);

  appBuf.PutGuid(guid);

  BOOST_CHECK(utility::SqlWcharToString(buffer) == "1da1ef8f-39ff-4d62-8b72-e8e9f3371801");
  BOOST_CHECK((reslen / sizeof(SQLWCHAR))
              == strlen("1da1ef8f-39ff-4d62-8b72-e8e9f3371801"));
}

BOOST_AUTO_TEST_CASE(TestPutBinaryToString) {
  char buffer[1024];
  SqlLen reslen = 0;

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_CHAR, buffer, sizeof(buffer),
                               &reslen);

  uint8_t binary[] = {0x21, 0x84, 0xF4, 0xDC, 0x01, 0x00, 0xFF, 0xF0};

  int32_t written = 0;

  appBuf.PutBinaryData(binary, sizeof(binary), written);

  BOOST_CHECK(!strcmp(buffer, "2184f4dc0100fff0"));
  BOOST_CHECK(reslen == strlen("2184f4dc0100fff0"));
}

BOOST_AUTO_TEST_CASE(TestPutBinaryToWString) {
  SQLWCHAR buffer[1024];
  SqlLen reslen = 0;

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_WCHAR, buffer, sizeof(buffer),
                               &reslen);

  uint8_t binary[] = {0x21, 0x84, 0xF4, 0xDC, 0x01, 0x00, 0xFF, 0xF0};

  int32_t written = 0;

  appBuf.PutBinaryData(binary, sizeof(binary), written);

  std::string bufferAsString = utility::SqlWcharToString(buffer);
  BOOST_CHECK(utility::SqlWcharToString(buffer) == "2184f4dc0100fff0");
  BOOST_CHECK((reslen / sizeof(SQLWCHAR)) == strlen("2184f4dc0100fff0"));
}

BOOST_AUTO_TEST_CASE(TestPutStringToString) {
  char buffer[1024];
  SqlLen reslen = 0;

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_CHAR, buffer, sizeof(buffer),
                               &reslen);

  std::string testString("Test string");

  appBuf.PutString(testString);

  BOOST_CHECK(!strcmp(buffer, testString.c_str()));
  BOOST_CHECK_EQUAL(static_cast< size_t >(reslen), testString.size());
}

BOOST_AUTO_TEST_CASE(TestPutStringToWstring) {
  SQLWCHAR buffer[1024];
  SqlLen reslen = 0;

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_WCHAR, buffer, sizeof(buffer),
                               &reslen);

  std::string testString("Test string");

  appBuf.PutString(testString);
  BOOST_CHECK(utility::SqlWcharToString(buffer) == "Test string");
}

BOOST_AUTO_TEST_CASE(TestPutStringToLong) {
  SQLINTEGER numBuf;
  SqlLen reslen = 0;

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_SIGNED_LONG, &numBuf,
                               sizeof(numBuf), &reslen);

  appBuf.PutString("424242424");
  BOOST_CHECK(numBuf == 424242424L);

  appBuf.PutString("-424242424");
  BOOST_CHECK(numBuf == -424242424L);
}

BOOST_AUTO_TEST_CASE(TestPutStringToTiny) {
  int8_t numBuf;
  SqlLen reslen = 0;

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_SIGNED_TINYINT, &numBuf,
                               sizeof(numBuf), &reslen);

  appBuf.PutString("12");
  BOOST_CHECK(numBuf == 12);

  appBuf.PutString("-12");
  BOOST_CHECK(numBuf == -12);
}

BOOST_AUTO_TEST_CASE(TestPutStringToFloat) {
  float numBuf;
  SqlLen reslen = 0;

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_FLOAT, &numBuf,
                               sizeof(numBuf), &reslen);

  appBuf.PutString("12.21");
  BOOST_CHECK_CLOSE_FRACTION(numBuf, 12.21, FLOAT_PRECISION);

  appBuf.PutString("-12.21");
  BOOST_CHECK_CLOSE_FRACTION(numBuf, -12.21, FLOAT_PRECISION);
}

BOOST_AUTO_TEST_CASE(TestPutIntToFloat) {
  float numBuf;
  SqlLen reslen = 0;

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_FLOAT, &numBuf,
                               sizeof(numBuf), &reslen);

  appBuf.PutInt8(5);
  BOOST_CHECK_CLOSE_FRACTION(numBuf, 5.0, FLOAT_PRECISION);

  appBuf.PutInt8(-5);
  BOOST_CHECK_CLOSE_FRACTION(numBuf, -5.0, FLOAT_PRECISION);

  appBuf.PutInt16(4242);
  BOOST_CHECK_CLOSE_FRACTION(numBuf, 4242.0, FLOAT_PRECISION);

  appBuf.PutInt16(-4242);
  BOOST_CHECK_CLOSE_FRACTION(numBuf, -4242.0, FLOAT_PRECISION);

  appBuf.PutInt32(1234567);
  BOOST_CHECK_CLOSE_FRACTION(numBuf, 1234567.0, FLOAT_PRECISION);

  appBuf.PutInt32(-1234567);
  BOOST_CHECK_CLOSE_FRACTION(numBuf, -1234567.0, FLOAT_PRECISION);
}

BOOST_AUTO_TEST_CASE(TestPutFloatToShort) {
  short numBuf;
  SqlLen reslen = 0;

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_SIGNED_SHORT, &numBuf,
                               sizeof(numBuf), &reslen);

  appBuf.PutDouble(5.42);
  BOOST_CHECK(numBuf == 5);

  appBuf.PutDouble(-5.42);
  BOOST_CHECK(numBuf == -5.0);

  appBuf.PutFloat(42.99f);
  BOOST_CHECK(numBuf == 42);

  appBuf.PutFloat(-42.99f);
  BOOST_CHECK(numBuf == -42);
}

BOOST_AUTO_TEST_CASE(TestPutDecimalToDouble) {
  double numBuf;
  SqlLen reslen = 0;

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_DOUBLE, &numBuf,
                               sizeof(numBuf), &reslen);

  common::Decimal decimal;

  BOOST_CHECK_CLOSE_FRACTION(static_cast< double >(decimal), 0.0,
                             FLOAT_PRECISION);

  appBuf.PutDecimal(decimal);
  BOOST_CHECK_CLOSE_FRACTION(numBuf, 0.0, FLOAT_PRECISION);

  int8_t mag1[] = {1, 0};

  decimal = common::Decimal(mag1, sizeof(mag1), 0, 1);

  appBuf.PutDecimal(decimal);
  BOOST_CHECK_CLOSE_FRACTION(numBuf, 256.0, FLOAT_PRECISION);

  int8_t mag2[] = {2, 23};

  decimal = common::Decimal(mag2, sizeof(mag2), 1, -1);

  appBuf.PutDecimal(decimal);
  BOOST_CHECK_CLOSE_FRACTION(numBuf, -53.5, FLOAT_PRECISION);
}

BOOST_AUTO_TEST_CASE(TestPutDecimalToLong) {
  SQLINTEGER numBuf;
  SqlLen reslen = 0;

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_SIGNED_LONG, &numBuf,
                               sizeof(numBuf), &reslen);

  common::Decimal decimal;

  appBuf.PutDecimal(decimal);
  BOOST_CHECK(numBuf == 0);

  int8_t mag1[] = {1, 0};

  decimal = common::Decimal(mag1, sizeof(mag1), 0, 1);

  appBuf.PutDecimal(decimal);
  BOOST_CHECK(numBuf == 256);

  int8_t mag2[] = {2, 23};

  decimal = common::Decimal(mag2, sizeof(mag2), 1, -1);

  appBuf.PutDecimal(decimal);
  BOOST_CHECK(numBuf == -53);
}

BOOST_AUTO_TEST_CASE(TestPutDecimalToString) {
  char strBuf[64];
  SqlLen reslen = 0;

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_CHAR, &strBuf, sizeof(strBuf),
                               &reslen);

  common::Decimal decimal;

  appBuf.PutDecimal(decimal);
  BOOST_CHECK(std::string(strBuf, reslen) == "0");

  int8_t mag1[] = {1, 0};

  decimal = common::Decimal(mag1, sizeof(mag1), 0, 1);

  appBuf.PutDecimal(decimal);
  BOOST_CHECK(std::string(strBuf, reslen) == "256");

  int8_t mag2[] = {2, 23};

  decimal = common::Decimal(mag2, sizeof(mag2), 1, -1);

  appBuf.PutDecimal(decimal);
  BOOST_CHECK(std::string(strBuf, reslen) == "-53.5");
}

BOOST_AUTO_TEST_CASE(TestPutDecimalToWString) {
  SQLWCHAR strBuf[64];
  SqlLen reslen = 0;

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_WCHAR, &strBuf,
                               sizeof(strBuf), &reslen);

  common::Decimal decimal;

  appBuf.PutDecimal(decimal);
  BOOST_CHECK(utility::SqlWcharToString(strBuf) == "0");

  int8_t mag1[] = {1, 0};

  decimal = common::Decimal(mag1, sizeof(mag1), 0, 1);

  appBuf.PutDecimal(decimal);
  BOOST_CHECK(utility::SqlWcharToString(strBuf) == "256");

  int8_t mag2[] = {2, 23};

  decimal = common::Decimal(mag2, sizeof(mag2), 1, -1);

  appBuf.PutDecimal(decimal);
  BOOST_CHECK(utility::SqlWcharToString(strBuf) == "-53.5");
}

BOOST_AUTO_TEST_CASE(TestPutDecimalToNumeric) {
  SQL_NUMERIC_STRUCT buf;
  SqlLen reslen = 0;

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_NUMERIC, &buf, sizeof(buf),
                               &reslen);

  common::Decimal decimal;

  appBuf.PutDecimal(decimal);
  BOOST_CHECK_EQUAL(1, buf.sign);  // Positive
  BOOST_CHECK_EQUAL(
      0, buf.scale);  // Scale is 0 by default according to specification
  BOOST_CHECK_EQUAL(
      1, buf.precision);  // Precision is 1 for default constructed Decimal (0).

  for (int i = 0; i < SQL_MAX_NUMERIC_LEN; ++i)
    BOOST_CHECK_EQUAL(0, buf.val[i]);

  // Trying to store 123.45 => 12345 => 0x3039 => [0x30, 0x39].
  uint8_t mag1[] = {0x30, 0x39};

  decimal =
      common::Decimal(reinterpret_cast< int8_t* >(mag1), sizeof(mag1), 2, 1);

  appBuf.PutDecimal(decimal);
  BOOST_CHECK_EQUAL(1, buf.sign);  // Positive
  BOOST_CHECK_EQUAL(
      0, buf.scale);  // Scale is 0 by default according to specification
  BOOST_CHECK_EQUAL(
      3, buf.precision);  // Precision is 3, as the scale is set to 0.

  // 123.45 => (scale=0) 123 => 0x7B => [0x7B].
  BOOST_CHECK_EQUAL(buf.val[0], 0x7B);

  for (int i = 1; i < SQL_MAX_NUMERIC_LEN; ++i)
    BOOST_CHECK_EQUAL(0, buf.val[i]);

  // Trying to store 12345.678 => 12345678 => 0xBC614E => [0xBC, 0x61, 0x4E].
  uint8_t mag2[] = {0xBC, 0x61, 0x4E};

  decimal =
      common::Decimal(reinterpret_cast< int8_t* >(mag2), sizeof(mag2), 3, -1);

  appBuf.PutDecimal(decimal);
  BOOST_CHECK_EQUAL(0, buf.sign);  // Negative
  BOOST_CHECK_EQUAL(
      0, buf.scale);  // Scale is 0 by default according to specification
  BOOST_CHECK_EQUAL(
      5, buf.precision);  // Precision is 5, as the scale is set to 0.

  // 12345.678 => (scale=0) 12345 => 0x3039 => [0x39, 0x30].
  BOOST_CHECK_EQUAL(buf.val[0], 0x39);
  BOOST_CHECK_EQUAL(buf.val[1], 0x30);

  for (int i = 2; i < SQL_MAX_NUMERIC_LEN; ++i)
    BOOST_CHECK_EQUAL(0, buf.val[i]);
}

BOOST_AUTO_TEST_CASE(TestPutDateToString) {
  char strBuf[64];
  SqlLen reslen = 0;

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_CHAR, &strBuf, sizeof(strBuf),
                               &reslen);

  Date date = common::MakeDateGmt(1999, 2, 22);

  appBuf.PutDate(date);

  BOOST_CHECK_EQUAL(std::string(strBuf, reslen), std::string("1999-02-22"));
}

BOOST_AUTO_TEST_CASE(TestPutDateToWString) {
  SQLWCHAR strBuf[64];
  SqlLen reslen = 0;

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_WCHAR, &strBuf, sizeof(strBuf),
                               &reslen);

  Date date = common::MakeDateGmt(1999, 2, 22);

  appBuf.PutDate(date);

  BOOST_CHECK_EQUAL(utility::SqlWcharToString(strBuf), std::string("1999-02-22"));
}

BOOST_AUTO_TEST_CASE(TestPutDateToDate) {
  SQL_DATE_STRUCT buf;
  SqlLen reslen = sizeof(buf);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_TDATE, &buf, sizeof(buf),
                               &reslen);

  Date date = common::MakeDateGmt(1984, 5, 27);

  appBuf.PutDate(date);

  BOOST_CHECK_EQUAL(1984, buf.year);
  BOOST_CHECK_EQUAL(5, buf.month);
  BOOST_CHECK_EQUAL(27, buf.day);
}

BOOST_AUTO_TEST_CASE(TestPutDateToTimestamp) {
  SQL_TIMESTAMP_STRUCT buf;

  SqlLen reslen = sizeof(buf);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_TTIMESTAMP, &buf, sizeof(buf),
                               &reslen);

  Date date = common::MakeDateGmt(1984, 5, 27);

  appBuf.PutDate(date);

  BOOST_CHECK_EQUAL(1984, buf.year);
  BOOST_CHECK_EQUAL(5, buf.month);
  BOOST_CHECK_EQUAL(27, buf.day);
  BOOST_CHECK_EQUAL(0, buf.hour);
  BOOST_CHECK_EQUAL(0, buf.minute);
  BOOST_CHECK_EQUAL(0, buf.second);
  BOOST_CHECK_EQUAL(0, buf.fraction);
}

BOOST_AUTO_TEST_CASE(TestPutTimeToString) {
  char strBuf[64];
  SqlLen reslen = 0;

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_CHAR, &strBuf, sizeof(strBuf),
                               &reslen);

  Time time = common::MakeTimeGmt(7, 15, 0);

  appBuf.PutTime(time);

  BOOST_CHECK_EQUAL(std::string(strBuf, reslen), std::string("07:15:00"));
}

BOOST_AUTO_TEST_CASE(TestPutTimeToWString) {
  SQLWCHAR strBuf[64];
  SqlLen reslen = 0;

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_WCHAR, &strBuf, sizeof(strBuf),
                               &reslen);

  Time time = common::MakeTimeGmt(7, 15, 0);

  appBuf.PutTime(time);

  BOOST_CHECK_EQUAL(utility::SqlWcharToString(strBuf), std::string("07:15:00"));
}

BOOST_AUTO_TEST_CASE(TestPutTimeToTime) {
  SQL_TIME_STRUCT buf;
  SqlLen reslen = sizeof(buf);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_TTIME, &buf, sizeof(buf),
                               &reslen);

  Time time = common::MakeTimeGmt(23, 51, 1);

  appBuf.PutTime(time);

  BOOST_CHECK_EQUAL(23, buf.hour);
  BOOST_CHECK_EQUAL(51, buf.minute);
  BOOST_CHECK_EQUAL(1, buf.second);
}

BOOST_AUTO_TEST_CASE(TestPutTimestampToString) {
  char strBuf[64];
  SqlLen reslen = 0;

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_CHAR, &strBuf, sizeof(strBuf),
                               &reslen);

  Timestamp date = common::MakeTimestampGmt(2018, 11, 1, 17, 45, 59);

  appBuf.PutTimestamp(date);

  BOOST_CHECK_EQUAL(std::string(strBuf, reslen),
                    std::string("2018-11-01 17:45:59"));
}

BOOST_AUTO_TEST_CASE(TestPutTimestampToWString) {
  SQLWCHAR strBuf[64];
  SqlLen reslen = 0;

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_WCHAR, &strBuf, sizeof(strBuf),
                               &reslen);

  Timestamp date = common::MakeTimestampGmt(2018, 11, 1, 17, 45, 59);

  appBuf.PutTimestamp(date);

  BOOST_CHECK_EQUAL(utility::SqlWcharToString(strBuf),
                    std::string("2018-11-01 17:45:59"));
}

BOOST_AUTO_TEST_CASE(TestPutTimestampToDate) {
  SQL_DATE_STRUCT buf;
  SqlLen reslen = sizeof(buf);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_TDATE, &buf, sizeof(buf),
                               &reslen);

  Timestamp ts = common::MakeTimestampGmt(2004, 8, 14, 6, 34, 51, 573948623);

  appBuf.PutTimestamp(ts);

  BOOST_CHECK_EQUAL(2004, buf.year);
  BOOST_CHECK_EQUAL(8, buf.month);
  BOOST_CHECK_EQUAL(14, buf.day);
}

BOOST_AUTO_TEST_CASE(TestPutTimestampToTime) {
  SQL_TIME_STRUCT buf;
  SqlLen reslen = sizeof(buf);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_TTIME, &buf, sizeof(buf),
                               &reslen);

  Timestamp ts = common::MakeTimestampGmt(2004, 8, 14, 6, 34, 51, 573948623);

  appBuf.PutTimestamp(ts);

  BOOST_CHECK_EQUAL(6, buf.hour);
  BOOST_CHECK_EQUAL(34, buf.minute);
  BOOST_CHECK_EQUAL(51, buf.second);
}

BOOST_AUTO_TEST_CASE(TestPutTimestampToTimestamp) {
  SQL_TIMESTAMP_STRUCT buf;
  SqlLen reslen = sizeof(buf);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_TTIMESTAMP, &buf, sizeof(buf),
                               &reslen);

  Timestamp ts = common::MakeTimestampGmt(2004, 8, 14, 6, 34, 51, 573948623);

  appBuf.PutTimestamp(ts);

  BOOST_CHECK_EQUAL(2004, buf.year);
  BOOST_CHECK_EQUAL(8, buf.month);
  BOOST_CHECK_EQUAL(14, buf.day);
  BOOST_CHECK_EQUAL(6, buf.hour);
  BOOST_CHECK_EQUAL(34, buf.minute);
  BOOST_CHECK_EQUAL(51, buf.second);
  BOOST_CHECK_EQUAL(573948623, buf.fraction);
}

BOOST_AUTO_TEST_CASE(TestGetGuidFromString) {
  char buffer[] = "1da1ef8f-39ff-4d62-8b72-e8e9f3371801";
  SqlLen reslen = sizeof(buffer) - 1;

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_CHAR, buffer,
                               sizeof(buffer) - 1, &reslen);

  Guid guid = appBuf.GetGuid();

  BOOST_CHECK_EQUAL(guid, Guid(0x1da1ef8f39ff4d62UL, 0x8b72e8e9f3371801UL));
}

BOOST_AUTO_TEST_CASE(TestGetGuidFromWString) {
  std::vector< SQLWCHAR > buffer =
      utility::ToWCHARVector("1da1ef8f-39ff-4d62-8b72-e8e9f3371801");
  SqlLen reslen = buffer.size() * sizeof(SQLWCHAR);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_WCHAR, buffer.data(),
                               buffer.size() * sizeof(SQLWCHAR), &reslen);

  Guid guid = appBuf.GetGuid();

  BOOST_CHECK_EQUAL(guid, Guid(0x1da1ef8f39ff4d62UL, 0x8b72e8e9f3371801UL));
}

BOOST_AUTO_TEST_CASE(TestGetStringFromLong) {
  long numBuf = 42;
  SqlLen reslen = sizeof(numBuf);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_SIGNED_LONG, &numBuf, reslen,
                               &reslen);

  std::string res = appBuf.GetString(32);

  BOOST_CHECK(res == "42");

  numBuf = -77;

  res = appBuf.GetString(32);

  BOOST_CHECK(res == "-77");
}

BOOST_AUTO_TEST_CASE(TestGetStringFromDouble) {
  double numBuf = 43.36;
  SqlLen reslen = sizeof(numBuf);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_DOUBLE, &numBuf, reslen,
                               &reslen);

  std::string res = appBuf.GetString(32);

  BOOST_CHECK(res == "43.36");

  numBuf = -58.91;

  res = appBuf.GetString(32);

  BOOST_CHECK(res == "-58.91");
}

BOOST_AUTO_TEST_CASE(TestGetStringFromString) {
  char buf[] = "Some data 32d2d5hs";
  SqlLen reslen = sizeof(buf);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_CHAR, &buf, reslen, &reslen);

  std::string res = appBuf.GetString(reslen);

  BOOST_CHECK(res.compare(buf));
}

BOOST_AUTO_TEST_CASE(TestGetStringFromWString) {
  std::vector< SQLWCHAR > buf = utility::ToWCHARVector("Some data 32d2d5hs");
  SqlLen reslen = buf.size() * sizeof(SQLWCHAR);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_WCHAR, buf.data(), reslen,
                               &reslen);

  std::string res = appBuf.GetString(reslen);

  BOOST_CHECK(utility::SqlWcharToString(buf.data()) == res);
}

BOOST_AUTO_TEST_CASE(TestGetFloatFromUshort) {
  unsigned short numBuf = 7162;
  SqlLen reslen = sizeof(numBuf);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_UNSIGNED_SHORT, &numBuf,
                               reslen, &reslen);

  float resFloat = appBuf.GetFloat();

  BOOST_CHECK_CLOSE_FRACTION(resFloat, 7162.0f, FLOAT_PRECISION);

  double resDouble = appBuf.GetDouble();

  BOOST_CHECK_CLOSE_FRACTION(resDouble, 7162.0, FLOAT_PRECISION);
}

BOOST_AUTO_TEST_CASE(TestGetFloatFromString) {
  char buf[] = "28.562";
  SqlLen reslen = sizeof(buf);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_CHAR, &buf, reslen, &reslen);

  float resFloat = appBuf.GetFloat();

  BOOST_CHECK_CLOSE_FRACTION(resFloat, 28.562f, FLOAT_PRECISION);

  double resDouble = appBuf.GetDouble();

  BOOST_CHECK_CLOSE_FRACTION(resDouble, 28.562, FLOAT_PRECISION);
}

BOOST_AUTO_TEST_CASE(TestGetFloatFromWString) {
  std::vector< SQLWCHAR > buf = utility::ToWCHARVector("28.562");
  SqlLen reslen = buf.size() * sizeof(SQLWCHAR);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_WCHAR, buf.data(), reslen,
                               &reslen);

  float resFloat = appBuf.GetFloat();

  BOOST_CHECK_CLOSE_FRACTION(resFloat, 28.562f, FLOAT_PRECISION);

  double resDouble = appBuf.GetDouble();

  BOOST_CHECK_CLOSE_FRACTION(resDouble, 28.562, FLOAT_PRECISION);
}

BOOST_AUTO_TEST_CASE(TestGetFloatFromFloat) {
  float buf = 207.49f;
  SqlLen reslen = sizeof(buf);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_FLOAT, &buf, reslen, &reslen);

  float resFloat = appBuf.GetFloat();

  BOOST_CHECK_CLOSE_FRACTION(resFloat, 207.49f, FLOAT_PRECISION);

  double resDouble = appBuf.GetDouble();

  BOOST_CHECK_CLOSE_FRACTION(resDouble, 207.49, FLOAT_PRECISION);
}

BOOST_AUTO_TEST_CASE(TestGetFloatFromDouble) {
  double buf = 893.162;
  SqlLen reslen = sizeof(buf);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_DOUBLE, &buf, reslen,
                               &reslen);

  float resFloat = appBuf.GetFloat();

  BOOST_CHECK_CLOSE_FRACTION(resFloat, 893.162f, FLOAT_PRECISION);

  double resDouble = appBuf.GetDouble();

  BOOST_CHECK_CLOSE_FRACTION(resDouble, 893.162, FLOAT_PRECISION);
}

BOOST_AUTO_TEST_CASE(TestGetIntFromString) {
  char buf[] = "39";
  SqlLen reslen = sizeof(buf);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_CHAR, &buf, reslen, &reslen);

  int64_t resInt64 = appBuf.GetInt64();

  BOOST_CHECK(resInt64 == 39);

  int32_t resInt32 = appBuf.GetInt32();

  BOOST_CHECK(resInt32 == 39);

  int16_t resInt16 = appBuf.GetInt16();

  BOOST_CHECK(resInt16 == 39);

  int8_t resInt8 = appBuf.GetInt8();

  BOOST_CHECK(resInt8 == 39);
}

BOOST_AUTO_TEST_CASE(TestGetIntFromWString) {
  std::vector< SQLWCHAR > buf = utility::ToWCHARVector("39");
  SqlLen reslen = buf.size() * sizeof(SQLWCHAR);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_WCHAR, buf.data(), reslen,
                               &reslen);

  int64_t resInt64 = appBuf.GetInt64();

  BOOST_CHECK(resInt64 == 39);

  int32_t resInt32 = appBuf.GetInt32();

  BOOST_CHECK(resInt32 == 39);

  int16_t resInt16 = appBuf.GetInt16();

  BOOST_CHECK(resInt16 == 39);

  int8_t resInt8 = appBuf.GetInt8();

  BOOST_CHECK(resInt8 == 39);
}

BOOST_AUTO_TEST_CASE(TestGetIntFromFloat) {
  float buf = -107.49f;
  SqlLen reslen = sizeof(buf);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_FLOAT, &buf, reslen, &reslen);

  int64_t resInt64 = appBuf.GetInt64();

  BOOST_CHECK(resInt64 == -107);

  int32_t resInt32 = appBuf.GetInt32();

  BOOST_CHECK(resInt32 == -107);

  int16_t resInt16 = appBuf.GetInt16();

  BOOST_CHECK(resInt16 == -107);

  int8_t resInt8 = appBuf.GetInt8();

  BOOST_CHECK(resInt8 == -107);
}

BOOST_AUTO_TEST_CASE(TestGetIntFromDouble) {
  double buf = 42.97f;
  SqlLen reslen = sizeof(buf);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_DOUBLE, &buf, reslen,
                               &reslen);

  int64_t resInt64 = appBuf.GetInt64();

  BOOST_CHECK(resInt64 == 42);

  int32_t resInt32 = appBuf.GetInt32();

  BOOST_CHECK(resInt32 == 42);

  int16_t resInt16 = appBuf.GetInt16();

  BOOST_CHECK(resInt16 == 42);

  int8_t resInt8 = appBuf.GetInt8();

  BOOST_CHECK(resInt8 == 42);
}

BOOST_AUTO_TEST_CASE(TestGetIntFromBigint) {
  uint64_t buf = 19;
  SqlLen reslen = sizeof(buf);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_UNSIGNED_BIGINT, &buf, reslen,
                               &reslen);

  int64_t resInt64 = appBuf.GetInt64();

  BOOST_CHECK(resInt64 == 19);

  int32_t resInt32 = appBuf.GetInt32();

  BOOST_CHECK(resInt32 == 19);

  int16_t resInt16 = appBuf.GetInt16();

  BOOST_CHECK(resInt16 == 19);

  int8_t resInt8 = appBuf.GetInt8();

  BOOST_CHECK(resInt8 == 19);
}

BOOST_AUTO_TEST_CASE(TestGetIntWithOffset) {
  struct GetIntWithOffsetTestStruct {
    uint64_t val;
    SqlLen reslen;
  };

  GetIntWithOffsetTestStruct buf[2] = {{12, sizeof(uint64_t)},
                                       {42, sizeof(uint64_t)}};

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_UNSIGNED_BIGINT, &buf[0].val,
                               sizeof(buf[0].val), &buf[0].reslen);

  int64_t val = appBuf.GetInt64();

  BOOST_CHECK(val == 12);

  appBuf.SetByteOffset(sizeof(GetIntWithOffsetTestStruct));

  val = appBuf.GetInt64();

  BOOST_CHECK(val == 42);

  appBuf.SetByteOffset(0);

  val = appBuf.GetInt64();

  BOOST_CHECK(val == 12);
}

BOOST_AUTO_TEST_CASE(TestSetStringWithOffset) {
  struct SetStringWithOffsetTestStruct {
    char val[64];
    SqlLen reslen;
  };

  SetStringWithOffsetTestStruct buf[2] = {{"", 0}, {"", 0}};

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_CHAR, &buf[0].val,
                               sizeof(buf[0].val), &buf[0].reslen);

  appBuf.PutString("Hello world!");

  std::string res(buf[0].val, buf[0].reslen);

  BOOST_CHECK(buf[0].reslen == strlen("Hello world!"));
  BOOST_CHECK(res == "Hello world!");
  BOOST_CHECK(res.size() == strlen("Hello world!"));

  appBuf.SetByteOffset(sizeof(SetStringWithOffsetTestStruct));

  appBuf.PutString("Hello with offset!");

  res.assign(buf[0].val, buf[0].reslen);

  BOOST_CHECK(res == "Hello world!");
  BOOST_CHECK(res.size() == strlen("Hello world!"));
  BOOST_CHECK(buf[0].reslen == strlen("Hello world!"));

  res.assign(buf[1].val, buf[1].reslen);

  BOOST_CHECK(res == "Hello with offset!");
  BOOST_CHECK(res.size() == strlen("Hello with offset!"));
  BOOST_CHECK(buf[1].reslen == strlen("Hello with offset!"));
}

BOOST_AUTO_TEST_CASE(TestGetDateFromString) {
  char buf[] = "1999-02-22";
  SqlLen reslen = sizeof(buf);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_CHAR, &buf[0], sizeof(buf),
                               &reslen);

  Date date = appBuf.GetDate();

  tm tmDate;

  bool success = common::DateToCTm(date, tmDate);

  BOOST_REQUIRE(success);

  BOOST_CHECK_EQUAL(1999, tmDate.tm_year + 1900);
  BOOST_CHECK_EQUAL(2, tmDate.tm_mon + 1);
  BOOST_CHECK_EQUAL(22, tmDate.tm_mday);
  BOOST_CHECK_EQUAL(0, tmDate.tm_hour);
  BOOST_CHECK_EQUAL(0, tmDate.tm_min);
  BOOST_CHECK_EQUAL(0, tmDate.tm_sec);
}

BOOST_AUTO_TEST_CASE(TestGetDateFromWString) {
  std::vector< SQLWCHAR > buf = utility::ToWCHARVector("1999-02-22");
  SqlLen reslen = buf.size() * sizeof(SQLWCHAR);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_WCHAR, buf.data(), reslen,
                               &reslen);

  Date date = appBuf.GetDate();

  tm tmDate;

  bool success = common::DateToCTm(date, tmDate);

  BOOST_REQUIRE(success);

  BOOST_CHECK_EQUAL(1999, tmDate.tm_year + 1900);
  BOOST_CHECK_EQUAL(2, tmDate.tm_mon + 1);
  BOOST_CHECK_EQUAL(22, tmDate.tm_mday);
  BOOST_CHECK_EQUAL(0, tmDate.tm_hour);
  BOOST_CHECK_EQUAL(0, tmDate.tm_min);
  BOOST_CHECK_EQUAL(0, tmDate.tm_sec);
}

BOOST_AUTO_TEST_CASE(TestGetTimeFromString) {
  char buf[] = "17:5:59";
  SqlLen reslen = sizeof(buf);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_CHAR, &buf[0], sizeof(buf),
                               &reslen);

  Time time = appBuf.GetTime();

  tm tmTime;

  bool success = common::TimeToCTm(time, tmTime);

  BOOST_REQUIRE(success);

  BOOST_CHECK_EQUAL(1970, tmTime.tm_year + 1900);
  BOOST_CHECK_EQUAL(1, tmTime.tm_mon + 1);
  BOOST_CHECK_EQUAL(1, tmTime.tm_mday);
  BOOST_CHECK_EQUAL(17, tmTime.tm_hour);
  BOOST_CHECK_EQUAL(5, tmTime.tm_min);
  BOOST_CHECK_EQUAL(59, tmTime.tm_sec);
}

BOOST_AUTO_TEST_CASE(TestGetTimeFromWString) {
  std::vector< SQLWCHAR > buf = utility::ToWCHARVector("17:5:59");
  SqlLen reslen = buf.size() * sizeof(SQLWCHAR);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_WCHAR, buf.data(), reslen,
                               &reslen);

  Time time = appBuf.GetTime();

  tm tmTime;

  bool success = common::TimeToCTm(time, tmTime);

  BOOST_REQUIRE(success);

  BOOST_CHECK_EQUAL(1970, tmTime.tm_year + 1900);
  BOOST_CHECK_EQUAL(1, tmTime.tm_mon + 1);
  BOOST_CHECK_EQUAL(1, tmTime.tm_mday);
  BOOST_CHECK_EQUAL(17, tmTime.tm_hour);
  BOOST_CHECK_EQUAL(5, tmTime.tm_min);
  BOOST_CHECK_EQUAL(59, tmTime.tm_sec);
}

BOOST_AUTO_TEST_CASE(TestGetTimestampFromString) {
  char buf[] = "2018-11-01 17:45:59";
  SqlLen reslen = sizeof(buf);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_CHAR, &buf[0], sizeof(buf),
                               &reslen);

  Timestamp date = appBuf.GetTimestamp();

  tm tmDate;

  bool success = common::TimestampToCTm(date, tmDate);

  BOOST_REQUIRE(success);

  BOOST_CHECK_EQUAL(2018, tmDate.tm_year + 1900);
  BOOST_CHECK_EQUAL(11, tmDate.tm_mon + 1);
  BOOST_CHECK_EQUAL(1, tmDate.tm_mday);
  BOOST_CHECK_EQUAL(17, tmDate.tm_hour);
  BOOST_CHECK_EQUAL(45, tmDate.tm_min);
  BOOST_CHECK_EQUAL(59, tmDate.tm_sec);
}

BOOST_AUTO_TEST_CASE(TestGetTimestampFromWString) {
  std::vector< SQLWCHAR > buf = utility::ToWCHARVector("2018-11-01 17:45:59");
  SqlLen reslen = buf.size() * sizeof(SQLWCHAR);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_WCHAR, buf.data(), reslen,
                               &reslen);

  Timestamp date = appBuf.GetTimestamp();

  tm tmDate;

  bool success = common::TimestampToCTm(date, tmDate);

  BOOST_REQUIRE(success);

  BOOST_CHECK_EQUAL(2018, tmDate.tm_year + 1900);
  BOOST_CHECK_EQUAL(11, tmDate.tm_mon + 1);
  BOOST_CHECK_EQUAL(1, tmDate.tm_mday);
  BOOST_CHECK_EQUAL(17, tmDate.tm_hour);
  BOOST_CHECK_EQUAL(45, tmDate.tm_min);
  BOOST_CHECK_EQUAL(59, tmDate.tm_sec);
}

BOOST_AUTO_TEST_CASE(TestGetDateFromDate) {
  SQL_DATE_STRUCT buf;

  buf.year = 1984;
  buf.month = 5;
  buf.day = 27;

  SqlLen reslen = sizeof(buf);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_TDATE, &buf, sizeof(buf),
                               &reslen);

  Date date = appBuf.GetDate();

  tm tmDate;

  bool success = common::DateToCTm(date, tmDate);

  BOOST_REQUIRE(success);

  BOOST_CHECK_EQUAL(1984, tmDate.tm_year + 1900);
  BOOST_CHECK_EQUAL(5, tmDate.tm_mon + 1);
  BOOST_CHECK_EQUAL(27, tmDate.tm_mday);
  BOOST_CHECK_EQUAL(0, tmDate.tm_hour);
  BOOST_CHECK_EQUAL(0, tmDate.tm_min);
  BOOST_CHECK_EQUAL(0, tmDate.tm_sec);
}

BOOST_AUTO_TEST_CASE(TestGetTimestampFromDate) {
  SQL_DATE_STRUCT buf;

  buf.year = 1984;
  buf.month = 5;
  buf.day = 27;

  SqlLen reslen = sizeof(buf);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_TDATE, &buf, sizeof(buf),
                               &reslen);

  Timestamp ts = appBuf.GetTimestamp();

  tm tmDate;

  bool success = common::TimestampToCTm(ts, tmDate);

  BOOST_REQUIRE(success);

  BOOST_CHECK_EQUAL(1984, tmDate.tm_year + 1900);
  BOOST_CHECK_EQUAL(5, tmDate.tm_mon + 1);
  BOOST_CHECK_EQUAL(27, tmDate.tm_mday);
  BOOST_CHECK_EQUAL(0, tmDate.tm_hour);
  BOOST_CHECK_EQUAL(0, tmDate.tm_min);
  BOOST_CHECK_EQUAL(0, tmDate.tm_sec);
}

BOOST_AUTO_TEST_CASE(TestGetTimestampFromTime) {
  SQL_TIME_STRUCT buf;

  buf.hour = 6;
  buf.minute = 34;
  buf.second = 51;

  SqlLen reslen = sizeof(buf);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_TTIME, &buf, sizeof(buf),
                               &reslen);

  Time time = appBuf.GetTime();

  tm tmTime;

  bool success = common::TimeToCTm(time, tmTime);

  BOOST_REQUIRE(success);

  BOOST_CHECK_EQUAL(1970, tmTime.tm_year + 1900);
  BOOST_CHECK_EQUAL(1, tmTime.tm_mon + 1);
  BOOST_CHECK_EQUAL(1, tmTime.tm_mday);
  BOOST_CHECK_EQUAL(6, tmTime.tm_hour);
  BOOST_CHECK_EQUAL(34, tmTime.tm_min);
  BOOST_CHECK_EQUAL(51, tmTime.tm_sec);
}

BOOST_AUTO_TEST_CASE(TestGetTimestampFromTimestamp) {
  SQL_TIMESTAMP_STRUCT buf;

  buf.year = 2004;
  buf.month = 8;
  buf.day = 14;
  buf.hour = 6;
  buf.minute = 34;
  buf.second = 51;
  buf.fraction = 573948623;

  SqlLen reslen = sizeof(buf);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_TTIMESTAMP, &buf, sizeof(buf),
                               &reslen);

  Timestamp ts = appBuf.GetTimestamp();

  tm tmDate;

  bool success = common::TimestampToCTm(ts, tmDate);

  BOOST_REQUIRE(success);

  BOOST_CHECK_EQUAL(2004, tmDate.tm_year + 1900);
  BOOST_CHECK_EQUAL(8, tmDate.tm_mon + 1);
  BOOST_CHECK_EQUAL(14, tmDate.tm_mday);
  BOOST_CHECK_EQUAL(6, tmDate.tm_hour);
  BOOST_CHECK_EQUAL(34, tmDate.tm_min);
  BOOST_CHECK_EQUAL(51, tmDate.tm_sec);
  BOOST_CHECK_EQUAL(573948623, ts.GetSecondFraction());
}

BOOST_AUTO_TEST_CASE(TestGetDateFromTimestamp) {
  SQL_TIMESTAMP_STRUCT buf;

  buf.year = 2004;
  buf.month = 8;
  buf.day = 14;
  buf.hour = 6;
  buf.minute = 34;
  buf.second = 51;
  buf.fraction = 573948623;

  SqlLen reslen = sizeof(buf);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_TTIMESTAMP, &buf, sizeof(buf),
                               &reslen);

  Date date = appBuf.GetDate();

  tm tmDate;

  bool success = common::DateToCTm(date, tmDate);

  BOOST_REQUIRE(success);

  BOOST_CHECK_EQUAL(2004, tmDate.tm_year + 1900);
  BOOST_CHECK_EQUAL(8, tmDate.tm_mon + 1);
  BOOST_CHECK_EQUAL(14, tmDate.tm_mday);
  BOOST_CHECK_EQUAL(6, tmDate.tm_hour);
  BOOST_CHECK_EQUAL(34, tmDate.tm_min);
  BOOST_CHECK_EQUAL(51, tmDate.tm_sec);
}

BOOST_AUTO_TEST_CASE(TestGetTimeFromTimestamp) {
  SQL_TIMESTAMP_STRUCT buf;

  buf.year = 2004;
  buf.month = 8;
  buf.day = 14;
  buf.hour = 6;
  buf.minute = 34;
  buf.second = 51;
  buf.fraction = 573948623;

  SqlLen reslen = sizeof(buf);

  ApplicationDataBuffer appBuf(OdbcNativeType::AI_TTIMESTAMP, &buf, sizeof(buf),
                               &reslen);

  Time time = appBuf.GetTime();

  tm tmTime;

  bool success = common::TimeToCTm(time, tmTime);

  BOOST_REQUIRE(success);

  BOOST_CHECK_EQUAL(1970, tmTime.tm_year + 1900);
  BOOST_CHECK_EQUAL(1, tmTime.tm_mon + 1);
  BOOST_CHECK_EQUAL(1, tmTime.tm_mday);
  BOOST_CHECK_EQUAL(6, tmTime.tm_hour);
  BOOST_CHECK_EQUAL(34, tmTime.tm_min);
  BOOST_CHECK_EQUAL(51, tmTime.tm_sec);
}

BOOST_AUTO_TEST_SUITE_END()
