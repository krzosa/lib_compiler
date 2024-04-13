typedef struct LC_Win32_FileIter {
    HANDLE           handle;
    WIN32_FIND_DATAW data;
} LC_Win32_FileIter;

LC_FUNCTION bool LC_IsDir(LC_Arena *arena, LC_String path) {
    wchar_t wbuff[1024];
    LC_CreateWidecharFromChar(wbuff, LC_StrLenof(wbuff), path.str, path.len);
    DWORD dwAttrib = GetFileAttributesW(wbuff);
    return dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

LC_FUNCTION LC_String LC_GetAbsolutePath(LC_Arena *arena, LC_String relative) {
    wchar_t wpath[1024];
    LC_CreateWidecharFromChar(wpath, LC_StrLenof(wpath), relative.str, relative.len);
    wchar_t wpath_abs[1024];
    DWORD   written = GetFullPathNameW((wchar_t *)wpath, LC_StrLenof(wpath_abs), wpath_abs, 0);
    if (written == 0)
        return LC_MakeEmptyString();
    LC_String path = LC_FromWidecharEx(arena, wpath_abs, written);
    LC_NormalizePathUnsafe(path);
    return path;
}

LC_FUNCTION bool LC_IsValid(LC_FileIter it) {
    return it.is_valid;
}

LC_FUNCTION void LC_Advance(LC_FileIter *it) {
    while (FindNextFileW(it->w32->handle, &it->w32->data) != 0) {
        WIN32_FIND_DATAW *data = &it->w32->data;

        // Skip '.' and '..'
        if (data->cFileName[0] == '.' && data->cFileName[1] == '.' && data->cFileName[2] == 0) continue;
        if (data->cFileName[0] == '.' && data->cFileName[1] == 0) continue;

        it->is_directory      = data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
        it->filename          = LC_FromWidecharEx(it->arena, data->cFileName, LC_WideLength(data->cFileName));
        const char *is_dir    = it->is_directory ? "/" : "";
        const char *separator = it->path.str[it->path.len - 1] == '/' ? "" : "/";
        it->relative_path     = LC_Format(it->arena, "%.*s%s%.*s%s", LC_Expand(it->path), separator, LC_Expand(it->filename), is_dir);
        it->absolute_path     = LC_GetAbsolutePath(it->arena, it->relative_path);
        it->is_valid          = true;

        if (it->is_directory) {
            LC_ASSERT(NULL, it->relative_path.str[it->relative_path.len - 1] == '/');
            LC_ASSERT(NULL, it->absolute_path.str[it->absolute_path.len - 1] == '/');
        }
        return;
    }

    it->is_valid = false;
    DWORD error  = GetLastError();
    LC_ASSERT(NULL, error == ERROR_NO_MORE_FILES);
    FindClose(it->w32->handle);
}

LC_FUNCTION LC_FileIter LC_IterateFiles(LC_Arena *scratch_arena, LC_String path) {
    LC_FileIter it = {0};
    it.arena       = scratch_arena;
    it.path        = path;

    LC_String modified_path = LC_Format(it.arena, "%.*s\\*", LC_Expand(path));
    wchar_t  *wbuff         = LC_PushArray(it.arena, wchar_t, modified_path.len + 1);
    int64_t   wsize         = LC_CreateWidecharFromChar(wbuff, modified_path.len + 1, modified_path.str, modified_path.len);
    LC_ASSERT(NULL, wsize);

    it.w32         = LC_PushStruct(it.arena, LC_Win32_FileIter);
    it.w32->handle = FindFirstFileW(wbuff, &it.w32->data);
    if (it.w32->handle == INVALID_HANDLE_VALUE) {
        it.is_valid = false;
        return it;
    }

    LC_ASSERT(NULL, it.w32->data.cFileName[0] == '.' && it.w32->data.cFileName[1] == 0);
    LC_Advance(&it);
    return it;
}

LC_FUNCTION LC_String LC_ReadFile(LC_Arena *arena, LC_String path) {
    LC_String result = LC_MakeEmptyString();

    wchar_t wpath[1024];
    LC_CreateWidecharFromChar(wpath, LC_StrLenof(wpath), path.str, path.len);
    HANDLE handle = CreateFileW(wpath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER file_size;
        if (GetFileSizeEx(handle, &file_size)) {
            if (file_size.QuadPart != 0) {
                result.len = (int64_t)file_size.QuadPart;
                result.str = (char *)LC_PushSizeNonZeroed(arena, result.len + 1);
                DWORD read;
                if (ReadFile(handle, result.str, (DWORD)result.len, &read, NULL)) { // @todo: can only read 32 byte size files?
                    if (read == result.len) {
                        result.str[result.len] = 0;
                    }
                }
            }
        }
        CloseHandle(handle);
    }

    return result;
}

LC_FUNCTION bool LC_EnableTerminalColors(void) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            if (SetConsoleMode(hOut, dwMode)) {
                return true;
            }
        }
    }
    return false;
}
