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

bool RocksStorage::Open(LPCSTR db_path, StorageScopeWriteRobustness robustness, bool open_existed_only, UINT file_thread_co, UINT logfile_num_max)
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
	ASSERT(_pDB == nullptr);
	ASSERT(opt);

	if(opt->create_if_missing)
		os::File::CreateDirectories(db_path, false);

	std::vector<std::string>	cfs;
	if(::rocksdb::DB::ListColumnFamilies(*opt, db_path, &cfs).ok())
	{
		std::vector<ColumnFamilyDescriptor>	cfds;
		std::vector<ColumnFamilyHandle*>	cfptrs;

		cfds.resize(cfs.size());
		cfptrs.resize(cfs.size());

		THREADSAFEMUTABLE_UPDATE(_AllDBs, new_obj);

		for(UINT i=0; i<cfs.size(); i++)
		{
			cfds[i].name = cfs[i];
			cfds[i].options = new_obj->operator[](cfs[i].c_str()).Opt;
		}

		if(::rocksdb::DB::Open(*opt, db_path, cfds, &cfptrs, &_pDB).ok())
		{
			for(UINT i=0; i<cfs.size(); i++)
				new_obj->operator[](cfs[i].c_str()).pCF = cfptrs[i];

			return true;
		}
	}
	else if(::rocksdb::DB::Open(*opt, db_path, &_pDB).ok())
	{
		return true;
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

} // namespace ext
