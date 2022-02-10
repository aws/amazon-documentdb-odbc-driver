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

#ifndef _IGNITE_ODBC_JNI_RESULT_SET
#define _IGNITE_ODBC_JNI_RESULT_SET

#include <string>
#include <ignite/odbc/common/concurrent.h>
#include <ignite/odbc/jni/java.h>

using ignite::odbc::common::concurrent::SharedPointer;
using ignite::odbc::jni::java::GlobalJObject;
using ignite::odbc::jni::java::JniContext;
using ignite::odbc::jni::java::JniErrorInfo;

namespace ignite {
namespace odbc {
namespace jni {
class ResultSet {
   public:
    ResultSet(SharedPointer< JniContext > jniContext,
              SharedPointer< GlobalJObject > resultSet);
    ~ResultSet();

    bool Next(bool& hasNext, JniErrorInfo& errInfo);
    bool GetString(const int columnIndex, std::string& value, bool& wasNull,
                   JniErrorInfo& errInfo);
    bool GetString(const std::string& columnName, std::string& value, bool& wasNull,
                   JniErrorInfo& errInfo);
    bool GetInt(const int columnIndex, int& value, bool& wasNull,
                   JniErrorInfo& errInfo);
    bool GetInt(const std::string& columnName, int& value,
                   bool& wasNull, JniErrorInfo& errInfo);

   private:
    SharedPointer< JniContext > _jniContext;
    SharedPointer< GlobalJObject > _resultSet;
};
}
}  // namespace odbc
}  // namespace ignite

#endif // _IGNITE_ODBC_JNI_RESULT_SET
