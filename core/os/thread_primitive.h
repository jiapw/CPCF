#pragma once

/**
 * @file thread_primitive.h
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
/** \addtogroup os 
 * @ingroup CPCF
 *  @{
 */
#include "predefines.h"
#include "../rt/runtime_base.h"
#include "../rt/type_traits.h"

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

#define EnterCSBlock(x) os::CriticalSection::_CCS_Holder MARCO_JOIN(_CCS_Holder_,__COUNTER__)(x);

#if defined(PLATFORM_DEBUG_BUILD)
#define ASSERT_NONRECURSIVE									\
	thread_local volatile int _nrcnt = 0;					\
	struct _NRDEC {											\
		INLFUNC ~_NRDEC() { os::AtomicDecrement(&_nrcnt); }	\
	} _nrdec;												\
	ASSERT(os::AtomicIncrement(&_nrcnt) <= 1);
#else
#define ASSERT_NONRECURSIVE  while(0); 
#endif

namespace os
{
/** \addtogroup thread_primitive
 * @ingroup os
 *  @{
 */
/**
 * @brief All Atomic operation return the value after operation, EXCEPT AtomicOr and AtomicAnd 
 * 
 */
#if defined(PLATFORM_WIN)
	FORCEINL int		AtomicIncrement(volatile int *theValue){ return _InterlockedIncrement((long volatile *)theValue); }
	FORCEINL int		AtomicDecrement(volatile int *theValue){ return _InterlockedDecrement((long volatile *)theValue); }
	FORCEINL int		AtomicAdd(int theAmount, volatile int *theValue){ return theAmount + _InterlockedExchangeAdd((long volatile *)theValue, (long)theAmount); }
	FORCEINL void		AtomicSet(int val, volatile int *theValue){ AtomicAdd(val-*theValue, theValue); }
	FORCEINL DWORD		AtomicOr(DWORD bits, volatile DWORD* theValue){ return (DWORD)_InterlockedOr((volatile long*)theValue, bits); }
	FORCEINL DWORD		AtomicAnd(DWORD bits, volatile DWORD* theValue){ return (DWORD)_InterlockedAnd((volatile long*)theValue, bits); }
	#if defined(PLATFORM_64BIT)
	FORCEINL __int64	AtomicIncrement(volatile __int64 *theValue){ return _InterlockedIncrement64((__int64 volatile *)theValue); }
	FORCEINL __int64	AtomicDecrement(volatile __int64 *theValue){ return _InterlockedDecrement64((__int64 volatile *)theValue); }
	FORCEINL __int64	AtomicAdd(__int64 theAmount, volatile __int64 *theValue){ return theAmount + _InterlockedExchangeAdd64((__int64 volatile *)theValue, (__int64)theAmount); }
	FORCEINL void		AtomicSet(__int64 val, volatile __int64 *theValue){ AtomicAdd(val-*theValue, theValue); }
	FORCEINL ULONGLONG	AtomicOr(ULONGLONG bits, volatile ULONGLONG* theValue){ return (ULONGLONG)_InterlockedOr64((volatile __int64*)theValue, bits); }
	FORCEINL ULONGLONG	AtomicAnd(ULONGLONG bits, volatile ULONGLONG* theValue){ return (ULONGLONG)_InterlockedAnd64((volatile __int64*)theValue, bits); }
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
	FORCEINL DWORD		AtomicOr(DWORD bits, volatile DWORD* theValue){ return (DWORD)OSAtomicOr32Orig((uint32_t)bits, (volatile uint32_t*)theValue); }
	FORCEINL DWORD		AtomicAnd(DWORD bits, volatile DWORD* theValue){ return (DWORD)OSAtomicAnd32Orig((uint32_t)bits, (volatile uint32_t*)theValue); }
    // Mac seems missing native support to bit ops of 64-bit
    // FORCEINL ULONGLONG    AtomicOr(ULONGLONG bits, volatile ULONGLONG* theValue){ return (ULONGLONG)OSAtomicOr64((uint64_t)bits, (volatile uint64_t*)theValue); }
	// FORCEINL ULONGLONG	AtomicAnd(ULONGLONG bits, volatile ULONGLONG* theValue){ return (ULONGLONG)OSAtomicAnd64((uint64_t)bits, (volatile uint64_t*)theValue); }
#else
	FORCEINL int		AtomicIncrement(volatile int *theValue){ return 1 + __sync_fetch_and_add(theValue,1); }
	FORCEINL int		AtomicDecrement(volatile int *theValue){ return __sync_fetch_and_sub(theValue,1) - 1; }
	FORCEINL __int64	AtomicIncrement(volatile __int64 *theValue){ return 1 + __sync_fetch_and_add(theValue,1); }
	FORCEINL __int64	AtomicDecrement(volatile __int64 *theValue){ return __sync_fetch_and_sub(theValue,1) - 1; }
	FORCEINL int		AtomicAdd(int theAmount, volatile int *theValue){ return theAmount + __sync_fetch_and_add(theValue, theAmount); }
	FORCEINL void		AtomicSet(int val, volatile int *theValue){ AtomicAdd(val-*theValue, theValue); }
	FORCEINL __int64	AtomicAdd(__int64 theAmount, volatile __int64 *theValue){ return theAmount + __sync_fetch_and_add(theValue, theAmount); }
	FORCEINL void		AtomicSet(__int64 val, volatile __int64 *theValue){ AtomicAdd(val-*theValue, theValue); }
	FORCEINL DWORD		AtomicOr(DWORD bits, volatile DWORD* theValue){ return (DWORD)__sync_fetch_and_or((volatile uint32_t*)theValue, (uint32_t)bits); }
	FORCEINL ULONGLONG	AtomicOr(ULONGLONG bits, volatile ULONGLONG* theValue){ return (ULONGLONG)__sync_fetch_and_or((volatile uint64_t*)theValue, (uint64_t)bits); }
	FORCEINL DWORD		AtomicAnd(DWORD bits, volatile DWORD* theValue){ return (DWORD)__sync_fetch_and_and((volatile uint32_t*)theValue, (uint32_t)bits); }
	FORCEINL ULONGLONG	AtomicAnd(ULONGLONG bits, volatile ULONGLONG* theValue){ return (ULONGLONG)__sync_fetch_and_and((volatile uint64_t*)theValue, (uint64_t)bits); }
#endif

INLFUNC SIZE_T GetCurrentThreadId()
{
#if defined(PLATFORM_WIN)
	return ::GetCurrentThreadId();
#elif defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
    uint64_t tid;
    VERIFY(pthread_threadid_np(NULL, &tid) == 0);
    return (SIZE_T)tid;
#else
	return (SIZE_T)pthread_self();
#endif
}

class AtomicLock  ///< a non-waiting CriticalSection, no thread-recursive
{
	volatile int	_iAtom;
public:
	FORCEINL AtomicLock(){ _iAtom = 0; }
	FORCEINL void Reset(){ _iAtom = 0; }
	FORCEINL bool TryLock()	///< must call Unlock ONCE, or Reset, if return true
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
#pragma warning(disable:4512) ///< assignment operator could not be generated
protected:
	CRITICAL_SECTION hCS;
public:
	FORCEINL void Lock()
	{	EnterCriticalSection(&hCS);
		if(_OwnerTID == GetCurrentThreadId())
		{	_Recurrence++;	}
		else{ _OwnerTID = GetCurrentThreadId(); _Recurrence = 1; }
	}
	FORCEINL void Unlock()
	{	_Recurrence--;
		if(_Recurrence == 0)_OwnerTID = 0;
		LeaveCriticalSection(&hCS); 
	}
	FORCEINL bool TryLock()
	{	if(TryEnterCriticalSection(&hCS))
		{	if(_OwnerTID == GetCurrentThreadId()){ _Recurrence++; }
			else{ _OwnerTID = GetCurrentThreadId(); _Recurrence = 1; }
			return true;
		}	
		return false;
	}
#pragma warning(default:4512) ///< assignment operator could not be generated
	CriticalSection(){ InitializeCriticalSection(&hCS); _OwnerTID = 0; }
	~CriticalSection(){ ASSERT(_OwnerTID == 0); DeleteCriticalSection(&hCS); }
#else
protected:
	friend class Event;
	pthread_mutex_t hCS;
public:
    FORCEINL void Lock()
    {   VERIFY(0 == pthread_mutex_lock(&hCS));
        if(_OwnerTID == GetCurrentThreadId())
        {    _Recurrence++;    }
        else{ _OwnerTID = GetCurrentThreadId(); _Recurrence = 1; }
    }
    FORCEINL void Unlock()
    {   _Recurrence--;
        if(_Recurrence == 0)_OwnerTID = 0;
        VERIFY(0 == pthread_mutex_unlock(&hCS));
    }
    FORCEINL bool TryLock()
    {   if(0 == pthread_mutex_trylock(&hCS))
        {   if(_OwnerTID == GetCurrentThreadId()){ _Recurrence++; }
            else{ _OwnerTID = GetCurrentThreadId(); _Recurrence = 1; }
            return true;
        }
        return false;
    }
	CriticalSection()
	{ 	pthread_mutexattr_t attributes;
		pthread_mutexattr_init(&attributes);
		pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_RECURSIVE);
	
