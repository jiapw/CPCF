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

#include "../os/predefines.h"
#include "../rt/string_type.h"
#include "../rt/buffer_type.h"
#include "../os/multi_thread.h"
#include "../os/file_dir.h"

#if defined(PLATFORM_ANDROID)
#pragma clang diagnostic ignored "-Wdeprecated-register"
#endif

#if defined(PLATFORM_WIN)
#include <Winsock2.h>
#include <ws2ipdef.h>
#include <WS2tcpip.h>
#else

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>

namespace inet
{
INLFUNC bool IN6_ADDR_EQUAL(LPCVOID x, LPCVOID y)
{	return ((const LONGLONG*)x)[0] == ((const LONGLONG*)y)[0]
	&& ((const LONGLONG*)x)[1] == ((const LONGLONG*)y)[1];
}
} // namespace inet

#endif

namespace inet
{

#ifndef PLATFORM_WIN
	typedef int SOCKET;
#define INVALID_SOCKET  (-1)
#define SOCKET_ERROR	(-1)
#else
	typedef ::SOCKET SOCKET;
#endif

#if defined(PLATFORM_IOS) || defined(PLATFORM_MAC) || defined(PLATFORM_ANDROID) || defined(PLATFORM_LINUX)
	typedef socklen_t SOCKET_SIZE_T;
#else
	typedef int SOCKET_SIZE_T;
#endif
	
extern void GetHostName(rt::String& name_out);

namespace _details
{
	template<typename t_ADDR>
	struct InetAddrT_Op;
	template<> struct InetAddrT_Op<sockaddr_in>
	{	static const int	SIN_ADDRESS_LEN = 4;
		static const int	SIN_FAMILY = AF_INET;
		INLFUNC static void		SetAny(sockaddr_in& x){	x.sin_addr.s_addr = INADDR_ANY; }
		INLFUNC static bool		IsEqual(const sockaddr_in& x, const sockaddr_in& y){ return x.sin_addr.s_addr == y.sin_addr.s_addr && x.sin_port == y.sin_port; }
		INLFUNC static LPBYTE	GetAddressPtr(const sockaddr_in& x){ return rt::_CastToNonconst((LPCBYTE)&x.sin_addr.s_addr); }
		INLFUNC static void		CopyAddress(LPVOID p, const sockaddr_in& x){ *((DWORD*)p) = x.sin_addr.s_addr; }
		INLFUNC static WORD*	GetPortPtr(const sockaddr_in& x){ return (WORD*)&x.sin_port; }
		INLFUNC static void		Init(sockaddr_in& x)
		{
#if defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
			x.sin_len = 16;
			rt::Zero(x.sin_zero);
#endif
			x.sin_family = AF_INET; x.sin_port = 0;
		}
		INLFUNC static void		SetBinaryAddress(sockaddr_in& x, LPCVOID data){ x.sin_addr.s_addr = *((DWORD*)data); }
		INLFUNC	static bool		IsAddressAny(const sockaddr_in& x){ return x.sin_addr.s_addr == INADDR_ANY; }
		INLFUNC	static bool		IsAddressLoopback(const sockaddr_in& x){ return x.sin_addr.s_addr == ntohl(INADDR_LOOPBACK); }
		INLFUNC static void		AssignLoopbackAddress(sockaddr_in& x){ x.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); }
		INLFUNC	static bool		IsAddressNone(const sockaddr_in& x){ return x.sin_addr.s_addr == INADDR_NONE; }
		INLFUNC static bool		IsAddressGhost(const sockaddr_in& x){ return false; }
	};
	template<> struct InetAddrT_Op<sockaddr_in6>
	{	static const int	SIN_ADDRESS_LEN = 16;
		static const int	SIN_FAMILY = AF_INET6;
		INLFUNC static void		SetAny(sockaddr_in6& x){ x.sin6_addr = in6addr_any; }
		INLFUNC static bool		IsEqual(const sockaddr_in6& x, const sockaddr_in6& y){ return x.sin6_port == y.sin6_port && memcmp(&x.sin6_addr, &y.sin6_addr, SIN_ADDRESS_LEN) == 0; }
		INLFUNC static LPBYTE	GetAddressPtr(const sockaddr_in6& x){ return rt::_CastToNonconst((LPCBYTE)&x.sin6_addr); }
		INLFUNC static void		CopyAddress(LPVOID p, const sockaddr_in6& x)
		{
			((__int64*)p)[0] = ((__int64*)&x.sin6_addr)[0];
			((__int64*)p)[1] = ((__int64*)&x.sin6_addr)[1];
		}
		INLFUNC static WORD*	GetPortPtr(const sockaddr_in6& x){ return (WORD*)&x.sin6_port; }
		INLFUNC static void		Init(sockaddr_in6& x){ rt::Zero(x); x.sin6_family = AF_INET6; }
		INLFUNC static void		SetBinaryAddress(sockaddr_in6& x, LPCVOID data){ memcpy(&x.sin6_addr, data, SIN_ADDRESS_LEN); }
		INLFUNC	static bool		IsAddressAny(const sockaddr_in6& x){ return IN6_ADDR_EQUAL(&x.sin6_addr, &in6addr_any); }
		INLFUNC	static bool		IsAddressLoopback(const sockaddr_in6& x){ return IN6_ADDR_EQUAL(&x.sin6_addr, &in6addr_loopback); }
		INLFUNC static void		AssignLoopbackAddress(sockaddr_in6& x){ memcpy(&x.sin6_addr, &in6addr_loopback, sizeof(in6addr_loopback)); }
		INLFUNC	static bool		IsAddressNone(const sockaddr_in6& x){ return ((__int64*)&x.sin6_addr)[0] == -1LL && ((__int64*)&x.sin6_addr)[1] == -1LL; }
		INLFUNC static bool		IsAddressGhost(const sockaddr_in6& x){ return IN6_IS_ADDR_V4MAPPED(&x.sin6_addr); }
	};
};
	
