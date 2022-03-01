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

#ifndef _IGNITE_ODBC_IMPL_BINARY_BINARY_UTILS
#define _IGNITE_ODBC_IMPL_BINARY_BINARY_UTILS

#include <ignite/odbc/binary/binary_enum_entry.h>
#include <ignite/odbc/binary/binary_type.h>
#include <ignite/odbc/common/utils.h>
#include <ignite/odbc/date.h>
#include <ignite/odbc/guid.h>
#include <ignite/odbc/time.h>
#include <ignite/odbc/timestamp.h>
#include <stdint.h>

namespace ignite {
namespace odbc {
namespace impl {

namespace interop {
class InteropInputStream;
class InteropOutputStream;
class InteropMemory;
}  // namespace interop

namespace binary {
using namespace ignite::odbc::impl::interop;
using namespace ignite::odbc::binary;
/**
 * Binary uilts.
 */
class IGNITE_IMPORT_EXPORT BinaryUtils {
 public:
  /**
   * Get data hash code.
   *
   * @param data Data pointer.
   * @param size Data size in bytes.
   * @return Hash code.
   */
  static int32_t GetDataHashCode(const void* data, size_t size);

  /**
   * Utility method to read signed 8-bit integer from stream.
   *
   * @param stream Stream.
   * @return Value.
   */
  static int8_t ReadInt8(InteropInputStream* stream);

  /**
   * Utility method to read signed 8-bit integer from memory.
   * @throw IgniteError if there is not enough memory.
   *
   * @param mem Memory.
   * @param pos Position in memory.
   * @return Value.
   */
  static int8_t ReadInt8(InteropMemory& mem, int32_t pos);

  /**
   * Utility method to read signed 8-bit integer from memory.
   * @warning Does not check if there is enough data in memory to read.
   *
   * @param mem Memory.
   * @param pos Position in memory.
   * @return Value.
   */
  static int8_t UnsafeReadInt8(InteropMemory& mem, int32_t pos);

  /**
   * Utility method to write signed 8-bit integer to stream.
   *
   * @param stream Stream.
   * @param val Value.
   */
  static void WriteInt8(InteropOutputStream* stream, int8_t val);

  /**
   * Utility method to read signed 8-bit integer array from stream.
   *
   * @param stream Stream.
   * @param res Target array.
   * @param len Array length.
   */
  static void ReadInt8Array(InteropInputStream* stream, int8_t* res,
                            int32_t len);

  /**
   * Utility method to write signed 8-bit integer array to stream.
   *
   * @param stream Stream.
   * @param val Value.
   * @param len Array length.
   */
  static void WriteInt8Array(InteropOutputStream* stream,
                             const int8_t* val, int32_t len);

  /**
   * Utility method to read boolean from stream.
   *
   * @param stream Stream.
   * @return Value.
   */
  static bool ReadBool(InteropInputStream* stream);

  /**
   * Utility method to write bool to stream.
   *
   * @param stream Stream.
   * @param val Value.
   */
  static void WriteBool(InteropOutputStream* stream, bool val);

  /**
   * Utility method to read bool array from stream.
   *
   * @param stream Stream.
   * @param res Target array.
   * @param len Array length.
   */
  static void ReadBoolArray(InteropInputStream* stream, bool* res,
                            int32_t len);

  /**
   * Utility method to write bool array to stream.
   *
   * @param stream Stream.
   * @param val Value.
   * @param len Array length.
   */
  static void WriteBoolArray(InteropOutputStream* stream,
                             const bool* val, int32_t len);

  /**
   * Utility method to read signed 16-bit integer from stream.
   *
   * @param stream Stream.
   * @return Value.
   */
  static int16_t ReadInt16(InteropInputStream* stream);

  /**
   * Utility method to read signed 16-bit integer from memory.
   * @throw IgniteError if there is not enough memory.
   *
   * @param mem Memory.
   * @param pos Position in memory.
   * @return Value.
   */
  static int16_t ReadInt16(InteropMemory& mem, int32_t pos);

  /**
   * Utility method to read signed 16-bit integer from memory.
   * @warning Does not check if there is enough data in memory to read.
   *
   * @param mem Memory.
   * @param pos Position in memory.
   * @return Value.
   */
  static int16_t UnsafeReadInt16(InteropMemory& mem, int32_t pos);

  /**
   * Utility method to write signed 16-bit integer to stream.
   *
   * @param stream Stream.
   * @param val Value.
   */
  static void WriteInt16(InteropOutputStream* stream, int16_t val);

  /**
   * Utility method to read signed 16-bit integer array from stream.
   *
   * @param stream Stream.
   * @param res Target array.
   * @param len Array length.
   */
  static void ReadInt16Array(InteropInputStream* stream, int16_t* res,
                             int32_t len);

