#pragma once

//////////////////////////////////////////////////////////////////////
// Cross-Platform Core Foundation (CPCF)
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of CPCF.  nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//////////////////////////////////////////////////////////////////////

#include "thread_primitive.h"
#include "kernel.h"

namespace rt
{
class Json;
};

namespace os
{

// Multi-threading
typedef DWORD	(*FUNC_THREAD_ROUTE)(LPVOID x);

class Thread
{
	LPVOID			__CB_Param;

	// callback by THISCALL_MFPTR
	THISCALL_POLYMORPHISM_DECLARE(DWORD, 0, OnRun, LPVOID param);
	LPVOID			__MFPTR_Obj;
	THISCALL_MFPTR	__MFPTR_Func;

	// callback by FUNC_THREAD_ROUTE
	FUNC_THREAD_ROUTE	__CB_Func;
	
public:
#if defined(PLATFORM_WIN)
	enum _tagThreadPriority
	{	PRIORITY_REALTIME = THREAD_PRIORITY_TIME_CRITICAL,
		PRIORITY_HIGH = THREAD_PRIORITY_ABOVE_NORMAL,
		PRIORITY_NORMAL = THREAD_PRIORITY_NORMAL,
		PRIORITY_LOW = THREAD_PRIORITY_BELOW_NORMAL,
		PRIORITY_IDLE = THREAD_PRIORITY_IDLE		
	};
#else
	enum _tagThreadPriority
	{	PRIORITY_REALTIME = 20,
		PRIORITY_HIGH = 5,
		PRIORITY_NORMAL = 0,
		PRIORITY_LOW = -5,
		PRIORITY_IDLE = -20		
	};
#endif
protected:

#if defined(PLATFORM_WIN)
	DWORD			_ThreadId;
#endif
	HANDLE			_hThread;
	bool			_bWantExit;
	DWORD			_ExitCode;
	DWORD			_Run();
	bool			_Create(UINT stack_size, ULONGLONG CPU_affinity);
	static void		__release_handle(HANDLE hThread);

public:
	static const int THREAD_OBJECT_DELETED_ON_RETURN = 0xfeed9038;	// the thread route return to skip clean up

	Thread();
	~Thread();

	bool	WaitForEnding(UINT time_wait_ms = INFINITE, bool terminate_if_timeout = false);
	DWORD	GetExitCode() const { return _ExitCode; }
	bool	IsRunning() const { return _hThread != NULL; }
	bool&	WantExit(){ return _bWantExit; }
	bool	WantExit() const { return _bWantExit; }
	void	TerminateForcely();
	void	DetachThread(){ if(_hThread){ __release_handle(_hThread); _hThread = NULL; } }
	void	SetPriority(UINT p = PRIORITY_HIGH);
	UINT	GetId();

	bool	Create(LPVOID obj, const THISCALL_MFPTR& on_run, LPVOID param = nullptr, ULONGLONG CPU_affinity = 0xffffffffffffffffULL, UINT stack_size = 0);
	bool	Create(FUNC_THREAD_ROUTE x, LPVOID thread_cookie = nullptr, ULONGLONG CPU_affinity = 0xffffffffffffffffULL, UINT stack_size = 0);

	template<typename T>
	bool	Create(T threadroute, ULONGLONG CPU_affinity = 0xffffffffffffffffULL, UINT stack_size = 0) // Caller should ensure the lifetime of variables captured by the lambda function
			{	__MFPTR_Obj = nullptr;
				ASSERT(_hThread == NULL);
				struct _call { 
					static DWORD _func(LPVOID p)
					{   DWORD ret = rt::CallLambda<DWORD>(0, *(T*)p);
						_SafeDel_ConstPtr((T*)p);
						return ret;
					}};
				__CB_Func = _call::_func;
				__CB_Param = _New(T(threadroute));
				ASSERT(__CB_Param);		
				if(_Create(stack_size, CPU_affinity))return true;
				_SafeDel_ConstPtr((T*)__CB_Param);
				return false;
			}
	static SIZE_T GetCurrentId(){ return GetCurrentThreadId(); }
	static void SetLabel(UINT thread_label);
	static UINT GetLabel();
	template<typename T>
	static void YieldRun(T threadroute, ULONGLONG CPU_affinity = 0xffffffffffffffffULL, UINT stack_size = 0)
			{	auto* t = _New(Thread);
				t->Create([threadroute,t](){
					threadroute();
					__release_handle(t->_hThread);
					t->_hThread = NULL;
					_SafeDel_ConstPtr(t);
					return THREAD_OBJECT_DELETED_ON_RETURN; 
				}, CPU_affinity, stack_size);
			}
};

#ifdef PLATFORM_DEBUG_BUILD
namespace _details
{
extern void	TrackedMemoryAllocationStatistic(rt::Json& json_out);
} // os::_details
#endif

/////////////////////////////////////
// Daemon/Service/Agent Control
enum _tagDaemonState
{
	DAEMON_STOPPED           = 0x1,
	DAEMON_START_PENDING     = 0x2,
	DAEMON_STOP_PENDING      = 0x3,
	DAEMON_RUNNING           = 0x4,
	DAEMON_CONTINUE_PENDING  = 0x5,
	DAEMON_PAUSE_PENDING     = 0x6,
	DAEMON_PAUSED            = 0x7,

