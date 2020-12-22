#include "tinyhttpd.h"

#if defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
#include <netdb.h>
#endif


namespace inet
{

const LPCSTR TinyHttpd::_MIMEs[19] = 
{
	("application/octet-stream"),		// 0

	("image/bmp"),						// 1
	("image/png"),						// 2
	("text/css"),						// 3
	("image/gif"),						// 4

	("text/html"),						// 5
	("image/x-icon"),					// 6
	("image/jpeg"),						// 7
	("application/x-javascript"),		// 8

	("application/pdf"),				// 9
	("image/svg+xml"),					//10
	("application/x-shockwave-flash"),	//11
	("image/tiff"),						//12

	("text/plain"),						//13
	("x-world/x-vrml"),					//14
	("audio/x-wav"),					//15
	("application/zip"),				//16
	("text/xml"),						//17
	("application/json")				//18
};

const LPCSTR TinyHttpd::MIME_STRING_BINARY =	TinyHttpd::_MIMEs[MIME_BINARY];
const LPCSTR TinyHttpd::MIME_STRING_BMP =		TinyHttpd::_MIMEs[MIME_BMP];
const LPCSTR TinyHttpd::MIME_STRING_PNG =		TinyHttpd::_MIMEs[MIME_PNG];
const LPCSTR TinyHttpd::MIME_STRING_CSS =		TinyHttpd::_MIMEs[MIME_CSS];
const LPCSTR TinyHttpd::MIME_STRING_GIF =		TinyHttpd::_MIMEs[MIME_GIF];
const LPCSTR TinyHttpd::MIME_STRING_HTML =		TinyHttpd::_MIMEs[MIME_HTML];
const LPCSTR TinyHttpd::MIME_STRING_ICON =		TinyHttpd::_MIMEs[MIME_ICON];
const LPCSTR TinyHttpd::MIME_STRING_JPEG =		TinyHttpd::_MIMEs[MIME_JPEG];
const LPCSTR TinyHttpd::MIME_STRING_JS =		TinyHttpd::_MIMEs[MIME_JS];
const LPCSTR TinyHttpd::MIME_STRING_PDF =		TinyHttpd::_MIMEs[MIME_PDF];
const LPCSTR TinyHttpd::MIME_STRING_SVG =		TinyHttpd::_MIMEs[MIME_SVG];
const LPCSTR TinyHttpd::MIME_STRING_SWF =		TinyHttpd::_MIMEs[MIME_SWF];
const LPCSTR TinyHttpd::MIME_STRING_TIFF =		TinyHttpd::_MIMEs[MIME_TIFF];
const LPCSTR TinyHttpd::MIME_STRING_TEXT =		TinyHttpd::_MIMEs[MIME_TEXT];
const LPCSTR TinyHttpd::MIME_STRING_VRML =		TinyHttpd::_MIMEs[MIME_VRML];
const LPCSTR TinyHttpd::MIME_STRING_WAVE =		TinyHttpd::_MIMEs[MIME_WAVE];
const LPCSTR TinyHttpd::MIME_STRING_ZIP =		TinyHttpd::_MIMEs[MIME_ZIP];
const LPCSTR TinyHttpd::MIME_STRING_XML =		TinyHttpd::_MIMEs[MIME_XML];
const LPCSTR TinyHttpd::MIME_STRING_JSON =		TinyHttpd::_MIMEs[MIME_JSON];


}

LPCSTR inet::TinyHttpd::_Ext2MIME(LPCSTR ext, int len)
{
	static const int ext_hash[29] = 
	{5, 18, 13, 14, 15, 3, 0, 13, 17, 12, 0, 4, 8, 6, 0, 0, 10, 0, 16, 7, 0, 2, 0, 0,11, 0, 0, 9, 1};

	static const WORD ext_hash_name[29] = 
	{0x7468, 0x6178, 0x7874, 0x7276, 0x6177, 0x7363, 0x0000, 0x6574, 0x6d78, 0x6974, 0x0000, 0x6967, 0x736a, 0x6369, 
	 0x0000, 0x0000, 0x7673, 0x0000, 0x697a, 0x706a, 0x0000, 0x6e70, 0x0000, 0x0000, 0x7773, 0x0000, 0x0000, 0x6470, 0x6d62
	};

	if(len>=2)
	{
		int a = (ext[0] - 'A')%('a' - 'A') + 'a';
		int b = (ext[1] - 'A')%('a' - 'A') + 'a';
		int h = ((123^a)*b)%29;
		if(ext_hash_name[h] == a + (b<<8))
			return _MIMEs[ext_hash[h]];
	}

	return _MIMEs[0];
}

