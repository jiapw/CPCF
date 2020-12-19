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

#include "../../../rt/string_type.h"
#include "../../../os/kernel.h"

namespace sec
{

template<UINT _LEN>
struct dword_op
{	INLFUNC static void set(LPDWORD x, DWORD v){ *x = v; dword_op<_LEN-1>::set(x+1,v); }
	INLFUNC static void set(LPDWORD x, LPCDWORD v){ *x = *v; dword_op<_LEN-1>::set(x+1,v+1); }
	INLFUNC static void xor_to(LPDWORD x, LPCDWORD v){ *x ^= *v; dword_op<_LEN-1>::xor_to(x+1,v+1); }
	INLFUNC static int  diff_count(LPCDWORD x, LPCDWORD v){ return ((int)(*x != *v)) + dword_op<_LEN-1>::diff_count(x+1,v+1); }
	INLFUNC static bool equ(LPCDWORD x, DWORD v){ if(*x != v)return false; return dword_op<_LEN-1>::equ(x+1,v); }
	INLFUNC static bool equ(LPCDWORD x, LPCDWORD v){ if(*x != *v)return false; return dword_op<_LEN-1>::equ(x+1,v+1); }
	INLFUNC static bool equ_sec(LPCDWORD x, LPCDWORD v){ return 0 == diff_count(x, v); }
	INLFUNC static bool not_equ_sec(LPCDWORD x, LPCDWORD v){ return diff_count(x, v) > 0; }
	INLFUNC static bool not_equ(LPCDWORD x, LPCDWORD v){ if(*x == *v)return false; return dword_op<_LEN-1>::not_equ(x+1,v+1); }
	INLFUNC static int  cmp(LPCDWORD x, LPCDWORD v){ if(*x > *v)return 1; if(*x < *v)return -1; return dword_op<_LEN-1>::cmp(x+1,v+1); }
	INLFUNC static bool equ_constant_time(LPCDWORD x, LPCDWORD v)
						{	bool ret = true;
							for(UINT i=0;i<_LEN;i++)
								ret = ret && x[i] == v[i];
							return ret;
						}
	INLFUNC static bool equ_constant_time(LPCDWORD x, DWORD v)
						{	bool ret = true;
							for(UINT i=0;i<_LEN;i++)
								ret = ret && x[i] == v;
							return ret;
						}
};
	template<> struct dword_op<0>
	{	INLFUNC static void set(LPDWORD x, DWORD v){}
		INLFUNC static void set(LPDWORD x, LPCDWORD v){}
		INLFUNC static void xor_to(LPDWORD x, LPCDWORD v){}
		INLFUNC static bool equ(LPCDWORD x, DWORD v){ return true; }
		INLFUNC static bool equ(LPCDWORD x, LPCDWORD v){ return true; }
		INLFUNC static int  diff_count(LPCDWORD x, LPCDWORD v){ return 0; }
		INLFUNC static bool not_equ(LPCDWORD x, LPCDWORD v){ return true; }
		INLFUNC static int  cmp(LPCDWORD x, LPCDWORD v){ return 0; }
	};

#pragma pack(push,1)
template<UINT _LEN>
class DataBlockRef
{   
protected:
	struct byte_data
	{	BYTE Bytes[_LEN];
		byte_data(DWORD v){ dword_op<_LEN/4>::set((LPDWORD)Bytes, v); }
	};
	INLFUNC static LPBYTE _Zero(){ return nullptr; }
	INLFUNC static LPBYTE _Void(){ return (LPBYTE)-1; }
	INLFUNC static LPCBYTE _GetZero(){ static const byte_data x(0); return x.Bytes; }
	INLFUNC static LPCBYTE _GetVoid(){ static const byte_data x(0xffffffff); return x.Bytes; }
	union {
		DWORD*	DWords;
		BYTE*	Bytes;
	};
	INLFUNC bool		_IsSymbolicZero() const { return Bytes == _Zero(); }
	INLFUNC bool		_IsSymbolicVoid() const { return Bytes == _Void(); }
public:
	typedef dword_op<_LEN/4> dwop;
	static const UINT LEN = _LEN;

