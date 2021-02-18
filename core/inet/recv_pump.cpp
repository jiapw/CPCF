#include "recv_pump.h"


namespace inet
{

bool AsyncIOCoreBase::_Init(os::FUNC_THREAD_ROUTE io_pump, UINT concurrency, UINT stack_size)
{
	ASSERT(_Core == INVALID_HANDLE_VALUE);
	ASSERT(_IOWorkers.GetSize() == 0);

	if(concurrency == 0)
		concurrency = os::GetNumberOfProcessors();

#if defined(PLATFORM_WIN)
	_Core = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, concurrency);
	if(_Core == INVALID_HANDLE_VALUE)goto FAILED;
#elif defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDRIOD)
#else
	#error IOEngine Unsupported Platform
#endif

	BOOL set_cpuaff = (concurrency == os::GetNumberOfProcessors()) && concurrency<sizeof(SIZE_T)*8;
	_IOWorkers.SetSize(concurrency);
	SIZE_T a = set_cpuaff?1:-1;
	for(UINT i=0;i<concurrency;i++)
	{
		VERIFY(_IOWorkers[i].Create(io_pump, this, a, stack_size));
		if(set_cpuaff)
			a <<= 1;
	}
	
	return true;

FAILED:
	Term();
	return false;
}

bool AsyncIOCoreBase::IsRunning() const
{
#if defined(PLATFORM_WIN)
	return _Core != INVALID_HANDLE_VALUE;
#elif defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDRIOD)
#else
#endif

	return false;
}

void AsyncIOCoreBase::Term()
{
	if(!IsRunning())return;

#if defined(PLATFORM_WIN)
	if(_Core != INVALID_HANDLE_VALUE)
	{
		for(UINT i=0;i<_IOWorkers.GetSize();i++)
			VERIFY(PostQueuedCompletionStatus(_Core,0,NULL,NULL));

		::CloseHandle(_Core);
		_Core = INVALID_HANDLE_VALUE;
	}
#elif defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDRIOD)
#else
	#error IOEngine Unsupported Platform
#endif

	_IOWorkers[0].WantExit() = true;
	for(UINT i=0;i<_IOWorkers.GetSize();i++)
		_IOWorkers[i].WaitForEnding(500, TRUE);

	_IOWorkers.SetSize(0);
}

bool AsyncIOCoreBase::_AddObject(AsyncIOCoreBase::IOOBJECT obj, LPVOID cookie)
{
	ASSERT(IsRunning());
#if defined(PLATFORM_WIN)
	HANDLE h = CreateIoCompletionPort(obj, _Core, (ULONG_PTR)cookie, 0);
	DWORD e = ::GetLastError();
	return _Core == h;
#elif defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDRIOD)
#else
	#error IOEngine Unsupported Platform
#endif
}

void AsyncIOCoreBase::_RemoveObject(AsyncIOCoreBase::IOOBJECT obj)
{
	ASSERT(IsRunning());
#if defined(PLATFORM_WIN)
	// in windows, no need to remove things from IOCP as long as the socket or file will be correctly closed
#elif defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDRIOD)
#else
	#error IOEngine Unsupported Platform
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
	//e.is_read = &handler->_pIOObject->_Overlapped_Read == pOverlapped;
	//ASSERT(IsRead || &handler->_pIOObject->_Overlapped_Write == pOverlapped);
#elif defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDRIOD)
#else
	#error IOEngine Unsupported Platform
#endif

	return e.bytes_transferred;
}

} // namespace inet