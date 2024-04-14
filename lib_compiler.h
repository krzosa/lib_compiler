/*
This is a compiler frontend in a single-header-file library form.
This is a **beta** so things may change between versions!

# How to use

In *one* of your C or C++ files to create the implementation:

```
#define LIB_COMPILER_IMPLEMENTATION
#include "lib_compiler.h"
```

In the rest of your files you can just include it like a regular
header.

# Examples

See online repository for code examples

# Overrides

You can override libc calls, the arena implementation using
preprocessor at compile time, here is an example of how you
would go about it:

```
#define LC_vsnprintf stbsp_vsnprintf
#define LC_MemoryZero(p, size) __builtin_memset(p, 0, size);
#define LC_MemoryCopy(dst, src, size) __builtin_memcpy(dst, src, size)

#define LIB_COMPILER_IMPLEMENTATION
#include "lib_compiler.h"
```

Look for '@override' to find things that can be overridden using macro preprocessor
Look for '@api' to find the main functions that you are supposed to use
Look for '@configurable' to find runtime callbacks you can register and other settings

# License (MIT)

See end of file

*/
#ifndef LIB_COMPILER_HEADER
#define LIB_COMPILER_HEADER

#define LIB_COMPILER_MAJOR 0
#define LIB_COMPILER_MINOR 6

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>

#ifndef LC_THREAD_LOCAL // @override
    #if defined(__cplusplus) && __cplusplus >= 201103L
        #define LC_THREAD_LOCAL thread_local
    #elif defined(__GNUC__)
        #define LC_THREAD_LOCAL __thread
    #elif defined(_MSC_VER)
        #define LC_THREAD_LOCAL __declspec(thread)
    #elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
        #define LC_THREAD_LOCAL _Thread_local
    #elif defined(__TINYC__)
        #define LC_THREAD_LOCAL _Thread_local
    #else
        #error Couldnt figure out thread local, needs to be provided manually
    #endif
#endif

#ifndef LC_FUNCTION // @override
    #define LC_FUNCTION
#endif

#ifndef LC_String // @override
typedef struct {
    char   *str;
    int64_t len;
} LC_String;
#endif

#ifndef LC_USE_CUSTOM_ARENA // @override
typedef struct LC_VMemory   LC_VMemory;
typedef struct LC_TempArena LC_TempArena;
typedef struct LC_Arena     LC_Arena;
typedef struct LC_SourceLoc LC_SourceLoc;

struct LC_VMemory {
    size_t   commit;
    size_t   reserve;
    uint8_t *data;
};

struct LC_Arena {
    LC_VMemory memory;
    int        alignment;
    size_t     len;
    size_t     base_len; // When popping to 0 this is the minimum "len" value
                         // It's so that Bootstrapped arena won't delete itself when Reseting.
};

struct LC_TempArena {
    LC_Arena *arena;
    size_t    pos;
};
#endif

typedef struct LC_MapEntry               LC_MapEntry;
typedef struct LC_Map                    LC_Map;
typedef struct LC_AST                    LC_AST;
typedef struct LC_ASTPackage             LC_ASTPackage;
typedef struct LC_ASTNoteList            LC_ASTNoteList;
typedef struct LC_ExprCompo              LC_ExprCompo;
typedef union LC_Val                     LC_Val;
typedef struct LC_ExprIdent              LC_ExprIdent;
typedef struct LC_ExprUnary              LC_ExprUnary;
typedef struct LC_ExprBinary             LC_ExprBinary;
typedef struct LC_ExprField              LC_ExprField;
typedef struct LC_ExprIndex              LC_ExprIndex;
typedef struct LC_ExprCompoItem          LC_ExprCompoItem;
typedef struct LC_ExprType               LC_ExprType;
typedef struct LC_ExprCast               LC_ExprCast;
typedef struct LC_ExprNote               LC_ExprNote;
typedef struct LC_StmtBlock              LC_StmtBlock;
typedef struct LC_StmtFor                LC_StmtFor;
typedef struct LC_StmtDefer              LC_StmtDefer;
typedef struct LC_StmtSwitch             LC_StmtSwitch;
typedef struct LC_StmtCase               LC_StmtCase;
typedef struct LC_StmtIf                 LC_StmtIf;
typedef struct LC_StmtBreak              LC_StmtBreak;
typedef struct LC_StmtAssign             LC_StmtAssign;
typedef struct LC_StmtExpr               LC_StmtExpr;
typedef struct LC_StmtVar                LC_StmtVar;
typedef struct LC_StmtConst              LC_StmtConst;
typedef struct LC_StmtReturn             LC_StmtReturn;
typedef struct LC_StmtNote               LC_StmtNote;
typedef struct LC_TypespecArray          LC_TypespecArray;
typedef struct LC_TypespecProc           LC_TypespecProc;
typedef struct LC_TypespecAggMem         LC_TypespecAggMem;
typedef struct LC_TypespecProcArg        LC_TypespecProcArg;
typedef struct LC_DeclBase               LC_DeclBase;
typedef struct LC_DeclProc               LC_DeclProc;
typedef struct LC_DeclTypedef            LC_DeclTypedef;
typedef struct LC_DeclAgg                LC_DeclAgg;
typedef struct LC_DeclVar                LC_DeclVar;
typedef struct LC_DeclConst              LC_DeclConst;
typedef struct LC_DeclNote               LC_DeclNote;
typedef union LC_ASTValue                LC_ASTValue;
typedef struct LC_TypeAndVal             LC_TypeAndVal;
typedef struct LC_TypeArray              LC_TypeArray;
typedef struct LC_TypePtr                LC_TypePtr;
typedef struct LC_TypeMemberList         LC_TypeMemberList;
typedef struct LC_TypeProc               LC_TypeProc;
typedef struct LC_TypeAgg                LC_TypeAgg;
typedef union LC_TypeValue               LC_TypeValue;
typedef struct LC_Type                   LC_Type;
typedef struct LC_TypeMember             LC_TypeMember;
typedef struct LC_Decl                   LC_Decl;
typedef struct LC_DeclStack              LC_DeclStack;
typedef struct LC_Map                    DeclScope;
typedef struct LC_ResolvedCompo          LC_ResolvedCompo;
typedef struct LC_ResolvedCompoItem      LC_ResolvedCompoItem;
typedef struct LC_ResolvedCompoArrayItem LC_ResolvedCompoArrayItem;
typedef struct LC_ResolvedArrayCompo     LC_ResolvedArrayCompo;
typedef union LC_TokenVal                LC_TokenVal;
typedef struct LC_Token                  LC_Token;
typedef struct LC_Lex                    LC_Lex;
typedef struct LC_Parser                 LC_Parser;
typedef struct LC_Resolver               LC_Resolver;
typedef struct LC_Lang                   LC_Lang;
typedef struct LC_ASTRef                 LC_ASTRef;
typedef struct LC_ASTRefList             LC_ASTRefList;
typedef struct LC_ASTFile                LC_ASTFile;
typedef LC_Val                           Expr_Atom;
typedef LC_TypespecArray                 Typespec_Pointer;
typedef uintptr_t                        LC_Intern;
typedef struct LC_GlobImport             LC_GlobImport;
typedef struct LC_UTF32Result            LC_UTF32Result;
typedef struct LC_UTF8Result             LC_UTF8Result;
typedef struct LC_UTF16Result            LC_UTF16Result;

typedef struct LC_StringNode LC_StringNode;
struct LC_StringNode {
    LC_StringNode *next;
    LC_String      string;
};

typedef struct LC_StringList LC_StringList;
struct LC_StringList {
    int64_t        node_count;
    int64_t        char_count;
    LC_StringNode *first;
    LC_StringNode *last;
};

typedef struct LC_String16 {
    wchar_t *str;
    int64_t  len;
} LC_String16;

struct LC_MapEntry {
    uint64_t key;
    uint64_t value;
};

struct LC_Map {
    LC_Arena    *arena;
    LC_MapEntry *entries;
    int          cap;
    int          len;
};

typedef struct LC_BigInt LC_BigInt;
struct LC_BigInt {
    unsigned digit_count;
    bool     is_negative;
    union {
        uint64_t  digit;
        uint64_t *digits;
    };
};

typedef enum LC_ASTKind {
    LC_ASTKind_Null,
    LC_ASTKind_Error,
    LC_ASTKind_Note,
    LC_ASTKind_NoteList,
    LC_ASTKind_File,
    LC_ASTKind_Package,
    LC_ASTKind_Ignore,
    LC_ASTKind_TypespecProcArg,
    LC_ASTKind_TypespecAggMem,
    LC_ASTKind_ExprCallItem,
    LC_ASTKind_ExprCompoundItem,
    LC_ASTKind_ExprNote, // a: int = #`sizeof(int)`;
    LC_ASTKind_StmtSwitchCase,
    LC_ASTKind_StmtSwitchDefault,
    LC_ASTKind_StmtElseIf,
    LC_ASTKind_StmtElse,
    LC_ASTKind_GlobImport,
    LC_ASTKind_DeclNote,
    LC_ASTKind_DeclProc,
    LC_ASTKind_DeclStruct,
    LC_ASTKind_DeclUnion,
    LC_ASTKind_DeclVar,
    LC_ASTKind_DeclConst,
    LC_ASTKind_DeclTypedef,
    LC_ASTKind_TypespecIdent,
    LC_ASTKind_TypespecField,
    LC_ASTKind_TypespecPointer,
    LC_ASTKind_TypespecArray,
    LC_ASTKind_TypespecProc,
    LC_ASTKind_StmtBlock,
    LC_ASTKind_StmtNote,
    LC_ASTKind_StmtReturn,
    LC_ASTKind_StmtBreak,
    LC_ASTKind_StmtContinue,
    LC_ASTKind_StmtDefer,
    LC_ASTKind_StmtFor,
    LC_ASTKind_StmtIf,
    LC_ASTKind_StmtSwitch,
    LC_ASTKind_StmtAssign,
    LC_ASTKind_StmtExpr,
    LC_ASTKind_StmtVar,
    LC_ASTKind_StmtConst,
    LC_ASTKind_ExprIdent,
    LC_ASTKind_ExprString,
    LC_ASTKind_ExprInt,
    LC_ASTKind_ExprFloat,
    LC_ASTKind_ExprBool,
    LC_ASTKind_ExprType,
    LC_ASTKind_ExprBinary,
    LC_ASTKind_ExprUnary,
    LC_ASTKind_ExprBuiltin,
    LC_ASTKind_ExprCall,
    LC_ASTKind_ExprCompound,
    LC_ASTKind_ExprCast,
    LC_ASTKind_ExprField,
    LC_ASTKind_ExprIndex,
    LC_ASTKind_ExprAddPtr,
    LC_ASTKind_ExprGetValueOfPointer,
    LC_ASTKind_ExprGetPointerOfValue,
    LC_ASTKind_Count,
    LC_ASTKind_FirstExpr     = LC_ASTKind_ExprIdent,
    LC_ASTKind_LastExpr      = LC_ASTKind_ExprGetPointerOfValue,
    LC_ASTKind_FirstStmt     = LC_ASTKind_StmtBlock,
    LC_ASTKind_LastStmt      = LC_ASTKind_StmtConst,
    LC_ASTKind_FirstTypespec = LC_ASTKind_TypespecIdent,
    LC_ASTKind_LastTypespec  = LC_ASTKind_TypespecProc,
    LC_ASTKind_FirstDecl     = LC_ASTKind_DeclProc,
    LC_ASTKind_LastDecl      = LC_ASTKind_DeclTypedef,
} LC_ASTKind;

typedef enum LC_TypeKind {
    LC_TypeKind_char,
    LC_TypeKind_uchar,
    LC_TypeKind_short,
    LC_TypeKind_ushort,
    LC_TypeKind_bool,
    LC_TypeKind_int,
    LC_TypeKind_uint,
    LC_TypeKind_long,
    LC_TypeKind_ulong,
    LC_TypeKind_llong,
    LC_TypeKind_ullong,
    LC_TypeKind_float,
    LC_TypeKind_double,
    LC_TypeKind_void,
    LC_TypeKind_Struct,
    LC_TypeKind_Union,
    LC_TypeKind_Pointer,
    LC_TypeKind_Array,
    LC_TypeKind_Proc,
    LC_TypeKind_UntypedInt,
    LC_TypeKind_UntypedFloat,
    LC_TypeKind_UntypedString,
    LC_TypeKind_Incomplete,
    LC_TypeKind_Completing,
    LC_TypeKind_Error,
    T_TotalCount,
    T_NumericCount = LC_TypeKind_void,
    T_Count        = LC_TypeKind_void + 1,
} LC_TypeKind;

typedef enum LC_DeclState {
    LC_DeclState_Unresolved,
    LC_DeclState_Resolving,
    LC_DeclState_Resolved,
    LC_DeclState_ResolvedBody, // proc
    LC_DeclState_Error,
    LC_DeclState_Count,
} LC_DeclState;

typedef enum LC_DeclKind {
    LC_DeclKind_Error,
    LC_DeclKind_Type,
    LC_DeclKind_Const,
    LC_DeclKind_Var,
    LC_DeclKind_Proc,
    LC_DeclKind_Import,
    LC_DeclKind_Count,
} LC_DeclKind;

typedef enum LC_TokenKind {
    LC_TokenKind_EOF,
    LC_TokenKind_Error,
    LC_TokenKind_Comment,
    LC_TokenKind_DocComment,
    LC_TokenKind_FileDocComment,
    LC_TokenKind_PackageDocComment,
    LC_TokenKind_Note,
    LC_TokenKind_Hash,
    LC_TokenKind_Ident,
    LC_TokenKind_Keyword,
    LC_TokenKind_String,
    LC_TokenKind_RawString,
    LC_TokenKind_Int,
    LC_TokenKind_Float,
    LC_TokenKind_Unicode,
    LC_TokenKind_OpenParen,
    LC_TokenKind_CloseParen,
    LC_TokenKind_OpenBrace,
    LC_TokenKind_CloseBrace,
    LC_TokenKind_OpenBracket,
    LC_TokenKind_CloseBracket,
    LC_TokenKind_Comma,
    LC_TokenKind_Question,
    LC_TokenKind_Semicolon,
    LC_TokenKind_Dot,
    LC_TokenKind_ThreeDots,
    LC_TokenKind_Colon,
    LC_TokenKind_Mul,
    LC_TokenKind_Div,
    LC_TokenKind_Mod,
    LC_TokenKind_LeftShift,
    LC_TokenKind_RightShift,
    LC_TokenKind_Add,
    LC_TokenKind_Sub,
    LC_TokenKind_Equals,
    LC_TokenKind_LesserThen,
    LC_TokenKind_GreaterThen,
    LC_TokenKind_LesserThenEq,
    LC_TokenKind_GreaterThenEq,
    LC_TokenKind_NotEquals,
    LC_TokenKind_BitAnd,
    LC_TokenKind_BitOr,
    LC_TokenKind_BitXor,
    LC_TokenKind_And,
    LC_TokenKind_Or,
    LC_TokenKind_AddPtr,
    LC_TokenKind_Neg,
    LC_TokenKind_Not,
    LC_TokenKind_Assign,
    LC_TokenKind_DivAssign,
    LC_TokenKind_MulAssign,
    LC_TokenKind_ModAssign,
    LC_TokenKind_SubAssign,
    LC_TokenKind_AddAssign,
    LC_TokenKind_BitAndAssign,
    LC_TokenKind_BitOrAssign,
    LC_TokenKind_BitXorAssign,
    LC_TokenKind_LeftShiftAssign,
    LC_TokenKind_RightShiftAssign,
    LC_TokenKind_Count,
} LC_TokenKind;

struct LC_ASTFile {
    LC_AST *package;
    LC_Lex *x;

    LC_AST *fimport;
    LC_AST *limport;
    LC_AST *fdecl;
    LC_AST *ldecl;
    LC_AST *fdiscarded;
    LC_AST *ldiscarded; // @build_if

    LC_Token *doc_comment;

    bool build_if;
};

struct LC_ASTPackage {
    LC_Intern     name;
    LC_DeclState  state;
    LC_String     path;
    LC_StringList injected_filepaths; // to sidestep regular file finding, implement single file packages etc.
    LC_AST       *ffile;
    LC_AST       *lfile;
    LC_AST       *fdiscarded;
    LC_AST       *ldiscarded; // #build_if

    LC_Token *doc_comment;

    // These are resolved later:
    // @todo: add foreign name?
    LC_Decl   *first_ordered;
    LC_Decl   *last_ordered;
    DeclScope *scope;
};

struct LC_ASTNoteList {
    LC_AST *first;
    LC_AST *last;
};

struct LC_ResolvedCompo {
    int                   count;
    LC_ResolvedCompoItem *first;
    LC_ResolvedCompoItem *last;
};

struct LC_ResolvedCompoItem {
    LC_ResolvedCompoItem *next;
    LC_AST               *comp; // for proc this may be null because we might match default value
    LC_AST               *expr; // for proc this is very important, the result, ordered expression
    LC_TypeMember        *t;
    bool                  varg;
    bool                  defaultarg;
};

struct LC_ResolvedCompoArrayItem {
    LC_ResolvedCompoArrayItem *next;
    LC_AST                    *comp;
    int                        index;
};

struct LC_ResolvedArrayCompo {
    int                        count;
    LC_ResolvedCompoArrayItem *first;
    LC_ResolvedCompoArrayItem *last;
};

struct LC_ExprCompo {
    LC_AST *name;  // :name{thing} or name(thing)
    LC_AST *first; // {LC_ExprCompoItem, LC_ExprCompoItem}
    LC_AST *last;
    int     size;
    union {
        LC_ResolvedCompo      *resolved_items;
        LC_ResolvedArrayCompo *resolved_array_items;
    };
};

struct LC_ExprCompoItem {
    LC_Intern name;
    LC_AST   *index;
    LC_AST   *expr;
};

// clang-format off
union LC_Val               { LC_BigInt i; double d; LC_Intern name; };
struct LC_ExprIdent       { LC_Intern name; LC_Decl *resolved_decl; };
struct LC_ExprUnary       { LC_TokenKind op; LC_AST *expr; };
struct LC_ExprBinary      { LC_TokenKind op; LC_AST *left; LC_AST *right; };
struct LC_ExprField       { LC_AST *left; LC_Intern right; LC_Decl *resolved_decl; LC_Decl *parent_decl; };
struct LC_ExprIndex       { LC_AST *base; LC_AST *index; };
struct LC_ExprType        { LC_AST *type; };
struct LC_ExprCast        { LC_AST *type; LC_AST *expr; };
struct LC_ExprNote        { LC_AST *expr; }; // v := #c(``)

typedef enum { SBLK_Norm, SBLK_Loop, SBLK_Proc, SBLK_Defer } LC_StmtBlockKind;
struct LC_StmtBlock       { LC_AST *first; LC_AST *last; LC_AST *first_defer; LC_StmtBlockKind kind; LC_Intern name; };
struct LC_StmtFor         { LC_AST *init; LC_AST *cond; LC_AST *inc; LC_AST *body; };
struct LC_StmtDefer       { LC_AST *next; LC_AST *body; };
struct LC_StmtSwitch      { LC_AST *first; LC_AST *last; LC_AST *expr; int total_switch_case_count; };
struct LC_StmtCase        { LC_AST *first; LC_AST *last; LC_AST *body; };
// 'else if' and 'else' is avaialable from 'first'.
// The else_if and else are also LC_StmtIf but
// have different kinds and don't use 'first', 'last'
struct LC_StmtIf          { LC_AST *expr; LC_AST *body; LC_AST *first; LC_AST *last; };
struct LC_StmtBreak       { LC_Intern name; };
struct LC_StmtAssign      { LC_TokenKind op; LC_AST *left; LC_AST *right; };
struct LC_StmtExpr        { LC_AST *expr; };
struct LC_StmtVar         { LC_AST *expr; LC_AST *type; LC_Intern name; LC_Decl *resolved_decl; };
struct LC_StmtConst       { LC_AST *expr; LC_Intern name; };
struct LC_StmtReturn      { LC_AST *expr; };
struct LC_StmtNote        { LC_AST *expr; }; // #c(``) in block
struct LC_TypespecArray   { LC_AST *base; LC_AST *index; };
struct LC_TypespecProc    { LC_AST *first; LC_AST *last; LC_AST *ret; bool vargs; bool vargs_any_promotion; };
struct LC_TypespecAggMem  { LC_Intern name; LC_AST *type; };
struct LC_TypespecProcArg { LC_Intern name; LC_AST *type; LC_AST *expr; LC_Decl *resolved_decl; };
struct LC_DeclBase        { LC_Intern name; LC_Token *doc_comment; LC_Decl *resolved_decl; };
struct LC_DeclProc        { LC_DeclBase base; LC_AST *body; LC_AST *type; };
struct LC_DeclTypedef     { LC_DeclBase base; LC_AST *type; };
struct LC_DeclAgg         { LC_DeclBase base; LC_AST *first; LC_AST *last; };
struct LC_DeclVar         { LC_DeclBase base; LC_AST *expr; LC_AST *type; };
struct LC_DeclConst       { LC_DeclBase base; LC_AST *expr; };
struct LC_DeclNote        { LC_DeclBase base; LC_AST *expr; bool processed; }; // #c(``) in file note list
struct LC_GlobImport      { LC_Intern name; LC_Intern path; bool resolved; LC_Decl *resolved_decl; };

struct LC_ASTRef          { LC_ASTRef *next; LC_ASTRef *prev; LC_AST *ast; };
struct LC_ASTRefList       { LC_ASTRef *first; LC_ASTRef *last; };
// clang-format on

struct LC_TypeAndVal {
    LC_Type *type;
    union {
        LC_Val v;
        union {
            LC_BigInt i;
            double    d;
            LC_Intern name;
        };
    };
};

struct LC_AST {
    LC_ASTKind kind;
    uint32_t   id;
    LC_AST    *next;
    LC_AST    *prev;
    LC_AST    *notes;
    LC_Token  *pos;

    LC_TypeAndVal const_val;
    LC_Type      *type;
    union {
        LC_ASTFile         afile;
        LC_ASTPackage      apackage;
        LC_ASTNoteList     anote_list;
        LC_ExprCompo       anote;
        LC_Val             eatom;
        LC_ExprIdent       eident;
        LC_ExprUnary       eunary;
        LC_ExprBinary      ebinary;
        LC_ExprField       efield;
        LC_ExprIndex       eindex;
        LC_ExprCompo       ecompo;
        LC_ExprCompoItem   ecompo_item;
        LC_ExprType        etype;
        LC_ExprCast        ecast;
        LC_ExprNote        enote;
        LC_StmtBlock       sblock;
        LC_StmtFor         sfor;
        LC_StmtDefer       sdefer;
        LC_StmtSwitch      sswitch;
        LC_StmtCase        scase;
        LC_StmtIf          sif;
        LC_StmtBreak       sbreak;
        LC_StmtBreak       scontinue;
        LC_StmtAssign      sassign;
        LC_StmtExpr        sexpr;
        LC_StmtVar         svar;
        LC_StmtConst       sconst;
        LC_StmtReturn      sreturn;
        LC_StmtNote        snote;
        LC_ExprIdent       tident;
        LC_TypespecArray   tarray;
        LC_TypespecArray   tpointer;
        LC_TypespecProc    tproc;
        LC_TypespecAggMem  tagg_mem;
        LC_TypespecProcArg tproc_arg;
        LC_DeclBase        dbase;
        LC_DeclProc        dproc;
        LC_DeclTypedef     dtypedef;
        LC_DeclAgg         dagg;
        LC_DeclVar         dvar;
        LC_DeclConst       dconst;
        LC_DeclNote        dnote;
        LC_GlobImport      gimport;
    };
};

struct LC_TypeArray {
    LC_Type *base;
    int      size;
};

struct LC_TypePtr {
    LC_Type *base;
};

struct LC_TypeMemberList {
    LC_TypeMember *first;
    LC_TypeMember *last;
    int            count;
};

struct LC_TypeProc {
    LC_TypeMemberList args;
    LC_Type          *ret;
    bool              vargs;
    bool              vargs_any_promotion;
};

struct LC_TypeAgg {
    LC_TypeMemberList mems;
};

struct LC_Type {
    LC_TypeKind kind;
    int         size;
    int         align;
    int         is_unsigned;
    int         id;
    int         padding;
    LC_Decl    *decl;
    union {
        LC_TypeArray tarray;
        LC_TypePtr   tptr;
        LC_TypeProc  tproc;
        LC_TypeAgg   tagg;
        LC_Type     *tbase;
        LC_Type     *tutdefault;
    };
};

struct LC_TypeMember {
    LC_TypeMember *next;
    LC_TypeMember *prev;
    LC_Intern      name;
    LC_Type       *type;
    LC_AST        *default_value_expr;
    LC_AST        *ast;
    int            offset;
};

struct LC_Decl {
    LC_DeclKind  kind;
    LC_DeclState state;
    uint8_t      is_foreign;

    LC_Decl  *next;
    LC_Decl  *prev;
    LC_Intern name;
    LC_Intern foreign_name;
    LC_AST   *ast;
    LC_AST   *package;

    LC_TypeMember *type_member;
    DeclScope     *scope;
    LC_Decl       *typedef_renamed_type_decl;
    union {
        LC_TypeAndVal val;
        struct {
            LC_Type *type;
            LC_Val   v;
        };
    };
};

typedef struct {
    LC_Arena *arena;
    LC_AST  **data;
    int       cap;
    int       len;
} LC_ASTArray;

typedef struct LC_ASTWalker LC_ASTWalker;
typedef void                LC_ASTWalkProc(LC_ASTWalker *, LC_AST *);
struct LC_ASTWalker {
    LC_ASTArray stack;

    int inside_builtin;
    int inside_discarded;
    int inside_note;

    uint8_t         visit_discarded;
    uint8_t         visit_notes;
    uint8_t         depth_first;
    uint8_t         dont_recurse; // breathfirst only
    void           *user_data;
    LC_ASTWalkProc *proc;
};

struct LC_DeclStack {
    LC_Decl **stack;
    int       len;
    int       cap;
};

union LC_TokenVal {
    LC_BigInt i;
    double    f64;
    LC_Intern ident;
};

struct LC_Token {
    LC_TokenKind kind;
    int          line;
    int          column;
    int          len;
    char        *str;
    LC_Lex      *lex;
    union {
        LC_BigInt i;
        double    f64;
        LC_Intern ident;
    };
};

struct LC_Lex {
    char     *at;
    char     *begin;
    LC_Intern file;
    int       line;
    int       column;
    LC_Token *tokens;
    int       token_count;
    bool      insert_semicolon;
};

struct LC_Parser {
    LC_Token *at;
    LC_Token *begin;
    LC_Token *end;
    LC_Lex   *x;
};

struct LC_Resolver {
    LC_Type     *compo_context_type;
    int          compo_context_array_size;
    LC_Type     *expected_ret_type;
    LC_AST      *package;
    DeclScope   *active_scope;
    LC_DeclStack locals;
    LC_Map       duplicate_map; // currently used for finding duplicates in compos
    LC_ASTArray  stmt_block_stack;
};

typedef struct LC_Printer LC_Printer;
struct LC_Printer {
    LC_Arena     *arena;
    LC_StringList list;
    int           indent;
    LC_Intern     last_filename;
    int           last_line_num;
    LC_ASTArray   out_block_stack;
};

const int LC_OPF_Error    = 1;
const int LC_OPF_UTConst  = 2;
const int LC_OPF_LValue   = 4;
const int LC_OPF_Const    = 8;
const int LC_OPF_Returned = 16;

// warning: I introduced a null compare using the values in the operand
// make sure to revisit that when modifying the struct
typedef struct {
    int flags;
    union {
        LC_Decl      *decl;
        LC_TypeAndVal val;
        struct {
            LC_Type *type;
            LC_Val   v;
        };
    };
} LC_Operand;

typedef enum {
    LC_ARCH_Invalid,
    LC_ARCH_X64,
    LC_ARCH_X86,
    LC_ARCH_Count,
} LC_ARCH;

typedef enum {
    LC_GEN_Invalid,
    LC_GEN_C,
    LC_GEN_Count,
} LC_GEN;

typedef enum {
    LC_OS_Invalid,
    LC_OS_WINDOWS,
    LC_OS_LINUX,
    LC_OS_MAC,
    LC_OS_Count,
} LC_OS;

typedef struct {
    LC_String path;
    LC_String content;
    int       line;
} LoadedFile;

typedef enum {
    LC_OPResult_Error,
    LC_OPResult_Ok,
    LC_OPResult_Bool,
} LC_OPResult;

typedef struct {
    int left;
    int right;
} LC_BindingPower;

typedef enum {
    LC_Binding_Prefix,
    LC_Binding_Infix,
    LC_Binding_Postfix,
} LC_Binding;

typedef enum {
    LC_CmpRes_LT,
    LC_CmpRes_GT,
    LC_CmpRes_EQ,
} LC_CmpRes;

typedef struct LC_FileIter LC_FileIter;
struct LC_FileIter {
    bool      is_valid;
    bool      is_directory;
    LC_String absolute_path;
    LC_String relative_path;
    LC_String filename;

    LC_String path;
    LC_Arena *arena;
    union {
        struct LC_Win32_FileIter *w32;
        void                     *dir;
    };
};

#define LC_LIST_KEYWORDS \
    X(for)               \
    X(import)            \
    X(if)                \
    X(else)              \
    X(return)            \
    X(defer)             \
    X(continue)          \
    X(break)             \
    X(default)           \
    X(case)              \
    X(typedef)           \
    X(switch)            \
    X(proc)              \
    X(struct)            \
    X(union)             \
    X(addptr)            \
    X(and)               \
    X(or)                \
    X(bit_and)           \
    X(bit_or)            \
    X(bit_xor)           \
    X(not )              \
    X(true)              \
    X(false)

#define LC_LIST_INTERNS    \
    X(foreign, true)       \
    X(api, true)           \
    X(weak, true)          \
    X(c, true)             \
    X(fallthrough, true)   \
    X(packed, true)        \
    X(not_init, true)      \
    X(unused, true)        \
    X(static_assert, true) \
    X(str, true)           \
    X(thread_local, true)  \
    X(dont_mangle, true)   \
    X(build_if, true)      \
    X(Any, false)          \
    X(main, false)         \
    X(debug_break, false)  \
    X(sizeof, false)       \
    X(alignof, false)      \
    X(typeof, false)       \
    X(lengthof, false)     \
    X(offsetof, false)

#define LC_LIST_TYPES \
    X(char, false)    \
    X(uchar, true)    \
    X(short, false)   \
    X(ushort, true)   \
    X(bool, false)    \
    X(int, false)     \
    X(uint, true)     \
    X(long, false)    \
    X(ulong, true)    \
    X(llong, false)   \
    X(ullong, true)   \
    X(float, false)   \
    X(double, false)

struct LC_Lang {
    LC_Arena *arena;

    LC_Arena *lex_arena;

    LC_Arena *ast_arena;
    int       ast_count;

    LC_Arena *decl_arena;
    int       decl_count;

    LC_Arena *type_arena;
    int       type_count;

    int errors;

    int    typeids;
    LC_Map type_map;

    LC_AST     *builtin_package;
    LC_Resolver resolver;
    LC_Parser  *parser;

    // Package registry
    LC_AST       *fpackage;
    LC_AST       *lpackage;
    LC_Intern     first_package;
    LC_ASTRefList ordered_packages;
    LC_StringList package_dirs;

    LC_Map   interns;
    LC_Map   declared_notes;
    LC_Map   foreign_names;
    LC_Map   implicit_any;
    unsigned unique_counter;

    LC_Intern first_keyword;
    LC_Intern last_keyword;

#define X(x) LC_Intern k##x;
    LC_LIST_KEYWORDS
#undef X

#define X(x, declare) LC_Intern i##x;
    LC_LIST_INTERNS
#undef X

    LC_Type types[14]; // be careful when changing
#define X(TNAME, IS_UNSIGNED) LC_Type *t##TNAME;
    LC_LIST_TYPES
#undef X

    // When adding new special pointer types make sure to
    // also update LC_SetPointerSizeAndAlign so that it properly
    // updates the types after LC_LangBegin
    int      pointer_size;
    int      pointer_align;
    LC_Type *tvoid;
    LC_Type *tpvoid;
    LC_Type *tpchar;

    LC_Type  ttstring;
    LC_Type *tstring;
    LC_Type *tuntypednil;
    LC_Type *tuntypedbool;
    LC_Type *tuntypedint;
    LC_Type  ttuntypedfloat;
    LC_Type *tuntypedfloat;
    LC_Type  ttuntypedstring;
    LC_Type *tuntypedstring;
    LC_Type  ttany;
    LC_Type *tany;

    LC_Token NullToken;
    LC_AST   NullAST;
    LC_Token BuiltinToken;
    LC_Lex   NullLEX;

    LC_Printer printer;
    LC_Parser  quick_parser;

    // @configurable
    LC_ARCH arch;
    LC_GEN  gen;
    LC_OS   os;

    bool emit_line_directives;
    bool breakpoint_on_error;
    bool use_colored_terminal_output;

    bool (*on_decl_parsed)(bool discarded, LC_AST *n); // returning 'true' from here indicates that declaration should be discarded
    void (*on_expr_parsed)(LC_AST *n);
    void (*on_stmt_parsed)(LC_AST *n);
    void (*on_typespec_parsed)(LC_AST *n);
    void (*on_decl_type_resolved)(LC_Decl *decl);
    void (*on_proc_body_resolved)(LC_Decl *decl);
    void (*on_expr_resolved)(LC_AST *expr, LC_Operand *op);
    void (*on_stmt_resolved)(LC_AST *n);
    void (*before_call_args_resolved)(LC_AST *n, LC_Type *type);
    void (*on_file_load)(LC_AST *package, LoadedFile *file);
    void (*on_message)(LC_Token *pos, char *str, int len); // pos and x can be null
    void (*on_fatal_error)(void);
    void *user_data;
};

extern LC_THREAD_LOCAL LC_Lang *L;
extern LC_Operand               LC_OPNull;

//
// Main @api
//

LC_FUNCTION LC_Lang *LC_LangAlloc(void);        // This allocates memory for LC_Lang which can be used to register callbacks and set configurables
LC_FUNCTION void     LC_LangBegin(LC_Lang *l);  // Prepare for compilation: init types, init builtins, set architecture variables stuff like that
LC_FUNCTION void     LC_LangEnd(LC_Lang *lang); // Deallocate language memory

LC_FUNCTION void          LC_RegisterPackageDir(char *dir);              // Add a package search directory
LC_FUNCTION LC_ASTRefList LC_ResolvePackageByName(LC_Intern name);       // Fully resolve a package and all it's dependences
LC_FUNCTION LC_String     LC_GenerateUnityBuild(LC_ASTRefList packages); // Generate the C program and return as a string

// Smaller passes for AST modification
LC_FUNCTION void LC_ParsePackagesUsingRegistry(LC_Intern name);   // These 3 functions are equivalent to LC_ResolvePackageByName,
LC_FUNCTION void LC_OrderAndResolveTopLevelDecls(LC_Intern name); // you can use them to hook into the compilation process - you can modify the AST
LC_FUNCTION void LC_ResolveAllProcBodies(void);                   // before resolving or use resolved top declarations to generate some code.
                                                                  // The Parse and Order functions can be called multiple times to accommodate this.

// Extended pass / optimization
LC_FUNCTION void LC_FindUnusedLocalsAndRemoveUnusedGlobalDecls(void); // Extended pass that you can execute once you have resolved all packages

// These three functions are used to implement LC_FindUnusedLocalsAndRemoveUnusedGlobalDecls
LC_FUNCTION LC_Map LC_CountDeclRefs(LC_Arena *arena);
LC_FUNCTION void   LC_RemoveUnreferencedGlobalDecls(LC_Map *map_of_visits);
LC_FUNCTION void   LC_ErrorOnUnreferencedLocals(LC_Map *map_of_visits);

// Notes
LC_FUNCTION void    LC_DeclareNote(LC_Intern intern);
LC_FUNCTION bool    LC_IsNoteDeclared(LC_Intern intern);
LC_FUNCTION LC_AST *LC_HasNote(LC_AST *ast, LC_Intern i);

// Quick parse functions
LC_FUNCTION LC_AST *LC_ParseStmtf(const char *str, ...);
LC_FUNCTION LC_AST *LC_ParseExprf(const char *str, ...);
LC_FUNCTION LC_AST *LC_ParseDeclf(const char *str, ...);

// AST Walking and copy
LC_FUNCTION LC_AST      *LC_CopyAST(LC_Arena *arena, LC_AST *n);    // Deep copy the AST
LC_FUNCTION LC_ASTArray  LC_FlattenAST(LC_Arena *arena, LC_AST *n); // This walks the passed down tree and generates a flat array of pointers, very nice to use for traversing AST
LC_FUNCTION void         LC_WalkAST(LC_ASTWalker *ctx, LC_AST *n);
LC_FUNCTION LC_ASTWalker LC_GetDefaultWalker(LC_Arena *arena, LC_ASTWalkProc *proc);

LC_FUNCTION void    LC_ReserveAST(LC_ASTArray *arr, int size);
LC_FUNCTION void    LC_PushAST(LC_ASTArray *arr, LC_AST *ast);
LC_FUNCTION void    LC_PopAST(LC_ASTArray *arr);
LC_FUNCTION LC_AST *LC_GetLastAST(LC_ASTArray *arr);

// Interning API
LC_FUNCTION LC_Intern LC_ILit(char *str);
LC_FUNCTION void      LC_InternTokens(LC_Lex *x);

LC_FUNCTION LC_Intern LC_InternStrLen(char *str, int len);
LC_FUNCTION LC_Intern LC_GetUniqueIntern(const char *name_for_debug);
LC_FUNCTION char     *LC_GetUniqueName(const char *name_for_debug);

//
// Package functions
//
LC_FUNCTION LC_Operand    LC_ImportPackage(LC_AST *import, LC_AST *dst, LC_AST *src);
LC_FUNCTION LC_Intern     LC_MakePackageNameFromPath(LC_String path);
LC_FUNCTION bool          LC_PackageNameValid(LC_Intern name);
LC_FUNCTION bool          LC_PackageNameDuplicate(LC_Intern name);
LC_FUNCTION void          LC_AddPackageToList(LC_AST *n);
LC_FUNCTION LC_AST       *LC_RegisterPackage(LC_String path);
LC_FUNCTION void          LC_AddFileToPackage(LC_AST *pkg, LC_AST *f);
LC_FUNCTION LC_AST       *LC_FindImportInRefList(LC_ASTRefList *arr, LC_Intern path);
LC_FUNCTION void          LC_AddASTToRefList(LC_ASTRefList *refs, LC_AST *ast);
LC_FUNCTION LC_ASTRefList LC_GetPackageImports(LC_AST *package);
LC_FUNCTION LC_AST       *LC_GetPackageByName(LC_Intern name);
LC_FUNCTION LC_StringList LC_ListFilesInPackage(LC_Arena *arena, LC_String path);
LC_FUNCTION LoadedFile    LC_ReadFileHook(LC_AST *package, LC_String path);
LC_FUNCTION void          LC_ParsePackage(LC_AST *n);
LC_FUNCTION void          LC_AddOrderedPackageToRefList(LC_AST *n);
LC_FUNCTION LC_AST       *LC_OrderPackagesAndBasicResolve(LC_AST *pos, LC_Intern name);
LC_FUNCTION void          LC_AddSingleFilePackage(LC_Intern name, LC_String path);

//
// Lexing functions
//
LC_FUNCTION LC_Lex   *LC_LexStream(char *file, char *str, int line);
LC_FUNCTION LC_String LC_GetTokenLine(LC_Token *token);

LC_FUNCTION void      LC_LexingError(LC_Token *pos, const char *str, ...);
LC_FUNCTION bool      LC_IsAssign(LC_TokenKind kind);
LC_FUNCTION bool      LC_IsHexDigit(char c);
LC_FUNCTION bool      LC_IsBinDigit(char c);
LC_FUNCTION uint64_t  LC_MapCharToNumber(char c);
LC_FUNCTION uint64_t  LC_GetEscapeCode(char c);
LC_FUNCTION LC_String LC_GetEscapeString(char c);
LC_FUNCTION void      LC_LexAdvance(LC_Lex *x);
LC_FUNCTION void      LC_EatWhitespace(LC_Lex *x);
LC_FUNCTION void      LC_EatIdent(LC_Lex *x);
LC_FUNCTION void      LC_SetTokenLen(LC_Lex *x, LC_Token *t);
LC_FUNCTION void      LC_EatUntilIncluding(LC_Lex *x, char c);
LC_FUNCTION LC_BigInt LC_LexBigInt(char *string, int len, uint64_t base);
LC_FUNCTION void      LC_LexNestedComments(LC_Lex *x, LC_Token *t);
LC_FUNCTION void      LC_LexStringLiteral(LC_Lex *x, LC_Token *t, LC_TokenKind kind);
LC_FUNCTION void      LC_LexUnicodeLiteral(LC_Lex *x, LC_Token *t);
LC_FUNCTION void      LC_LexIntOrFloat(LC_Lex *x, LC_Token *t);
LC_FUNCTION void      LC_LexCase2(LC_Lex *x, LC_Token *t, LC_TokenKind tk0, char c, LC_TokenKind tk1);
LC_FUNCTION void      LC_LexCase3(LC_Lex *x, LC_Token *t, LC_TokenKind tk, char c0, LC_TokenKind tk0, char c1, LC_TokenKind tk1);
LC_FUNCTION void      LC_LexCase4(LC_Lex *x, LC_Token *t, LC_TokenKind tk, char c0, LC_TokenKind tk0, char c1, LC_TokenKind tk1, char c2, LC_TokenKind tk2);
LC_FUNCTION void      LC_LexNext(LC_Lex *x, LC_Token *t);

//
// LC_Map API
//
LC_FUNCTION void         LC_MapReserve(LC_Map *map, int size);
LC_FUNCTION int          LC_NextPow2(int v);
LC_FUNCTION LC_MapEntry *LC_GetMapEntryEx(LC_Map *map, uint64_t key);
LC_FUNCTION bool         LC_InsertWithoutReplace(LC_Map *map, void *key, void *value);
LC_FUNCTION LC_MapEntry *LC_InsertMapEntry(LC_Map *map, uint64_t key, uint64_t value);
LC_FUNCTION LC_MapEntry *LC_GetMapEntry(LC_Map *map, uint64_t key);
LC_FUNCTION void         LC_MapInsert(LC_Map *map, LC_String keystr, void *value);
LC_FUNCTION void        *LC_MapGet(LC_Map *map, LC_String keystr);
LC_FUNCTION void         LC_MapInsertU64(LC_Map *map, uint64_t key, void *value);
LC_FUNCTION void        *LC_MapGetU64(LC_Map *map, uint64_t key);
LC_FUNCTION void        *LC_MapGetP(LC_Map *map, void *key);
LC_FUNCTION void         LC_MapInsertP(LC_Map *map, void *key, void *value);
LC_FUNCTION void         LC_MapClear(LC_Map *map);

//
// LC_AST Creation
//
LC_FUNCTION LC_AST *LC_CreateAST(LC_Token *pos, LC_ASTKind kind);
LC_FUNCTION LC_AST *LC_CreateUnary(LC_Token *pos, LC_TokenKind op, LC_AST *expr);
LC_FUNCTION LC_AST *LC_CreateBinary(LC_Token *pos, LC_AST *left, LC_AST *right, LC_TokenKind op);
LC_FUNCTION LC_AST *LC_CreateIndex(LC_Token *pos, LC_AST *left, LC_AST *index);

LC_FUNCTION bool LC_ContainsCallExpr(LC_AST *ast);
LC_FUNCTION void LC_SetASTPosOnAll(LC_AST *n, LC_Token *pos);
LC_FUNCTION bool LC_ContainsCBuiltin(LC_AST *n);

//
// LC_Type functions
//
LC_FUNCTION void           LC_SetPointerSizeAndAlign(int size, int align);
LC_FUNCTION LC_Type       *LC_CreateType(LC_TypeKind kind);
LC_FUNCTION LC_Type       *LC_CreateTypedef(LC_Decl *decl, LC_Type *base);
LC_FUNCTION LC_Type       *LC_CreatePointerType(LC_Type *type);
LC_FUNCTION LC_Type       *LC_CreateArrayType(LC_Type *type, int size);
LC_FUNCTION LC_Type       *LC_CreateProcType(LC_TypeMemberList args, LC_Type *ret, bool has_vargs, bool has_vargs_any_promotion);
LC_FUNCTION LC_Type       *LC_CreateIncompleteType(LC_Decl *decl);
LC_FUNCTION LC_Type       *LC_CreateUntypedIntEx(LC_Type *base, LC_Decl *decl);
LC_FUNCTION LC_Type       *LC_CreateUntypedInt(LC_Type *base);
LC_FUNCTION LC_TypeMember *LC_AddTypeToList(LC_TypeMemberList *list, LC_Intern name, LC_Type *type, LC_AST *ast);
LC_FUNCTION int            LC_GetLevelsOfIndirection(LC_Type *type);
LC_FUNCTION bool           LC_BigIntFits(LC_BigInt i, LC_Type *type);
LC_FUNCTION LC_Type       *LC_StripPointer(LC_Type *type);

//
// Parsing functions
//
LC_FUNCTION LC_AST *LC_ParseFile(LC_AST *package, char *filename, char *content, int line);
LC_FUNCTION LC_AST *LC_ParseTokens(LC_AST *package, LC_Lex *x);

LC_FUNCTION LC_Parser       LC_MakeParser(LC_Lex *x);
LC_FUNCTION LC_Parser      *LC_MakeParserQuick(char *str);
LC_FUNCTION LC_Token       *LC_Next(void);
LC_FUNCTION LC_Token       *LC_Get(void);
LC_FUNCTION LC_Token       *LC_GetI(int i);
LC_FUNCTION LC_Token       *LC_Is(LC_TokenKind kind);
LC_FUNCTION LC_Token       *LC_IsKeyword(LC_Intern intern);
LC_FUNCTION LC_Token       *LC_Match(LC_TokenKind kind);
LC_FUNCTION LC_Token       *LC_MatchKeyword(LC_Intern intern);
LC_FUNCTION LC_BindingPower LC_MakeBP(int left, int right);
LC_FUNCTION LC_BindingPower LC_GetBindingPower(LC_Binding binding, LC_TokenKind kind);
LC_FUNCTION LC_AST         *LC_ParseExprEx(int min_bp);
LC_FUNCTION LC_AST         *LC_ParseCompo(LC_Token *pos, LC_AST *left);
LC_FUNCTION LC_AST         *LC_ParseExpr(void);
LC_FUNCTION LC_AST         *LC_ParseProcType(LC_Token *pos);
LC_FUNCTION LC_AST         *LC_ParseType(void);
LC_FUNCTION LC_AST         *LC_ParseForStmt(LC_Token *pos);
LC_FUNCTION LC_AST         *LC_ParseSwitchStmt(LC_Token *pos);
LC_FUNCTION LC_AST         *LC_ParseStmt(bool check_semicolon);
LC_FUNCTION LC_AST         *LC_ParseStmtBlock(int flags);
LC_FUNCTION LC_AST         *LC_ParseProcDecl(LC_Token *name);
LC_FUNCTION LC_AST         *LC_ParseStruct(LC_ASTKind kind, LC_Token *ident);
LC_FUNCTION LC_AST         *LC_ParseTypedef(LC_Token *ident);
LC_FUNCTION LC_AST         *LC_CreateNote(LC_Token *pos, LC_Intern ident);
LC_FUNCTION LC_AST         *LC_ParseNote(void);
LC_FUNCTION LC_AST         *LC_ParseNotes(void);
LC_FUNCTION bool            LC_ResolveBuildIf(LC_AST *build_if);
LC_FUNCTION LC_AST         *LC_ParseImport(void);
LC_FUNCTION LC_AST         *LC_ParseDecl(LC_AST *file);
LC_FUNCTION bool            LC_EatUntilNextValidDecl(void);
LC_FUNCTION bool            LC_ParseHashBuildOn(LC_AST *n);
LC_FUNCTION LC_AST         *LC_ParseFileEx(LC_AST *package);

//
// Resolution functions
//
LC_FUNCTION void          LC_AddDecl(LC_DeclStack *scope, LC_Decl *decl);
LC_FUNCTION void          LC_InitDeclStack(LC_DeclStack *stack, int size);
LC_FUNCTION LC_DeclStack *LC_CreateDeclStack(int size);
LC_FUNCTION LC_Decl      *LC_FindDeclOnStack(LC_DeclStack *scp, LC_Intern name);

LC_FUNCTION DeclScope *LC_CreateScope(int size);
LC_FUNCTION LC_Decl   *LC_CreateDecl(LC_DeclKind kind, LC_Intern name, LC_AST *n);
LC_FUNCTION LC_Operand LC_AddDeclToScope(DeclScope *scp, LC_Decl *decl);
LC_FUNCTION LC_Decl   *LC_FindDeclInScope(DeclScope *scope, LC_Intern name);
LC_FUNCTION LC_Operand LC_ThereIsNoDecl(DeclScope *scp, LC_Decl *decl, bool check_locals);
LC_FUNCTION void       LC_MarkDeclError(LC_Decl *decl);
LC_FUNCTION LC_Decl   *LC_GetLocalOrGlobalDecl(LC_Intern name);
LC_FUNCTION LC_Operand LC_PutGlobalDecl(LC_Decl *decl);
LC_FUNCTION LC_Operand LC_CreateLocalDecl(LC_DeclKind kind, LC_Intern name, LC_AST *ast);
LC_FUNCTION LC_Decl   *LC_AddConstIntDecl(char *key, int64_t value);
LC_FUNCTION LC_Decl   *LC_GetBuiltin(LC_Intern name);
LC_FUNCTION void       LC_AddBuiltinConstInt(char *key, int64_t value);

LC_FUNCTION void                       LC_RegisterDeclsFromFile(LC_AST *file);
LC_FUNCTION void                       LC_ResolveDeclsFromFile(LC_AST *file);
LC_FUNCTION void                       LC_PackageDecls(LC_AST *package);
LC_FUNCTION void                       LC_ResolveProcBodies(LC_AST *package);
LC_FUNCTION void                       LC_ResolveIncompleteTypes(LC_AST *package);
LC_FUNCTION LC_Operand                 LC_ResolveNote(LC_AST *n, bool is_decl);
LC_FUNCTION LC_Operand                 LC_ResolveProcBody(LC_Decl *decl);
LC_FUNCTION LC_ResolvedCompoItem      *LC_AddResolvedCallItem(LC_ResolvedCompo *list, LC_TypeMember *t, LC_AST *comp, LC_AST *expr);
LC_FUNCTION LC_Operand                 LC_ResolveCompoCall(LC_AST *n, LC_Type *type);
LC_FUNCTION LC_Operand                 LC_ResolveCompoAggregate(LC_AST *n, LC_Type *type);
LC_FUNCTION LC_ResolvedCompoArrayItem *LC_AddResolvedCompoArrayItem(LC_ResolvedArrayCompo *arr, int index, LC_AST *comp);
LC_FUNCTION LC_Operand                 LC_ResolveCompoArray(LC_AST *n, LC_Type *type);
LC_FUNCTION LC_Operand                 LC_ResolveTypeOrExpr(LC_AST *n);
LC_FUNCTION LC_Operand                 LC_ExpectBuiltinWithOneArg(LC_AST *n);
LC_FUNCTION LC_Operand                 LC_ResolveBuiltin(LC_AST *n);
LC_FUNCTION bool                       LC_TryTyping(LC_AST *n, LC_Operand op);
LC_FUNCTION bool                       LC_TryDefaultTyping(LC_AST *n, LC_Operand *o);
LC_FUNCTION LC_Operand                 LC_ResolveNameInScope(LC_AST *n, LC_Decl *parent_decl);
LC_FUNCTION LC_Operand                 LC_ResolveExpr(LC_AST *expr);
LC_FUNCTION LC_Operand                 LC_ResolveExprAndPushCompoContext(LC_AST *expr, LC_Type *type);
LC_FUNCTION LC_Operand                 LC_ResolveExprEx(LC_AST *n);
LC_FUNCTION LC_Operand                 LC_ResolveStmtBlock(LC_AST *n);
LC_FUNCTION LC_Operand                 LC_ResolveVarDecl(LC_Decl *decl);
LC_FUNCTION LC_Operand                 LC_MakeSureNoDeferBlock(LC_AST *n, char *str);
LC_FUNCTION LC_Operand                 LC_MakeSureInsideLoopBlock(LC_AST *n, char *str);
LC_FUNCTION LC_Operand                 LC_MatchLabeledBlock(LC_AST *n);
LC_FUNCTION LC_Operand                 LC_ResolveStmt(LC_AST *n);
LC_FUNCTION LC_Operand                 LC_ResolveConstDecl(LC_Decl *decl);
LC_FUNCTION LC_Operand                 LC_ResolveName(LC_AST *pos, LC_Intern intern);
LC_FUNCTION LC_Operand                 LC_ResolveConstInt(LC_AST *n, LC_Type *int_type, uint64_t *out_size);
LC_FUNCTION LC_Operand                 LC_ResolveType(LC_AST *n);
LC_FUNCTION LC_Operand                 LC_ResolveBinaryExpr(LC_AST *n, LC_Operand l, LC_Operand r);
LC_FUNCTION LC_Operand                 LC_ResolveTypeVargs(LC_AST *pos, LC_Operand v);
LC_FUNCTION LC_Operand                 LC_ResolveTypeCast(LC_AST *pos, LC_Operand t, LC_Operand v);
LC_FUNCTION LC_Operand                 LC_ResolveTypeVarDecl(LC_AST *pos, LC_Operand t, LC_Operand v);
LC_FUNCTION LC_Operand                 LC_ResolveTypeAggregate(LC_AST *pos, LC_Type *type);

LC_FUNCTION LC_Operand  LC_OPError(void);
LC_FUNCTION LC_Operand  LC_OPConstType(LC_Type *type);
LC_FUNCTION LC_Operand  LC_OPDecl(LC_Decl *decl);
LC_FUNCTION LC_Operand  LC_OPType(LC_Type *type);
LC_FUNCTION LC_Operand  LC_OPLValueAndType(LC_Type *type);
LC_FUNCTION LC_Operand  LC_ConstCastFloat(LC_AST *pos, LC_Operand op);
LC_FUNCTION LC_Operand  LC_ConstCastInt(LC_AST *pos, LC_Operand op);
LC_FUNCTION LC_Operand  LC_OPInt(int64_t v);
LC_FUNCTION LC_Operand  LC_OPIntT(LC_Type *type, int64_t v);
LC_FUNCTION LC_Operand  LC_OPModDefaultUT(LC_Operand val);
LC_FUNCTION LC_Operand  LC_OPModType(LC_Operand op, LC_Type *type);
LC_FUNCTION LC_Operand  LC_OPModBool(LC_Operand op);
LC_FUNCTION LC_Operand  LC_OPModBoolV(LC_Operand op, int v);
LC_FUNCTION LC_Operand  LC_EvalBinary(LC_AST *pos, LC_Operand a, LC_TokenKind op, LC_Operand b);
LC_FUNCTION LC_Operand  LC_EvalUnary(LC_AST *pos, LC_TokenKind op, LC_Operand a);
LC_FUNCTION LC_OPResult LC_IsBinaryExprValidForType(LC_TokenKind op, LC_Type *type);
LC_FUNCTION LC_OPResult LC_IsUnaryOpValidForType(LC_TokenKind op, LC_Type *type);
LC_FUNCTION LC_OPResult LC_IsAssignValidForType(LC_TokenKind op, LC_Type *type);

//
// Error
//
LC_FUNCTION void       LC_IgnoreMessage(LC_Token *pos, char *str, int len);
LC_FUNCTION void       LC_SendErrorMessage(LC_Token *pos, LC_String s8);
LC_FUNCTION void       LC_SendErrorMessagef(LC_Lex *x, LC_Token *pos, const char *str, ...);
LC_FUNCTION LC_AST    *LC_ReportParseError(LC_Token *pos, const char *str, ...);
LC_FUNCTION LC_Operand LC_ReportASTError(LC_AST *n, const char *str, ...);
LC_FUNCTION LC_Operand LC_ReportASTErrorEx(LC_AST *n1, LC_AST *n2, const char *str, ...);
LC_FUNCTION void       LC_HandleFatalError(void);

#define LC_ASSERT(n, cond)                                                                                               \
    if (!(cond)) {                                                                                                       \
        LC_ReportASTError(n, "internal compiler error: assert condition failed: %s, inside of %s", #cond, __FUNCTION__); \
        LC_HandleFatalError();                                                                                           \
    }

//
// Code generation and printing helpers
//
LC_FUNCTION LC_StringList *LC_BeginStringGen(LC_Arena *arena);
LC_FUNCTION LC_String      LC_EndStringGen(LC_Arena *arena);

// clang-format off
#define LC_Genf(...) LC_Addf(L->printer.arena, &L->printer.list, __VA_ARGS__)
#define LC_GenLinef(...) do { LC_Genf("\n"); LC_GenIndent(); LC_Genf(__VA_ARGS__); } while (0)
// clang-format on

LC_FUNCTION void  LC_GenIndent(void);
LC_FUNCTION void  LC_GenLine(void);
LC_FUNCTION char *LC_Strf(const char *str, ...);
LC_FUNCTION char *LC_GenLCType(LC_Type *type);
LC_FUNCTION char *LC_GenLCTypeVal(LC_TypeAndVal v);
LC_FUNCTION char *LC_GenLCAggName(LC_Type *t);
LC_FUNCTION void  LC_GenLCNode(LC_AST *n);

// C code generation
LC_FUNCTION void LC_GenCHeader(LC_AST *package);
LC_FUNCTION void LC_GenCImpl(LC_AST *package);

LC_FUNCTION void LC_GenCLineDirective(LC_AST *node);
LC_FUNCTION void LC_GenLastCLineDirective(void);
LC_FUNCTION void LC_GenCLineDirectiveNum(int num);

LC_FUNCTION char     *LC_GenCTypeParen(char *str, char c);
LC_FUNCTION char     *LC_GenCType(LC_Type *type, char *str);
LC_FUNCTION LC_Intern LC_GetStringFromSingleArgNote(LC_AST *note);
LC_FUNCTION void      LC_GenCCompound(LC_AST *n);
LC_FUNCTION void      LC_GenCString(char *s, LC_Type *type);
LC_FUNCTION char     *LC_GenCVal(LC_TypeAndVal v, LC_Type *type);
LC_FUNCTION void      LC_GenCExpr(LC_AST *n);
LC_FUNCTION void      LC_GenCNote(LC_AST *note);
LC_FUNCTION void      LC_GenCVarExpr(LC_AST *n, bool is_declaration);
LC_FUNCTION void      LC_GenCDefers(LC_AST *block);
LC_FUNCTION void      LC_GenCDefersLoopBreak(LC_AST *n);
LC_FUNCTION void      LC_GenCDefersReturn(LC_AST *n);
LC_FUNCTION void      LC_GenCStmt2(LC_AST *n, int flags);
LC_FUNCTION void      LC_GenCStmt(LC_AST *n);
LC_FUNCTION void      LC_GenCExprParen(LC_AST *expr);
LC_FUNCTION void      LC_GenCStmtBlock(LC_AST *n);
LC_FUNCTION void      LC_GenCProcDecl(LC_Decl *decl);
LC_FUNCTION void      LC_GenCAggForwardDecl(LC_Decl *decl);
LC_FUNCTION void      LC_GenCTypeDecl(LC_Decl *decl);
LC_FUNCTION void      LC_GenCVarFDecl(LC_Decl *decl);

//
// Inline helpers
//
static inline bool LC_IsUTInt(LC_Type *x) { return x->kind == LC_TypeKind_UntypedInt; }
static inline bool LC_IsUTFloat(LC_Type *x) { return x->kind == LC_TypeKind_UntypedFloat; }
static inline bool LC_IsUTStr(LC_Type *x) { return x->kind == LC_TypeKind_UntypedString; }
static inline bool LC_IsUntyped(LC_Type *x) { return (x)->kind >= LC_TypeKind_UntypedInt && x->kind <= LC_TypeKind_UntypedString; }

static inline bool LC_IsNum(LC_Type *x) { return x->kind >= LC_TypeKind_char && x->kind <= LC_TypeKind_double; }
static inline bool LC_IsInt(LC_Type *x) { return (x->kind >= LC_TypeKind_char && x->kind <= LC_TypeKind_ullong) || LC_IsUTInt(x); }
static inline bool LC_IsIntLike(LC_Type *x) { return LC_IsInt(x) || x->kind == LC_TypeKind_Pointer || x->kind == LC_TypeKind_Proc; }
static inline bool LC_IsFloat(LC_Type *x) { return ((x)->kind == LC_TypeKind_float || (x->kind == LC_TypeKind_double)) || LC_IsUTFloat(x); }
static inline bool LC_IsSmallerThenInt(LC_Type *x) { return x->kind < LC_TypeKind_int; }

static inline bool LC_IsAggType(LC_Type *x) { return ((x)->kind == LC_TypeKind_Struct || (x)->kind == LC_TypeKind_Union); }
static inline bool LC_IsArray(LC_Type *x) { return (x)->kind == LC_TypeKind_Array; }
static inline bool LC_IsProc(LC_Type *x) { return (x)->kind == LC_TypeKind_Proc; }
static inline bool LC_IsVoidPtr(LC_Type *x) { return (x) == L->tpvoid; }
static inline bool LC_IsPtr(LC_Type *x) { return (x)->kind == LC_TypeKind_Pointer; }
static inline bool LC_IsPtrLike(LC_Type *x) { return (x)->kind == LC_TypeKind_Pointer || x->kind == LC_TypeKind_Proc; }
static inline bool LC_IsStr(LC_Type *x) { return x == L->tpchar || x == L->tstring; }

static inline bool LC_IsDecl(LC_AST *x) { return (x->kind >= LC_ASTKind_FirstDecl && x->kind <= LC_ASTKind_LastDecl); }
static inline bool LC_IsStmt(LC_AST *x) { return (x->kind >= LC_ASTKind_FirstStmt && x->kind <= LC_ASTKind_LastStmt); }
static inline bool LC_IsExpr(LC_AST *x) { return (x->kind >= LC_ASTKind_FirstExpr && x->kind <= LC_ASTKind_LastExpr); }
static inline bool LC_IsType(LC_AST *x) { return (x->kind >= LC_ASTKind_FirstTypespec && x->kind <= LC_ASTKind_LastTypespec); }
static inline bool LC_IsAgg(LC_AST *x) { return (x->kind == LC_ASTKind_DeclStruct || x->kind == LC_ASTKind_DeclUnion); }

// I tried removing this because I thought it's redundant
// but this reminded me that "Untyped bool" can appear from normal expressions: (a == b)
// This is required, maybe there is a way around it, not sure
static inline bool LC_IsUTConst(LC_Operand op) { return (op.flags & LC_OPF_UTConst) != 0; }
static inline bool LC_IsConst(LC_Operand op) { return (op.flags & LC_OPF_Const) != 0; }
static inline bool LC_IsLValue(LC_Operand op) { return (op.flags & LC_OPF_LValue) != 0; }
static inline bool LC_IsError(LC_Operand op) { return (op.flags & LC_OPF_Error) != 0; }

static inline LC_Type *LC_GetBase(LC_Type *x) { return (x)->tbase; }

//
// Stringifying functions
//
LC_FUNCTION const char *LC_OSToString(LC_OS os);
LC_FUNCTION const char *LC_GENToString(LC_GEN os);
LC_FUNCTION const char *LC_ARCHToString(LC_ARCH arch);
LC_FUNCTION const char *LC_ASTKindToString(LC_ASTKind kind);
LC_FUNCTION const char *LC_TypeKindToString(LC_TypeKind kind);
LC_FUNCTION const char *LC_DeclKindToString(LC_DeclKind decl_kind);
LC_FUNCTION const char *LC_TokenKindToString(LC_TokenKind token_kind);
LC_FUNCTION const char *LC_TokenKindToOperator(LC_TokenKind token_kind);

//
// bigint functions
//
LC_FUNCTION LC_BigInt LC_Bigint_u64(uint64_t val);
LC_FUNCTION uint64_t *LC_Bigint_ptr(LC_BigInt *big_int);
LC_FUNCTION size_t    LC_Bigint_bits_needed(LC_BigInt *big_int);
LC_FUNCTION void      LC_Bigint_init_unsigned(LC_BigInt *big_int, uint64_t value);
LC_FUNCTION void      LC_Bigint_init_signed(LC_BigInt *dest, int64_t value);
LC_FUNCTION void      LC_Bigint_init_bigint(LC_BigInt *dest, LC_BigInt *src);
LC_FUNCTION void      LC_Bigint_negate(LC_BigInt *dest, LC_BigInt *source);
LC_FUNCTION size_t    LC_Bigint_clz(LC_BigInt *big_int, size_t bit_count);
LC_FUNCTION bool      LC_Bigint_eql(LC_BigInt a, LC_BigInt b);
LC_FUNCTION bool      LC_Bigint_fits_in_bits(LC_BigInt *big_int, size_t bit_count, bool is_signed);
LC_FUNCTION uint64_t  LC_Bigint_as_unsigned(LC_BigInt *bigint);
LC_FUNCTION void      LC_Bigint_add(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2);
LC_FUNCTION void      LC_Bigint_add_wrap(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2, size_t bit_count, bool is_signed);
LC_FUNCTION void      LC_Bigint_sub(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2);
LC_FUNCTION void      LC_Bigint_sub_wrap(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2, size_t bit_count, bool is_signed);
LC_FUNCTION void      LC_Bigint_mul(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2);
LC_FUNCTION void      LC_Bigint_mul_wrap(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2, size_t bit_count, bool is_signed);
LC_FUNCTION void      LC_Bigint_unsigned_division(LC_BigInt *op1, LC_BigInt *op2, LC_BigInt *Quotient, LC_BigInt *Remainder);
LC_FUNCTION void      LC_Bigint_div_trunc(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2);
LC_FUNCTION void      LC_Bigint_div_floor(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2);
LC_FUNCTION void      LC_Bigint_rem(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2);
LC_FUNCTION void      LC_Bigint_mod(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2);
LC_FUNCTION void      LC_Bigint_or(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2);
LC_FUNCTION void      LC_Bigint_and(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2);
LC_FUNCTION void      LC_Bigint_xor(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2);
LC_FUNCTION void      LC_Bigint_shl(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2);
LC_FUNCTION void      LC_Bigint_shl_int(LC_BigInt *dest, LC_BigInt *op1, uint64_t shift);
LC_FUNCTION void      LC_Bigint_shl_trunc(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2, size_t bit_count, bool is_signed);
LC_FUNCTION void      LC_Bigint_shr(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2);
LC_FUNCTION void      LC_Bigint_not(LC_BigInt *dest, LC_BigInt *op, size_t bit_count, bool is_signed);
LC_FUNCTION void      LC_Bigint_truncate(LC_BigInt *dst, LC_BigInt *op, size_t bit_count, bool is_signed);
LC_FUNCTION LC_CmpRes LC_Bigint_cmp(LC_BigInt *op1, LC_BigInt *op2);
LC_FUNCTION char     *LC_Bigint_str(LC_BigInt *bigint, uint64_t base);
LC_FUNCTION int64_t   LC_Bigint_as_signed(LC_BigInt *bigint);
LC_FUNCTION LC_CmpRes LC_Bigint_cmp_zero(LC_BigInt *op);
LC_FUNCTION double    LC_Bigint_as_float(LC_BigInt *bigint);

//
// Unicode API
//
struct LC_UTF32Result {
    uint32_t out_str;
    int      advance;
    int      error;
};

struct LC_UTF8Result {
    uint8_t out_str[4];
    int     len;
    int     error;
};

struct LC_UTF16Result {
    uint16_t out_str[2];
    int      len;
    int      error;
};

LC_FUNCTION LC_UTF32Result LC_ConvertUTF16ToUTF32(uint16_t *c, int max_advance);
LC_FUNCTION LC_UTF8Result  LC_ConvertUTF32ToUTF8(uint32_t codepoint);
LC_FUNCTION LC_UTF32Result LC_ConvertUTF8ToUTF32(char *c, int max_advance);
LC_FUNCTION LC_UTF16Result LC_ConvertUTF32ToUTF16(uint32_t codepoint);
LC_FUNCTION int64_t        LC_CreateCharFromWidechar(char *buffer, int64_t buffer_size, wchar_t *in, int64_t inlen);
LC_FUNCTION int64_t        LC_CreateWidecharFromChar(wchar_t *buffer, int64_t buffer_size, char *in, int64_t inlen);

//
// Filesystem API
//
LC_FUNCTION bool      LC_IsDir(LC_Arena *temp, LC_String path);
LC_FUNCTION LC_String LC_GetAbsolutePath(LC_Arena *arena, LC_String relative);
LC_FUNCTION bool      LC_EnableTerminalColors(void);
LC_FUNCTION LC_String LC_ReadFile(LC_Arena *arena, LC_String path);

LC_FUNCTION bool        LC_IsValid(LC_FileIter it);
LC_FUNCTION void        LC_Advance(LC_FileIter *it);
LC_FUNCTION LC_FileIter LC_IterateFiles(LC_Arena *scratch_arena, LC_String path);

//
// Arena API
//
#define LC_PushSize(a, size) LC__PushSize(a, size)
#define LC_PushSizeNonZeroed(a, size) LC__PushSizeNonZeroed(a, size)

#define LC_PushArrayNonZeroed(a, T, c) (T *)LC__PushSizeNonZeroed(a, sizeof(T) * (c))
#define LC_PushStructNonZeroed(a, T) (T *)LC__PushSizeNonZeroed(a, sizeof(T))
#define LC_PushStruct(a, T) (T *)LC__PushSize(a, sizeof(T))
#define LC_PushArray(a, T, c) (T *)LC__PushSize(a, sizeof(T) * (c))

#define LC_Assertf(cond, ...) LC_ASSERT(NULL, cond)

#ifndef LC_USE_CUSTOM_ARENA
LC_FUNCTION void      LC_InitArenaEx(LC_Arena *a, size_t reserve);
LC_FUNCTION void      LC_InitArena(LC_Arena *a);
LC_FUNCTION void      LC_InitArenaFromBuffer(LC_Arena *arena, void *buffer, size_t size);
LC_FUNCTION LC_Arena *LC_BootstrapArena(void);
LC_FUNCTION LC_Arena  LC_PushArena(LC_Arena *arena, size_t size);
LC_FUNCTION LC_Arena *LC_PushArenaP(LC_Arena *arena, size_t size);

LC_FUNCTION void        *LC__PushSizeNonZeroed(LC_Arena *a, size_t size);
LC_FUNCTION void        *LC__PushSize(LC_Arena *arena, size_t size);
LC_FUNCTION LC_TempArena LC_BeginTemp(LC_Arena *arena);
LC_FUNCTION void         LC_EndTemp(LC_TempArena checkpoint);

LC_FUNCTION void LC_PopToPos(LC_Arena *arena, size_t pos);
LC_FUNCTION void LC_PopSize(LC_Arena *arena, size_t size);
LC_FUNCTION void LC_DeallocateArena(LC_Arena *arena);
LC_FUNCTION void LC_ResetArena(LC_Arena *arena);

LC_FUNCTION LC_VMemory LC_VReserve(size_t size);
LC_FUNCTION bool       LC_VCommit(LC_VMemory *m, size_t commit);
LC_FUNCTION void       LC_VDeallocate(LC_VMemory *m);
LC_FUNCTION bool       LC_VDecommitPos(LC_VMemory *m, size_t pos);
#endif // LC_USE_CUSTOM_ARENA

LC_FUNCTION size_t LC_GetAlignOffset(size_t size, size_t align);
LC_FUNCTION size_t LC_AlignUp(size_t size, size_t align);
LC_FUNCTION size_t LC_AlignDown(size_t size, size_t align);

#define LC_IS_POW2(x) (((x) & ((x)-1)) == 0)
#define LC_MIN(x, y) ((x) <= (y) ? (x) : (y))
#define LC_MAX(x, y) ((x) >= (y) ? (x) : (y))
#define LC_StrLenof(x) ((int64_t)((sizeof(x) / sizeof((x)[0]))))

#define LC_CLAMP_TOP(x, max) ((x) >= (max) ? (max) : (x))
#define LC_CLAMP_BOT(x, min) ((x) <= (min) ? (min) : (x))
#define LC_CLAMP(x, min, max) ((x) >= (max) ? (max) : (x) <= (min) ? (min) \
                                                                   : (x))
#define LC_KIB(x) ((x##ull) * 1024ull)
#define LC_MIB(x) (LC_KIB(x) * 1024ull)
#define LC_GIB(x) (LC_MIB(x) * 1024ull)
#define LC_TIB(x) (LC_GIB(x) * 1024ull)

//
// String API
//

typedef int LC_FindFlag;
enum {
    LC_FindFlag_None          = 0,
    LC_FindFlag_IgnoreCase    = 1,
    LC_FindFlag_MatchFindLast = 2,
};

typedef int LC_SplitFlag;
enum {
    LC_SplitFlag_None           = 0,
    LC_SplitFlag_IgnoreCase     = 1,
    LC_SplitFlag_SplitInclusive = 2,
};

static const bool LC_IgnoreCase = true;

#if defined(__has_attribute)
    #if __has_attribute(format)
        #define LC__PrintfFormat(fmt, va) __attribute__((format(printf, fmt, va)))
    #endif
#endif

#ifndef LC__PrintfFormat
    #define LC__PrintfFormat(fmt, va)
#endif

#define LC_Lit(string) LC_MakeString((char *)string, sizeof(string) - 1)
#define LC_Expand(string) (int)(string).len, (string).str

#define LC_FORMAT(allocator, str, result)                 \
    va_list args1;                                        \
    va_start(args1, str);                                 \
    LC_String result = LC_FormatV(allocator, str, args1); \
    va_end(args1)

#ifdef __cplusplus
    #define LC_IF_CPP(x) x
#else
    #define LC_IF_CPP(x)
#endif

LC_FUNCTION char LC_ToLowerCase(char a);
LC_FUNCTION char LC_ToUpperCase(char a);
LC_FUNCTION bool LC_IsWhitespace(char w);
LC_FUNCTION bool LC_IsAlphabetic(char a);
LC_FUNCTION bool LC_IsIdent(char a);
LC_FUNCTION bool LC_IsDigit(char a);
LC_FUNCTION bool LC_IsAlphanumeric(char a);

LC_FUNCTION int64_t       LC_StrLen(char *string);
LC_FUNCTION bool          LC_AreEqual(LC_String a, LC_String b, unsigned ignore_case LC_IF_CPP(= false));
LC_FUNCTION bool          LC_EndsWith(LC_String a, LC_String end, unsigned ignore_case LC_IF_CPP(= false));
LC_FUNCTION bool          LC_StartsWith(LC_String a, LC_String start, unsigned ignore_case LC_IF_CPP(= false));
LC_FUNCTION LC_String     LC_MakeString(char *str, int64_t len);
LC_FUNCTION LC_String     LC_CopyString(LC_Arena *allocator, LC_String string);
LC_FUNCTION LC_String     LC_CopyChar(LC_Arena *allocator, char *s);
LC_FUNCTION LC_String     LC_NormalizePath(LC_Arena *allocator, LC_String s);
LC_FUNCTION void          LC_NormalizePathUnsafe(LC_String s); // make sure there is no way string is const etc.
LC_FUNCTION LC_String     LC_Chop(LC_String string, int64_t len);
LC_FUNCTION LC_String     LC_Skip(LC_String string, int64_t len);
LC_FUNCTION LC_String     LC_GetPostfix(LC_String string, int64_t len);
LC_FUNCTION LC_String     LC_GetPrefix(LC_String string, int64_t len);
LC_FUNCTION bool          LC_Seek(LC_String string, LC_String find, LC_FindFlag flags LC_IF_CPP(= LC_FindFlag_None), int64_t *index_out LC_IF_CPP(= 0));
LC_FUNCTION int64_t       LC_Find(LC_String string, LC_String find, LC_FindFlag flags LC_IF_CPP(= LC_FindFlag_None));
LC_FUNCTION LC_String     LC_ChopLastSlash(LC_String s);
LC_FUNCTION LC_String     LC_ChopLastPeriod(LC_String s);
LC_FUNCTION LC_String     LC_SkipToLastSlash(LC_String s);
LC_FUNCTION LC_String     LC_SkipToLastPeriod(LC_String s);
LC_FUNCTION LC_String     LC_GetNameNoExt(LC_String s);
LC_FUNCTION LC_String     LC_MakeFromChar(char *string);
LC_FUNCTION LC_String     LC_MakeEmptyString(void);
LC_FUNCTION LC_StringList LC_MakeEmptyList(void);
LC_FUNCTION LC_String     LC_FormatV(LC_Arena *allocator, const char *str, va_list args1);
LC_FUNCTION LC_String     LC_Format(LC_Arena *allocator, const char *str, ...) LC__PrintfFormat(2, 3);

LC_FUNCTION LC_StringList LC_Split(LC_Arena *allocator, LC_String string, LC_String find, LC_SplitFlag flags LC_IF_CPP(= LC_SplitFlag_None));
LC_FUNCTION LC_String     LC_MergeWithSeparator(LC_Arena *allocator, LC_StringList list, LC_String separator LC_IF_CPP(= LC_Lit(" ")));
LC_FUNCTION LC_String     LC_MergeString(LC_Arena *allocator, LC_StringList list);

LC_FUNCTION LC_StringNode *LC_CreateNode(LC_Arena *allocator, LC_String string);
LC_FUNCTION void           LC_ReplaceNodeString(LC_StringList *list, LC_StringNode *node, LC_String new_string);
LC_FUNCTION void           LC_AddExistingNode(LC_StringList *list, LC_StringNode *node);
LC_FUNCTION void           LC_AddArray(LC_Arena *allocator, LC_StringList *list, char **array, int count);
LC_FUNCTION void           LC_AddArrayWithPrefix(LC_Arena *allocator, LC_StringList *list, char *prefix, char **array, int count);
LC_FUNCTION LC_StringList  LC_MakeList(LC_Arena *allocator, LC_String a);
LC_FUNCTION LC_StringList  LC_CopyList(LC_Arena *allocator, LC_StringList a);
LC_FUNCTION LC_StringList  LC_ConcatLists(LC_Arena *allocator, LC_StringList a, LC_StringList b);
LC_FUNCTION LC_StringNode *LC_AddNode(LC_Arena *allocator, LC_StringList *list, LC_String string);
LC_FUNCTION LC_StringNode *LC_Add(LC_Arena *allocator, LC_StringList *list, LC_String string);
LC_FUNCTION LC_String      LC_Addf(LC_Arena *allocator, LC_StringList *list, const char *str, ...) LC__PrintfFormat(3, 4);

LC_FUNCTION wchar_t *LC_ToWidechar(LC_Arena *allocator, LC_String string);

//
// Linked list API
//
#define LC_SLLAddMod(f, l, n, next) \
    do {                            \
        (n)->next = 0;              \
        if ((f) == 0) {             \
            (f) = (l) = (n);        \
        } else {                    \
            (l) = (l)->next = (n);  \
        }                           \
    } while (0)
#define LC_AddSLL(f, l, n) LC_SLLAddMod(f, l, n, next)

#define LC_SLLPopFirstMod(f, l, next) \
    do {                              \
        if ((f) == (l)) {             \
            (f) = (l) = 0;            \
        } else {                      \
            (f) = (f)->next;          \
        }                             \
    } while (0)
#define LC_SLLPopFirst(f, l) LC_SLLPopFirstMod(f, l, next)

#define LC_SLLStackAddMod(stack_base, new_stack_base, next) \
    do {                                                    \
        (new_stack_base)->next = (stack_base);              \
        (stack_base)           = (new_stack_base);          \
    } while (0)

#define LC_DLLAddMod(f, l, node, next, prev) \
    do {                                     \
        if ((f) == 0) {                      \
            (f) = (l)    = (node);           \
            (node)->prev = 0;                \
            (node)->next = 0;                \
        } else {                             \
            (l)->next    = (node);           \
            (node)->prev = (l);              \
            (node)->next = 0;                \
            (l)          = (node);           \
        }                                    \
    } while (0)
#define LC_DLLAdd(f, l, node) LC_DLLAddMod(f, l, node, next, prev)
#define LC_DLLAddFront(f, l, node) LC_DLLAddMod(l, f, node, prev, next)
#define LC_DLLRemoveMod(first, last, node, next, prev) \
    do {                                               \
        if ((first) == (last)) {                       \
            (first) = (last) = 0;                      \
        } else if ((last) == (node)) {                 \
            (last)       = (last)->prev;               \
            (last)->next = 0;                          \
        } else if ((first) == (node)) {                \
            (first)       = (first)->next;             \
            (first)->prev = 0;                         \
        } else {                                       \
            (node)->prev->next = (node)->next;         \
            (node)->next->prev = (node)->prev;         \
        }                                              \
        if (node) {                                    \
            (node)->prev = 0;                          \
            (node)->next = 0;                          \
        }                                              \
    } while (0)
#define LC_DLLRemove(first, last, node) LC_DLLRemoveMod(first, last, node, next, prev)

#define LC_ASTFor(it, x) for (LC_AST *it = x; it; it = it->next)
#define LC_DeclFor(it, x) for (LC_Decl *it = x; it; it = it->next)
#define LC_TypeFor(it, x) for (LC_TypeMember *it = x; it; it = it->next)

#if defined(__APPLE__) && defined(__MACH__)
    #define LC_OPERATING_SYSTEM_MAC 1
    #define LC_OPERATING_SYSTEM_UNIX 1
#elif defined(_WIN32)
    #define LC_OPERATING_SYSTEM_WINDOWS 1
#elif defined(__linux__)
    #define LC_OPERATING_SYSTEM_UNIX 1
    #define LC_OPERATING_SYSTEM_LINUX 1
#endif

#ifndef LC_OPERATING_SYSTEM_MAC
    #define LC_OPERATING_SYSTEM_MAC 0
#endif

#ifndef LC_OPERATING_SYSTEM_UNIX
    #define LC_OPERATING_SYSTEM_UNIX 0
#endif

#ifndef LC_OPERATING_SYSTEM_WINDOWS
    #define LC_OPERATING_SYSTEM_WINDOWS 0
#endif

#ifndef LC_OPERATING_SYSTEM_LINUX
    #define LC_OPERATING_SYSTEM_LINUX 0
#endif

#endif
#ifdef LIB_COMPILER_IMPLEMENTATION


#if __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wswitch"
    #pragma clang diagnostic ignored "-Wwritable-strings"
#endif

#ifndef LC_ParseFloat // @override
    #include <stdlib.h>
    #define LC_ParseFloat(str, len) strtod(str, NULL)
#endif

#ifndef LC_Print // @override
    #include <stdio.h>
    #define LC_Print(str, len) printf("%.*s", (int)len, str)
#endif

#ifndef LC_Exit // @override
    #include <stdio.h>
    #define LC_Exit(x) exit(x)
#endif

#ifndef LC_MemoryZero // @override
    #include <string.h>
    #define LC_MemoryZero(p, size) memset(p, 0, size)
#endif

#ifndef LC_MemoryCopy // @override
    #include <string.h>
    #define LC_MemoryCopy(dst, src, size) memcpy(dst, src, size);
#endif

#ifndef LC_vsnprintf // @override
    #include <stdio.h>
    #define LC_vsnprintf vsnprintf
#endif

#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#endif

LC_THREAD_LOCAL LC_Lang *L;

LC_FUNCTION LC_UTF32Result LC_ConvertUTF16ToUTF32(uint16_t *c, int max_advance) {
    LC_UTF32Result result;
    LC_MemoryZero(&result, sizeof(result));
    if (max_advance >= 1) {
        result.advance = 1;
        result.out_str = c[0];
        if (c[0] >= 0xD800 && c[0] <= 0xDBFF && c[1] >= 0xDC00 && c[1] <= 0xDFFF) {
            if (max_advance >= 2) {
                result.out_str = 0x10000;
                result.out_str += (uint32_t)(c[0] & 0x03FF) << 10u | (c[1] & 0x03FF);
                result.advance = 2;
            } else
                result.error = 2;
        }
    } else {
        result.error = 1;
    }
    return result;
}

LC_FUNCTION LC_UTF8Result LC_ConvertUTF32ToUTF8(uint32_t codepoint) {
    LC_UTF8Result result;
    LC_MemoryZero(&result, sizeof(result));

    if (codepoint <= 0x7F) {
        result.len        = 1;
        result.out_str[0] = (char)codepoint;
    } else if (codepoint <= 0x7FF) {
        result.len        = 2;
        result.out_str[0] = 0xc0 | (0x1f & (codepoint >> 6));
        result.out_str[1] = 0x80 | (0x3f & codepoint);
    } else if (codepoint <= 0xFFFF) { // 16 bit word
        result.len        = 3;
        result.out_str[0] = 0xe0 | (0xf & (codepoint >> 12)); // 4 bits
        result.out_str[1] = 0x80 | (0x3f & (codepoint >> 6)); // 6 bits
        result.out_str[2] = 0x80 | (0x3f & codepoint);        // 6 bits
    } else if (codepoint <= 0x10FFFF) {                       // 21 bit word
        result.len        = 4;
        result.out_str[0] = 0xf0 | (0x7 & (codepoint >> 18));  // 3 bits
        result.out_str[1] = 0x80 | (0x3f & (codepoint >> 12)); // 6 bits
        result.out_str[2] = 0x80 | (0x3f & (codepoint >> 6));  // 6 bits
        result.out_str[3] = 0x80 | (0x3f & codepoint);         // 6 bits
    } else {
        result.error = 1;
    }

    return result;
}

LC_FUNCTION LC_UTF32Result LC_ConvertUTF8ToUTF32(char *c, int max_advance) {
    LC_UTF32Result result;
    LC_MemoryZero(&result, sizeof(result));

    if ((c[0] & 0x80) == 0) { // Check if leftmost zero of first byte is unset
        if (max_advance >= 1) {
            result.out_str = c[0];
            result.advance = 1;
        } else result.error = 1;
    }

    else if ((c[0] & 0xe0) == 0xc0) {
        if ((c[1] & 0xc0) == 0x80) { // Continuation byte required
            if (max_advance >= 2) {
                result.out_str = (uint32_t)(c[0] & 0x1f) << 6u | (c[1] & 0x3f);
                result.advance = 2;
            } else result.error = 2;
        } else result.error = 2;
    }

    else if ((c[0] & 0xf0) == 0xe0) {
        if ((c[1] & 0xc0) == 0x80 && (c[2] & 0xc0) == 0x80) { // Two continuation bytes required
            if (max_advance >= 3) {
                result.out_str = (uint32_t)(c[0] & 0xf) << 12u | (uint32_t)(c[1] & 0x3f) << 6u | (c[2] & 0x3f);
                result.advance = 3;
            } else result.error = 3;
        } else result.error = 3;
    }

    else if ((c[0] & 0xf8) == 0xf0) {
        if ((c[1] & 0xc0) == 0x80 && (c[2] & 0xc0) == 0x80 && (c[3] & 0xc0) == 0x80) { // Three continuation bytes required
            if (max_advance >= 4) {
                result.out_str = (uint32_t)(c[0] & 0xf) << 18u | (uint32_t)(c[1] & 0x3f) << 12u | (uint32_t)(c[2] & 0x3f) << 6u | (uint32_t)(c[3] & 0x3f);
                result.advance = 4;
            } else result.error = 4;
        } else result.error = 4;
    } else result.error = 4;

    return result;
}

LC_FUNCTION LC_UTF16Result LC_ConvertUTF32ToUTF16(uint32_t codepoint) {
    LC_UTF16Result result;
    LC_MemoryZero(&result, sizeof(result));
    if (codepoint < 0x10000) {
        result.out_str[0] = (uint16_t)codepoint;
        result.out_str[1] = 0;
        result.len        = 1;
    } else if (codepoint <= 0x10FFFF) {
        uint32_t code     = (codepoint - 0x10000);
        result.out_str[0] = (uint16_t)(0xD800 | (code >> 10));
        result.out_str[1] = (uint16_t)(0xDC00 | (code & 0x3FF));
        result.len        = 2;
    } else {
        result.error = 1;
    }

    return result;
}

#define LC__HANDLE_DECODE_ERROR(question_mark)                            \
    {                                                                     \
        if (outlen < buffer_size - 1) buffer[outlen++] = (question_mark); \
        break;                                                            \
    }

LC_FUNCTION int64_t LC_CreateCharFromWidechar(char *buffer, int64_t buffer_size, wchar_t *in, int64_t inlen) {
    int64_t outlen = 0;
    for (int64_t i = 0; i < inlen && in[i];) {
        LC_UTF32Result decode = LC_ConvertUTF16ToUTF32((uint16_t *)(in + i), (int)(inlen - i));
        if (!decode.error) {
            i += decode.advance;
            LC_UTF8Result encode = LC_ConvertUTF32ToUTF8(decode.out_str);
            if (!encode.error) {
                for (int64_t j = 0; j < encode.len; j++) {
                    if (outlen < buffer_size - 1) {
                        buffer[outlen++] = encode.out_str[j];
                    }
                }
            } else LC__HANDLE_DECODE_ERROR('?');
        } else LC__HANDLE_DECODE_ERROR('?');
    }

    buffer[outlen] = 0;
    return outlen;
}

LC_FUNCTION int64_t LC_CreateWidecharFromChar(wchar_t *buffer, int64_t buffer_size, char *in, int64_t inlen) {
    int64_t outlen = 0;
    for (int64_t i = 0; i < inlen;) {
        LC_UTF32Result decode = LC_ConvertUTF8ToUTF32(in + i, (int)(inlen - i));
        if (!decode.error) {
            i += decode.advance;
            LC_UTF16Result encode = LC_ConvertUTF32ToUTF16(decode.out_str);
            if (!encode.error) {
                for (int64_t j = 0; j < encode.len; j++) {
                    if (outlen < buffer_size - 1) {
                        buffer[outlen++] = encode.out_str[j];
                    }
                }
            } else LC__HANDLE_DECODE_ERROR(0x003f);
        } else LC__HANDLE_DECODE_ERROR(0x003f);
    }

    buffer[outlen] = 0;
    return outlen;
}

#undef LC__HANDLE_DECODE_ERROR
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

LC_FUNCTION const char *LC_OSToString(LC_OS os) {
    switch (os) {
    case LC_OS_WINDOWS: return "OS_WINDOWS";
    case LC_OS_LINUX: return "OS_LINUX";
    case LC_OS_MAC: return "OS_MAC";
    default: return "UNKNOWN_OPERATING_SYSTEM";
    }
}

LC_FUNCTION const char *LC_GENToString(LC_GEN os) {
    switch (os) {
    case LC_GEN_C: return "GEN_C";
    default: return "UNKNOWN_GENERATOR";
    }
}

LC_FUNCTION const char *LC_ARCHToString(LC_ARCH arch) {
    switch (arch) {
    case LC_ARCH_X86: return "ARCH_X86";
    case LC_ARCH_X64: return "ARCH_X64";
    default: return "UNKNOWN_ARCHITECTURE";
    }
}

LC_FUNCTION const char *LC_ASTKindToString(LC_ASTKind kind) {
    static const char *strs[] = {
        "ast null",
        "ast error",
        "ast note",
        "ast note list",
        "ast file",
        "ast package",
        "ast ignore",
        "typespec procdure argument",
        "typespec aggregate member",
        "expr call item",
        "expr compound item",
        "expr note",
        "stmt switch case",
        "stmt switch default",
        "stmt else if",
        "stmt else",
        "import",
        "global note",
        "decl proc",
        "decl struct",
        "decl union",
        "decl var",
        "decl const",
        "decl typedef",
        "typespec ident",
        "typespec field",
        "typespec pointer",
        "typespec array",
        "typespec proc",
        "stmt block",
        "stmt note",
        "stmt return",
        "stmt break",
        "stmt continue",
        "stmt defer",
        "stmt for",
        "stmt if",
        "stmt switch",
        "stmt assign",
        "stmt expr",
        "stmt var",
        "stmt const",
        "expr ident",
        "expr string",
        "expr int",
        "expr float",
        "expr bool",
        "expr type",
        "expr binary",
        "expr unary",
        "expr builtin",
        "expr call",
        "expr compound",
        "expr cast",
        "expr field",
        "expr index",
        "expr pointerindex",
        "expr getvalueofpointer",
        "expr getpointerofvalue",
    };
    if (kind < 0 || kind >= LC_ASTKind_Count) {
        return "<invalid_ast_kind>";
    }
    return strs[kind];
}

LC_FUNCTION const char *LC_TypeKindToString(LC_TypeKind kind) {
    static const char *strs[] = {
        "LC_TypeKind_char",
        "LC_TypeKind_uchar",
        "LC_TypeKind_short",
        "LC_TypeKind_ushort",
        "LC_TypeKind_bool",
        "LC_TypeKind_int",
        "LC_TypeKind_uint",
        "LC_TypeKind_long",
        "LC_TypeKind_ulong",
        "LC_TypeKind_llong",
        "LC_TypeKind_ullong",
        "LC_TypeKind_float",
        "LC_TypeKind_double",
        "LC_TypeKind_void",
        "LC_TypeKind_Struct",
        "LC_TypeKind_Union",
        "LC_TypeKind_Pointer",
        "LC_TypeKind_Array",
        "LC_TypeKind_Proc",
        "LC_TypeKind_UntypedInt",
        "LC_TypeKind_UntypedFloat",
        "LC_TypeKind_UntypedString",
        "LC_TypeKind_Incomplete",
        "LC_TypeKind_Completing",
        "LC_TypeKind_Error",
    };
    if (kind < 0 || kind >= T_TotalCount) {
        return "<invalid_type_kind>";
    }
    return strs[kind];
}

LC_FUNCTION const char *LC_DeclKindToString(LC_DeclKind decl_kind) {
    static const char *strs[] = {
        "declaration of error kind",
        "type declaration",
        "const declaration",
        "variable declaration",
        "procedure declaration",
        "import declaration",
    };
    if (decl_kind < 0 || decl_kind >= LC_DeclKind_Count) {
        return "<invalid_decl_kind>";
    }
    return strs[decl_kind];
}

LC_FUNCTION const char *LC_TokenKindToString(LC_TokenKind token_kind) {
    static const char *strs[] = {
        "end of file",
        "token error",
        "comment",
        "doc comment",
        "file doc comment",
        "package doc comment",
        "note '@'",
        "hash '#'",
        "identifier",
        "keyword",
        "string literal",
        "raw string literal",
        "integer literal",
        "float literal",
        "unicode literal",
        "open paren '('",
        "close paren ')'",
        "open brace '{'",
        "close brace '}'",
        "open bracket '['",
        "close bracket ']'",
        "comma ','",
        "question mark '?'",
        "semicolon ';'",
        "period '.'",
        "three dots '...'",
        "colon ':'",
        "multiply '*'",
        "divide '/'",
        "modulo '%'",
        "left shift '<<'",
        "right shift '>>'",
        "add '+'",
        "subtract '-'",
        "equals '=='",
        "lesser then '<'",
        "greater then '>'",
        "lesser then or equal '<='",
        "greater then or equal '>='",
        "not equal '!='",
        "bit and '&'",
        "bit or '|'",
        "bit xor '^'",
        "and '&&'",
        "or '||'",
        "addptr keyword",
        "negation '~'",
        "exclamation '!'",
        "assignment '='",
        "assignment '/='",
        "assignment '*='",
        "assignment '%='",
        "assignment '-='",
        "assignment '+='",
        "assignment '&='",
        "assignment '|='",
        "assignment '^='",
        "assignment '<<='",
        "assignment '>>='",
    };

    if (token_kind < 0 || token_kind >= LC_TokenKind_Count) {
        return "<invalid_token_kind>";
    }
    return strs[token_kind];
}

LC_FUNCTION const char *LC_TokenKindToOperator(LC_TokenKind token_kind) {
    static const char *strs[] = {
        "end of file",
        "token error",
        "comment",
        "doc comment",
        "file doc comment",
        "package doc comment",
        "@",
        "#",
        "identifier",
        "keyword",
        "string literal",
        "raw string literal",
        "integer literal",
        "float literal",
        "unicode literal",
        "(",
        ")",
        "{",
        "}",
        "[",
        "]",
        ",",
        "?",
        ";",
        ".",
        "...",
        ":",
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
        "+",
        "~",
        "!",
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
    };
    if (token_kind < 0 || token_kind >= LC_TokenKind_Count) {
        return "<invalid_token_operator>";
    }
    return strs[token_kind];
}

#if __cplusplus
    #define LC_Alignof(...) alignof(__VA_ARGS__)
#else
    #define LC_Alignof(...) _Alignof(__VA_ARGS__)
#endif

#define LC_WRAP_AROUND_POWER_OF_2(x, pow2) (((x) & ((pow2)-1llu)))

#if defined(_MSC_VER)
    #define LC_DebugBreak() (L->breakpoint_on_error && IsDebuggerPresent() && (__debugbreak(), 0))
#else
    #define LC_DebugBreak() (L->breakpoint_on_error && (__builtin_trap(), 0))
#endif
#define LC_FatalError() (L->breakpoint_on_error ? LC_DebugBreak() : (LC_Exit(1), 0))

LC_FUNCTION void LC_IgnoreMessage(LC_Token *pos, char *str, int len) {
}

LC_FUNCTION void LC_SendErrorMessage(LC_Token *pos, LC_String s8) {
    if (L->on_message) {
        L->on_message(pos, s8.str, (int)s8.len);
    } else {
        if (pos) {
            LC_String line = LC_GetTokenLine(pos);
            LC_String fmt  = LC_Format(L->arena, "%s(%d,%d): error: %.*s\n%.*s", (char *)pos->lex->file, pos->line, pos->column, LC_Expand(s8), LC_Expand(line));
            LC_Print(fmt.str, fmt.len);
        } else {
            LC_Print(s8.str, s8.len);
        }
    }
    LC_DebugBreak();
}

LC_FUNCTION void LC_SendErrorMessagef(LC_Lex *x, LC_Token *pos, const char *str, ...) {
    LC_FORMAT(L->arena, str, s8);
    LC_SendErrorMessage(pos, s8);
}

LC_FUNCTION void LC_HandleFatalError(void) {
    if (L->on_fatal_error) {
        L->on_fatal_error();
        return;
    }
    LC_FatalError();
}

LC_FUNCTION void LC_MapReserve(LC_Map *map, int size) {
    LC_Map old_map = *map;

    LC_ASSERT(NULL, LC_IS_POW2(size));
    map->len = 0;
    map->cap = size;
    LC_ASSERT(NULL, map->arena);

    map->entries = LC_PushArray(map->arena, LC_MapEntry, map->cap);

    if (old_map.entries) {
        for (int i = 0; i < old_map.cap; i += 1) {
            LC_MapEntry *it = old_map.entries + i;
            if (it->key) LC_InsertMapEntry(map, it->key, it->value);
        }
    }
}

// FNV HASH (1a?)
LC_FUNCTION uint64_t LC_HashBytes(void *data, uint64_t size) {
    uint8_t *data8 = (uint8_t *)data;
    uint64_t hash  = (uint64_t)14695981039346656037ULL;
    for (uint64_t i = 0; i < size; i++) {
        hash = hash ^ (uint64_t)(data8[i]);
        hash = hash * (uint64_t)1099511628211ULL;
    }
    return hash;
}

LC_FUNCTION uint64_t LC_HashMix(uint64_t x, uint64_t y) {
    x ^= y;
    x *= 0xff51afd7ed558ccd;
    x ^= x >> 32;
    return x;
}

LC_FUNCTION int LC_NextPow2(int v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

LC_FUNCTION LC_MapEntry *LC_GetMapEntryEx(LC_Map *map, uint64_t key) {
    LC_ASSERT(NULL, key);
    if (map->len * 2 >= map->cap) {
        LC_MapReserve(map, map->cap * 2);
    }

    uint64_t hash = LC_HashBytes(&key, sizeof(key));
    if (hash == 0) hash += 1;
    uint64_t index = LC_WRAP_AROUND_POWER_OF_2(hash, map->cap);
    uint64_t i     = index;
    for (;;) {
        LC_MapEntry *it = map->entries + i;
        if (it->key == key || it->key == 0) {
            return it;
        }

        i = LC_WRAP_AROUND_POWER_OF_2(i + 1, map->cap);
        if (i == index) return NULL;
    }
    LC_ASSERT(NULL, !"invalid codepath");
}

LC_FUNCTION bool LC_InsertWithoutReplace(LC_Map *map, void *key, void *value) {
    LC_MapEntry *entry = LC_GetMapEntryEx(map, (uint64_t)key);
    if (entry->key != 0) return false;

    map->len += 1;
    entry->key   = (uint64_t)key;
    entry->value = (uint64_t)value;
    return true;
}

LC_FUNCTION LC_MapEntry *LC_InsertMapEntry(LC_Map *map, uint64_t key, uint64_t value) {
    LC_MapEntry *entry = LC_GetMapEntryEx(map, key);
    if (entry->key == key) {
        entry->value = value;
    }
    if (entry->key == 0) {
        entry->key   = key;
        entry->value = value;
        map->len += 1;
    }
    return entry;
}

LC_FUNCTION LC_MapEntry *LC_GetMapEntry(LC_Map *map, uint64_t key) {
    LC_MapEntry *entry = LC_GetMapEntryEx(map, key);
    if (entry && entry->key == key) {
        return entry;
    }
    return NULL;
}

LC_FUNCTION void LC_MapInsert(LC_Map *map, LC_String keystr, void *value) {
    uint64_t key = LC_HashBytes(keystr.str, keystr.len);
    LC_InsertMapEntry(map, key, (uint64_t)value);
}

LC_FUNCTION void *LC_MapGet(LC_Map *map, LC_String keystr) {
    uint64_t     key = LC_HashBytes(keystr.str, keystr.len);
    LC_MapEntry *r   = LC_GetMapEntry(map, key);
    return r ? (void *)r->value : 0;
}

LC_FUNCTION void LC_MapInsertU64(LC_Map *map, uint64_t key, void *value) {
    LC_InsertMapEntry(map, key, (uint64_t)value);
}

LC_FUNCTION void *LC_MapGetU64(LC_Map *map, uint64_t key) {
    LC_MapEntry *r = LC_GetMapEntry(map, key);
    return r ? (void *)r->value : 0;
}

LC_FUNCTION void *LC_MapGetP(LC_Map *map, void *key) {
    return LC_MapGetU64(map, (uint64_t)key);
}

LC_FUNCTION void LC_MapInsertP(LC_Map *map, void *key, void *value) {
    LC_InsertMapEntry(map, (uint64_t)key, (uint64_t)value);
}

LC_FUNCTION void LC_MapClear(LC_Map *map) {
    if (map->len != 0) LC_MemoryZero(map->entries, map->cap * sizeof(LC_MapEntry));
    map->len = 0;
}

LC_FUNCTION size_t LC_GetAlignOffset(size_t size, size_t align) {
    size_t mask = align - 1;
    size_t val  = size & mask;
    if (val) {
        val = align - val;
    }
    return val;
}

LC_FUNCTION size_t LC_AlignUp(size_t size, size_t align) {
    size_t result = size + LC_GetAlignOffset(size, align);
    return result;
}

LC_FUNCTION size_t LC_AlignDown(size_t size, size_t align) {
    size += 1; // Make sure when align is 8 doesn't get rounded down to 0
    size_t result = size - (align - LC_GetAlignOffset(size, align));
    return result;
}

typedef struct {
    int  len;
    char str[];
} INTERN_Entry;

LC_FUNCTION LC_Intern LC_InternStrLen(char *str, int len) {
    LC_String     key   = LC_MakeString(str, len);
    INTERN_Entry *entry = (INTERN_Entry *)LC_MapGet(&L->interns, key);
    if (entry == NULL) {
        LC_ASSERT(NULL, sizeof(INTERN_Entry) == sizeof(int));
        entry      = (INTERN_Entry *)LC_PushSize(L->arena, sizeof(int) + sizeof(char) * (len + 1));
        entry->len = len;
        LC_MemoryCopy(entry->str, str, len);
        entry->str[len] = 0;
        LC_MapInsert(&L->interns, key, entry);
    }

    return (uintptr_t)entry->str;
}

LC_FUNCTION LC_Intern LC_ILit(char *str) {
    return LC_InternStrLen(str, (int)LC_StrLen(str));
}

LC_FUNCTION LC_Intern LC_GetUniqueIntern(const char *name_for_debug) {
    LC_String name   = LC_Format(L->arena, "U%u_%s", ++L->unique_counter, name_for_debug);
    LC_Intern result = LC_InternStrLen(name.str, (int)name.len);
    return result;
}

LC_FUNCTION char *LC_GetUniqueName(const char *name_for_debug) {
    LC_String name = LC_Format(L->arena, "U%u_%s", ++L->unique_counter, name_for_debug);
    return name.str;
}

LC_FUNCTION void LC_DeclareNote(LC_Intern intern) {
    LC_MapInsertU64(&L->declared_notes, intern, (void *)intern);
}

LC_FUNCTION bool LC_IsNoteDeclared(LC_Intern intern) {
    void *p      = LC_MapGetU64(&L->declared_notes, intern);
    bool  result = p != NULL;
    return result;
}
LC_FUNCTION void LC_LexingError(LC_Token *pos, const char *str, ...) {
    LC_FORMAT(L->arena, str, s8);
    LC_SendErrorMessage(pos, s8);
    L->errors += 1;
    pos->kind = LC_TokenKind_Error;
}

#define LC_IF(cond, ...)                    \
    do {                                    \
        if (cond) {                         \
            LC_LexingError(t, __VA_ARGS__); \
            return;                         \
        }                                   \
    } while (0)

LC_FUNCTION bool LC_IsAssign(LC_TokenKind kind) {
    bool result = kind >= LC_TokenKind_Assign && kind <= LC_TokenKind_RightShiftAssign;
    return result;
}

LC_FUNCTION bool LC_IsHexDigit(char c) {
    bool result = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
    return result;
}

LC_FUNCTION bool LC_IsBinDigit(char c) {
    bool result = (c >= '0' && c <= '1');
    return result;
}

LC_FUNCTION uint64_t LC_MapCharToNumber(char c) {
    // clang-format off
    switch (c) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        case 'a': case 'A': return 10;
        case 'b': case 'B': return 11;
        case 'c': case 'C': return 12;
        case 'd': case 'D': return 13;
        case 'e': case 'E': return 14;
        case 'f': case 'F': return 15;
        default: return 255;
    }
    // clang-format on
}

LC_FUNCTION uint64_t LC_GetEscapeCode(char c) {
    switch (c) {
    case 'a': return '\a';
    case 'b': return '\b';
    case 'e': return 0x1B;
    case 'f': return '\f';
    case 'n': return '\n';
    case 'r': return '\r';
    case 't': return '\t';
    case 'v': return '\v';
    case '\\': return '\\';
    case '\'': return '\'';
    case '\"': return '\"';
    case '0': return '\0';
    default: return UINT64_MAX;
    }
}

LC_FUNCTION LC_String LC_GetEscapeString(char c) {
    switch (c) {
    case '\a': return LC_Lit("\\a");
    case '\b': return LC_Lit("\\b");
    case 0x1B: return LC_Lit("\\x1B");
    case '\f': return LC_Lit("\\f");
    case '\n': return LC_Lit("\\n");
    case '\r': return LC_Lit("\\r");
    case '\t': return LC_Lit("\\t");
    case '\v': return LC_Lit("\\v");
    case '\\': return LC_Lit("\\\\");
    case '\'': return LC_Lit("\\'");
    case '\"': return LC_Lit("\\\"");
    case '\0': return LC_Lit("\\0");
    default: return LC_Lit("");
    }
}

LC_FUNCTION void LC_LexAdvance(LC_Lex *x) {
    if (x->at[0] == 0) {
        return;
    } else if (x->at[0] == '\n') {
        x->line += 1;
        x->column = 0;
    }
    x->column += 1;
    x->at += 1;
}

LC_FUNCTION void LC_EatWhitespace(LC_Lex *x) {
    while (LC_IsWhitespace(x->at[0])) LC_LexAdvance(x);
}

LC_FUNCTION void LC_EatIdent(LC_Lex *x) {
    while (x->at[0] == '_' || LC_IsAlphanumeric(x->at[0])) LC_LexAdvance(x);
}

LC_FUNCTION void LC_SetTokenLen(LC_Lex *x, LC_Token *t) {
    t->len = (int)(x->at - t->str);
    LC_ASSERT(NULL, t->len < 2000000000);
}

LC_FUNCTION void LC_EatUntilIncluding(LC_Lex *x, char c) {
    while (x->at[0] != 0 && x->at[0] != c) LC_LexAdvance(x);
    LC_LexAdvance(x);
}

// @todo: add temporary allocation + copy at end to perm
LC_FUNCTION LC_BigInt LC_LexBigInt(char *string, int len, uint64_t base) {
    LC_ASSERT(NULL, base >= 2 && base <= 16);
    LC_BigInt m        = LC_Bigint_u64(1);
    LC_BigInt base_mul = LC_Bigint_u64(base);
    LC_BigInt result   = LC_Bigint_u64(0);

    LC_BigInt tmp = {0};
    for (int i = len - 1; i >= 0; --i) {
        uint64_t u = LC_MapCharToNumber(string[i]);
        LC_ASSERT(NULL, u < base);
        LC_BigInt val = LC_Bigint_u64(u);
        LC_Bigint_mul(&tmp, &val, &m);
        LC_BigInt new_val = tmp;
        LC_Bigint_add(&tmp, &result, &new_val);
        result = tmp;
        LC_Bigint_mul(&tmp, &m, &base_mul);
        m = tmp;
    }

    return result;
}

LC_FUNCTION void LC_LexNestedComments(LC_Lex *x, LC_Token *t) {
    t->kind = LC_TokenKind_Comment;
    LC_LexAdvance(x);

    if (x->at[0] == '*') {
        LC_LexAdvance(x);
        t->kind = LC_TokenKind_DocComment;

        if (x->at[0] == ' ' && x->at[1] == 'f' && x->at[2] == 'i' && x->at[3] == 'l' && x->at[4] == 'e') {
            t->kind = LC_TokenKind_FileDocComment;
        }

        if (x->at[0] == ' ' && x->at[1] == 'p' && x->at[2] == 'a' && x->at[3] == 'c' && x->at[4] == 'k' && x->at[5] == 'a' && x->at[6] == 'g' && x->at[7] == 'e') {
            t->kind = LC_TokenKind_PackageDocComment;
        }
    }

    int counter = 0;
    for (;;) {
        if (x->at[0] == '*' && x->at[1] == '/') {
            if (counter <= 0) break;
            counter -= 1;
        } else if (x->at[0] == '/' && x->at[1] == '*') {
            counter += 1;
            LC_LexAdvance(x);
        }
        LC_IF(x->at[0] == 0, "Unclosed block comment");
        LC_LexAdvance(x);
    }
    t->str += 2;
    LC_SetTokenLen(x, t);
    LC_LexAdvance(x);
    LC_LexAdvance(x);
}

LC_FUNCTION void LC_LexStringLiteral(LC_Lex *x, LC_Token *t, LC_TokenKind kind) {
    t->kind = kind;
    if (kind == LC_TokenKind_RawString) {
        LC_EatUntilIncluding(x, '`');
    } else if (kind == LC_TokenKind_String) {
        for (;;) {
            LC_IF(x->at[0] == '\n', "got a new line while parsing a '\"' string literal");
            LC_IF(x->at[0] == 0, "reached end of file during string lexing");
            if (x->at[0] == '"') break;
            if (x->at[0] == '\\' && x->at[1] == '"') LC_LexAdvance(x);
            LC_LexAdvance(x);
        }
        LC_LexAdvance(x);
    } else {
        LC_IF(1, "internal compiler error: unhandled case in %s", __FUNCTION__);
    }

    LC_SetTokenLen(x, t);
    t->len -= 2;
    t->str += 1;
}

LC_FUNCTION void LC_LexUnicodeLiteral(LC_Lex *x, LC_Token *t) {
    t->kind               = LC_TokenKind_Unicode;
    LC_UTF32Result decode = LC_ConvertUTF8ToUTF32(x->at, 4);
    LC_IF(decode.error, "invalid utf8 sequence");

    uint8_t c[8] = {0};
    for (int i = 0; i < decode.advance; i += 1) {
        c[i] = x->at[0];
        LC_LexAdvance(x);
    }
    uint64_t result = *(uint64_t *)&c[0];

    if (result == '\\') {
        LC_ASSERT(NULL, decode.advance == 1);
        result = LC_GetEscapeCode(x->at[0]);
        LC_IF(result == UINT64_MAX, "invalid escape code");
        LC_LexAdvance(x);
    }
    LC_IF(x->at[0] != '\'', "unclosed unicode literal");

    LC_Bigint_init_signed(&t->i, result);
    LC_LexAdvance(x);
    LC_SetTokenLen(x, t);
    t->str += 1;
    t->len -= 2;

    LC_IF(t->len == 0, "empty unicode literal");
}

LC_FUNCTION void LC_LexIntOrFloat(LC_Lex *x, LC_Token *t) {
    t->kind = LC_TokenKind_Int;
    for (;;) {
        if (x->at[0] == '.') {
            LC_IF(t->kind == LC_TokenKind_Float, "failed to parse a floating point number, invalid format, found multiple '.'");
            if (t->kind == LC_TokenKind_Int) t->kind = LC_TokenKind_Float;
        } else if (!LC_IsDigit(x->at[0])) break;
        LC_LexAdvance(x);
    }

    LC_SetTokenLen(x, t);
    if (t->kind == LC_TokenKind_Int) {
        t->i = LC_LexBigInt(t->str, t->len, 10);
    } else if (t->kind == LC_TokenKind_Float) {
        t->f64 = LC_ParseFloat(t->str, t->len);
    } else {
        LC_IF(1, "internal compiler error: unhandled case in %s", __FUNCTION__);
    }
}

LC_FUNCTION void LC_LexCase2(LC_Lex *x, LC_Token *t, LC_TokenKind tk0, char c, LC_TokenKind tk1) {
    t->kind = tk0;
    if (x->at[0] == c) {
        LC_LexAdvance(x);
        t->kind = tk1;
    }
}

LC_FUNCTION void LC_LexCase3(LC_Lex *x, LC_Token *t, LC_TokenKind tk, char c0, LC_TokenKind tk0, char c1, LC_TokenKind tk1) {
    t->kind = tk;
    if (x->at[0] == c0) {
        t->kind = tk0;
        LC_LexAdvance(x);
    } else if (x->at[0] == c1) {
        t->kind = tk1;
        LC_LexAdvance(x);
    }
}

LC_FUNCTION void LC_LexCase4(LC_Lex *x, LC_Token *t, LC_TokenKind tk, char c0, LC_TokenKind tk0, char c1, LC_TokenKind tk1, char c2, LC_TokenKind tk2) {
    t->kind = tk;
    if (x->at[0] == c0) {
        t->kind = tk0;
        LC_LexAdvance(x);
    } else if (x->at[0] == c1) {
        LC_LexAdvance(x);
        LC_LexCase2(x, t, tk1, c2, tk2);
    }
}

LC_FUNCTION void LC_LexNext(LC_Lex *x, LC_Token *t) {
    LC_EatWhitespace(x);
    LC_MemoryZero(t, sizeof(LC_Token));
    t->str    = x->at;
    t->line   = x->line + 1;
    t->column = x->column;
    t->lex    = x;
    char *c   = x->at;
    LC_LexAdvance(x);

    switch (c[0]) {
    case 0: t->kind = LC_TokenKind_EOF; break;
    case '(': t->kind = LC_TokenKind_OpenParen; break;
    case ')': t->kind = LC_TokenKind_CloseParen; break;
    case '{': t->kind = LC_TokenKind_OpenBrace; break;
    case '}': t->kind = LC_TokenKind_CloseBrace; break;
    case '[': t->kind = LC_TokenKind_OpenBracket; break;
    case ']': t->kind = LC_TokenKind_CloseBracket; break;
    case ',': t->kind = LC_TokenKind_Comma; break;
    case ':': t->kind = LC_TokenKind_Colon; break;
    case ';': t->kind = LC_TokenKind_Semicolon; break;
    case '~': t->kind = LC_TokenKind_Neg; break;
    case '#': t->kind = LC_TokenKind_Hash; break;
    case '@': t->kind = LC_TokenKind_Note; break;
    case '\'': LC_LexUnicodeLiteral(x, t); break;
    case '"': LC_LexStringLiteral(x, t, LC_TokenKind_String); break;
    case '`': LC_LexStringLiteral(x, t, LC_TokenKind_RawString); break;
    case '=': LC_LexCase2(x, t, LC_TokenKind_Assign, '=', LC_TokenKind_Equals); break;
    case '!': LC_LexCase2(x, t, LC_TokenKind_Not, '=', LC_TokenKind_NotEquals); break;
    case '*': LC_LexCase2(x, t, LC_TokenKind_Mul, '=', LC_TokenKind_MulAssign); break;
    case '%': LC_LexCase2(x, t, LC_TokenKind_Mod, '=', LC_TokenKind_ModAssign); break;
    case '+': LC_LexCase2(x, t, LC_TokenKind_Add, '=', LC_TokenKind_AddAssign); break;
    case '-': LC_LexCase2(x, t, LC_TokenKind_Sub, '=', LC_TokenKind_SubAssign); break;
    case '^': LC_LexCase2(x, t, LC_TokenKind_BitXor, '=', LC_TokenKind_BitXorAssign); break;
    case '&': LC_LexCase3(x, t, LC_TokenKind_BitAnd, '=', LC_TokenKind_BitAndAssign, '&', LC_TokenKind_And); break;
    case '|': LC_LexCase3(x, t, LC_TokenKind_BitOr, '=', LC_TokenKind_BitOrAssign, '|', LC_TokenKind_Or); break;
    case '>': LC_LexCase4(x, t, LC_TokenKind_GreaterThen, '=', LC_TokenKind_GreaterThenEq, '>', LC_TokenKind_RightShift, '=', LC_TokenKind_RightShiftAssign); break;
    case '<': LC_LexCase4(x, t, LC_TokenKind_LesserThen, '=', LC_TokenKind_LesserThenEq, '<', LC_TokenKind_LeftShift, '=', LC_TokenKind_LeftShiftAssign); break;
    case '.': {
        t->kind = LC_TokenKind_Dot;
        if (x->at[0] == '.' && x->at[1] == '.') {
            t->kind = LC_TokenKind_ThreeDots;
            LC_LexAdvance(x);
            LC_LexAdvance(x);
        }
    } break;

    case '0': {
        if (x->at[0] == 'x') {
            t->kind = LC_TokenKind_Int;
            LC_LexAdvance(x);
            while (LC_IsHexDigit(x->at[0])) LC_LexAdvance(x);
            LC_SetTokenLen(x, t);
            LC_IF(t->len < 3, "invalid hex number");
            t->i = LC_LexBigInt(t->str + 2, t->len - 2, 16);
            break;
        }
        if (x->at[0] == 'b') {
            t->kind = LC_TokenKind_Int;
            LC_LexAdvance(x);
            while (LC_IsBinDigit(x->at[0])) LC_LexAdvance(x);
            LC_SetTokenLen(x, t);
            LC_IF(t->len < 3, "invalid binary number");
            t->i = LC_LexBigInt(t->str + 2, t->len - 2, 2);
            break;
        }
    } // @fallthrough

    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': {
        LC_LexIntOrFloat(x, t);
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
    case 'L':
    case 'l':
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
    case 'U':
    case 'u':
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
    case '_': {
        t->kind = LC_TokenKind_Ident;
        LC_EatIdent(x);
    } break;

    case '/': {
        t->kind = LC_TokenKind_Div;
        if (x->at[0] == '=') {
            t->kind = LC_TokenKind_DivAssign;
            LC_LexAdvance(x);
        } else if (x->at[0] == '/') {
            t->kind = LC_TokenKind_Comment;
            LC_LexAdvance(x);
            while (x->at[0] != '\n' && x->at[0] != 0) LC_LexAdvance(x);
            LC_SetTokenLen(x, t);
        } else if (x->at[0] == '*') {
            LC_LexNestedComments(x, t);
        }
    } break;

    default: LC_IF(1, "invalid character");
    }
    if (t->len == 0 && t->kind != LC_TokenKind_String && t->kind != LC_TokenKind_RawString) LC_SetTokenLen(x, t);
    if (t->kind == LC_TokenKind_Comment) LC_LexNext(x, t);
}

LC_FUNCTION LC_Lex *LC_LexStream(char *file, char *str, int line) {
    LC_Lex *x = LC_PushStruct(L->lex_arena, LC_Lex);
    x->begin  = str;
    x->at     = str;
    x->file   = LC_ILit(file);
    x->line   = line;

    for (;;) {
        LC_Token *t = LC_PushStruct(L->lex_arena, LC_Token);
        if (!x->tokens) x->tokens = t;
        x->token_count += 1;

        LC_LexNext(x, t);
        if (t->kind == LC_TokenKind_EOF) break;
    }

    return x;
}

LC_FUNCTION LC_String LC_GetTokenLine(LC_Token *token) {
    LC_Lex       *x       = token->lex;
    LC_String     content = LC_MakeFromChar(x->begin);
    LC_StringList lines   = LC_Split(L->arena, content, LC_Lit("\n"), 0);

    LC_String l[3] = {LC_MakeEmptyString()};

    int line = 1;
    for (LC_StringNode *it = lines.first; it; it = it->next) {
        LC_String sline = it->string;
        if (token->line - 1 == line) {
            l[0] = LC_Format(L->arena, "> %.*s\n", LC_Expand(sline));
        }
        if (token->line + 1 == line) {
            l[2] = LC_Format(L->arena, "> %.*s\n", LC_Expand(sline));
            break;
        }
        if (token->line == line) {
            int       begin     = (int)(token->str - sline.str);
            LC_String left      = LC_GetPrefix(sline, begin);
            LC_String past_left = LC_Skip(sline, begin);
            LC_String mid       = LC_GetPrefix(past_left, token->len);
            LC_String right     = LC_Skip(past_left, token->len);

            char *green = "\033[32m";
            char *reset = "\033[0m";
            if (!L->use_colored_terminal_output) {
                green = ">>>>";
                reset = "<<<<";
            }
            l[1] = LC_Format(L->arena, "> %.*s%s%.*s%s%.*s\n", LC_Expand(left), green, LC_Expand(mid), reset, LC_Expand(right));
        }
        line += 1;
    }

    LC_String result = LC_Format(L->arena, "%.*s%.*s%.*s", LC_Expand(l[0]), LC_Expand(l[1]), LC_Expand(l[2]));
    return result;
}

LC_FUNCTION void LC_InternTokens(LC_Lex *x) {
    // @todo: add scratch, we can dump the LC_PushArray strings
    for (int i = 0; i < x->token_count; i += 1) {
        LC_Token *t = x->tokens + i;
        if (t->kind == LC_TokenKind_String) {
            int   string_len = 0;
            char *string     = LC_PushArray(L->arena, char, t->len);
            for (int i = 0; i < t->len; i += 1) {
                char c0 = t->str[i];
                char c1 = t->str[i + 1];
                if (i + 1 >= t->len) c1 = 0;

                if (c0 == '\\') {
                    uint64_t code = LC_GetEscapeCode(c1);
                    if (code == UINT64_MAX) {
                        LC_LexingError(t, "invalid escape code in string '%c%c'", c0, c1);
                        break;
                    }

                    c0 = (char)code;
                    i += 1;
                }

                string[string_len++] = c0;
            }
            t->ident = LC_InternStrLen(string, string_len);
        }
        if (t->kind == LC_TokenKind_Note || t->kind == LC_TokenKind_Ident || t->kind == LC_TokenKind_RawString) {
            t->ident = LC_InternStrLen(t->str, t->len);
        }
        if (t->kind == LC_TokenKind_Ident) {
            bool is_keyword = t->ident >= L->first_keyword && t->ident <= L->last_keyword;
            if (is_keyword) {
                t->kind = LC_TokenKind_Keyword;
                if (L->kaddptr == t->ident) t->kind = LC_TokenKind_AddPtr;
            }
        }
    }
}

#undef LC_IF

/*
The bigint code was written by Christoffer Lerno, he is the programmer
behind C3. He allowed me to use this code without any restrictions. Great guy!
You can check out C3 compiler: https://github.com/c3lang/c3c
He also writes very helpful blogs about compilers: https://c3.handmade.network/blog
*/

#ifndef malloc_arena
    #define malloc_arena(size) LC_PushSize(L->arena, size)
#endif
#define ALLOC_DIGITS(_digits) ((_digits) ? (uint64_t *)malloc_arena(sizeof(uint64_t) * (_digits)) : NULL)

LC_FUNCTION uint32_t LC_u32_min(uint32_t a, uint32_t b) {
    return a < b ? a : b;
}

LC_FUNCTION size_t LC_size_max(size_t a, size_t b) {
    return a > b ? a : b;
}

LC_FUNCTION unsigned LC_unsigned_max(unsigned a, unsigned b) {
    return a > b ? a : b;
}

LC_FUNCTION uint64_t *LC_Bigint_ptr(LC_BigInt *big_int) {
    return big_int->digit_count == 1 ? &big_int->digit : big_int->digits;
}

LC_FUNCTION LC_BigInt LC_Bigint_u64(uint64_t val) {
    LC_BigInt result = {0};
    LC_Bigint_init_unsigned(&result, val);
    return result;
}

LC_FUNCTION void LC_normalize(LC_BigInt *big_int) {
    uint64_t *digits        = LC_Bigint_ptr(big_int);
    unsigned  last_non_zero = UINT32_MAX;
    for (unsigned i = 0; i < big_int->digit_count; i++) {
        if (digits[i] != 0) {
            last_non_zero = i;
        }
    }
    if (last_non_zero == UINT32_MAX) {
        big_int->is_negative = false;
        big_int->digit_count = 0;
        return;
    }
    big_int->digit_count = last_non_zero + 1;
    if (!last_non_zero) {
        big_int->digit = digits[0];
    }
}

LC_FUNCTION char LC_digit_to_char(uint8_t digit, bool upper) {
    if (digit <= 9) {
        return (char)(digit + '0');
    }
    if (digit <= 35) {
        return (char)(digit + (upper ? 'A' : 'a') - 10);
    }
    LC_ASSERT(NULL, !"Can't reach");
    return 0;
}

LC_FUNCTION bool LC_bit_at_index(LC_BigInt *big_int, size_t index) {
    size_t digit_index = index / 64;
    if (digit_index >= big_int->digit_count) {
        return false;
    }
    size_t    digit_bit_index = index % 64;
    uint64_t *digits          = LC_Bigint_ptr(big_int);
    uint64_t  digit           = digits[digit_index];
    return ((digit >> digit_bit_index) & 0x1U) == 0x1U;
}

LC_FUNCTION size_t LC_Bigint_bits_needed(LC_BigInt *big_int) {
    size_t full_bits          = big_int->digit_count * 64;
    size_t leading_zero_count = LC_Bigint_clz(big_int, full_bits);
    size_t bits_needed        = full_bits - leading_zero_count;
    return bits_needed + big_int->is_negative;
}

LC_FUNCTION void LC_Bigint_init_unsigned(LC_BigInt *big_int, uint64_t value) {
    if (value == 0) {
        big_int->digit_count = 0;
        big_int->is_negative = false;
        return;
    }
    big_int->digit_count = 1;
    big_int->digit       = value;
    big_int->is_negative = false;
}

LC_FUNCTION void LC_Bigint_init_signed(LC_BigInt *dest, int64_t value) {
    if (value >= 0) {
        LC_Bigint_init_unsigned(dest, (uint64_t)value);
        return;
    }
    dest->is_negative = true;
    dest->digit_count = 1;
    dest->digit       = ((uint64_t)(-(value + 1))) + 1;
}

LC_FUNCTION void LC_Bigint_init_bigint(LC_BigInt *dest, LC_BigInt *src) {
    if (src->digit_count == 0) {
        LC_Bigint_init_unsigned(dest, 0);
        return;
    }
    if (src->digit_count == 1) {
        dest->digit_count = 1;
        dest->digit       = src->digit;
        dest->is_negative = src->is_negative;
        return;
    }
    dest->is_negative = src->is_negative;
    dest->digit_count = src->digit_count;
    dest->digits      = ALLOC_DIGITS(dest->digit_count);
    LC_MemoryCopy(dest->digits, src->digits, sizeof(uint64_t) * dest->digit_count);
}

LC_FUNCTION void LC_Bigint_negate(LC_BigInt *dest, LC_BigInt *source) {
    LC_Bigint_init_bigint(dest, source);
    dest->is_negative = !dest->is_negative;
    LC_normalize(dest);
}

LC_FUNCTION void LC_to_twos_complement(LC_BigInt *dest, LC_BigInt *source, size_t bit_count) {
    if (bit_count == 0 || source->digit_count == 0) {
        LC_Bigint_init_unsigned(dest, 0);
        return;
    }
    if (source->is_negative) {
        LC_BigInt negated = {0};
        LC_Bigint_negate(&negated, source);

        LC_BigInt inverted = {0};
        LC_Bigint_not(&inverted, &negated, bit_count, false);

        LC_BigInt one = {0};
        LC_Bigint_init_unsigned(&one, 1);

        LC_Bigint_add(dest, &inverted, &one);
        return;
    }

    dest->is_negative       = false;
    uint64_t *source_digits = LC_Bigint_ptr(source);
    if (source->digit_count == 1) {
        dest->digit = source_digits[0];
        if (bit_count < 64) {
            dest->digit &= (1ULL << bit_count) - 1;
        }
        dest->digit_count = 1;
        LC_normalize(dest);
        return;
    }
    unsigned digits_to_copy = (unsigned int)(bit_count / 64);
    unsigned leftover_bits  = (unsigned int)(bit_count % 64);
    dest->digit_count       = digits_to_copy + ((leftover_bits == 0) ? 0 : 1);
    if (dest->digit_count == 1 && leftover_bits == 0) {
        dest->digit = source_digits[0];
        if (dest->digit == 0) dest->digit_count = 0;
        return;
    }
    dest->digits = (uint64_t *)malloc_arena(dest->digit_count * sizeof(uint64_t));
    for (size_t i = 0; i < digits_to_copy; i += 1) {
        uint64_t digit  = (i < source->digit_count) ? source_digits[i] : 0;
        dest->digits[i] = digit;
    }
    if (leftover_bits != 0) {
        uint64_t digit               = (digits_to_copy < source->digit_count) ? source_digits[digits_to_copy] : 0;
        dest->digits[digits_to_copy] = digit & ((1ULL << leftover_bits) - 1);
    }
    LC_normalize(dest);
}

LC_FUNCTION size_t LC_Bigint_clz(LC_BigInt *big_int, size_t bit_count) {
    if (big_int->is_negative || bit_count == 0) {
        return 0;
    }
    if (big_int->digit_count == 0) {
        return bit_count;
    }
    size_t count = 0;
    for (size_t i = bit_count - 1;;) {
        if (LC_bit_at_index(big_int, i)) {
            return count;
        }
        count++;
        if (i == 0) break;
        i--;
    }
    return count;
}

LC_FUNCTION bool LC_Bigint_eql(LC_BigInt a, LC_BigInt b) {
    return LC_Bigint_cmp(&a, &b) == LC_CmpRes_EQ;
}

LC_FUNCTION void LC_from_twos_complement(LC_BigInt *dest, LC_BigInt *src, size_t bit_count, bool is_signed) {
    LC_ASSERT(NULL, !src->is_negative);

    if (bit_count == 0 || src->digit_count == 0) {
        LC_Bigint_init_unsigned(dest, 0);
        return;
    }

    if (is_signed && LC_bit_at_index(src, bit_count - 1)) {
        LC_BigInt negative_one = {0};
        LC_Bigint_init_signed(&negative_one, -1);

        LC_BigInt minus_one = {0};
        LC_Bigint_add(&minus_one, src, &negative_one);

        LC_BigInt inverted = {0};
        LC_Bigint_not(&inverted, &minus_one, bit_count, false);

        LC_Bigint_negate(dest, &inverted);
        return;
    }

    LC_Bigint_init_bigint(dest, src);
}

void LC_Bigint_init_data(LC_BigInt *dest, uint64_t *digits, unsigned int digit_count, bool is_negative) {
    if (digit_count == 0) {
        LC_Bigint_init_unsigned(dest, 0);
        return;
    }

    if (digit_count == 1) {
        dest->digit_count = 1;
        dest->digit       = digits[0];
        dest->is_negative = is_negative;
        LC_normalize(dest);
        return;
    }

    dest->digit_count = digit_count;
    dest->is_negative = is_negative;
    dest->digits      = ALLOC_DIGITS(digit_count);
    LC_MemoryCopy(dest->digits, digits, sizeof(uint64_t) * digit_count);

    LC_normalize(dest);
}

LC_FUNCTION bool LC_Bigint_fits_in_bits(LC_BigInt *big_int, size_t bit_count, bool is_signed) {
    LC_ASSERT(NULL, big_int->digit_count != 1 || big_int->digit != 0);
    if (bit_count == 0) {
        return LC_Bigint_cmp_zero(big_int) == LC_CmpRes_EQ;
    }
    if (big_int->digit_count == 0) {
        return true;
    }

    if (!is_signed) {
        size_t full_bits          = big_int->digit_count * 64;
        size_t leading_zero_count = LC_Bigint_clz(big_int, full_bits);
        return bit_count >= full_bits - leading_zero_count;
    }

    LC_BigInt one = {0};
    LC_Bigint_init_unsigned(&one, 1);

    LC_BigInt shl_amt = {0};
    LC_Bigint_init_unsigned(&shl_amt, bit_count - 1);

    LC_BigInt max_value_plus_one = {0};
    LC_Bigint_shl(&max_value_plus_one, &one, &shl_amt);

    LC_BigInt max_value = {0};
    LC_Bigint_sub(&max_value, &max_value_plus_one, &one);

    LC_BigInt min_value = {0};
    LC_Bigint_negate(&min_value, &max_value_plus_one);

    LC_CmpRes min_cmp = LC_Bigint_cmp(big_int, &min_value);
    LC_CmpRes max_cmp = LC_Bigint_cmp(big_int, &max_value);

    return (min_cmp == LC_CmpRes_GT || min_cmp == LC_CmpRes_EQ) && (max_cmp == LC_CmpRes_LT || max_cmp == LC_CmpRes_EQ);
}

LC_FUNCTION uint64_t LC_Bigint_as_unsigned(LC_BigInt *bigint) {
    LC_ASSERT(NULL, !bigint->is_negative);
    if (bigint->digit_count == 0) {
        return 0;
    }
    if (bigint->digit_count != 1) {
        LC_ASSERT(NULL, !"Bigint exceeds u64");
    }
    return bigint->digit;
}

#if defined(_MSC_VER)

LC_FUNCTION bool LC_add_u64_overflow(uint64_t op1, uint64_t op2, uint64_t *result) {
    *result = op1 + op2;
    return *result < op1 || *result < op2;
}

LC_FUNCTION bool LC_sub_u64_overflow(uint64_t op1, uint64_t op2, uint64_t *result) {
    *result = op1 - op2;
    return *result > op1;
}

LC_FUNCTION bool LC_mul_u64_overflow(uint64_t op1, uint64_t op2, uint64_t *result) {
    *result = op1 * op2;

    if (op1 == 0 || op2 == 0) return false;
    if (op1 > UINT64_MAX / op2) return true;
    if (op2 > UINT64_MAX / op1) return true;
    return false;
}

#else

LC_FUNCTION bool LC_add_u64_overflow(uint64_t op1, uint64_t op2, uint64_t *result) {
    return __builtin_uaddll_overflow((unsigned long long)op1, (unsigned long long)op2,
                                     (unsigned long long *)result);
}

LC_FUNCTION bool LC_sub_u64_overflow(uint64_t op1, uint64_t op2, uint64_t *result) {
    return __builtin_usubll_overflow((unsigned long long)op1, (unsigned long long)op2,
                                     (unsigned long long *)result);
}

LC_FUNCTION bool LC_mul_u64_overflow(uint64_t op1, uint64_t op2, uint64_t *result) {
    return __builtin_umulll_overflow((unsigned long long)op1, (unsigned long long)op2,
                                     (unsigned long long *)result);
}

#endif

LC_FUNCTION void LC_Bigint_add(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2) {
    if (op1->digit_count == 0) {
        LC_Bigint_init_bigint(dest, op2);
        return;
    }
    if (op2->digit_count == 0) {
        LC_Bigint_init_bigint(dest, op1);
        return;
    }
    if (op1->is_negative == op2->is_negative) {
        dest->is_negative = op1->is_negative;

        uint64_t *op1_digits = LC_Bigint_ptr(op1);
        uint64_t *op2_digits = LC_Bigint_ptr(op2);
        uint64_t  overflow   = LC_add_u64_overflow(op1_digits[0], op2_digits[0], &dest->digit);
        if (overflow == 0 && op1->digit_count == 1 && op2->digit_count == 1) {
            dest->digit_count = 1;
            LC_normalize(dest);
            return;
        }
        unsigned i           = 1;
        uint64_t first_digit = dest->digit;
        dest->digits         = ALLOC_DIGITS(LC_unsigned_max(op1->digit_count, op2->digit_count) + 1);
        dest->digits[0]      = first_digit;

        for (;;) {
            bool     found_digit = false;
            uint64_t x           = (uint64_t)overflow;
            overflow             = 0;

            if (i < op1->digit_count) {
                found_digit    = true;
                uint64_t digit = op1_digits[i];
                overflow += LC_add_u64_overflow(x, digit, &x);
            }

            if (i < op2->digit_count) {
                found_digit    = true;
                uint64_t digit = op2_digits[i];
                overflow += LC_add_u64_overflow(x, digit, &x);
            }

            dest->digits[i] = x;
            i += 1;

            if (!found_digit) {
                dest->digit_count = i;
                LC_normalize(dest);
                return;
            }
        }
    }
    LC_BigInt *op_pos;
    LC_BigInt *op_neg;
    if (op1->is_negative) {
        op_neg = op1;
        op_pos = op2;
    } else {
        op_pos = op1;
        op_neg = op2;
    }

    LC_BigInt op_neg_abs = {0};
    LC_Bigint_negate(&op_neg_abs, op_neg);
    LC_BigInt *bigger_op;
    LC_BigInt *smaller_op;
    switch (LC_Bigint_cmp(op_pos, &op_neg_abs)) {
    case LC_CmpRes_EQ:
        LC_Bigint_init_unsigned(dest, 0);
        return;
    case LC_CmpRes_LT:
        bigger_op         = &op_neg_abs;
        smaller_op        = op_pos;
        dest->is_negative = true;
        break;
    case LC_CmpRes_GT:
        bigger_op         = op_pos;
        smaller_op        = &op_neg_abs;
        dest->is_negative = false;
        break;
    default:
        LC_ASSERT(NULL, !"UNREACHABLE");
    }
    uint64_t *bigger_op_digits  = LC_Bigint_ptr(bigger_op);
    uint64_t *smaller_op_digits = LC_Bigint_ptr(smaller_op);
    uint64_t  overflow          = (uint64_t)LC_sub_u64_overflow(bigger_op_digits[0], smaller_op_digits[0], &dest->digit);
    if (overflow == 0 && bigger_op->digit_count == 1 && smaller_op->digit_count == 1) {
        dest->digit_count = 1;
        LC_normalize(dest);
        return;
    }
    uint64_t first_digit = dest->digit;
    dest->digits         = ALLOC_DIGITS(bigger_op->digit_count);
    dest->digits[0]      = first_digit;
    unsigned i           = 1;

    for (;;) {
        bool     found_digit   = false;
        uint64_t x             = bigger_op_digits[i];
        uint64_t prev_overflow = overflow;
        overflow               = 0;

        if (i < smaller_op->digit_count) {
            found_digit    = true;
            uint64_t digit = smaller_op_digits[i];
            overflow += LC_sub_u64_overflow(x, digit, &x);
        }
        if (LC_sub_u64_overflow(x, prev_overflow, &x)) {
            found_digit = true;
            overflow += 1;
        }
        dest->digits[i] = x;
        i += 1;

        if (!found_digit || i >= bigger_op->digit_count) {
            break;
        }
    }
    LC_ASSERT(NULL, overflow == 0);
    dest->digit_count = i;
    LC_normalize(dest);
}

LC_FUNCTION void LC_Bigint_add_wrap(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2, size_t bit_count, bool is_signed) {
    LC_BigInt unwrapped = {0};
    LC_Bigint_add(&unwrapped, op1, op2);
    LC_Bigint_truncate(dest, &unwrapped, bit_count, is_signed);
}

LC_FUNCTION void LC_Bigint_sub(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2) {
    LC_BigInt op2_negated = {0};
    LC_Bigint_negate(&op2_negated, op2);
    LC_Bigint_add(dest, op1, &op2_negated);
    return;
}

LC_FUNCTION void LC_Bigint_sub_wrap(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2, size_t bit_count, bool is_signed) {
    LC_BigInt op2_negated = {0};
    LC_Bigint_negate(&op2_negated, op2);
    LC_Bigint_add_wrap(dest, op1, &op2_negated, bit_count, is_signed);
    return;
}

LC_FUNCTION void mul_overflow(uint64_t op1, uint64_t op2, uint64_t *lo, uint64_t *hi) {
    uint64_t u1 = (op1 & 0xffffffff);
    uint64_t v1 = (op2 & 0xffffffff);
    uint64_t t  = (u1 * v1);
    uint64_t w3 = (t & 0xffffffff);
    uint64_t k  = (t >> 32);

    op1 >>= 32;
    t           = (op1 * v1) + k;
    k           = (t & 0xffffffff);
    uint64_t w1 = (t >> 32);

    op2 >>= 32;
    t = (u1 * op2) + k;
    k = (t >> 32);

    *hi = (op1 * op2) + w1 + k;
    *lo = (t << 32) + w3;
}

LC_FUNCTION void LC_mul_scalar(LC_BigInt *dest, LC_BigInt *op, uint64_t scalar) {
    LC_Bigint_init_unsigned(dest, 0);

    LC_BigInt bi_64;
    LC_Bigint_init_unsigned(&bi_64, 64);

    uint64_t *op_digits = LC_Bigint_ptr(op);
    size_t    i         = op->digit_count - 1;

    while (1) {
        LC_BigInt shifted;
        LC_Bigint_shl(&shifted, dest, &bi_64);

        uint64_t result_scalar;
        uint64_t carry_scalar;
        mul_overflow(scalar, op_digits[i], &result_scalar, &carry_scalar);

        LC_BigInt result;
        LC_Bigint_init_unsigned(&result, result_scalar);

        LC_BigInt carry;
        LC_Bigint_init_unsigned(&carry, carry_scalar);

        LC_BigInt carry_shifted;
        LC_Bigint_shl(&carry_shifted, &carry, &bi_64);

        LC_BigInt tmp;
        LC_Bigint_add(&tmp, &shifted, &carry_shifted);

        LC_Bigint_add(dest, &tmp, &result);

        if (i == 0) {
            break;
        }
        i -= 1;
    }
}

LC_FUNCTION void LC_Bigint_mul(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2) {
    if (op1->digit_count == 0 || op2->digit_count == 0) {
        LC_Bigint_init_unsigned(dest, 0);
        return;
    }
    uint64_t *op1_digits = LC_Bigint_ptr(op1);
    uint64_t *op2_digits = LC_Bigint_ptr(op2);

    uint64_t carry;
    mul_overflow(op1_digits[0], op2_digits[0], &dest->digit, &carry);
    if (carry == 0 && op1->digit_count == 1 && op2->digit_count == 1) {
        dest->is_negative = (op1->is_negative != op2->is_negative);
        dest->digit_count = 1;
        LC_normalize(dest);
        return;
    }

    LC_Bigint_init_unsigned(dest, 0);

    LC_BigInt bi_64;
    LC_Bigint_init_unsigned(&bi_64, 64);

    size_t i = op2->digit_count - 1;
    for (;;) {
        LC_BigInt shifted;
        LC_Bigint_shl(&shifted, dest, &bi_64);

        LC_BigInt scalar_result;
        LC_mul_scalar(&scalar_result, op1, op2_digits[i]);

        LC_Bigint_add(dest, &scalar_result, &shifted);

        if (i == 0) {
            break;
        }
        i -= 1;
    }

    dest->is_negative = (op1->is_negative != op2->is_negative);
    LC_normalize(dest);
}

LC_FUNCTION void LC_Bigint_mul_wrap(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2, size_t bit_count, bool is_signed) {
    LC_BigInt unwrapped = {0};
    LC_Bigint_mul(&unwrapped, op1, op2);
    LC_Bigint_truncate(dest, &unwrapped, bit_count, is_signed);
}

LC_FUNCTION unsigned LC_count_leading_zeros(uint32_t val) {
    if (val == 0) return 32;

#if _MSC_VER
    unsigned long Index;
    _BitScanReverse(&Index, val);
    return Index ^ 31;
#else
    return __builtin_clz(val);
#endif
}

// Make a 64-bit integer from a high / low pair of 32-bit integers.
LC_FUNCTION uint64_t LC_make_64(uint32_t hi, uint32_t lo) {
    return (((uint64_t)hi) << 32) | ((uint64_t)lo);
}

// Return the high 32 bits of a 64 bit value.
LC_FUNCTION uint32_t LC_hi_32(uint64_t value) {
    return (uint32_t)(value >> 32);
}

// Return the low 32 bits of a 64 bit value.
LC_FUNCTION uint32_t LC_lo_32(uint64_t val) {
    return (uint32_t)val;
}

// Implementation of Knuth's Algorithm D (Division of nonnegative integers)
// from "Art of Computer Programming, Volume 2", section 4.3.1, p. 272. The
// variables here have the same names as in the algorithm. Comments explain
// the algorithm and any deviation from it.
LC_FUNCTION void LC_knuth_div(uint32_t *u, uint32_t *v, uint32_t *q, uint32_t *r, unsigned m, unsigned n) {
    LC_ASSERT(NULL, u && "Must provide dividend");
    LC_ASSERT(NULL, v && "Must provide divisor");
    LC_ASSERT(NULL, q && "Must provide quotient");
    LC_ASSERT(NULL, u != v && u != q && v != q && "Must use different memory");
    LC_ASSERT(NULL, n > 1 && "n must be > 1");

    // b denotes the base of the number system. In our case b is 2^32.
    uint64_t b = ((uint64_t)1) << 32;

    // D1. [Normalize.] Set d = b / (v[n-1] + 1) and multiply all the digits of
    // u and v by d. Note that we have taken Knuth's advice here to use a power
    // of 2 value for d such that d * v[n-1] >= b/2 (b is the base). A power of
    // 2 allows us to shift instead of multiply and it is easy to determine the
    // shift amount from the leading zeros.  We are basically normalizing the u
    // and v so that its high bits are shifted to the top of v's range without
    // overflow. Note that this can require an extra word in u so that u must
    // be of length m+n+1.
    unsigned shift   = LC_count_leading_zeros(v[n - 1]);
    uint32_t v_carry = 0;
    uint32_t u_carry = 0;
    if (shift) {
        for (unsigned i = 0; i < m + n; ++i) {
            uint32_t u_tmp = u[i] >> (32 - shift);
            u[i]           = (u[i] << shift) | u_carry;
            u_carry        = u_tmp;
        }
        for (unsigned i = 0; i < n; ++i) {
            uint32_t v_tmp = v[i] >> (32 - shift);
            v[i]           = (v[i] << shift) | v_carry;
            v_carry        = v_tmp;
        }
    }
    u[m + n] = u_carry;

    // D2. [Initialize j.]  Set j to m. This is the loop counter over the places.
    int j = (int)m;
    do {
        // D3. [Calculate q'.].
        //     Set qp = (u[j+n]*b + u[j+n-1]) / v[n-1]. (qp=qprime=q')
        //     Set rp = (u[j+n]*b + u[j+n-1]) % v[n-1]. (rp=rprime=r')
        // Now test if qp == b or qp*v[n-2] > b*rp + u[j+n-2]; if so, decrease
        // qp by 1, increase rp by v[n-1], and repeat this test if rp < b. The test
        // on v[n-2] determines at high speed most of the cases in which the trial
        // value qp is one too large, and it eliminates all cases where qp is two
        // too large.
        uint64_t dividend = LC_make_64(u[j + n], u[j + n - 1]);
        uint64_t qp       = dividend / v[n - 1];
        uint64_t rp       = dividend % v[n - 1];
        if (qp == b || qp * v[n - 2] > b * rp + u[j + n - 2]) {
            qp--;
            rp += v[n - 1];
            if (rp < b && (qp == b || qp * v[n - 2] > b * rp + u[j + n - 2])) {
                qp--;
            }
        }

        // D4. [Multiply and subtract.] Replace (u[j+n]u[j+n-1]...u[j]) with
        // (u[j+n]u[j+n-1]..u[j]) - qp * (v[n-1]...v[1]v[0]). This computation
        // consists of a simple multiplication by a one-place number, combined with
        // a subtraction.
        // The digits (u[j+n]...u[j]) should be kept positive; if the result of
        // this step is actually negative, (u[j+n]...u[j]) should be left as the
        // true value plus b**(n+1), namely as the b's complement of
        // the true value, and a "borrow" to the left should be remembered.
        int64_t borrow = 0;
        for (unsigned i = 0; i < n; ++i) {
            uint64_t p      = ((uint64_t)qp) * ((uint64_t)(v[i]));
            int64_t  subres = ((int64_t)(u[j + i])) - borrow - LC_lo_32(p);
            u[j + i]        = LC_lo_32((uint64_t)subres);
            borrow          = LC_hi_32(p) - LC_hi_32((uint64_t)subres);
        }
        bool is_neg = u[j + n] < borrow;
        u[j + n] -= LC_lo_32((uint64_t)borrow);

        // D5. [Test remainder.] Set q[j] = qp. If the result of step D4 was
        // negative, go to step D6; otherwise go on to step D7.
        q[j] = LC_lo_32(qp);
        if (is_neg) {
            // D6. [Add back]. The probability that this step is necessary is very
            // small, on the order of only 2/b. Make sure that test data accounts for
            // this possibility. Decrease q[j] by 1
            q[j]--;
            // and add (0v[n-1]...v[1]v[0]) to (u[j+n]u[j+n-1]...u[j+1]u[j]).
            // A carry will occur to the left of u[j+n], and it should be ignored
            // since it cancels with the borrow that occurred in D4.
            bool carry = false;
            for (unsigned i = 0; i < n; i++) {
                uint32_t limit = LC_u32_min(u[j + i], v[i]);
                u[j + i] += v[i] + carry;
                carry = u[j + i] < limit || (carry && u[j + i] == limit);
            }
            u[j + n] += carry;
        }

        // D7. [Loop on j.]  Decrease j by one. Now if j >= 0, go back to D3.
    } while (--j >= 0);

    // D8. [Unnormalize]. Now q[...] is the desired quotient, and the desired
    // remainder may be obtained by dividing u[...] by d. If r is non-null we
    // compute the remainder (urem uses this).
    if (r) {
        // The value d is expressed by the "shift" value above since we avoided
        // multiplication by d by using a shift left. So, all we have to do is
        // shift right here.
        if (shift) {
            uint32_t carry = 0;
            for (int i = (int)n - 1; i >= 0; i--) {
                r[i]  = (u[i] >> shift) | carry;
                carry = u[i] << (32 - shift);
            }
        } else {
            for (int i = (int)n - 1; i >= 0; i--) {
                r[i] = u[i];
            }
        }
    }
}

LC_FUNCTION void LC_Bigint_unsigned_division(LC_BigInt *op1, LC_BigInt *op2, LC_BigInt *Quotient, LC_BigInt *Remainder) {
    LC_CmpRes cmp = LC_Bigint_cmp(op1, op2);
    if (cmp == LC_CmpRes_LT) {
        if (!Quotient) {
            LC_Bigint_init_unsigned(Quotient, 0);
        }
        if (!Remainder) {
            LC_Bigint_init_bigint(Remainder, op1);
        }
        return;
    }
    if (cmp == LC_CmpRes_EQ) {
        if (!Quotient) {
            LC_Bigint_init_unsigned(Quotient, 1);
        }
        if (!Remainder) {
            LC_Bigint_init_unsigned(Remainder, 0);
        }
        return;
    }

    uint64_t *lhs      = LC_Bigint_ptr(op1);
    uint64_t *rhs      = LC_Bigint_ptr(op2);
    unsigned  lhsWords = op1->digit_count;
    unsigned  rhsWords = op2->digit_count;

    // First, compose the values into an array of 32-bit words instead of
    // 64-bit words. This is a necessity of both the "short division" algorithm
    // and the Knuth "classical algorithm" which requires there to be native
    // operations for +, -, and * on an m bit value with an m*2 bit result. We
    // can't use 64-bit operands here because we don't have native results of
    // 128-bits. Furthermore, casting the 64-bit values to 32-bit values won't
    // work on large-endian machines.
    unsigned n = rhsWords * 2;
    unsigned m = (lhsWords * 2) - n;

    // Allocate space for the temporary values we need either on the stack, if
    // it will fit, or on the heap if it won't.
    uint32_t  space[128];
    uint32_t *U = NULL;
    uint32_t *V = NULL;
    uint32_t *Q = NULL;
    uint32_t *R = NULL;
    if ((Remainder ? 4 : 3) * n + 2 * m + 1 <= 128) {
        U = &space[0];
        V = &space[m + n + 1];
        Q = &space[(m + n + 1) + n];
        if (Remainder) {
            R = &space[(m + n + 1) + n + (m + n)];
        }
    } else {
        U = (uint32_t *)malloc_arena(sizeof(uint32_t) * (m + n + 1));
        V = (uint32_t *)malloc_arena(sizeof(uint32_t) * n);
        Q = (uint32_t *)malloc_arena(sizeof(uint32_t) * (m + n));
        if (Remainder) {
            R = (uint32_t *)malloc_arena(sizeof(uint32_t) * n);
        }
    }

    // Initialize the dividend
    LC_MemoryZero(U, (m + n + 1) * sizeof(uint32_t));
    for (unsigned i = 0; i < lhsWords; ++i) {
        uint64_t tmp = lhs[i];
        U[i * 2]     = LC_lo_32(tmp);
        U[i * 2 + 1] = LC_hi_32(tmp);
    }
    U[m + n] = 0; // this extra word is for "spill" in the Knuth algorithm.

    // Initialize the divisor
    LC_MemoryZero(V, (n) * sizeof(uint32_t));
    for (unsigned i = 0; i < rhsWords; ++i) {
        uint64_t tmp = rhs[i];
        V[i * 2]     = LC_lo_32(tmp);
        V[i * 2 + 1] = LC_hi_32(tmp);
    }

    // initialize the quotient and remainder
    LC_MemoryZero(Q, (m + n) * sizeof(uint32_t));
    if (Remainder) LC_MemoryZero(R, n * sizeof(uint32_t));

    // Now, adjust m and n for the Knuth division. n is the number of words in
    // the divisor. m is the number of words by which the dividend exceeds the
    // divisor (i.e. m+n is the length of the dividend). These sizes must not
    // contain any zero words or the Knuth algorithm fails.
    for (unsigned i = n; i > 0 && V[i - 1] == 0; i--) {
        n--;
        m++;
    }
    for (unsigned i = m + n; i > 0 && U[i - 1] == 0; i--) {
        m--;
    }

    // If we're left with only a single word for the divisor, Knuth doesn't work
    // so we implement the short division algorithm here. This is much simpler
    // and faster because we are certain that we can divide a 64-bit quantity
    // by a 32-bit quantity at hardware speed and short division is simply a
    // series of such operations. This is just like doing short division but we
    // are using base 2^32 instead of base 10.
    LC_ASSERT(NULL, n != 0 && "Divide by zero?");
    if (n == 1) {
        uint32_t divisor = V[0];
        uint32_t rem     = 0;
        for (int i = (int)m; i >= 0; i--) {
            uint64_t partial_dividend = LC_make_64(rem, U[i]);
            if (partial_dividend == 0) {
                Q[i] = 0;
                rem  = 0;
            } else if (partial_dividend < divisor) {
                Q[i] = 0;
                rem  = LC_lo_32(partial_dividend);
            } else if (partial_dividend == divisor) {
                Q[i] = 1;
                rem  = 0;
            } else {
                Q[i] = LC_lo_32(partial_dividend / divisor);
                rem  = LC_lo_32(partial_dividend - (Q[i] * divisor));
            }
        }
        if (R) {
            R[0] = rem;
        }
    } else {
        // Now we're ready to invoke the Knuth classical divide algorithm. In this
        // case n > 1.
        LC_knuth_div(U, V, Q, R, m, n);
    }

    // If the caller wants the quotient
    if (Quotient) {
        Quotient->is_negative = false;
        Quotient->digit_count = lhsWords;
        if (lhsWords == 1) {
            Quotient->digit = LC_make_64(Q[1], Q[0]);
        } else {
            Quotient->digits = ALLOC_DIGITS(lhsWords);
            for (size_t i = 0; i < lhsWords; i += 1) {
                Quotient->digits[i] = LC_make_64(Q[i * 2 + 1], Q[i * 2]);
            }
        }
    }

    // If the caller wants the remainder
    if (Remainder) {
        Remainder->is_negative = false;
        Remainder->digit_count = rhsWords;
        if (rhsWords == 1) {
            Remainder->digit = LC_make_64(R[1], R[0]);
        } else {
            Remainder->digits = ALLOC_DIGITS(rhsWords);
            for (size_t i = 0; i < rhsWords; i += 1) {
                Remainder->digits[i] = LC_make_64(R[i * 2 + 1], R[i * 2]);
            }
        }
    }
}

LC_FUNCTION void LC_Bigint_div_trunc(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2) {
    LC_ASSERT(NULL, op2->digit_count != 0); // division by zero
    if (op1->digit_count == 0) {
        LC_Bigint_init_unsigned(dest, 0);
        return;
    }
    uint64_t *op1_digits = LC_Bigint_ptr(op1);
    uint64_t *op2_digits = LC_Bigint_ptr(op2);
    if (op1->digit_count == 1 && op2->digit_count == 1) {
        dest->digit       = op1_digits[0] / op2_digits[0];
        dest->digit_count = 1;
        dest->is_negative = op1->is_negative != op2->is_negative;
        LC_normalize(dest);
        return;
    }
    if (op2->digit_count == 1 && op2_digits[0] == 1) {
        // X / 1 == X
        LC_Bigint_init_bigint(dest, op1);
        dest->is_negative = op1->is_negative != op2->is_negative;
        LC_normalize(dest);
        return;
    }

    LC_BigInt *op1_positive;
    LC_BigInt  op1_positive_data;
    if (op1->is_negative) {
        LC_Bigint_negate(&op1_positive_data, op1);
        op1_positive = &op1_positive_data;
    } else {
        op1_positive = op1;
    }

    LC_BigInt *op2_positive;
    LC_BigInt  op2_positive_data;
    if (op2->is_negative) {
        LC_Bigint_negate(&op2_positive_data, op2);
        op2_positive = &op2_positive_data;
    } else {
        op2_positive = op2;
    }

    LC_Bigint_unsigned_division(op1_positive, op2_positive, dest, NULL);
    dest->is_negative = op1->is_negative != op2->is_negative;
    LC_normalize(dest);
}

LC_FUNCTION void LC_Bigint_div_floor(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2) {
    if (op1->is_negative != op2->is_negative) {
        LC_Bigint_div_trunc(dest, op1, op2);
        LC_BigInt mult_again = {0};
        LC_Bigint_mul(&mult_again, dest, op2);
        mult_again.is_negative = op1->is_negative;
        if (LC_Bigint_cmp(&mult_again, op1) != LC_CmpRes_EQ) {
            LC_BigInt tmp = {0};
            LC_Bigint_init_bigint(&tmp, dest);
            LC_BigInt neg_one = {0};
            LC_Bigint_init_signed(&neg_one, -1);
            LC_Bigint_add(dest, &tmp, &neg_one);
        }
        LC_normalize(dest);
    } else {
        LC_Bigint_div_trunc(dest, op1, op2);
    }
}

LC_FUNCTION void LC_Bigint_rem(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2) {
    LC_ASSERT(NULL, op2->digit_count != 0); // division by zero
    if (op1->digit_count == 0) {
        LC_Bigint_init_unsigned(dest, 0);
        return;
    }
    uint64_t *op1_digits = LC_Bigint_ptr(op1);
    uint64_t *op2_digits = LC_Bigint_ptr(op2);

    if (op1->digit_count == 1 && op2->digit_count == 1) {
        dest->digit       = op1_digits[0] % op2_digits[0];
        dest->digit_count = 1;
        dest->is_negative = op1->is_negative;
        LC_normalize(dest);
        return;
    }
    if (op2->digit_count == 2 && op2_digits[0] == 0 && op2_digits[1] == 1) {
        // special case this divisor
        LC_Bigint_init_unsigned(dest, op1_digits[0]);
        dest->is_negative = op1->is_negative;
        LC_normalize(dest);
        return;
    }

    if (op2->digit_count == 1 && op2_digits[0] == 1) {
        // X % 1 == 0
        LC_Bigint_init_unsigned(dest, 0);
        return;
    }

    LC_BigInt *op1_positive;
    LC_BigInt  op1_positive_data;
    if (op1->is_negative) {
        LC_Bigint_negate(&op1_positive_data, op1);
        op1_positive = &op1_positive_data;
    } else {
        op1_positive = op1;
    }

    LC_BigInt *op2_positive;
    LC_BigInt  op2_positive_data;
    if (op2->is_negative) {
        LC_Bigint_negate(&op2_positive_data, op2);
        op2_positive = &op2_positive_data;
    } else {
        op2_positive = op2;
    }

    LC_Bigint_unsigned_division(op1_positive, op2_positive, NULL, dest);
    dest->is_negative = op1->is_negative;
    LC_normalize(dest);
}

LC_FUNCTION void LC_Bigint_mod(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2) {
    if (op1->is_negative) {
        LC_BigInt first_rem;
        LC_Bigint_rem(&first_rem, op1, op2);
        first_rem.is_negative = !op2->is_negative;
        LC_BigInt op2_minus_rem;
        LC_Bigint_add(&op2_minus_rem, op2, &first_rem);
        LC_Bigint_rem(dest, &op2_minus_rem, op2);
        dest->is_negative = false;
    } else {
        LC_Bigint_rem(dest, op1, op2);
        dest->is_negative = false;
    }
}

LC_FUNCTION void LC_Bigint_or(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2) {
    if (op1->digit_count == 0) {
        LC_Bigint_init_bigint(dest, op2);
        return;
    }
    if (op2->digit_count == 0) {
        LC_Bigint_init_bigint(dest, op1);
        return;
    }
    if (op1->is_negative || op2->is_negative) {
        size_t big_bit_count = LC_size_max(LC_Bigint_bits_needed(op1), LC_Bigint_bits_needed(op2));

        LC_BigInt twos_comp_op1 = {0};
        LC_to_twos_complement(&twos_comp_op1, op1, big_bit_count);

        LC_BigInt twos_comp_op2 = {0};
        LC_to_twos_complement(&twos_comp_op2, op2, big_bit_count);

        LC_BigInt twos_comp_dest = {0};
        LC_Bigint_or(&twos_comp_dest, &twos_comp_op1, &twos_comp_op2);

        LC_from_twos_complement(dest, &twos_comp_dest, big_bit_count, true);
    } else {
        dest->is_negative    = false;
        uint64_t *op1_digits = LC_Bigint_ptr(op1);
        uint64_t *op2_digits = LC_Bigint_ptr(op2);
        if (op1->digit_count == 1 && op2->digit_count == 1) {
            dest->digit_count = 1;
            dest->digit       = op1_digits[0] | op2_digits[0];
            LC_normalize(dest);
            return;
        }
        dest->digit_count = LC_unsigned_max(op1->digit_count, op2->digit_count);
        dest->digits      = ALLOC_DIGITS(dest->digit_count);
        for (size_t i = 0; i < dest->digit_count; i += 1) {
            uint64_t digit = 0;
            if (i < op1->digit_count) {
                digit |= op1_digits[i];
            }
            if (i < op2->digit_count) {
                digit |= op2_digits[i];
            }
            dest->digits[i] = digit;
        }
        LC_normalize(dest);
    }
}

LC_FUNCTION void LC_Bigint_and(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2) {
    if (op1->digit_count == 0 || op2->digit_count == 0) {
        LC_Bigint_init_unsigned(dest, 0);
        return;
    }
    if (op1->is_negative || op2->is_negative) {
        size_t big_bit_count = LC_size_max(LC_Bigint_bits_needed(op1), LC_Bigint_bits_needed(op2));

        LC_BigInt twos_comp_op1 = {0};
        LC_to_twos_complement(&twos_comp_op1, op1, big_bit_count);

        LC_BigInt twos_comp_op2 = {0};
        LC_to_twos_complement(&twos_comp_op2, op2, big_bit_count);

        LC_BigInt twos_comp_dest = {0};
        LC_Bigint_and(&twos_comp_dest, &twos_comp_op1, &twos_comp_op2);

        LC_from_twos_complement(dest, &twos_comp_dest, big_bit_count, true);
    } else {
        dest->is_negative    = false;
        uint64_t *op1_digits = LC_Bigint_ptr(op1);
        uint64_t *op2_digits = LC_Bigint_ptr(op2);
        if (op1->digit_count == 1 && op2->digit_count == 1) {
            dest->digit_count = 1;
            dest->digit       = op1_digits[0] & op2_digits[0];
            LC_normalize(dest);
            return;
        }

        dest->digit_count = LC_unsigned_max(op1->digit_count, op2->digit_count);
        dest->digits      = ALLOC_DIGITS(dest->digit_count);

        size_t i = 0;
        for (; i < op1->digit_count && i < op2->digit_count; i += 1) {
            dest->digits[i] = op1_digits[i] & op2_digits[i];
        }
        for (; i < dest->digit_count; i += 1) {
            dest->digits[i] = 0;
        }
        LC_normalize(dest);
    }
}

LC_FUNCTION void LC_Bigint_xor(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2) {
    if (op1->digit_count == 0) {
        LC_Bigint_init_bigint(dest, op2);
        return;
    }
    if (op2->digit_count == 0) {
        LC_Bigint_init_bigint(dest, op1);
        return;
    }
    if (op1->is_negative || op2->is_negative) {
        size_t big_bit_count = LC_size_max(LC_Bigint_bits_needed(op1), LC_Bigint_bits_needed(op2));

        LC_BigInt twos_comp_op1 = {0};
        LC_to_twos_complement(&twos_comp_op1, op1, big_bit_count);

        LC_BigInt twos_comp_op2 = {0};
        LC_to_twos_complement(&twos_comp_op2, op2, big_bit_count);

        LC_BigInt twos_comp_dest = {0};
        LC_Bigint_xor(&twos_comp_dest, &twos_comp_op1, &twos_comp_op2);

        LC_from_twos_complement(dest, &twos_comp_dest, big_bit_count, true);
    } else {
        dest->is_negative    = false;
        uint64_t *op1_digits = LC_Bigint_ptr(op1);
        uint64_t *op2_digits = LC_Bigint_ptr(op2);

        LC_ASSERT(NULL, op1->digit_count > 0 && op2->digit_count > 0);
        if (op1->digit_count == 1 && op2->digit_count == 1) {
            dest->digit_count = 1;
            dest->digit       = op1_digits[0] ^ op2_digits[0];
            LC_normalize(dest);
            return;
        }
        dest->digit_count = LC_unsigned_max(op1->digit_count, op2->digit_count);
        dest->digits      = ALLOC_DIGITS(dest->digit_count);
        size_t i          = 0;
        for (; i < op1->digit_count && i < op2->digit_count; i += 1) {
            dest->digits[i] = op1_digits[i] ^ op2_digits[i];
        }
        for (; i < dest->digit_count; i += 1) {
            if (i < op1->digit_count) {
                dest->digits[i] = op1_digits[i];
            } else if (i < op2->digit_count) {
                dest->digits[i] = op2_digits[i];
            } else {
                LC_ASSERT(NULL, !"Unreachable");
            }
        }
        LC_normalize(dest);
    }
}

LC_FUNCTION void LC_Bigint_shl(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2) {
    LC_ASSERT(NULL, !op2->is_negative);

    if (op2->digit_count == 0) {
        return;
    }
    if (op2->digit_count != 1) {
        LC_ASSERT(NULL, !"Unsupported: shift left by amount greater than 64 bit integer");
    }
    LC_Bigint_shl_int(dest, op1, LC_Bigint_as_unsigned(op2));
}

LC_FUNCTION void LC_Bigint_shl_int(LC_BigInt *dest, LC_BigInt *op1, uint64_t shift) {
    if (shift == 0) {
        LC_Bigint_init_bigint(dest, op1);
        return;
    }

    if (op1->digit_count == 0) {
        LC_Bigint_init_unsigned(dest, 0);
        return;
    }

    uint64_t *op1_digits = LC_Bigint_ptr(op1);

    if (op1->digit_count == 1 && shift < 64) {
        dest->digit = op1_digits[0] << shift;
        if (dest->digit > op1_digits[0]) {
            dest->digit_count = 1;
            dest->is_negative = op1->is_negative;
            return;
        }
    }

    uint64_t digit_shift_count    = shift / 64;
    uint64_t leftover_shift_count = shift % 64;

    dest->digits      = ALLOC_DIGITS(op1->digit_count + digit_shift_count + 1);
    dest->digit_count = (unsigned)digit_shift_count;
    uint64_t carry    = 0;
    for (size_t i = 0; i < op1->digit_count; i += 1) {
        uint64_t digit                  = op1_digits[i];
        dest->digits[dest->digit_count] = carry | (digit << leftover_shift_count);
        dest->digit_count++;
        if (leftover_shift_count > 0) {
            carry = digit >> (64 - leftover_shift_count);
        } else {
            carry = 0;
        }
    }
    dest->digits[dest->digit_count] = carry;
    dest->digit_count += 1;
    dest->is_negative = op1->is_negative;
    LC_normalize(dest);
}

LC_FUNCTION void LC_Bigint_shl_trunc(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2, size_t bit_count, bool is_signed) {
    LC_BigInt unwrapped = {0};
    LC_Bigint_shl(&unwrapped, op1, op2);
    LC_Bigint_truncate(dest, &unwrapped, bit_count, is_signed);
}

LC_FUNCTION void LC_Bigint_shr(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2) {
    LC_ASSERT(NULL, !op2->is_negative);

    if (op1->digit_count == 0) {
        LC_Bigint_init_unsigned(dest, 0);
        return;
    }

    if (op2->digit_count == 0) {
        LC_Bigint_init_bigint(dest, op1);
        return;
    }

    if (op2->digit_count != 1) {
        LC_ASSERT(NULL, !"Unsupported: shift right by amount greater than 64 bit integer");
    }

    uint64_t *op1_digits = LC_Bigint_ptr(op1);
    uint64_t  shift_amt  = LC_Bigint_as_unsigned(op2);

    if (op1->digit_count == 1) {
        dest->digit       = shift_amt < 64 ? op1_digits[0] >> shift_amt : 0;
        dest->digit_count = 1;
        dest->is_negative = op1->is_negative;
        LC_normalize(dest);
        return;
    }

    uint64_t digit_shift_count    = shift_amt / 64;
    uint64_t leftover_shift_count = shift_amt % 64;

    if (digit_shift_count >= op1->digit_count) {
        LC_Bigint_init_unsigned(dest, 0);
        return;
    }

    dest->digit_count = (unsigned)(op1->digit_count - digit_shift_count);
    uint64_t *digits;
    if (dest->digit_count == 1) {
        digits = &dest->digit;
    } else {
        digits       = ALLOC_DIGITS(dest->digit_count);
        dest->digits = digits;
    }

    uint64_t carry = 0;
    for (size_t op_digit_index = op1->digit_count - 1;;) {
        uint64_t digit            = op1_digits[op_digit_index];
        size_t   dest_digit_index = op_digit_index - digit_shift_count;
        digits[dest_digit_index]  = carry | (digit >> leftover_shift_count);
        carry                     = digit << (64 - leftover_shift_count);

        if (dest_digit_index == 0) break;
        op_digit_index -= 1;
    }
    dest->is_negative = op1->is_negative;
    LC_normalize(dest);
}

LC_FUNCTION void LC_Bigint_not(LC_BigInt *dest, LC_BigInt *op, size_t bit_count, bool is_signed) {
    if (bit_count == 0) {
        LC_Bigint_init_unsigned(dest, 0);
        return;
    }

    if (is_signed) {
        LC_BigInt twos_comp = {0};
        LC_to_twos_complement(&twos_comp, op, bit_count);

        LC_BigInt inverted = {0};
        LC_Bigint_not(&inverted, &twos_comp, bit_count, false);

        LC_from_twos_complement(dest, &inverted, bit_count, true);
        return;
    }

    LC_ASSERT(NULL, !op->is_negative);

    dest->is_negative   = false;
    uint64_t *op_digits = LC_Bigint_ptr(op);
    if (bit_count <= 64) {
        dest->digit_count = 1;
        if (op->digit_count == 0) {
            if (bit_count == 64) {
                dest->digit = UINT64_MAX;
            } else {
                dest->digit = (1ULL << bit_count) - 1;
            }
        } else if (op->digit_count == 1) {
            dest->digit = ~op_digits[0];
            if (bit_count != 64) {
                uint64_t
                    mask = (1ULL << bit_count) - 1;
                dest->digit &= mask;
            }
        }
        LC_normalize(dest);
        return;
    }
    dest->digit_count = (unsigned int)((bit_count + 63) / 64);
    LC_ASSERT(NULL, dest->digit_count >= op->digit_count);
    dest->digits = ALLOC_DIGITS(dest->digit_count);
    size_t i     = 0;
    for (; i < op->digit_count; i += 1) {
        dest->digits[i] = ~op_digits[i];
    }
    for (; i < dest->digit_count; i += 1) {
        dest->digits[i] = 0xffffffffffffffffULL;
    }
    size_t digit_index     = dest->digit_count - 1;
    size_t digit_bit_index = bit_count % 64;
    if (digit_bit_index != 0) {
        uint64_t
            mask = (1ULL << digit_bit_index) - 1;
        dest->digits[digit_index] &= mask;
    }
    LC_normalize(dest);
}

LC_FUNCTION void LC_Bigint_truncate(LC_BigInt *dst, LC_BigInt *op, size_t bit_count, bool is_signed) {
    LC_BigInt twos_comp;
    LC_to_twos_complement(&twos_comp, op, bit_count);
    LC_from_twos_complement(dst, &twos_comp, bit_count, is_signed);
}

LC_FUNCTION LC_CmpRes LC_Bigint_cmp(LC_BigInt *op1, LC_BigInt *op2) {
    if (op1->is_negative && !op2->is_negative) return LC_CmpRes_LT;
    if (!op1->is_negative && op2->is_negative) return LC_CmpRes_GT;
    if (op1->digit_count > op2->digit_count) return op1->is_negative ? LC_CmpRes_LT : LC_CmpRes_GT;
    if (op2->digit_count > op1->digit_count) return op1->is_negative ? LC_CmpRes_GT : LC_CmpRes_LT;
    if (op1->digit_count == 0) return LC_CmpRes_EQ;

    uint64_t *op1_digits = LC_Bigint_ptr(op1);
    uint64_t *op2_digits = LC_Bigint_ptr(op2);
    for (unsigned i = op1->digit_count - 1;; i--) {
        uint64_t op1_digit = op1_digits[i];
        uint64_t op2_digit = op2_digits[i];

        if (op1_digit > op2_digit) {
            return op1->is_negative ? LC_CmpRes_LT : LC_CmpRes_GT;
        }
        if (op1_digit < op2_digit) {
            return op1->is_negative ? LC_CmpRes_GT : LC_CmpRes_LT;
        }
        if (i == 0) {
            return LC_CmpRes_EQ;
        }
    }
}

LC_FUNCTION char *LC_Bigint_str(LC_BigInt *bigint, uint64_t base) {
    LC_StringList out = {0};
    if (bigint->digit_count == 0) {
        return "0";
    }
    if (bigint->is_negative) {
        LC_Addf(L->arena, &out, "-");
    }
    if (bigint->digit_count == 1 && base == 10) {
        LC_Addf(L->arena, &out, "%llu", (unsigned long long)bigint->digit);
    } else {
        size_t len   = bigint->digit_count * 64;
        char  *start = (char *)malloc_arena(len);
        char  *buf   = start;

        LC_BigInt  digit_bi = {0};
        LC_BigInt  a1       = {0};
        LC_BigInt  a2       = {0};
        LC_BigInt  base_bi  = {0};
        LC_BigInt *a        = &a1;
        LC_BigInt *other_a  = &a2;

        LC_Bigint_init_bigint(a, bigint);
        LC_Bigint_init_unsigned(&base_bi, base);

        for (;;) {
            LC_Bigint_rem(&digit_bi, a, &base_bi);
            uint8_t digit = (uint8_t)LC_Bigint_as_unsigned(&digit_bi);
            *(buf++)      = LC_digit_to_char(digit, false);
            LC_Bigint_div_trunc(other_a, a, &base_bi);
            {
                LC_BigInt *tmp = a;
                a              = other_a;
                other_a        = tmp;
            }
            if (LC_Bigint_cmp_zero(a) == LC_CmpRes_EQ) {
                break;
            }
        }

        // reverse

        for (char *ptr = buf - 1; ptr >= start; ptr--) {
            LC_Addf(L->arena, &out, "%c", *ptr);
        }
    }
    LC_String s = LC_MergeString(L->arena, out);
    return s.str;
}

LC_FUNCTION int64_t LC_Bigint_as_signed(LC_BigInt *bigint) {
    if (bigint->digit_count == 0) return 0;
    if (bigint->digit_count != 1) {
        LC_ASSERT(NULL, !"LC_BigInt larger than i64");
    }

    if (bigint->is_negative) {
        // TODO this code path is untested
        if (bigint->digit <= 9223372036854775808ULL) {
            return (-((int64_t)(bigint->digit - 1))) - 1;
        }
        LC_ASSERT(NULL, !"LC_BigInt does not fit in i64");
    }
    return (int64_t)bigint->digit;
}

LC_FUNCTION LC_CmpRes LC_Bigint_cmp_zero(LC_BigInt *op) {
    if (op->digit_count == 0) {
        return LC_CmpRes_EQ;
    }
    return op->is_negative ? LC_CmpRes_LT : LC_CmpRes_GT;
}

LC_FUNCTION double LC_Bigint_as_float(LC_BigInt *bigint) {
    if (LC_Bigint_fits_in_bits(bigint, 64, bigint->is_negative)) {
        return bigint->is_negative ? (double)LC_Bigint_as_signed(bigint) : (double)LC_Bigint_as_unsigned(bigint);
    }
    LC_BigInt div;
    uint64_t  mult = 0x100000000000ULL;
    double    mul  = 1;
    LC_Bigint_init_unsigned(&div, mult);
    LC_BigInt current;
    LC_Bigint_init_bigint(&current, bigint);
    double f = 0;
    do {
        LC_BigInt temp;
        LC_Bigint_mod(&temp, &current, &div);
        f += LC_Bigint_as_signed(&temp) * mul;
        mul *= mult;
        LC_Bigint_div_trunc(&temp, &current, &div);
        current = temp;
    } while (current.digit_count > 0);
    return f;
}
LC_Operand LC_OPNull;

LC_FUNCTION LC_Operand LC_OPError(void) {
    LC_Operand result = {LC_OPF_Error};
    return result;
}

LC_FUNCTION LC_Operand LC_OPConstType(LC_Type *type) {
    LC_Operand result = {LC_OPF_UTConst | LC_OPF_Const};
    result.type       = type;
    return result;
}

LC_FUNCTION LC_Operand LC_OPDecl(LC_Decl *decl) {
    LC_Operand result = {0};
    result.decl       = decl;
    return result;
}

LC_FUNCTION LC_Operand LC_OPType(LC_Type *type) {
    LC_Operand result = {0};
    result.type       = type;
    return result;
}

LC_FUNCTION LC_Operand LC_OPLValueAndType(LC_Type *type) {
    LC_Operand result = LC_OPType(type);
    result.flags      = LC_OPF_LValue;
    return result;
}

LC_FUNCTION LC_Operand LC_ReportASTError(LC_AST *n, const char *str, ...) {
    LC_FORMAT(L->arena, str, s8);
    LC_SendErrorMessage(n ? n->pos : NULL, s8);
    L->errors += 1;
    return LC_OPError();
}

LC_FUNCTION LC_Operand LC_ReportASTErrorEx(LC_AST *n1, LC_AST *n2, const char *str, ...) {
    LC_FORMAT(L->arena, str, s8);
    LC_SendErrorMessage(n1->pos, s8);
    LC_SendErrorMessage(n2->pos, s8);
    L->errors += 1;
    return LC_OPError();
}

LC_FUNCTION LC_Operand LC_ConstCastFloat(LC_AST *pos, LC_Operand op) {
    LC_ASSERT(pos, LC_IsUTConst(op));
    LC_ASSERT(pos, LC_IsUntyped(op.type));
    if (LC_IsUTInt(op.type)) op.val.d = LC_Bigint_as_float(&op.val.i);
    if (LC_IsUTStr(op.type)) return LC_ReportASTError(pos, "Trying to convert '%s' to float", LC_GenLCType(op.type));
    op.type = L->tuntypedfloat;
    return op;
}

LC_FUNCTION LC_Operand LC_ConstCastInt(LC_AST *pos, LC_Operand op) {
    LC_ASSERT(pos, LC_IsUTConst(op));
    LC_ASSERT(pos, LC_IsUntyped(op.type));
    if (LC_IsUTFloat(op.type)) {
        double v = op.val.d; // add rounding?
        LC_Bigint_init_signed(&op.val.i, (int64_t)v);
    }
    if (LC_IsUTStr(op.type)) return LC_ReportASTError(pos, "Trying to convert %s to int", LC_GenLCType(op.type));
    op.type = L->tuntypedint;
    return op;
}

LC_FUNCTION LC_Operand LC_OPInt(int64_t v) {
    LC_Operand op = {0};
    op.type       = L->tuntypedint;
    op.flags |= LC_OPF_UTConst | LC_OPF_Const;
    LC_Bigint_init_signed(&op.v.i, v);
    return op;
}

LC_FUNCTION LC_Operand LC_OPIntT(LC_Type *type, int64_t v) {
    LC_ASSERT(NULL, LC_IsInt(type));
    LC_Operand op = LC_OPInt(v);
    op.type       = type;
    return op;
}

LC_FUNCTION LC_Operand LC_OPModDefaultUT(LC_Operand val) {
    if (LC_IsUntyped(val.type)) {
        val.type = val.type->tbase;
    }
    return val;
}

LC_FUNCTION LC_Operand LC_OPModType(LC_Operand op, LC_Type *type) {
    if (LC_IsUTConst(op)) {
        if (LC_IsUTInt(op.type) && LC_IsFloat(type)) {
            op = LC_ConstCastFloat(NULL, op);
        }
    }
    op.type = type;
    return op;
}

LC_FUNCTION LC_Operand LC_OPModBool(LC_Operand op) {
    op.type = L->tuntypedbool;
    return op;
}

LC_FUNCTION LC_Operand LC_OPModBoolV(LC_Operand op, int v) {
    op.type = L->tuntypedbool;
    LC_Bigint_init_signed(&op.v.i, v);
    return op;
}

LC_FUNCTION LC_Operand LC_EvalBinary(LC_AST *pos, LC_Operand a, LC_TokenKind op, LC_Operand b) {
    LC_ASSERT(pos, LC_IsUTConst(a));
    LC_ASSERT(pos, LC_IsUTConst(b));
    LC_ASSERT(pos, LC_IsUntyped(a.type));
    LC_ASSERT(pos, LC_IsUntyped(b.type));
    LC_ASSERT(pos, a.type->kind == b.type->kind);

    LC_Operand c = LC_OPConstType(a.type);
    if (LC_IsUTStr(a.type)) {
        return LC_ReportASTError(pos, "invalid operand %s for binary expr of type untyped string", LC_TokenKindToString(op));
    }
    if (LC_IsUTFloat(a.type)) {
        switch (op) {
        case LC_TokenKind_Add: c.v.d = a.v.d + b.v.d; break;
        case LC_TokenKind_Sub: c.v.d = a.v.d - b.v.d; break;
        case LC_TokenKind_Mul: c.v.d = a.v.d * b.v.d; break;
        case LC_TokenKind_Div: {
            if (b.v.d == 0.0) return LC_ReportASTError(pos, "division by 0");
            c.v.d = a.v.d / b.v.d;
        } break;
        case LC_TokenKind_LesserThenEq: c = LC_OPModBoolV(c, a.v.d <= b.v.d); break;
        case LC_TokenKind_GreaterThenEq: c = LC_OPModBoolV(c, a.v.d >= b.v.d); break;
        case LC_TokenKind_GreaterThen: c = LC_OPModBoolV(c, a.v.d > b.v.d); break;
        case LC_TokenKind_LesserThen: c = LC_OPModBoolV(c, a.v.d < b.v.d); break;
        case LC_TokenKind_Equals: c = LC_OPModBoolV(c, a.v.d == b.v.d); break;
        case LC_TokenKind_NotEquals: c = LC_OPModBoolV(c, a.v.d != b.v.d); break;
        default: return LC_ReportASTError(pos, "invalid operand %s for binary expr of type untyped float", LC_TokenKindToString(op));
        }
    }
    if (LC_IsUTInt(a.type)) {
        switch (op) {
        case LC_TokenKind_BitXor: LC_Bigint_xor(&c.v.i, &a.v.i, &b.v.i); break;
        case LC_TokenKind_BitAnd: LC_Bigint_and(&c.v.i, &a.v.i, &b.v.i); break;
        case LC_TokenKind_BitOr: LC_Bigint_or(&c.v.i, &a.v.i, &b.v.i); break;
        case LC_TokenKind_Add: LC_Bigint_add(&c.v.i, &a.v.i, &b.v.i); break;
        case LC_TokenKind_Sub: LC_Bigint_sub(&c.v.i, &a.v.i, &b.v.i); break;
        case LC_TokenKind_Mul: LC_Bigint_mul(&c.v.i, &a.v.i, &b.v.i); break;
        case LC_TokenKind_Div: {
            if (b.v.i.digit_count == 0) return LC_ReportASTError(pos, "division by zero in constant expression");
            LC_Bigint_div_floor(&c.v.i, &a.v.i, &b.v.i);
        } break;
        case LC_TokenKind_Mod: {
            if (b.v.i.digit_count == 0) return LC_ReportASTError(pos, "modulo by zero in constant expression");
            LC_Bigint_mod(&c.v.i, &a.v.i, &b.v.i);
        } break;
        case LC_TokenKind_LeftShift: LC_Bigint_shl(&c.v.i, &a.v.i, &b.v.i); break;
        case LC_TokenKind_RightShift: LC_Bigint_shr(&c.v.i, &a.v.i, &b.v.i); break;
        case LC_TokenKind_And: {
            int left  = LC_Bigint_cmp_zero(&a.v.i) != LC_CmpRes_EQ;
            int right = LC_Bigint_cmp_zero(&b.v.i) != LC_CmpRes_EQ;
            c         = LC_OPModBoolV(c, left && right);
        } break;
        case LC_TokenKind_Or: {
            int left  = LC_Bigint_cmp_zero(&a.v.i) != LC_CmpRes_EQ;
            int right = LC_Bigint_cmp_zero(&b.v.i) != LC_CmpRes_EQ;
            c         = LC_OPModBoolV(c, left || right);
        } break;
        default: {
            LC_CmpRes cmp = LC_Bigint_cmp(&a.v.i, &b.v.i);
            switch (op) {
            case LC_TokenKind_LesserThenEq: c = LC_OPModBoolV(c, (cmp == LC_CmpRes_LT) || (cmp == LC_CmpRes_EQ)); break;
            case LC_TokenKind_GreaterThenEq: c = LC_OPModBoolV(c, (cmp == LC_CmpRes_GT) || (cmp == LC_CmpRes_EQ)); break;
            case LC_TokenKind_GreaterThen: c = LC_OPModBoolV(c, cmp == LC_CmpRes_GT); break;
            case LC_TokenKind_LesserThen: c = LC_OPModBoolV(c, cmp == LC_CmpRes_LT); break;
            case LC_TokenKind_Equals: c = LC_OPModBoolV(c, cmp == LC_CmpRes_EQ); break;
            case LC_TokenKind_NotEquals: c = LC_OPModBoolV(c, cmp != LC_CmpRes_EQ); break;
            default: return LC_ReportASTError(pos, "invalid operand %s for binary expr of type untyped int", LC_TokenKindToString(op));
            }
        }
        }
    }

    return c;
}

LC_FUNCTION LC_Operand LC_EvalUnary(LC_AST *pos, LC_TokenKind op, LC_Operand a) {
    LC_ASSERT(pos, LC_IsUTConst(a));
    LC_ASSERT(pos, LC_IsUntyped(a.type));
    LC_Operand c = a;

    if (LC_IsUTStr(a.type)) {
        return LC_ReportASTError(pos, "invalid operand %s for unary expr of type untyped string", LC_TokenKindToString(op));
    }
    if (LC_IsUTFloat(a.type)) {
        switch (op) {
        case LC_TokenKind_Sub: c.v.d = -a.v.d; break;
        case LC_TokenKind_Add: c.v.d = +a.v.d; break;
        default: return LC_ReportASTError(pos, "invalid operand %s for unary expr of type untyped float", LC_TokenKindToString(op));
        }
    }
    if (LC_IsUTInt(a.type)) {
        switch (op) {
        case LC_TokenKind_Not: c = LC_OPModBoolV(c, LC_Bigint_cmp_zero(&a.v.i) == LC_CmpRes_EQ); break;
        case LC_TokenKind_Sub: LC_Bigint_negate(&c.v.i, &a.v.i); break;
        case LC_TokenKind_Add: c = a; break;
        case LC_TokenKind_Neg: LC_Bigint_not(&c.v.i, &a.v.i, a.type->tbase->size * 8, !a.type->tbase->is_unsigned); break;
        default: return LC_ReportASTError(pos, "invalid operand %s for unary expr of type untyped int", LC_TokenKindToString(op));
        }
    }

    return c;
}

LC_FUNCTION bool LC_BigIntFits(LC_BigInt i, LC_Type *type) {
    LC_ASSERT(NULL, LC_IsInt(type));
    if (!LC_Bigint_fits_in_bits(&i, type->size * 8, !type->is_unsigned)) {
        return false;
    }
    return true;
}

LC_FUNCTION LC_OPResult LC_IsBinaryExprValidForType(LC_TokenKind op, LC_Type *type) {
    if (LC_IsFloat(type)) {
        switch (op) {
        case LC_TokenKind_Add: return LC_OPResult_Ok; break;
        case LC_TokenKind_Sub: return LC_OPResult_Ok; break;
        case LC_TokenKind_Mul: return LC_OPResult_Ok; break;
        case LC_TokenKind_Div: return LC_OPResult_Ok; break;
        case LC_TokenKind_LesserThenEq: return LC_OPResult_Bool; break;
        case LC_TokenKind_GreaterThenEq: return LC_OPResult_Bool; break;
        case LC_TokenKind_GreaterThen: return LC_OPResult_Bool; break;
        case LC_TokenKind_LesserThen: return LC_OPResult_Bool; break;
        case LC_TokenKind_Equals: return LC_OPResult_Bool; break;
        case LC_TokenKind_NotEquals: return LC_OPResult_Bool; break;
        default: return LC_OPResult_Error;
        }
    }
    if (LC_IsInt(type)) {
        switch (op) {
        case LC_TokenKind_BitXor: return LC_OPResult_Ok; break;
        case LC_TokenKind_BitAnd: return LC_OPResult_Ok; break;
        case LC_TokenKind_BitOr: return LC_OPResult_Ok; break;
        case LC_TokenKind_Add: return LC_OPResult_Ok; break;
        case LC_TokenKind_Sub: return LC_OPResult_Ok; break;
        case LC_TokenKind_Mul: return LC_OPResult_Ok; break;
        case LC_TokenKind_Div: return LC_OPResult_Ok; break;
        case LC_TokenKind_Mod: return LC_OPResult_Ok; break;
        case LC_TokenKind_LeftShift: return LC_OPResult_Ok; break;
        case LC_TokenKind_RightShift: return LC_OPResult_Ok; break;
        case LC_TokenKind_And: return LC_OPResult_Bool; break;
        case LC_TokenKind_Or: return LC_OPResult_Bool; break;
        case LC_TokenKind_LesserThenEq: return LC_OPResult_Bool; break;
        case LC_TokenKind_GreaterThenEq: return LC_OPResult_Bool; break;
        case LC_TokenKind_GreaterThen: return LC_OPResult_Bool; break;
        case LC_TokenKind_LesserThen: return LC_OPResult_Bool; break;
        case LC_TokenKind_Equals: return LC_OPResult_Bool; break;
        case LC_TokenKind_NotEquals: return LC_OPResult_Bool; break;
        default: return LC_OPResult_Error;
        }
    }
    if (LC_IsPtrLike(type)) {
        switch (op) {
        case LC_TokenKind_And: return LC_OPResult_Bool; break;
        case LC_TokenKind_Or: return LC_OPResult_Bool; break;
        case LC_TokenKind_LesserThenEq: return LC_OPResult_Bool; break;
        case LC_TokenKind_GreaterThenEq: return LC_OPResult_Bool; break;
        case LC_TokenKind_GreaterThen: return LC_OPResult_Bool; break;
        case LC_TokenKind_LesserThen: return LC_OPResult_Bool; break;
        case LC_TokenKind_Equals: return LC_OPResult_Bool; break;
        case LC_TokenKind_NotEquals: return LC_OPResult_Bool; break;
        default: return LC_OPResult_Error;
        }
    }

    return LC_OPResult_Error;
}

LC_FUNCTION LC_OPResult LC_IsUnaryOpValidForType(LC_TokenKind op, LC_Type *type) {
    if (LC_IsFloat(type)) {
        if (op == LC_TokenKind_Sub) return LC_OPResult_Ok;
        if (op == LC_TokenKind_Add) return LC_OPResult_Ok;
    }
    if (LC_IsInt(type)) {
        if (op == LC_TokenKind_Not) return LC_OPResult_Bool;
        if (op == LC_TokenKind_Sub) return LC_OPResult_Ok;
        if (op == LC_TokenKind_Add) return LC_OPResult_Ok;
        if (op == LC_TokenKind_Neg) return LC_OPResult_Ok;
    }
    if (LC_IsPtrLike(type)) {
        if (op == LC_TokenKind_Not) return LC_OPResult_Bool;
    }
    return LC_OPResult_Error;
}

LC_FUNCTION LC_OPResult LC_IsAssignValidForType(LC_TokenKind op, LC_Type *type) {
    if (op == LC_TokenKind_Assign) return LC_OPResult_Ok;
    if (LC_IsInt(type)) {
        switch (op) {
        case LC_TokenKind_DivAssign: return LC_OPResult_Ok;
        case LC_TokenKind_MulAssign: return LC_OPResult_Ok;
        case LC_TokenKind_ModAssign: return LC_OPResult_Ok;
        case LC_TokenKind_SubAssign: return LC_OPResult_Ok;
        case LC_TokenKind_AddAssign: return LC_OPResult_Ok;
        case LC_TokenKind_BitAndAssign: return LC_OPResult_Ok;
        case LC_TokenKind_BitOrAssign: return LC_OPResult_Ok;
        case LC_TokenKind_BitXorAssign: return LC_OPResult_Ok;
        case LC_TokenKind_LeftShiftAssign: return LC_OPResult_Ok;
        case LC_TokenKind_RightShiftAssign: return LC_OPResult_Ok;
        default: return LC_OPResult_Error;
        }
    }
    if (LC_IsFloat(type)) {
        switch (op) {
        case LC_TokenKind_DivAssign: return LC_OPResult_Ok;
        case LC_TokenKind_MulAssign: return LC_OPResult_Ok;
        case LC_TokenKind_SubAssign: return LC_OPResult_Ok;
        case LC_TokenKind_AddAssign: return LC_OPResult_Ok;
        default: return LC_OPResult_Error;
        }
    }
    return LC_OPResult_Error;
}

LC_FUNCTION int LC_GetLevelsOfIndirection(LC_Type *type) {
    if (type->kind == LC_TypeKind_Pointer) return LC_GetLevelsOfIndirection(type->tbase) + 1;
    if (type->kind == LC_TypeKind_Array) return LC_GetLevelsOfIndirection(type->tbase) + 1;
    return 0;
}

LC_FUNCTION LC_AST *LC_CreateAST(LC_Token *pos, LC_ASTKind kind) {
    LC_AST *n = LC_PushStruct(L->ast_arena, LC_AST);
    n->id     = ++L->ast_count;
    n->kind   = kind;
    n->pos    = pos;
    if (L->parser == &L->quick_parser) n->pos = &L->BuiltinToken;
    return n;
}

LC_FUNCTION LC_AST *LC_CreateUnary(LC_Token *pos, LC_TokenKind op, LC_AST *expr) {
    LC_AST *r      = LC_CreateAST(pos, LC_ASTKind_ExprUnary);
    r->eunary.op   = op;
    r->eunary.expr = expr;
    return r;
}

LC_FUNCTION LC_AST *LC_CreateBinary(LC_Token *pos, LC_AST *left, LC_AST *right, LC_TokenKind op) {
    LC_AST *r        = LC_CreateAST(pos, LC_ASTKind_ExprBinary);
    r->ebinary.op    = op;
    r->ebinary.left  = left;
    r->ebinary.right = right;
    return r;
}

LC_FUNCTION LC_AST *LC_CreateIndex(LC_Token *pos, LC_AST *left, LC_AST *index) {
    LC_AST *r       = LC_CreateAST(pos, LC_ASTKind_ExprIndex);
    r->eindex.index = index;
    r->eindex.base  = left;
    return r;
}

LC_FUNCTION void LC_SetPointerSizeAndAlign(int size, int align) {
    L->pointer_align = align;
    L->pointer_size  = size;
    if (L->tpvoid) {
        L->tpvoid->size  = size;
        L->tpvoid->align = align;
    }
    if (L->tpchar) {
        L->tpchar->size  = size;
        L->tpchar->align = align;
    }
}

LC_FUNCTION LC_Type *LC_CreateType(LC_TypeKind kind) {
    LC_Type *r = LC_PushStruct(L->type_arena, LC_Type);
    L->type_count += 1;
    r->kind = kind;
    r->id   = ++L->typeids;
    if (r->kind == LC_TypeKind_Proc || r->kind == LC_TypeKind_Pointer) {
        r->size        = L->pointer_size;
        r->align       = L->pointer_align;
        r->is_unsigned = true;
    }
    return r;
}

LC_FUNCTION LC_Type *LC_CreateTypedef(LC_Decl *decl, LC_Type *base) {
    LC_Type *n                      = LC_CreateType(base->kind);
    *n                              = *base;
    decl->typedef_renamed_type_decl = base->decl;
    n->decl                         = decl;
    return n;
}

LC_FUNCTION LC_Type *LC_CreatePointerType(LC_Type *type) {
    uint64_t key   = (uint64_t)type;
    LC_Type *entry = (LC_Type *)LC_MapGetU64(&L->type_map, key);
    if (entry) {
        return entry;
    }
    LC_Type *n = LC_CreateType(LC_TypeKind_Pointer);
    n->tbase   = type;
    LC_MapInsertU64(&L->type_map, key, n);
    return n;
}

LC_FUNCTION LC_Type *LC_CreateArrayType(LC_Type *type, int size) {
    uint64_t size_key = LC_HashBytes(&size, sizeof(size));
    uint64_t type_key = LC_HashBytes(type, sizeof(*type));
    uint64_t key      = LC_HashMix(size_key, type_key);
    LC_Type *entry    = (LC_Type *)LC_MapGetU64(&L->type_map, key);
    if (entry) {
        return entry;
    }
    LC_Type *n     = LC_CreateType(LC_TypeKind_Array);
    n->tbase       = type;
    n->tarray.size = size;
    n->size        = type->size * size;
    n->align       = type->align;
    LC_MapInsertU64(&L->type_map, key, n);
    return n;
}

LC_FUNCTION LC_Type *LC_CreateProcType(LC_TypeMemberList args, LC_Type *ret, bool has_vargs, bool has_vargs_any_promotion) {
    LC_ASSERT(NULL, ret);
    uint64_t key      = LC_HashBytes(ret, sizeof(*ret));
    key               = LC_HashMix(key, LC_HashBytes(&has_vargs, sizeof(has_vargs)));
    key               = LC_HashMix(key, LC_HashBytes(&has_vargs_any_promotion, sizeof(has_vargs_any_promotion)));
    int procarg_count = 0;
    LC_TypeFor(it, args.first) {
        key = LC_HashMix(LC_HashBytes(it->type, sizeof(it->type[0])), key);
        key = LC_HashMix(LC_HashBytes((char *)it->name, LC_StrLen((char *)it->name)), key);
        if (it->default_value_expr) {
            key = LC_HashMix(LC_HashBytes(&it->default_value_expr, sizeof(LC_AST *)), key);
        }
        procarg_count += 1;
    }
    LC_Type *n = (LC_Type *)LC_MapGetU64(&L->type_map, key);
    if (n) return n;

    n                            = LC_CreateType(LC_TypeKind_Proc);
    n->tproc.args                = args;
    n->tproc.vargs               = has_vargs;
    n->tproc.vargs_any_promotion = has_vargs_any_promotion;
    n->tproc.ret                 = ret;
    LC_MapInsertU64(&L->type_map, key, n);
    return n;
}

LC_FUNCTION LC_Type *LC_CreateIncompleteType(LC_Decl *decl) {
    LC_Type *n = LC_CreateType(LC_TypeKind_Incomplete);
    n->decl    = decl;
    return n;
}

LC_FUNCTION LC_Type *LC_CreateUntypedIntEx(LC_Type *base, LC_Decl *decl) {
    uint64_t hash_base   = LC_HashBytes(base, sizeof(*base));
    uint64_t untyped_int = LC_TypeKind_UntypedInt;
    uint64_t key         = LC_HashMix(hash_base, untyped_int);
    LC_Type *n           = (LC_Type *)LC_MapGetU64(&L->type_map, key);
    if (n) return n;

    n        = LC_CreateType(LC_TypeKind_UntypedInt);
    n->tbase = base;
    n->decl  = decl;
    return n;
}

LC_FUNCTION LC_Type *LC_CreateUntypedInt(LC_Type *base) {
    LC_Decl *decl = LC_CreateDecl(LC_DeclKind_Type, LC_ILit("UntypedInt"), &L->NullAST);
    LC_Type *n    = LC_CreateUntypedIntEx(base, decl);
    return n;
}

LC_FUNCTION LC_TypeMember *LC_AddTypeToList(LC_TypeMemberList *list, LC_Intern name, LC_Type *type, LC_AST *ast) {
    LC_TypeFor(it, list->first) {
        if (name == it->name) {
            return NULL;
        }
    }

    LC_TypeMember *r = LC_PushStruct(L->arena, LC_TypeMember);
    r->name          = name;
    r->type          = type;
    r->ast           = ast;
    LC_DLLAdd(list->first, list->last, r);
    list->count += 1;
    return r;
}

LC_FUNCTION LC_Type *LC_StripPointer(LC_Type *type) {
    if (type->kind == LC_TypeKind_Pointer) {
        return type->tbase;
    }
    return type;
}

LC_FUNCTION void LC_ReserveAST(LC_ASTArray *stack, int size) {
    if (size > stack->cap) {
        LC_AST **new_stack = LC_PushArray(stack->arena, LC_AST *, size);

        LC_MemoryCopy(new_stack, stack->data, stack->len * sizeof(LC_AST *));
        stack->data = new_stack;
        stack->cap  = size;
    }
}

LC_FUNCTION void LC_PushAST(LC_ASTArray *stack, LC_AST *ast) {
    LC_ASSERT(NULL, stack->len <= stack->cap);
    LC_ASSERT(NULL, stack->arena);
    if (stack->len == stack->cap) {
        int      new_cap   = stack->cap < 16 ? 16 : stack->cap * 2;
        LC_AST **new_stack = LC_PushArray(stack->arena, LC_AST *, new_cap);

        LC_MemoryCopy(new_stack, stack->data, stack->len * sizeof(LC_AST *));
        stack->data = new_stack;
        stack->cap  = new_cap;
    }
    stack->data[stack->len++] = ast;
}

LC_FUNCTION void LC_PopAST(LC_ASTArray *stack) {
    LC_ASSERT(NULL, stack->arena);
    LC_ASSERT(NULL, stack->len > 0);
    stack->len -= 1;
}

LC_FUNCTION LC_AST *LC_GetLastAST(LC_ASTArray *arr) {
    LC_ASSERT(NULL, arr->len > 0);
    LC_AST *result = arr->data[arr->len - 1];
    return result;
}

LC_FUNCTION void LC_WalkAST(LC_ASTWalker *ctx, LC_AST *n) {
    if (!ctx->depth_first) {
        ctx->proc(ctx, n);
    }

    if (ctx->dont_recurse) {
        ctx->dont_recurse = false;
        return;
    }

    LC_PushAST(&ctx->stack, n);
    switch (n->kind) {
    case LC_ASTKind_TypespecIdent:
    case LC_ASTKind_ExprIdent:
    case LC_ASTKind_ExprString:
    case LC_ASTKind_ExprInt:
    case LC_ASTKind_ExprFloat:
    case LC_ASTKind_GlobImport:
    case LC_ASTKind_ExprBool:
    case LC_ASTKind_StmtBreak:
    case LC_ASTKind_StmtContinue: break;

    case LC_ASTKind_Package: {
        LC_ASTFor(it, n->apackage.ffile) LC_WalkAST(ctx, it);

        ctx->inside_discarded += 1;
        if (ctx->visit_discarded) LC_ASTFor(it, n->apackage.fdiscarded) LC_WalkAST(ctx, it);
        ctx->inside_discarded -= 1;
    } break;

    case LC_ASTKind_File: {
        LC_ASTFor(it, n->afile.fimport) LC_WalkAST(ctx, it);
        LC_ASTFor(it, n->afile.fdecl) {
            if (ctx->visit_notes == false && it->kind == LC_ASTKind_DeclNote) continue;
            LC_WalkAST(ctx, it);
        }

        ctx->inside_discarded += 1;
        if (ctx->visit_discarded) LC_ASTFor(it, n->afile.fdiscarded) LC_WalkAST(ctx, it);
        ctx->inside_discarded -= 1;
    } break;

    case LC_ASTKind_DeclProc: {
        LC_WalkAST(ctx, n->dproc.type);
        if (n->dproc.body) LC_WalkAST(ctx, n->dproc.body);
    } break;
    case LC_ASTKind_NoteList: {
        if (ctx->visit_notes) LC_ASTFor(it, n->anote_list.first) LC_WalkAST(ctx, it);
    } break;
    case LC_ASTKind_TypespecProcArg: {
        LC_WalkAST(ctx, n->tproc_arg.type);
        if (n->tproc_arg.expr) LC_WalkAST(ctx, n->tproc_arg.expr);
    } break;
    case LC_ASTKind_TypespecAggMem: {
        LC_WalkAST(ctx, n->tproc_arg.type);
    } break;

    case LC_ASTKind_ExprNote: {
        ctx->inside_note += 1;
        if (ctx->visit_notes) LC_WalkAST(ctx, n->enote.expr);
        ctx->inside_note -= 1;
    } break;

    case LC_ASTKind_StmtSwitch: {
        LC_WalkAST(ctx, n->sswitch.expr);
        LC_ASTFor(it, n->sswitch.first) LC_WalkAST(ctx, it);
    } break;

    case LC_ASTKind_StmtSwitchCase: {
        LC_ASTFor(it, n->scase.first) LC_WalkAST(ctx, it);
        LC_WalkAST(ctx, n->scase.body);
    } break;
    case LC_ASTKind_StmtSwitchDefault: {
        LC_ASTFor(it, n->scase.first) LC_WalkAST(ctx, it);
        LC_WalkAST(ctx, n->scase.body);
    } break;

    case LC_ASTKind_StmtIf: {
        LC_WalkAST(ctx, n->sif.expr);
        LC_WalkAST(ctx, n->sif.body);
        LC_ASTFor(it, n->sif.first) LC_WalkAST(ctx, it);
    } break;
    case LC_ASTKind_StmtElse:
    case LC_ASTKind_StmtElseIf: {
        if (n->sif.expr) LC_WalkAST(ctx, n->sif.expr);
        LC_WalkAST(ctx, n->sif.body);
    } break;

    case LC_ASTKind_DeclNote: {
        ctx->inside_note += 1;
        if (ctx->visit_notes) LC_WalkAST(ctx, n->dnote.expr);
        ctx->inside_note -= 1;
    } break;
    case LC_ASTKind_DeclUnion:
    case LC_ASTKind_DeclStruct: {
        LC_ASTFor(it, n->dagg.first) LC_WalkAST(ctx, it);
    } break;

    case LC_ASTKind_DeclVar: {
        if (n->dvar.type) LC_WalkAST(ctx, n->dvar.type);
        if (n->dvar.expr) LC_WalkAST(ctx, n->dvar.expr);
    } break;
    case LC_ASTKind_DeclConst: {
        if (n->dconst.expr) LC_WalkAST(ctx, n->dconst.expr);
    } break;
    case LC_ASTKind_DeclTypedef: {
        LC_WalkAST(ctx, n->dtypedef.type);
    } break;
    case LC_ASTKind_TypespecField: {
        LC_WalkAST(ctx, n->efield.left);
    } break;

    case LC_ASTKind_TypespecPointer: {
        LC_WalkAST(ctx, n->tpointer.base);
    } break;
    case LC_ASTKind_TypespecArray: {
        LC_WalkAST(ctx, n->tarray.base);
        if (n->tarray.index) LC_WalkAST(ctx, n->tarray.index);
    } break;
    case LC_ASTKind_TypespecProc: {
        LC_ASTFor(it, n->tproc.first) LC_WalkAST(ctx, it);
        if (n->tproc.ret) LC_WalkAST(ctx, n->tproc.ret);
    } break;
    case LC_ASTKind_StmtBlock: {
        LC_ASTFor(it, n->sblock.first) LC_WalkAST(ctx, it);
        // hmm should we inline defers or maybe remove them from
        // the stmt list?
        // LC_ASTFor(it, n->sblock.first_defer) LC_WalkAST(ctx, it);
    } break;

    case LC_ASTKind_StmtNote: {
        ctx->inside_note += 1;
        if (ctx->visit_notes) LC_WalkAST(ctx, n->snote.expr);
        ctx->inside_note -= 1;
    } break;
    case LC_ASTKind_StmtReturn: {
        if (n->sreturn.expr) LC_WalkAST(ctx, n->sreturn.expr);
    } break;

    case LC_ASTKind_StmtDefer: {
        LC_WalkAST(ctx, n->sdefer.body);
    } break;
    case LC_ASTKind_StmtFor: {
        if (n->sfor.init) LC_WalkAST(ctx, n->sfor.init);
        if (n->sfor.cond) LC_WalkAST(ctx, n->sfor.cond);
        if (n->sfor.inc) LC_WalkAST(ctx, n->sfor.inc);
        LC_WalkAST(ctx, n->sfor.body);
    } break;

    case LC_ASTKind_StmtAssign: {
        LC_WalkAST(ctx, n->sassign.left);
        LC_WalkAST(ctx, n->sassign.right);
    } break;
    case LC_ASTKind_StmtExpr: {
        LC_WalkAST(ctx, n->sexpr.expr);
    } break;
    case LC_ASTKind_StmtVar: {
        if (n->svar.type) LC_WalkAST(ctx, n->svar.type);
        if (n->svar.expr) LC_WalkAST(ctx, n->svar.expr);
    } break;
    case LC_ASTKind_StmtConst: {
        LC_WalkAST(ctx, n->sconst.expr);
    } break;

    case LC_ASTKind_ExprType: {
        LC_WalkAST(ctx, n->etype.type);
    } break;
    case LC_ASTKind_ExprAddPtr:
    case LC_ASTKind_ExprBinary: {
        LC_WalkAST(ctx, n->ebinary.left);
        LC_WalkAST(ctx, n->ebinary.right);
    } break;
    case LC_ASTKind_ExprGetPointerOfValue:
    case LC_ASTKind_ExprGetValueOfPointer:
    case LC_ASTKind_ExprUnary: {
        LC_WalkAST(ctx, n->eunary.expr);
    } break;
    case LC_ASTKind_ExprCompoundItem:
    case LC_ASTKind_ExprCallItem: {
        if (n->ecompo_item.index) LC_WalkAST(ctx, n->ecompo_item.index);
        LC_WalkAST(ctx, n->ecompo_item.expr);
    } break;
    case LC_ASTKind_Note: {
        ctx->inside_note += 1;
        if (n->ecompo.name) LC_WalkAST(ctx, n->ecompo.name);
        LC_ASTFor(it, n->ecompo.first) LC_WalkAST(ctx, it);
        ctx->inside_note -= 1;
    } break;
    case LC_ASTKind_ExprBuiltin: {
        ctx->inside_builtin += 1;
        if (n->ecompo.name) LC_WalkAST(ctx, n->ecompo.name);
        LC_ASTFor(it, n->ecompo.first) LC_WalkAST(ctx, it);
        ctx->inside_builtin -= 1;
    } break;
    case LC_ASTKind_ExprCall:
    case LC_ASTKind_ExprCompound: {
        if (n->ecompo.name) LC_WalkAST(ctx, n->ecompo.name);
        LC_ASTFor(it, n->ecompo.first) LC_WalkAST(ctx, it);
    } break;
    case LC_ASTKind_ExprCast: {
        LC_WalkAST(ctx, n->ecast.type);
        LC_WalkAST(ctx, n->ecast.expr);
    } break;
    case LC_ASTKind_ExprField: {
        LC_WalkAST(ctx, n->efield.left);
    } break;
    case LC_ASTKind_ExprIndex: {
        LC_WalkAST(ctx, n->eindex.index);
        LC_WalkAST(ctx, n->eindex.base);
    } break;

    case LC_ASTKind_Ignore:
    case LC_ASTKind_Error:
    default: LC_ReportASTError(n, "internal compiler error: got invalid ast kind during ast walk: %s", LC_ASTKindToString(n->kind));
    }

    if (ctx->visit_notes && n->notes) {
        LC_WalkAST(ctx, n->notes);
    }
    LC_PopAST(&ctx->stack);

    if (ctx->depth_first) {
        ctx->proc(ctx, n);
    }
}

LC_FUNCTION LC_ASTWalker LC_GetDefaultWalker(LC_Arena *arena, LC_ASTWalkProc *proc) {
    LC_ASTWalker result = {0};
    result.stack.arena  = arena;
    result.proc         = proc;
    result.depth_first  = true;
    return result;
}

LC_FUNCTION void WalkAndFlattenAST(LC_ASTWalker *ctx, LC_AST *n) {
    LC_ASTArray *array = (LC_ASTArray *)ctx->user_data;
    LC_PushAST(array, n);
}

LC_FUNCTION LC_ASTArray LC_FlattenAST(LC_Arena *arena, LC_AST *n) {
    LC_ASTArray  array  = {arena};
    LC_ASTWalker walker = LC_GetDefaultWalker(arena, WalkAndFlattenAST);
    walker.user_data    = (void *)&array;
    LC_WalkAST(&walker, n);
    return array;
}

LC_FUNCTION void WalkToFindSizeofLike(LC_ASTWalker *w, LC_AST *n) {
    if (n->kind == LC_ASTKind_ExprBuiltin) {
        LC_ASSERT(n, n->ecompo.name->kind == LC_ASTKind_ExprIdent);
        if (n->ecompo.name->eident.name == L->isizeof || n->ecompo.name->eident.name == L->ialignof || n->ecompo.name->eident.name == L->ioffsetof) {
            ((bool *)w->user_data)[0] = true;
        }
    }
}

LC_FUNCTION bool LC_ContainsCBuiltin(LC_AST *n) {
    LC_TempArena checkpoint = LC_BeginTemp(L->arena);
    bool         found      = false;
    {
        LC_ASTWalker walker = LC_GetDefaultWalker(L->arena, WalkToFindSizeofLike);
        walker.depth_first  = false;
        walker.user_data    = (void *)&found;
        LC_WalkAST(&walker, n);
    }
    LC_EndTemp(checkpoint);
    return found;
}

LC_FUNCTION void SetASTPosOnAll_Walk(LC_ASTWalker *ctx, LC_AST *n) {
    n->pos = (LC_Token *)ctx->user_data;
}

LC_FUNCTION void LC_SetASTPosOnAll(LC_AST *n, LC_Token *pos) {
    LC_TempArena check  = LC_BeginTemp(L->arena);
    LC_ASTWalker walker = LC_GetDefaultWalker(L->arena, SetASTPosOnAll_Walk);
    walker.user_data    = (void *)pos;
    LC_WalkAST(&walker, n);
    LC_EndTemp(check);
}
LC_FUNCTION LC_AST *LC_CopyAST(LC_Arena *arena, LC_AST *n) {
    if (n == NULL) return NULL;

    LC_AST *result = LC_PushStruct(arena, LC_AST);
    result->kind   = n->kind;
    result->id     = ++L->ast_count;
    result->notes  = LC_CopyAST(arena, n->notes);
    result->pos    = n->pos;

    switch (n->kind) {
    case LC_ASTKind_File: {
        result->afile.x = n->afile.x;

        LC_ASTFor(it, n->afile.fimport) {
            LC_AST *it_copy = LC_CopyAST(arena, it);
            LC_DLLAdd(result->afile.fimport, result->afile.limport, it_copy);
        }
        LC_ASTFor(it, n->afile.fdecl) {
            LC_AST *it_copy = LC_CopyAST(arena, it);
            LC_DLLAdd(result->afile.fdecl, result->afile.ldecl, it_copy);
        }
        LC_ASTFor(it, n->afile.fdiscarded) {
            LC_AST *it_copy = LC_CopyAST(arena, it);
            LC_DLLAdd(result->afile.fdiscarded, result->afile.ldiscarded, it_copy);
        }

        result->afile.build_if = n->afile.build_if;
    } break;

    case LC_ASTKind_DeclProc: {
        result->dbase.name = n->dbase.name;
        result->dproc.body = LC_CopyAST(arena, n->dproc.body);
        result->dproc.type = LC_CopyAST(arena, n->dproc.type);
    } break;

    case LC_ASTKind_NoteList: {
        LC_ASTFor(it, n->anote_list.first) {
            LC_AST *it_copy = LC_CopyAST(arena, it);
            LC_DLLAdd(result->anote_list.first, result->anote_list.last, it_copy);
        }
    } break;

    case LC_ASTKind_TypespecAggMem:
    case LC_ASTKind_TypespecProcArg: {
        result->tproc_arg.name = n->tproc_arg.name;
        result->tproc_arg.type = LC_CopyAST(arena, n->tproc_arg.type);
        result->tproc_arg.expr = LC_CopyAST(arena, n->tproc_arg.expr);
    } break;

    case LC_ASTKind_ExprNote: {
        result->enote.expr = LC_CopyAST(arena, n->enote.expr);
    } break;

    case LC_ASTKind_StmtSwitch: {
        LC_ASTFor(it, n->sswitch.first) {
            LC_AST *it_copy = LC_CopyAST(arena, it);
            LC_DLLAdd(result->sswitch.first, result->sswitch.last, it_copy);
        }
        result->sswitch.total_switch_case_count = n->sswitch.total_switch_case_count;
        result->sswitch.expr                    = LC_CopyAST(arena, n->sswitch.expr);
    } break;

    case LC_ASTKind_StmtSwitchCase: {
        LC_ASTFor(it, n->scase.first) {
            LC_AST *it_copy = LC_CopyAST(arena, it);
            LC_DLLAdd(result->scase.first, result->scase.last, it_copy);
        }
        result->scase.body = LC_CopyAST(arena, n->scase.body);
    } break;

    case LC_ASTKind_StmtSwitchDefault: {
        LC_ASTFor(it, n->scase.first) {
            LC_AST *it_copy = LC_CopyAST(arena, it);
            LC_DLLAdd(result->scase.first, result->scase.last, it_copy);
        }
        result->scase.body = LC_CopyAST(arena, n->scase.body);
    } break;

    case LC_ASTKind_StmtIf: {
        LC_ASTFor(it, n->sif.first) {
            LC_AST *it_copy = LC_CopyAST(arena, it);
            LC_DLLAdd(result->sswitch.first, result->sswitch.last, it_copy);
        }
        result->sif.expr = LC_CopyAST(arena, n->sif.expr);
        result->sif.body = LC_CopyAST(arena, n->sif.body);
    } break;

    case LC_ASTKind_StmtElse:
    case LC_ASTKind_StmtElseIf: {
        result->sif.expr = LC_CopyAST(arena, n->sif.expr);
        result->sif.body = LC_CopyAST(arena, n->sif.body);
    } break;

    case LC_ASTKind_GlobImport: {
        result->gimport.name = n->gimport.name;
        result->gimport.path = n->gimport.path;
    } break;

    case LC_ASTKind_DeclNote: {
        result->dnote.expr = LC_CopyAST(arena, n->dnote.expr);
    } break;

    case LC_ASTKind_DeclUnion:
    case LC_ASTKind_DeclStruct: {
        result->dbase.name = n->dbase.name;
        LC_ASTFor(it, n->dagg.first) {
            LC_AST *it_copy = LC_CopyAST(arena, it);
            LC_DLLAdd(result->dagg.first, result->dagg.last, it_copy);
        }
    } break;

    case LC_ASTKind_DeclVar: {
        result->dbase.name = n->dbase.name;
        result->dvar.type  = LC_CopyAST(arena, n->dvar.type);
        result->dvar.expr  = LC_CopyAST(arena, n->dvar.expr);
    } break;

    case LC_ASTKind_DeclConst: {
        result->dbase.name  = n->dbase.name;
        result->dconst.expr = LC_CopyAST(arena, n->dconst.expr);
    } break;

    case LC_ASTKind_DeclTypedef: {
        result->dbase.name    = n->dbase.name;
        result->dtypedef.type = LC_CopyAST(arena, n->dtypedef.type);
    } break;

    case LC_ASTKind_ExprField:
    case LC_ASTKind_TypespecField: {
        result->efield.left  = LC_CopyAST(arena, n->efield.left);
        result->efield.right = n->efield.right;
    } break;

    case LC_ASTKind_TypespecIdent: {
        result->eident.name = n->eident.name;
    } break;

    case LC_ASTKind_TypespecPointer: {
        result->tpointer.base = LC_CopyAST(arena, n->tpointer.base);
    } break;

    case LC_ASTKind_TypespecArray: {
        result->tarray.base  = LC_CopyAST(arena, n->tarray.base);
        result->tarray.index = LC_CopyAST(arena, n->tarray.index);
    } break;

    case LC_ASTKind_TypespecProc: {
        LC_ASTFor(it, n->tproc.first) {
            LC_AST *it_copy = LC_CopyAST(arena, it);
            LC_DLLAdd(result->tproc.first, result->tproc.last, it_copy);
        }
        result->tproc.ret   = LC_CopyAST(arena, n->tproc.ret);
        result->tproc.vargs = n->tproc.vargs;
    } break;

    case LC_ASTKind_StmtBlock: {
        LC_ASTFor(it, n->sblock.first) {
            LC_AST *it_copy = LC_CopyAST(arena, it);
            LC_DLLAdd(result->sblock.first, result->sblock.last, it_copy);

            if (it_copy->kind == LC_ASTKind_StmtDefer) {
                LC_SLLStackAddMod(result->sblock.first_defer, it_copy, sdefer.next);
            }
        }
        if (n->sblock.first_defer) {
            LC_ASSERT(result, result->sblock.first_defer);
        }
        result->sblock.kind = n->sblock.kind;
        result->sblock.name = n->sblock.name;
    } break;

    case LC_ASTKind_StmtNote: {
        result->snote.expr = LC_CopyAST(arena, n->snote.expr);
    } break;

    case LC_ASTKind_StmtReturn: {
        result->sreturn.expr = LC_CopyAST(arena, n->sreturn.expr);
    } break;

    case LC_ASTKind_StmtBreak: {
        result->sbreak.name = n->sbreak.name;
    } break;

    case LC_ASTKind_StmtContinue: {
        result->scontinue.name = n->scontinue.name;
    } break;

    case LC_ASTKind_StmtDefer: {
        result->sdefer.body = LC_CopyAST(arena, n->sdefer.body);
    } break;

    case LC_ASTKind_StmtFor: {
        result->sfor.init = LC_CopyAST(arena, n->sfor.init);
        result->sfor.cond = LC_CopyAST(arena, n->sfor.cond);
        result->sfor.inc  = LC_CopyAST(arena, n->sfor.inc);
        result->sfor.body = LC_CopyAST(arena, n->sfor.body);
    } break;

    case LC_ASTKind_StmtAssign: {
        result->sassign.op    = n->sassign.op;
        result->sassign.left  = LC_CopyAST(arena, n->sassign.left);
        result->sassign.right = LC_CopyAST(arena, n->sassign.right);
    } break;

    case LC_ASTKind_StmtExpr: {
        result->sexpr.expr = LC_CopyAST(arena, n->sexpr.expr);
    } break;

    case LC_ASTKind_StmtVar: {
        result->svar.type = LC_CopyAST(arena, n->svar.type);
        result->svar.expr = LC_CopyAST(arena, n->svar.expr);
        result->svar.name = n->svar.name;
    } break;

    case LC_ASTKind_StmtConst: {
        result->sconst.expr = LC_CopyAST(arena, n->sconst.expr);
        result->sconst.name = n->sconst.name;
    } break;

    case LC_ASTKind_ExprIdent: {
        result->eident.name = n->eident.name;
    } break;

    case LC_ASTKind_ExprBool:
    case LC_ASTKind_ExprFloat:
    case LC_ASTKind_ExprInt:
    case LC_ASTKind_ExprString: {
        result->eatom = n->eatom;
    } break;

    case LC_ASTKind_ExprType: {
        result->etype.type = LC_CopyAST(arena, n->etype.type);
    } break;

    case LC_ASTKind_ExprAddPtr:
    case LC_ASTKind_ExprBinary: {
        result->ebinary.op    = n->ebinary.op;
        result->ebinary.left  = LC_CopyAST(arena, n->ebinary.left);
        result->ebinary.right = LC_CopyAST(arena, n->ebinary.right);
    } break;

    case LC_ASTKind_ExprGetPointerOfValue:
    case LC_ASTKind_ExprGetValueOfPointer:
    case LC_ASTKind_ExprUnary: {
        result->eunary.op   = n->eunary.op;
        result->eunary.expr = LC_CopyAST(arena, n->eunary.expr);
    } break;

    case LC_ASTKind_ExprCompoundItem:
    case LC_ASTKind_ExprCallItem: {
        result->ecompo_item.name  = n->ecompo_item.name;
        result->ecompo_item.index = LC_CopyAST(arena, n->ecompo_item.index);
        result->ecompo_item.expr  = LC_CopyAST(arena, n->ecompo_item.expr);
    } break;

    case LC_ASTKind_ExprBuiltin:
    case LC_ASTKind_Note:
    case LC_ASTKind_ExprCall:
    case LC_ASTKind_ExprCompound: {
        LC_ASTFor(it, n->ecompo.first) {
            LC_AST *it_copy = LC_CopyAST(arena, it);
            LC_DLLAdd(result->ecompo.first, result->ecompo.last, it_copy);
        }

        result->ecompo.size = n->ecompo.size;
        result->ecompo.name = LC_CopyAST(arena, n->ecompo.name);
    } break;

    case LC_ASTKind_ExprCast: {
        result->ecast.type = LC_CopyAST(arena, n->ecast.type);
        result->ecast.expr = LC_CopyAST(arena, n->ecast.expr);
    } break;

    case LC_ASTKind_ExprIndex: {
        result->eindex.index = LC_CopyAST(arena, n->eindex.index);
        result->eindex.base  = LC_CopyAST(arena, n->eindex.base);
    } break;

    default: LC_ReportASTError(n, "internal compiler error: failed to LC_CopyAST, got invalid ast kind: %s", LC_ASTKindToString(n->kind));
    }

    return result;
}

// clang-format off
#define LC_PUSH_COMP_ARRAY_SIZE(SIZE) int PREV_SIZE = L->resolver.compo_context_array_size; L->resolver.compo_context_array_size = SIZE;
#define LC_POP_COMP_ARRAY_SIZE() L->resolver.compo_context_array_size = PREV_SIZE
#define LC_PUSH_SCOPE(SCOPE) DeclScope *PREV_SCOPE = L->resolver.active_scope; L->resolver.active_scope = SCOPE
#define LC_POP_SCOPE() L->resolver.active_scope = PREV_SCOPE
#define LC_PUSH_LOCAL_SCOPE() int LOCAL_LEN = L->resolver.locals.len
#define LC_POP_LOCAL_SCOPE() L->resolver.locals.len = LOCAL_LEN
#define LC_PUSH_PACKAGE(PKG) LC_AST *PREV_PKG = L->resolver.package; L->resolver.package = PKG; LC_PUSH_SCOPE(PKG->apackage.scope)
#define LC_POP_PACKAGE() L->resolver.package = PREV_PKG; LC_POP_SCOPE()
#define LC_PROP_ERROR(OP, n, ...)  OP = __VA_ARGS__; if (LC_IsError(OP)) { n->kind = LC_ASTKind_Error; return OP; }
#define LC_DECL_PROP_ERROR(OP, ...) OP = __VA_ARGS__; if (LC_IsError(OP)) { LC_MarkDeclError(decl); return OP; }

#define LC_IF(COND, N, ...) if (COND) { LC_Operand R_ = LC_ReportASTError(N, __VA_ARGS__); N->kind = LC_ASTKind_Error; return R_; }
#define LC_DECL_IF(COND, ...) if (COND) { LC_MarkDeclError(decl); return LC_ReportASTError(__VA_ARGS__); }
#define LC_TYPE_IF(COND, ...) if (COND) { LC_MarkDeclError(decl); type->kind = LC_TypeKind_Error; return LC_ReportASTError(__VA_ARGS__); }
// clang-format on

LC_FUNCTION void LC_AddDecl(LC_DeclStack *scope, LC_Decl *decl) {
    if (scope->len + 1 > scope->cap) {
        LC_ASSERT(NULL, scope->cap);
        int       new_cap   = scope->cap * 2;
        LC_Decl **new_stack = LC_PushArray(L->arena, LC_Decl *, new_cap);
        LC_MemoryCopy(new_stack, scope->stack, scope->len * sizeof(LC_Decl *));
        scope->stack = new_stack;
        scope->cap   = new_cap;
    }
    scope->stack[scope->len++] = decl;
}

LC_FUNCTION void LC_InitDeclStack(LC_DeclStack *stack, int size) {
    stack->stack = LC_PushArray(L->arena, LC_Decl *, size);
    stack->cap   = size;
}

LC_FUNCTION LC_DeclStack *LC_CreateDeclStack(int size) {
    LC_DeclStack *stack = LC_PushStruct(L->arena, LC_DeclStack);
    LC_InitDeclStack(stack, size);
    return stack;
}

LC_FUNCTION LC_Decl *LC_FindDeclOnStack(LC_DeclStack *scp, LC_Intern name) {
    for (int i = 0; i < scp->len; i += 1) {
        LC_Decl *it = scp->stack[i];
        if (it->name == name) {
            return it;
        }
    }
    return NULL;
}

LC_FUNCTION void LC_MarkDeclError(LC_Decl *decl) {
    if (decl) {
        decl->kind  = LC_DeclKind_Error;
        decl->state = LC_DeclState_Error;
        if (decl->ast) decl->ast->kind = LC_ASTKind_Error;
    }
}

LC_FUNCTION LC_Decl *LC_CreateDecl(LC_DeclKind kind, LC_Intern name, LC_AST *n) {
    LC_Decl *decl = LC_PushStruct(L->decl_arena, LC_Decl);
    L->decl_count += 1;

    decl->name    = name;
    decl->kind    = kind;
    decl->ast     = n;
    decl->package = L->resolver.package;
    LC_ASSERT(n, decl->package);

    LC_AST *note = LC_HasNote(n, L->iforeign);
    if (note) {
        decl->is_foreign = true;
        if (note->anote.first) {
            if (note->anote.size != 1) LC_ReportASTError(note, "invalid format of @foreign(...), more then 1 argument");
            LC_AST *expr = note->anote.first->ecompo_item.expr;
            if (expr->kind == LC_ASTKind_ExprIdent) decl->foreign_name = expr->eident.name;
            if (expr->kind != LC_ASTKind_ExprIdent) LC_ReportASTError(note, "invalid format of @foreign(...), expected identifier");
        }
    }
    if (!decl->foreign_name) decl->foreign_name = decl->name;
    return decl;
}

LC_FUNCTION LC_Operand LC_ThereIsNoDecl(DeclScope *scp, LC_Decl *decl, bool check_locals) {
    LC_Decl *r = (LC_Decl *)LC_MapGetU64(scp, decl->name);
    if (check_locals && !r) {
        r = LC_FindDeclOnStack(&L->resolver.locals, decl->name);
    }
    if (r) {
        LC_MarkDeclError(r);
        LC_MarkDeclError(decl);
        return LC_ReportASTErrorEx(decl->ast, r->ast, "there are 2 decls with the same name '%s'", decl->name);
    }
    return LC_OPNull;
}

LC_FUNCTION LC_Operand LC_AddDeclToScope(DeclScope *scp, LC_Decl *decl) {
    LC_Operand LC_DECL_PROP_ERROR(op, LC_ThereIsNoDecl(scp, decl, false));
    LC_MapInsertU64(scp, decl->name, decl);
    return LC_OPDecl(decl);
}

LC_FUNCTION DeclScope *LC_CreateScope(int size) {
    DeclScope *scope = LC_PushStruct(L->arena, DeclScope);
    scope->arena     = L->arena;
    LC_MapReserve(scope, size);
    return scope;
}

LC_FUNCTION LC_Decl *LC_FindDeclInScope(DeclScope *scope, LC_Intern name) {
    LC_Decl *decl = (LC_Decl *)LC_MapGetU64(scope, name);
    return decl;
}

LC_FUNCTION LC_Decl *LC_GetLocalOrGlobalDecl(LC_Intern name) {
    LC_Decl *decl = LC_FindDeclInScope(L->resolver.active_scope, name);
    if (!decl && L->resolver.package->apackage.scope == L->resolver.active_scope) {
        decl = LC_FindDeclOnStack(&L->resolver.locals, name);
    }
    return decl;
}

LC_FUNCTION LC_Operand LC_PutGlobalDecl(LC_Decl *decl) {
    LC_Operand LC_DECL_PROP_ERROR(op, LC_AddDeclToScope(L->resolver.package->apackage.scope, decl));

    // :Mangle global scope name
    if (!decl->is_foreign && decl->package != L->builtin_package) {
        bool mangle = true;
        if (LC_HasNote(decl->ast, L->idont_mangle)) mangle = false;
        if (LC_HasNote(decl->ast, L->iapi)) mangle = false;
        if (decl->name == L->imain) {
            if (L->first_package) {
                if (L->first_package == decl->package->apackage.name) {
                    mangle = false;
                }
            } else mangle = false;
        }
        if (mangle) {
            LC_String name     = LC_Format(L->arena, "lc_%s_%s", (char *)decl->package->apackage.name, (char *)decl->name);
            decl->foreign_name = LC_InternStrLen(name.str, (int)name.len);
        }
    }

    LC_Decl *conflict = (LC_Decl *)LC_MapGetU64(&L->foreign_names, decl->foreign_name);
    if (conflict && !decl->is_foreign) {
        LC_ReportASTErrorEx(decl->ast, conflict->ast, "found two global declarations with the same foreign name: %s", decl->foreign_name);
    } else {
        LC_MapInsertU64(&L->foreign_names, decl->foreign_name, decl);
    }

    return op;
}

LC_FUNCTION LC_Operand LC_CreateLocalDecl(LC_DeclKind kind, LC_Intern name, LC_AST *ast) {
    LC_Decl *decl = LC_CreateDecl(kind, name, ast);
    decl->state   = LC_DeclState_Resolving;
    LC_Operand LC_DECL_PROP_ERROR(operr0, LC_ThereIsNoDecl(L->resolver.package->apackage.scope, decl, true));
    LC_AddDecl(&L->resolver.locals, decl);
    return LC_OPDecl(decl);
}

LC_FUNCTION LC_Decl *LC_AddConstIntDecl(char *key, int64_t value) {
    LC_Intern intern = LC_ILit(key);
    LC_Decl  *decl   = LC_CreateDecl(LC_DeclKind_Const, intern, &L->NullAST);
    decl->state      = LC_DeclState_Resolved;
    decl->type       = L->tuntypedint;
    LC_Bigint_init_signed(&decl->v.i, value);
    LC_AddDeclToScope(L->resolver.package->apackage.scope, decl);
    return decl;
}

LC_FUNCTION LC_Decl *LC_GetBuiltin(LC_Intern name) {
    LC_Decl *decl = (LC_Decl *)LC_MapGetU64(L->builtin_package->apackage.scope, name);
    return decl;
}

LC_FUNCTION void LC_AddBuiltinConstInt(char *key, int64_t value) {
    LC_PUSH_PACKAGE(L->builtin_package);
    LC_AddConstIntDecl(key, value);
    LC_POP_PACKAGE();
}

LC_FUNCTION LC_AST *LC_HasNote(LC_AST *ast, LC_Intern i) {
    if (ast && ast->notes) {
        LC_ASTFor(it, ast->notes->anote_list.first) {
            LC_ASSERT(it, "internal compiler error: note is not an identifier");
            if (it->anote.name->eident.name == i) {
                return it;
            }
        }
    }
    return NULL;
}

LC_FUNCTION void LC_RegisterDeclsFromFile(LC_AST *file) {
    LC_ASTFor(n, file->afile.fdecl) {
        if (n->dbase.resolved_decl) continue;
        if (n->kind == LC_ASTKind_DeclNote) continue;
        LC_Decl *decl = LC_CreateDecl(LC_DeclKind_Type, n->dbase.name, n);
        switch (n->kind) {
        case LC_ASTKind_DeclStruct:
        case LC_ASTKind_DeclUnion:
            decl->type  = LC_CreateIncompleteType(decl);
            decl->state = LC_DeclState_Resolved;
            decl->kind  = LC_DeclKind_Type;
            break;
        case LC_ASTKind_DeclTypedef: decl->kind = LC_DeclKind_Type; break;
        case LC_ASTKind_DeclProc: decl->kind = LC_DeclKind_Proc; break;
        case LC_ASTKind_DeclConst: decl->kind = LC_DeclKind_Const; break;
        case LC_ASTKind_DeclVar: decl->kind = LC_DeclKind_Var; break;
        default: LC_ReportASTError(n, "internal compiler error: got unhandled ast declaration kind in %s", __FUNCTION__);
        }
        LC_PutGlobalDecl(decl);
        n->dbase.resolved_decl = decl;
    }
}

LC_FUNCTION void LC_ResolveDeclsFromFile(LC_AST *file) {
    LC_ASTFor(n, file->afile.fdecl) {
        if (n->kind == LC_ASTKind_DeclNote) {
            LC_ResolveNote(n, false);
        } else {
            LC_ResolveName(n, n->dbase.name);
        }
    }
}

LC_FUNCTION void LC_PackageDecls(LC_AST *package) {
    LC_PUSH_PACKAGE(package);

    // Register top level declarations
    LC_ASTFor(file, package->apackage.ffile) {
        LC_ASTFor(import, file->afile.fimport) {
            if (import->gimport.resolved == false) LC_ReportASTError(import, "internal compiler error: unresolved import got into typechecking stage");
        }
        LC_RegisterDeclsFromFile(file);
    }

    // Resolve declarations by name
    LC_ASTFor(file, package->apackage.ffile) {
        LC_ResolveDeclsFromFile(file);
    }

    LC_POP_PACKAGE();
}

LC_FUNCTION void LC_ResolveProcBodies(LC_AST *package) {
    LC_PUSH_PACKAGE(package);

    LC_ASTFor(file, package->apackage.ffile) {
        LC_ASTFor(n, file->afile.fdecl) {
            if (n->kind == LC_ASTKind_DeclNote) continue;

            LC_Decl *decl = n->dbase.resolved_decl;
            if (decl->kind == LC_DeclKind_Proc) {
                LC_Operand op = LC_ResolveProcBody(decl);
                if (LC_IsError(op)) LC_MarkDeclError(decl);
            }
        }
    }

    LC_POP_PACKAGE();
}

LC_FUNCTION void LC_ResolveIncompleteTypes(LC_AST *package) {
    LC_PUSH_PACKAGE(package);

    LC_ASTFor(file, package->apackage.ffile) {
        LC_ASTFor(n, file->afile.fdecl) {
            if (n->kind == LC_ASTKind_DeclNote) continue;

            LC_Decl *decl = n->dbase.resolved_decl;
            if (decl->kind == LC_DeclKind_Type) {
                LC_ResolveTypeAggregate(decl->ast, decl->type);
            }
        }
    }

    LC_POP_PACKAGE();
}

LC_FUNCTION LC_Operand LC_ResolveNote(LC_AST *n, bool is_decl) {
    LC_AST *note = n->dnote.expr;
    if (n->kind == LC_ASTKind_ExprNote) note = n->enote.expr;
    else if (n->kind == LC_ASTKind_StmtNote) note = n->snote.expr;
    else {
        LC_ASSERT(n, n->kind == LC_ASTKind_DeclNote);
        if (n->dnote.processed) return LC_OPNull;
    }

    if (note->ecompo.name->eident.name == L->istatic_assert) {
        LC_IF(is_decl, note, "#static_assert cant be used as variable initializer");
        LC_Operand LC_PROP_ERROR(op, n, LC_ExpectBuiltinWithOneArg(note));
        LC_IF(!LC_IsUTConst(op) || !LC_IsUTInt(op.type), n, "static assert requires constant untyped integer value");
        int val = (int)LC_Bigint_as_signed(&op.val.i);
        LC_IF(!val, note, "#static_assert failed !");
        n->dnote.processed = true;
    }

    return LC_OPNull;
}

void SetConstVal(LC_AST *n, LC_TypeAndVal val) {
    LC_ASSERT(n, LC_IsUntyped(val.type));
    n->const_val = val;
}

LC_FUNCTION LC_Operand LC_ResolveProcBody(LC_Decl *decl) {
    if (decl->state == LC_DeclState_Error) return LC_OPError();
    if (decl->state == LC_DeclState_ResolvedBody) return LC_OPNull;
    LC_ASSERT(decl->ast, decl->state == LC_DeclState_Resolved);

    LC_AST *n = decl->ast;
    if (n->dproc.body == NULL) return LC_OPNull;
    L->resolver.locals.len = 0;
    LC_ASTFor(it, n->dproc.type->tproc.first) {
        if (it->kind == LC_ASTKind_Error) {
            LC_MarkDeclError(decl);
            return LC_OPError();
        }
        LC_ASSERT(it, it->type);
        LC_Operand LC_DECL_PROP_ERROR(op, LC_CreateLocalDecl(LC_DeclKind_Var, it->tproc_arg.name, it));
        op.decl->type               = it->type;
        op.decl->state              = LC_DeclState_Resolved;
        it->tproc_arg.resolved_decl = op.decl;
    }

    int errors_before = L->errors;
    LC_ASSERT(n, decl->type->tproc.ret);
    L->resolver.expected_ret_type = decl->type->tproc.ret;
    LC_Operand LC_DECL_PROP_ERROR(op, LC_ResolveStmtBlock(n->dproc.body));
    L->resolver.locals.len = 0;

    if (errors_before == L->errors && decl->type->tproc.ret != L->tvoid && !(op.flags & LC_OPF_Returned)) {
        LC_ReportASTError(n, "you can get through this procedure without hitting a return stmt, add a return to cover all control paths");
    }
    decl->state = LC_DeclState_ResolvedBody;
    if (L->on_proc_body_resolved) L->on_proc_body_resolved(decl);
    return LC_OPNull;
}

LC_FUNCTION LC_ResolvedCompoItem *LC_AddResolvedCallItem(LC_ResolvedCompo *list, LC_TypeMember *t, LC_AST *comp, LC_AST *expr) {
    LC_AST *duplicate1 = (LC_AST *)LC_MapGetP(&L->resolver.duplicate_map, list);
    if (t && !duplicate1) {
        for (LC_ResolvedCompoItem *it = list->first; it; it = it->next) {
            if (t == it->t) {
                LC_MapInsertP(&L->resolver.duplicate_map, list, comp);
                LC_MapInsertP(&L->resolver.duplicate_map, comp, it->comp); // duplicate2
                break;
            }
        }
    }
    LC_ResolvedCompoItem *match = LC_PushStruct(L->arena, LC_ResolvedCompoItem);
    list->count += 1;
    match->t    = t;
    match->expr = expr;
    match->comp = comp;
    LC_AddSLL(list->first, list->last, match);
    return match;
}

LC_FUNCTION LC_Operand LC_ResolveCompoCall(LC_AST *n, LC_Type *type) {
    LC_ASSERT(n, type->kind == LC_TypeKind_Proc);
    LC_IF(type->tproc.vargs && type->tagg.mems.count > n->ecompo.size, n, "calling procedure with invalid argument count, expected at least %d args, got %d, the procedure type is: %s", type->tagg.mems.count, n->ecompo.size, LC_GenLCType(type));

    bool named_field_appeared = false;
    LC_ASTFor(it, n->ecompo.first) {
        LC_IF(type->tproc.vargs && it->ecompo_item.name, it, "variadic procedures cannot have named arguments");
        LC_IF(it->ecompo_item.index, it, "index inside a call is not allowed");
        LC_IF(named_field_appeared && it->ecompo_item.name == 0, it, "mixing named and positional arguments is illegal");
        if (it->ecompo_item.name) named_field_appeared = true;
    }

    LC_ResolvedCompo *matches = LC_PushStruct(L->arena, LC_ResolvedCompo);
    LC_TypeMember    *type_it = type->tproc.args.first;
    LC_AST           *npos_it = n->ecompo.first;

    // greedy match unnamed arguments
    for (; type_it; type_it = type_it->next, npos_it = npos_it->next) {
        if (npos_it == NULL || npos_it->ecompo_item.name) break;
        LC_AddResolvedCallItem(matches, type_it, npos_it, npos_it->ecompo_item.expr);
    }

    // greedy match variadic arguments
    if (type->tproc.vargs) {
        for (; npos_it; npos_it = npos_it->next) {
            LC_ResolvedCompoItem *m = LC_AddResolvedCallItem(matches, NULL, npos_it, npos_it->ecompo_item.expr);
            m->varg                 = true;
        }
    }

    // for every required proc type argument we seek a named argument
    // in either default proc values or passed in call arguments
    for (; type_it; type_it = type_it->next) {
        LC_ASTFor(n_it, npos_it) {
            if (type_it->name == n_it->ecompo_item.name) {
                LC_AddResolvedCallItem(matches, type_it, n_it, n_it->ecompo_item.expr);
                goto end_of_outer_loop;
            }
        }

        if (type_it->default_value_expr) {
            LC_ResolvedCompoItem *m = LC_AddResolvedCallItem(matches, type_it, NULL, type_it->default_value_expr);
            m->defaultarg           = true;
        }
    end_of_outer_loop:;
    }

    // make sure we matched every item in call
    LC_ASTFor(n_it, n->ecompo.first) {
        LC_AST *expr     = n_it->ecompo_item.expr;
        bool    included = false;
        for (LC_ResolvedCompoItem *it = matches->first; it; it = it->next) {
            if (it->expr == expr) {
                included = true;
                break;
            }
        }

        LC_IF(!included, expr, "unknown argument to a procedure call, couldn't match it with any of the declared arguments, the procedure type is: %s", LC_GenLCType(type));
    }

    LC_IF(!type->tproc.vargs && matches->count != type->tproc.args.count, n, "invalid argument count passed in to procedure call, expected: %d, matched: %d, the procedure type is: %s", type->tproc.args.count, matches->count, LC_GenLCType(type));

    // error on duplicates
    LC_AST *duplicate1 = (LC_AST *)LC_MapGetP(&L->resolver.duplicate_map, matches);
    LC_AST *duplicate2 = duplicate1 ? (LC_AST *)LC_MapGetP(&L->resolver.duplicate_map, duplicate1) : NULL;
    LC_MapClear(&L->resolver.duplicate_map);
    if (duplicate1) {
        LC_Operand err = LC_ReportASTErrorEx(duplicate1, duplicate2, "two call items match the same procedure argument");
        n->kind        = LC_ASTKind_Error;
        return err;
    }

    // resolve
    for (LC_ResolvedCompoItem *it = matches->first; it; it = it->next) {
        if (it->varg) {
            if (type->tproc.vargs_any_promotion) {
                LC_Operand LC_PROP_ERROR(opexpr, it->expr, LC_ResolveExprAndPushCompoContext(it->expr, L->tany));
                LC_TryTyping(it->expr, LC_OPType(L->tany));
            } else {
                LC_Operand LC_PROP_ERROR(opexpr, it->expr, LC_ResolveExpr(it->expr));
                LC_Operand LC_PROP_ERROR(op, it->expr, LC_ResolveTypeVargs(it->expr, opexpr));
                LC_TryTyping(it->expr, op);
            }
            continue;
        }
        if (it->defaultarg) {
            continue;
        }
        LC_Operand LC_PROP_ERROR(opexpr, it->expr, LC_ResolveExprAndPushCompoContext(it->expr, it->t->type));
        LC_Operand LC_PROP_ERROR(op, it->expr, LC_ResolveTypeVarDecl(it->expr, LC_OPType(it->t->type), opexpr));
        LC_TryTyping(it->expr, op);
    }

    n->ecompo.resolved_items = matches;
    LC_Operand result        = LC_OPLValueAndType(type->tproc.ret);
    return result;
}

LC_FUNCTION LC_Operand LC_ResolveCompoAggregate(LC_AST *n, LC_Type *type) {
    LC_ASSERT(n, type->kind == LC_TypeKind_Union || type->kind == LC_TypeKind_Struct);
    LC_IF(n->ecompo.size > 1 && type->kind == LC_TypeKind_Union, n, "too many union initializers, expected 1 or 0 got %d", n->ecompo.size);
    LC_IF(n->ecompo.size > type->tagg.mems.count, n, "too many struct initializers, expected less then %d got instead %d", type->tagg.mems.count, n->ecompo.size);

    bool named_field_appeared = false;
    LC_ASTFor(it, n->ecompo.first) {
        LC_IF(type->kind == LC_TypeKind_Union && it->ecompo_item.name == 0, it, "unions can only be initialized using named arguments");
        LC_IF(named_field_appeared && it->ecompo_item.name == 0, it, "mixing named and positional arguments is illegal");
        LC_IF(it->ecompo_item.index, it, "index specifier in non array compound is illegal");
        if (it->ecompo_item.name) named_field_appeared = true;
    }

    LC_ResolvedCompo *matches = LC_PushStruct(L->arena, LC_ResolvedCompo);
    LC_TypeMember    *type_it = type->tagg.mems.first;
    LC_AST           *npos_it = n->ecompo.first;

    // greedy match unnamed arguments
    for (; type_it; type_it = type_it->next, npos_it = npos_it->next) {
        if (npos_it == NULL || npos_it->ecompo_item.name) break;
        LC_AddResolvedCallItem(matches, type_it, npos_it, npos_it->ecompo_item.expr);
    }

    // match named arguments
    for (; npos_it; npos_it = npos_it->next) {
        bool found = false;
        LC_TypeFor(type_it, type->tagg.mems.first) {
            if (type_it->name == npos_it->ecompo_item.name) {
                LC_AddResolvedCallItem(matches, type_it, npos_it, npos_it->ecompo_item.expr);
                found = true;
                break;
            }
        }

        LC_IF(!found, npos_it, "no matching declaration with name '%s' in type '%s'", npos_it->ecompo_item.name, LC_GenLCType(type));
    }

    // error on duplicates
    LC_AST *duplicate1 = (LC_AST *)LC_MapGetP(&L->resolver.duplicate_map, matches);
    LC_AST *duplicate2 = duplicate1 ? (LC_AST *)LC_MapGetP(&L->resolver.duplicate_map, duplicate1) : NULL;
    if (duplicate1) {
        LC_Operand err = LC_ReportASTErrorEx(duplicate1, duplicate2, "two compound items match the same struct variable");
        n->kind        = LC_ASTKind_Error;
        return err;
    }

    // resolve
    LC_Operand result = LC_OPLValueAndType(type);
    result.flags |= LC_OPF_Const;
    for (LC_ResolvedCompoItem *it = matches->first; it; it = it->next) {
        LC_Operand LC_PROP_ERROR(opexpr, it->expr, LC_ResolveExprAndPushCompoContext(it->expr, it->t->type));
        LC_Operand LC_PROP_ERROR(op, it->expr, LC_ResolveTypeVarDecl(it->expr, LC_OPType(it->t->type), opexpr));
        LC_TryTyping(it->expr, op);

        if (!(opexpr.flags & LC_OPF_Const)) result.flags &= ~LC_OPF_Const;
    }

    n->ecompo.resolved_items = matches;
    return result;
}

LC_FUNCTION LC_ResolvedCompoArrayItem *LC_AddResolvedCompoArrayItem(LC_ResolvedArrayCompo *arr, int index, LC_AST *comp) {
    LC_AST *duplicate1 = (LC_AST *)LC_MapGetP(&L->resolver.duplicate_map, arr);
    if (!duplicate1) {
        for (LC_ResolvedCompoArrayItem *it = arr->first; it; it = it->next) {
            if (index == it->index) {
                LC_MapInsertP(&L->resolver.duplicate_map, arr, comp);
                LC_MapInsertP(&L->resolver.duplicate_map, comp, it->comp);
                break;
            }
        }
    }
    LC_ResolvedCompoArrayItem *result = LC_PushStruct(L->arena, LC_ResolvedCompoArrayItem);
    result->index                     = index;
    result->comp                      = comp;
    arr->count += 1;
    LC_AddSLL(arr->first, arr->last, result);
    return result;
}

LC_FUNCTION LC_Operand LC_ResolveCompoArray(LC_AST *n, LC_Type *type) {
    LC_ASSERT(n, type->kind == LC_TypeKind_Array);
    LC_IF(n->ecompo.size > type->tarray.size, n, "too many array intializers, array is of size '%d', got '%d'", type->tarray.size, n->ecompo.size);

    bool index_appeared = false;
    LC_ASTFor(it, n->ecompo.first) {
        LC_IF(index_appeared && it->ecompo_item.index == NULL, it, "mixing indexed and positional arguments is illegal");
        LC_IF(it->ecompo_item.name, it, "named arguments are invalid in array compound literal");
        if (it->ecompo_item.index) index_appeared = true;
    }

    LC_ResolvedArrayCompo *matches = LC_PushStruct(L->arena, LC_ResolvedArrayCompo);
    LC_AST                *npos_it = n->ecompo.first;
    int                    index   = 0;

    // greedy match unnamed arguments
    for (; npos_it; npos_it = npos_it->next) {
        if (npos_it->ecompo_item.index) break;
        LC_ASSERT(n, index < type->tarray.size);
        LC_AddResolvedCompoArrayItem(matches, index++, npos_it);
    }

    // match indexed arguments
    for (; npos_it; npos_it = npos_it->next) {
        uint64_t   val = 0;
        LC_Operand LC_PROP_ERROR(op, npos_it, LC_ResolveConstInt(npos_it->ecompo_item.index, L->tint, &val));
        LC_IF(val > type->tarray.size, npos_it, "array index out of bounds, array is of size %d", type->tarray.size);
        LC_AddResolvedCompoArrayItem(matches, (int)val, npos_it);
    }

    // error on duplicates
    LC_AST *duplicate1 = (LC_AST *)LC_MapGetP(&L->resolver.duplicate_map, matches);
    LC_AST *duplicate2 = duplicate1 ? (LC_AST *)LC_MapGetP(&L->resolver.duplicate_map, duplicate1) : NULL;
    if (duplicate1) {
        LC_Operand err = LC_ReportASTErrorEx(duplicate1, duplicate2, "two items in compound array literal match the same index");
        n->kind        = LC_ASTKind_Error;
        return err;
    }

    // resolve
    LC_Operand result = LC_OPLValueAndType(type);
    result.flags |= LC_OPF_Const;
    for (LC_ResolvedCompoArrayItem *it = matches->first; it; it = it->next) {
        LC_AST    *expr = it->comp->ecompo_item.expr;
        LC_Operand LC_PROP_ERROR(opexpr, expr, LC_ResolveExprAndPushCompoContext(expr, type->tbase));
        LC_Operand LC_PROP_ERROR(op, expr, LC_ResolveTypeVarDecl(expr, LC_OPType(type->tbase), opexpr));
        LC_TryTyping(expr, op);

        if (!(opexpr.flags & LC_OPF_Const)) result.flags &= ~LC_OPF_Const;
    }

    n->ecompo.resolved_array_items = matches;
    return result;
}

LC_FUNCTION LC_Operand LC_ResolveTypeOrExpr(LC_AST *n) {
    if (n->kind == LC_ASTKind_ExprType) {
        LC_Operand LC_PROP_ERROR(result, n, LC_ResolveType(n->etype.type));
        n->type = result.type;
        return result;
    } else {
        LC_Operand LC_PROP_ERROR(result, n, LC_ResolveExpr(n));
        return result;
    }
}

LC_FUNCTION LC_Operand LC_ExpectBuiltinWithOneArg(LC_AST *n) {
    LC_IF(n->ecompo.size != 1, n, "expected 1 argument to builtin procedure, got: %d", n->ecompo.size);
    LC_IF(n->ecompo.first->ecompo_item.name, n, "named arguments in this builtin procedure are illegal");
    LC_AST    *expr = n->ecompo.first->ecompo_item.expr;
    LC_Operand LC_PROP_ERROR(op, expr, LC_ResolveTypeOrExpr(expr));
    LC_Operand LC_PROP_ERROR(maybe_err, n, LC_ResolveTypeAggregate(n, op.type));
    expr->type = op.type;
    return op;
}

LC_FUNCTION LC_Operand LC_ResolveBuiltin(LC_AST *n) {
    LC_Operand result = {0};
    if (n->ecompo.name == 0 || n->ecompo.name->kind != LC_ASTKind_ExprIdent) return result;
    LC_Intern ident = n->ecompo.name->eident.name;

    if (ident == L->ilengthof) {
        LC_Operand LC_PROP_ERROR(op, n, LC_ExpectBuiltinWithOneArg(n));
        if (LC_IsArray(op.type)) {
            result = LC_OPInt(op.type->tarray.size);
        } else if (LC_IsUTStr(op.type)) {
            int64_t length = LC_StrLen((char *)op.v.name);
            result         = LC_OPInt(length);
        } else LC_IF(1, n, "expected array or constant string type, got instead '%s'", LC_GenLCType(op.type));
        n->kind = LC_ASTKind_ExprBuiltin;
    } else if (ident == L->isizeof) {
        LC_Operand LC_PROP_ERROR(op, n, LC_ExpectBuiltinWithOneArg(n));
        LC_IF(LC_IsUntyped(op.type), n, "cannot get sizeof a value that is untyped: '%s'", LC_GenLCType(op.type));
        result  = LC_OPInt(op.type->size);
        n->kind = LC_ASTKind_ExprBuiltin;
    } else if (ident == L->ialignof) {
        LC_Operand LC_PROP_ERROR(op, n, LC_ExpectBuiltinWithOneArg(n));
        LC_IF(LC_IsUntyped(op.type), n, "cannot get alignof a value that is untyped: '%s'", LC_GenLCType(op.type));

        LC_AST *expr = n->ecompo.first->ecompo_item.expr;
        LC_IF(expr->kind != LC_ASTKind_ExprType, expr, "argument should be a type, instead it's '%s'", LC_ASTKindToString(expr->kind));

        result  = LC_OPInt(op.type->align);
        n->kind = LC_ASTKind_ExprBuiltin;
    } else if (ident == L->itypeof) {
        LC_Operand LC_PROP_ERROR(op, n, LC_ExpectBuiltinWithOneArg(n));
        LC_IF(LC_IsUntyped(op.type), n, "cannot get typeof a value that is untyped: '%s'", LC_GenLCType(op.type));
        result  = LC_OPInt(op.type->id);
        n->kind = LC_ASTKind_ExprBuiltin;
    } else if (ident == L->ioffsetof) {
        LC_IF(n->ecompo.size != 2, n, "expected 2 arguments to builtin procedure 'offsetof', got: %d", n->ecompo.size);
        LC_AST *a1 = n->ecompo.first;
        LC_AST *a2 = a1->next;
        LC_IF(a1->ecompo_item.name, a1, "named arguments in this builtin procedure are illegal");
        LC_IF(a2->ecompo_item.name, a2, "named arguments in this builtin procedure are illegal");
        LC_AST *a1e = a1->ecompo_item.expr;
        LC_AST *a2e = a2->ecompo_item.expr;
        LC_IF(a1e->kind != LC_ASTKind_ExprType, a1e, "first argument should be a type, instead it's '%s'", LC_ASTKindToString(a1e->kind));
        LC_Operand LC_PROP_ERROR(optype, a1e, LC_ResolveType(a1e->etype.type));
        LC_Operand LC_PROP_ERROR(maybe_err, n, LC_ResolveTypeAggregate(a1e, optype.type));
        LC_IF(!LC_IsAggType(optype.type), a1e, "expected aggregate type in first parameter of 'offsetof', instead got '%s'", LC_GenLCType(optype.type));
        LC_IF(a2e->kind != LC_ASTKind_ExprIdent, a2e, "expected identifier as second parameter to 'offsetof', instead got '%s'", LC_ASTKindToString(a2e->kind));
        a1e->type = optype.type;

        LC_Type       *type       = optype.type;
        LC_TypeMember *found_type = NULL;
        LC_TypeFor(it, type->tagg.mems.first) {
            if (it->name == a2e->eident.name) {
                found_type = it;
                break;
            }
        }

        LC_ASSERT(n, type->decl);
        LC_IF(!found_type, n, "field '%s' not found in '%s'", a2e->eident.name, type->decl->name);
        result  = LC_OPInt(found_type->offset);
        n->kind = LC_ASTKind_ExprBuiltin;
    }

    if (LC_IsUTConst(result)) {
        n->const_val = result.val;
    }

    return result;
}

LC_FUNCTION bool LC_TryTyping(LC_AST *n, LC_Operand op) {
    LC_ASSERT(n, n->type);

    if (LC_IsUntyped(n->type)) {
        if (LC_IsUTInt(n->type) && LC_IsFloat(op.type)) {
            LC_Operand in = {LC_OPF_UTConst | LC_OPF_Const};
            in.val        = n->const_val;
            LC_Operand op = LC_ConstCastFloat(NULL, in);
            SetConstVal(n, op.val);
        }
        if (L->tany == op.type) op = LC_OPModDefaultUT(LC_OPType(n->type));
        n->type = op.type;

        // Bounds check
        if (n->const_val.type && LC_IsUTInt(n->const_val.type)) {
            if (LC_IsInt(n->type) && !LC_BigIntFits(n->const_val.i, n->type)) {
                const char *val = LC_Bigint_str(&n->const_val.i, 10);
                LC_ReportASTError(n, "value '%s', doesn't fit into type '%s'", val, LC_GenLCType(n->type));
            }
        }
    }

    // I think it returns true to do this: (a, b)
    return true;
}

LC_FUNCTION bool LC_TryDefaultTyping(LC_AST *n, LC_Operand *o) {
    LC_ASSERT(n, n->type);
    if (LC_IsUntyped(n->type)) {
        n->type = n->type->tbase;
        if (o) o->type = n->type;
    }
    return true;
}

LC_FUNCTION LC_Operand LC_ResolveNameInScope(LC_AST *n, LC_Decl *parent_decl) {
    LC_ASSERT(n, n->kind == LC_ASTKind_ExprField || n->kind == LC_ASTKind_TypespecField);
    LC_PUSH_SCOPE(parent_decl->scope);
    LC_Operand op = LC_ResolveName(n, n->efield.right);
    LC_POP_SCOPE();
    if (LC_IsError(op)) {
        n->kind = LC_ASTKind_Error;
        return op;
    }

    LC_ASSERT(n, op.decl);
    n->efield.resolved_decl = op.decl;
    n->efield.parent_decl   = parent_decl;

    LC_ASSERT(n, op.decl->kind != LC_DeclKind_Import);
    return op;
}

LC_FUNCTION LC_Operand LC_ResolveExpr(LC_AST *expr) {
    return LC_ResolveExprAndPushCompoContext(expr, NULL);
}

LC_FUNCTION LC_Operand LC_ResolveExprAndPushCompoContext(LC_AST *expr, LC_Type *type) {
    LC_Type *save                  = L->resolver.compo_context_type;
    L->resolver.compo_context_type = type;
    LC_Operand LC_PROP_ERROR(result, expr, LC_ResolveExprEx(expr));
    L->resolver.compo_context_type = save;
    return result;
}

LC_FUNCTION LC_Operand LC_ResolveExprEx(LC_AST *n) {
    LC_Operand result = {0};
    LC_ASSERT(n, LC_IsExpr(n));

    switch (n->kind) {
    case LC_ASTKind_ExprFloat: {
        result.flags    = LC_OPF_UTConst | LC_OPF_Const;
        result.val.d    = n->eatom.d;
        result.val.type = L->tuntypedfloat;
        SetConstVal(n, result.val);
    } break;
    case LC_ASTKind_ExprInt: {
        result.flags    = LC_OPF_UTConst | LC_OPF_Const;
        result.val.i    = n->eatom.i;
        result.val.type = L->tuntypedint;
        SetConstVal(n, result.val);
    } break;
    case LC_ASTKind_ExprBool: {
        result.flags    = LC_OPF_UTConst | LC_OPF_Const;
        result.val.i    = n->eatom.i;
        result.val.type = L->tuntypedbool;
        SetConstVal(n, result.val);
    } break;
    case LC_ASTKind_ExprString: {
        result.flags    = LC_OPF_LValue | LC_OPF_UTConst | LC_OPF_Const;
        result.val.name = n->eatom.name;
        result.val.type = L->tuntypedstring;
        SetConstVal(n, result.val);
    } break;
    case LC_ASTKind_ExprType: {
        return LC_ReportASTError(n, "cannot use type as value");
    } break;

    case LC_ASTKind_ExprIdent: {
        LC_Operand LC_PROP_ERROR(op, n, LC_ResolveName(n, n->eident.name));
        LC_IF(op.decl->kind == LC_DeclKind_Type, n, "declaration is type, unexpected inside expression");
        LC_IF(op.decl->kind == LC_DeclKind_Import, n, "declaration is import, unexpected usage");

        n->eident.resolved_decl = op.decl;
        result.val              = op.decl->val;
        if (op.decl->kind == LC_DeclKind_Const) {
            result.flags |= LC_OPF_UTConst | LC_OPF_Const;
            SetConstVal(n, result.val);
        } else {
            result.flags |= LC_OPF_LValue | LC_OPF_Const;
        }
    } break;

    case LC_ASTKind_ExprCast: {
        LC_Operand LC_PROP_ERROR(optype, n, LC_ResolveType(n->ecast.type));
        LC_Operand LC_PROP_ERROR(opexpr, n, LC_ResolveExpr(n->ecast.expr));
        // :ConstantFold
        // the idea is that this will convert the literal into corresponding
        // type. In c :uint(32) will become 32u. This way we can avoid doing
        // typed arithmetic and let the backend handle it.
        if (LC_IsUTConst(opexpr) && (LC_IsNum(optype.type) || LC_IsStr(optype.type))) {
            if (LC_IsFloat(optype.type)) {
                LC_PROP_ERROR(opexpr, n, LC_ConstCastFloat(n, opexpr));
                SetConstVal(n, opexpr.val);
            } else if (LC_IsInt(optype.type)) {
                LC_PROP_ERROR(opexpr, n, LC_ConstCastInt(n, opexpr));
                SetConstVal(n, opexpr.val);
            } else if (LC_IsStr(optype.type)) {
                LC_IF(!LC_IsUTStr(opexpr.type), n, "cannot cast constant expression of type '%s' to '%s'", LC_GenLCType(opexpr.type), LC_GenLCType(optype.type));
                SetConstVal(n, opexpr.val);
            } else LC_IF(1, n, "cannot cast constant expression of type '%s' to '%s'", LC_GenLCType(opexpr.type), LC_GenLCType(optype.type));
            result.type = optype.type;
        } else {
            LC_PROP_ERROR(result, n, LC_ResolveTypeCast(n, optype, opexpr));
            LC_TryTyping(n->ecast.expr, result);
        }
        result.flags |= (opexpr.flags & LC_OPF_Const);
    } break;

    case LC_ASTKind_ExprUnary: {
        LC_PROP_ERROR(result, n, LC_ResolveExpr(n->eunary.expr));

        if (LC_IsUTConst(result)) {
            LC_PROP_ERROR(result, n, LC_EvalUnary(n, n->eunary.op, result));
            SetConstVal(n, result.val);
        } else {
            LC_OPResult r = LC_IsUnaryOpValidForType(n->eunary.op, result.type);
            LC_IF(r == LC_OPResult_Error, n, "invalid unary operation for type '%s'", LC_GenLCType(result.type));
            if (r == LC_OPResult_Bool) result = LC_OPModBool(result);
        }
    } break;

    case LC_ASTKind_ExprBinary: {
        LC_Operand LC_PROP_ERROR(left, n, LC_ResolveExpr(n->ebinary.left));
        LC_Operand LC_PROP_ERROR(right, n, LC_ResolveExpr(n->ebinary.right));
        LC_PROP_ERROR(result, n, LC_ResolveBinaryExpr(n, left, right));
    } break;

    case LC_ASTKind_ExprAddPtr: {
        LC_Operand LC_PROP_ERROR(left, n, LC_ResolveExpr(n->ebinary.left));
        LC_Operand LC_PROP_ERROR(right, n, LC_ResolveExpr(n->ebinary.right));
        LC_IF(!LC_IsInt(right.type), n, "trying to addptr non integer value of type '%s'", LC_GenLCType(left.type));
        if (LC_IsUTStr(left.type)) LC_TryDefaultTyping(n->ebinary.left, &left);
        LC_IF(!LC_IsPtr(left.type) && !LC_IsArray(left.type), n, "left type is required to be a pointer or array, instead got '%s'", LC_GenLCType(left.type));
        result = left;
        if (!LC_IsUTConst(right)) result.flags &= ~LC_OPF_Const;
        if (LC_IsArray(result.type)) result.type = LC_CreatePointerType(result.type->tbase);
        LC_TryTyping(n->ebinary.right, result);
    } break;

    case LC_ASTKind_ExprIndex: {
        LC_Operand LC_PROP_ERROR(opindex, n, LC_ResolveExpr(n->eindex.index));
        LC_Operand LC_PROP_ERROR(opexpr, n, LC_ResolveExpr(n->eindex.base));
        LC_IF(!LC_IsInt(opindex.type), n, "indexing with non integer value of type '%s'", LC_GenLCType(opindex.type));
        if (LC_IsUTStr(opexpr.type)) LC_TryDefaultTyping(n->eindex.base, &opexpr);
        LC_IF(!LC_IsPtr(opexpr.type) && !LC_IsArray(opexpr.type), n, "trying to index non indexable type '%s'", LC_GenLCType(opexpr.type));
        LC_IF(LC_IsVoidPtr(opexpr.type), n, "void is non indexable");
        LC_TryDefaultTyping(n->eindex.index, &opindex);
        result.type = LC_GetBase(opexpr.type);
        result.flags |= LC_OPF_LValue;
        LC_Operand LC_PROP_ERROR(maybe_err, n, LC_ResolveTypeAggregate(n, result.type));
    } break;

    case LC_ASTKind_ExprGetPointerOfValue: {
        LC_Operand LC_PROP_ERROR(op, n, LC_ResolveExpr(n->eunary.expr));
        LC_IF(!LC_IsLValue(op), n, "trying to access address of a temporal object");
        result.type = LC_CreatePointerType(op.type);
        result.flags |= (op.flags & LC_OPF_Const);
    } break;

    case LC_ASTKind_ExprGetValueOfPointer: {
        LC_Operand LC_PROP_ERROR(op, n, LC_ResolveExpr(n->eunary.expr));
        LC_IF(!LC_IsPtr(op.type), n, "trying to get value of non pointer type: '%s'", LC_GenLCType(op.type));
        result.type = LC_GetBase(op.type);
        result.flags |= LC_OPF_LValue;
        result.flags &= ~LC_OPF_Const;
        LC_Operand LC_PROP_ERROR(maybe_err, n, LC_ResolveTypeAggregate(n, result.type));
    } break;

    case LC_ASTKind_ExprField: {
        bool first_part_executed = false;

        LC_Operand op = {0};
        if (n->efield.left->kind == LC_ASTKind_ExprIdent) {
            LC_AST    *nf = n->efield.left;
            LC_Operand LC_PROP_ERROR(op_name, nf, LC_ResolveName(nf, nf->eident.name));

            // LC_Match (Package.) and fold (Package.Other) into just (Other)
            if (op_name.decl->kind == LC_DeclKind_Import) {
                first_part_executed      = true;
                nf->eident.resolved_decl = op_name.decl;
                nf->type                 = L->tvoid;

                LC_PROP_ERROR(op, n, LC_ResolveNameInScope(n, op_name.decl));
            }
        }

        if (!first_part_executed) {
            LC_ASTKind left_kind = n->efield.left->kind;
            LC_PROP_ERROR(op, n, LC_ResolveExpr(n->efield.left));

            LC_Type   *type = LC_StripPointer(op.type);
            LC_Operand LC_PROP_ERROR(maybe_err, n, LC_ResolveTypeAggregate(n, type));
            LC_IF(!LC_IsAggType(type), n->efield.left, "invalid operation, expected aggregate type, '%s' is not an aggregate", LC_GenLCType(type));
            LC_PROP_ERROR(op, n, LC_ResolveNameInScope(n, type->decl));
            LC_ASSERT(n, op.decl->kind == LC_DeclKind_Var);
            result.flags |= LC_OPF_LValue | LC_OPF_Const;
        }
        result.val = op.decl->val;
    } break;

    case LC_ASTKind_ExprCall: {
        LC_ASSERT(n, n->ecompo.name);
        LC_PROP_ERROR(result, n, LC_ResolveBuiltin(n));
        if (!result.type) {
            LC_Operand LC_PROP_ERROR(left, n, LC_ResolveExpr(n->ecompo.name));
            LC_IF(!LC_IsProc(left.type), n, "trying to call value of invalid type '%s', not a procedure", LC_GenLCType(left.type));
            if (L->before_call_args_resolved) L->before_call_args_resolved(n, left.type);
            LC_PROP_ERROR(result, n, LC_ResolveCompoCall(n, left.type));
        }
    } break;

    case LC_ASTKind_ExprCompound: {
        LC_Type *type = NULL;
        if (n->ecompo.name) {
            LC_PUSH_COMP_ARRAY_SIZE(n->ecompo.size);
            LC_Operand LC_PROP_ERROR(left, n, LC_ResolveTypeOrExpr(n->ecompo.name));
            type = left.type;
            LC_POP_COMP_ARRAY_SIZE();
        }
        if (!n->ecompo.name) type = L->resolver.compo_context_type;
        LC_IF(!type, n, "failed to deduce type of compound expression");
        LC_Operand LC_PROP_ERROR(maybe_err, n, LC_ResolveTypeAggregate(n, type));

        if (LC_IsAggType(type)) {
            LC_PROP_ERROR(result, n, LC_ResolveCompoAggregate(n, type));
        } else if (LC_IsArray(type)) {
            LC_PROP_ERROR(result, n, LC_ResolveCompoArray(n, type));
        } else {
            LC_IF(1, n, "compound of type '%s' is illegal, expected array, struct or union type", LC_GenLCType(type));
        }
    } break;

    default: LC_IF(1, n, "internal compiler error: unhandled expression kind '%s'", LC_ASTKindToString(n->kind));
    }

    n->type = result.type;
    if (n->type != L->tany && L->resolver.compo_context_type == L->tany) {
        LC_MapInsertP(&L->implicit_any, n, (void *)(intptr_t)1);
        result.flags &= ~LC_OPF_Const;
    }

    if (L->on_expr_resolved) L->on_expr_resolved(n, &result);
    return result;
}

LC_FUNCTION LC_Operand LC_ResolveStmtBlock(LC_AST *n) {
    LC_ASSERT(n, n->kind == LC_ASTKind_StmtBlock);

    LC_PUSH_LOCAL_SCOPE();
    LC_PushAST(&L->resolver.stmt_block_stack, n);

    LC_Operand result = {0};
    for (LC_AST *it = n->sblock.first; it; it = it->next) {
        LC_Operand op = LC_ResolveStmt(it);

        // We don't want to whine about non returned procedures if we spotted any errors
        // inside of it.
        if (LC_IsError(op) || (op.flags & LC_OPF_Returned)) result.flags |= LC_OPF_Returned;
    }

    LC_PopAST(&L->resolver.stmt_block_stack);
    LC_POP_LOCAL_SCOPE();

    if (L->on_stmt_resolved) L->on_stmt_resolved(n);
    return result;
}

LC_FUNCTION LC_Operand LC_ResolveVarDecl(LC_Decl *decl) {
    LC_ASSERT(decl->ast, decl->kind == LC_DeclKind_Var);
    LC_Operand result = {0};
    result.flags |= LC_OPF_Const;

    LC_AST    *n      = decl->ast;
    LC_Operand optype = {0};
    LC_Operand opexpr = {0};

    LC_AST *expr = n->dvar.expr;
    LC_AST *type = n->dvar.type;
    if (n->kind == LC_ASTKind_StmtVar) {
        expr = n->svar.expr;
        type = n->svar.type;
    } else {
        LC_ASSERT(n, n->kind == LC_ASTKind_DeclVar);
    }

    // special case := #c(``)
    if (expr && expr->kind == LC_ASTKind_ExprNote) {
        LC_Operand LC_PROP_ERROR(opnote, expr, LC_ResolveNote(expr, true));
        LC_IF(type == NULL, n, "invalid usage of unknown type, need to add type annotation");
        LC_DECL_PROP_ERROR(optype, LC_ResolveType(type));
        decl->type = optype.type;
    } else {
        if (type) {
            if (expr && expr->kind == LC_ASTKind_ExprCompound) {
                LC_PUSH_COMP_ARRAY_SIZE(expr->ecompo.size);
                LC_DECL_PROP_ERROR(optype, LC_ResolveType(type));
                LC_POP_COMP_ARRAY_SIZE();
            } else {
                LC_DECL_PROP_ERROR(optype, LC_ResolveType(type));
            }
        }
        if (expr) {
            LC_DECL_PROP_ERROR(opexpr, LC_ResolveExprAndPushCompoContext(expr, optype.type));
            if (!(opexpr.flags & LC_OPF_Const)) result.flags &= ~LC_OPF_Const;
        }

        LC_Operand LC_DECL_PROP_ERROR(opcast, LC_ResolveTypeVarDecl(n, optype, opexpr));
        if (expr) LC_TryTyping(expr, opcast);
        decl->val = opcast.val;
    }

    LC_ASSERT(n, decl->type);
    LC_Operand LC_PROP_ERROR(maybe_err, n, LC_ResolveTypeAggregate(n, decl->type));
    result.type = decl->type;
    return result;
}

LC_FUNCTION LC_Operand LC_MakeSureNoDeferBlock(LC_AST *n, char *str) {
    for (int i = L->resolver.stmt_block_stack.len - 1; i >= 0; i -= 1) {
        LC_AST *it = L->resolver.stmt_block_stack.data[i];
        if (it->kind == LC_ASTKind_Error) return LC_OPError();
        LC_ASSERT(it, it->kind == LC_ASTKind_StmtBlock);
        LC_IF(it->sblock.kind == SBLK_Defer, n, str);
    }
    return LC_OPNull;
}

LC_FUNCTION LC_Operand LC_MakeSureInsideLoopBlock(LC_AST *n, char *str) {
    bool loop_found = false;
    for (int i = L->resolver.stmt_block_stack.len - 1; i >= 0; i -= 1) {
        LC_AST *it = L->resolver.stmt_block_stack.data[i];
        if (it->kind == LC_ASTKind_Error) return LC_OPError();
        LC_ASSERT(it, it->kind == LC_ASTKind_StmtBlock);
        if (it->sblock.kind == SBLK_Loop) {
            loop_found = true;
            break;
        }
    }
    LC_IF(!loop_found, n, str);
    return LC_OPNull;
}

LC_FUNCTION LC_Operand LC_MatchLabeledBlock(LC_AST *n) {
    if (n->sbreak.name) {
        bool found = false;
        for (int i = L->resolver.stmt_block_stack.len - 1; i >= 0; i -= 1) {
            LC_AST *it = L->resolver.stmt_block_stack.data[i];
            if (it->kind == LC_ASTKind_Error) return LC_OPError();
            if (it->sblock.name == n->sbreak.name) {
                found = true;
                break;
            }
        }
        LC_IF(!found, n, "no label with name '%s'", n->sbreak.name);
    }
    return LC_OPNull;
}

LC_FUNCTION void WalkToFindCall(LC_ASTWalker *w, LC_AST *n) {
    if (n->kind == LC_ASTKind_ExprCall || n->kind == LC_ASTKind_ExprBuiltin) ((bool *)w->user_data)[0] = true;
}

LC_FUNCTION bool LC_ContainsCallExpr(LC_AST *ast) {
    LC_TempArena checkpoint = LC_BeginTemp(L->arena);
    bool         found_call = false;
    {
        LC_ASTWalker walker = LC_GetDefaultWalker(L->arena, WalkToFindCall);
        walker.depth_first  = false;
        walker.user_data    = (void *)&found_call;
        LC_WalkAST(&walker, ast);
    }
    LC_EndTemp(checkpoint);
    return found_call;
}

LC_FUNCTION LC_Operand LC_ResolveStmt(LC_AST *n) {
    LC_ASSERT(n, LC_IsStmt(n));

    LC_Operand result = {0};
    switch (n->kind) {
    case LC_ASTKind_StmtVar: {
        LC_Operand LC_PROP_ERROR(opdecl, n, LC_CreateLocalDecl(LC_DeclKind_Var, n->svar.name, n));
        LC_Operand LC_PROP_ERROR(op, n, LC_ResolveVarDecl(opdecl.decl));
        opdecl.decl->state    = LC_DeclState_Resolved;
        n->svar.resolved_decl = opdecl.decl;
        n->type               = op.type;
    } break;

    case LC_ASTKind_StmtConst: {
        LC_Operand LC_PROP_ERROR(opdecl, n, LC_CreateLocalDecl(LC_DeclKind_Const, n->sconst.name, n));
        LC_Operand LC_PROP_ERROR(op, n, LC_ResolveConstDecl(opdecl.decl));
        opdecl.decl->state = LC_DeclState_Resolved;
        n->type            = op.type;
    } break;

    case LC_ASTKind_StmtAssign: {
        LC_Operand LC_PROP_ERROR(left, n, LC_ResolveExpr(n->sassign.left));
        LC_IF(!LC_IsLValue(left), n, "assigning value to a temporal object (lvalue)");
        LC_Type *type = left.type;

        LC_OPResult valid = LC_IsAssignValidForType(n->sassign.op, type);
        LC_IF(valid == LC_OPResult_Error, n, "invalid assignment operation '%s' for type '%s'", LC_TokenKindToString(n->sassign.op), LC_GenLCType(type));

        LC_Operand LC_PROP_ERROR(right, n, LC_ResolveExprAndPushCompoContext(n->sassign.right, type));
        LC_PROP_ERROR(result, n, LC_ResolveTypeVarDecl(n, left, right));
        LC_TryTyping(n->sassign.left, result);
        LC_TryTyping(n->sassign.right, result);
        n->type = result.type;
    } break;

    case LC_ASTKind_StmtExpr: {
        LC_Operand LC_PROP_ERROR(op, n, LC_ResolveExpr(n->sexpr.expr));
        LC_TryDefaultTyping(n->sexpr.expr, &op);
        n->type               = op.type;
        bool    contains_call = LC_ContainsCallExpr(n->sexpr.expr);
        LC_AST *note          = LC_HasNote(n, L->iunused);
        LC_IF(!note && !contains_call, n, "very likely a bug, expression statement doesn't contain any calls so it doesn't do anything");
    } break;

    case LC_ASTKind_StmtReturn: {
        LC_PROP_ERROR(result, n, LC_MakeSureNoDeferBlock(n, "returning from defer block is illegal"));
        LC_Operand op = LC_OPType(L->tvoid);
        if (n->sreturn.expr) {
            LC_PROP_ERROR(op, n, LC_ResolveExprAndPushCompoContext(n->sreturn.expr, L->resolver.expected_ret_type));
        }

        if (!(op.type == L->resolver.expected_ret_type && op.type == L->tvoid)) {
            LC_PROP_ERROR(op, n, LC_ResolveTypeVarDecl(n, LC_OPType(L->resolver.expected_ret_type), op));
            if (n->sreturn.expr) LC_TryTyping(n->sreturn.expr, op);
        }
        result.flags |= LC_OPF_Returned;
    } break;

    case LC_ASTKind_StmtNote: LC_PROP_ERROR(result, n, LC_ResolveNote(n, false)); break;
    case LC_ASTKind_StmtContinue:
        LC_PROP_ERROR(result, n, LC_MakeSureNoDeferBlock(n, "continue inside of defer is illegal"));
        LC_PROP_ERROR(result, n, LC_MakeSureInsideLoopBlock(n, "continue outside of a for loop is illegal"));
    case LC_ASTKind_StmtBreak: {
        LC_PROP_ERROR(result, n, LC_MakeSureNoDeferBlock(n, "break inside of defer is illegal"));
        LC_PROP_ERROR(result, n, LC_MakeSureInsideLoopBlock(n, "break outside of a for loop is illegal"));
        LC_PROP_ERROR(result, n, LC_MatchLabeledBlock(n));
    } break;

    case LC_ASTKind_StmtDefer: {
        LC_PROP_ERROR(result, n, LC_MakeSureNoDeferBlock(n, "defer inside of defer is illegal"));
        LC_Operand LC_PROP_ERROR(op, n, LC_ResolveStmtBlock(n->sdefer.body));
        LC_AST    *parent_block = LC_GetLastAST(&L->resolver.stmt_block_stack);
        LC_ASSERT(n, parent_block->kind == LC_ASTKind_StmtBlock);
        LC_SLLStackAddMod(parent_block->sblock.first_defer, n, sdefer.next);
    } break;

    case LC_ASTKind_StmtSwitch: {
        LC_Operand LC_PROP_ERROR(opfirst, n, LC_ResolveExpr(n->sswitch.expr));
        LC_IF(!LC_IsInt(opfirst.type), n, "invalid type in switch condition '%s', it should be an integer", LC_GenLCType(opfirst.type));
        LC_TryDefaultTyping(n->sswitch.expr, &opfirst);

        bool all_returned = true;
        bool has_default  = n->sswitch.last && n->sswitch.last->kind == LC_ASTKind_StmtSwitchDefault;

        LC_Operand *ops = LC_PushArray(L->arena, LC_Operand, n->sswitch.total_switch_case_count);
        int         opi = 0;
        LC_ASTFor(case_it, n->sswitch.first) {
            if (case_it->kind == LC_ASTKind_StmtSwitchCase) {
                LC_ASTFor(case_expr_it, case_it->scase.first) {
                    LC_Operand LC_PROP_ERROR(opcase, n, LC_ResolveExpr(case_expr_it));
                    LC_IF(!LC_IsUTConst(opcase), case_expr_it, "expected an untyped constant");
                    ops[opi++] = opcase;

                    LC_Operand LC_PROP_ERROR(o, n, LC_ResolveTypeVarDecl(case_expr_it, opfirst, opcase));
                    LC_TryTyping(case_expr_it, o);
                }
            }
            LC_Operand LC_PROP_ERROR(opbody, case_it, LC_ResolveStmtBlock(case_it->scase.body));
            if (!(opbody.flags & LC_OPF_Returned)) all_returned = false;
        }
        LC_ASSERT(n, opi == n->sswitch.total_switch_case_count);

        for (int i = 0; i < opi; i += 1) {
            LC_Operand a = ops[i];
            for (int j = 0; j < opi; j += 1) {
                if (i == j) continue;
                LC_Operand b = ops[j];

                // bounds check error is thrown in LC_ResolveTypeVarDecl
                if (LC_BigIntFits(a.v.i, opfirst.type) && LC_BigIntFits(b.v.i, opfirst.type)) {
                    uint64_t au = LC_Bigint_as_unsigned(&a.v.i);
                    uint64_t bu = LC_Bigint_as_unsigned(&b.v.i);
                    LC_IF(au == bu, n, "duplicate fields, with value: %llu, in a switch statement", au);
                }
            }
        }

        if (all_returned && has_default) result.flags |= LC_OPF_Returned;
    } break;

    case LC_ASTKind_StmtFor: {
        LC_StmtFor *sfor = &n->sfor;
        LC_PUSH_LOCAL_SCOPE();

        if (sfor->init) {
            LC_Operand opinit = LC_ResolveStmt(sfor->init);
            if (LC_IsError(opinit)) {
                n->kind = LC_ASTKind_Error;
                LC_POP_LOCAL_SCOPE();
                return opinit;
            }

            LC_TryDefaultTyping(sfor->init, &opinit);
        }
        if (sfor->cond) {
            LC_Operand opcond = LC_ResolveExpr(sfor->cond);
            if (LC_IsError(opcond)) {
                n->kind = LC_ASTKind_Error;
                LC_POP_LOCAL_SCOPE();
                return opcond;
            }

            LC_TryDefaultTyping(sfor->cond, &opcond);
            if (!LC_IsIntLike(opcond.type)) {
                n->kind = LC_ASTKind_Error;
                LC_POP_LOCAL_SCOPE();
                return LC_ReportASTError(n, "invalid type in for condition '%s', it should be an integer or pointer", LC_GenLCType(opcond.type));
            }
        }
        if (sfor->inc) {
            LC_Operand opinc = LC_ResolveStmt(sfor->inc);
            if (LC_IsError(opinc)) {
                n->kind = LC_ASTKind_Error;
                LC_POP_LOCAL_SCOPE();
                return opinc;
            }
        }
        result = LC_ResolveStmtBlock(sfor->body);
        if (LC_IsError(result)) {
            n->kind = LC_ASTKind_Error;
            LC_POP_LOCAL_SCOPE();
            return result;
        }

        LC_POP_LOCAL_SCOPE();
    } break;

    case LC_ASTKind_StmtBlock: {
        // we don't handle errors here explicitly
        LC_Operand op = LC_ResolveStmtBlock(n);
        if (op.flags & LC_OPF_Returned) result.flags |= LC_OPF_Returned;
    } break;

    case LC_ASTKind_StmtIf: {
        LC_Operand LC_PROP_ERROR(op, n, LC_ResolveExpr(n->sif.expr));
        LC_TryDefaultTyping(n->sif.expr, &op);
        LC_IF(!LC_IsIntLike(op.type), n, "invalid type in if clause expression %s it should be an integer or pointer", LC_GenLCType(op.type));

        bool all_returned = true;
        bool has_else     = n->sif.last && n->sif.last->kind == LC_ASTKind_StmtElse;

        LC_Operand LC_PROP_ERROR(opbody, n, LC_ResolveStmtBlock(n->sif.body));
        if (!(opbody.flags & LC_OPF_Returned)) all_returned = false;

        LC_ASTFor(it, n->sif.first) {
            if (it->kind == LC_ASTKind_StmtElseIf) {
                LC_Operand LC_PROP_ERROR(op, it, LC_ResolveExpr(it->sif.expr));
                LC_TryDefaultTyping(it->sif.expr, &op);
                LC_IF(!LC_IsIntLike(op.type), n, "invalid type in if clause expression %s it should be an integer or pointer", LC_GenLCType(op.type));
            }
            LC_Operand LC_PROP_ERROR(opbody, it, LC_ResolveStmtBlock(it->sif.body));
            if (!(opbody.flags & LC_OPF_Returned)) all_returned = false;
        }

        if (all_returned && has_else) result.flags |= LC_OPF_Returned;
    } break;
    default: LC_IF(1, n, "internal compiler error: unhandled statement kind '%s'", LC_ASTKindToString(n->kind));
    }

    if (L->on_stmt_resolved) L->on_stmt_resolved(n);
    return result;
}

LC_FUNCTION LC_Operand LC_ResolveConstDecl(LC_Decl *decl) {
    LC_ASSERT(decl->ast, decl->kind == LC_DeclKind_Const);
    LC_AST *n    = decl->ast;
    LC_AST *expr = n->dconst.expr;
    if (n->kind == LC_ASTKind_StmtConst) {
        expr = n->sconst.expr;
    } else {
        LC_ASSERT(n, n->kind == LC_ASTKind_DeclConst);
    }

    LC_Operand LC_DECL_PROP_ERROR(opexpr, LC_ResolveExpr(expr));
    LC_DECL_IF(!LC_IsUTConst(opexpr), n, "expected an untyped constant");
    LC_DECL_IF(!LC_IsUntyped(opexpr.type), n, "type of constant expression is not a simple type");
    decl->val = opexpr.val;
    return opexpr;
}

LC_FUNCTION LC_Operand LC_ResolveName(LC_AST *pos, LC_Intern intern) {
    LC_Decl *decl = LC_GetLocalOrGlobalDecl(intern);
    LC_DECL_IF(!decl, pos, "undeclared identifier '%s'", intern);
    LC_DECL_IF(decl->state == LC_DeclState_Resolving, pos, "cyclic dependency %s", intern);
    if (decl->state == LC_DeclState_Error) return LC_OPError();
    if (decl->state == LC_DeclState_Resolved || decl->state == LC_DeclState_ResolvedBody) return LC_OPDecl(decl);
    LC_ASSERT(pos, decl->state == LC_DeclState_Unresolved);
    decl->state = LC_DeclState_Resolving;

    LC_AST *n = decl->ast;
    switch (decl->kind) {
    case LC_DeclKind_Const: {
        LC_Operand LC_DECL_PROP_ERROR(op, LC_ResolveConstDecl(decl));
    } break;
    case LC_DeclKind_Var: {
        LC_Operand LC_DECL_PROP_ERROR(op, LC_ResolveVarDecl(decl));
        LC_DECL_IF(!(op.flags & LC_OPF_Const), n, "non constant global declarations are illegal");
    } break;
    case LC_DeclKind_Proc: {
        LC_Operand LC_DECL_PROP_ERROR(optype, LC_ResolveType(n->dproc.type));
        decl->type       = optype.type;
        decl->type->decl = decl;
    } break;
    case LC_DeclKind_Import: {
        LC_ASSERT(n, decl->scope);
    } break;
    case LC_DeclKind_Type: {
        LC_ASSERT(n, n->kind == LC_ASTKind_DeclTypedef);
        LC_Operand LC_DECL_PROP_ERROR(op, LC_ResolveType(n->dtypedef.type));
        decl->val = op.val;

        // I have decided that aggregates cannot be hard typedefed.
        // It brings issues to LC_ResolveTypeAggregate and is not needed, what's needed
        // is typedef on numbers and pointers that create distinct new
        // types. I have never had a use for typedefing a struct to make
        // it more typesafe etc.
        LC_AST *is_weak = LC_HasNote(n, L->iweak);
        bool    is_agg  = op.type->decl && LC_IsAgg(op.type->decl->ast);
        if (!is_weak && !is_agg) decl->type = LC_CreateTypedef(decl, decl->type);
        LC_DECL_IF(is_weak && is_agg, n, "@weak doesn't work on aggregate types");
    } break;
    default: LC_DECL_IF(1, n, "internal compiler error: unhandled LC_DeclKind: '%s'", LC_DeclKindToString(decl->kind))
    }
    decl->state = LC_DeclState_Resolved;

    if (L->on_decl_type_resolved) L->on_decl_type_resolved(decl);
    LC_AST *pkg = decl->package;
    LC_DLLAdd(pkg->apackage.first_ordered, pkg->apackage.last_ordered, decl);
    return LC_OPDecl(decl);
}

LC_FUNCTION LC_Operand LC_ResolveConstInt(LC_AST *n, LC_Type *int_type, uint64_t *out_size) {
    LC_Operand LC_PROP_ERROR(op, n, LC_ResolveExpr(n));
    LC_IF(!LC_IsUTConst(op), n, "expected a constant untyped int");
    LC_IF(!LC_IsUTInt(op.type), n, "expected untyped int constant instead: '%s'", LC_GenLCType(op.type));
    LC_IF(!LC_BigIntFits(op.val.i, int_type), n, "constant value: '%s', doesn't fit in type '%s'", LC_GenLCTypeVal(op.val), LC_GenLCType(int_type));
    if (out_size) *out_size = LC_Bigint_as_unsigned(&op.val.i);
    LC_TryTyping(n, LC_OPType(int_type));
    return op;
}

LC_FUNCTION LC_Operand LC_ResolveType(LC_AST *n) {
    LC_ASSERT(n, LC_IsType(n));
    LC_Operand result = {0};

    switch (n->kind) {
    case LC_ASTKind_TypespecField: {
        LC_ASSERT(n, n->efield.left->kind == LC_ASTKind_TypespecIdent);
        LC_Operand LC_PROP_ERROR(l, n, LC_ResolveName(n, n->efield.left->eident.name));
        LC_IF(l.decl->kind != LC_DeclKind_Import, n, "only accessing '.' imports in type definitions is valid, you are trying to access: '%s'", LC_DeclKindToString(l.decl->kind));
        n->efield.left->eident.resolved_decl = l.decl;
        n->efield.left->type                 = L->tvoid;

        LC_Operand LC_PROP_ERROR(op, n, LC_ResolveNameInScope(n, l.decl));
        LC_IF(op.decl->kind != LC_DeclKind_Type, n, "expected reference to type, instead it's: '%s'", LC_DeclKindToString(op.decl->kind));
        result.type = op.decl->type;
    } break;

    case LC_ASTKind_TypespecIdent: {
        LC_Operand LC_PROP_ERROR(op, n, LC_ResolveName(n, n->eident.name));
        LC_IF(op.decl->kind != LC_DeclKind_Type, n, "identifier is not a type");
        result.type             = op.decl->type;
        n->eident.resolved_decl = op.decl;
    } break;

    case LC_ASTKind_TypespecPointer: {
        LC_Operand LC_PROP_ERROR(op, n, LC_ResolveType(n->tpointer.base));
        result.type = LC_CreatePointerType(op.type);
    } break;

    case LC_ASTKind_TypespecArray: {
        LC_Operand LC_PROP_ERROR(opbase, n, LC_ResolveType(n->tarray.base));
        uint64_t   size = L->resolver.compo_context_array_size;
        if (n->tarray.index) {
            LC_Operand LC_PROP_ERROR(opindex, n, LC_ResolveConstInt(n->tarray.index, L->tint, &size));
        }
        LC_IF(size == 0, n, "failed to deduce array size");
        LC_Operand LC_PROP_ERROR(maybe_err, n, LC_ResolveTypeAggregate(n, opbase.type));

        result.type = LC_CreateArrayType(opbase.type, (int)size);
    } break;

    case LC_ASTKind_TypespecProc: {
        LC_Type *ret = L->tvoid;
        if (n->tproc.ret) {
            LC_Operand LC_PROP_ERROR(op, n->tproc.ret, LC_ResolveType(n->tproc.ret));
            ret = op.type;
        }
        LC_Operand        LC_PROP_ERROR(maybe_err, n, LC_ResolveTypeAggregate(n, ret));
        LC_TypeMemberList typelist = {0};
        LC_ASTFor(it, n->tproc.first) {
            LC_Operand LC_PROP_ERROR(op, it, LC_ResolveType(it->tproc_arg.type));
            LC_Operand LC_PROP_ERROR(maybe_err, n, LC_ResolveTypeAggregate(it, op.type));

            LC_AST *expr = it->tproc_arg.expr;
            if (expr) {
                LC_Operand LC_PROP_ERROR(opexpr, expr, LC_ResolveExprAndPushCompoContext(expr, op.type));
                LC_Operand LC_PROP_ERROR(opfin, expr, LC_ResolveTypeVarDecl(expr, op, opexpr));
                LC_TryTyping(expr, opfin);
            }

            LC_TypeMember *mem = LC_AddTypeToList(&typelist, it->tproc_arg.name, op.type, it);
            LC_IF(!mem, it, "duplicate proc argument '%s'", it->tproc_arg.name);
            mem->default_value_expr = expr;
            it->type                = op.type;
        }

        LC_TypeFor(i, typelist.first) {
            LC_TypeFor(j, typelist.first) {
                LC_IF(i != j && i->name == j->name, i->ast, "procedure has 2 arguments with the same name");
            }
        }
        result.type = LC_CreateProcType(typelist, ret, n->tproc.vargs, n->tproc.vargs_any_promotion);
    } break;

    default: LC_IF(1, n, "internal compiler error: unhandled kind in LC_ResolveType '%s'", LC_ASTKindToString(n->kind));
    }

    n->type = result.type;
    LC_ASSERT(n, result.type);
    return result;
}

// clang-format off
// NA - Not applicable
// LC_RPS - OK if right side int of pointer size
// LC_LPS
// LC_TEQ - OK if types equal
// LC_RO0 - OK if right value is int equal nil
// LT - Left untyped to typed
// RT
// LF - Left to float
// RF
// LC_SR - String
enum { LC_INT, LC_FLOAT, LC_UT_INT, LC_UT_FLOAT, LC_UT_STR, LC_PTR, LC_VOID_PTR, LC_PROC, LC_AGG, LC_ARRAY, LC_ANY, LC_VOID, LC_TYPE_COUNT };
typedef enum { LC_NO, LC_OK, LC_LPS, LC_RPS, LC_TEQ, LC_NA, LC_RO0, LC_LT, LC_RT, LC_LF, LC_RF, LC_SR } LC_TypeRule;

LC_TypeRule CastingRules[LC_TYPE_COUNT][LC_TYPE_COUNT] = {
//\/:tgt( src)>   LC_INT      , LC_FLOAT , LC_UT_INT , LC_UT_FLOAT , LC_UT_STR , LC_PTR , LC_VOID_PTR , LC_PROC , LC_AGG , LC_ARRAY
/*[LC_INT] =        */{LC_OK  , LC_OK    , LC_OK     , LC_OK       , LC_NO     , LC_LPS , LC_LPS      , LC_NO   , LC_NO  , LC_NO}   ,
/*[LC_FLOAT] =      */{LC_OK  , LC_OK    , LC_OK     , LC_OK       , LC_NO     , LC_NO  , LC_NO       , LC_NO   , LC_NO  , LC_NO}   ,
/*[LC_UT_INT] =     */{LC_NA  , LC_NA    , LC_NA     , LC_NA       , LC_NO     , LC_NA  , LC_NA       , LC_NA   , LC_NA  , LC_NA}   ,
/*[LC_UT_FLOAT] =   */{LC_NA  , LC_NA    , LC_NA     , LC_NA       , LC_NO     , LC_NA  , LC_NA       , LC_NA   , LC_NA  , LC_NA}   ,
/*[LC_UT_STR] =     */{LC_NA  , LC_NA    , LC_NA     , LC_NA       , LC_NO     , LC_NA  , LC_NA       , LC_NA   , LC_NA  , LC_NA}   ,
/*[LC_PTR] =        */{LC_RPS , LC_NO    , LC_OK     , LC_NO       , LC_SR     , LC_OK  , LC_OK       , LC_OK   , LC_NO  , LC_NO}   ,
/*[LC_VOID_PTR] =   */{LC_RPS , LC_NO    , LC_OK     , LC_NO       , LC_OK     , LC_OK  , LC_OK       , LC_OK   , LC_NO  , LC_NO}   ,
/*[LC_PROC] =       */{LC_NO  , LC_NO    , LC_NO     , LC_NO       , LC_NO     , LC_OK  , LC_OK       , LC_OK   , LC_NO  , LC_NO}   ,
/*[LC_AGG] =        */{LC_NO  , LC_NO    , LC_NO     , LC_NO       , LC_SR     , LC_NO  , LC_NO       , LC_NO   , LC_NO  , LC_NO}   ,
/*[LC_ARRAY] =      */{LC_NO  , LC_NO    , LC_NO     , LC_NO       , LC_NO     , LC_NO  , LC_NO       , LC_NO   , LC_NO  , LC_NO}   ,
};

LC_TypeRule AssignRules[LC_TYPE_COUNT][LC_TYPE_COUNT] = {
//\/l r>            LC_INT    , LC_FLOAT , LC_UT_INT , LC_UT_FLOAT , LC_UT_STR , LC_PTR , LC_VOID_PTR , LC_PROC , LC_AGG , LC_ARRAY , LC_ANY
/*[LC_INT] =        */{LC_TEQ , LC_NO    , LC_OK     , LC_NO       , LC_NO     , LC_NO  , LC_NO       , LC_NO   , LC_NO  , LC_NO    , LC_NO}  ,
/*[LC_FLOAT] =      */{LC_NO  , LC_TEQ   , LC_OK     , LC_OK       , LC_NO     , LC_NO  , LC_NO       , LC_NO   , LC_NO  , LC_NO    , LC_NO}  ,
/*[LC_UT_INT] =     */{LC_NA  , LC_NA    , LC_NA     , LC_NA       , LC_NA     , LC_NA  , LC_NA       , LC_NA   , LC_NA  , LC_NA    , LC_NO}  ,
/*[LC_UT_FLOAT] =   */{LC_NA  , LC_NA    , LC_NA     , LC_NA       , LC_NA     , LC_NA  , LC_NA       , LC_NA   , LC_NA  , LC_NA    , LC_NO}  ,
/*[LC_UT_STR] =     */{LC_NA  , LC_NA    , LC_NA     , LC_NA       , LC_NA     , LC_NO  , LC_NA       , LC_NA   , LC_NA  , LC_NA    , LC_NO}  ,
/*[LC_PTR] =        */{LC_NO  , LC_NO    , LC_RO0    , LC_NO       , LC_SR     , LC_TEQ , LC_OK       , LC_NO   , LC_NO  , LC_NO    , LC_NO}  ,
/*[LC_VOID_PTR] =   */{LC_NO  , LC_NO    , LC_RO0    , LC_NO       , LC_OK     , LC_OK  , LC_OK       , LC_OK   , LC_NO  , LC_NO    , LC_NO}  ,
/*[LC_PROC] =       */{LC_NO  , LC_NO    , LC_RO0    , LC_NO       , LC_NO     , LC_NO  , LC_OK       , LC_TEQ  , LC_NO  , LC_NO    , LC_NO}  ,
/*[LC_AGG] =        */{LC_NO  , LC_NO    , LC_NO     , LC_NO       , LC_SR     , LC_NO  , LC_NO       , LC_NO   , LC_TEQ , LC_NO    , LC_TEQ} ,
/*[LC_ARRAY] =      */{LC_NO  , LC_NO    , LC_NO     , LC_NO       , LC_NO     , LC_NO  , LC_NO       , LC_NO   , LC_NO  , LC_TEQ   , LC_NO}  ,
/*[LC_ANY] =        */{LC_OK  , LC_OK    , LC_OK     , LC_OK       , LC_OK     , LC_OK  , LC_NO       , LC_OK   , LC_OK  , LC_OK    , LC_OK}  ,
};

LC_TypeRule BinaryRules[LC_TYPE_COUNT][LC_TYPE_COUNT] = {
//\/l r>           LC_INT     , LC_FLOAT , LC_UT_INT , LC_UT_FLOAT , LC_UT_STR , LC_PTR , LC_VOID_PTR , LC_PROC , LC_AGG , LC_ARRAY
/*[LC_INT]     =   */{LC_TEQ  , LC_NO    , LC_RT     , LC_NO       , LC_NO     , LC_NO  , LC_OK       , LC_OK   , LC_NO  , LC_NO    , }       ,
/*[LC_FLOAT]   =   */{LC_NO   , LC_TEQ   , LC_RT     , LC_RT       , LC_NO     , LC_NO  , LC_NO       , LC_NO   , LC_NO  , LC_NO    , }       ,
/*[LC_UT_INT]  =   */{LC_LT   , LC_LT    , LC_OK     , LC_LF       , LC_NO     , LC_OK  , LC_OK       , LC_OK   , LC_NO  , LC_NO    , }       ,
/*[LC_UT_FLOAT]=   */{LC_NO   , LC_LT    , LC_RF     , LC_OK       , LC_NO     , LC_NO  , LC_NO       , LC_NO   , LC_NO  , LC_NO    , }       ,
/*[LC_UT_STR] =    */{LC_NO   , LC_NO    , LC_NO     , LC_NO       , LC_OK     , LC_NO  , LC_NO       , LC_NO   , LC_NO  , LC_NO    , }       ,
/*[LC_PTR]     =   */{LC_OK   , LC_NO    , LC_OK     , LC_NO       , LC_NO     , LC_OK  , LC_OK       , LC_NO   , LC_NO  , LC_NO    , }       ,
/*[LC_VOID_PTR]=   */{LC_OK   , LC_NO    , LC_OK     , LC_NO       , LC_NO     , LC_OK  , LC_OK       , LC_NO   , LC_NO  , LC_NO    , }       ,
/*[LC_PROC]    =   */{LC_OK   , LC_NO    , LC_OK     , LC_NO       , LC_NO     , LC_NO  , LC_NO       , LC_NO   , LC_NO  , LC_NO    , }       ,
/*[LC_AGG]     =   */{LC_NO   , LC_NO    , LC_NO     , LC_NO       , LC_NO     , LC_NO  , LC_NO       , LC_NO   , LC_NO  , LC_NO    , }       ,
/*[LC_ARRAY]   =   */{LC_NO   , LC_NO    , LC_NO     , LC_NO       , LC_NO     , LC_NO  , LC_NO       , LC_NO   , LC_NO  , LC_NO    , }       ,
};
// clang-format on

int GetTypeCategory(LC_Type *x) {
    if (x->kind >= LC_TypeKind_char && x->kind <= LC_TypeKind_ullong) return LC_INT;
    if ((x->kind == LC_TypeKind_float) || (x->kind == LC_TypeKind_double)) return LC_FLOAT;
    if (x->kind == LC_TypeKind_UntypedInt) return LC_UT_INT;
    if (x->kind == LC_TypeKind_UntypedFloat) return LC_UT_FLOAT;
    if (x->kind == LC_TypeKind_UntypedString) return LC_UT_STR;
    if (x == L->tpvoid) return LC_VOID_PTR;
    if (x == L->tany) return LC_ANY;
    if (x->kind == LC_TypeKind_Pointer) return LC_PTR;
    if (x->kind == LC_TypeKind_Proc) return LC_PROC;
    if (LC_IsAggType(x)) return LC_AGG;
    if (LC_IsArray(x)) return LC_ARRAY;
    return LC_VOID;
}

LC_FUNCTION LC_Operand LC_ResolveBinaryExpr(LC_AST *n, LC_Operand l, LC_Operand r) {
    bool isconst = LC_IsConst(l) && LC_IsConst(r);

    LC_TypeRule rule = BinaryRules[GetTypeCategory(l.type)][GetTypeCategory(r.type)];
    LC_IF(rule == LC_NO, n, "cannot perform binary operation, types don't qualify for it, left: '%s' right: '%s'", LC_GenLCType(l.type), LC_GenLCType(r.type));
    LC_IF(rule == LC_TEQ && l.type != r.type, n, "cannot perform binary operation, types are incompatible, left: '%s' right: '%s'", LC_GenLCType(l.type), LC_GenLCType(r.type));
    if (rule == LC_LT) l = LC_OPModType(l, r.type);
    if (rule == LC_RT) r = LC_OPModType(r, l.type);
    if (rule == LC_LF) l = LC_ConstCastFloat(n, l);
    if (rule == LC_RF) r = LC_ConstCastFloat(n, r);
    LC_ASSERT(n, rule == LC_LT || rule == LC_RT || rule == LC_LF || rule == LC_RF || rule == LC_OK || rule == LC_TEQ);

    // WARNING: if we allow for more then boolean operations on pointers then
    // we need to fix the propagated type here, we are counting on it getting
    // modified to bool.
    LC_Operand op = LC_OPType(l.type);
    if (isconst) op.flags |= LC_OPF_Const;
    if (LC_IsUTConst(l) && LC_IsUTConst(r)) {
        LC_PROP_ERROR(op, n, LC_EvalBinary(n, l, n->ebinary.op, r));
        SetConstVal(n, op.val);
    } else {
        LC_OPResult r = LC_IsBinaryExprValidForType(n->ebinary.op, op.type);
        LC_IF(r == LC_OPResult_Error, n, "invalid binary operation for type '%s'", LC_GenLCType(op.type));
        (LC_TryTyping(n->ebinary.left, op), LC_TryTyping(n->ebinary.right, op));
        if (r == LC_OPResult_Bool) op = LC_OPModBool(op);
    }

    return op;
}

LC_FUNCTION LC_Operand LC_ResolveTypeVargs(LC_AST *pos, LC_Operand v) {
    if (LC_IsUntyped(v.type)) v = LC_OPModDefaultUT(v);            // untyped => typed
    if (LC_IsSmallerThenInt(v.type)) v = LC_OPModType(v, L->tint); // c int promotion
    if (v.type == L->tfloat) v = LC_OPModType(v, L->tdouble);      // c int promotion
    return v;
}

LC_FUNCTION LC_Operand LC_ResolveTypeCast(LC_AST *pos, LC_Operand t, LC_Operand v) {
    LC_TypeRule rule = CastingRules[GetTypeCategory(t.type)][GetTypeCategory(v.type)];
    LC_IF(rule == LC_NO, pos, "cannot cast, types are incompatible, left: '%s' right: '%s'", LC_GenLCType(t.type), LC_GenLCType(v.type));
    LC_IF(rule == LC_RPS && v.type->size != L->tpvoid->size, pos, "cannot cast, integer type on right is not big enough to hold a pointer, left: '%s' right: '%s'", LC_GenLCType(t.type), LC_GenLCType(v.type));
    LC_IF(rule == LC_LPS && t.type->size != L->tpvoid->size, pos, "cannot cast, integer type on left is not big enough to hold a pointer, left: '%s' right: '%s'", LC_GenLCType(t.type), LC_GenLCType(v.type));
    LC_IF(rule == LC_SR && !LC_IsStr(t.type), pos, "cannot cast untyped string to non string type: '%s'", LC_GenLCType(t.type));
    LC_ASSERT(pos, rule == LC_LPS || rule == LC_RPS || rule == LC_OK);

    LC_Operand op = LC_OPType(t.type);
    op.flags      = (v.flags & LC_OPF_LValue) | (v.flags & LC_OPF_Const);
    return op;
}

LC_FUNCTION LC_Operand LC_ResolveTypeVarDecl(LC_AST *pos, LC_Operand t, LC_Operand v) {
    t.v = v.v;
    LC_IF(t.type && t.type->kind == LC_TypeKind_void, pos, "cannot create a variable of type void");
    LC_IF(v.type && v.type->kind == LC_TypeKind_void, pos, "cannot assign void expression to a variable");
    if (v.type && t.type) {
        LC_TypeRule rule = AssignRules[GetTypeCategory(t.type)][GetTypeCategory(v.type)];
        LC_IF(rule == LC_NO, pos, "cannot assign, types are incompatible, variable type: '%s' expression type: '%s'", LC_GenLCType(t.type), LC_GenLCType(v.type));
        LC_IF(rule == LC_RPS && v.type->size != L->tpvoid->size, pos, "cannot assign, integer type of expression is not big enough to hold a pointer, left: '%s' right: '%s'", LC_GenLCType(t.type), LC_GenLCType(v.type));
        LC_IF(rule == LC_LPS && t.type->size != L->tpvoid->size, pos, "cannot assign, integer type of variable is not big enough to hold a pointer, left: '%s' right: '%s'", LC_GenLCType(t.type), LC_GenLCType(v.type));
        LC_IF(rule == LC_TEQ && t.type != v.type, pos, "cannot assign, types require explicit cast, variable type: '%s' expression type: '%s'", LC_GenLCType(t.type), LC_GenLCType(v.type));
        LC_IF(rule == LC_RO0 && v.type != L->tuntypednil, pos, "cannot assign, can assign only const integer equal to 0, variable type: '%s' expression type: '%s'", LC_GenLCType(t.type), LC_GenLCType(v.type));
        LC_IF(rule == LC_SR && !LC_IsStr(t.type), pos, "cannot assign untyped string to non string type: '%s'", LC_GenLCType(t.type));
        LC_ASSERT(pos, rule == LC_LPS || rule == LC_RPS || rule == LC_OK || rule == LC_TEQ || rule == LC_RO0 || rule == LC_SR);
        return t;
    }

    if (v.type) return LC_OPModDefaultUT(v); // NULL := untyped => default
    if (t.type) return t;                    // T := NULL => T
    return LC_ReportASTError(pos, "internal compiler error: failed to resolve type of variable, both type and expression are null");
}

LC_FUNCTION LC_Operand LC_ResolveTypeAggregate(LC_AST *pos, LC_Type *type) {
    LC_Decl *decl = type->decl;
    if (type->kind == LC_TypeKind_Error) return LC_OPError();
    LC_TYPE_IF(type->kind == LC_TypeKind_Completing, pos, "cyclic dependency in type '%s'", type->decl->name);
    if (type->kind != LC_TypeKind_Incomplete) return LC_OPNull;
    LC_PUSH_SCOPE(L->resolver.package->apackage.scope);

    LC_AST *n = decl->ast;
    LC_ASSERT(n, decl);
    LC_ASSERT(n, n->kind == LC_ASTKind_DeclStruct || n->kind == LC_ASTKind_DeclUnion);
    int decl_stack_size = 0;

    type->kind = LC_TypeKind_Completing;
    LC_ASTFor(it, n->dagg.first) {
        LC_Intern name = it->tagg_mem.name;

        LC_Operand op = LC_ResolveType(it->tagg_mem.type);
        if (LC_IsError(op)) {
            LC_MarkDeclError(decl);
            type->kind = LC_TypeKind_Error;
            continue; // handle error after we go through all fields
        }

        LC_Operand opc = LC_ResolveTypeAggregate(it, op.type);
        if (LC_IsError(opc)) {
            LC_MarkDeclError(decl);
            type->kind = LC_TypeKind_Error;
            continue; // handle error after we go through all fields
        }

        LC_TypeMember *mem = LC_AddTypeToList(&type->tagg.mems, name, op.type, it);
        LC_TYPE_IF(mem == NULL, it, "duplicate field '%s' in aggregate type '%s'", name, decl->name);
    }
    if (type->kind == LC_TypeKind_Error) {
        return LC_OPError();
    }
    LC_TYPE_IF(type->tagg.mems.count == 0, n, "aggregate type '%s' has no fields", decl->name);
    decl_stack_size += type->tagg.mems.count;

    LC_AST *packed = LC_HasNote(n, L->ipacked);
    if (n->kind == LC_ASTKind_DeclStruct) {
        type->kind      = LC_TypeKind_Struct;
        int field_sizes = 0;
        LC_TypeFor(it, type->tagg.mems.first) {
            int mem_align = packed ? 1 : it->type->align;
            LC_ASSERT(n, LC_IS_POW2(mem_align));
            type->size = (int)LC_AlignUp(type->size, mem_align);
            it->offset = type->size;
            field_sizes += it->type->size;
            type->align = LC_MAX(type->align, mem_align);
            type->size  = it->type->size + (int)LC_AlignUp(type->size, mem_align);
        }
        type->size    = (int)LC_AlignUp(type->size, type->align);
        type->padding = type->size - field_sizes;
    }

    if (n->kind == LC_ASTKind_DeclUnion) {
        type->kind = LC_TypeKind_Union;
        if (packed) LC_ReportASTError(packed, "@packed on union is invalid");
        LC_TypeFor(it, type->tagg.mems.first) {
            LC_ASSERT(n, LC_IS_POW2(it->type->align));
            type->size  = LC_MAX(type->size, it->type->size);
            type->align = LC_MAX(type->align, it->type->align);
        }
        type->size = (int)LC_AlignUp(type->size, type->align);
    }

    int map_size = LC_NextPow2(decl_stack_size * 2 + 1);
    decl->scope  = LC_CreateScope(map_size);

    LC_TypeFor(it, type->tagg.mems.first) {
        LC_Decl *d     = LC_CreateDecl(LC_DeclKind_Var, it->name, it->ast);
        d->state       = LC_DeclState_Resolved;
        d->type        = it->type;
        d->type_member = it;
        LC_AddDeclToScope(decl->scope, d);
    }

    LC_ASSERT(n, decl->scope->cap == map_size);

    if (L->on_decl_type_resolved) L->on_decl_type_resolved(decl);
    LC_AST *pkg = decl->package;
    LC_DLLAdd(pkg->apackage.first_ordered, pkg->apackage.last_ordered, decl);
    LC_POP_SCOPE();
    return LC_OPNull;
}

#undef LC_IF
#undef LC_DECL_IF
#undef LC_PROP_ERROR
#undef LC_DECL_PROP_ERROR
const int ParseStmtBlock_AllowSingleStmt = 1;

LC_FUNCTION LC_Parser LC_MakeParser(LC_Lex *x) {
    LC_Parser p = {0};
    p.at        = x->tokens;
    p.begin     = x->tokens;
    p.end       = x->tokens + x->token_count;
    p.x         = x;
    return p;
}

LC_FUNCTION LC_Parser *LC_MakeParserQuick(char *str) {
    LC_Lex *x = LC_LexStream("quick_lex", str, 0);
    LC_InternTokens(x);

    L->quick_parser = LC_MakeParser(x);
    L->parser       = &L->quick_parser;
    return L->parser;
}

LC_FUNCTION LC_AST *LC_ReportParseError(LC_Token *pos, const char *str, ...) {
    LC_FORMAT(L->arena, str, n);
    LC_SendErrorMessage(pos, n);
    L->errors += 1;
    LC_AST *r = LC_CreateAST(pos, LC_ASTKind_Error);
    return r;
}

LC_FUNCTION LC_Token *LC_Next(void) {
    if (L->parser->at < L->parser->end) {
        LC_Token *result = L->parser->at;
        L->parser->at += 1;
        return result;
    }
    return &L->NullToken;
}

LC_FUNCTION LC_Token *LC_Get(void) {
    if (L->parser->at < L->parser->end) {
        return L->parser->at;
    }
    return &L->NullToken;
}

LC_FUNCTION LC_Token *LC_GetI(int i) {
    LC_Token *result = L->parser->at + i;
    if (result >= L->parser->begin && result < L->parser->end) {
        return result;
    }
    return &L->NullToken;
}

LC_FUNCTION LC_Token *LC_Is(LC_TokenKind kind) {
    LC_Token *t = LC_Get();
    if (t->kind == kind) {
        return t;
    }
    return 0;
}

LC_FUNCTION LC_Token *LC_IsKeyword(LC_Intern intern) {
    LC_Token *t = LC_Get();
    if (t->kind == LC_TokenKind_Keyword && t->ident == intern) {
        return t;
    }
    return 0;
}

LC_FUNCTION LC_Token *LC_Match(LC_TokenKind kind) {
    LC_Token *t = LC_Get();
    if (t->kind == kind) {
        LC_Next();
        return t;
    }
    return 0;
}

LC_FUNCTION LC_Token *LC_MatchKeyword(LC_Intern intern) {
    LC_Token *t = LC_Get();
    if (t->kind == LC_TokenKind_Keyword && t->ident == intern) {
        LC_Next();
        return t;
    }
    return 0;
}

#define LC_EXPECT(token, KIND, context)                                                                                                                                   \
    LC_Token *token = LC_Match(KIND);                                                                                                                                     \
    if (!token) {                                                                                                                                                         \
        LC_Token *t = LC_Get();                                                                                                                                           \
        return LC_ReportParseError(t, "expected %s got instead %s, this happened while parsing: %s", LC_TokenKindToString(KIND), LC_TokenKindToString(t->kind), context); \
    }

#define LC_PROP_ERROR(expr, ...)              \
    expr = __VA_ARGS__;                       \
    do {                                      \
        if (expr->kind == LC_ASTKind_Error) { \
            return expr;                      \
        }                                     \
    } while (0)

// Pratt expression parser
// Based on this really good article: https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html
// clang-format off
LC_FUNCTION LC_BindingPower LC_MakeBP(int left, int right) {
    LC_BindingPower result = {left, right};
    return result;
}

LC_FUNCTION LC_BindingPower LC_GetBindingPower(LC_Binding binding, LC_TokenKind kind) {
    if (binding == LC_Binding_Prefix) goto Prefix;
    if (binding == LC_Binding_Infix) goto Infix;
    if (binding == LC_Binding_Postfix) goto Postfix;
    LC_ASSERT(NULL, !"invalid codepath");

Prefix:
    switch (kind) {
        case LC_TokenKind_OpenBracket: return LC_MakeBP(-2, 22);
        case LC_TokenKind_Mul: case LC_TokenKind_BitAnd: case LC_TokenKind_Keyword: case LC_TokenKind_OpenParen:
        case LC_TokenKind_Sub: case LC_TokenKind_Add: case LC_TokenKind_Neg: case LC_TokenKind_Not: case LC_TokenKind_OpenBrace: return LC_MakeBP(-2, 20);
        default: return LC_MakeBP(-1, -1);
    }
Infix:
    switch (kind) {
        case LC_TokenKind_Or: return LC_MakeBP(9, 10);
        case LC_TokenKind_And: return LC_MakeBP(11, 12);
        case LC_TokenKind_Equals: case LC_TokenKind_NotEquals: case LC_TokenKind_GreaterThen:
        case LC_TokenKind_GreaterThenEq: case LC_TokenKind_LesserThen: case LC_TokenKind_LesserThenEq: return LC_MakeBP(13, 14);
        case LC_TokenKind_Sub: case LC_TokenKind_Add: case LC_TokenKind_BitOr: case LC_TokenKind_BitXor: return LC_MakeBP(15, 16);
        case LC_TokenKind_RightShift: case LC_TokenKind_LeftShift: case LC_TokenKind_BitAnd:
        case LC_TokenKind_Mul: case LC_TokenKind_Div: case LC_TokenKind_Mod: return LC_MakeBP(17, 18);
        default: return LC_MakeBP(0, 0);
    }
Postfix:
    switch (kind) {
        case LC_TokenKind_Dot: case LC_TokenKind_OpenBracket: case LC_TokenKind_OpenParen: return LC_MakeBP(21, -2);
        default: return LC_MakeBP(-1, -1);
    }
}

LC_FUNCTION LC_AST *LC_ParseExprEx(int min_bp) {
    LC_AST *left = NULL;
    LC_Token *prev = LC_GetI(-1);
    LC_Token *t = LC_Next();
    LC_BindingPower prefixbp = LC_GetBindingPower(LC_Binding_Prefix, t->kind);

    // parse prefix expression
    switch (t->kind) {
        case LC_TokenKind_RawString:
        case LC_TokenKind_String: { left = LC_CreateAST(t, LC_ASTKind_ExprString); left->eatom.name = t->ident; } break;
        case LC_TokenKind_Ident: { left = LC_CreateAST(t, LC_ASTKind_ExprIdent); left->eident.name = t->ident; } break;
        case LC_TokenKind_Int: { left = LC_CreateAST(t, LC_ASTKind_ExprInt); left->eatom.i = t->i; } break;
        case LC_TokenKind_Float: { left = LC_CreateAST(t, LC_ASTKind_ExprFloat); left->eatom.d = t->f64; } break;
        case LC_TokenKind_Unicode: { left = LC_CreateAST(t, LC_ASTKind_ExprInt); left->eatom.i = t->i; } break;

        case LC_TokenKind_Keyword: {
            if (t->ident == L->kfalse) {
                left = LC_CreateAST(t, LC_ASTKind_ExprBool);
                LC_Bigint_init_signed(&left->eatom.i, false);
            } else if (t->ident == L->ktrue) {
                left = LC_CreateAST(t, LC_ASTKind_ExprBool);
                LC_Bigint_init_signed(&left->eatom.i, true);
            } else {
                return LC_ReportParseError(t, "got unexpected keyword '%s' while parsing expression", t->ident);
            }
        } break;

        case LC_TokenKind_AddPtr: {
            LC_EXPECT(open_paren, LC_TokenKind_OpenParen, "addptr");
            LC_AST *LC_PROP_ERROR(ptr, LC_ParseExprEx(0));
            LC_EXPECT(comma, LC_TokenKind_Comma, "addptr");
            LC_AST *LC_PROP_ERROR(offset, LC_ParseExprEx(0));
            LC_EXPECT(close_paren, LC_TokenKind_CloseParen, "addptr");
            left = LC_CreateBinary(t, ptr, offset, LC_TokenKind_EOF);
            left->kind = LC_ASTKind_ExprAddPtr;
        } break;

        case LC_TokenKind_Colon: {
            left = LC_CreateAST(t, LC_ASTKind_ExprType);
            LC_PROP_ERROR(left->etype.type, LC_ParseType());

            LC_Token *open = LC_Get();
            if (LC_Match(LC_TokenKind_OpenBrace)) {
                left = LC_ParseCompo(open, left);
                if (left->kind == LC_ASTKind_Error) {
                    L->parser->at = t;
                    return left;
                }
            } else if (LC_Match(LC_TokenKind_OpenParen)) {
                LC_AST *type = left;
                left = LC_CreateAST(open, LC_ASTKind_ExprCast);
                left->ecast.type = type->etype.type;
                type->kind = LC_ASTKind_Ignore;

                LC_PROP_ERROR(left->ecast.expr, LC_ParseExpr());
                LC_EXPECT(close_paren, LC_TokenKind_CloseParen, "cast expression");
            }
        } break;

        case LC_TokenKind_OpenBrace: {
            left = LC_ParseCompo(t, left);
            if (left->kind == LC_ASTKind_Error) {
                L->parser->at = prev;
                return left;
            }
        } break;

        case LC_TokenKind_Not: case LC_TokenKind_Neg: case LC_TokenKind_Add: case LC_TokenKind_Sub: {
            LC_AST *LC_PROP_ERROR(expr, LC_ParseExprEx(prefixbp.right));
            left = LC_CreateUnary(t, t->kind, expr);
        } break;

        case LC_TokenKind_BitAnd: {
            LC_AST *LC_PROP_ERROR(expr, LC_ParseExprEx(prefixbp.right));
            left = LC_CreateUnary(t, t->kind, expr);
            left->kind = LC_ASTKind_ExprGetPointerOfValue;
        } break;

        case LC_TokenKind_Mul: {
            LC_AST *LC_PROP_ERROR(expr, LC_ParseExprEx(prefixbp.right));
            left = LC_CreateUnary(t, t->kind, expr);
            left->kind = LC_ASTKind_ExprGetValueOfPointer;
        } break;

        case LC_TokenKind_OpenParen: {
            LC_AST *LC_PROP_ERROR(expr, LC_ParseExprEx(0));
            left = expr;
            LC_EXPECT(_c, LC_TokenKind_CloseParen, "expression");
        } break;

        default: return LC_ReportParseError(prev, "got invalid token: %s, while parsing expression", LC_TokenKindToString(t->kind));
    }

    for (;;) {
        t = LC_Get();

        // lets say [+] is left:1, right:2 and we parse 2+3+4
        // We pass min_bp of 2 to the next recursion
        // in recursion we check if left(1) > min_bp(2)
        // it's not so we don't recurse - we break
        // We do the for loop instead

        LC_BindingPower postfix_bp = LC_GetBindingPower(LC_Binding_Postfix, t->kind);
        LC_BindingPower infix_bp = LC_GetBindingPower(LC_Binding_Infix, t->kind);

        // parse postfix expression
        if (postfix_bp.left > min_bp) {
            LC_Next();
            switch (t->kind) {
                case LC_TokenKind_OpenBracket: {
                    LC_AST *LC_PROP_ERROR(index, LC_ParseExprEx(0));
                    LC_EXPECT(close_bracket, LC_TokenKind_CloseBracket, "index expression");
                    left = LC_CreateIndex(t, left, index);
                } break;
                case LC_TokenKind_OpenParen: {
                    LC_PROP_ERROR(left, LC_ParseCompo(t, left));
                } break;
                case LC_TokenKind_Dot: {
                    LC_EXPECT(ident, LC_TokenKind_Ident, "field access expression");
                    LC_AST *field = LC_CreateAST(t, LC_ASTKind_ExprField);
                    field->efield.left = left;
                    field->efield.right = ident->ident;
                    left = field;
                } break;
                default: {}
            }
        }

        // parse infix expression
        else if (infix_bp.left > min_bp) {
            t = LC_Next();
            LC_AST *LC_PROP_ERROR(right, LC_ParseExprEx(infix_bp.right));
            left = LC_CreateBinary(t, left, right, t->kind);
        }

        else break;
    }

    if (L->on_expr_parsed) L->on_expr_parsed(left);
    return left;
}
// clang-format on

LC_FUNCTION LC_AST *LC_ParseCompo(LC_Token *pos, LC_AST *left) {
    if (pos->kind != LC_TokenKind_OpenBrace && pos->kind != LC_TokenKind_OpenParen) {
        return LC_ReportParseError(pos, "internal compiler error: expected open brace or open paren in %s", __FUNCTION__);
    }

    LC_ASTKind   kind       = pos->kind == LC_TokenKind_OpenBrace ? LC_ASTKind_ExprCompound : LC_ASTKind_ExprCall;
    LC_TokenKind close_kind = pos->kind == LC_TokenKind_OpenBrace ? LC_TokenKind_CloseBrace : LC_TokenKind_CloseParen;
    LC_ASTKind   item_kind  = pos->kind == LC_TokenKind_OpenBrace ? LC_ASTKind_ExprCompoundItem : LC_ASTKind_ExprCallItem;

    LC_AST *n      = LC_CreateAST(pos, kind);
    n->ecompo.name = left;

    while (!LC_Is(close_kind)) {
        LC_Token *vpos = LC_Get();
        LC_AST   *v    = LC_CreateAST(vpos, item_kind);

        if (LC_Match(LC_TokenKind_OpenBracket)) {
            if (kind == LC_ASTKind_ExprCall) return LC_ReportParseError(vpos, "procedure calls cant have indexed arguments");
            LC_PROP_ERROR(v->ecompo_item.index, LC_ParseExpr());
            LC_EXPECT(close_bracket, LC_TokenKind_CloseBracket, "compo expression");
            LC_EXPECT(assign, LC_TokenKind_Assign, "compo expression");
            LC_PROP_ERROR(v->ecompo_item.expr, LC_ParseExpr());
        } else {
            LC_PROP_ERROR(v->ecompo_item.expr, LC_ParseExpr());
            if (LC_Match(LC_TokenKind_Assign)) {
                LC_AST *e = v->ecompo_item.expr;
                if (e->kind != LC_ASTKind_ExprIdent) return LC_ReportParseError(LC_GetI(-1), "named argument is required to be an identifier");
                LC_PROP_ERROR(v->ecompo_item.expr, LC_ParseExpr());
                v->ecompo_item.name = e->eident.name;
                e->kind             = LC_ASTKind_Ignore; // :FreeAST
            }
        }

        n->ecompo.size += 1;
        LC_DLLAdd(n->ecompo.first, n->ecompo.last, v);
        if (!LC_Match(LC_TokenKind_Comma)) break;
    }
    LC_EXPECT(close_token, close_kind, "compo expression");
    return n;
}

LC_FUNCTION LC_AST *LC_ParseExpr(void) {
    return LC_ParseExprEx(0);
}

LC_FUNCTION LC_AST *LC_ParseProcType(LC_Token *pos) {
    LC_AST *n = LC_CreateAST(pos, LC_ASTKind_TypespecProc);
    LC_EXPECT(open_paren, LC_TokenKind_OpenParen, "procedure typespec");
    if (!LC_Match(LC_TokenKind_CloseParen)) {
        for (;;) {
            if (LC_Match(LC_TokenKind_ThreeDots)) {
                n->tproc.vargs = true;

                LC_Token *any = LC_Get();
                if (any->kind == LC_TokenKind_Ident && any->ident == L->iAny) {
                    n->tproc.vargs_any_promotion = true;
                    LC_Next();
                }
                break;
            }
            LC_EXPECT(ident, LC_TokenKind_Ident, "procedure typespec argument list");
            LC_AST *v         = LC_CreateAST(ident, LC_ASTKind_TypespecProcArg);
            v->tproc_arg.name = ident->ident;

            LC_EXPECT(colon, LC_TokenKind_Colon, "procedure typespec argument list");
            LC_PROP_ERROR(v->tproc_arg.type, LC_ParseType());

            if (LC_Match(LC_TokenKind_Assign)) {
                LC_PROP_ERROR(v->tproc_arg.expr, LC_ParseExpr());
            }

            v->notes = LC_ParseNotes();
            LC_DLLAdd(n->tproc.first, n->tproc.last, v);

            if (!LC_Match(LC_TokenKind_Comma)) break;
        }
        LC_EXPECT(close_paren, LC_TokenKind_CloseParen, "procedure typespec argument list");
    }
    if (LC_Match(LC_TokenKind_Colon)) {
        LC_PROP_ERROR(n->tproc.ret, LC_ParseType());
    }
    return n;
}

LC_FUNCTION LC_AST *LC_ParseType(void) {
    LC_AST   *n = NULL;
    LC_Token *t = LC_Next();
    if (t->kind == LC_TokenKind_Ident) {
        n              = LC_CreateAST(t, LC_ASTKind_TypespecIdent);
        n->eident.name = t->ident;
        LC_Token *dot  = LC_Match(LC_TokenKind_Dot);
        if (dot) {
            LC_AST *field      = LC_CreateAST(t, LC_ASTKind_TypespecField);
            field->efield.left = n;
            LC_EXPECT(ident, LC_TokenKind_Ident, "field access typespec");
            field->efield.right = ident->ident;
            return field;
        }
    } else if (t->kind == LC_TokenKind_Mul) {
        n = LC_CreateAST(t, LC_ASTKind_TypespecPointer);
        LC_PROP_ERROR(n->tpointer.base, LC_ParseType());
    } else if (t->kind == LC_TokenKind_OpenBracket) {
        n = LC_CreateAST(t, LC_ASTKind_TypespecArray);
        if (!LC_Match(LC_TokenKind_CloseBracket)) {
            LC_PROP_ERROR(n->tarray.index, LC_ParseExpr());
            LC_EXPECT(close_bracket, LC_TokenKind_CloseBracket, "array typespec");
        }
        LC_PROP_ERROR(n->tarray.base, LC_ParseType());
    } else if (t->kind == LC_TokenKind_Keyword && t->ident == L->kproc) {
        LC_PROP_ERROR(n, LC_ParseProcType(t));
    } else {
        return LC_ReportParseError(t, "failed to parse typespec, invalid token %s", LC_TokenKindToString(t->kind));
    }

    if (L->on_typespec_parsed) L->on_typespec_parsed(n);
    return n;
}

LC_FUNCTION LC_AST *LC_ParseForStmt(LC_Token *pos) {
    LC_AST *n = LC_CreateAST(pos, LC_ASTKind_StmtFor);

    if (LC_Get()->kind != LC_TokenKind_OpenBrace) {
        if (!LC_Is(LC_TokenKind_Semicolon)) {
            LC_PROP_ERROR(n->sfor.init, LC_ParseStmt(false));
            if (n->sfor.init->kind == LC_ASTKind_StmtExpr) {
                n->sfor.cond       = n->sfor.init->sexpr.expr;
                n->sfor.init->kind = LC_ASTKind_Ignore; // :FreeAST
                n->sfor.init       = NULL;
                goto skip_to_last;
            } else if (n->sfor.init->kind != LC_ASTKind_StmtVar && n->sfor.init->kind != LC_ASTKind_StmtAssign) {
                return LC_ReportParseError(n->sfor.init->pos, "invalid for loop syntax, expected variable intializer or assignment");
            }
        }

        if (LC_Match(LC_TokenKind_Semicolon) && !LC_Is(LC_TokenKind_Semicolon)) {
            LC_PROP_ERROR(n->sfor.cond, LC_ParseExpr());
        }

    skip_to_last:;
        if (LC_Match(LC_TokenKind_Semicolon)) {
            LC_PROP_ERROR(n->sfor.inc, LC_ParseStmt(false));
            if (n->sfor.inc->kind != LC_ASTKind_StmtAssign && n->sfor.inc->kind != LC_ASTKind_StmtExpr) {
                return LC_ReportParseError(n->sfor.inc->pos, "invalid for loop syntax, expected assignment or expression");
            }
        }
    }

    LC_PROP_ERROR(n->sfor.body, LC_ParseStmtBlock(ParseStmtBlock_AllowSingleStmt));
    n->sfor.body->sblock.kind = SBLK_Loop;
    return n;
}

LC_FUNCTION LC_AST *LC_ParseSwitchStmt(LC_Token *pos) {
    LC_AST *n = LC_CreateAST(pos, LC_ASTKind_StmtSwitch);
    LC_PROP_ERROR(n->sswitch.expr, LC_ParseExpr());
    LC_EXPECT(open_brace, LC_TokenKind_OpenBrace, "switch statement");
    for (;;) {
        LC_Token *pos = LC_Get();
        if (LC_MatchKeyword(L->kcase)) {
            LC_AST *v = LC_CreateAST(pos, LC_ASTKind_StmtSwitchCase);
            do {
                LC_AST *LC_PROP_ERROR(expr, LC_ParseExpr());
                LC_DLLAdd(v->scase.first, v->scase.last, expr);
                n->sswitch.total_switch_case_count += 1;
            } while (LC_Match(LC_TokenKind_Comma));
            LC_EXPECT(colon, LC_TokenKind_Colon, "switch statement case");
            LC_PROP_ERROR(v->scase.body, LC_ParseStmtBlock(ParseStmtBlock_AllowSingleStmt));
            v->notes = LC_ParseNotes();
            LC_DLLAdd(n->sswitch.first, n->sswitch.last, v);
        } else if (LC_MatchKeyword(L->kdefault)) {
            LC_EXPECT(colon, LC_TokenKind_Colon, "switch statement default case");
            LC_AST *v = LC_CreateAST(pos, LC_ASTKind_StmtSwitchDefault);
            LC_PROP_ERROR(v->scase.body, LC_ParseStmtBlock(ParseStmtBlock_AllowSingleStmt));
            LC_EXPECT(close_brace, LC_TokenKind_CloseBrace, "switch statement default case");
            v->notes = LC_ParseNotes();
            LC_DLLAdd(n->sswitch.first, n->sswitch.last, v);
            break;
        } else {
            return LC_ReportParseError(LC_Get(), "invalid token while parsing switch statement");
        }

        if (LC_Match(LC_TokenKind_EOF)) return LC_ReportParseError(pos, "Unclosed '}' switch stmt, reached end of file");
        if (LC_Match(LC_TokenKind_CloseBrace)) break;
    }
    return n;
}

LC_FUNCTION LC_AST *LC_ParseStmt(bool check_semicolon) {
    LC_AST   *n    = 0;
    LC_Token *pos  = LC_Get();
    LC_Token *pos1 = LC_GetI(1);
    if (LC_MatchKeyword(L->kreturn)) {
        n = LC_CreateAST(pos, LC_ASTKind_StmtReturn);
        if (LC_Get()->kind != LC_TokenKind_Semicolon) {
            LC_PROP_ERROR(n->sreturn.expr, LC_ParseExpr());
        }
    }

    else if (LC_MatchKeyword(L->kbreak)) {
        n               = LC_CreateAST(pos, LC_ASTKind_StmtBreak);
        LC_Token *ident = LC_Match(LC_TokenKind_Ident);
        if (ident) n->sbreak.name = ident->ident;
    }

    else if (LC_MatchKeyword(L->kcontinue)) {
        n               = LC_CreateAST(pos, LC_ASTKind_StmtContinue);
        LC_Token *ident = LC_Match(LC_TokenKind_Ident);
        if (ident) n->scontinue.name = ident->ident;
    }

    else if (LC_MatchKeyword(L->kdefer)) {
        check_semicolon = false;
        n               = LC_CreateAST(pos, LC_ASTKind_StmtDefer);
        LC_PROP_ERROR(n->sdefer.body, LC_ParseStmtBlock(ParseStmtBlock_AllowSingleStmt));
        n->sdefer.body->sblock.kind = SBLK_Defer;
    }

    else if (LC_MatchKeyword(L->kfor)) {
        LC_PROP_ERROR(n, LC_ParseForStmt(pos));
        check_semicolon = false;
    }

    else if (LC_MatchKeyword(L->kswitch)) {
        LC_PROP_ERROR(n, LC_ParseSwitchStmt(pos));
        check_semicolon = false;
    }

    else if (LC_MatchKeyword(L->kif)) {
        n = LC_CreateAST(pos, LC_ASTKind_StmtIf);
        LC_PROP_ERROR(n->sif.expr, LC_ParseExpr());
        LC_PROP_ERROR(n->sif.body, LC_ParseStmtBlock(ParseStmtBlock_AllowSingleStmt));
        for (;;) {
            if (!LC_MatchKeyword(L->kelse)) break;

            LC_AST *v = LC_CreateAST(LC_GetI(-1), LC_ASTKind_StmtElse);
            if (LC_MatchKeyword(L->kif)) {
                v->kind = LC_ASTKind_StmtElseIf;
                LC_PROP_ERROR(v->sif.expr, LC_ParseExpr());
            }
            LC_PROP_ERROR(v->sif.body, LC_ParseStmtBlock(ParseStmtBlock_AllowSingleStmt));
            LC_DLLAdd(n->sif.first, n->sif.last, v);
        }
        check_semicolon = false;
    }

    else if (LC_Match(LC_TokenKind_Hash)) { // #c(``);
        n = LC_CreateAST(LC_Get(), LC_ASTKind_StmtNote);
        LC_PROP_ERROR(n->snote.expr, LC_ParseNote());
    } else if (pos->kind == LC_TokenKind_OpenBrace) { // { block }
        n               = LC_ParseStmtBlock(0);
        check_semicolon = false;
    }

    else if (pos->kind == LC_TokenKind_Ident && pos1->kind == LC_TokenKind_Colon) { // Name: ...
        LC_Next();
        LC_Next();

        if (LC_MatchKeyword(L->kfor)) {
            LC_PROP_ERROR(n, LC_ParseForStmt(LC_GetI(-1)));
            n->sfor.body->sblock.name = pos->ident;
            check_semicolon           = false;
        } else {
            n              = LC_CreateAST(pos, LC_ASTKind_StmtVar);
            LC_Intern name = pos->ident;
            if (LC_Match(LC_TokenKind_Assign)) {
                LC_PROP_ERROR(n->svar.expr, LC_ParseExpr());
                n->svar.name = name;
            } else if (LC_Match(LC_TokenKind_Colon)) {
                n->kind = LC_ASTKind_StmtConst;
                LC_PROP_ERROR(n->sconst.expr, LC_ParseExpr());
                n->sconst.name = name;
            } else {
                n->svar.name = name;
                LC_PROP_ERROR(n->svar.type, LC_ParseType());
                if (LC_Match(LC_TokenKind_Assign)) {
                    if (LC_Match(LC_TokenKind_Hash)) {
                        LC_AST *note = LC_CreateAST(LC_Get(), LC_ASTKind_ExprNote);
                        LC_PROP_ERROR(note->enote.expr, LC_ParseNote());
                        n->svar.expr = note;
                    } else {
                        LC_PROP_ERROR(n->svar.expr, LC_ParseExpr());
                    }
                }
            }
        }
    } else {
        n = LC_CreateAST(pos, LC_ASTKind_StmtExpr);
        LC_PROP_ERROR(n->sexpr.expr, LC_ParseExpr());

        LC_Token *t = LC_Get();
        if (LC_IsAssign(t->kind)) {
            LC_Next();
            LC_AST *left = n->sexpr.expr;

            n->kind = LC_ASTKind_StmtAssign;
            LC_PROP_ERROR(n->sassign.right, LC_ParseExpr());
            n->sassign.left = left;
            n->sassign.op   = t->kind;
        }
    }

    if (check_semicolon) {
        if (!LC_Match(LC_TokenKind_Semicolon)) return LC_ReportParseError(LC_GetI(-1), "statement lacks a semicolon at the end");
    }

    n->notes = LC_ParseNotes();
    return n;
}

LC_FUNCTION LC_AST *LC_ParseStmtBlock(int flags) {
    LC_AST *n = LC_CreateAST(LC_Get(), LC_ASTKind_StmtBlock);

    bool single_stmt = false;
    if (flags & ParseStmtBlock_AllowSingleStmt) {
        if (!LC_Is(LC_TokenKind_OpenBrace)) {
            LC_AST *LC_PROP_ERROR(v, LC_ParseStmt(true));
            LC_DLLAdd(n->sblock.first, n->sblock.last, v);
            single_stmt = true;
        }
    }

    if (!single_stmt) {
        LC_EXPECT(open_brace, LC_TokenKind_OpenBrace, "statement block");
        if (!LC_Match(LC_TokenKind_CloseBrace)) {
            for (;;) {
                LC_AST *v = LC_ParseStmt(true);

                // Eat until next statement in case of error
                if (v->kind == LC_ASTKind_Error) {
                    for (;;) {
                        if (LC_Is(LC_TokenKind_EOF) || LC_Is(LC_TokenKind_OpenBrace) || LC_Match(LC_TokenKind_CloseBrace)) return v;
                        if (LC_Match(LC_TokenKind_Semicolon)) break;
                        LC_Next();
                    }
                }

                if (L->on_stmt_parsed) L->on_stmt_parsed(v);
                LC_DLLAdd(n->sblock.first, n->sblock.last, v);
                if (LC_Match(LC_TokenKind_EOF)) return LC_ReportParseError(open_brace, "Unclosed '}' stmt list, reached end of file");
                if (LC_Match(LC_TokenKind_CloseBrace)) break;
            }
        }
    }

    if (L->on_stmt_parsed) L->on_stmt_parsed(n);
    return n;
}

LC_FUNCTION LC_AST *LC_ParseProcDecl(LC_Token *name) {
    LC_AST *n     = LC_CreateAST(name, LC_ASTKind_DeclProc);
    n->dbase.name = name->ident;
    LC_PROP_ERROR(n->dproc.type, LC_ParseProcType(name));

    LC_Token *ob = LC_Get();
    if (ob->kind == LC_TokenKind_OpenBrace) {
        // Here I added additional error handling which slows down compilation a bit.
        // We can for sure deduce where procs end and where they begin because of the syntaxes
        // nature - so to avoid any error spills from one procedure to another and I
        // seek for the last brace of procedure and set 'end' on parser to 1 after that token.
        LC_Token *cb              = ob;
        LC_Token *last_open_brace = ob;
        int       pair_counter    = 0;

        // Seek for the last '}' close brace of procedure
        for (;;) {
            LC_Token *d = LC_GetI(3);
            if (LC_GetI(0)->kind == LC_TokenKind_Ident && LC_GetI(1)->kind == LC_TokenKind_Colon && LC_GetI(2)->kind == LC_TokenKind_Colon && d->kind == LC_TokenKind_Keyword) {
                if (d->ident == L->kproc || d->ident == L->kstruct || d->ident == L->kunion || d->ident == L->ktypedef) {
                    break;
                }
            }

            LC_Token *token = LC_Next();
            if (token == &L->NullToken) break;
            if (token->kind == LC_TokenKind_OpenBrace) pair_counter += 1;
            if (token->kind == LC_TokenKind_OpenBrace) last_open_brace = token;
            if (token->kind == LC_TokenKind_CloseBrace) pair_counter -= 1;
            if (token->kind == LC_TokenKind_CloseBrace) cb = token;
        }
        if (pair_counter != 0) return LC_ReportParseError(last_open_brace, "unclosed open brace '{' inside this procedure");
        L->parser->at = ob;

        // Set the parsing boundary to one after the last close brace
        LC_Token *save_end = L->parser->end;
        L->parser->end     = cb + 1;
        n->dproc.body      = LC_ParseStmtBlock(0);
        L->parser->end     = save_end;
        if (n->dproc.body->kind == LC_ASTKind_Error) return n->dproc.body;

        n->dproc.body->sblock.kind = SBLK_Proc;
    } else {
        LC_EXPECT(semicolon, LC_TokenKind_Semicolon, "procedure declaration");
    }

    return n;
}

LC_FUNCTION LC_AST *LC_ParseStruct(LC_ASTKind kind, LC_Token *ident) {
    LC_AST *n     = LC_CreateAST(ident, kind);
    n->dbase.name = ident->ident;
    LC_EXPECT(open_brace, LC_TokenKind_OpenBrace, "struct declaration");
    for (;;) {
        LC_AST *v = LC_CreateAST(ident, LC_ASTKind_TypespecAggMem);
        LC_EXPECT(ident, LC_TokenKind_Ident, "struct member");
        v->tagg_mem.name = ident->ident;
        LC_EXPECT(colon, LC_TokenKind_Colon, "struct member");
        LC_PROP_ERROR(v->tagg_mem.type, LC_ParseType());
        LC_EXPECT(semicolon, LC_TokenKind_Semicolon, "struct member");

        v->notes = LC_ParseNotes();
        LC_DLLAdd(n->dagg.first, n->dagg.last, v);
        if (LC_Match(LC_TokenKind_EOF)) return LC_ReportParseError(ident, "Unclosed '}' struct, reached end of file");
        if (LC_Match(LC_TokenKind_CloseBrace)) break;
    }

    return n;
}

LC_FUNCTION LC_AST *LC_ParseTypedef(LC_Token *ident) {
    LC_AST *n     = LC_CreateAST(ident, LC_ASTKind_DeclTypedef);
    n->dbase.name = ident->ident;
    LC_PROP_ERROR(n->dtypedef.type, LC_ParseType());
    if (!LC_Match(LC_TokenKind_Semicolon)) return LC_ReportParseError(LC_GetI(-1), "expected semicolon ';' after typedef declaration, got instead %s", LC_TokenKindToString(LC_GetI(-1)->kind));
    return n;
}

LC_FUNCTION LC_AST *LC_CreateNote(LC_Token *pos, LC_Intern ident) {
    LC_AST *n = LC_CreateAST(pos, LC_ASTKind_Note);

    LC_AST *astident      = LC_CreateAST(pos, LC_ASTKind_ExprIdent);
    astident->eident.name = ident;
    n->ecompo.name        = astident;

    return n;
}

LC_FUNCTION LC_AST *LC_ParseNote(void) {
    LC_AST *n = NULL;

    // syntactic sugar
    // #`stuff` => #c(`stuff`)
    LC_Token *str_token = LC_Match(LC_TokenKind_RawString);
    if (str_token) {
        n = LC_CreateNote(str_token, L->ic);

        // Add CallItem
        {
            LC_AST *astcallitem                       = LC_CreateAST(str_token, LC_ASTKind_ExprCallItem);
            astcallitem->ecompo_item.expr             = LC_CreateAST(str_token, LC_ASTKind_ExprString);
            astcallitem->ecompo_item.expr->eatom.name = str_token->ident;
            LC_DLLAdd(n->ecompo.first, n->ecompo.last, astcallitem);
        }
    } else {

        LC_EXPECT(ident, LC_TokenKind_Ident, "note");
        if (!LC_IsNoteDeclared(ident->ident)) {
            LC_ReportParseError(ident, "unregistered note name: '%s'", ident->ident);
        }

        LC_AST *astident      = LC_CreateAST(ident, LC_ASTKind_ExprIdent);
        astident->eident.name = ident->ident;

        LC_Token *open_paren = LC_Match(LC_TokenKind_OpenParen);
        if (open_paren) {
            n = LC_ParseCompo(open_paren, astident);
        } else {
            n = LC_CreateAST(ident, LC_ASTKind_Note);
        }
        n->ecompo.name = astident;
        n->kind        = LC_ASTKind_Note;
    }
    return n;
}

LC_FUNCTION LC_AST *LC_ParseNotes(void) {
    LC_Token *pos   = LC_Get();
    LC_AST   *first = 0;
    LC_AST   *last  = 0;
    for (;;) {
        LC_Token *t = LC_Match(LC_TokenKind_Note);
        if (!t) break;
        LC_AST *n = LC_ParseNote();
        if (n->kind == LC_ASTKind_Error) continue;
        LC_DLLAdd(first, last, n);
    }

    if (first) {
        LC_AST *n           = LC_CreateAST(pos, LC_ASTKind_NoteList);
        n->anote_list.first = first;
        n->anote_list.last  = last;
        return n;
    }
    return 0;
}

LC_FUNCTION bool LC_ResolveBuildIf(LC_AST *build_if) {
    LC_ExprCompo *note = &build_if->anote;
    if (note->size != 1) {
        LC_ReportParseError(LC_GetI(-1), "invalid argument count for #build_if directive, expected 1, got %d", note->size);
        return true;
    }

    LC_ExprCompoItem *item = &note->first->ecompo_item;
    if (item->index != NULL || item->name != 0) {
        LC_ReportParseError(LC_GetI(-1), "invalid syntax, #build_if shouldn't have a named or indexed first argument");
        return true;
    }

    LC_PUSH_PACKAGE(L->builtin_package);
    LC_Operand op = LC_ResolveExpr(item->expr);
    LC_POP_PACKAGE();
    if (!LC_IsUTConst(op)) {
        LC_ReportParseError(LC_GetI(-1), "expected #build_if to have an untyped constant expcession");
        return true;
    }
    if (!LC_IsUTInt(op.type)) {
        LC_ReportParseError(LC_GetI(-1), "expected #build_if to have expression of type untyped int");
        return true;
    }

    int64_t result = LC_Bigint_as_signed(&op.v.i);
    return (bool)result;
}

LC_FUNCTION LC_AST *LC_ParseDecl(LC_AST *file) {
    LC_AST   *n           = 0;
    LC_Token *doc_comment = LC_Match(LC_TokenKind_DocComment);
    LC_Token *ident       = LC_Get();

    if (LC_Match(LC_TokenKind_Ident)) {
        if (LC_Match(LC_TokenKind_Colon)) {
            if (LC_Match(LC_TokenKind_Colon)) {
                if (LC_MatchKeyword(L->kproc)) {
                    LC_PROP_ERROR(n, LC_ParseProcDecl(ident));
                } else if (LC_MatchKeyword(L->kstruct)) {
                    LC_PROP_ERROR(n, LC_ParseStruct(LC_ASTKind_DeclStruct, ident));
                } else if (LC_MatchKeyword(L->kunion)) {
                    LC_PROP_ERROR(n, LC_ParseStruct(LC_ASTKind_DeclUnion, ident));
                } else if (LC_MatchKeyword(L->ktypedef)) {
                    LC_PROP_ERROR(n, LC_ParseTypedef(ident));
                } else {
                    n             = LC_CreateAST(ident, LC_ASTKind_DeclConst);
                    n->dbase.name = ident->ident;
                    if (LC_Match(LC_TokenKind_BitXor)) {
                        LC_AST *last_decl = file->afile.ldecl;
                        if (!last_decl || last_decl->kind != LC_ASTKind_DeclConst) return LC_ReportParseError(LC_GetI(-1), "invalid usage, there is no constant declaration preceding '^', this operator implies - PREV_CONST + 1");
                        LC_AST *left      = LC_CreateAST(n->pos, LC_ASTKind_ExprIdent);
                        left->eident.name = last_decl->dbase.name;
                        LC_AST *right     = LC_CreateAST(n->pos, LC_ASTKind_ExprInt);
                        right->eatom.i    = LC_Bigint_u64(1);

                        n->dconst.expr = LC_CreateBinary(n->pos, left, right, LC_TokenKind_Add);
                    } else if (LC_Match(LC_TokenKind_LeftShift)) {
                        LC_AST *last_decl = file->afile.ldecl;
                        if (!last_decl || last_decl->kind != LC_ASTKind_DeclConst) return LC_ReportParseError(LC_GetI(-1), "invalid usage, there is no constant declaration preceding '^', this operator implies - PREV_CONST << 1");
                        LC_AST *left      = LC_CreateAST(n->pos, LC_ASTKind_ExprIdent);
                        left->eident.name = last_decl->dbase.name;
                        LC_AST *right     = LC_CreateAST(n->pos, LC_ASTKind_ExprInt);
                        right->eatom.i    = LC_Bigint_u64(1);

                        n->dconst.expr = LC_CreateBinary(n->pos, left, right, LC_TokenKind_LeftShift);
                    } else {
                        LC_PROP_ERROR(n->dconst.expr, LC_ParseExpr());
                    }

                    if (!LC_Match(LC_TokenKind_Semicolon)) return LC_ReportParseError(LC_GetI(-1), "expected ';' semicolon");
                }
            } else if (LC_Match(LC_TokenKind_Assign)) {
                n = LC_CreateAST(ident, LC_ASTKind_DeclVar);
                LC_PROP_ERROR(n->dvar.expr, LC_ParseExpr());
                n->dbase.name = ident->ident;
                if (!LC_Match(LC_TokenKind_Semicolon)) return LC_ReportParseError(LC_GetI(-1), "expected ';' semicolon");
            } else {
                n             = LC_CreateAST(ident, LC_ASTKind_DeclVar);
                n->dbase.name = ident->ident;

                LC_PROP_ERROR(n->dvar.type, LC_ParseType());
                if (LC_Match(LC_TokenKind_Assign)) {
                    if (LC_Match(LC_TokenKind_Hash)) {
                        LC_AST *note = LC_CreateAST(LC_Get(), LC_ASTKind_ExprNote);
                        LC_PROP_ERROR(note->enote.expr, LC_ParseNote());
                        n->dvar.expr = note;
                    } else {
                        LC_PROP_ERROR(n->dvar.expr, LC_ParseExpr());
                    }
                }
                if (!LC_Match(LC_TokenKind_Semicolon)) return LC_ReportParseError(LC_GetI(-1), "expected ';' semicolon");
            }
        } else return LC_ReportParseError(ident, "got unexpected token: %s, while parsing declaration", LC_TokenKindToString(ident->kind));
    } else if (LC_Match(LC_TokenKind_Hash)) {
        n = LC_CreateAST(ident, LC_ASTKind_DeclNote);
        LC_PROP_ERROR(n->dnote.expr, LC_ParseNote());
        if (!LC_Match(LC_TokenKind_Semicolon)) return LC_ReportParseError(LC_GetI(-1), "expected ';' semicolon");
    } else if (LC_MatchKeyword(L->kimport)) {
        return LC_ReportParseError(LC_Get(), "imports can only appear at the top level");
    } else if (ident->kind == LC_TokenKind_EOF) return NULL;
    else return LC_ReportParseError(ident, "got unexpected token: %s, while parsing declaration", LC_TokenKindToString(ident->kind));

    LC_AST *notes = LC_ParseNotes();
    if (n) {
        n->notes             = notes;
        n->dbase.doc_comment = doc_comment;
    }
    return n;
}

LC_FUNCTION bool LC_EatUntilNextValidDecl(void) {
    for (;;) {
        LC_Token *a = LC_GetI(0);
        if (a->kind == LC_TokenKind_Keyword && a->ident == L->kimport) {
            return true;
        }

        LC_Token *d = LC_GetI(3);
        if (a->kind == LC_TokenKind_Ident && LC_GetI(1)->kind == LC_TokenKind_Colon && LC_GetI(2)->kind == LC_TokenKind_Colon && d->kind == LC_TokenKind_Keyword) {
            if (d->ident == L->kproc || d->ident == L->kstruct || d->ident == L->kunion || d->ident == L->ktypedef) {
                return false;
            }
        }

        LC_Token *token = LC_Next();
        if (token == &L->NullToken) {
            return false;
        }
    }
}

LC_FUNCTION bool LC_ParseHashBuildOn(LC_AST *n) {
    LC_Token *t0 = LC_GetI(0);
    LC_Token *t1 = LC_GetI(1);
    if (t0->kind == LC_TokenKind_Hash && t1->kind == LC_TokenKind_Ident && t1->ident == L->ibuild_if) {
        LC_Next();

        LC_AST *build_if     = LC_CreateAST(t1, LC_ASTKind_DeclNote);
        build_if->dnote.expr = LC_ParseNote();
        if (build_if->dnote.expr->kind == LC_ASTKind_Error) {
            LC_EatUntilNextValidDecl();
            return true;
        }

        if (!LC_Match(LC_TokenKind_Semicolon)) {
            LC_ReportParseError(LC_GetI(-1), "expected ';' semicolon");
            LC_EatUntilNextValidDecl();
            return true;
        }

        LC_AST *note_list = LC_CreateAST(t0, LC_ASTKind_NoteList);
        LC_DLLAdd(note_list->anote_list.first, note_list->anote_list.last, build_if);
        n->notes = note_list;

        return LC_ResolveBuildIf(build_if->dnote.expr);
    }
    return true;
}

LC_FUNCTION LC_AST *LC_ParseImport(void) {
    LC_AST   *n      = NULL;
    LC_Token *import = LC_MatchKeyword(L->kimport);
    if (import) {
        n = LC_CreateAST(import, LC_ASTKind_GlobImport);

        LC_Token *ident = LC_Match(LC_TokenKind_Ident);
        if (ident) n->gimport.name = ident->ident;

        LC_Token *path = LC_Match(LC_TokenKind_String);
        if (!path) return LC_ReportParseError(LC_GetI(-1), "expected string after an import, instead got %s", LC_TokenKindToString(LC_Get()->kind));

        n->gimport.path = path->ident;
        if (!LC_Match(LC_TokenKind_Semicolon)) return LC_ReportParseError(LC_GetI(-1), "expected ';' semicolon");
    }
    return n;
}

LC_FUNCTION void LC_AddFileToPackage(LC_AST *pkg, LC_AST *f) {
    f->afile.package = pkg;
    LC_DLLAdd(pkg->apackage.ffile, pkg->apackage.lfile, f);
}

LC_FUNCTION LC_AST *LC_ParseFileEx(LC_AST *package) {
    LC_Token *package_doc_comment = LC_Match(LC_TokenKind_PackageDocComment);

    LC_AST *n            = LC_CreateAST(LC_Get(), LC_ASTKind_File);
    n->afile.x           = L->parser->x;
    n->afile.doc_comment = LC_Match(LC_TokenKind_FileDocComment);
    n->afile.build_if    = LC_ParseHashBuildOn(n);

    // Parse imports
    while (!LC_Is(LC_TokenKind_EOF)) {
        LC_AST *import = LC_ParseImport();
        if (!import) break;

        if (import->kind == LC_ASTKind_Error) {
            bool is_import = LC_EatUntilNextValidDecl();
            if (!is_import) break;
        } else {
            LC_DLLAdd(n->afile.fimport, n->afile.limport, import);
        }
    }

    // Parse top level decls
    while (!LC_Is(LC_TokenKind_EOF)) {
        LC_AST *decl = LC_ParseDecl(n);
        if (!decl) continue;

        if (decl->kind == LC_ASTKind_Error) {
            LC_EatUntilNextValidDecl();
        } else {
            bool skip = false;

            LC_AST *build_if = LC_HasNote(decl, L->ibuild_if);
            if (build_if) {
                skip = !LC_ResolveBuildIf(build_if);
            }

            if (L->on_decl_parsed) {
                skip = L->on_decl_parsed(skip, decl);
            }

            if (skip) {
                LC_DLLAdd(n->afile.fdiscarded, n->afile.ldiscarded, decl);
            } else {
                LC_DLLAdd(n->afile.fdecl, n->afile.ldecl, decl);
            }
        }
    }

    if (package) {
        if (package->apackage.doc_comment) LC_ReportParseError(package_doc_comment, "there are more then 1 package doc comments in %s package", (char *)package->apackage.name);
        package->apackage.doc_comment = package_doc_comment;

        if (n->afile.build_if) {
            LC_AddFileToPackage(package, n);
        } else {
            LC_DLLAdd(package->apackage.fdiscarded, package->apackage.ldiscarded, n);
            n->afile.package = package;
        }
    }

    return n;
}

LC_FUNCTION LC_AST *LC_ParseTokens(LC_AST *package, LC_Lex *x) {
    LC_Parser p  = LC_MakeParser(x);
    L->parser    = &p;
    LC_AST *file = LC_ParseFileEx(package);
    return L->errors ? NULL : file;
}

LC_FUNCTION LC_AST *LC_ParseFile(LC_AST *package, char *filename, char *content, int line) {
    if (content == NULL) {
        LC_SendErrorMessagef(NULL, NULL, "internal compiler error: file passed to %s is null", __FUNCTION__);
        return NULL;
    }
    if (filename == NULL) {
        LC_SendErrorMessagef(NULL, NULL, "internal compiler error: filename passed to %s is null", __FUNCTION__);
        return NULL;
    }

    LC_Lex *x = LC_LexStream(filename, content, line);
    if (L->errors) return NULL;
    LC_InternTokens(x);

    LC_AST *file = LC_ParseTokens(package, x);
    if (!file) return NULL;
    return file;
}

LC_FUNCTION LC_AST *LC_ParseStmtf(const char *str, ...) {
    LC_FORMAT(L->arena, str, s8);
    LC_Parser *old    = L->parser;
    LC_Parser *p      = LC_MakeParserQuick(s8.str);
    LC_AST    *result = LC_ParseStmt(false);
    L->parser         = old;
    return result;
}

LC_FUNCTION LC_AST *LC_ParseExprf(const char *str, ...) {
    LC_FORMAT(L->arena, str, s8);
    LC_Parser *old    = L->parser;
    LC_Parser *p      = LC_MakeParserQuick(s8.str);
    LC_AST    *result = LC_ParseExpr();
    L->parser         = old;
    return result;
}

LC_FUNCTION LC_AST *LC_ParseDeclf(const char *str, ...) {
    LC_FORMAT(L->arena, str, s8);
    LC_Parser *old    = L->parser;
    LC_Parser *p      = LC_MakeParserQuick(s8.str);
    LC_AST    *result = LC_ParseDecl(&L->NullAST);
    L->parser         = old;
    return result;
}

#undef LC_EXPECT
#undef LC_PROP_ERROR
LC_FUNCTION LC_StringList *LC_BeginStringGen(LC_Arena *arena) {
    L->printer.list          = LC_MakeEmptyList();
    L->printer.arena         = arena;
    L->printer.last_filename = 0;
    L->printer.last_line_num = 0;
    L->printer.indent        = 0;
    return &L->printer.list;
}

LC_FUNCTION LC_String LC_EndStringGen(LC_Arena *arena) {
    LC_String result = LC_MergeString(arena, L->printer.list);
    return result;
}

LC_FUNCTION void LC_GenIndent(void) {
    LC_String s = LC_Lit("    ");
    for (int i = 0; i < L->printer.indent; i++) {
        LC_AddNode(L->printer.arena, &L->printer.list, s);
    }
}

LC_FUNCTION char *LC_Strf(const char *str, ...) {
    LC_FORMAT(L->arena, str, s8);
    return s8.str;
}

LC_FUNCTION void LC_GenLine(void) {
    LC_Genf("\n");
    LC_GenIndent();
}

LC_FUNCTION char *LC_GenLCType(LC_Type *type) {
    LC_StringList out = {0};
    for (LC_Type *it = type; it;) {
        if (it->kind == LC_TypeKind_Pointer) {
            LC_Addf(L->arena, &out, "*");
            it = it->tptr.base;
        } else if (it->kind == LC_TypeKind_Array) {
            LC_Addf(L->arena, &out, "[%d]", it->tarray.size);
            it = it->tarray.base;
        } else if (it->kind == LC_TypeKind_Proc) {
            LC_Addf(L->arena, &out, "proc(");
            LC_TypeFor(mem, it->tproc.args.first) {
                LC_Addf(L->arena, &out, "%s: %s", (char *)mem->name, LC_GenLCType(mem->type));
                if (mem->default_value_expr) LC_Addf(L->arena, &out, "/*has default value*/");
                if (mem->next) LC_Addf(L->arena, &out, ", ");
            }
            if (it->tproc.vargs) LC_Addf(L->arena, &out, "..");
            LC_Addf(L->arena, &out, ")");
            if (it->tproc.ret->kind != LC_TypeKind_void) LC_Addf(L->arena, &out, ": %s", LC_GenLCType(it->tproc.ret));
            break;
        } else if (it->decl) {
            LC_Decl *decl = it->decl;
            LC_ASSERT(decl->ast, decl);
            LC_Addf(L->arena, &out, "%s", (char *)decl->name);
            break;
        } else {
            LC_SendErrorMessagef(NULL, NULL, "internal compiler error: unhandled type kind in %s", __FUNCTION__);
        }
    }
    LC_String s = LC_MergeString(L->arena, out);
    return s.str;
}

LC_FUNCTION char *LC_GenLCTypeVal(LC_TypeAndVal v) {
    if (LC_IsInt(v.type) || LC_IsPtr(v.type) || LC_IsProc(v.type)) {
        return LC_Bigint_str(&v.i, 10);
    }
    if (LC_IsFloat(v.type)) {
        LC_String s = LC_Format(L->arena, "%f", v.d);
        return s.str;
    }
    LC_ASSERT(NULL, !"invalid codepath");
    return "";
}

LC_FUNCTION char *LC_GenLCAggName(LC_Type *t) {
    if (t->kind == LC_TypeKind_Struct) return "struct";
    if (t->kind == LC_TypeKind_Union) return "union";
    return NULL;
}

LC_FUNCTION void LC_GenLCNode(LC_AST *n) {
    switch (n->kind) {
    case LC_ASTKind_Package: {
        LC_ASTFor(it, n->apackage.ffile) {
            LC_GenLCNode(it);
        }
    } break;

    case LC_ASTKind_File: {
        LC_ASTFor(it, n->afile.fimport) {
            LC_GenLCNode(it);
        }

        LC_ASTFor(it, n->afile.fdecl) {
            LC_GenLCNode(it);
        }
        // @todo: we need to do something with notes so we can generate them in order!

    } break;

    case LC_ASTKind_GlobImport: {
        LC_GenLinef("import %s \"%s\";", (char *)n->gimport.name, (char *)n->gimport.path);
    } break;

    case LC_ASTKind_DeclProc: {
        LC_GenLinef("%s :: ", (char *)n->dbase.name);
        LC_GenLCNode(n->dproc.type);
        if (n->dproc.body) {
            LC_Genf(" ");
            LC_GenLCNode(n->dproc.body);
        } else {
            LC_Genf(";");
        }
    } break;

    case LC_ASTKind_DeclUnion:
    case LC_ASTKind_DeclStruct: {
        const char *agg = n->kind == LC_ASTKind_DeclUnion ? "union" : "struct";
        LC_GenLinef("%s :: %s {", (char *)n->dbase.name, agg);
        L->printer.indent += 1;
        LC_ASTFor(it, n->dagg.first) {
            LC_GenLine();
            LC_GenLCNode(it);
            LC_Genf(";");
        }
        L->printer.indent -= 1;
        LC_GenLinef("}");
    } break;

    case LC_ASTKind_TypespecAggMem: {
        LC_Genf("%s: ", (char *)n->tagg_mem.name);
        LC_GenLCNode(n->tagg_mem.type);
    } break;

    case LC_ASTKind_DeclVar: {
        LC_GenLinef("%s ", (char *)n->dbase.name);
        if (n->dvar.type) {
            LC_Genf(": ");
            LC_GenLCNode(n->dvar.type);
            if (n->dvar.expr) {
                LC_Genf("= ");
                LC_GenLCNode(n->dvar.expr);
            }
        } else {
            LC_Genf(":= ");
            LC_GenLCNode(n->dvar.expr);
        }
        LC_Genf(";");
    } break;

    case LC_ASTKind_DeclConst: {
        LC_GenLinef("%s :: ", (char *)n->dbase.name);
        LC_GenLCNode(n->dconst.expr);
        LC_Genf(";");
    } break;

    case LC_ASTKind_DeclTypedef: {
        LC_GenLinef("%s :: typedef ", (char *)n->dbase.name);
        LC_GenLCNode(n->dtypedef.type);
        LC_Genf(";");
    } break;

    case LC_ASTKind_ExprIdent:
    case LC_ASTKind_TypespecIdent: {
        LC_Genf("%s", (char *)n->eident.name);
    } break;

    case LC_ASTKind_ExprField:
    case LC_ASTKind_TypespecField: {
        LC_GenLCNode(n->efield.left);
        LC_Genf(".%s", (char *)n->efield.right);
    } break;

    case LC_ASTKind_TypespecPointer: {
        LC_Genf("*");
        LC_GenLCNode(n->tpointer.base);
    } break;

    case LC_ASTKind_TypespecArray: {
        LC_Genf("[");
        if (n->tarray.index) LC_GenLCNode(n->tarray.index);
        LC_Genf("]");
        LC_GenLCNode(n->tpointer.base);
    } break;

    case LC_ASTKind_TypespecProc: {
        LC_Genf("proc(");
        LC_ASTFor(it, n->tproc.first) {
            LC_GenLCNode(it);
            if (it != n->tproc.last) LC_Genf(", ");
        }
        if (n->tproc.vargs) {
            LC_Genf(", ...");
            if (n->tproc.vargs_any_promotion) LC_Genf("Any");
        }
        LC_Genf(")");
        if (n->tproc.ret) {
            LC_Genf(": ");
            LC_GenLCNode(n->tproc.ret);
        }
    } break;

    case LC_ASTKind_TypespecProcArg: {
        LC_Genf("%s: ", (char *)n->tproc_arg.name);
        LC_GenLCNode(n->tproc_arg.type);
        if (n->tproc_arg.expr) {
            LC_Genf(" = ");
            LC_GenLCNode(n->tproc_arg.expr);
        }
    } break;

    case LC_ASTKind_StmtBlock: {
        if (n->sblock.name && n->sblock.kind != SBLK_Loop) LC_Genf("%s: ", (char *)n->sblock.name);
        LC_Genf("{");
        L->printer.indent += 1;
        LC_ASTFor(it, n->sblock.first) {
            LC_GenLine();
            LC_GenLCNode(it);
            if (it->kind != LC_ASTKind_StmtBlock && it->kind != LC_ASTKind_StmtDefer && it->kind != LC_ASTKind_StmtFor && it->kind != LC_ASTKind_StmtIf && it->kind != LC_ASTKind_StmtSwitch) LC_Genf(";");
        }
        L->printer.indent -= 1;
        LC_GenLinef("}");
    } break;

    case LC_ASTKind_StmtReturn: {
        LC_Genf("return");
        if (n->sreturn.expr) {
            LC_Genf(" ");
            LC_GenLCNode(n->sreturn.expr);
        }
    } break;

    case LC_ASTKind_StmtBreak: {
        LC_Genf("break");
        if (n->sbreak.name) LC_Genf(" %s", (char *)n->sbreak.name);
    } break;

    case LC_ASTKind_StmtContinue: {
        LC_Genf("continue");
        if (n->scontinue.name) LC_Genf(" %s", (char *)n->scontinue.name);
    } break;

    case LC_ASTKind_StmtDefer: {
        LC_Genf("defer ");
        LC_GenLCNode(n->sdefer.body);
    } break;

    case LC_ASTKind_StmtFor: {
        LC_StmtBlock *sblock = &n->sfor.body->sblock;
        if (sblock->name && sblock->kind == SBLK_Loop) {
            LC_Genf("%s: ", (char *)sblock->name);
        }

        LC_Genf("for ");
        if (n->sfor.init) {
            LC_GenLCNode(n->sfor.init);
            if (n->sfor.cond) LC_Genf("; ");
        }

        if (n->sfor.cond) {
            LC_GenLCNode(n->sfor.cond);
            if (n->sfor.inc) {
                LC_Genf("; ");
                LC_GenLCNode(n->sfor.inc);
            }
        }

        LC_Genf(" ");
        LC_GenLCNode(n->sfor.body);
    } break;

    case LC_ASTKind_StmtElseIf:
        LC_Genf("else ");
    case LC_ASTKind_StmtIf: {
        LC_Genf("if ");
        LC_GenLCNode(n->sif.expr);
        LC_GenLCNode(n->sif.body);
        LC_ASTFor(it, n->sif.first) {
            LC_GenLCNode(it);
        }
    } break;

    case LC_ASTKind_StmtElse: {
        LC_Genf("else ");
        LC_GenLCNode(n->sif.body);
    } break;

    case LC_ASTKind_StmtSwitch: {
        LC_Genf("switch ");
        LC_GenLCNode(n->sswitch.expr);
        LC_Genf("{");
        L->printer.indent += 1;
        LC_ASTFor(it, n->sswitch.first) {
            LC_GenLine();
            LC_GenLCNode(it);
        }
        L->printer.indent -= 1;
        LC_Genf("}");
    } break;

    case LC_ASTKind_StmtSwitchCase: {
        LC_Genf("case ");
        LC_ASTFor(it, n->scase.first) {
            LC_GenLCNode(it);
            if (it != n->scase.last) LC_Genf(", ");
        }
        LC_Genf(": ");
        LC_GenLCNode(n->scase.body);
    } break;
    case LC_ASTKind_StmtSwitchDefault: {
        LC_Genf("default: ");
        LC_GenLCNode(n->scase.body);
    } break;

    case LC_ASTKind_StmtAssign: {
        LC_GenLCNode(n->sassign.left);
        LC_Genf(" %s ", LC_TokenKindToOperator(n->sassign.op));
        LC_GenLCNode(n->sassign.right);
    } break;

    case LC_ASTKind_StmtExpr: {
        LC_GenLCNode(n->sexpr.expr);
    } break;

    case LC_ASTKind_StmtVar: {
        LC_Genf("%s", (char *)n->svar.name);
        if (n->svar.type) {
            LC_Genf(": ");
            LC_GenLCNode(n->svar.type);
            if (n->svar.expr) {
                LC_Genf(" = ");
                LC_GenLCNode(n->svar.expr);
            }
        } else {
            LC_Genf(" := ");
            LC_GenLCNode(n->svar.expr);
        }
    } break;

    case LC_ASTKind_StmtConst: {
        LC_GenLinef("%s :: ", (char *)n->sconst.name);
        LC_GenLCNode(n->sconst.expr);
    } break;

    case LC_ASTKind_ExprString: {
        LC_Genf("`%s`", (char *)n->eatom.name);
    } break;

    case LC_ASTKind_ExprInt: {
        LC_Genf("%s", LC_Bigint_str(&n->eatom.i, 10));
    } break;

    case LC_ASTKind_ExprFloat: {
        LC_Genf("%f", n->eatom.d);
    } break;

    case LC_ASTKind_ExprBool: {
        int64_t value = LC_Bigint_as_unsigned(&n->eatom.i);
        if (value) {
            LC_Genf("true");
        } else {
            LC_Genf("false");
        }
    } break;

    case LC_ASTKind_ExprType: {
        LC_Genf(":");
        LC_GenLCNode(n->etype.type);
    } break;

    case LC_ASTKind_ExprBinary: {
        LC_Genf("(");
        LC_GenLCNode(n->ebinary.left);
        LC_Genf("%s", LC_TokenKindToOperator(n->ebinary.op));
        LC_GenLCNode(n->ebinary.right);
        LC_Genf(")");
    } break;

    case LC_ASTKind_ExprUnary: {
        LC_Genf("%s(", LC_TokenKindToOperator(n->eunary.op));
        LC_GenLCNode(n->eunary.expr);
        LC_Genf(")");
    } break;

    case LC_ASTKind_StmtNote: {
        LC_Genf("#");
        LC_GenLCNode(n->snote.expr);
    } break;

    case LC_ASTKind_ExprNote: {
        LC_Genf("#");
        LC_GenLCNode(n->enote.expr);
    } break;

    case LC_ASTKind_DeclNote: {
        LC_GenLinef("#");
        LC_GenLCNode(n->dnote.expr);
        LC_Genf(";");
    } break;

    case LC_ASTKind_Note:
    case LC_ASTKind_ExprBuiltin:
    case LC_ASTKind_ExprCall: {
        LC_GenLCNode(n->ecompo.name);
        LC_Genf("(");
        LC_ASTFor(it, n->ecompo.first) {
            LC_GenLCNode(it);
            if (it != n->ecompo.last) LC_Genf(", ");
        }
        LC_Genf(")");
    } break;

    case LC_ASTKind_ExprCompoundItem:
    case LC_ASTKind_ExprCallItem: {
        if (n->ecompo_item.name) {
            LC_Genf("%s = ", (char *)n->ecompo_item.name);
        }
        if (n->ecompo_item.index) {
            LC_Genf("[");
            LC_GenLCNode(n->ecompo_item.index);
            LC_Genf("] = ");
        }
        LC_GenLCNode(n->ecompo_item.expr);
    } break;

    case LC_ASTKind_ExprCompound: {
        if (n->ecompo.name) LC_GenLCNode(n->ecompo.name);
        LC_Genf("{");
        LC_ASTFor(it, n->ecompo.first) {
            LC_GenLCNode(it);
            if (it != n->ecompo.last) LC_Genf(", ");
        }
        LC_Genf("}");
    } break;

    case LC_ASTKind_ExprCast: {
        LC_Genf(":");
        LC_GenLCNode(n->ecast.type);
        LC_Genf("(");
        LC_GenLCNode(n->ecast.expr);
        LC_Genf(")");
    } break;

    case LC_ASTKind_ExprIndex: {
        LC_Genf("(");
        LC_GenLCNode(n->eindex.base);
        LC_Genf("[");
        LC_GenLCNode(n->eindex.index);
        LC_Genf("]");
        LC_Genf(")");
    } break;

    case LC_ASTKind_ExprAddPtr: {
        LC_Genf("addptr(");
        LC_GenLCNode(n->ebinary.left);
        LC_Genf(", ");
        LC_GenLCNode(n->ebinary.right);
        LC_Genf(")");
    } break;

    case LC_ASTKind_ExprGetValueOfPointer: {
        LC_Genf("*(");
        LC_GenLCNode(n->eunary.expr);
        LC_Genf(")");
    } break;

    case LC_ASTKind_ExprGetPointerOfValue: {
        LC_Genf("&(");
        LC_GenLCNode(n->eunary.expr);
        LC_Genf(")");
    } break;

    default: LC_ReportASTError(n, "internal compiler error: unhandled ast kind in %s", __FUNCTION__);
    }
}

const bool LC_GenCInternalGenerateSizeofs = true;

LC_FUNCTION void LC_GenCLineDirective(LC_AST *node) {
    if (L->emit_line_directives) {
        L->printer.last_line_num = node->pos->line;
        LC_GenLinef("#line %d", L->printer.last_line_num);
        LC_Intern file = node->pos->lex->file;
        if (file != L->printer.last_filename) {
            L->printer.last_filename = file;
            LC_Genf(" \"%s\"", (char *)L->printer.last_filename);
        }
    }
}

LC_FUNCTION void LC_GenLastCLineDirective(void) {
    if (L->emit_line_directives) {
        LC_Genf("#line %d", L->printer.last_line_num);
    }
}

LC_FUNCTION void LC_GenCLineDirectiveNum(int num) {
    if (L->emit_line_directives) {
        LC_Genf("#line %d", num);
    }
}

LC_FUNCTION char *LC_GenCTypeParen(char *str, char c) {
    return c && c != '[' ? LC_Strf("(%s)", str) : str;
}

LC_FUNCTION char *LC_GenCType(LC_Type *type, char *str) {
    switch (type->kind) {
    case LC_TypeKind_Pointer: {
        return LC_GenCType(type->tptr.base, LC_GenCTypeParen(LC_Strf("*%s", str), *str));
    } break;
    case LC_TypeKind_Array: {
        if (type->tarray.size == 0) {
            return LC_GenCType(type->tarray.base, LC_GenCTypeParen(LC_Strf("%s[]", str), *str));
        } else {
            return LC_GenCType(type->tarray.base, LC_GenCTypeParen(LC_Strf("%s[%d]", str, type->tarray.size), *str));
        }

    } break;
    case LC_TypeKind_Proc: {
        LC_StringList out = {0};
        LC_Addf(L->arena, &out, "(*%s)", str);
        LC_Addf(L->arena, &out, "(");
        if (type->tagg.mems.count == 0) {
            LC_Addf(L->arena, &out, "void");
        } else {
            int i = 0;
            for (LC_TypeMember *it = type->tproc.args.first; it; it = it->next) {
                LC_Addf(L->arena, &out, "%s%s", i == 0 ? "" : ", ", LC_GenCType(it->type, ""));
                i += 1;
            }
        }
        if (type->tproc.vargs) {
            LC_Addf(L->arena, &out, ", ...");
        }
        LC_Addf(L->arena, &out, ")");
        char *front  = LC_MergeString(L->arena, out).str;
        char *result = LC_GenCType(type->tproc.ret, front);
        return result;
    } break;
    default: return LC_Strf("%s%s%s", type->decl->foreign_name, str[0] ? " " : "", str);
    }
}

LC_FUNCTION LC_Intern LC_GetStringFromSingleArgNote(LC_AST *note) {
    LC_ASSERT(note, note->kind == LC_ASTKind_Note);
    LC_ASSERT(note, note->ecompo.first == note->ecompo.last);
    LC_AST *arg = note->ecompo.first;
    LC_ASSERT(note, arg->kind == LC_ASTKind_ExprCallItem);
    LC_AST *str = arg->ecompo_item.expr;
    LC_ASSERT(note, str->kind == LC_ASTKind_ExprString);
    return str->eatom.name;
}

LC_FUNCTION void LC_GenCCompound(LC_AST *n) {
    LC_Type *type = n->type;
    if (LC_IsAggType(type)) {
        LC_ResolvedCompo *rd = n->ecompo.resolved_items;
        LC_Genf("{");
        if (rd->first == NULL) LC_Genf("0");
        for (LC_ResolvedCompoItem *it = rd->first; it; it = it->next) {
            LC_Genf(".%s = ", (char *)it->t->name);
            LC_GenCExpr(it->expr);
            if (it->next) LC_Genf(", ");
        }
        LC_Genf("}");
    } else if (LC_IsArray(type)) {
        LC_ResolvedArrayCompo *rd = n->ecompo.resolved_array_items;
        LC_Genf("{");
        for (LC_ResolvedCompoArrayItem *it = rd->first; it; it = it->next) {
            LC_Genf("[%d] = ", it->index);
            LC_GenCExpr(it->comp->ecompo_item.expr);
            if (it->next) LC_Genf(", ");
        }
        LC_Genf("}");
    } else {
        LC_ReportASTError(n, "internal compiler error: got unhandled case in %s", __FUNCTION__);
    }
}

LC_THREAD_LOCAL bool GC_SpecialCase_GlobalScopeStringDecl;

LC_FUNCTION void LC_GenCString(char *s, LC_Type *type) {
    if (type == L->tstring) {
        if (!GC_SpecialCase_GlobalScopeStringDecl) LC_Genf("(LC_String)");
        LC_Genf("{ ");
    }
    LC_Genf("\"");
    for (int i = 0; s[i]; i += 1) {
        LC_String escape = LC_GetEscapeString(s[i]);
        if (escape.len) {
            LC_Genf("%.*s", LC_Expand(escape));
        } else {
            LC_Genf("%c", s[i]);
        }
    }
    LC_Genf("\"");
    if (type == L->tstring) LC_Genf(", %d }", (int)LC_StrLen(s));
}

LC_FUNCTION char *LC_GenCVal(LC_TypeAndVal v, LC_Type *type) {
    char *str = LC_GenLCTypeVal(v);
    switch (type->kind) {
    case LC_TypeKind_uchar:
    case LC_TypeKind_ushort:
    case LC_TypeKind_uint: str = LC_Strf("%su", str); break;
    case LC_TypeKind_ulong: str = LC_Strf("%sul", str); break;
    case LC_TypeKind_Pointer:
    case LC_TypeKind_Proc:
    case LC_TypeKind_ullong: str = LC_Strf("%sull", str); break;
    case LC_TypeKind_long: str = LC_Strf("%sull", str); break;
    case LC_TypeKind_llong: str = LC_Strf("%sull", str); break;
    case LC_TypeKind_float: str = LC_Strf("%sf", str); break;
    case LC_TypeKind_UntypedFloat: str = LC_Strf(" /*utfloat*/%s", str); break;
    case LC_TypeKind_UntypedInt: str = LC_Strf(" /*utint*/%sull", str); break;
    default: {
    }
    }
    if (LC_IsUTInt(v.type) && !LC_IsUntyped(type) && type->size < 4) {
        str = LC_Strf("(%s)%s", LC_GenCType(type, ""), str);
    }
    return str;
}

LC_FUNCTION void LC_GenCExpr(LC_AST *n) {
    LC_ASSERT(n, LC_IsExpr(n));
    intptr_t is_any = (intptr_t)LC_MapGetP(&L->implicit_any, n);
    if (is_any) LC_Genf("(LC_Any){%d, (%s[]){", n->type->id, LC_GenCType(n->type, ""));

    if (n->const_val.type) {
        bool contains_sizeof_like = LC_GenCInternalGenerateSizeofs ? LC_ContainsCBuiltin(n) : false;
        if (!contains_sizeof_like) {
            if (LC_IsUTStr(n->const_val.type)) {
                LC_GenCString((char *)n->const_val.name, n->type);
            } else {
                char *val = LC_GenCVal(n->const_val, n->type);
                LC_Genf("%s", val);
            }
            if (is_any) LC_Genf("}}");
            return;
        }
    }

    LC_Type *type = n->type;
    switch (n->kind) {
    case LC_ASTKind_ExprIdent: {
        LC_Genf("%s", (char *)n->eident.resolved_decl->foreign_name);
    } break;

    case LC_ASTKind_ExprCast: {
        LC_Genf("(");
        LC_Genf("(%s)", LC_GenCType(type, ""));
        LC_Genf("(");
        LC_GenCExpr(n->ecast.expr);
        LC_Genf(")");
        LC_Genf(")");
    } break;

    case LC_ASTKind_ExprUnary: {
        LC_Genf("%s(", LC_TokenKindToOperator(n->eunary.op));
        LC_GenCExpr(n->eunary.expr);
        LC_Genf(")");
    } break;

    case LC_ASTKind_ExprAddPtr: {
        LC_Genf("(");
        LC_GenCExpr(n->ebinary.left);
        LC_Genf("+");
        LC_GenCExpr(n->ebinary.right);
        LC_Genf(")");
    } break;

    case LC_ASTKind_ExprBinary: {
        LC_Genf("(");
        LC_GenCExpr(n->ebinary.left);
        LC_Genf("%s", LC_TokenKindToOperator(n->ebinary.op));
        LC_GenCExpr(n->ebinary.right);
        LC_Genf(")");
    } break;

    case LC_ASTKind_ExprIndex: {
        LC_Genf("(");
        LC_GenCExpr(n->eindex.base);
        LC_Genf("[");
        LC_GenCExpr(n->eindex.index);
        LC_Genf("]");
        LC_Genf(")");
    } break;

    case LC_ASTKind_ExprGetValueOfPointer: {
        LC_Genf("(*(");
        LC_GenCExpr(n->eunary.expr);
        LC_Genf("))");
    } break;

    case LC_ASTKind_ExprGetPointerOfValue: {
        LC_Genf("(&(");
        LC_GenCExpr(n->eunary.expr);
        LC_Genf("))");
    } break;

    case LC_ASTKind_ExprField: {
        if (n->efield.parent_decl->kind != LC_DeclKind_Import) {
            LC_Type *left_type = n->efield.left->type;
            LC_GenCExpr(n->efield.left);
            if (LC_IsPtr(left_type)) LC_Genf("->");
            else LC_Genf(".");
            LC_Genf("%s", (char *)n->efield.right);
        } else {
            LC_Genf("%s", (char *)n->efield.resolved_decl->foreign_name);
        }
    } break;

    case LC_ASTKind_ExprCall: {
        LC_ResolvedCompo *rd = n->ecompo.resolved_items;
        LC_GenCExpr(n->ecompo.name);
        LC_Genf("(");
        for (LC_ResolvedCompoItem *it = rd->first; it; it = it->next) {
            LC_GenCExpr(it->expr);
            if (it->next) LC_Genf(", ");
        }
        LC_Genf(")");
    } break;

    case LC_ASTKind_ExprCompound: {
        LC_Genf("(%s)", LC_GenCType(type, ""));
        LC_GenCCompound(n);
    } break;

    case LC_ASTKind_ExprBuiltin: {
        LC_ASSERT(n, n->ecompo.name->kind == LC_ASTKind_ExprIdent);
        if (n->ecompo.name->eident.name == L->isizeof) {
            LC_Genf("sizeof(");
            LC_AST *expr = n->ecompo.first->ecompo_item.expr;
            if (expr->kind == LC_ASTKind_ExprType) {
                LC_Genf("%s", LC_GenCType(expr->type, ""));
            } else {
                LC_GenCExpr(expr);
            }
            LC_Genf(")");
        } else if (n->ecompo.name->eident.name == L->ialignof) {
            LC_Genf("LC_Alignof(");
            LC_AST *expr = n->ecompo.first->ecompo_item.expr;
            if (expr->kind == LC_ASTKind_ExprType) {
                LC_Genf("%s", LC_GenCType(expr->type, ""));
            } else {
                LC_GenCExpr(expr);
            }
            LC_Genf(")");
        } else if (n->ecompo.name->eident.name == L->ioffsetof) {
            LC_AST *i1 = n->ecompo.first->ecompo_item.expr;
            LC_AST *i2 = n->ecompo.first->next->ecompo_item.expr;
            LC_Genf("offsetof(%s, %s)", LC_GenCType(i1->type, ""), (char *)i2->eident.name);
        } else {
            LC_ReportASTError(n, "internal compiler error: got unhandled case in %s / LC_ASTKind_ExprBuiltin", __FUNCTION__);
        }
    } break;

    default: LC_ReportASTError(n, "internal compiler error: got unhandled case in %s", __FUNCTION__);
    }

    if (is_any) LC_Genf("}}");
}

const int GC_Stmt_OmitSemicolonAndNewLine = 1;

LC_FUNCTION void LC_GenCNote(LC_AST *note) {
    if (note->ecompo.name->eident.name == L->ic) {
        LC_Genf("%s", (char *)LC_GetStringFromSingleArgNote(note));
    }
}

LC_FUNCTION void LC_GenCVarExpr(LC_AST *n, bool is_declaration) {
    if (LC_HasNote(n, L->inot_init)) return;

    LC_AST *e = n->dvar.expr;
    if (n->kind == LC_ASTKind_StmtVar) e = n->svar.expr;
    if (e) {
        LC_Genf(" = ");
        if (e->kind == LC_ASTKind_ExprNote) {
            LC_GenCNote(e->enote.expr);
        } else if (is_declaration && e->kind == LC_ASTKind_ExprCompound) {
            LC_GenCCompound(e);
        } else {
            LC_GenCExpr(e);
        }
    } else {
        LC_Genf(" = {0}");
    }
}

LC_FUNCTION void LC_GenCDefers(LC_AST *block) {
    LC_AST *first = block->sblock.first_defer;
    if (first == NULL) return;

    int save = L->printer.last_line_num;
    LC_GenLine();
    LC_GenLastCLineDirective();

    LC_GenLinef("/*defer*/");
    for (LC_AST *it = first; it; it = it->sdefer.next) {
        LC_GenCStmtBlock(it->sdefer.body);
    }

    L->printer.last_line_num = save + 1;
    LC_GenLine();
    LC_GenLastCLineDirective();
}

LC_FUNCTION void LC_GenCDefersLoopBreak(LC_AST *n) {
    LC_ASSERT(n, n->kind == LC_ASTKind_StmtBreak || n->kind == LC_ASTKind_StmtContinue);
    LC_AST *it = NULL;
    for (int i = L->printer.out_block_stack.len - 1; i >= 0; i -= 1) {
        it = L->printer.out_block_stack.data[i];
        LC_GenCDefers(it);
        LC_ASSERT(it, it->sblock.kind != SBLK_Proc);
        if (it->sblock.kind == SBLK_Loop) {
            if (!n->sbreak.name) break;
            if (n->sbreak.name && it->sblock.name == n->sbreak.name) break;
        }
    }
    LC_ASSERT(it, it->sblock.kind == SBLK_Loop);
}

LC_FUNCTION void LC_GenCDefersReturn(LC_AST *n) {
    LC_ASSERT(n, n->kind == LC_ASTKind_StmtReturn);
    LC_AST *it = NULL;
    for (int i = L->printer.out_block_stack.len - 1; i >= 0; i -= 1) {
        it = L->printer.out_block_stack.data[i];
        LC_GenCDefers(it);
        if (it->sblock.kind == SBLK_Proc) {
            break;
        }
    }
    LC_ASSERT(it, it);
    LC_ASSERT(it, it->sblock.kind == SBLK_Proc);
}

LC_FUNCTION void LC_GenCStmt2(LC_AST *n, int flags) {
    LC_ASSERT(n, LC_IsStmt(n));
    bool semicolon = !(flags & GC_Stmt_OmitSemicolonAndNewLine);

    if (semicolon) {
        LC_GenLine();
    }

    switch (n->kind) {
    case LC_ASTKind_StmtVar: {
        LC_Type *type = n->type;
        LC_Genf("%s", LC_GenCType(type, (char *)n->svar.name));
        LC_GenCVarExpr(n, true);
    } break;
    case LC_ASTKind_StmtExpr: LC_GenCExpr(n->sexpr.expr); break;

    case LC_ASTKind_StmtAssign: {
        // Assigning to array doesn't work in C so we need to handle that
        // specific compo case here. :CompoArray
        if (LC_IsArray(n->type) && n->sassign.right->kind == LC_ASTKind_ExprCompound) {
            LC_ASSERT(n, n->sassign.op == LC_TokenKind_Assign);
            LC_AST *expr = n->sassign.right;
            LC_Genf("memset(");
            LC_GenCExpr(n->sassign.left);
            LC_Genf(", 0, sizeof(");
            LC_GenCExpr(n->sassign.left);
            LC_Genf("));");

            LC_ResolvedArrayCompo *rd = expr->ecompo.resolved_array_items;
            for (LC_ResolvedCompoArrayItem *it = rd->first; it; it = it->next) {
                LC_GenCExpr(n->sassign.left);
                LC_Genf("[%d] = ", it->index);
                LC_GenCExpr(it->comp->ecompo_item.expr);
                LC_Genf(";");
            }

        } else {
            LC_GenCExpr(n->sassign.left);
            LC_Genf(" %s ", LC_TokenKindToOperator(n->sassign.op));
            LC_GenCExpr(n->sassign.right);
        }
    } break;
    default: LC_ReportASTError(n, "internal compiler error: got unhandled case in %s", __FUNCTION__);
    }

    if (semicolon) LC_Genf(";");
}

LC_FUNCTION void LC_GenCStmt(LC_AST *n) {
    LC_ASSERT(n, LC_IsStmt(n));
    LC_GenCLineDirective(n);
    switch (n->kind) {
    case LC_ASTKind_StmtConst:
    case LC_ASTKind_StmtDefer: break;
    case LC_ASTKind_StmtNote: {
        LC_GenLine();
        LC_GenCNote(n->snote.expr);
        LC_Genf(";");
    } break;

    case LC_ASTKind_StmtReturn: {
        LC_GenCDefersReturn(n);
        LC_GenLinef("return");
        if (n->sreturn.expr) {
            LC_Genf(" ");
            LC_GenCExpr(n->sreturn.expr);
        }
        LC_Genf(";");
    } break;

    case LC_ASTKind_StmtContinue:
    case LC_ASTKind_StmtBreak: {
        const char *stmt = n->kind == LC_ASTKind_StmtBreak ? "break" : "continue";
        LC_GenCDefersLoopBreak(n);
        if (n->sbreak.name) {
            LC_GenLinef("goto %s_%s;", (char *)n->sbreak.name, stmt);
        } else {
            LC_GenLinef("%s;", stmt);
        }
    } break;

    case LC_ASTKind_StmtBlock: {
        LC_GenLinef("/*block*/");
        LC_GenCStmtBlock(n);
    } break;

    case LC_ASTKind_StmtSwitch: {
        LC_GenLinef("switch(");
        LC_GenCExpr(n->sswitch.expr);
        LC_Genf(") {");

        L->printer.indent += 1;
        LC_ASTFor(it, n->sswitch.first) {
            LC_GenCLineDirective(it);
            if (it->kind == LC_ASTKind_StmtSwitchCase) {
                LC_ASTFor(label_it, it->scase.first) {
                    LC_GenLinef("case ");
                    LC_GenCExpr(label_it);
                    LC_Genf(":");
                }
            }
            if (it->kind == LC_ASTKind_StmtSwitchDefault) {
                LC_GenLinef("default:");
            }
            LC_GenCStmtBlock(it->scase.body);
            if (LC_HasNote(it, L->ifallthrough)) {
                LC_Genf(" /*@fallthough*/");
            } else {
                LC_Genf(" break;");
            }
        }
        L->printer.indent -= 1;
        LC_GenLinef("}");
    } break;

    case LC_ASTKind_StmtFor: {
        LC_GenLinef("for (");
        if (n->sfor.init) LC_GenCStmt2(n->sfor.init, GC_Stmt_OmitSemicolonAndNewLine);
        LC_Genf(";");
        if (n->sfor.cond) {
            LC_Genf(" ");
            LC_GenCExpr(n->sfor.cond);
        }
        LC_Genf(";");
        if (n->sfor.inc) {
            LC_Genf(" ");
            LC_GenCStmt2(n->sfor.inc, GC_Stmt_OmitSemicolonAndNewLine);
        }
        LC_Genf(")");
        LC_GenCStmtBlock(n->sfor.body);
    } break;

    case LC_ASTKind_StmtIf: {
        LC_GenLinef("if ");
        LC_GenCExprParen(n->sif.expr);
        LC_GenCStmtBlock(n->sif.body);
        LC_ASTFor(it, n->sif.first) {
            LC_GenCLineDirective(it);
            LC_GenLinef("else");
            if (it->kind == LC_ASTKind_StmtElseIf) {
                LC_Genf(" if ");
                LC_GenCExprParen(it->sif.expr);
            }
            LC_GenCStmtBlock(it->sif.body);
        }
    } break;

    default: LC_GenCStmt2(n, 0);
    }
}

LC_FUNCTION void LC_GenCExprParen(LC_AST *expr) {
    bool paren = expr->kind != LC_ASTKind_ExprBinary;
    if (paren) LC_Genf("(");
    LC_GenCExpr(expr);
    if (paren) LC_Genf(")");
}

LC_FUNCTION void LC_GenCStmtBlock(LC_AST *n) {
    LC_PushAST(&L->printer.out_block_stack, n);
    LC_ASSERT(n, n->kind == LC_ASTKind_StmtBlock);
    LC_Genf(" {");
    L->printer.indent += 1;
    LC_ASTFor(it, n->sblock.first) {
        LC_GenCStmt(it);
    }
    LC_GenCDefers(n);
    if (n->sblock.name) LC_GenLinef("%s_continue:;", (char *)n->sblock.name);
    L->printer.indent -= 1;
    LC_GenLinef("}");
    if (n->sblock.name) LC_GenLinef("%s_break:;", (char *)n->sblock.name);
    LC_PopAST(&L->printer.out_block_stack);
}

LC_FUNCTION void LC_GenCProcDecl(LC_Decl *decl) {
    LC_StringList out      = {0};
    LC_Type      *type     = decl->type;
    LC_AST       *n        = decl->ast;
    LC_AST       *typespec = n->dproc.type;

    LC_Addf(L->arena, &out, "%s(", (char *)decl->foreign_name);
    if (type->tagg.mems.count == 0) {
        LC_Addf(L->arena, &out, "void");
    } else {
        int i = 0;
        LC_ASTFor(it, typespec->tproc.first) {
            LC_Type *type = it->type;
            LC_Addf(L->arena, &out, "%s%s", i == 0 ? "" : ", ", LC_GenCType(type, (char *)it->tproc_arg.name));
            i += 1;
        }
    }
    if (type->tproc.vargs) {
        LC_Addf(L->arena, &out, ", ...");
    }
    LC_Addf(L->arena, &out, ")");
    char *front  = LC_MergeString(L->arena, out).str;
    char *result = LC_GenCType(type->tproc.ret, front);

    LC_GenLine();
    bool is_public = LC_HasNote(n, L->iapi) || decl->foreign_name == L->imain;
    if (!is_public) LC_Genf("static ");
    LC_Genf("%s", result);
}

LC_FUNCTION void LC_GenCAggForwardDecl(LC_Decl *decl) {
    LC_ASSERT(decl->ast, LC_IsAgg(decl->ast));
    char *agg = LC_GenLCAggName(decl->type);
    LC_GenLinef("typedef %s %s %s;", agg, (char *)decl->foreign_name, (char *)decl->foreign_name);
}

LC_FUNCTION void LC_GenCTypeDecl(LC_Decl *decl) {
    LC_AST *n = decl->ast;
    LC_ASSERT(n, decl->kind == LC_DeclKind_Type);
    if (n->kind == LC_ASTKind_DeclTypedef) {
        LC_Type *type = decl->typedef_renamed_type_decl ? decl->typedef_renamed_type_decl->type : decl->type;
        LC_GenLinef("typedef %s;", LC_GenCType(type, (char *)decl->foreign_name));
    } else {
        LC_Type  *type = decl->type;
        LC_Intern name = decl->foreign_name;
        {
            bool packed = LC_HasNote(n, L->ipacked) ? true : false;
            if (packed) LC_GenLinef("#pragma pack(push, 1)");

            LC_GenLinef("%s %s {", LC_GenLCAggName(type), name ? (char *)name : "");
            L->printer.indent += 1;
            for (LC_TypeMember *it = type->tagg.mems.first; it; it = it->next) {
                LC_GenLinef("%s;", LC_GenCType(it->type, (char *)it->name));
            }
            L->printer.indent -= 1;
            LC_GenLinef("};");
            if (packed) LC_GenLinef("#pragma pack(pop)");
            LC_GenLine();
        }
    }
}

LC_FUNCTION void LC_GenCVarFDecl(LC_Decl *decl) {
    if (!LC_HasNote(decl->ast, L->iapi)) return;
    LC_Type *type = decl->type; // make string arrays assignable
    LC_GenLinef("extern ");
    if (LC_HasNote(decl->ast, L->ithread_local)) LC_Genf("_Thread_local ");
    LC_Genf("%s;", LC_GenCType(type, (char *)decl->foreign_name));
}

LC_FUNCTION void LC_GenCHeader(LC_AST *package) {
    // C notes
    LC_ASTFor(file, package->apackage.ffile) {
        LC_ASTFor(it, file->afile.fdecl) {
            if (it->kind != LC_ASTKind_DeclNote) continue;

            LC_AST *note = it->dnote.expr;
            if (note->ecompo.name->eident.name == L->ic) {
                LC_GenLinef("%s", (char *)LC_GetStringFromSingleArgNote(note));
            }
        }
    }

    // struct forward decls
    LC_DeclFor(decl, package->apackage.first_ordered) {
        if (decl->is_foreign) continue;
        LC_AST *n = decl->ast;
        if (decl->kind == LC_DeclKind_Type && LC_IsAgg(n)) LC_GenCAggForwardDecl(decl);
    }

    // type decls
    LC_GenLine();
    LC_DeclFor(decl, package->apackage.first_ordered) {
        if (decl->is_foreign) continue;
        LC_AST *n = decl->ast;
        if (decl->kind == LC_DeclKind_Type) LC_GenCTypeDecl(decl);
    }

    // proc and var forward decls
    LC_DeclFor(decl, package->apackage.first_ordered) {
        if (decl->is_foreign) continue;
        LC_AST *n = decl->ast;
        if (decl->kind == LC_DeclKind_Var) {
            LC_GenCVarFDecl(decl);
        } else if (decl->kind == LC_DeclKind_Proc) {
            LC_GenCProcDecl(decl);
            LC_Genf(";");
        }
    }
}

LC_FUNCTION void LC_GenCImpl(LC_AST *package) {
    // implementation of vars
    LC_DeclFor(decl, package->apackage.first_ordered) {
        if (decl->kind == LC_DeclKind_Var && !decl->is_foreign) {
            LC_AST  *n    = decl->ast;
            LC_Type *type = decl->type; // make string arrays assignable
            LC_GenLine();
            if (!LC_HasNote(n, L->iapi)) LC_Genf("static ");
            if (LC_HasNote(n, L->ithread_local)) LC_Genf("_Thread_local ");
            LC_Genf("%s", LC_GenCType(type, (char *)decl->foreign_name));

            GC_SpecialCase_GlobalScopeStringDecl = true;
            LC_GenCVarExpr(n, true);
            GC_SpecialCase_GlobalScopeStringDecl = false;
            LC_Genf(";");
            LC_GenLine();
        }
    }

    // implementation of procs
    LC_DeclFor(decl, package->apackage.first_ordered) {
        LC_AST *n = decl->ast;
        if (decl->kind == LC_DeclKind_Proc && n->dproc.body && !decl->is_foreign) {
            LC_GenCLineDirective(n);
            LC_GenCProcDecl(decl);
            LC_GenCStmtBlock(n->dproc.body);
            LC_GenLine();
        }
    }
}

LC_FUNCTION void WalkAndCountDeclRefs(LC_ASTWalker *ctx, LC_AST *n) {
    LC_Decl *decl = NULL;
    if (n->kind == LC_ASTKind_ExprIdent || n->kind == LC_ASTKind_TypespecIdent) {
        if (n->eident.resolved_decl) decl = n->eident.resolved_decl;
    }
    if (n->kind == LC_ASTKind_ExprField) {
        if (n->efield.resolved_decl) decl = n->efield.resolved_decl;
    }
    if (decl) {
        LC_Map  *map_of_visits = (LC_Map *)ctx->user_data;
        intptr_t visited       = (intptr_t)LC_MapGetP(map_of_visits, decl);
        LC_MapInsertP(map_of_visits, decl, (void *)(visited + 1));
        if (visited == 0 && decl->ast->kind != LC_ASTKind_Null) {
            LC_WalkAST(ctx, decl->ast);
        }
    }
}

LC_FUNCTION LC_Map LC_CountDeclRefs(LC_Arena *arena) {
    LC_Map map = {arena};
    LC_MapReserve(&map, 512);

    LC_AST      *package = LC_GetPackageByName(L->first_package);
    LC_ASTWalker walker  = LC_GetDefaultWalker(arena, WalkAndCountDeclRefs);
    walker.user_data     = (void *)&map;
    walker.visit_notes   = true;
    LC_WalkAST(&walker, package);

    return map;
}

LC_FUNCTION void LC_RemoveUnreferencedGlobalDecls(LC_Map *map_of_visits) {
    for (LC_ASTRef *it = L->ordered_packages.first; it; it = it->next) {
        for (LC_Decl *decl = it->ast->apackage.first_ordered; decl;) {
            intptr_t ref_count = (intptr_t)LC_MapGetP(map_of_visits, decl);

            LC_Decl *remove = decl;
            decl            = decl->next;
            if (ref_count == 0 && remove->foreign_name != LC_ILit("main")) {
                LC_DLLRemove(it->ast->apackage.first_ordered, it->ast->apackage.last_ordered, remove);
            }
        }
    }
}

LC_FUNCTION void LC_ErrorOnUnreferencedLocals(LC_Map *map_of_visits) {
    LC_Decl *first = (LC_Decl *)L->decl_arena->memory.data;
    for (int i = 0; i < L->decl_count; i += 1) {
        LC_Decl *decl = first + i;
        if (decl->package == L->builtin_package) {
            continue;
        }

        intptr_t ref_count = (intptr_t)LC_MapGetP(map_of_visits, decl);
        if (ref_count == 0) {
            if (LC_IsStmt(decl->ast)) {
                if (!LC_HasNote(decl->ast, L->iunused)) LC_ReportASTError(decl->ast, "unused local variable '%s'", decl->name);
            }
        }
    }
}

LC_FUNCTION void LC_FindUnusedLocalsAndRemoveUnusedGlobalDecls(void) {
    if (L->errors) return;
    LC_TempArena check = LC_BeginTemp(L->arena);

    LC_Map map = LC_CountDeclRefs(check.arena);
    LC_ErrorOnUnreferencedLocals(&map);
    LC_RemoveUnreferencedGlobalDecls(&map);

    LC_EndTemp(check);
}
LC_FUNCTION LC_Operand LC_ImportPackage(LC_AST *import, LC_AST *dst, LC_AST *src) {
    DeclScope *dst_scope  = dst->apackage.scope;
    int        scope_size = LC_NextPow2(src->apackage.scope->len * 2 + 1);
    if (import && import->gimport.name) {
        LC_PUSH_PACKAGE(dst);
        LC_Decl *decl = LC_CreateDecl(LC_DeclKind_Import, import->gimport.name, import);
        decl->scope   = LC_CreateScope(scope_size);
        LC_PutGlobalDecl(decl);
        import->gimport.resolved_decl = decl;
        LC_POP_PACKAGE();
        dst_scope = decl->scope;
    }

    for (int i = 0; i < src->apackage.scope->cap; i += 1) {
        LC_MapEntry entry = src->apackage.scope->entries[i];
        if (entry.key != 0) {
            LC_Decl *decl = (LC_Decl *)entry.value;
            if (decl->package != src) continue;
            LC_Decl *existing = (LC_Decl *)LC_MapGetU64(dst_scope, decl->name);
            if (existing && decl->package == L->builtin_package) {
                continue;
            }
            if (existing) {
                LC_MarkDeclError(existing);
                LC_MarkDeclError(decl);
                return LC_ReportASTErrorEx(decl->ast, existing->ast, "name colission while importing '%s' into '%s', there are 2 decls with the same name '%s'", src->apackage.name, dst->apackage.name, decl->name);
            }
            LC_MapInsertU64(dst_scope, decl->name, decl);
        }
    }

    if (import && import->gimport.name) {
        LC_ASSERT(import, dst_scope->cap == scope_size);
    }

    if (import) import->gimport.resolved = true;
    return LC_OPNull;
}

LC_FUNCTION LC_Intern LC_MakePackageNameFromPath(LC_String path) {
    if (path.str[path.len - 1] == '/') path = LC_Chop(path, 1);
    LC_String s8name = LC_SkipToLastSlash(path);
    if (!LC_IsDir(L->arena, path) && LC_EndsWith(path, LC_Lit(".lc"), true)) {
        s8name = LC_ChopLastPeriod(s8name);
    }
    if (s8name.len == 0) {
        L->errors += 1;
        LC_SendErrorMessagef(NULL, NULL, "failed to extract name from path %.*s", LC_Expand(path));
        return LC_GetUniqueIntern("invalid_package_name");
    }
    LC_Intern result = LC_InternStrLen(s8name.str, (int)s8name.len);
    return result;
}

LC_FUNCTION bool LC_PackageNameValid(LC_Intern name) {
    char *str = (char *)name;
    if (LC_IsDigit(str[0])) return false;
    for (int i = 0; str[i]; i += 1) {
        bool is_valid = LC_IsIdent(str[i]) || LC_IsDigit(str[i]);
        if (!is_valid) return false;
    }
    return true;
}

LC_FUNCTION bool LC_PackageNameDuplicate(LC_Intern name) {
    LC_ASTFor(it, L->fpackage) {
        if (it->apackage.name == name) return true;
    }
    return false;
}

LC_FUNCTION void LC_AddPackageToList(LC_AST *n) {
    LC_Intern name = n->apackage.name;
    if (LC_PackageNameDuplicate(name)) {
        LC_SendErrorMessagef(NULL, NULL, "found 2 packages with the same name: '%s' / '%.*s'\n", name, LC_Expand(n->apackage.path));
        L->errors += 1;
        return;
    }
    if (!LC_PackageNameValid(name)) {
        LC_SendErrorMessagef(NULL, NULL, "invalid package name, please change the name of the package directory: '%s'\n", name);
        L->errors += 1;
        return;
    }
    LC_DLLAdd(L->fpackage, L->lpackage, n);
}

LC_FUNCTION LC_AST *LC_RegisterPackage(LC_String path) {
    LC_ASSERT(NULL, path.len != 0);
    LC_AST *n        = LC_CreateAST(NULL, LC_ASTKind_Package);
    n->apackage.name = LC_MakePackageNameFromPath(path);
    n->apackage.path = path;
    LC_AddPackageToList(n);
    return n;
}

LC_FUNCTION LC_AST *LC_FindImportInRefList(LC_ASTRefList *arr, LC_Intern path) {
    for (LC_ASTRef *it = arr->first; it; it = it->next) {
        if (it->ast->gimport.path == path) return it->ast;
    }
    return NULL;
}

LC_FUNCTION void LC_AddASTToRefList(LC_ASTRefList *refs, LC_AST *ast) {
    LC_ASTRef *ref = LC_PushStruct(L->arena, LC_ASTRef);
    ref->ast       = ast;
    LC_DLLAdd(refs->first, refs->last, ref);
}

LC_FUNCTION LC_ASTRefList LC_GetPackageImports(LC_AST *package) {
    LC_ASSERT(package, package->kind == LC_ASTKind_Package);

    LC_ASTRefList refs = {0};
    LC_ASTFor(file, package->apackage.ffile) {
        LC_ASTFor(import, file->afile.fimport) {
            LC_AST *found = LC_FindImportInRefList(&refs, import->gimport.path);
            if (found) {
                LC_ReportASTErrorEx(import, found, "duplicate import of: '%s', into package '%s'\n", import->gimport.path, package->apackage.name);
                continue;
            }
            LC_AddASTToRefList(&refs, import);
        }
    }

    return refs;
}

LC_FUNCTION void LC_RegisterPackageDir(char *dir) {
    LC_String sdir = LC_MakeFromChar(dir);
    if (!LC_IsDir(L->arena, sdir)) {
        LC_SendErrorMessagef(NULL, NULL, "dir with name '%s', doesn't exist\n", dir);
        return;
    }
    LC_AddNode(L->arena, &L->package_dirs, sdir);
}

LC_FUNCTION LC_AST *LC_GetPackageByName(LC_Intern name) {
    LC_ASTFor(it, L->fpackage) {
        if (it->apackage.name == name) return it;
    }

    LC_AST *result = NULL;
    for (LC_StringNode *it = L->package_dirs.first; it; it = it->next) {
        LC_String s    = it->string;
        LC_String path = LC_Format(L->arena, "%.*s/%s", LC_Expand(s), (char *)name);
        if (LC_IsDir(L->arena, path)) {
            if (result != NULL) {
                LC_SendErrorMessagef(NULL, NULL, "found 2 directories with the same name: '%.*s', '%.*s'\n", LC_Expand(path), LC_Expand(result->apackage.path));
                L->errors += 1;
                break;
            }
            result = LC_RegisterPackage(path);
        }
    }

    return result;
}

LC_FUNCTION LC_StringList LC_ListFilesInPackage(LC_Arena *arena, LC_String path) {
    LC_StringList result = LC_MakeEmptyList();
    for (LC_FileIter it = LC_IterateFiles(arena, path); LC_IsValid(it); LC_Advance(&it)) {
        if (LC_EndsWith(it.absolute_path, LC_Lit(".lc"), LC_IgnoreCase)) {
            LC_AddNode(arena, &result, it.absolute_path);
        }
    }
    return result;
}

LC_FUNCTION LoadedFile LC_ReadFileHook(LC_AST *package, LC_String path) {
    LoadedFile result = {path};
    if (L->on_file_load) {
        L->on_file_load(package, &result);
    } else {
        result.content = LC_ReadFile(L->arena, result.path);
    }

    return result;
}

LC_FUNCTION void LC_ParsePackage(LC_AST *n) {
    LC_ASSERT(n, n->kind == LC_ASTKind_Package);
    LC_ASSERT(n, n->apackage.scope == NULL);
    n->apackage.scope = LC_CreateScope(256);

    LC_StringList files = n->apackage.injected_filepaths;
    if (files.node_count == 0) {
        files = LC_ListFilesInPackage(L->arena, n->apackage.path);
        if (files.first == NULL) {
            LC_SendErrorMessagef(NULL, NULL, "no valid .lc files in '%.*s'", LC_Expand(n->apackage.path));
            n->apackage.state = LC_DeclState_Error;
            L->errors += 1;
            return;
        }
    }

    for (LC_StringNode *it = files.first; it; it = it->next) {
        LoadedFile file = LC_ReadFileHook(n, it->string);
        if (file.content.str == NULL) file.content.str = "";

        LC_AST *ast_file = LC_ParseFile(n, file.path.str, file.content.str, file.line);
        if (!ast_file) {
            n->apackage.state = LC_DeclState_Error;
            return;
        }
    }
}

LC_FUNCTION void LC_ParsePackagesUsingRegistry(LC_Intern name) {
    LC_AST *n = LC_GetPackageByName(name);
    if (!n) {
        LC_SendErrorMessagef(NULL, NULL, "no package with name '%s'\n", name);
        L->errors += 1;
        return;
    }
    if (n->apackage.scope) {
        return;
    }
    LC_ParsePackage(n);
    LC_ASTRefList imports = LC_GetPackageImports(n);
    for (LC_ASTRef *it = imports.first; it; it = it->next) {
        LC_ParsePackagesUsingRegistry(it->ast->gimport.path);
    }
}

LC_FUNCTION void LC_AddOrderedPackageToRefList(LC_AST *n) {
    LC_ASTRefList *ordered = &L->ordered_packages;
    for (LC_ASTRef *it = ordered->first; it; it = it->next) {
        if (it->ast->apackage.name == n->apackage.name) {
            return;
        }
    }
    LC_AddASTToRefList(ordered, n);
}

// Here we use import statements to produce a list of ordered packages.
// While we are at it we also resolve most top level declarations. I say
// most because aggregations are handled a bit differently, their resolution
// is deffered. This is added because a pointer doesn't require full typeinfo of
// an aggregate. It's just a number.
LC_FUNCTION LC_AST *LC_OrderPackagesAndBasicResolve(LC_AST *pos, LC_Intern name) {
    LC_AST *n = LC_GetPackageByName(name);
    if (n->apackage.state == LC_DeclState_Error) {
        return NULL;
    }
    if (n->apackage.state == LC_DeclState_Resolved) {
        // This function can be called multiple times, I assume user might
        // want to use type information to generate something. Pattern:
        // typecheck -> generate -> typecheck is expected!
        LC_PackageDecls(n);
        return n;
    }
    if (n->apackage.state == LC_DeclState_Resolving) {
        LC_ReportASTError(pos, "circular import '%s'", name);
        n->apackage.state = LC_DeclState_Error;
        return NULL;
    }
    LC_ASSERT(pos, n->apackage.state == LC_DeclState_Unresolved);
    n->apackage.state = LC_DeclState_Resolving;

    LC_Operand op = LC_ImportPackage(NULL, n, L->builtin_package);
    LC_ASSERT(pos, !LC_IsError(op));

    // Resolve all imports regardless of errors.
    // If current package has wrong import it means it's also
    // wrong but it should still look into all imports
    // despite this.
    int           wrong_import = 0;
    LC_ASTRefList refs         = LC_GetPackageImports(n);
    for (LC_ASTRef *it = refs.first; it; it = it->next) {
        LC_AST *import = LC_OrderPackagesAndBasicResolve(it->ast, it->ast->gimport.path);
        if (!import) {
            n->apackage.state = LC_DeclState_Error;
            wrong_import += 1;
            continue;
        }

        LC_Operand op = LC_ImportPackage(it->ast, n, import);
        if (LC_IsError(op)) {
            n->apackage.state = LC_DeclState_Error;
            wrong_import += 1;
            continue;
        }
    }

    if (wrong_import) return NULL;

    LC_PackageDecls(n);
    LC_AddOrderedPackageToRefList(n);
    n->apackage.state = LC_DeclState_Resolved;
    return n;
}

LC_FUNCTION void LC_OrderAndResolveTopLevelDecls(LC_Intern name) {
    L->first_package = name;
    LC_OrderPackagesAndBasicResolve(NULL, name);

    // Resolve still incomplete aggregate types, this operates on all packages
    // that didn't have errors so even if something broke in package ordering
    // it should still be fine to go forward with this and also proc body analysis
    for (LC_ASTRef *it = L->ordered_packages.first; it; it = it->next) {
        LC_AST *package = it->ast;
        LC_ASSERT(package, package->apackage.state == LC_DeclState_Resolved);
        LC_ResolveIncompleteTypes(package);
    }
}

LC_FUNCTION void LC_ResolveAllProcBodies(void) {
    // We don't need to check errors, only valid packages should have been put into
    // the list.
    for (LC_ASTRef *it = L->ordered_packages.first; it; it = it->next) {
        LC_AST *package = it->ast;
        LC_ASSERT(package, package->apackage.state == LC_DeclState_Resolved);
        LC_ResolveProcBodies(package);
    }
}

LC_FUNCTION LC_ASTRefList LC_ResolvePackageByName(LC_Intern name) {
    LC_ParsePackagesUsingRegistry(name);
    LC_ASTRefList empty = {0};
    if (L->errors) return empty;

    LC_OrderAndResolveTopLevelDecls(name);
    LC_ResolveAllProcBodies();
    return L->ordered_packages;
}

LC_FUNCTION LC_String LC_GenerateUnityBuild(LC_ASTRefList packages) {
    if (L->errors) return LC_MakeEmptyString();

    LC_BeginStringGen(L->arena);

    LC_GenLinef("#include <stdbool.h>");
    LC_GenLinef("#include <stddef.h>");
    LC_GenLinef("#ifndef LC_String_IMPL");
    LC_GenLinef("#define LC_String_IMPL");
    LC_GenLinef("typedef struct { char *str; long long len; } LC_String;");
    LC_GenLinef("#endif");
    LC_GenLinef("#ifndef LC_Any_IMPL");
    LC_GenLinef("#define LC_Any_IMPL");
    LC_GenLinef("typedef struct { int type; void *data; } LC_Any;");
    LC_GenLinef("#endif");

    LC_GenLinef("#ifndef LC_Alignof");
    LC_GenLinef("#if defined(__TINYC__)");
    LC_GenLinef("#define LC_Alignof(...) __alignof__(__VA_ARGS__)");
    LC_GenLinef("#else");
    LC_GenLinef("#define LC_Alignof(...) _Alignof(__VA_ARGS__)");
    LC_GenLinef("#endif");
    LC_GenLinef("#endif");
    LC_GenLinef("void *memset(void *, int, size_t);");

    for (LC_ASTRef *it = packages.first; it; it = it->next) LC_GenCHeader(it->ast);
    for (LC_ASTRef *it = packages.first; it; it = it->next) LC_GenCImpl(it->ast);
    LC_String s = LC_EndStringGen(L->arena);
    return s;
}

LC_FUNCTION void LC_AddSingleFilePackage(LC_Intern name, LC_String path) {
    LC_AST *n        = LC_CreateAST(0, LC_ASTKind_Package);
    n->apackage.name = name;
    n->apackage.path = path;
    LC_AddNode(L->arena, &n->apackage.injected_filepaths, path);
    LC_AddPackageToList(n);
}
LC_FUNCTION LC_Lang *LC_LangAlloc(void) {
    LC_Arena *arena      = LC_BootstrapArena();
    LC_Arena *lex_arena  = LC_PushStruct(arena, LC_Arena);
    LC_Arena *decl_arena = LC_PushStruct(arena, LC_Arena);
    LC_Arena *ast_arena  = LC_PushStruct(arena, LC_Arena);
    LC_Arena *type_arena = LC_PushStruct(arena, LC_Arena);
    LC_InitArena(lex_arena);
    LC_InitArena(decl_arena);
    LC_InitArena(ast_arena);
    LC_InitArena(type_arena);

    LC_Lang *l    = LC_PushStruct(arena, LC_Lang);
    l->arena      = arena;
    l->lex_arena  = lex_arena;
    l->decl_arena = decl_arena;
    l->ast_arena  = ast_arena;
    l->type_arena = type_arena;

    l->emit_line_directives        = true;
    l->breakpoint_on_error         = true;
    l->use_colored_terminal_output = true;

    return l;
}

LC_FUNCTION void LC_LangEnd(LC_Lang *lang) {
    LC_DeallocateArena(lang->lex_arena);
    LC_DeallocateArena(lang->type_arena);
    LC_DeallocateArena(lang->decl_arena);
    LC_DeallocateArena(lang->ast_arena);
    LC_DeallocateArena(lang->arena);
    if (L == lang) L = NULL;
}

LC_FUNCTION void LC_LangBegin(LC_Lang *l) {
    L = l;

    // Init default target settings
    {
        if (L->os == LC_OS_Invalid) {
            L->os = LC_OS_LINUX;
#if LC_OPERATING_SYSTEM_WINDOWS
            L->os = LC_OS_WINDOWS;
#elif LC_OPERATING_SYSTEM_MAC
            L->os = LC_OS_MAC;
#endif
        }
        if (L->arch == LC_ARCH_Invalid) {
            L->arch = LC_ARCH_X64;
        }
        if (L->gen == LC_GEN_Invalid) {
            L->gen = LC_GEN_C;
        }
    }

    //
    // Init declared notes, interns and foreign names checker
    //
    {
        L->declared_notes.arena = L->arena;
        L->interns.arena        = L->arena;
        L->foreign_names.arena  = L->arena;
        L->implicit_any.arena   = L->arena;

        LC_MapReserve(&L->declared_notes, 128);
        LC_MapReserve(&L->interns, 4096);
        LC_MapReserve(&L->foreign_names, 256);
        LC_MapReserve(&L->implicit_any, 64);

#define X(x) l->k##x = LC_InternStrLen(#x, sizeof(#x) - 1);
        LC_LIST_KEYWORDS
#undef X
        l->first_keyword = l->kfor;
        l->last_keyword  = l->kfalse;

#define X(x, declare) l->i##x = LC_InternStrLen(#x, sizeof(#x) - 1);
        LC_LIST_INTERNS
#undef X
#define X(x, declare) \
    if (declare) LC_DeclareNote(L->i##x);
        LC_LIST_INTERNS
#undef X
    }

    // Nulls
    {
        L->NullLEX.begin    = "builtin declarations";
        L->NullLEX.file     = LC_ILit("builtin declarations");
        L->BuiltinToken.lex = &L->NullLEX;
        L->BuiltinToken.str = "builtin declarations";
        L->BuiltinToken.len = sizeof("builtin declarations") - 1;
        L->NullAST.pos      = &L->BuiltinToken;
    }

    {
        LC_AST *builtins         = LC_CreateAST(0, LC_ASTKind_Package);
        L->builtin_package       = builtins;
        builtins->apackage.name  = LC_ILit("builtins");
        builtins->apackage.scope = LC_CreateScope(256);
        LC_AddPackageToList(builtins);
    }

    LC_InitDeclStack(&L->resolver.locals, 128);
    L->resolver.duplicate_map.arena = L->arena;
    LC_MapReserve(&L->resolver.duplicate_map, 32);

    L->resolver.stmt_block_stack.arena = L->arena;
    L->printer.out_block_stack.arena   = L->arena;

    LC_PUSH_PACKAGE(L->builtin_package);

    //
    // Init default type sizes using current platform
    //
    // Here we use the sizes of our current platform but
    // later on it gets swapped based on LC override global variables in
    // InitTarget
    //
    {
        l->type_map.arena = L->arena;
        LC_MapReserve(&l->type_map, 256);

        typedef long long          llong;
        typedef unsigned long long ullong;
        typedef unsigned long      ulong;
        typedef unsigned short     ushort;
        typedef unsigned char      uchar;
        typedef unsigned int       uint;

        L->pointer_align = LC_Alignof(void *);
        L->pointer_size  = sizeof(void *);

        int i = 0;
#define X(TNAME, IS_UNSIGNED)                      \
    l->types[i].kind        = LC_TypeKind_##TNAME; \
    l->types[i].size        = sizeof(TNAME);       \
    l->types[i].align       = LC_Alignof(TNAME);   \
    l->types[i].is_unsigned = IS_UNSIGNED;         \
    l->t##TNAME             = l->types + i++;

        LC_LIST_TYPES
#undef X

        //
        // Overwrite types with target
        //
        if (L->arch == LC_ARCH_X64) {
            LC_SetPointerSizeAndAlign(8, 8);
            if (L->os == LC_OS_WINDOWS) {
                L->tlong->size   = 4;
                L->tlong->align  = 4;
                L->tulong->size  = 4;
                L->tulong->align = 4;
            } else {
                L->tlong->size   = 8;
                L->tlong->align  = 8;
                L->tulong->size  = 8;
                L->tulong->align = 8;
            }
        } else if (L->arch == LC_ARCH_X86) {
            LC_SetPointerSizeAndAlign(4, 4);
            L->tlong->size   = 4;
            L->tlong->align  = 4;
            L->tulong->size  = 4;
            L->tulong->align = 4;
        }

        l->types[i].kind = LC_TypeKind_void;
        l->tvoid         = l->types + i++;

        // Init decls for types
        for (int i = 0; i < T_Count; i += 1) {
            char     *it     = (char *)LC_TypeKindToString((LC_TypeKind)i) + 12;
            LC_Intern intern = LC_ILit(it);
            LC_Type  *t      = l->types + i;
            t->id            = ++l->typeids;

            LC_Decl *decl = LC_CreateDecl(LC_DeclKind_Type, intern, &L->NullAST);
            decl->state   = LC_DeclState_Resolved;
            decl->type    = t;
            t->decl       = decl;
            LC_AddDeclToScope(L->builtin_package->apackage.scope, decl);

            if (t->kind == LC_TypeKind_uchar) decl->foreign_name = LC_ILit("unsigned char");
            if (t->kind == LC_TypeKind_ushort) decl->foreign_name = LC_ILit("unsigned short");
            if (t->kind == LC_TypeKind_uint) decl->foreign_name = LC_ILit("unsigned");
            if (t->kind == LC_TypeKind_ulong) decl->foreign_name = LC_ILit("unsigned long");
            if (t->kind == LC_TypeKind_llong) decl->foreign_name = LC_ILit("long long");
            if (t->kind == LC_TypeKind_ullong) decl->foreign_name = LC_ILit("unsigned long long");
        }
    }

    l->tpvoid = LC_CreatePointerType(l->tvoid);
    l->tpchar = LC_CreatePointerType(l->tchar);

    {
        l->tuntypedint  = LC_CreateUntypedInt(L->tint);
        l->tuntypedbool = LC_CreateUntypedInt(L->tbool);
        l->tuntypednil  = LC_CreateUntypedInt(L->tullong);

        l->ttuntypedfloat.kind       = LC_TypeKind_UntypedFloat;
        l->ttuntypedfloat.id         = ++L->typeids;
        l->tuntypedfloat             = &L->ttuntypedfloat;
        l->tuntypedfloat->tutdefault = l->tdouble;
        l->tuntypedfloat->decl       = LC_CreateDecl(LC_DeclKind_Type, LC_ILit("UntypedFloat"), &L->NullAST);

        l->ttuntypedstring.kind       = LC_TypeKind_UntypedString;
        l->ttuntypedstring.id         = ++L->typeids;
        l->tuntypedstring             = &L->ttuntypedstring;
        l->tuntypedstring->tutdefault = l->tpchar;
        l->tuntypedstring->decl       = LC_CreateDecl(LC_DeclKind_Type, LC_ILit("UntypedString"), &L->NullAST);
    }

    // Add builtin "String" type
    {
        L->ttstring.kind = LC_TypeKind_Incomplete;
        L->ttstring.id   = ++L->typeids;
        L->tstring       = &L->ttstring;

        LC_AST  *ast       = LC_ParseDeclf("String :: struct { str: *char; len: int; }");
        LC_Decl *decl      = LC_CreateDecl(LC_DeclKind_Type, ast->dbase.name, ast);
        decl->foreign_name = LC_ILit("LC_String");
        decl->state        = LC_DeclState_Resolved;
        decl->type         = L->tstring;
        L->tstring->decl   = decl;
        LC_AddDeclToScope(L->builtin_package->apackage.scope, decl);
        LC_Operand result = LC_ResolveTypeAggregate(ast, decl->type);
        LC_ASSERT(ast, !LC_IsError(result));
    }

    // Add builtin "Any" type
    {
        L->ttany.kind = LC_TypeKind_Incomplete;
        L->ttany.id   = ++L->typeids;
        L->tany       = &L->ttany;

        LC_AST  *ast       = LC_ParseDeclf("Any :: struct { type: int; data: *void; }");
        LC_Decl *decl      = LC_CreateDecl(LC_DeclKind_Type, ast->dbase.name, ast);
        decl->foreign_name = LC_ILit("LC_Any");
        decl->state        = LC_DeclState_Resolved;
        decl->type         = L->tany;
        L->tany->decl      = decl;
        LC_AddDeclToScope(L->builtin_package->apackage.scope, decl);
        LC_Operand result = LC_ResolveTypeAggregate(ast, decl->type);
        LC_ASSERT(ast, !LC_IsError(result));
    }

    LC_Decl *decl_nil = LC_AddConstIntDecl("nil", 0);
    decl_nil->type    = L->tuntypednil;

    for (int i = LC_ARCH_X64; i < LC_ARCH_Count; i += 1) LC_AddBuiltinConstInt((char *)LC_ARCHToString((LC_ARCH)i), i);
    for (int i = LC_OS_WINDOWS; i < LC_OS_Count; i += 1) LC_AddBuiltinConstInt((char *)LC_OSToString((LC_OS)i), i);
    for (int i = LC_GEN_C; i < LC_GEN_Count; i += 1) LC_AddBuiltinConstInt((char *)LC_GENToString((LC_GEN)i), i);
    LC_AddBuiltinConstInt("LC_ARCH", L->arch);
    LC_AddBuiltinConstInt("LC_GEN", L->gen);
    LC_AddBuiltinConstInt("LC_OS", L->os);

    LC_POP_PACKAGE();
}


#if _WIN32
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

#elif __linux__ || __APPLE__ || __unix__
    #include <sys/stat.h>
    #include <unistd.h>
    #include <limits.h>
    #include <time.h>
    #include <dirent.h>
    #include <sys/mman.h>

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

#endif

#ifndef LC_USE_CUSTOM_ARENA
    #if defined(LC_USE_ADDRESS_SANITIZER)
    #include <sanitizer/asan_interface.h>
#endif

#if !defined(ASAN_POISON_MEMORY_REGION)
    #define LC_ASAN_POISON_MEMORY_REGION(addr, size) ((void)(addr), (void)(size))
    #define LC_ASAN_UNPOISON_MEMORY_REGION(addr, size) ((void)(addr), (void)(size))
#else
    #define LC_ASAN_POISON_MEMORY_REGION(addr, size) ASAN_POISON_MEMORY_REGION(addr, size)
    #define LC_ASAN_UNPOISON_MEMORY_REGION(addr, size) ASAN_UNPOISON_MEMORY_REGION(addr, size)
#endif

#define LC_DEFAULT_RESERVE_SIZE LC_GIB(1)
#define LC_DEFAULT_ALIGNMENT 8
#define LC_COMMIT_ADD_SIZE LC_MIB(4)

LC_FUNCTION uint8_t *LC_V_AdvanceCommit(LC_VMemory *m, size_t *commit_size, size_t page_size) {
    size_t aligned_up_commit                            = LC_AlignUp(*commit_size, page_size);
    size_t to_be_total_commited_size                    = aligned_up_commit + m->commit;
    size_t to_be_total_commited_size_clamped_to_reserve = LC_CLAMP_TOP(to_be_total_commited_size, m->reserve);
    size_t adjusted_to_boundary_commit                  = to_be_total_commited_size_clamped_to_reserve - m->commit;
    LC_Assertf(adjusted_to_boundary_commit, "Reached the virtual memory reserved boundary");
    *commit_size = adjusted_to_boundary_commit;

    if (adjusted_to_boundary_commit == 0) {
        return 0;
    }
    uint8_t *result = m->data + m->commit;
    return result;
}

LC_FUNCTION void LC_PopToPos(LC_Arena *arena, size_t pos) {
    LC_Assertf(arena->len >= arena->base_len, "Bug: arena->len shouldn't ever be smaller then arena->base_len");
    pos         = LC_CLAMP(pos, arena->base_len, arena->len);
    size_t size = arena->len - pos;
    arena->len  = pos;
    LC_ASAN_POISON_MEMORY_REGION(arena->memory.data + arena->len, size);
}

LC_FUNCTION void LC_PopSize(LC_Arena *arena, size_t size) {
    LC_PopToPos(arena, arena->len - size);
}

LC_FUNCTION void LC_DeallocateArena(LC_Arena *arena) {
    LC_VDeallocate(&arena->memory);
}

LC_FUNCTION void LC_ResetArena(LC_Arena *arena) {
    LC_PopToPos(arena, 0);
}

LC_FUNCTION size_t LC__AlignLen(LC_Arena *a) {
    size_t align_offset = a->alignment ? LC_GetAlignOffset((uintptr_t)a->memory.data + (uintptr_t)a->len, a->alignment) : 0;
    size_t aligned      = a->len + align_offset;
    return aligned;
}

LC_FUNCTION void *LC__PushSizeNonZeroed(LC_Arena *a, size_t size) {
    size_t align_offset        = a->alignment ? LC_GetAlignOffset((uintptr_t)a->memory.data + (uintptr_t)a->len, a->alignment) : 0;
    size_t aligned_len         = a->len + align_offset;
    size_t size_with_alignment = size + align_offset;

    if (a->len + size_with_alignment > a->memory.commit) {
        if (a->memory.reserve == 0) {
            LC_Assertf(0, "Pushing on uninitialized arena with zero initialization turned off");
        }
        bool result = LC_VCommit(&a->memory, size_with_alignment + LC_COMMIT_ADD_SIZE);
        LC_Assertf(result, "%s(%d): Failed to commit memory more memory! reserve: %zu commit: %zu len: %zu size_with_alignment: %zu", LC_BeginTempdSourceLoc.file, LC_BeginTempdSourceLoc.line, a->memory.reserve, a->memory.commit, a->len, size_with_alignment);
        (void)result;
    }

    uint8_t *result = a->memory.data + aligned_len;
    a->len += size_with_alignment;
    LC_Assertf(a->len <= a->memory.commit, "%s(%d): Reached commit boundary! reserve: %zu commit: %zu len: %zu base_len: %zu alignment: %d size_with_alignment: %zu", LC_BeginTempdSourceLoc.file, LC_BeginTempdSourceLoc.line, a->memory.reserve, a->memory.commit, a->len, a->base_len, a->alignment, size_with_alignment);
    LC_ASAN_UNPOISON_MEMORY_REGION(result, size);
    return (void *)result;
}

LC_FUNCTION void *LC__PushSize(LC_Arena *arena, size_t size) {
    void *result = LC__PushSizeNonZeroed(arena, size);
    LC_MemoryZero(result, size);
    return result;
}

LC_FUNCTION LC_Arena LC_PushArena(LC_Arena *arena, size_t size) {
    LC_Arena result;
    LC_MemoryZero(&result, sizeof(result));
    result.memory.data    = LC_PushArrayNonZeroed(arena, uint8_t, size);
    result.memory.commit  = size;
    result.memory.reserve = size;
    result.alignment      = arena->alignment;
    return result;
}

LC_FUNCTION LC_Arena *LC_PushArenaP(LC_Arena *arena, size_t size) {
    LC_Arena *result = LC_PushStruct(arena, LC_Arena);
    *result          = LC_PushArena(arena, size);
    return result;
}

LC_FUNCTION void LC_InitArenaEx(LC_Arena *a, size_t reserve) {
    a->memory = LC_VReserve(reserve);
    LC_ASAN_POISON_MEMORY_REGION(a->memory.data, a->memory.reserve);
    a->alignment = LC_DEFAULT_ALIGNMENT;
}

LC_FUNCTION void LC_InitArena(LC_Arena *a) {
    LC_InitArenaEx(a, LC_DEFAULT_RESERVE_SIZE);
}

LC_FUNCTION LC_Arena *LC_BootstrapArena(void) {
    LC_Arena bootstrap_arena = {0};
    LC_InitArena(&bootstrap_arena);

    LC_Arena *arena = LC_PushStruct(&bootstrap_arena, LC_Arena);
    *arena          = bootstrap_arena;
    arena->base_len = arena->len;
    return arena;
}

LC_FUNCTION void LC_InitArenaFromBuffer(LC_Arena *arena, void *buffer, size_t size) {
    arena->memory.data    = (uint8_t *)buffer;
    arena->memory.commit  = size;
    arena->memory.reserve = size;
    arena->alignment      = LC_DEFAULT_ALIGNMENT;
    LC_ASAN_POISON_MEMORY_REGION(arena->memory.data, arena->memory.reserve);
}

LC_FUNCTION LC_TempArena LC_BeginTemp(LC_Arena *arena) {
    LC_TempArena result;
    result.pos   = arena->len;
    result.arena = arena;
    return result;
}

LC_FUNCTION void LC_EndTemp(LC_TempArena checkpoint) {
    LC_PopToPos(checkpoint.arena, checkpoint.pos);
}
    #if _WIN32
        const size_t LC_V_WIN32_PAGE_SIZE = 4096;

LC_FUNCTION LC_VMemory LC_VReserve(size_t size) {
    LC_VMemory result;
    LC_MemoryZero(&result, sizeof(result));
    size_t adjusted_size = LC_AlignUp(size, LC_V_WIN32_PAGE_SIZE);
    result.data          = (uint8_t *)VirtualAlloc(0, adjusted_size, MEM_RESERVE, PAGE_READWRITE);
    LC_Assertf(result.data, "Failed to reserve virtual memory");
    result.reserve = adjusted_size;
    return result;
}

LC_FUNCTION bool LC_VCommit(LC_VMemory *m, size_t commit) {
    uint8_t *pointer = LC_V_AdvanceCommit(m, &commit, LC_V_WIN32_PAGE_SIZE);
    if (pointer) {
        void *result = VirtualAlloc(pointer, commit, MEM_COMMIT, PAGE_READWRITE);
        LC_Assertf(result, "Failed to commit more memory");
        if (result) {
            m->commit += commit;
            return true;
        }
    }
    return false;
}

LC_FUNCTION void LC_VDeallocate(LC_VMemory *m) {
    BOOL result = VirtualFree(m->data, 0, MEM_RELEASE);
    LC_Assertf(result != 0, "Failed to release LC_VMemory");
}

LC_FUNCTION bool LC_VDecommitPos(LC_VMemory *m, size_t pos) {
    size_t aligned          = LC_AlignDown(pos, LC_V_WIN32_PAGE_SIZE);
    size_t adjusted_pos     = LC_CLAMP_TOP(aligned, m->commit);
    size_t size_to_decommit = m->commit - adjusted_pos;
    if (size_to_decommit) {
        uint8_t *base_address = m->data + adjusted_pos;
        BOOL     result       = VirtualFree(base_address, size_to_decommit, MEM_DECOMMIT);
        if (result) {
            m->commit -= size_to_decommit;
            return true;
        }
    }
    return false;
}
    #elif __linux__ || __APPLE__ || __unix__
        #define LC_V_UNIX_PAGE_SIZE 4096

LC_FUNCTION LC_VMemory LC_VReserve(size_t size) {
    LC_VMemory result       = {};
    size_t     size_aligned = LC_AlignUp(size, LC_V_UNIX_PAGE_SIZE);
    result.data             = (uint8_t *)mmap(0, size_aligned, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    LC_Assertf(result.data, "Failed to reserve memory using mmap!!");
    if (result.data) {
        result.reserve = size_aligned;
    }
    return result;
}

LC_FUNCTION bool LC_VCommit(LC_VMemory *m, size_t commit) {
    uint8_t *pointer = LC_V_AdvanceCommit(m, &commit, LC_V_UNIX_PAGE_SIZE);
    if (pointer) {
        int mprotect_result = mprotect(pointer, commit, PROT_READ | PROT_WRITE);
        LC_Assertf(mprotect_result == 0, "Failed to commit more memory using mmap");
        if (mprotect_result == 0) {
            m->commit += commit;
            return true;
        }
    }
    return false;
}

LC_FUNCTION void LC_VDeallocate(LC_VMemory *m) {
    int result = munmap(m->data, m->reserve);
    LC_Assertf(result == 0, "Failed to release virtual memory using munmap");
}
    #endif
#endif

#if __clang__
    #pragma clang diagnostic pop
#endif

#endif // LIB_COMPILER_IMPLEMENTATION

/*
Copyright (c) 2024 Krzosa Karol

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