    INLFUNC DataBlockRef(){ Bytes = nullptr; }
	INLFUNC DataBlockRef(decltype(NULL) x){ if(x==0){ Bytes = nullptr; }else if(x==-1){ Bytes = (LPBYTE)-1; }else{ ASSERT(0); } }
	INLFUNC DataBlockRef(const DataBlockRef& x){ Bytes = (LPBYTE)x.Bytes; }
	INLFUNC ~DataBlockRef(){}

	INLFUNC LPDWORD		GetDWords(){ return (LPDWORD)GetBytes(); }
	INLFUNC LPCDWORD	GetDWords() const { return (LPCDWORD)GetBytes(); }
	INLFUNC LPBYTE		GetBytes(){ ASSERT(Bytes); return Bytes; }
	INLFUNC LPCBYTE		GetBytes() const 
						{	if(_IsSymbolicZero())return _GetZero();
							if(_IsSymbolicVoid())return _GetVoid();
							return Bytes;
						}

	template< template<UINT l, bool s> class T >
	INLFUNC DataBlockRef(const T<_LEN, true>& x){ Bytes = (LPBYTE)x.Bytes; }
	template< template<UINT l, bool s> class T >
	INLFUNC DataBlockRef(const T<_LEN, false>& x){ Bytes = (LPBYTE)x.Bytes; }

    INLFUNC operator	LPCBYTE () const { return (LPCBYTE)GetBytes(); }
	INLFUNC operator	LPBYTE () { return (LPBYTE)GetBytes(); }
	template<typename T>
	INLFUNC const BYTE&	operator [](T i) const { return GetBytes()[i]; }
	template<typename T>
	INLFUNC BYTE&		operator [](T i){ return GetBytes()[i]; }
	INLFUNC UINT		GetLength() const { return _LEN; }

