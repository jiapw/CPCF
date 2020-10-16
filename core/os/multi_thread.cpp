#include "../rt/buffer_type.h"
#include "../rt/json.h"
#include "multi_thread.h"
#include "kernel.h"

/////////////////////////////////////////////////////////////////
// Platform independent implementations
//////////////////////////////////////////////////
// DelayedGarbageCollection
namespace os
{
namespace _details
{
	struct _GarbagItem
	{	LPCVOID			pObj;
		os::DelayedGarbageCollection::LPFUNC_DELETION	pDeletionFunction;
		int				TTL;
		void			Delete(){ pDeletionFunction((LPVOID)pObj); }
		bool			Tick() // return true when the item should be NOT deleted
		{	if(TTL)TTL--;
			return TTL;
		}
	};

	struct _GarbagBin
	{
		rt::BufferEx<_GarbagItem>		_PendingGarbag;
		os::CriticalSection				_PendingGarbagCCS;
		os::Thread						_GarbagDeletionThread;
		bool							_bExit;
		_GarbagBin(){ _bExit = false; }
		~_GarbagBin(){ Exit(); }
		void Exit()
		{
			_bExit = true;
			os::Sleep(10);

			if(_GarbagDeletionThread.IsRunning())
			{
				_GarbagDeletionThread.WantExit() = true;
				_GarbagDeletionThread.WaitForEnding();
			}
		}
	};
	_GarbagBin				g_GCB;
}
} // namespace os

void os::DelayedGarbageCollection::Exit()
{
	_details::g_GCB.Exit();
}

DWORD os::DelayedGarbageCollection::_DeletionThread(LPVOID)
{
	for(;;)
	{
		for(UINT i=0;i<5;i++)
		{
			os::Sleep(200);
			if(_details::g_GCB._GarbagDeletionThread.WantExit())goto DELETION_EXITING;
		}

		{	EnterCSBlock(_details::g_GCB._PendingGarbagCCS);
			UINT open = 0;
			for(UINT i=0;i<_details::g_GCB._PendingGarbag.GetSize();i++)
			{
				if(_details::g_GCB._PendingGarbag[i].Tick())
				{
					_details::g_GCB._PendingGarbag[open] = _details::g_GCB._PendingGarbag[i];
					open++;
				}
				else
				{
					_details::g_GCB._PendingGarbag[i].Delete();
				}
			}

			_details::g_GCB._PendingGarbag.ShrinkSize(open);
		}
	}

DELETION_EXITING:
	Flush();
	return 0;
}

void os::DelayedGarbageCollection::Flush()
{
	EnterCSBlock(_details::g_GCB._PendingGarbagCCS);
	for(UINT i=0;i<_details::g_GCB._PendingGarbag.GetSize();i++)
		_details::g_GCB._PendingGarbag[i].Delete();

	_details::g_GCB._PendingGarbag.SetSize();
}

void os::DelayedGarbageCollection::DeleteObject(LPCVOID x, DWORD TTL_msec, os::DelayedGarbageCollection::LPFUNC_DELETION delete_func)
{
	ASSERT(delete_func);

	if(x == nullptr)return;
	if(TTL_msec == 0 || _details::g_GCB._bExit)
	{	delete_func((LPVOID)x);
		return;
	}

	{	EnterCSBlock(_details::g_GCB._PendingGarbagCCS);
		if(_details::g_GCB._GarbagDeletionThread.IsRunning()){}
		else
		{	_details::g_GCB._GarbagDeletionThread.Create(_DeletionThread, nullptr);
		}

#ifdef PLATFORM_DEBUG_BUILD
		// check if the object is added for collection already (delete twice)
		for(UINT i=0;i<_details::g_GCB._PendingGarbag.GetSize();i++)
			ASSERT(_details::g_GCB._PendingGarbag[i].pObj != x);
#endif

		_details::_GarbagItem& n = _details::g_GCB._PendingGarbag.push_back();
		n.pObj = x;
		n.pDeletionFunction = delete_func;
		n.TTL = (TTL_msec+999)/1000;
	}
}

namespace os
{
namespace _details
{
thread_local bool g_IsMemoryExceptionEnabled = false;
} // namespace _details

bool IsMemoryExceptionEnabledInThread()
{
	return _details::g_IsMemoryExceptionEnabled;
}

void EnableMemoryExceptionInThread(bool y)
{
	_details::g_IsMemoryExceptionEnabled = y;
}
} // namespace os


