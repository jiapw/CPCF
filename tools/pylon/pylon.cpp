// pylon_d.cpp : Defines the entry point for the console application.
//
#include "pylon.h"
#include "scheduler.h"
#include "../../core/rt/xml_xhtml.h"
#include "pylon_client.h"

static const rt::SS ep_Core("/@core");
static const rt::SS ep_VPath("/@core/vpath");
static const rt::SS ep_Scheduler("/@scheduler");

Pylon::Pylon()
{
	m_IsRunning = false;
	m_WantRestart = false;
	m_pScheduler = NULL;
	_UpdateServiceEndpoints();
}

Pylon::~Pylon()
{
	ASSERT(m_IsRunning == false);

	for(UINT i=0;i<m_pVPaths.GetSize();i++)
		_SafeDel(m_pVPaths[i]);

	_SafeDel(m_pScheduler);
}


bool Pylon::LoadAllServices(LPCSTR config_fn)
{
	if(config_fn)
	{
		os::FileRead<CHAR>	file(config_fn);
		rt::XMLParser	xml;
		if(	file.GetSize() > 10 &&
			xml.Load(file, true, file.GetSize())
		)
		{	if(xml.EnterXPath("/pylon/vpath/map"))
				ReloadService(SERVICE_VPATH, rt::String_Ref(file, file.GetSize()));
			if(xml.EnterXPath("/pylon/scheduler"))
				ReloadService(SERVICE_SCHEDULER, rt::String_Ref(file, file.GetSize()));
			if(xml.EnterXPath("/pylon/echo/map"))
				ReloadService(SERVICE_ECHO, rt::String_Ref(file, file.GetSize()));

			_PylonConfigFilename = config_fn;
			return true;
		}
	}
	else
	{
		ReloadService(SERVICE_VPATH, 
			__STRING(
				<pylon>
					<vpath>
  						<map endpoint="/" path="./" readonly="false" />
					</vpath>
				</pylon>
			)
		);

		return true;
	}

	return false;
}

bool Pylon::ReloadService(_tagServiceNames svc, const rt::String_Ref& config_xml)
{
	EnterCSBlock(m_ServiceStateCS);

	bool eps_dirty = false;

	switch(svc)
	{
	case SERVICE_VPATH:
		{	rt::BufferEx<inet::HttpVirtualPath*>	vpath;

			if(!config_xml.IsEmpty())
			{
				rt::XMLParser	xml;
				xml.Load(config_xml.Begin(), true, config_xml.GetLength());
				if(xml.EnterXPath("/pylon/vpath/map"))
				{	
					rt::String ep, path;
					do
					{	if(	xml.GetAttribute("endpoint", ep) && 
							xml.GetAttribute_Path("path", path)
						)
						{	for(UINT i=0;i<m_pVPaths.GetSize();i++)
							{	
								if(m_pVPaths[i] && m_pVPaths[i]->GetEndPoint() == ep && m_pVPaths[i]->GetMappedPath() == path)
								{	
									vpath.push_back(m_pVPaths[i]);
									m_pVPaths[i] = NULL;
									goto EP_REUSED;
								}
							}
							// make a new one
							inet::HttpVirtualPath* newep = _New(inet::HttpVirtualPath);
							newep->SetEndPoint(ep);

							bool readonly;
							newep->SetMappedPath(path, (readonly = xml.GetAttribute_Bool("readonly",1)));
							vpath.push_back(newep);
							eps_dirty = true;
							_LOG("VPATH: "<<ep<<" => "<<path<<" readonly:"<<readonly?"true":"false");
					EP_REUSED:
							while(0);
						}
					}while(xml.EnterSucceedNode());
				}
			}

			for(UINT i=0;i<m_pVPaths.GetSize();i++)
				if(m_pVPaths[i])
				{	eps_dirty = true;
					_SafeDel_Delayed(m_pVPaths[i], DELAYED_RELEASE_TTL);
				}

			m_pVPaths.SetSize(vpath.GetSize());
			for(UINT i=0;i<m_pVPaths.GetSize();i++)
				m_pVPaths[i] = vpath[i];
		}
		break;
	case SERVICE_SCHEDULER:
		{
			if(!m_pScheduler)
			{	m_pScheduler = _New(Scheduler);
				m_pScheduler->SetEndPoint(ep_Scheduler.Begin());
				eps_dirty = true;
			}
			m_pScheduler->ReloadedConfig(config_xml);

			break;
		}
	case SERVICE_ECHO:
		{
			for(UINT i=0;i<m_EchoRequests.GetSize();i++)
				_SafeDel_Delayed(m_EchoRequests[i], DELAYED_RELEASE_TTL);

			m_EchoRequests.SetSize(0);
			rt::XMLParser	xml;
			xml.Load(config_xml.Begin(), true, config_xml.GetLength());
			if(xml.EnterXPath("/pylon/echo/map"))
			{	
				rt::String ep, path;
				do
				{	if(xml.GetAttribute("endpoint", ep))
					{	
						inet::HttpRequestEcho* echo = _New(inet::HttpRequestEcho);
						if(echo)
						{	echo->SetEndPoint(ep);
							m_EchoRequests.push_back(echo);
							_LOG("ECHO: "<<ep);
						}
					}
				}while(xml.EnterSucceedNode());
			}
			eps_dirty = true;
			break;
		}
	}

	if(eps_dirty)_UpdateServiceEndpoints();
	return eps_dirty;
}

