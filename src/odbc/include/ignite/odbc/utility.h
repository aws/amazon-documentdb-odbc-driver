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

#ifndef _IGNITE_ODBC_UTILITY
#define _IGNITE_ODBC_UTILITY

#ifdef min
#undef min
#endif  // min

#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>

#include <ignite/odbc/common/decimal.h>
#include <ignite/odbc/common/utils.h>
#include <stdint.h>

#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <string>

#include "ignite/odbc/impl/binary/binary_reader_impl.h"
#include "ignite/odbc/impl/binary/binary_writer_impl.h"
#include <sqltypes.h>
#include <sql.h>


namespace ignite {
namespace odbc {
namespace utility {
/** Using common version of the util. */
using common::IntoLower;
using namespace odbc::impl::binary;
using namespace odbc::common;

template < typename T >
T* GetPointerWithOffset(T* ptr, size_t offset) {
  uint8_t* ptrBytes = (uint8_t*)ptr;

  return (T*)(ptrBytes + offset);
}

/**
 * Copy utf-8 string to SQLCHAR buffer of the specific length. It will ensure
 * null terminated result, possibly truncted.
 * @param inBuffer UTF-8, null-terminated string to copy data from.
 * @param outBuffer SQLCHAR buffer to copy data to.
 * @param outBufferLenBytes Length of the output buffer, in bytes.
 * @return isTruncted Reference to indicator of whether the input string was
 * truncted in the output buffer.
 */
size_t CopyUtf8StringToSqlCharString(const char* inBuffer, SQLCHAR* outBuffer,
                                     size_t outBufferLenBytes,
                                     bool& isTruncated);

/**
 * Copy utf-8 string to SQLWCHAR buffer of the specific length. It will ensure
 * null terminated result, possibly truncted.
 * @param inBuffer UTF-8, null-terminated string to copy data from.
 * @param outBuffer SQLWCHAR buffer to copy data to.
 * @param outBufferLenBytes Length of the output buffer, in bytes.
 * @return isTruncted Reference to indicator of whether the input string was
 * truncted in the output buffer.
 */
size_t CopyUtf8StringToSqlWcharString(const char* inBuffer, SQLWCHAR* outBuffer,
                                      size_t outBufferLenBytes,
                                      bool& isTruncated);

/**
 * Copy string to buffer of the specific length.
 * @param str String to copy data from.
 * @param buf Buffer to copy data to.
 * @param buflen Length of the buffer.
 * @return Length of the resulting string in buffer.
 */
size_t CopyStringToBuffer(const std::string& str, SQLWCHAR* buf, size_t buflen,
                          bool lenInBytes = false);

/**
 * Read array from reader.
 * @param reader Reader.
 * @param res Resulting vector.
 */
void ReadByteArray(BinaryReaderImpl& reader,
                   std::vector< int8_t >& res);

/**
 * Read string from reader.
 * @param reader Reader.
 * @param str String.
 */
void ReadString(BinaryReaderImpl& reader, std::string& str);

/**
 * Write string using writer.
 * @param writer Writer.
 * @param str String.
 */
void WriteString(BinaryWriterImpl& writer,
                 const std::string& str);

/**
 * Read decimal value using reader.
 *
 * @param reader Reader.
 * @param decimal Decimal value.
 */
void ReadDecimal(BinaryReaderImpl& reader,
                 Decimal& decimal);

/**
 * Write decimal value using writer.
 *
 * @param writer Writer.
 * @param decimal Decimal value.
 */
void WriteDecimal(BinaryWriterImpl& writer,
                  const Decimal& decimal);

/**
 * Convert SQL string buffer to std::string.
 *
 * @param sqlStr SQL string buffer.
 * @param sqlStrLen SQL string length.
 * @param lenInBytes Indicator of whether the length indicates bytes or characters.
 * @return Standard string containing the same data.
 */
std::string SqlStringToString(const SQLWCHAR* sqlStr, int32_t sqlStrLen = SQL_NTS,
                              bool lenInBytes = false);

/**
 * Convert SQL string buffer to boost::optional< std::string >.
 *
 * @param sqlStr SQL string buffer.
 * @param sqlStrLen SQL string length.
 * @return Standard optional string containing the same data.
 * If sqlStrLen indicates null string, boost::none is returned.
 */
boost::optional< std::string > SqlStringToOptString(const SQLWCHAR* sqlStr,
                                                    int32_t sqlStrLen = SQL_NTS,
                                                    bool lenInBytes = false);

/**
 * Convert a wide string to UTF-8 encoded string.
 *
 * @param value wide string value to convert.
 * @return String value converted to UTF-8 encoding.
 */
std::string ToUtf8(const std::wstring& value);

/**
 * Convert a wide string to UTF-8 encoded string.
 *
 * @param value wide string value to convert.
 * @return String value converted to UTF-8 encoding.
 */
std::string ToUtf8(const wchar_t* value);

/**
 * Convert a UTF-8 encoded string to wide string.
 *
 * @param value UTF-8 encoded string.
 * @return String value converted to UTF-8 encoding.
 */
std::wstring FromUtf8(const std::string& value);

/**
 * Convert a UTF-8 encoded string to wide string.
 *
 * @param value Pointer to UTF-8 encoded string.
 * @return String value converted to UTF-8 encoding.
 */
std::wstring FromUtf8(const char* value);

/**
 * Convert a wide string to vector of unsigned short.
 *
 * @param value wide string value to convert.
 * @return String value converted to vector of unsigned short encoding.
 */
std::vector< SQLWCHAR > ToWCHARVector(const std::wstring& value);

/**
 * Convert a wide string to vector of unsigned short.
 *
 * @param value pointer to null-terminated wide string value to convert.
 * @return String value converted to vector of unsigned short encoding.
 */
std::vector< SQLWCHAR > ToWCHARVector(const wchar_t* value);

/**
 * Convert a UTF-8 string to vector of unsigned short.
 *
 * @param value wide string value to convert.
 * @return String value converted to vector of unsigned short encoding.
 */
std::vector< SQLWCHAR > ToWCHARVector(const std::string& value);

/**
 * Convert a UTF-8 string to vector of unsigned short.
 *
 * @param value pointer to null-terminated wide string value to convert.
 * @return String value converted to vector of unsigned short encoding.
 */
std::vector< SQLWCHAR > ToWCHARVector(const char* value);

/**
 * Convert binary data to hex dump form
 * @param data  pointer to data
 * @param count data length
 * @return standard string containing the formated hex dump
 */
std::string HexDump(const void* data, size_t count);
}  // namespace utility
}  // namespace odbc
}  // namespace ignite

namespace std {
/** 
 * Convert wstring to utf-8 encoding.
 */
inline std::ostream& operator<<(std::ostream& out, const std::wstring& value) {
  out << ignite::odbc::utility::ToUtf8(value);
  return out;
}
}  // namespace std

#endif  //_IGNITE_ODBC_UTILITY
