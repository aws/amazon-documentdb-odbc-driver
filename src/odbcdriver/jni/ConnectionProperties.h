#pragma once

#include <jni.h>
#include <string>
#include "JniEnv.h"

namespace jni {
class ConnectionProperties {
   private:
    jobject connection_properties_;

    inline ConnectionProperties(jobject connection_properties)
        : connection_properties_{connection_properties} {
    }

    static jmethodID GetMethodGetPropertiesFromConnectionString(
        JNIEnv* const env, const jclass connection_properties);

   public:
    static ConnectionProperties* GetPropertiesFromConnectionString(
        JniEnv* const jni_env, const std::string connectionString);

    inline jobject GetHandle() {
        return connection_properties_;
    };
};

}  // namespace jni