#include "scheduler.h"
#include "../../core/rt/xml_xhtml.h"
#include "../../core/rt/json.h"


bool Scheduler::_task::IsTriggered(const os::Timestamp& last_checked, const os::Timestamp& now) const
{
	switch(TriggerMode)
	{
	case TIGGERMODE_MANUAL:
		return false;
	case TIGGERMODE_PREIOD_SEC:
		return (LastLaunchTime.TimeLapse(now)/1000 >= TriggerParam);
	case TIGGERMODE_WHEN_MINUTE:
		return last_checked.GetLocalDateTime().Minute != TriggerParam && now.GetLocalDateTime().Minute == TriggerParam;
	case TIGGERMODE_WHEN_HOUR:
		return last_checked.GetLocalDateTime().Hour != TriggerParam && now.GetLocalDateTime().Hour == TriggerParam;
	case TIGGERMODE_WHEN_HOUR_MINUTE:
		return (last_checked.GetLocalDateTime().Hour*60 + last_checked.GetLocalDateTime().Minute != TriggerParam)
			&& (now.GetLocalDateTime().Hour*60 + now.GetLocalDateTime().Minute == TriggerParam);
	case TIGGERMODE_WHEN_WEEKDAY:
		return last_checked.GetLocalDateTime().DayOfWeek != TriggerParam && now.GetLocalDateTime().DayOfWeek == TriggerParam;
	case TIGGERMODE_WHEN_DAY:
		return last_checked.GetLocalDateTime().Day != TriggerParam && now.GetLocalDateTime().Day == TriggerParam;
	case TIGGERMODE_WHEN_MONTH:
		return last_checked.GetLocalDateTime().Month != TriggerParam && now.GetLocalDateTime().Month == TriggerParam;
	default:
		ASSERT(0);
	}

	return false;
}

Scheduler::_launch::_launch()
{
	stat_Launch = stat_LaunchFailed = 0;
	Duration = 4*7*24*3600;
	stat_Terminated = 0;
	TriggerMode = TIGGERMODE_ONETIME;
	ExitCode = 0;

	pLauncher = new os::LaunchProcess;
}

Scheduler::_exec::_exec()
{
	IsManuallyTriggered = false;
	IsSuspended = false;
}

Scheduler::_task::_task()
{
	TriggerMode = TIGGERMODE_MANUAL;
	LastLaunchTime.LoadCurrentTime();

	stat_ExecutionTimeMax = 0;
	stat_ExecutionTimeAvg = 0;
	stat_LaunchSkip = 0;

	Launched = false;
}

int Scheduler::ParseTimeSpan(const rt::String_Ref& x)
{	UINT offset;
	double v;
	if(offset = x.ToNumber(v))
	{
		switch(x[offset])
		{
		case 's':	return (UINT)(v + 0.5);
		case 'm':	return (UINT)(v*60 + 0.5);
		case 'h':	return (UINT)(v*3600 + 0.5);
		case 'd':	return (UINT)(v*3600*24 + 0.5);
		case 'w':	return (UINT)(v*3600*24*7 + 0.5);
		default :	return (UINT)(v*60 + 0.5);	// minutes for default
		}
	}
	return -1;
}

