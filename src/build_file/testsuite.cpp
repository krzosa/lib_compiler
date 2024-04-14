
enum TestResult {
    OK,
    Failed_Lex,
    Failed_Parse,
    Failed_Resolve,
    Failed_CCompile,
    Failed_Run,
    Failed_Condition,
    Failed_Package,
};

const char *TestResultString[] = {
    "OK",
    "Failed_Lex",
    "Failed_Parse",
    "Failed_Resolve",
    "Failed_CCompile",
    "Failed_Run",
    "Failed_Condition",
    "Failed_Package",
};

struct Test {
    char      *path;
    bool       msg_found[256];
    char      *msg_expected[256];
    int        msg_count;
    TestResult expected_result;
    bool       dont_run;
};

struct Task {
    bool      do_run;
    S8_String exe;
    S8_String cmd;
    Process   process;
};

enum TestKind {
    TestKind_File,
    TestKind_Dir,
};

struct TestDesc {
    TestKind  kind;
    S8_String absolute_path;
    S8_String filename;
    S8_String package_name;
    bool      dont_run;
};

TestResult Compile(S8_String name, S8_String file_to_compile) {
    TestResult  result  = OK;
    int         errcode = 0;
    Array<Task> tasks   = {MA_GetAllocator(Perm)};

    {
        Task tcc   = {};
        tcc.do_run = UseTCC;
        tcc.exe    = Fmt("%.*s/tcc_%.*s.exe", S8_Expand(name), S8_Expand(name));
        tcc.cmd    = Fmt("tcc -std=c99 %.*s " IF_MAC("-lm") IF_LINUX("-lm") " -o %.*s", S8_Expand(file_to_compile), S8_Expand(tcc.exe));
        tasks.add(tcc);

        Task clang   = {};
        clang.do_run = UseClang;
        clang.exe    = Fmt("%.*s/clang_%.*s.exe", S8_Expand(name), S8_Expand(name));
        clang.cmd    = Fmt("clang -Wno-string-plus-int -g -std=c99 " IF_MAC("-lm") IF_LINUX("-lm") " %.*s -o %.*s", S8_Expand(file_to_compile), S8_Expand(clang.exe));
        tasks.add(clang);

        Task cl   = {};
        cl.do_run = UseCL;
        cl.exe    = Fmt("%.*s/cl_%.*s.exe", S8_Expand(name), S8_Expand(name));
        cl.cmd    = Fmt("cl %.*s -Zi -std:c11 -nologo /Fd:%.*s.pdb -FC -Fe:%.*s", S8_Expand(file_to_compile), S8_Expand(file_to_compile), S8_Expand(cl.exe));
        tasks.add(cl);

        Task gcc   = {};
        gcc.do_run = UseGCC;
        gcc.exe    = Fmt("%.*s/gcc_%.*s.exe", S8_Expand(name), S8_Expand(name));
        gcc.cmd    = Fmt("gcc -Wno-string-plus-int -g -std=c99 " IF_MAC("-lm") IF_LINUX("-lm") " %.*s -o %.*s", S8_Expand(file_to_compile), S8_Expand(gcc.exe));
        tasks.add(gcc);
    }

    For(tasks) {
        if (!it.do_run) continue;
        errcode = Run(it.cmd);
        if (errcode != 0) {
            result = Failed_CCompile;
            goto end_of_test;
        }
        it.exe = OS_GetAbsolutePath(Perm, it.exe);
    }

    For(tasks) {
        if (!it.do_run) continue;
        it.process = RunEx(it.exe);
    }

    For(tasks) {
        if (!it.do_run) continue;
        errcode += Wait(&it.process);
    }

    if (errcode != 0) {
        result = Failed_Run;
        goto end_of_test;
    }
end_of_test:
    return result;
}

thread_local Test            *T;
thread_local Array<S8_String> Messages;

