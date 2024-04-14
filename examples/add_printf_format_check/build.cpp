void OnExprResolved_CheckPrintf(LC_AST *n, LC_Operand *op);

bool add_printf_format_check() {
    bool     result                   = false;
    LC_Lang *lang                     = LC_LangAlloc();
    lang->use_colored_terminal_output = UseColoredIO;
    lang->breakpoint_on_error         = BreakpointOnError;
    lang->user_data                   = (void *)&result;
    lang->on_expr_resolved            = OnExprResolved_CheckPrintf;
    lang->on_message                  = LC_IgnoreMessage;
    LC_LangBegin(lang);
    LC_RegisterPackageDir("../pkgs");
    LC_RegisterPackageDir("../examples");

    LC_Intern name = LC_ILit("add_printf_format_check");
    LC_ParseAndResolve(name);

    LC_LangEnd(lang);
    return result;
}

void OnExprResolved_CheckPrintf(LC_AST *n, LC_Operand *op) {
    if (n->kind == LC_ASTKind_ExprCall) {
        LC_AST *name = n->ecompo.name;
        if (name->kind == LC_ASTKind_ExprIdent) {
            LC_Decl *decl = name->eident.resolved_decl;
            if (decl->name == LC_ILit("printf")) {
                LC_ResolvedCompo *items  = n->ecompo.resolved_items;
                LC_AST           *string = items->first->expr;
                IO_Assert(string->kind == LC_ASTKind_ExprString);

                char                 *c    = (char *)string->eatom.name;
                LC_ResolvedCompoItem *item = items->first->next;
                for (int i = 0; c[i]; i += 1) {
                    if (c[i] == '%') {
                        if (item == NULL) {
                            LC_ReportASTError(n, "too few arguments passed");

                            // Test reports success if error thrown correctly
                            *(bool *)L->user_data = true;
                            return;
                        }
                        if (c[i + 1] == 'd') {
                            if (item->expr->type != L->tint) LC_ReportASTError(item->expr, "expected int type");
                            item = item->next;
                        } else if (c[i + 1] == 's') {
                            if (item->expr->type != L->tpchar) LC_ReportASTError(item->expr, "expected *char type");
                            item = item->next;
                        }
                    }
                }
                if (item != NULL) LC_ReportASTError(item->expr, "too many arguments passed");
            }
        }
    }
}
