#ifndef FIRST_CL_HEADER
#define FIRST_CL_HEADER

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifndef CL_API_FUNCTION
    #ifdef __cplusplus
        #define CL_API_FUNCTION extern "C"
    #else
        #define CL_API_FUNCTION
    #endif
#endif

#ifndef CL_INLINE
    #ifndef _MSC_VER
        #ifdef __cplusplus
            #define CL_INLINE inline
        #else
            #define CL_INLINE
        #endif
    #else
        #define CL_INLINE __forceinline
    #endif
#endif

#ifndef CL_Allocator
struct MA_Arena;
    #define CL_Allocator MA_Arena *
#endif

#ifndef AND_CL_STRING_TERMINATE_ON_NEW_LINE
    #define AND_CL_STRING_TERMINATE_ON_NEW_LINE &&*T->stream != '\n'
#endif

typedef enum CL_Kind {
    CL_EOF,
    CL_MUL,
    CL_DIV,
    CL_MOD,
    CL_LEFTSHIFT,
    CL_RIGHTSHIFT,
    CL_ADD,
    CL_SUB,
    CL_EQUALS,
    CL_LESSERTHEN,
    CL_GREATERTHEN,
    CL_LESSERTHEN_OR_EQUAL,
    CL_GREATERTHEN_OR_EQUAL,
    CL_NOTEQUALS,
    CL_BITAND,
    CL_BITOR,
    CL_BITXOR,
    CL_AND,
    CL_OR,
    CL_NEG,
    CL_NOT,
    CL_DECREMENT,
    CL_INCREMENT,
    CL_POSTDECREMENT,
    CL_POSTINCREMENT,
    CL_ASSIGN,
    CL_DIVASSIGN,
    CL_MULASSIGN,
    CL_MODASSIGN,
    CL_SUBASSIGN,
    CL_ADDASSIGN,
    CL_ANDASSIGN,
    CL_ORASSIGN,
    CL_XORASSIGN,
    CL_LEFTSHIFTASSIGN,
    CL_RIGHTSHIFTASSIGN,
    CL_OPENPAREN,
    CL_CLOSEPAREN,
    CL_OPENBRACE,
    CL_CLOSEBRACE,
    CL_OPENBRACKET,
    CL_CLOSEBRACKET,
    CL_COMMA,
    CL_MACRO_CONCAT,
    CL_PREPROC_STRINGIFY,
    CL_QUESTION,
    CL_THREEDOTS,
    CL_SEMICOLON,
    CL_DOT,
    CL_COLON,
    CL_TAG,
    CL_ARROW,
    CL_EXPRSIZEOF,
    CL_DOCCOMMENT,
    CL_COMMENT,
    CL_IDENTIFIER,
    CL_STRINGLIT,
    CL_CHARLIT,
    CL_ERROR,
    CL_FLOAT,
    CL_INT,
    CL_PREPROC_NULL,
    CL_PREPROC_DEFINE,
    CL_PREPROC_IFDEF,
    CL_PREPROC_IFNDEF,
    CL_PREPROC_INCLUDE,
    CL_PREPROC_ENDIF,
    CL_PREPROC_IF,
    CL_PREPROC_PRAGMA,
    CL_PREPROC_ERROR,
    CL_PREPROC_ELSE,
    CL_PREPROC_ELIF,
    CL_PREPROC_UNDEF,
    CL_KEYWORD_VOID,
    CL_KEYWORD_INT,
    CL_KEYWORD_CHAR,
    CL_KEYWORD_UNSIGNED,
    CL_KEYWORD_SIGNED,
    CL_KEYWORD_LONG,
    CL_KEYWORD_SHORT,
    CL_KEYWORD_DOUBLE,
    CL_KEYWORD_FLOAT,
    CL_KEYWORD__BOOL,
    CL_KEYWORD__COMPLEX,
    CL_KEYWORD__IMAGINARY,
    CL_KEYWORD_STATIC,
    CL_KEYWORD_AUTO,
    CL_KEYWORD_CONST,
    CL_KEYWORD_EXTERN,
    CL_KEYWORD_INLINE,
    CL_KEYWORD_REGISTER,
    CL_KEYWORD_RESTRICT,
    CL_KEYWORD_VOLATILE,
    CL_KEYWORD__THREAD_LOCAL,
    CL_KEYWORD__ATOMIC,
    CL_KEYWORD__NORETURN,
    CL_KEYWORD_STRUCT,
    CL_KEYWORD_UNION,
    CL_KEYWORD_ENUM,
    CL_KEYWORD_TYPEDEF,
    CL_KEYWORD_DEFAULT,
    CL_KEYWORD_BREAK,
    CL_KEYWORD_RETURN,
    CL_KEYWORD_SWITCH,
    CL_KEYWORD_IF,
    CL_KEYWORD_ELSE,
    CL_KEYWORD_FOR,
    CL_KEYWORD_WHILE,
    CL_KEYWORD_CASE,
    CL_KEYWORD_CONTINUE,
    CL_KEYWORD_DO,
    CL_KEYWORD_GOTO,
    CL_KEYWORD_SIZEOF,
    CL_KEYWORD__ALIGNAS,
    CL_KEYWORD__ALIGNOF,
    CL_KEYWORD__STATIC_ASSERT,
    CL_KEYWORD__GENERIC,
    CL_COUNT,
} CL_Kind;

