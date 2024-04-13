/*
I assume this is going to be a command line program, so:
- don't care about allocation
- global variables are OK
*/

#include "../core/core.c"
#include "../compiler/lib_compiler.c"

#include "x64.h"
#include "misc.cpp"
#include "registers.cpp"
#include "value.cpp"

Value GenExprInt(LC_AST *n) {
    uint64_t u      = LC_Bigint_as_unsigned(&n->eatom.i);
    Value    result = {Mode_Imm, INVALID_REGISTER, u};
    return result;
}

Value GenExprIdent(LC_AST *n) {
    LC_Decl *decl = n->eident.resolved_decl;
    X64Decl *x    = decl->x64;

    Register reg = AllocReg();

    if (decl->ast->kind == LC_ASTKind_StmtVar) {
        LC_GenLinef("lea %s, [RBP - %d]", Str(reg), x->offset);
    } else IO_Todo();

    Value result = {Mode_Direct, reg};
    return result;
}

Value GenExprField(LC_AST *n) {
    LC_Decl       *resolved_decl = n->efield.resolved_decl;
    LC_TypeMember *mem           = resolved_decl->type_member;

    Value value = GenExpr(n->efield.left);
    IO_Assert(value.mode == Mode_Direct);

    LC_GenLinef("add %s, %d", Str(value.reg), mem->offset);

    return value;
}

void LoadValueIntoReg(Value *v) {
    if (v->mode == Mode_Direct) {
        LC_GenLinef("mov %s, %s", Str(v->reg), Str(*v));
        v->mode = Mode_Reg;
    } else if (v->mode == Mode_Imm) {
        v->reg = AllocReg();
        LC_GenLinef("mov %s, %llu", Str(v->reg), v->u);
        v->mode = Mode_Reg;
    }
}

Value GenExprBinary(LC_AST *n) {
    Value l = GenExpr(n->ebinary.left);
    Value r = GenExpr(n->ebinary.right);
    LoadValueIntoReg(&l);
    LoadValueIntoReg(&r);

    switch (n->ebinary.op) {
    case LC_TokenKind_Add: LC_GenLinef("add %s, %s", Str(l), Str(r)); break;
    case LC_TokenKind_Sub: LC_GenLinef("sub %s, %s", Str(l), Str(r)); break;
    default: LC_ReportASTError(n, "internal compiler error: unhandled binary op %s in %s", TokenKindString[n->ebinary.op], __FUNCTION__);
    }

    DeallocReg(r.reg);
    return l;
}

Value GenExprIndex(LC_AST *n) {
    Value index = GenExpr(n->eindex.index);
    Value base  = GenExpr(n->eindex.base);

    IO_Assert(index.mode != Mode_Direct);
    IO_Assert(base.mode == Mode_Direct);

    int esize = n->type->size;
    if (index.mode == Mode_Imm) {
        index.u *= esize;
    } else if (index.mode == Mode_Reg) {
        LC_GenLinef("imul %s, %d", Str(index), esize);
    } else IO_Todo();
    LC_GenLinef("add %s, %s", Str(base.reg), Str(index));

    DeallocReg(index);
    return base;
}

Value GenExprGetPointerOfValue(LC_AST *n) {
    Value value = GenExpr(n->eunary.expr);
    IO_Assert(value.mode == Mode_Direct);
    value.mode = Mode_Reg;
    return value;
}

Value GenExprGetValueOfPointer(LC_AST *n) {
    Value value = GenExpr(n->eunary.expr);
    if (value.mode == Mode_Reg) {
        value.mode = Mode_Direct;
    } else if (value.mode == Mode_Direct) {
        LC_GenLinef("mov %s, %s", Str(value.reg), Str(value));
    } else IO_Todo();
    return value;
}

