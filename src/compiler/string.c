LC_FUNCTION int64_t LC__ClampTop(int64_t val, int64_t max) {
    if (val > max) val = max;
    return val;
}

LC_FUNCTION char LC_ToLowerCase(char a) {
    if (a >= 'A' && a <= 'Z') a += 32;
    return a;
}

LC_FUNCTION char LC_ToUpperCase(char a) {
    if (a >= 'a' && a <= 'z') a -= 32;
    return a;
}

LC_FUNCTION bool LC_IsWhitespace(char w) {
    bool result = w == '\n' || w == ' ' || w == '\t' || w == '\v' || w == '\r';
    return result;
}

LC_FUNCTION bool LC_IsAlphabetic(char a) {
    bool result = (a >= 'a' && a <= 'z') || (a >= 'A' && a <= 'Z');
    return result;
}

LC_FUNCTION bool LC_IsIdent(char a) {
    bool result = (a >= 'a' && a <= 'z') || (a >= 'A' && a <= 'Z') || a == '_';
    return result;
}

LC_FUNCTION bool LC_IsDigit(char a) {
    bool result = a >= '0' && a <= '9';
    return result;
}

LC_FUNCTION bool LC_IsAlphanumeric(char a) {
    bool result = LC_IsDigit(a) || LC_IsAlphabetic(a);
    return result;
}

LC_FUNCTION bool LC_AreEqual(LC_String a, LC_String b, unsigned ignore_case) {
    if (a.len != b.len) return false;
    for (int64_t i = 0; i < a.len; i++) {
        char A = a.str[i];
        char B = b.str[i];
        if (ignore_case) {
            A = LC_ToLowerCase(A);
            B = LC_ToLowerCase(B);
        }
        if (A != B)
            return false;
    }
    return true;
}

LC_FUNCTION bool LC_EndsWith(LC_String a, LC_String end, unsigned ignore_case) {
    LC_String a_end  = LC_GetPostfix(a, end.len);
    bool      result = LC_AreEqual(end, a_end, ignore_case);
    return result;
}

LC_FUNCTION bool LC_StartsWith(LC_String a, LC_String start, unsigned ignore_case) {
    LC_String a_start = LC_GetPrefix(a, start.len);
    bool      result  = LC_AreEqual(start, a_start, ignore_case);
    return result;
}

LC_FUNCTION LC_String LC_MakeString(char *str, int64_t len) {
    LC_String result;
    result.str = (char *)str;
    result.len = len;
    return result;
}

LC_FUNCTION LC_String LC_CopyString(LC_Arena *allocator, LC_String string) {
    char *copy = (char *)LC_PushSize(allocator, sizeof(char) * (string.len + 1));
    LC_MemoryCopy(copy, string.str, string.len);
    copy[string.len] = 0;
    LC_String result = LC_MakeString(copy, string.len);
    return result;
}

LC_FUNCTION LC_String LC_CopyChar(LC_Arena *allocator, char *s) {
    int64_t len  = LC_StrLen(s);
    char   *copy = (char *)LC_PushSize(allocator, sizeof(char) * (len + 1));
    LC_MemoryCopy(copy, s, len);
    copy[len]        = 0;
    LC_String result = LC_MakeString(copy, len);
    return result;
}

LC_FUNCTION LC_String LC_NormalizePath(LC_Arena *allocator, LC_String s) {
    LC_String copy = LC_CopyString(allocator, s);
    for (int64_t i = 0; i < copy.len; i++) {
        if (copy.str[i] == '\\')
            copy.str[i] = '/';
    }
    return copy;
}

LC_FUNCTION void LC_NormalizePathUnsafe(LC_String s) {
    for (int64_t i = 0; i < s.len; i++) {
        if (s.str[i] == '\\')
            s.str[i] = '/';
    }
}

LC_FUNCTION LC_String LC_Chop(LC_String string, int64_t len) {
    len              = LC__ClampTop(len, string.len);
    LC_String result = LC_MakeString(string.str, string.len - len);
    return result;
}

