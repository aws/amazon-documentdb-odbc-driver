
#include <jni.h>
#include <iostream>
#include "ConnectionProperties.h"
using namespace std;

namespace jni {

ConnectionProperties* ConnectionProperties::GetPropertiesFromConnectionString(
    JniEnv* const jni_env_, const std::string connectionString) {
    jni_env_->CreateJavaVM();
    JNIEnv* const env = jni_env_->GetJniEnv();
    static jclass cls_exception = env->FindClass("java/lang/Exception");
    static jmethodID m_exception_get_message =
        env->GetMethodID(cls_exception, "getMessage", "()Ljava/lang/String;");

    static jclass clsConnectionProperties = env->FindClass(
        "software/amazon/documentdb/jdbc/"
        "DocumentDbConnectionProperties");  // try to find the class
    if (clsConnectionProperties == nullptr) {
        cerr << "ERROR: class not found !" << endl;
        throw runtime_error("Class not found.");
    }
    // if class found, continue
    cout << "Class DocumentDbConnectionProperties found" << endl;

    jmethodID mGetPropertiesFromConnectionString =
        GetMethodGetPropertiesFromConnectionString(env,
                                                   clsConnectionProperties);
    if (mGetPropertiesFromConnectionString == nullptr) {
        cerr << "ERROR: method void mymain() not found !" << endl;
        throw runtime_error("Method not found.");
    }

    jstring jstr = env->NewStringUTF(connectionString.c_str());
    jobject connection_properties = env->CallStaticObjectMethod(
        clsConnectionProperties, mGetPropertiesFromConnectionString,
        jstr);  // call method
    if (env->ExceptionCheck()) {
        jstring o_message = (jstring)env->CallObjectMethod(
            env->ExceptionOccurred(), m_exception_get_message);
        jboolean isCopy;
        const char* message = env->GetStringUTFChars(o_message, &isCopy);
        env->ReleaseStringUTFChars(o_message, message);
        throw runtime_error(message);
        return nullptr;
    }

    return new ConnectionProperties(connection_properties);
}

jmethodID ConnectionProperties::GetMethodGetPropertiesFromConnectionString(
    JNIEnv* const env, const jclass clsConnectionProperties) {
    // find method
    static jmethodID mGetPropertiesFromConnectionString =
        env->GetStaticMethodID(clsConnectionProperties,
                               "getPropertiesFromConnectionString",
                               "(Ljava/lang/String;)"
                               "Lsoftware/amazon/documentdb/jdbc/"
                               "DocumentDbConnectionProperties;");
    if (mGetPropertiesFromConnectionString == nullptr) {
        cerr << "ERROR: method void getPropertiesFromConnectionString() not "
                "found !"
             << endl;
        return nullptr;
    } else {
        return mGetPropertiesFromConnectionString;
    }
}

}  // namespace jni