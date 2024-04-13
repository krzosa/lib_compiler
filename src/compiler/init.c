LC_FUNCTION LC_Lang *LC_LangAlloc(void) {
    LC_Arena *arena      = LC_BootstrapArena();
    LC_Arena *lex_arena  = LC_PushStruct(arena, LC_Arena);
    LC_Arena *decl_arena = LC_PushStruct(arena, LC_Arena);
    LC_Arena *ast_arena  = LC_PushStruct(arena, LC_Arena);
    LC_Arena *type_arena = LC_PushStruct(arena, LC_Arena);
    LC_InitArena(lex_arena);
    LC_InitArena(decl_arena);
    LC_InitArena(ast_arena);
    LC_InitArena(type_arena);

    LC_Lang *l    = LC_PushStruct(arena, LC_Lang);
    l->arena      = arena;
    l->lex_arena  = lex_arena;
    l->decl_arena = decl_arena;
    l->ast_arena  = ast_arena;
    l->type_arena = type_arena;

    l->emit_line_directives        = true;
    l->breakpoint_on_error         = true;
    l->use_colored_terminal_output = true;

    return l;
}

LC_FUNCTION void LC_LangEnd(LC_Lang *lang) {
    LC_DeallocateArena(lang->lex_arena);
    LC_DeallocateArena(lang->type_arena);
    LC_DeallocateArena(lang->decl_arena);
    LC_DeallocateArena(lang->ast_arena);
    LC_DeallocateArena(lang->arena);
    if (L == lang) L = NULL;
}

