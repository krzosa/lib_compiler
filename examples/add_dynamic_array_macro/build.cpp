bool add_dynamic_array_macro() {
    LC_Lang *lang                     = LC_LangAlloc();
    lang->use_colored_terminal_output = UseColoredIO;
    lang->breakpoint_on_error         = BreakpointOnError;
    LC_LangBegin(lang);

    LC_RegisterPackageDir("../pkgs");
    LC_RegisterPackageDir("../examples");

    LC_Intern name = LC_ILit("add_dynamic_array_macro");
    LC_ParsePackagesUsingRegistry(name);
    LC_BuildIfPass();
    if (L->errors) {
        LC_LangEnd(lang);
        return false;
    }

    Array<LC_Intern> array_of_to_gen = {MA_GetAllocator(L->arena)};

    LC_Intern init_array         = LC_ILit("init_array");
    LC_AST   *init_array_ast     = LC_ParseStmtf("{ init_array_base(REF, SIZE, sizeof(REF.data[0])); }");
    LC_AST   *init_array_add_ast = LC_ParseStmtf("{ REF.data[REF.len] = ITEM; REF.len += 1; }");

    LC_Intern add     = LC_ILit("add");
    LC_AST   *add_ast = LC_ParseStmtf(
        "{"
          "    try_growing_array(REF, sizeof(REF.data[0]));"
          "    REF.data[REF.len] = ITEM;"
          "    REF.len += 1;"
          "}");
    LC_Intern ITEM = LC_ILit("ITEM");
    LC_Intern REF  = LC_ILit("REF");

    LC_AST *package   = LC_GetPackageByName(name);
    LC_AST *first_ast = (LC_AST *)L->ast_arena->memory.data;
    for (int i = 0; i < L->ast_count; i += 1) {
        LC_AST *it = first_ast + i;

        // Detect and save to array every unique use of 'ArrayOfT'
        if (it->kind == LC_ASTKind_TypespecIdent) {
            S8_String name = S8_MakeFromChar((char *)it->eident.name);
            if (S8_StartsWith(name, "ArrayOf")) {
                S8_String s8   = S8_Skip(name, S8_Lit("ArrayOf").len);
                LC_Intern type = LC_InternStrLen(s8.str, (int)s8.len);

                bool is_unique = true;
                For2(in_array, array_of_to_gen) {
                    if (in_array == type) {
                        is_unique = false;
                        break;
                    }
                }

                if (is_unique) array_of_to_gen.add(type);
            }
        }

        if (it->kind == LC_ASTKind_StmtExpr && it->sexpr.expr->kind == LC_ASTKind_ExprCall) {
            LC_AST *name = it->sexpr.expr->ecompo.name;
            LC_AST *call = it->sexpr.expr;

            if (name->kind != LC_ASTKind_ExprIdent) {
                continue;
            }

            if (name->eident.name == add) {
                if (call->ecompo.size != 2) {
                    LC_ReportASTError(call, "wrong argument count in macro call");
                    continue;
                }
                LC_AST *first_item = call->ecompo.first;
                LC_AST *arr_ref    = first_item->ecompo_item.expr;
                LC_AST *item       = first_item->next->ecompo_item.expr;

                LC_AST     *macro = LC_CopyAST(L->arena, add_ast);
                LC_ASTArray array = LC_FlattenAST(L->arena, macro);

                for (int m_i = 0; m_i < array.len; m_i += 1) {
                    LC_AST *m_it = array.data[m_i];
                    if (m_it->kind == LC_ASTKind_ExprIdent) {
                        if (m_it->eident.name == ITEM) {
                            *m_it = *LC_CopyAST(L->arena, item);
                        } else if (m_it->eident.name == REF) {
                            *m_it = *LC_CopyAST(L->arena, arr_ref);
                        }
                    }
                    m_it->pos = it->pos;
                }

                macro->next = it->next;
                macro->prev = it->prev;
                *it         = *macro;
            }

            if (name->eident.name == init_array) {
                if (call->ecompo.size != 2) {
                    LC_ReportASTError(call, "wrong argument count in macro call");
                    continue;
                }

                LC_AST *first_item = call->ecompo.first;
                LC_AST *arr_ref    = first_item->ecompo_item.expr;
                LC_AST *item       = first_item->next->ecompo_item.expr;

                if (item->kind != LC_ASTKind_ExprCompound) {
                    LC_ReportASTError(item, "expected compound");
                    continue;
                }

                LC_AST     *macro = LC_CopyAST(L->arena, init_array_ast);
                LC_ASTArray array = LC_FlattenAST(L->arena, macro);
                for (int m_i = 0; m_i < array.len; m_i += 1) {
                    LC_AST *m_it = array.data[m_i];
                    if (m_it->kind != LC_ASTKind_ExprIdent) continue;

                    if (m_it->eident.name == LC_ILit("SIZE")) {
                        // This is a bit dangerous here because through
                        // this call we are bumping L->ast_count += 1
                        *m_it         = *LC_CreateAST(item->pos, LC_ASTKind_ExprInt);
                        m_it->eatom.i = LC_Bigint_u64(item->ecompo.size);
                    } else if (m_it->eident.name == REF) {
                        *m_it = *LC_CopyAST(L->arena, arr_ref);
                    }
                    m_it->pos = it->pos;
                }

                LC_ASTFor(item_it, item->ecompo.first) {
                    LC_AST     *init_item = LC_CopyAST(L->arena, init_array_add_ast);
                    LC_ASTArray array     = LC_FlattenAST(L->arena, init_item);
                    for (int m_i = 0; m_i < array.len; m_i += 1) {
                        LC_AST *m_it = array.data[m_i];

                        if (m_it->kind == LC_ASTKind_ExprIdent) {
                            if (m_it->eident.name == ITEM) {
                                *m_it = *LC_CopyAST(L->arena, item_it->ecompo_item.expr);
                            } else if (m_it->eident.name == REF) {
                                *m_it = *LC_CopyAST(L->arena, arr_ref);
                            }
                        }
                        m_it->pos = name->pos;
                    }

                    LC_DLLAdd(macro->sblock.first, macro->sblock.last, init_item);
                }

                macro->next = it->next;
                macro->prev = it->prev;
                *it         = *macro;
            }
        }
    }

    // @todo: take package into consideration, my current idea is to have a shared package
    // that is going to be imported into every other package, for now we use only the current package
    For(array_of_to_gen) {
        LC_AST *ast  = LC_ParseDeclf("ArrayOf%s :: struct { data: *%s; len: int; cap: int; }", (char *)it, (char *)it);
        LC_AST *file = package->apackage.ext->ffile;

        LC_DLLAdd(file->afile.fdecl, file->afile.ldecl, ast);
    }

    LC_OrderAndResolveTopLevelDecls(name);
    LC_ResolveAllProcBodies();
    if (L->errors) {
        LC_LangEnd(lang);
        return false;
    }

    LC_String code = LC_GenerateUnityBuild(L->ordered_packages);
    OS_MakeDir("examples/add_dynamic_array_macro");
    OS_WriteFile("examples/add_dynamic_array_macro/add_dynamic_array_macro.c", code);

    return true;
}