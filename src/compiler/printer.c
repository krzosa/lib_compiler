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
