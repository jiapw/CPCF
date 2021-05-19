#include "datagram_pump.h"

#if defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
#include <sys/epoll.h>
#include <unistd.h>
#elif defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
#include <sys/event.h>
#include <unistd.h>
#endif


namespace inet
{

#if defined(PLATFORM_WIN)
void DatagramSocket::_InitBuf(UINT mtu, UINT concurrency)
{
	ASSERT(concurrency);
	ASSERT(mtu);

	_Concurrency = concurrency;
	_RecvBlockSize = mtu + offsetof(RecvBlock, DataBuf);
	VERIFY(_ConcurrentRecvBuf.ChangeSize(_RecvBlockSize*concurrency, false));
	_ConcurrentRecvBuf.Zero();
	for(UINT i=0; i<_Concurrency; i++)
	{
		auto& block = _GetRecvBlock(i);
		block.Index = i;
		block.hSocket = _hSocket;
		block.Packet.RecvBuf = block.DataBuf;
		block.Packet.RecvBufSize = mtu;
	}
}
#endif

bool DatagramSocket::Create(const InetAddrV6 &bind_to, bool reuse_addr)
{
	return Socket::Create(bind_to, SOCK_DGRAM, reuse_addr);
}

bool DatagramSocket::Create(const InetAddr &bind_to, bool reuse_addr)
{
	return Socket::Create(bind_to, SOCK_DGRAM, reuse_addr);
}

bool AsyncDatagramCoreBase::_Init(os::FUNC_THREAD_ROUTE io_pump, UINT concurrency, UINT stack_size)
{
	ASSERT(!IsRunning());
	ASSERT(_IOWorkers.GetSize() == 0);

	if(concurrency == 0)
		concurrency = os::GetNumberOfProcessors();

#if defined(PLATFORM_WIN)
	_Core = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, concurrency);
#elif defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
    _Core = kqueue();
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
	_Core = epoll_create(1);
#else
	#error AsyncIOCore Unsupported Platform
#endif

	if(IsRunning())
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

void AsyncDatagramCoreBase::Term()
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
        ::close(c);
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
		::close(c);
#else
	#error AsyncIOCore Unsupported Platform
#endif
	}

#if defined(PLATFORM_IOS)
    for(UINT i=0;i<_IOWorkers.GetSize();i++)
        _IOWorkers[i].WaitForEnding(100, true);
#else
	for(UINT i=0;i<_IOWorkers.GetSize();i++)
		_IOWorkers[i].WaitForEnding();
#endif
    
	_IOWorkers.SetSize(0);
}

bool AsyncDatagramCoreBase::_AddObject(SOCKET obj, LPVOID cookie)
{
	ASSERT(IsRunning());
#if defined(PLATFORM_WIN)
	HANDLE h = CreateIoCompletionPort((HANDLE)obj, _Core, (ULONG_PTR)cookie, 0);
	DWORD e = ::GetLastError();
	return _Core == h;
#elif defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
    struct kevent event;
    rt::Zero(event);
    EV_SET(&event, obj, EVFILT_READ, EV_ADD|EV_ENABLE, 0, 0, cookie);
    return kevent(_Core, &event, 1, NULL, 0, NULL) != -1;
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
	epoll_event epevt;
	epevt.data.ptr = cookie;
	epevt.events = EPOLLIN;
	return epoll_ctl(_Core, EPOLL_CTL_ADD, obj, &epevt) == 0;
#else
	#error AsyncIOCore Unsupported Platform
#endif
}

