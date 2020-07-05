#include "../../core/rt/string_type.h"
#include "../../core/ext/botan/botan.h"
#include "test.h"

void rt::UnitTests::botan_hash()
{
	BYTE hash[20];

	_LOG("Input: \"hello world\"");

	sec::Hash<sec::HASH_MD5>().Calculate("hello world",11,hash);
	_LOG("MD5: "<<rt::tos::Binary<>(hash,sec::Hash<sec::HASH_MD5>::HASHSIZE));

	sec::Hash<sec::HASH_SHA1>().Calculate("hello world",11,hash);
	_LOG("SHA1: "<<rt::tos::Binary<>(hash,sec::Hash<sec::HASH_SHA1>::HASHSIZE));

	_LOGNL;

	_LOG("Input: \"GameWallpaperHD_2013-04_1366753EF3514652A80ECE50A9585524\"");

	sec::Hash<sec::HASH_SHA1>().Calculate("GameWallpaperHD_2013-04_1366753EF3514652A80ECE50A9585524",56,hash);
	_LOG("SHA1: "<<rt::tos::Binary<>(hash,sec::Hash<sec::HASH_SHA1>::HASHSIZE));
	_LOG("SHA1: "<<rt::tos::Binary<>(hash,16)<<" (first 16 bytes)");
}


void rt::UnitTests::botan_cipher()
{
  
/*
	{
		sec::BigNum a("0xf234567890abcdef1e");
		sec::BigNum b("-0x50000000000010a17f");
		sec::BigNum c("0x50000000000010a17f");

		
		_LOG(a);
		_LOG(b);

		sec::BigNum x;
		x.Add(a, b);		_LOG(x);
		x.Subtract(a, c);	_LOG(x);
		x.NegativeValue(a);	_LOG(x);

		sec::ECGFp  ec(sec::GFp_secp256k1);
		ec.DerivePublicKey(sec::BigNum("0x8cc5889e56694ef0ebb502f12bdf9f609fd03e3f9da2a9367de9823abeeb5a7c"));

			//0x74acc29cE7470010f7676d01F395756DAC982e60
			//0x8cc5889e56694ef0ebb502f12bdf9f609fd03e3f9da2a9367de9823abeeb5a7c
	}
*/
    LPCSTR text = "1234567890ABCDEFfedcba0987654321";
    
	{	sec::Cipher<sec::CIPHER_AES128>	cipher;
		cipher.SetKey("password!", 9);

		BYTE crypt[16];
		cipher.Encrypt(text, crypt, sizeof(crypt));

		_LOG("PlainText: "<<rt::String_Ref(text, sizeof(crypt)));
		_LOG("Encrypted: "<<rt::tos::Binary<>(crypt, sizeof(crypt)));

		char decrypt[sizeof(crypt)];
		cipher.Decrypt(crypt, decrypt, sizeof(crypt));

		_LOG("Decrypted: "<<rt::String_Ref(decrypt, sizeof(crypt)));
	}

	BYTE crypt[32];
	char decrypt[sizeof(crypt)];
	{	sec::Cipher<sec::CIPHER_AES256>	cipher;
		cipher.SetKey("password!", 9);

		
		cipher.Encrypt(text, crypt, sizeof(crypt));

		_LOG("PlainText: "<<rt::String_Ref(text, sizeof(crypt)));
		_LOG("Encrypted: "<<rt::tos::Binary<100>(crypt, sizeof(crypt)));
	}
	{	sec::Cipher<sec::CIPHER_AES256>	cipher;
		cipher.SetKey("password!", 9);
		
		cipher.Decrypt(crypt, decrypt, sizeof(crypt));

		_LOG("Decrypted: "<<rt::String_Ref(decrypt, sizeof(crypt)));
	}

    /*
	{	BYTE pp[16*3];	rt::Zero(pp);
		BYTE cp[16*3];
		sec::Cipher<sec::CIPHER_AES128>	cipher;
		cipher.SetKey("password!", 9);

		sec::_details::CipherInitVec<16> a(48);

		cipher.Encrypt(pp,cp,sizeof(pp));
		_LOG("Encrypted Unchained:");
		_LOG("    "<<rt::tos::Binary<>(&cp[ 0], 16));
		_LOG("    "<<rt::tos::Binary<>(&cp[16], 16));
		_LOG("    "<<rt::tos::Binary<>(&cp[32], 16));

		cipher.EncryptBlockChained(pp,cp,sizeof(pp),0);
		_LOG("Encrypted Chained:");
		_LOG("    "<<rt::tos::Binary<>(&cp[ 0], 16));
		_LOG("    "<<rt::tos::Binary<>(&cp[16], 16));
		_LOG("    "<<rt::tos::Binary<>(&cp[32], 16));
	}*/
}
