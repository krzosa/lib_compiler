LC_FUNCTION bool LC_EnableTerminalColors(void) {
    return true;
}

LC_FUNCTION bool LC_IsDir(LC_Arena *arena, LC_String path) {
    bool         result = false;
    LC_TempArena ch     = LC_BeginTemp(arena);
    LC_String    copy   = LC_CopyString(arena, path);

    struct stat s;
    if (stat(copy.str, &s) != 0) {
        result = false;
    } else {
        result = S_ISDIR(s.st_mode);
    }

    LC_EndTemp(ch);
    return result;
}

LC_FUNCTION LC_String LC_GetAbsolutePath(LC_Arena *arena, LC_String relative) {
    LC_String copy   = LC_CopyString(arena, relative);
    char     *buffer = (char *)LC_PushSizeNonZeroed(arena, PATH_MAX);
    realpath((char *)copy.str, buffer);
    LC_String result = LC_MakeFromChar(buffer);
    return result;
}

LC_FUNCTION bool LC_IsValid(LC_FileIter it) {
    return it.is_valid;
}

LC_FUNCTION void LC_Advance(LC_FileIter *it) {
    struct dirent *file = 0;
    while ((file = readdir((DIR *)it->dir)) != NULL) {
        if (file->d_name[0] == '.' && file->d_name[1] == '.' && file->d_name[2] == 0) continue;
        if (file->d_name[0] == '.' && file->d_name[1] == 0) continue;

        it->is_directory = file->d_type == DT_DIR;
        it->filename     = LC_CopyChar(it->arena, file->d_name);

        const char *dir_char_ending = it->is_directory ? "/" : "";
        const char *separator       = it->path.str[it->path.len - 1] == '/' ? "" : "/";
        it->relative_path           = LC_Format(it->arena, "%.*s%s%s%s", LC_Expand(it->path), separator, file->d_name, dir_char_ending);
        it->absolute_path           = LC_GetAbsolutePath(it->arena, it->relative_path);
        if (it->is_directory) it->absolute_path = LC_Format(it->arena, "%.*s/", LC_Expand(it->absolute_path));
        it->is_valid = true;
        return;
    }
    it->is_valid = false;
    closedir((DIR *)it->dir);
}

LC_FUNCTION LC_FileIter LC_IterateFiles(LC_Arena *arena, LC_String path) {
    LC_FileIter it = {0};
    it.arena       = arena;
    it.path = path = LC_CopyString(arena, path);
    it.dir         = (void *)opendir((char *)path.str);
    if (!it.dir) return it;

    LC_Advance(&it);
    return it;
}

LC_FUNCTION LC_String LC_ReadFile(LC_Arena *arena, LC_String path) {
    LC_String result = LC_MakeEmptyString();

    // ftell returns insane size if file is
    // a directory **on some machines** KEKW
    if (LC_IsDir(arena, path)) {
        return result;
    }

    path    = LC_CopyString(arena, path);
    FILE *f = fopen(path.str, "rb");
    if (f) {
        fseek(f, 0, SEEK_END);
        result.len = ftell(f);
        fseek(f, 0, SEEK_SET);

        result.str = (char *)LC_PushSizeNonZeroed(arena, result.len + 1);
        fread(result.str, result.len, 1, f);
        result.str[result.len] = 0;

        fclose(f);
    }

    return result;
}
