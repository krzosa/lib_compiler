#ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
#endif

#include "../core/core.c"

#define CL_Allocator MA_Arena *
#define CL_Allocate(a, s) MA_PushSizeNonZeroed(a, s)
#define CL_ASSERT IO_Assert
#define CL_VSNPRINTF stbsp_vsnprintf
#define CL_SNPRINTF stbsp_snprintf
#define AND_CL_STRING_TERMINATE_ON_NEW_LINE
#include "../standalone_libraries/clexer.c"

thread_local MA_Arena PernamentArena;
thread_local MA_Arena *Perm = &PernamentArena;

#include "cache.cpp"
#include "easy_strings.cpp"
#include "process.cpp"

S8_String CL_Flags = "/MP /Zi /FC /WX /W3 /wd4200 /diagnostics:column /nologo -D_CRT_SECURE_NO_WARNINGS /GF /Gm- /Oi";
S8_String CL_Link = "/link /incremental:no";
S8_String CL_StdOff = "/GR- /EHa-";
S8_String CL_StdOn = "/EHsc";
S8_String CL_Debug = "-Od -D_DEBUG -fsanitize=address -RTC1";
S8_String CL_Release = "-O2 -MT -DNDEBUG -GL";
S8_String CL_ReleaseLink = "-opt:ref -opt:icf";
/*
/FC      = Print full paths in diagnostics
/Gm-     = Old feature, 'minimal compilation', in case it's not off by default
/GF      = Pools strings as read-only. If you try to modify strings under /GF, an application error occurs.
/Oi      = Replaces some function calls with intrinsic
/MP      = Multithreaded compilation
/GR-     = Disable runtime type information
/EHa-    = Disable exceptions
/EHsc    = Enable exceptions
/MT      = Link static libc. The 'd' means debug version
/MD      = Link dynamic libc. The 'd' means debug version
/GL      = Whole program optimization
/RTC1    = runtime error checks
/opt:ref = eliminates functions and data that are never referenced
/opt:icf = eliminates redundant 'COMDAT's
*/

S8_String Clang_Flags = "-fdiagnostics-absolute-paths -Wno-writable-strings";
S8_String Clang_NoStd = "-fno-exceptions";
S8_String Clang_Debug = "-fsanitize=address -g";
/*
-std=c++11
 */

S8_String GCC_Flags = "-Wno-write-strings";
S8_String GCC_NoStd = "-fno-exceptions";
S8_String GCC_Debug = "-fsanitize=address -g";
