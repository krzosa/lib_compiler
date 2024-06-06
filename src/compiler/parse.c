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
LC_FUNCTION LC_Precedence LC_MakePrecedence(int left, int right) {
    LC_Precedence result = {left, right};
    return result;
}

LC_FUNCTION LC_Precedence LC_GetPrecedence(LC_PrecedenceKind p, LC_TokenKind kind) {
    if (p == LC_PrecedenceKind_Prefix) goto Prefix;
    if (p == LC_PrecedenceKind_Infix) goto Infix;
    if (p == LC_PrecedenceKind_Postfix) goto Postfix;
    LC_ASSERT(NULL, !"invalid codepath");

Prefix:
    switch (kind) {
        case LC_TokenKind_OpenBracket: return LC_MakePrecedence(-2, 22);
        case LC_TokenKind_Mul: case LC_TokenKind_BitAnd: case LC_TokenKind_Keyword: case LC_TokenKind_OpenParen:
        case LC_TokenKind_Sub: case LC_TokenKind_Add: case LC_TokenKind_Neg: case LC_TokenKind_Not: case LC_TokenKind_OpenBrace: return LC_MakePrecedence(-2, 20);
        default: return LC_MakePrecedence(-1, -1);
    }
Infix:
    switch (kind) {
        case LC_TokenKind_Or: return LC_MakePrecedence(9, 10);
        case LC_TokenKind_And: return LC_MakePrecedence(11, 12);
        case LC_TokenKind_Equals: case LC_TokenKind_NotEquals: case LC_TokenKind_GreaterThen:
        case LC_TokenKind_GreaterThenEq: case LC_TokenKind_LesserThen: case LC_TokenKind_LesserThenEq: return LC_MakePrecedence(13, 14);
        case LC_TokenKind_Sub: case LC_TokenKind_Add: case LC_TokenKind_BitOr: case LC_TokenKind_BitXor: return LC_MakePrecedence(15, 16);
        case LC_TokenKind_RightShift: case LC_TokenKind_LeftShift: case LC_TokenKind_BitAnd:
        case LC_TokenKind_Mul: case LC_TokenKind_Div: case LC_TokenKind_Mod: return LC_MakePrecedence(17, 18);
        default: return LC_MakePrecedence(0, 0);
    }
Postfix:
    switch (kind) {
        case LC_TokenKind_Dot: case LC_TokenKind_OpenBracket: case LC_TokenKind_OpenParen: return LC_MakePrecedence(21, -2);
        default: return LC_MakePrecedence(-1, -1);
    }
}

LC_FUNCTION LC_AST *LC_ParseExprEx(int min_bp) {
    LC_AST *left = NULL;
    LC_Token *prev = LC_GetI(-1);
    LC_Token *t = LC_Next();
    LC_Precedence prefixbp = LC_GetPrecedence(LC_PrecedenceKind_Prefix, t->kind);

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

        LC_Precedence postfix_bp = LC_GetPrecedence(LC_PrecedenceKind_Postfix, t->kind);
        LC_Precedence infix_bp = LC_GetPrecedence(LC_PrecedenceKind_Infix, t->kind);

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

LC_FUNCTION bool ParseHashBuildIf(LC_AST *n) {
    LC_Token *t0 = LC_GetI(0);
    LC_Token *t1 = LC_GetI(1);
    if (t0->kind == LC_TokenKind_Hash && t1->kind == LC_TokenKind_Ident && t1->ident == L->ibuild_if) {
        LC_Next();

        LC_AST *note = LC_ParseNote();
        if (note->kind == LC_ASTKind_Error) {
            LC_EatUntilNextValidDecl();
            return true;
        }

        if (!LC_Match(LC_TokenKind_Semicolon)) {
            LC_ReportParseError(LC_GetI(-1), "expected ';' semicolon");
            LC_EatUntilNextValidDecl();
            return true;
        }

        LC_AST *note_list = LC_CreateAST(t0, LC_ASTKind_NoteList);
        LC_DLLAdd(note_list->anote_list.first, note_list->anote_list.last, note);
        n->notes = note_list;

        return LC_ResolveBuildIf(note);
    }
    return true;
}

LC_FUNCTION bool LC_ResolveBuildIf(LC_AST *build_if) {
    LC_ExprCompo *note = &build_if->anote;
    if (note->size != 1) {
        LC_ReportParseError(LC_GetI(-1), "invalid argument count for #build_if directive, expected 1, got %d", note->size);
        return true;
    }

    LC_ExprCompoItem *item = &note->first->ecompo_item;
    if (item->index != NULL || item->name != 0) {
        LC_ReportParseError(LC_GetI(-1), "invalid syntax, you have passed in a named or indexed argument to #build_if");
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
    ParseHashBuildIf(n);

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
            LC_DLLAdd(n->afile.fdecl, n->afile.ldecl, decl);
            if (L->on_decl_parsed) L->on_decl_parsed(decl);
        }
    }

    if (package) {
        if (package->apackage.ext->doc_comment) LC_ReportParseError(package_doc_comment, "there are more then 1 package doc comments in %s package", (char *)package->apackage.name);
        package->apackage.ext->doc_comment = package_doc_comment;
        LC_AddFileToPackage(package, n);
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