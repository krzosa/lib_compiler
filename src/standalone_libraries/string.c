#include "string.h"
#include <stdarg.h>

#ifndef S8_VSNPRINTF
    #include <stdio.h>
    #define S8_VSNPRINTF vsnprintf
#endif

#ifndef S8_ALLOCATE
    #include <stdlib.h>
    #define S8_ALLOCATE(allocator, size) malloc(size)
#endif

#ifndef S8_ASSERT
    #include <assert.h>
    #define S8_ASSERT(x) assert(x)
#endif

#ifndef S8_MemoryCopy
    #include <string.h>
    #define S8_MemoryCopy(dst, src, s) memcpy(dst, src, s)
#endif

#ifndef S8_StaticFunc
    #if defined(__GNUC__) || defined(__clang__)
        #define S8_StaticFunc __attribute__((unused)) static
    #else
        #define S8_StaticFunc static
    #endif
#endif

S8_StaticFunc int64_t S8__ClampTop(int64_t val, int64_t max) {
    if (val > max) val = max;
    return val;
}

S8_API char CHAR_ToLowerCase(char a) {
    if (a >= 'A' && a <= 'Z') a += 32;
    return a;
}

S8_API char CHAR_ToUpperCase(char a) {
    if (a >= 'a' && a <= 'z') a -= 32;
    return a;
}

S8_API bool CHAR_IsWhitespace(char w) {
    bool result = w == '\n' || w == ' ' || w == '\t' || w == '\v' || w == '\r';
    return result;
}

S8_API bool CHAR_IsAlphabetic(char a) {
    bool result = (a >= 'a' && a <= 'z') || (a >= 'A' && a <= 'Z');
    return result;
}

S8_API bool CHAR_IsIdent(char a) {
    bool result = (a >= 'a' && a <= 'z') || (a >= 'A' && a <= 'Z') || a == '_';
    return result;
}

S8_API bool CHAR_IsDigit(char a) {
    bool result = a >= '0' && a <= '9';
    return result;
}

S8_API bool CHAR_IsAlphanumeric(char a) {
    bool result = CHAR_IsDigit(a) || CHAR_IsAlphabetic(a);
    return result;
}

S8_API bool S8_AreEqual(S8_String a, S8_String b, unsigned ignore_case) {
    if (a.len != b.len) return false;
    for (int64_t i = 0; i < a.len; i++) {
        char A = a.str[i];
        char B = b.str[i];
        if (ignore_case) {
            A = CHAR_ToLowerCase(A);
            B = CHAR_ToLowerCase(B);
        }
        if (A != B)
            return false;
    }
    return true;
}

S8_API bool S8_EndsWith(S8_String a, S8_String end, unsigned ignore_case) {
    S8_String a_end = S8_GetPostfix(a, end.len);
    bool result = S8_AreEqual(end, a_end, ignore_case);
    return result;
}

S8_API bool S8_StartsWith(S8_String a, S8_String start, unsigned ignore_case) {
    S8_String a_start = S8_GetPrefix(a, start.len);
    bool result = S8_AreEqual(start, a_start, ignore_case);
    return result;
}

S8_API S8_String S8_Make(char *str, int64_t len) {
    S8_String result;
    result.str = (char *)str;
    result.len = len;
    return result;
}

S8_API S8_String S8_Copy(S8_Allocator allocator, S8_String string) {
    char *copy = (char *)S8_ALLOCATE(allocator, sizeof(char) * (string.len + 1));
    S8_MemoryCopy(copy, string.str, string.len);
    copy[string.len] = 0;
    S8_String result = S8_Make(copy, string.len);
    return result;
}

S8_API S8_String S8_CopyChar(S8_Allocator allocator, char *s) {
    int64_t len = S8_Length(s);
    char *copy = (char *)S8_ALLOCATE(allocator, sizeof(char) * (len + 1));
    S8_MemoryCopy(copy, s, len);
    copy[len] = 0;
    S8_String result = S8_Make(copy, len);
    return result;
}

