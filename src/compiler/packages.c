LC_FUNCTION LC_Operand LC_ImportPackage(LC_AST *import, LC_AST *dst, LC_AST *src) {
    DeclScope *dst_scope  = dst->apackage.ext->scope;
    int        scope_size = LC_NextPow2(src->apackage.ext->scope->len * 2 + 1);
    if (import && import->gimport.name) {
        LC_PUSH_PACKAGE(dst);
        LC_Decl *decl = LC_CreateDecl(LC_DeclKind_Import, import->gimport.name, import);
        decl->scope   = LC_CreateScope(scope_size);
        LC_PutGlobalDecl(decl);
        import->gimport.resolved_decl = decl;
        LC_POP_PACKAGE();
        dst_scope = decl->scope;
    }

    for (int i = 0; i < src->apackage.ext->scope->cap; i += 1) {
        LC_MapEntry entry = src->apackage.ext->scope->entries[i];
        if (entry.key != 0) {
            LC_Decl *decl = (LC_Decl *)entry.value;
            if (decl->package != src) continue;
            LC_Decl *existing = (LC_Decl *)LC_MapGetU64(dst_scope, decl->name);
            if (existing && decl->package == L->builtin_package) {
                continue;
            }
            if (existing) {
                LC_MarkDeclError(existing);
                LC_MarkDeclError(decl);
                return LC_ReportASTErrorEx(decl->ast, existing->ast, "name colission while importing '%s' into '%s', there are 2 decls with the same name '%s'", src->apackage.name, dst->apackage.name, decl->name);
            }
            LC_MapInsertU64(dst_scope, decl->name, decl);
        }
    }

    if (import && import->gimport.name) {
        LC_ASSERT(import, dst_scope->cap == scope_size);
    }

    if (import) import->gimport.resolved = true;
    return LC_OPNull;
}

LC_FUNCTION LC_Intern LC_MakePackageNameFromPath(LC_String path) {
    if (path.str[path.len - 1] == '/') path = LC_Chop(path, 1);
    LC_String s8name = LC_SkipToLastSlash(path);
    if (!LC_IsDir(L->arena, path) && LC_EndsWith(path, LC_Lit(".lc"), true)) {
        s8name = LC_ChopLastPeriod(s8name);
    }
    if (s8name.len == 0) {
        L->errors += 1;
        LC_SendErrorMessagef(NULL, NULL, "failed to extract name from path %.*s", LC_Expand(path));
        return LC_GetUniqueIntern("invalid_package_name");
    }
    LC_Intern result = LC_InternStrLen(s8name.str, (int)s8name.len);
    return result;
}

LC_FUNCTION bool LC_PackageNameValid(LC_Intern name) {
    char *str = (char *)name;
    if (LC_IsDigit(str[0])) return false;
    for (int i = 0; str[i]; i += 1) {
        bool is_valid = LC_IsIdent(str[i]) || LC_IsDigit(str[i]);
        if (!is_valid) return false;
    }
    return true;
}

LC_FUNCTION bool LC_PackageNameDuplicate(LC_Intern name) {
    LC_ASTFor(it, L->fpackage) {
        if (it->apackage.name == name) return true;
    }
    return false;
}

LC_FUNCTION void LC_AddPackageToList(LC_AST *n) {
    LC_Intern name = n->apackage.name;
    if (LC_PackageNameDuplicate(name)) {
        LC_SendErrorMessagef(NULL, NULL, "found 2 packages with the same name: '%s' / '%.*s'\n", name, LC_Expand(n->apackage.path));
        L->errors += 1;
        return;
    }
    if (!LC_PackageNameValid(name)) {
        LC_SendErrorMessagef(NULL, NULL, "invalid package name, please change the name of the package directory: '%s'\n", name);
        L->errors += 1;
        return;
    }
    LC_DLLAdd(L->fpackage, L->lpackage, n);
}

LC_FUNCTION LC_AST *LC_RegisterPackage(LC_String path) {
    LC_ASSERT(NULL, path.len != 0);
    LC_AST *n        = LC_CreateAST(NULL, LC_ASTKind_Package);
    n->apackage.ext  = LC_PushStruct(L->arena, LC_ASTPackageExt);
    n->apackage.name = LC_MakePackageNameFromPath(path);
    n->apackage.path = path;
    LC_AddPackageToList(n);
    return n;
}

