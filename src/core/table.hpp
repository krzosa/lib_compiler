/*
    Hash table implementation:
        Pointers to values
        Open adressing
        Linear Probing
        Power of 2
        Robin Hood hashing
        Resizes on high probe count (min max load factor)

        Hash 0 is reserved for empty hash table entry
*/

template <class Value>
struct Table {
    struct Entry {
        uint64_t hash;
        uint64_t key;
        size_t   distance;
        Value    value;
    };

    M_Allocator allocator;
    size_t      len, cap;
    Entry      *values;

    static const size_t max_load_factor      = 80;
    static const size_t min_load_factor      = 50;
    static const size_t significant_distance = 8;

    // load factor calculation was rearranged
    // to get rid of division:
    //> 100 * len / cap = load_factor
    //> len * 100 = load_factor * cap
    inline bool reached_load_factor(size_t lfactor) {
        return (len + 1) * 100 >= lfactor * cap;
    }

    inline bool is_empty(Entry *entry) { return entry->hash == 0; }
    inline bool is_occupied(Entry *entry) { return entry->hash != 0; }

    void reserve(size_t size) {
        IO_Assert(size > cap && "New size is smaller then original size");
        IO_Assert(MA_IS_POW2(size));
        if (!allocator.p) allocator = M_GetSystemAllocator();

        Entry *old_values = values;
        size_t old_cap    = cap;

        values = (Entry *)M_Alloc(allocator, sizeof(Entry) * size);
        for (int i = 0; i < size; i += 1) values[i] = {};
        cap = size;

        IO_Assert(!(old_values == 0 && len != 0));
        if (len == 0) {
            if (old_values) M_Dealloc(allocator, old_values);
            return;
        }

        len = 0;
        for (size_t i = 0; i < old_cap; i += 1) {
            Entry *it = old_values + i;
            if (is_occupied(it)) {
                insert(it->key, it->value);
            }
        }
        M_Dealloc(allocator, old_values);
    }

    Entry *get_table_entry(uint64_t key) {
        if (len == 0) return 0;
        uint64_t hash = HashBytes(&key, sizeof(key));
        if (hash == 0) hash += 1;
        uint64_t index    = WRAP_AROUND_POWER_OF_2(hash, cap);
        uint64_t i        = index;
        uint64_t distance = 0;
        for (;;) {
            Entry *it = values + i;
            if (distance > it->distance) {
                return 0;
            }

            if (it->hash == hash && it->key == key) {
                return it;
            }

            distance += 1;
            i = WRAP_AROUND_POWER_OF_2(i + 1, cap);
            if (i == index) return 0;
        }
        IO_Assert(!"Invalid codepath");
    }

    void insert(uint64_t key, const Value &value) {
        if (reached_load_factor(max_load_factor)) {
            if (cap == 0) cap = 16; // 32 cause cap*2
            reserve(cap * 2);
        }

        uint64_t hash = HashBytes(&key, sizeof(key));
        if (hash == 0) hash += 1;
        uint64_t index     = WRAP_AROUND_POWER_OF_2(hash, cap);
        uint64_t i         = index;
        Entry    to_insert = {hash, key, 0, value};
        for (;;) {
            Entry *it = values + i;
            if (is_empty(it)) {
                *it = to_insert;
                len += 1;
                // If we have more then 8 consecutive items we try to resize
                if (to_insert.distance > 8 && reached_load_factor(min_load_factor)) {
                    reserve(cap * 2);
                }
                return;
            }
            if (it->hash == hash && it->key == key) {
                *it = to_insert;
                // If we have more then 8 consecutive items we try to resize
                if (to_insert.distance > 8 && reached_load_factor(min_load_factor)) {
                    reserve(cap * 2);
                }
                return;
            }

            // Robin hood hashing
            if (to_insert.distance > it->distance) {
                Entry temp = to_insert;
                to_insert  = *it;
                *it        = temp;
            }

            to_insert.distance += 1;
            i = WRAP_AROUND_POWER_OF_2(i + 1, cap);
            IO_Assert(i != index && "Did a full 360 through a hash table, no good :( that shouldnt be possible");
        }
        IO_Assert(!"Invalid codepath");
    }

    void remove(uint64_t key) {
        Entry *entry    = get_table_entry(key);
        entry->hash     = 0;
        entry->distance = 0;
        len -= 1;
    }

    Value *get(uint64_t key) {
        Entry *v = get_table_entry(key);
        if (!v) return 0;
        return &v->value;
    }

    Value get(uint64_t key, Value default_value) {
        Entry *v = get_table_entry(key);
        if (!v) return default_value;
        return v->value;
    }

    Value *gets(char *str) {
        int      len  = S8_Length(str);
        uint64_t hash = HashBytes(str, len);
        return get(hash);
    }

    Value gets(char *str, Value default_value) {
        int      len  = S8_Length(str);
        uint64_t hash = HashBytes(str, len);
        return get(hash, default_value);
    }

    Value *get(S8_String s) {
        uint64_t hash = HashBytes(s.str, (unsigned)s.len);
        return get(hash);
    }

    Value get(S8_String s, Value default_value) {
        uint64_t hash = HashBytes(s.str, (unsigned)s.len);
        return get(hash, default_value);
    }

    void put(S8_String s, const Value &value) {
        uint64_t hash = HashBytes(s.str, (unsigned)s.len);
        insert(hash, value);
    }

    void puts(char *str, const Value &value) {
        int      len  = S8_Length(str);
        uint64_t hash = HashBytes(str, len);
        insert(hash, value);
    }

    void reset() {
        len = 0;
        for (size_t i = 0; i < cap; i += 1) {
            Entry *it = values + i;
            it->hash  = 0;
        }
    }

    void dealloc() {
        M_Dealloc(allocator, values);
        len    = 0;
        cap    = 0;
        values = 0;
    }
};
