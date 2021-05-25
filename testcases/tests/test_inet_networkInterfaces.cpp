#include "../../core/inet/inet.h"
#include "../../core/inet/datagram_pump.h"
#include "test.h"

using namespace inet;

void rt::UnitTests::net_interfaces()
{
	auto scan = [](){
		rt::BufferEx<NetworkInterface>	nic;
		NetworkInterfaces::Populate(nic, false, false);

		for(auto it : nic)
		{
			_LOG("NIC: <"<<it.Name<<"> T"<<(it.Type&NITYPE_MASK)<<' '<<(it.IsOnline()?"online":"offline"));
			if(!it.IsOnline())continue;

			for(UINT i=0; i<it.v4Count; i++)
				_LOG("\tIPv4:"<<rt::tos::ip(InetAddr(it.v4[i].Local,0)).TrimAfterReverse(':')<<'/'
							  <<rt::tos::ip(InetAddr(it.v4[i].Boardcast,0)).TrimAfterReverse(':'));

			for(UINT i=0; i<it.v6Count; i++)
			{
				InetAddrV6 addr;
				addr.SetBinaryAddress(it.v6[i].Local);
				_LOG("\tIPv6:"<<rt::tos::ip(addr).TrimAfterReverse(':')<<
							  ((it.Type&NITYPE_MULTICAST)?"/multicast":""));
			}
		}
	};

	scan();
	NetworkInterfaces evt;

	for(;;)
	{
		os::Sleep(100);
		if(evt.GetState() == NetworkInterfaces::Reconfiguring)
		{
            _LOGC("\n\nNetwork Interface reconfiguring ...");
            for(;;)
            {
                os::Sleep(100);
                if(evt.GetState() == NetworkInterfaces::Reconfigured)
                {
                    _LOGC("Reconfigured:");
                    scan();
                    break;
                }
            }
		}
	}
}
