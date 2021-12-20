/*
 * Copyright <2021> Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 */

#pragma once

#include <jni.h>
#include <memory>
#include <string>
#include "JniEnv.h"

namespace jni {
class ConnectionProperties {
   private:
    jobject connection_properties_;

    static jmethodID GetMethodGetPropertiesFromConnectionString(
        JNIEnv* const env, const jclass connection_properties);

   public:
    static std::shared_ptr< ConnectionProperties > GetPropertiesFromConnectionString(
        JniEnv* const jni_env, const std::string connectionString);

    inline ConnectionProperties(jobject connection_properties)
        : connection_properties_{connection_properties} {};

    inline jobject GetHandle() {
        return connection_properties_;
    };
};

}  // namespace jni
