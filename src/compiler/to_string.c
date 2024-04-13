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
