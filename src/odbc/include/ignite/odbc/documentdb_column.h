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

#ifndef _IGNITE_ODBC_DOCUMENTDB_COLUMN
#define _IGNITE_ODBC_DOCUMENTDB_COLUMN

#include <stdint.h>
#include <ignite/odbc/app/application_data_buffer.h>
#include <ignite/odbc/jni/jdbc_column_metadata.h>
#include <ignite/odbc/impl/binary/binary_reader_impl.h>
#include <bsoncxx/document/view.hpp>

using namespace ignite::odbc::impl::binary;
using ignite::odbc::app::ApplicationDataBuffer;
using ignite::odbc::app::ConversionResult;
using ignite::odbc::jni::JdbcColumnMetadata;

namespace ignite {
namespace odbc {
/**
 * Result set column.
 */
class DocumentDbColumn {
 public:
  /**
   * Default constructor.
   */
  DocumentDbColumn() = delete;

  /**
   * Copy constructor.
   *
   * @param other Another instance.
   */
  DocumentDbColumn(const DocumentDbColumn& other) = default;

  /**
   * Copy operator.
   *
   * @param other Another instance.
   * @return This.
   */
  DocumentDbColumn& operator=(const DocumentDbColumn& other) = delete;

  /**
   * Updates the reference to the current document.
   */
  void Update(bsoncxx::document::view const& document);

  /**
   * Destructor.
   */
  ~DocumentDbColumn() = default;

  /**
   * Constructor.
   *
   * @param reader Reader to be used to retrieve column data.
   */
  DocumentDbColumn(bsoncxx::document::view& document,
                   JdbcColumnMetadata& columnMetadata, std::string& path);

  /**
   * Get column size in bytes.
   *
   * @return Column size.
   */
  int32_t GetSize() const {
    return size_;
  }

  /**
   * Read column data and store it in application data buffer.
   *
   * @param reader Reader to use.
   * @param dataBuf Application data buffer.
   * @return Operation result.
   */
  ConversionResult::Type ReadToBuffer(ApplicationDataBuffer& dataBuf) const;

 private:
  /** Setter for int8 data type */
  ConversionResult::Type PutInt8(
      ApplicationDataBuffer& dataBuf,
      bsoncxx::document::element const& element) const;
  /** Setter for int16 data type */
  ConversionResult::Type PutInt16(
      ApplicationDataBuffer& dataBuf,
      bsoncxx::document::element const& element) const;
  /** Setter for int32 data type */
  ConversionResult::Type PutInt32(
      ApplicationDataBuffer& dataBuf,
      bsoncxx::document::element const& element) const;
  /** Setter for int64 data type */
  ConversionResult::Type PutInt64(
      ApplicationDataBuffer& dataBuf,
      bsoncxx::document::element const& element) const;
  /** Setter for float data type */
  ConversionResult::Type PutFloat(
      ApplicationDataBuffer& dataBuf,
      bsoncxx::document::element const& element) const;
  /** Setter for double data type */
  ConversionResult::Type PutDouble(
      ApplicationDataBuffer& dataBuf,
      bsoncxx::document::element const& element) const;
  /** Setter for string data type */
  ConversionResult::Type PutString(
      ApplicationDataBuffer& dataBuf,
      bsoncxx::document::element const& element) const;
  /** Setter for decimal data type */
  ConversionResult::Type PutDecimal(
      ApplicationDataBuffer& dataBuf,
      bsoncxx::document::element const& element) const;
  /** Setter for time data type */
  ConversionResult::Type PutTime(
      ApplicationDataBuffer& dataBuf,
      bsoncxx::document::element const& element) const;
  /** Setter for date data type */
  ConversionResult::Type PutDate(
      ApplicationDataBuffer& dataBuf,
      bsoncxx::document::element const& element) const;
  /** Setter for timestamp data type */
  ConversionResult::Type PutTimestamp(
      ApplicationDataBuffer& dataBuf,
      bsoncxx::document::element const& element) const;
  /** Setter for binary data type */
  ConversionResult::Type PutBinaryData(
      ApplicationDataBuffer& dataBuf,
      bsoncxx::document::element const& element) const;

  /** Column type */
  int32_t type_;

  /** Column data size in bytes. */
  int32_t size_ = 0;

  bsoncxx::document::view& document_;

  JdbcColumnMetadata& columnMetadata_;

  std::string& path_;
};
}  // namespace odbc
}  // namespace ignite

#endif  //_IGNITE_ODBC_DOCUMENTDB_COLUMN