S8_API S8_String S8_NormalizePath(S8_Allocator allocator, S8_String s) {
    S8_String copy = S8_Copy(allocator, s);
    for (int64_t i = 0; i < copy.len; i++) {
        if (copy.str[i] == '\\')
            copy.str[i] = '/';
    }
    return copy;
}

S8_API void S8_NormalizePathUnsafe(S8_String s) {
    for (int64_t i = 0; i < s.len; i++) {
        if (s.str[i] == '\\')
            s.str[i] = '/';
    }
}

S8_API S8_String S8_Chop(S8_String string, int64_t len) {
    len = S8__ClampTop(len, string.len);
    S8_String result = S8_Make(string.str, string.len - len);
    return result;
}

S8_API S8_String S8_Skip(S8_String string, int64_t len) {
    len = S8__ClampTop(len, string.len);
    int64_t remain = string.len - len;
    S8_String result = S8_Make(string.str + len, remain);
    return result;
}

S8_API bool S8_IsPointerInside(S8_String string, char *p) {
    uintptr_t pointer = (uintptr_t)p;
    uintptr_t start = (uintptr_t)string.str;
    uintptr_t stop = start + (uintptr_t)string.len;
    bool result = pointer >= start && pointer < stop;
    return result;
}

S8_API S8_String S8_SkipToP(S8_String string, char *p) {
    if (S8_IsPointerInside(string, p)) {
        S8_String result = S8_Make(p, p - string.str);
        return result;
    }
    return string;
}

S8_API S8_String S8_SkipPast(S8_String string, S8_String a) {
    if (S8_IsPointerInside(string, a.str)) {
        S8_String on_p = S8_Make(a.str, a.str - string.str);
        S8_String result = S8_Skip(on_p, a.len);
        return result;
    }
    return string;
}

S8_API S8_String S8_GetPostfix(S8_String string, int64_t len) {
    len = S8__ClampTop(len, string.len);
    int64_t remain_len = string.len - len;
    S8_String result = S8_Make(string.str + remain_len, len);
    return result;
}

S8_API S8_String S8_GetPrefix(S8_String string, int64_t len) {
    len = S8__ClampTop(len, string.len);
    S8_String result = S8_Make(string.str, len);
    return result;
}

S8_API S8_String S8_GetNameNoExt(S8_String s) {
    return S8_SkipToLastSlash(S8_ChopLastPeriod(s));
}

S8_API S8_String S8_Slice(S8_String string, int64_t first_index, int64_t one_past_last_index) {
    if (one_past_last_index < 0) one_past_last_index = string.len + one_past_last_index + 1;
    if (first_index < 0) first_index = string.len + first_index;
    S8_ASSERT(first_index < one_past_last_index && "S8_Slice, first_index is bigger then one_past_last_index");
    S8_ASSERT(string.len > 0 && "Slicing string of length 0! Might be an error!");
    S8_String result = string;
    if (string.len > 0) {
        if (one_past_last_index > first_index) {
            first_index = S8__ClampTop(first_index, string.len - 1);
            one_past_last_index = S8__ClampTop(one_past_last_index, string.len);
            result.str += first_index;
            result.len = one_past_last_index - first_index;
        }
        else {
            result.len = 0;
        }
    }
    return result;
}

S8_API S8_String S8_Trim(S8_String string) {
    if (string.len == 0)
        return string;

    int64_t whitespace_begin = 0;
    for (; whitespace_begin < string.len; whitespace_begin++) {
        if (!CHAR_IsWhitespace(string.str[whitespace_begin])) {
            break;
        }
    }

    int64_t whitespace_end = string.len;
    for (; whitespace_end != whitespace_begin; whitespace_end--) {
        if (!CHAR_IsWhitespace(string.str[whitespace_end - 1])) {
            break;
        }
    }

    if (whitespace_begin == whitespace_end) {
        string.len = 0;
    }
    else {
        string = S8_Slice(string, whitespace_begin, whitespace_end);
    }

    return string;
}

