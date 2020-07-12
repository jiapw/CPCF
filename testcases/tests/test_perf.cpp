#include "../../essentials.h"
#include "../../core/ext/botan/botan.h"
#include "test.h"

struct vFun
{
	virtual void func(){}
};

struct non_vFun
{
	//__declspec(noinline) 
	void func(){}
};

template<typename T>
struct cb_t
{	static void func(LPVOID cookie)
	{	(*((T*)cookie))();
	}
};

void rt::UnitTests::callback_to_member_function()
{
	static const size_t t = 1000000000;

	vFun		vf;
	non_vFun	nvf;

	vFun*		pVF = &vf;

	// Call by cast to uni-class with function of same signature
	LPVOID		obj = &nvf;
	auto		funptr = &non_vFun::func;
	LPVOID		memfunc = (LPVOID&)funptr;
	struct CallWithSameFunctionSignature
	{	
		void func(){}
	};
	typedef	void (__thiscall CallWithSameFunctionSignature::* t_func)();

	// Call by callback function
	LPVOID cookie = &nvf;
	struct cb
	{	static void func(LPVOID cookie)
		{	((non_vFun*)cookie)->func();
		}
	};
	typedef	void (*t_callback_func)(LPVOID cookie);
	t_callback_func cbfunc = &cb::func;
	
	_LOG(sizeof(t_func));


	auto lambda = [&nvf]()
	{	nvf.func();
	};

	LPVOID cookie_t = &lambda;
	typedef cb_t<decltype(lambda)> cb_t_func;
	t_callback_func cbfunc_t = &cb_t_func::func;
	   	
	os::HighPerformanceCounter tm;

	tm.Restart();
	for(size_t i=0; i<t; i++)nvf.func();
	_LOG("Reference: "<<t*1000000/tm.TimeLapse()<<" cps");

	tm.Restart();
	for(size_t i=0; i<t; i++)pVF->func();
	_LOG("Virtual Function: "<<t*1000000/tm.TimeLapse()<<" cps");

	tm.Restart();
	for(size_t i=0; i<t; i++)
	{
		(((CallWithSameFunctionSignature*&)obj)->*(t_func&)memfunc)();
	}
	_LOG("Member Function Pointer: "<<t*1000000/tm.TimeLapse()<<" cps");

	tm.Restart();
	for(size_t i=0; i<t; i++)
	{
		cbfunc(cookie);
	}
	_LOG("Callback Function: "<<t*1000000/tm.TimeLapse()<<" cps");

	tm.Restart();
	for(size_t i=0; i<t; i++)
	{
		cbfunc_t(cookie_t);
	}
	_LOG("Callback Lambda Function: "<<t*1000000/tm.TimeLapse()<<" cps");

}

void rt::UnitTests::crypto_func()
{
	static const size_t t = 100000;

	rt::Buffer<BYTE>	data, out;
	data.SetSize(10*1024);
	data.RandomBits(4);
	out.SetSize(data.GetSize());

	{
		sec::Cipher<sec::CIPHER_AES128> cipher;
		cipher.SetKey("123",3);
		_LOG("ORG: "<<rt::tos::Base16OnStack<>(data, 48));
		cipher.EncryptBlockChained(data, out, 48, 1);
		_LOG("CIF: "<<rt::tos::Base16OnStack<>(out, 48));
		data.GetSub(0, 48).Zero();
		cipher.DecryptBlockChained(out, data, 48, 1);
		_LOG("DEC: "<<rt::tos::Base16OnStack<>(data, 48));
	}

	{
		sec::Cipher<sec::CIPHER_AES256> cipher;
		cipher.SetKey("123",3);
		_LOG("ORG: "<<rt::tos::Base16OnStack<>(data, 64));
		cipher.EncryptBlockChained(data, out, 64, 1);
		_LOG("CIF: "<<rt::tos::Base16OnStack<>(out, 64));
		data.GetSub(0, 64).Zero();
		cipher.DecryptBlockChained(out, data, 64, 1);
		_LOG("DEC: "<<rt::tos::Base16OnStack<>(data, 64));
	}

	{
		sec::Cipher<sec::CIPHER_AES128> cipher;
		cipher.SetKey("123",3);
		_LOG("ORG: "<<rt::tos::Base16OnStack<>(data, 64));
		cipher.Encrypt(data, out, 64);
		_LOG("CIF: "<<rt::tos::Base16OnStack<>(out, 64));
		data.GetSub(0, 64).Zero();
		cipher.Decrypt(out, data, 64);
		_LOG("DEC: "<<rt::tos::Base16OnStack<>(data, 64));
	}

	{
		sec::Cipher<sec::CIPHER_AES256> cipher;
		cipher.SetKey("123",3);
		_LOG("ORG: "<<rt::tos::Base16OnStack<>(data, 64));
		cipher.Encrypt(data, out, 64);
		_LOG("CIF: "<<rt::tos::Base16OnStack<>(out, 64));
		data.GetSub(0, 64).Zero();
		cipher.Decrypt(out, data, 64);
		_LOG("DEC: "<<rt::tos::Base16OnStack<>(data, 64));
	}

	{
		sec::Cipher<sec::CIPHER_AES256> cipher;
		cipher.SetKey("123",3);
		cipher.EncryptBlockChained(data, out, data.GetSize(), 10);
		cipher.DecryptBlockChained(out, data, data.GetSize(), 10);
		out.RandomBits(4);
		_LOG("AES/10k: "<<(data == out));
	}

	BYTE h[64];

	os::HighPerformanceCounter tm;

	{
		sec::Cipher<sec::CIPHER_AES128> cipher;
		cipher.SetKey("123",3);
		tm.Restart();
		for(size_t i=0; i<t; i++)
		{
			cipher.Encrypt(data, out, data.GetSize());
		}
		_LOGC("AES128-ECB/10k: "<<t*1000000/tm.TimeLapse()<<" kcps");
	}

	{
		sec::Cipher<sec::CIPHER_AES256> cipher;
		cipher.SetKey("123",3);
		tm.Restart();
		for(size_t i=0; i<t; i++)
		{
			cipher.Encrypt(data, out, data.GetSize());
		}
		_LOGC("AES256-ECB/10k: "<<t*1000000/tm.TimeLapse()<<" kcps");
	}

	tm.Restart();
	for(size_t i=0; i<t; i++)
	{
		sec::Hash<sec::HASH_SHA256>().Calculate(data, data.GetSize(), h);
	}
	_LOGC("HASH_SHA256/10k: "<<t*1000000/tm.TimeLapse()<<" kcps");

	tm.Restart();
	for(size_t i=0; i<t; i++)
	{
		sec::Hash<sec::HASH_SHA256>().Calculate(data, 64, h);
	}
	_LOGC("HASH_SHA256/64: "<<t*1000000/tm.TimeLapse()<<" kcps");

	tm.Restart();
	for(size_t i=0; i<t; i++)
	{
		sec::Hash<sec::HASH_CRC32>().Calculate(data, data.GetSize(), h);
	}
	_LOGC("HASH_CRC32/10k: "<<t*1000000/tm.TimeLapse()<<" kcps");

	tm.Restart();
	for(size_t i=0; i<t; i++)
	{
		sec::Hash<sec::HASH_CRC32>().Calculate(data, 8, h);
	}
	_LOGC("HASH_CRC32/8: "<<t*1000000/tm.TimeLapse()<<" kcps");

}
