struct Process {
    bool is_valid;
    char platform[32];
};

#if OS_WINDOWS
Process RunEx(S8_String in_cmd) {
    MA_Scratch   scratch;
    wchar_t     *application_name = NULL;
    wchar_t     *cmd              = S8_ToWidechar(scratch, in_cmd);
    BOOL         inherit_handles  = FALSE;
    DWORD        creation_flags   = 0;
    void        *enviroment       = NULL;
    wchar_t     *working_dir      = NULL;
    STARTUPINFOW startup_info     = {};
    startup_info.cb               = sizeof(STARTUPINFOW);
    Process result                = {};
    IO_Assert(sizeof(result.platform) >= sizeof(PROCESS_INFORMATION));
    PROCESS_INFORMATION *process_info = (PROCESS_INFORMATION *)result.platform;
    BOOL                 success      = CreateProcessW(application_name, cmd, NULL, NULL, inherit_handles, creation_flags, enviroment, working_dir, &startup_info, process_info);
    result.is_valid                   = true;
    if (!success) {
        result.is_valid = false;

        LPVOID lpMsgBuf;
        DWORD  dw = GetLastError();
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
        LocalFree(lpMsgBuf);

        IO_FatalErrorf("Failed to create process \ncmd: %.*s\nwindows_message: %s", S8_Expand(in_cmd), lpMsgBuf);
    }
    return result;
}

int Wait(Process *process) {
    IO_Assert(process->is_valid);
    PROCESS_INFORMATION *pi = (PROCESS_INFORMATION *)process->platform;
    WaitForSingleObject(pi->hProcess, INFINITE);

    DWORD exit_code;
    BOOL  err = GetExitCodeProcess(pi->hProcess, &exit_code);
    IO_Assert(err != 0);

    CloseHandle(pi->hProcess);
    CloseHandle(pi->hThread);
    process[0] = {};
    return (int)exit_code;
}
#else
    #include <spawn.h>
    #include <sys/wait.h>

struct TH_UnixProcess {
    pid_t pid;
};

extern char **environ;

Process RunEx(S8_String cmd) {
    MA_Scratch scratch;
    Process    result = {};
    IO_Assert(sizeof(result.platform) >= sizeof(TH_UnixProcess));
    TH_UnixProcess *u = (TH_UnixProcess *)result.platform;

    S8_String exec_file = cmd;
    S8_String argv      = "";
    int64_t   pos;
    if (S8_Seek(cmd, S8_Lit(" "), 0, &pos)) {
        exec_file = S8_GetPrefix(cmd, pos);
        argv      = S8_Skip(cmd, pos + 1);
    }

    exec_file = S8_Copy(scratch, exec_file);

    // Split string on whitespace and conform with argv format
    Array<char *> args = {MA_GetAllocator(scratch)};
    {
        args.add(exec_file.str);
        for (int64_t i = 0; i < argv.len;) {
            while (i < argv.len && CHAR_IsWhitespace(argv.str[i])) {
                i += 1;
            }

            S8_String word = {argv.str + i, 0};
            while (i < argv.len && !CHAR_IsWhitespace(argv.str[i])) {
                word.len += 1;
                i += 1;
            }
            word = S8_Copy(scratch, word);
            args.add(word.str);
        }
        args.add(NULL);
    }

    int err = posix_spawnp(&u->pid, exec_file.str, NULL, NULL, args.data, environ);
    if (err == 0) {
        result.is_valid = true;
    } else {
        perror("posix_spawnp error");
        IO_FatalErrorf("Failed to create process, cmd: %.*s", S8_Expand(cmd));
    }

    return result;
}

int Wait(Process *process) {
    if (!process->is_valid) return 1;
    TH_UnixProcess *u = (TH_UnixProcess *)process->platform;

    int status = 0;
    int pid    = waitpid(u->pid, &status, 0);
    IO_Assert(pid != -1);

    int result = 0;
    if (WIFEXITED(status)) {
        result = WEXITSTATUS(status);
    } else {
        result = 1;
    }

    process[0] = {};
    return result;
}
#endif

Process RunEx(Array<S8_String> s) {
    S8_String cmd  = Merge(s);
    Process   proc = RunEx(cmd);
    return proc;
}

Process RunEx(Array<S8_String> s, S8_String process_start_dir) {
    OS_MakeDir(process_start_dir);
    S8_String working_dir = OS_GetWorkingDir(Perm);
    OS_SetWorkingDir(process_start_dir);
    S8_String cmd  = Merge(s);
    Process   proc = RunEx(cmd);
    OS_SetWorkingDir(working_dir);
    return proc;
}

int Run(S8_String cmd) {
    Process process = RunEx(cmd);
    int     result  = Wait(&process);
    return result;
}

int Run(Array<S8_String> cmd) {
    S8_String cmds   = Merge(cmd);
    int       result = Run(cmds);
    return result;
}
