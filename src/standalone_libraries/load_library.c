#include "load_library.h"
#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>

LIB_Library LIB_LoadLibrary(char *str) {
    HMODULE module = LoadLibraryA(str);
    return (LIB_Library)module;
}

void *LIB_LoadSymbol(LIB_Library lib, char *symbol) {
    void *result = (void *)GetProcAddress((HMODULE)lib, symbol);
    return result;
}

bool LIB_UnloadLibrary(LIB_Library lib) {
    BOOL result = FreeLibrary((HMODULE)lib);
    if (result == 0) return false;
    return true;
}
#endif // _WIN32
