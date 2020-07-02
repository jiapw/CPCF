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

namespace _details
{

template<UINT _METHOD>
struct	_AES_DataAlign;
	template<> struct _AES_DataAlign<CIPHER_AES128>{ static const int Result = 128/8-1; };
	template<> struct _AES_DataAlign<CIPHER_AES256>{ static const int Result = 256/8-1; };

} // namespace _details
} // namespace sec



#ifdef PLATFORM_INTEL_IPP_SUPPORT

namespace sec
{
namespace _details
{

template<UINT _METHOD>
struct	_cipher_spec;
	template<> struct _cipher_spec<CIPHER_AES128>
	{	static const int ContextSize = 583;
		static const int KEY_HASHER = HASH_MD5;
		static const int BlockSize = 128/8;
	};		
	template<> struct _cipher_spec<CIPHER_AES256>
	{	static const int ContextSize = 1031;
		static const int KEY_HASHER = HASH_SHA256; 
		static const int BlockSize = 256/8;
	};

} // namespace _details
} // namespace sec


namespace sec
{

namespace _details
{	template<int _LEN>
	struct CipherInitVec
	{	DWORD _Vec[_LEN/4];
		CipherInitVec(UINT random_seed){ Init(random_seed); }
		DWORD Init(DWORD s){ return ~(_Vec[_LEN/4-1] = s*((CipherInitVec<_LEN-4>&)*this).Init(s)); }
		operator LPCBYTE() const { return (LPCBYTE)_Vec; }
	};
	template<> struct CipherInitVec<0>
	{	DWORD Init(DWORD s){ return s; }
	};
};

template<UINT _METHOD = CIPHER_AES128>
class Cipher;

#define DEF_AES_CIPHER(_METHOD, MethodName) \
template<> class Cipher<_METHOD> \
{	protected: BYTE _Context[_details::_cipher_spec<_METHOD>::ContextSize]; \
public: static const UINT DataBlockSize = _details::_cipher_spec<_METHOD>::BlockSize; \
		static const UINT NativeKeySize = _details::_HashSize< _details::_cipher_spec<_METHOD>::KEY_HASHER>::size; \
	INLFUNC Cipher(){ int len=0; ASSERT(ippStsNoErr == IPPCALL(ipps##MethodName##GetSize)(&len) && len <= (int)sizeof(_Context)); } \
	INLFUNC ~Cipher(){ rt::Zero(_Context); } \
	INLFUNC static void ComputeKey(LPVOID key, LPCVOID data, UINT size){ Hash<_details::_cipher_spec<_METHOD>::KEY_HASHER>().Calculate(data, size, key); } \
	INLFUNC void SetKey(LPCVOID key, UINT len) \
	{	BYTE hash[NativeKeySize]; \
		if(len != NativeKeySize){ ComputeKey(hash, key, len); key = hash; } \
		IPPCALL(ipps##MethodName##Init)((LPCBYTE)key, (IppsRijndaelKeyLength)(sizeof(hash)*8), (Ipps##MethodName##Spec*)_Context); \
	} \
	INLFUNC void Encrypt(LPCVOID pPlain, LPVOID pCrypt, UINT Len){ IPPCALL(ipps##MethodName##EncryptECB)((LPCBYTE)pPlain,(LPBYTE)pCrypt,(int)Len,(Ipps##MethodName##Spec*)_Context,IppsCPPaddingNONE); } \
	INLFUNC void Decrypt(LPCVOID pCrypt, LPVOID pPlain, UINT Len){ IPPCALL(ipps##MethodName##DecryptECB)((LPCBYTE)pCrypt,(LPBYTE)pPlain,(int)Len,(Ipps##MethodName##Spec*)_Context,IppsCPPaddingNONE); } \
	INLFUNC void EncryptBlockChained(LPCVOID pPlain, LPVOID pCrypt, UINT Len, UINT nonce) \
	{	_details::CipherInitVec<DataBlockSize> IV(nonce); \
		IPPCALL(ipps##MethodName##EncryptCBC)((LPCBYTE)pPlain,(LPBYTE)pCrypt,(int)Len,(Ipps##MethodName##Spec*)_Context,IV,IppsCPPaddingNONE); \
	} \
	INLFUNC void DecryptBlockChained(LPCVOID pCrypt, LPVOID pPlain, UINT Len, UINT nonce) \
	{	_details::CipherInitVec<DataBlockSize> IV(nonce); \
		IPPCALL(ipps##MethodName##DecryptCBC)((LPCBYTE)pCrypt,(LPBYTE)pPlain,(int)Len,(Ipps##MethodName##Spec*)_Context,IV,IppsCPPaddingNONE); \
	} \
};


DEF_AES_CIPHER(CIPHER_AES128, Rijndael128)
DEF_AES_CIPHER(CIPHER_AES256, Rijndael256)

#undef DEF_AES_CIPHER

} // namespace sec


#elif defined(PLATFORM_IOSwww)

//bridge to c interface from botan_objc.mm

namespace sec
{



} // namespace sec

#else
#endif
