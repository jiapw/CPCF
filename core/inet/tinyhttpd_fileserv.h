#include "tinyhttpd.h"
/** \addtogroup inet 
 * @ingroup CPCF
 *  @{
 */
/**
 * @file tinyhttpd_fileserv.h
 * @author JP Wang (wangjiaping@idea.edu.cn)
 * @brief 
 * @version 1.0
 * @date 2021-04-30
 * 
 * @copyright  
 * Cross-Platform Core Foundation (CPCF)
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *      * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials provided
 *        with the distribution.
 *      * Neither the name of CPCF.  nor the names of its
 *        contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *  
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   
 */

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
	typedef rt::hash_map<rt::String_Ref,_FileData*>	t_NameSpace;
	t_NameSpace						_NameSpace;
	FUNC_WebAssetsConvertion		_HttpDataConv;
	LPBYTE	_AddFile(const rt::String_Ref& path, UINT datalen, LPCSTR mime);

public:
	UINT	ImportZipFile(LPCSTR zip_file, LPCSTR path_prefix, UINT fsize_max = 10*1024*1024);  ///< import non-zero sized file only
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
/** @}*/