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
