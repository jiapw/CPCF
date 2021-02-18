#include "recv_pump.h"

#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDRIOD)
#include <sys/epoll.h>
#include <unistd.h>
#elif defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
#endif


namespace inet
{

bool AsyncIOCoreBase::_Init(os::FUNC_THREAD_ROUTE io_pump, UINT concurrency, UINT stack_size)
{
	ASSERT(!IsRunning());
	ASSERT(_IOWorkers.GetSize() == 0);

	if(concurrency == 0)
		concurrency = os::GetNumberOfProcessors();

#if defined(PLATFORM_WIN)
	_Core = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, concurrency);
#elif defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDRIOD)
	_Core = epoll_create(1);
#else
	#error AsyncIOCore Unsupported Platform
#endif

	if(_Core != IOCORE_INVALID)
	{
		bool set_cpuaff = (concurrency == os::GetNumberOfProcessors()) && concurrency<sizeof(SIZE_T)*8;
		_IOWorkers.SetSize(concurrency);
		SIZE_T a = set_cpuaff?1:-1;
		for(UINT i=0;i<concurrency;i++)
		{
			VERIFY(_IOWorkers[i].Create(io_pump, this, a, stack_size));
			if(set_cpuaff)
				a <<= 1;
		}
		
		return true;
	}

	Term();
	return false;
}

void AsyncIOCoreBase::Term()
{
	if(!IsRunning())return;

	_IOWorkers[0].WantExit() = true;

	if(IsRunning())
	{
		IOCORE c = _Core;
		_Core = (IOCORE)IOCORE_INVALID;
		
#if defined(PLATFORM_WIN)
		for(UINT i=0;i<_IOWorkers.GetSize();i++)
			VERIFY(PostQueuedCompletionStatus(c,0,NULL,NULL));

		::CloseHandle(c);
#elif defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDRIOD)
		::close(c);
#else
	#error AsyncIOCore Unsupported Platform
#endif
	}

	for(UINT i=0;i<_IOWorkers.GetSize();i++)
		_IOWorkers[i].WaitForEnding();

	_IOWorkers.SetSize(0);
}

bool AsyncIOCoreBase::_AddObject(SOCKET obj, LPVOID cookie)
{
	ASSERT(IsRunning());
#if defined(PLATFORM_WIN)
	HANDLE h = CreateIoCompletionPort((HANDLE)obj, _Core, (ULONG_PTR)cookie, 0);
	DWORD e = ::GetLastError();
	return _Core == h;
#elif defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDRIOD)
	epoll_event epevt;
	epevt.data.ptr = cookie;
	epevt.events = EPOLLIN;
	return epoll_ctl(_Core, EPOLL_CTL_ADD, obj, &epevt) == 0;
#else
	#error AsyncIOCore Unsupported Platform
#endif
}

void AsyncIOCoreBase::_RemoveObject(SOCKET obj)
{
	ASSERT(IsRunning());
#if defined(PLATFORM_WIN)
	// in windows, no need to remove things from IOCP as long as the socket or file will be correctly closed
#elif defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDRIOD)
	epoll_event epevt;
	VERIFY(epoll_ctl(_Core, EPOLL_CTL_DEL, obj, &epevt) == 0);
	// In kernel versions before 2.6.9, the EPOLL_CTL_DEL operation required a non-NULL pointer in event, even though this argument is ignored.
#else
	#error AsyncIOCore Unsupported Platform
#endif
}

bool AsyncIOCoreBase::_PickUpEvent(Event& e)
{
#if defined(PLATFORM_WIN)
	OVERLAPPED*	pOverlapped = NULL;
	::GetQueuedCompletionStatus(_Core, 
								&e.bytes_transferred, 
								(PULONG_PTR)&e.cookie,
								(LPOVERLAPPED*)&pOverlapped, INFINITE
								);
	if(!pOverlapped)return false;
	ASSERT(e.cookie);
	return e.bytes_transferred;
#elif defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDRIOD)
	epoll_event epevt;
	if(epoll_wait(_Core, &epevt, 1, 200) == 1 && (epevt.events&EPOLLIN) && IsRunning())
	{
		e.cookie = epevt.data.ptr;
		return true;
	}
#else
	#error AsyncIOCore Unsupported Platform
#endif

	return false;
}

} // namespace inet