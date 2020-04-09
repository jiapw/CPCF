#include "../../os/platform.h"
#include "rocksdb.inc"  // should come before "rocksdb.h"
#include "rocksdb.h"

namespace ext
{

const WriteOptions*	RocksDB::WriteOptionsFastRisky = nullptr;
const WriteOptions*	RocksDB::WriteOptionsDefault = nullptr;
const WriteOptions*	RocksDB::WriteOptionsRobust = nullptr;
const ReadOptions*	RocksDB::ReadOptionsDefault = nullptr;

namespace _details
{

struct __InitRocksDBOptions
{
	struct WriteOptionsMore: public rocksdb::WriteOptions
	{	WriteOptionsMore(int mode)
		{	switch(mode)
			{
			case 0: sync = false;	disableWAL = false; break;
			case 1:	sync = false;	disableWAL = true; break;
			case 2:	sync = true;	disableWAL = false; break;
			}
		}
	};

	__InitRocksDBOptions()
	{
		static const WriteOptionsMore	_WriteOptionsDefault = 0;
		static const WriteOptionsMore	_WriteOptionsFastRisky = 1;
		static const WriteOptionsMore	_WriteOptionsRobust = 2;
		
		RocksDB::WriteOptionsFastRisky = &_WriteOptionsFastRisky;
		RocksDB::WriteOptionsDefault = &_WriteOptionsDefault;
		RocksDB::WriteOptionsRobust = &_WriteOptionsRobust;

		static const ReadOptions	_ReadOptionsDefault;
		RocksDB::ReadOptionsDefault = &_ReadOptionsDefault;
	}
};

__InitRocksDBOptions	g_InitRocksDBOptions;

} // namespace _details

void RocksStorage::SetDBOpenOption(LPCSTR db_name, const ColumnFamilyOptions& opt)
{
	THREADSAFEMUTABLE_UPDATE(_AllDBs, new_db);
	auto& e = new_db->operator[](db_name);
	ASSERT(e.pCF == nullptr);
	e.Opt = opt;
}

bool RocksStorage::Open(LPCSTR db_path, RocksStorageScopeWriteRobustness robustness, bool open_existed_only, UINT file_thread_co, UINT logfile_num_max)
{
	ASSERT(_pDB == nullptr);

	::rocksdb::Options opt;
	opt.create_if_missing = !open_existed_only;
	opt.max_file_opening_threads = file_thread_co;
	opt.keep_log_file_num = logfile_num_max;

	switch(robustness)
	{	
	case DBWR_LEVEL_FASTEST:
		opt.disableDataSync = true;
		opt.use_fsync = false;
		opt.allow_os_buffer = true;
		break;
	case DBWR_LEVEL_DEFAULT:		
		opt.disableDataSync = false;
		opt.use_fsync = false;
		opt.allow_os_buffer = true;
		break;
	case DBWR_LEVEL_UNBUFFERED:
		opt.disableDataSync = false;
		opt.use_fsync = false;
		opt.allow_os_buffer = false;
		break;
	case DBWR_LEVEL_STRONG:
		opt.disableDataSync = false;
		opt.use_fsync = true;
		opt.allow_os_buffer = false;
		break;
	default:
		ASSERT(0);
		return false;
	}

	return Open(db_path, &opt);
}

bool RocksStorage::Open(LPCSTR db_path, const Options* opt)
{
	LPCSTR default_cfname = "default";

	ASSERT(_pDB == nullptr);
	ASSERT(opt);

	if(opt->create_if_missing)
		os::File::CreateDirectories(db_path, false);

	std::vector<std::string>	cfs;
	if(!::rocksdb::DB::ListColumnFamilies(*opt, db_path, &cfs).ok())
	{
		if(!opt->create_if_missing)return false;
		cfs.push_back(default_cfname);
	}

	{
		std::vector<ColumnFamilyDescriptor>	cfds;
		std::vector<ColumnFamilyHandle*>	cfptrs;

		cfds.resize(cfs.size());
		cfptrs.resize(cfs.size());

		THREADSAFEMUTABLE_UPDATE(_AllDBs, new_obj);

		bool drop_default = cfs.size() > 1;
		if(drop_default && _AllDBs.Get().find(default_cfname) != _AllDBs.Get().end())
			drop_default = false;

		for(UINT i=0; i<cfs.size(); i++)
		{
			cfds[i].name = cfs[i];
			auto it = new_obj->find(cfs[i].c_str());
			if(it == new_obj->end())
			{
				rt::String_Ref name(cfs[i]);
				int pos;
				if((pos = (int)name.FindCharacter(':')) > 0)
				{	auto wild_it = new_obj->find(ALLOCA_C_STRING(name.SubStr(pos+1)));
					if(wild_it != new_obj->end())
					{
						cfds[i].options = new_obj->operator[](cfs[i].c_str()).Opt = wild_it->second.Opt;
						continue;
					}
				}
			}

			cfds[i].options = new_obj->operator[](cfs[i].c_str()).Opt;
		}

		if(::rocksdb::DB::Open(*opt, db_path, cfds, &cfptrs, &_pDB).ok())
		{
			for(UINT i=0; i<cfs.size(); i++)
			{	
				if(drop_default && cfs[i] == default_cfname)
				{
					_pDB->DropColumnFamily(cfptrs[i]);
					_pDB->DestroyColumnFamilyHandle(cfptrs[i]);
					continue;
				}
				
				new_obj->operator[](cfs[i].c_str()).pCF = cfptrs[i];
			}

			if(drop_default)
				new_obj->erase(default_cfname);

			return true;
		}
	}

	return false;
}

RocksDB	RocksStorage::Get(const rt::String_Ref& name, bool create_auto)
{
	ASSERT(IsOpen());
	THREADSAFEMUTABLE_UPDATE(_AllDBs, new_obj);
	auto& db = _AllDBs.Get();

	LPCSTR sname = ALLOCA_C_STRING(name);
	auto it = db.find(sname);
	if(it != db.end())
	{
		if(it->second.pCF)
			return RocksDB(_pDB, it->second.pCF);
	}

	if(create_auto)
	{
		auto it = new_obj->find(sname);
		if(it == new_obj->end())
		{
			rt::String_Ref ss(sname);
			int pos = (int)ss.FindCharacter(':');
			auto wild_it = db.find(ss.SubStr(0, pos+1));
			if(wild_it != db.end())
				new_obj->operator[](sname).Opt = wild_it->second.Opt;
		}

		auto& cfe = new_obj->operator[](sname);
		if(_pDB->CreateColumnFamily(cfe.Opt, sname, &cfe.pCF).ok())
			return RocksDB(_pDB, cfe.pCF);
	}

	new_obj.Revert();
	return RocksDB();
}


void RocksStorage::Close()
{
	if(_pDB)
	{
		{	THREADSAFEMUTABLE_UPDATE(_AllDBs, new_obj);
			for(auto it : _AllDBs.Get())
				if(it.second.pCF)
				{
					_pDB->DestroyColumnFamilyHandle(it.second.pCF);
					it.second.pCF = nullptr;
				}

			_AllDBs.Clear();
			delete _pDB;
			_pDB = nullptr;
		}
	}
}

bool RocksDBStandalone::Open(LPCSTR db_path, RocksStorageScopeWriteRobustness robustness, bool open_existed_only, UINT file_thread_co, UINT logfile_num_max)
{
	if(_Storage.Open(db_path, robustness, open_existed_only, file_thread_co, logfile_num_max))
	{
		auto ret = _Storage.Get("default");
		ASSERT(!ret.IsEmpty());
		_pCF = ret._pCF;
		_pDB = ret._pDB;

		return true;
	}

	return false;
}

void RocksDBStandalone::Close()
{
	_Storage.Close();
	RocksDB::Empty();
}


} // namespace ext