	template<typename T>
	INLFUNC const T&	Cast() const { ASSERT(sizeof(T) == _LEN); return *((const T*)GetBytes()); }

public:
	INLFUNC bool	operator == (const DataBlockRef<_LEN>& x) const 
					{	if(Bytes == x.Bytes)return true;
						if(x._IsSymbolicVoid() || _IsSymbolicVoid() || x._IsSymbolicZero() || _IsSymbolicZero())return false;
						return dwop::equ(GetDWords(), x.GetDWords()); 
					}
	INLFUNC bool	operator != (const DataBlockRef<_LEN>& x) const 
					{	if(Bytes == x.Bytes)return false;
						if(x._IsSymbolicVoid() || _IsSymbolicVoid() || x._IsSymbolicZero() || _IsSymbolicZero())return true;
						return dwop::not_equ(GetDWords(), x.GetDWords());
					}
	INLFUNC bool	operator < (const DataBlockRef<_LEN>& x) const	
					{	if((x._IsSymbolicVoid() && !_IsSymbolicVoid()) || (_IsSymbolicZero() && !x._IsSymbolicZero()))return true;
						if(x._IsSymbolicVoid() || _IsSymbolicVoid() || x._IsSymbolicZero() || _IsSymbolicZero())return false;
						return dwop::cmp(GetDWords(), x.GetDWords()) < 0;
					}
	INLFUNC bool	operator > (const DataBlockRef<_LEN>& x) const
					{	if((!x._IsSymbolicVoid() && _IsSymbolicVoid()) || (!_IsSymbolicZero() && x._IsSymbolicZero()))return true;
						if(x._IsSymbolicVoid() || _IsSymbolicVoid() || x._IsSymbolicZero() || _IsSymbolicZero())return false;
						return dwop::cmp(GetDWords(), x.GetDWords()) > 0;
					}
public:
	struct hash_compare
	{	
		enum // parameters for hash table
		{	bucket_size = 4,	// 0 < bucket_size
			min_buckets = 8		// min_buckets = 2 ^^ N, 0 < N
		};
		INLFUNC size_t operator()(const DataBlockRef& key) const 
		{	if(!key._IsSymbolicZero())return 0;
			if(!key._IsSymbolicVoid())return (size_t)-1;
			return rt::_details::_HashValueFix<_LEN>::Val(key.Bytes);
		}
		INLFUNC bool operator()(const DataBlockRef& _Keyval1, const DataBlockRef& _Keyval2) const { return _Keyval1 < _Keyval2;	}
		template<typename OStream>
		friend INLFUNC OStream& operator <<(OStream& s, const DataBlockRef& d)
		{	return s << rt::tos::Base32LowercaseOnStack<_LEN*8/5 + 4>((LPCBYTE)d, _LEN);
		}
	};
};
#pragma pack(pop)


#pragma pack(push,1)
template<UINT _LEN, bool is_sec = false>
struct DataBlock
{
	union 
	{	DWORD	DWords[_LEN/sizeof(DWORD)];
		BYTE	Bytes[_LEN];
	};
	INLFUNC bool	_IsSymbolicZero() const { return false; }
	INLFUNC bool	_IsSymbolicVoid() const { return false; }
	LPDWORD			GetDWords(){ return DWords; }
	LPCDWORD		GetDWords() const { return DWords; }
	LPBYTE			GetBytes(){ return Bytes; }
	LPCBYTE			GetBytes() const { return Bytes; }

public:
	typedef dword_op<_LEN/4> dwop;
	static const UINT LEN = _LEN;
    operator	LPCBYTE () const { return (LPCBYTE)GetBytes(); }
	operator	LPBYTE () { return (LPBYTE)GetBytes(); }
	template<typename T>
	const BYTE&  operator [](T i) const { return GetBytes()[i]; }
	template<typename T>
	BYTE& operator [](T i){ return GetBytes()[i]; }
	INLFUNC UINT		GetLength() const { return _LEN; }
	INLFUNC void		CopyTo(LPVOID buf) const { *((DataBlock*)buf) = *this; }
	INLFUNC void		SwitchByteOrder(){ rt::SwapByteOrder(*this); }
	INLFUNC bool		FromBase16(const rt::String_Ref& str){ return os::Base16DecodeLength(str.GetLength()) == _LEN && os::Base16Decode(GetBytes(), _LEN, str.Begin(), str.GetLength()); }
	INLFUNC bool		FromBase32(const rt::String_Ref& str){ return os::Base32CrockfordDecode(this, sizeof(*this), str.Begin(), str.GetLength()); }
	INLFUNC bool		FromBase64(const rt::String_Ref& str){ SIZE_T len; return os::Base64DecodeLength(str.Begin(), str.GetLength()) == _LEN && os::Base64Decode(GetBytes(), &len, str.Begin(), str.GetLength()) && len == _LEN; }
	INLFUNC bool		FromString(const rt::String_Ref& str)
						{	if(str.GetLength() == os::Base16EncodeLength(LEN) && FromBase16(str))return true;
							if(str.GetLength() == os::Base32EncodeLength(LEN) && FromBase32(str))return true;
							if(str.GetLength() == os::Base64EncodeLength(LEN) && FromBase64(str))return true;
							rt::Zero(*this); return false;
						}
	INLFUNC void		ToBase64(rt::String& str) const	{ str.SetLength(os::Base64EncodeLength(_LEN)); os::Base64Encode(str, GetBytes(), _LEN); }
	INLFUNC void		ToBase32(rt::String& str) const { str.SetLength(os::Base32EncodeLength(_LEN)); os::Base32CrockfordFavCharEncode(str, GetBytes(), _LEN); }
	INLFUNC void		ToBase16(rt::String& str) const	{ str.SetLength(os::Base16EncodeLength(_LEN)); os::Base16Encode(str, GetBytes(), _LEN); }
	INLFUNC bool		IsZero() const { return dwop::equ(GetDWords(), (DWORD)0); }
	INLFUNC bool		IsVoid() const { return dwop::equ(GetDWords(), (DWORD)0xffffffff); }
	INLFUNC DataBlock&	Random(UINT seed){ rt::Randomizer(seed).Randomize(GetBytes(), _LEN); return *this; }

