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
    if (L->on_tokens_lexed) L->on_tokens_lexed(x);

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
    if (L->on_tokens_interned) L->on_tokens_interned(x);
}

#undef LC_IF
