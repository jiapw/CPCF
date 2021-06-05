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

#include "../os/thread_primitive.h"
#include "string_type_ops.h"
#include <algorithm>

namespace rt
{

namespace _details
{
	template<bool is_not_pod, typename t_Val>
	struct _xtor
	{	static FORCEINL void ctor(t_Val*p){ new (p) t_Val(); }
		static FORCEINL void ctor(t_Val*p, t_Val*end){ for(;p<end;p++) new (p) t_Val(); }
		static FORCEINL void ctor(t_Val*p, const t_Val& x){ new (p) t_Val(x); }
		static FORCEINL void dtor(t_Val& x){ x.~t_Val(); }
		static FORCEINL void dtor(t_Val*p, t_Val*end){ for(;p<end;p++) p->~t_Val(); }
		static FORCEINL void copy(t_Val*p, t_Val*end, const t_Val*from){ for(;p<end;p++,from++)*p = *from; }
	};
		template<typename t_Val>
		struct _xtor<false, t_Val>
		{	static FORCEINL void ctor(t_Val*p){}
			static FORCEINL void ctor(t_Val*p, t_Val*end){}
			static FORCEINL void ctor(t_Val*p, const t_Val& x){ rt::CopyByteTo(x, *p); }
			static FORCEINL void dtor(t_Val& x){}
			static FORCEINL void dtor(t_Val*p, t_Val*end){}
			static FORCEINL void copy(t_Val*p, t_Val*end, const t_Val*from){ memcpy(p, from, (LPCBYTE)end - (LPCBYTE)p); }
		};
};

template<typename t_Val, typename t_Index = SIZE_T>
class Buffer_Ref
{	
	static const bool IsElementNotPOD = !rt::TypeTraits<t_Val>::IsPOD;
protected:
	typedef _details::_xtor<!rt::TypeTraits<t_Val>::IsPOD, t_Val>	_xt;
	typedef int (* _PtFuncCompare)(const void *, const void *);
	typedef typename rt::TypeTraits<t_Index>::t_Signed	t_SignedIndex;

	t_Val*	_p;
	t_Index	_len;
public:
	typedef t_Val						ItemType;
	typedef ItemType*					LPItemType;
	typedef const ItemType*				LPCItemType;

	LPCItemType Begin()const { return &_p[0]; }
	LPItemType  Begin(){ return &_p[0]; }
	LPCItemType End()const { return &_p[GetSize()]; }
	LPItemType  End(){ return &_p[GetSize()]; }

	t_Val&			First(){ return _p[0]; }
	const t_Val&	First() const { return _p[0]; }
	t_Val&			Last(){ return _p[_len-1]; }
	const t_Val&	Last() const { return _p[_len-1]; }

	operator LPItemType(){ return &_p[0]; }
	operator LPCItemType()const { return &_p[0]; }
	operator LPVOID(){ return &_p[0]; }
	operator LPCVOID()const { return &_p[0]; }

	t_Index GetSize() const { return _len; }

public:
	// allow for(iterator : Buffer) syntax (C++ 11)
	t_Val*			begin(){ return _p; }
	const t_Val*	begin() const { return _p; }
	t_Val*			end(){ return _p + _len; }
	const t_Val*	end() const { return _p + _len; }

