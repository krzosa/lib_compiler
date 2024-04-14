// clang-format off
#define LC_PUSH_COMP_ARRAY_SIZE(SIZE) int PREV_SIZE = L->resolver.compo_context_array_size; L->resolver.compo_context_array_size = SIZE;
#define LC_POP_COMP_ARRAY_SIZE() L->resolver.compo_context_array_size = PREV_SIZE
#define LC_PUSH_SCOPE(SCOPE) DeclScope *PREV_SCOPE = L->resolver.active_scope; L->resolver.active_scope = SCOPE
#define LC_POP_SCOPE() L->resolver.active_scope = PREV_SCOPE
#define LC_PUSH_LOCAL_SCOPE() int LOCAL_LEN = L->resolver.locals.len
#define LC_POP_LOCAL_SCOPE() L->resolver.locals.len = LOCAL_LEN
#define LC_PUSH_PACKAGE(PKG) LC_AST *PREV_PKG = L->resolver.package; L->resolver.package = PKG; LC_PUSH_SCOPE(PKG->apackage.ext->scope)
#define LC_POP_PACKAGE() L->resolver.package = PREV_PKG; LC_POP_SCOPE()
#define LC_PROP_ERROR(OP, n, ...)  OP = __VA_ARGS__; if (LC_IsError(OP)) { n->kind = LC_ASTKind_Error; return OP; }
#define LC_DECL_PROP_ERROR(OP, ...) OP = __VA_ARGS__; if (LC_IsError(OP)) { LC_MarkDeclError(decl); return OP; }

#define LC_IF(COND, N, ...) if (COND) { LC_Operand R_ = LC_ReportASTError(N, __VA_ARGS__); N->kind = LC_ASTKind_Error; return R_; }
#define LC_DECL_IF(COND, ...) if (COND) { LC_MarkDeclError(decl); return LC_ReportASTError(__VA_ARGS__); }
#define LC_TYPE_IF(COND, ...) if (COND) { LC_MarkDeclError(decl); type->kind = LC_TypeKind_Error; return LC_ReportASTError(__VA_ARGS__); }
// clang-format on

LC_FUNCTION void LC_AddDecl(LC_DeclStack *scope, LC_Decl *decl) {
    if (scope->len + 1 > scope->cap) {
        LC_ASSERT(NULL, scope->cap);
        int       new_cap   = scope->cap * 2;
        LC_Decl **new_stack = LC_PushArray(L->arena, LC_Decl *, new_cap);
        LC_MemoryCopy(new_stack, scope->stack, scope->len * sizeof(LC_Decl *));
        scope->stack = new_stack;
        scope->cap   = new_cap;
    }
    scope->stack[scope->len++] = decl;
}

LC_FUNCTION void LC_InitDeclStack(LC_DeclStack *stack, int size) {
    stack->stack = LC_PushArray(L->arena, LC_Decl *, size);
    stack->cap   = size;
}

LC_FUNCTION LC_DeclStack *LC_CreateDeclStack(int size) {
    LC_DeclStack *stack = LC_PushStruct(L->arena, LC_DeclStack);
    LC_InitDeclStack(stack, size);
    return stack;
}

LC_FUNCTION LC_Decl *LC_FindDeclOnStack(LC_DeclStack *scp, LC_Intern name) {
    for (int i = 0; i < scp->len; i += 1) {
        LC_Decl *it = scp->stack[i];
        if (it->name == name) {
            return it;
        }
    }
    return NULL;
}

LC_FUNCTION void LC_MarkDeclError(LC_Decl *decl) {
    if (decl) {
        decl->kind  = LC_DeclKind_Error;
        decl->state = LC_DeclState_Error;
        if (decl->ast) decl->ast->kind = LC_ASTKind_Error;
    }
}

LC_FUNCTION LC_Decl *LC_CreateDecl(LC_DeclKind kind, LC_Intern name, LC_AST *n) {
    LC_Decl *decl = LC_PushStruct(L->decl_arena, LC_Decl);
    L->decl_count += 1;

    decl->name    = name;
    decl->kind    = kind;
    decl->ast     = n;
    decl->package = L->resolver.package;
    LC_ASSERT(n, decl->package);

    LC_AST *note = LC_HasNote(n, L->iforeign);
    if (note) {
        decl->is_foreign = true;
        if (note->anote.first) {
            if (note->anote.size != 1) LC_ReportASTError(note, "invalid format of @foreign(...), more then 1 argument");
            LC_AST *expr = note->anote.first->ecompo_item.expr;
            if (expr->kind == LC_ASTKind_ExprIdent) decl->foreign_name = expr->eident.name;
            if (expr->kind != LC_ASTKind_ExprIdent) LC_ReportASTError(note, "invalid format of @foreign(...), expected identifier");
        }
    }
    if (!decl->foreign_name) decl->foreign_name = decl->name;
    return decl;
}

