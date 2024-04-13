bool text_editor() {
    LC_Lang *lang                     = LC_LangAlloc();
    lang->use_colored_terminal_output = UseColoredIO;
    lang->breakpoint_on_error         = BreakpointOnError;
    LC_LangBegin(lang);
    defer { LC_LangEnd(lang); };

    LC_RegisterPackageDir("../pkgs");
    LC_RegisterPackageDir("../examples");

    LC_Intern     name     = LC_ILit("text_editor");
    LC_ASTRefList packages = LC_ResolvePackageByName(name);
    if (L->errors) return false;

    DebugVerifyAST(packages);
    if (L->errors) return false;

    LC_FindUnusedLocalsAndRemoveUnusedGlobalDecls();

    OS_MakeDir("examples");
    OS_MakeDir("examples/text_editor");
    OS_CopyFile(RaylibDLL, "examples/text_editor/raylib.dll", true);

    S8_String code = LC_GenerateUnityBuild(packages);
    S8_String path = "examples/text_editor/text_editor.c";
    OS_WriteFile(path, code);

    if (!UseCL) return true;
    S8_String cmd     = Fmt("cl %.*s -Zi -std:c11 -nologo -FC -Fd:examples/text_editor/a.pdb -Fe:examples/text_editor/text_editor.exe %.*s", S8_Expand(path), S8_Expand(RaylibLIB));
    int       errcode = Run(cmd);
    if (errcode != 0) return false;

    return true;
}