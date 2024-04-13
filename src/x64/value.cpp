const char *Str(Value value) {
    if (value.mode == Mode_Reg) {
        return LC_Strf("%s", Str(value.reg));
    } else if (value.mode == Mode_Direct) {
        return LC_Strf("[%s]", Str(value.reg));
    } else if (value.mode == Mode_Imm) {
        return LC_Strf("%llu", value.u);
    } else IO_Todo();
    return "INVALID_VALUE";
}

void DeallocReg(Value value) {
    if (value.mode == Mode_Imm) return;
    DeallocReg(value.reg);
}