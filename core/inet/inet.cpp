#include "inet.h"
#include <string.h>
#include <ctype.h>


#ifdef PLATFORM_WIN
#include <Iphlpapi.h>
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


#if defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
#include <sys/sockio.h>
#include <net/if_dl.h>
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
UINT GetLocalAddressT(t_ADDR* pOut, UINT OutSize, bool no_loopback, t_ADDR* pOut_Broadcast = nullptr, DWORD* subnet_mask = nullptr, LPCSTR interface_prefix = nullptr)
{
	typedef _details::InetAddrT_Op<typename t_ADDR::ADDRESS_TYPE>	OP;
	ASSERT(OutSize);
    rt::String_Ref iinit = interface_prefix;

	UINT nextAddr = 0;
#if defined(PLATFORM_WIN)
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
				{	if(pOut_Broadcast)
					{	DWORD bcast = ipt.dwAddr|~ipt.dwMask;
						pOut_Broadcast[nextAddr].SetBinaryAddress(&bcast);
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
		ASSERT(pOut_Broadcast == nullptr);
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
	SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	VERIFY(sockfd != INVALID_SOCKET);

	char ioctl_buffer[4096];
	struct ifconf ifc;
	ifc.ifc_len = sizeof(ioctl_buffer);
	ifc.ifc_buf = ioctl_buffer;
	
	if(ioctl(sockfd, SIOCGIFCONF, &ifc) >= 0)
	{
		LPSTR ptr = ioctl_buffer;
		LPSTR end = &ioctl_buffer[ifc.ifc_len];
		while(ptr < end)
		{
			struct ifreq *ifr = (struct ifreq *)ptr;
	
#if defined(PLATFORM_ANDROID) || defined(PLATFORM_LINUX)
			int len = (int)sizeof(struct ifreq);
#else

			int len = sizeof(ifr->ifr_name) + rt::max((int)sizeof(struct sockaddr), (int)ifr->ifr_addr.sa_len);
#endif
			ptr += len;	// for next one in buffer
			
			if(ifr->ifr_addr.sa_family != OP::SIN_FAMILY)continue;
						
			struct ifreq ifrcopy = *ifr;
			ioctl(sockfd, SIOCGIFFLAGS, &ifrcopy);
			if ((ifrcopy.ifr_flags & IFF_UP) == 0)continue;  // ignore if interface not up
            
            if(!iinit.IsEmpty() && !rt::String_Ref(ifr->ifr_name).StartsWith(iinit))
                continue;
		
			t_ADDR& addr = *(t_ADDR*)&ifr->ifr_addr;
			if(	!OP::IsAddressNone(addr) &&
				!OP::IsAddressLoopback(addr) &&
				!OP::IsAddressAny(addr) &&
				!OP::IsAddressGhost(addr)
			)
			{	if(pOut_Broadcast)
				{
					ifrcopy = *ifr;
					if (ioctl(sockfd, SIOCGIFBRDADDR, &ifrcopy) == 0)
					{
						pOut_Broadcast[nextAddr] = *(t_ADDR*)&ifrcopy.ifr_broadaddr;
					}
					if(subnet_mask)
					{
						ifrcopy = *ifr;
						if (ioctl(sockfd, SIOCGIFNETMASK, &ifrcopy) == 0)
						{
							subnet_mask[nextAddr] = *(DWORD*)(ifrcopy.ifr_broadaddr.sa_data+2);
						}
					}
				}
			
				pOut[nextAddr] = addr;
				nextAddr++;
				if(nextAddr >= OutSize)break;
			}
		}		
	}
	Socket(sockfd).Close();
#endif

	if(!no_loopback && OutSize>(UINT)nextAddr)
	{	pOut[nextAddr++].SetAsLoopback();
	}

	return nextAddr;
}


extern UINT GetLocalAddresses(InetAddrT<sockaddr_in>* pOut, UINT Size_InOut, bool no_loopback, InetAddrT<sockaddr_in>* pOut_Broadcast, DWORD* subnet_mask, LPCSTR interface_prefix)
{
	UINT co = GetLocalAddressT<InetAddrT<sockaddr_in>>(pOut, Size_InOut, no_loopback, pOut_Broadcast, subnet_mask, interface_prefix);
	return co;
}

#ifdef PLATFORM_IPV6_SUPPORT
extern UINT GetLocalAddresses(InetAddrV6* pOut, UINT Size_InOut, bool no_loopback, InetAddrV6* pOut_Broadcast, LPCSTR interface_prefix)
{
	UINT co = GetLocalAddressT<InetAddrV6>(pOut, Size_InOut, no_loopback, pOut_Broadcast, nullptr, interface_prefix);
	return co;
}
#endif

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
		if(	(getsockopt(tmp,SOL_SOCKET,SO_TYPE,(char*)&val,&len) == 0) &&
			(val == SOCK_STREAM)	// cause deadlock with RecvFrom on SOCK_DGRAM socket
		)
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
	int val;
	SOCKET_SIZE_T len = sizeof(val);
	return getsockopt(m_hSocket,SOL_SOCKET,SO_TYPE,(char*)&val,&len) == 0;
}

bool Socket::IsConnected() const
{
	if(!IsValid())return false;
#ifdef PLATFORM_IPV6_SUPPORT
	InetAddrV6 peeraddr;
	SOCKET_SIZE_T size = sizeof(InetAddrV6);
	return getpeername(m_hSocket,peeraddr,&size)==0;
#else
	InetAddr peeraddr;
	SOCKET_SIZE_T size = sizeof(InetAddr);
	return getpeername(m_hSocket,peeraddr,&size)==0;
#endif
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
	if(l==SOCKET_ERROR || (!Peek && l<=0))return false;
	len_out = l;
	return true;
}

bool Socket::__RecvFrom(LPVOID pData, UINT len, UINT& len_out, struct sockaddr &target, int addr_len, bool Peek)
{
	SOCKET_SIZE_T la = addr_len;
	int l = (int)recvfrom(m_hSocket,(char*)pData,len,Peek?MSG_PEEK:0,&target,&la);
	if(l==SOCKET_ERROR)return false;
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


} // namespace inet


/////////////////////////////////////////////////////////////////////////////////////
// SocketEvent

inet::SocketEvent::SocketEvent(DWORD signal_type)
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

void inet::SocketEvent::my_fd_set::assign_socket(const rt::BufferEx<SOCKET>& _Sockets)
{
#if defined(PLATFORM_WIN)
	_fd_set->fd_count = (u_int)_Sockets.GetSize();
	memcpy(_fd_set->fd_array,_Sockets.Begin(),_Sockets.GetSize()*sizeof(SOCKET));
#else
	for(UINT i=0;i<_Sockets.GetSize();i++)
		FD_SET(_Sockets[i], _fd_set);
#endif
}

int inet::SocketEvent::WaitForEvents(UINT timeout)
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

void inet::SocketEvent::Add(SOCKET s)
{
	for(UINT i=0;i<_Sockets.GetSize();i++)
		if(s == _Sockets[i])return;

	_Sockets.push_back(s);
#if !defined(PLATFORM_WIN)
	_FD_Max = rt::max(_FD_Max, s);
#endif
}

void inet::SocketEvent::Assign(SOCKET* p, UINT co)
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
void inet::SocketEvent::_UpdateFDMax()
{
	_FD_Max = 0;
	for(UINT i=0;i<_Sockets.GetSize();i++)
		_FD_Max = rt::max(_FD_Max, _Sockets[i]);
}
#endif

void inet::SocketEvent::Remove(SOCKET s)
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

void inet::SocketEvent::RemoveAll()
{
	_Sockets.SetSize(0);
}

UINT inet::SocketEvent::GetCount()
{
	return (UINT)_Sockets.GetSize();
}

inet::SOCKET inet::SocketEvent::my_fd_set::get_next_event()
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


