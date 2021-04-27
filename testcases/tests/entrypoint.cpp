#include "test.h"

void exp_tracking_proc_ip();
#pragma comment(lib,"Mpr.lib")


#if defined (PLATFORM_ANDROID) || defined (PLATFORM_IOS)
extern "C"
#endif
void TestMain()
{
	rt::String out;
	os::File::GetCurrentDirectory(out);
	_LOGC(out);
	
#if defined (PLATFORM_ANDROID) || defined (PLATFORM_IOS)
	{   rt::String cd;
		os::GetAppSandbox(cd, nullptr);
		os::File::SetCurrentDirectory(cd);
	}
#endif

	TYPETRAITS_UNITTEST_OUTPUT("../testlogs/");

	if(!os::CommandLine::Get().HasOption("verify"))
	{
		TYPETRAITS_UNITTEST(big_num);
//		TYPETRAITS_UNITTEST(crypto_func);
		//TYPETRAITS_UNITTEST(recv_pump);
		return;
	}
	else
	{
		TYPETRAITS_UNITTEST(rt);
		TYPETRAITS_UNITTEST(buffer);
		TYPETRAITS_UNITTEST(sortedpush);
		TYPETRAITS_UNITTEST(string_conv);
		TYPETRAITS_UNITTEST(string);
		TYPETRAITS_UNITTEST(encoding);
		TYPETRAITS_UNITTEST(json);
		TYPETRAITS_UNITTEST(express_tk);
		TYPETRAITS_UNITTEST(xml);
		//TYPETRAITS_UNITTEST(html);
		TYPETRAITS_UNITTEST(precompiler);
		
		TYPETRAITS_UNITTEST(timedate);
		TYPETRAITS_UNITTEST(smallmath);
		TYPETRAITS_UNITTEST(multithread);
		TYPETRAITS_UNITTEST(b64_encoding);
		TYPETRAITS_UNITTEST(binary_search);
		TYPETRAITS_UNITTEST(botan_cipher);
		TYPETRAITS_UNITTEST(botan_hash);
		TYPETRAITS_UNITTEST(sparsehash);
		TYPETRAITS_UNITTEST(async_queue);
		TYPETRAITS_UNITTEST(crypto_func);
		TYPETRAITS_UNITTEST(file);
		TYPETRAITS_UNITTEST(rocks_db);
		TYPETRAITS_UNITTEST(vm);
		TYPETRAITS_UNITTEST(async_write);
		TYPETRAITS_UNITTEST(recv_pump);
	
		if(0) // non-static test
		{
			TYPETRAITS_UNITTEST(filelist);
			TYPETRAITS_UNITTEST(pfw);
			TYPETRAITS_UNITTEST(sysinfo);
			TYPETRAITS_UNITTEST(plog);

			//TYPETRAITS_UNITTEST(commandline();
			//TYPETRAITS_UNITTEST(socket();
			//TYPETRAITS_UNITTEST(sockettimed();
			//TYPETRAITS_UNITTEST(delayed_deletion();
			//TYPETRAITS_UNITTEST(socket_io);
			//TYPETRAITS_UNITTEST(socket_io_recv);

			//TYPETRAITS_UNITTEST(http_client);
			//TYPETRAITS_UNITTEST(download);

			//TYPETRAITS_UNITTEST(http_nav);
			//TYPETRAITS_UNITTEST(httpd);
	
			//TYPETRAITS_UNITTEST(ipp_canvas);
			//TYPETRAITS_UNITTEST(ipp_matting);
			//TYPETRAITS_UNITTEST(ipp_imageproc);
			//TYPETRAITS_UNITTEST(ipp_image);
			//TYPETRAITS_UNITTEST(ipp_image_apps);
			//TYPETRAITS_UNITTEST(ipp_zlib);
			//TYPETRAITS_UNITTEST(ipp_zip);
			//TYPETRAITS_UNITTEST(mkl_vector);
		}
	}
}