void Scheduler::ReloadedConfig(const rt::String_Ref& config_xml)
{
	if(!_TriggerProbingThread.IsRunning())
		_TriggerProbingThread.Create(_TriggerProbing, this);

	EnterCSBlock(_TaskListCS);

	rt::String		val, dir;
	rt::XMLParser	xml;
	if(xml.Load(config_xml.Begin(),true,config_xml.GetLength()))
	{	
		if(xml.EnterXPath("/pylon/scheduler"))
		{
			_EnableLaunch = xml.GetAttribute_Bool("launchd", 0);
		}

		if(xml.EnterXPath("/pylon/scheduler/task[@cmdline]"))
		{
			rt::BufferEx<_task>			TaskList;
			rt::String_Ref tt[2];

			do
			{	DWORD mode = TIGGERMODE_MANUAL;
				int param = 0;
				double v;
				if(xml.GetAttribute("interval",val) && (param = ParseTimeSpan(val)) >0 )
				{
					mode = TIGGERMODE_PREIOD_SEC;
					param = rt::max(param,5);
				}
				else if((xml.GetAttribute("when_min",val) || xml.GetAttribute("when_minute",val))
						&& val.ToNumber(v) >= 0
						&& v>=0 && v <= 59
				)
				{	mode = TIGGERMODE_WHEN_MINUTE;
					param = (int)(v + 0.5);
				}
				else if((xml.GetAttribute("when_hour",val))
						&& val.ToNumber(v) >= 0
						&& v>=0 && v <= 23
				)
				{	mode = TIGGERMODE_WHEN_HOUR;
					param = (int)(v + 0.5);
				}
				else if(xml.GetAttribute("when_hm",val) &&
						val.Split(tt,2,':') == 2
				)
				{	
					mode = TIGGERMODE_WHEN_HOUR_MINUTE;
					int h,m;
					tt[0].ToNumber(h);
					tt[1].ToNumber(m);

					param = h*60 + m;
				}
				else if((xml.GetAttribute("when_day",val))
						&& val.ToNumber(v) >= 0
						&& v>=1 && v<=31
				)
				{	mode = TIGGERMODE_WHEN_DAY;
					param = (int)(v + 0.5);
				}
				else if(xml.GetAttribute("when_weekday",val))
				{	
					if(val.ToNumber(param) == -1)
					{
						val.MakeLower();
						param = os::Timestamp::ParseWeekdayName(val);
					}
					if(param >=1 && param <= 7)
						mode = TIGGERMODE_WHEN_WEEKDAY;
				}
				else if(xml.GetAttribute("when_month",val))
				{
					if(val.ToNumber(param) == -1)
					{
						val.MakeLower();
						param = os::Timestamp::ParseMonthName(val);
					}
					if(param >=1 && param <= 12)
						mode = TIGGERMODE_WHEN_MONTH;
				}

				if(xml.GetAttribute("cmdline",val))
				{
					xml.GetAttribute_Path("directory",dir);

					_task& nn = TaskList.push_back();

					// looking for existing task
					for(UINT i=0;i<_TaskList.GetSize();i++)
					{
						_task& ww = _TaskList[i];
						if(	ww.CmdLine == val &&
							ww.WorkingDirectory == dir &&
							ww.TriggerMode == mode &&
							ww.TriggerParam == param
						)
						{	rt::Swap(nn,ww);
							break;
						}
					}

					nn.TriggerMode = mode;
					nn.TriggerParam = param;
					nn.CmdLine = val;
					nn.WorkingDirectory = dir;
					nn.Duration = -1;
					xml.GetAttribute("label", nn.Label);
					if(xml.GetAttribute("duration",val))
						nn.Duration = ParseTimeSpan(val);
					if(nn.Duration < 0)
						nn.Duration = 10000*3600;
					else
						nn.Duration = rt::max(nn.Duration,10);
				}
			}while(xml.EnterSucceedNode());

			rt::Swap(_TaskList, TaskList);
		}

		if(xml.EnterXPath("/pylon/scheduler/daemon[@cmdline]"))
		{	
			rt::BufferEx<_exec>	DaemonList;

			do
			{	if(xml.GetAttribute("cmdline",val))
				{	
					_exec& nn = DaemonList.push_back();
					nn.CmdLine = val;
					xml.GetAttribute_Path("directory",dir);

					// looking for existing daemon
					for(UINT i=0;i<_DaemonList.GetSize();i++)
					{
						_exec& ww = _DaemonList[i];
						if(	ww.CmdLine == val &&
							ww.WorkingDirectory == dir
						)
						{	rt::Swap(nn,ww);
							break;
						}
					}

					nn.CmdLine = val;
					nn.WorkingDirectory = dir;
					nn.TriggerMode = TIGGERMODE_MANUAL;
					if(xml.HasAttribute("auto"))nn.TriggerMode = TIGGERMODE_AUTO;
				}
			}while(xml.EnterSucceedNode());

			for(UINT i=0;i<_DaemonList.GetSize();i++)
			{	// keep all running daemon from old list
				_exec& ww = _DaemonList[i];
				ASSERT(ww.pLauncher);
				if(ww.IsRunning())
				{	_exec& nn = DaemonList.push_back();
					rt::Swap(nn,ww);
				}
			}

			rt::Swap(_DaemonList, DaemonList);
		}
	}
}

Scheduler::Scheduler()
{
	_ManualTrigger.Reset();
	_EnableLaunch = false;
}

Scheduler::~Scheduler()
{
	if(_TriggerProbingThread.IsRunning())
	{
		_TriggerProbingThread.WantExit() = true;
		_TriggerProbingThread.WaitForEnding();
	}
}