	DAEMON_CONTROL_STOP		 = 0x11,
	DAEMON_CONTROL_PAUSE	 ,
	DAEMON_CONTROL_CONTINUE	 ,
	
};

//////////////////////////////////////////////
// DelayedGarbageCollection
class DelayedGarbageCollection	// singleton
{
public:
	typedef void (*LPFUNC_DELETION)(LPVOID x);
protected:
	static DWORD _DeletionThread(LPVOID);
	static void	DeleteObject(LPCVOID x, DWORD TTL_msec, LPFUNC_DELETION delete_func);
public:
	template<typename OBJ>
	static void DeleteObj(OBJ * ptr, int TTL_msec)
	{	struct _func{ static void delete_func(LPVOID x){ _SafeDel_ConstPtr((OBJ *)x); } };
		DeleteObject(ptr,TTL_msec,_func::delete_func);
	}
	template<typename OBJ>
	static void ReleaseObj(OBJ * ptr, int TTL_msec)
	{	struct _func{ static void release_func(LPVOID x){ ((OBJ *)x)->Release(); } };
		DeleteObject(ptr,TTL_msec,_func::release_func);
	}
	template<typename OBJ>
	static void DeleteArray(OBJ * ptr, int TTL_msec)
	{	struct _func{ static void delete_func(LPVOID x){ _SafeDelArray_ConstPtr((OBJ *)x); } };
		DeleteObject(ptr,TTL_msec,_func::delete_func);
	}
	static void Delete32AL(LPVOID ptr, int TTL_msec)
	{	struct _func{ static void delete_func(LPVOID x){ _SafeFree32AL_ConstPtr(x); } };
		DeleteObject(ptr,TTL_msec,_func::delete_func);
	}
	static void Delete(LPVOID ptr, int TTL_msec, LPFUNC_DELETION dfunc )
	{	DeleteObject(ptr,TTL_msec,dfunc);
	}
	static void Exit();
	static void Flush();
};

#define _SafeDel_Delayed(x, TTL_msec)		{ if(x){ os::DelayedGarbageCollection::DeleteObj(x,TTL_msec); x=nullptr; } }
#define _SafeDelArray_Delayed(x,TTL_msec)	{ if(x){ os::DelayedGarbageCollection::DeleteArray(x,TTL_msec); x=nullptr; } }
#define _SafeFree32AL_Delayed(x,TTL_msec)	{ if(x){ os::DelayedGarbageCollection::Delete32AL(x,TTL_msec); x=nullptr; } }
#define _SafeRelease_Delayed(x, TTL_msec)	{ if(x){ os::DelayedGarbageCollection::ReleaseObj(x,TTL_msec); x=nullptr; } }


namespace _details
{
template<typename t_MTMutable>
class _TSM_Updater
{
	typedef typename std::remove_reference_t<t_MTMutable>::t_Object t_Object;

	t_Object*		_Cloned;
	t_MTMutable&	_MTM;
	bool			_UpdateBegin;
public:
	INLFUNC _TSM_Updater(t_MTMutable& x, bool just_try = false):_MTM(x){ _UpdateBegin = _MTM.BeginUpdate(just_try); _Cloned = nullptr; }
	INLFUNC ~_TSM_Updater(){ if(_UpdateBegin)Commit(); }

	INLFUNC bool		IsUpdating() const { return _UpdateBegin; }
	INLFUNC bool		IsModified() const { return (bool)_Cloned; }
	INLFUNC const auto&	GetUnmodified() const { return _MTM.Get(); }
	INLFUNC auto&		GetModified() const { ASSERT(_Cloned); return *_Cloned; }
	
	INLFUNC bool		ReadyModify(bool from_new = false){ if(!_Cloned)_Cloned = from_new?_MTM.New():_MTM.Clone(); return (bool)_Cloned; }
	INLFUNC void		Revert(){ ASSERT(_UpdateBegin); _SafeDel(_Cloned); _MTM.EndUpdate(nullptr); _UpdateBegin = false; }
	INLFUNC void		Commit(){ ASSERT(_UpdateBegin); _MTM.EndUpdate(_Cloned); _UpdateBegin = false; }
	INLFUNC t_Object*	operator ->(){ ASSERT(_UpdateBegin); ReadyModify(); return _Cloned; }
	INLFUNC t_Object&	Get(){ ASSERT(_UpdateBegin); ReadyModify(); return *_Cloned; }
};
} // namespace _details

template<class T, UINT old_TTL = 2500>
class ThreadSafeMutable
{
	template<typename t_MTMutable>
	friend class _details::_TSM_Updater;

