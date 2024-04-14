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
    LC_DeclFor(decl, package->apackage.ext->first_ordered) {
        if (decl->is_foreign) continue;
        LC_AST *n = decl->ast;
        if (decl->kind == LC_DeclKind_Type && LC_IsAgg(n)) LC_GenCAggForwardDecl(decl);
    }

    // type decls
    LC_GenLine();
    LC_DeclFor(decl, package->apackage.ext->first_ordered) {
        if (decl->is_foreign) continue;
        LC_AST *n = decl->ast;
        if (decl->kind == LC_DeclKind_Type) LC_GenCTypeDecl(decl);
    }

    // proc and var forward decls
    LC_DeclFor(decl, package->apackage.ext->first_ordered) {
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
    LC_DeclFor(decl, package->apackage.ext->first_ordered) {
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
    LC_DeclFor(decl, package->apackage.ext->first_ordered) {
        LC_AST *n = decl->ast;
        if (decl->kind == LC_DeclKind_Proc && n->dproc.body && !decl->is_foreign) {
            LC_GenCLineDirective(n);
            LC_GenCProcDecl(decl);
            LC_GenCStmtBlock(n->dproc.body);
            LC_GenLine();
        }
    }
}