void HandleOutputMessageGlobalTest(LC_Token *pos, char *str, int len) {
    S8_String s = S8_Make(str, len);
    s           = S8_Trim(s);

    if (S8_StartsWith(s, S8_Lit("Executing: "), 0)) return;
    if (S8_StartsWith(s, S8_Lit("> "), 0)) return;
    if (T && T->msg_expected[0]) {
        for (int i = 0; i < 256; i += 1) {
            if (T->msg_expected[i] == 0) break;

            char *str   = T->msg_expected[i];
            bool *found = T->msg_found + i;
            if (!found[0]) {
                S8_String expected = S8_MakeFromChar(str);
                if (S8_Seek(s, expected, S8_FindFlag_IgnoreCase, NULL)) {
                    found[0] = true;
                    break;
                }
            }
        }
    }

    if (pos) s = S8_Format(Perm, "%s(%d,%d): error: %.*s\n", (char *)pos->lex->file, pos->line, pos->column, len, str);
    Messages.add(S8_Copy(Perm, s));
}

TestResult PrintGlobalTestResult(char *path, TestResult result) {
    const char *red   = "\033[31m";
    const char *reset = "\033[0m";
    const char *green = "\033[32m";
    if (!UseColoredIO) {
        red   = "";
        reset = "";
        green = "";
    }

    TestResult actual_result = result;
    if (result == T->expected_result) {
        result = OK;
    } else {
        result = Failed_Condition;
    }

    if (T->msg_expected[0]) {
        bool all_found = true;
        for (int i = 0; i < T->msg_count; i += 1) {
            if (T->msg_expected[i] == 0) {
                IO_Printf("%sCount of expected messages doesn't match expected messages%s\n", red, reset);
                all_found = false;
                break;
            }

            if (!T->msg_found[i]) {
                all_found = false;
            }
        }

        if (!all_found) {
            result = Failed_Condition;

            for (int i = 0; i < 256; i += 1) {
                char *str = T->msg_expected[i];
                if (str == 0) break;

                const char *color = green;
                if (!T->msg_found[i]) color = red;
                IO_Printf("%sMessage not found - %s%s\n", color, T->msg_expected[i], reset);
            }
        }
    }

    if (T->msg_count != Messages.len) {
        IO_Printf("%sCount of expected messages: %d doesn't match actual message count: %d%s\n", red, T->msg_count, Messages.len, reset);
        result = Failed_Condition;
    }

    const char *color = green;
    if (result != OK) {
        color = red;
        IO_Printf("%-50s - %s Expected result: %s, got instead: %s%s\n", path, color, TestResultString[T->expected_result], TestResultString[actual_result], reset);
    } else {
        IO_Printf("%-50s - %s%s%s\n", path, color, TestResultString[result], reset);
    }

    if (result != OK) {
        For(Messages) IO_Printf("%.*s", S8_Expand(it));
    }
    Messages.reset();
    return result;
}

void ParseErrorNotationAndFillGlobalTest(S8_String s) {
    S8_String count_lit    = S8_Lit("// #expected_error_count: ");
    S8_String errlit       = S8_Lit("// #error: ");
    S8_String resultlit    = S8_Lit("// #failed: ");
    S8_String dont_run_lit = S8_Lit("// #dont_run");
    S8_List   errors       = S8_Split(Perm, s, S8_Lit("\n"), 0);
    S8_For(it, errors) {
        if (S8_StartsWith(it->string, errlit, 0)) {
            S8_String error                 = S8_Skip(it->string, errlit.len);
            error                           = S8_Trim(error);
            error                           = S8_Copy(Perm, error);
            T->msg_expected[T->msg_count++] = error.str;
        }
        if (S8_StartsWith(it->string, resultlit, 0)) {
            if (S8_Seek(it->string, S8_Lit("parse"), 0, 0)) {
                T->expected_result = Failed_Parse;
            }
            if (S8_Seek(it->string, S8_Lit("resolve"), 0, 0)) {
                T->expected_result = Failed_Resolve;
            }
            if (S8_Seek(it->string, S8_Lit("lex"), 0, 0)) {
                T->expected_result = Failed_Lex;
            }
            if (S8_Seek(it->string, S8_Lit("package"), 0, 0)) {
                T->expected_result = Failed_Package;
            }
        }
        if (S8_StartsWith(it->string, count_lit, 0)) {
            S8_String count = S8_Skip(it->string, count_lit.len);
            count           = S8_Copy(Perm, count);
            T->msg_count    = atoi(count.str);
        }
        if (S8_StartsWith(it->string, dont_run_lit, 0)) {
            T->dont_run = true;
        }
    }
}

