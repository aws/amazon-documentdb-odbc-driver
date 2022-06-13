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
  if (!inBuffer)
    return 0;

  static std::wstring_convert< std::codecvt_utf8< wchar_t >, wchar_t >
      converter;
  std::wstring inString = converter.from_bytes(inBuffer);
  size_t inBufferLenChars = inString.size();
  size_t outBufferLenActual = std::min(
      inBufferLenChars, outBufferLenBytes ? (outBufferLenBytes - 1) : 0);

  if (!outBuffer)
    return inBufferLenChars;

  std::locale currentLocale("");
  const std::codecvt_utf8< char > convFacet;

  std::use_facet< std::ctype< wchar_t > >(currentLocale)
      .narrow(inString.data(), inString.data() + outBufferLenActual, '?',
              reinterpret_cast< char* >(outBuffer));
  if (outBufferLenBytes > 0) {
    outBuffer[outBufferLenActual] = 0;
  }

  return outBufferLenActual;
}

template< typename OutCharT >
size_t CopyUtf8StringToWcharString(const char* inBuffer, OutCharT* outBuffer,
                                      size_t outBufferLenBytes,
                                      bool& isTruncated) {
  size_t charSize = sizeof(SQLWCHAR);
  assert(sizeof(OutCharT) == charSize);

  size_t outBufferLenChars =
      outBufferLenBytes ? (outBufferLenBytes / charSize) - 1 : 0;
  // Does NOT include the null-terminating character.
  size_t inBufferLen = std::strlen(inBuffer);
  OutCharT* pOutBuffer;
  std::vector< OutCharT > targetProxy;

  if (outBuffer) {
    pOutBuffer = reinterpret_cast< OutCharT* >(outBuffer);
  } else {
    // Creates a proxy buffer so we can determine the required length.
    targetProxy.resize(inBufferLen + (1 * charSize));
    pOutBuffer = targetProxy.data();
    outBufferLenChars = inBufferLen;
  }

  // Setup conversion facet.
  typedef std::codecvt< OutCharT, char, std::mbstate_t > facet_type;
  const std::codecvt_utf8< OutCharT > convFacet;
  std::mbstate_t convState = std::mbstate_t();
  const char* pInBufferNext;
  OutCharT* pOutBufferNext;

  // translate characters:
  facet_type::result myresult =
      convFacet.in(convState, inBuffer, inBuffer + inBufferLen, pInBufferNext,
                   pOutBuffer, pOutBuffer + outBufferLenChars, pOutBufferNext);

  size_t lenConverted = 0;
  switch (myresult) {
    case facet_type::ok:
    case facet_type::partial:
      lenConverted = pOutBufferNext - pOutBuffer;
      // null-terminate target string, if room
      if (outBufferLenBytes >= charSize) {
        pOutBuffer[lenConverted] = 0;
      }
      isTruncated = (myresult == facet_type::partial);
      break;
    case facet_type::error:
      LOG_ERROR_MSG("Unable to convert character '" << *pInBufferNext << "'");
      lenConverted = pOutBufferNext - pOutBuffer;
      // null-terminate target string, if room
      if (outBufferLenBytes >= charSize) {
        pOutBuffer[lenConverted] = 0;
      }
      isTruncated = true;
      break;
    default:
      LOG_ERROR_MSG("Unexpected error converting string '" << inBuffer << "'");
      assert(false);
      break;
  }

  return lenConverted * charSize;
}

size_t CopyUtf8StringToSqlWcharString(const char* inBuffer, SQLWCHAR* outBuffer,
                                      size_t outBufferLenBytes,
                                      bool& isTruncated) {
  if (!inBuffer)
    return 0;

  size_t charSize = sizeof(SQLWCHAR);
  switch (charSize) {
    case 2:
      return CopyUtf8StringToWcharString(
          inBuffer, reinterpret_cast< char16_t* >(outBuffer), outBufferLenBytes,
          isTruncated);
      break;
    case 4:
      return CopyUtf8StringToWcharString(
          inBuffer, reinterpret_cast< char32_t* >(outBuffer), outBufferLenBytes,
          isTruncated);
      break;
    default:
      LOG_ERROR_MSG("Unexpected error converting string '" << inBuffer << "'");
      assert(false);
      return 0;
  }
}