LC_FUNCTION LC_String LC_Skip(LC_String string, int64_t len) {
    len              = LC__ClampTop(len, string.len);
    int64_t   remain = string.len - len;
    LC_String result = LC_MakeString(string.str + len, remain);
    return result;
}

LC_FUNCTION LC_String LC_GetPostfix(LC_String string, int64_t len) {
    len                  = LC__ClampTop(len, string.len);
    int64_t   remain_len = string.len - len;
    LC_String result     = LC_MakeString(string.str + remain_len, len);
    return result;
}

LC_FUNCTION LC_String LC_GetPrefix(LC_String string, int64_t len) {
    len              = LC__ClampTop(len, string.len);
    LC_String result = LC_MakeString(string.str, len);
    return result;
}

LC_FUNCTION LC_String LC_GetNameNoExt(LC_String s) {
    return LC_SkipToLastSlash(LC_ChopLastPeriod(s));
}

LC_FUNCTION LC_String LC_Slice(LC_String string, int64_t first_index, int64_t one_past_last_index) {
    if (one_past_last_index < 0) one_past_last_index = string.len + one_past_last_index + 1;
    if (first_index < 0) first_index = string.len + first_index;
    LC_ASSERT(NULL, first_index < one_past_last_index && "LC_Slice, first_index is bigger then one_past_last_index");
    LC_ASSERT(NULL, string.len > 0 && "Slicing string of length 0! Might be an error!");
    LC_String result = string;
    if (string.len > 0) {
        if (one_past_last_index > first_index) {
            first_index         = LC__ClampTop(first_index, string.len - 1);
            one_past_last_index = LC__ClampTop(one_past_last_index, string.len);
            result.str += first_index;
            result.len = one_past_last_index - first_index;
        } else {
            result.len = 0;
        }
    }
    return result;
}

LC_FUNCTION bool LC_Seek(LC_String string, LC_String find, LC_FindFlag flags, int64_t *index_out) {
    bool ignore_case = flags & LC_FindFlag_IgnoreCase ? true : false;
    bool result      = false;
    if (flags & LC_FindFlag_MatchFindLast) {
        for (int64_t i = string.len; i != 0; i--) {
            int64_t   index     = i - 1;
            LC_String substring = LC_Slice(string, index, index + find.len);
            if (LC_AreEqual(substring, find, ignore_case)) {
                if (index_out)
                    *index_out = index;
                result = true;
                break;
            }
        }
    } else {
        for (int64_t i = 0; i < string.len; i++) {
            LC_String substring = LC_Slice(string, i, i + find.len);
            if (LC_AreEqual(substring, find, ignore_case)) {
                if (index_out)
                    *index_out = i;
                result = true;
                break;
            }
        }
    }

    return result;
}

LC_FUNCTION int64_t LC_Find(LC_String string, LC_String find, LC_FindFlag flag) {
    int64_t result = -1;
    LC_Seek(string, find, flag, &result);
    return result;
}

LC_FUNCTION LC_StringList LC_Split(LC_Arena *allocator, LC_String string, LC_String find, LC_SplitFlag flags) {
    LC_StringList result = LC_MakeEmptyList();
    int64_t       index  = 0;

    LC_FindFlag find_flag = flags & LC_SplitFlag_IgnoreCase ? LC_FindFlag_IgnoreCase : LC_FindFlag_None;
    while (LC_Seek(string, find, find_flag, &index)) {
        LC_String before_match = LC_MakeString(string.str, index);
        LC_AddNode(allocator, &result, before_match);
        if (flags & LC_SplitFlag_SplitInclusive) {
            LC_String match = LC_MakeString(string.str + index, find.len);
            LC_AddNode(allocator, &result, match);
        }
        string = LC_Skip(string, index + find.len);
    }
    if (string.len) LC_AddNode(allocator, &result, string);
    return result;
}