	Buffer_Ref(){ _p=nullptr; _len=0; }
	Buffer_Ref(const Buffer_Ref<t_Val>& x){ _p = x._p; _len = x._len; }
	Buffer_Ref(const t_Val*	p, t_Index len){ _p = (t_Val*)p; _len = len; }
	Buffer_Ref GetSub(t_Index start, t_Index size){ return Buffer_Ref(_p+start, size); }
	template<typename T> void Set(const T& x)
	{	t_Val*	end = _p + _len;	
		for(t_Val* p = _p; p < end; p++)*p = x;
	}
	template<typename T>
	t_Val& operator [](T i){ ASSERT(((SIZE_T)i)<=_len); return _p[i]; }
	template<typename T>
	const t_Val& operator [](T i) const { ASSERT(((SIZE_T)i)<=_len); return _p[i]; }
	template<typename T> bool operator ==(const Buffer_Ref<T>& x) const
	{	if(x.GetSize() != GetSize())return false;
		for(t_Index i=0;i<GetSize();i++)
		{	if(_p[i] == x[i]){}
			else return false;
		}
		return true;
	}
	template<typename T> bool operator !=(const Buffer_Ref<T>& x) const
	{	if(x.GetSize() != GetSize())return true;
		for(t_Index i=0;i<GetSize();i++)
			if(_p[i] != x[i])return true;
		return false;
	}
	template<typename T> t_SignedIndex SearchItem(const T& x) const
	{	for(t_Index i=0;i<GetSize();i++)
			if(_p[i] == x)return i;
		return -1;
	}
	t_Val& MaxValue()
	{	ASSERT(GetSize());		auto* mp = &_p[0];
		for(t_Index i=1;i<GetSize();i++)if(*mp < _p[i]){ mp = &_p[i]; }
		return *mp;
	}
	const t_Val& MaxValue() const { return rt::_CastToNonconst(this)->MaxValue(); }
	t_SignedIndex Max() const
	{	if(GetSize())
		{	auto* mp = &_p[0];	t_SignedIndex m = 0;
			for(t_Index i=1;i<GetSize();i++)if(*mp < _p[i]){ mp = &_p[i]; m = i; }
			return m;
		}else return -1;
	}
	t_Val& MinValue()
	{	ASSERT(GetSize());		auto* mp = &_p[0];
		for(t_Index i=1;i<GetSize();i++)if(_p[i] < *mp){ mp = &_p[i]; }
		return *mp;
	}
	const t_Val& MinValue() const { return rt::_CastToNonconst(this)->MinValue(); }
	t_SignedIndex Min() const
	{	if(GetSize())
		{	auto* mp = &_p[0];	t_SignedIndex m = 0;
			for(t_Index i=1;i<GetSize();i++)if(_p[i] < *mp){ mp = &_p[i]; m = i; }
			return m;
		}else return -1;
	}
	t_SignedIndex SearchSortedItem(const t_Val& x) const // binary search
	{	return std::find(Begin(), End(), x) - Begin();
	}
	t_SignedIndex SearchLowerbound(const t_Val& x) const // binary search
	{	return std::lower_bound(Begin(), End(), x) - Begin();
	}
	void Zero(){ static_assert(rt::TypeTraits<t_Val>::IsPOD, "Zero() applies only to POD elements"); memset((LPVOID)_p, 0, _len*sizeof(t_Val)); }
	void Void(){ static_assert(rt::TypeTraits<t_Val>::IsPOD, "Void() applies only to POD elements"); memset((LPVOID)_p, 0xff, _len*sizeof(t_Val)); }
	template<typename T>
	void CopyFrom(const Buffer_Ref<T>& x)
	{	ASSERT(GetSize() == x.GetSize());
		for(t_Index i=0;i<GetSize();i++)
			_p[i] = x[i];
	}
	template<typename T>
	void CopyFrom(const T* x)
	{	for(t_Index i=0;i<GetSize();i++)
			_p[i] = x[i];
	}
	template<typename T>
	void CopyTo(T* x) const
	{	for(t_Index i=0;i<GetSize();i++)
			x[i] = _p[i];
	}
	void CopyFrom(const t_Val* x){ _details::_xtor<IsElementNotPOD, t_Val>::copy(_p, end(), x); }
	void CopyTo(t_Val* x) const { _details::_xtor<IsElementNotPOD, t_Val>::copy(x, x + _len, _p); }
	void Sort()
	{	struct _comp
		{	static int compare(const t_Val * a, const t_Val * b)
			{	if(*a < *b)return -1;
				if(*b < *a)return 1;
				return 0;
		}	};
		Sort<_comp>();
	}
	void SortDesc()
	{	struct _comp
		{	static int compare(const t_Val * a, const t_Val * b)
			{	if(*a < *b)return 1;
				if(*b < *a)return -1;
				return 0;
		}	};
		Sort<_comp>();
	}
	template<class T>
	void Sort()
	{
		qsort(Begin(), GetSize(), sizeof(t_Val), (_PtFuncCompare)T::compare);
	}
	template<class TFUNC>
	void Sort(TFUNC&& comp)
	{
		std::sort(Begin(), End(), comp);
	}
	void Shuffle(DWORD seed)
	{	if(GetSize())
		{	Randomizer rng(seed);	
			for(t_Index i=0;i<GetSize()-1;i++)
				rt::Swap((*this)[i], (*this)[i+ rng%(GetSize()-i)]);
		}
	}
	SSIZE_T PushSorted(const t_Val& x) // last item will be dropped
	{
		if(GetSize() == 0 || Last() < x)return -1;
		_xt::dtor(Last());
		auto* p = &Last();
		for(p--;p>=Begin();p--)
		{	if(x < *p){ rt::Copy(p[1], *p); }
			else break;
		}
		p++;
		_xt::ctor(p, x);
		return p - Begin();
	}
	t_Val& FindTopKth(SIZE_T k)	// Find Top-k Smallest value over unordered array (original values will be moved around)
	{	struct _Find	// https://www.geeksforgeeks.org/kth-smallestlargest-element-unsorted-array/  [ Method 4 (QuickSelect) ]
		{	static SIZE_T partition(t_Val* arr, SIZE_T l, SIZE_T r) 
			{	t_Val x(arr[r]);
				SIZE_T i = l;
				for(SIZE_T j = l; j <= r - 1; j++) 
				{ 	if(x < arr[j])continue;
					rt::Swap(arr[i], arr[j]);
					i++; 
				} 
				rt::Swap(arr[i], arr[r]);
				return i; 
			} 		
			static t_Val& kth(t_Val* arr, SIZE_T l, SIZE_T r, SIZE_T k)
			{	if(k > 0 && k <= r - l + 1) 
				{ 	SIZE_T pos = partition(arr, l, r); 
					if (pos-l == k-1) 
						return arr[pos]; 
					if (pos-l > k-1)
						return kth(arr, l, pos-1, k); 
					return kth(arr, pos+1, r, k-pos+l-1);
				} 
				ASSERT(0);	// k is more than number of elements in array 
				return arr[0];
			}
		};
		ASSERT(k>0);
		ASSERT(GetSize() >= k);
		return _Find::kth(_p, 0, GetSize()-1, k);
	}
	void RandomBits(DWORD seed = rand())
	{	static_assert(rt::TypeTraits<t_Val>::IsPOD, "RandomBits() applies only to POD elements");
		Randomizer rng(seed);
		t_Index int_size = GetSize()*sizeof(t_Val)/4;
		t_Index i=0;
		for(; i<int_size; i++)
			((DWORD*)_p)[i] = rng;
		i*=4;
		if(GetSize()*sizeof(t_Val) > i)
		{	
			DWORD last = rng;
			int b=0;
			for(;i<GetSize()*sizeof(t_Val);i++, b++)
				((BYTE*)_p)[i] = ((BYTE*)&last)[b];
		}
	}
	template<typename visitor>
	SIZE_T ForEach(const visitor& v)  // v(obj, idx, tot)
	{	SIZE_T i=0;
		while(i<GetSize())
		{	if(!rt::_details::_CallLambda<bool, decltype(v(_p[i],i,GetSize()))>(true, v, _p[i],i,GetSize()).retval)
				return i+1;
			i++;
		}
		return i;
	}
	template<typename visitor>
	SIZE_T ForEach(const visitor& v) const  // v(const obj, idx, tot)
	{	SIZE_T i=0;
		while(i<GetSize())
		{	if(!rt::_details::_CallLambda<bool, decltype(v(_p[i],i,GetSize()))>(true, v, _p[i],i,GetSize()).retval)
				return i+1;
			i++;
		}
		return i;
	}
	template<typename T>
	SSIZE_T Find(const T& v) const
	{	for(SIZE_T i=0; i<GetSize(); i++)
			if(_p[i] == v)return i;
		return -1;
	}
};


template<typename t_Val>
class Buffer:public Buffer_Ref<t_Val>
{	typedef Buffer_Ref<t_Val> _SC;
protected:
	static const bool IsElementNotPOD = !rt::TypeTraits<t_Val>::IsPOD;
	void __SafeFree()
	{	if(!_SC::_p)return;
		_SC::_xt::dtor(_SC::_p, _SC::_p+_SC::_len);
		_SafeFree32AL((LPCVOID&)_SC::_p);
	}
public:
	Buffer(){}
	Buffer(const t_Val* p, SIZE_T len){ *this = Buffer_Ref<t_Val>(p,len); }
	explicit Buffer(const Buffer_Ref<t_Val> &x){ *this=x; }	//copy ctor should be avoided, use reference for function parameters
	explicit Buffer(const Buffer<t_Val> &x){ *this=x; }	//copy ctor should be avoided, use reference for function parameters
	const Buffer_Ref<t_Val>& operator = (const Buffer<t_Val> &x){ *this = (Buffer_Ref<t_Val>&)x; return *this; }
	const Buffer_Ref<t_Val>& operator = (const Buffer_Ref<t_Val> &x)
    {	for(SIZE_T i=0;i<_SC::_len;i++)
			_SC::_xt::dtor(_SC::_p[i]);
        if(_SC::_len >= x.GetSize()){ _SC::_len = x.GetSize(); }
		else
		{	_SafeFree32AL(((LPVOID&)_SC::_p));
            _SC::_p = _Malloc32AL(t_Val, _SC::_len = x.GetSize());
			ASSERT(_SC::_p);
		}
		for(SIZE_T i=0;i<_SC::_len;i++)
			_SC::_xt::ctor(&_SC::_p[i], x[i]);
		return x;
	}
	~Buffer(){ __SafeFree(); }
	bool SetSize(SIZE_T co=0) //zero for clear
	{	
		if(co == _SC::_len)
		{	if(co == 0)__SafeFree();
			return true; 
		}
		else
		{	__SafeFree();
			_SC::_len = co;
			if(co)
			{	_SC::_p = _Malloc32AL(t_Val,co);
				if( _SC::_p )
				{	
					_SC::_xt::ctor(_SC::_p, _SC::_p+_SC::_len);
				}
				else
				{	_SC::_len = 0; return false; }
			}
		}
		return true;
	}
	bool ChangeSize(SIZE_T new_size) //Original data at front is preserved
	{	
		if(new_size<=_SC::_len){ ShrinkSize(new_size); return true; }
		else	//expand buffer
		{	t_Val* new_p = _Malloc32AL(t_Val,new_size);
			if(new_p)
			{	memcpy(new_p,_SC::_p,_SC::_len*sizeof(t_Val));
				_SafeFree32AL(_SC::_p);
				_SC::_p = new_p;
			
				for(SIZE_T i=_SC::_len;i<new_size;i++)_SC::_xt::ctor(&_SC::_p[i]); //call ctor for additional instances at back
				_SC::_len = new_size;
				return true;
			}
		}
		return false;
	}
	bool ExpandSize(SIZE_T new_size)	// no shrink
	{	if(new_size <= _SC::_len)return true;
		return ChangeSize(new_size);
	}
	void ShrinkSize(SIZE_T new_size)	// no expand
	{	if(new_size >= _SC::_len)return;
		if(new_size<_SC::_len)
		{	
			for(SIZE_T i=new_size;i<_SC::_len;i++)_SC::_xt::dtor(_SC::_p[i]);	//call dtor for unwanted instances at back
			_SC::_len = new_size;
		}
	}
	t_Val* Detach(){ auto* p = _SC::_p; _SC::_p = nullptr; _SC::_len = 0; return p; }
	auto Remove(const t_Val& v)
	{	UINT open = 0;
		for(UINT i=0; i<_SC::_len; i++)
			if(!(_SC::_p[i] == v))
			{	if(i != open)
				{	_SC::_xt::dtor(_SC::_p[open]);
					rt::Copy(_SC::_p[open], _SC::_p[i]);
				}
				open++;
			}
		auto ret = _SC::_len - open;
		_SC::_len = open;
		return ret;
	}
};

template<typename t_Val>
class BufferEx: public Buffer<t_Val>
{
	typedef Buffer<t_Val> _SC;
	bool _add_entry(SIZE_T co = 1) // the added entry's ctor is not called !!
	{	if(_SC::_len+co <= _len_reserved){} // expand elements only
		else // expand buffer
		{	SIZE_T new_buf_resv = rt::max(rt::max((SIZE_T)4, _SC::_len+co),_len_reserved*2);
			t_Val* pNewBuffer = _Malloc32AL(t_Val,new_buf_resv);
			if(pNewBuffer){}else
			{	new_buf_resv = _SC::_len+co;
				pNewBuffer = (t_Val*)_Malloc32AL(t_Val,new_buf_resv);
				if(pNewBuffer){}else{ return false; }
			}
			// copy old elements
			memcpy(pNewBuffer, _SC::_p,sizeof(t_Val)*_SC::_len);
			_SafeFree32AL(_SC::_p);
			_SC::_p = (t_Val*)pNewBuffer;
			_len_reserved = new_buf_resv;
		}
		_SC::_len+=co;
		return true;
	}
protected:
	SIZE_T	_len_reserved;
public:
	BufferEx(const BufferEx &x){ _len_reserved = 0; *this = x; }
	const BufferEx& operator = (const BufferEx &x)
	{	_SC::ShrinkSize(0);
		_SC::_len = x.GetSize();
		if(_SC::_len <= _len_reserved){}
		else
		{	_SafeFree32AL(_SC::_p);
			_SC::_p = _Malloc32AL(t_Val,_SC::_len);
			ASSERT(_SC::_p);
			_len_reserved = _SC::_len;
		}
		for(SIZE_T i=0; i<_SC::_len; i++)
			_SC::_xt::ctor(&_SC::_p[i], x[i]);
		return x;
	}
	BufferEx(){ _len_reserved=0; }
	bool SetSize(SIZE_T co=0) //zero for clear
	{	if(_SC::SetSize(co)){ _len_reserved=_SC::_len; return true; }
		else{ _len_reserved=0; return false; }
	}
	t_Val* Detach(bool maintain_reserve_size = false)
	{	if(_SC::_len == 0)return nullptr;
		auto* p = _SC::Detach();	ASSERT(p);
		if(maintain_reserve_size){ VERIFY(_SC::_p = _Malloc32AL(t_Val,_len_reserved)); }
		else _len_reserved = 0;
		return p;
	}
	SIZE_T GetSize() const { return _SC::GetSize(); } // make Visual Studio happy
	bool Clear(){ return SetSize(0); }
	bool ChangeSize(SIZE_T new_size, bool keep_old_data = true) // Original data at front is preserved
	{	if( new_size == _SC::_len )return true;
		if( new_size < _SC::_len ){ _SC::ShrinkSize(new_size); return true; }
		else 
		{	if(new_size <= _len_reserved){} // expand elements only
			else // expand buffer
			{	_len_reserved = rt::max(new_size, _len_reserved*2);
				LPBYTE pNewBuffer = (LPBYTE)_Malloc32AL(t_Val,_len_reserved);
				if(pNewBuffer){}else
				{	_len_reserved = new_size;
					pNewBuffer = (LPBYTE)_Malloc32AL(t_Val,_len_reserved);
					if(pNewBuffer){}else{ return false; }
				}
				// copy old elements
				if(keep_old_data)
					memcpy(pNewBuffer, _SC::_p,sizeof(t_Val)*_SC::_len);
				_SafeFree32AL(_SC::_p);
				_SC::_p = (t_Val*)pNewBuffer;
			}
			for(SIZE_T i=_SC::_len;i<new_size;i++)_SC::_xt::ctor(&_SC::_p[i]); // call ctor for additional instances at back
			_SC::_len = new_size;
			return true;
		}
	}
	bool ExpandSize(SIZE_T new_size, bool keep_old_data = true)	// no shrink
	{	if(new_size <= _SC::_len)return true;
		return ChangeSize(new_size, keep_old_data);
	}
	void ShrinkSize(SIZE_T new_size)	// no expand
	{	if(new_size >= _SC::_len)return;
		if(new_size<_SC::_len)
		{	
			for(SIZE_T i=new_size;i<_SC::_len;i++)_SC::_xt::dtor(_SC::_p[i]);	//call dtor for unwanted instances at back
			_SC::_len = new_size;
		}
	}
	SIZE_T GetReservedSize() const { return _len_reserved; }
	t_Val& push_front()
	{	VERIFY(_add_entry());
		memmove(&_SC::_p[1],&_SC::_p[0],sizeof(t_Val)*(_SC::_len-1));
		_SC::_xt::ctor(&_SC::_p[0]);
		return _SC::_p[0];
	}
	template<typename T>
	t_Val& push_front(const T& x)
	{	VERIFY(_add_entry());
		memmove(&_SC::_p[1],&_SC::_p[0],sizeof(t_Val)*(_SC::_len-1));
		_SC::_xt::ctor(&_SC::_p[0],x);
		return _SC::_p[0];
	}
	template<typename T>
	void push_front(const T* x, SIZE_T count)
	{	if(count == 0)return;
		SIZE_T sz = _SC::GetSize();
		VERIFY(_add_entry(count));
		memmove(&_SC::_p[count],&_SC::_p[0],sizeof(t_Val)*sz);
		for(SIZE_T sz = 0;sz<count;sz++){ _SC::_xt::ctor(&_SC::_p[sz], *x++); }
	}
	template<typename T>
	void push_front(const T& x, SIZE_T count)
	{	if(count == 0)return;
		SIZE_T sz = _SC::GetSize();
		VERIFY(_add_entry(count));
		memmove(&_SC::_p[count],&_SC::_p[0],sizeof(t_Val)*sz);
		for(SIZE_T i = 0;i<count;i++){ _SC::_xt::ctor(&_SC::_p[i], x); }
	}
	template<typename T>
	void push_both(const T& x, SIZE_T front_count, SIZE_T back_count)
	{	if(front_count + back_count == 0)return;
		SIZE_T sz = _SC::GetSize();
		VERIFY(ChangeSize(sz + front_count + back_count));
		memmove(&_SC::_p[front_count],&_SC::_p[0],sizeof(t_Val)*sz);
		for(SIZE_T i = 0;i<front_count;i++){ _SC::_xt::ctor(&_SC::_p[i], x); }
		for(SIZE_T i = sz+front_count;i<_SC::_len;i++){ _SC::_xt::ctor(&_SC::_p[i], x); }
	}
	t_Val& push_back()
	{	VERIFY(_add_entry());
		_SC::_xt::ctor(&_SC::_p[_SC::_len-1]);
		return _SC::_p[_SC::_len-1];
	}
	t_Val& push_back(const t_Val& x)
	{	VERIFY(_add_entry());
		_SC::_xt::ctor(&_SC::_p[_SC::_len-1],x);
		return _SC::_p[_SC::_len-1];
	}
	template<typename T>
	void push_back(const T* x, SIZE_T count)
	{	
		SIZE_T sz = _SC::GetSize();
		VERIFY(_add_entry(count));
		for(;sz<_SC::_len;sz++){ _SC::_xt::ctor(&_SC::_p[sz], *x++); }
	}
	t_Val* push_back_n(SIZE_T count)
	{
		SIZE_T sz = _SC::GetSize();
		return ChangeSize(sz + count)?&_SC::_p[sz]:nullptr;
	}
	bool push_back_n(SIZE_T count, const t_Val& v)
	{
		SIZE_T sz = _SC::GetSize();
		if(ChangeSize(sz + count)){ for(SIZE_T i=0; i<count; i++) _SC::_p[i+sz] = v; return true; }
		return false;
	}
	void erase(const t_Val* p)
	{	ASSERT(p < _SC::End());
		ASSERT(p >= _SC::Begin());
		erase(p - _SC::Begin());
	}
	void erase(SIZE_T index)
	{	ASSERT(index<_SC::_len);
		// call dtor for removed items
		_SC::_xt::dtor(_SC::_p[index]);
		_SC::_len--;
		memmove(&_SC::_p[index],&_SC::_p[index+1],sizeof(t_Val)*(_SC::_len-index));
	}
	void erase(const t_Val* begin, const t_Val* end) // *end will not be erased
	{	erase(begin - _SC::Begin(), end - _SC::Begin());
	}
	void erase(SIZE_T index_begin, SIZE_T index_end) // [index_end] will not be erased
	{	ASSERT(index_begin<=index_end);
		ASSERT(index_end<=_SC::_len);
		// call dtor for removed items
		for(SIZE_T i=index_begin;i<index_end;i++)_SC::_xt::dtor(_SC::_p[i]);
		memmove(&_SC::_p[index_begin],&_SC::_p[index_end],sizeof(t_Val)*(_SC::_len-index_end));
		_SC::_len-=(index_end-index_begin);
	}
	void pop_back()
	{	ASSERT(_SC::_len); 	_SC::_len--;
		_SC::_xt::dtor(_SC::_p[_SC::_len]);
	}
	void pop_front()
	{	ASSERT(_SC::_len); 	_SC::_len--;
		_SC::_xt::dtor(_SC::_p[0]);
		memmove(&_SC::_p[0],&_SC::_p[1],sizeof(t_Val)*_SC::_len);
	}
	void compact_memory()
	{	if(_SC::_len < _len_reserved)
		{	LPBYTE pNew = (LPBYTE)_Malloc32AL(t_Val,_SC::_len);
			if(pNew)
			{	memcpy(pNew,_SC::_p,sizeof(t_Val)*_SC::_len);
				_SafeFree32AL(_SC::_p);
				_SC::_p = pNew;
				_len_reserved = _SC::_len;
			}
		}
	};
	bool reserve(SIZE_T co)
	{	if(co>_len_reserved)
		{	LPBYTE pNew = (LPBYTE)_Malloc32AL(t_Val,co);
			if(pNew)
			{	memcpy(pNew,_SC::_p,sizeof(t_Val)*_SC::_len);
				_SafeFree32AL(_SC::_p);
				_SC::_p = (t_Val*)pNew;
				_len_reserved = co;
			}
			return (bool)(pNew!=nullptr);
		}
		return true;
	}
	template<typename T>
	T& push_back_pod() // sizeof T should be multiple of size of t_Val
	{	ASSERT(sizeof(T)%sizeof(t_Val) == 0);
		static_assert(rt::TypeTraits<T>::IsPOD && rt::TypeTraits<t_Val>::IsPOD, "push_back_pod takes only POD types");
		return *(T*)push_back_n(sizeof(T)/sizeof(t_Val));
	}
	template<typename T>
	void push_back_pod(const T& obj){ rt::Copy(push_back_pod<T>(), obj); }
	t_Val& first(){ ASSERT(_SC::_len); return _SC::_p[0]; }
	const t_Val& first()const{  ASSERT(_SC::_len); return _SC::_p[0]; }
	t_Val& last(){  ASSERT(_SC::_len); return _SC::_p[_SC::_len-1]; }
	const t_Val& last()const{  ASSERT(_SC::_len); return _SC::_p[_SC::_len-1]; }

