const size_t LC_V_WIN32_PAGE_SIZE = 4096;

LC_FUNCTION LC_VMemory LC_VReserve(size_t size) {
    LC_VMemory result;
    LC_MemoryZero(&result, sizeof(result));
    size_t adjusted_size = LC_AlignUp(size, LC_V_WIN32_PAGE_SIZE);
    result.data          = (uint8_t *)VirtualAlloc(0, adjusted_size, MEM_RESERVE, PAGE_READWRITE);
    LC_Assertf(result.data, "Failed to reserve virtual memory");
    result.reserve = adjusted_size;
    return result;
}

LC_FUNCTION bool LC_VCommit(LC_VMemory *m, size_t commit) {
    uint8_t *pointer = LC_V_AdvanceCommit(m, &commit, LC_V_WIN32_PAGE_SIZE);
    if (pointer) {
        void *result = VirtualAlloc(pointer, commit, MEM_COMMIT, PAGE_READWRITE);
        LC_Assertf(result, "Failed to commit more memory");
        if (result) {
            m->commit += commit;
            return true;
        }
    }
    return false;
}

LC_FUNCTION void LC_VDeallocate(LC_VMemory *m) {
    BOOL result = VirtualFree(m->data, 0, MEM_RELEASE);
    LC_Assertf(result != 0, "Failed to release LC_VMemory");
}

LC_FUNCTION bool LC_VDecommitPos(LC_VMemory *m, size_t pos) {
    size_t aligned          = LC_AlignDown(pos, LC_V_WIN32_PAGE_SIZE);
    size_t adjusted_pos     = LC_CLAMP_TOP(aligned, m->commit);
    size_t size_to_decommit = m->commit - adjusted_pos;
    if (size_to_decommit) {
        uint8_t *base_address = m->data + adjusted_pos;
        BOOL     result       = VirtualFree(base_address, size_to_decommit, MEM_DECOMMIT);
        if (result) {
            m->commit -= size_to_decommit;
            return true;
        }
    }
    return false;
}