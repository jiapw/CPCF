#include "../../core/rt/string_type.h"
#include "../../core/os/file_dir.h"
#include "../../core/inet/http_client.h"
#include "../../core/inet/tinyhttpd.h"
#include "../../core/inet/tinyhttpd_fileserv.h"
#include "test.h"



bool http_cb(LPVOID param, UINT msg, LPVOID cookie)
{
	switch(msg)
	{
	case inet::HTTPCLIENT_EVENT_URL_PARSED:		_LOG("// Request: "<<(LPCSTR)param); break;
	case inet::HTTPCLIENT_EVENT_DNS_RESOLVED:	_LOG("// DNS resolved"); break;
	case inet::HTTPCLIENT_EVENT_CONNECTED:		_LOG("// Connected to "<<(LPCSTR)param); break;
	case inet::HTTPCLIENT_EVENT_TLS_CONNECTED:	_LOG("// TLS Connected"); break;
	case inet::HTTPCLIENT_EVENT_FIRSTBYTE:		_LOG("// Server responsed"); break;
	case inet::HTTPCLIENT_EVENT_HEADER_RECEIVED:
		{	inet::HttpSession::ResponseHeader* h = ((inet::HttpSession::ResponseHeader*)param);
			if(h->IsParsedOk())
			{	_LOG("// Header received");
				_LOG(h->m_RawHeader);
			}
			else
			{	_LOG("// Header received, corrupted");
			}
			break;
		}
	case inet::HTTPCLIENT_EVENT_CONTENT_RECEIVING:
		{	int prog = (int)(SIZE_T)param;
			rt::tos::FileSize<true> pp(prog);
			//_LOG("// Content received: "<<pp);
			break;
		}
	case inet::HTTPCLIENT_EVENT_DONE:
		{	int size = (int)(SIZE_T)param;
			rt::tos::FileSize<true> pp(size);
			_LOG("// Completed, total "<<pp);
			break;
		}
	case inet::HTTPCLIENT_EVENT_PROTOCAL_ERROR: 
		_LOG("// Error occured, abort!!"); break;
	case inet::HTTPCLIENT_EVENT_REDIRECT:
		_LOG("// Redirect to "<<(LPCSTR)param); break;
	case inet::HTTPCLIENT_EVENT_TIMEOUT:
		_LOG("// Timeout."); break;
	}
	return true;
}

void rt::UnitTests::download()
{
	inet::HttpDownloader	dlc;
	dlc.SetItemEventCallback(http_cb, nullptr);
	//dlc.SetTask("http://gameswalls.com/shin-megami-tensei-digital-devil-saga-2/shin-megami-tensei/1024x768", "test.html", true);
	//dlc.SetTask("http://wallfive.com/wallpaper/games", "test.html", nullptr, true);
	dlc.SetTask("http://zhuanlan.zhihu.com/p/150789618?utm_source=wechat_session/", "test.html", nullptr, true);
	dlc.Start();
	while(dlc.GetState() < inet::HTTP_DLC_STOPPED)
	{	os::Sleep(500);
		_LOG("Downloaded: "<<dlc.GetProgress()/10.0f<<"%      B/W:"<<rt::tos::FileSize<>((UINT)dlc.GetBandwidth())<<"/sec");
	}
}

os::File	http_data_save;
int http_data_cb(LPCBYTE data, UINT data_len, UINT start_pos, bool fin, LPVOID cookie)
{
	if(fin || data_len > 2*1024)
	{
		//_LOG("WR: "<<data_len);
		VERIFY(data_len == http_data_save.Write(data, data_len));
		return data_len;
	}

	return 0;
}

