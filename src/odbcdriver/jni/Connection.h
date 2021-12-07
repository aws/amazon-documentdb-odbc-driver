#pragma once

#include <jni.h>
#include <string>
#include "JniEnv.h"
#include "ConnectionProperties.h"

namespace jni {
class Connection {
   private:
    JniEnv* const jni_env_;
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