  /**
   * Utility method to write signed 16-bit integer array to stream.
   *
   * @param stream Stream.
   * @param val Value.
   * @param len Array length.
   */
  static void WriteInt16Array(InteropOutputStream* stream,
                              const int16_t* val, int32_t len);

  /**
   * Utility method to read unsigned 16-bit integer from stream.
   *
   * @param stream Stream.
   * @return Value.
   */
  static uint16_t ReadUInt16(InteropInputStream* stream);

  /**
   * Utility method to write unsigned 16-bit integer to stream.
   *
   * @param stream Stream.
   * @param val Value.
   */
  static void WriteUInt16(InteropOutputStream* stream, uint16_t val);

  /**
   * Utility method to read unsigned 16-bit integer array from stream.
   *
   * @param stream Stream.
   * @param res Target array.
   * @param len Array length.
   */
  static void ReadUInt16Array(InteropInputStream* stream,
                              uint16_t* res, int32_t len);

  /**
   * Utility method to write unsigned 16-bit integer array to stream.
   *
   * @param stream Stream.
   * @param val Value.
   * @param len Array length.
   */
  static void WriteUInt16Array(InteropOutputStream* stream,
                               const uint16_t* val, int32_t len);

  /**
   * Utility method to read signed 32-bit integer from stream.
   *
   * @param stream Stream.
   * @return Value.
   */
  static int32_t ReadInt32(InteropInputStream* stream);

  /**
   * Utility method to read signed 32-bit integer from memory.
   * @throw IgniteError if there is not enough memory.
   *
   * @param mem Memory.
   * @param pos Position in memory.
   * @return Value.
   */
  static int32_t ReadInt32(InteropMemory& mem, int32_t pos);

  /**
   * Utility method to read signed 32-bit integer from memory.
   * @warning Does not check if there is enough data in memory to read.
   *
   * @param mem Memory.
   * @param pos Position in memory.
   * @return Value.
   */
  static int32_t UnsafeReadInt32(InteropMemory& mem, int32_t pos);

  /**
   * Utility method to write signed 32-bit integer to stream.
   *
   * @param stream Stream.
   * @param val Value.
   */
  static void WriteInt32(InteropOutputStream* stream, int32_t val);

  /**
   * Utility method to read signed 32-bit integer array from stream.
   *
   * @param stream Stream.
   * @param res Target array.
   * @param len Array length.
   */
  static void ReadInt32Array(InteropInputStream* stream, int32_t* res,
                             int32_t len);

  /**
   * Utility method to write signed 32-bit integer array to stream.
   *
   * @param stream Stream.
   * @param val Value.
   * @param len Array length.
   */
  static void WriteInt32Array(InteropOutputStream* stream,
                              const int32_t* val, int32_t len);

  /**
   * Utility method to read signed 64-bit integer from stream.
   *
   * @param stream Stream.
   * @return Value.
   */
  static int64_t ReadInt64(InteropInputStream* stream);

  /**
   * Utility method to write signed 64-bit integer to stream.
   *
   * @param stream Stream.
   * @param val Value.
   */
  static void WriteInt64(InteropOutputStream* stream, int64_t val);

  /**
   * Utility method to read signed 64-bit integer array from stream.
   *
   * @param stream Stream.
   * @param res Target array.
   * @param len Array length.
   */
  static void ReadInt64Array(InteropInputStream* stream, int64_t* res,
                             int32_t len);

  /**
   * Utility method to write signed 64-bit integer array to stream.
   *
   * @param stream Stream.
   * @param val Value.
   * @param len Array length.
   */
  static void WriteInt64Array(InteropOutputStream* stream,
                              const int64_t* val, int32_t len);

  /**
   * Utility method to read float from stream.
   *
   * @param stream Stream.
   * @return Value.
   */
  static float ReadFloat(InteropInputStream* stream);

  /**
   * Utility method to write float to stream.
   *
   * @param stream Stream.
   * @param val Value.
   */
  static void WriteFloat(InteropOutputStream* stream, float val);

  /**
   * Utility method to read float array from stream.
   *
   * @param stream Stream.
   * @param res Target array.
   * @param len Array length.
   */
  static void ReadFloatArray(InteropInputStream* stream, float* res,
                             int32_t len);

  /**
   * Utility method to write float array to stream.
   *
   * @param stream Stream.
   * @param val Value.
   * @param len Array length.
   */
  static void WriteFloatArray(InteropOutputStream* stream,
                              const float* val, int32_t len);

  /**
   * Utility method to read double from stream.
   *
   * @param stream Stream.
   * @return Value.
   */
  static double ReadDouble(InteropInputStream* stream);

  /**
   * Utility method to write double to stream.
   *
   * @param stream Stream.
   * @param val Value.
   */
  static void WriteDouble(InteropOutputStream* stream, double val);

  /**
   * Utility method to read double array from stream.
   *
   * @param stream Stream.
   * @param res Target array.
   * @param len Array length.
   */
  static void ReadDoubleArray(InteropInputStream* stream, double* res,
                              int32_t len);

