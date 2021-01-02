#include "tinyhttpd.h"

namespace inet
{


class HttpServerFiles:public HttpHandler<HttpServerFiles>
{
public:
#pragma pack(push, 1)
	struct _FileData
	{
		rt::String_Ref	uri;
		UINT			datasize;
		rt::String_Ref	mime;
		BYTE			data[1];
		LPCBYTE			GetPayload() const { return data + uri.GetLength() + 1; }
		int				GetPayloadSize() const { return datasize - (int)uri.GetLength() - 1; }
	};
#pragma pack(pop)

protected:
	//typedef rt::hash_map<rt::String_Ref,_FileData*,rt::String_Ref::hash_compare>	t_NameSpace;
	typedef std::map<rt::String_Ref,_FileData*>	t_NameSpace;
	t_NameSpace						_NameSpace;
	FUNC_WebAssetsConvertion		_HttpDataConv;
	LPBYTE	_AddFile(const rt::String_Ref& path, UINT datalen, LPCSTR mime);

public:
	UINT	ImportZipFile(LPCSTR zip_file, LPCSTR path_prefix, UINT fsize_max = 10*1024*1024);  // import non-zero sized file only
	void	AddFile(const rt::String_Ref& path, LPCVOID pdata, UINT datalen, LPCSTR mime);
	bool	OnRequest(HttpResponse& resp);
	void	SendResponse(HttpResponse& resp, const rt::String_Ref& path);
	void	SetDataConversion(FUNC_WebAssetsConvertion p){ _HttpDataConv = p; }
	void	RemoveAllFiles();
	HttpServerFiles();
	~HttpServerFiles();
};

class HttpVirtualPath:public HttpHandler<HttpVirtualPath>
{
	static const SIZE_T		MAX_FILELOAD_SIZE = 1024*1024*1024;
protected:
	FUNC_WebAssetsConvertion	_HttpDataConv;
	rt::String					_MappedPath;
	int							_MaxAge;
	bool						_ReadOnly;
public:
	HttpVirtualPath();
	const rt::String_Ref GetMappedPath() const { return _MappedPath; }
	bool	OnRequest(HttpResponse& resp);
	void	SetCacheControl(int maxage_sec){ _MaxAge = maxage_sec; }
	void	SetMappedPath(LPCSTR p, bool readonly = true);
	void	SetDataConversion(FUNC_WebAssetsConvertion p){ _HttpDataConv = p; }
};



} // namespace inet