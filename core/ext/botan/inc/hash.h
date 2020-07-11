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

#include "ipp_cp.h"


namespace sec
{

enum _tagHashMethod
{	HASH_CRC32 = 0,
	HASH_MD5,
	HASH_SHA1,	// aka SHA160
	HASH_SHA224,
	HASH_SHA256,
	HASH_SHA384,
	HASH_SHA512,
	HASH_MAX,
};

namespace _details
{
	template<int HASH_METHOD>
	struct _HashSize;
		template<> struct _HashSize<HASH_CRC32>
		{	static const int size = 4;
		};
		template<> struct _HashSize<HASH_MD5>
		{	static const int size = 16; 
		};
		template<> struct _HashSize<HASH_SHA1>
		{	static const int size = 20; 
		};
		template<> struct _HashSize<HASH_SHA224>
		{	static const int size = 28; 
		};
		template<> struct _HashSize<HASH_SHA256>
		{	static const int size = 32; 
		};
		template<> struct _HashSize<HASH_SHA384>
		{	static const int size = 48; 
		};
		template<> struct _HashSize<HASH_SHA512>
		{	static const int size = 64; 
		};

} // namespace _details
} // namespace sec


#if defined(PLATFORM_INTEL_IPP_SUPPORT)
namespace sec
{
namespace _details
{
	template<UINT _METHOD>
	struct _get_hash_context_size;
		template<> struct _get_hash_context_size<HASH_MD5>{ static const UINT Result = 103; };
		template<> struct _get_hash_context_size<HASH_SHA1>{ static const UINT Result = 111; };
		template<> struct _get_hash_context_size<HASH_SHA224>{ static const UINT Result = 119; };
		template<> struct _get_hash_context_size<HASH_SHA256>{ static const UINT Result = 119; };
		template<> struct _get_hash_context_size<HASH_SHA384>{ static const UINT Result = 239; };
		template<> struct _get_hash_context_size<HASH_SHA512>{ static const UINT Result = 239; };