LC_FUNCTION LC_AST *LC_FindImportInRefList(LC_ASTRefList *arr, LC_Intern path) {
    for (LC_ASTRef *it = arr->first; it; it = it->next) {
        if (it->ast->gimport.path == path) return it->ast;
    }
    return NULL;
}

LC_FUNCTION void LC_AddASTToRefList(LC_ASTRefList *refs, LC_AST *ast) {
    LC_ASTRef *ref = LC_PushStruct(L->arena, LC_ASTRef);
    ref->ast       = ast;
    LC_DLLAdd(refs->first, refs->last, ref);
}

LC_FUNCTION LC_ASTRefList LC_GetPackageImports(LC_AST *package) {
    LC_ASSERT(package, package->kind == LC_ASTKind_Package);

    LC_ASTRefList refs = {0};
    LC_ASTFor(file, package->apackage.ext->ffile) {
        LC_ASTFor(import, file->afile.fimport) {
            LC_AST *found = LC_FindImportInRefList(&refs, import->gimport.path);
            if (found) {
                LC_ReportASTErrorEx(import, found, "duplicate import of: '%s', into package '%s'\n", import->gimport.path, package->apackage.name);
                continue;
            }
            LC_AddASTToRefList(&refs, import);
        }
    }

    return refs;
}

LC_FUNCTION void LC_RegisterPackageDir(char *dir) {
    LC_String sdir = LC_MakeFromChar(dir);
    if (!LC_IsDir(L->arena, sdir)) {
        LC_SendErrorMessagef(NULL, NULL, "dir with name '%s', doesn't exist\n", dir);
        return;
    }
    LC_AddNode(L->arena, &L->package_dirs, sdir);
}

LC_FUNCTION LC_AST *LC_GetPackageByName(LC_Intern name) {
    LC_ASTFor(it, L->fpackage) {
        if (it->apackage.name == name) return it;
    }

    LC_AST *result = NULL;
    for (LC_StringNode *it = L->package_dirs.first; it; it = it->next) {
        LC_String s    = it->string;
        LC_String path = LC_Format(L->arena, "%.*s/%s", LC_Expand(s), (char *)name);
        if (LC_IsDir(L->arena, path)) {
            if (result != NULL) {
                LC_SendErrorMessagef(NULL, NULL, "found 2 directories with the same name: '%.*s', '%.*s'\n", LC_Expand(path), LC_Expand(result->apackage.path));
                L->errors += 1;
                break;
            }
            result = LC_RegisterPackage(path);
        }
    }

    return result;
}

LC_FUNCTION LC_StringList LC_ListFilesInPackage(LC_Arena *arena, LC_String path) {
    LC_StringList result = LC_MakeEmptyList();
    for (LC_FileIter it = LC_IterateFiles(arena, path); LC_IsValid(it); LC_Advance(&it)) {
        if (LC_EndsWith(it.absolute_path, LC_Lit(".lc"), LC_IgnoreCase)) {
            LC_AddNode(arena, &result, it.absolute_path);
        }
    }
    return result;
}

LC_FUNCTION LoadedFile LC_ReadFileHook(LC_AST *package, LC_String path) {
    LoadedFile result = {path};
    if (L->on_file_load) {
        L->on_file_load(package, &result);
    } else {
        result.content = LC_ReadFile(L->arena, result.path);
    }

    return result;
}

LC_FUNCTION void LC_ParsePackage(LC_AST *n) {
    LC_ASSERT(n, n->kind == LC_ASTKind_Package);
    LC_ASSERT(n, n->apackage.ext->scope == NULL);
    n->apackage.ext->scope = LC_CreateScope(256);

    LC_StringList files = n->apackage.ext->injected_filepaths;
    if (files.node_count == 0) {
        files = LC_ListFilesInPackage(L->arena, n->apackage.path);
        if (files.first == NULL) {
            LC_SendErrorMessagef(NULL, NULL, "no valid .lc files in '%.*s'", LC_Expand(n->apackage.path));
            n->apackage.state = LC_DeclState_Error;
            L->errors += 1;
            return;
        }
    }

    for (LC_StringNode *it = files.first; it; it = it->next) {
        LoadedFile file = LC_ReadFileHook(n, it->string);
        if (file.content.str == NULL) file.content.str = "";

        LC_AST *ast_file = LC_ParseFile(n, file.path.str, file.content.str, file.line);
        if (!ast_file) {
            n->apackage.state = LC_DeclState_Error;
            return;
        }
    }
}

