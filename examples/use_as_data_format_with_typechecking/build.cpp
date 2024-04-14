void OnExprResolved_MakeSureConstantsWork(LC_AST *n, LC_Operand *op);
bool use_as_data_format_with_typechecking() {
    LC_Lang *lang                     = LC_LangAlloc();
    lang->use_colored_terminal_output = UseColoredIO;
    lang->breakpoint_on_error         = BreakpointOnError;
    lang->on_expr_resolved            = OnExprResolved_MakeSureConstantsWork;
    LC_LangBegin(lang);
    LC_RegisterPackageDir("../examples/");
    LC_RegisterPackageDir("../pkgs");

    LC_Intern     name     = LC_ILit("use_as_data_format_with_typechecking");
    LC_ASTRefList packages = LC_ResolvePackageByName(name);
    if (L->errors) {
        LC_LangEnd(lang);
        return false;
    }

    LC_AST *package = LC_GetPackageByName(name);

    // Constants
    {
        LC_Decl *int_value = LC_FindDeclInScope(package->apackage.ext->scope, LC_ILit("IntValue"));
        int64_t  IntValue  = LC_Bigint_as_signed(&int_value->val.i);
        IO_Assert(IntValue == 232);
    }

    {
        LC_Decl *decl = LC_FindDeclInScope(package->apackage.ext->scope, LC_ILit("FloatValue"));
        IO_Assert(decl->val.d == 13.0);
    }

    {
        LC_Decl *decl = LC_FindDeclInScope(package->apackage.ext->scope, LC_ILit("SomeString"));
        IO_Assert(decl->val.name == LC_ILit("Thing"));
    }

    // Player

    int64_t LEVEL_BEGIN = -1;
    {
        LC_Decl *int_value = LC_FindDeclInScope(package->apackage.ext->scope, LC_ILit("LEVEL_BEGIN"));
        LEVEL_BEGIN        = LC_Bigint_as_signed(&int_value->val.i);
        IO_Assert(LEVEL_BEGIN == 2);
    }

    {
        /* @todo:
            double DashSpeed = GetFloatValue(p, "Player.DashSpeed");
            double Hurtbox = GetFloatValue(p, "Player[0].Something.min.x");
            int64_t DashVFloorSnapDist = GetIntValue(p, "Player.DashVFloorSnapDist");

            double Variable = GetFloatValue(p, "Variable");
        */

        LC_Decl          *decl  = LC_FindDeclInScope(package->apackage.ext->scope, LC_ILit("Player"));
        LC_ResolvedCompo *items = decl->ast->dvar.expr->ecompo.resolved_items;
        for (LC_ResolvedCompoItem *it = items->first; it; it = it->next) {
            LC_Intern name = it->t->name;

            if (name == LC_ILit("DashSpeed")) {
                double DashSpeed = it->expr->const_val.d;
                IO_Assert(DashSpeed == 240.0);
            } else if (name == LC_ILit("DashCooldown")) {
                double DashCooldown = it->expr->const_val.d;
                IO_Assert(DashCooldown > 0.19 && DashCooldown < 0.21);
            } else if (name == LC_ILit("Level")) {
                int64_t Level = LC_Bigint_as_signed(&it->expr->const_val.i);
                IO_Assert(Level == LEVEL_BEGIN);
            }
        }
    }

    {
        LC_Decl *decl = LC_FindDeclInScope(package->apackage.ext->scope, LC_ILit("Variable"));
        IO_Assert(decl->val.d == 32.0);
    }

    {
        LC_Decl *decl = LC_FindDeclInScope(package->apackage.ext->scope, LC_ILit("Reference"));
        IO_Assert(decl->val.d == 64.0);
    }

    LC_LangEnd(lang);
    return true;
}

void OnExprResolved_MakeSureConstantsWork(LC_AST *n, LC_Operand *op) {
    switch (n->kind) {
    case LC_ASTKind_ExprCast:
    case LC_ASTKind_ExprAddPtr:
    case LC_ASTKind_ExprIndex:
    case LC_ASTKind_ExprGetPointerOfValue:
    case LC_ASTKind_ExprGetValueOfPointer:
    case LC_ASTKind_ExprCall:
        LC_ReportASTError(n, "illegal expression kind");
        return;
    default: {
    }
    }

    if (LC_IsFloat(op->type)) op->type = L->tuntypedfloat;
    if (LC_IsInt(op->type)) op->type = L->tuntypedint;
    if (LC_IsStr(op->type)) op->type = L->tuntypedstring;
    op->flags |= LC_OPF_UTConst;
}