#include "clexer.h"
#include <stdarg.h>

/*
- I'm pretty sure I can remove allocations for most of the current functions.
- I also can fix ResolvePath stuff so that it uses string+len and doesn't need allocations
- Add lexing options like in stb_c_lexer.h

Instead of AND_CL_STRING_TERMINATE_ON_NEW_LINE he is doing some weird cool stuff with redefining
https://github.com/nothings/stb/blob/master/stb_c_lexer.h

CL_MULTILINE_SSTRINGS
CL_DOLLAR_IDENT

- Add proper string parsing, as additional function, CL_ParseString() or something, this is the only one that would need allocations

*/

#ifndef CL_PRIVATE_FUNCTION
    #if defined(__GNUC__) || defined(__clang__)
        #define CL_PRIVATE_FUNCTION __attribute__((unused)) static
    #else
        #define CL_PRIVATE_FUNCTION static
    #endif
#endif

#ifndef CL_Allocate
    #include <stdlib.h>
    #define CL_Allocate(allocator, size) malloc(size)
#endif

#ifndef CL_STRING_TO_DOUBLE
    #include <stdlib.h>
    #define CL_STRING_TO_DOUBLE(str, len) strtod(str, 0)
#endif

#ifndef CL_ASSERT
    #include <assert.h>
    #define CL_ASSERT(x) assert(x)
#endif

#ifndef CL_VSNPRINTF
    #include <stdio.h>
    #define CL_VSNPRINTF vsnprintf
#endif

#ifndef CL_SNPRINTF
    #include <stdio.h>
    #define CL_SNPRINTF snprintf
#endif

#ifndef CL__MemoryCopy
    #include <string.h>
    #define CL__MemoryCopy(dst, src, s) memcpy(dst, src, s)
#endif

#ifndef CL_MemoryZero
    #include <string.h>
    #define CL_MemoryZero(p, size) memset(p, 0, size)
#endif

#ifndef CL_FileExists
    #define CL_FileExists CL__FileExists
    #include <stdio.h>
CL_PRIVATE_FUNCTION bool CL_FileExists(char *name) {
    bool result = false;
    FILE *f = fopen(name, "rb");
    if (f) {
        result = true;
        fclose(f);
    }
    return result;
}
#endif

CL_PRIVATE_FUNCTION void CL_ReportError(CL_Lexer *T, CL_Token *token, const char *string, ...);

CL_PRIVATE_FUNCTION char *CL_PushStringCopy(CL_Allocator arena, char *p, int size) {
    char *copy_buffer = (char *)CL_Allocate(arena, size + 1);
    CL__MemoryCopy(copy_buffer, p, size);
    copy_buffer[size] = 0;
    return copy_buffer;
}

CL_INLINE void CL_Advance(CL_Lexer *T) {
    if (*T->stream == '\n') {
        T->line += 1;
        T->column = 0;
    }
    else if (*T->stream == ' ') {
        T->column += 1;
    }
    else if (*T->stream == '\t') {
        T->column += 1;
    }
    else if (*T->stream == 0) {
        return;
    }
    T->stream += 1;
}

CL_INLINE bool CL_IsAlphabetic(char c) {
    bool result = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    return result;
}

CL_INLINE bool CL_IsNumeric(char c) {
    bool result = (c >= '0' && c <= '9');
    return result;
}