namespace os
{
namespace _details
{
///////////////////////////////////////////////////////////////////////////////////////////////////
//// 32byte Aligned Memory allocation layout
//// additional 32+1+4 bytes (or 32+1+8 bytes in Win64) is added to every memory block
//// 
////                 |----  Offset  ----|
//// /-- LEN-BYTE --\/-- Prefix bytes -\#/---------- User block -----------\/-- Suffix bytes --\
//// UUUUUUUUUUUUUUUUU ......... UUUUUUUUUUUUUUUUUUUUUUUUUU .... UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU
//// ^                                  ^^
//// |Original                          ||
//// \ allocated        reserved offset /\ first aligned address after LEN-BYTE, the return 
////
//// 1. LEN-BYTE (size_t) indicates the size of User block
//// 
//// 2. If the p is returned by malloc and subtracted by 8, then &p[32-p%32] is the output address
////    and the offset=32-p%32 is saved at p[offset-1] as one byte the legal range of offset is from
////    1 to 32, and this value will be checked when free and used to calculate the original address
//// 
//// 3. The Prefix bytes (size=offset-1) and Suffix bytes (size=33-offset) will be check to ensure 
////    no boundary memory written occurred (buffer overflow)
/////////////////////////////////////////////////////////////////////////////////////////////////////

LPVOID Malloc32AL(size_t size)   //size in byte
{
#pragma warning(disable:4244 4127)	// conversion from 'int' to 'BYTE', possible loss of data
									// conditional expression is constant
	LPBYTE p;
	//32 for alignment, 1 for suffix, 4(or 8) for LEN-BYTE
	if(g_IsMemoryExceptionEnabled)
	{
		if(size >= size+32+1+sizeof(size_t))
			throw std::bad_alloc();

		p = new BYTE[size+32+1+sizeof(size_t)]; 
	}
	else
	{
		if(size >= size+32+1+sizeof(size_t))
			return nullptr;

		p = new(std::nothrow) BYTE[size+32+1+sizeof(size_t)];
	}

#if defined CPF_MEMORY_LEAK_ADDRESS
	if( p == (LPBYTE)CPF_MEMORY_LEAK_ADDRESS )
	{	ASSERT(0);
		//Leaked memory block is located, check [Call Stack] for high-level code
	}
#endif

	if(p)
	{	//Record user block size for determining the high boundary of the memory block
		*((size_t*)p) = size; //size of User block
		p += sizeof(size_t);
		int offset;
		offset = 32 - (int)( ((size_t)p) & 0x1f);	// align to 32
		p[offset-1] = offset;	//Store offset

#if defined CPF_MEMORY_LEAK_ADDRESS
		if( &p[offset] == (LPBYTE)CPF_MEMORY_LEAK_ADDRESS )
		{	ASSERT(0);
			//abnormal used memory block is located, check [Call Stack] for high-level code
		}
#endif
		//Additional guard bytes is set for detecting broundry-overwriting, 
		//which will be checked when memory free. 
		int i;
		for(i=0;i<offset-1;i++)p[i] = 0x61+i;				//Set prefix bytes to 0x61+i = "abcde..."
		for(i=0;i<33-offset;i++)p[offset+size+i] = 0x41+i;	//Set suffix bytes to 0x41+i = "ABCDE..."

#ifdef CPF_REPORT_MEMORY_ACTIVITY
		_LOG("User block [0x"<<((LPVOID)&p[offset])<<"] allocated.\n");
#endif

		return &p[offset];
	}
	else  //Allocation failed
	{	
#ifdef CPF_REPORT_MEMORY_ACTIVITY
		_LOG("\n\nOut of memory: Failed to allocate "<<((size+41)/1024)<<"KB memory block!\7\n");
#endif
		ASSERT(0);
		return nullptr;
	}
}

void Free32AL(LPCVOID ptr_in)
{
	if( ptr_in ) //NULL pointer is allowed but take no effect
	{	
#ifdef CPF_REPORT_MEMORY_ACTIVITY
		_LOG(_T("User block [0x")<<ptr_in<<_T("] try to free.\n"));
#endif
		LPCBYTE ptr = reinterpret_cast<LPCBYTE>(ptr_in);

		bool Prefix_Err_Detected = false; 
		bool Suffix_Err_Detected = false;
		size_t user_block_size = ((size_t)(-1));
		int offset = ptr[-1];
		if( offset <=32 && offset>0 )
		{	//Detect buffer overrun
			user_block_size = *((size_t*)&ptr[-offset-sizeof(size_t)]);

			int i; 
			for(i=0;i<offset-1;i++)
			{// check prefix bytes
				if( ptr[-(offset-i)] == 0x61+i ){}
				else{ Prefix_Err_Detected = true; break; }
			}
			for(i=0;i<33-offset;i++)
			{// check suffix bytes
				if( ptr[user_block_size+i] == 0x41+i ){}
				else{ Suffix_Err_Detected = true; break; }
			}

			if( !Prefix_Err_Detected && !Suffix_Err_Detected )
			{	
				//ensure heap correctness 
				//_CheckHeap;
			
#ifdef CPF_SECURE_MEMORY_RELEASE
				rt::Zero((LPVOID)ptr_in, user_block_size);	// security enforcement
#endif
				delete [] (&ptr[-offset-sizeof(size_t)]);
				return;
			}
		}
		else{ Prefix_Err_Detected = true; }

		LPCSTR emsg = nullptr;
		if(Prefix_Err_Detected && Suffix_Err_Detected)
		{	emsg = ("Both low and high");	}
		else if( Prefix_Err_Detected )
		{	emsg = ("Low");	}
		else if( Suffix_Err_Detected )
		{	emsg = ("High");	}
		else{ ASSERT(0); }

		_LOG("Abnormal block at 0x"<<((LPVOID)ptr)<<", "<<emsg<<" boundry was overwritten.");

		ASSERT(0);
	}
}
#pragma warning(default:4244 4127)

}} // namespace os::_details


