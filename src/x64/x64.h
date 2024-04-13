struct X64Decl {
    int offset;
};

/*
a: int;
a   // d lvalue
&a  // r not lvalue
*&a // d lvalue
*a  // illegal

b: *int;
*b  // d lvalue
b   // d lvalue
&b  // r not lvalue
**b // illegal
*/
enum Mode {
    Mode_Imm,    // Value is a constant and can be folded into instruction
    Mode_Reg,    // Value is in register
    Mode_Direct, // Register that holds address to value, (but this is lvalue, not "*a" but just "a")
};

#define REGISTERS\
    X(INVALID_REGISTER)\
    X(RAX) X(RBX) X(RCX) X(RDX) X(RSI) X(RDI) X(RBP) X(RSP)\
    X(R8)  X(R9)  X(R10) X(R11) X(R12) X(R13) X(R14) X(R15)

enum Register {
    #define X(x) x,
    REGISTERS
    #undef X
};

struct Value {
    Mode     mode;
    Register reg;
    uint64_t u;
};

Value GenExpr(LC_AST *n);