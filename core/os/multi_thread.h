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

#include "predefines.h"
#include "../rt/runtime_base.h"
#include "../rt/type_traits.h"
#include "kernel.h"

#if defined(PLATFORM_WIN)
#include <intrin.h> 

#else
#include <sys/time.h>
#include <pthread.h>

#if defined (PLATFORM_ANDROID)
#include <asm-generic/errno.h>
#elif defined (PLATFORM_MAC) || defined (PLATFORM_IOS)
#include <libkern/OSAtomic.h>
#include <errno.h>
#endif

#endif


///////////////////////////////////////////
// Classes for synchronization multi-threading application
namespace os
{

// Multi-threading
typedef DWORD	(*FUNC_THREAD_ROUTE)(LPVOID x);

class Thread
{
	LPVOID			__CB_Param;

	// callback by THISCALL_MFPTR
	THISCALL_POLYMORPHISM_DECLARE(DWORD, OnRun, LPVOID param);
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
    static UINT GetCurrentId();
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

// All Atomic operation return the value after operation, EXCEPT AtomicOr
#if defined(PLATFORM_WIN)
	FORCEINL int		AtomicIncrement(volatile int *theValue){ return _InterlockedIncrement((long volatile *)theValue); }
	FORCEINL int		AtomicDecrement(volatile int *theValue){ return _InterlockedDecrement((long volatile *)theValue); }
	FORCEINL int		AtomicAdd(int theAmount, volatile int *theValue){ return theAmount + _InterlockedExchangeAdd((long volatile *)theValue, (long)theAmount); }
	FORCEINL void		AtomicSet(int val, volatile int *theValue){ AtomicAdd(val-*theValue, theValue); }
	FORCEINL DWORD		AtomicOr(DWORD bits, volatile DWORD* theValue){ return (DWORD)_InterlockedOr((volatile long*)theValue, bits); }
	#if defined(PLATFORM_64BIT)
	FORCEINL __int64	AtomicIncrement(volatile __int64 *theValue){ return _InterlockedIncrement64((__int64 volatile *)theValue); }
	FORCEINL __int64	AtomicDecrement(volatile __int64 *theValue){ return _InterlockedDecrement64((__int64 volatile *)theValue); }
	FORCEINL __int64	AtomicAdd(__int64 theAmount, volatile __int64 *theValue){ return theAmount + _InterlockedExchangeAdd64((__int64 volatile *)theValue, (__int64)theAmount); }
	FORCEINL void		AtomicSet(__int64 val, volatile __int64 *theValue){ AtomicAdd(val-*theValue, theValue); }
	FORCEINL ULONGLONG	AtomicOr(ULONGLONG bits, volatile ULONGLONG* theValue){ return (ULONGLONG)_InterlockedOr64((volatile __int64*)theValue, bits); }
	#endif
#elif defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
	FORCEINL int		AtomicIncrement(volatile int *theValue){ return OSAtomicIncrement32Barrier(theValue); }
	FORCEINL int		AtomicDecrement(volatile int *theValue){ return OSAtomicDecrement32Barrier(theValue); }
	FORCEINL __int64	AtomicIncrement(volatile __int64 *theValue){ return OSAtomicIncrement64Barrier(theValue); }
	FORCEINL __int64	AtomicDecrement(volatile __int64 *theValue){ return OSAtomicDecrement64Barrier(theValue); }
	FORCEINL int		AtomicAdd(int theAmount, volatile int *theValue){ return OSAtomicAdd32Barrier(theAmount, theValue); }
	FORCEINL void		AtomicSet(int val, volatile int *theValue){ AtomicAdd(val-*theValue, theValue); }
	FORCEINL __int64	AtomicAdd(__int64 theAmount, volatile __int64 *theValue){ return OSAtomicAdd64Barrier(theAmount, theValue); }
	FORCEINL void		AtomicSet(__int64 val, volatile __int64 *theValue){ AtomicAdd(val-*theValue, theValue); }
#else
	FORCEINL int		AtomicIncrement(volatile int *theValue){ return 1 + __sync_fetch_and_add(theValue,1); }
	FORCEINL int		AtomicDecrement(volatile int *theValue){ return __sync_fetch_and_sub(theValue,1) - 1; }
	FORCEINL __int64	AtomicIncrement(volatile __int64 *theValue){ return 1 + __sync_fetch_and_add(theValue,1); }
	FORCEINL __int64	AtomicDecrement(volatile __int64 *theValue){ return __sync_fetch_and_sub(theValue,1) - 1; }
	FORCEINL int		AtomicAdd(int theAmount, volatile int *theValue){ return theAmount + __sync_fetch_and_add(theValue, theAmount); }
	FORCEINL void		AtomicSet(int val, volatile int *theValue){ AtomicAdd(val-*theValue, theValue); }
	FORCEINL __int64	AtomicAdd(__int64 theAmount, volatile __int64 *theValue){ return theAmount + __sync_fetch_and_add(theValue, theAmount); }
	FORCEINL void		AtomicSet(__int64 val, volatile __int64 *theValue){ AtomicAdd(val-*theValue, theValue); }
#endif

// Desgined for cross function reusing of TempVar that involves buffer allocation on heap
template<typename T, int IDX = 0>
struct LocalScopeTempVar
{
protected:
	T& _t;
	static T& _get(bool release = false)
	{	thread_local T x;
		thread_local bool inuse = false;
		if(release){ ASSERT(inuse); inuse = false; }
		else { ASSERT(!inuse); inuse = true; }
		return x;
	}
public:
	LocalScopeTempVar():_t(_get()){}
	~LocalScopeTempVar(){ _get(true); }

