# Compiler front-end in a single-header-file C library

I have no illusions here, this is not the next big language. What I propose is very simple - a return to C. Not to the language as it was, but a return to the ideal of 'C'. A return to something in it - that is more then it. A small language supplemented with modern ideas - distributed as a dependency free, easy to use single-header-file library.

- **User has full control over compilation!**
- **No dependencies, permissive license, single file that compile both in C and C++!**
- **Simpler then C:** core of the language is 4000 loc and entire package 11k loc.
- **Statically typed, procedural and modern:** it's a mix of Go/Odin/Jai with "return to C" as ideal.
- **Complete:** it supports conditional compilation, modularity via packages etc.
- **State of art error handling techniques** like AST poisoning, proper parsing recovery, catches tons of errors without misreporting!
- **Great C integration:** using C libraries feels native, the language compiles easily to C with great debug info.

## Example or [you can try the language online](https://krzosa.xyz/playground.html)

``` odin
import "raylib";

main :: proc(): int {
    InitWindow(800, 600, "Thing");
    SetTargetFPS(60);

    for !WindowShouldClose() {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
```

**Program examples:**

- [Hello world](examples/hello_world)
- [Text editor](examples/text_editor)
- [Path-finding visualizer](examples/pathfinding_visualizer)

**Examples of hooking into the compilation:**

- [Generating type information](examples/generate_type_info)
- [Adding printf checks](examples/add_printf_format_check)
- [Adding instrumentation to every procedure](examples/add_instrumentation)
- [Adding a generic dynamic array using AST modification](examples/add_dynamic_array_macro)

**Other:**

- [Using the language as a data format (with type safety)](examples/use_as_data_format_with_typechecking)

## How to integrate

To create the implementation do this in **one** of your C or C++ files:

``` c
#define LIB_COMPILER_IMPLEMENTATION
#include "lib_compiler.h"
```

## How to build the compiler executable

Simply compile one file with whatever compiler you want:

``` bash
clang tools/lcompile.c -o lc.exe
```

Once you have the executable you can compile examples and stuff:

``` bash
lc.exe examples/create_raylib_window
```

When in doubt: `./lc.exe --help`.

## How to run the test suite

```  bash
clang++ src/build_tool/main.cpp -o bld.exe
./bld.exe
```

You only need to compile the build tool once. Afterwards just call `./bld.exe`. There are multiple testing options so checkout: `./bld.exe --help`.

## Further plans

- **My priority is to improve the C user API, harden the compiler, accommodate things that I didn't foresee and stuff like that.**
- I want to implement a bytecode backend (in the future) so that the language can be used like Lua.
- New features are of second priority unless they important. I'm considering the addition of overloaded procedures because it would greatly aid in writing macros.

# Language overview

The language is strictly procedural, I have taken most of the inspiration from C, Golang, Ion, Jai and Odin. There are no classes, methods etc. only procedures and data types.

## Packages

Package is a real directory on your disk that contains source files. It's also an aggregation of declarations - functions, constants, variables, types which you can `import` into your program.

``` odin
import "libc";
import RL "raylib";

main :: proc(): int {
    RL.SetClipboardText("**magic**");
    printf("**magic**");
    return 0;
}
```

<!-- Packages need to follow a "directed acyclic graph" kind of modularity (you can google the image of how the graph looks). Essentially it means that you cannot perform a 'cyclic' import, 2 packages cannot import each other creating a loop. Such a situation breaks modularity, when this happens you stop having a layer, a library. It becomes a spaghetti ball. -->

<!-- **Important** thing to note here is that packages break the goal of seamless C integration! This outlines exactly how important I think this system is. Due to introduction of packages - all declaration names in the output code need to be mangled. To make it simple for the programmer I introduced a simple naming scheme, all symbols are named like: `lc_{package}_{symbol}`. -->

## Constants

By which I mean the literal values but also the named declarations. There are 3 constant types in the language: untyped ints, untyped floats and untyped strings. The "untyped" here means that the type is inferred from context. There are no suffixes like 'ull', no random overflows and the values are also bounds checked for safety. If you have ever used Go, I have a good news for you, you are already familiar with this concept.

``` odin
binary         :: 0b10000110;
hex1           :: 0xabcd3245;
hex2           :: 0xABCD3245;
decimal        :: 1235204950;
floating_point :: 2342.44230;

boolean        :: false;
nil_val        :: nil;

string_val     :: "Something";
raw_string     :: `
{
    "cool_json": "yes",
    "cool_array": ["a", "b", 32],
}
`;
```

