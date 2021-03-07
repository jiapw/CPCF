#include "inet.h"
#include <string.h>
#include <ctype.h>


#ifdef PLATFORM_WIN
#include <Iphlpapi.h>
#include <Ipifcons.h>
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

#else
#include <netdb.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>

#if defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
#include <sys/sockio.h>
#include <net/if_dl.h>
#if defined(PLATFORM_IOS)
#include <CoreFoundation/CoreFoundation.h>
#include <notify_keys.h>
#else
#include <sys/kern_event.h>
#endif // #if defined(PLATFORM_IOS)
#else
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#endif

#endif

namespace inet
{

#ifdef PLATFORM_WIN
struct _w32_socket_init
{	
	_w32_socket_init()
	{	WORD wVersionRequested = MAKEWORD(2,2);
		WSADATA wsaData;
		WSAStartup(wVersionRequested, &wsaData);
	}
	~_w32_socket_init(){ WSACleanup(); }
};

_w32_socket_init _w32_socket_init_obj;
#endif

void GetHostName(rt::String& name_out)
{
	char name[1024];
	if(0 == gethostname(name, sizeof(name)))
	{	
		LPSTR end = strchr(name, '.');
		if(end)
		{	name_out = rt::String(name, end);
		}
		else
		{	name_out = name;
		}
	}
	else
	{	name_out.Empty();
	}
}

template<typename t_ADDR>
UINT GetLocalAddressT(t_ADDR* pOut, UINT OutSize, bool no_loopback, t_ADDR* broadcast_addr = nullptr, DWORD* subnet_mask = nullptr, LPCSTR interface_prefix = nullptr, rt::String* if_names = nullptr)
{
	typedef _details::InetAddrT_Op<typename t_ADDR::ADDRESS_TYPE>	OP;
	ASSERT(OutSize);
    
	UINT nextAddr = 0;
#if defined(PLATFORM_WIN)
    ASSERT(interface_prefix == nullptr); // not supported on Windows
	ASSERT(if_names == nullptr); // not supported on Windows
	if(OP::SIN_FAMILY == AF_INET)
	{	// IP v4
		ULONG buflen = sizeof(MIB_IPADDRROW)*128;
		MIB_IPADDRTABLE* ipaddrtable = (MIB_IPADDRTABLE*)_alloca(buflen);
		if(NO_ERROR == GetIpAddrTable(ipaddrtable, &buflen, false))
		{
			for(UINT i=0; i<(UINT)ipaddrtable->dwNumEntries; i++)
			{
				MIB_IPADDRROW& ipt = ipaddrtable->table[i];
				pOut[nextAddr].SetBinaryAddress(&ipt.dwAddr);
				if(	!OP::IsAddressNone(pOut[nextAddr]) &&
					!OP::IsAddressLoopback(pOut[nextAddr]) &&
					!OP::IsAddressAny(pOut[nextAddr]) &&
					!OP::IsAddressGhost(pOut[nextAddr])
				)
				{
                    if(broadcast_addr)
					{	DWORD bcast = ipt.dwAddr|~ipt.dwMask;
						broadcast_addr[nextAddr].SetBinaryAddress(&bcast);
					}
					if(subnet_mask)
					{	subnet_mask[nextAddr] = ipt.dwMask;
					}
					nextAddr++;
					if(nextAddr >= OutSize)break;
				}
			}
		}
	}
	else
	{	// AF_INET6
		ASSERT(broadcast_addr == nullptr);
		ASSERT(subnet_mask == nullptr);

		char szHostname[256];
		gethostname(szHostname, sizeof(szHostname));

		struct addrinfo aiHints;
		struct addrinfo *aiList = nullptr;
		//memset(&aiHints, 0, sizeof(aiHints));
		rt::Zero(aiHints);
		aiHints.ai_family = OP::SIN_FAMILY;
		int ret = false;
		if(0 == getaddrinfo(szHostname, NULL, &aiHints, &aiList) && aiList)
		{	
			struct addrinfo *p = aiList;
			while(p)
			{
				if(	p->ai_addrlen == sizeof(t_ADDR) &&
					!OP::IsAddressNone((t_ADDR&)*p->ai_addr) &&
					!OP::IsAddressLoopback((t_ADDR&)*p->ai_addr) &&
					!OP::IsAddressAny((t_ADDR&)*p->ai_addr) &&
					!OP::IsAddressGhost((t_ADDR&)*p->ai_addr)
				)
				{	OP::CopyAddress(OP::GetAddressPtr(pOut[nextAddr]), (t_ADDR&)*p->ai_addr);
					nextAddr++;
					if(nextAddr >= OutSize)break;
				}
				p = p->ai_next;
			}
		}
		if(aiList)freeaddrinfo(aiList);
	}
#else

    rt::String_Ref nic_prefix[64];
    int nic_prefix_co = rt::String_Ref(interface_prefix).Split(nic_prefix, sizeofArray(nic_prefix), ",;");
    int top_nic_idx = 100;

    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
    {
        if(!ifa->ifa_addr)continue;
        if(ifa->ifa_addr->sa_family != OP::SIN_FAMILY)continue;
        if((ifa->ifa_flags & IFF_UP) == 0)continue;  // ignore if interface not up
/*
        if(ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
             // is a valid IP4 Address
             tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
             char addressBuffer[INET_ADDRSTRLEN];
             inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
             printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
        } else if (ifa->ifa_addr->sa_family == AF_INET6) { // check it is IP6
             // is a valid IP6 Address
             tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
             char addressBuffer[INET6_ADDRSTRLEN];
             inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
             printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
        }
*/
        int prefix_idx = -1;
        if(nic_prefix_co)
        {
            for(UINT i=0; i<nic_prefix_co; i++)
                if(rt::String_Ref(ifa->ifa_name).StartsWith(nic_prefix[i]))
                {   prefix_idx = (int)i;
                    goto GET_LISTED;
                }
            
            continue;
        }
        
GET_LISTED:
        t_ADDR& addr = *(t_ADDR*)ifa->ifa_addr;
        if( !OP::IsAddressNone(addr) &&
            !OP::IsAddressLoopback(addr) &&
            !OP::IsAddressAny(addr) &&
            !OP::IsAddressGhost(addr)
        )
        {   UINT add_idx = nextAddr;
            if(prefix_idx <= top_nic_idx)  // take the last IP for a weird scenario that the real IPv6 IP of dpd_ip on iOS is the last one.
            {   // add in front if a preferred NIC found
                add_idx = 0;
                top_nic_idx = prefix_idx;
        
                pOut[nextAddr] = pOut[0];
                if(if_names)if_names[nextAddr] = if_names[0];
                if(subnet_mask)subnet_mask[nextAddr] = subnet_mask[0];
                if(broadcast_addr)broadcast_addr[nextAddr] = broadcast_addr[0];
            }
                          
            pOut[add_idx] = addr;
            if(if_names)if_names[add_idx] = ifa->ifa_name;
            if(broadcast_addr)broadcast_addr[add_idx] = *(t_ADDR*)ifa->ifa_broadaddr;
            if(subnet_mask)
            {
                ASSERT(ifa->ifa_addr->sa_family == AF_INET);
                subnet_mask[add_idx] = *(DWORD*)(ifa->ifa_netmask->sa_data+2);
            }

            nextAddr++;
            if(nextAddr >= OutSize)break;
        }
     }
     if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
#endif

	if(!no_loopback && OutSize>(UINT)nextAddr)
	{	pOut[nextAddr++].SetAsLoopback();
	}

	return nextAddr;
}

extern UINT GetLocalAddresses(InetAddrT<sockaddr_in>* pOut, UINT Size_InOut, bool no_loopback, InetAddrT<sockaddr_in>* pOut_Broadcast, DWORD* subnet_mask, LPCSTR interface_prefix, rt::String* if_names)
{
	UINT co = GetLocalAddressT<InetAddrT<sockaddr_in>>(pOut, Size_InOut, no_loopback, pOut_Broadcast, subnet_mask, interface_prefix, if_names);
	return co;
}

extern UINT GetLocalAddresses(InetAddrV6* pOut, UINT Size_InOut, bool no_loopback, InetAddrV6* pOut_Broadcast, LPCSTR interface_prefix, rt::String* if_names)
{
	UINT co = GetLocalAddressT<InetAddrV6>(pOut, Size_InOut, no_loopback, pOut_Broadcast, nullptr, interface_prefix, if_names);
	return co;
}

////////////////////////////////////////////////////////////
// Socket
int Socket::GetLastError()
{
#ifdef PLATFORM_WIN
	return WSAGetLastError();
#else
	return errno;
#endif
}

bool Socket::IsLastErrorUnrecoverableForDatagram()
{
	auto errcode = GetLastError();
#if defined(PLATFORM_WIN)
	return errcode != WSAECONNRESET && errcode != WSAENETRESET && errcode != WSAEMSGSIZE && errcode != 0;
#else
	return true;
#endif
}

bool Socket::IsLastOpPending()
{
	int e = GetLastError();
#if defined(PLATFORM_WIN)
	return e == WSAEWOULDBLOCK || e == WSAEINPROGRESS;
#else
	return e == EINPROGRESS || e == EWOULDBLOCK;
#endif
}

Socket::Socket()
{
	m_hSocket = INVALID_SOCKET;
}

Socket::Socket(SOCKET s)
{
	m_hSocket = s;
}

bool Socket::__Create(const struct sockaddr &BindTo, int addr_len, int nSocketType, bool reuse_addr, int AF)
{
	ASSERT(m_hSocket == INVALID_SOCKET);
	m_hSocket = socket(AF, nSocketType, 0);

	if(INVALID_SOCKET != m_hSocket)
	{
		int on = 1;
		if(SOCK_STREAM == nSocketType)
		{	linger l = {1,0};
			VERIFY(0==::setsockopt(m_hSocket,SOL_SOCKET,SO_LINGER,(char*)&l,sizeof(linger)));
#if defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
			VERIFY(0==::setsockopt(m_hSocket,SOL_SOCKET,SO_NOSIGPIPE,(void *)&on, sizeof(on)));
#endif
		}
#if defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
		else if(SOCK_DGRAM == nSocketType)
		{
			VERIFY(0==::setsockopt(m_hSocket,SOL_SOCKET,SO_BROADCAST,(void *)&on, sizeof(on)));
		}
#endif
		if(reuse_addr)
		{	VERIFY(0==setsockopt(m_hSocket,SOL_SOCKET,SO_REUSEADDR,(char*)&on,sizeof(on)));
//			VERIFY(0==::setsockopt(m_hSocket,SOL_SOCKET,SO_REUSEPORT,(char *)&on, sizeof(on)));
		}
		
		if(0==bind(m_hSocket,&BindTo,addr_len))
		{
			return true;
		}
	}
	
	
	
	_LOG_WARNING("Socket Error = "<<GetLastError());
	Close();
	return false;
}

void Socket::Attach(SOCKET hSocket)
{
	ASSERT(m_hSocket == INVALID_SOCKET);
	m_hSocket = hSocket;
	ASSERT(m_hSocket != INVALID_SOCKET);
}

SOCKET Socket::Detach()
{
	SOCKET s;
	s = m_hSocket;
	m_hSocket = INVALID_SOCKET;
	return s;
}

void Socket::Close()
{
	if(m_hSocket != INVALID_SOCKET)
	{
		SOCKET tmp = m_hSocket;
		m_hSocket = INVALID_SOCKET;

		int val;
		SOCKET_SIZE_T len = sizeof(val);
#if !defined(PLATFORM_LINUX) && !defined(PLATFORM_ANDROID)
		if(	(getsockopt(tmp,SOL_SOCKET,SO_TYPE,(char*)&val,&len) == 0) &&
			(val == SOCK_STREAM)	// cause deadlock with RecvFrom on SOCK_DGRAM socket
		)
#endif
		{	shutdown(tmp,2);	 // SD_BOTH == 2
		}

#ifdef PLATFORM_WIN
		closesocket(tmp);
#else
		close(tmp);
#endif
	}
}

bool Socket::__GetPeerName(struct sockaddr &ConnectedTo, int addr_len) const
{
	ASSERT(m_hSocket != INVALID_SOCKET);
	SOCKET_SIZE_T size = addr_len;
	return getpeername(m_hSocket,&ConnectedTo,&size)==0 && size == addr_len;
}

bool Socket::__GetBindName(struct sockaddr &bind, int addr_len)	const	// address of this socket
{
	ASSERT(m_hSocket != INVALID_SOCKET);
	SOCKET_SIZE_T size = addr_len;
	return getsockname(m_hSocket,&bind,&size)==0;
}

bool Socket::__ConnectTo(const struct sockaddr &target, int addr_len)
{
	return connect(m_hSocket,&target,addr_len)==0;
}

bool Socket::IsValid() const
{
    if(INVALID_SOCKET == m_hSocket)return false;

	int val;
	SOCKET_SIZE_T len = sizeof(val);
	return getsockopt(m_hSocket,SOL_SOCKET,SO_TYPE,(char*)&val,&len) == 0;
}

bool Socket::IsConnected() const
{
	if(!IsValid())return false;

	InetAddrV6 peeraddr;
	SOCKET_SIZE_T size = sizeof(InetAddrV6);
	return getpeername(m_hSocket,peeraddr,&size)==0;
}

bool Socket::Send(LPCVOID pData, UINT len)
{
	return len==send(m_hSocket,(const char*)pData,len,0);
}

bool Socket::__SendTo(LPCVOID pData, UINT len,const struct sockaddr &target, int addr_len)
{
	return len==sendto(m_hSocket,(const char*)pData,len,0,&target,addr_len);
}

bool Socket::Recv(LPVOID pData, UINT len, UINT& len_out, bool Peek)
{
	UINT l = (UINT)recv(m_hSocket,(char*)pData,len,Peek?MSG_PEEK:0);
    if(l==SOCKET_ERROR)
    {   len_out = 0;
        return false;
    }
	len_out = l;
	return true;
}

bool Socket::__RecvFrom(LPVOID pData, UINT len, UINT& len_out, struct sockaddr &target, int addr_len, bool Peek)
{
	SOCKET_SIZE_T la = addr_len;
	int l = (int)recvfrom(m_hSocket,(char*)pData,len,Peek?MSG_PEEK:0,&target,&la);
    if(l==SOCKET_ERROR)
    {   len_out = 0;
        return false;
    }
	len_out = l;
	return la == addr_len;
}

bool Socket::SetBufferSize(int reserved_size, bool receiving_sending)
{
	return 0 == setsockopt(m_hSocket,SOL_SOCKET,receiving_sending?SO_RCVBUF:SO_SNDBUF,(char*)&reserved_size,sizeof(int));
}

bool Socket::Listen(UINT pending_size)
{
	return 0 == listen(m_hSocket,pending_size);
}
	
bool Socket::__Accept(Socket& connected_out, struct sockaddr& peer_addr, int addr_len)
{
	SOCKET_SIZE_T la = addr_len;
	SOCKET sock = accept(m_hSocket,&peer_addr,&la);
	if(INVALID_SOCKET != sock)
	{
		connected_out.Attach(sock);
		return la == addr_len;
	}
	else
	{
		return false;
	}
}

void Socket::EnableNonblockingIO(bool enable)
{
#if defined(PLATFORM_WIN)
	u_long flag = enable;
	ioctlsocket(m_hSocket, FIONBIO, &flag);
#else
	u_long flag = enable;
	ioctl(m_hSocket, FIONBIO, &flag);
#endif
}

void Socket::EnableDatagramBroadcast(bool enable)
{
#if !defined(PLATFORM_WIN)
	int option = enable;
    if(-1 == setsockopt(m_hSocket, SOL_SOCKET, SO_BROADCAST, (char *)&option, sizeof(option)))
        _LOG_WARNING("[NET]: failed to enable broadcast (err:"<<strerror(errno)<<')');
#endif
}

SocketTimed::_FD::_FD(SOCKET s)
{
	FD_ZERO(&_fd);
	FD_SET(s, &_fd);
}

SocketTimed::SocketTimed()
{
	SetSendTimeout(5000);
	SetRecvTimeout(30*1000);
}


void SocketTimed::SetRecvTimeout(DWORD msec)
{
	_timeout_recv.tv_sec = msec/1000;
	_timeout_recv.tv_usec = (msec%1000)*1000;
}

void SocketTimed::SetSendTimeout(DWORD msec)
{
	_timeout_send.tv_sec = msec/1000;
	_timeout_send.tv_usec = (msec%1000)*1000;
}

bool SocketTimed::__Create(const struct sockaddr &BindTo, int addr_len, int nSocketType, bool reuse_addr, int AF)
{
	_LastSelectRet = 1;
	if(Socket::__Create(BindTo, addr_len, nSocketType, reuse_addr, AF))
	{	Socket::EnableNonblockingIO(true);
		return true;
	}
	else
		return false;
}

bool SocketTimed::__ConnectTo(const struct sockaddr &target, int addr_len)
{
	timeval timeout = _timeout_send;
	_LastSelectRet = 1;
	int ret = connect(m_hSocket,&target,addr_len);
	
	return	ret == 0 ||
			(	ret < 0 &&
				IsLastOpPending() &&
				(_LastSelectRet = select(1 + (int)m_hSocket, NULL, _FD(m_hSocket), NULL, &timeout)) == 1
			);
}

bool SocketTimed::__Accept(Socket& connected_out, struct sockaddr& peer_addr, int addr_len)
{
	timeval timeout = _timeout_recv;
	return	(_LastSelectRet = select(1 + (int)m_hSocket, _FD(m_hSocket), NULL, NULL, &timeout)) == 1 &&
			Socket::__Accept(connected_out, peer_addr, addr_len);
}

bool SocketTimed::Recv(LPVOID pData, UINT len, UINT& len_out, bool Peek)
{
	timeval timeout = _timeout_recv;
	ASSERT(Peek == false);
	return	((_LastSelectRet = select(1 + (int)m_hSocket, _FD(m_hSocket), NULL, NULL, &timeout)) == 1) &&
			Socket::Recv(pData, len, len_out, false);
}

bool SocketTimed::__RecvFrom(LPVOID pData, UINT len, UINT& len_out, struct sockaddr &target, int addr_len, bool Peek )
{
	timeval timeout = _timeout_recv;

	ASSERT(Peek == false);
	return	(_LastSelectRet = select(1 + (int)m_hSocket, _FD(m_hSocket), NULL, NULL, &timeout)) == 1 &&
			Socket::__RecvFrom(pData, len, len_out, target, addr_len, false);
}

bool SocketTimed::Send(LPCVOID pData, UINT len, bool drop_if_busy)
{
	ASSERT(len);
//#ifdef 
//	static const int flag = 0;
//#else
//	static const int flag = MSG_NOSIGNAL;	// maybe for linux
//#endif
	int ret = 0;
	LPCSTR d = (LPCSTR)pData;
	while(len>0)
	{	
		ret = (int)send(m_hSocket,d,rt::min(32*1024U, len),0);
		if(ret>0)
		{	len -= ret;
			d += ret;
			continue;
		}

		ASSERT(ret == -1);
		timeval timeout = _timeout_send;
		
		if(drop_if_busy || !IsLastOpPending())return false;

		// wait to death
		while((_LastSelectRet = select(1 + (int)m_hSocket, NULL, _FD(m_hSocket), NULL, &timeout)) == 0);
		if(_LastSelectRet == 1)
			continue;
		else
			return false;
	}
	
	return true;
}

bool SocketTimed::__SendTo(LPCVOID pData, UINT len,const struct sockaddr &target, int addr_len, bool drop_if_busy)
{
	int ret = 0;
	timeval timeout;

	do
	{	timeout = _timeout_send;
		ret = (int)sendto(m_hSocket,(const char*)pData,len,0,&target,addr_len);
		if(ret == len)return true;
	}while(	!drop_if_busy &&
			ret < 0 &&
			IsLastOpPending() &&
			(_LastSelectRet = select(1 + (int)m_hSocket, NULL, _FD(m_hSocket), NULL, &timeout)) == 1
		  );

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////
// SocketEvent

SocketEvent::SocketEvent(DWORD signal_type)
#if !defined(PLATFORM_WIN)
	:fd_set_read(_Sockets)
	,fd_set_write(_Sockets)
	,fd_set_exception(_Sockets)
#endif
{
#if !defined(PLATFORM_WIN)
	_FD_Max = 0;
#endif
	if(signal_type&SEVT_ReadIsReady)fd_set_read.alloc();
	if(signal_type&SEVT_WriteIsReady)fd_set_write.alloc();
	if(signal_type&SEVT_Exception)fd_set_exception.alloc();
}

void SocketEvent::my_fd_set::assign_socket(const rt::BufferEx<SOCKET>& _Sockets)
{
#if defined(PLATFORM_WIN)
	_fd_set->fd_count = (u_int)_Sockets.GetSize();
	memcpy(_fd_set->fd_array,_Sockets.Begin(),_Sockets.GetSize()*sizeof(SOCKET));
#else
	for(UINT i=0;i<_Sockets.GetSize();i++)
		FD_SET(_Sockets[i], _fd_set);
#endif
}

int SocketEvent::WaitForEvents(UINT timeout)
{
	if(GetCount() == 0)return 0;

	if(fd_set_read.is_allocated()){ fd_set_read.assign_socket(_Sockets); fd_set_read.clear_event(); }
	if(fd_set_write.is_allocated()){ fd_set_write.assign_socket(_Sockets); fd_set_write.clear_event(); }
	if(fd_set_exception.is_allocated()){ fd_set_exception.assign_socket(_Sockets); fd_set_exception.clear_event(); }

	timeval		tm;
	timeval*	ptm = 0;
	if(timeout != INFINITE)
	{	ptm = &tm;
		tm.tv_sec = timeout/1000;
		tm.tv_usec = (timeout%1000)*1000000;
	}

#if defined(PLATFORM_WIN)
	int ret = select(0, fd_set_read, fd_set_write, fd_set_exception, ptm);
#else
	int ret = select(_FD_Max+1, fd_set_read, fd_set_write, fd_set_exception, ptm);
#endif
	
	return ret;
}

void SocketEvent::Add(SOCKET s)
{
	for(UINT i=0;i<_Sockets.GetSize();i++)
		if(s == _Sockets[i])return;

	_Sockets.push_back(s);
#if !defined(PLATFORM_WIN)
	_FD_Max = rt::max(_FD_Max, s);
#endif
}

void SocketEvent::Assign(SOCKET* p, UINT co)
{
	_Sockets.SetSize(co);
	memcpy(_Sockets, p, co*sizeof(SOCKET));

#if !defined(PLATFORM_WIN)
	if(fd_set_read._fd_set)rt::Zero(fd_set_read._fd_set); // memset(fd_set_read._fd_set, 0, sizeof(fd_set));
	if(fd_set_write._fd_set)rt::Zero(fd_set_write._fd_set); // memset(fd_set_write._fd_set, 0, sizeof(fd_set));
	if(fd_set_exception._fd_set)rt::Zero(fd_set_exception._fd_set); //memset(fd_set_exception._fd_set, 0, sizeof(fd_set));
	_UpdateFDMax();
#endif
}

#if !defined(PLATFORM_WIN)
void SocketEvent::_UpdateFDMax()
{
	_FD_Max = 0;
	for(UINT i=0;i<_Sockets.GetSize();i++)
		_FD_Max = rt::max(_FD_Max, _Sockets[i]);
}
#endif

void SocketEvent::Remove(SOCKET s)
{
	for(UINT i=0;i<_Sockets.GetSize();i++)
		if(s == _Sockets[i])_Sockets.erase(i);

#if !defined(PLATFORM_WIN)
	if(fd_set_read._fd_set)FD_CLR(s, fd_set_read._fd_set);
	if(fd_set_write._fd_set)FD_CLR(s, fd_set_write._fd_set);
	if(fd_set_exception._fd_set)FD_CLR(s, fd_set_exception._fd_set);
	_UpdateFDMax();
#endif
}

void SocketEvent::RemoveAll()
{
	_Sockets.SetSize(0);
}

UINT SocketEvent::GetCount()
{
	return (UINT)_Sockets.GetSize();
}

SOCKET SocketEvent::my_fd_set::get_next_event()
{
	if(_fd_set == nullptr)return INVALID_SOCKET;

#if defined(PLATFORM_WIN)
	_last_getevent++;
	if(_last_getevent < (int)_fd_set->fd_count)
		return _fd_set->fd_array[_last_getevent];
#else
	for(;;)
	{	_last_getevent++;
		if(_last_getevent >= _Sockets.GetSize())break;
		if(FD_ISSET(_Sockets[_last_getevent], _fd_set))return _Sockets[_last_getevent];
	}
#endif

	return INVALID_SOCKET;
}

namespace _details
{
#if defined(PLATFORM_IOS) || defined(PLATFORM_ANDROID)
static const UINT _NetworkInterfacesReconfigClaimDown = 2500; // msec
#else
static const UINT _NetworkInterfacesReconfigClaimDown = 1500; // msec
#endif

#if defined(PLATFORM_IOS)
CFStringRef _NotifySCNetworkChange = CFSTR(kNotifySCNetworkChange);
#endif
}


NetworkInterfaces::NetworkInterfaces()
{
#if defined(PLATFORM_WIN)
	struct _call
	{	static void func(PVOID CallerContext, PMIB_IPINTERFACE_ROW Row, MIB_NOTIFICATION_TYPE NotificationType)
		{
            ((NetworkInterfaces*)CallerContext)->_LastEventFired = os::Timestamp::Get();
		}
	};

	ASSERT(_CallbackHandle == INVALID_HANDLE_VALUE);
	if(NO_ERROR != ::NotifyIpInterfaceChange(AF_UNSPEC, _call::func, this, false, &_CallbackHandle))
		_LOG_WARNING("[NET]: NotifyIpInterfaceChange failed");
#elif defined(PLATFORM_IOS)
    struct _call
    {
        static void func(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo)
        {
            if(CFStringCompare(name, _details::_NotifySCNetworkChange, 0) == kCFCompareEqualTo)
                *((LONGLONG*)observer) = os::Timestamp::Get();
        }
    };

    CFNotificationCenterAddObserver(CFNotificationCenterGetDarwinNotifyCenter(), //center
                                        &_LastEventFired, // observer
                                        _call::func, // callback
                                        _details::_NotifySCNetworkChange, // event name
                                        NULL, // object
                                        CFNotificationSuspensionBehaviorDeliverImmediately
    );
#else
	_WaitingThread.Create(this, &NetworkInterfaces::_WaitingFunc);
#endif
}

NetworkInterfaces::~NetworkInterfaces()
{
#if defined(PLATFORM_WIN)
    if(_CallbackHandle != INVALID_HANDLE_VALUE)
        ::CancelMibChangeNotify2(_CallbackHandle);
#elif defined(PLATFORM_IOS)
    CFNotificationCenterRemoveObserver(CFNotificationCenterGetDarwinNotifyCenter(), &_LastEventFired, NULL, NULL);
#else
    auto t = _NetLinkSocket;
    _NetLinkSocket = -1;
    ::close(t);
    _WaitingThread.WantExit() = true;
    _WaitingThread.WaitForEnding();
#endif
}

#if !defined(PLATFORM_WIN) && !defined(PLATFORM_IOS)
void NetworkInterfaces::_WaitingFunc()
{
	while(!_WaitingThread.WantExit())
	{
#if defined(PLATFORM_MAC)
        _NetLinkSocket = socket(PF_SYSTEM, SOCK_RAW, SYSPROTO_EVENT);
        
        kev_request key;
        key.vendor_code = KEV_VENDOR_APPLE;
        key.kev_class = KEV_NETWORK_CLASS;
        key.kev_subclass = KEV_ANY_SUBCLASS;
        
        int code = ioctl(_NetLinkSocket, SIOCSKEVFILT, &key);
        kern_event_msg msg;
        while(_NetLinkSocket>=0)
        {
            code = recv(_NetLinkSocket, &msg, sizeof(msg), 0);
            if(code<sizeof(kern_event_msg))break;
            // check type of event
            if(msg.event_code == KEV_DL_IF_DETACHED || msg.event_code == KEV_DL_IF_ATTACHED ||
               msg.event_code == KEV_DL_LINK_OFF || msg.event_code == KEV_DL_LINK_ON ||
               msg.event_code == KEV_DL_LINK_ADDRESS_CHANGED
            )
            {  _LastEventFired = os::Timestamp::Get(); }
        }
#else
		struct sockaddr_nl addr;
		rt::Zero(addr);
		addr.nl_family = AF_NETLINK;
		addr.nl_pid = getpid();
		addr.nl_groups = /*RTMGRP_LINK|*/RTMGRP_IPV4_IFADDR|RTMGRP_IPV6_IFADDR;

		_NetLinkSocket = socket(AF_NETLINK,SOCK_RAW,NETLINK_ROUTE);
		if(_NetLinkSocket<-1 || bind(_NetLinkSocket,(struct sockaddr *)&addr,sizeof(addr))<0)
		{
			::close(_NetLinkSocket);
			os::Sleep(1000);
			continue;
		}

		while(_NetLinkSocket>=0)
		{
			int status;
			char buf[4096];
			struct iovec iov = { buf, sizeof buf };
			struct sockaddr_nl snl;
			struct msghdr msg = { (void*)&snl, sizeof snl, &iov, 1, NULL, 0, 0};
			struct nlmsghdr *h;

			status = recvmsg(_NetLinkSocket, &msg, 0);

			if(status < 0)
			{
				/* Socket non-blocking so bail out once we have read everything */
				if(errno == EWOULDBLOCK || errno == EAGAIN)
					continue;

				break;
			}

			if(status == 0)break;

			for(h = (struct nlmsghdr *)buf; NLMSG_OK(h, (unsigned int)status); h = NLMSG_NEXT (h, status))
			{
				auto type = h->nlmsg_type;
				if(type == NLMSG_DONE || type == NLMSG_ERROR)
					break;
					
				if(type == RTM_NEWADDR || type == RTM_DELADDR)
                {
                    _LastEventFired = os::Timestamp::Get();
					break;
				}
			}
		}
#endif // #if defined(PLATFORM_MAC)
		
		os::Sleep(1000);
	}
}
#endif // #if !defined(PLATFORM_WIN) && !defined(PLATFORM_IOS)

NetworkInterfaces::ConfigState NetworkInterfaces::GetState() const
{
    if(_LastEventFired)
    {
        if(os::Timestamp::Get() - _LastEventFired > _details::_NetworkInterfacesReconfigClaimDown)
        {
            _LastEventFired = 0;
            return Reconfigured;
        }
        else return Reconfiguring;
    }
    else return Unchanged;
}

bool NetworkInterfaces::_IsIPv6AddressTrivial(LPCBYTE ipv6)
{
	if(ipv6[0] == 0xfe && (ipv6[1]&0xc0) == 0x80)return true;  // link-local, maybe multicast? https://menandmice.com/blog/ipv6-reference-multicast

	return false;
}

bool NetworkInterfaces::_IsIPv4AddressTrivial(LPCBYTE ipv4)
{
	if(ipv4[0] == 169 && ipv4[1] == 254)return true;	// link-local

	return false;
}

bool NetworkInterfaces::Populate(rt::BufferEx<NetworkInterface>& list, bool only_up, bool skip_loopback)
{
	list.ShrinkSize(0);

#if defined(PLATFORM_WIN)
	rt::Buffer<BYTE> data_buf;
	data_buf.SetSize(40960);

	ULONG outBufLen = (ULONG)data_buf.GetSize();
	DWORD gaa = GAA_FLAG_SKIP_ANYCAST|GAA_FLAG_INCLUDE_PREFIX|GAA_FLAG_SKIP_DNS_SERVER;

	auto dwRetVal = GetAdaptersAddresses(AF_UNSPEC, gaa, NULL, (IP_ADAPTER_ADDRESSES *)data_buf.Begin(), &outBufLen);
	if(dwRetVal == ERROR_BUFFER_OVERFLOW)
	{
		data_buf.SetSize(outBufLen + 1024);
		outBufLen = (ULONG)data_buf.GetSize();
		dwRetVal = GetAdaptersAddresses(AF_UNSPEC, gaa, NULL, (IP_ADAPTER_ADDRESSES *)data_buf.Begin(), &outBufLen);
	}
	if(dwRetVal != NO_ERROR)return false;

	auto* nic = (IP_ADAPTER_ADDRESSES *)data_buf.Begin();
    for(; nic; nic = nic->Next)
	{
		if(only_up && nic->OperStatus != IfOperStatusUp)continue;
		if(skip_loopback && nic->IfType == IF_TYPE_SOFTWARE_LOOPBACK)continue;

		auto& itm = list.push_back();
		rt::Zero(itm);

		{	auto len = os::__UTF8(nic->FriendlyName).SubStrHead(sizeof(itm.Name)-1).CopyTo(itm.Name);
			if(len < sizeof(itm.Name)-1)
				itm.Name[len++] = ':';
			os::__UTF8(nic->Description).SubStrHead(sizeof(itm.Name)-1-len).CopyTo(&itm.Name[len]);
		}

		if(nic->ReceiveLinkSpeed != 0xffffffffffffffff && nic->ReceiveLinkSpeed != 0xffffffffffffffff)
			itm.LinkSpeed = (nic->ReceiveLinkSpeed + nic->TransmitLinkSpeed)/2;

		itm.MTU = nic->Mtu;

		switch(nic->IfType)
		{
		case IF_TYPE_SOFTWARE_LOOPBACK:	itm.Type = NITYPE_LOOPBACK; 	break;
		case IF_TYPE_IEEE80211:
		case IF_TYPE_IEEE8023AD_LAG:
		case IF_TYPE_IEEE802154:		
		case IF_TYPE_ETHERNET_CSMACD:	
		case IF_TYPE_IEEE80216_WMAN:	itm.Type = NITYPE_LAN; 		    break;
		case IF_TYPE_USB:				itm.Type = NITYPE_USB; 		    break;
		case IF_TYPE_WWANPP:			
		case IF_TYPE_WWANPP2:			itm.Type = NITYPE_CELLULAR; 	break;
		}

        if(!(nic->Flags&IP_ADAPTER_NO_MULTICAST))
            itm.Type |= NITYPE_MULTICAST;

		//if(nic->TunnelType != TUNNEL_TYPE_NONE)

		if(nic->OperStatus == IfOperStatusUp)itm.Type |= NITYPE_ONLINE;
		
		// copy first address per-AF
		auto* addr = nic->FirstUnicastAddress;
		while(addr && itm.v4Count < sizeofArray(NetworkInterface::v4) && itm.v6Count < sizeofArray(NetworkInterface::v6))
		{
			if(addr->Address.lpSockaddr->sa_family == AF_INET)
			{
				auto& ip = itm.v4[itm.v4Count++];
					
				ip.Local = *(DWORD*)(((InetAddr*)addr->Address.lpSockaddr)->GetBinaryAddress());
				ConvertLengthToIpv4Mask(addr->OnLinkPrefixLength, (PULONG)&ip.SubnetMask);
				ip.Boardcast = ip.Local|~ip.SubnetMask;
			}
			else if(addr->Address.lpSockaddr->sa_family == AF_INET6)
			{
				rt::CopyByteTo<16>(((InetAddrV6*)addr->Address.lpSockaddr)->GetBinaryAddress(), itm.v6[itm.v6Count++].Local);
			}

			addr = addr->Next;
		}
	}
#else
	struct ifaddrs * ifaps = nullptr;
	if(getifaddrs(&ifaps))return false;
	
	auto* ifap = ifaps;
	for(; ifap; ifap = ifap->ifa_next)
	{
		auto flag = ifap->ifa_flags;
        bool online = (flag&IFF_UP) &&
                      ifap->ifa_addr &&
                      (ifap->ifa_addr->sa_family == AF_INET || ifap->ifa_addr->sa_family == AF_INET6);
                      
        
		if(only_up && (flag&IFF_UP) == 0)continue;
		if(skip_loopback && (flag&IFF_LOOPBACK))continue;

		rt::String_Ref name(ifap->ifa_name);
		name = name.SubStrHead(sizeof(NetworkInterface::Name)-1);

		NetworkInterface* pitm = nullptr;
		for(UINT i=0; i<list.GetSize(); i++)
		{
			if(!pitm && memcmp(name.Begin(), list[i].Name, name.GetLength()+1) == 0)
            {	pitm = &list[i];
                break;
            }
		}
        
		if(pitm == nullptr)
		{
            UINT if_type;
			if(flag&IFF_LOOPBACK){ if_type = NITYPE_LOOPBACK; }
			else if(flag&IFF_POINTOPOINT){ if_type = NITYPE_ADHOC; }
			{
				// sadly to guess type based on interface name, which is not reliable
				if(name.StartsWith("bridge") || name.StartsWith("ap") || name.StartsWith("swlan")){ if_type = NITYPE_HOTSPOT; }
				else if(name.StartsWith("awdl") || name.StartsWith("p2p")){ if_type = NITYPE_ADHOC; }  // Apple Wireless Direct Link (AirDrop,AirPlay), can be bluetooth
				else if(name.StartsWith("llw") || name.FindString("wlan")>=0 || name.StartsWith("eth") || name.StartsWith("wlp") || name.StartsWith("en")|| name.StartsWith("em")){ if_type = NITYPE_LAN; }
				else if(name.StartsWith("XHC") || name.StartsWith("usb")){ if_type = NITYPE_USB; }
				else if(name.StartsWith("pdp_ip") || name.StartsWith("rmnet")){ if_type = NITYPE_CELLULAR; }
				else if(name.StartsWith("utun")){ if_type = NITYPE_VPN; }
				else if(name.StartsWith("gif") || name.StartsWith("stf") || name.StartsWith("sit") || name.StartsWith("ipsec")){ if_type = NITYPE_TUNNEL; }
			}
            
            if(skip_loopback && if_type == NITYPE_TUNNEL)continue;
            
            pitm = &list.push_back();
            auto& itm = *pitm;

            rt::Zero(itm);
            name.CopyTo(itm.Name);

            itm.Type = if_type;
            if(online)itm.Type |= NITYPE_ONLINE;
            if(flag&IFF_MULTICAST)itm.Type |= NITYPE_MULTICAST;
		}
        else
        {
            if(online)pitm->Type |= NITYPE_ONLINE;
        }

        if(online)
        {
            auto& itm = *pitm;
            if(ifap->ifa_addr->sa_family == AF_INET && itm.v4Count < sizeofArray(NetworkInterface::v4)) // ipv4
            {
				auto& ip = itm.v4[itm.v4Count++];
                ip.Local = *(DWORD*)(((InetAddr*)ifap->ifa_addr)->GetBinaryAddress());
                
                if(ifap->ifa_netmask)
                    ip.SubnetMask = *(DWORD*)(((InetAddr*)ifap->ifa_netmask)->GetBinaryAddress());
                    
                if(ifap->ifa_broadaddr)
                    ip.Boardcast = *(DWORD*)(((InetAddr*)ifap->ifa_broadaddr)->GetBinaryAddress());
                else if(ifap->ifa_netmask)
                    ip.Boardcast = ip.Local|~ip.SubnetMask;
            }
            
            if(ifap->ifa_addr->sa_family == AF_INET6 && itm.v6Count < sizeofArray(NetworkInterface::v6)) // ipv6
            {               
                rt::CopyByteTo<16>(((InetAddrV6*)ifap->ifa_addr)->GetBinaryAddress(), itm.v6[itm.v6Count++].Local);
            }
        }
	}
	
	freeifaddrs(ifaps);
	
#endif // #if defined(PLATFORM_WIN)
	return true;
}


} // namespace inet
