#pragma once
#include "../../essentials.h"

namespace rt
{

struct UnitTests
{
	static void encoding();
	static void string();
	static void string_conv();
	static void rt();
	static void buffer();
	static void xml();
	static void html();
	static void json();
	static void file();
	static void async_write();
	static void timedate();
	static void http_client();
	static void http_nav();
	static void socket();
	static void pfw();
	static void sockettimed();
	static void multithread();
	static void download();
	static void b64_encoding();
	static void delayed_deletion();
	static void sysinfo();
	static void socket_io();
	static void socket_socket_event();
	static void recv_pump();
	static void net_interfaces();
	static void filelist();
	static void smallmath();
	static void vm();
	static void binary_search();
	static void plog();
	static void precompiler();
	static void commandline();
	static void sortedpush();
	static void sparsehash();
	static void async_queue();
	static void crypto_func();
	static void rocks_db();
	static void httpd();
	static void rocksdb_serve();
	static void big_num();

	static void botan_hash();
	static void botan_cipher();
	static void ipp_zlib();
	static void ipp_canvas();
	static void ipp_image();
	static void ipp_major_color();
	static void express_tk();
	static void ipp_zip();

	static void callback_to_member_function();

	static void image_to_text();

	#if defined(PLATFORM_INTEL_IPP_SUPPORT)
	static void ipp_matting();
	static void ipp_image_apps();
	static void ipp_imageproc();
	#endif

	#if defined(PLATFORM_INTEL_MKL_SUPPORT)
	static void mkl_vector();
	static void mkl_linequ();
	#endif
};

} // namespace rt


