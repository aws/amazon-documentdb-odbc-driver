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
