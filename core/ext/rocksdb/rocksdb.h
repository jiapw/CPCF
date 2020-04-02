#pragma once

#include "../../rt/type_traits.h"
#include "../../os/multi_thread.h"
#include "../../os/file_dir.h"
#include "rocksdb_conf.h"

#include "./include/db.h"
#include "./include/slice_transform.h"
#include "./include/merge_operator.h"

namespace ext
{

typedef ::rocksdb::WriteOptions	WriteOptions;
typedef ::rocksdb::ReadOptions	ReadOptions;
typedef ::rocksdb::Iterator		Iterator;
typedef ::rocksdb::Options		Options;


class SliceDyn: public ::rocksdb::Slice
{
public:
	INLFUNC SliceDyn(){ data_ = nullptr; }
	INLFUNC ~SliceDyn(){ _SafeFree32AL(data_); }
	template<typename StrExp>
	INLFUNC SliceDyn(StrExp& se){ data_ = nullptr; *this = se; }
	template<typename StrExp>
	INLFUNC StrExp& operator = (StrExp& se)
	{	_SafeFree32AL(data_);
		size_ = se.GetLength();
		if((data_ = _Malloc32AL(char, size_)))
		{	se.CopyTo((LPSTR)data_);
		}else{ size_ = 0; }
		return se;
	}
};

class SliceValue: public ::rocksdb::Slice
{
protected:
	char	_embedded[8];
public:
	INLFUNC SliceValue():Slice(nullptr, 0){}
	INLFUNC SliceValue(LPCVOID p, SIZE_T sz):Slice((char*)p, sz){}

	INLFUNC SliceValue(int i):Slice(_embedded, sizeof(i)){ *((int*)_embedded) = i; }
	INLFUNC SliceValue(BYTE i):Slice(_embedded, sizeof(i)){ *((BYTE*)_embedded) = i; }
	INLFUNC SliceValue(WORD i):Slice(_embedded, sizeof(i)){ *((WORD*)_embedded) = i; }
	INLFUNC SliceValue(DWORD i):Slice(_embedded, sizeof(i)){ *((DWORD*)_embedded) = i; }
	INLFUNC SliceValue(ULONGLONG i):Slice(_embedded, sizeof(i)){ *((ULONGLONG*)_embedded) = i; }
	INLFUNC SliceValue(LONGLONG i):Slice(_embedded, sizeof(i)){ *((LONGLONG*)_embedded) = i; }
	INLFUNC SliceValue(float i):Slice(_embedded, sizeof(i)){ *((float*)_embedded) = i; }
	INLFUNC SliceValue(double i):Slice(_embedded, sizeof(i)){ *((double*)_embedded) = i; }

	INLFUNC SliceValue(LPSTR str):Slice(str, str?strlen(str):0){}
	INLFUNC SliceValue(LPCSTR str):Slice(str, str?strlen(str):0){}
	
	INLFUNC SliceValue(const SliceDyn& i):Slice(i){}
	INLFUNC SliceValue(const Slice& i):Slice(i){}

	template<typename T>
	INLFUNC SliceValue(const T& x)
		:Slice((LPCSTR)rt::GetDataPtr(x), rt::GetDataSize(x))
	{}

	//template<size_t LEN>
	//INLFUNC SliceValue(char str[LEN]):Slice(str, LEN-1){}
	//template<size_t LEN>
	//INLFUNC SliceValue(const char str[LEN]):Slice(str, LEN-1){}

	INLFUNC SIZE_T GetSize() const { return size(); }

	INLFUNC rt::String_Ref ToString(SIZE_T off = 0) const { ASSERT(size_>=off); return rt::String_Ref(data_ + off, size_ - off); }
	template<typename T>
	const T& To(SIZE_T off = 0) const
	{	ASSERT(size_ >= off + sizeof(T));
		return *((T*)(data_ + off));
	}
};

#define SliceValueNull		::rocksdb::Slice()
#define SliceValueSS(x)		::rocksdb::Slice(x, sizeof(x)-1)

class RocksCursor
{
	::rocksdb::Iterator* iter;
	friend class RocksDB;

