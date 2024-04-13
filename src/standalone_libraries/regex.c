#include "regex.h"

#ifndef RE_ASSERT
    #include <assert.h>
    #define RE_ASSERT(x) assert(x)
#endif

#ifndef RE_STRICT_ASSERT
    #define RE_STRICT_ASSERT RE_ASSERT
#endif

#ifndef RE_MemoryZero
    #include <string.h>
    #define RE_MemoryZero(p, size) memset(p, 0, size)
#endif

typedef struct RE__Arena {
    char *buff;
    RE_Int len;
    RE_Int cap;
} RE_Arena;

struct RE_String {
    char *str;
    RE_Int len;
};

struct RE_Utf32Result {
    uint32_t out_str;
    int advance;
    int error;
};
static RE_Regex RE_NullRegex;
static char RE_NullChar;

struct RE_Parser {
    RE_String string;
    RE_Int i;
    RE_Regex *first;
    RE_Regex *last;
};
RE_API RE_Regex *RE1_ParseEx(RE_Arena *arena, char *string);
RE_API RE_Regex *RE2_ParseEx(RE_Arena *arena, char *string, RE_Int len);

RE_StaticFunc void *RE_PushSize(RE_Arena *arena, RE_Int size) {
    if (arena->len + size > arena->cap) {
        RE_ASSERT(!"RE_Regex: Not enough memory passed for this regex");
    }
    void *result = arena->buff + arena->len;
    arena->len += size;
    return result;
}

RE_StaticFunc RE_Arena RE_ArenaFromBuffer(char *buff, RE_Int size) {
    RE_Arena result;
    result.len = 0;
    result.cap = size;
    result.buff = buff;
    return result;
}

RE_StaticFunc RE_String RE_Skip(RE_String string, RE_Int len) {
    if (len > string.len) len = string.len;
    RE_Int remain = string.len - len;
    RE_String result;
    result.str = string.str + len;
    result.len = remain;
    return result;
}

RE_StaticFunc RE_Int RE_StringLength(char *string) {
    RE_Int len = 0;
    while (*string++ != 0) len++;
    return len;
}

RE_StaticFunc RE_Utf32Result RE_ConvertUTF8ToUTF32(char *c, int max_advance) {
    RE_Utf32Result result;
    RE_MemoryZero(&result, sizeof(result));

    if ((c[0] & 0x80) == 0) { // Check if leftmost zero of first byte is unset
        if (max_advance >= 1) {
            result.out_str = c[0];
            result.advance = 1;
        }
        else result.error = 1;
    }

    else if ((c[0] & 0xe0) == 0xc0) {
        if ((c[1] & 0xc0) == 0x80) { // Continuation byte required
            if (max_advance >= 2) {
                result.out_str = (uint32_t)(c[0] & 0x1f) << 6u | (c[1] & 0x3f);
                result.advance = 2;
            }
            else result.error = 2;
        }
        else result.error = 2;
    }

    else if ((c[0] & 0xf0) == 0xe0) {
        if ((c[1] & 0xc0) == 0x80 && (c[2] & 0xc0) == 0x80) { // Two continuation bytes required
            if (max_advance >= 3) {
                result.out_str = (uint32_t)(c[0] & 0xf) << 12u | (uint32_t)(c[1] & 0x3f) << 6u | (c[2] & 0x3f);
                result.advance = 3;
            }
            else result.error = 3;
        }
        else result.error = 3;
    }

    else if ((c[0] & 0xf8) == 0xf0) {
        if ((c[1] & 0xc0) == 0x80 && (c[2] & 0xc0) == 0x80 && (c[3] & 0xc0) == 0x80) { // Three continuation bytes required
            if (max_advance >= 4) {
                result.out_str = (uint32_t)(c[0] & 0xf) << 18u | (uint32_t)(c[1] & 0x3f) << 12u | (uint32_t)(c[2] & 0x3f) << 6u | (uint32_t)(c[3] & 0x3f);
                result.advance = 4;
            }
            else result.error = 4;
        }
        else result.error = 4;
    }
    else result.error = 4;

    return result;
}

