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

LC_FUNCTION void LC_CountDeclRefs(LC_Arena *arena, LC_Map *map, LC_AST *ast) {
    LC_ASTWalker walker = LC_GetDefaultWalker(arena, WalkAndCountDeclRefs);
    walker.user_data    = (void *)map;
    walker.visit_notes  = true;
    LC_WalkAST(&walker, ast);
}

LC_FUNCTION LC_Decl *LC_RemoveUnreferencedGlobalDeclsPass(LC_Map *map_of_visits) {
    LC_Decl *first_removed_decl = NULL;
    LC_Decl *last_removed_decl  = NULL;
    for (LC_ASTRef *it = L->ordered_packages.first; it; it = it->next) {
        for (LC_Decl *decl = it->ast->apackage.ext->first_ordered; decl;) {
            intptr_t ref_count = (intptr_t)LC_MapGetP(map_of_visits, decl);

            LC_Decl *remove = decl;
            decl            = decl->next;
            if (ref_count == 0 && remove->foreign_name != LC_ILit("main")) {
                LC_DLLRemove(it->ast->apackage.ext->first_ordered, it->ast->apackage.ext->last_ordered, remove);
                LC_DLLAdd(first_removed_decl, last_removed_decl, remove);
            }
        }
    }
    return first_removed_decl;
}

LC_FUNCTION void LC_ErrorOnUnreferencedLocalsPass(LC_Map *map_of_visits) {
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

LC_FUNCTION void LC_FindUnusedLocalsAndRemoveUnusedGlobalDeclsPass(void) {
    if (L->errors) return;
    LC_TempArena check = LC_BeginTemp(L->arena);

    LC_AST *package = LC_GetPackageByName(L->first_package);
    LC_Map  map     = {check.arena};
    LC_MapReserve(&map, 512);

    LC_CountDeclRefs(check.arena, &map, package);
    LC_Decl *first_removed_decl = LC_RemoveUnreferencedGlobalDeclsPass(&map);
    for (LC_Decl *it = first_removed_decl; it; it = it->next) {
        LC_CountDeclRefs(check.arena, &map, it->ast);
    }
    LC_ErrorOnUnreferencedLocalsPass(&map);

    LC_EndTemp(check);
}