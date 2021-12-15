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

#include <chrono>
#include <ctime>
#include <iostream>
#include <ratio>
#include <jni.h>

#include "JniEnv.h"

using namespace std;
using namespace chrono;

namespace jni {
JniEnv::JniEnv() {
}
void JniEnv::CreateJavaVM() {
    if (jvm_ != nullptr && env_ != nullptr) {
        return;
    }

    int numOfOptions = 1;
    JavaVMInitArgs vm_args = JavaVMInitArgs();  // Initialization arguments
    JavaVMOption *options =
        new JavaVMOption[numOfOptions];  // JVM invocation options

    // where to find java .class
    options[0].optionString =
        (char *)"-Djava.class.path=./documentdb-jdbc-1.0.0-all.jar";
    options[0].extraInfo = nullptr;
    // minimum Java version
    vm_args.version = JNI_VERSION_1_8;
    // number of options
    vm_args.nOptions = numOfOptions;
    vm_args.options = options;
    // invalid options make the JVM init fail
    vm_args.ignoreUnrecognized = false;

    //=============== load and initialize
    // Java VM and JNI interface =============
    jint rc = JNI_CreateJavaVM(&jvm_, (void **)&env_, &vm_args);  // YES !!
    delete[] options;  // we then no longer need the initialisation options.
    if (rc != JNI_OK) {
        // TO DO: error processing...
        throw std::runtime_error("Unable to create Java VM.");
    }
}

}  // namespace jni