#define RE_DLL_QUEUE_REMOVE(first, last, node) \
    do {                                       \
        if ((first) == (last)) {               \
            (first) = (last) = 0;              \
        }                                      \
        else if ((last) == (node)) {           \
            (last) = (last)->prev;             \
            (last)->next = 0;                  \
        }                                      \
        else if ((first) == (node)) {          \
            (first) = (first)->next;           \
            (first)->prev = 0;                 \
        }                                      \
        else {                                 \
            (node)->prev->next = (node)->next; \
            (node)->next->prev = (node)->prev; \
        }                                      \
        if (node) (node)->prev = 0;            \
    } while (0)

#define RE_DLL_QUEUE_ADD(f, l, node) \
    do {                             \
        if ((f) == 0) {              \
            (f) = (l) = (node);      \
            (node)->prev = 0;        \
            (node)->next = 0;        \
        }                            \
        else {                       \
            (l)->next = (node);      \
            (node)->prev = (l);      \
            (node)->next = 0;        \
            (l) = (node);            \
        }                            \
    } while (0)

RE_StaticFunc char *RE_GetP(RE_Parser *P) {
    if (P->i >= P->string.len) return &RE_NullChar;
    return P->string.str + P->i;
}

RE_StaticFunc char RE_Get(RE_Parser *P) {
    if (P->i >= P->string.len) return 0;
    return P->string.str[P->i];
}

RE_StaticFunc char RE_Get1(RE_Parser *P) {
    if ((P->i + 1) >= P->string.len || P->i >= P->string.len) return 0;
    return P->string.str[P->i + 1];
}

RE_StaticFunc void RE_Advance(RE_Parser *P) {
    if (P->i >= P->string.len) return;
    P->i += 1;
}