DWORD Scheduler::_TriggerProbing(LPVOID x)
{
	Scheduler* pThis = (Scheduler*)x;
	os::Timestamp	time_last;
	time_last.LoadCurrentTime();

PROBE_TIGGER:
	// sleep ...
	{	int count_down = 2*1000;
		while(count_down>0)
		{
			if(pThis->_ManualTrigger.WaitSignal(500))
			{	// something triggered
				pThis->_ManualTrigger.Reset();
				break;
			}
			count_down -= 500;
			if(pThis->_TriggerProbingThread.WantExit())return 0;
		}
	}

	{	EnterCSBlock(pThis->_TaskListCS);
		os::Timestamp	tm;
		tm.LoadCurrentTime();

		// check tasks
		for(UINT i=0;i<pThis->_TaskList.GetSize();i++)
		{
			_task& t = pThis->_TaskList[i];
			ASSERT(t.pLauncher);
			if(t.IsRunning())
			{
				if(t.IsSuspended || t.Duration < t.LastLaunchTime.TimeLapse(tm)/1000)
				{
					t.pLauncher->Terminate();
					t.stat_Terminated++;
				}
				else if(t.IsTriggered(time_last, tm))
				{	t.stat_LaunchSkip++;
				}
			}
			else
			{	
				if(t.IsSuspended)continue;
				if(t.Launched)
				{
					ASSERT(t.pLauncher);
					int msec = (int)t.pLauncher->GetExecutionTime();

					if(t.stat_ExecutionTimeMax)
					{	t.stat_ExecutionTimeMax = rt::max(t.stat_ExecutionTimeMax,msec);
						t.stat_ExecutionTimeAvg = (t.stat_ExecutionTimeAvg*(t.stat_Launch - t.stat_LaunchFailed-1) + msec)/
												  (t.stat_Launch - t.stat_LaunchFailed);
					}
					else
					{	t.stat_ExecutionTimeMax = msec;
						t.stat_ExecutionTimeAvg = msec;
					}
					t.Launched = false;
				}

				if(t.IsTriggered(time_last, tm) || t.IsManuallyTriggered)
				{	
					t.stat_Launch++;
					t.LastLaunchTime.LoadCurrentTime();

					os::Timestamp::Fields local = t.LastLaunchTime.GetLocalDateTime();
					os::Timestamp::Fields utc = t.LastLaunchTime.GetDateTime();

					os::Timestamp yesterday = t.LastLaunchTime;
					yesterday.ShiftDays(-1);
					os::Timestamp::Fields yesterday_local = yesterday.GetLocalDateTime();
					os::Timestamp::Fields yesterday_utc = yesterday.GetDateTime();

					rt::String env = 
						__SS(PYLON_LOCAL_HOUR=) + rt::tos::Number(local.Hour).RightAlign(2,'0') + '\0' + 
						__SS(PYLON_HOUR=) + rt::tos::Number(utc.Hour).RightAlign(2,'0') + '\0' + 

						__SS(PYLON_LOCAL_MINUTE=) + rt::tos::Number(local.Minute).RightAlign(2,'0') + '\0' + 
						__SS(PYLON_MINUTE=) + rt::tos::Number(utc.Minute).RightAlign(2,'0') + '\0' + 

						__SS(PYLON_LOCAL_SECOND=) + rt::tos::Number(local.Second).RightAlign(2,'0') + '\0' + 
						__SS(PYLON_SECOND=) + rt::tos::Number(utc.Second).RightAlign(2,'0') + '\0' + 

						__SS(PYLON_LOCAL_HM=) + rt::tos::Timestamp<false, false, ' ', '-'>(local) + '\0' + 
						__SS(PYLON_HM=) + rt::tos::Timestamp<false, false, ' ', '-'>(utc) + '\0';

					env += 
						__SS(PYLON_LOCAL_TODAY=) + rt::tos::TimestampDate<'-'>(local) + '\0' + 
						__SS(PYLON_TODAY=) + rt::tos::TimestampDate<'-'>(utc) + '\0' + 
						__SS(PYLON_LOCAL_YESTERDAY=) + rt::tos::TimestampDate<'-'>(yesterday_local) + '\0' + 
						__SS(PYLON_YESTERDAY=) + rt::tos::TimestampDate<'-'>(yesterday_utc) + '\0' + 

						__SS(PYLON_LOCAL_TIMESTAMP=) + rt::tos::Timestamp<>(local) + '\0' + 
						__SS(PYLON_TIMESTAMP=) + rt::tos::Timestamp<>(utc) + '\0' +
						__SS(PYLON_LOCAL_DATEHOUR=) + rt::tos::TimestampDate<'-'>(local) + '-' + rt::tos::Number(local.Hour).RightAlign(2,'0') + '\0' + 
						__SS(PYLON_DATEHOUR=) + rt::tos::TimestampDate<'-'>(utc) + '-' + rt::tos::Number(local.Hour).RightAlign(2,'0') + '\0' + 
						__SS(PYLON=1) + '\0';
						
					yesterday = t.LastLaunchTime;
					for(UINT i=0;i<16;i++)
					{
						yesterday_local = yesterday.GetLocalDateTime();
						yesterday_utc = yesterday.GetDateTime();

						env += 
							__SS(PYLON_LOCAL_TODAY-) + i + '=' + rt::tos::TimestampDate<'-'>(yesterday_local) + '\0' + 
							__SS(PYLON_TODAY-) + i + '=' + rt::tos::TimestampDate<'-'>(yesterday_utc) + '\0';

						yesterday.ShiftDays(-1);
					}

					env += t.LastEnvVars + '\0';

					ASSERT(t.pLauncher);
					if(t.pLauncher->Launch(
									t.CmdLine, 
									os::LaunchProcess::FLAG_SAVE_OUTPUT,  
									t.WorkingDirectory.IsEmpty()?NULL:t.WorkingDirectory.Begin(),
									env
					))
					{	t.IsManuallyTriggered = false;
						t.Launched = true;
					}
					else
					{	t.stat_LaunchFailed++;
						t.IsManuallyTriggered = false;
						t.RunningLog = "Pylon Launch failed";
					}

					t.LastEnvVars.Empty();
				}
			}
		}

		// check deamon
		for(UINT i=0;i<pThis->_DaemonList.GetSize();i++)
		{
			_exec& d = pThis->_DaemonList[i];
			if(d.IsSuspended)
			{
				if(d.IsRunning())
				{
					d.pLauncher->Terminate();
					d.IsManuallyTriggered = false;
				}
				continue;
			}

			if(!d.IsRunning() && (d.TriggerMode == TIGGERMODE_AUTO || d.IsManuallyTriggered))
			{
				d.stat_Launch++;
				d.LastLaunchTime.LoadCurrentTime();
				if(d.pLauncher->Launch(d.CmdLine,os::LaunchProcess::FLAG_SAVE_OUTPUT,d.WorkingDirectory.IsEmpty()?NULL:d.WorkingDirectory.Begin()))
				{	
				}
				else
				{	d.stat_LaunchFailed++;
				}
			}
		}

		time_last = tm;
	}

	goto PROBE_TIGGER;
}