LC_FUNCTION void LC_ParsePackagesUsingRegistry(LC_Intern name) {
    LC_AST *n = LC_GetPackageByName(name);
    if (!n) {
        LC_SendErrorMessagef(NULL, NULL, "no package with name '%s'\n", name);
        L->errors += 1;
        return;
    }
    if (n->apackage.ext->scope) {
        return;
    }
    LC_ParsePackage(n);
    LC_ASTRefList imports = LC_GetPackageImports(n);
    for (LC_ASTRef *it = imports.first; it; it = it->next) {
        LC_ParsePackagesUsingRegistry(it->ast->gimport.path);
    }
}

LC_FUNCTION void LC_BuildIfPass(void) {
    LC_ASTFor(n, L->fpackage) {
        for (LC_AST *fit = n->apackage.ext->ffile; fit;) {
            LC_AST *next = fit->next;

            LC_AST *build_if = LC_HasNote(fit, L->ibuild_if);
            if (build_if) {
                if (!LC_ResolveBuildIf(build_if)) {
                    LC_DLLRemove(n->apackage.ext->ffile, n->apackage.ext->lfile, fit);
                    LC_AddASTToRefList(&L->discarded, fit);
                    fit = next;
                    continue;
                }
            }

            for (LC_AST *dit = fit->afile.fdecl; dit; dit = dit->next) {
                LC_AST *build_if = LC_HasNote(dit, L->ibuild_if);
                if (build_if) {
                    if (!LC_ResolveBuildIf(build_if)) {
                        LC_DLLRemove(fit->afile.fdecl, fit->afile.ldecl, dit);
                        LC_AddASTToRefList(&L->discarded, dit);
                    }
                }
            }

            fit = next;
        }
    }
}

LC_FUNCTION void LC_AddOrderedPackageToRefList(LC_AST *n) {
    LC_ASTRefList *ordered = &L->ordered_packages;
    for (LC_ASTRef *it = ordered->first; it; it = it->next) {
        if (it->ast->apackage.name == n->apackage.name) {
            return;
        }
    }
    LC_AddASTToRefList(ordered, n);
}

// Here we use import statements to produce a list of ordered packages.
// While we are at it we also resolve most top level declarations. I say
// most because aggregations are handled a bit differently, their resolution
// is deffered. This is added because a pointer doesn't require full typeinfo of
// an aggregate. It's just a number.
LC_FUNCTION LC_AST *LC_OrderPackagesAndBasicResolve(LC_AST *pos, LC_Intern name) {
    LC_AST *n = LC_GetPackageByName(name);
    if (n->apackage.state == LC_DeclState_Error) {
        return NULL;
    }
    if (n->apackage.state == LC_DeclState_Resolved) {
        // This function can be called multiple times, I assume user might
        // want to use type information to generate something. Pattern:
        // typecheck -> generate -> typecheck is expected!
        LC_PackageDecls(n);
        return n;
    }
    if (n->apackage.state == LC_DeclState_Resolving) {
        LC_ReportASTError(pos, "circular import '%s'", name);
        n->apackage.state = LC_DeclState_Error;
        return NULL;
    }
    LC_ASSERT(pos, n->apackage.state == LC_DeclState_Unresolved);
    n->apackage.state = LC_DeclState_Resolving;

    LC_Operand op = LC_ImportPackage(NULL, n, L->builtin_package);
    LC_ASSERT(pos, !LC_IsError(op));

    // Resolve all imports regardless of errors.
    // If current package has wrong import it means it's also
    // wrong but it should still look into all imports
    // despite this.
    int           wrong_import = 0;
    LC_ASTRefList refs         = LC_GetPackageImports(n);
    for (LC_ASTRef *it = refs.first; it; it = it->next) {
        LC_AST *import = LC_OrderPackagesAndBasicResolve(it->ast, it->ast->gimport.path);
        if (!import) {
            n->apackage.state = LC_DeclState_Error;
            wrong_import += 1;
            continue;
        }

        LC_Operand op = LC_ImportPackage(it->ast, n, import);
        if (LC_IsError(op)) {
            n->apackage.state = LC_DeclState_Error;
            wrong_import += 1;
            continue;
        }
    }

    if (wrong_import) return NULL;

    LC_PackageDecls(n);
    LC_AddOrderedPackageToRefList(n);
    n->apackage.state = LC_DeclState_Resolved;
    return n;
}

