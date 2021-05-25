#include "../../core/inet/inet.h"
#include "../../core/inet/datagram_pump.h"
#include "test.h"

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