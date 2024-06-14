/* This build file runs all the tests, compiles them using all
available compilers, cleans the house and walks the dog. Wowie.
*/
#include "src/build_tool/library.cpp"

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
#include "src/compiler/lib_compiler.c"

#include <thread>

bool    UseClang;
bool    UseCL;
bool    UseTCC;
bool    UseGCC;
bool    QuickRun;
bool    BuildX64Sandbox;
bool    BreakpointOnError;
S8_List TestsToRun;
bool    UseColoredIO;
int     ThreadCount = 24;
bool    PrintAllFunctions;

S8_String RaylibLIB;
S8_String RaylibDLL;

#include "src/build_file/ast_verify.cpp"
#include "src/build_file/test_readme.cpp"
#include "src/build_file/testsuite.cpp"
#include "src/build_file/package_compiler.cpp"

#include "examples/add_printf_format_check/build.cpp"
#include "examples/pathfinding_visualizer/build.cpp"
#include "examples/generate_type_info/build.cpp"
#include "examples/add_dynamic_array_macro/build.cpp"
#include "examples/text_editor/build.cpp"
#include "examples/hello_world/build.cpp"
#include "examples/create_raylib_window/build.cpp"
#include "examples/add_instrumentation/build.cpp"
#include "examples/use_as_data_format_with_typechecking/build.cpp"
#include "examples/add_source_location_macro/build.cpp"

