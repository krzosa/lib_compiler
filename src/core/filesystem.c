#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
    #include <stdio.h>
    #include <stdlib.h>

OS_API bool OS_EnableTerminalColors(void) {
    // Enable color terminal output
    {
        // Set output mode to handle virtual terminal sequences
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE) {
            DWORD dwMode = 0;
            if (GetConsoleMode(hOut, &dwMode)) {
                dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                if (SetConsoleMode(hOut, dwMode)) {
                    return true;
                } else {
                    IO_Printf("Failed to enable colored terminal output C\n");
                }
            } else {
                IO_Printf("Failed to enable colored terminal output B\n");
            }
        } else {
            IO_Printf("Failed to enable colored terminal output A\n");
        }
    }
    return false;
}

OS_API bool OS_IsAbsolute(S8_String path) {
    bool result = path.len > 3 && CHAR_IsAlphabetic(path.str[0]) && path.str[1] == ':' && path.str[2] == '/';
    return result;
}

OS_API S8_String OS_GetExePath(MA_Arena *arena) {
    wchar_t wbuffer[1024];
    DWORD   wsize = GetModuleFileNameW(0, wbuffer, MA_Lengthof(wbuffer));
    IO_Assert(wsize != 0);

    S8_String path = S8_FromWidecharEx(arena, wbuffer, wsize);
    S8_NormalizePathUnsafe(path);
    return path;
}

OS_API S8_String OS_GetExeDir(MA_Arena *arena) {
    MA_Temp   scratch = MA_GetScratch();
    S8_String path    = OS_GetExePath(scratch.arena);
    path              = S8_ChopLastSlash(path);
    path              = S8_Copy(arena, path);
    MA_ReleaseScratch(scratch);
    return path;
}

OS_API S8_String OS_GetWorkingDir(MA_Arena *arena) {
    wchar_t wbuffer[1024];
    DWORD   wsize = GetCurrentDirectoryW(MA_Lengthof(wbuffer), wbuffer);
    IO_Assert(wsize != 0);
    IO_Assert(wsize < 1022);
    wbuffer[wsize++] = '/';
    wbuffer[wsize]   = 0;

    S8_String path = S8_FromWidecharEx(arena, wbuffer, wsize);
    S8_NormalizePathUnsafe(path);
    return path;
}

OS_API void OS_SetWorkingDir(S8_String path) {
    wchar_t wpath[1024];
    UTF_CreateWidecharFromChar(wpath, MA_Lengthof(wpath), path.str, path.len);
    SetCurrentDirectoryW(wpath);
}

OS_API S8_String OS_GetAbsolutePath(MA_Arena *arena, S8_String relative) {
    wchar_t wpath[1024];
    UTF_CreateWidecharFromChar(wpath, MA_Lengthof(wpath), relative.str, relative.len);
    wchar_t wpath_abs[1024];
    DWORD   written = GetFullPathNameW((wchar_t *)wpath, MA_Lengthof(wpath_abs), wpath_abs, 0);
    if (written == 0)
        return S8_MakeEmpty();
    S8_String path = S8_FromWidecharEx(arena, wpath_abs, written);
    S8_NormalizePathUnsafe(path);
    return path;
}

OS_API bool OS_FileExists(S8_String path) {
    wchar_t wbuff[1024];
    UTF_CreateWidecharFromChar(wbuff, MA_Lengthof(wbuff), path.str, path.len);
    DWORD attribs = GetFileAttributesW(wbuff);
    bool  result  = attribs == INVALID_FILE_ATTRIBUTES ? false : true;
    return result;
}

OS_API bool OS_IsDir(S8_String path) {
    wchar_t wbuff[1024];
    UTF_CreateWidecharFromChar(wbuff, MA_Lengthof(wbuff), path.str, path.len);
    DWORD dwAttrib = GetFileAttributesW(wbuff);
    return dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

OS_API bool OS_IsFile(S8_String path) {
    wchar_t wbuff[1024];
    UTF_CreateWidecharFromChar(wbuff, MA_Lengthof(wbuff), path.str, path.len);
    DWORD dwAttrib = GetFileAttributesW(wbuff);
    bool  is_file  = (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) == 0;
    return dwAttrib != INVALID_FILE_ATTRIBUTES && is_file;
}

OS_API double OS_GetTime(void) {
    static int64_t counts_per_second;
    if (counts_per_second == 0) {
        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);
        counts_per_second = freq.QuadPart;
    }

    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);
    double result = (double)time.QuadPart / (double)counts_per_second;
    return result;
}