S8_API S8_String S8_TrimEnd(S8_String string) {
    int64_t whitespace_end = string.len;
    for (; whitespace_end != 0; whitespace_end--) {
        if (!CHAR_IsWhitespace(string.str[whitespace_end - 1])) {
            break;
        }
    }

    S8_String result = S8_GetPrefix(string, whitespace_end);
    return result;
}

S8_API S8_String S8_ToLowerCase(S8_Allocator allocator, S8_String s) {
    S8_String copy = S8_Copy(allocator, s);
    for (int64_t i = 0; i < copy.len; i++) {
        copy.str[i] = CHAR_ToLowerCase(copy.str[i]);
    }
    return copy;
}

S8_API S8_String S8_ToUpperCase(S8_Allocator allocator, S8_String s) {
    S8_String copy = S8_Copy(allocator, s);
    for (int64_t i = 0; i < copy.len; i++) {
        copy.str[i] = CHAR_ToUpperCase(copy.str[i]);
    }
    return copy;
}

S8_API bool S8_Seek(S8_String string, S8_String find, S8_FindFlag flags, int64_t *index_out) {
    bool ignore_case = flags & S8_FindFlag_IgnoreCase ? true : false;
    bool result = false;
    if (flags & S8_FindFlag_MatchFindLast) {
        for (int64_t i = string.len; i != 0; i--) {
            int64_t index = i - 1;
            S8_String substring = S8_Slice(string, index, index + find.len);
            if (S8_AreEqual(substring, find, ignore_case)) {
                if (index_out)
                    *index_out = index;
                result = true;
                break;
            }
        }
    }
    else {
        for (int64_t i = 0; i < string.len; i++) {
            S8_String substring = S8_Slice(string, i, i + find.len);
            if (S8_AreEqual(substring, find, ignore_case)) {
                if (index_out)
                    *index_out = i;
                result = true;
                break;
            }
        }
    }

    return result;
}

S8_API int64_t S8_Find(S8_String string, S8_String find, S8_FindFlag flag) {
    int64_t result = -1;
    S8_Seek(string, find, flag, &result);
    return result;
}

S8_API S8_List S8_Split(S8_Allocator allocator, S8_String string, S8_String find, S8_SplitFlag flags) {
    S8_List result = S8_MakeEmptyList();
    int64_t index = 0;

    S8_FindFlag find_flag = flags & S8_SplitFlag_IgnoreCase ? S8_FindFlag_IgnoreCase : S8_FindFlag_None;
    while (S8_Seek(string, find, find_flag, &index)) {
        S8_String before_match = S8_Make(string.str, index);
        S8_AddNode(allocator, &result, before_match);
        if (flags & S8_SplitFlag_SplitInclusive) {
            S8_String match = S8_Make(string.str + index, find.len);
            S8_AddNode(allocator, &result, match);
        }
        string = S8_Skip(string, index + find.len);
    }
    if (string.len) S8_AddNode(allocator, &result, string);
    return result;
}

S8_API S8_String S8_MergeWithSeparator(S8_Allocator allocator, S8_List list, S8_String separator) {
    if (list.node_count == 0) return S8_MakeEmpty();
    if (list.char_count == 0) return S8_MakeEmpty();

    int64_t base_size = (list.char_count + 1);
    int64_t sep_size = (list.node_count - 1) * separator.len;
    int64_t size = base_size + sep_size;
    char *buff = (char *)S8_ALLOCATE(allocator, sizeof(char) * (size + 1));
    S8_String string = S8_Make(buff, 0);
    for (S8_Node *it = list.first; it; it = it->next) {
        S8_ASSERT(string.len + it->string.len <= size);
        S8_MemoryCopy(string.str + string.len, it->string.str, it->string.len);
        string.len += it->string.len;
        if (it != list.last) {
            S8_MemoryCopy(string.str + string.len, separator.str, separator.len);
            string.len += separator.len;
        }
    }
    S8_ASSERT(string.len == size - 1);
    string.str[size] = 0;
    return string;
}

S8_API S8_String S8_Merge(S8_Allocator allocator, S8_List list) {
    return S8_MergeWithSeparator(allocator, list, S8_Lit(""));
}

