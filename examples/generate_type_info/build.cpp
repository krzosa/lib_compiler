bool generate_type_info() {
    OS_DeleteFile("../examples/generate_type_info/generated.lc");

    LC_Lang *lang                     = LC_LangAlloc();
    lang->use_colored_terminal_output = UseColoredIO;
    lang->breakpoint_on_error         = BreakpointOnError;
    LC_LangBegin(lang);

    LC_RegisterPackageDir("../pkgs");
    LC_RegisterPackageDir("../examples");

    LC_Intern name = LC_ILit("generate_type_info");

    LC_ParsePackagesUsingRegistry(name);
    if (L->errors) {
        LC_LangEnd(lang);
        return false;
    }

    LC_OrderAndResolveTopLevelDecls(name);
    if (L->errors) {
        LC_LangEnd(lang);
        return false;
    }

    LC_BeginStringGen(L->arena);
    LC_AST *package = LC_GetPackageByName(name);

    // Iterate through all the types and generate struct member information
    LC_Type *first = (LC_Type *)L->type_arena->memory.data;
    for (int i = 0; i < L->type_count; i += 1) {
        LC_Type *type = first + i;
        if (type->kind != LC_TypeKind_Struct) continue;

        LC_GenLinef("%s_Members := :[]StructMem{", (char *)type->decl->name);
        L->printer.indent += 1;

        LC_TypeFor(it, type->tagg.mems.first) {
            LC_GenLinef("{name = \"%s\", offset = %d},", (char *)it->name, it->offset);
        }

        L->printer.indent -= 1;
        LC_GenLinef("};");
    }

    S8_String code = LC_EndStringGen(L->arena);
    S8_String path = OS_GetAbsolutePath(L->arena, "../examples/generate_type_info/generated.lc");
    OS_WriteFile(path, code);

    LC_ParseFile(package, path.str, code.str, 0);
    if (L->errors) {
        LC_LangEnd(lang);
        return false;
    }

    // Resolve decls again with new content added in
    LC_OrderAndResolveTopLevelDecls(name);
    LC_ResolveAllProcBodies();
    LC_FindUnusedLocalsAndRemoveUnusedGlobalDecls();

    bool result = L->errors ? false : true;
    LC_LangEnd(lang);
    return result;
}