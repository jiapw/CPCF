#include "../../core/os/kernel.h"
#include "../../core/os/multi_thread.h"
#include "../../core/os/file_zip.h"
#include "../../core/os/high_level.h"
#include "../../core/os/precompiler.h"
#include <string>
#include <algorithm>

#pragma warning(disable: 4838)

#include "test.h"




void rt::UnitTests::lockfile()
{	
	os::File out;
	out.Open("lockfile.txt", os::File::Normal_WriteText);
	if (!out.IsLockAcquired())
	{
		auto ans = out.Lock(false);
		if (ans)
		{
			_LOG("Locked");
			os::Sleep(10000);
			out.Unlock();
			_LOG("Unlocked");
		}
		else
		{
			_LOG("failed");
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



















