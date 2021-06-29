#include "../../core/inet/inet.h"
#include "../../core/inet/datagram_pump.h"
#include "test.h"

//#include "../../../3rdparty/CRCpp/inc/CRC.h"
#include <iostream> // Includes ::std::cout
#include <cstdint>  // Includes ::std::uint32_t

#include <random>
using namespace inet;

volatile int packet_count = 0;
volatile int packet_error = 0;

struct SocketIOObj : public DatagramSocket
{
	void OnRecv(Datagram* g)
	{
		if (g)
			os::AtomicIncrement(&packet_count);
		else
			os::AtomicIncrement(&packet_error);
	}
};

struct SocketIOObj_Routing : public DatagramSocket
{
	rt::String Name;

	void OnRecv(Datagram* g)
	{
		if (g)
			_LOG("[" << Name << "] Recv: \"" << rt::DS(g->RecvBuf, g->RecvSize) << "\" from: " << rt::tos::ip(g->PeerAddressV4).TrimBefore(':'));
	}

	SocketIOObj_Routing(const rt::String_Ref& name) :Name(name) {}
};

void rt::UnitTests::recv_pump()
{
	{
		DatagramPump<SocketIOObj_Routing>	core;
		VERIFY(core.Init());

		InetAddr remote_addr;
		remote_addr.SetAsLocal();
		remote_addr.SetPort(33010);
		SocketIOObj_Routing	remote("Remote");
		remote.Create(remote_addr);
		core.AddObject(&remote);

		InetAddr local_addr;
		local_addr.SetAsLocal();
		local_addr.SetPort(33000);
		SocketIOObj_Routing	local("Local");
		local.Create(local_addr, true);
		core.AddObject(&local);

		_LOGC("Local: " << rt::tos::ip(local_addr));

		InetAddr local_send_addr;
		local_send_addr.SetAsAny();
		local_send_addr.SetPort(33000);
		Socket local_send;
		local_send.Create(local_send_addr, SOCK_DGRAM, true);

		for (UINT i = 0; i < 5; i++)
		{

			local_send.SendTo("from any", sizeof("from any") - 1, remote_addr);
			os::Sleep(100);

			local.SendTo("from local", sizeof("from local") - 1, remote_addr);
			os::Sleep(100);

			remote.SendTo("from remote", sizeof("from remote") - 1, local_addr);
			os::Sleep(1000);
		}

		core.Term();
	}


	{	rt::Buffer<SocketIOObj> sockets;
	sockets.SetSize(100);

	int port_base = 21000;
	for (UINT i = 0; i < sockets.GetSize(); i++)
	{
		InetAddr addr("0.0.0.0", port_base + i);
		VERIFY(sockets[i].Create(addr));
		sockets[i].SetBufferSize(2000);
	}

	DatagramPump<SocketIOObj>	core;
	VERIFY(core.Init());

	InetAddr addr;
	addr.SetAsLocal();

	Socket sender;
	sender.Create(addr, SOCK_DGRAM);

	for (UINT q = 0; q < 3; q++)
		for (UINT i = 0; i < sockets.GetSize(); i++)
		{
			addr.SetPort(port_base + i);
			sockets[i].SendTo(&i, 4, addr);
		}

	os::Sleep(2000);
	_LOG("1 Packet Recv = " << packet_count << ", " << packet_error);

	for (UINT i = 0; i < sockets.GetSize(); i++)
	{
		core.AddObject(&sockets[i]);
	}

	os::Sleep(2000);
	_LOG("2 Packet Recv = " << packet_count << ", " << packet_error);

	for (UINT i = 0; i < sockets.GetSize(); i++)
	{
		addr.SetPort(port_base + i);
		sockets[i].SendTo(&i, 4, addr);
	}

	os::Sleep(2000);
	_LOG("3 Packet Recv = " << packet_count << ", " << packet_error);

	for (UINT i = 0; i < sockets.GetSize(); i++)
	{
		addr.SetPort(port_base + i);
		sender.SendTo(&i, 4, addr);
	}

	os::Sleep(2000);
	_LOG("4 Packet Recv = " << packet_count << ", " << packet_error);
	}
}

