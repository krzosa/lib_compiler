S8_String Fmt(const char *str, ...) S8__PrintfFormat(1, 2);

Array<S8_String> operator+(Array<S8_String> a, Array<S8_String> b) {
    Array<S8_String> c = a.copy(MA_GetAllocator(Perm));
    c.add_array(b);
    return c;
}

Array<S8_String> operator+(Array<S8_String> a, S8_String b) {
    Array<S8_String> c = a.copy(MA_GetAllocator(Perm));
    c.add(b);
    return c;
}

Array<S8_String> operator+(S8_String a, Array<S8_String> b) {
    Array<S8_String> c = b.copy(MA_GetAllocator(Perm));
    c.insert(a, 0);
    return c;
}

Array<S8_String> operator+(S8_String a, S8_String b) {
    Array<S8_String> c = {MA_GetAllocator(Perm)};
    c.add(a);
    c.add(b);
    return c;
}

Array<S8_String> &operator+=(Array<S8_String> &a, Array<S8_String> b) {
    a.add_array(b);
    return a;
}

Array<S8_String> &operator+=(Array<S8_String> &a, S8_String s) {
    a.add(s);
    return a;
}

//@todo: split on any whitespace instead!
Array<S8_String> Split(S8_String s, S8_String sep = " ") {
    S8_List          list   = S8_Split(Perm, s, sep, 0);
    Array<S8_String> result = {MA_GetAllocator(Perm)};
    S8_For(it, list) result.add(it->string);
    return result;
}

S8_String Merge(MA_Arena *arena, Array<S8_String> list, S8_String separator = " "_s) {
    int64_t char_count = 0;
    For(list) char_count += it.len;
    if (char_count == 0) return {};
    int64_t node_count = list.len;

    int64_t   base_size = (char_count + 1);
    int64_t   sep_size  = (node_count - 1) * separator.len;
    int64_t   size      = base_size + sep_size;
    char     *buff      = (char *)MA_PushSize(arena, sizeof(char) * (size + 1));
    S8_String string    = S8_Make(buff, 0);
    For(list) {
        IO_Assert(string.len + it.len <= size);
        MA_MemoryCopy(string.str + string.len, it.str, it.len);
        string.len += it.len;
        if (!list.is_last(it)) {
            MA_MemoryCopy(string.str + string.len, separator.str, separator.len);
            string.len += separator.len;
        }
    }
    IO_Assert(string.len == size - 1);
    string.str[size] = 0;
    return string;
}

S8_String Merge(Array<S8_String> list, S8_String separator = " ") {
    return Merge(Perm, list, separator);
}

S8_String Fmt(const char *str, ...) {
    S8_FORMAT(Perm, str, str_fmt);
    return str_fmt;
}

Array<S8_String> ListDir(char *dir) {
    Array<S8_String> result = {MA_GetAllocator(Perm)};
    for (OS_FileIter it = OS_IterateFiles(Perm, S8_MakeFromChar(dir)); OS_IsValid(it); OS_Advance(&it)) {
        result.add(S8_Copy(Perm, it.absolute_path));
    }
    return result;
}

Array<S8_String> CMD_Make(char **argv, int argc) {
    Array<S8_String> result = {MA_GetAllocator(Perm)};
    for (int i = 1; i < argc; i += 1) {
        S8_String it = S8_MakeFromChar(argv[i]);
        result.add(it);
    }
    return result;
}

S8_String CMD_Get(Array<S8_String> &cmd, S8_String name, S8_String default_value = "") {
    For(cmd) {
        int64_t idx = 0;
        if (S8_Seek(it, "="_s, 0, &idx)) {
            S8_String key   = S8_GetPrefix(it, idx);
            S8_String value = S8_Skip(it, idx + 1);
            if (key == name) {
                return value;
            }
        }
    }
    return default_value;
}

bool CMD_Match(Array<S8_String> &cmd, S8_String name) {
    For(cmd) {
        if (it == name) return true;
    }
    return false;
}