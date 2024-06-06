#define WASM_IMPORT(name) __attribute__((import_name(#name))) name
#define WASM_EXPORT(name) __attribute__((export_name(#name))) name

double WASM_IMPORT(JS_ParseFloat)(char *str, int len);
void   WASM_IMPORT(JS_ConsoleLog)(char *str, int len);
char  *WASM_IMPORT(JS_LoadFile)(void);

#define LC_ParseFloat(str, len) JS_ParseFloat(str, len)
#define LC_Print(str, len) JS_ConsoleLog(str, len)
#define LC_Exit(x) (*(volatile int *)0 = 0)
#define LC_THREAD_LOCAL
#define LC_MemoryZero(p, size) __builtin_memset(p, 0, size);
#define LC_MemoryCopy(dst, src, size) __builtin_memcpy(dst, src, size)

#include "../standalone_libraries/stb_sprintf.h"
#include "../standalone_libraries/stb_sprintf.c"
#define LC_vsnprintf stbsp_vsnprintf
#include "../compiler/lib_compiler.c"

bool LC_IsDir(LC_Arena *temp, LC_String path) {
    if (LC_AreEqual(path, LC_Lit("virtual_dir"), false)) return true;
    if (LC_AreEqual(path, LC_Lit("virtual_dir/libc"), false)) return true;
    return false;
}

LC_FileIter LC_IterateFiles(LC_Arena *arena, LC_String path) {
    LC_FileIter result  = {0};
    result.is_valid     = true;
    result.is_directory = false;
    result.filename     = LC_Lit("file.lc");

    if (LC_StartsWith(path, LC_Lit("virtual_dir"), false)) {
        result.absolute_path = LC_Format(arena, "%.*s/file.lc", LC_Expand(path));
        result.relative_path = LC_Format(arena, "%.*s/file.lc", LC_Expand(path));
    }
    return result;
}

bool LC_IsValid(LC_FileIter it) {
    return it.is_valid;
}

void LC_Advance(LC_FileIter *it) {
    it->is_valid = false;
}

LC_String LC_ReadFile(LC_Arena *arena, LC_String path) {
    LC_String result = LC_Lit("main :: proc(): int { return 0; }");
    return result;
}

LC_FUNCTION void LC_VDeallocate(LC_VMemory *m) {}
LC_FUNCTION bool LC_VCommit(LC_VMemory *m, size_t commit) { return false; }

char     Memory[64 * 1024 * 24];
LC_Arena MainArena;

LC_Lang *Wasm_LC_LangAlloc(LC_Arena *arena) {
    LC_Arena *lex_arena = LC_PushStruct(arena, LC_Arena);
    *lex_arena          = LC_PushArena(arena, 64 * 1024 * 4);

    LC_Arena *ast_arena = LC_PushStruct(arena, LC_Arena);
    *ast_arena          = LC_PushArena(arena, 64 * 1024 * 4);

    LC_Arena *decl_arena = LC_PushStruct(arena, LC_Arena);
    *decl_arena          = LC_PushArena(arena, 64 * 1024 * 2);

    LC_Arena *type_arena = LC_PushStruct(arena, LC_Arena);
    *type_arena          = LC_PushArena(arena, 64 * 1024 * 2);

    LC_Lang *l    = LC_PushStruct(arena, LC_Lang);
    l->arena      = arena;
    l->lex_arena  = lex_arena;
    l->decl_arena = decl_arena;
    l->type_arena = type_arena;
    l->ast_arena  = ast_arena;

    return l;
}

void Wasm_OnFileLoad(LC_AST *package, LoadedFile *file) {
    if (LC_StartsWith(file->path, LC_Lit("virtual_dir/libc/file.lc"), false)) {
        file->content = LC_Lit(
            " size_t   :: typedef ullong; @foreign"
            " printf   :: proc(format: *char, ...): int; @foreign"
            " memset   :: proc(s:  *void, value: int, n: size_t): *void; @foreign"
            " memcpy   :: proc(s1: *void, s2: *void, n: size_t): *void; @foreign");
    } else {
        file->content.str = JS_LoadFile();
        file->content.len = LC_StrLen(file->content.str);
    }
}

void WASM_EXPORT(test)(void) {
    LC_MemoryZero(&MainArena, sizeof(MainArena));
    LC_InitArenaFromBuffer(&MainArena, Memory, sizeof(Memory));
    LC_Lang *lang = Wasm_LC_LangAlloc(&MainArena);
    {
        lang->on_file_load = Wasm_OnFileLoad;
        lang->os           = 32;
        lang->arch         = 32;
    }
    LC_LangBegin(lang);
    {
        LC_AddBuiltinConstInt("LC_WASM", 32);
        LC_AddBuiltinConstInt("ARCH_WASM", 32);
        L->tlong->size   = 4;
        L->tlong->align  = 4;
        L->tulong->size  = 4;
        L->tulong->align = 4;
        LC_SetPointerSizeAndAlign(4, 4);
    }

    LC_RegisterPackageDir("virtual_dir");

    LC_Intern name = LC_ILit("file");
    LC_AddSingleFilePackage(name, LC_Lit("file.lc"));
    LC_ParseAndResolve(name);

    if (L->errors == 0) {
        LC_BeginStringGen(L->arena);
        for (LC_ASTRef *it = L->ordered_packages.first; it; it = it->next) LC_GenCHeader(it->ast);
        for (LC_ASTRef *it = L->ordered_packages.last; it; it = it->next) LC_GenCImpl(it->ast);
        LC_String result = LC_EndStringGen(L->arena);

        LC_String code_output = LC_Lit("//\n// Code output\n//");
        JS_ConsoleLog(code_output.str, code_output.len);
        JS_ConsoleLog(result.str, (int)result.len);
    }

    LC_LangEnd(lang);
}
