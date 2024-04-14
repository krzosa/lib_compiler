void ReadmeReadFile(LC_AST *package, LoadedFile *file) {
    S8_String result = {};
    if (package->apackage.name == LC_ILit("readme_test")) {
        S8_String code = *(S8_String *)L->user_data;
        file->content  = S8_Copy(L->arena, code);
    } else {
        file->content = OS_ReadFile(L->arena, file->path);
    }
}

void TestReadme() {
    MA_Scratch scratch;

    S8_String readme_path = OS_GetAbsolutePath(scratch, "../README.md");
    S8_String readme      = OS_ReadFile(scratch, readme_path);
    S8_List   split       = S8_Split(scratch, readme, "```");
    S8_For(it, split) {
        if (S8_StartsWith(it->string, " odin", S8_IgnoreCase)) {
            S8_String code = S8_Skip(it->string, 5);

            LC_Lang *lang                     = LC_LangAlloc();
            lang->use_colored_terminal_output = UseColoredIO;
            lang->user_data                   = (void *)&code;
            lang->on_file_load                = ReadmeReadFile;
            LC_LangBegin(lang);
            LC_DeclareNote(LC_ILit("do_something"));
            LC_DeclareNote(LC_ILit("serialize"));
            defer { LC_LangEnd(lang); };

            LC_RegisterPackageDir("../pkgs");
            LC_Intern name = LC_ILit("readme_test");
            LC_AddSingleFilePackage(name, readme_path);
            LC_ParseAndResolve(name);
            // LC_FindUnusedLocalsAndRemoveUnusedGlobalDeclsPass();
        }
    }
}