template<typename t_ADDR>
class InetAddrT : public t_ADDR
{	typedef _details::InetAddrT_Op<t_ADDR>	OP;	
public:
	typedef t_ADDR	ADDRESS_TYPE;
	// constructors
	INLFUNC InetAddrT(){ OP::Init(*this); OP::SetAny(*this); } // Default 0.0.0.0:0
	INLFUNC explicit InetAddrT(const t_ADDR& sin) { memcpy(this, &sin, sizeof(t_ADDR)); }
	INLFUNC explicit InetAddrT(SOCKET sock_peer){ InetAddrT(); SetAsPeer(sock_peer); }
	INLFUNC InetAddrT(WORD ushPort, LPCVOID pAddressBin) // dotted IP addr string or domain
	{	OP::Init(*this);
		SetBinaryAddress(pAddressBin);
		SetPort(ushPort);
	}
	INLFUNC InetAddrT(LPCSTR pHostname, WORD ushPort = 0) // dotted IP addr string or domain
	{	OP::Init(*this);
		SetAddress(pHostname, ushPort);
	}
	INLFUNC bool IsLoopback() const { return _details::InetAddrT_Op<t_ADDR>::IsAddressLoopback(*this); }
	INLFUNC bool SetAsLocal(bool no_loopback = false){ return GetLocalAddresses(this,1,no_loopback); }
	INLFUNC void SetAsLoopback(){ return _details::InetAddrT_Op<t_ADDR>::AssignLoopbackAddress(*this); }
	INLFUNC void SetAsAny(){ return _details::InetAddrT_Op<t_ADDR>::SetAny(*this); }
	INLFUNC bool SetAsPeer(SOCKET peer)
	{	SOCKET_SIZE_T len = sizeof(t_ADDR);
		return 0 == getpeername(peer,(sockaddr*)this,&len) && len == sizeof(t_ADDR);
	}
	bool SetAddress(LPCSTR pHostname, WORD port = 0)  // www.xxx.com:pp (port is optional)
	{
		rt::String_Ref ap[2];
		if(2 == rt::String_Ref(pHostname).Split(ap, 2, ':'))
		{	
			LPSTR phost = (LPSTR)alloca(ap[0].GetLength() + 1);
			memcpy(phost, ap[0].Begin(), ap[0].GetLength());
			phost[ap[0].GetLength()] = 0;
			pHostname = phost;
			if(!ap[1].IsEmpty())
			{
				WORD port;
				ap[1].ToNumber(port);
				SetPort(port);
			}
			else if(port)
			{	SetPort(port);
			}
		}
		else{ SetPort(port); }

		if(!ap[0].IsEmpty())
		{
			// http://uw714doc.sco.com/en/SDK_netapi/sockC.PortIPv4appIPv6.html
#if !defined(PLATFORM_MAX_COMPATIBILITY)
			if(inet_pton(OP::SIN_FAMILY, pHostname, OP::GetAddressPtr(*this)))return true;
#endif
			struct addrinfo aiHints;
			struct addrinfo *aiList = nullptr;
			rt::Zero(aiHints);
			//memset(&aiHints, 0, sizeof(aiHints));
			aiHints.ai_family = OP::SIN_FAMILY;
			bool ret = false;
			if(	0 == getaddrinfo(pHostname, nullptr, &aiHints, &aiList) && 
				aiList && 
				aiList->ai_addrlen == sizeof(t_ADDR) &&
				!OP::IsAddressNone((t_ADDR&)*aiList->ai_addr)
			)
			{	OP::CopyAddress(OP::GetAddressPtr(*this), (t_ADDR&)*aiList->ai_addr);
				ret = true;
			}
			if(aiList)freeaddrinfo(aiList);
			return ret;
		}
		else
		{	return GetLocalAddresses(this, 1, true)>0;
		}
	}
	INLFUNC void CopyAddress(LPVOID p) const { OP::CopyAddress(p, *this); }
	INLFUNC LPCBYTE GetBinaryAddress() const { return OP::GetAddressPtr(*this); }
	INLFUNC LPCWORD GetBinaryPort() const { return OP::GetPortPtr(*this); }
	INLFUNC void SetBinaryAddress(LPCVOID addr_bin){ OP::SetBinaryAddress(*this, addr_bin); }
	INLFUNC void SetPort(const WORD ushPort = 0){ *OP::GetPortPtr(*this) = htons(ushPort); }
	INLFUNC void SetBinaryPort(LPCWORD pPort){ *OP::GetPortPtr(*this) = *pPort; }
	INLFUNC LPCSTR GetDottedDecimalAddress(LPSTR text_out) const	// buf size = 16/46 for ipv4/ipv6
	{	return inet_ntop(OP::SIN_FAMILY, OP::GetAddressPtr(*this), text_out, 47);
	}
	INLFUNC WORD	GetPort() const	{ return ntohs(*OP::GetPortPtr(*this)); } // Get port and address (even though they're public)
	INLFUNC bool	IsValidDestination() const
	{	return !OP::IsAddressAny(*this) && !OP::IsAddressLoopback(*this) && !OP::IsAddressNone(*this) && !OP::IsAddressGhost(*this) && GetPort()!=0;
	}

