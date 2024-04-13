#define SRC_CACHE_ENTRY_COUNT 1024

struct SRC_CacheEntry {
    uint64_t filepath_hash;
    uint64_t file_hash;
    uint64_t includes_hash;
    uint64_t total_hash;
};

struct SRC_Cache {
    int            entry_cap;
    int            entry_len;
    SRC_CacheEntry entries[SRC_CACHE_ENTRY_COUNT];
};

double         SRC_Time;
SRC_Cache     *SRC_InMemoryCache;
SRC_Cache     *SRC_FromFileCache;
S8_String      SRC_CacheFilename;
CL_SearchPaths SRC_SearchPaths = {}; // @todo;

void SRC_InitCache(MA_Arena *arena, S8_String cachefilename) {
    SRC_CacheFilename = cachefilename;

    SRC_InMemoryCache            = MA_PushStruct(arena, SRC_Cache);
    SRC_InMemoryCache->entry_cap = SRC_CACHE_ENTRY_COUNT;

    SRC_FromFileCache            = MA_PushStruct(arena, SRC_Cache);
    SRC_FromFileCache->entry_cap = SRC_CACHE_ENTRY_COUNT;

    S8_String cache = OS_ReadFile(arena, SRC_CacheFilename);
    if (cache.len) MA_MemoryCopy(SRC_FromFileCache, cache.str, cache.len);
}

void SRC_SaveCache() {
    S8_String dump = S8_Make((char *)SRC_InMemoryCache, sizeof(SRC_Cache));
    OS_WriteFile(SRC_CacheFilename, dump);
}

SRC_CacheEntry *SRC_AddHash(uint64_t filepath, uint64_t file, uint64_t includes) {
    IO_Assert(SRC_InMemoryCache->entry_len + 1 < SRC_InMemoryCache->entry_cap);
    SRC_CacheEntry *result = SRC_InMemoryCache->entries + SRC_InMemoryCache->entry_len++;
    result->filepath_hash  = filepath;
    result->file_hash      = file;
    result->includes_hash  = includes;
    result->total_hash     = HashBytes(result, sizeof(uint64_t) * 3);
    return result;
}

SRC_CacheEntry *SRC_FindCache(SRC_Cache *cache, uint64_t filepath_hash) {
    for (int cache_i = 0; cache_i < cache->entry_len; cache_i += 1) {
        SRC_CacheEntry *it = cache->entries + cache_i;
        if (it->filepath_hash == filepath_hash) {
            return it;
        }
    }
    return 0;
}

SRC_CacheEntry *SRC_HashFile(S8_String file, char *parent_file) {
    char *resolved_file = CL_ResolveFilepath(Perm, &SRC_SearchPaths, file.str, parent_file, false);
    if (!resolved_file) {
        IO_Printf("Failed to resolve file: %.*s\n", S8_Expand(file));
        return 0;
    }

    uint64_t        filepath_hash = HashBytes(resolved_file, S8_Length(resolved_file));
    SRC_CacheEntry *entry         = SRC_FindCache(SRC_InMemoryCache, filepath_hash);
    if (entry) return entry;

    S8_String filecontent = OS_ReadFile(Perm, S8_MakeFromChar(resolved_file));
    IO_Assert(filecontent.str);

    uint64_t file_hash     = HashBytes(filecontent.str, filecontent.len);
    uint64_t includes_hash = 13;

    CL_Lexer lexer        = CL_Begin(Perm, filecontent.str, resolved_file);
    lexer.select_includes = true;

    for (CL_Token token = CL_Next(&lexer); token.kind != CL_EOF; token = CL_Next(&lexer)) {
        if (token.is_system_include) continue;

        S8_String       file_it = S8_MakeFromChar(token.string_literal);
        SRC_CacheEntry *cache   = SRC_HashFile(file_it, resolved_file);
        if (!cache) {
            // error was reported already IO_Printf("Missing cache for: %.*s\n", S8_Expand(file_it));
            continue;
        }

        includes_hash = HashMix(includes_hash, cache->total_hash);
    }

    SRC_CacheEntry *result = SRC_AddHash(filepath_hash, file_hash, includes_hash);
    return result;
}

bool SRC_WasModified(S8_String file, S8_String artifact_path) {
    double time_start = OS_GetTime();

    if (OS_FileExists(file) == false) {
        IO_Printf("FAILED File doesnt exist: %.*s\n", S8_Expand(file));
        exit(0);
    }
    if (OS_IsAbsolute(file) == false) {
        file = OS_GetAbsolutePath(Perm, file);
    }

    S8_String without_ext = S8_ChopLastPeriod(file);
    S8_String name_only   = S8_SkipToLastSlash(without_ext);

    if (artifact_path.len == 0) artifact_path = S8_Format(Perm, "%.*s.%s", S8_Expand(name_only), IF_WINDOWS_ELSE("obj", "o"));
    bool modified = OS_FileExists(artifact_path) == false;

    SRC_CacheEntry *in_memory = SRC_HashFile(file, 0);
    IO_Assert(in_memory);

    if (modified == false) {
        SRC_CacheEntry *from_file = SRC_FindCache(SRC_FromFileCache, in_memory->filepath_hash);
        if (from_file == 0 || (in_memory->total_hash != from_file->total_hash)) {
            modified = true;
        }
    }

    SRC_Time = SRC_Time + (OS_GetTime() - time_start);

    return modified;
}