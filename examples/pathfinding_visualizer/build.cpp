bool pathfinding_visualizer() {
    LC_Lang *lang                     = LC_LangAlloc();
    lang->use_colored_terminal_output = UseColoredIO;
    lang->breakpoint_on_error         = BreakpointOnError;
    LC_LangBegin(lang);

    LC_RegisterPackageDir("../examples/");
    LC_RegisterPackageDir("../pkgs");

    LC_Intern     name     = LC_ILit("pathfinding_visualizer");
    LC_ASTRefList packages = LC_ResolvePackageByName(name);
    LC_FindUnusedLocalsAndRemoveUnusedGlobalDecls();
    if (L->errors) {
        LC_LangEnd(lang);
        return false;
    }

    DebugVerifyAST(packages);
    if (L->errors) {
        LC_LangEnd(lang);
        return false;
    }

    OS_MakeDir("examples");
    OS_MakeDir("examples/pathfinding_visualizer");
    S8_String code = LC_GenerateUnityBuild(packages);
    S8_String path = "examples/pathfinding_visualizer/pathfinding_visualizer.c";
    OS_WriteFile(path, code);

    if (!UseCL) {
        LC_LangEnd(lang);
        return true;
    }

    S8_String cmd     = Fmt("cl %.*s -Zi -std:c11 -nologo -FC -Fd:examples/pathfinding_visualizer/a.pdb -Fe:examples/pathfinding_visualizer/pathfinding_visualizer.exe %.*s", S8_Expand(path), S8_Expand(RaylibLIB));
    int       errcode = Run(cmd);
    OS_CopyFile(RaylibDLL, "examples/pathfinding_visualizer/raylib.dll", true);

    LC_LangEnd(lang);
    bool result = errcode == 0;
    return result;
}