static const char page_scheduledtask_part1[] = 
"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">"
"<html xmlns=\"http://www.w3.org/1999/xhtml\">"
"<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />"
"<title>Scheduled Task Admin</title>"
"<style type=\"text/css\">"
"<!--"
"body,td,th { font-family: Verdana, Arial, Helvetica, sans-serif; font-size: 12px; }"
"body,td { background-color:#FFFFFF; padding-left:4px; }"
"th { background-color:#DDD; font-weight:bold; }"
"tr { height:18px; }"
"td { background-color:#eee; }"
"a { color: #33339A; }"
"-->"
"</style></head><body><br />";

static const char caption_box[] = "<span style=\"font-size:14px; font-weight:bold;\">";
static const char caption_box_close[] = "</span>";
static const char page_close[] = "</body></html>";

bool Scheduler::_SearchItem(const rt::String_Ref& tsk, _task** pitem, UINT* item_type)
{
	if(tsk.IsEmpty())return false;

	if(tsk[0] >='0' && tsk[0] <='9')
	{
		int tid;
		tsk.ToNumber(tid);

		if(tid > (int)(_DaemonList.GetSize() + _TaskList.GetSize()))
		{	
			for(UINT i=0;i<_LaunchList.GetSize();i++)
				if(_LaunchList[i].LastLaunchTime == tid)
				{	*pitem = ((_task*)&_LaunchList[i]);
					*item_type = ITEM_LAUNCH;
					return true;
				}
		}
		else if(tid>=0)
		{
			if(tid < (int)_TaskList.GetSize())
			{	*pitem = &_TaskList[tid];	
				*item_type = ITEM_TASK;
				return true;
			}
			else
			{	*pitem = (_task*)&_DaemonList[tid-_TaskList.GetSize()];	
				*item_type = ITEM_DAEMON;
				return true;
			}
		}
	}
	else
	{
		for(UINT i=0;i<_LaunchList.GetSize();i++)
			if(_LaunchList[i].Label == tsk)
			{	*pitem = ((_task*)&_LaunchList[i]);
				*item_type = ITEM_LAUNCH;
				return true;
			}
		for(UINT i=0;i<_TaskList.GetSize();i++)
			if(_TaskList[i].Label == tsk)
			{	*pitem = ((_task*)&_TaskList[i]);
				*item_type = ITEM_TASK;
				return true;
			}		
		for(UINT i=0;i<_DaemonList.GetSize();i++)
			if(_DaemonList[i].Label == tsk)
			{	*pitem = ((_task*)&_DaemonList[i]);
				*item_type = ITEM_DAEMON;
				return true;
			}
	}

	return false;
}

