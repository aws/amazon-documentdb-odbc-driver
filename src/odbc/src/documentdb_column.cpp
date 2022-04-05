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

#include <chrono>
#include <ctime>
#include "ignite/odbc/documentdb_column.h"
#include <ignite/odbc/impl/interop/interop_stream_position_guard.h>
#include "ignite/odbc/utility.h"
#include "bsoncxx/types.hpp"
#include "bsoncxx/json.hpp"

namespace ignite {
namespace odbc {

void DocumentDbColumn::Update(bsoncxx::document::view const& document) {
  document_ = document;
}

DocumentDbColumn::DocumentDbColumn(bsoncxx::document::view& document,
                         JdbcColumnMetadata& columnMetadata, std::string& path)
    : type_(columnMetadata.GetColumnType()),
      document_(document),
      columnMetadata_(columnMetadata),
      path_(path) {
}

int64_t ToValidLong(int64_t value, ConversionResult::Type& convRes, int64_t max,
                    int64_t min) {
  int64_t intValue = value;
  if (intValue > max) {
    intValue = max;
    convRes = ConversionResult::AI_FAILURE;
  } else if (intValue < min) {
    intValue = min;
    convRes = ConversionResult::AI_FAILURE;
  }
  return intValue;
}

int64_t ToValidLong(std::string const& value, ConversionResult::Type& convRes,
               int64_t max, int64_t min) {
  int64_t intValue = 0;
  try {
    intValue =
        ToValidLong(std::stoll(value.c_str(), nullptr, 0), convRes, max, min);
  } catch (std::invalid_argument const&) {
    convRes = ConversionResult::AI_FAILURE;
  } catch (std::out_of_range const&) {
    convRes = ConversionResult::AI_FAILURE;
  }
  return intValue;
}

ConversionResult::Type DocumentDbColumn::PutInt8(
    ApplicationDataBuffer& dataBuf,
    bsoncxx::document::element const& element) const {
  ConversionResult::Type convRes = ConversionResult::Type::AI_SUCCESS;
  bsoncxx::type docType = element.type();
  boost::optional< int8_t > value{};
  switch (docType) {
    case bsoncxx::type::k_int32:
      value =
          ToValidLong(element.get_int32().value, convRes, INT8_MAX, INT8_MIN);
      break;
    case bsoncxx::type::k_int64:
      value =
          ToValidLong(element.get_int64().value, convRes, INT8_MAX, INT8_MIN);
      break;
    case bsoncxx::type::k_double:
      if (element.get_double().value > INT8_MAX
          || element.get_double().value < INT8_MIN) {
        convRes = ConversionResult::AI_FAILURE;
      } else {
        value = element.get_double().value != 0 ? 1 : 0;
      }
      break;
    case bsoncxx::type::k_decimal128:
      value = ToValidLong(element.get_decimal128().value.to_string(), convRes,
                     INT8_MAX, INT8_MIN);
      break;
    case bsoncxx::type::k_utf8:
      value = ToValidLong(element.get_utf8().value.to_string(), convRes,
                     INT8_MAX, INT8_MIN);
      break;
    case bsoncxx::type::k_bool:
      value = element.get_bool().value ? 1 : 0;
      break;
    case bsoncxx::type::k_null:
      break;
    default:
      convRes = ConversionResult::AI_UNSUPPORTED_CONVERSION;
      break;
  }
  if (convRes == ConversionResult::Type::AI_SUCCESS) {
    dataBuf.PutInt8(value);
  }
  return convRes;
}

ConversionResult::Type DocumentDbColumn::PutInt16(
    ApplicationDataBuffer& dataBuf,
    bsoncxx::document::element const& element) const {
  ConversionResult::Type convRes = ConversionResult::Type::AI_SUCCESS;
  bsoncxx::type docType = element.type();
  boost::optional< int16_t > value{};
  switch (docType) {
    case bsoncxx::type::k_int32:
      value =
          ToValidLong(element.get_int32().value, convRes, INT16_MAX, INT16_MIN);
      break;
    case bsoncxx::type::k_int64:
      value =
          ToValidLong(element.get_int64().value, convRes, INT16_MAX, INT16_MIN);
      break;
    case bsoncxx::type::k_double:
      if (element.get_double().value > INT16_MAX
          || element.get_double().value < INT16_MIN) {
        convRes = ConversionResult::AI_FAILURE;
      } else {
        value = element.get_double().value;
      }
      break;
    case bsoncxx::type::k_decimal128:
      value = ToValidLong(element.get_decimal128().value.to_string(), convRes, INT16_MAX,
                     INT16_MIN);
      break;
    case bsoncxx::type::k_utf8:
      value = ToValidLong(element.get_utf8().value.to_string(), convRes,
                     INT16_MAX, INT16_MIN);
      break;
    case bsoncxx::type::k_bool:
      value = element.get_bool().value ? 1 : 0;
      break;
    case bsoncxx::type::k_date:
      value = element.get_date().value.count();
      break;
    case bsoncxx::type::k_timestamp:
      value = element.get_timestamp().timestamp;
      break;
    case bsoncxx::type::k_null:
      break;
    default:
      convRes = ConversionResult::AI_UNSUPPORTED_CONVERSION;
      break;
  }
  if (convRes == ConversionResult::Type::AI_SUCCESS) {
    dataBuf.PutInt16(value);
  }
  return convRes;
}

ConversionResult::Type DocumentDbColumn::PutInt32(
    ApplicationDataBuffer& dataBuf,
    bsoncxx::document::element const& element) const {
  ConversionResult::Type convRes = ConversionResult::Type::AI_SUCCESS;
  bsoncxx::type docType = element.type();
  boost::optional< int32_t > value{};
  switch (docType) {
    case bsoncxx::type::k_int32:
      value = element.get_int32().value;
      break;
    case bsoncxx::type::k_int64:
      value =
          ToValidLong(element.get_int64().value, convRes, INT32_MAX, INT32_MIN);
      break;
    case bsoncxx::type::k_double:
      if (element.get_double().value > INT32_MAX
          || element.get_double().value < INT32_MIN) {
        convRes = ConversionResult::AI_FAILURE;
      } else {
        value = element.get_double().value;
      }
      break;
    case bsoncxx::type::k_decimal128:
      value = ToValidLong(element.get_decimal128().value.to_string(), convRes,
                     INT32_MAX, INT32_MIN);
      break;
    case bsoncxx::type::k_utf8:
      value = ToValidLong(element.get_utf8().value.to_string(), convRes,
                     INT32_MAX, INT32_MIN);
      break;
    case bsoncxx::type::k_bool:
      value = element.get_bool().value ? 1 : 0;
      break;
    case bsoncxx::type::k_date:
      value = element.get_date().value.count();
      break;
    case bsoncxx::type::k_timestamp:
      value = element.get_timestamp().timestamp;
      break;
    case bsoncxx::type::k_null:
      break;
    default:
      convRes = ConversionResult::AI_UNSUPPORTED_CONVERSION;
      break;
  }
  if (convRes == ConversionResult::Type::AI_SUCCESS) {
    dataBuf.PutInt32(value);
  }
  return convRes;
}

ConversionResult::Type DocumentDbColumn::PutInt64(
    ApplicationDataBuffer& dataBuf,
    bsoncxx::document::element const& element) const {
  ConversionResult::Type convRes = ConversionResult::Type::AI_SUCCESS;
  bsoncxx::type docType = element.type();
  boost::optional< int64_t > value{};
  switch (docType) {
    case bsoncxx::type::k_int32:
      value = element.get_int32().value;
      break;
    case bsoncxx::type::k_int64:
      value = element.get_int64().value;
      break;
    case bsoncxx::type::k_double:
      if (element.get_double().value > INT64_MAX
          || element.get_double().value < INT64_MIN) {
        convRes = ConversionResult::AI_FAILURE;
      } else {
        value = element.get_double().value;
      }
      break;
    case bsoncxx::type::k_decimal128:
      value = ToValidLong(element.get_decimal128().value.to_string(), convRes,
                     INT64_MAX, INT64_MIN);
      break;
    case bsoncxx::type::k_utf8:
      value = ToValidLong(element.get_utf8().value.to_string(), convRes,
                     INT64_MAX, INT64_MIN);
      break;
    case bsoncxx::type::k_bool:
      value = element.get_bool().value ? 1 : 0;
      break;
    case bsoncxx::type::k_date:
      value = element.get_date().value.count();
      break;
    case bsoncxx::type::k_timestamp:
      value = element.get_timestamp().timestamp;
      break;
    case bsoncxx::type::k_null:
      break;
    default:
      convRes = ConversionResult::AI_UNSUPPORTED_CONVERSION;
      break;
  }
  if (convRes == ConversionResult::Type::AI_SUCCESS) {
    dataBuf.PutInt64(value);
  }
  return convRes;
}

ConversionResult::Type DocumentDbColumn::PutFloat(
    ApplicationDataBuffer& dataBuf,
    bsoncxx::document::element const& element) const {
  ConversionResult::Type convRes = ConversionResult::Type::AI_SUCCESS;
  bsoncxx::type docType = element.type();
  boost::optional< float > value{};
  switch (docType) {
    case bsoncxx::type::k_int32:
      value = element.get_int32().value;
      break;
    case bsoncxx::type::k_int64:
      value = element.get_int64().value;
      break;
    case bsoncxx::type::k_double:
      value = element.get_double().value;
      break;
    case bsoncxx::type::k_decimal128:
      value = std::stof(element.get_decimal128().value.to_string());
      break;
    case bsoncxx::type::k_utf8:
      value = std::stof(element.get_utf8().value.to_string());
      break;
    case bsoncxx::type::k_bool:
      value = element.get_bool().value ? 1 : 0;
      break;
    case bsoncxx::type::k_date:
      value = element.get_date().value.count();
      break;
    case bsoncxx::type::k_timestamp:
      value = element.get_timestamp().timestamp;
      break;
    case bsoncxx::type::k_null:
      break;
    default:
      convRes = ConversionResult::AI_UNSUPPORTED_CONVERSION;
      break;
  }
  if (convRes == ConversionResult::Type::AI_SUCCESS) {
    dataBuf.PutFloat(value);
  }
  return convRes;
}

ConversionResult::Type DocumentDbColumn::PutDouble(
    ApplicationDataBuffer& dataBuf,
    bsoncxx::document::element const& element) const {
  ConversionResult::Type convRes = ConversionResult::Type::AI_SUCCESS;
  bsoncxx::type docType = element.type();
  boost::optional< double > value{};
  switch (docType) {
    case bsoncxx::type::k_int32:
      value = element.get_int32().value;
      break;
    case bsoncxx::type::k_int64:
      value = element.get_int64().value;
      break;
    case bsoncxx::type::k_double:
      value = element.get_double().value;
      break;
    case bsoncxx::type::k_decimal128:
      value = std::stod(element.get_decimal128().value.to_string());
      break;
    case bsoncxx::type::k_utf8:
      value = std::stod(element.get_utf8().value.to_string());
      break;
    case bsoncxx::type::k_bool:
      value = element.get_bool().value ? 1 : 0;
      break;
    case bsoncxx::type::k_date:
      value = element.get_date().value.count();
      break;
    case bsoncxx::type::k_timestamp:
      value = element.get_timestamp().timestamp;
      break;
    case bsoncxx::type::k_null:
      break;
    default:
      convRes = ConversionResult::AI_UNSUPPORTED_CONVERSION;
      break;
  }
  if (convRes == ConversionResult::Type::AI_SUCCESS) {
    dataBuf.PutDouble(value);
  }
  return convRes;
}

ConversionResult::Type
    DocumentDbColumn::PutString(
    ApplicationDataBuffer& dataBuf,
    bsoncxx::document::element const& element) const {
  ConversionResult::Type convRes = ConversionResult::AI_SUCCESS;  
  bsoncxx::type docType = element.type();
  boost::optional< std::string > value{};
  switch (docType) {
    case bsoncxx::type::k_int32:
      value = std::to_string(element.get_int32().value);
      break;
    case bsoncxx::type::k_int64:
      value = std::to_string(element.get_int64().value);
      break;
    case bsoncxx::type::k_double:
      value = std::to_string(element.get_double().value);
      break;
    case bsoncxx::type::k_decimal128:
      value = element.get_decimal128().value.to_string();
      break;
    case bsoncxx::type::k_utf8:
      value = element.get_utf8().value.to_string();
      break;
    case bsoncxx::type::k_binary: {
      std::stringstream ss;
      ss << std::hex;
      int32_t array_size = element.get_binary().size;
      const uint8_t* data = element.get_binary().bytes;
      for (int i = 0; i < array_size; i++) {
        ss << std::setw(2) << std::setfill('0') << (int)data[i];
      }
      value = ss.str();
      break;
    }
    case bsoncxx::type::k_oid:
      value = element.get_oid().value.to_string();
      break;
    case bsoncxx::type::k_bool:
      value = std::to_string(element.get_bool().value);
      break;
    case bsoncxx::type::k_date:
      value = std::to_string(std::chrono::system_clock::to_time_t(
          std::chrono::system_clock::time_point(element.get_date().value)));
      break;
    case bsoncxx::type::k_timestamp:
      value = std::to_string(element.get_timestamp().timestamp);
      break;
    case bsoncxx::type::k_null:
      break;
    case bsoncxx::type::k_maxkey:
      value = "MAXKEY";
      break;
    case bsoncxx::type::k_minkey:
      value = "MINKEY";
      break;
    case bsoncxx::type::k_document:
      // probably need to convert to string
      value = bsoncxx::to_json(element.get_document().value);
      break;
    case bsoncxx::type::k_array:
      // probably need to convert to string
      value = bsoncxx::to_json(element.get_array().value);
      break;
    default:
      convRes = ConversionResult::AI_UNSUPPORTED_CONVERSION;
      break;
  }
  if (convRes == ConversionResult::Type::AI_SUCCESS) {
    dataBuf.PutString(value);
  }
  return convRes;
}

ConversionResult::Type DocumentDbColumn::PutDecimal(
    ApplicationDataBuffer& dataBuf,
    bsoncxx::document::element const& element) const {
  ConversionResult::Type convRes = ConversionResult::Type::AI_SUCCESS;
  bsoncxx::type docType = element.type();
  boost::optional< common::Decimal > value{};
  switch (docType) {
    case bsoncxx::type::k_int32:
      value = common::Decimal(element.get_int32().value);
      break;
    case bsoncxx::type::k_int64:
      value = common::Decimal(element.get_int64().value);
      break;
    case bsoncxx::type::k_double:
      value = common::Decimal(std::to_string(element.get_double().value));
      break;
    case bsoncxx::type::k_decimal128:
      value = common::Decimal(element.get_decimal128().value.to_string());
      break;
    case bsoncxx::type::k_utf8:
      value = common::Decimal(element.get_utf8().value.to_string());
      break;
    case bsoncxx::type::k_bool:
      value = common::Decimal(element.get_bool().value ? 1 : 0);
      break;
    default:
      convRes = ConversionResult::AI_UNSUPPORTED_CONVERSION;
      break;
  }
  if (convRes == ConversionResult::AI_SUCCESS) {
    dataBuf.PutDecimal(value);
  }
  return convRes;
}

ConversionResult::Type DocumentDbColumn::PutTime(
    ApplicationDataBuffer& dataBuf,
    bsoncxx::document::element const& element) const {
  ConversionResult::Type convRes = ConversionResult::Type::AI_SUCCESS;
  bsoncxx::type docType = element.type();
  boost::optional< Time > value{};
  switch (docType) {
    case bsoncxx::type::k_int64:
      break;
      value = Time(element.get_int64().value);
    case bsoncxx::type::k_date:
      value = Time(element.get_date().value.count());
      break;
    case bsoncxx::type::k_timestamp:
      // TODO: Determine if this is correct to milliseconds
      // https://bitquill.atlassian.net/browse/AD-679
      value = Time(element.get_timestamp().timestamp);
      break;
    case bsoncxx::type::k_utf8:
      // TODO: Determine if we could support reading data as string
      // https://bitquill.atlassian.net/browse/AD-680
      convRes = ConversionResult::AI_UNSUPPORTED_CONVERSION;
      break;
    default:
      convRes = ConversionResult::AI_UNSUPPORTED_CONVERSION;
      break;
  }
  if (convRes == ConversionResult::AI_SUCCESS) {
    dataBuf.PutTime(value);
  }
  return convRes;
}

ConversionResult::Type DocumentDbColumn::PutDate(
    ApplicationDataBuffer& dataBuf,
    bsoncxx::document::element const& element) const {
  ConversionResult::Type convRes = ConversionResult::Type::AI_SUCCESS;
  bsoncxx::type docType = element.type();
  boost::optional< Date > value{};
  switch (docType) {
    case bsoncxx::type::k_int64:
      break;
      value = Date(element.get_int64().value);
    case bsoncxx::type::k_date:
      value = Date(element.get_date().value.count());
      break;
    case bsoncxx::type::k_timestamp:
      // TODO: Determine if this is correct to milliseconds
      // https://bitquill.atlassian.net/browse/AD-679
      value = Date(element.get_timestamp().timestamp);
      break;
    case bsoncxx::type::k_utf8:
      // TODO: Determine if we could support reading data as string
      // https://bitquill.atlassian.net/browse/AD-680
      convRes = ConversionResult::AI_UNSUPPORTED_CONVERSION;
      break;
    default:
      convRes = ConversionResult::AI_UNSUPPORTED_CONVERSION;
      break;
  }
  if (convRes == ConversionResult::AI_SUCCESS) {
    dataBuf.PutDate(value);

  }
  return convRes;
}

ConversionResult::Type DocumentDbColumn::PutTimestamp(
    ApplicationDataBuffer& dataBuf,
    bsoncxx::document::element const& element) const {
  ConversionResult::Type convRes = ConversionResult::Type::AI_SUCCESS;
  bsoncxx::type docType = element.type();
  boost::optional< Timestamp > value{};
  switch (docType) {
    case bsoncxx::type::k_int64:
      value = Timestamp(element.get_int64().value);
      break;
    case bsoncxx::type::k_date:
      value = Timestamp(element.get_date().value.count());
      break;
    case bsoncxx::type::k_timestamp:
      // TODO: Determine if this is correct to milliseconds
      // https://bitquill.atlassian.net/browse/AD-679
      value = Timestamp(element.get_timestamp().timestamp);
      break;
    case bsoncxx::type::k_utf8:
      // TODO: Determine if we could support reading data as string
      // https://bitquill.atlassian.net/browse/AD-680
      convRes = ConversionResult::AI_UNSUPPORTED_CONVERSION;
      break;
    default:
      convRes = ConversionResult::AI_UNSUPPORTED_CONVERSION;
      break;
  }
  if (convRes == ConversionResult::AI_SUCCESS) {
    dataBuf.PutTimestamp(value);
  }
  return convRes;
}

ConversionResult::Type DocumentDbColumn::PutBinaryData(
    ApplicationDataBuffer& dataBuf,
    bsoncxx::document::element const& element) const {
  ConversionResult::Type convRes = ConversionResult::AI_SUCCESS;
  bsoncxx::type docType = element.type();
  const void* value = nullptr;
  size_t length = 0;
  switch (docType) {
    case bsoncxx::type::k_utf8:
      value = element.get_utf8().value.to_string().c_str();
      length = element.get_utf8().value.to_string().length();
      break;
    case bsoncxx::type::k_binary:
      value = element.get_binary().bytes;
      length = element.get_binary().size;
      break;
    case bsoncxx::type::k_oid:
      value = element.get_oid().value.bytes();
      length = element.get_oid().value.size();
      break;
    case bsoncxx::type::k_null:
      break;
    default:
      convRes = ConversionResult::AI_UNSUPPORTED_CONVERSION;
      break;
  }
  if (convRes == ConversionResult::Type::AI_SUCCESS) {
    if (value) {
      int32_t len_written = 0;
      convRes = dataBuf.PutBinaryData(value, length, len_written);
    } else {
      convRes = dataBuf.PutNull();
    }
  }
  return convRes;
}

ConversionResult::Type DocumentDbColumn::ReadToBuffer(
    ApplicationDataBuffer& dataBuf) const {

  auto element = document_[path_];
  // Invalid (or missing) element is null
  if (!element) {
    dataBuf.PutNull();
    return ConversionResult::AI_SUCCESS;
  }
  
  ConversionResult::Type convRes = ConversionResult::AI_SUCCESS;

  switch (type_) {

    case JDBC_TYPE_BOOLEAN:
    case JDBC_TYPE_SMALLINT:
      convRes = PutInt8(dataBuf, element);
      break;

    case JDBC_TYPE_TINYINT:
      convRes = PutInt16(dataBuf, element);
      break;

    case JDBC_TYPE_INTEGER:
      convRes = PutInt32(dataBuf, element);
      break;

    case JDBC_TYPE_BIGINT:
      convRes = PutInt64(dataBuf, element);
      break;

    case JDBC_TYPE_FLOAT:
      convRes = PutFloat(dataBuf, element);
      break;

    case JDBC_TYPE_DOUBLE:
      convRes = PutDouble(dataBuf, element);
      break;

    case JDBC_TYPE_VARCHAR:
    case JDBC_TYPE_CHAR:
    case JDBC_TYPE_NCHAR:
    case JDBC_TYPE_NVARCHAR:
    case JDBC_TYPE_LONGVARCHAR:
    case JDBC_TYPE_LONGNVARCHAR:
      convRes = PutString(dataBuf, element);
      break;

    case JDBC_TYPE_NULL:
      convRes = dataBuf.PutNull();
      break;

    case JDBC_TYPE_BINARY:
    case JDBC_TYPE_VARBINARY:
      convRes = PutBinaryData(dataBuf, element);
      break;

    case JDBC_TYPE_DECIMAL:
      convRes = PutDecimal(dataBuf, element);
      break;

    case JDBC_TYPE_DATE:
      convRes = PutDate(dataBuf, element);
      break;

    case JDBC_TYPE_TIMESTAMP:
      convRes = PutTimestamp(dataBuf, element);
      break;

    case JDBC_TYPE_TIME:
      convRes = PutTime(dataBuf, element);
      break;
    
    default:
      return ConversionResult::AI_UNSUPPORTED_CONVERSION;
  }

  return convRes;
}
}  // namespace odbc
}  // namespace ignite