#ifdef PLATFORM_DEBUG_BUILD
namespace os
{
namespace _details
{

bool _TMA_Exit = false;

struct _TMA
{
	struct MemBlock
	{
		SIZE_T		Size;
		UINT		MemoSize;
		char		Memo[1];
	};

	os::CriticalSection _CS;
	rt::hash_map<SIZE_T, MemBlock*>	_TrackedMemory;
	~_TMA(){ _TMA_Exit = true; }
};

_TMA& _GetTMA()
{
	static _TMA _;
	return _;
}

LPVOID TrackMemoryAllocation(LPVOID p, SIZE_T sz, bool no_ctor, LPCSTR type, UINT co, LPCSTR fn, LPCSTR func, UINT line)
{
	if(_TMA_Exit || p == nullptr)return p;

	EnterCSBlock(_GetTMA()._CS);
	ASSERT(_GetTMA()._TrackedMemory.find((SIZE_T&)p) == _GetTMA()._TrackedMemory.end());

	// make sure zero memory footprint on heap
	LPCSTR s1 = no_ctor?"Malloc<":"New<";
	LPCSTR s2, s3;

	rt::String_Ref type_trim = rt::String_Ref(type).TrimAfter('(');  // remove ctor arguments

	if(co>1)
	{	auto x = type_trim + '[' + ']';
		s2 = ALLOCA_C_STRING(x);
	}
	else
	{	s2 = ALLOCA_C_STRING(type_trim);
	}

	{	auto x = rt::SS("> by ") + func + "() in " + rt::String_Ref(fn).GetFileName() + ':' + line;
		s3 = ALLOCA_C_STRING(x);
	}

	auto s = rt::SS() + s1 + s2 + s3;

	_TMA::MemBlock* n;
	UINT memo_size = (UINT)s.GetLength();
	if(g_IsMemoryExceptionEnabled)
		n = (_TMA::MemBlock*) new BYTE[memo_size + sizeof(_TMA::MemBlock)];
	else
		n = (_TMA::MemBlock*) new (std::nothrow) BYTE[memo_size + sizeof(_TMA::MemBlock)];

	ASSERT(n);
	n->Size = sz;
	n->MemoSize = memo_size;
	n->Memo[s.CopyTo(n->Memo)] = 0;

	_GetTMA()._TrackedMemory[(SIZE_T&)p] = n;
	return p;
}

void UntrackMemoryAllocation(LPCVOID p)
{
	if(_TMA_Exit)return;

	if(p)
	{
		LPBYTE to_be_del;
		{	EnterCSBlock(_GetTMA()._CS);
			auto it = _GetTMA()._TrackedMemory.find((SIZE_T&)p);
			ASSERT(it != _GetTMA()._TrackedMemory.end());
			to_be_del = (LPBYTE)it->second;
			_GetTMA()._TrackedMemory.erase(it);
		}

		delete [] to_be_del;
	}
}

void DumpTrackedMemoryAllocation(bool verbose)
{
	if(_TMA_Exit)return;

	if(verbose)_LOGC(" ");

	EnterCSBlock(_GetTMA()._CS);
	SIZE_T tot = 0;
	if(_GetTMA()._TrackedMemory.size())
	{
		if(verbose)_LOGC("Dump Tracked Memory Blocks ("<<_GetTMA()._TrackedMemory.size()<<"):");
		for(auto it = _GetTMA()._TrackedMemory.begin(); it != _GetTMA()._TrackedMemory.end(); it++)
		{
			_LOGC("[0x"<<it->first<<"] "<<it->second->Size<<"B "<<it->second->Memo);
			if(verbose)
			{
				auto x = rt::String_Ref((LPCSTR)it->first, rt::min(72, (int)it->second->Size));
				LPSTR d = ALLOCA_C_STRING(x);
				for(int i=0; d[i]; i++)
					if(d[i]<' ')d[i] = ' ';
				_LOGC("    = \""<<d<<'"');
			}
			tot += it->second->Size;
		}
		if(verbose)_LOGC("Tracked memory blocks take "<<rt::tos::FileSize<>(tot));
	}
	else
	{	if(verbose)_LOGC("No tracked memory blocks");
	}
}

void TrackedMemoryAllocationStatistic(rt::Json& json)
{
	static const UINT top_alloc_max = 16;

	rt::TopWeightedValues<rt::String_Ref, top_alloc_max*3, ULONGLONG>	top_alloc;

	ULONGLONG total_size = 0;
	ULONGLONG total_memo_size = 0;
	ULONGLONG total_block = 0;

	{	EnterCSBlock(_GetTMA()._CS);
		for(auto it = _GetTMA()._TrackedMemory.begin(); it != _GetTMA()._TrackedMemory.end(); it++)
		{
			total_size += it->second->Size;
			total_memo_size += it->second->MemoSize + sizeof(_TMA::MemBlock);
			top_alloc.Sample(rt::String_Ref(it->second->Memo), it->second->Size);
		}
		total_block = _GetTMA()._TrackedMemory.size();

		// make sure zero memory footprint on heap
		// dump strings to stack, so that we can safely release the lock then compose the json output
		for(UINT i=0; i<top_alloc_max; i++)
		{
			rt::String_Ref& a = top_alloc.Get(i);
			LPSTR memo = (LPSTR)_alloca(a.GetLength());
			a.CopyTo(memo);
			a._p = memo;
		}
	}

	json.Object((
		J(TotalAllocation) = total_size,
		J(TotalBlocks) = total_block,
		J(TotalMemo) = total_memo_size
	));

	{	auto scope = json.ScopeAppendingKey("TopAllocators");
		json.Array();
		for(UINT i=0; i<10; i++)
		{
			rt::String_Ref memo;
			ULONGLONG size;
			UINT count;
			if(!top_alloc.Get(i, &memo, &size, &count))break;
			json << (
				J(Allocator) = rt::JsonEscapeString(memo),
				J(TotalSize) = size,
				J(TotalBlocks) = count
			);
		}
	}
}

}} // namespace os::_details
#endif