	T* operator ->(){ return &_t; }
	T& operator  *(){ return _t; }
	operator T&   (){ return _t; }
};

class AtomicLock  //  a non-waiting CriticalSection, not thread-recursive
{
	volatile int	_iAtom;
public:
	FORCEINL AtomicLock(){ _iAtom = 0; }
	FORCEINL void Reset(){ _iAtom = 0; }
	FORCEINL bool TryLock()	// must call Unlock ONCE, or Reset, after returns true
	{	int ret = AtomicIncrement(&_iAtom);
		if(ret == 1)return true;
		AtomicDecrement(&_iAtom);
		return false;
	}
	FORCEINL bool IsLocked(){ return _iAtom>=1; }
	FORCEINL void Unlock(){ AtomicDecrement(&_iAtom); }
#ifdef PLATFORM_DEBUG_BUILD
	FORCEINL ~AtomicLock(){ ASSERT(_iAtom == 0);  }
#endif
};
typedef AtomicLock AtomicLocker;

#define EnterCSBlock(x) os::CriticalSection::_CCS_Holder MARCO_JOIN(_CCS_Holder_,__COUNTER__)(x);


class CriticalSection
{
	CriticalSection(const CriticalSection&x){ ASSERT(0); }
	SIZE_T		_OwnerTID;
	int			_Recurrence;
public:
	class _CCS_Holder
	{	CriticalSection& _CCS;
	public:
		FORCEINL _CCS_Holder(const CriticalSection& so):_CCS((CriticalSection&)so){ _CCS.Lock(); }
		FORCEINL ~_CCS_Holder(){ _CCS.Unlock(); }
	};
#if defined(PLATFORM_WIN)
#pragma warning(disable:4512) // assignment operator could not be generated
protected:
	CRITICAL_SECTION hCS;
public:
	FORCEINL void Lock()
	{	EnterCriticalSection(&hCS);
		if(_OwnerTID == os::Thread::GetCurrentId())
		{	_Recurrence++;	}
		else{ _OwnerTID = os::Thread::GetCurrentId(); _Recurrence = 1; }
	}
	FORCEINL void Unlock()
	{	_Recurrence--;
		if(_Recurrence == 0)_OwnerTID = 0;
		LeaveCriticalSection(&hCS); 
	}
	FORCEINL bool TryLock()
	{	if(TryEnterCriticalSection(&hCS))
		{	if(_OwnerTID == os::Thread::GetCurrentId())
			{	_Recurrence++;	}
			else{ _OwnerTID = os::Thread::GetCurrentId(); _Recurrence = 1; }
			return true;
		}	
		return false;
	}
#pragma warning(default:4512) // assignment operator could not be generated
#else
protected:
	friend class Event;
	pthread_mutex_t hCS;
public:
	FORCEINL void Lock(){ VERIFY(0 == pthread_mutex_lock(&hCS)); _OwnerTID = os::Thread::GetCurrentId(); }
	FORCEINL void Unlock(){ _OwnerTID = 0; VERIFY(0 == pthread_mutex_unlock(rt::_CastToNonconst(&hCS))); }
	FORCEINL bool TryLock()
	{	if(0 == pthread_mutex_trylock(&hCS)){ _OwnerTID = os::Thread::GetCurrentId(); return true; }
		return false;
	}
#endif
	bool IsOwnedByCurrentThread() const { return _OwnerTID == os::Thread::GetCurrentId(); }
	CriticalSection();
	~CriticalSection();
};

class TryEnterCSBlock
{	bool _bEntered;
	CriticalSection& _CCS;
public:
	INLFUNC TryEnterCSBlock(CriticalSection& so):_CCS(so){ _bEntered = so.TryLock(); }
	INLFUNC TryEnterCSBlock(CriticalSection& so, UINT timeout):_CCS(so)
	{	_bEntered = so.TryLock(); 
	}
	INLFUNC ~TryEnterCSBlock(){ if(_bEntered)_CCS.Unlock(); }
	INLFUNC bool IsLocked() const { return _bEntered; }
};

/*
#define EnterRWLBlockShared(x) os::ReaderWriterLock::_RWL_Holder MARCO_JOIN(_RWL_Holder_,__COUNTER__)(x, true);
#define EnterRWLBlockExclusive(x) os::ReaderWriterLock::_RWL_Holder MARCO_JOIN(_RWL_Holder_,__COUNTER__)(x, false);

#define CheckReaderWriterLockRecursiveLock

class ReaderWriterLock
{
#ifdef CheckReaderWriterLockRecursiveLock
	rt::hash_set<int>	locked_threads;
	CriticalSection		locked_threads_cs;
#endif

	bool		_shared_locked;
#if defined(PLATFORM_WIN)
	///////////////////////////////////////////////////////////
	// Lock/Unlock in ReaderWriterLock is not recursive !!!
	#if !defined(PLATFORM_MAX_COMPATIBILITY)
	SRWLOCK		_lock;
public:
	ReaderWriterLock(){ InitializeSRWLock(&_lock); }
	INLFUNC void Lock(bool shared = false)
	{ 
#ifdef CheckReaderWriterLockRecursiveLock
		locked_threads_cs.Lock();
		VERIFY(locked_threads.insert(os::Thread::GetCurrentId()).second);
		locked_threads_cs.Unlock();
#endif
		_shared_locked = shared; if(shared){::AcquireSRWLockShared(&_lock);}else{ ::AcquireSRWLockExclusive(&_lock);} 
		_LOG("Thread "<<os::Thread::GetCurrentId()<<" lock");
	}
	INLFUNC void Unlock()
	{	
#ifdef CheckReaderWriterLockRecursiveLock
		locked_threads_cs.Lock();
		ASSERT(locked_threads.find(os::Thread::GetCurrentId()) != locked_threads.end());
		locked_threads.erase(os::Thread::GetCurrentId());
		locked_threads_cs.Unlock();
#endif
		_LOG("Thread "<<os::Thread::GetCurrentId()<<" unlock");
		if(_shared_locked){::ReleaseSRWLockShared(&_lock);}else{ ::ReleaseSRWLockExclusive(&_lock);} 	
	}
	~ReaderWriterLock(){} // SRW locks do not need to be explicitly destroyed. 
	#else 
public:
	ReaderWriterLock(){ ASSERT(0); }
	INLFUNC void Lock(bool shared = false){ ASSERT(0); }
	INLFUNC void Unlock(){ ASSERT(0); }
	~ReaderWriterLock(){ ASSERT(0); }
	#endif // !defined(PLATFORM_MAX_COMPATIBILITY)
#else
#endif
	class _RWL_Holder
	{
		ReaderWriterLock& _RWL;
	public:
		INLFUNC _RWL_Holder(ReaderWriterLock& so, bool lock_shared):_RWL(so)
		{	_RWL.Lock(lock_shared);
		}
		INLFUNC ~_RWL_Holder(){ _RWL.Unlock(); }
	};
};

*/

class Event
{
	Event(const Event&x){ ASSERT(0); }
#if defined(PLATFORM_WIN)
protected:
	HANDLE	hEvent;
public:	
	FORCEINL bool WaitSignal(DWORD Timeout = INFINITE){ return WaitForSingleObject(hEvent,Timeout) == WAIT_OBJECT_0; }
	FORCEINL bool IsSignaled(){ return WaitSignal(0); }
	FORCEINL void Set(){ VERIFY(::SetEvent(hEvent)); }
	FORCEINL void Reset(){ VERIFY(::ResetEvent(hEvent)); }
	FORCEINL void Pulse(){ VERIFY(::PulseEvent(hEvent)); }
#else
protected:
	CriticalSection	hMutex;
	pthread_cond_t	hEvent;
	bool			bSet;
	bool			bIsPulsed;
public:	
	INLFUNC bool WaitSignal(DWORD Timeout = INFINITE)
	{	EnterCSBlock(hMutex);
		if(Timeout == INFINITE)
		{	while(!bSet)
			{	if(0 != pthread_cond_wait(&hEvent, &hMutex.hCS))return false;
				if(bIsPulsed)
				{	bIsPulsed = false;
					return true;
				}
			}
		}
		else
		{	struct timespec ts;
			{	struct timeval tv;
				gettimeofday(&tv, NULL);
				ts.tv_sec = tv.tv_sec + Timeout/1000;
				ts.tv_nsec = tv.tv_usec*1000 + (Timeout%1000)*1000000;
			}
			while(!bSet)
			{	if(0 != pthread_cond_timedwait(&hEvent,&hMutex.hCS,&ts))return false;
				if(bIsPulsed)
				{	bIsPulsed = false;
					return true;
				}				
			}
		}
		return true;
	}
	FORCEINL bool IsSignaled(){ return bSet; }
	FORCEINL void Pulse()
	{	EnterCSBlock(hMutex);
		bSet = false;
		pthread_cond_signal(&hEvent);
	}
	FORCEINL void Set()
	{	EnterCSBlock(hMutex);
		bSet = true;
		pthread_cond_broadcast(&hEvent);
	}
	FORCEINL void Reset()
	{	EnterCSBlock(hMutex);
		bSet = false;
	}
#endif
	Event();
	~Event();
};


#if defined(PLATFORM_DEBUG_BUILD)
#define ASSERT_NONRECURSIVE									\
	thread_local int _nrcnt = 0;							\
	struct _NRDEC {							\
		INLFUNC ~_NRDEC() { os::AtomicDecrement(&_nrcnt); }	\
	} _nrdec;												\
	ASSERT(os::AtomicIncrement(&_nrcnt) <= 1);
#else
#define ASSERT_NONRECURSIVE  while(0); 
#endif


class ThreadFence
{
	struct _fence
	{	volatile int	_num_reached;
		Event			_release_sign;
	};
	_fence	_fences[2];
	int		_toggle;
	int		_num_thread;
public:
	INLFUNC ThreadFence(){ SetThreadCount(0); }
	INLFUNC void SetThreadCount(int num_thread)
	{	_num_thread = num_thread;
		_toggle = 0;
		_fences[_toggle]._release_sign.Reset();
		_fences[_toggle]._num_reached = 0;
	}
	INLFUNC  int GetThreadCount() const { return _num_thread; }
	FORCEINL bool WaitOthers(bool auto_release_others)	// return true on all others thread are reached, you are the latest one
	{	int reached = os::AtomicIncrement(&_fences[_toggle]._num_reached);
		if(reached < _num_thread)
		{	_fences[_toggle]._release_sign.WaitSignal();
			return false;
		}
		else if(reached == _num_thread)
		{	if(auto_release_others)
				ReleaseOthers();
			return true;
		}
		ASSERT(0);
		return false;
	}
	FORCEINL void ReleaseOthers()	// release all other threads, supposed to be called by the latest reached thread
	{
		_toggle = (_toggle+1)&1;
		_fences[_toggle]._release_sign.Reset();
		_fences[_toggle]._num_reached = 0;
		_fences[(_toggle+1)&1]._release_sign.Set();
	}
};


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

template<typename ITERATOR>
class ProgressMonitor
{
	ITERATOR					_Total;
	ITERATOR*					_pProg;
	HighPerformanceCounter		_Timer;
	Thread						_Thread;
	float						_RebasedPrecentage;
	ULONGLONG					_RebasedTime;
	int							_LastRemainTime;
	int							_UnderestimateCount;
protected:
	void _Display(float precentage, int passed_msec, int remaining_msec)
	{
		printf("%5.2f%% [", 100*precentage);
		for(int i=0;i<30;i++)
		{	float t = i - precentage*30;
			if(t<0)
				putchar('>');
			else if(t<0.5)
				putchar('=');
			else
				putchar('-');
		}
		if(precentage>0.0001f)
			printf("], Remains: %s          \r", (LPCSTR)rt::tos::TimeSpan<>((int)(passed_msec*(1.0f-precentage)/precentage)));
		else
			printf("], Estimating remaining time ...      \r");
	}
	void _MonitorRoute()
	{
		while(!_Thread.WantExit())
		{	int passed_msec = (int)(_Timer.TimeLapse()/1000000);
			float precentage = (*_pProg/(_Total+0.0f));
			int remain = (int)((passed_msec - _RebasedTime)*(1.0f-precentage+_RebasedPrecentage)/(precentage-_RebasedPrecentage));
			_Display(precentage, passed_msec, remain);
			if(remain > _LastRemainTime)
			{	_UnderestimateCount++;
				if(_UnderestimateCount > 20)         
				{	_UnderestimateCount = 0;
					_RebasedTime = passed_msec;
					_RebasedPrecentage = precentage;
				}
			}
			else
			{	_UnderestimateCount = 0;
			}
			_LastRemainTime = remain;
			os::Sleep(500);
		}
		printf("                                                                    \r");
	}
public:
	ProgressMonitor(ITERATOR* prog, ITERATOR total)
	{	_Total = total; _pProg = prog;
		_RebasedPrecentage = 0;
		_RebasedTime = 0;
		_LastRemainTime = 0;
		_UnderestimateCount = 0;
		_Timer.LoadCurrentCount();
		struct _call{ 
		static DWORD _func(LPVOID p){ ((ProgressMonitor*)p)->_MonitorRoute(); return 0; }
		};
		_Thread.Create(_call::_func, this);
	}
	~ProgressMonitor()
	{
		_Thread.WantExit() = true;
		_Thread.WaitForEnding();
	}
};


//////////////////////////////////////////////
// GarbageCollection
class GarbageCollection	// singleton
{
public:
	typedef void (*LPFUNC_DELETION)(LPVOID x);
protected:
	static DWORD _DeletionThread(LPVOID);
	static void	DeleteObject(LPVOID x, DWORD TTL_msec, LPFUNC_DELETION delete_func);
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
};

#define _SafeDel_Delayed(x, TTL_msec)		{ if(x){ os::GarbageCollection::DeleteObj(x,TTL_msec); x=nullptr; } }
#define _SafeDelArray_Delayed(x,TTL_msec)	{ if(x){ os::GarbageCollection::DeleteArray(x,TTL_msec); x=nullptr; } }
#define _SafeFree32AL_Delayed(x,TTL_msec)	{ if(x){ os::GarbageCollection::Delete32AL(x,TTL_msec); x=nullptr; } }
#define _SafeRelease_Delayed(x, TTL_msec)	{ if(x){ os::GarbageCollection::ReleaseObj(x,TTL_msec); x=nullptr; } }

///////////////////////////////////////////////////
// One managing thread is doing Update/Commit/GetBack
// multiple thread consuming thread is doing Lock/Unlock
template<class t_Obj>
class TimeLimitedThreadSafeAccess // The Max using time of the object from Get should < the interval between two Update/Commit
{
	struct _obj_intl: public t_Obj
	{	volatile int	_lock_count;
		_obj_intl(){ _lock_count = 0; }
	};
	_obj_intl				_obj[2];
	volatile _obj_intl*		_front;
	volatile _obj_intl*		_back;
	
public:
	TimeLimitedThreadSafeAccess(){ _front = &_obj[0]; _back = &_obj[1]; }
	FORCEINL const t_Obj* Get() const { return (_obj_intl*)_front; } // not safe, for back-compatility (it is safe when call in the Update/Commit thread)
	FORCEINL t_Obj*	GetBack(){ while(_back->_lock_count)os::Sleep(0); return (_obj_intl*)_back; } // not safe, for back-compatility (it is safe when call in the Update/Commit thread)
	FORCEINL t_Obj*	Lock() // non-wait. better one ensure thread safeness, never wait for other asychronized even while holding the Lock
	{	_obj_intl* ret = (_obj_intl*)_front;
		os::AtomicIncrement(&ret->_lock_count);
		return ret;
	}
	FORCEINL void	Unlock(const t_Obj* p)
	{	os::AtomicDecrement(&((_obj_intl*)p)->_lock_count);
	}
	FORCEINL t_Obj*	Update()
	{	while(_back->_lock_count)os::Sleep(0); // ensure no consuming thread is touching the back buffer
		return (t_Obj*)_back;
	}
	FORCEINL void	Commit(){ ASSERT(_back->_lock_count == 0); rt::Swap(_front, _back); }
};
	
// Thread-Safe, Lock-Free, Triple Buffering, one Consume thread and one produce thread
template<class t_Obj>
class DroppableQueue // The Max using time of the object from Get should < the interval between two Update/Commit
{
	volatile UINT		_Head;
	volatile UINT		_Tail;
	t_Obj				_Bufs[4];
public:
	INLFUNC const t_Obj* Consume_Begin() // fail when empty
	{	if(_Head == _Tail)return nullptr;
		return &_Bufs[_Head&3];
	}
	INLFUNC void Consume_End(){	_Head++; }
	INLFUNC t_Obj& Produce_Begin(){ return _Bufs[_Tail&3]; } // always producable
	INLFUNC void Produce_End()
	{	if((_Head&3) != ((_Tail+1)&3))_Tail++;
	}
};


// Thread-Safe, Lock-Free
// Assuming single Consumer and multiple Producer
// The capacity will be round up to power of 2
template<typename T>
class ProducerConsumerQueue
{
    UINT						_IndexMask;
    rt::Buffer<T>				_Buf;
    rt::Buffer<volatile BYTE>	_BufFin;
    volatile INT				_Count;
    volatile UINT				_Head;
    volatile UINT				_Tail;
    FORCEINL UINT				_SizeInConsumerView()
    {	UINT h = _Head;	UINT t = _Tail;
        return t >= h?(t-h):(((UINT)_Buf.GetSize()) + t - h);
    }
public:
    FORCEINL ProducerConsumerQueue(){ Reset(0); }
    void Reset(UINT capacity)
    {	_Count = 0;
        _Head = 0;
        _Tail = 0;
        if(capacity)
        {	// make power of 2
            capacity = rt::max(2U,capacity);
            _IndexMask = 0x3fffffff;
            for(UINT i=0;i<30;i++)
            {	if(capacity&(~_IndexMask))
			{	if(capacity&_IndexMask)
            {	capacity = (capacity&(~_IndexMask))<<1;
                _IndexMask = (_IndexMask<<1) + 1;
            }
            else { capacity = capacity&(~_IndexMask); }
				break;
			}
                _IndexMask >>= 1;
            }
            _IndexMask = capacity - 1;
            _Buf.SetSize(capacity);
            _BufFin.SetSize(capacity);
            _BufFin.Zero();
        }
        else
        {	_Buf.SetSize(0);
            _BufFin.SetSize(0);
        }
    }
    FORCEINL UINT GetCapacity() const { return _Buf.GetSize(); }
    FORCEINL T* Head()
    {	UINT size = _SizeInConsumerView();
        return (size && _BufFin[_Head&_IndexMask])?&_Buf[_Head&_IndexMask]:nullptr;
    }
    FORCEINL bool Consume()	// remove the head, return true if there is more items. use while((p = Head())==nullptr); to fetch the next
    {
    #if defined(PLATFORM_DEBUG_BUILD)
        UINT size = _SizeInConsumerView();
        ASSERT(size && _BufFin[_Head&_IndexMask]);
    #endif
        _BufFin[_Head&_IndexMask] = 0;
        //_LOG((_Head&_IndexMask)<<" consumed");
        os::AtomicIncrement((volatile INT*)&_Head);
        INT c = os::AtomicDecrement(&_Count);
        ASSERT(c>=0);
        return c != 0;
    }
    FORCEINL T* Produce_Begin(bool* pIsFirstOne = nullptr)
    {	INT c = os::AtomicIncrement(&_Count);
        if(pIsFirstOne)*pIsFirstOne = (c == 1);
        if(c>(INT)_Buf.GetSize())
        {	os::AtomicDecrement(&_Count);
            return nullptr;
        }	
        UINT new_tail = os::AtomicIncrement((volatile INT*)&_Tail);
        UINT pos = (new_tail-1)&_IndexMask;
        ASSERT(_BufFin[pos] == 0);
        return &_Buf[pos];
    }
    FORCEINL void Produce_End(T* p)
    {	ASSERT(_BufFin[p - _Buf] == 0);
        _BufFin[p - _Buf] = 1; 
    }
    template<typename T2>
    FORCEINL bool Produce(T2& x, bool* pIsFirstOne = nullptr)
    {	T* p = Produce_Begin(pIsFirstOne);
        if(p)
        {	*p = x;
            Produce_End(p);
            return true;
        }
        return false;
    }
};


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

