typedef struct {
    int  len;
    char str[];
} INTERN_Entry;

LC_FUNCTION LC_Intern LC_InternStrLen(char *str, int len) {
    LC_String     key   = LC_MakeString(str, len);
    INTERN_Entry *entry = (INTERN_Entry *)LC_MapGet(&L->interns, key);
    if (entry == NULL) {
        LC_ASSERT(NULL, sizeof(INTERN_Entry) == sizeof(int));
        entry      = (INTERN_Entry *)LC_PushSize(L->arena, sizeof(int) + sizeof(char) * (len + 1));
        entry->len = len;
        LC_MemoryCopy(entry->str, str, len);
        entry->str[len] = 0;
        LC_MapInsert(&L->interns, key, entry);
    }

    return (uintptr_t)entry->str;
}

LC_FUNCTION LC_Intern LC_ILit(char *str) {
    return LC_InternStrLen(str, (int)LC_StrLen(str));
}

LC_FUNCTION LC_Intern LC_GetUniqueIntern(const char *name_for_debug) {
    LC_String name   = LC_Format(L->arena, "U%u_%s", ++L->unique_counter, name_for_debug);
    LC_Intern result = LC_InternStrLen(name.str, (int)name.len);
    return result;
}

LC_FUNCTION char *LC_GetUniqueName(const char *name_for_debug) {
    LC_String name = LC_Format(L->arena, "U%u_%s", ++L->unique_counter, name_for_debug);
    return name.str;
}

LC_FUNCTION void LC_DeclareNote(LC_Intern intern) {
    LC_MapInsertU64(&L->declared_notes, intern, (void *)intern);
}

LC_FUNCTION bool LC_IsNoteDeclared(LC_Intern intern) {
    void *p      = LC_MapGetU64(&L->declared_notes, intern);
    bool  result = p != NULL;
    return result;
}