LC_FUNCTION void LC_OrderAndResolveTopLevelDecls(LC_Intern name) {
    L->first_package = name;
    LC_OrderPackagesAndBasicResolve(NULL, name);

    // Resolve still incomplete aggregate types, this operates on all packages
    // that didn't have errors so even if something broke in package ordering
    // it should still be fine to go forward with this and also proc body analysis
    for (LC_ASTRef *it = L->ordered_packages.first; it; it = it->next) {
        LC_AST *package = it->ast;
        LC_ASSERT(package, package->apackage.state == LC_DeclState_Resolved);
        LC_ResolveIncompleteTypes(package);
    }
}

LC_FUNCTION void LC_ResolveAllProcBodies(void) {
    // We don't need to check errors, only valid packages should have been put into
    // the list.
    for (LC_ASTRef *it = L->ordered_packages.first; it; it = it->next) {
        LC_AST *package = it->ast;
        LC_ASSERT(package, package->apackage.state == LC_DeclState_Resolved);
        LC_ResolveProcBodies(package);
    }
}

LC_FUNCTION LC_ASTRefList LC_ResolvePackageByName(LC_Intern name) {
    LC_ParsePackagesUsingRegistry(name);
    LC_BuildIfPass();
    LC_ASTRefList empty = {0};
    if (L->errors) return empty;

    LC_OrderAndResolveTopLevelDecls(name);
    LC_ResolveAllProcBodies();
    return L->ordered_packages;
}

LC_FUNCTION LC_String LC_GenerateUnityBuild(LC_ASTRefList packages) {
    if (L->errors) return LC_MakeEmptyString();

    LC_BeginStringGen(L->arena);

    LC_GenLinef("#include <stdbool.h>");
    LC_GenLinef("#include <stddef.h>");
    LC_GenLinef("#ifndef LC_String_IMPL");
    LC_GenLinef("#define LC_String_IMPL");
    LC_GenLinef("typedef struct { char *str; long long len; } LC_String;");
    LC_GenLinef("#endif");
    LC_GenLinef("#ifndef LC_Any_IMPL");
    LC_GenLinef("#define LC_Any_IMPL");
    LC_GenLinef("typedef struct { int type; void *data; } LC_Any;");
    LC_GenLinef("#endif");

    LC_GenLinef("#ifndef LC_Alignof");
    LC_GenLinef("#if defined(__TINYC__)");
    LC_GenLinef("#define LC_Alignof(...) __alignof__(__VA_ARGS__)");
    LC_GenLinef("#else");
    LC_GenLinef("#define LC_Alignof(...) _Alignof(__VA_ARGS__)");
    LC_GenLinef("#endif");
    LC_GenLinef("#endif");
    LC_GenLinef("void *memset(void *, int, size_t);");

    for (LC_ASTRef *it = packages.first; it; it = it->next) LC_GenCHeader(it->ast);
    for (LC_ASTRef *it = packages.first; it; it = it->next) LC_GenCImpl(it->ast);
    LC_String s = LC_EndStringGen(L->arena);
    return s;
}

LC_FUNCTION void LC_AddSingleFilePackage(LC_Intern name, LC_String path) {
    LC_AST *n        = LC_CreateAST(0, LC_ASTKind_Package);
    n->apackage.ext  = LC_PushStruct(L->arena, LC_ASTPackageExt);
    n->apackage.name = name;
    n->apackage.path = path;
    LC_AddNode(L->arena, &n->apackage.ext->injected_filepaths, path);
    LC_AddPackageToList(n);
}