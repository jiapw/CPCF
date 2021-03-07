#include "../../core/inet/inet.h"
#include "../../core/inet/recv_pump.h"
#include "test.h"

inet::Socket	a,b,c;

void rt::UnitTests::socket()
{
	struct _call
	{
		static DWORD _func(LPVOID p)
		{
			LPCSTR name = (LPCSTR)p;
			inet::SocketEvent	sevt;
			sevt.Add(a);
			sevt.Add(b);

			for(;;)
			{
				_LOG(name<<": on call");
				int ret = sevt.WaitForEvents();
				if(ret > 0)
				{
					for(int i=0;i<ret;i++)
					{
						inet::SOCKET s = sevt.GetNextSocketEvent_Read();
						char message[512];
						inet::InetAddr addr;
						UINT l = 0;
						((inet::Socket&)s).RecvFrom(message, sizeof(message), l, addr);
						if(l <= 0)return 0;
						_LOG(name<<": "<<rt::String_Ref(message, l)<<"\ton "<<addr.GetPort());
					}

					_LOG(name<<": Sleep 2 sec");
					os::Sleep(2000);
				}
				else return 0;
			}
		}
	};

    inet::InetAddr ips[4];
    UINT co = inet::GetLocalAddresses(ips, 4, true, nullptr, nullptr, "en");
    
    for(UINT i=0; i<co; i++)
    {
        _LOG(ips[i]);
    }
    
    return;

	inet::InetAddr addr;
	addr.SetAsLocal();
		
	addr.SetPort(10000);
	a.Create(addr, SOCK_DGRAM);

	addr.SetPort(10001);
	b.Create(addr, SOCK_DGRAM);

	addr.SetPort(10002);
	c.Create(addr, SOCK_DGRAM);

	os::Thread th1,th2,th3,th4;
	th1.Create(_call::_func, (LPVOID)"Thread 1");
	th2.Create(_call::_func, (LPVOID)"Thread 2");
	th3.Create(_call::_func, (LPVOID)"Thread 3");
	th4.Create(_call::_func, (LPVOID)"Thread 4");

	os::Sleep(1500);
	addr.SetPort(10000);
	c.SendTo("Message 1",9, addr);

	os::Sleep(500);
	c.SendTo("Message 2",9, addr);
	addr.SetPort(10001);
	b.SendTo("Message 3",9, addr);
	b.SendTo("Message 4",9, addr);
	b.SendTo("Message 5",9, addr);
	b.SendTo("Message 6",9, addr);
	b.SendTo("Message 7",9, addr);

	os::Sleep(8000);

	a.Close();
	b.Close();
	c.Close();

	th1.WaitForEnding();
	th2.WaitForEnding();
	th3.WaitForEnding();
	th4.WaitForEnding();
}


void test_socket_io(bool recv)
{
	if(recv)
	{
		inet::Socket	s;
		//s.Create(inet::InetAddr(INADDR_ANY, 2000), SOCK_DGRAM);
		if(!s.Create(inet::InetAddr("10.0.0.15", 2000), SOCK_DGRAM))return;
		//if(!s.Create(inet::InetAddr("192.168.1.111", 2000), SOCK_DGRAM))return;
		s.EnableNonblockingIO();

		inet::SocketEvent	sevt;
		sevt.Add(s);

		int q = 0;
		for(;;)
		{
			int ret = sevt.WaitForEvents();
			if(ret > 0)
			{
				for(int i=0;i<ret;i++)
				{
					inet::SOCKET s = sevt.GetNextSocketEvent_Read();
					char message[512];
					inet::InetAddr addr;
					UINT l = 0;
					((inet::Socket&)s).RecvFrom(message, sizeof(message), l, addr);
					if(l <= 0)return;
					_LOG(l<<"\tfrom "<<rt::tos::ip(addr)<<'\t'<<q++);
				}
			}
			else return;
		}
	}
	else
	{
		inet::Socket	s;
		s.Create(inet::InetAddr(INADDR_ANY, 10001), SOCK_DGRAM);
		s.EnableNonblockingIO();

		for(;;)
		{
			s.SendTo("123",3, inet::InetAddr("10.0.0.12",10000));
			os::Sleep(1000);
		}
	}
}

void rt::UnitTests::socket_io()
{
	os::Thread recv;
	recv.Create(
		[](){
			test_socket_io(true);
		}
	);

	test_socket_io(false);
}

