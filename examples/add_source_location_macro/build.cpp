/* This function is called after compiler determined procedure type but
** before the arguments got verified. This opens up a window for call manipulation.
**
** I basically swoop in and add the {file, line} information to the call LC_AST before
** the arguments get resolved.
*/
void ModifyCallsDuringResolution(LC_AST *n, LC_Type *type) {
    LC_TypeMember *tm = type->tproc.args.last;
    if (tm && tm->type->kind == LC_TypeKind_Struct && tm->type->decl->name == LC_ILit("SourceLoc")) {
        LC_AST *call_item           = LC_CreateAST(n->pos, LC_ASTKind_ExprCallItem);
        call_item->ecompo_item.name = LC_ILit("source_loc");
        call_item->ecompo_item.expr = LC_ParseExprf("{\"%s\", %d}", n->pos->lex->file, n->pos->line);
        LC_SetASTPosOnAll(call_item->ecompo_item.expr, n->pos);

        LC_DLLAdd(n->ecompo.first, n->ecompo.last, call_item);
        n->ecompo.size += 1;
    }
}

bool add_source_location_macro() {
    LC_Lang *lang                     = LC_LangAlloc();
    lang->use_colored_terminal_output = UseColoredIO;
    lang->breakpoint_on_error         = BreakpointOnError;
    lang->before_call_args_resolved   = ModifyCallsDuringResolution;
    LC_LangBegin(lang);
    LC_RegisterPackageDir("../examples/");
    LC_RegisterPackageDir("../pkgs");

    LC_Intern name = LC_ILit("add_source_location_macro");
    LC_ParseAndResolve(name);
    if (L->errors) {
        LC_LangEnd(lang);
        return false;
    }

    LC_String code = LC_GenerateUnityBuild();
    LC_LangEnd(lang);

    OS_MakeDir("examples/add_source_location_macro");
    OS_WriteFile("examples/add_source_location_macro/add_source_location_macro.c", code);

    return true;
}