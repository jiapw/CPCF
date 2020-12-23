#pragma once

#include "../../../../CPCF/core/rt/string_type.h"
#include "../../../../CPCF/core/rt/buffer_type.h"
#include "src/sparsehash/dense_hash_map"
#include "src/sparsehash/dense_hash_set"
#include "src/sparsehash/sparse_hash_map"
#include "src/sparsehash/sparse_hash_set"

namespace ext
{

namespace _details
{

template<int _LEN>
struct zero_bits
{	BYTE	bits[_LEN];
	INLFUNC zero_bits(){ rt::Zero(bits); }
};

template<int _LEN, unsigned int minus = 0>
struct void_bits
{	BYTE	bits[_LEN];
	INLFUNC void_bits(){ rt::Void(bits); bits[0] -= minus; }
};

} // namespace _details

enum HASHKEY_CTOR_TYPE
{
	CTOR_ZERO = 0,
	CTOR_VOID = -1
};

template<typename T, bool is_pod = rt::TypeTraits<T>::IsPOD>
struct key_traits;
	template<typename T>
	struct key_traits<T, false>
	{	static const T& empty_key(){ static const T x(CTOR_ZERO); return x; }
		static const T& deleted_key(){ static const T x(CTOR_VOID); return x; }
	};
	template<typename T>
	struct key_traits<T, true>
	{	static const T& empty_key(){ static const _details::void_bits<sizeof(T), 1> x; return (const T&)x; }
		static const T& deleted_key(){ static const _details::void_bits<sizeof(T), 2> x; return (const T&)x; }
	};
	template<>
	struct key_traits<rt::String_Ref>
	{	static const rt::String_Ref& empty_key(){ static const rt::String_Ref x; return x; }
		static const rt::String_Ref& deleted_key(){ static const rt::String_Ref x("\x0\x0\x0\x0", 4); return x; }
	};
	template<>
	struct key_traits<rt::String>
	{	static const rt::String& empty_key(){ return (const rt::String&)key_traits<rt::String_Ref>::empty_key(); }
		static const rt::String& deleted_key(){ return (const rt::String&)key_traits<rt::String_Ref>::deleted_key(); }
	};
	template<typename POD>
	struct key_traits<rt::PodRef<POD>, false>
	{	static const rt::PodRef<POD>& empty_key(){ static const rt::PodRef<POD> a((POD*)0); return a; }
		static const rt::PodRef<POD>& deleted_key(){ static const rt::PodRef<POD> a((POD*)-1); return a; }
	};
    
template<typename KEY, typename VALUE, typename hash_compare = SPARSEHASH_HASH<KEY>>
class fast_map: public google::dense_hash_map<KEY, VALUE, hash_compare>
{   typedef google::dense_hash_map<KEY, VALUE, hash_compare> _SC;
public:
	INLFUNC fast_map()
    {	_SC::set_empty_key(key_traits<KEY>::empty_key());
		_SC::set_deleted_key(key_traits<KEY>::deleted_key());
	}
	INLFUNC const VALUE& get(KEY k, const VALUE& v) const
	{	auto it = _SC::find(k);
		return it != _SC::end()?it->second:v;
	}
	INLFUNC VALUE& get(KEY k)
	{	auto it = _SC::find(k);
		ASSERT(it != _SC::end());
		return it->second;
	}
};

template<typename KEY, typename hash_compare = SPARSEHASH_HASH<KEY>>
class fast_set: public google::dense_hash_set<KEY, hash_compare>
{   typedef google::dense_hash_set<KEY, hash_compare> _SC;
public:
	INLFUNC fast_set()
    {	_SC::set_empty_key(key_traits<KEY>::empty_key());
		_SC::set_deleted_key(key_traits<KEY>::deleted_key());
	}
	INLFUNC bool has(const KEY&& k) const { return _SC::find(k) != _SC::end(); }
	INLFUNC bool has(const KEY& k) const { return _SC::find(k) != _SC::end(); }
};

template<typename KEY, typename VALUE, typename hash_compare = SPARSEHASH_HASH<KEY>>
class fast_map_ptr: public fast_map<KEY, VALUE*, hash_compare>
{   typedef fast_map<KEY, VALUE*, hash_compare> _SC;
public:
    INLFUNC fast_map_ptr(){}
	INLFUNC VALUE* get(const KEY& k) const
	{
		auto it = _SC::find(k);
        if(it != _SC::end())return it->second;
		return nullptr;
	}
	INLFUNC bool has(const KEY& k) const
	{
		return _SC::find(k) != _SC::end();
	}
	INLFUNC VALUE* take(const KEY& k)
	{
		auto it = _SC::find(k);
		if(it == _SC::end())return nullptr;
		VALUE* ret = it->second;
		_SC::erase(it);
		return ret;
	}
	INLFUNC void safe_delete_pointers() // handle the case when partial memory in VALUE involved in KEY
	{
		rt::BufferEx<VALUE*> ptrs;
		auto it = _SC::begin();
		for(; it != _SC::end(); it++)
			if(it->second)
				ptrs.push_back(it->second);
		_SC::clear();
		for(UINT i=0;i<ptrs.GetSize();i++)
			_SafeDel_ConstPtr(ptrs[i]);
	}
};

template<typename t_Tag, typename t_Count = int, typename hash_compare = SPARSEHASH_HASH<t_Tag>>
class fast_counter: public fast_map<t_Tag, t_Count, hash_compare>
{   typedef fast_map<t_Tag, t_Count, hash_compare> _SC;
public:
	INLFUNC void Count(const t_Tag& tag, t_Count w = 1)
	{	auto it = _SC::find(tag);
        if(it == _SC::end())_SC::insert(std::pair<t_Tag, t_Count>(tag,w));
		else
		{	it->second += w;
		}
	}
	INLFUNC t_Count Get(const t_Tag& tag)
	{	auto it = _SC::find(tag);
		return it == _SC::end()?0:it->second;
	}
};

} // ext
