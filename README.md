# CPCF

It is a C++ library provides primitives for building cross-platform native systems and applications. It is refactored from a collection of codes accumulated when I was doing research on Computer Graphics/Vision and Distributed Systems in ICT/CAS, MSRA and MSR since 2002. 

The library favors performance (more speed, less resource) over simplicity and maintainability, use with caution. It is designed for all major platforms: Windows, Linux, MacOSX and mobile platforms iOS and Andriod. Only three desktop platforms are intensively tested. iOS should be fine, Andriod is not tested for a long time. 


### Major Components

##### Data Manipulation
* `rt::TypeTraits` Extensible type traits framework
* `rt::String`, `rt::String_Ref` Memory efficient strings operations and expression (based on string without trailing NULL)
* `rt::JsonObject`, `J()` Memory efficient simply Json parser and composer
* `rt::XMLParser`, `rt::XMLComposer` XML parser and composer support xpath expression
* `rt::Buffer` Simple linear container with assuming *move* syntax
* `rt::Vec<>` Small math vector for numeric operations
* `rt::Mat3x3`, `rt::Mat4x4` Small math matrix for numeric operations

##### Advanced Data Manipulation
* `ipp::Image<>` High performance image processing operations for both LDR and HDR
* `ipp::ImageDecoder`,`ipp::ImageEncoder` Image codec supports GIF/PNG/JPG/PFM/EXR

##### OS Primitives
* `_LOG(x)`, `_LOG_ERROR(x)`,`_LOG_HIGHLIGHT(x)`,etc. Runtime logging solution for various IDEs and devices
* `os::File` File and Directory manipulation
* `os::UTF8Encode`,`os::UTF8Decode` Multilingual string support
* `rt::tos::Base64`, `rt::tos::Base32`, `rt::tos::Base16`, `os::UrlEncode`,`os::UrlDecode` Binary data encoding
* `os::Timestamp` High precision time and date representation
* `os::HighPerformanceCounter` High precision timing
* `os::Thread` Simple thread creation and control
* `os::CriticalSection`, `os::Event`, `os::AtomicXXX` Concurrency primitives
* `os::ParallelWriter<>` Multiple writer and single reader scheme

##### Networking
* `inet::Socket` basic communication with sync/async socket
* `inet::TinyHttpd` Light-weight full feature web container
* `inet::HttpServerFiles`, `inet::HttpVirtualPath` Light-weight static web server
* `inet::HttpSession` HTTP connection supports HTTP HTTPS and Proxy
* `inet::HttpNavigator` Web client supports redirection, designed for crawler
* `inet::HttpDownloader` High performance concurrent downloading with scheduler

##### Reusable Modules
* `os::LaunchProcess` Process creation, control and I/O pipe
* `os::ParallelFileWriter` Non-blocking multiple writer on files, designed for high performance logging
* `os::Precompiler` A compiler for code preprocessing on multiple files, macro define and substitute, file include and macro control `#ifdef/#ifndef/#elif/#else/#endif/#if defined(...)`
* `os::UserInputEventPump` Abstraction for user input of keyboard, touch and mouse for desktop and mobile platforms

##### Real-time 3D Rendering
* `gl::RenderContext` OpenGL initialization for desktop and mobile platforms
* `gl::ShaderProgram`, `gl::ShaderCode` OpenGL shading language support
* `gl::ShaderSourceCodeLibrary` Runtime shader compilation and management 
* `gl::Object` Geometry representation and surface properties
* `gl::ArcBall<>` Intuitive user interactive for scene/object rotation
* `gl::gdiCanvas` 2D graphics drawing interface (text, image, line, rectangle and polygon)
* `gl::gdiFont` Bitmap font representation for `gl::gdiCanvas`

### Dependency
The library is completely self-contained. It embedded copies of other projects and credited below.

CPCF embeds the Intel IPP 2021.1. & MKL 2021.3 (Static linking libraries) for image processing, cryptography and advanced numeric operations. **Download static library package and follow instructions** at https://github.com/jiapw/CPCF/releases/tag/CPCF-Libs

#### External Projects Embedded
* Intel IPP: https://software.intel.com/en-us/intel-ipp
* LZMA SDK: https://www.7-zip.org/sdk.html
* Botan: https://botan.randombit.net/
* exprtk: http://www.partow.net/programming/exprtk/index.html
* libgif: http://giflib.sourceforge.net/
* libpng: http://www.libpng.org/pub/png/libpng.html
* libjpeg: http://libjpeg.sourceforge.net/
* OpenEXR: http://www.openexr.com/index.html
* RocksDB: https://rocksdb.org/
* Snappy: https://github.com/google/snappy
* zlib: https://zlib.net/
* concurrentqueue: https://github.com/cameron314/concurrentqueue
* Google sparsehash: http://goog-sparsehash.sourceforge.net/
