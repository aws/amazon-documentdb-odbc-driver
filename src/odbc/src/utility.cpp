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

namespace ignite {
namespace odbc {
namespace utility {
using namespace odbc::impl::binary;
using namespace odbc::common;

size_t CopyStringToBuffer(const std::string& str, wchar_t* buf, size_t buflen) {
  static std::wstring_convert< std::codecvt_utf8< wchar_t >, wchar_t >
      converter;
  size_t char_size = sizeof(wchar_t);

  if (buflen < 0 || buflen % char_size != 0)
    return 0;

  std::wstring str0;
  str0 = converter.from_bytes(str);

  size_t charsToCopy =
      std::min(str0.size(), ((buflen - char_size) / char_size));
  if (buf && charsToCopy > 0) {
    std::wcsncpy(buf, str0.data(), charsToCopy);
  }
  if (buf && buflen >= char_size) {
    buf[charsToCopy] = 0;
  }

  size_t bytesRequired = str0.size() * char_size;
  size_t bytesToCopy = std::min(bytesRequired, (buflen - char_size));
  return (buflen > 0) ? bytesToCopy : bytesRequired;
}

size_t CopyStringToBuffer(const std::string& str, unsigned short* buf,
                          size_t buflen) {
  static std::wstring_convert< std::codecvt_utf8< wchar_t >, wchar_t >
      converter;
  size_t char_size = sizeof(unsigned short);

  if (buflen < 0 || buflen % char_size != 0)
    return 0;

  std::wstring str0;
  str0 = converter.from_bytes(str);

  size_t charsToCopy =
      std::min(str0.size(), ((buflen - char_size) / char_size));
  if (buf && charsToCopy > 0) {
    const wchar_t* data = str0.data();
    for (int i = 0; i < charsToCopy; i++) {
      buf[i] = data[i];
    }
  }
  if (buf && buflen >= char_size) {
    buf[charsToCopy] = 0;
  }

  size_t bytesRequired = str0.size() * char_size;
  size_t bytesToCopy = std::min(bytesRequired, (buflen - char_size));
  return (buflen > 0) ? bytesToCopy : bytesRequired;
}

size_t CopyStringToBuffer(const std::string& str, char* buf, size_t buflen) {
  static std::wstring_convert< std::codecvt_utf8< wchar_t >, wchar_t >
      converter;
  size_t char_size = sizeof(char);

  if (buflen < 0 || buflen % char_size != 0)
    return 0;

  std::wstring str0;
  str0 = converter.from_bytes(str);

  size_t charsToCopy =
      std::min(str0.size(), ((buflen - char_size) / char_size));
  if (buf && charsToCopy > 0) {
    std::locale currentLocale("");
    std::use_facet< std::ctype< wchar_t > >(currentLocale)
        .narrow(str0.data(), str0.data() + charsToCopy, '?', buf);
  }
  if (buf && buflen >= char_size) {
    buf[charsToCopy] = 0;
  }

  size_t bytesRequired = str0.size() * char_size;
  size_t bytesToCopy = std::min(bytesRequired, (buflen - char_size));
  return (buflen > 0) ? bytesToCopy : bytesRequired;
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

std::string SqlStringToString(const unsigned char* sqlStr, int32_t sqlStrLen,
                              size_t char_size) {
  size_t wchar_t_size = sizeof(wchar_t);
 
  std::string res;

  if (char_size == wchar_t_size) {
    static std::wstring_convert< std::codecvt_utf8< wchar_t >, wchar_t >
        converter;
    if (sqlStrLen == SQL_NTS) {
      res =converter.to_bytes(reinterpret_cast< const wchar_t* >(sqlStr));
    } else if (sqlStr > 0) {
      res = converter.to_bytes(reinterpret_cast< const wchar_t* >(sqlStr),
                         reinterpret_cast< const wchar_t* >(sqlStr + sqlStrLen));
    }
  } else if (char_size == 2) {
    static std::wstring_convert< std::codecvt_utf8< char16_t >, char16_t >
        converter_utf16;
    if (sqlStrLen == SQL_NTS) {
      res = converter_utf16.to_bytes(reinterpret_cast< const char16_t* >(sqlStr));
    } else if (sqlStr > 0) {
      res = converter_utf16.to_bytes(
          reinterpret_cast< const char16_t* >(sqlStr),
          reinterpret_cast< const char16_t* >(sqlStr + sqlStrLen));
    }
  } else if (char_size == 4) {
    static std::wstring_convert< std::codecvt_utf8< char32_t >, char32_t >
        converter_utf32;
    if (sqlStrLen == SQL_NTS) {
      res =
          converter_utf32.to_bytes(reinterpret_cast< const char32_t* >(sqlStr));
    } else if (sqlStr > 0) {
      res = converter_utf32.to_bytes(
          reinterpret_cast< const char32_t* >(sqlStr),
          reinterpret_cast< const char32_t* >(sqlStr + sqlStrLen));
    }
  }

  return res;
}

boost::optional< std::string > SqlStringToOptString(const unsigned char* sqlStr,
                                                    int32_t sqlStrLen,
                                                    size_t char_size) {
  if (!sqlStr)
    return boost::none;

  return SqlStringToString(sqlStr, sqlStrLen, char_size);
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