int main(int argc, char **argv) {
    MA_InitScratch();
    UseColoredIO = OS_EnableTerminalColors();

    {
        CmdParser p = MakeCmdParser(Perm, argc, argv, "I'm a build tool for this codebase, by default I build the entire test suite");
        AddBool(&p, &BuildX64Sandbox, "build-x64-sandbox", "build the x64 sandbox program using msvc");
        AddBool(&p, &QuickRun, "quick", "build tests using tcc compiler only");
        AddBool(&p, &BreakpointOnError, "breakpoint", "breakpoint if a compiler error is thrown");
        AddBool(&p, &PrintAllFunctions, "print-functions", "prints all functions marked with LC_FUNCTION");
        AddInt(&p, &ThreadCount, "threads", "number of threads to use when running the test suite");
        AddList(&p, &TestsToRun, "tests", "run only these particular tests");
        if (!ParseCmd(Perm, &p)) return 0;
    }

    //
    // Find compilers in the path
    //
    {
        char            *path_string = getenv("PATH");
        Array<S8_String> path        = Split(path_string, IF_WINDOWS_ELSE(";", ":"));

        char *end = IF_WINDOWS_ELSE(".exe", "");
        For(path) {
            S8_String clpath    = Fmt("%.*s/cl%s", S8_Expand(it), end);
            S8_String clangpath = Fmt("%.*s/clang%s", S8_Expand(it), end);
            S8_String tccpath   = Fmt("%.*s/tcc%s", S8_Expand(it), end);
            S8_String gccpath   = Fmt("%.*s/gcc%s", S8_Expand(it), end);

            if (OS_FileExists(clpath)) UseCL = true;
            if (OS_FileExists(clangpath)) UseClang = true;
            if (OS_FileExists(tccpath)) UseTCC = true;
#if OS_WINDOWS == 0
            if (OS_FileExists(gccpath)) UseGCC = true;
#endif
        }
    }

    if (UseClang == false && UseCL == false && UseTCC == false && UseGCC == false) {
        IO_FatalErrorf("Found no supported compiler on the PATH! Make sure to have GCC, Clang, CL/MSVC or TCC on the PATH");
    }

    if (QuickRun) {
        if (!UseTCC) IO_FatalErrorf("TCC is not on the path");
        UseClang = false;
        UseCL    = false;
        UseGCC   = false;
        UseTCC   = true;
    }

    RaylibLIB = OS_GetAbsolutePath(Perm, "../pkgs/raylib/raylib-5.0_win64_msvc16/lib/raylibdll.lib");
    RaylibDLL = OS_GetAbsolutePath(Perm, "../pkgs/raylib/raylib-5.0_win64_msvc16/lib/raylib.dll");

    if (BuildX64Sandbox) {
        PushDir("x64_sandbox");
        S8_String cc = "cl";

        Array<S8_String> flags = {MA_GetAllocator(Perm)};
        flags += "/MP /Zi -D_CRT_SECURE_NO_WARNINGS";
        flags += "/FC /WX /W3 /wd4200 /diagnostics:column /nologo";
        flags += "/GF /Gm- /Oi";
        flags += "/GR- /EHa-";
        flags += "/D_DEBUG -RTC1 -Od";
        flags += Fmt("/Fe:x64main.exe");
        OS_DeleteFile("x64main.pdb");
        Run(cc + "../../src/x64/x64main.cpp" + flags);
        return 0;
    }

    PackageCompiler();
    IO_Printf("Compiler packed successfully: lib_compiler.h\n");

    if (ShouldRun("test_readme")) {
        TestReadme();
    }

    OS_MakeDir("examples");
    if (ShouldRun("add_printf_format_check")) {
        bool result = add_printf_format_check();
        if (result) IO_Printf("%-50s - OK\n", "add_printf_format_check");
        else IO_Printf("%-50s - ERROR\n", "add_printf_format_check");
    }

    if (ShouldRun("generate_type_info")) {
        bool result = generate_type_info();
        if (result) IO_Printf("%-50s - OK\n", "generate_type_info");
        else IO_Printf("%-50s - ERROR\n", "generate_type_info");
    }

    if (ShouldRun("add_dynamic_array_macro")) {
        bool result = add_dynamic_array_macro();
        if (result) IO_Printf("%-50s - OK\n", "add_dynamic_array_macro");
        else IO_Printf("%-50s - ERROR\n", "add_dynamic_array_macro");
    }

    if (ShouldRun("text_editor")) {
        bool result = text_editor();
        if (result) IO_Printf("%-50s - OK\n", "text_editor");
        else IO_Printf("%-50s - ERROR\n", "text_editor");
    }

    if (ShouldRun("pathfinding_visualizer")) {
        bool result = pathfinding_visualizer();
        if (result) IO_Printf("%-50s - OK\n", "pathfinding_visualizer");
        else IO_Printf("%-50s - ERROR\n", "pathfinding_visualizer");
    }

    if (ShouldRun("hello_world")) {
        bool result = hello_world();
        if (result) IO_Printf("%-50s - OK\n", "hello_world");
        else IO_Printf("%-50s - ERROR\n", "hello_world");
    }

    if (ShouldRun("create_raylib_window")) {
        bool result = create_raylib_window();
        if (result) IO_Printf("%-50s - OK\n", "create_raylib_window");
        else IO_Printf("%-50s - ERROR\n", "create_raylib_window");
    }

    if (ShouldRun("add_instrumentation")) {
        bool result = add_instrumentation();
        if (result) IO_Printf("%-50s - OK\n", "add_instrumentation");
        else IO_Printf("%-50s - ERROR\n", "add_instrumentation");
    }

    bool build_wasm = false;
    if (build_wasm && ShouldRun("wasm_playground") && UseClang) {
        OS_MakeDir("wasm_playground");
        int result = Run("clang --target=wasm32 -mbulk-memory -Oz -Wno-writable-strings --no-standard-libraries -Wl,--strip-all -Wl,--import-memory -Wl,--no-entry -o wasm_playground/playground.wasm ../src/wasm_playground/wasm_main.c -DOS_WASM=1");

        S8_String index    = OS_ReadFile(Perm, "../src/wasm_playground/index.html");
        S8_List   programs = S8_MakeEmptyList();

        OS_SetWorkingDir("wasm_playground"); // so that RegisterDir("../../pkgs") works
        for (OS_FileIter it = OS_IterateFiles(Perm, "../../src/wasm_playground/"); OS_IsValid(it); OS_Advance(&it)) {
            if (S8_EndsWith(it.filename, ".lc", false)) {
                RunTestFile({TestKind_File, it.absolute_path, it.filename, "not_needed", true});

                S8_String file = OS_ReadFile(Perm, it.absolute_path);
                file           = S8_ReplaceAll(Perm, file, S8_Lit("\\"), S8_Lit("\\\\"), true);
                S8_AddF(Perm, &programs, "`%.*s`,\n", S8_Expand(file));
            }
        }
        OS_SetWorkingDir("..");

        S8_String programs_string = S8_Merge(Perm, programs);
        S8_String new_index       = S8_ReplaceAll(Perm, index, "<InsertPrograms>", programs_string, false);

        OS_WriteFile("wasm_playground/playground.html", new_index);
        OS_CopyFile("../src/wasm_playground/run_server.bat", "wasm_playground/run_server.bat", true);

        if (result == 0) IO_Printf("%-50s - OK\n", "wasm_playground");
        else IO_Printf("%-50s - ERROR\n", "wasm_playground");
    }

    if (ShouldRun("use_as_data_format_with_typechecking")) {
        bool result = use_as_data_format_with_typechecking();
        if (result) IO_Printf("%-50s - OK\n", "use_as_data_format_with_typechecking");
        else IO_Printf("%-50s - ERROR\n", "use_as_data_format_with_typechecking");
    }

    if (ShouldRun("add_source_location_macro")) {
        bool result = add_source_location_macro();
        if (result) IO_Printf("%-50s - OK\n", "add_source_location_macro");
        else IO_Printf("%-50s - ERROR\n", "add_source_location_macro");
    }

    Array<Process> processes = {MA_GetAllocator(Perm)};
    if (ShouldRun("compilation")) {
        //
        // Test if things compile in C and C++ mode on all available compilers
        //
        S8_String        working_dir = PushDir("targets");
        Array<S8_String> files       = {MA_GetAllocator(Perm)};

        files.add("../../../tests/compilation/test_compile_packed.c");
        files.add("../../../tests/compilation/test_compile_packed_cpp.c");
        files.add("../../../tests/compilation/test_compile_with_core.c");
        files.add("../../../tests/compilation/test_compile_with_core_cpp.cpp");
        files.add("../../../tests/compilation/test_compile_header.cpp");
        files.add("../../../tools/lcompile.c");

        For(files) {
            S8_String name_no_ext     = S8_GetNameNoExt(it);
            S8_String exe             = Fmt("%.*s.exe", S8_Expand(name_no_ext));
            S8_String file_to_compile = it;
            bool      is_cpp          = S8_EndsWith(it, ".cpp");

            if (UseCL) {
                S8_String cc = "cl";

                Array<S8_String> flags = {MA_GetAllocator(Perm)};
                flags += "/Zi -D_CRT_SECURE_NO_WARNINGS";
                flags += "/FC /WX /W3 /wd4200 /diagnostics:column /nologo";
                flags += Fmt("/Fe:%.*s /Fd:%.*s.pdb", S8_Expand(exe), S8_Expand(name_no_ext));

                Array<S8_String> link = {MA_GetAllocator(Perm)};
                link += "/link /incremental:no";

                S8_String dir = Fmt("%.*s_cl_debug_" OS_NAME, S8_Expand(name_no_ext));
                Process   p   = RunEx(cc + file_to_compile + flags, dir);
                processes.add(p);
            }

            if (UseClang) {
                S8_String cc = "clang";

                Array<S8_String> flags = {MA_GetAllocator(Perm)};
                flags += "-g -Wno-write-strings";
                flags += "-fdiagnostics-absolute-paths";
                flags += "-fsanitize=address";
                if (is_cpp) flags += "-std=c++11";
                IF_LINUX(flags += "-lm";)
                flags += Fmt("-o %.*s", S8_Expand(exe));

                S8_String dir = Fmt("%.*s_clang_debug_" OS_NAME, S8_Expand(name_no_ext));
                Process   p   = RunEx(cc + file_to_compile + flags, dir);
                processes.add(p);
            }

            if (UseGCC) {
                S8_String cc = "gcc";

                Array<S8_String> flags = {MA_GetAllocator(Perm)};
                flags += "-g -Wno-write-strings";
                flags += "-fsanitize=address";
                if (is_cpp) flags += "-std=c++11";
                IF_LINUX(flags += "-lm";)
                flags += Fmt("-o %.*s", S8_Expand(exe));

                S8_String dir = Fmt("%.*s_gcc_debug_" OS_NAME, S8_Expand(name_no_ext));
                Process   p   = RunEx(cc + file_to_compile + flags, dir);
                processes.add(p);
            }
        }
        OS_SetWorkingDir(working_dir);
    }

    RunTests();

    //
    // Wait for compilation from before tests to finish
    //
    int result = 0;
    For(processes) {
        int exit_code = Wait(&it);
        if (exit_code != 0) result = exit_code;
    }

    return result;
}