	INLFUNC bool		IsUpdating() const { return _UpdateBegin; }
	INLFUNC bool		IsModified() const { return (bool)_Cloned; }
	INLFUNC const auto&	GetUnmodified() const { return _MTM.Get(); }
	INLFUNC auto&		GetModified() const { ASSERT(_Cloned); return *_Cloned; }
	
	INLFUNC bool		ReadyModify(bool from_new = false){ if(!_Cloned)_Cloned = from_new?_MTM.New():_MTM.Clone(); return (bool)_Cloned; }
	INLFUNC void		Revert(){ ASSERT(_UpdateBegin); _SafeDel(_Cloned); _MTM.EndUpdate(nullptr); _UpdateBegin = false; }
	INLFUNC void		Commit(){ ASSERT(_UpdateBegin); _MTM.EndUpdate(_Cloned); _UpdateBegin = false; }
	INLFUNC t_Object*	operator ->(){ ASSERT(_UpdateBegin); ReadyModify(); return _Cloned; }
	INLFUNC t_Object&	Get(){ ASSERT(_UpdateBegin); ReadyModify(); return *_Cloned; }

	INLFUNC ~_TSM_Updater(){ if(_UpdateBegin)Commit(); }
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
	INLFUNC bool	BeginUpdate(bool just_try = false)
					{	if(just_try)return _cs.TryLock();
						_cs.Lock(); return true;
					}
	INLFUNC void	EndUpdate(T* pNew = nullptr) // nullptr indicates no change
					{	if(pNew)
						{	T* pOld = _p;
							_p = pNew;
							_SafeDel_Delayed(pOld, old_TTL);
						}
						_cs.Unlock();
					}
public:
	typedef T			t_Object;
	INLFUNC	ThreadSafeMutable(){ _p = nullptr; }
	INLFUNC ~ThreadSafeMutable(){ Clear();	}