	template<UINT _METHOD>
	struct _get_hmachash_context_size;
		template<> struct _get_hmachash_context_size<HASH_MD5>{ static const UINT Result = 103; };
		template<> struct _get_hmachash_context_size<HASH_SHA1>{ static const UINT Result = 163; };
		template<> struct _get_hmachash_context_size<HASH_SHA224>{ static const UINT Result = 119; };
		template<> struct _get_hmachash_context_size<HASH_SHA256>{ static const UINT Result = 119; };
		template<> struct _get_hmachash_context_size<HASH_SHA384>{ static const UINT Result = 239; };
		template<> struct _get_hmachash_context_size<HASH_SHA512>{ static const UINT Result = 239; };
}

template<UINT _METHOD = HASH_MD5> class Hash;
template<UINT _METHOD = HASH_MD5> class HmacHash;

#define HASH_IPP(tag)	template<> class Hash<HASH_##tag> \
						{	public: \
							static const int HASHSIZE = _details::_HashSize<HASH_##tag>::size; \
							protected:	BYTE	_Context[_details::_get_hash_context_size<HASH_##tag>::Result]; \
							public:		Hash(){ int sz; IPPCALL(ipps##tag##GetSize)(&sz); ASSERT(sz <= (int)sizeof(_Context)); Reset(); } \
										void Reset(){ IPPCALL(ipps##tag##Init)((Ipps##tag##State*)_Context); } \
										template<typename T> void Update(const T& x){ Update(&x, sizeof(x)); } \
										void Update(LPCVOID data, UINT size){ IPPCALL(ipps##tag##Update)((LPCBYTE)data,size,(Ipps##tag##State*)_Context); } \
										void Finalize(LPVOID HashValue){ IPPCALL(ipps##tag##Final)((LPBYTE)HashValue,(Ipps##tag##State*)_Context); } \
										void Calculate(LPCVOID data, UINT size, LPVOID HashValue){ Reset(); Update(data, size); Finalize(HashValue); } \
						}; \
						template<> class HmacHash<HASH_##tag> \
						{	public: \
							static const int HASHSIZE = _details::_HashSize<HASH_##tag>::size; \
							protected:	BYTE	_Context[_details::_get_hmachash_context_size<HASH_##tag>::Result]; \
							public:		HmacHash(LPCVOID key, UINT key_len){ int sz; IPPCALL(ippsHMAC##tag##GetSize)(&sz); ASSERT(sz <= (int)sizeof(_Context)); Reset(key, key_len); } \
										void Reset(LPCVOID key, UINT key_len){ IPPCALL(ippsHMAC##tag##Init)((LPCBYTE)key, key_len, (IppsHMAC##tag##State*)_Context); } \
										void Update(const rt::String_Ref& x){ Update(x.Begin(), (UINT)x.GetLength()); } \
										void Update(LPCVOID data, UINT size){ IPPCALL(ippsHMAC##tag##Update)((LPCBYTE)data,size,(IppsHMAC##tag##State*)_Context); } \
										void Finalize(LPVOID HashValue){ IPPCALL(ippsHMAC##tag##Final)((LPBYTE)HashValue,_details::_HashSize<HASH_##tag>::size,(IppsHMAC##tag##State*)_Context); } \
										void Calculate(LPCVOID message, UINT message_len, LPVOID HashValue){ Update(message, message_len); Finalize(HashValue); } \
						}; \

		HASH_IPP(MD5)
		HASH_IPP(SHA1)
		HASH_IPP(SHA224)
		HASH_IPP(SHA256)
		HASH_IPP(SHA384)
		HASH_IPP(SHA512)

#undef HASH_IPP

template<> class Hash<HASH_CRC32>
{	
protected:
	Ipp32u	_Context;
public:
	Hash(){ Reset(); }
	void	Reset(){ _Context = 0; }//~((DWORD)0); }
	void	Reset(LPCVOID HashValue){ _Context = *((Ipp32u*)HashValue); IPPCALL(ippsSwapBytes_32u_I)((Ipp32u*)&_Context, 1); }
	template<typename T> void Update(const T& x){ Update(&x, sizeof(x)); }
	void	Update(LPCVOID data, UINT size){ IPPCALL(ippsCRC32_8u)((LPCBYTE)data, size, &_Context); }
	void	Finalize(LPVOID HashValue){ *((Ipp32u*)HashValue) = _Context; IPPCALL(ippsSwapBytes_32u_I)((Ipp32u*)HashValue, 1); }
	DWORD	GetCRC() const { DWORD ret = _Context; IPPCALL(ippsSwapBytes_32u_I)((Ipp32u*)&ret, 1); return ret; }
	void	Calculate(LPCVOID data, UINT size, LPVOID HashValue){ Reset(); Update(data, size); Finalize(HashValue); }
	DWORD	Calculate(LPCVOID data, UINT size){ DWORD a; Calculate(data,size,&a); return a; }
};

} // namespace sec

#else // back by Botan
namespace sec
{

namespace _details
{
	template<int HASH_METHOD>
	struct _HashTrait;
		template<> struct _HashTrait<HASH_CRC32> {	typedef Botan::CRC32 type;	};
		template<> struct _HashTrait<HASH_MD5>	 {	typedef Botan::MD5 type;	};
		template<> struct _HashTrait<HASH_SHA1>	 {	typedef Botan::SHA_160 type;};
		template<> struct _HashTrait<HASH_SHA224>{	typedef Botan::SHA_224 type;};
		template<> struct _HashTrait<HASH_SHA256>{	typedef Botan::SHA_256 type;};
		template<> struct _HashTrait<HASH_SHA384>{	typedef Botan::SHA_384 type;};
		template<> struct _HashTrait<HASH_SHA512>{	typedef Botan::SHA_512 type;};
}


template<int HASH_METHOD = HASH_MD5>
class Hash
{
public:
	static const int HASHSIZE = _details::_HashSize<HASH_METHOD>::size;
	typedef typename _details::_HashTrait<HASH_METHOD>::type HashType;
protected:
	HashType*	Hasher;
public:
	INLFUNC Hash()
	{	Hasher = NULL;
		try
		{	Hasher = _New(HashType);
		}catch(std::exception& e){ _LOG_EXPCEPTION(e.what()); }
	}
	INLFUNC ~Hash()
	{	try{ _SafeDel(Hasher); }
		catch(std::exception& e){ _LOG_EXPCEPTION(e.what()); }
	}
	INLFUNC void Reset()
	{	try{ Hasher->clear(); return; }
		catch(std::exception& e){ _LOG_EXPCEPTION(e.what()); }
	}
	INLFUNC void Update(LPCVOID data, SIZE_T size)
	{	try{ Hasher->update((LPCBYTE)data, size); return; }
		catch(std::exception& e){ _LOG_EXPCEPTION(e.what()); }
	}
	INLFUNC void Finalize(LPVOID HashValue)
	{	try{ Hasher->final((LPBYTE)HashValue); return; }
		catch(std::exception& e){ _LOG_EXPCEPTION(e.what()); }			
	}
	INLFUNC void Calculate(LPCVOID data, SIZE_T size, LPVOID HashValue)
	{	try
		{	Hasher->clear();
			Hasher->update((LPCBYTE)data, size);
			Hasher->final((LPBYTE)HashValue);
		}
		catch(std::exception& e){ _LOG_EXPCEPTION(e.what()); }
	}
	INLFUNC void Calculate(LPCVOID data, SIZE_T size, rt::String& hash_hex)
	{
		BYTE hash[HASHSIZE];
		Calculate(data,size,hash);

		hash_hex.SetLength(HASHSIZE*2);
		os::Base16Encode(hash_hex.Begin(), hash, HASHSIZE);
	}
};

} // namespace sec

#if defined(PLATFORM_IOS)
#include <CommonCrypto/CommonDigest.h>

namespace sec
{

#define HASH_CC(tag, ctxtag)    template<> class Hash<HASH_##tag>   \
                        {   typedef CC_##ctxtag##_CTX CTX;     \
                        public: static const int HASHSIZE = _details::_HashSize<HASH_##tag>::size;  \
                        protected:  CTX _Ctx; \
                        public:     Hash(){ Reset(); }  \
                                    void Reset(){ CC_##tag##_Init(&_Ctx); } \
                                    template<typename T> void Update(const T& x){ Update(&x, sizeof(x)); } \
                                    void Update(LPCVOID data, UINT size){ CC_##tag##_Update(&_Ctx, data, size); } \
                                    void Finalize(LPVOID HashValue){ CC_##tag##_Final((LPBYTE)HashValue, &_Ctx); } \
                                    void Calculate(LPCVOID data, UINT size, LPVOID HashValue){ CC_##tag(data, size, (LPBYTE)HashValue); } \
                        };
   
    HASH_CC(MD5,   MD5)
    HASH_CC(SHA1,   SHA1)
    HASH_CC(SHA224, SHA256)
    HASH_CC(SHA256, SHA256)
    HASH_CC(SHA384, SHA512)
    HASH_CC(SHA512, SHA512)

} // namespace sec


#endif // #if defined(PLATFORM_IOS)
#endif // #ifdef PLATFORM_INTEL_IPP_SUPPORT




