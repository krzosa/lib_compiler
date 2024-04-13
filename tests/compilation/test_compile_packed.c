#define LIB_COMPILER_IMPLEMENTATION
#include "../../lib_compiler.h"

int main(int argc, char **argv) {
    bool colored = LC_EnableTerminalColors();

    LC_StringList dirs  = {0};
    LC_Arena      arena = {0};

    LC_String package = LC_MakeEmptyString();
    for (int i = 1; i < argc; i += 1) {
        LC_String arg = LC_MakeFromChar(argv[i]);
        if (LC_StartsWith(arg, LC_Lit("dir="), LC_IgnoreCase)) {
            LC_String dir = LC_Skip(arg, 4);
            LC_AddNode(&arena, &dirs, dir);
        } else {
            package = arg;
        }
    }

    if (package.len == 0) {
        printf("package name not passed\n");
        return 0;
    }

    LC_Lang *lang                     = LC_LangAlloc();
    lang->use_colored_terminal_output = colored;
    LC_LangBegin(lang);
    LC_RegisterPackageDir(".");
    for (LC_StringNode *it = dirs.first; it; it = it->next) {
        LC_RegisterPackageDir(it->string.str);
    }

    LC_Intern     name     = LC_InternStrLen(package.str, (int)package.len);
    LC_ASTRefList packages = LC_ResolvePackageByName(name);
    if (lang->errors) return 1;

    LC_String code = LC_GenerateUnityBuild(packages);
    (void)code;
}