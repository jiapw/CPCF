#include "../../core/rt/string_type.h"
#include "../../core/rt/buffer_type.h"
#include "../../core/rt/small_math.h"
#include "../../core/rt/xml_xhtml.h"
#include "../../core/rt/json.h"
#include "../../core/os/kernel.h"
#include "../../core/os/multi_thread.h"
#include "../../core/os/file_dir.h"
#include "../../core/os/file_zip.h"
#include "../../core/os/high_level.h"
#include "../../core/inet/inet.h"
#include "../../core/ext/ipp/ipp_image.h"

#if defined(PLATFORM_WIN)
#include "../../core/os/user_inputs.h"
#endif



void diff_infoset(os::Process::Info* prev, int prev_co, os::Process::Info* now, int now_co, rt::String& result)
{
    result.Empty();
    
	for(int i=0;i<now_co;i++)
	{
		os::Process::Info& n = now[i];
		os::Process::Info* match = nullptr;
		for(int j=0;j<prev_co;j++)
		{
			if(n.PID == prev[j].PID)
			{
				match = &prev[j];
				break;
			}
		}

		if(match)
		{
		}
		else
		{
			result += rt::SS("NEW,") + n.Name.GetFileName() + ',' + rt::tos::Timestamp<>(n.StartTime) + '\n';
		}
	}

	for(int i=0;i<prev_co;i++)
	{
		os::Process::Info& n = prev[i];
		os::Process::Info* match = nullptr;
		for(int j=0;j<now_co;j++)
		{
			if(n.PID == now[j].PID)
			{
				match = &now[j];
				break;
			}
		}

		if(match == nullptr)
		{
			result += rt::SS("DEL,") + n.Name.GetFileName() + ',' + rt::tos::Timestamp<>(n.StartTime) + '\n';
		}
	}
}

#if defined(PLATFORM_WIN)

rt::String all;

void exp_tracking_proc_ip()
{
    struct _ui: public os::UserInputSubscriber
    {
        void OnUserInputEvent(const os::UserInputEvent& x)
        {
            _LOG(all);
        }
    };
    
    _ui ui;
    os::UserInputEventPump::Get()->AddSubscriber(&ui);
    
	rt::String log_fn;
	os::GetAppSandbox(log_fn, "tester");

	os::File proc_log;
	if(!proc_log.Open(log_fn + "/proc.log", os::File::Normal_AppendText))return;
	proc_log.Write(rt::SS("---,---,") + rt::tos::Timestamp<>(os::Timestamp::Get()) + '\n');

	os::File ip_log;
	if(!ip_log.Open(log_fn + "/ip.log", os::File::Normal_AppendText))return;
	proc_log.Write(rt::SS("---,---,") + rt::tos::Timestamp<>(os::Timestamp::Get()) + '\n');

	rt::Buffer<os::Process::Info>	proc_list[2];
	rt::Buffer<os::Process::Info>	ip_list[2];

    rt::String diff;
    
	for(;;)
	{
		os::Process::Populate(proc_list[1]);
		diff_infoset(proc_list[0].Begin(), (int)proc_list[0].GetSize(), proc_list[1].Begin(), (int)proc_list[1].GetSize(), diff);
        
        if(!diff.IsEmpty())
        {
            proc_log.Write(diff);
            _LOG(diff);
            all += diff;
        }

		rt::Swap(proc_list[0], proc_list[1]);

		inet::InetAddr addr[256];
		UINT co = inet::GetLocalAddresses(addr, 256, true);

		ip_list[1].SetSize(co);
		for(UINT i=0;i<co;i++)
		{	ip_list[1][i].PID = *((UINT*)addr[i].GetBinaryAddress());

			char pp[1024];
			ip_list[1][i].Name = addr[i].GetDottedDecimalAddress(pp);
			ip_list[1][i].StartTime = 0;
		}
		diff_infoset(ip_list[0].Begin(), (int)ip_list[0].GetSize(), ip_list[1].Begin(), (int)ip_list[1].GetSize(), diff);
		rt::Swap(ip_list[0], ip_list[1]);
        
        if(!diff.IsEmpty())
        {
            proc_log.Write(diff);
            _LOG(diff);
            all += diff;
        }
        
		os::Sleep(15*1000);
        
        proc_log.Flush();
        ip_log.Flush();
    }
}

void image_to_text()
{
	ipp::Image_1c8u img;
	rt::String b = os::__UTF8(L"öc");
	rt::String w = os::__UTF8(L"¡¡");

	rt::String out;

	if(img.Load("qrcode.png"))
	{
		for(UINT y=0; y<img.GetHeight(); y++)
		{
			for(UINT x=0; x<img.GetWidth(); x++)
			{
				if(img(x,y).x > 100)
					out += w;
				else
					out += b;
			}
			out += "\r";
		}
	}

	os::File::SaveText("qrcode.txt", out, true);
}

#endif
