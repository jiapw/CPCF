#include "../../core/os/kernel.h"
#include "../../core/os/multi_thread.h"
#include "../../core/os/file_dir.h"
#include "../../core/os/file_zip.h"
#include "../../core/os/high_level.h"
#include "../../core/os/precompiler.h"
#include <string>
#include <algorithm>

#pragma warning(disable: 4838)

#include "test.h"







void rt::UnitTests::encoding()
{
	rt::Randomizer r; 

	for(UINT i=0;i<100000;i++)
	{
		char buf[20];
		for(UINT b=1;b<20;b++)
		{
			r.Randomize(buf, b);
			rt::tos::Base32CrockfordOnStack<> base32(buf, b);
			if(i==0)
			{	_LOG(rt::tos::Binary<>(buf,b)<<" = "<<base32);
			}

			char buf_dec[20];
			int dec_b = (int)os::Base32DecodeLength(base32.GetLength());
			os::Base32CrockfordDecode(buf_dec, dec_b, base32, base32.GetLength());
			if(dec_b != b || memcmp(buf_dec, buf, b) != 0)
			{	_LOG_ERROR("ERR: "<<rt::tos::Binary<>(buf,b)<<" = "<<base32<<" => "<<rt::tos::Binary<>(buf_dec, dec_b));
				return;
			}
		}
	}

	for(UINT i=0;i<100000;i++)
	{
		char buf[20];
		for(UINT b=1;b<20;b++)
		{
			r.Randomize(buf, b);
			rt::tos::Base32CrockfordLowercaseOnStack<> base32(buf, b);
			if(i==0)
			{	_LOG(rt::tos::Binary<>(buf,b)<<" = "<<base32);
			}

			char buf_dec[20];
			int dec_b = (int)os::Base32DecodeLength(base32.GetLength());
			os::Base32CrockfordDecode(buf_dec, dec_b, base32, base32.GetLength());
			if(dec_b != b || memcmp(buf_dec, buf, b) != 0)
			{	_LOG_ERROR("ERR: "<<rt::tos::Binary<>(buf,b)<<" = "<<base32<<" => "<<rt::tos::Binary<>(buf_dec, dec_b));
				return;
			}
		}
	}

	for(UINT i=0;i<100000;i++)
	{
		char buf[20];
		for(UINT b=1;b<20;b++)
		{
			r.Randomize(buf, b);
			rt::tos::Base32OnStack<> base32(buf, b);
			if(i==0)
			{	_LOG(rt::tos::Binary<>(buf,b)<<" = "<<base32);
			}

			char buf_dec[20];
			int dec_b = (int)os::Base32DecodeLength(base32.GetLength());
			os::Base32Decode(buf_dec, dec_b, base32, base32.GetLength());
			if(dec_b != b || memcmp(buf_dec, buf, b) != 0)
			{	_LOG_ERROR("ERR: "<<rt::tos::Binary<>(buf,b)<<" = "<<base32<<" => "<<rt::tos::Binary<>(buf_dec, dec_b));
				return;
			}
		}
	}

	for(UINT i=0;i<100000;i++)
	{
		char buf[20];
		for(UINT b=1;b<20;b++)
		{
			r.Randomize(buf, b);
			rt::tos::Base32LowercaseOnStack<> base32(buf, b);
			if(i==0)
			{	_LOG(rt::tos::Binary<>(buf,b)<<" = "<<base32);
			}

			char buf_dec[20];
			int dec_b = (int)os::Base32DecodeLength(base32.GetLength());
			os::Base32Decode(buf_dec, dec_b, base32, base32.GetLength());
			if(dec_b != b || memcmp(buf_dec, buf, b) != 0)
			{	_LOG_ERROR(rt::tos::Binary<>(buf,b)<<" = "<<base32<<" => "<<rt::tos::Binary<>(buf_dec, dec_b));
				return;
			}
		}
	}

	for(UINT i=0;i<100000;i++)
	{
		char buf[20];
		for(UINT b=1;b<20;b++)
		{
			r.Randomize(buf, b);
			rt::tos::Base32CrockfordOnStack<> base32(buf, b);
			if(i==0)
			{	_LOG(rt::tos::Binary<>(buf,b)<<" = "<<base32);
			}

			char buf_dec[20];
			int dec_b = (int)os::Base32DecodeLength(base32.GetLength());
			os::Base32CrockfordDecode(buf_dec, dec_b, base32, base32.GetLength());
			if(dec_b != b || memcmp(buf_dec, buf, b) != 0)
			{	_LOG_ERROR(rt::tos::Binary<>(buf,b)<<" = "<<base32<<" => "<<rt::tos::Binary<>(buf_dec, dec_b));
				return;
			}
		}
	}
}