/*
User needs to copy particular filename to keep it.

for (OS_FileIter it = OS_IterateFiles(it); OS_IsValid(iter); OS_Advance(it)) {
}

*/

typedef struct OS_Win32_FileIter {
    HANDLE           handle;
    WIN32_FIND_DATAW data;
} OS_Win32_FileIter;

OS_API bool OS_IsValid(OS_FileIter it) {
    return it.is_valid;
}

OS_API void OS_Advance(OS_FileIter *it) {
    while (FindNextFileW(it->w32->handle, &it->w32->data) != 0) {
        WIN32_FIND_DATAW *data = &it->w32->data;

        // Skip '.' and '..'
        if (data->cFileName[0] == '.' && data->cFileName[1] == '.' && data->cFileName[2] == 0) continue;
        if (data->cFileName[0] == '.' && data->cFileName[1] == 0) continue;

        it->is_directory      = data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
        it->filename          = S8_FromWidecharEx(it->arena, data->cFileName, S8_WideLength(data->cFileName));
        const char *is_dir    = it->is_directory ? "/" : "";
        const char *separator = it->path.str[it->path.len - 1] == '/' ? "" : "/";
        it->relative_path     = S8_Format(it->arena, "%.*s%s%.*s%s", S8_Expand(it->path), separator, S8_Expand(it->filename), is_dir);
        it->absolute_path     = OS_GetAbsolutePath(it->arena, it->relative_path);
        it->is_valid          = true;

        if (it->is_directory) {
            IO_Assert(it->relative_path.str[it->relative_path.len - 1] == '/');
            IO_Assert(it->absolute_path.str[it->absolute_path.len - 1] == '/');
        }
        return;
    }

    it->is_valid = false;
    DWORD error  = GetLastError();
    IO_Assert(error == ERROR_NO_MORE_FILES);
    FindClose(it->w32->handle);
}

OS_API OS_FileIter OS_IterateFiles(MA_Arena *scratch_arena, S8_String path) {
    OS_FileIter it = {0};
    it.arena       = scratch_arena;
    it.path        = path;

    S8_String modified_path = S8_Format(it.arena, "%.*s\\*", S8_Expand(path));
    wchar_t  *wbuff         = MA_PushArray(it.arena, wchar_t, modified_path.len + 1);
    int64_t   wsize         = UTF_CreateWidecharFromChar(wbuff, modified_path.len + 1, modified_path.str, modified_path.len);
    IO_Assert(wsize);

    it.w32         = MA_PushStruct(it.arena, OS_Win32_FileIter);
    it.w32->handle = FindFirstFileW(wbuff, &it.w32->data);
    if (it.w32->handle == INVALID_HANDLE_VALUE) {
        it.is_valid = false;
        return it;
    }

    IO_Assert(it.w32->data.cFileName[0] == '.' && it.w32->data.cFileName[1] == 0);
    OS_Advance(&it);
    return it;
}

OS_API OS_Result OS_MakeDir(S8_String path) {
    wchar_t wpath[1024];
    UTF_CreateWidecharFromChar(wpath, MA_Lengthof(wpath), path.str, path.len);
    BOOL      success = CreateDirectoryW(wpath, NULL);
    OS_Result result  = OS_SUCCESS;
    if (success == 0) {
        DWORD error = GetLastError();
        if (error == ERROR_ALREADY_EXISTS) {
            result = OS_ALREADY_EXISTS;
        } else if (error == ERROR_PATH_NOT_FOUND) {
            result = OS_PATH_NOT_FOUND;
        } else {
            IO_Assert(0);
        }
    }
    return result;
}

OS_API OS_Result OS_CopyFile(S8_String from, S8_String to, bool overwrite) {
    wchar_t wfrom[1024];
    UTF_CreateWidecharFromChar(wfrom, MA_Lengthof(wfrom), from.str, from.len);

    wchar_t wto[1024];
    UTF_CreateWidecharFromChar(wto, MA_Lengthof(wto), to.str, to.len);

    BOOL fail_if_exists = !overwrite;
    BOOL success        = CopyFileW(wfrom, wto, fail_if_exists);

    OS_Result result = OS_SUCCESS;
    if (success == FALSE)
        result = OS_FAILURE;
    return result;
}

