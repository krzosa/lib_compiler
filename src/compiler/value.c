LC_Operand LC_OPNull;

LC_FUNCTION LC_Operand LC_OPError(void) {
    LC_Operand result = {LC_OPF_Error};
    return result;
}

LC_FUNCTION LC_Operand LC_OPConstType(LC_Type *type) {
    LC_Operand result = {LC_OPF_UTConst | LC_OPF_Const};
    result.type       = type;
    return result;
}

LC_FUNCTION LC_Operand LC_OPDecl(LC_Decl *decl) {
    LC_Operand result = {0};
    result.decl       = decl;
    return result;
}

LC_FUNCTION LC_Operand LC_OPType(LC_Type *type) {
    LC_Operand result = {0};
    result.type       = type;
    return result;
}

LC_FUNCTION LC_Operand LC_OPLValueAndType(LC_Type *type) {
    LC_Operand result = LC_OPType(type);
    result.flags      = LC_OPF_LValue;
    return result;
}

LC_FUNCTION LC_Operand LC_ReportASTError(LC_AST *n, const char *str, ...) {
    LC_FORMAT(L->arena, str, s8);
    LC_SendErrorMessage(n ? n->pos : NULL, s8);
    L->errors += 1;
    return LC_OPError();
}

LC_FUNCTION LC_Operand LC_ReportASTErrorEx(LC_AST *n1, LC_AST *n2, const char *str, ...) {
    LC_FORMAT(L->arena, str, s8);
    LC_SendErrorMessage(n1->pos, s8);
    LC_SendErrorMessage(n2->pos, s8);
    L->errors += 1;
    return LC_OPError();
}

LC_FUNCTION LC_Operand LC_ConstCastFloat(LC_AST *pos, LC_Operand op) {
    LC_ASSERT(pos, LC_IsUTConst(op));
    LC_ASSERT(pos, LC_IsUntyped(op.type));
    if (LC_IsUTInt(op.type)) op.val.d = LC_Bigint_as_float(&op.val.i);
    if (LC_IsUTStr(op.type)) return LC_ReportASTError(pos, "Trying to convert '%s' to float", LC_GenLCType(op.type));
    op.type = L->tuntypedfloat;
    return op;
}

LC_FUNCTION LC_Operand LC_ConstCastInt(LC_AST *pos, LC_Operand op) {
    LC_ASSERT(pos, LC_IsUTConst(op));
    LC_ASSERT(pos, LC_IsUntyped(op.type));
    if (LC_IsUTFloat(op.type)) {
        double v = op.val.d; // add rounding?
        LC_Bigint_init_signed(&op.val.i, (int64_t)v);
    }
    if (LC_IsUTStr(op.type)) return LC_ReportASTError(pos, "Trying to convert %s to int", LC_GenLCType(op.type));
    op.type = L->tuntypedint;
    return op;
}

LC_FUNCTION LC_Operand LC_OPInt(int64_t v) {
    LC_Operand op = {0};
    op.type       = L->tuntypedint;
    op.flags |= LC_OPF_UTConst | LC_OPF_Const;
    LC_Bigint_init_signed(&op.v.i, v);
    return op;
}

LC_FUNCTION LC_Operand LC_OPIntT(LC_Type *type, int64_t v) {
    LC_ASSERT(NULL, LC_IsInt(type));
    LC_Operand op = LC_OPInt(v);
    op.type       = type;
    return op;
}

LC_FUNCTION LC_Operand LC_OPModDefaultUT(LC_Operand val) {
    if (LC_IsUntyped(val.type)) {
        val.type = val.type->tbase;
    }
    return val;
}

LC_FUNCTION LC_Operand LC_OPModType(LC_Operand op, LC_Type *type) {
    if (LC_IsUTConst(op)) {
        if (LC_IsUTInt(op.type) && LC_IsFloat(type)) {
            op = LC_ConstCastFloat(NULL, op);
        }
    }
    op.type = type;
    return op;
}

LC_FUNCTION LC_Operand LC_OPModBool(LC_Operand op) {
    op.type = L->tuntypedbool;
    return op;
}

LC_FUNCTION LC_Operand LC_OPModBoolV(LC_Operand op, int v) {
    op.type = L->tuntypedbool;
    LC_Bigint_init_signed(&op.v.i, v);
    return op;
}