bool Scheduler::OnRequest(inet::HttpResponse& resp)
{
	rt::String_Ref lnpath = resp.GetLnPath(this);

	if(resp.HttpVerb == inet::HTTP_GET)
	{
		if(lnpath == "/trigger")
		{
			EnterCSBlock(_TaskListCS);

			rt::String_Ref taskid = resp.GetQueryParam("tsk");
			rt::String_Ref cmd = resp.GetQueryParam("cmd");

			_task* pt = NULL;
			UINT   item_type;
			if(!taskid.IsEmpty() && !cmd.IsEmpty() && _SearchItem(taskid, &pt, &item_type))
			{
				if(cmd == "term")
				{
					if(pt->IsRunning())
						pt->pLauncher->Terminate();
				}
				else if(item_type != ITEM_LAUNCH)
				{
					if(cmd == "run")
					{
						pt->LastEnvVars = resp.GetQueryParam("env");
						pt->LastEnvVars.Replace('|', '\0');
						pt->IsManuallyTriggered = true;
						if(!resp.GetQueryParam("json").IsEmpty())
						{	
							_ManualTrigger.Set();
							resp.SendJSONP_Begin();
							resp.SendChuncked((J(r) = true));
							resp.SendJSONP_End();
							return true;
						}
					}
					else if(cmd == "stop")
					{
						pt->IsManuallyTriggered = false;
					}
					else if(cmd == "enable")
					{
						pt->IsSuspended = false;
					}
					else if(cmd == "disable")
					{
						pt->IsSuspended = true;
					}
				}
				else
				{	resp.SendHttpError(inet::HTTP_NOT_ALLOWED);
					return true;
				}

				_ManualTrigger.Set();
				resp.SendRedirection(inet::HTTP_MOVED, L1_Path.Begin(), L1_Path.GetLength());
				return true;
			}
			else
			{	resp.SendHttpError(inet::HTTP_NOT_FOUND);
				return true;
			}
		}
		else if(lnpath == "/state")
		{	
			rt::String_Ref taskid = resp.GetQueryParam("tsk");

			EnterCSBlock(_TaskListCS);

			_task* pt = NULL;
			UINT   item_type;
			if(!taskid.IsEmpty() && _SearchItem(taskid, &pt, &item_type))
			{	
				resp.SendChuncked_Begin(inet::TinyHttpd::MIME_STRING_JS);
				if(pt->IsRunning())
				{
					resp.SendChuncked((	J(handle) = taskid,
										J(running) = true
					));
				}
				else
				{	resp.SendChuncked((	J(handle) = taskid,
										J(running) = false,
										J(exitcode) = pt->GetExitCode()
					));
				}
				resp.SendChuncked_End();
				return true;
			}
		}
		else if(lnpath == "/log")
		{
			rt::String_Ref taskid = resp.GetQueryParam("tsk");

			EnterCSBlock(_TaskListCS);

			_task* pt = NULL;
			UINT   item_type;
			if(!taskid.IsEmpty() && _SearchItem(taskid, &pt, &item_type))
			{	
				rt::String_Ref log = pt->GetConsoleOutput();
				resp.Send(log.Begin(), log.GetLength(), inet::TinyHttpd::MIME_STRING_TEXT);
				return true;
			}
		}
		else if(lnpath == "/launch")
		{
			EnterCSBlock(_TaskListCS);

			if(_EnableLaunch && !resp.Query.IsEmpty())
			{	
				rt::String_Ref cmdline = resp.GetQueryParam("cmdline");
				rt::String_Ref workdir = resp.GetQueryParam("workdir");
				rt::String_Ref duration = resp.GetQueryParam("duration");
				rt::String_Ref label = resp.GetQueryParam("label");

				if(!cmdline.IsEmpty())
				{
					EnterCSBlock(_TaskListCS);
					_launch& item = _LaunchList.push_back();
					item.CmdLine = cmdline;
					item.WorkingDirectory = workdir;
					if(!duration.IsEmpty())
						item.Duration = ParseTimeSpan(duration);
					item.LastLaunchTime.LoadCurrentTime();
					item.stat_Launch = 1;
					item.Label = label;
					if(!item.pLauncher->Launch(item.CmdLine, os::LaunchProcess::FLAG_SAVE_OUTPUT, item.WorkingDirectory))
						item.stat_LaunchFailed = 1;

					resp.SendChuncked_Begin(inet::TinyHttpd::MIME_STRING_XML);
					resp.SendChuncked((	J(handle) = item.LastLaunchTime._Timestamp,
										J(running) = !!item.stat_LaunchFailed
					));						
					resp.SendChuncked_End();

					return true;
				}
			}
			
			resp.SendHttpError(inet::HTTP_FORBIDDEN);
			return true;
		}
		else
		{	// display state
			resp.SendChuncked_Begin(inet::TinyHttpd::_MIMEs[inet::TinyHttpd::MIME_HTML]);
		
			if(_TaskListCS.TryLock())
			{	
				resp.SendChuncked(page_scheduledtask_part1,sizeof(page_scheduledtask_part1)-1);

				{
					static const char table_header[] = 
					"<br /><br /><table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"2\">"
					"<tr style=\"font-weight:bold;\">"
					"<th width=\"60\" style=\"text-align:left;padding-left:4px;\">#</th><th width=\"100\">Label</th><th>Command</th>"
					"<th width=\"200\">Trigger Condition</th><th width=\"100\">Max Duration</th><th width=\"200\">Statistic</th><th width=\"200\">State</th></tr>";

					if(_TaskList.GetSize() + _DaemonList.GetSize())
					{
						resp.SendChuncked(rt::SS(caption_box));
						resp.SendChuncked(rt::SS("Scheduled Tasks: (") + (_TaskList.GetSize() + _DaemonList.GetSize()) + rt::SS(" items)"));
						resp.SendChuncked(rt::SS(caption_box_close));

						resp.SendChuncked(rt::SS(table_header));

						for(UINT i=0;i<_TaskList.GetSize();i++)
						{
							_task& t = _TaskList[i];
							ComposeItemHtml(resp, &t, i, ITEM_TASK);
						}

						for(UINT i=0;i<_DaemonList.GetSize();i++)
						{
							_exec& t = _DaemonList[i];
							ComposeItemHtml(resp, &t, i + _TaskList.GetSize(), ITEM_DAEMON);
						}

						resp.SendChuncked(rt::SS("</table><br/><br/>"));
					}

					if(_LaunchList.GetSize())
					{
						resp.SendChuncked(rt::SS(caption_box));
						resp.SendChuncked(rt::SS("Launched Jobs: (") + _LaunchList.GetSize() + rt::SS(" items)"));
						resp.SendChuncked(rt::SS(caption_box_close));

						resp.SendChuncked(rt::SS(table_header));

						for(int i=_LaunchList.GetSize()-1;i>=0;i--)
						{
							_launch& t = _LaunchList[i];
							ComposeItemHtml(resp, &t, i, ITEM_LAUNCH);
						}

						resp.SendChuncked(rt::SS("</table>"));
					}
				}

				resp.SendChuncked(page_close,sizeof(page_close)-1);
				_TaskListCS.Unlock();
			}
			else
			{	static const char busy_page[] = 
				"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">"
				"<html xmlns=\"http://www.w3.org/1999/xhtml\">"
				"<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />"
				"<meta http-equiv=\"Refresh\" Content=\"2;\">"
				"<title>Scheduled Task Admin</title>"
				"<style type=\"text/css\">"
				"<!--"
				"body,td,th { font-family: Verdana, Arial, Helvetica, sans-serif; font-size: 14px; font-weight:bold; }"
				"body { background-color:#FFFFFF; padding-left:4px; }"
				"-->"
				"</style></head><body><br />Tasks scheduler is busy, reloading ... </body></html>";

				resp.SendChuncked(busy_page,sizeof(busy_page)-1);
			}
		
			resp.SendChuncked_End();
			return true;
		}
	}

	resp.SendHttpError(inet::HTTP_NOT_FOUND);
	return true;
}

