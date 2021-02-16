#include <jni.h>
#include <string>

#include "../../../../../../../essentials.h"
#include "../../../../../../../core/ext/botan/botan.h"
#include "crypt.h"


void aes_v8_encrypt_blocks(void* in, void* out, int n, AES_KEY* key)
{
    for (int i=0;i<n;i++)
    {
        aes_v8_encrypt( ((uint8_t*)in)+i*16, ((uint8_t*)out)+i*16, key);
    }
}

extern unsigned char *SHA256(const unsigned char *d, size_t n, unsigned char *md);
void TestArmv8()
{
    _LOG("deso arm64v8 crypt start");

    _LOG("deso arm64v8 #1");

    unsigned char uk[32];
    sec::Hash<sec::HASH_SHA256> sha256;
    sha256.Calculate("123", 3, uk);

    AES_KEY aes_key;
    int r = aes_v8_set_encrypt_key(uk, 256, &aes_key);


    rt::Buffer<BYTE>	data, out;
    data.SetSize(10*1024);
    data.RandomBits(4);
    out.SetSize(data.GetSize());

    for(int i=0; i<data.GetSize(); i++)
        data[i] = i;

    aes_v8_encrypt_blocks(data, out, data.GetSize()/16, &aes_key);

    _LOG("deso aes256-ecb : " << int(out[0])<<"," << int(out[1])<<","<< int(out[2])<<","<< int(out[3]));

    _LOG("deso arm64v8 #2");
    int64_t t =100000;
    os::HighPerformanceCounter tm;
    tm.Restart();
    for (int i=0;i<t;i++)
    {
        aes_v8_encrypt_blocks(data, out, data.GetSize()/16, &aes_key);
    }
    _LOGC("deso AES256-ECB/10k: "<<t*1000000/tm.TimeLapse()<<" kcps");


    _LOG("deso arm64v8 #3");
    SHA256(data, data.GetSize(), (unsigned char*)out);
    _LOG("deso sha256 : " << int(out[0])<<"," << int(out[1])<<","<< int(out[2])<<","<< int(out[3]));

    _LOG("deso arm64v8 #4");
    tm.Restart();
    for (int i=0;i<t;i++)
    {
        SHA256(data, data.GetSize(), (unsigned char*)out);
    }
    _LOGC("deso SHA256/10k: "<<t*1000000/tm.TimeLapse()<<" kcps");

    _LOG("deso arm64v8 #5");
    tm.Restart();
    for (int i=0;i<t;i++)
    {
        SHA256(data, 64, (unsigned char*)out);
    }
    _LOGC("deso SHA256/64: "<<t*1000000/tm.TimeLapse()<<" kcps");

    _LOG("deso arm64v8 crypt end");
}


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

    TestArmv8();
    //TestMain();

    //return env->NewStringUTF(s.GetString());
    return env->NewStringUTF(hello.c_str());
}