LC_FUNCTION LC_Operand LC_EvalBinary(LC_AST *pos, LC_Operand a, LC_TokenKind op, LC_Operand b) {
    LC_ASSERT(pos, LC_IsUTConst(a));
    LC_ASSERT(pos, LC_IsUTConst(b));
    LC_ASSERT(pos, LC_IsUntyped(a.type));
    LC_ASSERT(pos, LC_IsUntyped(b.type));
    LC_ASSERT(pos, a.type->kind == b.type->kind);

    LC_Operand c = LC_OPConstType(a.type);
    if (LC_IsUTStr(a.type)) {
        return LC_ReportASTError(pos, "invalid operand %s for binary expr of type untyped string", LC_TokenKindToString(op));
    }
    if (LC_IsUTFloat(a.type)) {
        switch (op) {
        case LC_TokenKind_Add: c.v.d = a.v.d + b.v.d; break;
        case LC_TokenKind_Sub: c.v.d = a.v.d - b.v.d; break;
        case LC_TokenKind_Mul: c.v.d = a.v.d * b.v.d; break;
        case LC_TokenKind_Div: {
            if (b.v.d == 0.0) return LC_ReportASTError(pos, "division by 0");
            c.v.d = a.v.d / b.v.d;
        } break;
        case LC_TokenKind_LesserThenEq: c = LC_OPModBoolV(c, a.v.d <= b.v.d); break;
        case LC_TokenKind_GreaterThenEq: c = LC_OPModBoolV(c, a.v.d >= b.v.d); break;
        case LC_TokenKind_GreaterThen: c = LC_OPModBoolV(c, a.v.d > b.v.d); break;
        case LC_TokenKind_LesserThen: c = LC_OPModBoolV(c, a.v.d < b.v.d); break;
        case LC_TokenKind_Equals: c = LC_OPModBoolV(c, a.v.d == b.v.d); break;
        case LC_TokenKind_NotEquals: c = LC_OPModBoolV(c, a.v.d != b.v.d); break;
        default: return LC_ReportASTError(pos, "invalid operand %s for binary expr of type untyped float", LC_TokenKindToString(op));
        }
    }
    if (LC_IsUTInt(a.type)) {
        switch (op) {
        case LC_TokenKind_BitXor: LC_Bigint_xor(&c.v.i, &a.v.i, &b.v.i); break;
        case LC_TokenKind_BitAnd: LC_Bigint_and(&c.v.i, &a.v.i, &b.v.i); break;
        case LC_TokenKind_BitOr: LC_Bigint_or(&c.v.i, &a.v.i, &b.v.i); break;
        case LC_TokenKind_Add: LC_Bigint_add(&c.v.i, &a.v.i, &b.v.i); break;
        case LC_TokenKind_Sub: LC_Bigint_sub(&c.v.i, &a.v.i, &b.v.i); break;
        case LC_TokenKind_Mul: LC_Bigint_mul(&c.v.i, &a.v.i, &b.v.i); break;
        case LC_TokenKind_Div: {
            if (b.v.i.digit_count == 0) return LC_ReportASTError(pos, "division by zero in constant expression");
            LC_Bigint_div_floor(&c.v.i, &a.v.i, &b.v.i);
        } break;
        case LC_TokenKind_Mod: {
            if (b.v.i.digit_count == 0) return LC_ReportASTError(pos, "modulo by zero in constant expression");
            LC_Bigint_mod(&c.v.i, &a.v.i, &b.v.i);
        } break;
        case LC_TokenKind_LeftShift: LC_Bigint_shl(&c.v.i, &a.v.i, &b.v.i); break;
        case LC_TokenKind_RightShift: LC_Bigint_shr(&c.v.i, &a.v.i, &b.v.i); break;
        case LC_TokenKind_And: {
            int left  = LC_Bigint_cmp_zero(&a.v.i) != LC_CmpRes_EQ;
            int right = LC_Bigint_cmp_zero(&b.v.i) != LC_CmpRes_EQ;
            c         = LC_OPModBoolV(c, left && right);
        } break;
        case LC_TokenKind_Or: {
            int left  = LC_Bigint_cmp_zero(&a.v.i) != LC_CmpRes_EQ;
            int right = LC_Bigint_cmp_zero(&b.v.i) != LC_CmpRes_EQ;
            c         = LC_OPModBoolV(c, left || right);
        } break;
        default: {
            LC_CmpRes cmp = LC_Bigint_cmp(&a.v.i, &b.v.i);
            switch (op) {
            case LC_TokenKind_LesserThenEq: c = LC_OPModBoolV(c, (cmp == LC_CmpRes_LT) || (cmp == LC_CmpRes_EQ)); break;
            case LC_TokenKind_GreaterThenEq: c = LC_OPModBoolV(c, (cmp == LC_CmpRes_GT) || (cmp == LC_CmpRes_EQ)); break;
            case LC_TokenKind_GreaterThen: c = LC_OPModBoolV(c, cmp == LC_CmpRes_GT); break;
            case LC_TokenKind_LesserThen: c = LC_OPModBoolV(c, cmp == LC_CmpRes_LT); break;
            case LC_TokenKind_Equals: c = LC_OPModBoolV(c, cmp == LC_CmpRes_EQ); break;
            case LC_TokenKind_NotEquals: c = LC_OPModBoolV(c, cmp != LC_CmpRes_EQ); break;
            default: return LC_ReportASTError(pos, "invalid operand %s for binary expr of type untyped int", LC_TokenKindToString(op));
            }
        }
        }
    }

    return c;
}

