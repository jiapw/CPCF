#include "../../core/os/kernel.h"

#pragma warning(disable: 4838)

#include "test.h"

void rt::UnitTests::encoding()
{
	rt::Randomizer r;

	for (UINT i = 0; i < 100000; i++)
	{
		char buf[20];
		for (UINT b = 1; b < 20; b++)
		{
			r.Randomize(buf, b);
			rt::tos::Base32CrockfordOnStack<> base32(buf, b);
			if (i == 0)
			{
				_LOG(rt::tos::Binary<>(buf, b) << " = " << base32);
			}

			char buf_dec[20];
			int dec_b = (int)os::Base32DecodeLength(base32.GetLength());
			os::Base32CrockfordDecode(buf_dec, dec_b, base32, base32.GetLength());
			if (dec_b != b || memcmp(buf_dec, buf, b) != 0)
			{
				_LOG_ERROR("ERR: " << rt::tos::Binary<>(buf, b) << " = " << base32 << " => " << rt::tos::Binary<>(buf_dec, dec_b));
				return;
			}
		}
	}

	for (UINT i = 0; i < 100000; i++)
	{
		char buf[20];
		for (UINT b = 1; b < 20; b++)
		{
			r.Randomize(buf, b);
			rt::tos::Base32CrockfordLowercaseOnStack<> base32(buf, b);
			if (i == 0)
			{
				_LOG(rt::tos::Binary<>(buf, b) << " = " << base32);
			}

			char buf_dec[20];
			int dec_b = (int)os::Base32DecodeLength(base32.GetLength());
			os::Base32CrockfordDecode(buf_dec, dec_b, base32, base32.GetLength());
			if (dec_b != b || memcmp(buf_dec, buf, b) != 0)
			{
				_LOG_ERROR("ERR: " << rt::tos::Binary<>(buf, b) << " = " << base32 << " => " << rt::tos::Binary<>(buf_dec, dec_b));
				return;
			}
		}
	}

	for (UINT i = 0; i < 100000; i++)
	{
		char buf[20];
		for (UINT b = 1; b < 20; b++)
		{
			r.Randomize(buf, b);
			rt::tos::Base32OnStack<> base32(buf, b);
			if (i == 0)
			{
				_LOG(rt::tos::Binary<>(buf, b) << " = " << base32);
			}

			char buf_dec[20];
			int dec_b = (int)os::Base32DecodeLength(base32.GetLength());
			os::Base32Decode(buf_dec, dec_b, base32, base32.GetLength());
			if (dec_b != b || memcmp(buf_dec, buf, b) != 0)
			{
				_LOG_ERROR("ERR: " << rt::tos::Binary<>(buf, b) << " = " << base32 << " => " << rt::tos::Binary<>(buf_dec, dec_b));
				return;
			}
		}
	}

	for (UINT i = 0; i < 100000; i++)
	{
		char buf[20];
		for (UINT b = 1; b < 20; b++)
		{
			r.Randomize(buf, b);
			rt::tos::Base32LowercaseOnStack<> base32(buf, b);
			if (i == 0)
			{
				_LOG(rt::tos::Binary<>(buf, b) << " = " << base32);
			}

			char buf_dec[20];
			int dec_b = (int)os::Base32DecodeLength(base32.GetLength());
			os::Base32Decode(buf_dec, dec_b, base32, base32.GetLength());
			if (dec_b != b || memcmp(buf_dec, buf, b) != 0)
			{
				_LOG_ERROR(rt::tos::Binary<>(buf, b) << " = " << base32 << " => " << rt::tos::Binary<>(buf_dec, dec_b));
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
	//tm._Timestamp here equals ms since 00:00:00 January 1, 1970.
	stm.SetDateTime(rt::tos::Number(tm._Timestamp));	_LOG((stm.GetDateTime()));
	}

	{	os::Timestamp tm;
	tm.SetDateTime(1980, 3, 2);
	_LOG("Day: " << tm.GetDateTime().DayOfWeek << " == " << os::Timestamp::GetDayOfWeek(1980, 3, 2));
	tm.SetDateTime(1980, 3, 4);
	_LOG("Day: " << tm.GetDateTime().DayOfWeek << " == " << os::Timestamp::GetDayOfWeek(1980, 3, 4));

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
	_LOG("FromInternetTimeFormat: " << rt::tos::Timestamp<>(tt, false));

	char ttt[30];
	tt.GetDateTime().ToInternetTimeFormat(ttt);
	_LOG("ToInternetTimeFormat: " << ttt);
	}

	{	os::Date32 dd;
	dd.SetDate(2012, 1, 20);
	_LOG("LastMonth: " << rt::tos::Date<>(dd.LastMonth()));

	os::Date32 dd2 = dd;
	_LOG("Date32 diff 1: " << dd2 - dd << ',' << dd - dd2);

	dd2++;
	_LOG("Date32 diff 1: " << dd2 - dd << ',' << dd - dd2);

	dd2++;
	_LOG("Date32 diff 1: " << dd2 - dd << ',' << dd - dd2);
	}

	{	_LOG("Waiting for 2 seconds ...");
	os::Timestamp	t1, t2;
	os::TickCount	tick;
	t1.LoadCurrentTime();
	tick.LoadCurrentTick();
	os::Sleep(2100);
	t2.LoadCurrentTime();
	_LOG("os::TickCount in sec: " << tick.TimeLapse() / 1000);
	}
}

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

	for (UINT i = 0; i < sizeofArray(testcase); i++)
	{
		rt::String_Ref text(testcase[i][0]);
		rt::String_Ref b64(testcase[i][1]);

		char buf[100];
		ASSERT(os::Base64EncodeLength(text.GetLength()) == b64.GetLength());
		os::Base64Encode(buf, text.Begin(), text.GetLength());
		_LOG(rt::String_Ref(buf, b64.GetLength()) << " = " << b64);

		ASSERT(os::Base64DecodeLength(b64.Begin(), b64.GetLength()) == text.GetLength());
		SIZE_T len;
		_LOG("Full Data: " << os::Base64Decode(buf, &len, b64.Begin(), b64.GetLength()));
		_LOG(rt::String_Ref(buf, len) << " = " << text);
	}

	{	// robust decoding
		char buf[100];
		rt::String_Ref b64("MTIzNDU\n\rew");
		SIZE_T  len;
		_LOG("Full Data: " << os::Base64Decode(buf, &len, b64.Begin(), b64.GetLength()));
		_LOG(rt::String_Ref(buf, len));
	}

	_LOG(rt::tos::Base64OnStack<256>("123456", 6));
	}

	return;
}