	t_Val& insert(SIZE_T index)
	{	VERIFY(_add_entry());
		if(index<_SC::_len-1)	
		{	memmove(&_SC::_p[index+1],&_SC::_p[index],(_SC::_len-index-1)*sizeof(t_Val));
			_SC::_xt::ctor(&_SC::_p[index]);
		}
		return _SC::_p[index];
	}
	void insert(SIZE_T index, const t_Val& x)
	{	VERIFY(_add_entry());
		if(index<_SC::_len-1)	
		{	memmove(&_SC::_p[index+1],&_SC::_p[index],(_SC::_len-index-1)*sizeof(t_Val));
			_SC::_xt::ctor(&_SC::_p[index]);
		}
		_SC::_p[index] = x;
	}
	t_Val* insert_n(SIZE_T index, SIZE_T count)
	{
		if(reserve(_SC::GetSize() + count))
		{	memmove(&_SC::_p[index+count], &_SC::_p[index], (_SC::GetSize() - index)*sizeof(t_Val));
			for(SIZE_T i = index;i<index+count;i++)_SC::_xt::ctor(&_SC::_p[i]);
			_SC::_len += count;
			return &_SC::_p[index];
		}	
		return nullptr;
	}
	void insert_n(SIZE_T index, SIZE_T count, const t_Val& x)
	{
		VERIFY(reserve(_SC::GetSize() + count));
		memmove(&_SC::_p[index+count], &_SC::_p[index], (_SC::GetSize() - index)*sizeof(t_Val));
		for(SIZE_T i = index;i<index+count;i++)_SC::_xt::ctor(&_SC::_p[i], x);
		_SC::_len += count;
	}
	SSIZE_T PushSorted(const t_Val& x)
	{
		if(GetSize() == 0 || x<first()){ push_front(x); return 0; }
		if(	_len_reserved == _SC::GetSize() &&
			!reserve(_SC::GetSize()*2 + 1)
		)
		{	return -1;
		}
		_SC::_len++;
		auto* p = &_SC::Last();
		for(p--;;p--)
		{	if(x < *p){ rt::Copy(p[1], *p); }
			else break;
		}
		p++;
		_SC::_xt::ctor(p, x);
		return p - _SC::Begin();
	}
	template<typename compr_less>
	SSIZE_T PushSorted(const t_Val& x, compr_less& compr)
	{
		if(GetSize() == 0 || compr(x, first())){ push_front(x); return 0; }
		if(	_len_reserved == _SC::GetSize() &&
			!reserve(_SC::GetSize()*2 + 1)
		)
		{	return -1;
		}
		_SC::_len++;
		auto* p = &_SC::Last();
		for(p--;;p--)
		{	if(compr(x, *p)){ rt::Copy(p[1], *p); }
			else break;
		}
		p++;
		_SC::_xt::ctor(p, x);
		return p - _SC::Begin();
	}
	SIZE_T PushUnique(const t_Val& x)
	{	auto idx = _SC::Find(x);
		if(idx>=0)return (SIZE_T)idx;
		push_back(x);
		return _SC::GetSize() - 1;
	}
};

template<typename T>
class TypeTraits<Buffer<T>>
{
public:	
	typedef Buffer<T> t_Val;
	typedef T t_Element;
	typedef Buffer<T> t_Signed;
	typedef Buffer<T> t_Unsigned;
	static const int Typeid = _typeid_buffer;
	static const bool IsPOD = false;
	static const bool IsNumeric = false;
};

template<typename T>
class TypeTraits<BufferEx<T>>
{
public:	
	typedef BufferEx<T> t_Val;
	typedef T t_Element;
	typedef BufferEx<T> t_Signed;
	typedef BufferEx<T> t_Unsigned;
	static const int Typeid = _typeid_buffer;
	static const bool IsPOD = false;
	static const bool IsNumeric = false;
};

template<class t_Ostream, typename t_Ele>
t_Ostream& operator<<(t_Ostream& Ostream, const _details::_LOG_FULLDATA<rt::Buffer_Ref<t_Ele>> & vec)
{	Ostream<<'{';
	if(rt::TypeTraits<typename rt::TypeTraits<t_Ele>::t_Element>::Typeid == rt::_typeid_8s)
	{
		for(SIZE_T i=0;i<vec.GetSize();i++)
		{	if(i)
				Ostream<<','<<'"'<<vec[i]<<'"';
			else
				Ostream<<'"'<<vec[i]<<'"';
		}
	}
	else
	{
		for(SIZE_T i=0;i<vec.GetSize();i++)
		{	if(i)
				Ostream<<','<<vec[i];
			else
				Ostream<<vec[i];
		}
	}
	Ostream<<'}';
	return Ostream;
}

template<class t_Ostream, typename t_Ele>
t_Ostream& operator<<(t_Ostream& Ostream, const rt::Buffer_Ref<t_Ele> & vec)
{
	if(rt::TypeTraits<typename rt::TypeTraits<t_Ele>::t_Element>::Typeid == rt::_typeid_8s)
	{
		if(vec.GetSize()>5)
		{	
			Ostream<<'{';
			for(SIZE_T i=0;i<3;i++)
				Ostream<<'"'<<vec[i]<<'"'<<',';
			Ostream<<" ... ,\"";
			Ostream<<vec.End()[-1]<<"\"} // size="<<vec.GetSize();
			return Ostream;
		}
	}
	else
	{
		if(vec.GetSize()>10)
		{	
			Ostream<<'{';
			for(SIZE_T i=0;i<8;i++)
				Ostream<<vec[i]<<',';
			Ostream<<" ... ,";
			Ostream<<vec.End()[-1]<<"} // size="<<vec.GetSize();
			return Ostream;
		}
	}

	return Ostream << rt::LOG_FULLDATA(vec);
}
} // namespace rt

