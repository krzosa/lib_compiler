bool text_editor() {
    LC_Lang *lang                     = LC_LangAlloc();
    lang->use_colored_terminal_output = UseColoredIO;
    lang->breakpoint_on_error         = BreakpointOnError;
    LC_LangBegin(lang);
    defer { LC_LangEnd(lang); };

    LC_RegisterPackageDir("../pkgs");
    LC_RegisterPackageDir("../examples/text_editor");

    LC_Intern name = LC_ILit("entry_point");
    LC_ParseAndResolve(name);
    if (L->errors) return false;

    DebugVerifyAST(L->ordered_packages);
    if (L->errors) return false;

    LC_FindUnusedLocalsAndRemoveUnusedGlobalDeclsPass();

    OS_MakeDir("examples");
    OS_MakeDir("examples/text_editor");
    OS_CopyFile(RaylibDLL, "examples/text_editor/raylib.dll", true);

    LC_String code = LC_GenerateUnityBuild();
    S8_String path = "examples/text_editor/text_editor.c";
    OS_WriteFile(path, code);

    if (!UseCL) return true;
    S8_String cmd     = Fmt("cl %.*s -Zi -std:c11 -nologo -FC -Fd:examples/text_editor/a.pdb -Fe:examples/text_editor/text_editor.exe %.*s", S8_Expand(path), S8_Expand(RaylibLIB));
    int       errcode = Run(cmd);
    if (errcode != 0) return false;

    return true;
}