		VERIFY(0==pthread_mutex_init(&hCS, &attributes));
	
		pthread_mutexattr_destroy(&attributes);
		_OwnerTID = 0;
	}
	~CriticalSection(){	pthread_mutex_destroy(&hCS); ASSERT(_OwnerTID == 0); }
#endif // #if defined(PLATFORM_WIN)

	bool IsOwnedByCurrentThread() const { return _OwnerTID == GetCurrentThreadId(); }
};

class TryEnterCSBlock
{	bool _bEntered;
	CriticalSection& _CCS;
public:
	INLFUNC TryEnterCSBlock(CriticalSection& so):_CCS(so){ _bEntered = so.TryLock(); }
	INLFUNC ~TryEnterCSBlock(){ if(_bEntered)_CCS.Unlock(); }
	INLFUNC bool IsLocked() const { return _bEntered; }
	INLFUNC operator bool () const { return _bEntered; }
};

class Event
{
	Event(const Event&x){ ASSERT(0); }
#if defined(PLATFORM_WIN)
protected:
	HANDLE	hEvent;
public:	
	bool WaitSignal(DWORD Timeout = INFINITE){ return WaitForSingleObject(hEvent,Timeout) == WAIT_OBJECT_0; }
	bool IsSignaled(){ return WaitSignal(0); }
	void Set(){ VERIFY(::SetEvent(hEvent)); }
	void Reset(){ VERIFY(::ResetEvent(hEvent)); }
	void Pulse(){ VERIFY(::PulseEvent(hEvent)); }
	Event(){ VERIFY(hEvent = ::CreateEvent(NULL,true,false,NULL)); }
	~Event(){ ::CloseHandle(hEvent); }
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
				if(bIsPulsed){ bIsPulsed = false; return true; }
		}	}
		else
		{	struct timespec ts;
			{	struct timeval tv;
				gettimeofday(&tv, NULL);
				ts.tv_sec = tv.tv_sec + Timeout/1000;
				ts.tv_nsec = tv.tv_usec*1000 + (Timeout%1000)*1000000;
			}
			while(!bSet)
			{	if(0 != pthread_cond_timedwait(&hEvent,&hMutex.hCS,&ts))return false;
				if(bIsPulsed){ bIsPulsed = false; return true; }				
		}	}
		return true;
	}
	FORCEINL bool IsSignaled(){ return bSet; }
	FORCEINL void Pulse(){ EnterCSBlock(hMutex); bSet = false; pthread_cond_signal(&hEvent); }
	FORCEINL void Set(){ EnterCSBlock(hMutex); bSet = true; pthread_cond_broadcast(&hEvent); }
	FORCEINL void Reset(){ EnterCSBlock(hMutex); bSet = false; }
	Event(){ VERIFY(0 == pthread_cond_init(&hEvent,NULL)); bSet = false; }
	~Event(){ pthread_cond_destroy(&hEvent); }
#endif
};

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
	FORCEINL bool WaitOthers(bool auto_release_others)	///< return true on all others thread are reached, you are the latest one
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
	FORCEINL void ReleaseOthers()	///< release all other threads, supposed to be called by the latest reached thread
	{
		_toggle = (_toggle+1)&1;
		_fences[_toggle]._release_sign.Reset();
		_fences[_toggle]._num_reached = 0;
		_fences[(_toggle+1)&1]._release_sign.Set();
	}
};
/** @}*/
} // namespace os
/** @}*/