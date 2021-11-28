#pragma once

#ifdef NDEBUG
#  define SH_DEBUG 0
#  define SH_RELEASE 1
#else
#  define SH_DEBUG 1
#  define SH_RELEASE 0
#endif

#ifdef _MSC_VER
#  define SH_CC_MSVC 1
#else
#  define SH_CC_MSVC 0
#endif

#if defined(__GNUC__) && !SH_CC_CLANG
#  define SH_CC_GCC 1
#else
#  define SH_CC_GCC 0
#endif

#ifdef __clang__
#  define SH_CC_CLANG 1
#else
#  define SH_CC_CLANG 0
#endif

#ifdef __EMSCRIPTEN__
#  define SH_CC_EMSCRIPTEN 1
#else
#  define SH_CC_EMSCRIPTEN 0
#endif

#ifdef __MINGW32__
#  define SH_CC_MINGW 1
#else
#  define SH_CC_MINGW 0
#endif

#ifdef _WIN32
#  define SH_OS_WINDOWS 1
#else
#  define SH_OS_WINDOWS 0
#endif

#ifdef __APPLE__
#  define SH_OS_MACOS 1
#else
#  define SH_OS_MACOS 0
#endif

#ifdef __linux__
#  define SH_OS_LINUX 1
#else
#  define SH_OS_LINUX 0
#endif

#ifdef __unix__
#  define SH_OS_UNIX 1
#else
#  define SH_OS_UNIX 0
#endif

#ifdef __FreeBSD_version
#  define SH_OS_BSD_FREE 1
#else
#  define SH_OS_BSD_FREE 0
#endif

#ifdef __NetBSD__
#  define SH_OS_BSD_NET 1
#else
#  define SH_OS_BSD_NET 0
#endif

#ifdef __DragonFly__
#  define SH_OS_BSD_DRAGONFLY 1
#else
#  define SH_OS_BSD_DRAGONFLY 0
#endif