	INLFUNC const T&	Get() const { static T _t; return _p?*_p:_t; }
	INLFUNC const T*	operator -> () const { return &Get(); }
	INLFUNC T*			Clone() const { return _p?_New(T(*_p)):_New(T); }
	INLFUNC T*			New() const { return _New(T); }
	INLFUNC void		Clear(){ _cs.Lock(); _SafeDel(_p); _cs.Unlock(); }
	
	//unsafe in multi-thread
	INLFUNC T&			GetObject(){ ASSERT(_cs.IsOwnedByCurrentThread()); return (T&)Get(); }
};

#define THREADSAFEMUTABLE_UPDATE(org_obj, new_obj)	os::_details::_TSM_Updater<decltype(org_obj)> new_obj(org_obj, false)
#define THREADSAFEMUTABLE_SET(org_obj, new_obj)		os::_details::_TSM_Updater<decltype(org_obj)> new_obj(org_obj, false); new_obj.ReadyModify(true)
#define THREADSAFEMUTABLE_TRYUPDATE(org_obj, new_obj)	os::_details::_TSM_Updater<decltype(org_obj)> new_obj(org_obj, true)


template<typename t_ActualWriter>
class ParallelWriter
{
protected:
	rt::Buffer<BYTE>	_WritePool;
	double				_AvgAttenuation;

protected:
	struct _Chunk
	{	UINT	size;		
		UINT	length;		// INFINITE indicates the chunk is not finalized
		BYTE	data[1];	// real size set is by SetLogEntrySize
		_Chunk*	GetNext(){ return (_Chunk*)(data + size); }
	};
	static const SIZE_T _ChunkHeaderSize = sizeof(UINT)*2;
	struct _WriteBuf
	{	LPBYTE			pBuf;
		volatile int	Used;
	};
	_WriteBuf			_WriteBuffers[2];

protected:
	_WriteBuf*			_FrontBuf;
	_WriteBuf*			_BackBuf;
	UINT				_WriteDownInterval;
	UINT				_WriteBufferSize;
	os::Thread			_WriteThread;

