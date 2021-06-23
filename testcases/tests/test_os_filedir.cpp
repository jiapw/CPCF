#include "../../core/os/file_dir.h"
#include <string>
#include <algorithm>

#pragma warning(disable: 4838)

#include "test.h"
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
	data.SetSize(23 * 1024 * 1024);
	data.RandomBits(94784);

	LPCSTR fn = "async_write.test";

	{
		os::FileWrite f;
		if (f.Open(fn, os::FileWrite::FW_TRUNCATE | os::FileWrite::FW_ASYNC, 8))
		{
			for (UINT i = 0; i < data.GetSize(); i += 23 * 1024)
			{
				if (!f.Write(&data[i], 23 * 1024))
					return;
			}
		}

		f.WriteHeader(0x12345678deadbeefULL);
		f.Close();

		os::FileBuffer<BYTE> load;
		load.Open(fn);

		_LOG("Size Header: " << load.GetSize() - data.GetSize());
		_LOG("Header = " << rt::tos::HexNum<>(*(ULONGLONG*)load.Begin()));
		_LOG("Content Match: " << (0 == memcmp(load.Begin() + 8, data.Begin(), data.GetSize())));

	}

	{
		os::FileWrite f;
		if (f.Open(fn, os::FileWrite::FW_TRUNCATE | os::FileWrite::FW_UTF8SIGN))
		{
			for (UINT i = 0; i < data.GetSize(); i += 23 * 1024)
			{
				if (!f.Write(&data[i], 23 * 1024))
					return;
			}
		}

		f.Close();

		os::FileBuffer<BYTE> load;
		load.Open(fn);

		_LOG("Size Header: " << load.GetSize() - data.GetSize());
		_LOG("Header = " << rt::tos::Base16OnStack<>(load.Begin(), 3));
		_LOG("Content Match: " << (0 == memcmp(load.Begin() + 3, data.Begin(), data.GetSize())));

	}

	os::File::RemovePath(fn);
}

void rt::UnitTests::file()
{
	{	os::FileBuffer<BYTE>	file("D:/JPG001.jpg");
	os::File out;
	out.Open("data_define.hpp", os::File::Normal_WriteText);
	out.Write(rt::SS("static const BYTE _Data[") + (ULONGLONG)file.GetSize() + "] = {\n");
	LPCBYTE p = file;
	UINT sz = (UINT)file.GetSize();
	while (sz)
	{
		UINT block = rt::min(32U, sz);
		out.Write("\t", 1);
		for (UINT i = 0; i < block; i++)
		{
			char buf[10];
			sprintf(buf, "0x%02X,", p[i]);
			if (sz == block && i == block - 1)
				out.Write(buf, 4);
			else
				out.Write(buf, 5);
		}
		sz -= block;
		p += block;
		out.Write("\n", 1);
	}
	out.Write("};\n", 3);
	out.Close();
	}


	{	rt::String	fn;
	for (int i = 0; i < 10; i++)
	{
		LPCSTR filename = "test.txt";
		if (os::File::ProbeAvailableFilename(filename, fn))
			filename = fn;
		os::File(filename, os::File::Normal_Write).Write("123", 3);
		_LOG(fn);
	}

	os::File::Remove("test.txt");
	os::FileList fl;
	fl.Populate(".", ").txt");
	for (UINT i = 0; i < fl.GetCount(); i++)
	{
		auto t = fl.GetFilename(i);
		os::File::Remove(fl.GetFilename(i).Begin() + 1);
	}
	}

	LPCSTR fn = "test_\xe4\xbd\xa0\xe5\xa5\xbd.txt";

	os::File	file;
	if (file.Open(fn, os::File::Normal_Write))
	{
		file.Write(rt::String_Ref("Testing String wrote to file !!"));
		file.Close();
	}

	rt::String val;
	if (file.Open(fn) &&
		val.SetLength((SIZE_T)file.GetLength()) &&
		(SIZE_T)(file.Read(val, (UINT)val.GetLength()) == val.GetLength())
		)
	{
		_LOG("File Content: " << val);
		file.Close();
		_LOG("File Remove: " << os::File::Remove(fn));
		_LOG("File Remove: " << os::File::Remove(fn));
	}

	{	os::FilePacked_Writer f;
	f.Open("fpack.ckdp");
	f.Write(0x1001, "1234567890", 8);
	f.Write(0x1002, "1234567890", 7);
	f.Write(2, "1234567890", 6);
	f.Write(1, "1234567890", 5);
	}

	{	os::FilePacked_Reader f;
	f.Open("fpack.ckdp");
	rt::String str;
	UINT idx = f.FindFirstFile(2);
	if (idx != INFINITE)
	{
		str.SetLength(f.GetFileSize(idx));
		f.Read(idx, str);
		_LOG("2 => " << str);
	}

	if ((idx = f.FindFirstFile(0x1000, 0xf000)) != INFINITE)
	{
		do
		{
			str.SetLength(f.GetFileSize(idx));
			f.Read(idx, str);
			_LOG(f.GetFileName(idx) << " => " << str);
		} while ((idx = f.FindNextFile()) != INFINITE);
	}
	}
}

void rt::UnitTests::filelist()
{
	rt::String fp;
	os::FileList	fl;
	fl.Populate("../tests", ".cpp");
	fl.Sort();
	for (UINT i = 0; i < fl.GetCount(); i++)
	{
		_LOG(fl.GetFilename(i));
	}

	rt::Buffer<os::Process::Info>   list;
	os::Process::Populate(list);

	_LOG("\nProcess Listed " << !!list.GetSize());
	for (UINT i = 0; i < list.GetSize(); i++)
	{
		_LOGC(list[i].Name.GetFileName() << '(' << list[i].PID << "), " << rt::tos::Timestamp<>(list[i].StartTime));
	}
}
void rt::UnitTests::lockfile()
{
	/*{
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
	}*/
	{
		os::File out1;
		out1.Open("lockfile.txt", os::File::Normal_Read);
		auto ans1 = out1.Lock();
		_LOG("Fd " << out1.GetFD()<<" ans " << ans1);
		os::File out2;
		out2.Open("lockfile.txt", os::File::Normal_Read);
		auto ans2 = out2.Lock();
		_LOG("Fd " << out2.GetFD() << " ans " << ans2);
		out1.Unlock();
		//out2.Unlock();
	}
}