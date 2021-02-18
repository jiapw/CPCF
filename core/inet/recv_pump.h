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

#include "inet.h"

namespace inet
{

class AsyncIOCoreBase
{
public:
#if defined(PLATFORM_WIN)
	typedef	HANDLE	IOCORE;
	static const SIZE_T IOCORE_INVALID = (SIZE_T)INVALID_HANDLE_VALUE;
#elif defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDRIOD)
	typedef	int		IOCORE;
	static const int 	IOCORE_INVALID = -1;
#else
	#error AsyncIOCore Unsupported Platform
#endif

protected:
	IOCORE					_Core = (IOCORE)IOCORE_INVALID;
	rt::Buffer<os::Thread>	_IOWorkers;

	struct Event
	{
#if defined(PLATFORM_WIN)
		DWORD	bytes_transferred;
#endif		
		LPVOID	cookie;
	};

protected:
	bool _Init(os::FUNC_THREAD_ROUTE io_pump, UINT concurrency = 0, UINT stack_size = 0);
	bool _AddObject(SOCKET obj, LPVOID cookie);
	void _RemoveObject(SOCKET obj);
	bool _PickUpEvent(Event& e);

public:
	bool IsRunning() const { return _Core != (IOCORE)IOCORE_INVALID; }
	void Term();
	~AsyncIOCoreBase(){ Term(); }
};

class IOObject: public inet::Socket
{
	template<typename IOObject>
	friend class RecvPump;