void rt::UnitTests::http_client()
{
	// Testing HTTPS
	{	inet::HttpSession	http;
		http.SetHangingTimeout(2000);
		http.SetItemEventCallback(http_cb, nullptr);

		//http.SetProxy(inet::InetAddr("213.181.73.145",8080));
		//http.SetProxy(inet::InetAddr("122.96.59.103",82));
		//http.Request_Get("http://www.wallcoo.net/anime/illustration_CG_Girls_Collection_Artbook/index.html");
		//http.SetResponseTimeout(5000);

		LPCSTR url = "https://www.baidu.com";
		//LPCSTR url = "https://www.google.de";
		//LPCSTR url = "https://itunes.apple.com/us/app/facebook/id284882215?mt=8";
		//LPCSTR url = "http://www.flash-screen.com/free-wallpaper/fractal-wallpapers/sea-shells-wallpaper,1920x1080,24487.jpg";

		if(	http.Request_Get(url) &&
			http.WaitResponse()
		)
		{
			os::File("https.txt", os::File::Normal_Write).Write(http.GetResponse(), http.GetResponseLength());
		}
		else
		{	_LOG("Failed");
		}
	
		_LOG("DNS: "<<rt::tos::TimeSpan<false>(http.m_Timing_DNS));
		_LOG("Connecting: "<<rt::tos::TimeSpan<false>(http.m_Timing_Connection));
		_LOG("Waiting: "<<rt::tos::TimeSpan<false>(http.m_Timing_Wait));
		_LOG("Transmision: "<<rt::tos::TimeSpan<false>(http.m_Timing_Transmission));

		http_data_save.Open("https_streamed.txt", os::File::Normal_Write);
		http.SetDataCallback(http_data_cb, nullptr);

		if(	http.Request_Get(url) &&
			http.WaitResponse()
		){}
		else
		{	_LOG("Failed");
		}

		http_data_save.Close();
	
		_LOG("DNS: "<<rt::tos::TimeSpan<false>(http.m_Timing_DNS));
		_LOG("Connecting: "<<rt::tos::TimeSpan<false>(http.m_Timing_Connection));
		_LOG("Waiting: "<<rt::tos::TimeSpan<false>(http.m_Timing_Wait));
		_LOG("Transmision: "<<rt::tos::TimeSpan<false>(http.m_Timing_Transmission));
	}


	// Testing HTTP
	//LPCSTR url = "http://wallfive.com/wallpaper/games";
	//LPCSTR ext = ".htm";
	LPCSTR url = "http://jiapingwang.com/internal/sunset.jpg";
	LPCSTR ext = ".jpg";

	{	inet::HttpSession http;
		http.SetDataCallback(http_data_cb, nullptr);
		
		for (int i = 0; i < 4; i++)
		{
			if(http_data_save.Open(rt::String_Ref("test_") + i + ext, os::File::Normal_Write))
			{
				if (http.Request_Get(url) &&
					http.WaitResponse() &&
					http.GetResponseParsedHeader().m_StateCode == inet::HTTP_OK
				)
				{	_LOG(http.GetResponseLength() << 
						" CHUNCKED:" << http.GetResponseParsedHeader().m_ChunkedTransfer << 
						" C-Len:" <<http.GetResponseParsedHeader().m_ContentLength<<
						" Total: "<<http.GetResponseLengthTotal()<<
						" Memory: "<<http.GetInternalBufferSize()
					);
				}
				else
				{ 
					_LOG("Failed");
				}

				http_data_save.Close();
				os::Sleep(1000);
			}
			else
			{	_LOG("Failed to open saving file");
			}
		}
	}

	{	inet::HttpSession http;

		//http.SetItemEventCallback(http_cb, nullptr);
		for (int i = 4; i < 8; i++)
		{
			if (http.Request_Get(url) &&
				http.WaitResponse() &&
				http.GetResponseParsedHeader().m_StateCode == inet::HTTP_OK &&
				http.GetResponseLength()
			)
			{
				_LOG(http.GetResponseLength() << 
					" CHUNCKED:" << http.GetResponseParsedHeader().m_ChunkedTransfer << 
					" C-Len:" <<http.GetResponseParsedHeader().m_ContentLength<<
					" Total: "<<http.GetResponseLengthTotal()<<
					" Memory: "<<http.GetInternalBufferSize()
				);				
				os::File(rt::String_Ref("test_") + i + ext, os::File::Normal_Write).Write(http.GetResponse(), http.GetResponseLength());
			}
			else
			{
				_LOG("Failed");
			}
			os::Sleep(1000);
		}
	}
}

void rt::UnitTests::http_nav()
{
	
	inet::HttpNavigator	http;
	http.SetItemEventCallback(http_cb, nullptr);
	if(http.NavigateTo("http://bing.com/") && http.GetResponseLength()>5*1024)
	{
		_LOG("Document Downloaded");
	}
}


void rt::UnitTests::httpd()
{
	struct _obj
	{
		inet::HttpEndpoint*		ep[2];
		inet::HttpVirtualPath	path;
		inet::TinyHttpd			wwwd;
		os::Thread				clock;

		_obj(LPCSTR wr)
		{	
			path.SetMappedPath(wr);
			path.SetEndPoint("/p");
			ep[0] = &path;
						
			inet::InetAddr addr;
			addr.SetAsLocal();
			addr.SetPort(8080);
			wwwd.SetEndpoints(ep, 1);
			wwwd.Start(addr);

			_LOG("Listening at: http://"<<rt::tos::ip(wwwd.GetBindedAddress())<<"/p/index.htm\n");
		}
	};
	
	/*
	inet::InetAddr addr;
	addr.SetAsLocal();
	addr.SetPort(8888);
	inet::Socket s;
	s.Create(addr);
	s.Listen(5);
	inet::Socket conn;
	s.Accept(conn, addr);
	_LOG(inet::Socket::GetLastError());
	char buf[1024];
	UINT len = 0;
	s.Recv(buf, 1024, len);
	_LOG(len<<"ERR="<<inet::Socket::GetLastError());
	*/

	//////////////////////////////////////////////////////
	// Launch tinyhttpd
	rt::String dir;
	os::File::GetCurrentDirectory(dir);
	_LOG("Current Directory: "<<dir);

#if defined(PLATFORM_WIN)
	new _obj("../shared_tests/web_assets/");
	os::Sleep();
#elif defined(PLATFORM_MAC)
	new _obj("./web_assets/");
	os::Sleep();	
#else
	new _obj("./web_assets/");
#endif
}


