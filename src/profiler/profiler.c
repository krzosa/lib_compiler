#if defined(LC_ENABLE_PROFILER)
    #include "spall.h"
static SpallProfile spall_ctx;
static SpallBuffer  spall_buffer;

LC_FUNCTION void LC_BeginProfiling() {
    spall_ctx = spall_init_file("lc.spall", 1);

    int            buffer_size = 1 * 1024 * 1024 * 4;
    unsigned char *buffer      = (unsigned char *)malloc(buffer_size);

    spall_buffer.length = buffer_size;
    spall_buffer.data   = buffer;
    spall_buffer_init(&spall_ctx, &spall_buffer);
}

LC_FUNCTION void LC_EndProfiling() {
    spall_buffer_quit(&spall_ctx, &spall_buffer);
    free(spall_buffer.data);
    spall_quit(&spall_ctx);
}

    #if _WIN32
        #include <Windows.h>
double LC_GetTime(void) {
    static double invfreq;
    if (!invfreq) {
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        invfreq = 1000000.0 / frequency.QuadPart;
    }
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return counter.QuadPart * invfreq;
}
    #else
        #include <unistd.h>
double LC_GetTime(void) {
    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    return (((double)spec.tv_sec) * 1000000) + (((double)spec.tv_nsec) / 1000);
}
    #endif

struct LC_ProfileScope {
    LC_ProfileScope(const char *fn, int size) {
        spall_buffer_begin(&spall_ctx, &spall_buffer, fn, size - 1, LC_GetTime());
    }
    ~LC_ProfileScope() {
        spall_buffer_end(&spall_ctx, &spall_buffer, LC_GetTime());
    }
};

    #define LC_ProfileScope() \
        LC_ProfileScope TIME_SCOPE_(__FUNCTION__, sizeof(__FUNCTION__))
#else
    #define LC_ProfileScope()
LC_FUNCTION void LC_BeginProfiling() {
}
LC_FUNCTION void LC_EndProfiling() {
}
#endif