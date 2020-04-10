#pragma once

#include "../../rt/type_traits.h"
#include "../../os/multi_thread.h"
#include "../../os/file_dir.h"
#include "rocksdb_conf.h"

#include "./include/db.h"
#include "./include/slice_transform.h"
#include "./include/merge_operator.h"
#include "./include/comparator.h"

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
	INLFUNC RocksCursor(const RocksCursor&& x){ iter = x.iter; }	// move constructor, enable return by RocksDB::First/Last
	INLFUNC	~RocksCursor(){ Empty(); }
	INLFUNC void				operator = (RocksCursor&& x){ _SafeDel_Untracked(iter); iter = x.iter; x.iter = nullptr; }

	template<typename T>
	INLFUNC const T&			Value() const { return *(T*)iter->value().data(); }
	template<typename T>
	INLFUNC const T&			Key() const { return *(T*)iter->key().data(); }

	INLFUNC const SliceValue	Key() const { return (const SliceValue&)iter->key(); }
	INLFUNC const SliceValue	Value() const { return (const SliceValue&)iter->value(); }
	INLFUNC SIZE_T				KeyLength() const { return iter->key().size(); }
	INLFUNC SIZE_T				ValueLength() const { return iter->value().size(); }
	INLFUNC bool				IsValid() const { return iter && iter->Valid(); }
	INLFUNC void				Next(){ iter->Next(); }
	INLFUNC void				Prev(){ iter->Prev(); }
	INLFUNC void				Next(UINT co){ while(co--)iter->Next(); }
	INLFUNC void				Prev(UINT co){ while(co--)iter->Prev(); }
	INLFUNC bool				IsEmpty() const { return iter == nullptr; }
	INLFUNC void				Empty(){ _SafeDel_Untracked(iter); }
};


class RocksDB
{
	friend class RocksStorage;
	friend class RocksDBStandalone;

protected:
	::rocksdb::DB*		_pDB;
	::rocksdb::ColumnFamilyHandle*	_pCF;

public:
	static const WriteOptions*	WriteOptionsFastRisky;
	static const WriteOptions*	WriteOptionsDefault;
	static const WriteOptions*	WriteOptionsRobust;
	static const ReadOptions*	ReadOptionsDefault;

	enum DBOpenType
	{
		DBOT_SMALL_DB = 0,	// optimized for small db
		DBOT_DEFAULT,
		DBOT_LOOKUP,		// optimized point lookup, no range scan
	};

	RocksDB(::rocksdb::DB* db, ::rocksdb::ColumnFamilyHandle* cf):_pDB(db),_pCF(cf){}

public:
	RocksDB(){ Empty(); }
	RocksDB(const RocksDB& x){ _pDB = x._pDB; _pCF = x._pCF; }

