#include "rocksdb.h"
#include "../../inet/tinyhttpd.h"


namespace ext
{
class RocksDB;

class RocksDBServe: public inet::TinyHttpd
{
	struct RocksDBHandler:public inet::HttpHandler<RocksDBHandler>
	{
		rt::String		Mime;
		rt::String		L1_Path;
		RocksDB*		pDB;
		bool			bBinaryKey;
		auto			GetKey(inet::HttpResponse& resp, const rt::String_Ref& varname, rt::String& ws) -> rt::String_Ref;
		void			SendKey(inet::HttpResponse& resp, const rt::String_Ref& key, rt::String& ws);
		bool			OnRequest(inet::HttpResponse& resp);
		bool			OnRequestList(inet::HttpResponse& resp, bool no_val);
		RocksDBHandler(RocksDB* p):pDB(p){}
	};

	ReadOptions						_ReadOpt;
	WriteOptions					_WriteOpt;
	rt::BufferEx<RocksDBHandler*>	_Endpoints;

public:
	~RocksDBServe();
	void RocksMap(RocksDB* pDB, const rt::String_Ref& L1_path, bool key_is_binary = false, LPCSTR mime = inet::TinyHttpd::MIME_STRING_JSON);
};

} // namespace ext