LC_FUNCTION LC_Operand LC_EvalUnary(LC_AST *pos, LC_TokenKind op, LC_Operand a) {
    LC_ASSERT(pos, LC_IsUTConst(a));
    LC_ASSERT(pos, LC_IsUntyped(a.type));
    LC_Operand c = a;

    if (LC_IsUTStr(a.type)) {
        return LC_ReportASTError(pos, "invalid operand %s for unary expr of type untyped string", LC_TokenKindToString(op));
    }
    if (LC_IsUTFloat(a.type)) {
        switch (op) {
        case LC_TokenKind_Sub: c.v.d = -a.v.d; break;
        case LC_TokenKind_Add: c.v.d = +a.v.d; break;
        default: return LC_ReportASTError(pos, "invalid operand %s for unary expr of type untyped float", LC_TokenKindToString(op));
        }
    }
    if (LC_IsUTInt(a.type)) {
        switch (op) {
        case LC_TokenKind_Not: c = LC_OPModBoolV(c, LC_Bigint_cmp_zero(&a.v.i) == LC_CmpRes_EQ); break;
        case LC_TokenKind_Sub: LC_Bigint_negate(&c.v.i, &a.v.i); break;
        case LC_TokenKind_Add: c = a; break;
        case LC_TokenKind_Neg: LC_Bigint_not(&c.v.i, &a.v.i, a.type->tbase->size * 8, !a.type->tbase->is_unsigned); break;
        default: return LC_ReportASTError(pos, "invalid operand %s for unary expr of type untyped int", LC_TokenKindToString(op));
        }
    }

    return c;
}

LC_FUNCTION bool LC_BigIntFits(LC_BigInt i, LC_Type *type) {
    LC_ASSERT(NULL, LC_IsInt(type));
    if (!LC_Bigint_fits_in_bits(&i, type->size * 8, !type->is_unsigned)) {
        return false;
    }
    return true;
}

LC_FUNCTION LC_OPResult LC_IsBinaryExprValidForType(LC_TokenKind op, LC_Type *type) {
    if (LC_IsFloat(type)) {
        switch (op) {
        case LC_TokenKind_Add: return LC_OPResult_Ok; break;
        case LC_TokenKind_Sub: return LC_OPResult_Ok; break;
        case LC_TokenKind_Mul: return LC_OPResult_Ok; break;
        case LC_TokenKind_Div: return LC_OPResult_Ok; break;
        case LC_TokenKind_LesserThenEq: return LC_OPResult_Bool; break;
        case LC_TokenKind_GreaterThenEq: return LC_OPResult_Bool; break;
        case LC_TokenKind_GreaterThen: return LC_OPResult_Bool; break;
        case LC_TokenKind_LesserThen: return LC_OPResult_Bool; break;
        case LC_TokenKind_Equals: return LC_OPResult_Bool; break;
        case LC_TokenKind_NotEquals: return LC_OPResult_Bool; break;
        default: return LC_OPResult_Error;
        }
    }
    if (LC_IsInt(type)) {
        switch (op) {
        case LC_TokenKind_BitXor: return LC_OPResult_Ok; break;
        case LC_TokenKind_BitAnd: return LC_OPResult_Ok; break;
        case LC_TokenKind_BitOr: return LC_OPResult_Ok; break;
        case LC_TokenKind_Add: return LC_OPResult_Ok; break;
        case LC_TokenKind_Sub: return LC_OPResult_Ok; break;
        case LC_TokenKind_Mul: return LC_OPResult_Ok; break;
        case LC_TokenKind_Div: return LC_OPResult_Ok; break;
        case LC_TokenKind_Mod: return LC_OPResult_Ok; break;
        case LC_TokenKind_LeftShift: return LC_OPResult_Ok; break;
        case LC_TokenKind_RightShift: return LC_OPResult_Ok; break;
        case LC_TokenKind_And: return LC_OPResult_Bool; break;
        case LC_TokenKind_Or: return LC_OPResult_Bool; break;
        case LC_TokenKind_LesserThenEq: return LC_OPResult_Bool; break;
        case LC_TokenKind_GreaterThenEq: return LC_OPResult_Bool; break;
        case LC_TokenKind_GreaterThen: return LC_OPResult_Bool; break;
        case LC_TokenKind_LesserThen: return LC_OPResult_Bool; break;
        case LC_TokenKind_Equals: return LC_OPResult_Bool; break;
        case LC_TokenKind_NotEquals: return LC_OPResult_Bool; break;
        default: return LC_OPResult_Error;
        }
    }
    if (LC_IsPtrLike(type)) {
        switch (op) {
        case LC_TokenKind_And: return LC_OPResult_Bool; break;
        case LC_TokenKind_Or: return LC_OPResult_Bool; break;
        case LC_TokenKind_LesserThenEq: return LC_OPResult_Bool; break;
        case LC_TokenKind_GreaterThenEq: return LC_OPResult_Bool; break;
        case LC_TokenKind_GreaterThen: return LC_OPResult_Bool; break;
        case LC_TokenKind_LesserThen: return LC_OPResult_Bool; break;
        case LC_TokenKind_Equals: return LC_OPResult_Bool; break;
        case LC_TokenKind_NotEquals: return LC_OPResult_Bool; break;
        default: return LC_OPResult_Error;
        }
    }

    return LC_OPResult_Error;
}

