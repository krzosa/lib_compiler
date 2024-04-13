
CmdParser MakeCmdParser(MA_Arena *arena, int argc, char **argv, const char *custom_help) {
    CmdParser result = {argc, argv, arena, custom_help};
    return result;
}

void AddBool(CmdParser *p, bool *result, const char *name, const char *help) {
    CmdDecl *decl     = MA_PushStruct(p->arena, CmdDecl);
    decl->kind        = CmdDeclKind_Bool;
    decl->name        = S8_MakeFromChar((char *)name);
    decl->help        = S8_MakeFromChar((char *)help);
    decl->bool_result = result;
    SLL_QUEUE_ADD(p->fdecl, p->ldecl, decl);
}

void AddInt(CmdParser *p, int *result, const char *name, const char *help) {
    CmdDecl *decl    = MA_PushStruct(p->arena, CmdDecl);
    decl->kind       = CmdDeclKind_Int;
    decl->name       = S8_MakeFromChar((char *)name);
    decl->help       = S8_MakeFromChar((char *)help);
    decl->int_result = result;
    SLL_QUEUE_ADD(p->fdecl, p->ldecl, decl);
}

void AddList(CmdParser *p, S8_List *result, const char *name, const char *help) {
    CmdDecl *decl     = MA_PushStruct(p->arena, CmdDecl);
    decl->kind        = CmdDeclKind_List;
    decl->name        = S8_MakeFromChar((char *)name);
    decl->help        = S8_MakeFromChar((char *)help);
    decl->list_result = result;
    SLL_QUEUE_ADD(p->fdecl, p->ldecl, decl);
}

void AddEnum(CmdParser *p, int *result, const char *name, const char *help, const char **enum_options, int enum_option_count) {
    CmdDecl *decl           = MA_PushStruct(p->arena, CmdDecl);
    decl->kind              = CmdDeclKind_Enum;
    decl->name              = S8_MakeFromChar((char *)name);
    decl->help              = S8_MakeFromChar((char *)help);
    decl->enum_result       = result;
    decl->enum_options      = enum_options;
    decl->enum_option_count = enum_option_count;
    SLL_QUEUE_ADD(p->fdecl, p->ldecl, decl);
}

CmdDecl *FindDecl(CmdParser *p, S8_String name) {
    for (CmdDecl *it = p->fdecl; it; it = it->next) {
        if (S8_AreEqual(it->name, name, true)) {
            return it;
        }
    }
    return NULL;
}

S8_String StrEnumValues(MA_Arena *arena, CmdDecl *decl) {
    S8_List list = {0};
    S8_AddF(arena, &list, "[");
    for (int i = 0; i < decl->enum_option_count; i += 1) {
        S8_AddF(arena, &list, "%s", decl->enum_options[i]);
        if (i != decl->enum_option_count - 1) S8_AddF(arena, &list, "|");
    }
    S8_AddF(arena, &list, "]");
    return S8_Merge(arena, list);
}

void PrintCmdUsage(CmdParser *p) {
    IO_Printf("%s\nCommands:\n", p->custom_help);
    for (CmdDecl *it = p->fdecl; it; it = it->next) {
        IO_Printf("  ");
        if (it->kind == CmdDeclKind_List) {
            S8_String example = S8_Format(p->arena, "-%.*s a b c", S8_Expand(it->name));
            IO_Printf("%-30.*s %.*s\n", S8_Expand(example), S8_Expand(it->help));
        } else if (it->kind == CmdDeclKind_Bool) {
            S8_String example = S8_Format(p->arena, "-%.*s", S8_Expand(it->name));
            IO_Printf("%-30.*s %.*s\n", S8_Expand(example), S8_Expand(it->help));
        } else if (it->kind == CmdDeclKind_Enum) {
            S8_String enum_vals = StrEnumValues(p->arena, it);
            S8_String example   = S8_Format(p->arena, "-%.*s %.*s", S8_Expand(it->name), S8_Expand(enum_vals));
            IO_Printf("%-30.*s %.*s\n", S8_Expand(example), S8_Expand(it->help));
        } else if (it->kind == CmdDeclKind_Int) {
            S8_String example = S8_Format(p->arena, "-%.*s 8", S8_Expand(it->name));
            IO_Printf("%-30.*s %.*s\n", S8_Expand(example), S8_Expand(it->help));
        } else IO_Todo();
    }
}

