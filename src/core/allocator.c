#ifndef MA_CMalloc
    #include <stdlib.h>
    #define MA_CMalloc(x) malloc(x)
    #define MA_CFree(x) free(x)
    #define MA_CRealloc(p, size) realloc(p, size)
#endif

MA_API M_Allocator MA_BootstrapExclusive(void) {
    MA_Arena  bootstrap_arena = {0};
    MA_Arena *arena           = MA_PushStruct(&bootstrap_arena, MA_Arena);
    *arena                    = bootstrap_arena;
    arena->base_len           = arena->len;
    return MA_GetExclusiveAllocator(arena);
}

MA_API void *M__AllocNonZeroed(M_Allocator allocator, size_t size) {
    void *p = allocator.p(allocator.obj, M_AllocatorOp_Allocate, NULL, size, 0);
    return p;
}

MA_API void *M__Alloc(M_Allocator allocator, size_t size) {
    void *p = allocator.p(allocator.obj, M_AllocatorOp_Allocate, NULL, size, 0);
    MA_MemoryZero(p, size);
    return p;
}

MA_API void *M__AllocCopy(M_Allocator allocator, void *p, size_t size) {
    void *copy_buffer = M__AllocNonZeroed(allocator, size);
    MA_MemoryCopy(copy_buffer, p, size);
    return copy_buffer;
}

MA_API void M__Dealloc(M_Allocator allocator, void *p) {
    allocator.p(allocator.obj, M_AllocatorOp_Deallocate, p, 0, 0);
}

MA_API void *M__Realloc(M_Allocator allocator, void *p, size_t size, size_t old_size) {
    void *result = allocator.p(allocator.obj, M_AllocatorOp_Reallocate, p, size, old_size);
    // @todo: add old_size? because we can't zero
    return result;
}

MA_StaticFunc void *M_ClibAllocatorProc(void *allocator, M_AllocatorOp kind, void *p, size_t size, size_t old_size) {
    if (kind == M_AllocatorOp_Allocate) {
        return MA_CMalloc(size);
    }

    if (kind == M_AllocatorOp_Deallocate) {
        MA_CFree(p);
        return NULL;
    }

    if (kind == M_AllocatorOp_Reallocate) {
        return MA_CRealloc(p, size);
    }

    MA_Assertf(0, "MA_Arena invalid codepath");
    return NULL;
}

MA_API void *MA_AllocatorProc(void *allocator, M_AllocatorOp kind, void *p, size_t size, size_t old_size) {
    if (kind == M_AllocatorOp_Allocate) {
        return MA__PushSizeNonZeroed((MA_Arena *)allocator, size);
    }

    else if (kind == M_AllocatorOp_Reallocate) {
        void *new_p = MA__PushSizeNonZeroed((MA_Arena *)allocator, size);
        MA_MemoryCopy(new_p, p, old_size);
        return new_p;
    }

    else if (kind == M_AllocatorOp_Deallocate) {
        return NULL;
    }

    MA_Assertf(0, "MA_Arena invalid codepath");
    return NULL;
}

MA_API void *MA_ExclusiveAllocatorProc(void *allocator, M_AllocatorOp kind, void *p, size_t size, size_t old_size) {
    MA_Arena *arena = (MA_Arena *)allocator;
    if (kind == M_AllocatorOp_Reallocate) {
        if (size > old_size) {
            size_t size_to_push = size - old_size;
            void  *result       = MA__PushSizeNonZeroed(arena, size_to_push);
            if (!p) p = result;
            return p;
        }
    }

    if (kind == M_AllocatorOp_Deallocate) {
        MA_DeallocateArena(arena);
        return NULL;
    }

    MA_Assertf(0, "MA_Arena invalid codepath");
    return NULL;
}

MA_API M_Allocator MA_GetExclusiveAllocator(MA_Arena *arena) {
    M_Allocator allocator = {(int *)arena, MA_ExclusiveAllocatorProc};
    return allocator;
}

MA_API M_Allocator MA_GetAllocator(MA_Arena *arena) {
    M_Allocator allocator = {(int *)arena, MA_AllocatorProc};
    return allocator;
}

MA_API M_Allocator M_GetSystemAllocator(void) {
    M_Allocator allocator;
    allocator.obj = 0;
    allocator.p   = M_ClibAllocatorProc;
    return allocator;
}