S8_String PushDir(S8_String dir_name) {
    S8_String prev = OS_GetWorkingDir(Perm);
    OS_MakeDir(dir_name);
    OS_SetWorkingDir(dir_name);
    return prev;
}

void RunTestFile(TestDesc it) {
    LC_Lang *lang                     = LC_LangAlloc();
    lang->on_message                  = HandleOutputMessageGlobalTest;
    lang->use_colored_terminal_output = UseColoredIO;
    lang->breakpoint_on_error         = BreakpointOnError;
    LC_LangBegin(lang);

    Test test         = {};
    test.dont_run     = it.dont_run;
    T                 = &test;
    TestResult result = OK;

    S8_String s = OS_ReadFile(L->arena, it.absolute_path);
    if (s.len == 0) {
        IO_FatalErrorf("Failed to open file %.*s", S8_Expand(it.absolute_path));
    }

    ParseErrorNotationAndFillGlobalTest(s);

    S8_String code     = {};
    S8_String out_path = {};

    LC_RegisterPackageDir("../../pkgs");

    LC_Intern name = LC_ILit("testing");
    LC_AddSingleFilePackage(name, it.absolute_path);

    L->first_package = name;
    LC_ParsePackagesPass(name);
    LC_BuildIfPass();
    if (L->errors) {
        result = Failed_Parse;
        goto end_of_test;
    }

    LC_OrderAndResolveTopLevelPass(name);
    LC_ResolveProcBodiesPass();
    LC_FindUnusedLocalsAndRemoveUnusedGlobalDeclsPass();
    if (L->errors) {
        result = Failed_Resolve;
        goto end_of_test;
    }

    DebugVerifyAST(L->ordered_packages);

    OS_MakeDir(it.filename);
    code     = LC_GenerateUnityBuild();
    out_path = S8_Format(L->arena, "%.*s/%.*s.c", S8_Expand(it.filename), S8_Expand(it.filename));
    OS_WriteFile(out_path, code);

    {
        LC_BeginStringGen(L->arena);
        LC_GenLCNode(L->ordered_packages.first->ast);
        S8_String c = LC_EndStringGen(L->arena);
        S8_String p = S8_Format(L->arena, "%.*s/%.*s.generated.lc", S8_Expand(it.filename), S8_Expand(it.filename));
        p           = OS_GetAbsolutePath(L->arena, p);
        OS_WriteFile(p, c);
        LC_ParseFile(NULL, p.str, c.str, 0);
    }

    if (!T->dont_run) result = Compile(it.filename, out_path);

end_of_test:
    result = PrintGlobalTestResult(it.filename.str, result);
    LC_LangEnd(lang);
}

void RunTestDir(TestDesc it, S8_String package_name) {
    LC_Lang *lang                     = LC_LangAlloc();
    lang->on_message                  = HandleOutputMessageGlobalTest;
    lang->use_colored_terminal_output = UseColoredIO;
    lang->breakpoint_on_error         = BreakpointOnError;
    LC_LangBegin(lang);

    Test test         = {};
    T                 = &test;
    TestResult result = OK;

    S8_String path_to_test_file = S8_Format(L->arena, "%.*s/test.txt", S8_Expand(it.absolute_path));
    S8_String error_notation    = OS_ReadFile(L->arena, path_to_test_file);
    if (error_notation.str) ParseErrorNotationAndFillGlobalTest(error_notation);

    LC_Intern first_package = LC_ILit(package_name.str);
    LC_RegisterPackageDir(it.absolute_path.str);
    LC_RegisterPackageDir("../../pkgs");
    LC_ParseAndResolve(first_package);
    LC_FindUnusedLocalsAndRemoveUnusedGlobalDeclsPass();
    if (L->errors != 0) result = Failed_Package;

    if (result == OK && T->expected_result == OK) {
        DebugVerifyAST(L->ordered_packages);

        OS_MakeDir(it.filename);
        S8_String code     = LC_GenerateUnityBuild();
        S8_String out_path = S8_Format(L->arena, "%.*s/%.*s.c", S8_Expand(it.filename), S8_Expand(it.filename));
        OS_WriteFile(out_path, code);
        if (!T->dont_run) result = Compile(it.filename, out_path);
    }

    result = PrintGlobalTestResult(it.filename.str, result);
    LC_LangEnd(lang);
}