Value GenExprCall(LC_AST *n) {
    Register abi_regs[] = {RCX, RDX, R8, R9};
    int      i          = 0;

    LC_ResolvedCompo *com = n->ecompo.resolved_items;
    for (LC_ResolvedCompoItem *it = com->first; it; it = it->next) {
        IO_Assert(i < 4);

        LC_Type *type  = it->t->type;
        Value    value = GenExpr(it->expr);
        Register reg   = abi_regs[i++];
        LC_GenLinef("mov %s, %s", Str(reg), Str(value.reg));

        DeallocReg(value);
    }

    LC_GenLinef("call");
    Value result = {0};
    if (n->type->tproc.ret != L->tvoid) {
        result.reg  = AllocReg();
        result.mode = Mode_Reg;
    }
    return result;
}

Value GenExpr(LC_AST *n) {
    IO_Assert(LC_IsExpr(n));
    switch (n->kind) {
    case LC_ASTKind_ExprIndex: return GenExprIndex(n); break;
    case LC_ASTKind_ExprInt: return GenExprInt(n); break;
    case LC_ASTKind_ExprIdent: return GenExprIdent(n); break;
    case LC_ASTKind_ExprField: return GenExprField(n); break;
    case LC_ASTKind_ExprBinary: return GenExprBinary(n); break;
    case LC_ASTKind_ExprGetPointerOfValue: return GenExprGetPointerOfValue(n); break;
    case LC_ASTKind_ExprGetValueOfPointer: return GenExprGetValueOfPointer(n); break;
    case LC_ASTKind_ExprCall: return GenExprCall(n); break;
    }

    LC_ReportASTError(n, "internal compiler error: unhandled ast kind %s in %s", ASTKindString[n->kind], __FUNCTION__);
    return {};
}

void GenEpilogue() {
    LC_GenLinef("mov rsp, rbp");
    LC_GenLinef("pop rbp");
}

void GenStmtReturn(LC_AST *n) {
    if (n->sreturn.expr) {
        Value value = GenExpr(n->sreturn.expr);
        LC_GenLinef("mov RAX, %s", Str(value));
        DeallocReg(value);
    }

    GenEpilogue();
    LC_GenLinef("ret");
}

void GenStmtVar(LC_AST *n) {
    LC_Decl *decl = n->svar.resolved_decl;
    X64Decl *x    = decl->x64;
    LC_AST  *expr = n->svar.expr;

    if (expr) {
        Value value = GenExpr(expr);
        LC_GenLinef("lea RAX, [RBP - %d]", x->offset);
        if (value.mode == Mode_Imm) {
            LC_GenLinef("mov qword [RAX], %u", value.u);
        } else if (value.mode == Mode_Direct) {
            LC_GenLinef("mov RBX, [%s]", Str(value.reg));
            LC_GenLinef("mov qword [RAX], RBX");
        } else if (value.mode == Mode_Reg) {
            LC_GenLinef("mov qword [RAX], %s", Str(value.reg));
        } else IO_Todo();

        DeallocReg(value);
    } else {
        LC_GenLinef("lea RAX, [RBP - %d]", x->offset);
        for (int i = 0; i < n->type->size; i += 1) {
            LC_GenLinef("mov byte [RAX+%d], 0", i);
        }
    }
}

void GenStmtAssign(LC_AST *n) {
    Value l = GenExpr(n->sassign.left);
    IO_Assert(l.mode == Mode_Direct);
    Value r = GenExpr(n->sassign.right);

    LC_TokenKind op = n->sassign.op;
    switch (op) {
    case LC_TokenKind_Assign: LC_GenLinef("mov qword %s, %s", Str(l), Str(r)); break;
    case LC_TokenKind_AddAssign: LC_GenLinef("add qword %s, %s", Str(l), Str(r)); break;
    case LC_TokenKind_SubAssign: LC_GenLinef("sub qword %s, %s", Str(l), Str(r)); break;
    default: LC_ReportASTError(n, "internal compiler error: unhandled assign op %s in %s", TokenKindString[n->ebinary.op], __FUNCTION__);
    }

    DeallocReg(l);
    DeallocReg(r);
}