	// operators added for efficiency
	INLFUNC const InetAddrT& operator = (const t_ADDR& sin){ memcpy(this, &sin, sizeof(t_ADDR)); return *this; }

	INLFUNC operator const sockaddr& ()const{ return *((sockaddr*) this); }
	INLFUNC operator const sockaddr* ()const{ return (const sockaddr*) this; }
	INLFUNC operator const t_ADDR* ()const{ return (const t_ADDR*) this; }

	INLFUNC operator sockaddr& (){ return *((sockaddr*) this); }
	INLFUNC operator sockaddr* (){ return (sockaddr*) this; }
	INLFUNC operator t_ADDR* (){ return (t_ADDR*) this; }

	INLFUNC bool operator == (const t_ADDR& x) const { return OP::IsEqual(*this, x); }
	INLFUNC bool operator != (const t_ADDR& x) const { return !OP::IsEqual(*this, x); }
};
template<class t_Ostream, typename t_ADDR>
t_Ostream& operator<<(t_Ostream& Ostream, const InetAddrT<t_ADDR>& x)
{	char buf[64];
	Ostream<<x.GetDottedDecimalAddress(buf)<<':'<<x.GetPort();
	return Ostream; 
}
    
struct InetAddr: public InetAddrT<sockaddr_in>
{
    INLFUNC InetAddr() = default;
    INLFUNC explicit InetAddr(const sockaddr_in& sin){ memcpy(this, &sin, sizeof(sockaddr_in)); }
    INLFUNC explicit InetAddr(SOCKET sock_peer):InetAddrT<sockaddr_in>(sock_peer){}
    INLFUNC InetAddr(DWORD ipv4, WORD ushPort):InetAddrT<sockaddr_in>(ushPort, &ipv4){}
	INLFUNC InetAddr(WORD ushPort, LPCVOID pAddressBin):InetAddrT<sockaddr_in>(ushPort, pAddressBin){}
    INLFUNC InetAddr(LPCSTR pHostname, WORD ushPort = 0):InetAddrT<sockaddr_in>(pHostname, ushPort){}
};
typedef InetAddrT<sockaddr_in6>    InetAddrV6;
    