bool os::Thread::WaitForEnding(UINT time_wait_ms, bool terminate_if_timeout)
{
	Sleep(0);
	while(time_wait_ms > (UINT)100)
	{	
		if(!IsRunning())return true;
		Sleep(100);
		if(time_wait_ms!=INFINITE)time_wait_ms -= 100;
	}

	if(terminate_if_timeout)
		TerminateForcely();

	return false;
}

os::Thread::Thread()
{
	__MFPTR_Obj = nullptr;
	__CB_Func = nullptr;
	_hThread = NULL;
	_ExitCode = INFINITE;
}

os::Thread::~Thread()
{
	ASSERT(_hThread == NULL);
}

bool os::Thread::Create(LPVOID obj, const THISCALL_MFPTR& on_run, LPVOID param, ULONGLONG cpu_aff, UINT stack_size)
{
	ASSERT(_hThread == NULL);

	__CB_Func = nullptr;
	__MFPTR_Obj = obj;
	__MFPTR_Func = on_run;
	__CB_Param = param;

	return _Create(stack_size, cpu_aff);
}

bool os::Thread::Create(FUNC_THREAD_ROUTE x, LPVOID thread_cookie, ULONGLONG cpu_aff, UINT stack_size)
{
	ASSERT(_hThread == NULL);

	__CB_Func = x;
	__CB_Param = thread_cookie;
	__MFPTR_Obj = nullptr;
	__MFPTR_Func.Zero();

	return _Create(stack_size, cpu_aff);
}

DWORD os::Thread::_Run()
{
#if defined(PLATFORM_ANDROID)
	struct _thread_call
	{	static void thread_exit_handler(int sig)
		{ 	pthread_exit(0);
	}	};

	struct sigaction actions;
	memset(&actions, 0, sizeof(actions)); 
	sigemptyset(&actions.sa_mask);
	actions.sa_flags = 0; 
	actions.sa_handler = _thread_call::thread_exit_handler;
	sigaction(SIGUSR2,&actions,NULL);
#endif

	while(!_hThread)os::Sleep(10);
	os::Sleep(10);

	DWORD ret;
	if(__MFPTR_Obj)
	{
		ret = THISCALL_POLYMORPHISM_INVOKE(OnRun, __MFPTR_Obj, __MFPTR_Func, __CB_Param);
	}
	else
	{	
		ASSERT(__CB_Func);
		ret = __CB_Func(__CB_Param);
	}

	if(ret != THREAD_OBJECT_DELETED_ON_RETURN)
	{
		_ExitCode = ret;
		__release_handle(_hThread);
		_hThread = NULL;
	} // else never touch this again.

	return ret;
}

