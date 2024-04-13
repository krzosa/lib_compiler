LC_FUNCTION LC_AST *LC_CreateAST(LC_Token *pos, LC_ASTKind kind) {
    LC_AST *n = LC_PushStruct(L->ast_arena, LC_AST);
    n->id     = ++L->ast_count;
    n->kind   = kind;
    n->pos    = pos;
    if (L->parser == &L->quick_parser) n->pos = &L->BuiltinToken;
    return n;
}

LC_FUNCTION LC_AST *LC_CreateUnary(LC_Token *pos, LC_TokenKind op, LC_AST *expr) {
    LC_AST *r      = LC_CreateAST(pos, LC_ASTKind_ExprUnary);
    r->eunary.op   = op;
    r->eunary.expr = expr;
    return r;
}

LC_FUNCTION LC_AST *LC_CreateBinary(LC_Token *pos, LC_AST *left, LC_AST *right, LC_TokenKind op) {
    LC_AST *r        = LC_CreateAST(pos, LC_ASTKind_ExprBinary);
    r->ebinary.op    = op;
    r->ebinary.left  = left;
    r->ebinary.right = right;
    return r;
}

LC_FUNCTION LC_AST *LC_CreateIndex(LC_Token *pos, LC_AST *left, LC_AST *index) {
    LC_AST *r       = LC_CreateAST(pos, LC_ASTKind_ExprIndex);
    r->eindex.index = index;
    r->eindex.base  = left;
    return r;
}

LC_FUNCTION void LC_SetPointerSizeAndAlign(int size, int align) {
    L->pointer_align = align;
    L->pointer_size  = size;
    if (L->tpvoid) {
        L->tpvoid->size  = size;
        L->tpvoid->align = align;
    }
    if (L->tpchar) {
        L->tpchar->size  = size;
        L->tpchar->align = align;
    }
}

LC_FUNCTION LC_Type *LC_CreateType(LC_TypeKind kind) {
    LC_Type *r = LC_PushStruct(L->type_arena, LC_Type);
    L->type_count += 1;
    r->kind = kind;
    r->id   = ++L->typeids;
    if (r->kind == LC_TypeKind_Proc || r->kind == LC_TypeKind_Pointer) {
        r->size        = L->pointer_size;
        r->align       = L->pointer_align;
        r->is_unsigned = true;
    }
    return r;
}

LC_FUNCTION LC_Type *LC_CreateTypedef(LC_Decl *decl, LC_Type *base) {
    LC_Type *n                      = LC_CreateType(base->kind);
    *n                              = *base;
    decl->typedef_renamed_type_decl = base->decl;
    n->decl                         = decl;
    return n;
}

LC_FUNCTION LC_Type *LC_CreatePointerType(LC_Type *type) {
    uint64_t key   = (uint64_t)type;
    LC_Type *entry = (LC_Type *)LC_MapGetU64(&L->type_map, key);
    if (entry) {
        return entry;
    }
    LC_Type *n = LC_CreateType(LC_TypeKind_Pointer);
    n->tbase   = type;
    LC_MapInsertU64(&L->type_map, key, n);
    return n;
}

LC_FUNCTION LC_Type *LC_CreateArrayType(LC_Type *type, int size) {
    uint64_t size_key = LC_HashBytes(&size, sizeof(size));
    uint64_t type_key = LC_HashBytes(type, sizeof(*type));
    uint64_t key      = LC_HashMix(size_key, type_key);
    LC_Type *entry    = (LC_Type *)LC_MapGetU64(&L->type_map, key);
    if (entry) {
        return entry;
    }
    LC_Type *n     = LC_CreateType(LC_TypeKind_Array);
    n->tbase       = type;
    n->tarray.size = size;
    n->size        = type->size * size;
    n->align       = type->align;
    LC_MapInsertU64(&L->type_map, key, n);
    return n;
}

LC_FUNCTION LC_Type *LC_CreateProcType(LC_TypeMemberList args, LC_Type *ret, bool has_vargs, bool has_vargs_any_promotion) {
    LC_ASSERT(NULL, ret);
    uint64_t key      = LC_HashBytes(ret, sizeof(*ret));
    key               = LC_HashMix(key, LC_HashBytes(&has_vargs, sizeof(has_vargs)));
    key               = LC_HashMix(key, LC_HashBytes(&has_vargs_any_promotion, sizeof(has_vargs_any_promotion)));
    int procarg_count = 0;
    LC_TypeFor(it, args.first) {
        key = LC_HashMix(LC_HashBytes(it->type, sizeof(it->type[0])), key);
        key = LC_HashMix(LC_HashBytes((char *)it->name, LC_StrLen((char *)it->name)), key);
        if (it->default_value_expr) {
            key = LC_HashMix(LC_HashBytes(&it->default_value_expr, sizeof(LC_AST *)), key);
        }
        procarg_count += 1;
    }
    LC_Type *n = (LC_Type *)LC_MapGetU64(&L->type_map, key);
    if (n) return n;

    n                            = LC_CreateType(LC_TypeKind_Proc);
    n->tproc.args                = args;
    n->tproc.vargs               = has_vargs;
    n->tproc.vargs_any_promotion = has_vargs_any_promotion;
    n->tproc.ret                 = ret;
    LC_MapInsertU64(&L->type_map, key, n);
    return n;
}

LC_FUNCTION LC_Type *LC_CreateIncompleteType(LC_Decl *decl) {
    LC_Type *n = LC_CreateType(LC_TypeKind_Incomplete);
    n->decl    = decl;
    return n;
}

LC_FUNCTION LC_Type *LC_CreateUntypedIntEx(LC_Type *base, LC_Decl *decl) {
    uint64_t hash_base   = LC_HashBytes(base, sizeof(*base));
    uint64_t untyped_int = LC_TypeKind_UntypedInt;
    uint64_t key         = LC_HashMix(hash_base, untyped_int);
    LC_Type *n           = (LC_Type *)LC_MapGetU64(&L->type_map, key);
    if (n) return n;

    n        = LC_CreateType(LC_TypeKind_UntypedInt);
    n->tbase = base;
    n->decl  = decl;
    return n;
}

LC_FUNCTION LC_Type *LC_CreateUntypedInt(LC_Type *base) {
    LC_Decl *decl = LC_CreateDecl(LC_DeclKind_Type, LC_ILit("UntypedInt"), &L->NullAST);
    LC_Type *n    = LC_CreateUntypedIntEx(base, decl);
    return n;
}

LC_FUNCTION LC_TypeMember *LC_AddTypeToList(LC_TypeMemberList *list, LC_Intern name, LC_Type *type, LC_AST *ast) {
    LC_TypeFor(it, list->first) {
        if (name == it->name) {
            return NULL;
        }
    }

    LC_TypeMember *r = LC_PushStruct(L->arena, LC_TypeMember);
    r->name          = name;
    r->type          = type;
    r->ast           = ast;
    LC_DLLAdd(list->first, list->last, r);
    list->count += 1;
    return r;
}

LC_FUNCTION LC_Type *LC_StripPointer(LC_Type *type) {
    if (type->kind == LC_TypeKind_Pointer) {
        return type->tbase;
    }
    return type;
}
