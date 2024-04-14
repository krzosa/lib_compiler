#include "../src/core/core.c"

#define LC_USE_CUSTOM_ARENA
#define LC_Arena MA_Arena
#define LC__PushSizeNonZeroed MA__PushSizeNonZeroed
#define LC__PushSize MA__PushSize
#define LC_InitArena MA_Init
#define LC_DeallocateArena MA_DeallocateArena
#define LC_BootstrapArena MA_Bootstrap
#define LC_TempArena MA_Temp
#define LC_BeginTemp MA_BeginTemp
#define LC_EndTemp MA_EndTemp

#define LC_String S8_String
#define LIB_COMPILER_IMPLEMENTATION
#include "../lib_compiler.h"

int main(int argc, char **argv) {
    bool     colored = LC_EnableTerminalColors();
    MA_Arena arena   = {0};
    S8_List  dirs    = {0};

    CmdParser p                  = MakeCmdParser(&arena, argc, argv, "I'm a tool that compiles lc packages - for example: ./lc.exe <package>");
    p.require_one_standalone_arg = true;
    AddList(&p, &dirs, "dirs", "additional directories where I can find packages!");
    if (!ParseCmd(&arena, &p))
        return 0;

    IO_Assert(p.args.node_count == 1);
    S8_String package         = p.args.first->string;
    S8_String path_to_package = S8_ChopLastSlash(package);
    path_to_package           = S8_Copy(&arena, path_to_package);

    package = S8_SkipToLastSlash(package);
    package = S8_Copy(&arena, package);

    LC_Lang *lang                     = LC_LangAlloc();
    lang->use_colored_terminal_output = colored;
    LC_LangBegin(lang);
    LC_RegisterPackageDir("pkgs");
    LC_RegisterPackageDir(path_to_package.str);
    S8_For(it, dirs) { LC_RegisterPackageDir(it->string.str); }

    LC_Intern     name     = LC_InternStrLen(package.str, (int)package.len);
    LC_ASTRefList packages = LC_ParseAndResolve(name);
    if (lang->errors) return 1;

    S8_String code = LC_GenerateUnityBuild(packages);
    OS_WriteFile(S8_Lit("output.c"), code);
}