CL_INLINE bool CL_IsHexNumeric(char c) {
    bool result = (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
    return result;
}

CL_INLINE bool CL_IsWhitespace(char c) {
    bool result = c == ' ' || c == '\n' || c == '\r' || c == '\t';
    return result;
}

CL_INLINE bool CL_IsAlphanumeric(char c) {
    bool result = CL_IsAlphabetic(c) || CL_IsNumeric(c);
    return result;
}

CL_API_FUNCTION void CL_SetTokenLength(CL_Lexer *T, CL_Token *token) {
    intptr_t diff = T->stream - token->str;
    CL_ASSERT(diff < 2147483647);
    token->len = (int)diff;
}

CL_PRIVATE_FUNCTION uint64_t CL_CharMapToNumber(char c) {
    switch (c) {
        case '0': return 0; break;
        case '1': return 1; break;
        case '2': return 2; break;
        case '3': return 3; break;
        case '4': return 4; break;
        case '5': return 5; break;
        case '6': return 6; break;
        case '7': return 7; break;
        case '8': return 8; break;
        case '9': return 9; break;
        case 'a':
        case 'A': return 10; break;
        case 'b':
        case 'B': return 11; break;
        case 'c':
        case 'C': return 12; break;
        case 'd':
        case 'D': return 13; break;
        case 'e':
        case 'E': return 14; break;
        case 'f':
        case 'F': return 15; break;
        default: return 255;
    }
}

CL_PRIVATE_FUNCTION uint64_t CL_ParseInteger(CL_Lexer *T, CL_Token *token, char *string, uint64_t len, uint64_t base) {
    CL_ASSERT(base >= 2 && base <= 16);
    uint64_t acc = 0;
    for (uint64_t i = 0; i < len; i++) {
        uint64_t num = CL_CharMapToNumber(string[i]);
        if (num >= base) {
            CL_ReportError(T, token, "Internal compiler error! Failed to parse a number");
            break;
        }
        acc *= base;
        acc += num;
    }
    return acc;
}

typedef struct CL_UTF32Result {
    uint32_t out_str;
    int advance;
    int error;
} CL_UTF32Result;

CL_PRIVATE_FUNCTION CL_UTF32Result CL_UTF8ToUTF32(char *c, int max_advance) {
    CL_UTF32Result result = {0};

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

// @todo I think I should look at this again
CL_PRIVATE_FUNCTION void CL_ParseCharLiteral(CL_Lexer *T, CL_Token *token) {
    token->kind = CL_CHARLIT;
    token->str = T->stream;
    while (*T->stream != '\'') {
        if (*T->stream == '\\') {
            CL_Advance(T);
        }
        if (*T->stream == 0) {
            CL_ReportError(T, token, "Unclosed character literal!");
            return;
        }
        CL_Advance(T);
    }
    CL_SetTokenLength(T, token);

    if (token->str[0] == '\\') {
        switch (token->str[1]) {
            case '\\': token->u64 = '\\'; break;
            case '\'': token->u64 = '\''; break;
            case '"': token->u64 = '"'; break;
            case 't': token->u64 = '\t'; break;
            case 'v': token->u64 = '\v'; break;
            case 'f': token->u64 = '\f'; break;
            case 'n': token->u64 = '\n'; break;
            case 'r': token->u64 = '\r'; break;
            case 'a': token->u64 = '\a'; break;
            case 'b': token->u64 = '\b'; break;
            case '0': token->u64 = '\0'; break;
            case 'x':
            case 'X': CL_ASSERT(!"Not implemented"); break; // Hex constant
            case 'u': CL_ASSERT(!"Not implemented"); break; // Unicode constant
            default: {
                CL_ReportError(T, token, "Unknown escape code");
            }
        }
    }

    else {
        if (token->len > 4) {
            CL_ReportError(T, token, "This character literal has invalid format, it's too big");
            goto skip_utf_encode;
        }

        token->u64 = 0;
        int i = 0;

        for (; i < token->len;) {
            CL_UTF32Result result = CL_UTF8ToUTF32(token->str + i, (int)token->len);
            i += result.advance;
            token->u64 |= result.out_str << (8 * (token->len - i));
            if (result.error) {
                CL_ReportError(T, token, "This character literal couldnt be parsed as utf8");
                break;
            }
        }
        if (i != token->len) {
            CL_ReportError(T, token, "Character literal decode error");
        }
    }

skip_utf_encode:
    CL_Advance(T);
}

// It combines strings, verifies the escape sequences but doesn't do any allocations
// so the final string actually needs additional transformation pass. A pass
// that will combine the string snippets, replace escape sequences with actual values etc.
//
// @warning: @not_sure: we are not setting token->string_literal
//
// "String 1"  "String 2" - those strings snippets are combined
// @todo: look at this again
// @todo: make a manual correct version that user can execute if he needs to
CL_PRIVATE_FUNCTION void CL_CheckString(CL_Lexer *T, CL_Token *token) {
    token->kind = CL_STRINGLIT;
combine_next_string_literal:
    while (*T->stream != '"' && *T->stream != 0 AND_CL_STRING_TERMINATE_ON_NEW_LINE) {
        if (*T->stream == '\\') {
            CL_Advance(T);
            switch (*T->stream) {
                case 'a':
                case 'b':
                case 'e':
                case 'f':
                case 'n':
                case 'r':
                case 't':
                case 'v':
                case '\\':
                case '\'':
                case '?':
                case '"':
                case 'x':
                case 'X': // Hex constant
                case 'u': // Unicode constant
                case 'U':
                    break;
                case '0': // octal numbers or null
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                    break;
                default: {
                    CL_ReportError(T, token, "Invalid escape sequence");
                    return;
                }
            }
        }
        CL_Advance(T);
    }
    CL_Advance(T);

    // Try to seek if there is a consecutive string.
    // If there is such string we try to combine it.
    {
        char *seek_for_next_string = T->stream;
        while (CL_IsWhitespace(*seek_for_next_string)) {
            seek_for_next_string += 1;
        }

        if (*seek_for_next_string == '"') {
            seek_for_next_string += 1;
            while (T->stream != seek_for_next_string) CL_Advance(T);
            goto combine_next_string_literal;
        }
    }
    CL_SetTokenLength(T, token);
}

CL_PRIVATE_FUNCTION void CL_IsIdentifierKeyword(CL_Token *token) {
    if (token->len == 1) return;
    char *c = token->str;
    switch (c[0]) {
        case 'v': {
            switch (c[1]) {
                case 'o': {
                    if (CL_StringsAreEqual(token->str, token->len, "void", 4)) {
                        token->kind = CL_KEYWORD_VOID;
                    }
                    else if (CL_StringsAreEqual(token->str, token->len, "volatile", 8)) {
                        token->kind = CL_KEYWORD_VOLATILE;
                    }
                } break;
            }
        } break;
        case 'i': {
            switch (c[1]) {
                case 'n': {
                    if (CL_StringsAreEqual(token->str, token->len, "int", 3)) {
                        token->kind = CL_KEYWORD_INT;
                    }
                    else if (CL_StringsAreEqual(token->str, token->len, "inline", 6)) {
                        token->kind = CL_KEYWORD_INLINE;
                    }
                } break;
                case 'f': {
                    if (CL_StringsAreEqual(token->str, token->len, "if", 2)) {
                        token->kind = CL_KEYWORD_IF;
                    }
                } break;
            }
        } break;
        case 'c': {
            switch (c[1]) {
                case 'h': {
                    if (CL_StringsAreEqual(token->str, token->len, "char", 4)) {
                        token->kind = CL_KEYWORD_CHAR;
                    }
                } break;
                case 'o': {
                    if (CL_StringsAreEqual(token->str, token->len, "const", 5)) {
                        token->kind = CL_KEYWORD_CONST;
                    }
                    else if (CL_StringsAreEqual(token->str, token->len, "continue", 8)) {
                        token->kind = CL_KEYWORD_CONTINUE;
                    }
                } break;
                case 'a': {
                    if (CL_StringsAreEqual(token->str, token->len, "case", 4)) {
                        token->kind = CL_KEYWORD_CASE;
                    }
                } break;
            }
        } break;
        case 'u': {
            switch (c[1]) {
                case 'n': {
                    if (CL_StringsAreEqual(token->str, token->len, "unsigned", 8)) {
                        token->kind = CL_KEYWORD_UNSIGNED;
                    }
                    else if (CL_StringsAreEqual(token->str, token->len, "union", 5)) {
                        token->kind = CL_KEYWORD_UNION;
                    }
                } break;
            }
        } break;
        case 's': {
            switch (c[1]) {
                case 'i': {
                    if (CL_StringsAreEqual(token->str, token->len, "signed", 6)) {
                        token->kind = CL_KEYWORD_SIGNED;
                    }
                    else if (CL_StringsAreEqual(token->str, token->len, "sizeof", 6)) {
                        token->kind = CL_KEYWORD_SIZEOF;
                    }
                } break;
                case 'h': {
                    if (CL_StringsAreEqual(token->str, token->len, "short", 5)) {
                        token->kind = CL_KEYWORD_SHORT;
                    }
                } break;
                case 't': {
                    if (CL_StringsAreEqual(token->str, token->len, "static", 6)) {
                        token->kind = CL_KEYWORD_STATIC;
                    }
                    else if (CL_StringsAreEqual(token->str, token->len, "struct", 6)) {
                        token->kind = CL_KEYWORD_STRUCT;
                    }
                } break;
                case 'w': {
                    if (CL_StringsAreEqual(token->str, token->len, "switch", 6)) {
                        token->kind = CL_KEYWORD_SWITCH;
                    }
                } break;
            }
        } break;
        case 'l': {
            switch (c[1]) {
                case 'o': {
                    if (CL_StringsAreEqual(token->str, token->len, "long", 4)) {
                        token->kind = CL_KEYWORD_LONG;
                    }
                } break;
            }
        } break;
        case 'd': {
            switch (c[1]) {
                case 'o': {
                    if (CL_StringsAreEqual(token->str, token->len, "double", 6)) {
                        token->kind = CL_KEYWORD_DOUBLE;
                    }
                    else if (CL_StringsAreEqual(token->str, token->len, "do", 2)) {
                        token->kind = CL_KEYWORD_DO;
                    }
                } break;
                case 'e': {
                    if (CL_StringsAreEqual(token->str, token->len, "default", 7)) {
                        token->kind = CL_KEYWORD_DEFAULT;
                    }
                } break;
            }
        } break;
        case 'f': {
            switch (c[1]) {
                case 'l': {
                    if (CL_StringsAreEqual(token->str, token->len, "float", 5)) {
                        token->kind = CL_KEYWORD_FLOAT;
                    }
                } break;
                case 'o': {
                    if (CL_StringsAreEqual(token->str, token->len, "for", 3)) {
                        token->kind = CL_KEYWORD_FOR;
                    }
                } break;
            }
        } break;
        case '_': {
            switch (c[1]) {
                case 'B': {
                    if (CL_StringsAreEqual(token->str, token->len, "_Bool", 5)) {
                        token->kind = CL_KEYWORD__BOOL;
                    }
                } break;
                case 'C': {
                    if (CL_StringsAreEqual(token->str, token->len, "_Complex", 8)) {
                        token->kind = CL_KEYWORD__COMPLEX;
                    }
                } break;
                case 'I': {
                    if (CL_StringsAreEqual(token->str, token->len, "_Imaginary", 10)) {
                        token->kind = CL_KEYWORD__IMAGINARY;
                    }
                } break;
                case 'T': {
                    if (CL_StringsAreEqual(token->str, token->len, "_Thread_local", 13)) {
                        token->kind = CL_KEYWORD__THREAD_LOCAL;
                    }
                } break;
                case 'A': {
                    if (CL_StringsAreEqual(token->str, token->len, "_Atomic", 7)) {
                        token->kind = CL_KEYWORD__ATOMIC;
                    }
                    else if (CL_StringsAreEqual(token->str, token->len, "_Alignas", 8)) {
                        token->kind = CL_KEYWORD__ALIGNAS;
                    }
                    else if (CL_StringsAreEqual(token->str, token->len, "_Alignof", 8)) {
                        token->kind = CL_KEYWORD__ALIGNOF;
                    }
                } break;
                case 'N': {
                    if (CL_StringsAreEqual(token->str, token->len, "_Noreturn", 9)) {
                        token->kind = CL_KEYWORD__NORETURN;
                    }
                } break;
                case 'S': {
                    if (CL_StringsAreEqual(token->str, token->len, "_Static_assert", 14)) {
                        token->kind = CL_KEYWORD__STATIC_ASSERT;
                    }
                } break;
                case 'G': {
                    if (CL_StringsAreEqual(token->str, token->len, "_Generic", 8)) {
                        token->kind = CL_KEYWORD__GENERIC;
                    }
                } break;
            }
        } break;
        case 'a': {
            switch (c[1]) {
                case 'u': {
                    if (CL_StringsAreEqual(token->str, token->len, "auto", 4)) {
                        token->kind = CL_KEYWORD_AUTO;
                    }
                } break;
            }
        } break;
        case 'e': {
            switch (c[1]) {
                case 'x': {
                    if (CL_StringsAreEqual(token->str, token->len, "extern", 6)) {
                        token->kind = CL_KEYWORD_EXTERN;
                    }
                } break;
                case 'n': {
                    if (CL_StringsAreEqual(token->str, token->len, "enum", 4)) {
                        token->kind = CL_KEYWORD_ENUM;
                    }
                } break;
                case 'l': {
                    if (CL_StringsAreEqual(token->str, token->len, "else", 4)) {
                        token->kind = CL_KEYWORD_ELSE;
                    }
                } break;
            }
        } break;
        case 'r': {
            switch (c[1]) {
                case 'e': {
                    if (CL_StringsAreEqual(token->str, token->len, "register", 8)) {
                        token->kind = CL_KEYWORD_REGISTER;
                    }
                    else if (CL_StringsAreEqual(token->str, token->len, "restrict", 8)) {
                        token->kind = CL_KEYWORD_RESTRICT;
                    }
                    else if (CL_StringsAreEqual(token->str, token->len, "return", 6)) {
                        token->kind = CL_KEYWORD_RETURN;
                    }
                } break;
            }
        } break;
        case 't': {
            switch (c[1]) {
                case 'y': {
                    if (CL_StringsAreEqual(token->str, token->len, "typedef", 7)) {
                        token->kind = CL_KEYWORD_TYPEDEF;
                    }
                } break;
            }
        } break;
        case 'b': {
            switch (c[1]) {
                case 'r': {
                    if (CL_StringsAreEqual(token->str, token->len, "break", 5)) {
                        token->kind = CL_KEYWORD_BREAK;
                    }
                } break;
            }
        } break;
        case 'w': {
            switch (c[1]) {
                case 'h': {
                    if (CL_StringsAreEqual(token->str, token->len, "while", 5)) {
                        token->kind = CL_KEYWORD_WHILE;
                    }
                } break;
            }
        } break;
        case 'g': {
            switch (c[1]) {
                case 'o': {
                    if (CL_StringsAreEqual(token->str, token->len, "goto", 4)) {
                        token->kind = CL_KEYWORD_GOTO;
                    }
                } break;
            }
        } break;
    }
}

CL_PRIVATE_FUNCTION void CL_EatMacroWhitespace(CL_Lexer *T) {
    while (T->stream[0] == ' ' || T->stream[0] == '\t') CL_Advance(T);
}

CL_PRIVATE_FUNCTION void CL_EatUntil(CL_Lexer *T, char c) {
    while (T->stream[0] != c && T->stream[0] != 0) CL_Advance(T);
}

CL_PRIVATE_FUNCTION void CL_LexMacroInclude(CL_Lexer *T, CL_Token *token) {
    token->kind = CL_PREPROC_INCLUDE;
    CL_EatMacroWhitespace(T);
    char end = 0;
    if (*T->stream == '"') {
        end = '"';
    }
    else if (*T->stream == '<') {
        end = '>';
        token->is_system_include = true;
    }
    else {
        CL_ReportError(T, token, "Invalid include directive, file not specified");
        return;
    }
    CL_Advance(T);

    token->str = T->stream;
    while (*T->stream != end) {
        if (*T->stream == 0) {
            CL_ReportError(T, token, "Invalid include directive, reached end of file while reading filename");
        }
        if (*T->stream == '\n') {
            CL_ReportError(T, token, "Invalid include directive filename, got newline character while reading filename");
        }
        CL_Advance(T);
    }
    CL_SetTokenLength(T, token);
    CL_Advance(T);

    // @not_sure: this is because we want null terminated input into path resolution stuff
    token->string_literal = CL_PushStringCopy(T->arena, token->str, token->len);
}

CL_PRIVATE_FUNCTION bool CL_LexMacro(CL_Lexer *T, CL_Token *token) {
    CL_EatMacroWhitespace(T);
    token->str = T->stream;
    while (CL_IsAlphabetic(*T->stream)) CL_Advance(T);
    CL_SetTokenLength(T, token);

    switch (*token->str) {
        case 'd':
            if (CL_StringsAreEqual(token->str, token->len, "define", 6)) {
                token->kind = CL_PREPROC_DEFINE;
            }
            break;

        case 'i':
            if (CL_StringsAreEqual(token->str, token->len, "ifdef", 5)) {
                token->kind = CL_PREPROC_IFDEF;
            }
            else if (CL_StringsAreEqual(token->str, token->len, "ifndef", 6)) {
                token->kind = CL_PREPROC_IFNDEF;
            }
            else if (CL_StringsAreEqual(token->str, token->len, "include", 7)) {
                token->kind = CL_PREPROC_INCLUDE;
                CL_LexMacroInclude(T, token);
            }
            else if (CL_StringsAreEqual(token->str, token->len, "if", 2)) {
                token->kind = CL_PREPROC_IF;
            }
            break;

        case 'e':
            if (CL_StringsAreEqual(token->str, token->len, "endif", 5)) {
                token->kind = CL_PREPROC_ENDIF;
            }
            else if (CL_StringsAreEqual(token->str, token->len, "error", 5)) {
                token->kind = CL_PREPROC_ERROR;
                CL_EatMacroWhitespace(T);
                token->str = T->stream;
                CL_EatUntil(T, '\n');
                CL_SetTokenLength(T, token);
            }
            else if (CL_StringsAreEqual(token->str, token->len, "else", 4)) {
                token->kind = CL_PREPROC_ELSE;
            }
            else if (CL_StringsAreEqual(token->str, token->len, "elif", 4)) {
                token->kind = CL_PREPROC_ELIF;
            }
            break;

        case 'p':
            if (CL_StringsAreEqual(token->str, token->len, "pragma", 6)) {
                token->kind = CL_PREPROC_PRAGMA;
            }
            break;

        case 'u':
            if (CL_StringsAreEqual(token->str, token->len, "undef", 5)) {
                token->kind = CL_PREPROC_UNDEF;
            }
            break;
        default: return false;
    }
    return true;
}

// Skipped space here is for case #define Memes (a), this is not a function like macro because of space
static uint32_t CL_TokenID; // @todo: make it read only
CL_PRIVATE_FUNCTION void CL_PrepareToken(CL_Lexer *T, CL_Token *token, bool skipped_space) {
    CL_MemoryZero(token, sizeof(*token));
    token->str = T->stream;
    token->line = T->line;
    token->column = T->column;
    token->file = T->file;
    token->id = ++CL_TokenID;
    if (skipped_space) token->is_there_whitespace_before_token = true;
    CL_Advance(T);
}

CL_PRIVATE_FUNCTION void CL_DefaultTokenize(CL_Lexer *T, CL_Token *token) {
    char *c = token->str;
    switch (*c) {
        case 0: break;
        case '(': token->kind = CL_OPENPAREN; break;
        case ')': token->kind = CL_CLOSEPAREN; break;
        case '{': token->kind = CL_OPENBRACE; break;
        case '}': token->kind = CL_CLOSEBRACE; break;
        case '[': token->kind = CL_OPENBRACKET; break;
        case ']': token->kind = CL_CLOSEBRACKET; break;
        case ',': token->kind = CL_COMMA; break;
        case '~': token->kind = CL_NEG; break;
        case '?': token->kind = CL_QUESTION; break;
        case ';': token->kind = CL_SEMICOLON; break;
        case ':': token->kind = CL_COLON; break;
        case '.': {
            token->kind = CL_DOT;
            if (T->stream[0] == '.' && T->stream[1] == '.') {
                CL_Advance(T);
                CL_Advance(T);
                token->kind = CL_THREEDOTS;
            }
        } break;
        case '/': {
            token->kind = CL_DIV;
            if (*T->stream == '/') {
                token->kind = CL_COMMENT;
                CL_Advance(T);
                CL_EatUntil(T, '\n');
                CL_SetTokenLength(T, token);
            }
            else if (*T->stream == '*') {
                token->kind = CL_COMMENT;
                CL_Advance(T);
                for (;;) {
                    if (T->stream[0] == '*' && T->stream[1] == '/') {
                        break;
                    }
                    if (T->stream[0] == 0) {
                        CL_ReportError(T, token, "Unclosed block comment");
                        goto error_end_path;
                    }
                    CL_Advance(T);
                }
                token->str += 2;
                CL_SetTokenLength(T, token);
                CL_Advance(T);
                CL_Advance(T);
            }
            else if (*T->stream == '=') {
                token->kind = CL_DIVASSIGN;
                CL_Advance(T);
            }
        } break;
        case '#': {
            if (*T->stream == '#') {
                token->kind = CL_MACRO_CONCAT;
                CL_Advance(T);
            }
            else {
                bool is_macro_directive = CL_LexMacro(T, token);
                if (is_macro_directive) {
                    T->inside_of_macro = true;
                }
                else {
                    if (!T->inside_of_macro) {
                        CL_ReportError(T, token, "Invalid preprocessor directive");
                        goto error_end_path;
                    }

                    token->kind = CL_PREPROC_STRINGIFY;
                    token->str = T->stream;
                    while (*T->stream == '_' || CL_IsAlphanumeric(*T->stream))
                        CL_Advance(T);
                    CL_SetTokenLength(T, token);
                }
            }
        } break;
        case '>': {
            if (*T->stream == '=') {
                token->kind = CL_GREATERTHEN_OR_EQUAL;
                CL_Advance(T);
            }
            else if (*T->stream == '>') {
                CL_Advance(T);
                if (*T->stream == '=') {
                    CL_Advance(T);
                    token->kind = CL_RIGHTSHIFTASSIGN;
                }
                else {
                    token->kind = CL_RIGHTSHIFT;
                }
            }
            else {
                token->kind = CL_GREATERTHEN;
            }
        } break;
        case '<': {
            token->kind = CL_LESSERTHEN;
            if (*T->stream == '=') {
                token->kind = CL_LESSERTHEN_OR_EQUAL;
                CL_Advance(T);
            }
            else if (*T->stream == '<') {
                CL_Advance(T);
                if (*T->stream == '=') {
                    CL_Advance(T);
                    token->kind = CL_LEFTSHIFTASSIGN;
                }
                else {
                    token->kind = CL_LEFTSHIFT;
                }
            }
        } break;
        case '&': {
            if (*T->stream == '=') {
                token->kind = CL_ANDASSIGN;
                CL_Advance(T);
            }
            else if (*T->stream == '&') {
                token->kind = CL_AND;
                CL_Advance(T);
            }
            else {
                token->kind = CL_BITAND;
            }
        } break;
        case '-': {
            if (*T->stream == '-') {
                token->kind = CL_DECREMENT;
                CL_Advance(T);
            }
            else if (*T->stream == '=') {
                token->kind = CL_SUBASSIGN;
                CL_Advance(T);
            }
            else {
                token->kind = CL_SUB;
            }
        } break;
        case '+': {
            if (*T->stream == '+') {
                token->kind = CL_INCREMENT;
                CL_Advance(T);
            }
            else if (*T->stream == '=') {
                token->kind = CL_ADDASSIGN;
                CL_Advance(T);
            }
            else {
                token->kind = CL_ADD;
            }
        } break;
        case '|': {
            if (*T->stream == '|') {
                token->kind = CL_OR;
                CL_Advance(T);
            }
            else if (*T->stream == '=') {
                token->kind = CL_ORASSIGN;
                CL_Advance(T);
            }
            else {
                token->kind = CL_BITOR;
            }
        } break;
        case '=': {
            if (*T->stream != '=') {
                token->kind = CL_ASSIGN;
            }
            else {
                CL_Advance(T);
                token->kind = CL_EQUALS;
            }
        } break;
        case '!': {
            if (*T->stream != '=') {
                token->kind = CL_NOT;
            }
            else {
                CL_Advance(T);
                token->kind = CL_NOTEQUALS;
            }
        } break;
        case '*': {
            token->kind = CL_MUL;
            if (*T->stream == '=') {
                CL_Advance(T);
                token->kind = CL_MULASSIGN;
            }
        } break;
        case '%': {
            token->kind = CL_MOD;
            if (*T->stream == '=') {
                token->kind = CL_MODASSIGN;
                CL_Advance(T);
            }
        } break;
        case '^': {
            token->kind = CL_BITXOR;
            if (*T->stream == '=') {
                CL_Advance(T);
                token->kind = CL_XORASSIGN;
            }
        } break;
        case '"': {
            CL_CheckString(T, token);
        } break;
        case '\'': {
            CL_ParseCharLiteral(T, token);
        } break;
        case 'U': { // @todo Unicode32
            if (*T->stream == '"') {
                token->fix = CL_PREFIX_U32;
                CL_Advance(T);
                CL_CheckString(T, token);
            }
            else if (*T->stream == '\'') {
                token->fix = CL_PREFIX_U32;
                CL_Advance(T);
                CL_ParseCharLiteral(T, token);
            }
            else goto parse_regular_char;
        } break;
        case 'u': {                        // Unicode16
            if (*T->stream == '8') {       // Unicode8
                if (T->stream[1] == '"') { // U8 STRING
                    token->fix = CL_PREFIX_U8;
                    CL_Advance(T);
                    CL_Advance(T);
                    CL_CheckString(T, token);
                }
                else if (T->stream[1] == '\'') { // U8 CHAR
                    token->fix = CL_PREFIX_U8;
                    CL_Advance(T);
                    CL_Advance(T);
                    CL_ParseCharLiteral(T, token);
                }
                else goto parse_regular_char;
            }
            else if (*T->stream == '"') { // U16 STRING
                token->fix = CL_PREFIX_U16;
                CL_Advance(T);
                CL_CheckString(T, token);
            }
            else if (*T->stream == '\'') { // U16 CHAR
                CL_Advance(T);
                CL_ParseCharLiteral(T, token);
            }
            else goto parse_regular_char;
        }
        case 'L': { // Widechar
            if (*T->stream == '"') {
                token->fix = CL_PREFIX_L;
                CL_Advance(T);
                CL_CheckString(T, token); // @todo UTF16
            }
            else if (*T->stream == '\'') {
                token->fix = CL_PREFIX_L;
                CL_Advance(T);
                CL_ParseCharLiteral(T, token);
            }
            else goto parse_regular_char;
        } break;
        case 'A':
        case 'a':
        case 'B':
        case 'b':
        case 'C':
        case 'c':
        case 'D':
        case 'd':
        case 'E':
        case 'e':
        case 'F':
        case 'f':
        case 'G':
        case 'g':
        case 'H':
        case 'h':
        case 'I':
        case 'i':
        case 'J':
        case 'j':
        case 'K':
        case 'k':
        /*case 'L':*/ case 'l':
        case 'M':
        case 'm':
        case 'N':
        case 'n':
        case 'O':
        case 'o':
        case 'P':
        case 'p':
        case 'Q':
        case 'q':
        case 'R':
        case 'r':
        case 'S':
        case 's':
        case 'T':
        case 't':
        // case 'U': case 'u':
        case 'V':
        case 'v':
        case 'W':
        case 'w':
        case 'X':
        case 'x':
        case 'Y':
        case 'y':
        case 'Z':
        case 'z':
        case '_':
        parse_regular_char : {
            token->kind = CL_IDENTIFIER;
            while (*T->stream == '_' || CL_IsAlphanumeric(*T->stream)) {
                CL_Advance(T);
            }
            CL_SetTokenLength(T, token);
            CL_IsIdentifierKeyword(token);
        } break;
        case '0': {
            if (*T->stream == 'x' || *T->stream == 'X') {
                token->kind = CL_INT;
                token->is_hex = true;
                CL_Advance(T);
                while (CL_IsHexNumeric(*T->stream)) {
                    CL_Advance(T);
                }
                uint64_t len = T->stream - token->str;
                CL_ASSERT(len > 2);
                token->u64 = CL_ParseInteger(T, token, token->str + 2, len - 2, 16);
                break;
            }
        }
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            token->kind = CL_INT;
            for (;;) {
                if (*T->stream == '.') {
                    if (token->kind == CL_FLOAT) {
                        CL_ReportError(T, token, "Failed to parse a floating point number, invalid format, found multiple '.'");
                    }

                    if (token->kind == CL_INT) {
                        token->kind = CL_FLOAT;
                    }
                }
                else if (CL_IsNumeric(*T->stream) == false) {
                    break;
                }
                CL_Advance(T);
            }

            if (token->kind == CL_INT) {
                uint64_t len = T->stream - token->str;
                CL_ASSERT(len > 0);
                token->u64 = CL_ParseInteger(T, token, token->str, len, 10);
            }

            else if (token->kind == CL_FLOAT) {
                token->f64 = CL_STRING_TO_DOUBLE(token->str, token->len);
            }

            else {
                CL_ASSERT(token->kind == CL_ERROR);
            }

            if (*T->stream == 'f' || *T->stream == 'F') {
                CL_Advance(T);
                token->fix = CL_SUFFIX_F;
            }

            else if (*T->stream == 'l' || *T->stream == 'L') {
                CL_Advance(T);
                token->fix = CL_SUFFIX_L;
                if (*T->stream == 'l' || *T->stream == 'L') {
                    CL_Advance(T);
                    token->fix = CL_SUFFIX_LL;
                    if (*T->stream == 'u' || *T->stream == 'U') {
                        CL_Advance(T);
                        token->fix = CL_SUFFIX_ULL;
                    }
                }
                else if (*T->stream == 'u' || *T->stream == 'U') {
                    CL_Advance(T);
                    token->fix = CL_SUFFIX_UL;
                }
            }

            else if (*T->stream == 'u' || *T->stream == 'U') {
                CL_Advance(T);
                token->fix = CL_SUFFIX_U;
                if (*T->stream == 'l' || *T->stream == 'L') {
                    CL_Advance(T);
                    token->fix = CL_SUFFIX_UL;
                    if (*T->stream == 'l' || *T->stream == 'L') {
                        CL_Advance(T);
                        token->fix = CL_SUFFIX_ULL;
                    }
                }
            }

        } break;

        default: {
            CL_ReportError(T, token, "Unhandled character, skipping ...");
        } break;
    }

error_end_path:;
}

CL_PRIVATE_FUNCTION bool CL_EatWhitespace(CL_Lexer *T) {
    bool skipped = false;
    for (;;) {
        if (CL_IsWhitespace(*T->stream)) {
            if (*T->stream == '\n') T->inside_of_macro = false;
            CL_Advance(T);
            skipped = true;
        }
        else if (T->stream[0] == '\\' && T->stream[1] == '\n') {
            CL_Advance(T);
            CL_Advance(T);
            skipped = true;
        }
        else if (T->stream[0] == '\\' && T->stream[1] == '\r' && T->stream[2] == '\n') {
            CL_Advance(T);
            CL_Advance(T);
            CL_Advance(T);
            skipped = true;
        }
        else {
            break;
        }
    }
    return skipped;
}

CL_PRIVATE_FUNCTION void CL_TryToFinalizeToken(CL_Lexer *T, CL_Token *token) {
    if (!token->len) {
        CL_SetTokenLength(T, token);
    }
    if (T->inside_of_macro) {
        token->is_inside_macro = true;
    }
}

CL_PRIVATE_FUNCTION void CL_InitNextToken(CL_Lexer *T, CL_Token *token) {
    // Skip comments, comments get allocated on perm and gathered on the Tokenizer.
    // First non comment token gets those comments attached.
    for (;;) {
        bool skipped = CL_EatWhitespace(T);
        CL_PrepareToken(T, token, skipped);
        CL_DefaultTokenize(T, token);

        if (token->kind == CL_EOF) {
            break;
        }

        if (T->select_includes) {
            if (token->kind != CL_PREPROC_INCLUDE) continue;
        }

        if (T->select_macros) {
            if (!token->is_inside_macro) continue;
        }

        if (T->select_comments) {
            if (token->kind != CL_COMMENT) continue;
        }

        if (T->skip_comments) {
            if (token->kind == CL_COMMENT) continue;
        }

        if (T->skip_macros) {
            if (token->is_inside_macro) continue;
        }

        break;
    }
    CL_TryToFinalizeToken(T, token);
}

CL_API_FUNCTION CL_Token CL_Next(CL_Lexer *T) {
    CL_Token result;
    CL_MemoryZero(&result, sizeof(CL_Token));
    CL_InitNextToken(T, &result);
    return result;
}

CL_API_FUNCTION CL_Lexer CL_Begin(CL_Allocator arena, char *stream, char *filename) {
    CL_Lexer lexer = {0};
    lexer.stream = lexer.stream_begin = stream;
    lexer.file = filename;
    lexer.arena = arena;
    lexer.skip_comments = true;
    return lexer;
}

//
//
//

CL_PRIVATE_FUNCTION char *CL_ChopLastSlash(CL_Allocator arena, char *str) {
    int i = 0;
    int slash_pos = -1;
    while (str[i]) {
        if (str[i] == '/') {
            slash_pos = i;
        }
        i += 1;
    }

    char *result = str;
    if (slash_pos != -1) {
        result = CL_PushStringCopy(arena, str, slash_pos);
    }
    else {
        result = (char *)"./";
    }
    return result;
}

CL_PRIVATE_FUNCTION char *CL_JoinPath(CL_Allocator arena, char *a, char *b) {
    int alen = CL_StringLength(a);
    int blen = CL_StringLength(b);
    int additional_len = 0;

    if (alen && a[alen - 1] != '/') additional_len = 1;
    char *result = (char *)CL_Allocate(arena, sizeof(char) * (alen + blen + 1 + additional_len));
    CL__MemoryCopy(result, a, alen);
    if (additional_len) result[alen++] = '/';
    CL__MemoryCopy(result + alen, b, blen);
    result[alen + blen] = 0;
    return result;
}

CL_PRIVATE_FUNCTION bool CL_IsAbsolutePath(char *path) {
#if _WIN32
    bool result = CL_IsAlphabetic(path[0]) && path[1] == ':' && path[2] == '/';
#else
    bool result = path[0] == '/';
#endif
    return result;
}

CL_PRIVATE_FUNCTION char *CL_SkipToLastSlash(char *p) {
    int last_slash = 0;
    for (int i = 0; p[i]; i += 1) {
        if (p[i] == '/') last_slash = i;
    }
    return p + last_slash;
}

CL_API_FUNCTION char *CL_ResolveFilepath(CL_Allocator arena, CL_SearchPaths *search_paths, char *filename, char *parent_file, bool is_system_include) {
    CL_SearchPaths null_search_paths = {0};
    if (search_paths == 0) search_paths = &null_search_paths;

    if (search_paths->file_begin_to_ignore) {
        char *name = CL_SkipToLastSlash(filename);
        int namelen = CL_StringLength(name);
        char *ignore = search_paths->file_begin_to_ignore;
        int ignorelen = CL_StringLength(ignore);
        if (namelen > ignorelen) {
            namelen = ignorelen;
        }
        if (CL_StringsAreEqual(name, namelen, search_paths->file_begin_to_ignore, ignorelen)) {
            return 0;
        }
    }

    if (CL_IsAbsolutePath(filename) && CL_FileExists(filename)) {
        return filename;
    }

    if (is_system_include) {
        for (int path_i = 0; path_i < search_paths->system_include_path_count; path_i += 1) {
            char *path_it = search_paths->system_include_path[path_i];
            char *file = CL_JoinPath(arena, path_it, filename);
            if (CL_FileExists(file)) {
                return file;
            }
        }
    }
    else {
        if (parent_file) {
            char *parent_dir = CL_ChopLastSlash(arena, parent_file);
            char *file = CL_JoinPath(arena, parent_dir, filename);
            if (CL_FileExists(file)) {
                return file;
            }
        }

        for (int path_i = 0; path_i < search_paths->include_path_count; path_i += 1) {
            char *path_it = search_paths->include_path[path_i];
            char *file = CL_JoinPath(arena, path_it, filename);
            if (CL_FileExists(file)) {
                return file;
            }
        }
    }
    return 0;
}

//
//
//

const char *CL_FixString[] = {
    "SUFFIX_INVALID",
    "SUFFIX_U",
    "SUFFIX_UL",
    "SUFFIX_ULL",
    "SUFFIX_L",
    "SUFFIX_LL",
    "SUFFIX_F",
    "SUFFIX_FL",
    "PREFIX_U8",
    "PREFIX_U16",
    "PREFIX_U32",
    "PREFIX_L",
};

const char *CL_KindString[] = {
    "EOF",
    "*",
    "/",
    "%",
    "<<",
    ">>",
    "+",
    "-",
    "==",
    "<",
    ">",
    "<=",
    ">=",
    "!=",
    "&",
    "|",
    "^",
    "&&",
    "||",
    "~",
    "!",
    "--",
    "++",
    "--",
    "++",
    "=",
    "/=",
    "*=",
    "%=",
    "-=",
    "+=",
    "&=",
    "|=",
    "^=",
    "<<=",
    ">>=",
    "(",
    ")",
    "{",
    "}",
    "[",
    "]",
    ",",
    "##",
    "#Stringify",
    "?",
    "...",
    ";",
    ".",
    ":",
    "TAG",
    "->",
    "SIZEOF",
    "DOCCOMMENT",
    "COMMENT",
    "IDENTIFIER",
    "STRING_LITERAL",
    "CHARACTER_LITERAL",
    "ERROR TOKEN",
    "FLOAT",
    "INT",
    "PREPROC_NULL",
    "PREPROC_DEFINE",
    "PREPROC_IFDEF",
    "PREPROC_IFNDEF",
    "PREPROC_INCLUDE",
    "PREPROC_ENDIF",
    "PREPROC_IF",
    "PREPROC_PRAGMA",
    "PREPROC_ERROR",
    "PREPROC_ELSE",
    "PREPROC_ELIF",
    "PREPROC_UNDEF",
    "KEYWORD_VOID",
    "KEYWORD_INT",
    "KEYWORD_CHAR",
    "KEYWORD_UNSIGNED",
    "KEYWORD_SIGNED",
    "KEYWORD_LONG",
    "KEYWORD_SHORT",
    "KEYWORD_DOUBLE",
    "KEYWORD_FLOAT",
    "KEYWORD__BOOL",
    "KEYWORD__COMPLEX",
    "KEYWORD__IMAGINARY",
    "KEYWORD_STATIC",
    "KEYWORD_AUTO",
    "KEYWORD_CONST",
    "KEYWORD_EXTERN",
    "KEYWORD_INLINE",
    "KEYWORD_REGISTER",
    "KEYWORD_RESTRICT",
    "KEYWORD_VOLATILE",
    "KEYWORD__THREAD_LOCAL",
    "KEYWORD__ATOMIC",
    "KEYWORD__NORETURN",
    "KEYWORD_STRUCT",
    "KEYWORD_UNION",
    "KEYWORD_ENUM",
    "KEYWORD_TYPEDEF",
    "KEYWORD_DEFAULT",
    "KEYWORD_BREAK",
    "KEYWORD_RETURN",
    "KEYWORD_SWITCH",
    "KEYWORD_IF",
    "KEYWORD_ELSE",
    "KEYWORD_FOR",
    "KEYWORD_WHILE",
    "KEYWORD_CASE",
    "KEYWORD_CONTINUE",
    "KEYWORD_DO",
    "KEYWORD_GOTO",
    "KEYWORD_SIZEOF",
    "KEYWORD__ALIGNAS",
    "KEYWORD__ALIGNOF",
    "KEYWORD__STATIC_ASSERT",
    "KEYWORD__GENERIC",
};

CL_API_FUNCTION void CL_StringifyMessage(char *buff, int buff_size, CL_Message *msg) {
    CL_SNPRINTF(buff, buff_size, "%s(%d,%d): %15s", msg->token.file, msg->token.line + 1, msg->token.column + 1, msg->string);
}

CL_API_FUNCTION void CL_Stringify(char *buff, int buff_size, CL_Token *token) {
    const char *token_kind = "UNKNOWN";
    if (token->kind < CL_COUNT) token_kind = CL_KindString[token->kind];
    CL_SNPRINTF(buff, buff_size, "%s(%d,%d): %15s %15.*s", token->file, token->line + 1, token->column + 1, token_kind, token->len, token->str);
}

#define CL_SLL_QUEUE_ADD_MOD(f, l, n, next) \
    do {                                    \
        (n)->next = 0;                      \
        if ((f) == 0) {                     \
            (f) = (l) = (n);                \
        }                                   \
        else {                              \
            (l) = (l)->next = (n);          \
        }                                   \
    } while (0)
#define CL_SLL_QUEUE_ADD(f, l, n) CL_SLL_QUEUE_ADD_MOD(f, l, n, next)

#define CL__FORMAT(arena, string, result)                 \
    va_list args1, args2;                                 \
    va_start(args1, string);                              \
    va_copy(args2, args1);                                \
    int len = CL_VSNPRINTF(0, 0, string, args2);          \
    va_end(args2);                                        \
    char *result = (char *)CL_Allocate((arena), len + 1); \
    CL_VSNPRINTF(result, len + 1, string, args1);         \
    va_end(args1)

CL_PRIVATE_FUNCTION void CL_ReportError(CL_Lexer *T, CL_Token *token, const char *string, ...) {
    CL__FORMAT(T->arena, string, message_string);
    CL_Message *result = (CL_Message *)CL_Allocate(T->arena, sizeof(CL_Message));
    CL_MemoryZero(result, sizeof(CL_Message));
    CL_SLL_QUEUE_ADD(T->first_message, T->last_message, result);

    result->string = (char *)string;
    result->token = *token;
    token->kind = CL_ERROR;
    token->error = result;
    T->errors += 1;
}
