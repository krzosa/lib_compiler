bool OnDeclParsed_AddInstrumentation(bool discard, LC_AST *n) {
    if (discard) return discard;

    if (n->kind == LC_ASTKind_DeclProc && n->dproc.body) {
        if (n->dbase.name == LC_ILit("BeginProc")) return false;
        if (n->dbase.name == LC_ILit("EndProc")) return false;

        LC_AST *body = n->dproc.body;

        LC_AST *begin = LC_ParseStmtf("BeginProc(\"%s\", \"%s\", %d)", n->dbase.name, n->pos->lex->file, n->pos->line);
        LC_AST *end   = LC_ParseStmtf("defer EndProc();");

        LC_DLLAddFront(body->sblock.first, body->sblock.last, end);
        LC_DLLAddFront(body->sblock.first, body->sblock.last, begin);
    }
    return discard;
}

bool add_instrumentation() {
    LC_Lang *lang                     = LC_LangAlloc();
    lang->use_colored_terminal_output = UseColoredIO;
    lang->breakpoint_on_error         = BreakpointOnError;
    lang->on_decl_parsed              = OnDeclParsed_AddInstrumentation;
    LC_LangBegin(lang);

    LC_RegisterPackageDir("../pkgs");
    LC_RegisterPackageDir("../examples");

    LC_Intern     name     = LC_ILit("add_instrumentation");
    LC_ASTRefList packages = LC_ResolvePackageByName(name);
    if (L->errors) {
        LC_LangEnd(lang);
        return false;
    }

    DebugVerifyAST(packages);
    if (L->errors) {
        LC_LangEnd(lang);
        return false;
    }

    S8_String code = LC_GenerateUnityBuild(packages);
    S8_String path = "examples/add_instrumentation/add_instrumentation.c";
    OS_MakeDir("examples");
    OS_MakeDir("examples/add_instrumentation");
    OS_WriteFile(path, code);
    if (!UseCL) {
        LC_LangEnd(lang);
        return true;
    }

    S8_String cmd     = Fmt("cl %.*s -Zi -std:c11 -nologo -FC -Fd:examples/add_instrumentation/a.pdb -Fe:examples/add_instrumentation/add_instrumentation.exe %.*s", S8_Expand(path), S8_Expand(RaylibLIB));
    int       errcode = Run(cmd);
    if (errcode != 0) {
        LC_LangEnd(lang);
        return false;
    }
    // Run("examples/add_instrumentation/add_instrumentation.exe");

    LC_LangEnd(lang);
    return true;
}