LPCSTR inet::TinyHttpd::_GetMIME(const rt::String_Ref& filename)
{
	rt::String_Ref ext = filename.GetExtName();
	if(ext.GetLength() && ext[0] == '.')
	{	ext = ext.TrimLeft(1);
		//ext.TruncateLeft(1);
		return _Ext2MIME(ext.Begin(), (int)ext.GetLength());
	}
	return nullptr;
}

inet::TinyHttpd::TinyHttpd(void)
{
	m_IOHangTimeout = 1000;
	m_Concurrency = 0;
	__ConcurrencyCount = 0;
	m_IsConcurrencyRestricted = false;
}

inet::TinyHttpd::~TinyHttpd(void)
{
	Stop();
	_EndPoints.Clear();
}

void inet::TinyHttpd::SetConcurrencyRestricted(bool restricted)
{
	ASSERT(m_ListenThreads.GetSize() == 0); // set before Start
	m_IsConcurrencyRestricted = restricted;
}

bool inet::TinyHttpd::Start(const InetAddr* pBindAddress, int address_count, int concurrency)
{
	ASSERT(0 == m_Listeners.GetSize());
	ASSERT(0 == m_ListenThreads.GetSize());
	if(concurrency == 0)
		concurrency = rt::min(os::GetNumberOfProcessors()*2, 16U);
	
	m_Concurrency = concurrency;
	__ConcurrencyCount = 0;

	if(m_IsConcurrencyRestricted)
		m_Concurrency++;

	VERIFY(m_Listeners.SetSize(address_count));

	UINT socket_ok = 0;
	for(int i=0;i<address_count;i++)
	{
		listening_port& port = m_Listeners[socket_ok];
		port.m_pHttpd = nullptr;
		port.m_Socket = (SOCKET)socket(AF_INET, SOCK_STREAM, 0); //getprotobyname("tcp")->p_proto);
		if(port.m_Socket != INVALID_SOCKET)
		{
			{	int on = 1;
				setsockopt(port.m_Socket, SOL_SOCKET, SO_REUSEADDR, (LPCSTR)&on, sizeof(on));	
			}
			if(	(0 == bind(port.m_Socket, (struct sockaddr*)(pBindAddress + i), sizeof(InetAddr))) &&
				(0 == listen(port.m_Socket, 16))
			)
			{	socket_ok++;
				port.m_pHttpd = this;
				port.m_Address = pBindAddress[i];
			}
			else
			{	Socket().Attach(port.m_Socket);
				port.m_Socket = INVALID_SOCKET;
				_LOG("HTTPD Socket Error = "<<inet::Socket::GetLastError());
			}
		}
		else
		{	ASSERT(0);
		}
	}

	if(0 == socket_ok)
	{	
		m_Listeners.SetSize();
		return false;
	}

	VERIFY(m_Listeners.ChangeSize(socket_ok));

	VERIFY(m_ListenThreads.SetSize(socket_ok*m_Concurrency));
	
	int thread_id = 0;
	for(UINT i=0;i<socket_ok;i++)
	{
		listening_port& port = m_Listeners[i];
		ASSERT(port.m_pHttpd && port.m_Socket != INVALID_SOCKET);
		
		for(UINT th=0; th<m_Concurrency; th++)
		{
			struct _call:public TinyHttpd
			{	static DWORD _func(LPVOID lpParameter)
				{	
					listening_port& p = *((listening_port*)lpParameter);
					p.m_pHttpd->_request_handling_thread(p);
					return 0;
				}
			};

			VERIFY(m_ListenThreads[th + thread_id].Create(_call::_func, &m_Listeners[i]));
		}

		thread_id += m_Concurrency;
	}

	return true;
}

inet::InetAddr inet::TinyHttpd::GetBindedAddress() const
{
	if(m_Listeners.GetSize())
	{
		inet::InetAddr	binded;
		SOCKET_SIZE_T len = sizeof(binded);
		getsockname(m_Listeners[0].m_Socket,(sockaddr*)&binded,&len);
		return binded;
	}
	else return inet::InetAddr();
}

