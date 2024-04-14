thread_local LC_Map DebugASTMap;
thread_local bool   DebugInsideDiscarded;

void DebugVerify_WalkAST(LC_ASTWalker *ctx, LC_AST *n) {
    bool created = LC_InsertWithoutReplace(&DebugASTMap, n, (void *)(intptr_t)100);
    if (!created) LC_ReportASTError(n, "we got to the same ast node twice using ast walker");
    // AST_Decl doesn't need to hold types, it's just a standin for LC_Decl
    if (LC_IsDecl(n)) {
        if (n->type) {
            LC_ReportASTError(n, "decl holds type");
        }
    }

    if (LC_IsStmt(n)) {
        // These might pop up in a for loop as expressions
        if (n->kind == LC_ASTKind_StmtVar || n->kind == LC_ASTKind_StmtConst || n->kind == LC_ASTKind_StmtAssign || n->kind == LC_ASTKind_StmtExpr) {
            if (!n->type) {
                LC_ReportASTError(n, "typed stmt doesn't hold type");
            }
        } else {
            if (n->type) {
                LC_ReportASTError(n, "stmt holds type");
            }
        }
    }

    if (LC_IsType(n) && DebugInsideDiscarded == false) {
        if (!n->type) {
            LC_ReportASTError(n, "typespec doesn't hold type");
        }
    }

    if (LC_IsExpr(n)) {
        bool parent_is_const   = false;
        bool parent_is_builtin = false;
        for (int i = ctx->stack.len - 1; i >= 0; i -= 1) {
            LC_AST *parent = ctx->stack.data[i];
            if (parent->kind == LC_ASTKind_DeclConst || parent->kind == LC_ASTKind_StmtConst || parent->const_val.type) {
                parent_is_const = true;
            }
            if (parent->kind == LC_ASTKind_ExprBuiltin) {
                parent_is_builtin = true;
            }
        }

        bool should_have_type = parent_is_builtin == false && ctx->inside_note == 0 && DebugInsideDiscarded == false;
        if (should_have_type && !n->type) {
            LC_ReportASTError(n, "expression doesn't have type");
        }

        if (should_have_type && !parent_is_const) {
            bool type_is_ok = !LC_IsUntyped(n->type) || n->type == L->tuntypedbool;
            if (!type_is_ok) {
                LC_ReportASTError(n, "expression is still untyped");
            }
        }

        if (should_have_type && n->kind == LC_ASTKind_ExprIdent) {
            if (n->eident.resolved_decl == NULL) {
                LC_ReportASTError(n, "identifier doesn't hold resolved declaration");
            }
        }
    }
}

void VerifyCopy_Walk(LC_ASTWalker *ctx, LC_AST *n) {
    int *counter = (int *)ctx->user_data;
    counter[0] += 1;
}

void VerifyASTCopy(LC_ASTRefList packages) {
    for (LC_ASTRef *it = packages.first; it; it = it->next) {
        LC_ASTFor(file, it->ast->apackage.ext->ffile) {
            LC_TempArena c    = LC_BeginTemp(L->arena);
            LC_AST      *copy = LC_CopyAST(L->arena, file);

            int copy_counter     = 0;
            int original_counter = 0;

            {
                LC_ASTWalker walker = LC_GetDefaultWalker(L->arena, VerifyCopy_Walk);
                walker.user_data    = (void *)&copy_counter;
                LC_WalkAST(&walker, copy);
                walker.user_data = (void *)&original_counter;
                LC_WalkAST(&walker, copy);
            }

            IO_Assert(copy_counter == original_counter);
            IO_Assert(copy_counter > 1);
            IO_Assert(original_counter > 1);

            LC_EndTemp(c);
        }
    }
}

void DebugVerifyAST(LC_ASTRefList packages) {
    LC_TempArena checkpoint = LC_BeginTemp(L->arena);

    DebugASTMap = {L->arena};
    LC_MapReserve(&DebugASTMap, 4096);

    LC_ASTWalker walker = LC_GetDefaultWalker(L->arena, DebugVerify_WalkAST);
    walker.visit_notes  = true;
    for (LC_ASTRef *it = packages.first; it; it = it->next) LC_WalkAST(&walker, it->ast);
    LC_WalkAST(&walker, L->tstring->decl->ast);
    LC_WalkAST(&walker, L->tany->decl->ast);

    DebugInsideDiscarded = true;
    for (LC_ASTRef *it = L->discarded.first; it; it = it->next) LC_WalkAST(&walker, it->ast);
    DebugInsideDiscarded = false;

    for (int i = 0; i < L->ast_count; i += 1) {
        LC_AST *it = (LC_AST *)L->ast_arena->memory.data + i;
        if (it == L->builtin_package) continue;
        if (it->kind == LC_ASTKind_Ignore) continue;
        intptr_t value = (intptr_t)LC_MapGetP(&DebugASTMap, it);

        if (value != 100) {
            LC_ReportASTError(it, "marking verification failed!");
        }
    }
    LC_MapClear(&DebugASTMap);

    VerifyASTCopy(packages);
    LC_EndTemp(checkpoint);
}