namespace os
{

namespace _details
{
thread_local UINT __Thread_Label = 0;
} // namespace _details


void Thread::SetLabel(UINT thread_label)
{
	_details::__Thread_Label = thread_label;
}

UINT Thread::GetLabel()
{
	return _details::__Thread_Label;
}

} // namespace os


//////////////////////////////////////////////////////////
// All Windows implementations
#if defined(PLATFORM_WIN)

void os::Thread::__release_handle(HANDLE hThread)
{
	::CloseHandle(hThread);
}

bool os::Thread::_Create(UINT stack_size, ULONGLONG CPU_affinity)
{
	ASSERT(_hThread == NULL);
	_bWantExit = false;

	struct _call
	{	static DWORD WINAPI _func(LPVOID p)
		{	return ((Thread*)p)->_Run();
	}	};

	_hThread = ::CreateThread(NULL, stack_size, _call::_func, this, 0, &_ThreadId);
	SetThreadAffinityMask(_hThread, (DWORD_PTR&)CPU_affinity);
	return _hThread != NULL;
}

UINT os::Thread::GetId()
{
	return _ThreadId;
}

void os::Thread::SetPriority(UINT p)
{
	SetThreadPriority(_hThread, p);
}

void os::Thread::TerminateForcely()
{
	if(_hThread)
	{
		_LOG_WARNING("Thread #"<<GetId()<<" will be terminated forcely, which is unsafe in most cases.");

		::TerminateThread(_hThread, -1);
		__release_handle(_hThread);
		_hThread = NULL;
	}
}

bool os::Thread::IsRunning() const
{
	if(_hThread == NULL)return false;
	return ::WaitForSingleObject(_hThread, 0) == WAIT_TIMEOUT;
}


#else
//////////////////////////////////////////////////////////
// All linux/BSD implementations
#include <pthread.h>
#if defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
#import <mach/thread_act.h>
#endif

bool os::Thread::IsRunning() const
{
	return _hThread != NULL;
}

bool os::Thread::_Create(UINT stack_size, ULONGLONG CPU_affinity)
{
	ASSERT(_hThread == NULL);
	_bWantExit = false;

	pthread_attr_t attr;
	pthread_attr_t* set_attr = nullptr;

    struct _call
    {    static LPVOID _func(LPVOID p)
        {    return (LPVOID)(unsigned long)((Thread*)p)->_Run();
        }    };

#if defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
    if(stack_size)
    {
        pthread_attr_init(&attr);
        if(stack_size)
            pthread_attr_setstacksize(&attr, rt::max((int)PTHREAD_STACK_MIN,(int)stack_size));
        set_attr = &attr;
    }
    
    if(CPU_affinity != 0xffffffffffffffffULL)
    {
        if(pthread_create_suspended_np((pthread_t*)&_hThread, set_attr, _call::_func, this))
        {
            _hThread = NULL;
            return false;
        }
        
        mach_port_t mach_thread = pthread_mach_thread_np(*(pthread_t*)&_hThread);
        thread_affinity_policy_data_t policyData[64];
        int count = 0;
        for(UINT i=0; i<sizeof(SIZE_T)*8; i++)
            if(CPU_affinity&(1ULL<<i))
                policyData[count].affinity_tag = i;
        
        thread_policy_set(mach_thread, THREAD_AFFINITY_POLICY, (thread_policy_t)&policyData, count);
        thread_resume(mach_thread);
        return true;
    }
    
#else
	if(stack_size || CPU_affinity != 0xffffffffffffffffULL)
	{
		pthread_attr_init(&attr);
		
		if(stack_size)
			pthread_attr_setstacksize(&attr, rt::max((int)PTHREAD_STACK_MIN,(int)stack_size));
		
		if(CPU_affinity != 0xffffffffffffffffULL)
		{
			cpu_set_t cpuset;
			rt::Zero(cpuset);
			for(UINT i=0; i<sizeof(SIZE_T)*8; i++)
				if(CPU_affinity&(1ULL<<i))
					CPU_SET(i, &cpuset);
		
			#ifndef PLATFORM_ANDROID
			pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpuset);
			#endif
		}
		set_attr = &attr;
	}