void rt::UnitTests::timedate()
{
	{	os::Timestamp	tm;
		tm.LoadCurrentTime();
		_LOGC(rt::tos::Timestamp<>(tm));
		os::Sleep(10);
		tm.LoadCurrentTime();
		_LOGC(rt::tos::Timestamp<>(tm));
		os::Sleep(10);
		tm.LoadCurrentTime();
		_LOGC(rt::tos::Timestamp<>(tm));

		os::Timestamp	stm;
		stm.SetDateTime("1980/3/2 12:21:33.321");	_LOG((stm.GetDateTime()));
		stm.SetDateTime("1980/3/2 12:21:33");	_LOG((stm.GetDateTime()));
		stm.SetDateTime("1980/3/2 12:21");	_LOG((stm.GetDateTime()));
		stm.SetDateTime("12:21:33.321");	_LOG((stm.GetDateTime()));
		stm.SetDateTime("12:21:33");	_LOG((stm.GetDateTime()));
		stm.SetDateTime("1980/3/2");	_LOG((stm.GetDateTime()));
		stm.SetDateTime("12:21");	_LOG((stm.GetDateTime()));
		stm.SetDateTime(rt::tos::Number(tm._Timestamp));	_LOG((stm.GetDateTime()));
	}

	{	os::Timestamp tm;
		tm.SetDateTime(1980,3,2);
		_LOG("Day: "<<tm.GetDateTime().DayOfWeek<<" == "<<os::Timestamp::GetDayOfWeek(1980,3,2));
		tm.SetDateTime(1980,3,4);
		_LOG("Day: "<<tm.GetDateTime().DayOfWeek<<" == "<<os::Timestamp::GetDayOfWeek(1980,3,4));
		//_LOG(os::Timestamp::DaysSince0000(1970));
	}

	{	os::Timestamp tm;
		tm.LoadCurrentTime();
		os::Sleep(50);
		LONGLONG time_lapse = tm.TimeLapse();
		_LOG("Timelapse is between [50, 400]: " << (50 <= time_lapse && time_lapse <= 400));
	}

	{	os::HighPerformanceCounter	hpc;
		hpc.LoadCurrentCount();
		os::Sleep(1000);
		_LOGC(hpc.TimeLapse() << " nano-sec");
		os::Sleep(1500);
		_LOGC(hpc.TimeLapse() << " nano-sec");

		hpc.SetOutputMillisecond();
		hpc.LoadCurrentCount();
		os::Sleep(1000);
		_LOGC(hpc.TimeLapse() << " msec");
		os::Sleep(1500);
		_LOGC(hpc.TimeLapse() << " msec");
	}

	{	os::Timestamp tm;
		tm.LoadCurrentTime();
		_LOGC(rt::tos::Timestamp<>(tm));

		tm.SetLocalDateTime(os::Timestamp::Fields("2001-4-3 13:45:3"));
		_LOG(rt::tos::Timestamp<>(tm));

		tm.SetLocalDateTime(os::Timestamp::Fields("2011-7-13"));
		_LOG(rt::tos::Timestamp<>(tm));
	}
	
	{	os::Timestamp	tt;
		os::Timestamp::Fields f;
		f.FromInternetTimeFormat("Tue, 15 Nov 1994 12:45:26 GMT");
		tt.SetDateTime(f);
		_LOG("FromInternetTimeFormat: "<<rt::tos::Timestamp<>(tt, false));

		char ttt[30];
		tt.GetDateTime().ToInternetTimeFormat(ttt);
		_LOG("ToInternetTimeFormat: "<<ttt);
	}

	{	os::Date32 dd;
		dd.SetDate(2012,1,20);
		_LOG("LastMonth: "<<rt::tos::Date<>(dd.LastMonth()));

		os::Date32 dd2 = dd;
		_LOG("Date32 diff 1: "<<dd2 - dd<<','<<dd - dd2);

		dd2++;
		_LOG("Date32 diff 1: "<<dd2 - dd<<','<<dd - dd2);

		dd2++;
		_LOG("Date32 diff 1: "<<dd2 - dd<<','<<dd - dd2);
	}

	{	_LOG("Waiting for 2 seconds ...");
		os::Timestamp	t1,t2;
		os::TickCount	tick;
		t1.LoadCurrentTime();
		tick.LoadCurrentTick();
		os::Sleep(2100);
		t2.LoadCurrentTime();
		_LOG("os::TickCount in sec: " << tick.TimeLapse() / 1000);
	}
}