	RocksCursor();
	RocksCursor(::rocksdb::Iterator* it){ iter = it; }
	RocksCursor(const RocksCursor& x) = delete; // RocksCursor can only be constructed by RocksDB

public:
	RocksCursor(const RocksCursor&& x){ iter = x.iter; }	// move constructor, enable return by RocksDB::First/Last
	void	operator = (RocksCursor&& x){ _SafeDel_Untracked(iter); iter = x.iter; x.iter = nullptr; }

	template<typename T>
	INLFUNC const T&			Value() const { return *(T*)iter->value().data(); }
	template<typename T>
	INLFUNC const T&			Key() const { return *(T*)iter->key().data(); }

	INLFUNC const SliceValue	Key() const { return (const SliceValue&)iter->key(); }
	INLFUNC const SliceValue	Value() const { return (const SliceValue&)iter->value(); }
	INLFUNC SIZE_T				KeyLength() const { return iter->key().size(); }
	INLFUNC SIZE_T				ValueLength() const { return iter->value().size(); }
	INLFUNC						~RocksCursor(){ _SafeDel_Untracked(iter); }
	INLFUNC bool				IsValid() const { return iter && iter->Valid(); }
	INLFUNC void				Next(){ iter->Next(); }
	INLFUNC void				Prev(){ iter->Prev(); }
	INLFUNC void				Next(UINT co){ while(co--)iter->Next(); }
	INLFUNC void				Prev(UINT co){ while(co--)iter->Prev(); }
	INLFUNC bool				IsEmpty() const { return iter == nullptr; }
};

class RocksDB
{
protected:
	::rocksdb::DB*		_pDB;

public:
	static const WriteOptions*	WriteOptionsFastRisky;
	static const WriteOptions*	WriteOptionsDefault;
	static const WriteOptions*	WriteOptionsRobust;
	static const ReadOptions*	ReadOptionsDefault;

	enum DBScopeWriteRobustness
	{								// disableDataSync  use_fsync  allow_os_buffer
		DBWR_LEVEL_FASTEST = 0,		// true				false		true
		DBWR_LEVEL_DEFAULT,			// false			false		true
		DBWR_LEVEL_UNBUFFERED,		// false			false		false
		DBWR_LEVEL_STRONG			// false			true		false
	};

	// ## disableDataSync
	// If true, then the contents of manifest and data files are not synced
	// to stable storage. Their contents remain in the OS buffers till the
	// OS decides to flush them. This option is good for bulk-loading
	// of data. Once the bulk-loading is complete, please issue a
	// sync to the OS to flush all dirty buffesrs to stable storage.

	// ## use_fsync
	// If true, then every store to stable storage will issue a fsync.
	// If false, then every store to stable storage will issue a fdatasync.
	// This parameter should be set to true while storing data to
	// filesystem like ext3 that can lose files after a reboot.

