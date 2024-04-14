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

    LC_Token *doc_comment;
};

// To minimize package ast size, we want all nodes to be equal in size,
// a lot of things are easier then and we can loop through all asts for free etc.
typedef struct LC_ASTPackageExt LC_ASTPackageExt;
struct LC_ASTPackageExt {
    LC_StringList injected_filepaths; // to sidestep regular file finding, implement single file packages etc.
    LC_AST       *ffile;
    LC_AST       *lfile;

    // These are resolved later:
    // @todo: add foreign name?
    LC_Decl   *first_ordered;
    LC_Decl   *last_ordered;
    DeclScope *scope;
};

struct LC_ASTPackage {
    LC_DeclState      state;
    LC_Intern         name;
    LC_String         path;
    LC_Token         *doc_comment;
    LC_ASTPackageExt *ext;
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
union  LC_Val             { LC_BigInt i; double d; LC_Intern name; };
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
struct LC_ASTRefList      { LC_ASTRef *first; LC_ASTRef *last; };
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
    int inside_note;

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
    LC_ASTRefList discarded;

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