void TestThread(Array<TestDesc> tests) {
    MA_InitScratch();
    For(tests) {
        if (it.kind == TestKind_Dir) {
            RunTestDir(it, it.package_name);
        } else if (it.kind == TestKind_File) {
            RunTestFile(it);
        }
    }
}

bool ShouldSkip(S8_String filename, bool is_directory = false) {
    if (TestsToRun.first) {
        bool found = false;
        S8_For(match, TestsToRun) {
            if (match->string == filename) {
                found = true;
                break;
            }
        }
        if (!found) {
            return true;
        }
    }

    if (is_directory) {
        if (S8_StartsWith(filename, "example_", true) || S8_StartsWith(filename, "compilation", true))
            return true;
    }

    return false;
}

bool ShouldRun(S8_String filename) {
    bool result = !ShouldSkip(filename, false);
    return result;
}

void RunTests() {
    MA_Scratch scratch;
    int        test_count  = 0;
    S8_String  working_dir = PushDir("test_result");

    Array<TestDesc> tests = {MA_GetAllocator(scratch)};

    for (OS_FileIter it = OS_IterateFiles(scratch, S8_Lit("../../tests")); OS_IsValid(it); OS_Advance(&it)) {
        if (ShouldSkip(it.filename, it.is_directory)) continue;
        TestDesc desc = {it.is_directory ? TestKind_Dir : TestKind_File, it.absolute_path, it.filename, "main"};
        tests.add(desc);
    }

    test_count                          = tests.len;
    const int              thread_count = ThreadCount;
    Array<Array<TestDesc>> work         = {MA_GetAllocator(scratch)};
    Array<std::thread *>   threads      = {MA_GetAllocator(scratch)};

    for (int i = 0; i < thread_count; i += 1) {
        work.add({MA_GetAllocator(scratch)});
    }

    int i = 0;
    For(tests) {
        work[i].add(it);
        i = (i + 1) % thread_count;
    }

    For(work) threads.add(new std::thread(TestThread, it));
    For(threads) it->join();

    //
    // Non automated tests
    //
    if (ShouldRun("example_ui_and_hot_reloading")) {
        test_count += 1;
        TestResult result = OK;
        Test       t      = {};
        t.path            = OS_GetAbsolutePath(scratch, "../../tests/example_ui_and_hot_reloading/").str;
        t.expected_result = OK;
        T                 = &t;

        LC_Lang *lang                     = LC_LangAlloc();
        lang->on_message                  = HandleOutputMessageGlobalTest;
        lang->use_colored_terminal_output = UseColoredIO;
        lang->breakpoint_on_error         = BreakpointOnError;
        LC_LangBegin(lang);
        LC_RegisterPackageDir(T->path);
        LC_ParseAndResolve(LC_ILit("dll"));
        LC_ParseAndResolve(LC_ILit("exe"));
        result = L->errors >= 1 ? Failed_Package : OK;

        if (result == OK) {
            DebugVerifyAST(L->ordered_packages);
            // DebugVerifyAST(exe_packages);

            S8_String prev = PushDir("example_ui_and_hot_reloading");
            LC_String code = LC_GenerateUnityBuild();
            OS_WriteFile("example_ui_and_hot_reloading", code);
            OS_SetWorkingDir(prev);
        }

        LC_LangEnd(lang);
        PrintGlobalTestResult("example_ui_and_hot_reloading", result);
#if 0
        OS_SystemF("clang unity_exe.c -o platform.exe -g -O0 -I\"../..\"");
        OS_SystemF("clang unity_dll.c -o game.dll -O0 -shared -g -I\"../..\" -Wl,-export:APP_Update");
#endif
    }

    OS_SetWorkingDir(working_dir);
    IO_Printf("Total test count = %d\n", test_count);
}
