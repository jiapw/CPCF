#include <jni.h>
#include <string>

#include "../../../../../../../essentials.h"
#include "crypt.h"


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

    _LOG("deso arm64v8 crypt start");

    unsigned char uk[256];
    AES_KEY aes_key;
    unsigned char uk2[256];
    int r = aes_v8_set_encrypt_key(uk, 256, &aes_key);



    unsigned char buf_in[1024*10];
    unsigned char buf_out[1024*10];

    //aes_v8_encrypt(buf_in, buf_out, &aes_key);


    for (int i=0;i<100000;i++)
    {
        for (int j=0;j<sizeof(buf_in)/16;j++)
        {
            aes_v8_encrypt(buf_in, buf_out, &aes_key);
        }
    }



    _LOG("deso arm64v8 crypt end");

    //TestMain();

    //return env->NewStringUTF(s.GetString());
    return env->NewStringUTF(hello.c_str());
}
