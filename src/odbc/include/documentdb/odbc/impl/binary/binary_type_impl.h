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

#ifndef _DOCUMENTDB_ODBC_IMPL_BINARY_BINARY_TYPE_IMPL
#define _DOCUMENTDB_ODBC_IMPL_BINARY_BINARY_TYPE_IMPL

#include <documentdb/odbc/common/utils.h>
#include <documentdb/odbc/documentdb_error.h>
#include <stdint.h>

#include <memory>

/**
 * Some SFINAE magic to check existence of the specified method with the
 * specified signature in the BinaryType<T> class.
 *
 * This macro declares checker for the method.
 */
#define DOCUMENTDB_DECLARE_BINARY_TYPE_METHOD_CHECKER(method, sign)     \
  template < typename T >                                               \
  class IsDeclaredBinaryType##method {                                  \
    typedef char one;                                                   \
    typedef char two[2];                                                \
                                                                        \
    template < class U, U >                                             \
    struct test;                                                        \
                                                                        \
    template < typename C >                                             \
    static one& helper(test< sign, &C::method >*);                      \
    template < typename C >                                             \
    static two& helper(...);                                            \
                                                                        \
   public:                                                              \
    const static bool value =                                           \
        (sizeof(helper< documentdb::odbc::binary::BinaryType< T > >(0)) \
         == sizeof(one));                                               \
  }

namespace documentdb {
namespace odbc {
namespace binary {
class BinaryReader;
class BinaryWriter;

template < typename T >
struct BinaryType;

template <>
struct DOCUMENTDB_IMPORT_EXPORT BinaryType< DocumentDbError > {
  static int32_t GetTypeId();

  static void GetTypeName(std::string& dst) {
    dst = "DocumentDbError";
  }

  static int32_t GetFieldId(const char* name);

  static bool IsNull(const DocumentDbError&) {
    return false;
  }

  static void GetNull(DocumentDbError& dst);

  static void Write(BinaryWriter& writer, const DocumentDbError& obj);

  static void Read(BinaryReader& reader, DocumentDbError& dst);
};
}  // namespace binary
}  // namespace odbc
namespace odbc {
namespace impl {
namespace binary {
/**
 * Write helper. Takes care of proper writing of pointers.
 */
template < typename T >
struct WriteHelper {
  template < typename W >
  static void Write(W& writer, const T& val) {
    writer.template WriteTopObject0< documentdb::odbc::binary::BinaryWriter >(val);
  }
};

/**
 * Specialization for the pointer case.
 */
template < typename T >
struct WriteHelper< T* > {
  template < typename W >
  static void Write(W& writer, const T* val) {
    if (!val)
      writer.WriteNull0();
    else
      writer.template WriteTopObject0< documentdb::odbc::binary::BinaryWriter >(
          *val);
  }
};

/**
 * Read helper. Takes care of proper reading of pointers.
 */
template < typename T >
struct ReadHelper {
  template < typename R >
  static T Read(R& reader) {
    T res;

    Read< R >(reader, res);

    return res;
  }

  template < typename R >
  static void Read(R& reader, T& val) {
    reader.template ReadTopObject0< documentdb::odbc::binary::BinaryReader, T >(
        val);
  }
};

/**
 * Specialization for the pointer case.
 */
template < typename T >
struct ReadHelper< T* > {
  template < typename R >
  static T* Read(R& reader) {
    if (reader.SkipIfNull())
      return 0;

    std::unique_ptr< T > res(new T());

    reader.template ReadTopObject0< documentdb::odbc::binary::BinaryReader, T >(
        *res);

    return res.release();
  }

  template < typename R >
  static void Read(R& reader, T*& ptr) {
    ptr = Read< R >(reader);
  }
};

DOCUMENTDB_DECLARE_BINARY_TYPE_METHOD_CHECKER(GetAffinityFieldName,
                                          void (*)(std::string&));

/**
 * This type is used to get affinity field name for binary types which have
 * not GetAffinityFieldName method defined.
 */
template < typename T >
struct AffinityFieldNameGetterDefault {
  static void Get(std::string& affField) {
    affField.clear();
  }
};

/**
 * This type is used to get affinity field name for binary types which have
 * GetAffinityFieldName method defined.
 */
template < typename T >
struct AffinityFieldNameGetterMethod {
  static void Get(std::string& affField) {
    documentdb::odbc::binary::BinaryType< T >::GetAffinityFieldName(affField);
  }
};

/**
 * This type is used to get affinity field name for pointers to binary types
 * which have GetAffinityFieldName method defined.
 */
template < typename T >
struct AffinityFieldNameGetterMethod< T* > {
  static void Get(std::string& affField) {
    documentdb::odbc::binary::BinaryType< T >::GetAffinityFieldName(affField);
  }
};

/**
 * Get affinity field name for the specified object.
 * Determines the best method to use based on user-defined methods.
 *
 * @param affField Affinity field name.
 * @return Affinity field name.
 */
template < typename T >
void GetAffinityFieldName(std::string& affField) {
  using namespace common;

  // Choosing right getter to use.
  typedef typename Conditional<
      // Checking if the BinaryType<T>::GetAffinityFieldName declared
      IsDeclaredBinaryTypeGetAffinityFieldName< T >::value,

      // True case. Using user-provided method.
      AffinityFieldNameGetterMethod< T >,

      // False case. Using default implementation.
      AffinityFieldNameGetterDefault< T >

      >::type AffinityFieldNameGetter;

  // Call itself.
  AffinityFieldNameGetter::Get(affField);
}
}  // namespace binary
}  // namespace impl
}  // namespace odbc
}  // namespace documentdb

#endif  //_DOCUMENTDB_ODBC_IMPL_BINARY_BINARY_TYPE_IMPL