#endif

    if(0 == pthread_create((pthread_t*)&_hThread, set_attr, _call::_func, this))
        return true;

	_hThread = NULL;
	return false;
}

UINT os::Thread::GetId()
{
#if defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
	return (SIZE_T)pthread_mach_thread_np(*(pthread_t*)&_hThread);
#else
	return *(SIZE_T*)&_hThread;
#endif
}

void os::Thread::SetPriority(UINT p)
{
	struct sched_param sp;
	sp.sched_priority = p;
	pthread_setschedparam(*(pthread_t*)&_hThread, SCHED_OTHER, &sp);
}

void os::Thread::TerminateForcely()
{
	if(_hThread)
	{
#if defined(PLATFORM_ANDROID)
		pthread_kill(*(pthread_t*)&_hThread, SIGUSR2);
#else
		pthread_cancel(*(pthread_t*)&_hThread);
#endif
		_hThread = NULL;
	}
}

void os::Thread::__release_handle(HANDLE hThread)
{
	pthread_detach((pthread_t&)hThread);
}

#endif


#if defined(PLATFORM_WIN)
////////////////////////////////////////////////////////////
// CLaunchProcess
os::LaunchProcess::LaunchProcess()
{
	_hProcess = INVALID_HANDLE_VALUE;
	hChildStdoutRdDup = INVALID_HANDLE_VALUE;
	hChildStdinRd = INVALID_HANDLE_VALUE;
	hChildStdinWrDup = INVALID_HANDLE_VALUE;
	hChildStdoutWr = INVALID_HANDLE_VALUE;

	_Callback = nullptr;

	_ExitCode = 0;
	_ExitTime = 0;
	_ExecutionTime = 0;
}

os::LaunchProcess::~LaunchProcess()
{
	IsRunning();
	_ClearAll();
}


void os::LaunchProcess::_ClearAll()
{	
	_OutputHookThread.WantExit() = true;
	_OutputHookThread.WaitForEnding();

	#define _SafeCloseHandle(x) { if((x)!=INVALID_HANDLE_VALUE){ ::CloseHandle(x); x=INVALID_HANDLE_VALUE; } }

	_SafeCloseHandle(_hProcess);

	_SafeCloseHandle(hChildStdoutRdDup);	//make hook thread exit
	_SafeCloseHandle(hChildStdinRd);
	_SafeCloseHandle(hChildStdinWrDup);
	_SafeCloseHandle(hChildStdoutWr);


	#undef _SafeCloseHandle
}