namespace rt
{

namespace _details
{
template<bool has_trailing = true>
struct Trailing
{	template<typename X>
	static void Clear(X& x){ x._Bits[X::RT_BLOCK_COUNT - 1] &= (~(typename X::RT_BLOCK_TYPE)(0))>>(X::RT_BLOCK_SIZE - (X::BIT_SIZE%X::RT_BLOCK_SIZE)); }
};	
	template<> struct Trailing<false>
	{	template<typename X>
		static void Clear(X& x){};
	};

template<UINT bit_size, bool refer>
class BooleanArrayStg
{	template<bool> friend struct Trailing;
protected:
	static_assert(refer == false, "Fix-sized BooleanArray Refer is not supportted");
	typedef DWORD RT_BLOCK_TYPE;
	static const UINT	BIT_SIZE = bit_size;
	static const UINT	RT_BLOCK_SIZE = sizeof(RT_BLOCK_TYPE)*8;
	static const UINT	RT_BLOCK_COUNT = (BIT_SIZE + RT_BLOCK_SIZE - 1)/RT_BLOCK_SIZE;

	RT_BLOCK_TYPE			_Bits[RT_BLOCK_COUNT];
	void					_ClearTrailingBits(){ Trailing<(bit_size%RT_BLOCK_SIZE)!=0>::Clear(*this); }
};
template<>
class BooleanArrayStg<0, false>
{
protected:
	typedef DWORD RT_BLOCK_TYPE;
	static const UINT		RT_BLOCK_SIZE = sizeof(RT_BLOCK_TYPE)*8;
	UINT					BIT_SIZE;
	UINT					RT_BLOCK_COUNT;