LC_FUNCTION LC_Operand LC_ThereIsNoDecl(DeclScope *scp, LC_Decl *decl, bool check_locals) {
    LC_Decl *r = (LC_Decl *)LC_MapGetU64(scp, decl->name);
    if (check_locals && !r) {
        r = LC_FindDeclOnStack(&L->resolver.locals, decl->name);
    }
    if (r) {
        LC_MarkDeclError(r);
        LC_MarkDeclError(decl);
        return LC_ReportASTErrorEx(decl->ast, r->ast, "there are 2 decls with the same name '%s'", decl->name);
    }
    return LC_OPNull;
}

LC_FUNCTION LC_Operand LC_AddDeclToScope(DeclScope *scp, LC_Decl *decl) {
    LC_Operand LC_DECL_PROP_ERROR(op, LC_ThereIsNoDecl(scp, decl, false));
    LC_MapInsertU64(scp, decl->name, decl);
    return LC_OPDecl(decl);
}

LC_FUNCTION DeclScope *LC_CreateScope(int size) {
    DeclScope *scope = LC_PushStruct(L->arena, DeclScope);
    scope->arena     = L->arena;
    LC_MapReserve(scope, size);
    return scope;
}

LC_FUNCTION LC_Decl *LC_FindDeclInScope(DeclScope *scope, LC_Intern name) {
    LC_Decl *decl = (LC_Decl *)LC_MapGetU64(scope, name);
    return decl;
}

LC_FUNCTION LC_Decl *LC_GetLocalOrGlobalDecl(LC_Intern name) {
    LC_Decl *decl = LC_FindDeclInScope(L->resolver.active_scope, name);
    if (!decl && L->resolver.package->apackage.ext->scope == L->resolver.active_scope) {
        decl = LC_FindDeclOnStack(&L->resolver.locals, name);
    }
    return decl;
}

LC_FUNCTION LC_Operand LC_PutGlobalDecl(LC_Decl *decl) {
    LC_Operand LC_DECL_PROP_ERROR(op, LC_AddDeclToScope(L->resolver.package->apackage.ext->scope, decl));

    // :Mangle global scope name
    if (!decl->is_foreign && decl->package != L->builtin_package) {
        bool mangle = true;
        if (LC_HasNote(decl->ast, L->idont_mangle)) mangle = false;
        if (LC_HasNote(decl->ast, L->iapi)) mangle = false;
        if (decl->name == L->imain) {
            if (L->first_package) {
                if (L->first_package == decl->package->apackage.name) {
                    mangle = false;
                }
            } else mangle = false;
        }
        if (mangle) {
            LC_String name     = LC_Format(L->arena, "lc_%s_%s", (char *)decl->package->apackage.name, (char *)decl->name);
            decl->foreign_name = LC_InternStrLen(name.str, (int)name.len);
        }
    }

    LC_Decl *conflict = (LC_Decl *)LC_MapGetU64(&L->foreign_names, decl->foreign_name);
    if (conflict && !decl->is_foreign) {
        LC_ReportASTErrorEx(decl->ast, conflict->ast, "found two global declarations with the same foreign name: %s", decl->foreign_name);
    } else {
        LC_MapInsertU64(&L->foreign_names, decl->foreign_name, decl);
    }

    return op;
}

LC_FUNCTION LC_Operand LC_CreateLocalDecl(LC_DeclKind kind, LC_Intern name, LC_AST *ast) {
    LC_Decl *decl = LC_CreateDecl(kind, name, ast);
    decl->state   = LC_DeclState_Resolving;
    LC_Operand LC_DECL_PROP_ERROR(operr0, LC_ThereIsNoDecl(L->resolver.package->apackage.ext->scope, decl, true));
    LC_AddDecl(&L->resolver.locals, decl);
    return LC_OPDecl(decl);
}

LC_FUNCTION LC_Decl *LC_AddConstIntDecl(char *key, int64_t value) {
    LC_Intern intern = LC_ILit(key);
    LC_Decl  *decl   = LC_CreateDecl(LC_DeclKind_Const, intern, &L->NullAST);
    decl->state      = LC_DeclState_Resolved;
    decl->type       = L->tuntypedint;
    LC_Bigint_init_signed(&decl->v.i, value);
    LC_AddDeclToScope(L->resolver.package->apackage.ext->scope, decl);
    return decl;
}

LC_FUNCTION LC_Decl *LC_GetBuiltin(LC_Intern name) {
    LC_Decl *decl = (LC_Decl *)LC_MapGetU64(L->builtin_package->apackage.ext->scope, name);
    return decl;
}

LC_FUNCTION void LC_AddBuiltinConstInt(char *key, int64_t value) {
    LC_PUSH_PACKAGE(L->builtin_package);
    LC_AddConstIntDecl(key, value);
    LC_POP_PACKAGE();
}

LC_FUNCTION LC_AST *LC_HasNote(LC_AST *ast, LC_Intern i) {
    if (ast && ast->notes) {
        LC_ASTFor(it, ast->notes->anote_list.first) {
            LC_ASSERT(it, "internal compiler error: note is not an identifier");
            if (it->anote.name->eident.name == i) {
                return it;
            }
        }
    }
    return NULL;
}