	// ## allow_os_buffer
	// Hint the OS that it should not buffer disk I/O. Enabling this
	// parameter may improve performance but increases pressure on the
	// system cache.
	// The exact behavior of this parameter is platform dependent.
	//
	// On POSIX systems, after RocksDB reads data from disk it will
	// mark the pages as "unneeded". The operating system may - or may not
	// - evict these pages from memory, reducing pressure on the system
	// cache. If the disk block is requested again this can result in
	// additional disk I/O.
	//
	// On WINDOWS system, files will be opened in "unbuffered I/O" mode
	// which means that data read from the disk will not be cached or
	// bufferized. The hardware buffer of the devices may however still
	// be used. Memory mapped files are not impacted by this parameter.

public:
	RocksDB(){ _pDB = nullptr; }
	~RocksDB(){ Close(); }
	bool Open(LPCSTR db_path, DBScopeWriteRobustness robustness = DBWR_LEVEL_DEFAULT, bool open_existed_only = false, UINT file_thread_co = 2, UINT logfile_num_max = 1);
	bool Open(LPCSTR db_path, const Options* opt);
	bool IsOpen() const { return _pDB!=nullptr; }
	void Close(){ if(_pDB){ delete _pDB; _pDB = nullptr; } }
	bool Set(const SliceValue& k, const SliceValue& val, const WriteOptions* opt = WriteOptionsDefault){ ASSERT(_pDB); return _pDB->Put(*opt, k, val).ok(); }
	bool Merge(const SliceValue& k, const SliceValue& val, const WriteOptions* opt = WriteOptionsDefault){ ASSERT(_pDB); return _pDB->Merge(*opt, k, val).ok(); }
	bool Get(const SliceValue& k, std::string& str, const ReadOptions* opt = ReadOptionsDefault) const { ASSERT(_pDB); return _pDB->Get(*opt, k, &str).ok(); }
	bool Has(const SliceValue& k, const ReadOptions* opt = ReadOptionsDefault) const { thread_local std::string t; return Get(k, t, opt); }
	template<typename t_POD>
	bool Get(const SliceValue& k, t_POD* valout, const ReadOptions* opt = ReadOptionsDefault) const
	{	ASSERT_NONRECURSIVE;
		thread_local std::string temp;
		ASSERT(_pDB);
		if(_pDB->Get(*opt, k, &temp).ok() && temp.length() == sizeof(t_POD))
		{	memcpy(valout, temp.data(), sizeof(t_POD));
			return true;
		}else return false;
	}
	template<typename t_NUM>
	t_NUM GetAs(const SliceValue& k, t_NUM default_val = 0, const ReadOptions* opt = ReadOptionsDefault) const
	{	ASSERT_NONRECURSIVE;
		thread_local std::string temp;
		ASSERT(_pDB);
		return (_pDB->Get(*opt, k, &temp).ok() && temp.length() == sizeof(t_NUM))?
			   *((t_NUM*)temp.data()):default_val;
	}
	template<typename t_Type>
	const t_Type* Fetch(const SliceValue& k, SIZE_T* len_out = nullptr, const ReadOptions* opt = ReadOptionsDefault) const // Get a inplace referred buffer, will be invalid after next Fetch
	{	ASSERT_NONRECURSIVE;
		thread_local std::string temp;
		ASSERT(_pDB);
		if(_pDB->Get(*opt, k, &temp).ok() && temp.length() >= sizeof(t_Type))
		{	if(len_out)*len_out = temp.length();
			return (t_Type*)temp.data();
		}
		else
		{	if(len_out)*len_out = 0;
			return nullptr;
		}
	}
	rt::String_Ref Fetch(const SliceValue& k, const ReadOptions* opt = ReadOptionsDefault) const
	{	ASSERT_NONRECURSIVE;
		thread_local std::string temp;
		ASSERT(_pDB);
		return (_pDB->Get(*opt, k, &temp).ok())?
				rt::String_Ref(temp.data(), temp.length()):rt::String_Ref();
	}
	RocksCursor Find(const SliceValue& begin, const ReadOptions* opt = ReadOptionsDefault) const
	{	::rocksdb::Iterator* it = rt::_CastToNonconst(_pDB)->NewIterator(*opt);
		ASSERT(it);
		it->Seek(begin);
		return RocksCursor(it);
	}
	RocksCursor First(const ReadOptions* opt = ReadOptionsDefault) const
	{	::rocksdb::Iterator* it = rt::_CastToNonconst(_pDB)->NewIterator(*opt);
		ASSERT(it);
		it->SeekToFirst();
		return RocksCursor(it);
	}
	RocksCursor Last(const ReadOptions* opt = ReadOptionsDefault) const
	{	::rocksdb::Iterator* it = rt::_CastToNonconst(_pDB)->NewIterator(*opt);
		ASSERT(it);
		it->SeekToLast();
		return RocksCursor(it);
	}
	bool Delete(const SliceValue& k, const WriteOptions* opt = WriteOptionsDefault){ ASSERT(_pDB); return _pDB->Delete(*opt, k).ok(); }
	template<typename func_visit>
	SIZE_T ScanBackward(const func_visit& v, const SliceValue& begin, const ReadOptions* opt = ReadOptionsDefault) const
	{	ASSERT(_pDB);
		RocksCursor it = _pDB->NewIterator(*opt);
		ASSERT(!it.IsEmpty());
		SIZE_T ret = 0;
		for(it.iter->Seek(begin); it.IsValid(); it.Prev())
		{	ret++;
			if(!rt::_details::_CallLambda<bool, decltype(v(it))>(true, v, it).retval)
				break;
		}
		return ret;
	}
	template<typename func_visit>
	SIZE_T ScanBackward(const func_visit& v, const ReadOptions* opt = ReadOptionsDefault) const
	{	ASSERT(_pDB);
		RocksCursor it = _pDB->NewIterator(*opt);
		ASSERT(!it.IsEmpty());
		SIZE_T ret = 0;
		for(it.iter->SeekToLast(); it.IsValid(); it.Prev())
		{	ret++;
			if(!rt::_details::_CallLambda<bool, decltype(v(it))>(true, v, it).retval)
				break;
		}
		return ret;
	}
	template<typename func_visit>
	SIZE_T Scan(const func_visit& v, const SliceValue& begin, const ReadOptions* opt = ReadOptionsDefault) const
	{	ASSERT(_pDB);
		RocksCursor it = _pDB->NewIterator(*opt);
		ASSERT(!it.IsEmpty());
		SIZE_T ret = 0;
		for(it.iter->Seek(begin); it.IsValid(); it.Next())
		{	ret++;
			if(!rt::_details::_CallLambda<bool, decltype(v(it))>(true, v, it).retval)
				break;
		}
		return ret;
	}
	template<typename func_visit>
	SIZE_T Scan(const func_visit& v, const ReadOptions* opt = ReadOptionsDefault) const
	{	ASSERT(_pDB);
		RocksCursor it = _pDB->NewIterator(*opt);
		ASSERT(!it.IsEmpty());
		SIZE_T ret = 0;
		for(it.iter->SeekToFirst(); it.IsValid(); it.Next())
		{	ret++;
			if(!rt::_details::_CallLambda<bool, decltype(v(it))>(true, v, it).retval)
				break;
		}
		return ret;
	}
	template<typename func_visit>
	SIZE_T ScanPrefix(const func_visit& v, const SliceValue& prefix, const ReadOptions* opt = ReadOptionsDefault) const
	{	ASSERT(_pDB);
		RocksCursor it = _pDB->NewIterator(*opt);
		ASSERT(!it.IsEmpty());
		SIZE_T ret = 0;
		for(it.iter->Seek(prefix); it.IsValid() && it.Key().starts_with(prefix); it.Next())
		{	ret++;
			if(!rt::_details::_CallLambda<bool, decltype(v(it))>(true, v, it).retval)
				break;
		}
		return ret;
	}
	static void Nuke(LPCSTR db_path){ os::File::RemovePath(db_path); }
};

template<char separator = ':'>
class SeparatorPrefixTransform : public ::rocksdb::SliceTransform 
{
public:
	explicit SeparatorPrefixTransform() = default;
    virtual const char* Name() const override { return "SeparatorPrefixTransform"; }
	virtual ::rocksdb::Slice Transform(const ::rocksdb::Slice& src) const override {
		const char* p = src.data();
		const char* sep = strchr(p, separator);
		if(sep)
		{	return ::rocksdb::Slice(src.data(), (int)(sep - p));
		}else return src;
	}
	virtual bool InDomain(const ::rocksdb::Slice& src) const override {
		return true;
	}
	virtual bool InRange(const ::rocksdb::Slice& dst) const override {
		return dst[dst.size()-1] == ':' || strchr(dst.data(), ':') == nullptr;
	}
	virtual bool SameResultWhenAppended(const ::rocksdb::Slice& prefix) const override {
		return strchr(prefix.data(), ':') != nullptr;
	}
};

#define ALLOCA_DBKEY(varname, ...)	auto	varname##_strexp = __VA_ARGS__;	\
									char*	varname##_buf = (char*)alloca(varname##_strexp.GetLength());	\
									UINT	varname##_strlen = (UINT)varname##_strexp.CopyTo(varname##_buf); \
									SliceValue varname(varname##_buf, varname##_strlen); \

namespace _details
{

template<int LEN>
struct cmp
{	static INLFUNC bool less(LPCBYTE a, LPCBYTE b){ return *a < *b || (*a == *b && cmp<LEN-1>::less(a+1, b+1)); }
	static INLFUNC bool less_equal(LPCBYTE a, LPCBYTE b){ return *a < *b || ((*a == *b) && cmp<LEN-1>::less_equal(a+1, b+1)); }
	static INLFUNC bool great_equal(LPCBYTE a, LPCBYTE b){ return *a > *b || ((*a == *b) && cmp<LEN-1>::great_equal(a+1, b+1)); }
};
	template<> struct cmp<1>
	{	static INLFUNC bool less(LPCBYTE a, LPCBYTE b){ return *a < *b; }
		static INLFUNC bool less_equal(LPCBYTE a, LPCBYTE b){ return *a <= *b; }
		static INLFUNC bool great_equal(LPCBYTE a, LPCBYTE b){ return *a >= *b; }
	};
} // namespace _details

#pragma pack(push, 1)
template<typename T>
struct DbKeyDescendent  // for POD
{	BYTE	Data[sizeof(T)];
	static_assert(rt::TypeTraits<T>::IsPOD, "DbKeyDescendent<T>: T should be POD");
	DbKeyDescendent() = default;
	DbKeyDescendent(const DbKeyDescendent& x) = default;
	DbKeyDescendent(const T& x){ *this = x; }
	auto		operator = (const DbKeyDescendent& x) -> const DbKeyDescendent& { rt::CopyByteTo<sizeof(T)>(&x, this); return x; }
	const T&	operator = (const T& x)
				{	static_assert(sizeof(T) <= ULONGLONG);
					ULONGLONG a = *(ULONGLONG*)&x;
					a = 0xffffffffffffffffULL - a;
					rt::SwitchByteOrderTo(*(T*)&a, *(T*)Data);
					return x;
				}
				operator T() const
				{	ULONGLONG x = 0;
					rt::SwitchByteOrderTo(*(T*)Data, *(T*)&x);
					x = 0xffffffffffffffffULL - x;
					return (T&)x;
				}
	bool		operator < (const DbKeyDescendent& x) const { return _details::cmp<sizeof(T)>::less(Data, x.Data); }
	bool		operator <= (const DbKeyDescendent& x) const { return _details::cmp<sizeof(T)>::less_equal(Data, x.Data); }
	bool		operator >= (const DbKeyDescendent& x) const { return _details::cmp<sizeof(T)>::great_equal(Data, x.Data); }
	T			Original() const { return *this; }
};

template<typename T>
struct DbKeyAscendent  // for POD
{	BYTE	Data[sizeof(T)];
	static_assert(rt::TypeTraits<T>::IsPOD, "DbKeyDescendent<T>: T should be POD");
	DbKeyAscendent() = default;
	DbKeyAscendent(const DbKeyAscendent& x) = default;
	DbKeyAscendent(const T& x){ *this = x; }
	auto		operator = (const DbKeyAscendent& x) -> const DbKeyAscendent& { rt::CopyByteTo<sizeof(T)>(&x, this); return x; }
	const T&	operator = (const T& x)
				{
					rt::SwitchByteOrderTo(*(T*)&x, *(T*)Data);
					return x;
				}
				operator T() const
				{	T x;
					rt::SwitchByteOrderTo(*(T*)Data, *(T*)&x);
					return x;
				}
	bool		operator < (const DbKeyAscendent& x) const { return _details::cmp<sizeof(T)>::less(Data, x.Data); }
	bool		operator <= (const DbKeyAscendent& x) const { return _details::cmp<sizeof(T)>::less_equal(Data, x.Data); }
	bool		operator >= (const DbKeyAscendent& x) const { return _details::cmp<sizeof(T)>::great_equal(Data, x.Data); }
	T			Original() const { return *this; }
};
#pragma pack(pop)

} // namespace ext

