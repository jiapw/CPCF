#pragma once
//////////////////////////////////////////////////////////////////////
// Cross-Platform Core Foundation (CPCF)
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of CPCF.  nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//////////////////////////////////////////////////////////////////////

#include "hash.h"

namespace sec
{
//////////////////////////////////////////////////////
// Symmetric Cryptography AES
enum _tagCipherMethod
{
	CIPHER_AES128 = 1,
	CIPHER_AES256
};

enum _tagCipherMode
{
	CIPHER_ECB = 0,
	CIPHER_CBC
};

namespace _details
{

template<UINT _METHOD>
struct	_AES_Traits;
	template<> struct _AES_Traits<CIPHER_AES128>
    {   static const int DataAlign = 128/8-1;
        static const int KEY_HASHER = HASH_MD5;
        static const int BlockSize = 128/8;
    };
	template<> struct _AES_Traits<CIPHER_AES256>
    {   static const int DataAlign = 128/8-1;
        static const int KEY_HASHER = HASH_SHA256;
        static const int BlockSize = 128/8;
    };

template<int _LEN>
struct CipherInitVec: public DataBlock<_LEN>
{    CipherInitVec(UINT random_seed){ Init(random_seed); }
     void Init(DWORD s){ return rt::Randomizer(s).Randomize(*this); }
};

} // namespace _details
} // namespace sec


#ifdef PLATFORM_INTEL_IPP_SUPPORT_disabled 
// TBD: IPP's AES256 produce inconsistent ciphertext that don't match with ones 
// from Botan and CommonCrypto, and also the IPP Rijndael functions are unexpected slow !!!