/*volatile int _count = 0;
volatile int _error = 0;
struct SocketIOObj_Routing1 : public DatagramSocket
{
	rt::String Name;

	void OnRecv(Datagram* g)
	{
		if (g)
		{						
			//_LOG("[" << Name << "] Recv: \"" << rt::DS(g->RecvBuf, g->RecvSize) << "\" from: " << rt::tos::ip(g->PeerAddressV4).TrimBefore(':'));
			std::uint32_t crc;
			crc = CRC::Calculate(g->RecvBuf, g->RecvSize, CRC::CRC_32());
			for (UINT i = 0; i < 2000; i++)
			{
				os::Sleep(0);
				if (crc != CRC::Calculate(g->RecvBuf, g->RecvSize, CRC::CRC_32()))
				{
					os::AtomicIncrement(&_error);
					_LOG("Find it!");
					_LOG("[" << Name << "] Recv: \"" << rt::DS(g->RecvBuf, g->RecvSize) << "\" from: " << rt::tos::ip(g->PeerAddressV4).TrimBefore(':'));
					_LOG(crc);
					_LOG(CRC::Calculate(g->RecvBuf, g->RecvSize, CRC::CRC_32()));					
				}				
			}
			os::AtomicIncrement(&_count);
			if (_count % 500000 == 0)
				_LOG(_count<<" "<< Name);
		}
			
	}

	SocketIOObj_Routing1(const rt::String_Ref& name) :Name(name) {}
};

char charRand()
{
	char tmp;
	std::random_device rd;
	std::default_random_engine random(rd());
	tmp = random() % 36;	
	if (tmp < 10) {			
		tmp += '0';
	}
	else {			
		tmp -= 10;
		tmp += 'A';
	}
	return tmp;

}*/

//void rt::UnitTests::pump_client()
//{
//	InetAddr remote_addr("192.168.218.72", 33010);
//
//	InetAddr local_send_addr;
//	local_send_addr.SetAsLocal();
//	local_send_addr.SetPort(33000);
//	Socket local_send;
//	local_send.Create(local_send_addr, SOCK_DGRAM, true);
//
//	char send_message[500];
//	for (;;)
//	{
//		for ( int i = 0; i < sizeof(send_message); i++)
//		{
//			send_message[i] = charRand();
//		}
//		local_send.SendTo(send_message, sizeof(send_message), remote_addr);
//		os::Sleep(1);
//	}
//}
//void rt::UnitTests::pump_client2()
//{
//	InetAddr remote_addr("192.168.218.72", 33020);
//
//	InetAddr local_send_addr;
//	local_send_addr.SetAsLocal();
//	local_send_addr.SetPort(32000);
//	Socket local_send;
//	local_send.Create(local_send_addr, SOCK_DGRAM, true);
//
//	char send_message[500];
//	for (;;)
//	{
//		for (int i = 0; i < sizeof(send_message); i++)
//		{
//			send_message[i] = charRand();
//		}
//		local_send.SendTo(send_message, sizeof(send_message), remote_addr);
//		os::Sleep(1);
//	}
//}
//void rt::UnitTests::pump_server()
//{		
//	DatagramPump<SocketIOObj_Routing1>	core;
//	VERIFY(core.Init());
//	InetAddr remote_addr1("192.168.218.72", 33010);
//	InetAddr remote_addr2("192.168.218.72", 33020);
//	SocketIOObj_Routing1	remote1("Remote1");
//	remote1.Create(remote_addr1);
//	SocketIOObj_Routing1	remote2("Remote2");
//	remote2.Create(remote_addr2);
//	core.AddObject(&remote1);
//	core.AddObject(&remote2);
//	for (;;)
//	{
//	}
//
//	core.Term();
//
//}