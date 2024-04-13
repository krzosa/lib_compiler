#if defined(LC_USE_ADDRESS_SANITIZER)
    #include <sanitizer/asan_interface.h>
#endif

#if !defined(ASAN_POISON_MEMORY_REGION)
    #define LC_ASAN_POISON_MEMORY_REGION(addr, size) ((void)(addr), (void)(size))
    #define LC_ASAN_UNPOISON_MEMORY_REGION(addr, size) ((void)(addr), (void)(size))
#else
    #define LC_ASAN_POISON_MEMORY_REGION(addr, size) ASAN_POISON_MEMORY_REGION(addr, size)
    #define LC_ASAN_UNPOISON_MEMORY_REGION(addr, size) ASAN_UNPOISON_MEMORY_REGION(addr, size)
#endif

#define LC_DEFAULT_RESERVE_SIZE LC_GIB(1)
#define LC_DEFAULT_ALIGNMENT 8
#define LC_COMMIT_ADD_SIZE LC_MIB(4)

LC_FUNCTION uint8_t *LC_V_AdvanceCommit(LC_VMemory *m, size_t *commit_size, size_t page_size) {
    size_t aligned_up_commit                            = LC_AlignUp(*commit_size, page_size);
    size_t to_be_total_commited_size                    = aligned_up_commit + m->commit;
    size_t to_be_total_commited_size_clamped_to_reserve = LC_CLAMP_TOP(to_be_total_commited_size, m->reserve);
    size_t adjusted_to_boundary_commit                  = to_be_total_commited_size_clamped_to_reserve - m->commit;
    LC_Assertf(adjusted_to_boundary_commit, "Reached the virtual memory reserved boundary");
    *commit_size = adjusted_to_boundary_commit;

    if (adjusted_to_boundary_commit == 0) {
        return 0;
    }
    uint8_t *result = m->data + m->commit;
    return result;
}

LC_FUNCTION void LC_PopToPos(LC_Arena *arena, size_t pos) {
    LC_Assertf(arena->len >= arena->base_len, "Bug: arena->len shouldn't ever be smaller then arena->base_len");
    pos         = LC_CLAMP(pos, arena->base_len, arena->len);
    size_t size = arena->len - pos;
    arena->len  = pos;
    LC_ASAN_POISON_MEMORY_REGION(arena->memory.data + arena->len, size);
}

LC_FUNCTION void LC_PopSize(LC_Arena *arena, size_t size) {
    LC_PopToPos(arena, arena->len - size);
}

LC_FUNCTION void LC_DeallocateArena(LC_Arena *arena) {
    LC_VDeallocate(&arena->memory);
}

LC_FUNCTION void LC_ResetArena(LC_Arena *arena) {
    LC_PopToPos(arena, 0);
}

LC_FUNCTION size_t LC__AlignLen(LC_Arena *a) {
    size_t align_offset = a->alignment ? LC_GetAlignOffset((uintptr_t)a->memory.data + (uintptr_t)a->len, a->alignment) : 0;
    size_t aligned      = a->len + align_offset;
    return aligned;
}

LC_FUNCTION void *LC__PushSizeNonZeroed(LC_Arena *a, size_t size) {
    size_t align_offset        = a->alignment ? LC_GetAlignOffset((uintptr_t)a->memory.data + (uintptr_t)a->len, a->alignment) : 0;
    size_t aligned_len         = a->len + align_offset;
    size_t size_with_alignment = size + align_offset;

    if (a->len + size_with_alignment > a->memory.commit) {
        if (a->memory.reserve == 0) {
            LC_Assertf(0, "Pushing on uninitialized arena with zero initialization turned off");
        }
        bool result = LC_VCommit(&a->memory, size_with_alignment + LC_COMMIT_ADD_SIZE);
        LC_Assertf(result, "%s(%d): Failed to commit memory more memory! reserve: %zu commit: %zu len: %zu size_with_alignment: %zu", LC_BeginTempdSourceLoc.file, LC_BeginTempdSourceLoc.line, a->memory.reserve, a->memory.commit, a->len, size_with_alignment);
        (void)result;
    }

    uint8_t *result = a->memory.data + aligned_len;
    a->len += size_with_alignment;
    LC_Assertf(a->len <= a->memory.commit, "%s(%d): Reached commit boundary! reserve: %zu commit: %zu len: %zu base_len: %zu alignment: %d size_with_alignment: %zu", LC_BeginTempdSourceLoc.file, LC_BeginTempdSourceLoc.line, a->memory.reserve, a->memory.commit, a->len, a->base_len, a->alignment, size_with_alignment);
    LC_ASAN_UNPOISON_MEMORY_REGION(result, size);
    return (void *)result;
}

LC_FUNCTION void *LC__PushSize(LC_Arena *arena, size_t size) {
    void *result = LC__PushSizeNonZeroed(arena, size);
    LC_MemoryZero(result, size);
    return result;
}

LC_FUNCTION LC_Arena LC_PushArena(LC_Arena *arena, size_t size) {
    LC_Arena result;
    LC_MemoryZero(&result, sizeof(result));
    result.memory.data    = LC_PushArrayNonZeroed(arena, uint8_t, size);
    result.memory.commit  = size;
    result.memory.reserve = size;
    result.alignment      = arena->alignment;
    return result;
}

LC_FUNCTION LC_Arena *LC_PushArenaP(LC_Arena *arena, size_t size) {
    LC_Arena *result = LC_PushStruct(arena, LC_Arena);
    *result          = LC_PushArena(arena, size);
    return result;
}

LC_FUNCTION void LC_InitArenaEx(LC_Arena *a, size_t reserve) {
    a->memory = LC_VReserve(reserve);
    LC_ASAN_POISON_MEMORY_REGION(a->memory.data, a->memory.reserve);
    a->alignment = LC_DEFAULT_ALIGNMENT;
}

LC_FUNCTION void LC_InitArena(LC_Arena *a) {
    LC_InitArenaEx(a, LC_DEFAULT_RESERVE_SIZE);
}

LC_FUNCTION LC_Arena *LC_BootstrapArena(void) {
    LC_Arena bootstrap_arena = {0};
    LC_InitArena(&bootstrap_arena);

    LC_Arena *arena = LC_PushStruct(&bootstrap_arena, LC_Arena);
    *arena          = bootstrap_arena;
    arena->base_len = arena->len;
    return arena;
}

LC_FUNCTION void LC_InitArenaFromBuffer(LC_Arena *arena, void *buffer, size_t size) {
    arena->memory.data    = (uint8_t *)buffer;
    arena->memory.commit  = size;
    arena->memory.reserve = size;
    arena->alignment      = LC_DEFAULT_ALIGNMENT;
    LC_ASAN_POISON_MEMORY_REGION(arena->memory.data, arena->memory.reserve);
}

LC_FUNCTION LC_TempArena LC_BeginTemp(LC_Arena *arena) {
    LC_TempArena result;
    result.pos   = arena->len;
    result.arena = arena;
    return result;
}

LC_FUNCTION void LC_EndTemp(LC_TempArena checkpoint) {
    LC_PopToPos(checkpoint.arena, checkpoint.pos);
}