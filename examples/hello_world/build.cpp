bool hello_world() {
    LC_Lang *lang                     = LC_LangAlloc();
    lang->use_colored_terminal_output = UseColoredIO;
    lang->breakpoint_on_error         = BreakpointOnError;
    LC_LangBegin(lang);
    LC_RegisterPackageDir("../examples/");
    LC_RegisterPackageDir("../pkgs");

    LC_Intern name = LC_ILit("hello_world");
    LC_ParseAndResolve(name);
    if (lang->errors) {
        LC_LangEnd(lang);
        return false;
    }

    OS_MakeDir("examples/hello_world");
    LC_ParseAndResolve(name);
    LC_String code = LC_GenerateUnityBuild();

    S8_String path = "examples/hello_world/hello_world.c";
    OS_WriteFile(path, code);
    LC_LangEnd(lang);
    if (UseCL) {
        S8_String cmd = Fmt("cl %.*s -nologo -FC -Fd:examples/hello_world/hello_world.pdb -Fe:examples/hello_world/hello_world.exe", S8_Expand(path));
        if (Run(cmd) != 0) return false;
    }
    return true;
}