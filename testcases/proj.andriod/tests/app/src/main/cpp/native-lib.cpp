#include <jni.h>
#include <string>

#include "../../../../../../../essentials.h"

extern "C" void TestMain();

extern "C" JNIEXPORT jstring JNICALL
Java_com_cpcf_tests_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";

    rt::String s = (
            J(info) = "Hello from CPCF"
            );

    _LOG(s);

    TestMain();

    return env->NewStringUTF(s);
}