	rt::BufferEx<RT_BLOCK_TYPE>_Bits;
	void					_ClearTrailingBits(){ if(RT_BLOCK_COUNT)_Bits[RT_BLOCK_COUNT - 1] &= (~(RT_BLOCK_TYPE)0)>>(RT_BLOCK_SIZE - (BIT_SIZE%RT_BLOCK_SIZE)); }
public:
	void	SetBitSize(UINT bit_size, bool keep_existing_data = true)
			{	
				BIT_SIZE = bit_size;
				RT_BLOCK_COUNT = (BIT_SIZE + RT_BLOCK_SIZE - 1)/RT_BLOCK_SIZE;
				VERIFY(_Bits.ChangeSize(RT_BLOCK_COUNT, keep_existing_data));
				_ClearTrailingBits();
			}
};
template<>
class BooleanArrayStg<0, true>
{
protected:
	typedef DWORD RT_BLOCK_TYPE;
	static const UINT		RT_BLOCK_SIZE = sizeof(RT_BLOCK_TYPE)*8;
	UINT					BIT_SIZE;
	UINT					RT_BLOCK_COUNT;

	RT_BLOCK_TYPE*			_Bits;
	void					_ClearTrailingBits(){ if(RT_BLOCK_COUNT)_Bits[RT_BLOCK_COUNT - 1] &= (~(RT_BLOCK_TYPE)0)>>(RT_BLOCK_SIZE - (BIT_SIZE%RT_BLOCK_SIZE)); }
public:
	BooleanArrayStg(){ _Bits = nullptr; BIT_SIZE = RT_BLOCK_COUNT = 0; }
	void	Init(LPCVOID p, UINT bit_size) // bit_size must be multiple of 32
			{	ASSERT(bit_size%32 == 0);
				_Bits = (RT_BLOCK_TYPE*)p;
				BIT_SIZE = bit_size;
				RT_BLOCK_COUNT = (bit_size+RT_BLOCK_SIZE-1)/RT_BLOCK_SIZE;
			}
};

} // namespace _details

template<UINT bit_size = 0, bool refer = false>
class BooleanArray: public _details::BooleanArrayStg<bit_size, refer>
{
	typedef _details::BooleanArrayStg<bit_size, refer>	_SC;
    typedef typename _SC::RT_BLOCK_TYPE RT_BLOCK_TYPE;
	static const UINT RT_BLOCK_SIZE = _SC::RT_BLOCK_SIZE;
	
protected:
	static RT_BLOCK_TYPE	_BlockBitmask(UINT idx){ return ((RT_BLOCK_TYPE)1)<<(idx%RT_BLOCK_SIZE); }
public:
	class Index
	{	friend class BooleanArray;
		UINT			BlockOffset;
		RT_BLOCK_TYPE	Bitmask;
	public:
		Index(UINT idx)
		{	BlockOffset = idx/RT_BLOCK_SIZE;
			Bitmask = BooleanArray::_BlockBitmask(idx);
		}
		void operator ++(int)
		{	if(Bitmask == (1<<(RT_BLOCK_SIZE-1)))
			{	Bitmask = 1;
				BlockOffset++;
			}
			else Bitmask <<= 1;
		}
	};
	LPCVOID	GetBits() const { return _SC::_Bits; }
	bool	Get(const Index& idx) const { return _SC::_Bits[idx.BlockOffset]&idx.Bitmask; }
	DWORD	Get(UINT idx, UINT bit_count) // bit_count <= 32
			{	ASSERT(bit_count <= 32);
				if(idx + bit_count > _SC::BIT_SIZE)bit_count = _SC::BIT_SIZE - idx;
				if(bit_count == 0)return 0;
				return (DWORD)(((*(ULONGLONG*)&_SC::_Bits[idx/RT_BLOCK_SIZE]) >> (idx%RT_BLOCK_SIZE)) & ((1ULL<<bit_count)-1));
			}
	bool	Set(const Index& idx, bool v = true)
			{	bool org = !!(_SC::_Bits[idx.BlockOffset]&idx.Bitmask);
				if(v)
					_SC::_Bits[idx.BlockOffset] |= idx.Bitmask;
				else
					_SC::_Bits[idx.BlockOffset] &= ~idx.Bitmask;
				return org;
			}
	void	Set(UINT idx, DWORD bits, UINT bit_count) // bit_count <= 32
			{	if(bit_count)
				{	ASSERT(idx + bit_count < _SC::BIT_SIZE);
					ASSERT(bit_count <= 32);
					ULONGLONG& ull = *(ULONGLONG*)&_SC::_Bits[idx/RT_BLOCK_SIZE];
					UINT shift = idx%RT_BLOCK_SIZE;
					ull = (ull&~(((1ULL<<bit_count)-1)<<shift)) | (((ULONGLONG)bits) << shift);
				}
			}
	bool	AtomicSet(const Index& idx) // return the bit value before atomic set
			{
				return idx.Bitmask & os::AtomicOr(idx.Bitmask, (volatile UINT*)&_SC::_Bits[idx.BlockOffset]);
			}
	bool	AtomicReset(const Index& idx) // return the bit value before atomic set
			{
				return idx.Bitmask & os::AtomicAnd(~idx.Bitmask, (volatile UINT*)&_SC::_Bits[idx.BlockOffset]);
			}
	void	Reset(const Index& idx){ Set(idx, false); }
	void	ResetAll(){ memset(_SC::_Bits, 0, _SC::RT_BLOCK_COUNT*sizeof(RT_BLOCK_TYPE)); }
	void	SetAll(){ memset(_SC::_Bits, 0xff, _SC::RT_BLOCK_COUNT*sizeof(RT_BLOCK_TYPE)); _SC::_ClearTrailingBits(); }
	bool	IsAllReset() const { for(UINT i=0; i<_SC::RT_BLOCK_COUNT; i++)if(_SC::_Bits[i])return false; return true; }
	UINT	PopCount() const { UINT pc = 0; for(UINT i=0; i<_SC::RT_BLOCK_COUNT; i++)pc += rt::PopCount(_SC::_Bits[i]); return pc; }
	void	operator ^= (const BooleanArray& x){ for(UINT i=0;i<_SC::RT_BLOCK_COUNT; i++)_SC::_Bits[i] ^= x._Bits[i]; }
	void	operator |= (const BooleanArray& x){ for(UINT i=0;i<_SC::RT_BLOCK_COUNT; i++)_SC::_Bits[i] |= x._Bits[i]; }
	template<typename CB>
	UINT	VisitOnes(CB&& cb) const	// visit all ones
			{	UINT hit = 0;	UINT i=0;
				for(; i<_SC::RT_BLOCK_COUNT-1; i++)
				{	RT_BLOCK_TYPE bits = _SC::_Bits[i];
					if(bits)
					{	for(UINT b=0; b<RT_BLOCK_SIZE; b++)
						{	if(bits&(1ULL<<b))
							{	cb(i*RT_BLOCK_SIZE + b);
								hit++;
							}
						}
					}
				}
				RT_BLOCK_TYPE bits = _SC::_Bits[i];
				if(bits)
				{	for(UINT b=0; b<(bit_size%RT_BLOCK_SIZE); b++)
					{	if(bits&(1ULL<<b))
						{	cb(i*RT_BLOCK_SIZE + b);
							hit++;
						}
					}
				}
				return hit;
			}
	template<typename CB>
	void	ForEach(CB&& cb) const	// visit all ones
			{	UINT i=0;
				for(; i<sizeofArray(_SC::_Bits)-1; i++)
				{	RT_BLOCK_TYPE bits = _SC::_Bits[i];
					for(UINT b=0; b<RT_BLOCK_SIZE; b++)cb(bits&(1ULL<<b));
				}
				RT_BLOCK_TYPE bits = _SC::_Bits[i];
				for(UINT b=0; b<(_SC::BIT_SIZE%RT_BLOCK_SIZE); b++)cb(bits&(1ULL<<b));
			}
	template<char one = '1', char zero = '.'>
	auto	ToString(rt::String& append) const
			{	ForEach([&append](bool v){
					append += v?one:zero;
				});
				return append;
			}
	template<char sep = ','>
	auto	ToStringWithIndex(rt::String& append) const
			{	if(VisitOnes([&append](UINT v){
					append += rt::tos::Number(v);
					append += sep;
				}))append.Shorten(1);
				return append;
			}
	BooleanArray(){}
	BooleanArray(std::initializer_list<bool> a_args)
			{	rt::Zero(_SC::_Bits);
				UINT i = 0;
				for(auto b: a_args)Set(i++, b);
			}
	void	Shift(int s){ if(s>0){LeftShift((UINT)s);}else{{RightShift((UINT)-s);}} }
	void	LeftShift(UINT s)
			{	if(s == 0)return;					if(s > bit_size){ ResetAll(); return; }
				UINT offset = s/RT_BLOCK_SIZE;			s = s%RT_BLOCK_SIZE;
				UINT i = _SC::RT_BLOCK_COUNT - 1;
				for (; 0 < i - offset; i--)
					_SC::_Bits[i] = (_SC::_Bits[i - offset] << s) | (_SC::_Bits[i - offset - 1] >> (RT_BLOCK_SIZE - s));
				_SC::_Bits[i] = _SC::_Bits[i - offset] << s;
				if(i)rt::Zero(_SC::_Bits, (i-1)*RT_BLOCK_SIZE/8);
				_SC::_ClearTrailingBits();
			}
	void	RightShift(UINT s)
			{	if(s == 0)return;					if(s > bit_size){ ResetAll(); return; }
				UINT offset = s/RT_BLOCK_SIZE;	s = s%RT_BLOCK_SIZE;
				UINT i = 0;
				for (; i + offset < _SC::RT_BLOCK_COUNT - 1; i++)
					_SC::_Bits[i] = (_SC::_Bits[i + offset] >> s) | (_SC::_Bits[i + offset + 1] << (RT_BLOCK_SIZE - s));
				_SC::_Bits[i] = _SC::_Bits[i + offset] >> s;
				if(i + 1 < _SC::RT_BLOCK_COUNT)rt::Zero(&_SC::_Bits[i+1], (_SC::RT_BLOCK_COUNT - i - 1)*RT_BLOCK_SIZE/8);
			}
};
typedef BooleanArray<0, true> BooleanArrayRef;
} // namespace rt