DWORD inet::TinyHttpd::GetBindedPort() const
{
	if(m_Listeners.GetSize())
	{
		struct sockaddr_in sa;
		SOCKET_SIZE_T len = sizeof(sa);
		getsockname(m_Listeners[0].m_Socket,(sockaddr*)&sa,&len);
		return htons(sa.sin_port);
	}
	else return 0;
}

void inet::TinyHttpd::ReplaceEndpoint(LPHTTPENDPOINT ep)
{
	THREADSAFEMUTABLE_UPDATE(_EndPoints, eps);

	t_EndPoints::iterator it = eps->find(ep->GetEndPoint());
	ASSERT(it != eps->end());
	it->second = ep;
}

bool inet::TinyHttpd::SetEndpoints(LPHTTPENDPOINT* ep, UINT count)
{
	THREADSAFEMUTABLE_UPDATE(_EndPoints, neweps);
	neweps.ReadyModify(true);

	for(UINT i=0;i<count;i++)
	{
		auto it = neweps->find(ep[i]->GetEndPoint());
		if(it != neweps->end())
		{
			neweps.Revert();
			return false;  // duplicated endpoint
		}

		neweps->operator[](ep[i]->GetEndPoint()) = ep[i];
	}

	return true;
}

bool inet::TinyHttpd::AddEndpoint(LPHTTPENDPOINT ep)		// httpd will NOT manage the lifecycle of eps
{
	THREADSAFEMUTABLE_UPDATE(_EndPoints, neweps);
	if(neweps->find(ep->GetEndPoint()) != neweps->end())
	{
		neweps.Revert();
		return false;
	}

	neweps->operator[](ep->GetEndPoint()) = ep;
	return true;
}

void ::inet::TinyHttpd::Stop()
{
	if(m_Listeners.GetSize())
	{
		for(UINT i=0; i<m_Listeners.GetSize(); i++)
		{
			Socket h;
			h.Attach(m_Listeners[i].m_Socket);
			m_Listeners[i].m_Socket = INVALID_SOCKET;
			h.Close();
		}

		if(m_ListenThreads.GetSize())
		{
			for(UINT i=0;i<m_ListenThreads.GetSize();i++)
			{	
				if(!m_ListenThreads[i].WaitForEnding(1000))
					m_ListenThreads[i].TerminateForcely();
			}
			m_ListenThreads.SetSize(0);
		}

		m_Listeners.SetSize(0);
	}
}

rt::String_Ref inet::HttpResponse::GetHeaderField(LPCSTR name) const
{
	LPCSTR line;
	if(	!RemainHeader.IsEmpty() &&
		(line = strstr(RemainHeader.Begin(),name))
	)
	{	line += strlen(name);
		while(*line <= ' ' && line < RemainHeader.End())line++;
		LPCSTR end = line;
		while(*end >= ' ' && end < RemainHeader.End())end++;

		return rt::String_Ref(line,end).TrimSpace();
	}

	return rt::String_Ref();
}

bool inet::HttpResponse::ParseRequestRange(ULONGLONG total_size, ULONGLONG* offset, UINT* length) const
{
	rt::String_Ref header = GetHeaderField("Range: bytes=");
	if(header.IsEmpty())return false;

	rt::String_Ref rf[3];
	static const rt::CharacterSet sep("-");
	LONGLONG from = 0, to;
	if(header.Split<false, 0, 0>(rf, 3, sep) == 2)
	{	
		if(!rf[0].IsEmpty())
		{	rf[0].ToNumber(from);
		if(!rf[1].IsEmpty())
			rf[1].ToNumber(to);
		else
			to = total_size - 1;
		}
		else if(!rf[0].IsEmpty())
		{	rf[1].ToNumber(from);
			from = total_size - from;
			to = total_size - 1;
		}

		if(from>=0 && from <= to && to < (LONGLONG)total_size && to - from + 1 < 0xffffffffLL)
		{
			*offset = from;
			*length = (UINT)(to - from + 1);
			return true;
		}
	}

	return false;
}


