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

public:
	INLFUNC RocksDB(){ _pDB = nullptr; }
	INLFUNC ~RocksDB(){ Close(); }
	INLFUNC bool Open(LPCSTR db_path, bool open_existed_only = false, const Options* opt = nullptr)
	{	ASSERT(_pDB == nullptr);
		if(opt == nullptr)
		{	::rocksdb::Options* my;
			opt = my = _StackNew(Options)();
			my->create_if_missing = !open_existed_only;
		}
		if(!open_existed_only)os::File::CreateDirectories(db_path, false);
		::rocksdb::DB* p;
		if(::rocksdb::DB::Open(*opt, db_path, &p).ok())
		{	_pDB = p;	return true; }
		else return false;
	}
	INLFUNC bool IsOpen() const { return _pDB!=nullptr; }
	INLFUNC void Close(){ if(_pDB){ delete _pDB; _pDB = nullptr; } }
	INLFUNC bool Set(const SliceValue& k, const SliceValue& val, const WriteOptions* opt = WriteOptionsDefault){ ASSERT(_pDB); return _pDB->Put(*opt, k, val).ok(); }
	INLFUNC bool Merge(const SliceValue& k, const SliceValue& val, const WriteOptions* opt = WriteOptionsDefault){ ASSERT(_pDB); return _pDB->Merge(*opt, k, val).ok(); }
	INLFUNC bool Get(const SliceValue& k, std::string& str, const ReadOptions* opt = ReadOptionsDefault) const { ASSERT(_pDB); return _pDB->Get(*opt, k, &str).ok(); }
	INLFUNC bool Has(const SliceValue& k, const ReadOptions* opt = ReadOptionsDefault) const { thread_local std::string t; return Get(k, t, opt); }
	template<typename t_POD>
	INLFUNC bool Get(const SliceValue& k, t_POD* valout, const ReadOptions* opt = ReadOptionsDefault) const
	{	ASSERT_NONRECURSIVE;
		thread_local std::string temp;
		ASSERT(_pDB);
		if(_pDB->Get(*opt, k, &temp).ok() && temp.length() == sizeof(t_POD))
		{	memcpy(valout, temp.data(), sizeof(t_POD));
			return true;
		}else return false;
	}
	template<typename t_NUM>
	INLFUNC t_NUM GetAs(const SliceValue& k, t_NUM default_val = 0, const ReadOptions* opt = ReadOptionsDefault) const
	{	ASSERT_NONRECURSIVE;
		thread_local std::string temp;
		ASSERT(_pDB);
		return (_pDB->Get(*opt, k, &temp).ok() && temp.length() == sizeof(t_NUM))?
			   *((t_NUM*)temp.data()):default_val;
	}
	template<typename t_Type>
	INLFUNC const t_Type* Fetch(const SliceValue& k, SIZE_T* len_out = nullptr, const ReadOptions* opt = ReadOptionsDefault) const // Get a inplace referred buffer, will be invalid after next Fetch
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
	INLFUNC rt::String_Ref Fetch(const SliceValue& k, const ReadOptions* opt = ReadOptionsDefault) const
	{	ASSERT_NONRECURSIVE;
		thread_local std::string temp;
		ASSERT(_pDB);
		return (_pDB->Get(*opt, k, &temp).ok())?
				rt::String_Ref(temp.data(), temp.length()):rt::String_Ref();
	}
	INLFUNC RocksCursor Find(const SliceValue& begin, const ReadOptions* opt = ReadOptionsDefault)
	{	::rocksdb::Iterator* it = _pDB->NewIterator(*opt);
		ASSERT(it);
		it->Seek(begin);
		return RocksCursor(it);
	}
	INLFUNC RocksCursor First(const ReadOptions* opt = ReadOptionsDefault)
	{	::rocksdb::Iterator* it = _pDB->NewIterator(*opt);
		ASSERT(it);
		it->SeekToFirst();
		return RocksCursor(it);
	}
	INLFUNC RocksCursor Last(const ReadOptions* opt = ReadOptionsDefault)
	{	::rocksdb::Iterator* it = _pDB->NewIterator(*opt);
		ASSERT(it);
		it->SeekToLast();
		return RocksCursor(it);
	}
	INLFUNC bool Delete(const SliceValue& k, const WriteOptions* opt = WriteOptionsDefault){ ASSERT(_pDB); return _pDB->Delete(*opt, k).ok(); }
	template<typename func_visit>
	INLFUNC SIZE_T ScanBackward(const func_visit& v, const SliceValue& begin, const ReadOptions* opt = ReadOptionsDefault) const
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
	INLFUNC SIZE_T ScanBackward(const func_visit& v, const ReadOptions* opt = ReadOptionsDefault) const
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
	INLFUNC SIZE_T Scan(const func_visit& v, const SliceValue& begin, const ReadOptions* opt = ReadOptionsDefault) const
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
	INLFUNC SIZE_T Scan(const func_visit& v, const ReadOptions* opt = ReadOptionsDefault) const
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
	INLFUNC SIZE_T ScanPrefix(const func_visit& v, const SliceValue& prefix, const ReadOptions* opt = ReadOptionsDefault) const
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

} // namespace ext