	bool IsEmpty() const { return _pDB == nullptr; }
	void Empty(){ _pDB = nullptr; _pCF = nullptr; }
	bool Set(const SliceValue& k, const SliceValue& val, const WriteOptions* opt = WriteOptionsDefault){ ASSERT(_pDB); return _pDB->Put(*opt, _pCF, k, val).ok(); }
	bool Merge(const SliceValue& k, const SliceValue& val, const WriteOptions* opt = WriteOptionsDefault){ ASSERT(_pDB); return _pDB->Merge(*opt, _pCF, k, val).ok(); }
	bool Get(const SliceValue& k, std::string& str, const ReadOptions* opt = ReadOptionsDefault) const { ASSERT(_pDB); return _pDB->Get(*opt, _pCF, k, &str).ok(); }
	bool Has(const SliceValue& k, const ReadOptions* opt = ReadOptionsDefault) const { thread_local std::string t; return Get(k, t, opt); }
	template<typename t_POD>
	bool Get(const SliceValue& k, t_POD* valout, const ReadOptions* opt = ReadOptionsDefault) const
	{	ASSERT_NONRECURSIVE;
		thread_local std::string temp;
		ASSERT(_pDB);
		if(_pDB->Get(*opt, _pCF, k, &temp).ok() && temp.length() == sizeof(t_POD))
		{	memcpy(valout, temp.data(), sizeof(t_POD));
			return true;
		}else return false;
	}
	template<typename t_NUM>
	t_NUM GetAs(const SliceValue& k, t_NUM default_val = 0, const ReadOptions* opt = ReadOptionsDefault) const
	{	ASSERT_NONRECURSIVE;
		thread_local std::string temp;
		ASSERT(_pDB);
		return (_pDB->Get(*opt, _pCF, k, &temp).ok() && temp.length() == sizeof(t_NUM))?
			   *((t_NUM*)temp.data()):default_val;
	}
	template<typename t_Type>
	const t_Type* Fetch(const SliceValue& k, SIZE_T* len_out = nullptr, const ReadOptions* opt = ReadOptionsDefault) const // Get a inplace referred buffer, will be invalid after next Fetch
	{	ASSERT_NONRECURSIVE;
		thread_local std::string temp;
		ASSERT(_pDB);
		if(_pDB->Get(*opt, _pCF, k, &temp).ok() && temp.length() >= sizeof(t_Type))
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
		return (_pDB->Get(*opt, _pCF, k, &temp).ok())?
				rt::String_Ref(temp.data(), temp.length()):rt::String_Ref();
	}
	RocksCursor Find(const SliceValue& begin, const ReadOptions* opt = ReadOptionsDefault) const
	{	::rocksdb::Iterator* it = rt::_CastToNonconst(_pDB)->NewIterator(*opt, _pCF);
		ASSERT(it);
		it->Seek(begin);
		return RocksCursor(it);
	}
	RocksCursor First(const ReadOptions* opt = ReadOptionsDefault) const
	{	::rocksdb::Iterator* it = rt::_CastToNonconst(_pDB)->NewIterator(*opt, _pCF);
		ASSERT(it);
		it->SeekToFirst();
		return RocksCursor(it);
	}
	RocksCursor Last(const ReadOptions* opt = ReadOptionsDefault) const
	{	::rocksdb::Iterator* it = rt::_CastToNonconst(_pDB)->NewIterator(*opt, _pCF);
		ASSERT(it);
		it->SeekToLast();
		return RocksCursor(it);
	}
	bool Delete(const SliceValue& k, const WriteOptions* opt = WriteOptionsDefault){ ASSERT(_pDB); return _pDB->Delete(*opt, _pCF, k).ok(); }
	template<typename func_visit>
	SIZE_T ScanBackward(const func_visit& v, const SliceValue& begin, const ReadOptions* opt = ReadOptionsDefault) const
	{	ASSERT(_pDB);
		RocksCursor it = _pDB->NewIterator(*opt, _pCF);
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
		RocksCursor it = _pDB->NewIterator(*opt, _pCF);
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
		RocksCursor it = _pDB->NewIterator(*opt, _pCF);
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
		RocksCursor it = _pDB->NewIterator(*opt, _pCF);
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
		RocksCursor it = _pDB->NewIterator(*opt, _pCF);
		ASSERT(!it.IsEmpty());
		SIZE_T ret = 0;
		for(it.iter->Seek(prefix); it.IsValid() && it.Key().starts_with(prefix); it.Next())
		{	ret++;
			if(!rt::_details::_CallLambda<bool, decltype(v(it))>(true, v, it).retval)
				break;
		}
		return ret;
	}
};

enum RocksStorageScopeWriteRobustness
{								// disableDataSync  use_fsync  allow_os_buffer
	DBWR_LEVEL_FASTEST = 0,		// true				false		true
	DBWR_LEVEL_DEFAULT,			// false			false		true
	DBWR_LEVEL_UNBUFFERED,		// false			false		false
	DBWR_LEVEL_STRONG			// false			true		false
};

namespace _details
{
template<int LEN, bool is_pod>
struct _pod_equal;
template<> struct _pod_equal<1, true>{ static bool is(LPCBYTE x, LPCBYTE y){ return *x == *y; } };
template<> struct _pod_equal<2, true>{ static bool is(LPCBYTE x, LPCBYTE y){ return *(WORD*)x == *(WORD*)y; } };
template<> struct _pod_equal<3, true>{ static bool is(LPCBYTE x, LPCBYTE y){ return *(WORD*)x == *(WORD*)y && _pod_equal<1, true>::is(x+2, y+2); } };
template<> struct _pod_equal<4, true>{ static bool is(LPCBYTE x, LPCBYTE y){ return *(DWORD*)x == *(DWORD*)y; } };
template<> struct _pod_equal<5, true>{ static bool is(LPCBYTE x, LPCBYTE y){ return *(DWORD*)x == *(DWORD*)y && _pod_equal<1, true>::is(x+4, y+4); } };
template<> struct _pod_equal<6, true>{ static bool is(LPCBYTE x, LPCBYTE y){ return *(DWORD*)x == *(DWORD*)y && _pod_equal<2, true>::is(x+4, y+4); } };
template<> struct _pod_equal<7, true>{ static bool is(LPCBYTE x, LPCBYTE y){ return *(DWORD*)x == *(DWORD*)y && _pod_equal<3, true>::is(x+4, y+4); } };
template<> struct _pod_equal<8, true>{ static bool is(LPCBYTE x, LPCBYTE y){ return *(ULONGLONG*)x == *(ULONGLONG*)y; } };
template<int LEN>
struct _pod_equal<LEN, true>{ static bool is(LPCBYTE x, LPCBYTE y){ return *(ULONGLONG*)x == *(ULONGLONG*)y && _pod_equal<LEN-8, true>::is(x+8, y+8); } };

struct _compare
{	template<typename T> static auto 
	INLFUNC with(const T& x, const T& y) -> decltype(x.compare_with(y)) { return x.compare_with(y); }
	template<typename T, typename ... ARGS> static auto 
	INLFUNC with(const T& x, const T& y, ARGS ...args) -> decltype((x < y && x == y)*1) 
			{	if(x<y)return -1;
				if(y<x)return +1;
				return 0;
			}
	template<typename T> static auto 
	INLFUNC equal(const T& x, const T& y) -> decltype(_pod_equal<sizeof(T), rt::TypeTraits<T>::IsPOD>::is((LPCBYTE)&x, (LPCBYTE)&y)) { return _pod_equal<sizeof(T), rt::TypeTraits<T>::IsPOD>::is((LPCBYTE)&x, (LPCBYTE)&y); }
	template<typename T, typename ... ARGS> static auto 
	INLFUNC equal(const T& x, const T& y, ARGS ...args) -> decltype(x == y) { return x == y; }
};
} // namespace _details

