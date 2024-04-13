#define MAX_REG 3
bool RegTaken[MAX_REG] = {0, 0, 0};
Register Regs[MAX_REG] = {R10, R11, R12};

Register AllocReg() {
    for (int i = 0; i < 3; i += 1) {
        if (RegTaken[i] == false) {
            RegTaken[i] = true;
            return Regs[i];
        }
    }
    IO_InvalidCodepath();
    return INVALID_REGISTER;
}

void DeallocReg(Register reg) {
    int i = (int)(reg - R10);
    RegTaken[i] = false;
}

const char *Str(Register reg) {
    const char *registers[] = {
        #define X(x) #x,
        REGISTERS
        #undef X
    };
    return registers[reg];
}