rt::String_Ref inet::HttpResponse::GetQueryParam(const rt::String_Ref& name)
{
	if(!Query.IsEmpty())
	{
		int offset = (int)name.GetLength()-1;
		while((offset = (int)Query.FindCharacter('=',offset))>=0)
		{
			if( Query.Begin()[offset - name.GetLength() - 1]<'0' &&
				memcmp(name.Begin(),&Query[offset - name.GetLength()],name.GetLength()) == 0
			)
			{	int tail = (int)Query.FindCharacter('&',offset+1);
				if(tail>=0)
				{	return rt::String_Ref(&Query[offset+1],&Query[tail]);
				}
				else
				{	return rt::String_Ref(&Query[offset+1],Query.End());
				}
			}
			offset++;
		}
	}
	
	return rt::String_Ref();
}

rt::String_Ref inet::HttpResponse::GetLnPath(LPCHTTPENDPOINT ep)
{
	if(ep->GetEndPoint().GetLength()>1)
	{
		ASSERT(ep->GetEndPoint().GetLength() <= URI.GetLength());
		return URI.TrimLeft(ep->GetEndPoint().GetLength());
	}
	else return URI;
}

bool inet::HttpResponse::_Send(LPCVOID p, int len)
{
	return _SocketAccepted.Send(p, len); // timed socket will handle fragmentating
}

inet::SOCKET inet::HttpResponse::TakeOver()
{
	return _SocketAccepted.Detach();
}

LPBYTE inet::HttpResponse::GetWorkSpace(UINT sz, bool preserve_existing_content)
{
	if(_Workspace.GetSize() >= sz)return _Workspace;

	if(preserve_existing_content)
	{	if(_Workspace.ChangeSize(sz))return _Workspace;
	}
	else
	{	if(_Workspace.SetSize(sz))return _Workspace;
	}

	return nullptr;
}


void inet::HttpResponse::SendHttpError(int http_status)
{
	static const LPCSTR Error400[] = 
	{	"Bad Request",					//400
		"Unauthorized",					//401
		"Payment Required",				//402
		"Forbidden",					//403
		"Not Found",					//404
		"Method Not Allowed",			//405
		"Not Acceptable",				//406
		"Proxy Authentication Required",//407
		"Request Timeout",				//408
		"Conflict "						//409 	
		//410 Gone 
		//411 Length Required 
		//412 Precondition Failed 
		//413 Request Entity Too Large 
		//414 Request-URI Too Long 
		//415 Unsupported Media Type 
		//416 Requested Range Not Satisfiable 
		//417 Expectation Failed 
		//418 I'm a teapot 
		//422 Unprocessable Entity (WebDAV) (RFC 4918 ) 
		//423 Locked (WebDAV) (RFC 4918 ) 
		//424 Failed Dependency (WebDAV) (RFC 4918 ) 
		//425 Unordered Collection 
		//426 Upgrade Required (RFC 2817 )
		//449 Retry With 
	};
	static const LPCSTR Error500[] = 
	//500
	{	"Internal Server Error",		//500
		"Not Implemented",				//501
		"Bad Gateway",					//502
		"Service Unavailable",			//503
		"Gateway Timeout",				//504
		"HTTP Version Not Supported",	//505
		"Variant Also Negotiates",		//506
		"Insufficient Storage",			//507
		"Unknown Error",				//508
		"Bandwidth Limit Exceeded",		//509
	};

	ASSERT(!_SocketAccepted.IsEmpty());
	char header[256];

	LPCSTR sss = Error500[8];
	if(http_status>=400 && http_status<=409)sss = Error400[http_status - 400];
	if(http_status>=500 && http_status<=509)sss = Error500[http_status - 500];

	static const char response_temp[] = 
	"<html><head><title>%d - %s</title></head><body>"
	"<h2>HTTP Status %d</h2>"
	"<p><strong>%d - %s</strong></p>"
	"</body></html>";

	char msg[256];
	int mlen = sprintf(msg,response_temp,http_status,sss,http_status,http_status,sss);

	int hlen = sprintf(header,	"HTTP/1.1 %d %s\r\n"
								"Connection: close\r\n"
								"Access-Control-Allow-Origin: *\r\n"
								"Cache-Control: no-cache, no-store\r\n"
								"Pragma: no-cache\r\n"
								"Content-Type: %s\r\n"
								"Content-Length: %d\r\n"
								"\r\n",
								http_status,sss,TinyHttpd::_MIMEs[TinyHttpd::MIME_HTML],mlen);
	_Send(header,hlen);
	_Send(msg,mlen);
	_SocketAccepted.Close();
}