RE_StaticFunc RE_Regex *RE_ParseSingle(RE_Parser *P, RE_Arena *arena, RE_Regex **first, RE_Regex **last) {
    RE_Regex *regex = (RE_Regex *)RE_PushSize(arena, sizeof(RE_Regex));
    RE_MemoryZero(regex, sizeof(*regex));
    char *c = RE_GetP(P);
    RE_Int size_left = P->string.len - P->i;
    RE_Advance(P);
    switch (*c) {
        case ')': RE_STRICT_ASSERT(regex->kind != RE_MATCH_NULL && "Invalid regex syntax, ')' appeared without matching '('"); break;
        case '\0': RE_STRICT_ASSERT(regex->kind != RE_MATCH_NULL && "Invalid regex syntax, reached end of string obruptly"); break;
        case '.': regex->kind = RE_MATCH_ANY; break;
        case '^': regex->kind = RE_MATCH_FRONT; break;
        case '$': regex->kind = RE_MATCH_BACK; break;

        case '*': {
            if (*last) {
                regex->kind = RE_MATCH_ZERO_OR_MORE;
                RE_Regex *prev = *last;
                RE_DLL_QUEUE_REMOVE(*first, *last, *last);
                regex->child = prev;
            }
            else {
                RE_STRICT_ASSERT(!"Invalid regex syntax, '*' is not attached to anything");
            }
        } break;

        case '+': {
            if (*last) {
                regex->kind = RE_MATCH_ONE_OR_MORE;
                RE_Regex *prev = *last;
                RE_DLL_QUEUE_REMOVE(*first, *last, *last);
                regex->child = prev;
            }
            else {
                RE_STRICT_ASSERT(!"Invalid regex syntax, '+' is not attached to anything");
            }
        } break;

        case '?': {
            if (*last) {
                regex->kind = RE_MATCH_ZERO_OR_ONE;
                RE_Regex *prev = *last;
                RE_DLL_QUEUE_REMOVE(*first, *last, *last);
                regex->child = prev;
            }
            else {
                RE_STRICT_ASSERT(!"Invalid regex syntax, '?' is not attached to anything");
            }
        } break;

        case '[': {
            regex->kind = RE_MATCH_SELECTED;
            if (RE_Get(P) == '^') {
                regex->kind = RE_MATCH_NOT_SELECTED;
                RE_Advance(P);
            }
            while (RE_Get(P) != 0 && RE_Get(P) != ']') {
                RE_Regex *r = RE_ParseSingle(P, arena, &regex->group.first, &regex->group.last);
                if (r->kind == RE_MATCH_NULL) {
                    regex->kind = RE_MATCH_NULL;
                    break;
                }
                if (r->kind == RE_MATCH_WORD && RE_Get(P) == '-') {
                    char word = RE_Get1(P);
                    if (word >= '!' && word <= '~') {
                        RE_Advance(P);
                        RE_Regex *right = RE_ParseSingle(P, arena, 0, 0);
                        if (right->kind == RE_MATCH_NULL) {
                            regex->kind = RE_MATCH_NULL;
                            break;
                        }
                        RE_ASSERT(right->kind == RE_MATCH_WORD);
                        RE_ASSERT(right->word == word);
                        r->word_min = word > r->word ? r->word : word;
                        r->word_max = word > r->word ? word : r->word;
                        r->kind = RE_MATCH_RANGE;
                    }
                }
                RE_DLL_QUEUE_ADD(regex->group.first, regex->group.last, r);
            }
            RE_Advance(P);
        } break;

        case '(': {
            regex->kind = RE_MATCH_GROUP;
            while (RE_Get(P) != 0 && RE_Get(P) != ')') {
                RE_Regex *r = RE_ParseSingle(P, arena, &regex->group.first, &regex->group.last);
                if (r->kind == RE_MATCH_NULL) {
                    regex->kind = RE_MATCH_NULL;
                    break;
                }
                RE_DLL_QUEUE_ADD(regex->group.first, regex->group.last, r);
            }
            RE_Advance(P);
        } break;

        case '|': {
            if (*last) {
                regex->kind = RE_MATCH_OR;
                RE_Regex *left = *last;
                RE_Regex *right = RE_ParseSingle(P, arena, first, last);
                if (right->kind == RE_MATCH_NULL) {
                    regex->kind = RE_MATCH_NULL;
                    RE_STRICT_ASSERT(!"Invalid regex syntax, '|' appeared but it's right option is invalid");
                }
                else {
                    RE_DLL_QUEUE_REMOVE(*first, *last, left);
                    regex->left = left;
                    regex->right = right;
                }
            }
        } break;

        case '\\': {
            regex->kind = RE_MATCH_WORD;
            regex->word = RE_Get(P);
            switch (regex->word) {
                case 'n': regex->word = '\n'; break;
                case 't': regex->word = '\t'; break;
                case 'r': regex->word = '\r'; break;
                case 'w': regex->kind = RE_MATCH_ANY_WORD; break;
                case 'd': regex->kind = RE_MATCH_ANY_DIGIT; break;
                case 's': regex->kind = RE_MATCH_ANY_WHITESPACE; break;
                case '\0': {
                    regex->kind = RE_MATCH_NULL;
                    RE_STRICT_ASSERT(!"Invalid regex syntax, escape '\\' followed by end of string");
                } break;
            }
            RE_Advance(P);
        } break;

        default: {
            regex->kind = RE_MATCH_WORD;
            RE_Utf32Result decode = RE_ConvertUTF8ToUTF32(c, (int)size_left);
            if (decode.error) {
                regex->kind = RE_MATCH_NULL;
                RE_STRICT_ASSERT(!"Invalid regex syntax, string is an invalid utf8");
            }
            else {
                regex->word32 = decode.out_str;
                for (int i = 0; i < decode.advance - 1; i += 1)
                    RE_Advance(P);
            }
        }
    }

    return regex;
}

