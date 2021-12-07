
#include <jni.h>
#include "Connection.h"
#include <iostream>
using namespace std;

namespace jni {

void Connection::Connect() {
    if (connection_ != nullptr) {
        return;
    }

    jni_env_->CreateJavaVM();
    JNIEnv* const env = jni_env_->GetJniEnv();
    static jclass cls_documentdb_connection = env->FindClass(
        "software/amazon/documentdb/jdbc/"
        "DocumentDbConnection");  // try to find the class
    if (cls_documentdb_connection == nullptr) {
        cerr << "ERROR: class not found !" << endl;
        throw runtime_error("Class not found.");
    }
    cout << "DocumentDbConnection class found." << endl;

    static jmethodID m_dcoumentdb_constructor = env->GetMethodID(
        cls_documentdb_connection, "<init>",
        "(Lsoftware/amazon/documentdb/jdbc/DocumentDbConnectionProperties;)V");
    if (m_dcoumentdb_constructor == nullptr) {
        throw runtime_error(
            "ERROR: constructor DocumentDbConnection(connectionProperties) not "
            "found!");
    }
    cout << "DocumentDbConnection constructor found." << endl;

    connection_ =
        env->NewObject(cls_documentdb_connection, m_dcoumentdb_constructor,
                       connection_properties_.GetHandle());
    if (connection_ == nullptr) {
        throw runtime_error("Unable to construct DocumentDbConnection");
    }
    cout << "Constructor created." << endl;
}

Connection::~Connection() {
    if (jni_env_ != nullptr && connection_ != nullptr) {
        JNIEnv* const env = jni_env_->GetJniEnv();
        static jclass cls_documentdb_connection = env->FindClass(
            "software/amazon/documentdb/jdbc/"
            "DocumentDbConnection");  // try to find the class
        if (cls_documentdb_connection == nullptr) {
            cerr << "ERROR: class DocumentDbConnection not found !" << endl;
            return;
        }
        cout << "DocumentDbConnection class found." << endl;

        static jmethodID m_dcoumentdb_close =
            env->GetMethodID(cls_documentdb_connection, "doClose", "()V");
        if (m_dcoumentdb_close == nullptr) {
            cerr << "ERROR: method DocumentDbConnection.doClose not found !"
                 << endl;
            return;
        }
        env->CallVoidMethod(connection_, m_dcoumentdb_close);
        connection_ = nullptr;
    }
}

}  // namespace jni