extern UINT GetLocalAddresses(InetAddrT<sockaddr_in>* pOut, UINT out_size, bool no_loopback, InetAddrT<sockaddr_in>* pOut_Broadcast = nullptr, DWORD* subnet_mask = nullptr, LPCSTR interface_prefix = nullptr, rt::String* if_names = nullptr);
extern UINT GetLocalAddresses(InetAddrV6* pOut, UINT out_size, bool no_loopback, InetAddrV6* pOut_Broadcast = nullptr, LPCSTR interface_prefix = nullptr, rt::String* if_names = nullptr);

class Socket
{
	typedef const struct sockaddr CSA;
	typedef struct sockaddr SA;
protected:
	SOCKET m_hSocket;
	bool __Create(const struct sockaddr &BindTo, int addr_len, int nSocketType, bool reuse_addr, int AF);
	bool __GetPeerName(struct sockaddr &ConnectedTo, int addr_len) const;	// address of the peer
	bool __GetBindName(struct sockaddr &BindTo, int addr_len) const;		// address of this socket
	bool __ConnectTo(const struct sockaddr &target, int addr_len);
	bool __Accept(Socket& connected_out, struct sockaddr& peer_addr, int addr_len);
	bool __SendTo(LPCVOID pData, UINT len,const struct sockaddr &target, int addr_len);
	bool __RecvFrom(LPVOID pData, UINT len, UINT& len_out, struct sockaddr &target, int addr_len, bool Peek = false);
public:
	bool Create(const InetAddr &BindTo,int nSocketType = SOCK_STREAM, bool reuse_addr = false){ return __Create((CSA&)BindTo, sizeof(InetAddr), nSocketType, reuse_addr, PF_INET); }
	bool GetPeerName(InetAddr &ConnectedTo) const { return __GetPeerName((SA&)ConnectedTo, sizeof(InetAddr)); }
	bool GetBindName(InetAddr &BindTo) const { return __GetBindName((SA&)BindTo, sizeof(InetAddr)); }
	bool ConnectTo(const InetAddr &target){ return __ConnectTo((SA&)target, sizeof(InetAddr)); }
	bool Accept(Socket& connected_out, InetAddr& peer_addr){ return __Accept(connected_out, (SA&)peer_addr, sizeof(InetAddr)); }
	bool SendTo(LPCVOID pData, UINT len,const InetAddr &target){ return __SendTo(pData, len, (SA&)target, sizeof(InetAddr)); }
	bool RecvFrom(LPVOID pData, UINT len, UINT& len_out, InetAddr &target){ return __RecvFrom(pData, len, len_out, (SA&)target, sizeof(InetAddr));  }

	bool Create(const InetAddrV6 &BindTo,int nSocketType = SOCK_STREAM, bool reuse_addr = false){ return __Create((CSA&)BindTo, sizeof(InetAddrV6), nSocketType, reuse_addr, PF_INET6); }
	bool GetPeerName(InetAddrV6 &ConnectedTo) const { return __GetPeerName((SA&)ConnectedTo, sizeof(InetAddrV6)); }
	bool GetBindName(InetAddrV6 &BindTo) const { return __GetBindName((SA&)BindTo, sizeof(InetAddrV6)); }
	bool ConnectTo(const InetAddrV6 &target){ return __ConnectTo((SA&)target, sizeof(InetAddrV6)); }
	bool Accept(Socket& connected_out, InetAddrV6& peer_addr){ return __Accept(connected_out, (SA&)peer_addr, sizeof(InetAddrV6)); }
	bool SendTo(LPCVOID pData, UINT len,const InetAddrV6 &target){ return __SendTo(pData, len, (SA&)target, sizeof(InetAddrV6)); }
	bool RecvFrom(LPVOID pData, UINT len, UINT& len_out, InetAddrV6 &target){ return __RecvFrom(pData, len, len_out, (SA&)target, sizeof(InetAddrV6));  }

