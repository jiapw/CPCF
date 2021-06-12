#pragma once
#include "../../core/inet/tinyhttpd.h"
#include "../../core/os/high_level.h"

class Scheduler: public inet::HttpHandler<Scheduler>
{
protected:
	enum
	{	TIGGERMODE_MANUAL = 0,
		TIGGERMODE_PREIOD_SEC = 1,
		TIGGERMODE_WHEN_MINUTE,
		TIGGERMODE_WHEN_HOUR,
		TIGGERMODE_WHEN_HOUR_MINUTE,
		TIGGERMODE_WHEN_WEEKDAY,
		TIGGERMODE_WHEN_DAY,
		TIGGERMODE_WHEN_MONTH,
		TIGGERMODE_AUTO,
		TIGGERMODE_ONETIME
	};
	struct _launch
	{
		DWORD				TriggerMode;
		os::LaunchProcess*	pLauncher;
		os::Timestamp		LastLaunchTime;

		rt::String			WorkingDirectory;
		rt::String			CmdLine;
		rt::String			Label;
		rt::String			RunningLog;
		rt::String			LastEnvVars;
		int					ExitCode;
		os::Timestamp		ExitTime;

		int					stat_Launch;
		int					stat_LaunchFailed;
		int					stat_Terminated;
		int					Duration;			// in sec

		bool				IsRunning() const;
		int					GetExitCode() const;
		LONGLONG			GetExitTime() const;
		rt::String_Ref		GetConsoleOutput();

		_launch();
	};
	struct _exec:public _launch		// continuous running
	{
		bool				IsSuspended;
		bool				IsManuallyTriggered;
		
		_exec();
	};
	struct _task:public _exec	// preiodic running
	{
		int					TriggerParam;
		int					stat_LaunchSkip;	// previous launch is still running
		int					stat_ExecutionTimeAvg;	//msec
		int					stat_ExecutionTimeMax;	//msec
		bool				Launched;
		
		bool				IsTriggered(const os::Timestamp& last_checked, const os::Timestamp& now) const;
		_task();
	};

	os::CriticalSection			_TaskListCS;
	rt::BufferEx<_task>			_TaskList;
	rt::BufferEx<_exec>			_DaemonList;
	rt::BufferEx<_launch>		_LaunchList;
	bool						_SearchItem(const rt::String_Ref& tsk, _task** pitem, UINT* item_type);

	bool						_EnableLaunch;

	os::Event					_ManualTrigger;
	os::Thread					_TriggerProbingThread;
	static DWORD				_TriggerProbing(LPVOID x);
	static int					ParseTimeSpan(const rt::String_Ref& x);

	enum _tagItemType
	{	ITEM_LAUNCH = 0,
		ITEM_DAEMON,
		ITEM_TASK
	};
	void						ComposeItemHtml(inet::HttpResponse& resp, LPCVOID  item, int i, UINT item_type);

public:
	bool	OnRequest(inet::HttpResponse& resp);
	void	ReloadedConfig(const rt::String_Ref& config_xml);
	Scheduler();
	~Scheduler();
};