void rt::UnitTests::socket_socket_event()
{
	inet::Socket recv[2];
	recv[0].Create(inet::InetAddr("11.1.1.22", 20001), SOCK_DGRAM);
	recv[1].Create(inet::InetAddr("11.1.1.22", 20002), SOCK_DGRAM);

	inet::SocketEvent se;
	se.Add(recv[0]);
	se.Add(recv[1]);

	os::Thread sender;
	sender.Create([](){

		inet::InetAddr target[2] = {
			{"11.1.1.22", 20001},
			{"11.1.1.22", 20002}
		};

		inet::Socket send;
		send.Create(inet::InetAddr("11.1.1.22", 20000), SOCK_DGRAM);

		for(UINT i=0;;i++)
		{
			char buf[1024];
			os::Sleep(1000);

			UINT len = (UINT)(rt::SS("Msg ") + i).CopyTo(buf);
			UINT idx = i%sizeofArray(target);
			send.SendTo(buf, len, target[idx]);
			_LOG("To "<<idx<<": "<<rt::String_Ref(buf, len));
		}
	});

	for(;;)
	{
		UINT count = se.WaitForEvents();
		if(count>0)
		{
			char buf[1024];
			UINT read = 0;
			inet::InetAddr from;

			for(UINT i=0; i<count; i++)
			{
				inet::Socket sock;
				sock.Attach(se.GetNextSocketEvent_Read());
				sock.RecvFrom(buf, sizeof(buf), read, from);
				sock.Detach();  // don't miss that

				_LOG(rt::String_Ref(buf, read));
			}		
		}
	}
}

volatile int packet_count = 0;
volatile int packet_error = 0;

struct SocketIOObj: public inet::IOObjectDatagram
{
	void OnRecv(LPVOID data, UINT size, LPCVOID from = nullptr, UINT from_size = 0)
	{
		if(data && size)
			os::AtomicIncrement(&packet_count);
		else
			os::AtomicIncrement(&packet_error);
	}
};

void rt::UnitTests::recv_pump()
{
	rt::Buffer<SocketIOObj> sockets;
	sockets.SetSize(100);

	int port_base = 21000;
	for(UINT i=0; i<sockets.GetSize(); i++)
	{
		inet::InetAddr addr("0.0.0.0", port_base + i);
		VERIFY(sockets[i].Create(addr, SOCK_DGRAM));
		sockets[i].SetBufferSize(2000);
	}

	inet::RecvPump<SocketIOObj>	core;
	VERIFY(core.Init());
	
	inet::InetAddr addr;
	addr.SetAsLocal();

	inet::Socket sender;
	sender.Create(addr, SOCK_DGRAM);

	for(UINT q=0; q<3; q++)
	for(UINT i=0; i<sockets.GetSize(); i++)
	{
		addr.SetPort(port_base + i);
		sockets[i].SendTo(&i, 4, addr);
	}

	os::Sleep(2000);
	_LOG("1 Packet Recv = "<<packet_count<<", "<<packet_error);

	for(UINT i=0; i<sockets.GetSize(); i++)
	{
		core.AddObject(&sockets[i]);
	}

	os::Sleep(2000);
	_LOG("2 Packet Recv = "<<packet_count<<", "<<packet_error);
	
	for(UINT i=0; i<sockets.GetSize(); i++)
	{
		addr.SetPort(port_base + i);
		sockets[i].SendTo(&i, 4, addr);
	}

	os::Sleep(2000);
	_LOG("3 Packet Recv = "<<packet_count<<", "<<packet_error);

	for(UINT i=0; i<sockets.GetSize(); i++)
	{
		addr.SetPort(port_base + i);
		sender.SendTo(&i, 4, addr);
	}

	os::Sleep(2000);
	_LOG("4 Packet Recv = "<<packet_count<<", "<<packet_error);
}

void rt::UnitTests::net_interfaces()
{
	auto scan = [](){
		rt::BufferEx<inet::NetworkInterface>	nic;
		inet::NetworkInterfaces::Populate(nic, false, false, false);

		for(auto it : nic)
		{
			_LOG("NIC: <"<<it.Name<<"> T"<<(it.Type&inet::NITYPE_MASK)<<' '<<(it.IsOnline()?"online":"offline"));
			if(!it.IsOnline())continue;

			if(it.HasIPv4())
				_LOG("\tIPv4:"<<rt::tos::ip(inet::InetAddr(it.IPv4_Local,0)).TrimAfterReverse(':')<<'/'
							  <<rt::tos::ip(inet::InetAddr(it.IPv4_Boardcast,0)).TrimAfterReverse(':'));

			if(it.HasIPv6())
			{
				inet::InetAddrV6 addr;
				addr.SetBinaryAddress(it.IPv6_Local);
				_LOG("\tIPv6:"<<rt::tos::ip(addr).TrimAfterReverse(':')<<
							  ((it.Type&inet::NITYPE_MULTICAST)?"/multicast":""));
			}
		}
	};

	scan();
	inet::NetworkInterfaces evt;

	for(;;)
	{
		os::Sleep(100);
		if(evt.GetState() == inet::NetworkInterfaces::Reconfiguring)
		{
            _LOGC("\n\nNetwork Interface reconfiguring ...");
            for(;;)
            {
                os::Sleep(100);
                if(evt.GetState() == inet::NetworkInterfaces::Reconfigured)
                {
                    _LOGC("Reconfigured:");
                    scan();
                    break;
                }
            }
		}
	}
}
