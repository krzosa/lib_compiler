#include "core.h"
#include "../standalone_libraries/stb_sprintf.c"
#define IO_VSNPRINTF stbsp_vsnprintf
#define IO_SNPRINTF stbsp_snprintf
#include "../standalone_libraries/io.c"
#define MA_Assertf(x, ...) IO_Assertf(x, __VA_ARGS__)
#include "../standalone_libraries/arena.c"
#define RE_ASSERT(x) IO_Assert(x)
#include "../standalone_libraries/regex.c"
#include "../standalone_libraries/unicode.c"
#define S8_VSNPRINTF stbsp_vsnprintf
#define S8_ALLOCATE(allocator, size) MA_PushSize(allocator, size)
#define S8_ASSERT(x) IO_Assert(x)
#define S8_MemoryCopy MA_MemoryCopy
#include "../standalone_libraries/string.c"
#define MU_ASSERT IO_Assert
#include "../standalone_libraries/multimedia.h"
#include "../standalone_libraries/hash.c"
#include "../standalone_libraries/load_library.c"
#include "filesystem.c"

#include "cmd.c"
#include "allocator.c"