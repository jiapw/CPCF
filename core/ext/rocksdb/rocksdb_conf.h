#pragma once

#include "../../os/platform.h"
#include <functional>

#define ROCKSDB_LITE 1
#define ROCKSDB_PORTABLE 1

#if defined(PLATFORM_WIN)
    #define OS_WIN 1
#elif defined(PLATFORM_MAC) || defined(PLATFORM_IOS)
    #define OS_MACOSX 1
    #define ROCKSDB_PLATFORM_POSIX 1
#else
    #define ROCKSDB_PLATFORM_POSIX 1
#endif

#if defined(ROCKSDB_PLATFORM_POSIX)
    #define ROCKSDB_LIB_IO_POSIX 1
#endif