void rt::UnitTests::sysinfo()
{
	rt::String exe, host, user;
	os::GetExecutableFilename(exe);
	os::GetHostName(host);
	os::GetLogonUserName(user);

	_LOGC("Executable: " << exe);
	_LOGC("User: " << user << '@' << host);

	_LOGC("#CPU: " << os::GetNumberOfProcessors());
	_LOGC("#CPU (Physical): " << os::GetNumberOfPhysicalProcessors());

	rt::String os_name, os_version;
	os::GetOSVersion(os_version, false);
	os::GetOSVersion(os_name, true);
	_LOGC("OS: " << os_name << "  " << os_version);

	ULONGLONG busy[2], total[2];
	{
		os::GetProcessorTimes(busy, total);
		os::Sleep(1000);
		os::GetProcessorTimes(busy + 1, total + 1);
		_LOGC("INV: " << (total[1] - total[0]));
	}
	_LOGC("CPU USAGE: " << 100 * (busy[1] - busy[0]) / (total[1] - total[0]) << '%');
	{	ULONGLONG free, total;
	os::GetSystemMemoryInfo(&free, &total);
	_LOGC("MEMORY: " << rt::tos::FileSize<>(free) << '/' << rt::tos::FileSize<>(total));
	}
	{	SIZE_T free, total;
	os::GetProcessMemoryLoad(&free, &total);
	_LOGC("MEMORY Load: " << rt::tos::FileSize<>(free) << '/' << rt::tos::FileSize<>(total));
	}

	_LOGC("FREE DISK: " << rt::tos::FileSize<>(os::GetFreeDiskSpace("./")));
	_LOGC("Power State: " << os::GetPowerState());

	rt::String uid;
	os::GetDeviceUID(uid);
	_LOGC("DevUID: " << uid);
}

void rt::UnitTests::vm()
{
	LPVOID p = os::VMAlloc(1024 * 1024 * 1024);	
	_LOG("1G VM: " << !!p);
	((LPBYTE)p)[1024 * 1024 * 1024 - 1] = 0;
	os::VMFree(p, 1024 * 1024 * 1024);

	os::FileMapping map;

	os::File::Remove("filemapping.data");
	map.Open("filemapping.data", 100 * 1024 * 1024, false);
	_LOG("100M File Mapping: " << !!map.GetBasePtr());

	_LOG("Write Head");
	((LPBYTE)map.GetBasePtr())[0] = 0;
	_LOG("Write Tail");
	((LPBYTE)map.GetBasePtr())[100 * 1024 * 1024 - 1] = 0;
	map.Close();

	os::File::Remove("filemapping.data");
}