	void _WriteDownBackBuffer()
	{	_Chunk* p = (_Chunk*)_BackBuf->pBuf;
		int used = _BackBuf->Used;
		_Chunk* pend = (_Chunk*)(_BackBuf->pBuf + rt::min<int>(used,(int)(_WriteBufferSize - _ChunkHeaderSize)));
	
		for(;p<pend;p = p->GetNext())
		{	
			if(p->size)
			{	if(p->length==INFINITE){ stat_UnfinalizedChunk++; continue; }
				if(p->length>0)
					if(!((t_ActualWriter*)this)->Write(p->data,p->length))
						stat_FileError++;
			}
			else break;
		}
		((t_ActualWriter*)this)->Flush();

		// Use Rate
		{	int fr;
			if(_BackBuf->Used >= (int)_WriteBufferSize)
			{	fr = 100;	}
			else
			{	fr = 100*(_BackBuf->Used/1024)/(_WriteBufferSize/1024);	}

			if(stat_BufferUsagePeek)
			{
				if(fr > stat_BufferUsagePeek)stat_BufferUsagePeek = fr;
				stat_BufferUsageAvg = (int)(fr + _AvgAttenuation*(stat_BufferUsageAvg - fr) + 0.5);
			}
			else
			{	stat_BufferUsagePeek = fr;
				stat_BufferUsageAvg = fr;
			}
		}

		_BackBuf->Used = 0;
		rt::Zero(_BackBuf->pBuf,_WriteBufferSize);
	}