bool os::LaunchProcess::Launch(LPCSTR cmdline, DWORD flag, LPCSTR pWorkDirectory, LPCSTR pEnvVariable)
{
	VERIFY(!IsRunning());

	rt::String cmd(cmdline);
	if(cmd.IsEmpty())return false;

	_Flag = flag;
	bool hook_output = (FLAG_SAVE_OUTPUT&flag) || (FLAG_ROUTE_OUTPUT&flag) || _Callback!=NULL;

	PROCESS_INFORMATION piProcInfo;
	STARTUPINFOW siStartInfo;

	rt::Zero( &piProcInfo, sizeof(PROCESS_INFORMATION) );
	rt::Zero( &siStartInfo, sizeof(STARTUPINFO) );
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.wShowWindow = (WORD)(flag&FLAG_HIDE_WINDOW)?SW_HIDE:SW_SHOW;
	siStartInfo.dwFlags = STARTF_USESHOWWINDOW;

	if(hook_output)
	{	
		HANDLE hChildStdoutRd,hChildStdinWr;
		/////////////////////////////////////////////////////////////////////////
		// Creating a Child Process with Redirected Input and Output
		// 1. create pipes
		SECURITY_ATTRIBUTES saAttr;
		bool fSuccess; 

		// Set the bInheritHandle flag so pipe handles are inherited. 
		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = true; 
		saAttr.lpSecurityDescriptor = NULL;

		// Create a pipe for the child process's STDOUT. 
		::CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0);

		// Create non-inheritable read handle and close the inheritable read handle. 
		fSuccess = DuplicateHandle(	GetCurrentProcess(), hChildStdoutRd,
									GetCurrentProcess(),&hChildStdoutRdDup , 0,
									false, DUPLICATE_SAME_ACCESS);
		CloseHandle(hChildStdoutRd);

		// Create a pipe for the child process's STDIN. 
		::CreatePipe(&hChildStdinRd, &hChildStdinWr, &saAttr, 0);

		// Duplicate the write handle to the pipe so it is not inherited. 
		fSuccess = DuplicateHandle(	GetCurrentProcess(), hChildStdinWr, 
									GetCurrentProcess(), &hChildStdinWrDup, 0, 
									false, DUPLICATE_SAME_ACCESS); 
		CloseHandle(hChildStdinWr);

		///////////////////////////////////////////////////////
		// 2. create child process
		siStartInfo.hStdError = hChildStdoutWr;
		siStartInfo.hStdOutput = hChildStdoutWr;
		siStartInfo.hStdInput = hChildStdinRd;
		siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

		_hProcess = INVALID_HANDLE_VALUE;
		_Output.Empty();
		_OutputHookThread.Create(_OutputHookRoutine, this);
		_OutputHookThread.SetPriority(os::Thread::PRIORITY_HIGH);
	}

	_ExitCode = STILL_ACTIVE;

	rt::Buffer<WCHAR>	env;
	if(pEnvVariable)
	{
		LPCWSTR s = ::GetEnvironmentStringsW();
		UINT s_len=0;
		for(;;s_len++)
			if(s[s_len] == 0 && s[s_len+1] == 0)
				break;

		UINT a_len=0;
		for(;;a_len++)
			if(pEnvVariable[a_len] == 0 && pEnvVariable[a_len+1] == 0)
				break;

		os::__UTF16 addon(rt::String_Ref(pEnvVariable, a_len));

		env.SetSize(a_len + 1 + s_len + 1 + 1);
		env.Zero();
		memcpy(&env[0], s, sizeof(WCHAR)*s_len);
		memcpy(&env[s_len+1], addon.Begin(), sizeof(WCHAR)*a_len);
		
		pEnvVariable = (LPCSTR)env.Begin();
	}

	// Create the child process. 
	bool ret =
	CreateProcessW(	NULL, 
					os::__UTF16(cmd).Begin(),	// command line 
					NULL,				// process security attributes 
					NULL,				// primary thread security attributes 
					true,				// handles are inherited 
					pEnvVariable?CREATE_UNICODE_ENVIRONMENT:0,				// creation flags 
					(LPVOID)pEnvVariable,
					os::__UTF16(pWorkDirectory),
					&siStartInfo,		// STARTUPINFO pointer 
					&piProcInfo);		// receives PROCESS_INFORMATION 

	if(ret)
	{	
		_hProcess = piProcInfo.hProcess;
		CloseHandle( piProcInfo.hThread );
		return true;
	}
	else
	{
		_LOG_WARNING("Error launching process: "<<cmd<<" ERR: "<<GetLastError());
		_ClearAll();
		return false;
	}
}

LPCSTR os::LaunchProcess::GetOutput()
{
	return _Output;
}

UINT os::LaunchProcess::GetOutputLen()
{
	return (UINT)_Output.GetLength();
}

void os::LaunchProcess::CopyOutput(rt::String& out)
{
	EnterCSBlock(_CCS);
	out = _Output;
}

bool os::LaunchProcess::WaitForEnding(DWORD timeout)
{
	os::TickCount t;
	t.LoadCurrentTick();

	::Sleep(500);

	while(IsRunning())
	{	::Sleep(500);
		if((DWORD)t.TimeLapse()>timeout)return false;
	}

	return true;
}

void os::LaunchProcess::Terminate()
{
	if(IsRunning())
	{
		::TerminateProcess(_hProcess,-1);
		os::Sleep(0);
		while(IsRunning())os::Sleep(1);
		_ClearAll();
	}
}

bool os::LaunchProcess::IsRunning()
{
	if(_hProcess!=INVALID_HANDLE_VALUE)
	{
		bool exited = false;
		VERIFY(::GetExitCodeProcess(_hProcess,(LPDWORD)&_ExitCode));
		exited = (_ExitCode!=STILL_ACTIVE);

		if(exited)
		{	
			FILETIME creat,exit,foo;
			GetProcessTimes(_hProcess,&creat,&exit,&foo,&foo);

			if(*((__int64*)&exit))
			{
				_ExecutionTime = (UINT)((((ULONGLONG&)exit) - ((ULONGLONG&)creat))/10000);
				_ExitTime = (*((__int64*)&exit))/10000LL - 11644473600000LL;
			
				_ClearAll();
				return false;
			}
		}
		return true;
	}

	return false;
}

void os::LaunchProcess::_HookedOutput(char* buffer, UINT dwRead)
{
	if(_Flag&FLAG_ROUTE_OUTPUT){ buffer[dwRead]=0; printf(buffer); }
	if(_Flag&FLAG_SAVE_OUTPUT)
	{	EnterCSBlock(_CCS);
		int i = (int)_Output.GetLength();
		_RemoveCarriageReturn(_Output, rt::String_Ref(buffer, dwRead));
	}
	if(_Callback)
	{	_Callback(buffer, dwRead, _Callback_Cookie);
	}
}