LC_FUNCTION LC_String LC_MergeWithSeparator(LC_Arena *allocator, LC_StringList list, LC_String separator) {
    if (list.node_count == 0) return LC_MakeEmptyString();
    if (list.char_count == 0) return LC_MakeEmptyString();

    int64_t   base_size = (list.char_count + 1);
    int64_t   sep_size  = (list.node_count - 1) * separator.len;
    int64_t   size      = base_size + sep_size;
    char     *buff      = (char *)LC_PushSize(allocator, sizeof(char) * (size + 1));
    LC_String string    = LC_MakeString(buff, 0);
    for (LC_StringNode *it = list.first; it; it = it->next) {
        LC_ASSERT(NULL, string.len + it->string.len <= size);
        LC_MemoryCopy(string.str + string.len, it->string.str, it->string.len);
        string.len += it->string.len;
        if (it != list.last) {
            LC_MemoryCopy(string.str + string.len, separator.str, separator.len);
            string.len += separator.len;
        }
    }
    LC_ASSERT(NULL, string.len == size - 1);
    string.str[size] = 0;
    return string;
}

LC_FUNCTION LC_String LC_MergeString(LC_Arena *allocator, LC_StringList list) {
    return LC_MergeWithSeparator(allocator, list, LC_Lit(""));
}

LC_FUNCTION LC_String LC_ChopLastSlash(LC_String s) {
    LC_String result = s;
    LC_Seek(s, LC_Lit("/"), LC_FindFlag_MatchFindLast, &result.len);
    return result;
}

LC_FUNCTION LC_String LC_ChopLastPeriod(LC_String s) {
    LC_String result = s;
    LC_Seek(s, LC_Lit("."), LC_FindFlag_MatchFindLast, &result.len);
    return result;
}

LC_FUNCTION LC_String LC_SkipToLastSlash(LC_String s) {
    int64_t   pos;
    LC_String result = s;
    if (LC_Seek(s, LC_Lit("/"), LC_FindFlag_MatchFindLast, &pos)) {
        result = LC_Skip(result, pos + 1);
    }
    return result;
}

LC_FUNCTION LC_String LC_SkipToLastPeriod(LC_String s) {
    int64_t   pos;
    LC_String result = s;
    if (LC_Seek(s, LC_Lit("."), LC_FindFlag_MatchFindLast, &pos)) {
        result = LC_Skip(result, pos + 1);
    }
    return result;
}

LC_FUNCTION int64_t LC_StrLen(char *string) {
    int64_t len = 0;
    while (*string++ != 0)
        len++;
    return len;
}

LC_FUNCTION LC_String LC_MakeFromChar(char *string) {
    LC_String result;
    result.str = (char *)string;
    result.len = LC_StrLen(string);
    return result;
}

LC_FUNCTION LC_String LC_MakeEmptyString(void) {
    return LC_MakeString(0, 0);
}

LC_FUNCTION LC_StringList LC_MakeEmptyList(void) {
    LC_StringList result;
    result.first      = 0;
    result.last       = 0;
    result.char_count = 0;
    result.node_count = 0;
    return result;
}

LC_FUNCTION LC_String LC_FormatV(LC_Arena *allocator, const char *str, va_list args1) {
    va_list args2;
    va_copy(args2, args1);
    int64_t len = LC_vsnprintf(0, 0, str, args2);
    va_end(args2);

    char *result = (char *)LC_PushSize(allocator, sizeof(char) * (len + 1));
    LC_vsnprintf(result, (int)(len + 1), str, args1);
    LC_String res = LC_MakeString(result, len);
    return res;
}

LC_FUNCTION LC_String LC_Format(LC_Arena *allocator, const char *str, ...) {
    LC_FORMAT(allocator, str, result);
    return result;
}

LC_FUNCTION LC_StringNode *LC_CreateNode(LC_Arena *allocator, LC_String string) {
    LC_StringNode *result = (LC_StringNode *)LC_PushSize(allocator, sizeof(LC_StringNode));
    result->string        = string;
    result->next          = 0;
    return result;
}

LC_FUNCTION void LC_ReplaceNodeString(LC_StringList *list, LC_StringNode *node, LC_String new_string) {
    list->char_count -= node->string.len;
    list->char_count += new_string.len;
    node->string = new_string;
}

LC_FUNCTION void LC_AddExistingNode(LC_StringList *list, LC_StringNode *node) {
    if (list->first) {
        list->last->next = node;
        list->last       = list->last->next;
    } else {
        list->first = list->last = node;
    }
    list->node_count += 1;
    list->char_count += node->string.len;
}