	void _WriteRoute()
	{	int time_to_sleep = _WriteDownInterval;
		os::Timestamp tm;
        rt::String fn;

		int _last_claim = 0;
		os::Timestamp _last_cps;
		_last_cps.LoadCurrentTime();

		for(;;)
		{
			while(time_to_sleep > 300)
			{	os::Sleep(300);
				time_to_sleep -= 300;
				if(_WriteThread.WantExit())
					goto CLOSING_FILE;
			}

			ASSERT(time_to_sleep > 0);
			os::Sleep(time_to_sleep);

			tm.LoadCurrentTime();

			_WriteDownBackBuffer();
			rt::Swap(_BackBuf,_FrontBuf);

			{	float cps = (stat_TotalClaimed - _last_claim)*1000/(float)(_last_cps.TimeLapse(tm));
				if(stat_ClaimPerSecond<0)
					stat_ClaimPerSecond = cps;
				else stat_ClaimPerSecond = (stat_ClaimPerSecond*9 + cps)/10;

				_last_claim = stat_TotalClaimed;
				_last_cps = tm;
			}

			if(stat_ClaimFailure || stat_FinalizeFailure || stat_FileError || stat_UnfinalizedChunk)
				LogAlert();

			int time_used = (int)tm.TimeLapse();

			{	int IOUage;
				if(	time_used < (int)_WriteDownInterval )
					IOUage = 100*time_used/_WriteDownInterval;
				else
					IOUage = 100;

				if( stat_FileIOUsagePeek )
				{
					if(IOUage>stat_FileIOUsagePeek)stat_FileIOUsagePeek = IOUage;
					stat_FileIOUsageAvg = (int)(IOUage + _AvgAttenuation*(stat_FileIOUsageAvg - IOUage) + 0.5);
				}
				else
				{
					stat_FileIOUsagePeek = IOUage;
					stat_FileIOUsageAvg = IOUage;
				}
			}

			time_to_sleep = rt::min<int>(_WriteDownInterval, rt::max<int>(100, _WriteDownInterval - time_used));
		}	
CLOSING_FILE:
		_WriteDownBackBuffer();
		rt::Swap(_BackBuf,_FrontBuf);
		_WriteDownBackBuffer();
		((t_ActualWriter*)this)->Exit();
	}

public: // statistic
	int					stat_FileError;
	int					stat_UnfinalizedChunk;
	float				stat_ClaimPerSecond;
	int					stat_BufferUsageAvg;			// percentage
	int					stat_BufferUsagePeek;			// percentage
	int					stat_FileIOUsageAvg;			// percentage
	int					stat_FileIOUsagePeek;			// percentage
	volatile int		stat_TotalClaimed;
	volatile int		stat_ClaimFailure;
	volatile int		stat_FinalizeFailure;

public:
	ParallelWriter()
	{	_WriteDownInterval = 0;

		rt::Zero(_WriteBuffers,sizeof(_WriteBuffers));
		_FrontBuf = _WriteBuffers;
		_BackBuf = _FrontBuf + 1;

		stat_BufferUsagePeek = 0;
		stat_BufferUsageAvg = 0;
		stat_FileIOUsageAvg = 0;
		stat_FileIOUsagePeek = 0;

		SetWriteDownInterval(1000);
	}
	~ParallelWriter(){ Close(); }
	bool	Open(UINT buffer_size = 1024*1024)
	{	ASSERT(!_WriteThread.IsRunning());
		stat_ClaimPerSecond = -1;
		stat_BufferUsagePeek = 0;
		stat_BufferUsageAvg = 0;
		stat_FileError = 0;
		stat_UnfinalizedChunk = 0;
		stat_ClaimFailure = 0;
		stat_FinalizeFailure = 0;
		stat_FileIOUsageAvg = 0;			// percentage
		stat_FileIOUsagePeek = 0;			// percentage
		stat_TotalClaimed = 0;

		_WriteBufferSize = (buffer_size + 3) & 0xffffffffc;
		if(_WritePool.SetSize(2*_WriteBufferSize))
		{
			_FrontBuf->pBuf = _WritePool;
			_FrontBuf->Used = 0;
			_BackBuf->pBuf = &_WritePool[_WriteBufferSize];
			_BackBuf->Used = 0;

			struct _call
			{	static DWORD _func(LPVOID pThis)
				{	((ParallelWriter*)pThis)->_WriteRoute();
					return 0;
				}
			};
			return _WriteThread.Create(_call::_func,this);
		}
		_WritePool.SetSize(0);
		return false;
	}
	void	Close()
	{	stat_ClaimPerSecond = -1;
		if(_WriteThread.IsRunning())
		{	_WriteThread.WantExit() = true;
			_WriteThread.WaitForEnding(1000, true);
		}
	}
	void	LogAlert()
	{	static const rt::SS precentage("% / ");
		_LOG_WARNING("ParallelFileWriter Alert"<<
						"\n - Total Claim      : "<<stat_TotalClaimed<<
						"\n - Claim Failure    : "<<stat_ClaimFailure<<
						"\n - Finalize Failure : "<<stat_FinalizeFailure<<
						"\n - Unfinalized      : "<<stat_UnfinalizedChunk<<
						"\n - File I/O Failure : "<<stat_FileError<<
						"\n - Buffer Load      : "<<stat_BufferUsageAvg<<precentage<<stat_BufferUsagePeek<<
						"%\n - File I/O Load    : "<<stat_FileIOUsageAvg<<precentage<<stat_FileIOUsagePeek<<'%'
		);
	}
	bool	IsOpen() const { return _WriteThread.IsRunning(); }
	UINT	GetWriteDownInterval() const { return _WriteDownInterval; }
	void	SetWriteDownInterval(UINT write_interval_msec)
	{	_WriteDownInterval = rt::max<UINT>(100,write_interval_msec);
		_AvgAttenuation = pow(0.64, write_interval_msec/1000.0);		// Attenuate to 0.01 after 10 sec
		ASSERT_FLOAT(_AvgAttenuation);
	}
	UINT	GetBufferSize() const { return _WriteBufferSize; }