void inet::HttpResponse::Send(LPCVOID p, int len, LPCSTR mime, UINT maxage)
{
	ASSERT(!_SocketAccepted.IsEmpty());
	char header[256];
	int hlen = (int)
			  (rt::SS(	"HTTP/1.1 200 OK\r\n"
						"Connection: close\r\n"
						"Access-Control-Allow-Origin: *\r\n"
						"Content-Type: ") + mime + 
						rt::SS("\r\nContent-Length: ") + len + 
						rt::SS("\r\nCache-Control: ")
			  ).CopyTo(header);
	if(maxage)
	{	hlen += (int)(rt::SS("max-age=") + maxage + '\r' + '\n').CopyTo(header+hlen);
	}
	else
	{	hlen += (int)rt::SS("no-cache, no-store\r\nPragma: no-cache\r\n").CopyTo(header+hlen);
	}
		
	header[hlen++] = '\r';
	header[hlen++] = '\n';
	
	_Send(header,hlen);
	if(len)_Send(p,len);
	_SocketAccepted.Close();
}

void inet::HttpResponse::Send(LPCVOID p, int len, LPCSTR mime, ULONGLONG partial_from, ULONGLONG partial_to, ULONGLONG total_size)
{
	ASSERT(!_SocketAccepted.IsEmpty());
	char header[256];
	int hlen = (int)
		(rt::SS(	"HTTP/1.1 200 Partial Content\r\n"
					"Access-Control-Allow-Origin: *\r\n"
					"Connection: close\r\n"
					"Content-Type: ") + mime + 
					rt::SS("\r\nContent-Length: ") + len + 
					rt::SS("Content-Range: bytes ") + partial_from + '-' + partial_to + '/' + total_size + rt::SS("\r\n") + 
					rt::SS("\r\nCache-Control: no-cache, no-store\r\nPragma: no-cache\r\n\r\n")
			).CopyTo(header);

	_Send(header,hlen);
	if(len)_Send(p,len);
	_SocketAccepted.Close();
}


void inet::HttpResponse::SendChuncked_Begin(LPCSTR mime, UINT maxage)
{
	ASSERT(!_SocketAccepted.IsEmpty());
	char header[256];
	int len = (int)
			  (rt::SS(	"HTTP/1.1 200 OK\r\n"
						"Access-Control-Allow-Origin: *\r\n"
						"Connection: close\r\n"
						"Content-Type: ") + mime + 
						rt::SS("\r\nCache-Control: ")
			  ).CopyTo(header);
	if(maxage)
	{	len += (int)(rt::SS("max-age=") + maxage + '\r' + '\n').CopyTo(header+len);
	}
	else
	{	len += (int)rt::SS("no-cache, no-store\r\nPragma: no-cache\r\n").CopyTo(header+len);
	}
		
	header[len++] = '\r';
	header[len++] = '\n';
				   
	_Send(header,len);
}

void inet::HttpResponse::SendJSONP_Begin()
{
	SendChuncked_Begin("application/x-javascript; charset=utf-8");
	rt::String_Ref cb = GetQueryParam("callback");
	if(!cb.IsEmpty())
	{	_JSON_Only = false;
		CHAR buf[TinyHttpd::HTTP_REQUEST_HEADER_SIZEMAX];
		SendChuncked(buf, (int)(cb + '(').CopyTo(buf));
	}
	else
	{	_JSON_Only = true;
	}
	__FirstJsonObject = true;
}

void inet::HttpResponse::SendJSONP_ArrayBegin()
{
	char a = '[';
	SendChuncked(&a,1);
	__FirstJsonObject = true;
}

void inet::HttpResponse::SendJSONP_ArrayEnd()
{
	char a = ']';
	SendChuncked(&a,1);
}

void inet::HttpResponse::SendJSONP_End()
{	
	if(!_JSON_Only)SendChuncked(");", 2);
	SendChuncked_End();
}

void inet::HttpResponse::SendJSONP_Empty()
{
	SendChuncked("{}",2);
}

