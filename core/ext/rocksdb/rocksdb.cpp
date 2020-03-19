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

bool RocksDB::Open(LPCSTR db_path, DBScopeWriteRobustness robustness, bool open_existed_only, UINT file_thread_co, UINT logfile_num_max)
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

bool RocksDB::Open(LPCSTR db_path, const Options* opt)
{
	ASSERT(_pDB == nullptr);
	ASSERT(opt);

	if(opt->create_if_missing)os::File::CreateDirectories(db_path, false);

	::rocksdb::DB* p;
	if(::rocksdb::DB::Open(*opt, db_path, &p).ok())
	{
		_pDB = p;
		return true; 
	}

	return false;
}



} // namespace ext