void Pylon::_UpdateServiceEndpoints()
{
	EnterCSBlock(m_ServiceStateCS);

	inet::LPHTTPENDPOINT* eps = (inet::LPHTTPENDPOINT*)alloca(sizeof(inet::LPHTTPENDPOINT)*
								(2 + m_pVPaths.GetSize() + m_EchoRequests.GetSize()));
	int eps_used = 0;

	// vpath
	for(UINT i=0;i<m_pVPaths.GetSize();i++)
		eps[eps_used++] = m_pVPaths[i];

	// Scheduler
	if(m_pScheduler)
		eps[eps_used++] = m_pScheduler;

	// Echo
	for(UINT i=0;i<m_EchoRequests.GetSize();i++)
		eps[eps_used++] = m_EchoRequests[i];

	// pylon core service
	SetEndPoint(ep_Core.Begin());
	eps[eps_used++] = this;

	m_Httpd.SetEndpoints(eps, eps_used);
}


bool Pylon::Start(DWORD port, LPCSTR binding)
{
	m_Httpd.SetHangingTimeout(10000);

	ASSERT(!m_IsRunning);
	if(binding == NULL)binding = "127.0.0.1";

	inet::InetAddr	add(binding, (WORD)port);
	if(m_IsRunning = m_Httpd.Start(add))
		m_StartTime.LoadCurrentTime();

	return m_IsRunning;
}

void Pylon::Stop()
{
	if(m_IsRunning)
	{	
		m_Httpd.Stop();
		m_IsRunning = false;
	}
}