class RocksDBOpenOption
{
	::rocksdb::ColumnFamilyOptions	Opt;

public:
	operator const ::rocksdb::ColumnFamilyOptions*() const { return &Opt; }
	operator ::rocksdb::ColumnFamilyOptions*(){ return &Opt; }
	operator const ::rocksdb::ColumnFamilyOptions&() const { return Opt; }
	operator ::rocksdb::ColumnFamilyOptions&(){ return Opt; }

	auto	SetKeyUnordered(UINT cache_size_mb = 10){ Opt.OptimizeForPointLookup(cache_size_mb); return *this; }
	template<class KeyType>
	auto	SetKeyOrder()
			{	struct cmp: public ::rocksdb::Comparator
				{	virtual int Compare(const ::rocksdb::Slice& a, const ::rocksdb::Slice& b) const override
					{	ASSERT(a.size()>=sizeof(KeyType));
						ASSERT(b.size()>=sizeof(KeyType));
						auto& x = *(const KeyType*)a.data();
						auto& y = *(const KeyType*)b.data();
						return _details::_compare::with(x ,y);
					}
					virtual bool Equal(const ::rocksdb::Slice& a, const ::rocksdb::Slice& b) const override 
					{	ASSERT(a.size()>=sizeof(KeyType));
						ASSERT(b.size()>=sizeof(KeyType));
						return _details::_compare::equal(*(const KeyType*)a.data(), *(const KeyType*)b.data());
					}
					rt::String _keytype_name;
					cmp(){ _keytype_name = rt::TypeNameToString<KeyType>(); }
					virtual const char* Name() const { return _keytype_name; }
					virtual void FindShortestSeparator(std::string* start, const ::rocksdb::Slice& limit) const {}
					virtual void FindShortSuccessor(std::string* key) const {}
				};
				static const cmp _cmp;
				Opt.comparator = &_cmp;
				return *this;
			}
};

class RocksStorage
{
	friend class RocksDB;
public:

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

protected:
	::rocksdb::DB*		_pDB;
	struct CFEntry
	{
		::rocksdb::ColumnFamilyHandle*	pCF;
		::rocksdb::ColumnFamilyOptions	Opt;
		CFEntry(){ pCF = nullptr; }
	};
	typedef rt::hash_map<rt::String, CFEntry, rt::String::hash_compare> T_DBS;
	os::ThreadSafeMutable<T_DBS>	_AllDBs;
	::rocksdb::ColumnFamilyOptions	_DefaultOpenOpt;

public:
	RocksStorage(){ _pDB = nullptr; }
	~RocksStorage(){ Close(); }
	bool		Open(LPCSTR db_path, RocksStorageScopeWriteRobustness robustness = DBWR_LEVEL_DEFAULT, bool open_existed_only = false, UINT file_thread_co = 2, UINT logfile_num_max = 1);
	bool		Open(LPCSTR db_path, const Options* opt);
	bool		IsOpen() const { return _pDB!=nullptr; }
	void		Close();
	RocksDB		Get(const rt::String_Ref& name, bool create_auto = true);

	// first ':' in the name will be treated as wild prefix
	// so you can set db_name to "abc:" and all column famlity with db_name starts with "abc:" will be applied the specified options
	void		SetDBOpenOption(LPCSTR db_name, const RocksDBOpenOption& opt); 
	void		SetDBDefaultOpenOption(const RocksDBOpenOption& opt){ _DefaultOpenOpt = opt; }

	static void Nuke(LPCSTR db_path){ os::File::RemovePath(db_path); }
};

class RocksDBStandalone: public RocksDB
{
	RocksStorage	_Storage;
private: 
	RocksDBStandalone(const RocksDB& x);
	void Empty();

public:
	RocksDBStandalone(){ Close(); }
	bool	Open(LPCSTR db_path, RocksStorageScopeWriteRobustness robustness = DBWR_LEVEL_DEFAULT, bool open_existed_only = false, UINT file_thread_co = 2, UINT logfile_num_max = 1);
	bool	IsOpen() const { return _Storage.IsOpen(); }
	void	Close();
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


} // namespace ext