	template<bool sb> INLFUNC void	From(const DataBlock<_LEN, sb>& x){	dwop::set(DWords, x.GetDWords()); }
	template<bool sb> INLFUNC int	Compare(const DataBlock<_LEN, sb>& x) const { return dwop::cmp(GetDWords(), x.GetDWords()); }
	template<bool sb> INLFUNC void	operator ^= (const DataBlock<_LEN, sb>& x){ dwop::xor_to(DWords, x.GetDWords()); }
	template<bool sb> INLFUNC bool	operator < (const DataBlock<_LEN, sb>& x) const	{ return dwop::cmp(GetDWords(), x.GetDWords()) < 0; }
	template<bool sb> INLFUNC bool	operator <= (const DataBlock<_LEN, sb>& x) const { return dwop::cmp(GetDWords(), x.GetDWords()) <= 0; }
	template<bool sb> INLFUNC bool	operator > (const DataBlock<_LEN, sb>& x) const { return dwop::cmp(GetDWords(), x.GetDWords()) > 0; }
	template<bool sb> INLFUNC bool	operator >= (const DataBlock<_LEN, sb>& x) const { return dwop::cmp(GetDWords(), x.GetDWords()) >= 0; }
	// equ_sec and not_equ_sec are time-consistent comparing to defense against timing attack
	template<bool sb> INLFUNC bool	operator == (const DataBlock<_LEN, sb>& x) const { return is_sec?dwop::equ_sec(GetDWords(), x.GetDWords()):dwop::equ(GetDWords(), x.GetDWords()); }
	template<bool sb> INLFUNC bool	operator != (const DataBlock<_LEN, sb>& x) const { return is_sec?dwop::not_equ_sec(GetDWords(), x.GetDWords()):dwop::not_equ(GetDWords(), x.GetDWords()); }

	INLFUNC const DataBlock& Zero(){ dwop::set(GetDWords(), (DWORD)0); return *this; }
	INLFUNC const DataBlock& Void(){ dwop::set(GetDWords(), (DWORD)0xffffffff); return *this; }

	TYPETRAITS_DECLARE_POD;
public:
	struct hash_compare
	{	
		enum // parameters for hash table
		{	bucket_size = 4,	// 0 < bucket_size
			min_buckets = 8		// min_buckets = 2 ^^ N, 0 < N
		};
		INLFUNC size_t operator()(const DataBlock& key) const { return rt::_details::_HashValueFix<_LEN>::Val(key.Bytes); }
		INLFUNC bool operator()(const DataBlock& _Keyval1, const DataBlock& _Keyval2) const	{ return _Keyval1 < _Keyval2; }
	};
	INLFUNC rt::String_Ref ToString() const { return rt::String_Ref((LPCSTR)Bytes,_LEN); }

public:
	template<bool sb>
	INLFUNC const DataBlock<_LEN, sb>& operator = (const DataBlock<_LEN, sb>& x){ From(x); }

	typedef DataBlockRef<_LEN> RefType;
	INLFUNC RefType Ref(){ return RefType(*this); }
	INLFUNC const RefType Ref() const { return RefType(*this); }

	INLFUNC operator DataBlock<_LEN, false>& (){ return *(DataBlock<_LEN, false>*)this; }
	INLFUNC operator const DataBlock<_LEN, false>& () const { return *(const DataBlock<_LEN, false>*)this; }

	template<typename OStream>
	friend INLFUNC OStream& operator <<(OStream& s, const DataBlock& d)
	{	return s << rt::tos::Base32LowercaseOnStack<_LEN*8/5 + 4>((LPCBYTE)d, _LEN);
	}
};
	template<UINT _LEN>
	struct DataBlock<_LEN, true>: public DataBlock<_LEN, false>
	{
		~DataBlock(){ rt::Zero(*this); }
	};
#pragma pack(pop)



} // namespace sec
