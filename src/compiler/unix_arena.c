#define LC_V_UNIX_PAGE_SIZE 4096

LC_FUNCTION LC_VMemory LC_VReserve(size_t size) {
    LC_VMemory result       = {};
    size_t     size_aligned = LC_AlignUp(size, LC_V_UNIX_PAGE_SIZE);
    result.data             = (uint8_t *)mmap(0, size_aligned, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    LC_Assertf(result.data, "Failed to reserve memory using mmap!!");
    if (result.data) {
        result.reserve = size_aligned;
    }
    return result;
}

LC_FUNCTION bool LC_VCommit(LC_VMemory *m, size_t commit) {
    uint8_t *pointer = LC_V_AdvanceCommit(m, &commit, LC_V_UNIX_PAGE_SIZE);
    if (pointer) {
        int mprotect_result = mprotect(pointer, commit, PROT_READ | PROT_WRITE);
        LC_Assertf(mprotect_result == 0, "Failed to commit more memory using mmap");
        if (mprotect_result == 0) {
            m->commit += commit;
            return true;
        }
    }
    return false;
}

LC_FUNCTION void LC_VDeallocate(LC_VMemory *m) {
    int result = munmap(m->data, m->reserve);
    LC_Assertf(result == 0, "Failed to release virtual memory using munmap");
}