bool InvalidCmdArg(CmdParser *p, S8_String arg) {
    IO_Printf("invalid command line argument: %.*s\n", S8_Expand(arg));
    return false;
}

bool ParseCmd(MA_Arena *arg_arena, CmdParser *p) {
    for (int i = 1; i < p->argc; i += 1) {
        S8_String arg = S8_MakeFromChar(p->argv[i]);

        if (S8_AreEqual(arg, S8_Lit("--help"), true) || S8_AreEqual(arg, S8_Lit("-h"), true) || S8_AreEqual(arg, S8_Lit("-help"), true)) {
            PrintCmdUsage(p);
            return false;
        }

        if (arg.str[0] == '-') {
            arg = S8_Skip(arg, 1);
            if (arg.str[0] == '-') {
                arg = S8_Skip(arg, 1);
            }

            CmdDecl *decl = FindDecl(p, arg);
            if (!decl) return InvalidCmdArg(p, arg);

            if (decl->kind == CmdDeclKind_Bool) {
                *decl->bool_result = !*decl->bool_result;
            } else if (decl->kind == CmdDeclKind_Int) {
                if (i + 1 >= p->argc) {
                    IO_Printf("expected at least 1 argument after %.*s\n", S8_Expand(arg));
                    return false;
                }
                S8_String num = S8_MakeFromChar(p->argv[++i]);
                for (int i = 0; i < num.len; i += 1) {
                    if (!CHAR_IsDigit(num.str[i])) {
                        IO_Printf("expected argument to be a number, got instead: %.*s", S8_Expand(num));
                        return false;
                    }
                }
                int count           = atoi(num.str);
                decl->int_result[0] = count;

            } else if (decl->kind == CmdDeclKind_Enum) {
                if (i + 1 >= p->argc) {
                    IO_Printf("expected at least 1 argument after %.*s\n", S8_Expand(arg));
                    return false;
                }
                S8_String option_from_cmd = S8_MakeFromChar(p->argv[++i]);

                bool found_option = false;
                for (int i = 0; i < decl->enum_option_count; i += 1) {
                    S8_String option = S8_MakeFromChar((char *)decl->enum_options[i]);
                    if (S8_AreEqual(option, option_from_cmd, true)) {
                        *decl->enum_result = i;
                        found_option       = true;
                        break;
                    }
                }

                if (!found_option) {
                    IO_Printf("expected one of the enum values: %.*s", S8_Expand(StrEnumValues(p->arena, decl)));
                    IO_Printf(" got instead: %.*s\n", S8_Expand(option_from_cmd));
                    return false;
                }

            } else if (decl->kind == CmdDeclKind_List) {
                if (i + 1 >= p->argc) {
                    IO_Printf("expected at least 1 argument after %.*s\n", S8_Expand(arg));
                    return false;
                }

                i += 1;
                for (int counter = 0; i < p->argc; i += 1, counter += 1) {
                    S8_String arg = S8_MakeFromChar(p->argv[i]);
                    if (arg.str[0] == '-') {
                        if (counter == 0) {
                            IO_Printf("expected at least 1 argument after %.*s\n", S8_Expand(arg));
                            return false;
                        }
                        i -= 1;
                        break;
                    }

                    S8_AddNode(arg_arena, decl->list_result, arg);
                }
            } else IO_Todo();

        } else {
            if (p->require_one_standalone_arg && p->args.node_count == 0) {
                S8_AddNode(arg_arena, &p->args, arg);
            } else {
                return InvalidCmdArg(p, arg);
            }
        }
    }

    if (p->require_one_standalone_arg && p->args.node_count == 0) {
        PrintCmdUsage(p);
        return false;
    }

    return true;
}
