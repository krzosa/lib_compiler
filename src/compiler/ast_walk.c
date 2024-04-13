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