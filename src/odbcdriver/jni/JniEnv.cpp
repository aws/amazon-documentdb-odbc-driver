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

    options[0].optionString =
        (char *)"-Djava.class.path=./documentdb-jdbc-1.0.0-all.jar";  // where to find
                                                              // java .class
    options[0].extraInfo = nullptr;
    vm_args.version = JNI_VERSION_1_8;  // minimum Java version
    vm_args.nOptions = numOfOptions;    // number of options
    vm_args.options = options;
    vm_args.ignoreUnrecognized =
        false;  // invalid options make the JVM init fail

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