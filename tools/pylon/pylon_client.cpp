#include "pylon_client.h"
#include "../../core/inet/http_client.h"


int pylon_copy(const rt::String_Ref& dest_in, const rt::String_Ref& src)
{
	rt::String_Ref dest = dest_in;
	rt::String dest_string;
	if(dest.Last() == '/')
	{
		dest_string = dest + src.GetFileName();
		dest = dest_string;
	}

	if(dest.StartsWith("http://") || dest.StartsWith("HTTP://"))
	{
		// Upload
		rt::String fn(src);
		os::FileBuffer<BYTE>	file;
		inet::HttpSession		http;
		if(file.Open(fn))
		{
			if(	http.Request_PostFile(rt::String(dest), file, file.GetSize(), fn.GetFileName().Begin(), false) &&
				http.WaitResponse()
			)
			{	
				if(http.GetResponseParsedHeader().m_StateCode < 400)
				{
					_LOG(src);
					return 0;
				}
				else
				{	_LOG_WARNING(src<<" uploading failed");
				}
			}
			else
			{	_LOG_WARNING(dest<<" is not reachable");
			}
		}
		else
		{	_LOG_WARNING(src<<" is failed to open");
		}
	}
	else if(src.StartsWith("http://") || src.StartsWith("HTTP://"))
	{
		inet::HttpSession		http;
		if( http.Request_Get(rt::String(src)) &&
			http.WaitResponse()
		)
		{	if(http.GetResponseParsedHeader().m_StateCode < 300)
			{	
				os::FileBuffer<BYTE> file;
				if(file.Open(rt::String(dest), http.GetResponseLength(), false))
				{
					memcpy(file, http.GetResponse(), http.GetResponseLength());
					_LOG(dest);
					return 0;
				}
			}
			else
			{	_LOG_WARNING(src<<" is not found");
			}
		}
		else
		{	_LOG_WARNING(src<<" is not reachable");
		}
	}

	return -1;
}

bool Endpoint::Parse(const rt::String_Ref& ep)
{
	if(ep.StartsWith("http://") || ep.StartsWith("HTTP://"))
	{
		if(ep.SubStr(8).FindCharacter('/') > 0)
		{
			if(ep.Last() == '/')
			{	*((rt::String*)this) = ep + rt::SS("@scheduler/");
			}
			else
			{	*((rt::String*)this) = ep + '/';
			}
			return true;
		}
		else
		{
			*((rt::String*)this) = ep + rt::SS("/@scheduler/");
			return true;
		}
	}
	else
	{	_LOG_ERROR("endpoint should be a http:// url");
		return false;
	}
}



int  pylon_launch(const rt::String_Ref& endpoint, const rt::String_Ref& cmdline, const rt::String_Ref& workdir, bool wait, bool dump_log)
{
	Endpoint ep;
	if(ep.Parse(endpoint))
	{
		inet::HttpSession http;
		if(workdir.IsEmpty())
			if(!http.Request_Get(ep + rt::SS("launch?cmdline=") + cmdline))return -1001;
		else
			if(!http.Request_Get(ep + rt::SS("launch?cmdline=") + cmdline + rt::SS("&workdir=") + workdir))return -1001;

		LONGLONG tid;
		if(	http.WaitResponse() &&
			http.GetResponseParsedHeader().m_StateCode < 300 &&
			http.GetResponseLength()
		)
		{	rt::JsonObject json((LPCSTR)http.GetResponse(), http.GetResponseLength());
			_LOG("Job Handle: "<<(tid = json.GetValueAs<LONGLONG>("handle")));
			if(!json.GetValueAs<bool>("running", false) || tid == 0)
			{	_LOG_WARNING("ERR: Job launch failed");
				return -1;
			}
		}

		return pylon_jobstat(endpoint, rt::tos::Number(tid), wait, dump_log);
	}

	return -1;
}

int  pylon_jobstat(const rt::String_Ref& endpoint, const rt::String_Ref& job_id, bool wait, bool dump_log)
{
	Endpoint ep;
	inet::HttpSession http;

	if(ep.Parse(endpoint))
	{
		LONGLONG tid;
		job_id.ToNumber(tid);

		for(;;)
		{	
			if(	http.Request_Get(ep + rt::SS("state?tsk=") + tid) &&
				http.WaitResponse() && 
				http.GetResponseParsedHeader().m_StateCode < 300 &&
				http.GetResponseLength()
			)
			{	
				rt::JsonObject json((LPCSTR)http.GetResponse(), http.GetResponseLength());
				if(json.GetValue("running") == rt::SS("false"))
				{
					int exitcode = json.GetValueAs<int>("exitcode", -10002);
					_LOG("Job ["<<tid<<"] is finished, exitcode = "<<exitcode);
				
					if(dump_log)
					{
						if(	http.Request_Get(ep + rt::SS("log?tsk=") + tid) && 
							http.WaitResponse() &&
							http.GetResponseParsedHeader().m_StateCode < 300
						)
						{
							_LOG("Console Output:\n"<<rt::String_Ref((LPCSTR)http.GetResponse(), http.GetResponseLength()));
						}
					}

					return exitcode;
				}
			}
			os::Sleep(500);
		}
	}

	return -10003;
}