S8_API S8_String S8_ReplaceAll(S8_Allocator allocator, S8_String string, S8_String replace, S8_String with, bool ignore_case) {
    S8_SplitFlag split_flag = ignore_case ? S8_SplitFlag_IgnoreCase : S8_SplitFlag_None;
    S8_List list = S8_Split(allocator, string, replace, split_flag | S8_SplitFlag_SplitInclusive);
    for (S8_Node *it = list.first; it; it = it->next) {
        if (S8_AreEqual(it->string, replace, ignore_case)) {
            S8_ReplaceNodeString(&list, it, with);
        }
    }
    S8_String result = S8_Merge(allocator, list);
    return result;
}

S8_API S8_List S8_FindAll(S8_Allocator allocator, S8_String string, S8_String find, bool ignore_case) { // @untested
    S8_List result = S8_MakeEmptyList();
    int64_t index = 0;

    S8_FindFlag find_flag = ignore_case ? S8_FindFlag_IgnoreCase : 0;
    while (S8_Seek(string, find, find_flag, &index)) {
        S8_String match = S8_Make(string.str + index, find.len);
        S8_AddNode(allocator, &result, match);
        string = S8_Skip(string, index + find.len);
    }
    return result;
}

S8_API S8_String S8_ChopLastSlash(S8_String s) {
    S8_String result = s;
    S8_Seek(s, S8_Lit("/"), S8_FindFlag_MatchFindLast, &result.len);
    return result;
}

S8_API S8_String S8_ChopLastPeriod(S8_String s) {
    S8_String result = s;
    S8_Seek(s, S8_Lit("."), S8_FindFlag_MatchFindLast, &result.len);
    return result;
}

S8_API S8_String S8_SkipToLastSlash(S8_String s) {
    int64_t pos;
    S8_String result = s;
    if (S8_Seek(s, S8_Lit("/"), S8_FindFlag_MatchFindLast, &pos)) {
        result = S8_Skip(result, pos + 1);
    }
    return result;
}

S8_API S8_String S8_SkipToLastPeriod(S8_String s) {
    int64_t pos;
    S8_String result = s;
    if (S8_Seek(s, S8_Lit("."), S8_FindFlag_MatchFindLast, &pos)) {
        result = S8_Skip(result, pos + 1);
    }
    return result;
}

S8_API int64_t S8_Length(char *string) {
    int64_t len = 0;
    while (*string++ != 0)
        len++;
    return len;
}

S8_API int64_t S8_WideLength(wchar_t *string) {
    int64_t len = 0;
    while (*string++ != 0)
        len++;
    return len;
}

S8_API S8_String S8_MakeFromChar(char *string) {
    S8_String result;
    result.str = (char *)string;
    result.len = S8_Length(string);
    return result;
}

S8_API S8_String S8_MakeEmpty(void) {
    return S8_Make(0, 0);
}

S8_API S8_List S8_MakeEmptyList(void) {
    S8_List result;
    result.first = 0;
    result.last = 0;
    result.char_count = 0;
    result.node_count = 0;
    return result;
}

S8_API S8_String S8_FormatV(S8_Allocator allocator, const char *str, va_list args1) {
    va_list args2;
    va_copy(args2, args1);
    int64_t len = S8_VSNPRINTF(0, 0, str, args2);
    va_end(args2);

    char *result = (char *)S8_ALLOCATE(allocator, sizeof(char) * (len + 1));
    S8_VSNPRINTF(result, (int)(len + 1), str, args1);
    S8_String res = S8_Make(result, len);
    return res;
}

S8_API S8_String S8_Format(S8_Allocator allocator, const char *str, ...) {
    S8_FORMAT(allocator, str, result);
    return result;
}

S8_API S8_Node *S8_CreateNode(S8_Allocator allocator, S8_String string) {
    S8_Node *result = (S8_Node *)S8_ALLOCATE(allocator, sizeof(S8_Node));
    result->string = string;
    result->next = 0;
    return result;
}

