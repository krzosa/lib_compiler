#if __cplusplus
    #define LC_Alignof(...) alignof(__VA_ARGS__)
#else
    #define LC_Alignof(...) _Alignof(__VA_ARGS__)
#endif

#define LC_WRAP_AROUND_POWER_OF_2(x, pow2) (((x) & ((pow2)-1llu)))

#if defined(_MSC_VER)
    #define LC_DebugBreak() (L->breakpoint_on_error && IsDebuggerPresent() && (__debugbreak(), 0))
#else
    #define LC_DebugBreak() (L->breakpoint_on_error && (__builtin_trap(), 0))
#endif
#define LC_FatalError() (L->breakpoint_on_error ? LC_DebugBreak() : (LC_Exit(1), 0))

LC_FUNCTION void LC_IgnoreMessage(LC_Token *pos, char *str, int len) {
}

LC_FUNCTION void LC_SendErrorMessage(LC_Token *pos, LC_String s8) {
    if (L->on_message) {
        L->on_message(pos, s8.str, (int)s8.len);
    } else {
        if (pos) {
            LC_String line = LC_GetTokenLine(pos);
            LC_String fmt  = LC_Format(L->arena, "%s(%d,%d): error: %.*s\n%.*s", (char *)pos->lex->file, pos->line, pos->column, LC_Expand(s8), LC_Expand(line));
            LC_Print(fmt.str, fmt.len);
        } else {
            LC_Print(s8.str, s8.len);
        }
    }
    LC_DebugBreak();
}

LC_FUNCTION void LC_SendErrorMessagef(LC_Lex *x, LC_Token *pos, const char *str, ...) {
    LC_FORMAT(L->arena, str, s8);
    LC_SendErrorMessage(pos, s8);
}

LC_FUNCTION void LC_HandleFatalError(void) {
    if (L->on_fatal_error) {
        L->on_fatal_error();
        return;
    }
    LC_FatalError();
}

LC_FUNCTION void LC_MapReserve(LC_Map *map, int size) {
    LC_Map old_map = *map;

    LC_ASSERT(NULL, LC_IS_POW2(size));
    map->len = 0;
    map->cap = size;
    LC_ASSERT(NULL, map->arena);

    map->entries = LC_PushArray(map->arena, LC_MapEntry, map->cap);

    if (old_map.entries) {
        for (int i = 0; i < old_map.cap; i += 1) {
            LC_MapEntry *it = old_map.entries + i;
            if (it->key) LC_InsertMapEntry(map, it->key, it->value);
        }
    }
}

// FNV HASH (1a?)
LC_FUNCTION uint64_t LC_HashBytes(void *data, uint64_t size) {
    uint8_t *data8 = (uint8_t *)data;
    uint64_t hash  = (uint64_t)14695981039346656037ULL;
    for (uint64_t i = 0; i < size; i++) {
        hash = hash ^ (uint64_t)(data8[i]);
        hash = hash * (uint64_t)1099511628211ULL;
    }
    return hash;
}

LC_FUNCTION uint64_t LC_HashMix(uint64_t x, uint64_t y) {
    x ^= y;
    x *= 0xff51afd7ed558ccd;
    x ^= x >> 32;
    return x;
}

LC_FUNCTION int LC_NextPow2(int v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

LC_FUNCTION LC_MapEntry *LC_GetMapEntryEx(LC_Map *map, uint64_t key) {
    LC_ASSERT(NULL, key);
    if (map->len * 2 >= map->cap) {
        LC_MapReserve(map, map->cap * 2);
    }

    uint64_t hash = LC_HashBytes(&key, sizeof(key));
    if (hash == 0) hash += 1;
    uint64_t index = LC_WRAP_AROUND_POWER_OF_2(hash, map->cap);
    uint64_t i     = index;
    for (;;) {
        LC_MapEntry *it = map->entries + i;
        if (it->key == key || it->key == 0) {
            return it;
        }

        i = LC_WRAP_AROUND_POWER_OF_2(i + 1, map->cap);
        if (i == index) return NULL;
    }
    LC_ASSERT(NULL, !"invalid codepath");
}

LC_FUNCTION bool LC_InsertWithoutReplace(LC_Map *map, void *key, void *value) {
    LC_MapEntry *entry = LC_GetMapEntryEx(map, (uint64_t)key);
    if (entry->key != 0) return false;

    map->len += 1;
    entry->key   = (uint64_t)key;
    entry->value = (uint64_t)value;
    return true;
}

LC_FUNCTION LC_MapEntry *LC_InsertMapEntry(LC_Map *map, uint64_t key, uint64_t value) {
    LC_MapEntry *entry = LC_GetMapEntryEx(map, key);
    if (entry->key == key) {
        entry->value = value;
    }
    if (entry->key == 0) {
        entry->key   = key;
        entry->value = value;
        map->len += 1;
    }
    return entry;
}

LC_FUNCTION LC_MapEntry *LC_GetMapEntry(LC_Map *map, uint64_t key) {
    LC_MapEntry *entry = LC_GetMapEntryEx(map, key);
    if (entry && entry->key == key) {
        return entry;
    }
    return NULL;
}

LC_FUNCTION void LC_MapInsert(LC_Map *map, LC_String keystr, void *value) {
    uint64_t key = LC_HashBytes(keystr.str, keystr.len);
    LC_InsertMapEntry(map, key, (uint64_t)value);
}

LC_FUNCTION void *LC_MapGet(LC_Map *map, LC_String keystr) {
    uint64_t     key = LC_HashBytes(keystr.str, keystr.len);
    LC_MapEntry *r   = LC_GetMapEntry(map, key);
    return r ? (void *)r->value : 0;
}

LC_FUNCTION void LC_MapInsertU64(LC_Map *map, uint64_t key, void *value) {
    LC_InsertMapEntry(map, key, (uint64_t)value);
}

LC_FUNCTION void *LC_MapGetU64(LC_Map *map, uint64_t key) {
    LC_MapEntry *r = LC_GetMapEntry(map, key);
    return r ? (void *)r->value : 0;
}

LC_FUNCTION void *LC_MapGetP(LC_Map *map, void *key) {
    return LC_MapGetU64(map, (uint64_t)key);
}

LC_FUNCTION void LC_MapInsertP(LC_Map *map, void *key, void *value) {
    LC_InsertMapEntry(map, (uint64_t)key, (uint64_t)value);
}

LC_FUNCTION void LC_MapClear(LC_Map *map) {
    if (map->len != 0) LC_MemoryZero(map->entries, map->cap * sizeof(LC_MapEntry));
    map->len = 0;
}

LC_FUNCTION size_t LC_GetAlignOffset(size_t size, size_t align) {
    size_t mask = align - 1;
    size_t val  = size & mask;
    if (val) {
        val = align - val;
    }
    return val;
}

LC_FUNCTION size_t LC_AlignUp(size_t size, size_t align) {
    size_t result = size + LC_GetAlignOffset(size, align);
    return result;
}

LC_FUNCTION size_t LC_AlignDown(size_t size, size_t align) {
    size += 1; // Make sure when align is 8 doesn't get rounded down to 0
    size_t result = size - (align - LC_GetAlignOffset(size, align));
    return result;
}