namespace rt
{
namespace _details
{
template<class t_Storage>
class StreamT: protected t_Storage
{	typedef t_Storage _SC;
protected:
	UINT	m_Pos;
	UINT	m_Used;
public:
	StreamT(){ m_Pos = 0; m_Used = 0; }
	StreamT(LPCBYTE p, UINT len, UINT used_len):t_Storage(p,len){ m_Pos = 0; m_Used = used_len; }
	SIZE_T GetLength() const { return m_Used; }
	LPBYTE GetInternalBuffer(){ return (LPBYTE)_SC::_p; }
	LPCBYTE GetInternalBuffer() const { return (LPCBYTE)_SC::_p; }
	void Rewind(){ m_Pos = 0; }
	LONGLONG Seek(SSIZE_T offset, int nFrom = rt::_File::Seek_Begin)
	{	SSIZE_T newp = 0;
		switch(nFrom)
		{	case rt::_File::Seek_Begin:	newp = offset; break;
			case rt::_File::Seek_Current: newp = m_Pos + offset; break;
			case rt::_File::Seek_End:		newp = m_Used + offset; break;
			default: ASSERT(0);
		}
		if(newp >=0 && newp <= (SSIZE_T)_SC::_len)m_Pos = (UINT)newp;
		return m_Pos;
	}
};

template<class t_Storage>
class OStreamT: public StreamT<t_Storage>
{	typedef StreamT<t_Storage> _SC;
public:
	OStreamT(){}
	OStreamT(LPCBYTE p, UINT len):StreamT<t_Storage>(p,len,0){}
	UINT Write(LPCVOID pBuf, UINT co)
	{	co = rt::min((UINT)(_SC::_len - _SC::m_Pos), co);
		if(co)
		{	memcpy(_SC::_p + _SC::m_Pos,pBuf,co);
			_SC::m_Pos += co;
			if(_SC::m_Used < _SC::m_Pos)_SC::m_Used = _SC::m_Pos;
		}
		return co;
	}
};

template<class t_Storage>
class IStreamT: public StreamT<t_Storage>
{	typedef StreamT<t_Storage> _SC;
public:
	IStreamT(LPCBYTE p, UINT len):StreamT<t_Storage>(p,len,len){}
	UINT Read(LPVOID pBuf, UINT co)
	{	ASSERT(_SC::m_Pos <= _SC::_len);
		co = rt::min(co, (UINT)(_SC::_len - _SC::m_Pos));
		memcpy(pBuf,_SC::_p + _SC::m_Pos,co);
		_SC::m_Pos += co;
		return co;
	}
};

} // namespace _details

class OStream: public _details::OStreamT<Buffer<BYTE>>
{	typedef _details::OStreamT<Buffer<BYTE>> _SC;
public:
	UINT Write(LPCVOID pBuf, UINT co)
	{	if(ChangeSize(rt::max(_len,(SIZE_T)(m_Pos+co))))
		{	memcpy(_p + m_Pos,pBuf,co);
			m_Pos += co;
			return co;
		}
		else return 0;
	}
	LONGLONG Seek(SSIZE_T offset, int nFrom = rt::_File::Seek_Begin)
	{	SSIZE_T newp = 0;
		switch(nFrom)
		{	case rt::_File::Seek_Begin:	newp = offset; break;
			case rt::_File::Seek_Current: newp = m_Pos+offset; break;
			case rt::_File::Seek_End:		newp = _len + offset; break;
			default: ASSERT(0);
		}
		if(newp >=0 && ChangeSize(rt::max(_len,(SIZE_T)newp)))
			m_Pos = (UINT)newp;
		return m_Pos;
	}
	bool SetLength(UINT sz){ m_Pos = rt::min(m_Pos,sz); m_Used = sz; return ChangeSize((UINT)sz); }
};

typedef _details::OStreamT<Buffer_Ref<BYTE> > OStream_Ref;
typedef _details::IStreamT<Buffer_Ref<BYTE> > IStream_Ref;

template<UINT LEN>
class OStreamFixed: public OStream_Ref
{
	BYTE	_Buffer[LEN];
public:
	OStreamFixed():OStream_Ref(_Buffer,LEN){}
	bool SetLength(UINT sz){ if(sz <= LEN){ m_Pos = rt::min(m_Pos,sz); m_Used = sz; return true; } return false; }
};

class CircularBuffer // not thread-safe
{
protected:
#pragma pack(1)
	struct _BlockHeader
	{	SIZE_T	Next;
		SIZE_T	PayloadSize;
		SIZE_T	PayloadLength;
	};
	struct _Block: public _BlockHeader
	{	BYTE	Payload[1];
	};
public:
	struct Block
	{	SIZE_T	Length;
		BYTE	Data[1];
	};
#pragma pack()
protected:
	LPBYTE			_Buffer;
	SIZE_T			_BufferSize;
	SIZE_T			Back;
	SIZE_T			NewBlock;
	_Block*			pLastBlock;	
public:
	CircularBuffer()
	{	_Buffer = nullptr;
		SetSize(0);
	}
	~CircularBuffer(){ _SafeFree32AL(_Buffer); }
	void   SetSize(SIZE_T sz)
	{	_SafeFree32AL(_Buffer);
		if(sz)
		{	_Buffer = (LPBYTE)_Malloc32AL(BYTE,sz);
			_BufferSize = sz;
			ASSERT(_Buffer);
		}
		else
		{	_Buffer = nullptr;
			_BufferSize = 0;
		}
		Back = NewBlock = 0;
		pLastBlock = nullptr;
	}
	LPBYTE Push(SIZE_T size_max)
	{	size_max = _EnlargeTo32AL(size_max);
		SIZE_T	block_size = size_max + sizeof(_BlockHeader);
		_Block*	p;
		if(	(block_size + NewBlock < _BufferSize && NewBlock >= Back) ||
			(block_size + NewBlock < Back && NewBlock < Back)
		)
		{	p = (_Block*)(_Buffer + NewBlock);
			NewBlock += block_size;
		}
		else if(NewBlock >= Back && Back > block_size)
		{	p = (_Block*)_Buffer;
			NewBlock = block_size;
		}
		else return nullptr;

		p->PayloadSize = size_max;
		p->Next = INFINITE;
		p->PayloadLength = 0;
		
		if(pLastBlock)pLastBlock->Next = ((LPBYTE)p) - _Buffer;
		pLastBlock = p;
		return p->Payload;
	}
	void Pop()
	{
		if(Back != NewBlock)
		{
			_Block* p = (_Block*)(_Buffer + Back);
			if(p == pLastBlock)
			{	ASSERT(p->Next == INFINITE);
				pLastBlock = nullptr;
				NewBlock = Back = 0;
			}
			else
			{	ASSERT(p->Next != INFINITE);
				Back = p->Next;
			}
		}
	}
	void SetBlockSize(LPCVOID pushed, SIZE_T finalized)	// call after Push()
	{
		ASSERT(finalized);
		_Block* block = (_Block*)(((LPBYTE)pushed) - sizeof(_BlockHeader));
		ASSERT(block->PayloadSize >= finalized);
		block->PayloadLength = finalized;
	}
	const Block* Peek() const
	{
		Block* p = (Back != NewBlock)?(Block*)(_Buffer + Back + sizeof(_BlockHeader) - sizeof(SIZE_T)):nullptr;
		if(p && p->Length>0)
		{	return p;
		}
		else
			return nullptr;
	}
};


template<typename T, UINT TOP_K=1, typename T_WEIGHT = int, bool keep_latest_value = false>
class TopWeightedValues
{
	template<typename _T, UINT _TOP_K, typename _WEIGHT, bool s> friend class TopWeightedValues;
	struct _val
	{	T			Val;
		UINT		Count;
		T_WEIGHT	Wei;
	};
public:
	static const int TOP_COUNT_CLAMP = rt::TypeTraits<UINT>::MaxVal()/8*7; // all weight will be halfed if top exceeds TOP_WEIGHT_CLAMP
	static const int UNMATCHED = 0;
	static const int MATCHED = 1;
	static const int MATCHED_WITH_TOP = 2;
protected:
	_val	_TopValues[TOP_K];
	int		_Match(const T& val, T_WEIGHT wei)	// 0: not match, 1: matched and no promote, 2: matched
			{
				if(_TopValues[0].Val == val)
				{	_TopValues[0].Wei += wei;
					_TopValues[0].Count++;
					if(keep_latest_value)_TopValues[0].Val = val;
					return MATCHED_WITH_TOP;
				}
				if(_TopValues[0].Wei == 0)
				{	_TopValues[0].Wei = wei;
					_TopValues[0].Val = val;
					_TopValues[0].Count = 1;
					return MATCHED_WITH_TOP;
				}
				int ret = ((TopWeightedValues<T,TOP_K-1,T_WEIGHT>*)(void*)&_TopValues[1])->_Match(val, wei);
				if(ret == MATCHED_WITH_TOP)
				{	if(_TopValues[1].Wei > _TopValues[0].Wei)
					{	rt::Swap(_TopValues[1], _TopValues[0]);
						return MATCHED_WITH_TOP;
					}else return MATCHED;
				}
				return ret;
			}
	auto	_WeightSum() const { return _TopValues[0].Wei + ((TopWeightedValues<T,TOP_K-1,T_WEIGHT>*)&_TopValues[1])->_WeightSum(); }
public:
	TopWeightedValues(){ Reset(); }
	static	UINT GetSize(){ return TOP_K; }
	UINT	GetUniqueValueCount() const 
			{	for(UINT i=0; i<TOP_K; i++)
					if(_TopValues[i].Count == 0)return i;
				return TOP_K;
			}
	int		FindValue(const T& v) const
			{	for(UINT i=0; i<TOP_K; i++)
				{	if(_TopValues[i].Count == 0)return -1;
					if(_TopValues[i].Val == v)return i;
				}
				return -1;
			}
	void	Reset(){ rt::Zero(*this); }
	void	ClampWeight(T_WEIGHT weight_max)
			{	for(UINT i=0; i<TOP_K; i++)
				{	if(_TopValues[i].Wei > weight_max)
						_TopValues[i].Wei = weight_max;
					_TopValues[i].Count = 0;
				}
			}
	int		Sample(const T& val, T_WEIGHT wei = 1)		// UNMATCHED / MATCHED / MATCH_WITH_TOP, 0: no match, 1: matched but not the top one no promote, 2: matched with top one
			{	int ret = _Match(val, wei);
				if(ret == MATCHED_WITH_TOP)
				{	if(	_TopValues[0].Count > TOP_COUNT_CLAMP || 
						_TopValues[0].Wei > rt::TypeTraits<T_WEIGHT>::MaxVal()/8*7
					)
					{	for(UINT i=0; i<TOP_K; i++)
						{	_TopValues[i].Count >>=1;
							_TopValues[i].Wei /= 2;
						}
					}
					return ret;
				}
				else if(ret == UNMATCHED && wei > _TopValues[TOP_K-1].Wei)
				{	UINT i=TOP_K-1;
					for(; i>0; i--)
					{	if(_TopValues[i-1].Wei < wei){ _TopValues[i] = _TopValues[i-1]; }
						else break;
					}
					_TopValues[i].Val = val;
					_TopValues[i].Wei = wei;
					_TopValues[i].Count = 1;
				}
				return ret;
			}
	UINT	GetCapacity() const { return TOP_K; }
	bool	IsEmpty() const { return GetWeight() <= 0; }
	auto	GetWeight() const { return _TopValues[0].Wei; }
	auto	GetWeight(UINT i) const { return _TopValues[i].Wei; }
	auto	GetCount() const { return _TopValues[0].Count; }
	auto	GetCount(UINT i) const { return _TopValues[i].Count; }
	auto&	operator[](UINT i) const { return Get(i); }
	auto&	Get(UINT i = 0) const { return _TopValues[i].Val; }
	auto&	operator[](UINT i) { return Get(i); }
	auto&	Get(UINT i = 0) { return _TopValues[i].Val; }
	bool	Get(UINT i, T* val, T_WEIGHT* wei, UINT* count = nullptr) const
			{	if(_TopValues[i].Wei>0)
				{	*wei = _TopValues[i].Wei;
					*val = _TopValues[i].Val;
					if(count)*count = _TopValues[i].Count;
					return true; 
				}else return false;
			}
	void	Remove(UINT i)
			{	if(((int)TOP_K) - (int)i - 1 > 0)memmove(&_TopValues[i], &_TopValues[i+1], sizeof(_val)*(TOP_K - i - 1));
				_TopValues[TOP_K-1].Wei = 0;
				_TopValues[TOP_K-1].Count = 0;
			}
	bool	IsSignificant(T_WEIGHT min_weight = 0) const { return _TopValues[0].Wei > min_weight && _TopValues[0].Wei > _WeightSum()/2; }
	UINT	GetSignificantRatio(T_WEIGHT min_weight = 0) const { return _TopValues[0].Wei > min_weight?(UINT)(_TopValues[0].Wei*100/_WeightSum()):0; }
};
	template<typename T, typename T_WEIGHT, bool s>
	class TopWeightedValues<T, 0, T_WEIGHT, s>
	{	template<typename _T, UINT _TOP_K, typename _WEIGHT, bool _s> friend class TopWeightedValues;
		protected:	int		_Match(const T& val, T_WEIGHT wei){ return 0; }
					auto	_WeightSum() const { return 0; }
	};

template<class t_Ostream, typename t_Ele, UINT TOP, typename t_Wei, bool S>
t_Ostream& operator<<(t_Ostream& Ostream, const TopWeightedValues<t_Ele, TOP, t_Wei, S> & vec)
{	Ostream<<'{';
	if(rt::TypeTraits<typename rt::TypeTraits<t_Ele>::t_Element>::Typeid == rt::_typeid_8s)
	{
		for(UINT i=0;i<vec.GetSize();i++)
		{	if(i)
				Ostream<<','<<' '<<'"'<<vec[i]<<"\"="<<vec.GetWeight(i);
			else
				Ostream<<'"'<<vec[i]<<"\"="<<vec.GetWeight(i);
		}
	}
	else
	{
		for(UINT i=0;i<vec.GetSize();i++)
		{	if(i)
				Ostream<<','<<' '<<vec[i]<<'='<<vec.GetWeight(i);
			else
				Ostream<<vec[i]<<'='<<vec.GetWeight(i);
		}
	}
	Ostream<<'}';
	return Ostream;
}

} // namespace rt