  /**
   * Utility method to write double array to stream.
   *
   * @param stream Stream.
   * @param val Value.
   * @param len Array length.
   */
  static void WriteDoubleArray(InteropOutputStream* stream,
                               const double* val, int32_t len);

  /**
   * Utility method to read Guid from stream.
   *
   * @param stream Stream.
   */
  static Guid ReadGuid(InteropInputStream* stream);

  /**
   * Utility method to write Guid to stream.
   *
   * @param stream Stream.
   * @param val Value.
   */
  static void WriteGuid(InteropOutputStream* stream, Guid val);

  /**
   * Utility method to read Date from stream.
   *
   * @param stream Stream.
   */
  static Date ReadDate(InteropInputStream* stream);

  /**
   * Utility method to write Date to stream.
   *
   * @param stream Stream.
   * @param val Value.
   */
  static void WriteDate(InteropOutputStream* stream, Date val);

  /**
   * Utility method to read Timestamp from stream.
   *
   * @param stream Stream.
   * @return Value.
   */
  static Timestamp ReadTimestamp(InteropInputStream* stream);

  /**
   * Utility method to write Timestamp to stream.
   *
   * @param stream Stream.
   * @param val Value.
   */
  static void WriteTimestamp(InteropOutputStream* stream,
                             Timestamp val);

  /**
   * Utility method to read Time from stream.
   *
   * @param stream Stream.
   * @return Value.
   */
  static Time ReadTime(InteropInputStream* stream);

  /**
   * Utility method to write Time to stream.
   *
   * @param stream Stream.
   * @param val Value.
   */
  static void WriteTime(InteropOutputStream* stream, Time val);

  /**
   * Utility method to read BinaryEnumEntry from stream.
   *
   * @param stream Stream.
   * @return Value.
   */
  static BinaryEnumEntry ReadBinaryEnumEntry(
      InteropInputStream* stream);

  /**
   * Utility method to write BinaryEnumEntry to stream.
   *
   * @param stream Stream.
   * @param val Value.
   */
  static void WriteBinaryEnumEntry(InteropOutputStream* stream,
                                   BinaryEnumEntry val) {
    WriteBinaryEnumEntry(stream, val.GetTypeId(), val.GetOrdinal());
  }

  /**
   * Utility method to write Binary Enum Entry to stream.
   *
   * @param stream Stream.
   * @param typeId Type ID.
   * @param ordinal Ordinal.
   */
  static void WriteBinaryEnumEntry(InteropOutputStream* stream,
                                   int32_t typeId, int32_t ordinal);

  /**
   * Utility method to write string to stream.
   *
   * @param stream Stream.
   * @param val Value.
   * @param len Length.
   */
  static void WriteString(InteropOutputStream* stream, const char* val,
                          int32_t len);

  /**
   * Get default value for the type.
   *
   * @return Null value for non primitive types and zeroes for primitives.
   */
  template < typename T >
  static T GetDefaultValue() {
    T res;

    BinaryType< T >::GetNull(res);

    return res;
  }
};

template <>
inline int8_t BinaryUtils::GetDefaultValue< int8_t >() {
  return 0;
}

template <>
inline int16_t BinaryUtils::GetDefaultValue< int16_t >() {
  return 0;
}

template <>
inline uint16_t BinaryUtils::GetDefaultValue< uint16_t >() {
  return 0;
}

template <>
inline int32_t BinaryUtils::GetDefaultValue< int32_t >() {
  return 0;
}

template <>
inline int64_t BinaryUtils::GetDefaultValue< int64_t >() {
  return 0;
}

template <>
inline bool BinaryUtils::GetDefaultValue< bool >() {
  return false;
}

template <>
inline float BinaryUtils::GetDefaultValue< float >() {
  return 0.0f;
}

template <>
inline double BinaryUtils::GetDefaultValue< double >() {
  return 0.0;
}

template <>
inline Guid BinaryUtils::GetDefaultValue< Guid >() {
  return Guid();
}

template <>
inline Date BinaryUtils::GetDefaultValue< Date >() {
  return Date();
}

template <>
inline Timestamp BinaryUtils::GetDefaultValue< Timestamp >() {
  return Timestamp();
}

template <>
inline Time BinaryUtils::GetDefaultValue< Time >() {
  return Time();
}

template <>
inline BinaryEnumEntry
BinaryUtils::GetDefaultValue< BinaryEnumEntry >() {
  return BinaryEnumEntry();
}

template <>
inline std::string BinaryUtils::GetDefaultValue< std::string >() {
  return std::string();
}
}  // namespace binary
}  // namespace impl
}  // namespace odbc
}  // namespace ignite

#endif  //_IGNITE_ODBC_IMPL_BINARY_BINARY_UTILS
