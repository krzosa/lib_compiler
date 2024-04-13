typedef struct M_Allocator M_Allocator;

typedef enum M_AllocatorOp {
    M_AllocatorOp_Invalid,
    M_AllocatorOp_Allocate,
    M_AllocatorOp_Deallocate,
    M_AllocatorOp_Reallocate,
} M_AllocatorOp;

typedef void *M_AllocatorProc(void *allocator, M_AllocatorOp kind, void *p, size_t size, size_t old_size);
MA_API void  *MA_AllocatorProc(void *allocator, M_AllocatorOp kind, void *p, size_t size, size_t old_size);
MA_API void  *MA_ExclusiveAllocatorProc(void *allocator, M_AllocatorOp kind, void *p, size_t size, size_t old_size);

struct M_Allocator {
    // it's int for type safety because C++ somehow allows this:
    // struct Array { M_Allocator allocator; }
    // Array = {arena}; WTF???!??!?
    // without actually passing M_Allocator but just a pointer
    int             *obj;
    M_AllocatorProc *p;
};

#define M_AllocStruct(a, T) (T *)M_Alloc((a), sizeof(T))
#define M_AllocArray(a, T, c) (T *)M_Alloc((a), sizeof(T) * (c))
#define M_AllocStructNonZeroed(a, T) (T *)M_AllocNonZeroed((a), sizeof(T))
#define M_AllocArrayNonZeroed(a, T, c) (T *)M_AllocNonZeroed((a), sizeof(T) * (c))
#define M_AllocStructCopy(a, T, p) (T *)M_PushCopy(a, (p), sizeof(T))

#define M_Alloc(a, size) M__Alloc(a, size)
#define M_AllocNonZeroed(a, size) M__AllocNonZeroed(a, size)
#define M_AllocCopy(a, p, size) M__AllocCopy(a, p, size)
#define M_Realloc(a, p, size, old_size) M__Realloc(a, p, size, old_size)
#define M_Dealloc(a, p) M__Dealloc(a, p)

MA_API void *M__AllocNonZeroed(M_Allocator allocator, size_t size);
MA_API void *M__Alloc(M_Allocator allocator, size_t size);
MA_API void *M__AllocCopy(M_Allocator allocator, void *p, size_t size);
MA_API void *M__Realloc(M_Allocator allocator, void *p, size_t size, size_t old_size);
MA_API void  M__Dealloc(M_Allocator allocator, void *p);

MA_API M_Allocator M_GetSystemAllocator(void);
MA_API M_Allocator MA_GetExclusiveAllocator(MA_Arena *arena);
MA_API M_Allocator MA_GetAllocator(MA_Arena *arena);
MA_API M_Allocator MA_BootstrapExclusive(void);
