#ifndef FIRST_IO_HEADER
#define FIRST_IO_HEADER
#include <stdbool.h>

#ifndef IO_API
    #ifdef __cplusplus
        #define IO_API extern "C"
    #else
        #define IO_API
    #endif
#endif

typedef enum IO_ErrorResult {
    IO_ErrorResult_Continue,
    IO_ErrorResult_Break,
    IO_ErrorResult_Exit,
} IO_ErrorResult;

#if defined(_MSC_VER)
    #define IO_DebugBreak() (__debugbreak(), 0)
#else
    #define IO_DebugBreak() (__builtin_trap(), 0)
#endif

#ifndef IO_THREAD_LOCAL
    #if defined(__cplusplus) && __cplusplus >= 201103L
        #define IO_THREAD_LOCAL thread_local
    #elif defined(__GNUC__)
        #define IO_THREAD_LOCAL __thread
    #elif defined(_MSC_VER)
        #define IO_THREAD_LOCAL __declspec(thread)
    #elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
        #define IO_THREAD_LOCAL _Thread_local
    #elif defined(__TINYC__)
        #define IO_THREAD_LOCAL _Thread_local
    #else
        #error Couldnt figure out thread local, needs to be provided manually
    #endif
#endif

#if defined(__has_attribute)
    #if __has_attribute(format)
        #define IO__PrintfFormat(fmt, va) __attribute__((format(printf, fmt, va)))
    #endif
#endif

#ifndef IO__PrintfFormat
    #define IO__PrintfFormat(fmt, va)
#endif

typedef void IO_MessageHandler(int kind, const char *file, int line, char *str, int len);
extern IO_THREAD_LOCAL void (*IO_User_OutputMessage)(int kind, const char *file, int line, char *str, int len);

#define IO__STRINGIFY(x) #x
#define IO__TOSTRING(x) IO__STRINGIFY(x)
#define IO_LINE IO__TOSTRING(__LINE__)

#define IO_Assert(x) !(x) && IO__FatalError((__FILE__ "(" IO_LINE "): "      \
                                                      "error: " #x "\n")) && \
                         IO_DebugBreak()
#define IO_FatalErrorf(...)                                             \
    do {                                                                \
        bool result = IO__FatalErrorf(__FILE__, __LINE__, __VA_ARGS__); \
        if (result) IO_DebugBreak();                                    \
    } while (0)
#define IO_FatalError(...)                                                            \
    do {                                                                              \
        bool result = IO__FatalError(__FILE__ "(" IO_LINE "): error - " __VA_ARGS__); \
        if (result) IO_DebugBreak();                                                  \
    } while (0)
#define IO_Assertf(x, ...)                                                  \
    do {                                                                    \
        if (!(x)) {                                                         \
            bool result = IO__FatalErrorf(__FILE__, __LINE__, __VA_ARGS__); \
            if (result) IO_DebugBreak();                                    \
        }                                                                   \
    } while (0)

#define IO_InvalidElseIf(c)   \
    else if (c) {             \
        IO_InvalidCodepath(); \
    }
#define IO_InvalidElse()      \
    else {                    \
        IO_InvalidCodepath(); \
    }
#define IO_InvalidCodepath() IO_FatalError("This codepath is invalid")
#define IO_InvalidDefaultCase()                             \
default: {                                                  \
    IO_FatalError("Entered invalid switch statement case"); \
}
#define IO_Todo() IO_FatalError("This codepath is not implemented yet")

IO_API bool           IO__FatalErrorf(const char *file, int line, const char *msg, ...) IO__PrintfFormat(3, 4);
IO_API void           IO__Printf(int kind, const char *file, int line, const char *msg, ...) IO__PrintfFormat(4, 5);
IO_API bool           IO__FatalError(const char *msg);
IO_API void           IO_Print(int kind, const char *file, int line, char *msg, int len);
IO_API void           IO_OutputMessage(char *str, int len);
IO_API IO_ErrorResult IO_OutputError(char *str, int len);
IO_API void           IO_Exit(int error_code);
IO_API bool           IO_IsDebuggerPresent(void);

static const int IO_KindPrintf   = 1;
static const int IO_KindWarningf = 2;

#define IO_Printf(...) IO__Printf(IO_KindPrintf, __FILE__, __LINE__, __VA_ARGS__)
#define IO_Warningf(...) IO__Printf(IO_KindWarningf, __FILE__, __LINE__, __VA_ARGS__)
#endif