## Types

The type system is strict, most conversions require explicit casts.

``` odin
a: int   = 10;
b: long  = :long(a);  // all of these assignments require a cast
c: float = :float(a);
d: short = :short(a);
e: llong = 1234;      // untyped constants automatically fit into the context
                      // no matter the type
```

There is one exception which is the `*void` type. It has very permissive semantics.

``` odin
i1: int;
v : *void = &i1;
i2: *int  = v;
```

When using regular pointers, again, casts are necessary.

``` odin
i1: *int   = nil;
i2: *float = :*float(i1);
```

Compound types are read from left to right. This creates a nice mirroring between spoken and the symbolic language:

``` odin
a:  *int;         // a:  pointer '*' to an int
b:  [32]*int;     // an: array of 32 pointers to int
c:  *proc(): int; // a:  pointer to a proc type that returns int
```

## Structs and unions

``` odin
Node :: struct {
    v1: Value;
    v2: Value;
}

Value :: union {
    i64: llong;
    u64: ullong;
}
```

## Basic types

The language uses exactly the same types as C for greater compatibility. I don't want to go against the current. I want the language to be as simple as possible.

``` odin
c: char;   uc: uchar;   s: short; us: ushort;
i: int;    ui: uint;    l: long;  ul: ulong;
ll: llong; ull: ullong; f: float; d: double;
b: bool;
```

## Typedefs

Typedef allows you to create an alias to an existsing type but with additional layer of type safety. `my_int` is not an `int`, it's a different type and so it requires a cast.

```
my_int :: typedef int;

i: int = 10;
mi: my_int = :my_int(i); // requires a cast
```

A simple alias can be defined by adding a '@weak' note:

```
my_int: typedef int; @weak

i: int = 10;
mi: my_int = i;
```

## Compound expressions

These are expressions meant for initializing structs and unions.

- Particular members can be targeted by name.
- Members that were not mentioned are initialized to zero.

``` odin
Vector2  :: struct { x: float; y: float; }
Camera2D :: struct {
    offset:   Vector2;
    target:   Vector2;
    rotation: float;
    zoom:     float;
}

Camera: Camera2D = {
    offset = {100, 100},
    zoom   = 1.0,
};

SecondCamera := :Camera2D{{1, 1}, zoom = 1.0};
```

## Arrays and compound expressions

``` odin
array: [100]int; // array of 100 integers

inited_array: [100]int = {
    0,
    [4]  = 4,
    [50] = 50,
    [99] = 99,
    // remaining values are '0'ed
};
```

## Named and default procedure arguments

``` odin
RED           :: 0xff0000ff;
DrawRectangle :: proc(x: int, y: int, w: int, h: int, color: uint = RED);

main :: proc(): int {
    DrawRectangle(x = 100, y = 100, w = 50, h = 50);
    return 0;
}
```

## Simulating enums

The language doesn't have enums but similar feature can be easily simulated. Operator '^' allows you to generate a list of enumerated constants: 1, 2, 3 ... using typedef you can create a simple integer type with additional type safety.

```
TextKind :: typedef int;
TEXT_INVALID :: 0;
TEXT_BOLD    :: ^; // 1
TEXT_CURSIVE :: ^; // 2
TEXT_NORMAL  :: ^; // 3
```

## Enumerating bit flags

Operator '<<' allows you to generate a list of enumerated flags: 0b1, 0b10, 0b100 ...

```
EntityProps :: typedef uint;
ENTITY_CAN_BURN       :: 0b1;
ENTITY_CAN_MOVE       :: <<; // 0b10
ENTITY_CAN_BE_SHOCKED :: <<; // 0b100
ENTITY_WALKING        :: <<; // 0b1000
```

## `*char` and String types

The language by default is very `*char` friendly. Strings default to the cstring type which functions in exactly the same way as in C. It functions as an array of bytes which is null terminated by a null `0`.

But the language also has it's own String type:

```
String :: struct {
    str: *char;
    len:  int;
}
```

Untyped strings work well with both:

``` odin
a: String = "sized string";
b: *char  = "c string";
c:        = "this defaults to *char";
```

`String` is a structure so you need to index it using `.str` or `.len`.

``` odin
test :: proc() {
    a: *char = "C string";
    c := a[1];

    b: String = "Len string";
    c = b.str[1];
    len := b.len; @unused
}
```

A `lengthof` operator can be used to learn how big a constant string is, this also works for arrays:

``` odin
str :: "String";
len :: lengthof(str);
```