OS_API OS_Result OS_DeleteFile(S8_String path) {
    wchar_t wpath[1024];
    UTF_CreateWidecharFromChar(wpath, MA_Lengthof(wpath), path.str, path.len);
    BOOL      success = DeleteFileW(wpath);
    OS_Result result  = OS_SUCCESS;
    if (success == 0)
        result = OS_PATH_NOT_FOUND;
    return result;
}

OS_API OS_Result OS_DeleteDir(S8_String path, unsigned flags) {
    IO_Todo();
    return OS_FAILURE;
    #if 0
    if (flags & OS_RECURSIVE) {
        MA_Temp scratch = MA_GetScratch();
        S8_List list = OS_ListDir(scratch.arena, path, OS_RECURSIVE);
        S8_Node *dirs_to_remove = 0;
        for (S8_Node *it = list.first; it; it = it->next) {
            if (!S8_EndsWith(it->string, S8_Lit("/"), S8_IgnoreCase)) {
                OS_DeleteFile(it->string);
            }
            else {
                S8_Node *node = S8_CreateNode(scratch.arena, it->string);
                SLL_STACK_ADD(dirs_to_remove, node);
            }
        }
        for (S8_Node *it = dirs_to_remove; it; it = it->next) {
            OS_DeleteDir(it->string, OS_NO_FLAGS);
        }
        OS_Result result = OS_DeleteDir(path, OS_NO_FLAGS);
        MA_ReleaseScratch(scratch);
        return result;
    }
    else {
        wchar_t wpath[1024];
        UTF_CreateWidecharFromChar(wpath, MA_Lengthof(wpath), path.str, path.len);
        BOOL success = RemoveDirectoryW(wpath);
        OS_Result result = OS_SUCCESS;
        if (success == 0)
            result = OS_PATH_NOT_FOUND;
        return result;
    }
    #endif
}

static OS_Result OS__WriteFile(S8_String path, S8_String data, bool append) {
    wchar_t wpath[1024];
    UTF_CreateWidecharFromChar(wpath, MA_Lengthof(wpath), path.str, path.len);
    OS_Result result = OS_FAILURE;

    DWORD access               = GENERIC_WRITE;
    DWORD creation_disposition = CREATE_ALWAYS;
    if (append) {
        access               = FILE_APPEND_DATA;
        creation_disposition = OPEN_ALWAYS;
    }

    HANDLE handle = CreateFileW(wpath, access, 0, NULL, creation_disposition, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle != INVALID_HANDLE_VALUE) {
        DWORD bytes_written = 0;
        IO_Assert(data.len == (DWORD)data.len); // @Todo: can only read 32 byte size files?
        BOOL error = WriteFile(handle, data.str, (DWORD)data.len, &bytes_written, NULL);
        if (error == TRUE) {
            if (bytes_written == data.len) {
                result = OS_SUCCESS;
            }
        }
        CloseHandle(handle);
    } else result = OS_PATH_NOT_FOUND;

    return result;
}

OS_API OS_Result OS_AppendFile(S8_String path, S8_String string) {
    return OS__WriteFile(path, string, true);
}

OS_API OS_Result OS_WriteFile(S8_String path, S8_String string) {
    return OS__WriteFile(path, string, false);
}

OS_API S8_String OS_ReadFile(MA_Arena *arena, S8_String path) {
    bool      success    = false;
    S8_String result     = S8_MakeEmpty();
    MA_Temp   checkpoint = MA_BeginTemp(arena);

    wchar_t wpath[1024];
    UTF_CreateWidecharFromChar(wpath, MA_Lengthof(wpath), path.str, path.len);
    HANDLE handle = CreateFileW(wpath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER file_size;
        if (GetFileSizeEx(handle, &file_size)) {
            if (file_size.QuadPart != 0) {
                result.len = (int64_t)file_size.QuadPart;
                result.str = (char *)MA_PushSizeNonZeroed(arena, result.len + 1);
                DWORD read;
                if (ReadFile(handle, result.str, (DWORD)result.len, &read, NULL)) { // @todo: can only read 32 byte size files?
                    if (read == result.len) {
                        success                = true;
                        result.str[result.len] = 0;
                    }
                }
            }
        }
        CloseHandle(handle);
    }

    if (!success) {
        result = S8_MakeEmpty();
        MA_EndTemp(checkpoint);
    }

    return result;
}