	void Close();
	SOCKET Detach();
	void Attach(SOCKET hSocket);
	Socket();
	Socket(SOCKET s);
	~Socket(){ Close(); }
	
	operator		SOCKET() const { return m_hSocket; }
	bool			IsEmpty() const { return m_hSocket == INVALID_SOCKET; }

public: //helpers
	static	int		GetLastError();
	static  bool	IsLastErrorUnrecoverableForDatagram();
	static	bool	IsLastOpPending();

public:
	bool			IsValid() const;
	bool			IsConnected() const;
	bool			SetBufferSize(int reserved_size, bool receiving_sending = true); // true for receiving buffer
	bool			Listen(UINT pending_size);

	bool			Send(LPCVOID pData, UINT len);
	bool			Recv(LPVOID pData, UINT len, UINT& len_out, bool Peek = false);
	void			SetTimeout(DWORD msec){}

    void			EnableNonblockingIO(bool enable = true);
	void			EnableDatagramBroadcast(bool enable = true);
	INLFUNC bool	IsLastOperationTimeout() const { return false; }
};

class SocketTimed:public Socket
{
	typedef const struct sockaddr CSA;
	typedef struct sockaddr SA;
	timeval	_timeout_send;
	timeval	_timeout_recv;
	struct _FD
	{	fd_set	_fd;
		_FD(SOCKET s);
		operator fd_set*(){ return &_fd; }
	};
	int		_LastSelectRet;

	bool	__Create(const struct sockaddr &BindTo, int addr_len, int nSocketType, bool reuse_addr, int AF);
	bool	__ConnectTo(const struct sockaddr &target, int addr_len);
	bool	__Accept(Socket& connected_out, struct sockaddr& peer_addr, int addr_len);
	bool	__SendTo(LPCVOID pData, UINT len,const struct sockaddr &target, int addr_len, bool drop_if_busy = false);
	bool	__RecvFrom(LPVOID pData, UINT len, UINT& len_out, struct sockaddr &target, int addr_len, bool Peek = false);

public:
	SocketTimed();
	void	Attach(SOCKET sock){ Socket::Attach(sock); EnableNonblockingIO(true); }
	SOCKET	Detach(){ EnableNonblockingIO(false); return Socket::Detach(); }
	bool	Send(LPCVOID pData, UINT len, bool drop_if_busy = false);
	bool	Recv(LPVOID pData, UINT len, UINT& len_out, bool Peek = false);

	bool	Create(const InetAddr &BindTo,int nSocketType = SOCK_STREAM, bool reuse_addr = false){ return __Create((CSA&)BindTo, sizeof(InetAddr), nSocketType, reuse_addr, PF_INET); }
	bool	ConnectTo(const InetAddr &target){ return __ConnectTo((SA&)target, sizeof(InetAddr)); }
	bool	Accept(Socket& connected_out, InetAddr& peer_addr){ return __Accept(connected_out, (SA&)peer_addr, sizeof(InetAddr)); }
	bool	SendTo(LPCVOID pData, UINT len,const InetAddr &target, bool drop_if_busy = false){ return __SendTo(pData, len, (SA&)target, sizeof(InetAddr), drop_if_busy); }
	bool	RecvFrom(LPVOID pData, UINT len, UINT& len_out, InetAddr &target){ return __RecvFrom(pData, len, len_out, (SA&)target, sizeof(InetAddr));  }

	bool	Create(const InetAddrV6 &BindTo,int nSocketType = SOCK_STREAM, bool reuse_addr = false){ return __Create((CSA&)BindTo, sizeof(InetAddrV6), nSocketType, reuse_addr, PF_INET6); }
	bool	ConnectTo(const InetAddrV6 &target){ return __ConnectTo((SA&)target, sizeof(InetAddr)); }
	bool	Accept(Socket& connected_out, InetAddrV6& peer_addr){ return __Accept(connected_out, (SA&)peer_addr, sizeof(InetAddrV6)); }
	bool	SendTo(LPCVOID pData, UINT len,const InetAddrV6 &target, bool drop_if_busy = false){ return __SendTo(pData, len, (SA&)target, sizeof(InetAddrV6), drop_if_busy); }
	bool	RecvFrom(LPVOID pData, UINT len, UINT& len_out, InetAddrV6 &target){ return __RecvFrom(pData, len, len_out, (SA&)target, sizeof(InetAddrV6));  }