bool Scheduler::_launch::IsRunning() const
{
	return pLauncher && pLauncher->IsRunning();
}

int	Scheduler::_launch::GetExitCode() const
{
	if(pLauncher)
		return pLauncher->GetExitCode();
	else
		return ExitCode;
}

LONGLONG Scheduler::_launch::GetExitTime() const
{
	if(pLauncher)
		return pLauncher->GetExitTime();
	else
		return ExitTime;
}

rt::String_Ref Scheduler::_launch::GetConsoleOutput()
{
	if(pLauncher)
		pLauncher->CopyOutput(RunningLog);
	
	return RunningLog;
}

void Scheduler::ComposeItemHtml(inet::HttpResponse& resp, LPCVOID pt, int i, UINT item_type)
{
	static const char TD[] = "</td><td>";
	static const char WDIR[] = "<br/><strong>Work Directory:</strong> ";
	_task& t = *((_task*)pt);
	__int64 taskid = i;

	resp.SendChuncked("<tr>",4);
	resp.SendChuncked(&TD[5],4);

		if(item_type != ITEM_LAUNCH)
		{	resp.SendChuncked(rt::tos::Number(i));
		}
		else
		{	resp.SendChuncked(rt::tos::Number(t.LastLaunchTime));
			taskid = t.LastLaunchTime._Timestamp;
		}

		resp.SendChuncked(TD,sizeof(TD)-1);

		resp.SendChuncked(t.Label);

		resp.SendChuncked(TD,sizeof(TD)-1);

		resp.SendChuncked(t.CmdLine);
		if(!t.WorkingDirectory.IsEmpty())
		{	resp.SendChuncked(WDIR,sizeof(WDIR)-1);
			resp.SendChuncked(t.WorkingDirectory);
		}
		resp.SendChuncked(TD,sizeof(TD)-1);
		
		if(item_type == ITEM_TASK)
		{
			if(t.TriggerMode == TIGGERMODE_PREIOD_SEC)
			{	resp.SendChuncked("Every ",6);
				resp.SendChuncked(rt::tos::TimeSpan<>(t.TriggerParam*1000));
			}
			else
			{	static const char mn[] = ". midnight";
				switch(t.TriggerMode)
				{
				case TIGGERMODE_MANUAL:
						resp.SendChuncked("Manually");
						break;
				case TIGGERMODE_WHEN_MINUTE:	
						resp.SendChuncked("Hourly when minute = ");
						resp.SendChuncked(rt::tos::Number(t.TriggerParam));
						break;
				case TIGGERMODE_WHEN_HOUR:
						resp.SendChuncked(rt::String_Ref("Daily at 0",9 + (t.TriggerParam<=9)));
						resp.SendChuncked(rt::tos::Number(t.TriggerParam));
						resp.SendChuncked(":00");
						break;
				case TIGGERMODE_WHEN_HOUR_MINUTE:
						resp.SendChuncked(rt::String_Ref("Daily at ") + t.TriggerParam/60 + ':' + t.TriggerParam%60);
						break;
				case TIGGERMODE_WHEN_WEEKDAY:
						resp.SendChuncked("Weekly at ");
						resp.SendChuncked(os::Timestamp::GetDayOfWeekName(t.TriggerParam));
						resp.SendChuncked(mn,sizeof(mn)-1);
						break;
				case TIGGERMODE_WHEN_DAY:
						resp.SendChuncked("Monthly at day ");
						resp.SendChuncked(rt::tos::Number(t.TriggerParam));
						resp.SendChuncked(mn+1,sizeof(mn)-2);
						break;
				case TIGGERMODE_WHEN_MONTH:
						resp.SendChuncked("Once a year at ");
						resp.SendChuncked(os::Timestamp::GetMonthName(t.TriggerParam));
						resp.SendChuncked(". 1st");
						resp.SendChuncked(mn+1,sizeof(mn)-2);
						break;
				default:
						ASSERT(0);
				}
			}
		}
		else if(item_type == ITEM_DAEMON)
		{	switch(t.TriggerMode)
			{	
			case TIGGERMODE_MANUAL:
				resp.SendChuncked("Continuous/Manual");
				break;
			case TIGGERMODE_AUTO:
				resp.SendChuncked("Continuous/Auto");
				break;
			default:
				ASSERT(0);
			}
		}
		else
		{	resp.SendChuncked("One-time Launch");
		}

		if(t.IsSuspended && item_type != ITEM_LAUNCH)
		{
			resp.SendChuncked(" (Disabled)");
		}

		resp.SendChuncked(TD,sizeof(TD)-1);

		if(item_type == ITEM_TASK || item_type == ITEM_LAUNCH)
			resp.SendChuncked(rt::tos::TimeSpan<>(((__int64)t.Duration)*1000));

		resp.SendChuncked(TD,sizeof(TD)-1);

		if(item_type == ITEM_LAUNCH)
		{
			if(t.IsRunning())
			{	resp.SendChuncked(rt::SS("Running for ") + rt::tos::TimeSpan<>(t.LastLaunchTime.TimeLapse()));
			}
			else
			{	
				if(!t.stat_LaunchFailed)
					resp.SendChuncked(rt::SS("Duration: ") + rt::tos::TimeSpan<>(rt::max(0LL,t.LastLaunchTime.TimeLapse(t.GetExitTime()))));
			}
		}
		else
		{
			resp.SendChuncked(rt::tos::Number(t.stat_Launch));
			resp.SendChuncked(rt::String_Ref(" launches",7 + (t.stat_Launch>1)*2));

			if(item_type == ITEM_TASK)
			{
				if(t.stat_Launch - t.stat_LaunchFailed - t.stat_LaunchSkip)
				{
					resp.SendChuncked((LPCSTR)(	rt::String_Ref(" ( ", 3) + rt::tos::TimeSpan<false>(t.stat_ExecutionTimeAvg) + 
												rt::String_Ref(" / ", 3) + rt::tos::TimeSpan<false>(t.stat_ExecutionTimeMax) + 
												')'
									 ));
				}
			}

			static const char comma[] = ", ";

			if(item_type == ITEM_TASK && t.stat_LaunchSkip)
			{	resp.SendChuncked(comma,2);
				resp.SendChuncked(rt::tos::Number(t.stat_LaunchSkip));
				resp.SendChuncked(" skipped");
			}
			if(t.stat_LaunchFailed)
			{	resp.SendChuncked(comma,2);
				resp.SendChuncked(rt::tos::Number(t.stat_LaunchFailed));
				resp.SendChuncked(" failed");
			}
			if(item_type == ITEM_TASK && t.stat_Terminated)
			{	resp.SendChuncked(comma,2);
				resp.SendChuncked(rt::tos::Number(t.stat_Terminated));
				resp.SendChuncked(" terminated forcely");
			}
		}

		if(t.stat_Launch > t.stat_LaunchFailed)
		{
			resp.SendChuncked(rt::SS(" [ <a target=\"_blank\" href=\"") + L1_Path + rt::SS("/log?tsk=") + taskid + rt::SS("\">Log</a> ]"));
		}

		resp.SendChuncked(TD,sizeof(TD)-1);

		static const rt::SS trigger_uri("/trigger?tsk=");

		if(t.IsRunning())
		{	
			resp.SendChuncked(	rt::SS("Running since ") + rt::tos::Timestamp<false>(t.LastLaunchTime.GetLocalDateTime()) + 
								rt::SS(" [ <a href=\"") + L1_Path + trigger_uri + taskid + rt::SS("&cmd=term\">Terminate</a> ]")
							);
		}

		if(item_type == ITEM_LAUNCH)
		{
			if(t.IsRunning()){}
			else
			{	
				if(t.stat_LaunchFailed)
				{	resp.SendChuncked(rt::SS("Launch failed"));
				}
				else if(t.stat_Terminated)
				{	resp.SendChuncked(rt::SS("Terminated"));
				}
				else
				{
					resp.SendChuncked(rt::SS("Finished, ExitCode: ") + t.GetExitCode());
				}
			}
		}
		else
		{
			if(t.IsRunning()){}
			else if(t.IsManuallyTriggered)
			{	resp.SendChuncked("Triggered ");
				if(item_type != ITEM_TASK && t.TriggerMode != TIGGERMODE_AUTO)
					resp.SendChuncked(rt::SS(" [ <a href=\"") + L1_Path + trigger_uri + taskid + rt::SS("&cmd=stop\">Stop</a> ]"));
			}
			else
			{	resp.SendChuncked("Stopped");
				resp.SendChuncked(rt::SS(" [ <a href=\"") + L1_Path + trigger_uri + taskid + rt::SS("&cmd=run\">Run</a> ]"));
			}

			if(t.IsSuspended)
			{
				resp.SendChuncked(rt::SS(" [ <a href=\"") + L1_Path + trigger_uri + taskid + rt::SS("&cmd=enable\">Enable</a> ]"));
			}
			else
			{
				resp.SendChuncked(rt::SS(" [ <a href=\"") + L1_Path + trigger_uri + taskid + rt::SS("&cmd=disable\">Disable</a> ]"));
			}
		}

	resp.SendChuncked(TD,5);
	resp.SendChuncked("</tr>",5);
}