## Pointer arithmetic

``` odin
import "libc";

test :: proc() {
    a: *int = malloc(sizeof(:int) * 10);
    c: *int = addptr(a, 4);
    d: *int = &a[4];
    // there is no: (a + 4)
}
```

## Casting between types

```
a: float = :float(10);

b := &a;
c := :*int(b);
```

## If statement

``` odin
if_statement_showcase :: proc(cond: int) {
    if cond == 1 {
        //
    } else if cond == 2 {
        //
    } else {
        //
    }
}
```

## Switch statement

``` odin
import "libc";
switch_statement_showcase :: proc(value: int)
{
    switch value
    {
        case 0,4,8:
        {
            printf("0, 4, 8\n");
        }
        case 1, 2, 3:
        {
            printf("1, 2, 3\n");
        } @fallthrough
        case 5:
        {
            printf("this activates when 1, 2, 3 and 5\n");
        }
        default: printf("default case");
    }
}
```

## For loop

``` odin
for_loop_showcase :: proc() {

    // infinite loop
    for {
        /* .. */
    }

    cond := true;
    for cond {
        break;
    }

    for i := 0; i < 4; i += 1 {
        //
    }

    i := 0;
    for i < 4 {
        i += 1;
    }

    i = 0;
    for i < 4; i += 1 {
        //
    }
}
```

## Labeled loops, breaks and continues

You can label a loop and use that label to control the outer loop.

``` odin
test :: proc() {
    cond := true;
    outer_loop: for cond {
        for cond {
            break outer_loop; // continue works too
        }
    }
}
```

## Arithmetic operators

``` odin
arithmetic_operators :: proc() {
    add                   := 1 +  2;
    sub                   := 1 -  2;
    mul                   := 1 *  2;
    div                   := 1 /  2;
    mod                   := 1 %  2;

    and                   := 1 && 2;
    or                    := 1 || 2;
    equals                := 1 == 2;
    not_equals            := 1 != 2;
    greater_then          := 1 >  2;
    greater_then_or_equal := 1 >= 2;
    lesser_then           := 1 <  2;
    lesser_then_or_equal  := 1 <= 2;

    left_shift            := 1 >> 2;
    right_shift           := 1 << 2;
    bit_or                := 1 |  2;
    bit_and               := 1 &  2;
    bit_xor               := 1 ^  2;

    not                   := !1;
    negate                := ~1;
}
```

## Arithmetic assignment operators

``` odin
arithmetic_assign_operators :: proc() {
    a  := 1;
    a  /= 1;
    a  *= 1;
    a  %= 1;
    a  -= 1;
    a  += 1;
    a  &= 1;
    a  |= 1;
    a  ^= 1;
    a <<= 1;
    a >>= 1;
}
```


## Defer statement

A defer statement allows to postpone a certain action to the end of scope. For example - you might allocate memory and use defer just bellow your allocation to free the memory. This free will be deferred, it will happen a bit later.

``` odin
import "libc";
test :: proc() {
    // it's useful to keep certain pairs of actions together:
    file := fopen("data.txt", "rb");
    if (file) {
        defer fclose(file);
        // .. do things
    }
}
```

## Notes

You can annotate declarations, statements etc. This puts metadata on the AST which you can access from a metaprogram.

``` odin
// a note that starts with '#', doesn't bind to anything, it basically acts as a directive.
#do_something(a, b, c);


Data :: struct {
    a:  int;
    b: *int;
} @serialize // binds to AST of Data
```

The language uses notes to implement a bunch of features, for example:

``` odin
#static_assert(sizeof(:int) == 4);


test :: proc() {
    // ...
} @build_if(LC_OS == OS_WINDOWS)
```

## Conditional compilation

It's important for a low level language to have some form of platform awareness. We don't have includes and preprocessor here so I decided to implement a construct that works during parsing stage. It discards declarations and doesn't let them into typechecking based on the evaluation of the inner expression.

'#build_if' can appear at the top of the file, it decides whether to compile that particular file or not:

```  odin
#build_if(LC_OS == OS_WINDOWS);
```

'@build_if' can be bound to any top level declaration.

``` odin
LONG_MAX :: 0xFFFFFFFF;         @build_if(LC_OS == OS_WINDOWS)
LONG_MAX :: 0xFFFFFFFFFFFFFFFF; @build_if(LC_OS == OS_MAC || LC_OS == OS_LINUX)
```

## Builtins (like sizeof)