void inet::HttpResponse::SendRedirection(UINT code, LPCSTR url, int url_len)
{
	ASSERT(code>=300 && code<307);
	ASSERT(code != 300); // not implemented

	static const LPCSTR code300[] = 
	{	"Multiple Choices",				//300
		"Moved Permanently",			//301
		"Found",						//302
		"See Other",					//303
		"Not Modified",					//304
		"Use Proxy",					//305
		"Switch Proxy",					//306
		"Temporary Redirect"			//307
	};

	if(code != HTTP_NOT_MODIFIED)
	{
		ASSERT(url);
		LPCSTR status_str = code300[code%100];

		static const char tail[] = "\r\nContent-Length: 0\r\n\r\n";
		LPSTR buf = (LPSTR)alloca(9 + 3 + 1 + 20 + 12 + url_len + sizeof(tail) + 10);
		int len	=	(int)
					(	rt::String_Ref("HTTP/1.1 ",9) + 
						code + ' ' + status_str + 
						rt::String_Ref("\r\nLocation: ",12) + 
						rt::String_Ref(url,url_len) + 
						rt::String_Ref(tail, sizeof(tail)-1)
					).CopyTo(buf);

		_Send(buf,len);
	}
	else // a w32::inet::HTTP_NOT_MODIFIED header (304)
	{
		static const char response_msg[] = "HTTP/1.1 304 Not Modified\r\n\r\n";
		_Send(response_msg,sizeof(response_msg)-1);
	}

	_SocketAccepted.Close();
}


void inet::HttpResponse::SendChuncked(LPCVOID p, int len)
{
	ASSERT(!_SocketAccepted.IsEmpty());
	_Send(p,len);
}

void inet::HttpResponse::SendChuncked_End()
{
	ASSERT(!_SocketAccepted.IsEmpty());
	_SocketAccepted.Close();
}

inet::TinyHttpd::_Response::_Response()
{
	_RecvBufUsed = 0;
	_RecvBufSize = HTTP_RECV_BLOCK;
	_RecvBuf = (LPSTR)_Malloc32AL(BYTE,_RecvBufSize);
	_RecvBufUsedExpected = 0;
}

void inet::TinyHttpd::_Response::Clear()
{
	_RecvBufUsed = 0;
	_RecvBufUsedExpected = 0;
	_SocketAccepted.Close();
}

inet::TinyHttpd::_Response::~_Response()
{
	_SafeFree32AL(_RecvBuf);
}

bool inet::TinyHttpd::_Response::ExtendRecvBuf()
{
	LPSTR p = _Malloc32AL(char,_RecvBufSize*2+1);  // 1 is for the tailing zero
	if(p)
	{	memcpy(p,_RecvBuf,_RecvBufUsed);
		_RecvBufSize = _RecvBufSize*2;
		_SafeFree32AL(_RecvBuf);
		_RecvBuf = p;
		return true;
	}
	else return false;
}

UINT inet::TinyHttpd::_Response::_ConvertToUTF8(LPSTR pInOut, UINT len)
{
	UINT close = 0;
	for(UINT open = 0;open<len;open++)
	{
		if(pInOut[open] != '%')
			pInOut[close++] = pInOut[open];
		else
		{
			pInOut[close++] = (pInOut[open+2]<='9'?pInOut[open+2]-'0':(pInOut[open+2]-'A'+10)) |
							  ((pInOut[open+1]<='9'?pInOut[open+1]-'0':(pInOut[open+1]-'A'+10))<<4);
			open+=2;
		}
	}
	pInOut[close] = '\0';

	return close;
}

