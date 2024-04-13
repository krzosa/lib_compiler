bool hello_world() {
    LC_Lang *lang                     = LC_LangAlloc();
    lang->use_colored_terminal_output = UseColoredIO;
    lang->breakpoint_on_error         = BreakpointOnError;
    LC_LangBegin(lang);
    LC_RegisterPackageDir("../examples/");
    LC_RegisterPackageDir("../pkgs");

    LC_Intern     name     = LC_ILit("hello_world");
    LC_ASTRefList packages = LC_ResolvePackageByName(name);
    if (L->errors) {
        LC_LangEnd(lang);
        return false;
    }

    OS_MakeDir("examples/hello_world");
    S8_String code = LC_GenerateUnityBuild(packages);
    S8_String path = "examples/hello_world/hello_world.c";
    OS_WriteFile(path, code);

    bool success = true;
    if (UseCL) {
        S8_String cmd = Fmt("cl %.*s -Zi -std:c11 -nologo -FC -Fd:examples/hello_world/hello_world.pdb -Fe:examples/hello_world/hello_world.exe %.*s", S8_Expand(path), S8_Expand(RaylibLIB));
        if (Run(cmd) != 0) success = false;
    }

    LC_LangEnd(lang);
    return success;
}