``` odin
a := sizeof(:int);
b := alignof(:int);

A :: struct { a: int; b: int; }
c := offsetof(:A, b);

d := lengthof("asd");
e := typeof(d);

#static_assert(offsetof(:A, b) == 4);
```

## Raw C code when targeting the C generator

``` odin
#`#include <stdlib.h>`;

global_variable := 0;

main :: proc(): int {
    a: *int = #`malloc(32)`;
    #`a[0] = lc_package_global_variable`;
    return a[0];
}
```

## Any and typeof

```  odin
Any :: struct {
    type: int;
    data: *void;
}
```

``` odin
test :: proc() {
    a: Any = 32;
    b: Any = "thing";
    c: Any = b;

    i: int = 10;
    d: Any = &i;
}
```

``` odin
print :: proc(v: Any) {
    switch(v.type) {
        case typeof(:int): {
            val := :*int(v.data);
            // ..
        }
        case typeof(:float): {
            val := :*float(v.data);
            // ..
        }
    }
}

main :: proc() {
    print(32);
    print("asd");
}
```

## Variadic arguments with Any promotion

``` odin
import "libc";

any_vargs :: proc(fmt: *char, ...Any) {
    va: va_list;
    va_start(va, fmt);

    for i := 0; fmt[i]; i += 1 {
        if fmt[i] == '%' {
            arg := va_arg_any(va);

            if arg.type == typeof(:int) {
                val := :*int(arg.data);
                // ..
            } else {
                // ..
            }
        }
    }

    va_end(va);
}

main :: proc() {
    any_vargs("testing % %", 32, "thing");
}
```

## Comments

``` odin
// I'm a line comment

/* I'm a block comment
    /*
        I can even nest!
    */
*/
```

## Doc comments

``` odin
/** package

This is a package doc comment,
    there can only be 1 per package,
    it appears at top of the file.
It binds to the package AST.

*/

/** file

This is a file doc comment,
    there can only be 1 per file,
    it appears at top of the file under package doc comment.
It binds to the file AST.

*/

/**

This is a top level declaration doc comment,
    1 per declaration,
    it appears above the declaration.
It binds to the declaration AST.

*/
A :: proc(): int {
    return 0;
}
```

## Packed structs

``` odin
A :: struct {
    a: short;
    b: char;
    c: int;
    d: char;
    e: llong;
    f: uchar;
} @packed
```

## Useful resources for compiler development

* https://c3.handmade.network/blog/p/8632-handling_parsing_and_semantic_errors_in_a_compiler - great article by Christoffer Lerno (C3 author) about handling errors in compilers.
* https://www.youtube.com/watch?v=bNJhtKPugSQ - Walter Bright (D author) talks about AST poisoning here.
* https://bitwise.handmade.network/ - series by Per Vognsen where he actually creates a C like language, very helpful, very hands on!
* https://hero.handmade.network/episode/code/day206/ - this episode of handmade hero started me on the entire compiler journey, a long, long time ago.
* https://www.youtube.com/watch?v=TH9VCN6UkyQ&list=PLmV5I2fxaiCKfxMBrNsU1kgKJXD3PkyxO - I have re-watched this playlist many this, searching for keywords and ideas. Jonathan Blow's compiler was a big inspiration of mine when learning programming languages.
* A Retargetable C Compiler: Design and Implementation by Christopher W. Fraser and David R. Hanson - sometimes looked at this as a reference to figure stuff out. Very helpful resource on compiler construction, the way it's written reads more like documentation but you use what you have.
* Compiler Construction by Niklaus Wirth - https://people.inf.ethz.ch/wirth/CompilerConstruction/index.html - have to learn to read Pascal / Oberon but he goes over implementing all the stages of the compiler. Wirth's project of implementing an entire hardware / software stack is really inspiring.
* https://github.com/rui314/chibicc - great resource for learning how to write a very dumb x64 backend. I like the format, you go over git commits, doesn't work well on github though, with Sublime Merge it's a pleasure to follow.
* https://go.dev/blog/constants - article on golang type system, untyped types, constants that kind of stuff.
* https://github.com/JoshuaManton/sif - looked at this as a reference from time to time, author seems like a Jonathan Blow fan so it was a good resource informed by similar resources as I used.
* https://github.com/gingerbill/odin - I sometimes peeked at the compiler to figure stuff out when I was confused.
https://c3.handmade.network/blog - Christoffer Lerno (C3 author) blog.
* https://github.com/c3lang/c3c - I sometimes looked at C3 compiler as a reference, the author also let me use his bigint library, thanks a lot! :)
