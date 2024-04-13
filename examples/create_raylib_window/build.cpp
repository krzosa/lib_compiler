bool create_raylib_window() {
    LC_Lang *lang                     = LC_LangAlloc();
    lang->use_colored_terminal_output = UseColoredIO;
    lang->breakpoint_on_error         = BreakpointOnError;
    LC_LangBegin(lang);
    LC_RegisterPackageDir("../examples/");
    LC_RegisterPackageDir("../pkgs");

    LC_Intern     name     = LC_ILit("create_raylib_window");
    LC_ASTRefList packages = LC_ResolvePackageByName(name);
    if (L->errors) {
        LC_LangEnd(lang);
        return false;
    }

    OS_MakeDir("examples/create_raylib_window");
    S8_String path = "examples/create_raylib_window/create_raylib_window.c";
    S8_String code = LC_GenerateUnityBuild(packages);
    OS_WriteFile(path, code);

    bool success = true;
    if (UseCL) {
        OS_CopyFile(RaylibDLL, "examples/create_raylib_window/raylib.dll", true);
        S8_String cmd     = Fmt("cl %.*s -Zi -std:c11 -nologo -FC -Fd:examples/create_raylib_window/a.pdb -Fe:examples/create_raylib_window/create_raylib_window.exe %.*s", S8_Expand(path), S8_Expand(RaylibLIB));
        int       errcode = Run(cmd);
        if (errcode != 0) success = false;
    }

    return success;
}