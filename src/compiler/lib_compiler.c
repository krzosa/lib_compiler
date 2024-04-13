#include "lib_compiler.h"

#if __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wswitch"
    #pragma clang diagnostic ignored "-Wwritable-strings"
#endif

#ifndef LC_ParseFloat // @override
    #include <stdlib.h>
    #define LC_ParseFloat(str, len) strtod(str, NULL)
#endif

#ifndef LC_Print // @override
    #include <stdio.h>
    #define LC_Print(str, len) printf("%.*s", (int)len, str)
#endif

#ifndef LC_Exit // @override
    #include <stdio.h>
    #define LC_Exit(x) exit(x)
#endif

#ifndef LC_MemoryZero // @override
    #include <string.h>
    #define LC_MemoryZero(p, size) memset(p, 0, size)
#endif

#ifndef LC_MemoryCopy // @override
    #include <string.h>
    #define LC_MemoryCopy(dst, src, size) memcpy(dst, src, size);
#endif

#ifndef LC_vsnprintf // @override
    #include <stdio.h>
    #define LC_vsnprintf vsnprintf
#endif

#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#endif

LC_THREAD_LOCAL LC_Lang *L;

#include "unicode.c"
#include "string.c"
#include "to_string.c"
#include "common.c"
#include "intern.c"
#include "lex.c"
#include "bigint.c"
#include "value.c"
#include "ast.c"
#include "ast_walk.c"
#include "ast_copy.c"
#include "resolver.c"
#include "resolve.c"
#include "parse.c"
#include "printer.c"
#include "genc.c"
#include "extended_passes.c"
#include "packages.c"
#include "init.c"

#if _WIN32
    #include "win32_filesystem.c"
#elif __linux__ || __APPLE__ || __unix__
    #include <sys/stat.h>
    #include <unistd.h>
    #include <limits.h>
    #include <time.h>
    #include <dirent.h>
    #include <sys/mman.h>

    #include "unix_filesystem.c"
#endif

#ifndef LC_USE_CUSTOM_ARENA
    #include "arena.c"
    #if _WIN32
        #include "win32_arena.c"
    #elif __linux__ || __APPLE__ || __unix__
        #include "unix_arena.c"
    #endif
#endif

#if __clang__
    #pragma clang diagnostic pop
#endif
