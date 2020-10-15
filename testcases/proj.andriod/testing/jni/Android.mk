LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := shared_testing
LOCAL_SRC_FILES :=	\
../../../../core/rt/xml_xhtml.cpp \
../../../../core/rt/json.cpp \
../../../../core/os/kernel.cpp \
../../../../core/os/kernal_encode.cpp \
../../../../core/os/kernel_log.cpp \
../../../../core/os/high_level.cpp \
../../../../core/os/file_zip.cpp \
../../../../core/os/file_dir.cpp \
../../../../core/os/multi_thread.cpp \
../../../../core/os/precompiler.cpp \
../../../../core/inet/inet.cpp \
../../../../core/inet/http_client.cpp \
../../../../core/inet/tinyhttpd.cpp \
../../../../core/inet/tinyhttpd_fileserv.cpp \
../../../../core/ext/botan/botan.cpp \
../../../../core/ext/lib_inc.c \
../../../../core/ext/exprtk/exprtk.cpp \
../../../../core/ext/rocksdb\rocksdb.cpp \
../../../../testcases/tests/entrypoint.cpp \
../../../../testcases/tests/test_api.cpp \
../../../../testcases/tests/test_ext.cpp \
../../../../testcases/tests/test_http.cpp \
../../../../testcases/tests/test_botan.cpp \
../../../../testcases/tests/test_perf.cpp \
test.c \

LOCAL_LDLIBS    := -lm -llog
LOCAL_CPP_FEATURES += exceptions 
LOCAL_CPP_FEATURES += rtti

LOCAL_CPPFLAGS += -std=c++14


APP_OPTIM := debug
LOCAL_CFLAGS := -D_DEBUG
LOCAL_CFLAGS += -D_GLIBCXX_PERMIT_BACKWARD_HASH
LOCAL_CFLAGS += -DPLATFORM_ANDROID
LOCAL_CFLAGS += -DPNG_ARM_NEON_OPT=0
LOCAL_CFLAGS += -DOS_ANDROID


include $(BUILD_SHARED_LIBRARY)