	void	SetTimeout(DWORD msec){ SetRecvTimeout(msec); SetSendTimeout(msec); }
	bool	IsLastOperationTimeout() const { return _LastSelectRet == 0; }
	void	SetRecvTimeout(DWORD send_msec);
	void	SetSendTimeout(DWORD send_msec);
};


enum NetworkInterfaceType
{
	NITYPE_UNKNOWN = 0,
	NITYPE_LOOPBACK,
	NITYPE_ADHOC,		// p2p direct link
	NITYPE_HOTSPOT,		// personal hotspot
	NITYPE_LAN,			// wifi or ethernet
	NITYPE_CELLULAR,
	NITYPE_USB,
	NITYPE_VPN,
	NITYPE_TUNNEL,		// virtual interface
	NITYPE_MASK = 0xff,

	NITYPE_IPV4			= 0x0100,
	NITYPE_IPV6			= 0x0200,
	NITYPE_ONLINE		= 0x1000,
	NITYPE_MULTICAST	= 0x2000,
	NITYPE_BROADCAST	= 0x4000,
    NITYPE_ADDRESS_DUPLICATED = 0x8000,
};

struct NetworkInterface
{
#if defined(PLATFORM_WIN)
	char		Name[256];
	ULONGLONG	LinkSpeed;
	UINT		MTU;
	DWORD		IanaType;
#else
	char		Name[32];
#endif
	DWORD		Type;				// NetworkInterfaceType
	DWORD		IPv4_Local;
	DWORD		IPv4_Boardcast;		// or p2p destination NICTYPE_ADHOC
	DWORD		IPv4_SubnetMask;
	BYTE		IPv6_Local[16];

	bool		IsOnline() const { return NITYPE_ONLINE&Type; }
	bool		HasIPv4() const { return NITYPE_IPV4&Type; }
	bool		HasIPv6() const { return NITYPE_IPV6&Type; }
	bool		IsDualStack() const { return (NITYPE_IPV4|NITYPE_IPV6) == (Type&(NITYPE_IPV4|NITYPE_IPV6)); }
};

class NetworkInterfaces
{
public:
    enum ConfigState
    {
        Unchanged = 0,
        Reconfiguring,
        Reconfigured
    };
protected:
#if defined(PLATFORM_WIN)
	HANDLE		_CallbackHandle = INVALID_HANDLE_VALUE;
#elif defined(PLATFORM_IOS)
#else // for linux and mac
	int			_NetLinkSocket = -1;
	os::Thread	_WaitingThread;
	void		_WaitingFunc();
#endif

    mutable LONGLONG    _LastEventFired = 0;
    
	static bool	        _IsIPv6AddressTrivial(LPCBYTE ipv6);
	static bool	        _IsIPv4AddressTrivial(LPCBYTE ipv4);
public:
	NetworkInterfaces();
	~NetworkInterfaces();

    ConfigState	GetState() const;
	static bool	Populate(rt::BufferEx<NetworkInterface>& list, bool only_up = true, bool skip_misc = true, bool dedup = true);
};

enum _tagSocketEventType
{
	SEVT_ReadIsReady	= 1,
	SEVT_WriteIsReady	= 2,
	SEVT_Exception		= 4
};

class SocketEvent
{
	struct	my_fd_set
	{	
	#if !defined(PLATFORM_WIN)
		rt::BufferEx<SOCKET>& _Sockets;
		INLFUNC my_fd_set(rt::BufferEx<SOCKET>& x):_Sockets(x){ _fd_set = nullptr; }
	#else
		INLFUNC my_fd_set(){ _fd_set = nullptr; }
	#endif
		int		_last_getevent;
		fd_set*	_fd_set;
		
		INLFUNC ~my_fd_set(){ _SafeFree32AL(_fd_set); }