void GenStmt(LC_AST *n) {
    OutASTLine(n);
    IO_Assert(LC_IsStmt(n));

    switch (n->kind) {
    case LC_ASTKind_StmtExpr:
        Value val = GenExpr(n->sexpr.expr);
        DeallocReg(val);
        break;
    case LC_ASTKind_StmtVar: GenStmtVar(n); break;
    case LC_ASTKind_StmtReturn: GenStmtReturn(n); break;
    case LC_ASTKind_StmtAssign: GenStmtAssign(n); break;
    default: LC_ReportASTError(n, "internal compiler error: unhandled ast kind %s in %s", ASTKindString[n->kind], __FUNCTION__);
    }
}

void GenStmtBlock(LC_AST *n) {
    LC_ASTFor(it, n->sblock.first) {
        GenStmt(it);
    }
}

void WalkToComputeStackSize(LC_ASTWalker *w, LC_AST *n) {
    int *stack_size = (int *)w->user_data;
    if (n->kind == LC_ASTKind_StmtVar) {
        LC_Decl *decl = n->svar.resolved_decl;
        decl->x64     = MA_PushStruct(L->arena, X64Decl);

        stack_size[0] += n->type->size;
        decl->x64->offset = stack_size[0]; // stack goes backwards
        stack_size[0]     = (int)MA_AlignUp(stack_size[0], 8);
    }
}

void GenProc(LC_Decl *decl) {
    LC_AST *n = decl->ast;
    OutASTLine(n);

    LC_GenLinef("global %s", decl->foreign_name);
    LC_GenLinef("%s:", decl->foreign_name);

    int stack_size = 0;
    AST_WalkBreathFirst(MA_GetAllocator(L->arena), WalkToComputeStackSize, n->dproc.body, (void *)&stack_size);

    LC_GenLinef("push rbp");
    LC_GenLinef("mov  rbp, rsp");

    if ((stack_size % 16) == 0) stack_size += 8;
    LC_GenLinef("sub  rsp, %d", stack_size);

    printer.indent += 1;
    for (LC_TypeMember *it = n->type->tproc.args.first; it; it = it->next) {
    }

    GenStmtBlock(n->dproc.body);
    printer.indent -= 1;

    GenEpilogue();
}

void GenPackages(LC_ASTRefList packages) {
    LC_GenLinef("BITS 64");

    LC_GenLinef("section .text");
    for (LC_ASTRef *it = packages.first; it; it = it->next) {
        LC_AST *n = it->ast;

        LC_DeclFor(decl, n->apackage.first_ordered) {
            if (decl->kind != LC_DeclKind_Proc) continue;
            if (decl->is_foreign) continue;
            GenProc(decl);
        }
    }
}

int main(int argc, char **argv) {
    MA_Arena arena = {0};
    for (OS_FileIter iter = OS_IterateFiles(&arena, S8_Lit("../../src/x64/tests")); OS_IsValid(iter); OS_Advance(&iter)) {
        if (!S8_EndsWith(iter.filename, S8_Lit("first.lc"))) continue;
        LC_Lang *lang                     = LC_LangAlloc();
        lang->use_colored_terminal_output = UseColoredIO;
        LC_LangBegin(lang);
        LC_ASTRefList package = ParseAndResolveFile(iter.absolute_path.str);
        LC_BeginStringGen(L->arena);
        GenPackages(package);
        S8_String s    = LC_EndStringGen(L->arena);
        S8_String asmf = S8_Format(L->arena, "%.*s.asm", S8_Expand(iter.filename));

        OS_WriteFile(asmf, s);
        OS_SystemF("..\\..\\tools\\nasm.exe %.*s -o %.*s.obj -gcv8 -f  win64 -O0", S8_Expand(asmf), S8_Expand(asmf));
        OS_SystemF("\"C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.36.32532/bin/Hostx64/x64/link.exe\" %.*s.obj /nologo /subsystem:console /debug /entry:main /out:%.*s.exe", S8_Expand(asmf), S8_Expand(asmf));
        int result = OS_SystemF("%.*s.exe", S8_Expand(asmf));
        IO_Assert(result == 0);
        LC_LangEnd();
    }

    IO_DebugBreak();
    return 0;
}
