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
#include <string>
#include "JniEnv.h"
#include "ConnectionProperties.h"

namespace jni {
class Connection {
   private:
    JniEnv* jni_env_;
    ConnectionProperties connection_properties_;
    jobject connection_;

   public:
    inline Connection(JniEnv* const jni_env,
                      ConnectionProperties connection_properties)
        : jni_env_{jni_env},
          connection_properties_{connection_properties},
          connection_{nullptr} {
        jni_env_->CreateJavaVM();
    }

    void Connect();
    inline jobject GetHandle() {
        return connection_;
    }
    ~Connection();
};
}  // namespace jni
