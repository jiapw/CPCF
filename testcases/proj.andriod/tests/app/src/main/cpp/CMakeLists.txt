# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPLATFORM_ANDROID -DPNG_ARM_NEON_OPT=0 -DOS_ANDROID")

add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             native-lib.cpp
             ../../../../../../../essentials.cpp
             ../../../../../../../testcases/tests/entrypoint.cpp
             ../../../../../../../testcases/tests/test_api.cpp
             ../../../../../../../testcases/tests/test_ext.cpp
             ../../../../../../../testcases/tests/test_botan.cpp
             ../../../../../../../testcases/tests/test_perf.cpp
             ../../../../../../../testcases/tests/test_http.cpp
             ../../../../../../../core/rt/xml_xhtml.cpp
             ../../../../../../../core/os/precompiler.cpp
             ../../../../../../../core/os/high_level.cpp
             ../../../../../../../core/os/file_zip.cpp
             ../../../../../../../core/ext/lib_inc.c
             ../../../../../../../core/ext/botan/botan.cpp
             ../../../../../../../core/ext/exprtk/exprtk.cpp
             ../../../../../../../core/ext/rocksdb/rocksdb.cpp
             ../../../../../../../core/ext/rocksdb/rocksdb_serving.cpp
             #../../../../../../../core/ext/exprtk/exprtk.cpp
             ../../../../../../../core/inet/tinyhttpd.cpp
             ../../../../../../../core/inet/tinyhttpd_fileserv.cpp
             ../../../../../../../core/inet/http_client.cpp

        )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       native-lib

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )