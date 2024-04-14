LC_FUNCTION void LC_RegisterDeclsFromFile(LC_AST *file) {
    LC_ASTFor(n, file->afile.fdecl) {
        if (n->dbase.resolved_decl) continue;
        if (n->kind == LC_ASTKind_DeclNote) continue;
        LC_Decl *decl = LC_CreateDecl(LC_DeclKind_Type, n->dbase.name, n);
        switch (n->kind) {
        case LC_ASTKind_DeclStruct:
        case LC_ASTKind_DeclUnion:
            decl->type  = LC_CreateIncompleteType(decl);
            decl->state = LC_DeclState_Resolved;
            decl->kind  = LC_DeclKind_Type;
            break;
        case LC_ASTKind_DeclTypedef: decl->kind = LC_DeclKind_Type; break;
        case LC_ASTKind_DeclProc: decl->kind = LC_DeclKind_Proc; break;
        case LC_ASTKind_DeclConst: decl->kind = LC_DeclKind_Const; break;
        case LC_ASTKind_DeclVar: decl->kind = LC_DeclKind_Var; break;
        default: LC_ReportASTError(n, "internal compiler error: got unhandled ast declaration kind in %s", __FUNCTION__);
        }
        LC_PutGlobalDecl(decl);
        n->dbase.resolved_decl = decl;
    }
}

LC_FUNCTION void LC_ResolveDeclsFromFile(LC_AST *file) {
    LC_ASTFor(n, file->afile.fdecl) {
        if (n->kind == LC_ASTKind_DeclNote) {
            LC_ResolveNote(n, false);
        } else {
            LC_ResolveName(n, n->dbase.name);
        }
    }
}

LC_FUNCTION void LC_PackageDecls(LC_AST *package) {
    LC_PUSH_PACKAGE(package);

    // Register top level declarations
    LC_ASTFor(file, package->apackage.ffile) {
        LC_ASTFor(import, file->afile.fimport) {
            if (import->gimport.resolved == false) LC_ReportASTError(import, "internal compiler error: unresolved import got into typechecking stage");
        }
        LC_RegisterDeclsFromFile(file);
    }

    // Resolve declarations by name
    LC_ASTFor(file, package->apackage.ffile) {
        LC_ResolveDeclsFromFile(file);
    }

    LC_POP_PACKAGE();
}

LC_FUNCTION void LC_ResolveProcBodies(LC_AST *package) {
    LC_PUSH_PACKAGE(package);

    LC_ASTFor(file, package->apackage.ffile) {
        LC_ASTFor(n, file->afile.fdecl) {
            if (n->kind == LC_ASTKind_DeclNote) continue;

            LC_Decl *decl = n->dbase.resolved_decl;
            if (decl->kind == LC_DeclKind_Proc) {
                LC_Operand op = LC_ResolveProcBody(decl);
                if (LC_IsError(op)) LC_MarkDeclError(decl);
            }
        }
    }

    LC_POP_PACKAGE();
}

LC_FUNCTION void LC_ResolveIncompleteTypes(LC_AST *package) {
    LC_PUSH_PACKAGE(package);

    LC_ASTFor(file, package->apackage.ffile) {
        LC_ASTFor(n, file->afile.fdecl) {
            if (n->kind == LC_ASTKind_DeclNote) continue;

            LC_Decl *decl = n->dbase.resolved_decl;
            if (decl->kind == LC_DeclKind_Type) {
                LC_ResolveTypeAggregate(decl->ast, decl->type);
            }
        }
    }

    LC_POP_PACKAGE();
}

LC_FUNCTION LC_Operand LC_ResolveNote(LC_AST *n, bool is_decl) {
    LC_AST *note = n->dnote.expr;
    if (n->kind == LC_ASTKind_ExprNote) note = n->enote.expr;
    else if (n->kind == LC_ASTKind_StmtNote) note = n->snote.expr;
    else {
        LC_ASSERT(n, n->kind == LC_ASTKind_DeclNote);
        if (n->dnote.processed) return LC_OPNull;
    }

    if (note->ecompo.name->eident.name == L->istatic_assert) {
        LC_IF(is_decl, note, "#static_assert cant be used as variable initializer");
        LC_Operand LC_PROP_ERROR(op, n, LC_ExpectBuiltinWithOneArg(note));
        LC_IF(!LC_IsUTConst(op) || !LC_IsUTInt(op.type), n, "static assert requires constant untyped integer value");
        int val = (int)LC_Bigint_as_signed(&op.val.i);
        LC_IF(!val, note, "#static_assert failed !");
        n->dnote.processed = true;
    }

    return LC_OPNull;
}

void SetConstVal(LC_AST *n, LC_TypeAndVal val) {
    LC_ASSERT(n, LC_IsUntyped(val.type));
    n->const_val = val;
}

LC_FUNCTION LC_Operand LC_ResolveProcBody(LC_Decl *decl) {
    if (decl->state == LC_DeclState_Error) return LC_OPError();
    if (decl->state == LC_DeclState_ResolvedBody) return LC_OPNull;
    LC_ASSERT(decl->ast, decl->state == LC_DeclState_Resolved);

    LC_AST *n = decl->ast;
    if (n->dproc.body == NULL) return LC_OPNull;
    L->resolver.locals.len = 0;
    LC_ASTFor(it, n->dproc.type->tproc.first) {
        if (it->kind == LC_ASTKind_Error) {
            LC_MarkDeclError(decl);
            return LC_OPError();
        }
        LC_ASSERT(it, it->type);
        LC_Operand LC_DECL_PROP_ERROR(op, LC_CreateLocalDecl(LC_DeclKind_Var, it->tproc_arg.name, it));
        op.decl->type               = it->type;
        op.decl->state              = LC_DeclState_Resolved;
        it->tproc_arg.resolved_decl = op.decl;
    }

    int errors_before = L->errors;
    LC_ASSERT(n, decl->type->tproc.ret);
    L->resolver.expected_ret_type = decl->type->tproc.ret;
    LC_Operand LC_DECL_PROP_ERROR(op, LC_ResolveStmtBlock(n->dproc.body));
    L->resolver.locals.len = 0;

    if (errors_before == L->errors && decl->type->tproc.ret != L->tvoid && !(op.flags & LC_OPF_Returned)) {
        LC_ReportASTError(n, "you can get through this procedure without hitting a return stmt, add a return to cover all control paths");
    }
    decl->state = LC_DeclState_ResolvedBody;
    if (L->on_proc_body_resolved) L->on_proc_body_resolved(decl);
    return LC_OPNull;
}

LC_FUNCTION LC_ResolvedCompoItem *LC_AddResolvedCallItem(LC_ResolvedCompo *list, LC_TypeMember *t, LC_AST *comp, LC_AST *expr) {
    LC_AST *duplicate1 = (LC_AST *)LC_MapGetP(&L->resolver.duplicate_map, list);
    if (t && !duplicate1) {
        for (LC_ResolvedCompoItem *it = list->first; it; it = it->next) {
            if (t == it->t) {
                LC_MapInsertP(&L->resolver.duplicate_map, list, comp);
                LC_MapInsertP(&L->resolver.duplicate_map, comp, it->comp); // duplicate2
                break;
            }
        }
    }
    LC_ResolvedCompoItem *match = LC_PushStruct(L->arena, LC_ResolvedCompoItem);
    list->count += 1;
    match->t    = t;
    match->expr = expr;
    match->comp = comp;
    LC_AddSLL(list->first, list->last, match);
    return match;
}

LC_FUNCTION LC_Operand LC_ResolveCompoCall(LC_AST *n, LC_Type *type) {
    LC_ASSERT(n, type->kind == LC_TypeKind_Proc);
    LC_IF(type->tproc.vargs && type->tagg.mems.count > n->ecompo.size, n, "calling procedure with invalid argument count, expected at least %d args, got %d, the procedure type is: %s", type->tagg.mems.count, n->ecompo.size, LC_GenLCType(type));

    bool named_field_appeared = false;
    LC_ASTFor(it, n->ecompo.first) {
        LC_IF(type->tproc.vargs && it->ecompo_item.name, it, "variadic procedures cannot have named arguments");
        LC_IF(it->ecompo_item.index, it, "index inside a call is not allowed");
        LC_IF(named_field_appeared && it->ecompo_item.name == 0, it, "mixing named and positional arguments is illegal");
        if (it->ecompo_item.name) named_field_appeared = true;
    }

    LC_ResolvedCompo *matches = LC_PushStruct(L->arena, LC_ResolvedCompo);
    LC_TypeMember    *type_it = type->tproc.args.first;
    LC_AST           *npos_it = n->ecompo.first;

    // greedy match unnamed arguments
    for (; type_it; type_it = type_it->next, npos_it = npos_it->next) {
        if (npos_it == NULL || npos_it->ecompo_item.name) break;
        LC_AddResolvedCallItem(matches, type_it, npos_it, npos_it->ecompo_item.expr);
    }

    // greedy match variadic arguments
    if (type->tproc.vargs) {
        for (; npos_it; npos_it = npos_it->next) {
            LC_ResolvedCompoItem *m = LC_AddResolvedCallItem(matches, NULL, npos_it, npos_it->ecompo_item.expr);
            m->varg                 = true;
        }
    }

    // for every required proc type argument we seek a named argument
    // in either default proc values or passed in call arguments
    for (; type_it; type_it = type_it->next) {
        LC_ASTFor(n_it, npos_it) {
            if (type_it->name == n_it->ecompo_item.name) {
                LC_AddResolvedCallItem(matches, type_it, n_it, n_it->ecompo_item.expr);
                goto end_of_outer_loop;
            }
        }

        if (type_it->default_value_expr) {
            LC_ResolvedCompoItem *m = LC_AddResolvedCallItem(matches, type_it, NULL, type_it->default_value_expr);
            m->defaultarg           = true;
        }
    end_of_outer_loop:;
    }

    // make sure we matched every item in call
    LC_ASTFor(n_it, n->ecompo.first) {
        LC_AST *expr     = n_it->ecompo_item.expr;
        bool    included = false;
        for (LC_ResolvedCompoItem *it = matches->first; it; it = it->next) {
            if (it->expr == expr) {
                included = true;
                break;
            }
        }

        LC_IF(!included, expr, "unknown argument to a procedure call, couldn't match it with any of the declared arguments, the procedure type is: %s", LC_GenLCType(type));
    }

    LC_IF(!type->tproc.vargs && matches->count != type->tproc.args.count, n, "invalid argument count passed in to procedure call, expected: %d, matched: %d, the procedure type is: %s", type->tproc.args.count, matches->count, LC_GenLCType(type));

    // error on duplicates
    LC_AST *duplicate1 = (LC_AST *)LC_MapGetP(&L->resolver.duplicate_map, matches);
    LC_AST *duplicate2 = duplicate1 ? (LC_AST *)LC_MapGetP(&L->resolver.duplicate_map, duplicate1) : NULL;
    LC_MapClear(&L->resolver.duplicate_map);
    if (duplicate1) {
        LC_Operand err = LC_ReportASTErrorEx(duplicate1, duplicate2, "two call items match the same procedure argument");
        n->kind        = LC_ASTKind_Error;
        return err;
    }

    // resolve
    for (LC_ResolvedCompoItem *it = matches->first; it; it = it->next) {
        if (it->varg) {
            if (type->tproc.vargs_any_promotion) {
                LC_Operand LC_PROP_ERROR(opexpr, it->expr, LC_ResolveExprAndPushCompoContext(it->expr, L->tany));
                LC_TryTyping(it->expr, LC_OPType(L->tany));
            } else {
                LC_Operand LC_PROP_ERROR(opexpr, it->expr, LC_ResolveExpr(it->expr));
                LC_Operand LC_PROP_ERROR(op, it->expr, LC_ResolveTypeVargs(it->expr, opexpr));
                LC_TryTyping(it->expr, op);
            }
            continue;
        }
        if (it->defaultarg) {
            continue;
        }
        LC_Operand LC_PROP_ERROR(opexpr, it->expr, LC_ResolveExprAndPushCompoContext(it->expr, it->t->type));
        LC_Operand LC_PROP_ERROR(op, it->expr, LC_ResolveTypeVarDecl(it->expr, LC_OPType(it->t->type), opexpr));
        LC_TryTyping(it->expr, op);
    }

    n->ecompo.resolved_items = matches;
    LC_Operand result        = LC_OPLValueAndType(type->tproc.ret);
    return result;
}

LC_FUNCTION LC_Operand LC_ResolveCompoAggregate(LC_AST *n, LC_Type *type) {
    LC_ASSERT(n, type->kind == LC_TypeKind_Union || type->kind == LC_TypeKind_Struct);
    LC_IF(n->ecompo.size > 1 && type->kind == LC_TypeKind_Union, n, "too many union initializers, expected 1 or 0 got %d", n->ecompo.size);
    LC_IF(n->ecompo.size > type->tagg.mems.count, n, "too many struct initializers, expected less then %d got instead %d", type->tagg.mems.count, n->ecompo.size);

    bool named_field_appeared = false;
    LC_ASTFor(it, n->ecompo.first) {
        LC_IF(type->kind == LC_TypeKind_Union && it->ecompo_item.name == 0, it, "unions can only be initialized using named arguments");
        LC_IF(named_field_appeared && it->ecompo_item.name == 0, it, "mixing named and positional arguments is illegal");
        LC_IF(it->ecompo_item.index, it, "index specifier in non array compound is illegal");
        if (it->ecompo_item.name) named_field_appeared = true;
    }

    LC_ResolvedCompo *matches = LC_PushStruct(L->arena, LC_ResolvedCompo);
    LC_TypeMember    *type_it = type->tagg.mems.first;
    LC_AST           *npos_it = n->ecompo.first;

    // greedy match unnamed arguments
    for (; type_it; type_it = type_it->next, npos_it = npos_it->next) {
        if (npos_it == NULL || npos_it->ecompo_item.name) break;
        LC_AddResolvedCallItem(matches, type_it, npos_it, npos_it->ecompo_item.expr);
    }

    // match named arguments
    for (; npos_it; npos_it = npos_it->next) {
        bool found = false;
        LC_TypeFor(type_it, type->tagg.mems.first) {
            if (type_it->name == npos_it->ecompo_item.name) {
                LC_AddResolvedCallItem(matches, type_it, npos_it, npos_it->ecompo_item.expr);
                found = true;
                break;
            }
        }

        LC_IF(!found, npos_it, "no matching declaration with name '%s' in type '%s'", npos_it->ecompo_item.name, LC_GenLCType(type));
    }

    // error on duplicates
    LC_AST *duplicate1 = (LC_AST *)LC_MapGetP(&L->resolver.duplicate_map, matches);
    LC_AST *duplicate2 = duplicate1 ? (LC_AST *)LC_MapGetP(&L->resolver.duplicate_map, duplicate1) : NULL;
    if (duplicate1) {
        LC_Operand err = LC_ReportASTErrorEx(duplicate1, duplicate2, "two compound items match the same struct variable");
        n->kind        = LC_ASTKind_Error;
        return err;
    }

    // resolve
    LC_Operand result = LC_OPLValueAndType(type);
    result.flags |= LC_OPF_Const;
    for (LC_ResolvedCompoItem *it = matches->first; it; it = it->next) {
        LC_Operand LC_PROP_ERROR(opexpr, it->expr, LC_ResolveExprAndPushCompoContext(it->expr, it->t->type));
        LC_Operand LC_PROP_ERROR(op, it->expr, LC_ResolveTypeVarDecl(it->expr, LC_OPType(it->t->type), opexpr));
        LC_TryTyping(it->expr, op);

        if (!(opexpr.flags & LC_OPF_Const)) result.flags &= ~LC_OPF_Const;
    }

    n->ecompo.resolved_items = matches;
    return result;
}

LC_FUNCTION LC_ResolvedCompoArrayItem *LC_AddResolvedCompoArrayItem(LC_ResolvedArrayCompo *arr, int index, LC_AST *comp) {
    LC_AST *duplicate1 = (LC_AST *)LC_MapGetP(&L->resolver.duplicate_map, arr);
    if (!duplicate1) {
        for (LC_ResolvedCompoArrayItem *it = arr->first; it; it = it->next) {
            if (index == it->index) {
                LC_MapInsertP(&L->resolver.duplicate_map, arr, comp);
                LC_MapInsertP(&L->resolver.duplicate_map, comp, it->comp);
                break;
            }
        }
    }
    LC_ResolvedCompoArrayItem *result = LC_PushStruct(L->arena, LC_ResolvedCompoArrayItem);
    result->index                     = index;
    result->comp                      = comp;
    arr->count += 1;
    LC_AddSLL(arr->first, arr->last, result);
    return result;
}

LC_FUNCTION LC_Operand LC_ResolveCompoArray(LC_AST *n, LC_Type *type) {
    LC_ASSERT(n, type->kind == LC_TypeKind_Array);
    LC_IF(n->ecompo.size > type->tarray.size, n, "too many array intializers, array is of size '%d', got '%d'", type->tarray.size, n->ecompo.size);

    bool index_appeared = false;
    LC_ASTFor(it, n->ecompo.first) {
        LC_IF(index_appeared && it->ecompo_item.index == NULL, it, "mixing indexed and positional arguments is illegal");
        LC_IF(it->ecompo_item.name, it, "named arguments are invalid in array compound literal");
        if (it->ecompo_item.index) index_appeared = true;
    }

    LC_ResolvedArrayCompo *matches = LC_PushStruct(L->arena, LC_ResolvedArrayCompo);
    LC_AST                *npos_it = n->ecompo.first;
    int                    index   = 0;

    // greedy match unnamed arguments
    for (; npos_it; npos_it = npos_it->next) {
        if (npos_it->ecompo_item.index) break;
        LC_ASSERT(n, index < type->tarray.size);
        LC_AddResolvedCompoArrayItem(matches, index++, npos_it);
    }

    // match indexed arguments
    for (; npos_it; npos_it = npos_it->next) {
        uint64_t   val = 0;
        LC_Operand LC_PROP_ERROR(op, npos_it, LC_ResolveConstInt(npos_it->ecompo_item.index, L->tint, &val));
        LC_IF(val > type->tarray.size, npos_it, "array index out of bounds, array is of size %d", type->tarray.size);
        LC_AddResolvedCompoArrayItem(matches, (int)val, npos_it);
    }

    // error on duplicates
    LC_AST *duplicate1 = (LC_AST *)LC_MapGetP(&L->resolver.duplicate_map, matches);
    LC_AST *duplicate2 = duplicate1 ? (LC_AST *)LC_MapGetP(&L->resolver.duplicate_map, duplicate1) : NULL;
    if (duplicate1) {
        LC_Operand err = LC_ReportASTErrorEx(duplicate1, duplicate2, "two items in compound array literal match the same index");
        n->kind        = LC_ASTKind_Error;
        return err;
    }

    // resolve
    LC_Operand result = LC_OPLValueAndType(type);
    result.flags |= LC_OPF_Const;
    for (LC_ResolvedCompoArrayItem *it = matches->first; it; it = it->next) {
        LC_AST    *expr = it->comp->ecompo_item.expr;
        LC_Operand LC_PROP_ERROR(opexpr, expr, LC_ResolveExprAndPushCompoContext(expr, type->tbase));
        LC_Operand LC_PROP_ERROR(op, expr, LC_ResolveTypeVarDecl(expr, LC_OPType(type->tbase), opexpr));
        LC_TryTyping(expr, op);

        if (!(opexpr.flags & LC_OPF_Const)) result.flags &= ~LC_OPF_Const;
    }

    n->ecompo.resolved_array_items = matches;
    return result;
}

LC_FUNCTION LC_Operand LC_ResolveTypeOrExpr(LC_AST *n) {
    if (n->kind == LC_ASTKind_ExprType) {
        LC_Operand LC_PROP_ERROR(result, n, LC_ResolveType(n->etype.type));
        n->type = result.type;
        return result;
    } else {
        LC_Operand LC_PROP_ERROR(result, n, LC_ResolveExpr(n));
        return result;
    }
}

LC_FUNCTION LC_Operand LC_ExpectBuiltinWithOneArg(LC_AST *n) {
    LC_IF(n->ecompo.size != 1, n, "expected 1 argument to builtin procedure, got: %d", n->ecompo.size);
    LC_IF(n->ecompo.first->ecompo_item.name, n, "named arguments in this builtin procedure are illegal");
    LC_AST    *expr = n->ecompo.first->ecompo_item.expr;
    LC_Operand LC_PROP_ERROR(op, expr, LC_ResolveTypeOrExpr(expr));
    LC_Operand LC_PROP_ERROR(maybe_err, n, LC_ResolveTypeAggregate(n, op.type));
    expr->type = op.type;
    return op;
}

LC_FUNCTION LC_Operand LC_ResolveBuiltin(LC_AST *n) {
    LC_Operand result = {0};
    if (n->ecompo.name == 0 || n->ecompo.name->kind != LC_ASTKind_ExprIdent) return result;
    LC_Intern ident = n->ecompo.name->eident.name;

    if (ident == L->ilengthof) {
        LC_Operand LC_PROP_ERROR(op, n, LC_ExpectBuiltinWithOneArg(n));
        if (LC_IsArray(op.type)) {
            result = LC_OPInt(op.type->tarray.size);
        } else if (LC_IsUTStr(op.type)) {
            int64_t length = LC_StrLen((char *)op.v.name);
            result         = LC_OPInt(length);
        } else LC_IF(1, n, "expected array or constant string type, got instead '%s'", LC_GenLCType(op.type));
        n->kind = LC_ASTKind_ExprBuiltin;
    } else if (ident == L->isizeof) {
        LC_Operand LC_PROP_ERROR(op, n, LC_ExpectBuiltinWithOneArg(n));
        LC_IF(LC_IsUntyped(op.type), n, "cannot get sizeof a value that is untyped: '%s'", LC_GenLCType(op.type));
        result  = LC_OPInt(op.type->size);
        n->kind = LC_ASTKind_ExprBuiltin;
    } else if (ident == L->ialignof) {
        LC_Operand LC_PROP_ERROR(op, n, LC_ExpectBuiltinWithOneArg(n));
        LC_IF(LC_IsUntyped(op.type), n, "cannot get alignof a value that is untyped: '%s'", LC_GenLCType(op.type));

        LC_AST *expr = n->ecompo.first->ecompo_item.expr;
        LC_IF(expr->kind != LC_ASTKind_ExprType, expr, "argument should be a type, instead it's '%s'", LC_ASTKindToString(expr->kind));

        result  = LC_OPInt(op.type->align);
        n->kind = LC_ASTKind_ExprBuiltin;
    } else if (ident == L->itypeof) {
        LC_Operand LC_PROP_ERROR(op, n, LC_ExpectBuiltinWithOneArg(n));
        LC_IF(LC_IsUntyped(op.type), n, "cannot get typeof a value that is untyped: '%s'", LC_GenLCType(op.type));
        result  = LC_OPInt(op.type->id);
        n->kind = LC_ASTKind_ExprBuiltin;
    } else if (ident == L->ioffsetof) {
        LC_IF(n->ecompo.size != 2, n, "expected 2 arguments to builtin procedure 'offsetof', got: %d", n->ecompo.size);
        LC_AST *a1 = n->ecompo.first;
        LC_AST *a2 = a1->next;
        LC_IF(a1->ecompo_item.name, a1, "named arguments in this builtin procedure are illegal");
        LC_IF(a2->ecompo_item.name, a2, "named arguments in this builtin procedure are illegal");
        LC_AST *a1e = a1->ecompo_item.expr;
        LC_AST *a2e = a2->ecompo_item.expr;
        LC_IF(a1e->kind != LC_ASTKind_ExprType, a1e, "first argument should be a type, instead it's '%s'", LC_ASTKindToString(a1e->kind));
        LC_Operand LC_PROP_ERROR(optype, a1e, LC_ResolveType(a1e->etype.type));
        LC_Operand LC_PROP_ERROR(maybe_err, n, LC_ResolveTypeAggregate(a1e, optype.type));
        LC_IF(!LC_IsAggType(optype.type), a1e, "expected aggregate type in first parameter of 'offsetof', instead got '%s'", LC_GenLCType(optype.type));
        LC_IF(a2e->kind != LC_ASTKind_ExprIdent, a2e, "expected identifier as second parameter to 'offsetof', instead got '%s'", LC_ASTKindToString(a2e->kind));
        a1e->type = optype.type;

        LC_Type       *type       = optype.type;
        LC_TypeMember *found_type = NULL;
        LC_TypeFor(it, type->tagg.mems.first) {
            if (it->name == a2e->eident.name) {
                found_type = it;
                break;
            }
        }

        LC_ASSERT(n, type->decl);
        LC_IF(!found_type, n, "field '%s' not found in '%s'", a2e->eident.name, type->decl->name);
        result  = LC_OPInt(found_type->offset);
        n->kind = LC_ASTKind_ExprBuiltin;
    }

    if (LC_IsUTConst(result)) {
        n->const_val = result.val;
    }

    return result;
}

LC_FUNCTION bool LC_TryTyping(LC_AST *n, LC_Operand op) {
    LC_ASSERT(n, n->type);

    if (LC_IsUntyped(n->type)) {
        if (LC_IsUTInt(n->type) && LC_IsFloat(op.type)) {
            LC_Operand in = {LC_OPF_UTConst | LC_OPF_Const};
            in.val        = n->const_val;
            LC_Operand op = LC_ConstCastFloat(NULL, in);
            SetConstVal(n, op.val);
        }
        if (L->tany == op.type) op = LC_OPModDefaultUT(LC_OPType(n->type));
        n->type = op.type;

        // Bounds check
        if (n->const_val.type && LC_IsUTInt(n->const_val.type)) {
            if (LC_IsInt(n->type) && !LC_BigIntFits(n->const_val.i, n->type)) {
                const char *val = LC_Bigint_str(&n->const_val.i, 10);
                LC_ReportASTError(n, "value '%s', doesn't fit into type '%s'", val, LC_GenLCType(n->type));
            }
        }
    }

    // I think it returns true to do this: (a, b)
    return true;
}

LC_FUNCTION bool LC_TryDefaultTyping(LC_AST *n, LC_Operand *o) {
    LC_ASSERT(n, n->type);
    if (LC_IsUntyped(n->type)) {
        n->type = n->type->tbase;
        if (o) o->type = n->type;
    }
    return true;
}

LC_FUNCTION LC_Operand LC_ResolveNameInScope(LC_AST *n, LC_Decl *parent_decl) {
    LC_ASSERT(n, n->kind == LC_ASTKind_ExprField || n->kind == LC_ASTKind_TypespecField);
    LC_PUSH_SCOPE(parent_decl->scope);
    LC_Operand op = LC_ResolveName(n, n->efield.right);
    LC_POP_SCOPE();
    if (LC_IsError(op)) {
        n->kind = LC_ASTKind_Error;
        return op;
    }

    LC_ASSERT(n, op.decl);
    n->efield.resolved_decl = op.decl;
    n->efield.parent_decl   = parent_decl;

    LC_ASSERT(n, op.decl->kind != LC_DeclKind_Import);
    return op;
}

LC_FUNCTION LC_Operand LC_ResolveExpr(LC_AST *expr) {
    return LC_ResolveExprAndPushCompoContext(expr, NULL);
}

LC_FUNCTION LC_Operand LC_ResolveExprAndPushCompoContext(LC_AST *expr, LC_Type *type) {
    LC_Type *save                  = L->resolver.compo_context_type;
    L->resolver.compo_context_type = type;
    LC_Operand LC_PROP_ERROR(result, expr, LC_ResolveExprEx(expr));
    L->resolver.compo_context_type = save;
    return result;
}

LC_FUNCTION LC_Operand LC_ResolveExprEx(LC_AST *n) {
    LC_Operand result = {0};
    LC_ASSERT(n, LC_IsExpr(n));

    switch (n->kind) {
    case LC_ASTKind_ExprFloat: {
        result.flags    = LC_OPF_UTConst | LC_OPF_Const;
        result.val.d    = n->eatom.d;
        result.val.type = L->tuntypedfloat;
        SetConstVal(n, result.val);
    } break;
    case LC_ASTKind_ExprInt: {
        result.flags    = LC_OPF_UTConst | LC_OPF_Const;
        result.val.i    = n->eatom.i;
        result.val.type = L->tuntypedint;
        SetConstVal(n, result.val);
    } break;
    case LC_ASTKind_ExprBool: {
        result.flags    = LC_OPF_UTConst | LC_OPF_Const;
        result.val.i    = n->eatom.i;
        result.val.type = L->tuntypedbool;
        SetConstVal(n, result.val);
    } break;
    case LC_ASTKind_ExprString: {
        result.flags    = LC_OPF_LValue | LC_OPF_UTConst | LC_OPF_Const;
        result.val.name = n->eatom.name;
        result.val.type = L->tuntypedstring;
        SetConstVal(n, result.val);
    } break;
    case LC_ASTKind_ExprType: {
        return LC_ReportASTError(n, "cannot use type as value");
    } break;

    case LC_ASTKind_ExprIdent: {
        LC_Operand LC_PROP_ERROR(op, n, LC_ResolveName(n, n->eident.name));
        LC_IF(op.decl->kind == LC_DeclKind_Type, n, "declaration is type, unexpected inside expression");
        LC_IF(op.decl->kind == LC_DeclKind_Import, n, "declaration is import, unexpected usage");

        n->eident.resolved_decl = op.decl;
        result.val              = op.decl->val;
        if (op.decl->kind == LC_DeclKind_Const) {
            result.flags |= LC_OPF_UTConst | LC_OPF_Const;
            SetConstVal(n, result.val);
        } else {
            result.flags |= LC_OPF_LValue | LC_OPF_Const;
        }
    } break;

    case LC_ASTKind_ExprCast: {
        LC_Operand LC_PROP_ERROR(optype, n, LC_ResolveType(n->ecast.type));
        LC_Operand LC_PROP_ERROR(opexpr, n, LC_ResolveExpr(n->ecast.expr));
        // :ConstantFold
        // the idea is that this will convert the literal into corresponding
        // type. In c :uint(32) will become 32u. This way we can avoid doing
        // typed arithmetic and let the backend handle it.
        if (LC_IsUTConst(opexpr) && (LC_IsNum(optype.type) || LC_IsStr(optype.type))) {
            if (LC_IsFloat(optype.type)) {
                LC_PROP_ERROR(opexpr, n, LC_ConstCastFloat(n, opexpr));
                SetConstVal(n, opexpr.val);
            } else if (LC_IsInt(optype.type)) {
                LC_PROP_ERROR(opexpr, n, LC_ConstCastInt(n, opexpr));
                SetConstVal(n, opexpr.val);
            } else if (LC_IsStr(optype.type)) {
                LC_IF(!LC_IsUTStr(opexpr.type), n, "cannot cast constant expression of type '%s' to '%s'", LC_GenLCType(opexpr.type), LC_GenLCType(optype.type));
                SetConstVal(n, opexpr.val);
            } else LC_IF(1, n, "cannot cast constant expression of type '%s' to '%s'", LC_GenLCType(opexpr.type), LC_GenLCType(optype.type));
            result.type = optype.type;
        } else {
            LC_PROP_ERROR(result, n, LC_ResolveTypeCast(n, optype, opexpr));
            LC_TryTyping(n->ecast.expr, result);
        }
        result.flags |= (opexpr.flags & LC_OPF_Const);
    } break;

    case LC_ASTKind_ExprUnary: {
        LC_PROP_ERROR(result, n, LC_ResolveExpr(n->eunary.expr));

        if (LC_IsUTConst(result)) {
            LC_PROP_ERROR(result, n, LC_EvalUnary(n, n->eunary.op, result));
            SetConstVal(n, result.val);
        } else {
            LC_OPResult r = LC_IsUnaryOpValidForType(n->eunary.op, result.type);
            LC_IF(r == LC_OPResult_Error, n, "invalid unary operation for type '%s'", LC_GenLCType(result.type));
            if (r == LC_OPResult_Bool) result = LC_OPModBool(result);
        }
    } break;

    case LC_ASTKind_ExprBinary: {
        LC_Operand LC_PROP_ERROR(left, n, LC_ResolveExpr(n->ebinary.left));
        LC_Operand LC_PROP_ERROR(right, n, LC_ResolveExpr(n->ebinary.right));
        LC_PROP_ERROR(result, n, LC_ResolveBinaryExpr(n, left, right));
    } break;

    case LC_ASTKind_ExprAddPtr: {
        LC_Operand LC_PROP_ERROR(left, n, LC_ResolveExpr(n->ebinary.left));
        LC_Operand LC_PROP_ERROR(right, n, LC_ResolveExpr(n->ebinary.right));
        LC_IF(!LC_IsInt(right.type), n, "trying to addptr non integer value of type '%s'", LC_GenLCType(left.type));
        if (LC_IsUTStr(left.type)) LC_TryDefaultTyping(n->ebinary.left, &left);
        LC_IF(!LC_IsPtr(left.type) && !LC_IsArray(left.type), n, "left type is required to be a pointer or array, instead got '%s'", LC_GenLCType(left.type));
        result = left;
        if (!LC_IsUTConst(right)) result.flags &= ~LC_OPF_Const;
        if (LC_IsArray(result.type)) result.type = LC_CreatePointerType(result.type->tbase);
        LC_TryTyping(n->ebinary.right, result);
    } break;

    case LC_ASTKind_ExprIndex: {
        LC_Operand LC_PROP_ERROR(opindex, n, LC_ResolveExpr(n->eindex.index));
        LC_Operand LC_PROP_ERROR(opexpr, n, LC_ResolveExpr(n->eindex.base));
        LC_IF(!LC_IsInt(opindex.type), n, "indexing with non integer value of type '%s'", LC_GenLCType(opindex.type));
        if (LC_IsUTStr(opexpr.type)) LC_TryDefaultTyping(n->eindex.base, &opexpr);
        LC_IF(!LC_IsPtr(opexpr.type) && !LC_IsArray(opexpr.type), n, "trying to index non indexable type '%s'", LC_GenLCType(opexpr.type));
        LC_IF(LC_IsVoidPtr(opexpr.type), n, "void is non indexable");
        LC_TryDefaultTyping(n->eindex.index, &opindex);
        result.type = LC_GetBase(opexpr.type);
        result.flags |= LC_OPF_LValue;
        LC_Operand LC_PROP_ERROR(maybe_err, n, LC_ResolveTypeAggregate(n, result.type));
    } break;

    case LC_ASTKind_ExprGetPointerOfValue: {
        LC_Operand LC_PROP_ERROR(op, n, LC_ResolveExpr(n->eunary.expr));
        LC_IF(!LC_IsLValue(op), n, "trying to access address of a temporal object");
        result.type = LC_CreatePointerType(op.type);
        result.flags |= (op.flags & LC_OPF_Const);
    } break;

    case LC_ASTKind_ExprGetValueOfPointer: {
        LC_Operand LC_PROP_ERROR(op, n, LC_ResolveExpr(n->eunary.expr));
        LC_IF(!LC_IsPtr(op.type), n, "trying to get value of non pointer type: '%s'", LC_GenLCType(op.type));
        result.type = LC_GetBase(op.type);
        result.flags |= LC_OPF_LValue;
        result.flags &= ~LC_OPF_Const;
        LC_Operand LC_PROP_ERROR(maybe_err, n, LC_ResolveTypeAggregate(n, result.type));
    } break;

    case LC_ASTKind_ExprField: {
        bool first_part_executed = false;

        LC_Operand op = {0};
        if (n->efield.left->kind == LC_ASTKind_ExprIdent) {
            LC_AST    *nf = n->efield.left;
            LC_Operand LC_PROP_ERROR(op_name, nf, LC_ResolveName(nf, nf->eident.name));

            // LC_Match (Package.) and fold (Package.Other) into just (Other)
            if (op_name.decl->kind == LC_DeclKind_Import) {
                first_part_executed      = true;
                nf->eident.resolved_decl = op_name.decl;
                nf->type                 = L->tvoid;

                LC_PROP_ERROR(op, n, LC_ResolveNameInScope(n, op_name.decl));
            }
        }

        if (!first_part_executed) {
            LC_ASTKind left_kind = n->efield.left->kind;
            LC_PROP_ERROR(op, n, LC_ResolveExpr(n->efield.left));

            LC_Type   *type = LC_StripPointer(op.type);
            LC_Operand LC_PROP_ERROR(maybe_err, n, LC_ResolveTypeAggregate(n, type));
            LC_IF(!LC_IsAggType(type), n->efield.left, "invalid operation, expected aggregate type, '%s' is not an aggregate", LC_GenLCType(type));
            LC_PROP_ERROR(op, n, LC_ResolveNameInScope(n, type->decl));
            LC_ASSERT(n, op.decl->kind == LC_DeclKind_Var);
            result.flags |= LC_OPF_LValue | LC_OPF_Const;
        }
        result.val = op.decl->val;
    } break;

    case LC_ASTKind_ExprCall: {
        LC_ASSERT(n, n->ecompo.name);
        LC_PROP_ERROR(result, n, LC_ResolveBuiltin(n));
        if (!result.type) {
            LC_Operand LC_PROP_ERROR(left, n, LC_ResolveExpr(n->ecompo.name));
            LC_IF(!LC_IsProc(left.type), n, "trying to call value of invalid type '%s', not a procedure", LC_GenLCType(left.type));
            if (L->before_call_args_resolved) L->before_call_args_resolved(n, left.type);
            LC_PROP_ERROR(result, n, LC_ResolveCompoCall(n, left.type));
        }
    } break;

    case LC_ASTKind_ExprCompound: {
        LC_Type *type = NULL;
        if (n->ecompo.name) {
            LC_PUSH_COMP_ARRAY_SIZE(n->ecompo.size);
            LC_Operand LC_PROP_ERROR(left, n, LC_ResolveTypeOrExpr(n->ecompo.name));
            type = left.type;
            LC_POP_COMP_ARRAY_SIZE();
        }
        if (!n->ecompo.name) type = L->resolver.compo_context_type;
        LC_IF(!type, n, "failed to deduce type of compound expression");
        LC_Operand LC_PROP_ERROR(maybe_err, n, LC_ResolveTypeAggregate(n, type));

        if (LC_IsAggType(type)) {
            LC_PROP_ERROR(result, n, LC_ResolveCompoAggregate(n, type));
        } else if (LC_IsArray(type)) {
            LC_PROP_ERROR(result, n, LC_ResolveCompoArray(n, type));
        } else {
            LC_IF(1, n, "compound of type '%s' is illegal, expected array, struct or union type", LC_GenLCType(type));
        }
    } break;

    default: LC_IF(1, n, "internal compiler error: unhandled expression kind '%s'", LC_ASTKindToString(n->kind));
    }

    n->type = result.type;
    if (n->type != L->tany && L->resolver.compo_context_type == L->tany) {
        LC_MapInsertP(&L->implicit_any, n, (void *)(intptr_t)1);
        result.flags &= ~LC_OPF_Const;
    }

    if (L->on_expr_resolved) L->on_expr_resolved(n, &result);
    return result;
}

LC_FUNCTION LC_Operand LC_ResolveStmtBlock(LC_AST *n) {
    LC_ASSERT(n, n->kind == LC_ASTKind_StmtBlock);

    LC_PUSH_LOCAL_SCOPE();
    LC_PushAST(&L->resolver.stmt_block_stack, n);

    LC_Operand result = {0};
    for (LC_AST *it = n->sblock.first; it; it = it->next) {
        LC_Operand op = LC_ResolveStmt(it);

        // We don't want to whine about non returned procedures if we spotted any errors
        // inside of it.
        if (LC_IsError(op) || (op.flags & LC_OPF_Returned)) result.flags |= LC_OPF_Returned;
    }

    LC_PopAST(&L->resolver.stmt_block_stack);
    LC_POP_LOCAL_SCOPE();

    if (L->on_stmt_resolved) L->on_stmt_resolved(n);
    return result;
}

LC_FUNCTION LC_Operand LC_ResolveVarDecl(LC_Decl *decl) {
    LC_ASSERT(decl->ast, decl->kind == LC_DeclKind_Var);
    LC_Operand result = {0};
    result.flags |= LC_OPF_Const;

    LC_AST    *n      = decl->ast;
    LC_Operand optype = {0};
    LC_Operand opexpr = {0};

    LC_AST *expr = n->dvar.expr;
    LC_AST *type = n->dvar.type;
    if (n->kind == LC_ASTKind_StmtVar) {
        expr = n->svar.expr;
        type = n->svar.type;
    } else {
        LC_ASSERT(n, n->kind == LC_ASTKind_DeclVar);
    }

    // special case := #c(``)
    if (expr && expr->kind == LC_ASTKind_ExprNote) {
        LC_Operand LC_PROP_ERROR(opnote, expr, LC_ResolveNote(expr, true));
        LC_IF(type == NULL, n, "invalid usage of unknown type, need to add type annotation");
        LC_DECL_PROP_ERROR(optype, LC_ResolveType(type));
        decl->type = optype.type;
    } else {
        if (type) {
            if (expr && expr->kind == LC_ASTKind_ExprCompound) {
                LC_PUSH_COMP_ARRAY_SIZE(expr->ecompo.size);
                LC_DECL_PROP_ERROR(optype, LC_ResolveType(type));
                LC_POP_COMP_ARRAY_SIZE();
            } else {
                LC_DECL_PROP_ERROR(optype, LC_ResolveType(type));
            }
        }
        if (expr) {
            LC_DECL_PROP_ERROR(opexpr, LC_ResolveExprAndPushCompoContext(expr, optype.type));
            if (!(opexpr.flags & LC_OPF_Const)) result.flags &= ~LC_OPF_Const;
        }

        LC_Operand LC_DECL_PROP_ERROR(opcast, LC_ResolveTypeVarDecl(n, optype, opexpr));
        if (expr) LC_TryTyping(expr, opcast);
        decl->val = opcast.val;
    }

    LC_ASSERT(n, decl->type);
    LC_Operand LC_PROP_ERROR(maybe_err, n, LC_ResolveTypeAggregate(n, decl->type));
    result.type = decl->type;
    return result;
}

LC_FUNCTION LC_Operand LC_MakeSureNoDeferBlock(LC_AST *n, char *str) {
    for (int i = L->resolver.stmt_block_stack.len - 1; i >= 0; i -= 1) {
        LC_AST *it = L->resolver.stmt_block_stack.data[i];
        if (it->kind == LC_ASTKind_Error) return LC_OPError();
        LC_ASSERT(it, it->kind == LC_ASTKind_StmtBlock);
        LC_IF(it->sblock.kind == SBLK_Defer, n, str);
    }
    return LC_OPNull;
}

LC_FUNCTION LC_Operand LC_MakeSureInsideLoopBlock(LC_AST *n, char *str) {
    bool loop_found = false;
    for (int i = L->resolver.stmt_block_stack.len - 1; i >= 0; i -= 1) {
        LC_AST *it = L->resolver.stmt_block_stack.data[i];
        if (it->kind == LC_ASTKind_Error) return LC_OPError();
        LC_ASSERT(it, it->kind == LC_ASTKind_StmtBlock);
        if (it->sblock.kind == SBLK_Loop) {
            loop_found = true;
            break;
        }
    }
    LC_IF(!loop_found, n, str);
    return LC_OPNull;
}

LC_FUNCTION LC_Operand LC_MatchLabeledBlock(LC_AST *n) {
    if (n->sbreak.name) {
        bool found = false;
        for (int i = L->resolver.stmt_block_stack.len - 1; i >= 0; i -= 1) {
            LC_AST *it = L->resolver.stmt_block_stack.data[i];
            if (it->kind == LC_ASTKind_Error) return LC_OPError();
            if (it->sblock.name == n->sbreak.name) {
                found = true;
                break;
            }
        }
        LC_IF(!found, n, "no label with name '%s'", n->sbreak.name);
    }
    return LC_OPNull;
}

LC_FUNCTION void WalkToFindCall(LC_ASTWalker *w, LC_AST *n) {
    if (n->kind == LC_ASTKind_ExprCall || n->kind == LC_ASTKind_ExprBuiltin) ((bool *)w->user_data)[0] = true;
}

LC_FUNCTION bool LC_ContainsCallExpr(LC_AST *ast) {
    LC_TempArena checkpoint = LC_BeginTemp(L->arena);
    bool         found_call = false;
    {
        LC_ASTWalker walker = LC_GetDefaultWalker(L->arena, WalkToFindCall);
        walker.depth_first  = false;
        walker.user_data    = (void *)&found_call;
        LC_WalkAST(&walker, ast);
    }
    LC_EndTemp(checkpoint);
    return found_call;
}

LC_FUNCTION LC_Operand LC_ResolveStmt(LC_AST *n) {
    LC_ASSERT(n, LC_IsStmt(n));

    LC_Operand result = {0};
    switch (n->kind) {
    case LC_ASTKind_StmtVar: {
        LC_Operand LC_PROP_ERROR(opdecl, n, LC_CreateLocalDecl(LC_DeclKind_Var, n->svar.name, n));
        LC_Operand LC_PROP_ERROR(op, n, LC_ResolveVarDecl(opdecl.decl));
        opdecl.decl->state    = LC_DeclState_Resolved;
        n->svar.resolved_decl = opdecl.decl;
        n->type               = op.type;
    } break;

    case LC_ASTKind_StmtConst: {
        LC_Operand LC_PROP_ERROR(opdecl, n, LC_CreateLocalDecl(LC_DeclKind_Const, n->sconst.name, n));
        LC_Operand LC_PROP_ERROR(op, n, LC_ResolveConstDecl(opdecl.decl));
        opdecl.decl->state = LC_DeclState_Resolved;
        n->type            = op.type;
    } break;

    case LC_ASTKind_StmtAssign: {
        LC_Operand LC_PROP_ERROR(left, n, LC_ResolveExpr(n->sassign.left));
        LC_IF(!LC_IsLValue(left), n, "assigning value to a temporal object (lvalue)");
        LC_Type *type = left.type;

        LC_OPResult valid = LC_IsAssignValidForType(n->sassign.op, type);
        LC_IF(valid == LC_OPResult_Error, n, "invalid assignment operation '%s' for type '%s'", LC_TokenKindToString(n->sassign.op), LC_GenLCType(type));

        LC_Operand LC_PROP_ERROR(right, n, LC_ResolveExprAndPushCompoContext(n->sassign.right, type));
        LC_PROP_ERROR(result, n, LC_ResolveTypeVarDecl(n, left, right));
        LC_TryTyping(n->sassign.left, result);
        LC_TryTyping(n->sassign.right, result);
        n->type = result.type;
    } break;

    case LC_ASTKind_StmtExpr: {
        LC_Operand LC_PROP_ERROR(op, n, LC_ResolveExpr(n->sexpr.expr));
        LC_TryDefaultTyping(n->sexpr.expr, &op);
        n->type               = op.type;
        bool    contains_call = LC_ContainsCallExpr(n->sexpr.expr);
        LC_AST *note          = LC_HasNote(n, L->iunused);
        LC_IF(!note && !contains_call, n, "very likely a bug, expression statement doesn't contain any calls so it doesn't do anything");
    } break;

    case LC_ASTKind_StmtReturn: {
        LC_PROP_ERROR(result, n, LC_MakeSureNoDeferBlock(n, "returning from defer block is illegal"));
        LC_Operand op = LC_OPType(L->tvoid);
        if (n->sreturn.expr) {
            LC_PROP_ERROR(op, n, LC_ResolveExprAndPushCompoContext(n->sreturn.expr, L->resolver.expected_ret_type));
        }

        if (!(op.type == L->resolver.expected_ret_type && op.type == L->tvoid)) {
            LC_PROP_ERROR(op, n, LC_ResolveTypeVarDecl(n, LC_OPType(L->resolver.expected_ret_type), op));
            if (n->sreturn.expr) LC_TryTyping(n->sreturn.expr, op);
        }
        result.flags |= LC_OPF_Returned;
    } break;

    case LC_ASTKind_StmtNote: LC_PROP_ERROR(result, n, LC_ResolveNote(n, false)); break;
    case LC_ASTKind_StmtContinue:
        LC_PROP_ERROR(result, n, LC_MakeSureNoDeferBlock(n, "continue inside of defer is illegal"));
        LC_PROP_ERROR(result, n, LC_MakeSureInsideLoopBlock(n, "continue outside of a for loop is illegal"));
    case LC_ASTKind_StmtBreak: {
        LC_PROP_ERROR(result, n, LC_MakeSureNoDeferBlock(n, "break inside of defer is illegal"));
        LC_PROP_ERROR(result, n, LC_MakeSureInsideLoopBlock(n, "break outside of a for loop is illegal"));
        LC_PROP_ERROR(result, n, LC_MatchLabeledBlock(n));
    } break;

    case LC_ASTKind_StmtDefer: {
        LC_PROP_ERROR(result, n, LC_MakeSureNoDeferBlock(n, "defer inside of defer is illegal"));
        LC_Operand LC_PROP_ERROR(op, n, LC_ResolveStmtBlock(n->sdefer.body));
        LC_AST    *parent_block = LC_GetLastAST(&L->resolver.stmt_block_stack);
        LC_ASSERT(n, parent_block->kind == LC_ASTKind_StmtBlock);
        LC_SLLStackAddMod(parent_block->sblock.first_defer, n, sdefer.next);
    } break;

    case LC_ASTKind_StmtSwitch: {
        LC_Operand LC_PROP_ERROR(opfirst, n, LC_ResolveExpr(n->sswitch.expr));
        LC_IF(!LC_IsInt(opfirst.type), n, "invalid type in switch condition '%s', it should be an integer", LC_GenLCType(opfirst.type));
        LC_TryDefaultTyping(n->sswitch.expr, &opfirst);

        bool all_returned = true;
        bool has_default  = n->sswitch.last && n->sswitch.last->kind == LC_ASTKind_StmtSwitchDefault;

        LC_Operand *ops = LC_PushArray(L->arena, LC_Operand, n->sswitch.total_switch_case_count);
        int         opi = 0;
        LC_ASTFor(case_it, n->sswitch.first) {
            if (case_it->kind == LC_ASTKind_StmtSwitchCase) {
                LC_ASTFor(case_expr_it, case_it->scase.first) {
                    LC_Operand LC_PROP_ERROR(opcase, n, LC_ResolveExpr(case_expr_it));
                    LC_IF(!LC_IsUTConst(opcase), case_expr_it, "expected an untyped constant");
                    ops[opi++] = opcase;

                    LC_Operand LC_PROP_ERROR(o, n, LC_ResolveTypeVarDecl(case_expr_it, opfirst, opcase));
                    LC_TryTyping(case_expr_it, o);
                }
            }
            LC_Operand LC_PROP_ERROR(opbody, case_it, LC_ResolveStmtBlock(case_it->scase.body));
            if (!(opbody.flags & LC_OPF_Returned)) all_returned = false;
        }
        LC_ASSERT(n, opi == n->sswitch.total_switch_case_count);

        for (int i = 0; i < opi; i += 1) {
            LC_Operand a = ops[i];
            for (int j = 0; j < opi; j += 1) {
                if (i == j) continue;
                LC_Operand b = ops[j];

                // bounds check error is thrown in LC_ResolveTypeVarDecl
                if (LC_BigIntFits(a.v.i, opfirst.type) && LC_BigIntFits(b.v.i, opfirst.type)) {
                    uint64_t au = LC_Bigint_as_unsigned(&a.v.i);
                    uint64_t bu = LC_Bigint_as_unsigned(&b.v.i);
                    LC_IF(au == bu, n, "duplicate fields, with value: %llu, in a switch statement", au);
                }
            }
        }

        if (all_returned && has_default) result.flags |= LC_OPF_Returned;
    } break;

    case LC_ASTKind_StmtFor: {
        LC_StmtFor *sfor = &n->sfor;
        LC_PUSH_LOCAL_SCOPE();

        if (sfor->init) {
            LC_Operand opinit = LC_ResolveStmt(sfor->init);
            if (LC_IsError(opinit)) {
                n->kind = LC_ASTKind_Error;
                LC_POP_LOCAL_SCOPE();
                return opinit;
            }

            LC_TryDefaultTyping(sfor->init, &opinit);
        }
        if (sfor->cond) {
            LC_Operand opcond = LC_ResolveExpr(sfor->cond);
            if (LC_IsError(opcond)) {
                n->kind = LC_ASTKind_Error;
                LC_POP_LOCAL_SCOPE();
                return opcond;
            }

            LC_TryDefaultTyping(sfor->cond, &opcond);
            if (!LC_IsIntLike(opcond.type)) {
                n->kind = LC_ASTKind_Error;
                LC_POP_LOCAL_SCOPE();
                return LC_ReportASTError(n, "invalid type in for condition '%s', it should be an integer or pointer", LC_GenLCType(opcond.type));
            }
        }
        if (sfor->inc) {
            LC_Operand opinc = LC_ResolveStmt(sfor->inc);
            if (LC_IsError(opinc)) {
                n->kind = LC_ASTKind_Error;
                LC_POP_LOCAL_SCOPE();
                return opinc;
            }
        }
        result = LC_ResolveStmtBlock(sfor->body);
        if (LC_IsError(result)) {
            n->kind = LC_ASTKind_Error;
            LC_POP_LOCAL_SCOPE();
            return result;
        }

        LC_POP_LOCAL_SCOPE();
    } break;

    case LC_ASTKind_StmtBlock: {
        // we don't handle errors here explicitly
        LC_Operand op = LC_ResolveStmtBlock(n);
        if (op.flags & LC_OPF_Returned) result.flags |= LC_OPF_Returned;
    } break;

    case LC_ASTKind_StmtIf: {
        LC_Operand LC_PROP_ERROR(op, n, LC_ResolveExpr(n->sif.expr));
        LC_TryDefaultTyping(n->sif.expr, &op);
        LC_IF(!LC_IsIntLike(op.type), n, "invalid type in if clause expression %s it should be an integer or pointer", LC_GenLCType(op.type));

        bool all_returned = true;
        bool has_else     = n->sif.last && n->sif.last->kind == LC_ASTKind_StmtElse;

        LC_Operand LC_PROP_ERROR(opbody, n, LC_ResolveStmtBlock(n->sif.body));
        if (!(opbody.flags & LC_OPF_Returned)) all_returned = false;

        LC_ASTFor(it, n->sif.first) {
            if (it->kind == LC_ASTKind_StmtElseIf) {
                LC_Operand LC_PROP_ERROR(op, it, LC_ResolveExpr(it->sif.expr));
                LC_TryDefaultTyping(it->sif.expr, &op);
                LC_IF(!LC_IsIntLike(op.type), n, "invalid type in if clause expression %s it should be an integer or pointer", LC_GenLCType(op.type));
            }
            LC_Operand LC_PROP_ERROR(opbody, it, LC_ResolveStmtBlock(it->sif.body));
            if (!(opbody.flags & LC_OPF_Returned)) all_returned = false;
        }

        if (all_returned && has_else) result.flags |= LC_OPF_Returned;
    } break;
    default: LC_IF(1, n, "internal compiler error: unhandled statement kind '%s'", LC_ASTKindToString(n->kind));
    }

    if (L->on_stmt_resolved) L->on_stmt_resolved(n);
    return result;
}

LC_FUNCTION LC_Operand LC_ResolveConstDecl(LC_Decl *decl) {
    LC_ASSERT(decl->ast, decl->kind == LC_DeclKind_Const);
    LC_AST *n    = decl->ast;
    LC_AST *expr = n->dconst.expr;
    if (n->kind == LC_ASTKind_StmtConst) {
        expr = n->sconst.expr;
    } else {
        LC_ASSERT(n, n->kind == LC_ASTKind_DeclConst);
    }

    LC_Operand LC_DECL_PROP_ERROR(opexpr, LC_ResolveExpr(expr));
    LC_DECL_IF(!LC_IsUTConst(opexpr), n, "expected an untyped constant");
    LC_DECL_IF(!LC_IsUntyped(opexpr.type), n, "type of constant expression is not a simple type");
    decl->val = opexpr.val;
    return opexpr;
}

LC_FUNCTION LC_Operand LC_ResolveName(LC_AST *pos, LC_Intern intern) {
    LC_Decl *decl = LC_GetLocalOrGlobalDecl(intern);
    LC_DECL_IF(!decl, pos, "undeclared identifier '%s'", intern);
    LC_DECL_IF(decl->state == LC_DeclState_Resolving, pos, "cyclic dependency %s", intern);
    if (decl->state == LC_DeclState_Error) return LC_OPError();
    if (decl->state == LC_DeclState_Resolved || decl->state == LC_DeclState_ResolvedBody) return LC_OPDecl(decl);
    LC_ASSERT(pos, decl->state == LC_DeclState_Unresolved);
    decl->state = LC_DeclState_Resolving;

    LC_AST *n = decl->ast;
    switch (decl->kind) {
    case LC_DeclKind_Const: {
        LC_Operand LC_DECL_PROP_ERROR(op, LC_ResolveConstDecl(decl));
    } break;
    case LC_DeclKind_Var: {
        LC_Operand LC_DECL_PROP_ERROR(op, LC_ResolveVarDecl(decl));
        LC_DECL_IF(!(op.flags & LC_OPF_Const), n, "non constant global declarations are illegal");
    } break;
    case LC_DeclKind_Proc: {
        LC_Operand LC_DECL_PROP_ERROR(optype, LC_ResolveType(n->dproc.type));
        decl->type       = optype.type;
        decl->type->decl = decl;
    } break;
    case LC_DeclKind_Import: {
        LC_ASSERT(n, decl->scope);
    } break;
    case LC_DeclKind_Type: {
        LC_ASSERT(n, n->kind == LC_ASTKind_DeclTypedef);
        LC_Operand LC_DECL_PROP_ERROR(op, LC_ResolveType(n->dtypedef.type));
        decl->val = op.val;

        // I have decided that aggregates cannot be hard typedefed.
        // It brings issues to LC_ResolveTypeAggregate and is not needed, what's needed
        // is typedef on numbers and pointers that create distinct new
        // types. I have never had a use for typedefing a struct to make
        // it more typesafe etc.
        LC_AST *is_weak = LC_HasNote(n, L->iweak);
        bool    is_agg  = op.type->decl && LC_IsAgg(op.type->decl->ast);
        if (!is_weak && !is_agg) decl->type = LC_CreateTypedef(decl, decl->type);
        LC_DECL_IF(is_weak && is_agg, n, "@weak doesn't work on aggregate types");
    } break;
    default: LC_DECL_IF(1, n, "internal compiler error: unhandled LC_DeclKind: '%s'", LC_DeclKindToString(decl->kind))
    }
    decl->state = LC_DeclState_Resolved;

    if (L->on_decl_type_resolved) L->on_decl_type_resolved(decl);
    LC_AST *pkg = decl->package;
    LC_DLLAdd(pkg->apackage.ext->first_ordered, pkg->apackage.ext->last_ordered, decl);
    return LC_OPDecl(decl);
}

LC_FUNCTION LC_Operand LC_ResolveConstInt(LC_AST *n, LC_Type *int_type, uint64_t *out_size) {
    LC_Operand LC_PROP_ERROR(op, n, LC_ResolveExpr(n));
    LC_IF(!LC_IsUTConst(op), n, "expected a constant untyped int");
    LC_IF(!LC_IsUTInt(op.type), n, "expected untyped int constant instead: '%s'", LC_GenLCType(op.type));
    LC_IF(!LC_BigIntFits(op.val.i, int_type), n, "constant value: '%s', doesn't fit in type '%s'", LC_GenLCTypeVal(op.val), LC_GenLCType(int_type));
    if (out_size) *out_size = LC_Bigint_as_unsigned(&op.val.i);
    LC_TryTyping(n, LC_OPType(int_type));
    return op;
}

LC_FUNCTION LC_Operand LC_ResolveType(LC_AST *n) {
    LC_ASSERT(n, LC_IsType(n));
    LC_Operand result = {0};

    switch (n->kind) {
    case LC_ASTKind_TypespecField: {
        LC_ASSERT(n, n->efield.left->kind == LC_ASTKind_TypespecIdent);
        LC_Operand LC_PROP_ERROR(l, n, LC_ResolveName(n, n->efield.left->eident.name));
        LC_IF(l.decl->kind != LC_DeclKind_Import, n, "only accessing '.' imports in type definitions is valid, you are trying to access: '%s'", LC_DeclKindToString(l.decl->kind));
        n->efield.left->eident.resolved_decl = l.decl;
        n->efield.left->type                 = L->tvoid;

        LC_Operand LC_PROP_ERROR(op, n, LC_ResolveNameInScope(n, l.decl));
        LC_IF(op.decl->kind != LC_DeclKind_Type, n, "expected reference to type, instead it's: '%s'", LC_DeclKindToString(op.decl->kind));
        result.type = op.decl->type;
    } break;

    case LC_ASTKind_TypespecIdent: {
        LC_Operand LC_PROP_ERROR(op, n, LC_ResolveName(n, n->eident.name));
        LC_IF(op.decl->kind != LC_DeclKind_Type, n, "identifier is not a type");
        result.type             = op.decl->type;
        n->eident.resolved_decl = op.decl;
    } break;

    case LC_ASTKind_TypespecPointer: {
        LC_Operand LC_PROP_ERROR(op, n, LC_ResolveType(n->tpointer.base));
        result.type = LC_CreatePointerType(op.type);
    } break;

    case LC_ASTKind_TypespecArray: {
        LC_Operand LC_PROP_ERROR(opbase, n, LC_ResolveType(n->tarray.base));
        uint64_t   size = L->resolver.compo_context_array_size;
        if (n->tarray.index) {
            LC_Operand LC_PROP_ERROR(opindex, n, LC_ResolveConstInt(n->tarray.index, L->tint, &size));
        }
        LC_IF(size == 0, n, "failed to deduce array size");
        LC_Operand LC_PROP_ERROR(maybe_err, n, LC_ResolveTypeAggregate(n, opbase.type));

        result.type = LC_CreateArrayType(opbase.type, (int)size);
    } break;

    case LC_ASTKind_TypespecProc: {
        LC_Type *ret = L->tvoid;
        if (n->tproc.ret) {
            LC_Operand LC_PROP_ERROR(op, n->tproc.ret, LC_ResolveType(n->tproc.ret));
            ret = op.type;
        }
        LC_Operand        LC_PROP_ERROR(maybe_err, n, LC_ResolveTypeAggregate(n, ret));
        LC_TypeMemberList typelist = {0};
        LC_ASTFor(it, n->tproc.first) {
            LC_Operand LC_PROP_ERROR(op, it, LC_ResolveType(it->tproc_arg.type));
            LC_Operand LC_PROP_ERROR(maybe_err, n, LC_ResolveTypeAggregate(it, op.type));

            LC_AST *expr = it->tproc_arg.expr;
            if (expr) {
                LC_Operand LC_PROP_ERROR(opexpr, expr, LC_ResolveExprAndPushCompoContext(expr, op.type));
                LC_Operand LC_PROP_ERROR(opfin, expr, LC_ResolveTypeVarDecl(expr, op, opexpr));
                LC_TryTyping(expr, opfin);
            }

            LC_TypeMember *mem = LC_AddTypeToList(&typelist, it->tproc_arg.name, op.type, it);
            LC_IF(!mem, it, "duplicate proc argument '%s'", it->tproc_arg.name);
            mem->default_value_expr = expr;
            it->type                = op.type;
        }

        LC_TypeFor(i, typelist.first) {
            LC_TypeFor(j, typelist.first) {
                LC_IF(i != j && i->name == j->name, i->ast, "procedure has 2 arguments with the same name");
            }
        }
        result.type = LC_CreateProcType(typelist, ret, n->tproc.vargs, n->tproc.vargs_any_promotion);
    } break;

    default: LC_IF(1, n, "internal compiler error: unhandled kind in LC_ResolveType '%s'", LC_ASTKindToString(n->kind));
    }

    n->type = result.type;
    LC_ASSERT(n, result.type);
    return result;
}

// clang-format off
// NA - Not applicable
// LC_RPS - OK if right side int of pointer size
// LC_LPS
// LC_TEQ - OK if types equal
// LC_RO0 - OK if right value is int equal nil
// LT - Left untyped to typed
// RT
// LF - Left to float
// RF
// LC_SR - String
enum { LC_INT, LC_FLOAT, LC_UT_INT, LC_UT_FLOAT, LC_UT_STR, LC_PTR, LC_VOID_PTR, LC_PROC, LC_AGG, LC_ARRAY, LC_ANY, LC_VOID, LC_TYPE_COUNT };
typedef enum { LC_NO, LC_OK, LC_LPS, LC_RPS, LC_TEQ, LC_NA, LC_RO0, LC_LT, LC_RT, LC_LF, LC_RF, LC_SR } LC_TypeRule;

LC_TypeRule CastingRules[LC_TYPE_COUNT][LC_TYPE_COUNT] = {
//\/:tgt( src)>   LC_INT      , LC_FLOAT , LC_UT_INT , LC_UT_FLOAT , LC_UT_STR , LC_PTR , LC_VOID_PTR , LC_PROC , LC_AGG , LC_ARRAY
/*[LC_INT] =        */{LC_OK  , LC_OK    , LC_OK     , LC_OK       , LC_NO     , LC_LPS , LC_LPS      , LC_NO   , LC_NO  , LC_NO}   ,
/*[LC_FLOAT] =      */{LC_OK  , LC_OK    , LC_OK     , LC_OK       , LC_NO     , LC_NO  , LC_NO       , LC_NO   , LC_NO  , LC_NO}   ,
/*[LC_UT_INT] =     */{LC_NA  , LC_NA    , LC_NA     , LC_NA       , LC_NO     , LC_NA  , LC_NA       , LC_NA   , LC_NA  , LC_NA}   ,
/*[LC_UT_FLOAT] =   */{LC_NA  , LC_NA    , LC_NA     , LC_NA       , LC_NO     , LC_NA  , LC_NA       , LC_NA   , LC_NA  , LC_NA}   ,
/*[LC_UT_STR] =     */{LC_NA  , LC_NA    , LC_NA     , LC_NA       , LC_NO     , LC_NA  , LC_NA       , LC_NA   , LC_NA  , LC_NA}   ,
/*[LC_PTR] =        */{LC_RPS , LC_NO    , LC_OK     , LC_NO       , LC_SR     , LC_OK  , LC_OK       , LC_OK   , LC_NO  , LC_NO}   ,
/*[LC_VOID_PTR] =   */{LC_RPS , LC_NO    , LC_OK     , LC_NO       , LC_OK     , LC_OK  , LC_OK       , LC_OK   , LC_NO  , LC_NO}   ,
/*[LC_PROC] =       */{LC_NO  , LC_NO    , LC_NO     , LC_NO       , LC_NO     , LC_OK  , LC_OK       , LC_OK   , LC_NO  , LC_NO}   ,
/*[LC_AGG] =        */{LC_NO  , LC_NO    , LC_NO     , LC_NO       , LC_SR     , LC_NO  , LC_NO       , LC_NO   , LC_NO  , LC_NO}   ,
/*[LC_ARRAY] =      */{LC_NO  , LC_NO    , LC_NO     , LC_NO       , LC_NO     , LC_NO  , LC_NO       , LC_NO   , LC_NO  , LC_NO}   ,
};

LC_TypeRule AssignRules[LC_TYPE_COUNT][LC_TYPE_COUNT] = {
//\/l r>            LC_INT    , LC_FLOAT , LC_UT_INT , LC_UT_FLOAT , LC_UT_STR , LC_PTR , LC_VOID_PTR , LC_PROC , LC_AGG , LC_ARRAY , LC_ANY
/*[LC_INT] =        */{LC_TEQ , LC_NO    , LC_OK     , LC_NO       , LC_NO     , LC_NO  , LC_NO       , LC_NO   , LC_NO  , LC_NO    , LC_NO}  ,
/*[LC_FLOAT] =      */{LC_NO  , LC_TEQ   , LC_OK     , LC_OK       , LC_NO     , LC_NO  , LC_NO       , LC_NO   , LC_NO  , LC_NO    , LC_NO}  ,
/*[LC_UT_INT] =     */{LC_NA  , LC_NA    , LC_NA     , LC_NA       , LC_NA     , LC_NA  , LC_NA       , LC_NA   , LC_NA  , LC_NA    , LC_NO}  ,
/*[LC_UT_FLOAT] =   */{LC_NA  , LC_NA    , LC_NA     , LC_NA       , LC_NA     , LC_NA  , LC_NA       , LC_NA   , LC_NA  , LC_NA    , LC_NO}  ,
/*[LC_UT_STR] =     */{LC_NA  , LC_NA    , LC_NA     , LC_NA       , LC_NA     , LC_NO  , LC_NA       , LC_NA   , LC_NA  , LC_NA    , LC_NO}  ,
/*[LC_PTR] =        */{LC_NO  , LC_NO    , LC_RO0    , LC_NO       , LC_SR     , LC_TEQ , LC_OK       , LC_NO   , LC_NO  , LC_NO    , LC_NO}  ,
/*[LC_VOID_PTR] =   */{LC_NO  , LC_NO    , LC_RO0    , LC_NO       , LC_OK     , LC_OK  , LC_OK       , LC_OK   , LC_NO  , LC_NO    , LC_NO}  ,
/*[LC_PROC] =       */{LC_NO  , LC_NO    , LC_RO0    , LC_NO       , LC_NO     , LC_NO  , LC_OK       , LC_TEQ  , LC_NO  , LC_NO    , LC_NO}  ,
/*[LC_AGG] =        */{LC_NO  , LC_NO    , LC_NO     , LC_NO       , LC_SR     , LC_NO  , LC_NO       , LC_NO   , LC_TEQ , LC_NO    , LC_TEQ} ,
/*[LC_ARRAY] =      */{LC_NO  , LC_NO    , LC_NO     , LC_NO       , LC_NO     , LC_NO  , LC_NO       , LC_NO   , LC_NO  , LC_TEQ   , LC_NO}  ,
/*[LC_ANY] =        */{LC_OK  , LC_OK    , LC_OK     , LC_OK       , LC_OK     , LC_OK  , LC_NO       , LC_OK   , LC_OK  , LC_OK    , LC_OK}  ,
};

LC_TypeRule BinaryRules[LC_TYPE_COUNT][LC_TYPE_COUNT] = {
//\/l r>           LC_INT     , LC_FLOAT , LC_UT_INT , LC_UT_FLOAT , LC_UT_STR , LC_PTR , LC_VOID_PTR , LC_PROC , LC_AGG , LC_ARRAY
/*[LC_INT]     =   */{LC_TEQ  , LC_NO    , LC_RT     , LC_NO       , LC_NO     , LC_NO  , LC_OK       , LC_OK   , LC_NO  , LC_NO    , }       ,
/*[LC_FLOAT]   =   */{LC_NO   , LC_TEQ   , LC_RT     , LC_RT       , LC_NO     , LC_NO  , LC_NO       , LC_NO   , LC_NO  , LC_NO    , }       ,
/*[LC_UT_INT]  =   */{LC_LT   , LC_LT    , LC_OK     , LC_LF       , LC_NO     , LC_OK  , LC_OK       , LC_OK   , LC_NO  , LC_NO    , }       ,
/*[LC_UT_FLOAT]=   */{LC_NO   , LC_LT    , LC_RF     , LC_OK       , LC_NO     , LC_NO  , LC_NO       , LC_NO   , LC_NO  , LC_NO    , }       ,
/*[LC_UT_STR] =    */{LC_NO   , LC_NO    , LC_NO     , LC_NO       , LC_OK     , LC_NO  , LC_NO       , LC_NO   , LC_NO  , LC_NO    , }       ,
/*[LC_PTR]     =   */{LC_OK   , LC_NO    , LC_OK     , LC_NO       , LC_NO     , LC_OK  , LC_OK       , LC_NO   , LC_NO  , LC_NO    , }       ,
/*[LC_VOID_PTR]=   */{LC_OK   , LC_NO    , LC_OK     , LC_NO       , LC_NO     , LC_OK  , LC_OK       , LC_NO   , LC_NO  , LC_NO    , }       ,
/*[LC_PROC]    =   */{LC_OK   , LC_NO    , LC_OK     , LC_NO       , LC_NO     , LC_NO  , LC_NO       , LC_NO   , LC_NO  , LC_NO    , }       ,
/*[LC_AGG]     =   */{LC_NO   , LC_NO    , LC_NO     , LC_NO       , LC_NO     , LC_NO  , LC_NO       , LC_NO   , LC_NO  , LC_NO    , }       ,
/*[LC_ARRAY]   =   */{LC_NO   , LC_NO    , LC_NO     , LC_NO       , LC_NO     , LC_NO  , LC_NO       , LC_NO   , LC_NO  , LC_NO    , }       ,
};
// clang-format on

int GetTypeCategory(LC_Type *x) {
    if (x->kind >= LC_TypeKind_char && x->kind <= LC_TypeKind_ullong) return LC_INT;
    if ((x->kind == LC_TypeKind_float) || (x->kind == LC_TypeKind_double)) return LC_FLOAT;
    if (x->kind == LC_TypeKind_UntypedInt) return LC_UT_INT;
    if (x->kind == LC_TypeKind_UntypedFloat) return LC_UT_FLOAT;
    if (x->kind == LC_TypeKind_UntypedString) return LC_UT_STR;
    if (x == L->tpvoid) return LC_VOID_PTR;
    if (x == L->tany) return LC_ANY;
    if (x->kind == LC_TypeKind_Pointer) return LC_PTR;
    if (x->kind == LC_TypeKind_Proc) return LC_PROC;
    if (LC_IsAggType(x)) return LC_AGG;
    if (LC_IsArray(x)) return LC_ARRAY;
    return LC_VOID;
}

LC_FUNCTION LC_Operand LC_ResolveBinaryExpr(LC_AST *n, LC_Operand l, LC_Operand r) {
    bool isconst = LC_IsConst(l) && LC_IsConst(r);

    LC_TypeRule rule = BinaryRules[GetTypeCategory(l.type)][GetTypeCategory(r.type)];
    LC_IF(rule == LC_NO, n, "cannot perform binary operation, types don't qualify for it, left: '%s' right: '%s'", LC_GenLCType(l.type), LC_GenLCType(r.type));
    LC_IF(rule == LC_TEQ && l.type != r.type, n, "cannot perform binary operation, types are incompatible, left: '%s' right: '%s'", LC_GenLCType(l.type), LC_GenLCType(r.type));
    if (rule == LC_LT) l = LC_OPModType(l, r.type);
    if (rule == LC_RT) r = LC_OPModType(r, l.type);
    if (rule == LC_LF) l = LC_ConstCastFloat(n, l);
    if (rule == LC_RF) r = LC_ConstCastFloat(n, r);
    LC_ASSERT(n, rule == LC_LT || rule == LC_RT || rule == LC_LF || rule == LC_RF || rule == LC_OK || rule == LC_TEQ);

    // WARNING: if we allow for more then boolean operations on pointers then
    // we need to fix the propagated type here, we are counting on it getting
    // modified to bool.
    LC_Operand op = LC_OPType(l.type);
    if (isconst) op.flags |= LC_OPF_Const;
    if (LC_IsUTConst(l) && LC_IsUTConst(r)) {
        LC_PROP_ERROR(op, n, LC_EvalBinary(n, l, n->ebinary.op, r));
        SetConstVal(n, op.val);
    } else {
        LC_OPResult r = LC_IsBinaryExprValidForType(n->ebinary.op, op.type);
        LC_IF(r == LC_OPResult_Error, n, "invalid binary operation for type '%s'", LC_GenLCType(op.type));
        (LC_TryTyping(n->ebinary.left, op), LC_TryTyping(n->ebinary.right, op));
        if (r == LC_OPResult_Bool) op = LC_OPModBool(op);
    }

    return op;
}

LC_FUNCTION LC_Operand LC_ResolveTypeVargs(LC_AST *pos, LC_Operand v) {
    if (LC_IsUntyped(v.type)) v = LC_OPModDefaultUT(v);            // untyped => typed
    if (LC_IsSmallerThenInt(v.type)) v = LC_OPModType(v, L->tint); // c int promotion
    if (v.type == L->tfloat) v = LC_OPModType(v, L->tdouble);      // c int promotion
    return v;
}

LC_FUNCTION LC_Operand LC_ResolveTypeCast(LC_AST *pos, LC_Operand t, LC_Operand v) {
    LC_TypeRule rule = CastingRules[GetTypeCategory(t.type)][GetTypeCategory(v.type)];
    LC_IF(rule == LC_NO, pos, "cannot cast, types are incompatible, left: '%s' right: '%s'", LC_GenLCType(t.type), LC_GenLCType(v.type));
    LC_IF(rule == LC_RPS && v.type->size != L->tpvoid->size, pos, "cannot cast, integer type on right is not big enough to hold a pointer, left: '%s' right: '%s'", LC_GenLCType(t.type), LC_GenLCType(v.type));
    LC_IF(rule == LC_LPS && t.type->size != L->tpvoid->size, pos, "cannot cast, integer type on left is not big enough to hold a pointer, left: '%s' right: '%s'", LC_GenLCType(t.type), LC_GenLCType(v.type));
    LC_IF(rule == LC_SR && !LC_IsStr(t.type), pos, "cannot cast untyped string to non string type: '%s'", LC_GenLCType(t.type));
    LC_ASSERT(pos, rule == LC_LPS || rule == LC_RPS || rule == LC_OK);

    LC_Operand op = LC_OPType(t.type);
    op.flags      = (v.flags & LC_OPF_LValue) | (v.flags & LC_OPF_Const);
    return op;
}

LC_FUNCTION LC_Operand LC_ResolveTypeVarDecl(LC_AST *pos, LC_Operand t, LC_Operand v) {
    t.v = v.v;
    LC_IF(t.type && t.type->kind == LC_TypeKind_void, pos, "cannot create a variable of type void");
    LC_IF(v.type && v.type->kind == LC_TypeKind_void, pos, "cannot assign void expression to a variable");
    if (v.type && t.type) {
        LC_TypeRule rule = AssignRules[GetTypeCategory(t.type)][GetTypeCategory(v.type)];
        LC_IF(rule == LC_NO, pos, "cannot assign, types are incompatible, variable type: '%s' expression type: '%s'", LC_GenLCType(t.type), LC_GenLCType(v.type));
        LC_IF(rule == LC_RPS && v.type->size != L->tpvoid->size, pos, "cannot assign, integer type of expression is not big enough to hold a pointer, left: '%s' right: '%s'", LC_GenLCType(t.type), LC_GenLCType(v.type));
        LC_IF(rule == LC_LPS && t.type->size != L->tpvoid->size, pos, "cannot assign, integer type of variable is not big enough to hold a pointer, left: '%s' right: '%s'", LC_GenLCType(t.type), LC_GenLCType(v.type));
        LC_IF(rule == LC_TEQ && t.type != v.type, pos, "cannot assign, types require explicit cast, variable type: '%s' expression type: '%s'", LC_GenLCType(t.type), LC_GenLCType(v.type));
        LC_IF(rule == LC_RO0 && v.type != L->tuntypednil, pos, "cannot assign, can assign only const integer equal to 0, variable type: '%s' expression type: '%s'", LC_GenLCType(t.type), LC_GenLCType(v.type));
        LC_IF(rule == LC_SR && !LC_IsStr(t.type), pos, "cannot assign untyped string to non string type: '%s'", LC_GenLCType(t.type));
        LC_ASSERT(pos, rule == LC_LPS || rule == LC_RPS || rule == LC_OK || rule == LC_TEQ || rule == LC_RO0 || rule == LC_SR);
        return t;
    }

    if (v.type) return LC_OPModDefaultUT(v); // NULL := untyped => default
    if (t.type) return t;                    // T := NULL => T
    return LC_ReportASTError(pos, "internal compiler error: failed to resolve type of variable, both type and expression are null");
}

LC_FUNCTION LC_Operand LC_ResolveTypeAggregate(LC_AST *pos, LC_Type *type) {
    LC_Decl *decl = type->decl;
    if (type->kind == LC_TypeKind_Error) return LC_OPError();
    LC_TYPE_IF(type->kind == LC_TypeKind_Completing, pos, "cyclic dependency in type '%s'", type->decl->name);
    if (type->kind != LC_TypeKind_Incomplete) return LC_OPNull;
    LC_PUSH_SCOPE(L->resolver.package->apackage.ext->scope);

    LC_AST *n = decl->ast;
    LC_ASSERT(n, decl);
    LC_ASSERT(n, n->kind == LC_ASTKind_DeclStruct || n->kind == LC_ASTKind_DeclUnion);
    int decl_stack_size = 0;

    type->kind = LC_TypeKind_Completing;
    LC_ASTFor(it, n->dagg.first) {
        LC_Intern name = it->tagg_mem.name;

        LC_Operand op = LC_ResolveType(it->tagg_mem.type);
        if (LC_IsError(op)) {
            LC_MarkDeclError(decl);
            type->kind = LC_TypeKind_Error;
            continue; // handle error after we go through all fields
        }

        LC_Operand opc = LC_ResolveTypeAggregate(it, op.type);
        if (LC_IsError(opc)) {
            LC_MarkDeclError(decl);
            type->kind = LC_TypeKind_Error;
            continue; // handle error after we go through all fields
        }

        LC_TypeMember *mem = LC_AddTypeToList(&type->tagg.mems, name, op.type, it);
        LC_TYPE_IF(mem == NULL, it, "duplicate field '%s' in aggregate type '%s'", name, decl->name);
    }
    if (type->kind == LC_TypeKind_Error) {
        return LC_OPError();
    }
    LC_TYPE_IF(type->tagg.mems.count == 0, n, "aggregate type '%s' has no fields", decl->name);
    decl_stack_size += type->tagg.mems.count;

    LC_AST *packed = LC_HasNote(n, L->ipacked);
    if (n->kind == LC_ASTKind_DeclStruct) {
        type->kind      = LC_TypeKind_Struct;
        int field_sizes = 0;
        LC_TypeFor(it, type->tagg.mems.first) {
            int mem_align = packed ? 1 : it->type->align;
            LC_ASSERT(n, LC_IS_POW2(mem_align));
            type->size = (int)LC_AlignUp(type->size, mem_align);
            it->offset = type->size;
            field_sizes += it->type->size;
            type->align = LC_MAX(type->align, mem_align);
            type->size  = it->type->size + (int)LC_AlignUp(type->size, mem_align);
        }
        type->size    = (int)LC_AlignUp(type->size, type->align);
        type->padding = type->size - field_sizes;
    }

    if (n->kind == LC_ASTKind_DeclUnion) {
        type->kind = LC_TypeKind_Union;
        if (packed) LC_ReportASTError(packed, "@packed on union is invalid");
        LC_TypeFor(it, type->tagg.mems.first) {
            LC_ASSERT(n, LC_IS_POW2(it->type->align));
            type->size  = LC_MAX(type->size, it->type->size);
            type->align = LC_MAX(type->align, it->type->align);
        }
        type->size = (int)LC_AlignUp(type->size, type->align);
    }

    int map_size = LC_NextPow2(decl_stack_size * 2 + 1);
    decl->scope  = LC_CreateScope(map_size);

    LC_TypeFor(it, type->tagg.mems.first) {
        LC_Decl *d     = LC_CreateDecl(LC_DeclKind_Var, it->name, it->ast);
        d->state       = LC_DeclState_Resolved;
        d->type        = it->type;
        d->type_member = it;
        LC_AddDeclToScope(decl->scope, d);
    }

    LC_ASSERT(n, decl->scope->cap == map_size);

    if (L->on_decl_type_resolved) L->on_decl_type_resolved(decl);
    LC_AST *pkg = decl->package;
    LC_DLLAdd(pkg->apackage.ext->first_ordered, pkg->apackage.ext->last_ordered, decl);
    LC_POP_SCOPE();
    return LC_OPNull;
}

#undef LC_IF
#undef LC_DECL_IF
#undef LC_PROP_ERROR
#undef LC_DECL_PROP_ERROR