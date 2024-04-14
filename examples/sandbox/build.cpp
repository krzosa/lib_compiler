namespace Sandbox {
Array<LC_Intern> TypesToGen;

void RegisterMarkedTypes(LC_AST *n) {
    if (n->kind == LC_ASTKind_TypespecIdent) {
        S8_String name     = S8_MakeFromChar((char *)n->eident.name);
        S8_String array_of = "ArrayOf";
        if (S8_StartsWith(name, array_of)) {
            if (name == "ArrayOfName") return;

            name             = S8_Skip(name, array_of.len);
            LC_Intern intern = LC_InternStrLen(name.str, (int)name.len);

            bool exists = false;
            For(TypesToGen) {
                if (intern == it) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                TypesToGen.add(intern);
            }
        }
    }
}

void WalkAndRename(LC_ASTWalker *ctx, LC_AST *n) {
    LC_Intern *p = NULL;
    if (n->kind == LC_ASTKind_TypespecIdent || n->kind == LC_ASTKind_ExprIdent) {
        p = &n->eident.name;
    }
    if (LC_IsDecl(n)) {
        p = &n->dbase.name;
    }

    if (p) {
        LC_Intern user = *(LC_Intern *)ctx->user_data;
        S8_String p8   = S8_MakeFromChar((char *)*p);

        if (S8_Seek(p8, "Name")) {
            S8_String new_name = S8_ReplaceAll(L->arena, p8, "Name", S8_MakeFromChar((char *)user));
            *p                 = LC_InternStrLen(new_name.str, (int)new_name.len);
        }
    }
}

void BeforeCallArgsResolved(LC_AST *n, LC_Type *type) {
}

} // namespace Sandbox

bool sandbox() {
    using namespace Sandbox;

    LC_Lang *lang                     = LC_LangAlloc();
    lang->use_colored_terminal_output = UseColoredIO;
    lang->breakpoint_on_error         = BreakpointOnError;
    lang->on_typespec_parsed          = RegisterMarkedTypes;
    lang->before_call_args_resolved   = BeforeCallArgsResolved;
    LC_LangBegin(lang);

    LC_RegisterPackageDir("../pkgs");
    LC_RegisterPackageDir("../examples");

    LC_Intern name = LC_ILit("sandbox");
    LC_ParsePackagesUsingRegistry(name);
    LC_BuildIfPass();
    if (L->errors) {
        LC_LangEnd(lang);
        return false;
    }

    TypesToGen.allocator = MA_GetAllocator(lang->arena);

    //
    // Remove dynamic array file
    //
    LC_AST *package            = LC_GetPackageByName(name);
    LC_AST *dynamic_array_file = NULL;
    LC_ASTFor(it, package->apackage.ext->ffile) {
        S8_String path = S8_MakeFromChar((char *)it->afile.x->file);
        if (S8_EndsWith(path, "dynamic_array.lc")) {
            dynamic_array_file = it;
            DLL_QUEUE_REMOVE(package->apackage.ext->ffile, package->apackage.ext->lfile, it);
            break;
        }
    }

    // Generate copies
    LC_ASTWalker walker = LC_GetDefaultWalker(L->arena, WalkAndRename);
    For(TypesToGen) {
        LC_AST *new_array_file = LC_CopyAST(L->arena, dynamic_array_file);

        walker.user_data = (void *)&it;
        LC_WalkAST(&walker, new_array_file);
        LC_DLLAdd(package->apackage.ext->ffile, package->apackage.ext->lfile, new_array_file);
    }

    LC_OrderAndResolveTopLevelDecls(name);
    LC_ResolveAllProcBodies();
    if (L->errors) {
        LC_LangEnd(lang);
        return false;
    }

    S8_String code = LC_GenerateUnityBuild(L->ordered_packages);
    S8_String path = "examples/sandbox/sandbox.c";

    OS_MakeDir("examples/sandbox");
    OS_WriteFile(path, code);
    LC_LangEnd(lang);

    if (UseCL) {
        OS_CopyFile(RaylibDLL, "examples/sandbox/raylib.dll", true);
        S8_String cmd     = Fmt("cl %.*s -Zi -std:c11 -nologo -FC -Fd:examples/sandbox/a.pdb -Fe:examples/sandbox/sandbox.exe %.*s", S8_Expand(path), S8_Expand(RaylibLIB));
        int       errcode = Run(cmd);
        if (errcode != 0) return false;
    }

    return true;
}