UINT inet::TinyHttpd::_Response::OnDataRecv(int newly_received_size)
{

	if(_RecvBufUsedExpected) // continue partial request content body
	{
		if(_RecvBufUsed >= _RecvBufUsedExpected)
		{
			goto REQUEST_DATA_IS_READY;
		}
		else
			return HTTP_FALSE;
	}
	else	// start from the beginning
	{	if(_RecvBufUsed > 4)
		{
			if(	*((UINT*)_RecvBuf) == HTTP_GET ||	// 'GET '
				*((UINT*)_RecvBuf) == HTTP_POST		// 'POST'
			){}	
			else return HTTP_FAIL; // only GET/POST supported
		}
		else return HTTP_FALSE; // too short, nothing to do with

		HttpVerb = *((UINT*)_RecvBuf);
		
		UINT i = 0;
		if(newly_received_size + 3 < (int)_RecvBufUsed)
			i = _RecvBufUsed - (newly_received_size + 3);

		static const UINT end_tag = 0x0a0d0a0d; //MAKEFOURCC('\r','\n','\r','\n');
		// scan for end-tag
		for(;(i+3)<_RecvBufUsed;i++)
		{
			if(*((DWORD*)&_RecvBuf[i]) == end_tag)
			{
				_RecvBuf[i + 2] = '\0';

				i += 4;
				_RequestHeaderSize = i;

				LPSTR http,tail;
				// check content length
				int content_length = 0;
				if((http = strstr((LPSTR)_RecvBuf,"Content-Length: ")) &&
					(tail = strchr(http,'\r'))
				)
				{	*tail = '\0';
					content_length = atoi(http + 16);
					*tail = '\r';
					if(content_length > HTTP_REQUEST_SIZEMAX) // post data too large
						return HTTP_OUTOFMEMORY;

					ASSERT(_RecvBufUsedExpected == 0);
					_RecvBufUsedExpected = content_length + _RequestHeaderSize;

					if(_RecvBufUsed >= _RecvBufUsedExpected)
						goto REQUEST_DATA_IS_READY;
					else
						return HTTP_FALSE;
				}
				else
				{	_RecvBufUsedExpected = _RequestHeaderSize;
					goto REQUEST_DATA_IS_READY;
				}
			}
		}

		if(_RecvBufUsed < HTTP_REQUEST_SIZEMAX)
			return HTTP_FALSE; // receive more data
		else
			return HTTP_FAIL; // too much data received, yet HTTP header is still not ended
	}

	ASSERT(0);

REQUEST_DATA_IS_READY:

	ASSERT(_RequestHeaderSize);
	ASSERT(_RecvBufUsedExpected >= _RequestHeaderSize);
	ASSERT(_RecvBufUsed >= _RecvBufUsedExpected);

	UINT hlen = _RequestHeaderSize;
	// Parse request header
	LPSTR hdr = (LPSTR)&_RecvBuf[0];
	LPSTR tail;
	LPSTR http;
	if(	(hlen > 4+1+4+3+4) &&
		(tail = strchr(hdr+5,'\r')) &&
		(*tail = '\0',true) &&
		(http = strstr(hdr+5,"HTTP/1.")) &&
		(http[-1] == ' ')
	){}
	else return HTTP_INVALIDARG; // illegal header

	http[-1] = '\0';
	
	{	LPCSTR pURI = strchr(hdr,' ') + 1;	//UTF-8
		UINT URI_len = _ConvertToUTF8((LPSTR)pURI,(UINT)(http - pURI - 1));
		URI = rt::String_Ref(pURI, URI_len);
		{	// parse '?'
			LPSTR pQ = (LPSTR)strchr(pURI,'?');
			if(pQ)
			{
				*pQ = '\0';
				int uri_len = (int)(pQ - pURI);
				Query = rt::String_Ref(pQ + 1, URI_len - uri_len - 1);
				URI = rt::String_Ref(pURI, uri_len);
			}
			else
			{
				Query.Empty();
			}
		}
	}
	RemainHeader = rt::String_Ref(tail+2, hlen - (int)((LPSTR)tail+2-hdr) - 2);

	Body = rt::String_Ref(&_RecvBuf[_RequestHeaderSize], &_RecvBuf[_RecvBufUsedExpected]);
	_RecvBuf[_RecvBufUsedExpected] = 0;

	// no range allowed
	// if(strstr(RemainHeader.Begin(),"Range: "))return HTTP_FAIL;

	return HTTP_OK;
}

inet::HttpChunckedXMLWriter::HttpChunckedXMLWriter(inet::HttpResponse& r)
	:_Response(r)
{	
	_BufUsed = 0;
	_Response.SendChuncked_Begin(TinyHttpd::_MIMEs[TinyHttpd::MIME_XML]);
}

inet::HttpChunckedXMLWriter::~HttpChunckedXMLWriter()
{
	if(_BufUsed>0)
		_Response.SendChuncked(_Buf,_BufUsed);
	_Response.SendChuncked_End();
}

void inet::HttpChunckedXMLWriter::Write(LPCVOID ppp, int len)
{
	LPCBYTE p = (LPCBYTE)ppp;
	while(len >= sizeof(_Buf))
	{	_Response.SendChuncked(p,sizeof(_Buf));
		len -= sizeof(_Buf);
	}

	if(_BufUsed + len <= sizeof(_Buf))
	{
		memcpy(_Buf+_BufUsed,p,len);
		_BufUsed+=len;
	}
	else
	{
		_Response.SendChuncked(_Buf,_BufUsed);
		memcpy(_Buf,p,len);
		_BufUsed = len;
	}
}

