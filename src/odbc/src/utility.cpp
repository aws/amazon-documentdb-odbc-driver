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

#include "ignite/odbc/utility.h"

#include <ignite/odbc/impl/binary/binary_utils.h>

#include <cassert>
#include <codecvt>

#include "ignite/odbc/system/odbc_constants.h"
#include "ignite/odbc/log.h"

namespace ignite {
namespace odbc {
namespace utility {
using namespace odbc::impl::binary;
using namespace odbc::common;

size_t CopyUtf8StringToSqlCharString(const char* inBuffer, SQLCHAR* outBuffer,
                                     size_t outBufferLenBytes,
                                     bool& isTruncated) {
  if (!inBuffer || (outBuffer && outBufferLenBytes == 0))
    return 0;

  // Need to convert input string to wide-char to get the
  // length in characters - as well as get .narrow() to work, as expected
  // Otherwise, it would be impossible to safely determine the
  // output buffer length needed.
  static std::wstring_convert< std::codecvt_utf8< wchar_t >, wchar_t >
      converter;
  std::wstring inString = converter.from_bytes(inBuffer);
  size_t inBufferLenChars = inString.size();

  // If no output buffer, return REQUIRED length.
  if (!outBuffer)
    return inBufferLenChars;

  size_t outBufferLenActual = std::min(inBufferLenChars, outBufferLenBytes - 1);

  std::locale currentLocale("");
  std::use_facet< std::ctype< wchar_t > >(currentLocale)
      .narrow(inString.data(), inString.data() + outBufferLenActual, '?',
              reinterpret_cast< char* >(outBuffer));

  outBuffer[outBufferLenActual] = 0;
  isTruncated = (outBufferLenActual < inBufferLenChars);

  return outBufferLenActual;
}

template < typename OutCharT >
size_t CopyUtf8StringToWcharString(const char* inBuffer, OutCharT* outBuffer,
                                   size_t outBufferLenBytes,
                                   bool& isTruncated) {
  if (!inBuffer || (outBuffer && outBufferLenBytes == 0))
    return 0;

  size_t wCharSize = sizeof(SQLWCHAR);
  // This is intended to convert to the SQLWCHAR. Ensure we have the same size.
  assert(sizeof(OutCharT) == wCharSize);
  assert((outBufferLenBytes % wCharSize) == 0);

  // The number of characters that can be safely transfered, excluding the
  // null terminating character.
  size_t outBufferLenChars;
  // Find the lenght (in bytes) of the input string.
  // This does NOT include the null-terminating character.
  size_t inBufferLen = std::strlen(inBuffer);
  OutCharT* pOutBuffer;
  std::vector< OutCharT > targetProxy;

  if (outBuffer) {
    pOutBuffer = reinterpret_cast< OutCharT* >(outBuffer);
    outBufferLenChars = (outBufferLenBytes / wCharSize) - 1;
  } else {
    // Creates a proxy buffer so we can determine the required length.
    // This buffer will be ignored and automatically deleted after use.
    targetProxy.resize(inBufferLen + 1);
    pOutBuffer = targetProxy.data();
    outBufferLenChars = inBufferLen;
  }

  // Setup conversion facet.
  const std::codecvt_utf8< OutCharT > convFacet;
  std::mbstate_t convState = std::mbstate_t();
  // Pointer to next for input.
  const char* pInBufferNext;
  // Pointer to next for output.
  OutCharT* pOutBufferNext;

  // translate characters:
  const char* inBufferEnd = (inBuffer + inBufferLen);
  std::codecvt_base::result result =
      convFacet.in(convState, inBuffer, inBufferEnd, pInBufferNext,
                   pOutBuffer, pOutBuffer + outBufferLenChars, pOutBufferNext);

  size_t lenConverted = 0;
  switch (result) {
    case std::codecvt_base::ok:
    case std::codecvt_base::partial:
      // The number of characters converted (in OutCharT)
      lenConverted = pOutBufferNext - pOutBuffer;
      // null-terminate target string, if room
      pOutBuffer[lenConverted] = 0;

      // Note:
      isTruncated = (result == std::codecvt_base::partial
                     || (inBufferEnd != pInBufferNext));
      break;
    case std::codecvt_base::error:
      // Error returned if unable to convert character.
      LOG_ERROR_MSG("Unable to convert character '" << *pInBufferNext << "'");

      // The number of characters converted (in OutCharT)
      lenConverted = pOutBufferNext - pOutBuffer;
      // null-terminate target string, if room.
      pOutBuffer[lenConverted] = 0;

      isTruncated = true;
      break;
    default:
      // This situation occurs if the source and target are the same encoding.
      // Impossible?
      LOG_ERROR_MSG("Unexpected error converting string '" << inBuffer << "'");
      assert(false);
      break;
  }

  // Return the number of bytes transfered or required.
  return lenConverted * wCharSize;
}

size_t CopyUtf8StringToSqlWcharString(const char* inBuffer, SQLWCHAR* outBuffer,
                                      size_t outBufferLenBytes,
                                      bool& isTruncated) {
  if (!inBuffer)
    return 0;

  // Handles SQLWCHAR if either UTF-16 and UTF-32
  size_t wCharSize = sizeof(SQLWCHAR);
  switch (wCharSize) {
    case 2:
      return CopyUtf8StringToWcharString(
          inBuffer, reinterpret_cast< char16_t* >(outBuffer), outBufferLenBytes,
          isTruncated);
    case 4:
      return CopyUtf8StringToWcharString(
          inBuffer, reinterpret_cast< char32_t* >(outBuffer), outBufferLenBytes,
          isTruncated);
    default:
      LOG_ERROR_MSG("Unexpected error converting string '" << inBuffer << "'");
      assert(false);
      return 0;
  }
}

// High-level entry point to handle buffer size in either bytes or characters
size_t CopyStringToBuffer(const std::string& str, SQLWCHAR* buf, size_t buflen,
                          bool& isTruncated, bool isLenInBytes) {
  size_t wCharSize = sizeof(SQLWCHAR);

  // Ensure non-zero length in bytes is a multiple of wide char size.
  assert(!isLenInBytes || (buflen % wCharSize == 0));

  // Convert buffer length to bytes.
  size_t bufLenInBytes = isLenInBytes ? buflen : buflen * wCharSize;
  isTruncated = false;
  size_t bytesWritten = CopyUtf8StringToSqlWcharString(
      str.c_str(), buf, bufLenInBytes, isTruncated);
  return isLenInBytes ? bytesWritten : bytesWritten / wCharSize;
}

void ReadString(BinaryReaderImpl& reader, std::string& str) {
  int32_t strLen = reader.ReadString(0, 0);

  if (strLen > 0) {
    str.resize(strLen);

    reader.ReadString(&str[0], static_cast< int32_t >(str.size()));
  } else {
    str.clear();

    if (strLen == 0) {
      char dummy;

      reader.ReadString(&dummy, sizeof(dummy));
    }
  }
}

void WriteString(BinaryWriterImpl& writer, const std::string& str) {
  writer.WriteString(str.data(), static_cast< int32_t >(str.size()));
}

void ReadDecimal(BinaryReaderImpl& reader, Decimal& decimal) {
  int8_t hdr = reader.ReadInt8();

  assert(hdr == IGNITE_TYPE_DECIMAL);

  IGNITE_UNUSED(hdr);

  int32_t scale = reader.ReadInt32();

  int32_t len = reader.ReadInt32();

  std::vector< int8_t > mag;

  mag.resize(len);

  BinaryUtils::ReadInt8Array(reader.GetStream(), mag.data(),
                             static_cast< int32_t >(mag.size()));

  int32_t sign = 1;

  if (mag[0] < 0) {
    mag[0] &= 0x7F;

    sign = -1;
  }

  Decimal res(mag.data(), static_cast< int32_t >(mag.size()), scale, sign);

  decimal.Swap(res);
}

void WriteDecimal(BinaryWriterImpl& writer, const Decimal& decimal) {
  writer.WriteInt8(IGNITE_TYPE_DECIMAL);

  const BigInteger& unscaled = decimal.GetUnscaledValue();

  writer.WriteInt32(decimal.GetScale());

  FixedSizeArray< int8_t > magnitude;

  unscaled.MagnitudeToBytes(magnitude);

  int8_t addBit = unscaled.GetSign() == -1 ? -0x80 : 0;

  if (magnitude[0] < 0) {
    writer.WriteInt32(magnitude.GetSize() + 1);
    writer.WriteInt8(addBit);
  } else {
    writer.WriteInt32(magnitude.GetSize());
    magnitude[0] |= addBit;
  }

  BinaryUtils::WriteInt8Array(writer.GetStream(), magnitude.GetData(),
                              magnitude.GetSize());
}

std::string SqlWcharToString(const SQLWCHAR* sqlStr, int32_t sqlStrLen,
                              bool isLenInBytes) {
  if (!sqlStr)
    return std::string();

  size_t char_size = sizeof(SQLWCHAR);

  assert(char_size == sizeof(wchar_t) || char_size == 2);

  static std::wstring_convert< std::codecvt_utf8< wchar_t >, wchar_t >
      converter;
  std::wstring sqlStr0;
  if (sqlStrLen == SQL_NTS) {
    for (int i = 0; sqlStr[i] != 0; i++) {
      sqlStr0.push_back(sqlStr[i]);
    }
  } else if (sqlStrLen > 0) {
    size_t charsToCopy = isLenInBytes ? (sqlStrLen / char_size) : sqlStrLen;
    sqlStr0.reserve(charsToCopy + 1);
    for (int i = 0; i < charsToCopy && sqlStr[i] != 0; i++) {
      sqlStr0.push_back(sqlStr[i]);
    }
  }
  return converter.to_bytes(sqlStr0);
}

boost::optional< std::string > SqlWcharToOptString(const SQLWCHAR* sqlStr,
                                                    int32_t sqlStrLen,
                                                    bool isLenInBytes) {
  if (!sqlStr)
    return boost::none;

  return SqlWcharToString(sqlStr, sqlStrLen, isLenInBytes);
}

std::string SqlCharToString(const SQLCHAR* sqlStr, int32_t sqlStrLen) {
  std::string res;

  const char* sqlStrC = reinterpret_cast< const char* >(sqlStr);

  if (!sqlStr || !sqlStrLen)
    return res;

  if (sqlStrLen == SQL_NTS)
    res.assign(sqlStrC);
  else if (sqlStrLen > 0)
    res.assign(sqlStrC, sqlStrLen);

  return res;
}

std::string ToUtf8(const std::wstring& value) {
  return ToUtf8(value.c_str());
}

std::string ToUtf8(const wchar_t* value) {
  static std::wstring_convert< std::codecvt_utf8< wchar_t >, wchar_t >
      converter;
  return converter.to_bytes(value);
}

std::wstring FromUtf8(const std::string& value) {
  return FromUtf8(value.c_str());
}

std::wstring FromUtf8(const char* value) {
  static std::wstring_convert< std::codecvt_utf8< wchar_t >, wchar_t >
      converter;
  return converter.from_bytes(value);
}

void ReadByteArray(BinaryReaderImpl& reader, std::vector< int8_t >& res) {
  int32_t len = reader.ReadInt8Array(0, 0);

  if (len > 0) {
    res.resize(len);

    reader.ReadInt8Array(&res[0], static_cast< int32_t >(res.size()));
  } else
    res.clear();
}

std::vector< SQLWCHAR > ToWCHARVector(const std::string& value) {
  return ToWCHARVector(value.c_str());
}

std::vector< SQLWCHAR > ToWCHARVector(const char* value) {
  size_t wCharSize = sizeof(SQLWCHAR);
  size_t inBufferLenBytes = std::strlen(value);
  // Handle worst-case scenario where there is a one-to-one mapping.
  std::vector< SQLWCHAR > outBuffer(inBufferLenBytes + 1);
  bool isTruncated = false;
  size_t length = CopyUtf8StringToSqlWcharString(
      value, outBuffer.data(), outBuffer.size() * wCharSize, isTruncated);
  outBuffer.resize((length / wCharSize) + 1);
  return outBuffer;
}

std::string HexDump(const void* data, size_t count) {
  std::stringstream dump;
  size_t cnt = 0;
  for (const uint8_t *p = (const uint8_t*)data,
                     *e = (const uint8_t*)data + count;
       p != e; ++p) {
    if (cnt++ % 16 == 0) {
      dump << std::endl;
    }
    dump << std::hex << std::setfill('0') << std::setw(2) << (int)*p << " ";
  }
  return dump.str();
}
}  // namespace utility
}  // namespace odbc
}  // namespace ignite
