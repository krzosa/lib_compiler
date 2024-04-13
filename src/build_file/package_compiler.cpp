bool Expand(MA_Arena *arena, S8_List *out, S8_String filepath) {
    S8_String c = OS_ReadFile(arena, filepath);
    if (c.str == 0) return false;
    // S8_String name = S8_SkipToLastSlash(filepath);
    // S8_AddF(arena, out, "// %.*s\n", S8_Expand(name));

    S8_String path    = S8_ChopLastSlash(filepath);
    S8_String include = S8_Lit("#include \"");
    for (;;) {
        int64_t idx = -1;
        if (S8_Seek(c, include, 0, &idx)) {
            S8_String str_to_add = S8_GetPrefix(c, idx);
            S8_AddNode(arena, out, str_to_add);
            S8_String save = c;
            c              = S8_Skip(c, idx + include.len);

            S8_String filename = c;
            filename.len       = 0;
            while (filename.str[filename.len] != '"' && filename.len < c.len) {
                filename.len += 1;
            }

            c                  = S8_Skip(c, filename.len + 1);
            S8_String inc_path = S8_Format(arena, "%.*s/%.*s", S8_Expand(path), S8_Expand(filename));

            bool dont_expand_and_remove_include = false;
            if (S8_EndsWith(inc_path, "lib_compiler.h")) dont_expand_and_remove_include = true;

            if (!dont_expand_and_remove_include) {
                if (!Expand(arena, out, inc_path)) {
                    S8_String s = S8_GetPrefix(save, save.len - c.len);
                    S8_AddNode(arena, out, s);
                }
            }
        } else {
            S8_AddNode(arena, out, c);
            break;
        }
    }

    return true;
}

S8_String ExpandIncludes(MA_Arena *arena, S8_String filepath) {
    S8_List   out    = S8_MakeEmptyList();
    S8_String result = S8_MakeEmpty();
    Expand(arena, &out, filepath);
    result = S8_Merge(arena, out);
    return result;
}

void PackageCompiler() {
    MA_Scratch scratch;
    S8_String  header = ExpandIncludes(scratch, S8_Lit("../src/compiler/lib_compiler.h"));
    S8_String  impl   = ExpandIncludes(scratch, S8_Lit("../src/compiler/lib_compiler.c"));

    S8_List list = {0};
    S8_AddF(scratch, &list, R"FOO(/*
This is a compiler frontend in a single-header-file library form.
This is a **beta** so things may change between versions!

# How to use

In *one* of your C or C++ files to create the implementation:

```
#define LIB_COMPILER_IMPLEMENTATION
#include "lib_compiler.h"
```

In the rest of your files you can just include it like a regular
header.

# Examples

See online repository for code examples

# Overrides

You can override libc calls, the arena implementation using
preprocessor at compile time, here is an example of how you
would go about it:

```
#define LC_vsnprintf stbsp_vsnprintf
#define LC_MemoryZero(p, size) __builtin_memset(p, 0, size);
#define LC_MemoryCopy(dst, src, size) __builtin_memcpy(dst, src, size)

#define LIB_COMPILER_IMPLEMENTATION
#include "lib_compiler.h"
```

Look for '@override' to find things that can be overridden using macro preprocessor
Look for '@api' to find the main functions that you are supposed to use
Look for '@configurable' to find runtime callbacks you can register and other settings

# License (MIT)

See end of file

*/
)FOO");
    S8_AddNode(scratch, &list, header);
    S8_AddF(scratch, &list, "\n#ifdef LIB_COMPILER_IMPLEMENTATION\n");
    S8_AddNode(scratch, &list, impl);
    S8_AddF(scratch, &list, "\n#endif // LIB_COMPILER_IMPLEMENTATION\n");
    S8_AddF(scratch, &list, R"FOO(
/*
Copyright (c) 2024 Krzosa Karol

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
)FOO");
    S8_String result = S8_Merge(scratch, list);

    if (PrintAllFunctions) {
        S8_List lines = S8_Split(Perm, result, S8_Lit("\n"), 0);
        S8_For(_line, lines) {
            S8_String line = _line->string;

            if (!S8_Seek(line, "{", S8_FindFlag_MatchFindLast)) {
                continue;
            }

            int64_t idx = 0;
            if (S8_Seek(line, ")", 0, &idx)) {
                line.len = idx + 1;
            }

            S8_String proc_name = line;
            if (S8_Seek(line, "(", 0, &idx)) {
                proc_name.len = idx;
                for (int64_t i = proc_name.len - 1; i > 0; i -= 1) {
                    bool is_ident = CHAR_IsIdent(proc_name.str[i]) || CHAR_IsDigit(proc_name.str[i]);
                    if (!is_ident) {
                        proc_name = S8_Skip(proc_name, i + 1);
                        break;
                    }
                }
            }

            if (S8_StartsWith(line, "LC_FUNCTION", false)) {
                if (PrintAllFunctions) IO_Printf("%.*s;\n", S8_Expand(line));
            }
        }
    }

    OS_WriteFile(S8_Lit("../lib_compiler.h"), result);
}