bool Pylon::OnRequest(inet::HttpResponse& resp)
{
	rt::String_Ref lnpath = resp.GetLnPath(this);

	if(lnpath == "/stat")
	{
		resp.SendJSONP_Begin();
		{	
			rt::String host, user;
			os::GetHostName(host);
			os::GetLogonUserName(user);

			rt::String local_ips;
			inet::InetAddr ip[64];
			UINT co = inet::GetLocalAddresses(ip, sizeofArray(ip), true);
			for(UINT i=0;i<co;i++)
			{	if(i)local_ips += rt::SS(", ");
				CHAR buf[256];
				local_ips += ip[i].GetDottedDecimalAddress(buf);
			}
			
			resp.SendChuncked(	(	J(localTime) = rt::tos::Timestamp<>(os::Timestamp::Get().GetLocalDateTime()),
									J(startTime) = rt::tos::Timestamp<>(m_StartTime.GetLocalDateTime()),
									J(host) = host,
									J(user) = user,
									J(endpoints) = local_ips
			));
		}
		resp.SendJSONP_End();
	}
	else if(lnpath == "/vpath")
	{
		resp.SendJSONP_Begin();
		{	CHAR buf[4000];
			resp.SendChuncked("[",1);
			EnterCSBlock(m_ServiceStateCS);
				for(UINT i=0;i<m_pVPaths.GetSize();i++)
				{
					resp.SendChuncked(buf, 
						(	rt::String_Ref("\t{ ep: \"") + m_pVPaths[i]->GetEndPoint() + "\", path: \"" + 
							m_pVPaths[i]->GetMappedPath() + rt::String_Ref("\" },", (i < m_pVPaths.GetSize() - 1)?4:3) + '\n'
						).CopyTo(buf)
					);
				}	
			resp.SendChuncked("]",1);
		}
		resp.SendJSONP_End();		
	}
	else if(lnpath == "/echo")
	{
		resp.SendJSONP_Begin();
		{	CHAR buf[4000];
			resp.SendChuncked("[",1);
			EnterCSBlock(m_ServiceStateCS);
				for(UINT i=0;i<m_EchoRequests.GetSize();i++)
				{
					resp.SendChuncked(buf, 
						(	rt::String_Ref("\t{ ep: \"") + m_EchoRequests[i]->GetEndPoint() + rt::String_Ref("\" },", (i < m_pVPaths.GetSize() - 1)?4:3) + '\n'
						).CopyTo(buf)
					);
				}	
			resp.SendChuncked("]",1);
		}
		resp.SendJSONP_End();		
	}
	else if(lnpath == "/shutdown")
	{
		m_WantRestart = false;
		m_EndOfRun.Set();
		resp.Send("shutdowning ...", 15, inet::TinyHttpd::_MIMEs[inet::TinyHttpd::MIME_TEXT]);
	}
	else if(lnpath == "/reboot")
	{
		m_WantRestart = true;
		m_EndOfRun.Set();
		resp.Send("rebooting ...", 13, inet::TinyHttpd::_MIMEs[inet::TinyHttpd::MIME_TEXT]);
	}
	else if(lnpath.SubStr(0,8) == "/reload/")
	{	
		_tagServiceNames svc_name;

		rt::String_Ref n = lnpath.TrimLeft(8);
		if(n == "vpath")
		{	svc_name = SERVICE_VPATH;
		}
		else if(n == "scheduler")
		{	svc_name = SERVICE_SCHEDULER;
		}
		else if(n == "echo")
		{	svc_name = SERVICE_ECHO;
		}
		else
		{	return false;
		}
		
		resp.SendJSONP_Begin();

		os::FileRead<CHAR>	file(_PylonConfigFilename);
		if(file.GetSize() > 10 && ReloadService(svc_name, rt::String_Ref(file, file.GetSize())))
		{
			resp.SendChuncked("{ result: 0 }");
		}
		else
		{	resp.SendChuncked("{ result: -1 }");
		}
		resp.SendJSONP_End();
	}
	else
	{	static const CHAR help[] = 
		"\n\n"
		"Pylon Core Service Simple Commands:\n"
		"/@core/shutdown      shutdown and exit the Pylon HTTP Daemon process\n"
		"/@core/reboot        restart Pylon HTTP Daemon, without exit the process\n"
		"\n\n"
		"Pylon Core Service JSONP APIs:\n"
		"/@core/reload/vpath        reload virtual folder mapping config\n"
		"/@core/reload/scheduler    reload scheduler task/daemon list\n"
		"/@core/reload/echo         reload echo endpoint list\n"
		"\n\n"
		"Internal Service Stat Page:\n"
		"/@core/vpath       virtual folder mapping list\n"
		"/@core/echo        echo endpoint list\n"
		"/@scheduler        scheduler stat\n"
		;
		resp.SendChuncked_Begin(inet::TinyHttpd::_MIMEs[inet::TinyHttpd::MIME_TEXT]);
			resp.SendChuncked("Pylon Config File: ");
			resp.SendChuncked(_PylonConfigFilename);
			resp.SendChuncked(help, sizeof(help)-1);
		resp.SendChuncked_End();
	}

	return true;
}