LC_FUNCTION LC_OPResult LC_IsUnaryOpValidForType(LC_TokenKind op, LC_Type *type) {
    if (LC_IsFloat(type)) {
        if (op == LC_TokenKind_Sub) return LC_OPResult_Ok;
        if (op == LC_TokenKind_Add) return LC_OPResult_Ok;
    }
    if (LC_IsInt(type)) {
        if (op == LC_TokenKind_Not) return LC_OPResult_Bool;
        if (op == LC_TokenKind_Sub) return LC_OPResult_Ok;
        if (op == LC_TokenKind_Add) return LC_OPResult_Ok;
        if (op == LC_TokenKind_Neg) return LC_OPResult_Ok;
    }
    if (LC_IsPtrLike(type)) {
        if (op == LC_TokenKind_Not) return LC_OPResult_Bool;
    }
    return LC_OPResult_Error;
}

LC_FUNCTION LC_OPResult LC_IsAssignValidForType(LC_TokenKind op, LC_Type *type) {
    if (op == LC_TokenKind_Assign) return LC_OPResult_Ok;
    if (LC_IsInt(type)) {
        switch (op) {
        case LC_TokenKind_DivAssign: return LC_OPResult_Ok;
        case LC_TokenKind_MulAssign: return LC_OPResult_Ok;
        case LC_TokenKind_ModAssign: return LC_OPResult_Ok;
        case LC_TokenKind_SubAssign: return LC_OPResult_Ok;
        case LC_TokenKind_AddAssign: return LC_OPResult_Ok;
        case LC_TokenKind_BitAndAssign: return LC_OPResult_Ok;
        case LC_TokenKind_BitOrAssign: return LC_OPResult_Ok;
        case LC_TokenKind_BitXorAssign: return LC_OPResult_Ok;
        case LC_TokenKind_LeftShiftAssign: return LC_OPResult_Ok;
        case LC_TokenKind_RightShiftAssign: return LC_OPResult_Ok;
        default: return LC_OPResult_Error;
        }
    }
    if (LC_IsFloat(type)) {
        switch (op) {
        case LC_TokenKind_DivAssign: return LC_OPResult_Ok;
        case LC_TokenKind_MulAssign: return LC_OPResult_Ok;
        case LC_TokenKind_SubAssign: return LC_OPResult_Ok;
        case LC_TokenKind_AddAssign: return LC_OPResult_Ok;
        default: return LC_OPResult_Error;
        }
    }
    return LC_OPResult_Error;
}

LC_FUNCTION int LC_GetLevelsOfIndirection(LC_Type *type) {
    if (type->kind == LC_TypeKind_Pointer) return LC_GetLevelsOfIndirection(type->tbase) + 1;
    if (type->kind == LC_TypeKind_Array) return LC_GetLevelsOfIndirection(type->tbase) + 1;
    return 0;
}