void AsyncDatagramCoreBase::_RemoveObject(SOCKET obj)
{
	ASSERT(IsRunning());
#if defined(PLATFORM_WIN)
	// in windows, no need to remove things from IOCP as long as the socket or file will be correctly closed
#elif defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
    struct kevent event;
    EV_SET(&event, obj, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    VERIFY(kevent(_Core, &event, 1, NULL, 0, NULL) != -1);
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
	epoll_event epevt;
	VERIFY(epoll_ctl(_Core, EPOLL_CTL_DEL, obj, &epevt) == 0);
	// In kernel versions before 2.6.9, the EPOLL_CTL_DEL operation required a non-NULL pointer in event, even though this argument is ignored.
#else
	#error AsyncIOCore Unsupported Platform
#endif
}

struct _FD
{	fd_set	_fd;
	_FD(SOCKET s){ FD_ZERO(&_fd); FD_SET(s, &_fd); }
	operator fd_set*(){ return &_fd; }
};

bool DatagramSocket::__SendTo(LPCVOID pData, UINT len, LPCVOID addr, int addr_len, bool drop_if_busy)
{
	int ret = 0;
	static const timeval timeout = { 0, 100000 }; // 100 msec

	do
	{	
		ret = (int)sendto(_hSocket,(const char*)pData,len,0,(const sockaddr*)addr,addr_len);
		if(ret == len)return true;
	}while(	!drop_if_busy &&
			ret < 0 &&
			IsLastOpPending() &&
			(select(1 + (int)_hSocket, NULL, _FD(_hSocket), NULL, (timeval*)&timeout)) == 1
		  );

	return false;
}

#if defined(PLATFORM_IOS) || defined(PLATFORM_MAC) || defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
namespace _details
{
template<int SIZE, int ITER = 0, bool STOP = ITER == SIZE>
struct CopyAllEvents
{
#if defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
    template<typename T>
    static void Copy(AsyncDatagramCoreBase::Event& evt, const T* ke, int batch)
    {   if(ITER < batch)
        {   evt.cookies[ITER] = ke[ITER].udata;
            CopyAllEvents<SIZE,ITER+1>::Copy(evt, ke, batch);
        }
    }
#else
    template<typename T>
    static void Copy(AsyncDatagramCoreBase::Event& evt, const T* ke, int batch)
    {   if(ITER < batch)
        {
            if(ke[ITER].events&EPOLLIN)
                evt.cookies[evt.count++] = ke[ITER].data.ptr;
            CopyAllEvents<SIZE,ITER+1>::Copy(evt, ke, batch);
        }
    }
#endif
};
    template<int SIZE, int ITER>
    struct CopyAllEvents<SIZE, ITER, true>
    {
        template<typename T>
        static void Copy(AsyncDatagramCoreBase::Event& evt, const T* ke, int batch){}
    };
} // namespace _details
#endif

bool AsyncDatagramCoreBase::_PickUpEvent(Event& e)
{
    if(!IsRunning())return false;

#if defined(PLATFORM_WIN)
	OVERLAPPED*	pOverlapped = NULL;
	::GetQueuedCompletionStatus(_Core, 
								&e.bytes_transferred, 
								(PULONG_PTR)&e.cookie,
								(LPOVERLAPPED*)&e.overlapped, INFINITE
								);
	if(!e.overlapped)return false;
	ASSERT(e.cookie);
	return true;
#elif defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
    struct kevent evts[AsyncDatagramCoreBase::EVENT_BATCH_SIZE];
    int batch = 0;
    if((batch = kevent(_Core, NULL, 0, evts, AsyncDatagramCoreBase::EVENT_BATCH_SIZE, NULL)) > 0 && IsRunning())
    {
        _details::CopyAllEvents<sizeofArray(evts)>::Copy(e, evts, batch);
        e.count = batch;
        return true;
    }
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDROID)
	epoll_event epevts[AsyncDatagramCoreBase::EVENT_BATCH_SIZE];
    int batch = 0;
	if((batch = epoll_wait(_Core, epevts, AsyncDatagramCoreBase::EVENT_BATCH_SIZE, 200)) > 0 && IsRunning())
	{
        e.count = 0;
        _details::CopyAllEvents<sizeofArray(epevts)>::Copy(e, epevts, batch);
		return true;
	}
#else
	#error AsyncIOCore Unsupported Platform
#endif

	return false;
}

#if defined(PLATFORM_WIN)
namespace _details
{
} // namespace _details

bool DatagramSocket::RecvBlock::PumpNext()
{
	ASSERT(Packet.RecvBuf == (LPBYTE)DataBuf);
	WSABUF b = { (UINT)Packet.RecvBufSize, (LPSTR)Packet.RecvBuf };

	Packet.PeerAddressSize = sizeof(InetAddrV6);
	Packet.RecvSize = 0;
	DWORD flag = 0;
	if(::WSARecvFrom(hSocket, &b, 1, &Packet.RecvSize, &flag, (sockaddr*)&Packet.PeerAddressFamily, &Packet.PeerAddressSize, &Overlapped, nullptr) != SOCKET_ERROR)
		return true;

	auto last_error = ::WSAGetLastError();
	if(last_error == WSA_IO_PENDING)return true;
	if(last_error == WSA_INVALID_HANDLE || last_error == WSA_OPERATION_ABORTED)
	{	
		hSocket = INVALID_SOCKET; // already closed by other thread
		goto FAILED;
	}

	if(inet::Socket::IsErrorUnrecoverable(last_error))goto FAILED;
	
	// attmpt to recover
	for(UINT i=0; i<3; i++)
	{
		Packet.PeerAddressSize = sizeof(InetAddrV6);
		Packet.RecvSize = 0;
		DWORD flag = 0;
		if(::WSARecvFrom(hSocket, &b, 1, &Packet.RecvSize, &flag, (sockaddr*)&Packet.PeerAddressFamily, &Packet.PeerAddressSize, &Overlapped, nullptr) != SOCKET_ERROR)
			return true;

		auto err = ::WSAGetLastError();
		if(err == WSA_IO_PENDING)return true;
		if(err == last_error || inet::Socket::IsErrorUnrecoverable(err))
			break;

		last_error = err;
	}

FAILED:
	if(hSocket != INVALID_SOCKET)
		inet::Socket(hSocket).Close();  // interrupt all pending recv on 

	Overlapped.hEvent = INVALID_HANDLE_VALUE;

	_LOG_WARNING("PumpNext fatal error ="<<last_error);
	return false;
}
#endif

} // namespace inet