RE_StaticFunc RE_Int RE_MatchSingle(RE_Regex *regex, RE_String string) {
    switch (regex->kind) {
        case RE_MATCH_ZERO_OR_MORE: {
            RE_Int result = 0;
            for (; string.len;) {
                // @idea
                // In this case (asd)*(asd) we just quit with 0
                // when we meet asd
                // Maybe this should be collapsed in parsing stage/
                // asd should be combined with *asd etc. cause
                // now it's a bit weird but I dont know why you would
                // type that in the first place
                if (RE_MatchSingle(regex->next, string) != -1) break;
                RE_Int index = RE_MatchSingle(regex->child, string);
                if (index == -1) break;
                string = RE_Skip(string, index);
                result += index;
            }
            return result;
        } break;

        case RE_MATCH_ONE_OR_MORE: {
            RE_Int result = 0;
            for (; string.len;) {
                RE_Int index = RE_MatchSingle(regex->child, string);
                if (index == -1) break;
                string = RE_Skip(string, index);
                result += index;
            }

            if (result == 0) return -1;
            return result;
        } break;

        case RE_MATCH_OR: {
            RE_Int right = RE_MatchSingle(regex->right, string);
            RE_Int left = RE_MatchSingle(regex->left, string);
            if (left > right) return left;
            else return right;
        } break;

        case RE_MATCH_GROUP: {
            RE_Int result = 0;
            for (RE_Regex *it = regex->group.first; it; it = it->next) {
                if (string.len == 0) return -1;
                RE_Int index = RE_MatchSingle(it, string);
                if (index == -1) return -1;
                result += index;
                string = RE_Skip(string, index);
            }
            return result;
        } break;

        case RE_MATCH_NOT_SELECTED: {
            for (RE_Regex *it = regex->group.first; it; it = it->next) {
                RE_Int index = RE_MatchSingle(it, string);
                if (index != -1) return -1;
            }
            RE_Utf32Result decode = RE_ConvertUTF8ToUTF32(string.str, (int)string.len);
            if (decode.error) return -1;
            return decode.advance;
        } break;

        case RE_MATCH_SELECTED: {
            for (RE_Regex *it = regex->group.first; it; it = it->next) {
                RE_Int index = RE_MatchSingle(it, string);
                if (index != -1) return index;
            }
            return -1;
        } break;

        case RE_MATCH_RANGE: {
            if (string.str[0] >= regex->word_min && string.str[0] <= regex->word_max)
                return 1;
            return -1;
        }

        case RE_MATCH_ANY_WORD: {
            if ((string.str[0] >= 'a' && string.str[0] <= 'z') || (string.str[0] >= 'A' && string.str[0] <= 'Z'))
                return 1;
            return -1;
        } break;

        case RE_MATCH_ANY_DIGIT: {
            if (string.str[0] >= '0' && string.str[0] <= '9')
                return 1;
            return -1;
        } break;

        case RE_MATCH_ANY_WHITESPACE: {
            if (string.str[0] == ' ' || string.str[0] == '\n' || string.str[0] == '\t' || string.str[0] == '\r')
                return 1;
            return -1;
        } break;

        case RE_MATCH_ANY: {
            if (string.str[0] != '\n') {
                return 1;
            }
            return -1;
        } break;

        case RE_MATCH_ZERO_OR_ONE: {
            RE_Int index = RE_MatchSingle(regex->child, string);
            if (index == -1) index = 0;
            return index;
        } break;

        case RE_MATCH_WORD: {
            RE_Utf32Result decode = RE_ConvertUTF8ToUTF32(string.str, (int)string.len);
            if (decode.error) return -1;
            if (decode.out_str == regex->word32) return decode.advance;
            return -1;
        } break;

        case RE_MATCH_BACK:
        case RE_MATCH_NULL: return -1;

        default: RE_ASSERT(!"Invalid codepath");
    }
    return -1;
}