LC_FUNCTION void LC_AddArray(LC_Arena *allocator, LC_StringList *list, char **array, int count) {
    for (int i = 0; i < count; i += 1) {
        LC_String s = LC_MakeFromChar(array[i]);
        LC_AddNode(allocator, list, s);
    }
}

LC_FUNCTION void LC_AddArrayWithPrefix(LC_Arena *allocator, LC_StringList *list, char *prefix, char **array, int count) {
    for (int i = 0; i < count; i += 1) {
        LC_Addf(allocator, list, "%s%s", prefix, array[i]);
    }
}

LC_FUNCTION LC_StringList LC_MakeList(LC_Arena *allocator, LC_String a) {
    LC_StringList result = LC_MakeEmptyList();
    LC_AddNode(allocator, &result, a);
    return result;
}

LC_FUNCTION LC_StringList LC_CopyList(LC_Arena *allocator, LC_StringList a) {
    LC_StringList result = LC_MakeEmptyList();
    for (LC_StringNode *it = a.first; it; it = it->next) LC_AddNode(allocator, &result, it->string);
    return result;
}

LC_FUNCTION LC_StringList LC_ConcatLists(LC_Arena *allocator, LC_StringList a, LC_StringList b) {
    LC_StringList result = LC_MakeEmptyList();
    for (LC_StringNode *it = a.first; it; it = it->next) LC_AddNode(allocator, &result, it->string);
    for (LC_StringNode *it = b.first; it; it = it->next) LC_AddNode(allocator, &result, it->string);
    return result;
}

LC_FUNCTION LC_StringNode *LC_AddNode(LC_Arena *allocator, LC_StringList *list, LC_String string) {
    LC_StringNode *node = LC_CreateNode(allocator, string);
    LC_AddExistingNode(list, node);
    return node;
}

LC_FUNCTION LC_StringNode *LC_Add(LC_Arena *allocator, LC_StringList *list, LC_String string) {
    LC_String      copy = LC_CopyString(allocator, string);
    LC_StringNode *node = LC_CreateNode(allocator, copy);
    LC_AddExistingNode(list, node);
    return node;
}

LC_FUNCTION LC_String LC_Addf(LC_Arena *allocator, LC_StringList *list, const char *str, ...) {
    LC_FORMAT(allocator, str, result);
    LC_AddNode(allocator, list, result);
    return result;
}

LC_FUNCTION LC_String16 LC_ToWidecharEx(LC_Arena *allocator, LC_String string) {
    LC_ASSERT(NULL, sizeof(wchar_t) == 2);
    wchar_t    *buffer = (wchar_t *)LC_PushSize(allocator, sizeof(wchar_t) * (string.len + 1));
    int64_t     size   = LC_CreateWidecharFromChar(buffer, string.len + 1, string.str, string.len);
    LC_String16 result = {buffer, size};
    return result;
}

LC_FUNCTION wchar_t *LC_ToWidechar(LC_Arena *allocator, LC_String string) {
    LC_String16 result = LC_ToWidecharEx(allocator, string);
    return result.str;
}

LC_FUNCTION LC_String LC_FromWidecharEx(LC_Arena *allocator, wchar_t *wstring, int64_t wsize) {
    LC_ASSERT(NULL, sizeof(wchar_t) == 2);

    int64_t   buffer_size = (wsize + 1) * 2;
    char     *buffer      = (char *)LC_PushSize(allocator, buffer_size);
    int64_t   size        = LC_CreateCharFromWidechar(buffer, buffer_size, wstring, wsize);
    LC_String result      = LC_MakeString(buffer, size);

    LC_ASSERT(NULL, size < buffer_size);
    return result;
}

LC_FUNCTION int64_t LC_WideLength(wchar_t *string) {
    int64_t len = 0;
    while (*string++ != 0)
        len++;
    return len;
}

LC_FUNCTION LC_String LC_FromWidechar(LC_Arena *allocator, wchar_t *wstring) {
    int64_t   size   = LC_WideLength(wstring);
    LC_String result = LC_FromWidecharEx(allocator, wstring, size);
    return result;
}
