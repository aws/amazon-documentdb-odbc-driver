#pragma once

namespace jni {
#include <jni.h>

class JniEnv {
   private:
    JavaVM *jvm_ = nullptr;
    JNIEnv *env_ = nullptr;

   public:
    JniEnv();
    void CreateJavaVM();
    inline JNIEnv* GetJniEnv() {
        return env_;
    };
    inline ~JniEnv() {
        if (jvm_ != nullptr) {
            jvm_->DestroyJavaVM();
            jvm_ = nullptr;
        }
    }
};

}   // namespace jni
