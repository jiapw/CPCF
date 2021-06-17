#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include "../../core/os/file_dir.h"
#include "../../core/os/kernel.h"

#include <string>
void TestMain(int  mode=0);

int _tmain(int argc, _TCHAR* argv[])
{
	os::CommandLine Cmdline;
	Cmdline.Parse(argc, argv);

	if (argc == 1)
	{
		TestMain(0);
	}
	else if(std::wstring(argv[1]) == L"1")
	{		
		TestMain(1);		
	}
	else if (std::wstring(argv[1]) == L"2")
	{
		TestMain(2);
	}
	// _DumpMemoryAllocations;
	return 0;
}