namespace sec
{
namespace _details
{

template<UINT _METHOD>
struct	_cipher_spec;
	template<> struct _cipher_spec<CIPHER_AES128>
	{	static const int ContextSize = 583;
	};
	template<> struct _cipher_spec<CIPHER_AES256>
	{	static const int ContextSize = 1031;
	};

} // namespace _details

template<UINT _METHOD>
class Cipher;

#define DEF_AES_CIPHER(_METHOD, MethodName) \
template<> class Cipher<_METHOD> \
{		protected: BYTE _Context[_details::_cipher_spec<_METHOD>::ContextSize]; \
public: static const UINT DataBlockSize = _details::_AES_Traits<_METHOD>::BlockSize; \
		static const UINT NativeKeySize = _details::_HashSize<_details::_AES_Traits<_METHOD>::KEY_HASHER>::size; \
	INLFUNC Cipher(){ int len=0; ASSERT(ippStsNoErr == IPPCALL(ipps##MethodName##GetSize)(&len) && len <= (int)sizeof(_Context)); } \
	INLFUNC ~Cipher(){ rt::Zero(_Context); } \
	INLFUNC static void ComputeKey(LPVOID key, LPCVOID data, UINT size){ Hash<_details::_AES_Traits<_METHOD>::KEY_HASHER>().Calculate(data, size, key); } \
	INLFUNC void SetKey(LPCVOID key, UINT len) \
	{	BYTE hash[NativeKeySize]; \
		if(len != NativeKeySize){ ComputeKey(hash, key, len); key = hash; } \
		IPPCALL(ipps##MethodName##Init)((LPCBYTE)key, (IppsRijndaelKeyLength)(sizeof(hash)*8), (Ipps##MethodName##Spec*)_Context); \
        if(len != NativeKeySize)rt::Zero(hash); \
	} \
	INLFUNC void Encrypt(LPCVOID pPlain, LPVOID pCrypt, UINT Len){ ASSERT((Len%DataBlockSize) == 0); IPPCALL(ipps##MethodName##EncryptECB)((LPCBYTE)pPlain,(LPBYTE)pCrypt,(int)Len,(Ipps##MethodName##Spec*)_Context,IppsCPPaddingPKCS7); } \
	INLFUNC void Decrypt(LPCVOID pCrypt, LPVOID pPlain, UINT Len){ ASSERT((Len%DataBlockSize) == 0); IPPCALL(ipps##MethodName##DecryptECB)((LPCBYTE)pCrypt,(LPBYTE)pPlain,(int)Len,(Ipps##MethodName##Spec*)_Context,IppsCPPaddingPKCS7); } \
	INLFUNC void EncryptBlockChained(LPCVOID pPlain, LPVOID pCrypt, UINT Len, UINT nonce) \
	{	_details::CipherInitVec<DataBlockSize> IV(nonce); \
		ASSERT((Len%DataBlockSize) == 0); \
		IPPCALL(ipps##MethodName##EncryptCBC)((LPCBYTE)pPlain,(LPBYTE)pCrypt,(int)Len,(Ipps##MethodName##Spec*)_Context,IV,IppsCPPaddingNONE); \
	} \
	INLFUNC void DecryptBlockChained(LPCVOID pCrypt, LPVOID pPlain, UINT Len, UINT nonce) \
	{	_details::CipherInitVec<DataBlockSize> IV(nonce); \
		ASSERT((Len%DataBlockSize) == 0); \
		IPPCALL(ipps##MethodName##DecryptCBC)((LPCBYTE)pCrypt,(LPBYTE)pPlain,(int)Len,(Ipps##MethodName##Spec*)_Context,IV,IppsCPPaddingNONE); \
	} \
};


DEF_AES_CIPHER(CIPHER_AES128, Rijndael128)
DEF_AES_CIPHER(CIPHER_AES256, Rijndael256)

#undef DEF_AES_CIPHER

} // namespace sec

#else

#if defined(PLATFORM_IOS)

#include <CommonCrypto/CommonCryptor.h>

namespace sec
{
template<UINT _METHOD>
class Cipher
{
public:
    static const UINT DataBlockSize = _details::_AES_Traits<_METHOD>::BlockSize;
    static const UINT NativeKeySize = _details::_HashSize<_details::_AES_Traits<_METHOD>::KEY_HASHER>::size;
protected:
    int             _CCRef_Op;   // kCCEncrypt or kCCDecrypt
    int             _CCRef_Opt;  // 0 or kCCOptionECBMode   // its default is CBC
    CCCryptorRef    _CCRef;
    BYTE            _Hash[NativeKeySize];
    void            _EnsureInit(int op, int opt, LPCVOID vi = nullptr)
                    {   if(_CCRef && op == _CCRef_Op && opt == _CCRef_Opt)
                        {   if(vi){ ASSERT(_CCRef_Opt == 0); CCCryptorReset(_CCRef, vi); }
                            return;
                        }
                        _CCRef_Op = op;     _CCRef_Opt = opt;
                        if(_CCRef)CCCryptorRelease(_CCRef);
                        CCCryptorCreate(_CCRef_Op, kCCAlgorithmAES, _CCRef_Opt, _Hash, NativeKeySize, vi, &_CCRef);
                    }
public:
    Cipher(){ _CCRef = nullptr; }
    ~Cipher(){ Empty(); }
    void            Empty(){ if(_CCRef){ CCCryptorRelease(_CCRef); _CCRef = nullptr; } rt::Zero(_Hash); }
    static void     ComputeKey(LPVOID key, LPCVOID data, UINT size){ Hash<_details::_AES_Traits<_METHOD>::KEY_HASHER>().Calculate(data, size, key); }
    void            SetKey(LPCVOID key, UINT len)
                    {   if(len != NativeKeySize){ ComputeKey(_Hash, key, len); }
                        else { memcpy(_Hash, key, len); }
                        _CCRef_Op = -1;  // force _EnsureInit re-create the _CCRef
                    }
    void            Encrypt(LPCVOID pPlain, LPVOID pCrypt, UINT Len)
                    {   _EnsureInit(kCCEncrypt, kCCOptionECBMode);
                        size_t out = 0;         ASSERT((Len&_details::_AES_Traits<_METHOD>::DataAlign) == 0);
                        CCCryptorUpdate(_CCRef, pPlain, Len, pCrypt, Len, &out);
                        ASSERT(out == Len);
                    }
    void            Decrypt(LPCVOID pCrypt, LPVOID pPlain, UINT Len)
                    {   _EnsureInit(kCCDecrypt, kCCOptionECBMode);
                        size_t out = 0;         ASSERT((Len&_details::_AES_Traits<_METHOD>::DataAlign) == 0);
                        CCCryptorUpdate(_CCRef, pCrypt, Len, pPlain, Len, &out);
                        ASSERT(out == Len);
                    }
    void            EncryptBlockChained(LPCVOID pPlain, LPVOID pCrypt, UINT Len, UINT nonce)
                    {   _details::CipherInitVec<DataBlockSize> IV(nonce);
                        _EnsureInit(kCCEncrypt, 0, &IV);
                        size_t out = 0;         ASSERT((Len&_details::_AES_Traits<_METHOD>::DataAlign) == 0);
                        CCCryptorUpdate(_CCRef, pPlain, Len, pCrypt, Len, &out);
                        if(out<Len)
                        {   size_t fin = 0;
                            CCCryptorFinal(_CCRef, ((LPBYTE)pCrypt) + out, Len - out, &fin);
                            out += fin;
                        }
                        ASSERT(out == Len);
                    }
    void            DecryptBlockChained(LPCVOID pCrypt, LPVOID pPlain, UINT Len, UINT nonce)
                    {   _details::CipherInitVec<DataBlockSize> IV(nonce);
                        _EnsureInit(kCCDecrypt, 0, &IV);
                        size_t out = 0;         ASSERT((Len&_details::_AES_Traits<_METHOD>::DataAlign) == 0);
                        CCCryptorUpdate(_CCRef, pCrypt, Len, pPlain, Len, &out);
                        if(out<Len)
                        {   size_t fin = 0;
                            CCCryptorFinal(_CCRef, ((LPBYTE)pCrypt) + out, Len - out, &fin);
                            out += fin;
                        }
                        ASSERT(out == Len);
                    }
};
} // namespace sec

#else

namespace sec
{
namespace _details
{
template<UINT _METHOD>
struct	_cipher_spec;
	template<> struct _cipher_spec<CIPHER_AES128>
	{	typedef Botan::AES_128		Cipher;
	};
	template<> struct _cipher_spec<CIPHER_AES256>
	{	typedef Botan::AES_256		Cipher;
	};
} // namespace _details

template<UINT _METHOD>
class Cipher
{
	typename _details::_cipher_spec<_METHOD>::Cipher	_Cipher;
public:
    static const UINT DataBlockSize = _details::_AES_Traits<_METHOD>::BlockSize;
    static const UINT NativeKeySize = _details::_HashSize<_details::_AES_Traits<_METHOD>::KEY_HASHER>::size;
	static void     ComputeKey(LPVOID key, LPCVOID data, UINT size){ Hash<_details::_AES_Traits<_METHOD>::KEY_HASHER>().Calculate(data, size, key); }
	void            SetKey(LPCVOID key, UINT len)
                    {   BYTE hash[NativeKeySize];
                        if(len != NativeKeySize){ ComputeKey(hash, key, len); key = hash; }
                        _Cipher.set_key((LPCBYTE)key, NativeKeySize);
                        if(len != NativeKeySize)rt::Zero(hash);
                    }
    void            Encrypt(LPCVOID pPlain, LPVOID pCrypt, UINT Len)
                    {	ASSERT((Len%DataBlockSize) == 0);
						_Cipher.encrypt_n((LPCBYTE)pPlain, (LPBYTE)pCrypt, Len/DataBlockSize);
                    }
    void            Decrypt(LPCVOID pCrypt, LPVOID pPlain, UINT Len)
                    {	ASSERT((Len%DataBlockSize) == 0);
						_Cipher.decrypt_n((LPCBYTE)pCrypt, (LPBYTE)pPlain, Len/DataBlockSize);
                    }
    void            EncryptBlockChained(LPCVOID pPlain, LPVOID pCrypt, UINT Len, UINT nonce)
                    {   _details::CipherInitVec<DataBlockSize> IV(nonce);
                        ASSERT((Len%DataBlockSize) == 0);
                        auto* p = (const DataBlock<DataBlockSize>*)pPlain;
                        auto* c = (DataBlock<DataBlockSize>*)pCrypt;
                        Len /= DataBlockSize;
                        for(UINT i=0; i<Len; i++, p++, c++)
                        {    IV ^= *p;
                            Encrypt(&IV, c, DataBlockSize);
                            c->CopyTo(IV);
                        }
                    }
    void            DecryptBlockChained(LPCVOID pCrypt, LPVOID pPlain, UINT Len, UINT nonce)
                    {   _details::CipherInitVec<DataBlockSize> IV(nonce);
                        ASSERT((Len%DataBlockSize) == 0);
                        const DataBlock<DataBlockSize>* iv = &IV;
                        auto* p = (DataBlock<DataBlockSize>*)pPlain;
                        auto* c = (const DataBlock<DataBlockSize>*)pCrypt;
                        Len /= DataBlockSize;
                        for(UINT i=0; i<Len; i++, p++, c++)
                        {   Decrypt(c, p, DataBlockSize);
                            *p ^= *iv;
                            iv = c;
                        }
                    }
};
} // namespace sec
#endif // #ifdef defined(PLATFORM_IOS)
#endif // #ifdef PLATFORM_INTEL_IPP_SUPPORT