	bool RecvFrom(LPVOID pData, UINT len, UINT& len_out, InetAddr &target);
	bool RecvFrom(LPVOID pData, UINT len, UINT& len_out, InetAddrV6 &target);
	bool Recv(LPVOID pData, UINT len, UINT& len_out, bool Peek);

protected:
	rt::BufferEx<BYTE>	_RecvBuf;

#if defined(PLATFORM_WIN)
	WSAOVERLAPPED		_OverlappedRecv;
#elif defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_ANDRIOD)
#else
	#error AsyncIOCore Unsupported Platform
#endif
public:
	void	SetBufferSize(UINT sz = 1500){ VERIFY(_RecvBuf.ChangeSize(sz, false)); }
	SOCKET	GetHandle() const { return m_hSocket; }
	LPBYTE	GetBuffer(){ return _RecvBuf; }
	UINT	GetBufferSize() const { return _RecvBuf.GetSize(); }
};

template<typename t_IOObject> // IOObjectDatagram or IOObjectStream
class RecvPump;

class IOObjectDatagram: public IOObject // UDP
{
	template<typename t_IOObject>
	friend class RecvPump;

protected:
	BYTE	_RecvFrom[sizeof(inet::InetAddrV6)];
	int		_RecvFromSize;
#if defined(PLATFORM_WIN)
	DWORD	_Flag;
	bool	PumpNext()
			{	ASSERT(_RecvBuf.GetSize());
				WSABUF b = { (UINT)_RecvBuf.GetSize(), (LPSTR)_RecvBuf.Begin() };
				_RecvFromSize = sizeof(_RecvFrom);
				DWORD recv = 0;
				if(	::WSARecvFrom(m_hSocket, &b, 1, &recv, &_Flag, (sockaddr*)_RecvFrom, &_RecvFromSize, &_OverlappedRecv, nullptr) == 0 ||
					::WSAGetLastError() == WSA_IO_PENDING
				)return true;
				_OverlappedRecv.hEvent = INVALID_HANDLE_VALUE;
				return false;
			}
#endif
public:
	LPCBYTE		GetRecvFromPtr() const { return _RecvFrom; }
	UINT		GetRecvFromSize() const { return _RecvFromSize; }
	static void OnRecv(LPVOID data, UINT size, LPCVOID from = nullptr, UINT from_size = 0){ ASSERT(0); } // should be overrided
};

struct IOObjectStream: public IOObject // TCP
{
	template<typename t_IOObject>
	friend class RecvPump;
	static void OnRecv(LPVOID data, UINT size){ ASSERT(0); } // should be overrided
protected:
#if defined(PLATFORM_WIN)
	DWORD	_Flag;
	bool	PumpNext()
			{	ASSERT(_RecvBuf.GetSize());
				WSABUF b = { (UINT)_RecvBuf.GetSize(), (LPSTR)_RecvBuf.Begin() };
				DWORD recv = 0;
				if(::WSARecv(m_hSocket, &b, 1, &recv, &_Flag, &_OverlappedRecv, nullptr) == 0 || ::WSAGetLastError() == WSA_IO_PENDING)return true;
				_OverlappedRecv.hEvent = INVALID_HANDLE_VALUE;
				return false;
			}
#endif
};

namespace _details
{
#if defined(PLATFORM_WIN)
template<typename T>
void OnRecv(IOObjectDatagram* p, T* obj, UINT size)
{	obj->OnRecv(obj->GetBuffer(), size, obj->GetRecvFromPtr(), obj->GetRecvFromSize());
}
template<typename T>
void OnRecv(IOObjectStream* p, T* obj, UINT size)
{	obj->OnRecv(obj->GetBuffer(), size);
}
#else
template<typename T>
bool OnRecv(IOObjectDatagram* p, T* obj)
{	socklen_t addr_len = sizeof(inet::InetAddrV6);
	int len = ::recvfrom(p->GetHandle(), obj->GetBuffer(), obj->GetBufferSize(), 0, (sockaddr*)obj->GetRecvFromPtr(), &addr_len);
	if(len>0){ obj->OnRecv(obj->GetBuffer(), len, obj->GetRecvFromPtr(), addr_len); return true; }
	return len >= 0;
}
template<typename T>
bool OnRecv(IOObjectStream* p, T* obj)
{	int len = ::recv(p->GetHandle(), obj->GetBuffer(), obj->GetBufferSize(), 0);
	if(len>0){ obj->OnRecv(obj->GetBuffer(), len); return true; }
	return len >= 0;
}
#endif
} // namespace _details

template<typename t_IOObject> // IOObjectDatagram or IOObjectStream
class RecvPump: public AsyncIOCoreBase
{
	void _IOPump()
	{
		Event evt;
		while(IsRunning() && !_IOWorkers[0].WantExit())
		{
			if(AsyncIOCoreBase::_PickUpEvent(evt))
			{
#if defined(PLATFORM_WIN)
				_details::OnRecv((t_IOObject*)evt.cookie, (t_IOObject*)evt.cookie, evt.bytes_transferred);
				if(!((t_IOObject*)evt.cookie)->PumpNext())
					((t_IOObject*)evt.cookie)->OnRecv(nullptr, 0); // indicate error
#else
				if(!_details::OnRecv((t_IOObject*)evt.cookie, (t_IOObject*)evt.cookie))
					((t_IOObject*)evt.cookie)->OnRecv(nullptr, 0); // indicate error
#endif				
			}
		}
	}

public:
	bool Init(UINT concurrency = 0, UINT stack_size = 0)
	{
		struct _call
		{	static DWORD _func(LPVOID p)
			{	((RecvPump*)p)->_IOPump();
				return 0;
			}
		};
		return _Init(_call::_func, concurrency, stack_size);
	}

	bool AddObject(t_IOObject* obj) // // lifecycle is **not** maintained by RecvPump
	{	if(!_AddObject(obj->GetHandle(), obj))return false;
#if defined(PLATFORM_WIN)
		if(!obj->PumpNext()){ obj->OnRecv(nullptr, 0); return false; }
#endif
		return true;
	}

	void RemoveObject(t_IOObject* obj)
	{	
#if defined(PLATFORM_WIN)
		obj->_OverlappedRecv.hEvent = INVALID_HANDLE_VALUE;
#endif
		_RemoveObject(obj->GetHandle());
	}
};


} // namespace inet