		SOCKET			get_next_event();
		INLFUNC void	clear_event(){ _last_getevent = -1; }
		INLFUNC void	alloc(){ ASSERT(_fd_set == nullptr); _fd_set = _Malloc32AL(fd_set,1); rt::Zero(*_fd_set); /* memset(_fd_set, 0, sizeof(fd_set)); */ }
		INLFUNC bool	is_allocated(){ return _fd_set != nullptr; }
		void			assign_socket(const rt::BufferEx<SOCKET>& sock);
		INLFUNC			operator fd_set* (){ return _fd_set; }
	};

	my_fd_set			fd_set_read;
	my_fd_set			fd_set_write;
	my_fd_set			fd_set_exception;

	rt::BufferEx<SOCKET>	_Sockets;
#if !defined(PLATFORM_WIN)
	SOCKET					_FD_Max;
	void					_UpdateFDMax();
#endif
public:
	SocketEvent(DWORD signal_type = SEVT_ReadIsReady);
	INT					WaitForEvents(UINT timeout = INFINITE);		// num of sockets ready: 0 for timeout and SOCKET_ERROR for error
	void				Add(SOCKET s);
	void				Remove(SOCKET s);
	void				RemoveAll();
	UINT				GetCount();
	void				Assign(SOCKET* p, UINT co);
	INLFUNC SOCKET		GetAt(UINT i){ return _Sockets[i]; }
	INLFUNC SOCKET		GetNextSocketEvent_Read(){ return fd_set_read.get_next_event(); }
	INLFUNC SOCKET		GetNextSocketEvent_Write(){ return fd_set_write.get_next_event(); }
	INLFUNC SOCKET		GetNextSocketEvent_Exception(){ return fd_set_exception.get_next_event(); }
};

enum _tagHttpStatus
{	
	HTTP_OK						= 200,
	HTTP_CREATED				= 201,
	HTTP_ACCEPTED				= 202,
	HTTP_NON_AUTH				= 203,
	HTTP_NO_CONTENT				= 204,
	HTTP_RESET_CONTENT			= 205,
	HTTP_PARTIAL_CONTENT		= 206,

	HTTP_MULTIPLE				= 300,
	HTTP_MOVED					= 301,
	HTTP_FOUND					= 302,
	HTTP_SEE_OTHER				= 303,
	HTTP_NOT_MODIFIED			= 304,
	HTTP_USE_PROXY				= 305,
	HTTP_SWITCH_PROXY			= 306,
	HTTP_REDIRECT				= 307,

	HTTP_BAD_REQUEST			= 400,
	HTTP_UNAUTHORIZED			= 401,
	HTTP_PAYMENT_REQUIRED		= 402,
	HTTP_FORBIDDEN				= 403,
	HTTP_NOT_FOUND				= 404,
	HTTP_NOT_ALLOWED			= 405,
	HTTP_NOT_ACCEPTABLE			= 406,
	HTTP_PROXY_AUTH				= 407,
	HTTP_TIMEOUT				= 408,
	HTTP_CONFLICT				= 409,

	HTTP_INTERNAL_ERROR			= 500,
	HTTP_NOT_IMPLEMENTED		= 501,
	HTTP_BAD_GATEWAY			= 502,
	HTTP_UNAVAILABLE			= 503,
	HTTP_GATEWAY_TIMEOUT		= 504,
	HTTP_VERSION				= 505,
	HTTP_NEGOTIATES				= 506,
	HTTP_INSUFFICIENT_STORAGE	= 507,

	HTTP_BANDWIDTH_EXCEEDED		= 509,

	// Internal Use
	HTTP_DELAYED_HANDLING		= 600,
};

} // namespace inet

namespace rt
{
namespace tos
{

struct ip:public ::rt::tos::S_<100>
{
	ip(const ::inet::InetAddr& x)
	{
		int len = (int)strlen(x.GetDottedDecimalAddress(_p));
		_p[len++] = ':';
		_len += len + rt::_details::string_ops::itoa(x.GetPort(), _p + len);
	};
    ip(const ::inet::InetAddrV6& x)
    {
        _p[0] = '[';
        int len = (int)strlen(x.GetDottedDecimalAddress(_p + 1)) + 1;
        _p[len++] = ']';
        _p[len++] = ':';
        _len += len + rt::_details::string_ops::itoa(x.GetPort(), _p + len);
    };
};

}
}