void rt::UnitTests::commandline()
{
	os::CommandLine cmd;
	cmd.SetOptionDefault("TEST_OPT", "SomeValue");
	cmd.LoadEnvironmentVariablesAsOptions();

	rt::String text = "Shell: %ComSpec%\nTest: [%TEST_OPT%]";
	cmd.SubstituteOptions(text);

	_LOG(text);
}

void rt::UnitTests::async_write()
{
	rt::Buffer<BYTE>	data;
	data.SetSize(23*1024*1024);
	data.RandomBits(94784);

	LPCSTR fn = "async_write.test";

	{
		os::FileWrite f;
		if(f.Open(fn, os::FileWrite::FW_TRUNCATE|os::FileWrite::FW_ASYNC, 8))
		{
			for(UINT i=0; i<data.GetSize(); i+=23*1024)
			{
				if(!f.Write(&data[i], 23*1024))
					return;
			}
		}

		f.WriteHeader(0x12345678deadbeefULL);
		f.Close();

		os::FileBuffer<BYTE> load;
		load.Open(fn);

		_LOG("Size Header: "<<load.GetSize() - data.GetSize());
		_LOG("Header = "<<rt::tos::HexNum<>(*(ULONGLONG*)load.Begin()));
		_LOG("Content Match: "<<(0==memcmp(load.Begin() + 8, data.Begin(), data.GetSize())));
	}

	{
		os::FileWrite f;
		if(f.Open(fn, os::FileWrite::FW_TRUNCATE|os::FileWrite::FW_UTF8SIGN))
		{
			for(UINT i=0; i<data.GetSize(); i+=23*1024)
			{
				if(!f.Write(&data[i], 23*1024))
					return;
			}
		}

		f.Close();

		os::FileBuffer<BYTE> load;
		load.Open(fn);

		_LOG("Size Header: "<<load.GetSize() - data.GetSize());
		_LOG("Header = "<<rt::tos::Base16OnStack<>(load.Begin(), 3));
		_LOG("Content Match: "<<(0==memcmp(load.Begin() + 3, data.Begin(), data.GetSize())));
	}

	os::File::RemovePath(fn);
}