RE_API bool RE1_AreEqual(char *regex, char *string) {
    char buff[4096];
    RE_Regex *re = RE1_Parse(buff, sizeof(buff), regex);
    bool result = RE3_AreEqual(re, string, RE_StringLength(string));
    return result;
}

RE_API bool RE2_AreEqual(RE_Regex *regex, char *string) {
    return RE3_AreEqual(regex, string, RE_StringLength(string));
}

RE_API bool RE3_AreEqual(RE_Regex *regex, char *string, RE_Int len) {
    RE_Int result = RE3_MatchFront(regex, string, len, string);
    return result == len ? true : false;
}

RE_API RE_Match RE1_Find(char *regex, char *string) {
    char buff[4096];
    RE_Regex *re = RE1_Parse(buff, sizeof(buff), regex);
    RE_Match result = RE2_Find(re, string);
    return result;
}

RE_API RE_Match RE2_Find(RE_Regex *regex, char *string) {
    return RE3_Find(regex, string, RE_StringLength(string));
}

RE_API RE_Match RE3_Find(RE_Regex *regex, char *string, RE_Int len) {
    RE_Match result;
    for (RE_Int i = 0; i < len; i += 1) {
        result.size = RE3_MatchFront(regex, string + i, len - i, string);
        if (result.size != -1) {
            result.pos = i;
            return result;
        }
    }

    result.size = 0;
    result.pos = -1;
    return result;
}

RE_API RE_Match RE2_FindAgain(RE_Regex *regex, char *string, RE_Match prev_match) {
    return RE2_Find(regex, string + prev_match.pos);
}

RE_API RE_Match RE3_FindAgain(RE_Regex *regex, char *string, RE_Int len, RE_Match prev_match) {
    return RE3_Find(regex, string + prev_match.pos, len - prev_match.pos);
}

RE_API RE_Int RE3_MatchFront(RE_Regex *regex, char *string, RE_Int len, char *string_front) {
    RE_String re_string;
    re_string.str = string;
    re_string.len = len;
    RE_Int submatch_len = 0;
    for (RE_Regex *it = regex; it; it = it->next) {
        if (it->kind == RE_MATCH_FRONT) {
            if (re_string.str == string_front)
                continue;
            return -1;
        }
        if (it->kind == RE_MATCH_BACK) {
            if (re_string.len == 0)
                continue;
            return -1;
        }

        RE_Int index = RE_MatchSingle(it, re_string);
        if (index == -1) return -1;
        re_string = RE_Skip(re_string, index);
        submatch_len += index;
    }
    return submatch_len;
}

RE_API RE_Regex *RE1_ParseEx(RE_Arena *arena, char *string) {
    return RE2_ParseEx(arena, string, RE_StringLength(string));
}

RE_API RE_Regex *RE2_ParseEx(RE_Arena *arena, char *string, RE_Int len) {
    RE_Parser P;
    RE_MemoryZero(&P, sizeof(P));
    P.string.str = string;
    P.string.len = len;

    for (; P.i < P.string.len;) {
        RE_Regex *regex = RE_ParseSingle(&P, arena, &P.first, &P.last);
        RE_DLL_QUEUE_ADD(P.first, P.last, regex);
        if (regex->kind == RE_MATCH_NULL) {
            P.first = &RE_NullRegex;
            break;
        }
    }
    return P.first;
}

RE_API RE_Regex *RE1_Parse(char *buff, RE_Int buffsize, char *string) {
    RE_Arena arena = RE_ArenaFromBuffer(buff, buffsize);
    RE_Regex *result = RE1_ParseEx(&arena, string);
    return result;
}

RE_API RE_Regex *RE2_Parse(char *buff, RE_Int buffsize, char *string, RE_Int len) {
    RE_Arena arena = RE_ArenaFromBuffer(buff, buffsize);
    RE_Regex *result = RE2_ParseEx(&arena, string, len);
    return result;
}