	INLFUNC LPSTR ClaimWriting(UINT size)
	{	ASSERT(size);
		_WriteBuf* buf = _FrontBuf;
        int chunk = os::AtomicAdd(size + _ChunkHeaderSize, &buf->Used) - (size + _ChunkHeaderSize);
		os::AtomicIncrement(&stat_TotalClaimed);
		if(chunk + (int)size + (int)_ChunkHeaderSize <= (int)_WriteBufferSize)
		{
			_Chunk* p = (_Chunk*)&buf->pBuf[chunk];
			p->size = size;
			p->length = INFINITE;
			return (LPSTR)p->data;
		}
		else
		{	// this may happen when the writer is much faster then the sync thread
			os::AtomicIncrement(&stat_ClaimFailure);
			return nullptr;
		}
	}

	INLFUNC void FinalizeWritten(LPSTR pBuf, UINT len)
	{	_Chunk* p = (_Chunk*)&pBuf[-((int)_ChunkHeaderSize)];

		if(	len <= p->size &&
			p->length == INFINITE && p->size < _WriteBufferSize)
		{	p->length = len;	}
		else 
			os::AtomicIncrement(&stat_FinalizeFailure);
			// This may happen when it takes too long time between a paired 
			// ClaimWriting/FinalizeWritten calling. e.g. longer than m_WriteDownInterval
			// other possiblity is the caller ruined the buffer (run down)  
	}