S8_API void S8_ReplaceNodeString(S8_List *list, S8_Node *node, S8_String new_string) {
    list->char_count -= node->string.len;
    list->char_count += new_string.len;
    node->string = new_string;
}

S8_API void S8_AddExistingNode(S8_List *list, S8_Node *node) {
    if (list->first) {
        list->last->next = node;
        list->last = list->last->next;
    }
    else {
        list->first = list->last = node;
    }
    list->node_count += 1;
    list->char_count += node->string.len;
}

S8_API void S8_AddArray(S8_Allocator allocator, S8_List *list, char **array, int count) {
    for (int i = 0; i < count; i += 1) {
        S8_String s = S8_MakeFromChar(array[i]);
        S8_AddNode(allocator, list, s);
    }
}

S8_API void S8_AddArrayWithPrefix(S8_Allocator allocator, S8_List *list, char *prefix, char **array, int count) {
    for (int i = 0; i < count; i += 1) {
        S8_AddF(allocator, list, "%s%s", prefix, array[i]);
    }
}

S8_API S8_List S8_MakeList(S8_Allocator allocator, S8_String a) {
    S8_List result = S8_MakeEmptyList();
    S8_AddNode(allocator, &result, a);
    return result;
}

S8_API S8_List S8_CopyList(S8_Allocator allocator, S8_List a) {
    S8_List result = S8_MakeEmptyList();
    for (S8_Node *it = a.first; it; it = it->next) S8_AddNode(allocator, &result, it->string);
    return result;
}

S8_API S8_List S8_ConcatLists(S8_Allocator allocator, S8_List a, S8_List b) {
    S8_List result = S8_MakeEmptyList();
    for (S8_Node *it = a.first; it; it = it->next) S8_AddNode(allocator, &result, it->string);
    for (S8_Node *it = b.first; it; it = it->next) S8_AddNode(allocator, &result, it->string);
    return result;
}

S8_API S8_Node *S8_AddNode(S8_Allocator allocator, S8_List *list, S8_String string) {
    S8_Node *node = S8_CreateNode(allocator, string);
    S8_AddExistingNode(list, node);
    return node;
}

S8_API S8_Node *S8_Add(S8_Allocator allocator, S8_List *list, S8_String string) {
    S8_String copy = S8_Copy(allocator, string);
    S8_Node *node = S8_CreateNode(allocator, copy);
    S8_AddExistingNode(list, node);
    return node;
}

S8_API S8_String S8_AddF(S8_Allocator allocator, S8_List *list, const char *str, ...) {
    S8_FORMAT(allocator, str, result);
    S8_AddNode(allocator, list, result);
    return result;
}

#ifdef FIRST_UTF_HEADER

S8_API S16_String S8_ToWidecharEx(S8_Allocator allocator, S8_String string) {
    S8_ASSERT(sizeof(wchar_t) == 2);
    wchar_t *buffer = (wchar_t *)S8_ALLOCATE(allocator, sizeof(wchar_t) * (string.len + 1));
    int64_t size = UTF_CreateWidecharFromChar(buffer, string.len + 1, string.str, string.len);
    S16_String result = {buffer, size};
    return result;
}

S8_API wchar_t *S8_ToWidechar(S8_Allocator allocator, S8_String string) {
    S16_String result = S8_ToWidecharEx(allocator, string);
    return result.str;
}

S8_API S8_String S8_FromWidecharEx(S8_Allocator allocator, wchar_t *wstring, int64_t wsize) {
    S8_ASSERT(sizeof(wchar_t) == 2);

    int64_t buffer_size = (wsize + 1) * 2;
    char *buffer = (char *)S8_ALLOCATE(allocator, buffer_size);
    int64_t size = UTF_CreateCharFromWidechar(buffer, buffer_size, wstring, wsize);
    S8_String result = S8_Make(buffer, size);

    S8_ASSERT(size < buffer_size);
    return result;
}

S8_API S8_String S8_FromWidechar(S8_Allocator allocator, wchar_t *wstring) {
    int64_t size = S8_WideLength(wstring);
    S8_String result = S8_FromWidecharEx(allocator, wstring, size);
    return result;
}

#endif