LC_FUNCTION void LC_LangBegin(LC_Lang *l) {
    L = l;

    // Init default target settings
    {
        if (L->os == LC_OS_Invalid) {
            L->os = LC_OS_LINUX;
#if LC_OPERATING_SYSTEM_WINDOWS
            L->os = LC_OS_WINDOWS;
#elif LC_OPERATING_SYSTEM_MAC
            L->os = LC_OS_MAC;
#endif
        }
        if (L->arch == LC_ARCH_Invalid) {
            L->arch = LC_ARCH_X64;
        }
        if (L->gen == LC_GEN_Invalid) {
            L->gen = LC_GEN_C;
        }
    }

    //
    // Init declared notes, interns and foreign names checker
    //
    {
        L->declared_notes.arena = L->arena;
        L->interns.arena        = L->arena;
        L->foreign_names.arena  = L->arena;
        L->implicit_any.arena   = L->arena;

        LC_MapReserve(&L->declared_notes, 128);
        LC_MapReserve(&L->interns, 4096);
        LC_MapReserve(&L->foreign_names, 256);
        LC_MapReserve(&L->implicit_any, 64);

#define X(x) l->k##x = LC_InternStrLen(#x, sizeof(#x) - 1);
        LC_LIST_KEYWORDS
#undef X
        l->first_keyword = l->kfor;
        l->last_keyword  = l->kfalse;

#define X(x, declare) l->i##x = LC_InternStrLen(#x, sizeof(#x) - 1);
        LC_LIST_INTERNS
#undef X
#define X(x, declare) \
    if (declare) LC_DeclareNote(L->i##x);
        LC_LIST_INTERNS
#undef X
    }

    // Nulls
    {
        L->NullLEX.begin    = "builtin declarations";
        L->NullLEX.file     = LC_ILit("builtin declarations");
        L->BuiltinToken.lex = &L->NullLEX;
        L->BuiltinToken.str = "builtin declarations";
        L->BuiltinToken.len = sizeof("builtin declarations") - 1;
        L->NullAST.pos      = &L->BuiltinToken;
    }

    {
        LC_AST *builtins         = LC_CreateAST(0, LC_ASTKind_Package);
        L->builtin_package       = builtins;
        builtins->apackage.name  = LC_ILit("builtins");
        builtins->apackage.scope = LC_CreateScope(256);
        LC_AddPackageToList(builtins);
    }

    LC_InitDeclStack(&L->resolver.locals, 128);
    L->resolver.duplicate_map.arena = L->arena;
    LC_MapReserve(&L->resolver.duplicate_map, 32);

    L->resolver.stmt_block_stack.arena = L->arena;
    L->printer.out_block_stack.arena   = L->arena;

    LC_PUSH_PACKAGE(L->builtin_package);

    //
    // Init default type sizes using current platform
    //
    // Here we use the sizes of our current platform but
    // later on it gets swapped based on LC override global variables in
    // InitTarget
    //
    {
        l->type_map.arena = L->arena;
        LC_MapReserve(&l->type_map, 256);

        typedef long long          llong;
        typedef unsigned long long ullong;
        typedef unsigned long      ulong;
        typedef unsigned short     ushort;
        typedef unsigned char      uchar;
        typedef unsigned int       uint;

        L->pointer_align = LC_Alignof(void *);
        L->pointer_size  = sizeof(void *);

        int i = 0;
#define X(TNAME, IS_UNSIGNED)                      \
    l->types[i].kind        = LC_TypeKind_##TNAME; \
    l->types[i].size        = sizeof(TNAME);       \
    l->types[i].align       = LC_Alignof(TNAME);   \
    l->types[i].is_unsigned = IS_UNSIGNED;         \
    l->t##TNAME             = l->types + i++;

        LC_LIST_TYPES
#undef X

        //
        // Overwrite types with target
        //
        if (L->arch == LC_ARCH_X64) {
            LC_SetPointerSizeAndAlign(8, 8);
            if (L->os == LC_OS_WINDOWS) {
                L->tlong->size   = 4;
                L->tlong->align  = 4;
                L->tulong->size  = 4;
                L->tulong->align = 4;
            } else {
                L->tlong->size   = 8;
                L->tlong->align  = 8;
                L->tulong->size  = 8;
                L->tulong->align = 8;
            }
        } else if (L->arch == LC_ARCH_X86) {
            LC_SetPointerSizeAndAlign(4, 4);
            L->tlong->size   = 4;
            L->tlong->align  = 4;
            L->tulong->size  = 4;
            L->tulong->align = 4;
        }

        l->types[i].kind = LC_TypeKind_void;
        l->tvoid         = l->types + i++;

        // Init decls for types
        for (int i = 0; i < T_Count; i += 1) {
            char     *it     = (char *)LC_TypeKindToString((LC_TypeKind)i) + 12;
            LC_Intern intern = LC_ILit(it);
            LC_Type  *t      = l->types + i;
            t->id            = ++l->typeids;

            LC_Decl *decl = LC_CreateDecl(LC_DeclKind_Type, intern, &L->NullAST);
            decl->state   = LC_DeclState_Resolved;
            decl->type    = t;
            t->decl       = decl;
            LC_AddDeclToScope(L->builtin_package->apackage.scope, decl);

            if (t->kind == LC_TypeKind_uchar) decl->foreign_name = LC_ILit("unsigned char");
            if (t->kind == LC_TypeKind_ushort) decl->foreign_name = LC_ILit("unsigned short");
            if (t->kind == LC_TypeKind_uint) decl->foreign_name = LC_ILit("unsigned");
            if (t->kind == LC_TypeKind_ulong) decl->foreign_name = LC_ILit("unsigned long");
            if (t->kind == LC_TypeKind_llong) decl->foreign_name = LC_ILit("long long");
            if (t->kind == LC_TypeKind_ullong) decl->foreign_name = LC_ILit("unsigned long long");
        }
    }

    l->tpvoid = LC_CreatePointerType(l->tvoid);
    l->tpchar = LC_CreatePointerType(l->tchar);

    {
        l->tuntypedint  = LC_CreateUntypedInt(L->tint);
        l->tuntypedbool = LC_CreateUntypedInt(L->tbool);
        l->tuntypednil  = LC_CreateUntypedInt(L->tullong);

        l->ttuntypedfloat.kind       = LC_TypeKind_UntypedFloat;
        l->ttuntypedfloat.id         = ++L->typeids;
        l->tuntypedfloat             = &L->ttuntypedfloat;
        l->tuntypedfloat->tutdefault = l->tdouble;
        l->tuntypedfloat->decl       = LC_CreateDecl(LC_DeclKind_Type, LC_ILit("UntypedFloat"), &L->NullAST);

        l->ttuntypedstring.kind       = LC_TypeKind_UntypedString;
        l->ttuntypedstring.id         = ++L->typeids;
        l->tuntypedstring             = &L->ttuntypedstring;
        l->tuntypedstring->tutdefault = l->tpchar;
        l->tuntypedstring->decl       = LC_CreateDecl(LC_DeclKind_Type, LC_ILit("UntypedString"), &L->NullAST);
    }

    // Add builtin "String" type
    {
        L->ttstring.kind = LC_TypeKind_Incomplete;
        L->ttstring.id   = ++L->typeids;
        L->tstring       = &L->ttstring;

        LC_AST  *ast       = LC_ParseDeclf("String :: struct { str: *char; len: int; }");
        LC_Decl *decl      = LC_CreateDecl(LC_DeclKind_Type, ast->dbase.name, ast);
        decl->foreign_name = LC_ILit("LC_String");
        decl->state        = LC_DeclState_Resolved;
        decl->type         = L->tstring;
        L->tstring->decl   = decl;
        LC_AddDeclToScope(L->builtin_package->apackage.scope, decl);
        LC_Operand result = LC_ResolveTypeAggregate(ast, decl->type);
        LC_ASSERT(ast, !LC_IsError(result));
    }

    // Add builtin "Any" type
    {
        L->ttany.kind = LC_TypeKind_Incomplete;
        L->ttany.id   = ++L->typeids;
        L->tany       = &L->ttany;

        LC_AST  *ast       = LC_ParseDeclf("Any :: struct { type: int; data: *void; }");
        LC_Decl *decl      = LC_CreateDecl(LC_DeclKind_Type, ast->dbase.name, ast);
        decl->foreign_name = LC_ILit("LC_Any");
        decl->state        = LC_DeclState_Resolved;
        decl->type         = L->tany;
        L->tany->decl      = decl;
        LC_AddDeclToScope(L->builtin_package->apackage.scope, decl);
        LC_Operand result = LC_ResolveTypeAggregate(ast, decl->type);
        LC_ASSERT(ast, !LC_IsError(result));
    }

    LC_Decl *decl_nil = LC_AddConstIntDecl("nil", 0);
    decl_nil->type    = L->tuntypednil;

    for (int i = LC_ARCH_X64; i < LC_ARCH_Count; i += 1) LC_AddBuiltinConstInt((char *)LC_ARCHToString((LC_ARCH)i), i);
    for (int i = LC_OS_WINDOWS; i < LC_OS_Count; i += 1) LC_AddBuiltinConstInt((char *)LC_OSToString((LC_OS)i), i);
    for (int i = LC_GEN_C; i < LC_GEN_Count; i += 1) LC_AddBuiltinConstInt((char *)LC_GENToString((LC_GEN)i), i);
    LC_AddBuiltinConstInt("LC_ARCH", L->arch);
    LC_AddBuiltinConstInt("LC_GEN", L->gen);
    LC_AddBuiltinConstInt("LC_OS", L->os);

    LC_POP_PACKAGE();
}