DWORD os::LaunchProcess::_OutputHookRoutine(LPVOID p)
{
	LaunchProcess* pThis = (LaunchProcess*)p;

	char buffer[1024];

	pThis->_ExitCode = STILL_ACTIVE;

	DWORD dwRead,exitcode;
	exitcode = STILL_ACTIVE;
	while(exitcode==STILL_ACTIVE)
	{	
		if(PeekNamedPipe(pThis->hChildStdoutRdDup,NULL,0,NULL,&dwRead,0))
		{	if( dwRead )
			{	if( ReadFile( pThis->hChildStdoutRdDup, buffer, rt::min((DWORD)sizeof(buffer)-1,dwRead), &dwRead, NULL) && dwRead )
				{	pThis->_HookedOutput(buffer,dwRead);
					continue;
				}
			}
		}

		Sleep(100);
		if(pThis->hChildStdoutRdDup==INVALID_HANDLE_VALUE)return 0;
		if(pThis->_hProcess != INVALID_HANDLE_VALUE)
			GetExitCodeProcess(pThis->_hProcess,&exitcode);
		else return 0;
	} 

	// check for words before death of the process
	do
	{	dwRead = 0;
		if(PeekNamedPipe(pThis->hChildStdoutRdDup,NULL,0,NULL,&dwRead,0))
		{	if( dwRead )
			{	if( ReadFile( pThis->hChildStdoutRdDup, buffer, rt::min((DWORD)sizeof(buffer)-1,dwRead), &dwRead, NULL) && dwRead )
					pThis->_HookedOutput(buffer,dwRead);
			}
		}
	}while(dwRead);

	pThis->_ExitCode = exitcode;
	return 0;
}

bool os::LaunchProcess::SendToStdin(LPCVOID str, UINT len)
{
	DWORD wlen = 0;
	return WriteFile(hChildStdinWrDup, str, len, &wlen, NULL) && wlen == len;
}

void os::LaunchProcess::_RemoveCarriageReturn(rt::String& output, const rt::String_Ref& add)
{
	if(add.IsEmpty())return;

	LPCSTR p = add.Begin();
	LPCSTR end = add.End();
	LPCSTR last_linend = nullptr;
	LPCSTR last_copied = p;

	if(!output.IsEmpty() && output.Last() == '\r')
	{
		if(*p == '\n')
		{	p++;
			output += '\n';
			last_copied = last_linend = p;
		}
		else
		{	int i = (int)output.FindCharacterReverse('\n');
			if(i<0){ output.Empty(); }
			else{ output.SetLength(i+1); }
			last_linend = p;
		}
	}
	else
	{	LPCSTR s = p;
		while(*s != '\r' && *s != '\n' && s < end)s++;
		if(*s == '\r')
		{
			if(s+1 < end)
			{	
				if(s[1] == '\n')
				{	last_linend = s+2;
				}
				else
				{	int i = (int)output.FindCharacterReverse('\n');
					if(i<0){ output.Empty(); }
					else{ output.SetLength(i + 1); }
					p = last_copied = last_linend = s + 1;
				}
			}
			else
			{	output += rt::String_Ref(p,s+1);
				return;
			}	
		}
		else if(s == end)
		{
			output += add;
			return;
		}
		else
		{	ASSERT(*s == '\n');
			last_linend = p = s+1;
		}
	}

	for(;p < end;p++)
	{

		if(*p == '\n'){ last_linend = p+1; }
		else if(*p == '\r')
		{
			if(p+1<end)
			{
				if(p[1] == '\n')
				{	last_linend = p + 2;
					p++;
				}
				else
				{	ASSERT(last_linend >= last_copied);
					if(last_linend != last_copied)
					{	output += rt::String_Ref(last_copied,last_linend); 
					}
					last_copied = last_linend = p + 1;
				}
			}
			else
			{
				output += rt::String_Ref(last_copied, p+1);
				return;
			}
		}
	}

	if(last_copied < end)
		output += rt::String_Ref(last_copied, end);
}


void os::LaunchProcess::SetOutputCallback(FUNC_HOOKEDOUTPUT func, LPVOID cookie)
{
	_Callback = func;
	_Callback_Cookie = cookie;
}

bool os::LaunchProcess::SendInput(LPCSTR p, UINT len)
{
	if(hChildStdinWrDup)
	{	DWORD w;
		return WriteFile(hChildStdinWrDup, p, len, &w, NULL) && w == len;
	}
	return false;
}

#endif // #if defined(PLATFORM_WIN)