void rt::UnitTests::file()
{
	{	os::FileBuffer<BYTE>	file("D:/ArtSq/Coin-FE/Dev/proj/unit_test/captcha_bg.jpg");
		os::File out;
		out.Open("data_define.hpp", os::File::Normal_WriteText);
		out.Write(rt::SS("static const BYTE _Data[") + (ULONGLONG)file.GetSize() + "] = {\n");
		LPCBYTE p = file;
		UINT sz = (UINT)file.GetSize();
		while(sz)
		{
			UINT block = rt::min(32U, sz);
			out.Write("\t",1);
			for(UINT i=0;i<block;i++)
			{
				char buf[10];
				sprintf(buf,"0x%02X,",p[i]);
				if(sz == block && i == block-1)
					out.Write(buf, 4);
				else
					out.Write(buf, 5);
			}
			sz-=block;
			p+=block;
			out.Write("\n",1);
		}
		out.Write("};\n",3);
		out.Close();
	}


	{	rt::String	fn;
		for(int i=0;i<10;i++)
		{
			LPCSTR filename = "test.txt";
			if(os::File::ProbeAvailableFilename(filename, fn))
				filename = fn;
			os::File(filename, os::File::Normal_Write).Write("123",3);
			_LOG(fn);
		}

		os::File::Remove("test.txt");
		os::FileList fl;
		fl.Populate(".", ").txt");
		for(UINT i=0; i<fl.GetCount(); i++)
			os::File::Remove(fl.GetFilename(i).Begin() + 1);
	}

	LPCSTR fn = "test_\xe4\xbd\xa0\xe5\xa5\xbd.txt";
	
	os::File	file;
	if(file.Open(fn, os::File::Normal_Write))
	{
		file.Write(rt::String_Ref("Testing String wrote to file !!"));
		file.Close();
	}

	rt::String val;
	if(	file.Open(fn) &&
		val.SetLength((SIZE_T)file.GetLength()) &&
		(SIZE_T)(file.Read(val,(UINT)val.GetLength()) == val.GetLength())
	)
	{	_LOG("File Content: "<<val);
		file.Close();
		_LOG("File Remove: "<<os::File::Remove(fn));
		_LOG("File Remove: "<<os::File::Remove(fn));
	}

	{	os::FilePacked_Writer f;
		f.Open("fpack.ckdp");
		f.Write(0x1001, "1234567890" ,8);
		f.Write(0x1002, "1234567890" ,7);
		f.Write(2, "1234567890" ,6);
		f.Write(1, "1234567890" ,5);
	}

	{	os::FilePacked_Reader f;
		f.Open("fpack.ckdp");
		rt::String str;
		UINT idx = f.FindFirstFile(2);
		if(idx != INFINITE)
		{	str.SetLength(f.GetFileSize(idx));
			f.Read(idx,str);
			_LOG("2 => "<<str);
		}

		if((idx = f.FindFirstFile(0x1000,0xf000)) != INFINITE)
		{
			do
			{	str.SetLength(f.GetFileSize(idx));
				f.Read(idx,str);
				_LOG(f.GetFileName(idx)<<" => "<<str);
			}while((idx = f.FindNextFile()) != INFINITE);
		}
	}
}

void rt::UnitTests::pfw()
{
	{	os::ParallelFileWriter	pfw;

		pfw.Open("pfw_test.txt", false);
		pfw.SetWriteDownInterval(1000);


		struct _call
		{
			static DWORD _func(LPVOID p)
			{
				os::ParallelFileWriter* w = (os::ParallelFileWriter*)p;
				for(UINT i=0;i<5000;i++)
				{
					w->WriteString(rt::String_Ref("ParallelFileWriter Test ") + rt::tos::Timestamp<>(os::Timestamp::Get()) + rt::SS(" \r\n"));
					os::Sleep(10);
				}
				return 0;
			}
		};

		rt::Buffer<os::Thread>	threads;
		threads.SetSize(100);
		for(UINT i=0;i<threads.GetSize();i++)
		{
			threads[i].Create(_call::_func, &pfw);
		}

		for(UINT i=0;i<threads.GetSize();i++)
			threads[i].WaitForEnding();

		pfw.LogAlert();
		pfw.Close();

		os::FileReadLine file;
		file.Open("pfw_test.txt");

		rt::String_Ref line;
		int lines = 0;
		int len = -1;

		while(file.GetNextLine(line))
		{
			if(len == -1)
			{	len = (int)line.GetLength();
			}
			else
			{	ASSERT(len == line.GetLength());
			}
		
			lines++;
		}

		_LOG("Total lines: "<<lines);
	}

	{	os::ParallelFileWriter	pfw;
		pfw.Open("pfw_test_%HOUR%.txt");
		pfw.SetWriteDownInterval(500);

		for(UINT i=0;i<100;i++)
		{	pfw.WriteLine(rt::SS("Some thing appended") + rt::tos::Timestamp<>(os::Timestamp::Get()));
			os::Sleep(100);
		}
	}

	{	os::ParallelFileWriter	pfw;
		pfw.Open("pfw_test_%HOUR%.txt");
		pfw.SetWriteDownInterval(500);

		for(UINT i=0;i<100;i++)
		{	pfw.WriteLine(rt::SS("Some thing appended again") + rt::tos::Timestamp<>(os::Timestamp::Get()));
			os::Sleep(100);
		}
	}
}

