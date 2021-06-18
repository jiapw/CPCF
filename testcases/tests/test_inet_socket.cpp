#include "../../core/inet/inet.h"
#include "../../core/inet/datagram_pump.h"
#include "test.h"

using namespace inet;

Socket	a, b, c;

void rt::UnitTests::socket()
{
	struct _call
	{
		static DWORD _func(LPVOID p)
		{
			LPCSTR name = (LPCSTR)p;
			SocketEvent	sevt;
			sevt.Add(a);
			sevt.Add(b);

			for (;;)
			{
				_LOG(name << ": on call");
				int ret = sevt.WaitForEvents();
				if (ret > 0)
				{
					for (int i = 0; i < ret; i++)
					{
						SOCKET s = sevt.GetNextSocketEvent_Read();
						char message[512];
						InetAddr addr;
						UINT l = 0;
						((Socket&)s).RecvFrom(message, sizeof(message), l, addr);
						if (l <= 0)return 0;
						_LOG(name << ": " << rt::String_Ref(message, l) << "\ton " << addr.GetPort());
					}

					_LOG(name << ": Sleep 2 sec");
					os::Sleep(2000);
				}
				else return 0;
			}
		}
	};

	InetAddr ips[4];
	// not support windows 
	//UINT co = GetLocalAddresses(ips, 4, true, nullptr, nullptr, "en");
	UINT co = GetLocalAddresses(ips, 4, true);
	for (UINT i = 0; i < co; i++)
	{
		_LOG(ips[i]);
	}

	//return;

	InetAddr addr;
	addr.SetAsLocal();

	addr.SetPort(10000);
	a.Create(addr, SOCK_DGRAM);

	addr.SetPort(10001);
	b.Create(addr, SOCK_DGRAM);

	addr.SetPort(10002);
	c.Create(addr, SOCK_DGRAM);

	os::Thread th1, th2, th3, th4;
	th1.Create(_call::_func, (LPVOID)"Thread 1");
	th2.Create(_call::_func, (LPVOID)"Thread 2");
	th3.Create(_call::_func, (LPVOID)"Thread 3");
	th4.Create(_call::_func, (LPVOID)"Thread 4");

	os::Sleep(1500);
	addr.SetPort(10000);
	c.SendTo("Message 1", 9, addr);

	os::Sleep(500);
	c.SendTo("Message 2", 9, addr);
	addr.SetPort(10001);
	b.SendTo("Message 3", 9, addr);
	b.SendTo("Message 4", 9, addr);
	b.SendTo("Message 5", 9, addr);
	b.SendTo("Message 6", 9, addr);
	b.SendTo("Message 7", 9, addr);

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
	InetAddr Addr;
	Addr.SetAsLocal();
	Addr.SetPort(10000);
	if (recv)
	{
		Socket	s;
		//s.Create(InetAddr(INADDR_ANY, 2000), SOCK_DGRAM);
		//if(!s.Create(InetAddr("10.0.0.15", 2000), SOCK_DGRAM))return;
		//if(!s.Create(InetAddr("192.168.1.111", 2000), SOCK_DGRAM))return;

		if (!s.Create(Addr, SOCK_DGRAM))return;
		s.EnableNonblockingIO();

		SocketEvent	sevt;
		sevt.Add(s);

		int q = 0;
		for (;;)
		{
			int ret = sevt.WaitForEvents();
			if (ret > 0)
			{
				for (int i = 0; i < ret; i++)
				{
					SOCKET s = sevt.GetNextSocketEvent_Read();
					char message[512];
					InetAddr addr;
					UINT l = 0;
					((Socket&)s).RecvFrom(message, sizeof(message), l, addr);
					if (l <= 0)return;
					_LOG(l << "\tfrom " << rt::tos::ip(addr) << '\t' << q++);
				}
			}
			else return;
		}
	}
	else
	{
		Socket	s;
		s.Create(InetAddr(INADDR_ANY, 10001), SOCK_DGRAM);
		s.EnableNonblockingIO();

		for (;;)
		{
			//s.SendTo("123",3, InetAddr("10.0.0.12",10000));
			s.SendTo("123", 3, Addr);
			os::Sleep(1000);
		}
	}
}

void rt::UnitTests::socket_io()
{
	os::Thread recv;
	recv.Create(
		[]() {
			test_socket_io(true);
		}
	);

	test_socket_io(false);
}

void rt::UnitTests::socket_socket_event()
{
	InetAddr addr1;
	addr1.SetAsLocal();
	addr1.SetPort(20001);
	InetAddr addr2;
	addr2.SetAsLocal();
	addr2.SetPort(20002);

	Socket recv[2];
	//recv[0].Create(InetAddr("11.1.1.22", 20001), SOCK_DGRAM);
	//recv[1].Create(InetAddr("11.1.1.22", 20002), SOCK_DGRAM);
	recv[0].Create(addr1, SOCK_DGRAM);
	recv[1].Create(addr2, SOCK_DGRAM);

	SocketEvent se;
	se.Add(recv[0]);
	se.Add(recv[1]);

	os::Thread sender;
	sender.Create([&addr1, &addr2]() {

		/*	InetAddr target[2] = {
				{"11.1.1.22", 20001},
				{"11.1.1.22", 20002}
			};*/
		InetAddr target[2];
		target[0] = addr1;
		target[1] = addr2;

		Socket send;
		//send.Create(InetAddr("11.1.1.22", 20000), SOCK_DGRAM);
		InetAddr addr3;
		addr3.SetAsLocal();
		addr3.SetPort(20000);
		send.Create(addr3, SOCK_DGRAM);

		for (UINT i = 0;; i++)
		{
			char buf[1024];
			os::Sleep(1000);

			UINT len = (UINT)(rt::SS("Msg ") + i).CopyTo(buf);
			UINT idx = i % sizeofArray(target);
			send.SendTo(buf, len, target[idx]);
			_LOG("To " << idx << ": " << rt::String_Ref(buf, len));
		}
		});

	for (;;)
	{
		UINT count = se.WaitForEvents();
		if (count > 0)
		{
			char buf[1024];
			UINT read = 0;
			InetAddr from;

			for (UINT i = 0; i < count; i++)
			{
				Socket sock;
				sock.Attach(se.GetNextSocketEvent_Read());
				sock.RecvFrom(buf, sizeof(buf), read, from);
				sock.Detach();  // don't miss that
			/*	equals
				sock = se.GetNextSocketEvent_Read();
				((Socket&)sock).RecvFrom(buf, sizeof(buf), read, from);*/
				_LOG(rt::String_Ref(buf, read));
			}
		}
	};

}

void rt::UnitTests::sockettimed()
{
	inet::SocketTimed s;
	inet::InetAddr addr;
	addr.SetAsLocal();
	s.Create(addr);
	if (s.ConnectTo(inet::InetAddr("i-funbox.com", 80)))
	{
		_LOG("connected");
	}
	else
	{
		_LOG("failed");
	}

}