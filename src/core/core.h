#ifndef FIRST_CORE_HEADER
#define FIRST_CORE_HEADER

#include "../standalone_libraries/preproc_env.h"
#include "../standalone_libraries/stb_sprintf.h"
#include "../standalone_libraries/io.h"
#include "../standalone_libraries/arena.h"
#include "../standalone_libraries/unicode.h"
#include "../standalone_libraries/string.h"
#include "../standalone_libraries/hash.h"
#include "../standalone_libraries/linked_list.h"
#include "../standalone_libraries/regex.h"
#include "../standalone_libraries/multimedia.h"
#include "../standalone_libraries/load_library.h"
#include "filesystem.h"
#include "cmd.h"
#include "allocator.h"

#if LANG_CPP
    #include "../standalone_libraries/defer.hpp"
    #include "table.hpp"
    #include "array.hpp"
#endif

#endif