void rt::UnitTests::plog()
{
#ifndef PLATFORM_DISABLE_LOG
	os::ParallelLog log(500);

	for(;;)
	{	_LOG("Tick: "<<os::TickCount::Get());
		os::Sleep(100);
	}

	//log.Write("test", "test_file", 0, "test_func", rt::LOGTYPE_IN_CONSOLE|rt::LOGTYPE_INFORMATIONAL);
#endif
}

/*
void test_ipp_zip()
{
	os::FileZip	zip;
	zip.SetCompressionMode();

	zip.Open("TestOut.zip", os::File::Normal_Write,false) &&
	zip.AddZeroSizedEntry("test") &&
	zip.AddFile("1.txt", "Content of 1.txtContent of 1.txtContent of 1.txtContent of 1.txtContent of 1.txtContent of 1.txtContent of 1.txtContent of 1.txtContent of 1.txtContent of 1.txt", 160) &&
	zip.AddFile("test/2.txt", "Content of 2.txtContent of 2.txtContent of 2.txtContent of 2.txtContent of 2.txtContent of 2.txtContent of 2.txtContent of 2.txtContent of 2.txtContent of 2.txt", 160) &&
	zip.Save();

//#ifdef PLATFORM_WIN
//	os::EnableCrashDump("shared_test", true, true);
//	_asm int 3;
//#endif
}
*/

void rt::UnitTests::b64_encoding()
{
	{	static const LPCSTR testcase[][2] = 
		{
			{ "1", "MQ==" },
			{ "12", "MTI=" },
			{ "123", "MTIz" },
			{ "1234", "MTIzNA=="},
			{ "12345", "MTIzNDU=" }
		};

		for(UINT i=0;i<sizeofArray(testcase);i++)
		{
			rt::String_Ref text(testcase[i][0]);
			rt::String_Ref b64(testcase[i][1]);

			char buf[100];
			ASSERT(os::Base64EncodeLength(text.GetLength())==b64.GetLength());
			os::Base64Encode(buf, text.Begin(), text.GetLength());
			_LOG(rt::String_Ref(buf, b64.GetLength())<<" = "<<b64);
			
			ASSERT(os::Base64DecodeLength(b64.Begin(), b64.GetLength())==text.GetLength());
			SIZE_T len;
			_LOG("Full Data: "<<os::Base64Decode(buf, &len, b64.Begin(), b64.GetLength()));
			_LOG(rt::String_Ref(buf, len)<<" = "<<text);
		}

		{	// robust decoding
			char buf[100];
			rt::String_Ref b64("MTIzNDU\n\rew");
			SIZE_T  len;
			_LOG("Full Data: "<<os::Base64Decode(buf, &len, b64.Begin(), b64.GetLength()));
			_LOG(rt::String_Ref(buf, len));
		}

		_LOG(rt::tos::Base64OnStack<256>("123456", 6));
	}

	return;
}

struct A
{
	os::TickCount tc;
	A(){ tc.LoadCurrentTick(); }
	~A()
	{	_LOG("killed, after "<<tc.TimeLapse()<<" msec");
	}
};