UINT inet::TinyHttpd::GetBindedAddresses(inet::InetAddr* pOut, UINT OutLen)
{
	OutLen = rt::min(OutLen, (UINT)m_Listeners.GetSize());
	for(UINT i=0;i<OutLen;i++)
		pOut[i] = m_Listeners[i].m_Address;

	return OutLen;
}

bool inet::TinyHttpd::Start(int port, int concurrency)
{
	inet::InetAddr	addrs[256];
	UINT count = inet::GetLocalAddresses(addrs, sizeofArray(addrs), true);
	if(count == 0)
	{
		count = 1;
		addrs[0].SetAsLoopback();
	}
	for(UINT i=0;i<count;i++)
		addrs[i].SetPort(port);

	return Start(addrs, count, concurrency);
}

void inet::TinyHttpd::_request_handling_thread(listening_port& listener)
{
	_Response	response;
	while(listener.m_Socket != INVALID_SOCKET)
	{
WAIT_NEXT_REQUEST:
		// wait for incoming request
		SOCKET_SIZE_T sin_size = sizeof(response._RemoteName);
		response.Clear();
		SOCKET accepted_socket = (SOCKET)accept(listener.m_Socket, (struct sockaddr*)&response._RemoteName, &sin_size);

		if(listener.m_Socket == INVALID_SOCKET)return;
		if(accepted_socket == INVALID_SOCKET)continue;

		response._SocketAccepted.Attach(accepted_socket);
		response._SocketAccepted.SetTimeout(m_IOHangTimeout);
		if(response._SocketAccepted.IsEmpty())
		{	os::Sleep(50);
			continue;
		}
		// receiving request and close
		for(;;)
		{
			if(response.GetBufRemain() < HTTP_RECV_BLOCK)response.ExtendRecvBuf();
			UINT recv_len = 0;
			response._SocketAccepted.Recv(response._RecvBuf + response._RecvBufUsed,response.GetBufRemain(),recv_len);

			if(recv_len>0)
			{	response._RecvBufUsed += recv_len;
				UINT ret = response.OnDataRecv(recv_len);
				if(ret == HTTP_OK)goto REQUEST_IS_READY;
				else if(ret == HTTP_FALSE)continue;
			}
		
			response._SocketAccepted.Close();
			os::Sleep(50);
			goto WAIT_NEXT_REQUEST;
		}

REQUEST_IS_READY:
		if(m_IsConcurrencyRestricted)
		{
			if(os::AtomicIncrement(&__ConcurrencyCount)>=(int)m_Concurrency)
			{
				os::AtomicDecrement(&__ConcurrencyCount);
				response.SendHttpError(503);
				response._SocketAccepted.Close();
				continue;
			}
		}

		// handling request
		UINT len = 1;
		while(response.URI[len] != '/' && len < response.URI.GetLength())
			len++;

		if(!_EndPoints.IsEmpty())
		{	
			auto& EndPoints = _EndPoints.Get();
			auto p = EndPoints.find(rt::String_Ref(response.URI.Begin(),len));

			if(p == EndPoints.end())
				p = EndPoints.find(rt::String_Ref("/",1));

			if(p!=EndPoints.end())
			{	
				bool ret = p->second->HandleRequest(&response);			
				if(!ret)response.SendHttpError(501);
			}
			else
			{	
				response.SendHttpError(404);
			}
		}
		else
		{	response.SendHttpError(404);
		}

		if(m_IsConcurrencyRestricted)
			os::AtomicDecrement(&__ConcurrencyCount);

		response._SocketAccepted.Close();
	}
}

bool inet::HttpRequestEcho::OnRequest(HttpResponse& resp)
{
	resp.SendChuncked_Begin(TinyHttpd::_MIMEs[TinyHttpd::MIME_TEXT]);
		resp.SendChuncked(rt::SS("URL: "));
		resp.SendChuncked(resp.URI);
		resp.SendChuncked(rt::SS("\nQuery: "));
		if(!resp.Query.IsEmpty())resp.SendChuncked(resp.Query);
		if(!resp.Body.IsEmpty())
			resp.SendChuncked(rt::SS("\nPostData :") + resp.Body.GetLength() + 'b');
		resp.SendChuncked(rt::SS("\n\n** Rest of HTTP Header **\n"));
		resp.SendChuncked(resp.RemainHeader);
	resp.SendChuncked_End();

	return true;
}