bool Pylon::WaitForRestart()
{
	m_EndOfRun.Reset();
	m_EndOfRun.WaitSignal();
	return m_WantRestart;
}

void Pylon::OnDaemonControl(DWORD dwControl)			// dwControl = SERVICE_CONTROL_STOP/SERVICE_CONTROL_SHUTDOWN/ ...
{
	if( dwControl == SERVICE_CONTROL_STOP || 
		dwControl == SERVICE_CONTROL_SHUTDOWN
	)
	{	_LOG("shutdown request from daemon controller.");
		m_EndOfRun.Set();
	}	
}


int RunPylon(os::CommandLine& cmdline)
{
#if defined(PLATFORM_WIN)
	os::EnableCrashDump("pylon", true);
#endif

	LPCSTR val;

	if(cmdline.HasOption("run") || cmdline.HasOption("daemon"))
	{
		Pylon	pylon;
		bool	AsDaemon = false;

		rt::String	Config;
		rt::String binding = cmdline.GetOption("bind", "0.0.0.0");
		DWORD port = cmdline.GetOptionAs<DWORD>("port", 8080);

		{	
			LPCSTR daemonname;
			if(daemonname = cmdline.SearchOptionEx("daemon"))
			{	if(!pylon.InitializeDaemonController(daemonname))
				{	return -10002;
				}
				else
				{	AsDaemon = true;
				}
			}
		}

		Config = cmdline.GetText(0);
		if(Config.IsEmpty())
		{	_LOG_WARNING("config_filenanme is not specified, use default vpath mapping.");
		}

		pylon.LoadAllServices(Config);

		bool bRestart = false;
		do
		{	
			if(!pylon.Start(port, binding))
			{	if(AsDaemon)pylon.ReportDaemonStatus(os::DAEMON_STOPPED);
				os::Sleep(5000);
				return -10012;
			}

			if(AsDaemon)pylon.ReportDaemonStatus(os::DAEMON_RUNNING);

			{	CHAR add[100];
				pylon.GetBindedAddress(add);
				_LOG("\nPylon is online: http://"<<rt::String_Ref(add)<<':'<<pylon.GetBindedPort()<<'/');
			}

			bRestart = pylon.WaitForRestart();

			if(bRestart)
			{	_LOG("Pylon is rebooting ...");		}
			else
			{	_LOG("Pylon is shuting down ...");	}

			pylon.Stop();
			if(!bRestart)
				if(AsDaemon)pylon.ReportDaemonStatus(os::DAEMON_STOPPED);
		}while(bRestart);

		return 0;
	}
	else if(cmdline.HasOption("copy") && cmdline.GetTextCount() == 2)
	{
		return pylon_copy(cmdline.GetText(0), cmdline.GetText(1));
	}
	else if((val = cmdline.SearchOptionEx("launch")) && cmdline.GetTextCount()>0)
	{
		rt::String_Ref cmdline_in = cmdline.GetText(0);
		rt::String_Ref workdir;
		if(cmdline.GetTextCount()>1)
			workdir = cmdline.GetText(1);

		return pylon_launch(val, cmdline_in, workdir, cmdline.HasOption("wait")!=NULL, cmdline.HasOption("log")!=NULL);
	}
	else if((val = cmdline.SearchOptionEx("jobstat")) && cmdline.GetTextCount()>0)
	{
		return pylon_jobstat(val, cmdline.GetText(0), cmdline.HasOption("wait")!=NULL, cmdline.HasOption("log")!=NULL);
	}

	_LOG("pylon config_filenanme [/run] [/deamon:service_name] [/port:nn] [/bind:ip]");
	_LOG("supported module names: vpath scheduler\n");
	_LOG("pylon /copy destination source");
	_LOG("pylon /launch:endpoint[@scheduler] cmdline workdir [/wait] [/log]");
	_LOG("pylon /jobstat:endpoint[@scheduler] jobId [/wait] [/log]");
	//_LOG("pylon /cfgen:service_name to have an example of the config file");

	return -1;
}