typedef enum CL_Fix {
    CL_FIX_NONE,
    CL_SUFFIX_U,
    CL_SUFFIX_UL,
    CL_SUFFIX_ULL,
    CL_SUFFIX_L,
    CL_SUFFIX_LL,
    CL_SUFFIX_F,
    CL_SUFFIX_FL,
    CL_PREFIX_U8,
    CL_PREFIX_U16,
    CL_PREFIX_U32,
    CL_PREFIX_L,
} CL_Fix;

typedef struct CL_Token CL_Token;
struct CL_Token {
    CL_Kind kind;
    CL_Fix fix;

    bool is_hex : 1;
    bool is_inside_macro : 1;
    bool is_system_include : 1;
    bool is_there_whitespace_before_token : 1;

    uint32_t id;
    int len;
    char *str;

    // Not storing line_begin like I would normally cause the user could
    // override the line and file information using directives.
    // On error need to do search if I want nice error context.
    int line, column;
    char *file;

    union {
        double f64;
        uint64_t u64;
        char *intern;
        char *string_literal;
        struct CL_Message *error;
    };
};

typedef struct CL_Message CL_Message;
struct CL_Message {
    CL_Message *next;
    char *string;
    CL_Token token;
};

typedef struct CL_Lexer CL_Lexer;
struct CL_Lexer {
    CL_Message *first_message;
    CL_Message *last_message;
    int errors;

    char *stream;
    char *stream_begin;
    int line;
    int column;
    char *file;
    bool inside_of_macro;

    // filters
    bool skip_comments : 1;
    bool skip_macros : 1;
    bool select_includes : 1;
    bool select_comments : 1;
    bool select_macros : 1;

    CL_Allocator arena;
};

typedef struct CL_SearchPaths CL_SearchPaths;
struct CL_SearchPaths {
    char **include_path;
    int include_path_count;

    char **system_include_path;
    int system_include_path_count;

    char *file_begin_to_ignore;
};

CL_API_FUNCTION CL_Token CL_Next(CL_Lexer *T);
CL_API_FUNCTION CL_Lexer CL_Begin(CL_Allocator arena, char *stream, char *filename);
CL_API_FUNCTION char *CL_ResolveFilepath(CL_Allocator arena, CL_SearchPaths *search_paths, char *filename, char *parent_file, bool is_system_include);
CL_API_FUNCTION void CL_StringifyMessage(char *buff, int buff_size, CL_Message *msg);
CL_API_FUNCTION void CL_Stringify(char *buff, int buff_size, CL_Token *token);

extern const char *CL_FixString[];
extern const char *CL_KindString[];

CL_INLINE int CL_StringLength(char *string) {
    int len = 0;
    while (*string++ != 0) len++;
    return len;
}

CL_INLINE bool CL_StringsAreEqual(char *a, int64_t alen, const char *b, int64_t blen) {
    if (alen != blen) return false;
    for (int i = 0; i < alen; i += 1) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

CL_INLINE bool CL_IsIdentifier(CL_Token *token, char *str) {
    int str_len = CL_StringLength(str);
    bool result = token->kind == CL_IDENTIFIER && CL_StringsAreEqual(token->str, token->len, str, str_len);
    return result;
}

CL_INLINE bool CL_IsAssign(CL_Kind op) {
    bool result = op >= CL_ASSIGN && op <= CL_RIGHTSHIFTASSIGN;
    return result;
}

CL_INLINE bool CL_IsKeywordType(CL_Kind op) {
    bool result = op >= CL_KEYWORD_VOID && op <= CL_KEYWORD__IMAGINARY;
    return result;
}

CL_INLINE bool CL_IsKeywordTypeOrSpec(CL_Kind op) {
    bool result = op >= CL_KEYWORD_VOID && op <= CL_KEYWORD_TYPEDEF;
    return result;
}

CL_INLINE bool CL_IsMacro(CL_Kind kind) {
    bool result = kind >= CL_PREPROC_DEFINE && kind <= CL_PREPROC_UNDEF;
    return result;
}

CL_INLINE bool CL_IsKeyword(CL_Kind kind) {
    bool result = kind >= CL_KEYWORD_VOID && kind <= CL_KEYWORD__GENERIC;
    return result;
}

CL_INLINE bool CL_IsKeywordOrIdent(CL_Kind kind) {
    bool result = CL_IsKeyword(kind) || kind == CL_IDENTIFIER;
    return result;
}

#endif