	template<class T>
	INLFUNC bool WriteString(const T& x)	// string expression or json definition
	{	
		UINT len = (UINT)x.GetLength();
		LPSTR p = ClaimWriting(len);
		if(p)
		{	VERIFY(len == x.CopyTo((char*)p));
			FinalizeWritten(p,len);
			return true;
		}else return false;
	}

	template<class T>
	INLFUNC bool WriteLine(const T& x)	// string expression or json definition
	{
		UINT len = (UINT)x.GetLength();
		LPSTR p = ClaimWriting(len + 2);
		if(p)
		{	VERIFY(len == x.CopyTo((char*)p));
			*((WORD*)(p + len)) = 0x0a0d;	// "\r\n"
			FinalizeWritten(p,len+2);
			return true;
		}else return false;
	}
	INLFUNC bool WriteString(const rt::String_Ref& str){ ASSERT(str.GetLength()<INT_MAX); return Write(str.Begin(),(UINT)str.GetLength()); }
	INLFUNC bool Write(LPCVOID str, UINT len)
	{	
		LPSTR p = ClaimWriting(len);
		if(p)
		{	memcpy(p,str,len);
			FinalizeWritten(p,len);
			return true;
		}else return false;
	}
};

class ParallelLog
{
protected:
	struct LogE
	{	int  type;
		int  line;
		int  log_len;  // include terminate-zero
		int  filename_len;
		int  funcname_len;
		char text[1];
	};
	struct LogWriter
	{	void Flush(){};
		void Exit(){};
		bool Write(LPCVOID p, UINT size)
		{	const LogE& e = *((LogE*)p);
			os::_details::LogWriteDefault(e.text, &e.text[e.log_len], e.line, &e.text[e.log_len + e.filename_len], e.type, nullptr);
			return true;
		}
	};

	ParallelWriter<LogWriter>	_PW;

public:
	ParallelLog(UINT writedown_interval = 500, UINT buffer_size = 1024*1024)
	{	VERIFY(_PW.Open(buffer_size)); _PW.SetWriteDownInterval(writedown_interval);
		struct _call
		{	static void _log(LPCSTR log, LPCSTR file, int line_num, LPCSTR func, int type, LPVOID cookie)
			{	((ParallelLog*)cookie)->Write(log, file, line_num, func, type);
			}
		};
		os::_details::SetLogWriteFunction(_call::_log, this);
	}
	~ParallelLog(){ _PW.Close(); os::_details::SetLogWriteFunction(); }
	INLFUNC void Write(LPCSTR log, LPCSTR file, int line_num, LPCSTR func, int type)
	{	ASSERT(log && file && func);
		int log_len = (int)strlen(log)+1;
		int filename_len = (int)strlen(file)+1;
		int funcname_len = (int)strlen(func)+1;
		int tot_len = 5*sizeof(int) + log_len + filename_len + funcname_len;
		LogE* e = (LogE*)_PW.ClaimWriting(tot_len);
		if(e)
		{	e->filename_len = filename_len;
			e->funcname_len = funcname_len;
			e->line = line_num;
			e->log_len = log_len;
			e->type = type;
			memcpy(e->text, log, log_len);
			memcpy(e->text + log_len, file, filename_len);
			memcpy(e->text + log_len + filename_len, func, funcname_len);
		}
		_PW.FinalizeWritten((LPSTR)e, tot_len);
	}
};


} // namespace os

