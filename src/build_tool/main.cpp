#include "library.cpp"

int main(int argument_count, char **arguments) {
    MA_InitScratch();
    OS_MakeDir(S8_Lit("build"));
    OS_SetWorkingDir(S8_Lit("build"));
    S8_String working_dir = OS_GetWorkingDir(Perm);
    IO_Printf("WORKING DIR: %.*s\n", S8_Expand(working_dir));

    Array<S8_String> cmd = CMD_Make(arguments, argument_count);
    S8_String        cc  = CMD_Get(cmd, "cc", IF_WINDOWS("cl") IF_MAC("clang") IF_LINUX("gcc"));

    // Search for build file in the project directory
    S8_String build_file = {};
    {
        for (OS_FileIter it = OS_IterateFiles(Perm, S8_Lit("..")); OS_IsValid(it); OS_Advance(&it)) {
            if (S8_Seek(it.filename, S8_Lit("build_file.c"), S8_IgnoreCase)) {
                build_file = it.absolute_path;
            }
        }

        if (build_file.str == 0) {
            IO_Printf("Couldnt find build file in current dir: %.*s, exiting ... \n", S8_Expand(working_dir));
            IO_Printf("- Proper build file contains 'build_file.c' in it's name\n");
            IO_Printf("- Alternative compiler can be chosen like this: bld cc=clang\n");
            return 0;
        }
    }

    SRC_InitCache(Perm, "build_tool.cache");
    S8_String name_no_ext = S8_GetNameNoExt(build_file);
    S8_String exe_name    = S8_Format(Perm, "%.*s.exe", S8_Expand(name_no_ext));

    // Compile the build file only if code changed
    if (SRC_WasModified(build_file, exe_name)) {
        double time   = OS_GetTime();
        int    result = 0;
        if (cc == "cl") {
            Array<S8_String> flags = {MA_GetAllocator(Perm)};
            flags += "-nologo -Zi";
            flags += "-WX -W3 -wd4200 -diagnostics:column";
            flags += Fmt("/Fe:%.*s /Fd:%.*s.pdb", S8_Expand(exe_name), S8_Expand(exe_name));
            result = Run(cc + build_file + flags);
        } else if (cc == "clang") {
            Array<S8_String> flags = {MA_GetAllocator(Perm)};
            cc                     = "clang++";

            flags += "-std=c++11 -g";
            flags += "-fdiagnostics-absolute-paths";
            flags += "-Wno-writable-strings";
            flags += "-lm";
            flags += Fmt("-o %.*s", S8_Expand(exe_name));
            result = Run(cc + build_file + flags);
        } else {
            IO_Assert(cc == "gcc");
            cc = "g++";

            Array<S8_String> flags = {MA_GetAllocator(Perm)};
            flags += "-std=c++11 -g";
            flags += "-Wno-write-strings";
            flags += "-lm";
            flags += Fmt("-o %.*s", S8_Expand(exe_name));
            result = Run(cc + build_file + flags);
        }

        if (result != 0) {
            IO_Printf("FAILED compilation of the build file!\n");
            OS_DeleteFile("build_tool.cache");
            return 1;
        }
        time = OS_GetTime() - time;
        IO_Printf("TIME Build file compilation: %f\n", time);
    }

    // Run the build file
    double time = OS_GetTime();
    if (build_file.str) {
        exe_name   = OS_GetAbsolutePath(Perm, exe_name);
        int result = Run(exe_name + cmd);
        if (result != 0) {
            IO_Printf("FAILED execution of the build file, deleting cache!\n");
            OS_DeleteFile("build_tool.cache");
            return 1;
        }
    }

    SRC_SaveCache();
    time = OS_GetTime() - time;
    IO_Printf("TIME total build file execution: %f\n", time);
}
