#include <jni.h>
#include <string>

#include "../../../../../../../essentials.h"
#include "../../../../../../../core/rt/string_type.h"
#include "../../../../../../../core/ext/botan/botan.h"

void TestArmv8()
{
    rt::Buffer<BYTE>	data, out;
    data.SetSize(10*1024);
    data.RandomBits(4);
    out.SetSize(data.GetSize());

    for(int i=0; i<data.GetSize(); i++)
        data[i] = i;


    {
        rt::Buffer<BYTE>	plain;
        plain.SetSize(data.GetSize());
        sec::Cipher<sec::CIPHER_AES128> cipher;
        cipher.SetKey("123",3);
        cipher.Encrypt(data, out, data.GetSize());
        cipher.Decrypt(out, plain, data.GetSize());
        _LOGC("AES-128-ECB/10k: "<<(data == plain));
        out.RandomBits(4);
    }

    {
        rt::Buffer<BYTE>	plain;
        plain.SetSize(data.GetSize());
        sec::Cipher<sec::CIPHER_AES128> cipher;
        cipher.SetKey("123",3);
        cipher.EncryptBlockChained(data, out, data.GetSize(), 10);
        cipher.DecryptBlockChained(out, plain, data.GetSize(), 10);
        _LOGC("AES-128-CBC/10k: "<<(data == plain));
        out.RandomBits(4);
    }

    {
        rt::Buffer<BYTE>	plain;
        plain.SetSize(data.GetSize());
        sec::Cipher<sec::CIPHER_AES256> cipher;
        cipher.SetKey("123",3);
        cipher.Encrypt(data, out, data.GetSize());
        cipher.Decrypt(out, plain, data.GetSize());
        _LOGC("AES-256-ECB/10k: "<<(data == plain));
        out.RandomBits(4);
    }

    {
        rt::Buffer<BYTE>	plain;
        plain.SetSize(data.GetSize());
        sec::Cipher<sec::CIPHER_AES256> cipher;
        cipher.SetKey("123",3);
        cipher.EncryptBlockChained(data, out, data.GetSize(), 10);
        cipher.DecryptBlockChained(out, plain, data.GetSize(), 10);
        _LOGC("AES-256-CBC/10k: "<<(data == plain));
        out.RandomBits(4);
    }

    {
        BYTE h[64];
        sec::Hash<sec::HASH_SHA256> hash;
        hash.Calculate("123456",6,h);

        _LOG("SHA256: " << rt::tos::Binary<128>(h, sec::Hash<sec::HASH_SHA256>::HASHSIZE));
        _LOG("SHA256: " << rt::tos::Binary<128>(h, 16) << " (first 16 bytes)");
    }



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