void rt::UnitTests::delayed_deletion()
{
	A* p = new A;
	_SafeDel_Delayed(p, 2000);
	p = new A;
	_SafeDel_Delayed(p, 5000);

	os::Sleep(9000);
}

void rt::UnitTests::sysinfo()
{
	rt::String exe, host, user;
	os::GetExecutableFilename(exe);
	os::GetHostName(host);
	os::GetLogonUserName(user);

	_LOGC("Executable: "<<exe);
	_LOGC("User: "<<user<<'@'<<host);

	_LOGC("#CPU: "<<os::GetNumberOfProcessors());
	_LOGC("#CPU (Physical): "<<os::GetNumberOfPhysicalProcessors());
	
	rt::String os_name, os_version;
	os::GetOSVersion(os_version, false);
	os::GetOSVersion(os_name, true);
	_LOGC("OS: "<<os_name<<"  "<<os_version);
	
	ULONGLONG busy[2], total[2];
	{
		os::GetProcessorTimes(busy, total);
		os::Sleep(1000);
		os::GetProcessorTimes(busy+1, total+1);
		_LOGC("INV: "<<(total[1] - total[0]));
	}
	_LOGC("CPU USAGE: "<<100*(busy[1]-busy[0])/(total[1]-total[0])<<'%');
	{	ULONGLONG free, total;
		os::GetSystemMemoryInfo(&free, &total);
		_LOGC("MEMORY: " << rt::tos::FileSize<>(free) << '/' << rt::tos::FileSize<>(total));
	}
	{	SIZE_T free, total;
		os::GetProcessMemoryLoad(&free, &total);
		_LOGC("MEMORY Load: "<<rt::tos::FileSize<>(free)<<'/'<<rt::tos::FileSize<>(total));
	}
	
	_LOGC("FREE DISK: "<<rt::tos::FileSize<>(os::GetFreeDiskSpace("./")));
	_LOGC("Power State: "<<os::GetPowerState());

	rt::String uid;
	os::GetDeviceUID(uid);
	_LOGC("DevUID: "<<uid);
}


void rt::UnitTests::filelist()
{
	rt::String fp;
	os::FileList	fl;
	fl.Populate("../tests", ".cpp");
	fl.Sort();
	for(UINT i=0;i<fl.GetCount();i++)
	{
		_LOG(fl.GetFilename(i));
	}
    
    rt::Buffer<os::Process::Info>   list;
    os::Process::Populate(list);

	_LOG("\nProcess Listed "<<!!list.GetSize());
	for(UINT i=0;i<list.GetSize();i++)
	{
		_LOGC(list[i].Name.GetFileName()<<'('<<list[i].PID<<"), "<<rt::tos::Timestamp<>(list[i].StartTime));
	}
}

void rt::UnitTests::sockettimed()
{
	inet::SocketTimed s;
	inet::InetAddr addr;
	addr.SetAsLocal();
	s.Create(addr);
	if(s.ConnectTo(inet::InetAddr("i-funbox.com",80)))
	{
		_LOG("connected");
	}
	else
	{
		_LOG("failed");
	}

}





void rt::UnitTests::vm()
{
	LPVOID p = os::VMAlloc(1024*1024*1024);
	_LOG("1G VM: "<<!!p);

	((LPBYTE)p)[1024*1024*1024 - 1] = 0;
	os::VMFree(p, 1024*1024*1024);

	os::FileMapping map;
	
	os::File::Remove("filemapping.data");
	map.Open("filemapping.data", 100*1024*1024, false);
	_LOG("100M File Mapping: "<<!!map.GetBasePtr());

	_LOG("Write Head");
	((LPBYTE)map.GetBasePtr())[0] = 0;
	_LOG("Write Tail");
	((LPBYTE)map.GetBasePtr())[100*1024*1024 - 1] = 0;
	map.Close();

	os::File::Remove("filemapping.data");
}


