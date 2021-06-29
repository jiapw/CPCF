#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include "../../core/os/file_dir.h"
#include "../../core/os/kernel.h"

#include <string>
void TestMain();

int _tmain(int argc, _TCHAR* argv[])
{
	os::CommandLine Cmdline;
	Cmdline.Parse(argc, argv);
	TestMain();
	
	// _DumpMemoryAllocations;
	return 0;
}