OS_API int64_t OS_GetFileModTime(S8_String file) {
    FILETIME         time = {0};
    WIN32_FIND_DATAW data;

    wchar_t wpath[1024];
    UTF_CreateWidecharFromChar(wpath, 1024, file.str, file.len);
    HANDLE handle = FindFirstFileW(wpath, &data);
    if (handle != INVALID_HANDLE_VALUE) {
        FindClose(handle);
        time = data.ftLastWriteTime;
    } else {
        return -1;
    }
    int64_t result = (int64_t)time.dwHighDateTime << 32 | time.dwLowDateTime;
    return result;
}

OS_API OS_Date OS_GetDate(void) {
    SYSTEMTIME local;
    GetLocalTime(&local);

    OS_Date result = {0};
    result.year    = local.wYear;
    result.month   = local.wMonth;
    result.day     = local.wDay;
    result.hour    = local.wHour;
    result.second  = local.wSecond;
    // result.milliseconds = local.wMilliseconds;
    return result;
}

#elif __linux__ || __APPLE__ || __unix__
    #include <unistd.h>
    #include <limits.h>
    #include <sys/stat.h>
    #include <time.h>
    #include <dirent.h>

    #if OS_MAC
        #include <mach-o/dyld.h>

OS_API S8_String OS_GetExePath(MA_Arena *arena) {
    char     buf[PATH_MAX];
    uint32_t bufsize = PATH_MAX;
    if (_NSGetExecutablePath(buf, &bufsize)) {
        return S8_MakeEmpty();
    }

    S8_String result = S8_Copy(arena, S8_MakeFromChar(buf));
    return result;
}

    #else

OS_API S8_String OS_GetExePath(MA_Arena *arena) {
    char buffer[PATH_MAX] = {};
    if (readlink("/proc/self/exe", buffer, PATH_MAX) == -1) {
        return S8_MakeEmpty();
    }
    S8_String result = S8_Copy(arena, S8_MakeFromChar(buffer));
    return result;
}

    #endif

OS_API bool OS_EnableTerminalColors(void) { return true; }

OS_API bool OS_IsAbsolute(S8_String path) {
    bool result = path.len >= 1 && path.str[0] == '/';
    return result;
}

OS_API S8_String OS_GetExeDir(MA_Arena *arena) {
    MA_Temp   scratch = MA_GetScratch();
    S8_String path    = OS_GetExePath(scratch.arena);
    S8_String dir     = S8_ChopLastSlash(path);
    S8_String copy    = S8_Copy(arena, dir);
    MA_ReleaseScratch(scratch);
    return copy;
}

OS_API S8_String OS_GetWorkingDir(MA_Arena *arena) {
    char     *buffer = (char *)MA_PushSizeNonZeroed(arena, PATH_MAX);
    char     *cwd    = getcwd(buffer, PATH_MAX);
    S8_String result = S8_MakeFromChar(cwd);
    return result;
}

OS_API void OS_SetWorkingDir(S8_String path) {
    MA_Temp   scratch = MA_GetScratch();
    S8_String copy    = S8_Copy(scratch.arena, path);
    chdir(copy.str);
    MA_ReleaseScratch(scratch);
}

OS_API S8_String OS_GetAbsolutePath(MA_Arena *arena, S8_String relative) {
    MA_Temp   scratch = MA_GetScratch1(arena);
    S8_String copy    = S8_Copy(scratch.arena, relative);

    char *buffer = (char *)MA_PushSizeNonZeroed(arena, PATH_MAX);
    realpath((char *)copy.str, buffer);
    S8_String result = S8_MakeFromChar(buffer);

    MA_ReleaseScratch(scratch);
    return result;
}

OS_API bool OS_FileExists(S8_String path) {
    MA_Temp   scratch = MA_GetScratch();
    S8_String copy    = S8_Copy(scratch.arena, path);

    bool result = false;
    if (access((char *)copy.str, F_OK) == 0) {
        result = true;
    }

    MA_ReleaseScratch(scratch);
    return result;
}

OS_API bool OS_IsDir(S8_String path) {
    MA_Temp   scratch = MA_GetScratch();
    S8_String copy    = S8_Copy(scratch.arena, path);

    struct stat s;
    if (stat(copy.str, &s) != 0)
        return false;

    bool result = S_ISDIR(s.st_mode);
    MA_ReleaseScratch(scratch);
    return result;
}

