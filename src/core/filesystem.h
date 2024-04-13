// Quick and dirty filesystem operations

#ifndef OS_API
    #define OS_API
#endif

typedef enum OS_Result {
    OS_SUCCESS,
    OS_ALREADY_EXISTS,
    OS_PATH_NOT_FOUND,
    OS_FAILURE,
} OS_Result;

enum {
    OS_NO_FLAGS = 0,
    OS_RECURSIVE = 1,
    OS_RELATIVE_PATHS = 2,
};

typedef struct OS_Date OS_Date;
struct OS_Date {
    uint32_t year;
    uint32_t month;
    uint32_t day;
    uint32_t hour;
    uint32_t minute;
    uint32_t second;
};

typedef struct OS_FileIter OS_FileIter;
struct OS_FileIter {
    bool is_valid;
    bool is_directory;
    S8_String absolute_path;
    S8_String relative_path;
    S8_String filename;

    S8_String path;
    MA_Arena *arena;
    union {
        struct OS_Win32_FileIter *w32;
        void *dir;
    };
};

OS_API bool OS_IsAbsolute(S8_String path);
OS_API S8_String OS_GetExePath(MA_Arena *arena);
OS_API S8_String OS_GetExeDir(MA_Arena *arena);
OS_API S8_String OS_GetWorkingDir(MA_Arena *arena);
OS_API void OS_SetWorkingDir(S8_String path);
OS_API S8_String OS_GetAbsolutePath(MA_Arena *arena, S8_String relative);
OS_API bool OS_FileExists(S8_String path);
OS_API bool OS_IsDir(S8_String path);
OS_API bool OS_IsFile(S8_String path);
OS_API double OS_GetTime(void);
OS_API OS_Result OS_MakeDir(S8_String path);
OS_API OS_Result OS_CopyFile(S8_String from, S8_String to, bool overwrite);
OS_API OS_Result OS_DeleteFile(S8_String path);
OS_API OS_Result OS_DeleteDir(S8_String path, unsigned flags);
OS_API OS_Result OS_AppendFile(S8_String path, S8_String string);
OS_API OS_Result OS_WriteFile(S8_String path, S8_String string);
OS_API S8_String OS_ReadFile(MA_Arena *arena, S8_String path);
OS_API int OS_SystemF(const char *string, ...);
OS_API int64_t OS_GetFileModTime(S8_String file);
OS_API OS_Date OS_GetDate(void);
OS_API S8_String UTF_CreateStringFromWidechar(MA_Arena *arena, wchar_t *wstr, int64_t wsize);
OS_API bool OS_ExpandIncludesList(MA_Arena *arena, S8_List *out, S8_String filepath);
OS_API S8_String OS_ExpandIncludes(MA_Arena *arena, S8_String filepath);
OS_API bool OS_EnableTerminalColors(void);

OS_API bool OS_IsValid(OS_FileIter it);
OS_API void OS_Advance(OS_FileIter *it);
OS_API OS_FileIter OS_IterateFiles(MA_Arena *scratch_arena, S8_String path);