size_t CopyStringToBuffer(const std::string& str, SQLWCHAR* buf, size_t buflen,
                          bool lenInBytes) {
  size_t charSize = sizeof(SQLWCHAR);

  assert(buflen >= 0 && (!lenInBytes || (buflen % charSize == 0)));

  size_t bufLenInBytes = lenInBytes ? buflen : buflen * charSize;
  bool isTruncated = false;
  size_t bytesWritten = CopyUtf8StringToSqlWcharString(
      str.c_str(), buf, bufLenInBytes, isTruncated);
  return lenInBytes ? bytesWritten : bytesWritten / charSize;
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

void WriteString(BinaryWriterImpl& writer,
                 const std::string& str) {
  writer.WriteString(str.data(), static_cast< int32_t >(str.size()));
}

void ReadDecimal(BinaryReaderImpl& reader,
                 Decimal& decimal) {
  int8_t hdr = reader.ReadInt8();

  assert(hdr == IGNITE_TYPE_DECIMAL);

  IGNITE_UNUSED(hdr);

  int32_t scale = reader.ReadInt32();

  int32_t len = reader.ReadInt32();

  std::vector< int8_t > mag;

  mag.resize(len);

  BinaryUtils::ReadInt8Array(
      reader.GetStream(), mag.data(), static_cast< int32_t >(mag.size()));

  int32_t sign = 1;

  if (mag[0] < 0) {
    mag[0] &= 0x7F;

    sign = -1;
  }

  Decimal res(mag.data(), static_cast< int32_t >(mag.size()),
                            scale, sign);

  decimal.Swap(res);
}

void WriteDecimal(BinaryWriterImpl& writer,
                  const Decimal& decimal) {
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

  BinaryUtils::WriteInt8Array(
      writer.GetStream(), magnitude.GetData(), magnitude.GetSize());
}

std::string SqlStringToString(const SQLWCHAR* sqlStr, int32_t sqlStrLen,
                              bool lenInBytes) {
  std::string result;
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
    size_t charsToCopy = lenInBytes ? (sqlStrLen / char_size) : sqlStrLen;
    sqlStr0.reserve(charsToCopy + 1);
    for (int i = 0; i < charsToCopy && sqlStr[i] != 0; i++) {
      sqlStr0.push_back(sqlStr[i]);
    }
  }
  return converter.to_bytes(sqlStr0);
}

boost::optional< std::string > SqlStringToOptString(const SQLWCHAR* sqlStr,
                                                    int32_t sqlStrLen,
                                                    bool lenInBytes) {
  if (!sqlStr)
    return boost::none;

  return SqlStringToString(sqlStr, sqlStrLen, lenInBytes);
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

void ReadByteArray(BinaryReaderImpl& reader,
                   std::vector< int8_t >& res) {
  int32_t len = reader.ReadInt8Array(0, 0);

  if (len > 0) {
    res.resize(len);

    reader.ReadInt8Array(&res[0], static_cast< int32_t >(res.size()));
  } else
    res.clear();
}

std::vector< SQLWCHAR > ToWCHARVector(const std::wstring& value) {
  return ToWCHARVector(value.c_str());
}

std::vector< SQLWCHAR > ToWCHARVector(const wchar_t* value) {
  std::vector< SQLWCHAR > result;
  for (int i = 0; value[i] != 0; i++) {
    result.push_back(value[i]);
  }
  result.push_back(0);
  return result;
}

std::vector< SQLWCHAR > ToWCHARVector(const std::string& value) {
  return ToWCHARVector(FromUtf8(value));
}

std::vector< SQLWCHAR > ToWCHARVector(const char* value) {
  return ToWCHARVector(FromUtf8(value));
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