OS_API bool OS_IsFile(S8_String path) {
    MA_Temp   scratch = MA_GetScratch();
    S8_String copy    = S8_Copy(scratch.arena, path);

    struct stat s;
    if (stat(copy.str, &s) != 0)
        return false;
    bool result = S_ISREG(s.st_mode);
    MA_ReleaseScratch(scratch);
    return result;
}

OS_API double OS_GetTime(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t timeu64 = (((uint64_t)ts.tv_sec) * 1000000ull) + ((uint64_t)ts.tv_nsec) / 1000ull;
    double   timef   = (double)timeu64;
    double   result  = timef / 1000000.0; // Microseconds to seconds
    return result;
}

OS_API bool OS_IsValid(OS_FileIter it) {
    return it.is_valid;
}

OS_API void OS_Advance(OS_FileIter *it) {
    struct dirent *file = 0;
    while ((file = readdir((DIR *)it->dir)) != NULL) {
        if (file->d_name[0] == '.' && file->d_name[1] == '.' && file->d_name[2] == 0) continue;
        if (file->d_name[0] == '.' && file->d_name[1] == 0) continue;

        it->is_directory = file->d_type == DT_DIR;
        it->filename     = S8_CopyChar(it->arena, file->d_name);

        const char *dir_char_ending = it->is_directory ? "/" : "";
        const char *separator       = it->path.str[it->path.len - 1] == '/' ? "" : "/";
        it->relative_path           = S8_Format(it->arena, "%.*s%s%s%s", S8_Expand(it->path), separator, file->d_name, dir_char_ending);
        it->absolute_path           = OS_GetAbsolutePath(it->arena, it->relative_path);
        if (it->is_directory) it->absolute_path = S8_Format(it->arena, "%.*s/", S8_Expand(it->absolute_path));
        it->is_valid = true;
        return;
    }
    it->is_valid = false;
    closedir((DIR *)it->dir);
}

OS_API OS_FileIter OS_IterateFiles(MA_Arena *arena, S8_String path) {
    OS_FileIter it = {0};
    it.arena       = arena;
    it.path = path = S8_Copy(arena, path);
    it.dir         = (void *)opendir((char *)path.str);
    if (!it.dir) return it;

    OS_Advance(&it);
    return it;
}

OS_API OS_Result OS_MakeDir(S8_String path) {
    MA_Temp scratch = MA_GetScratch();
    path            = S8_Copy(scratch.arena, path);
    int error       = mkdir(path.str, 0755);
    MA_ReleaseScratch(scratch);
    return error == 0 ? OS_SUCCESS : OS_FAILURE;
}

OS_API OS_Result OS_CopyFile(S8_String from, S8_String to, bool overwrite) {
    const char *ow     = overwrite ? "-n" : "";
    int         result = OS_SystemF("cp %s %.*s %.*s", ow, S8_Expand(from), S8_Expand(to));
    return result == 0 ? OS_SUCCESS : OS_FAILURE;
}

OS_API OS_Result OS_DeleteFile(S8_String path) {
    int result = OS_SystemF("rm %.*s", S8_Expand(path));
    return result == 0 ? OS_SUCCESS : OS_FAILURE;
}

OS_API OS_Result OS_DeleteDir(S8_String path, unsigned flags) {
    IO_Assert(flags & OS_RECURSIVE);
    int result = OS_SystemF("rm -r %.*s", S8_Expand(path));
    return result == 0 ? OS_SUCCESS : OS_FAILURE;
}

OS_API int64_t OS_GetFileModTime(S8_String file) {
    MA_Temp scratch = MA_GetScratch();
    file            = S8_Copy(scratch.arena, file);

    struct stat attrib = {};
    stat(file.str, &attrib);
    struct timespec ts     = attrib.IF_LINUX_ELSE(st_mtim, st_mtimespec);
    int64_t         result = (((int64_t)ts.tv_sec) * 1000000ll) + ((int64_t)ts.tv_nsec) / 1000ll;

    MA_ReleaseScratch(scratch);
    return result;
}

OS_API OS_Date OS_GetDate(void) {
    time_t    t    = time(NULL);
    struct tm date = *localtime(&t);

    OS_Date s = {0};
    s.second  = date.tm_sec;
    s.year    = date.tm_year;
    s.month   = date.tm_mon;
    s.day     = date.tm_mday;
    s.hour    = date.tm_hour;
    s.minute  = date.tm_min;

    return s;
}