	T*		_p;
	os::CriticalSection _cs;
protected:
	INLFUNC bool		BeginUpdate(bool just_try = false){ if(just_try)return _cs.TryLock(); _cs.Lock(); return true; }
	INLFUNC void		EndUpdate(T* pNew = nullptr)/* nullptr indicates no change */
						{	if(pNew){	T* pOld = _p; _p = pNew; _SafeDel_Delayed(pOld, old_TTL); }
							_cs.Unlock();
						}
public:
	typedef T			t_Object;
	INLFUNC	ThreadSafeMutable(){ _p = nullptr; }
	INLFUNC ~ThreadSafeMutable(){ Clear();	}

	INLFUNC const T&	Get() const { static const T _t; return _p?*_p:_t; }
	INLFUNC const T*	operator -> () const { return &Get(); }
	INLFUNC T*			Clone() const { return _p?_New(T(*_p)):_New(T); }
	INLFUNC T*			New() const { return _New(T); }
	INLFUNC void		Clear(){ _cs.Lock(); _SafeDel(_p); _cs.Unlock(); }
	INLFUNC bool		IsEmpty() const { return _p == nullptr; }
	
	//unsafe in multi-thread
	INLFUNC T&			GetObject(){ ASSERT(_cs.IsOwnedByCurrentThread()); return (T&)Get(); }
};

#define THREADSAFEMUTABLE_UPDATE(org_obj, new_obj)	os::_details::_TSM_Updater<decltype(org_obj)> new_obj(org_obj, false)
#define THREADSAFEMUTABLE_SET(org_obj, new_obj)		os::_details::_TSM_Updater<decltype(org_obj)> new_obj(org_obj, false); new_obj.ReadyModify(true)
#define THREADSAFEMUTABLE_TRYUPDATE(org_obj, new_obj)	os::_details::_TSM_Updater<decltype(org_obj)> new_obj(org_obj, true)


#if defined(PLATFORM_WIN)

class LaunchProcess
{
#if defined(PLATFORM_WIN)
	HANDLE hChildStdinRd, hChildStdinWrDup, 
		   hChildStdoutWr,hChildStdoutRdDup;

public:
	typedef void	(*FUNC_HOOKEDOUTPUT)(char* p, UINT len, LPVOID cookie);

protected:
	FUNC_HOOKEDOUTPUT	_Callback;
	LPVOID				_Callback_Cookie;
	DWORD				_Flag;
	os::Thread			_OutputHookThread;
	static DWORD		_OutputHookRoutine(LPVOID);
	void				_HookedOutput(char* p, UINT len);

protected:
	os::CriticalSection	_CCS;
	HANDLE				_hProcess;
	rt::String			_Output;
	int					_ExitCode;
	os::Timestamp		_ExecutionTime;		// in msec
	os::Timestamp		_ExitTime;

	static void			_RemoveCarriageReturn(rt::String& output, const rt::String_Ref& add);
#endif

	void				_ClearAll();

public:
	enum
	{	FLAG_ROUTE_OUTPUT	= 0x1,
		FLAG_SAVE_OUTPUT	= 0x2,	// retrieve by GetOutput/GetOutputLen
		FLAG_HIDE_WINDOW	= 0x4,
	};
	LaunchProcess();
	~LaunchProcess();
	void		SetOutputCallback(FUNC_HOOKEDOUTPUT func, LPVOID cookie);
	bool		Launch(LPCSTR cmdline, DWORD flag = FLAG_ROUTE_OUTPUT, LPCSTR pWorkDirectory = nullptr, LPCSTR pEnvVariableAddon = nullptr);  // pEnvVariableAddon is \0 seperated multiple strings (UTF8), ends with \0\0
	bool		WaitForEnding(DWORD timeout = INFINITE); // return false when timeout
	void		Terminate();
	bool		IsRunning();
	LPCSTR		GetOutput();
	UINT		GetOutputLen();
	void		CopyOutput(rt::String& out);
	bool		SendInput(const rt::String_Ref& str){ return str.IsEmpty()?true:SendInput(str.Begin(), (UINT)str.GetLength()); }
	bool		SendInput(LPCSTR p, UINT len);
	auto		GetExecutionTime() const { return _ExecutionTime; }	// available after IsRunning() returns false!	
	auto		GetExitTime() const { return _ExitTime; }			// available after IsRunning() returns false!	
	int			GetExitCode() const { return _ExitCode; }			// available after IsRunning() returns false!
	bool		SendToStdin(LPCVOID str, UINT len);
};

#endif

} // namespace os