OS_API OS_Result OS_AppendFile(S8_String path, S8_String string) {
    MA_Temp scratch = MA_GetScratch();
    path            = S8_Copy(scratch.arena, path);

    OS_Result result = OS_FAILURE;
    FILE     *f      = fopen((const char *)path.str, "a");
    if (f) {
        result = OS_SUCCESS;

        size_t written = fwrite(string.str, 1, string.len, f);
        if (written < string.len) {
            result = OS_FAILURE;
        }

        int error = fclose(f);
        if (error != 0) {
            result = OS_FAILURE;
        }
    }

    MA_ReleaseScratch(scratch);
    return result;
}

OS_API S8_String OS_ReadFile(MA_Arena *arena, S8_String path) {
    S8_String result = {};

    // ftell returns insane size if file is
    // a directory **on some machines** KEKW
    if (OS_IsDir(path)) {
        return result;
    }

    MA_Temp scratch = MA_GetScratch1(arena);
    path            = S8_Copy(scratch.arena, path);

    FILE *f = fopen(path.str, "rb");
    if (f) {
        fseek(f, 0, SEEK_END);
        result.len = ftell(f);
        fseek(f, 0, SEEK_SET);

        result.str = (char *)MA_PushSizeNonZeroed(arena, result.len + 1);
        fread(result.str, result.len, 1, f);
        result.str[result.len] = 0;

        fclose(f);
    }

    MA_ReleaseScratch(scratch);
    return result;
}

OS_API OS_Result OS_WriteFile(S8_String path, S8_String string) {
    MA_Temp scratch = MA_GetScratch();
    path            = S8_Copy(scratch.arena, path);

    OS_Result result = OS_FAILURE;
    FILE     *f      = fopen((const char *)path.str, "w");
    if (f) {
        result = OS_SUCCESS;

        size_t written = fwrite(string.str, 1, string.len, f);
        if (written < string.len) {
            result = OS_FAILURE;
        }

        int error = fclose(f);
        if (error != 0) {
            result = OS_FAILURE;
        }
    }

    MA_ReleaseScratch(scratch);
    return result;
}
#endif

#if _WIN32 || __linux__ || __APPLE__ || __unix__
OS_API int OS_SystemF(const char *string, ...) {
    MA_Temp scratch = MA_GetScratch();
    S8_FORMAT(scratch.arena, string, result);
    IO_Printf("Executing: %.*s\n", S8_Expand(result));
    fflush(stdout);
    int error_code = system(result.str);
    MA_ReleaseScratch(scratch);
    return error_code;
}

OS_API bool OS_ExpandIncludesList(MA_Arena *arena, S8_List *out, S8_String filepath) {
    S8_String c = OS_ReadFile(arena, filepath);
    if (c.str == 0) return false;
    S8_String path    = S8_ChopLastSlash(filepath);
    S8_String include = S8_Lit("#include \"");
    for (;;) {
        int64_t idx = -1;
        if (S8_Seek(c, include, 0, &idx)) {
            S8_String str_to_add = S8_GetPrefix(c, idx);
            S8_AddNode(arena, out, str_to_add);
            S8_String save = c;
            c              = S8_Skip(c, idx + include.len);

            S8_String filename = c;
            filename.len       = 0;
            while (filename.str[filename.len] != '"' && filename.len < c.len) {
                filename.len += 1;
            }

            c                  = S8_Skip(c, filename.len + 1);
            S8_String inc_path = S8_Format(arena, "%.*s/%.*s", S8_Expand(path), S8_Expand(filename));
            if (!OS_ExpandIncludesList(arena, out, inc_path)) {
                S8_String s = S8_GetPrefix(save, save.len - c.len);
                S8_AddNode(arena, out, s);
            }
        } else {
            S8_AddNode(arena, out, c);
            break;
        }
    }
    return true;
}

OS_API S8_String OS_ExpandIncludes(MA_Arena *arena, S8_String filepath) {
    S8_List   out    = S8_MakeEmptyList();
    S8_String result = S8_MakeEmpty();
    MA_ScratchScope(s) {
        OS_ExpandIncludesList(s.arena, &out, filepath);
        result = S8_Merge(arena, out);
    }
    return result;
}
#endif