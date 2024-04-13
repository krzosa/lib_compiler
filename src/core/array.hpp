// Iterating and removing elements
//
//     ForArrayRemovable(array) {
//         ForArrayRemovablePrepare(array);
//         if (it == 4) ForArrayRemovableDeclare();
//     }
//
#define ForArrayRemovable(a) for (int __i = 0; __i < (a).len; __i += 1)
#define ForArrayRemovablePrepare(a) \
    auto &it        = (a)[__i];     \
    bool  remove_it = false;        \
    defer {                         \
        if (remove_it) {            \
            (a).ordered_remove(it); \
            __i -= 1;               \
        }                           \
    }
#define ForArrayRemovableDeclare() (remove_it = true)
#define For2(it, array) for (auto &it : (array))
#define For(array) For2(it, array)

template <class T>
struct Array {
    M_Allocator allocator;
    T          *data;
    int         cap, len;

    T &operator[](int index) {
        IO_Assert(index >= 0 && index < len);
        return data[index];
    }

    bool is_first(T &item) { return &item == first(); }
    bool is_last(T &item) { return &item == last(); }

    bool contains(T &item) {
        bool result = &item >= data && &item < data + len;
        return result;
    }

    int get_index(T &item) {
        IO_Assert((data <= &item) && ((data + len) > &item));
        size_t offset = &item - data;
        return (int)offset;
    }

    void add(const T &item) {
        try_growing();
        data[len++] = item;
    }

    // Struct needs to have 'value_to_sort_by' field
    void sorted_insert_decreasing(T item) {
        int insert_index = -1;
        For(*this) {
            if (it.value_to_sort_by <= item.value_to_sort_by) {
                insert_index = get_index(it);
                insert(item, insert_index);
                break;
            }
        }

        if (insert_index == -1) {
            add(item);
        }
    }

    void bounded_add(T item) {
        IO_Assert(len + 1 <= cap);
        try_growing(); // in case of error
        data[len++] = item;
    }

    T *alloc(const T &item) {
        try_growing();
        T *ref = data + len++;
        *ref   = item;
        return ref;
    }

    T *alloc() {
        try_growing();
        T *ref = data + len++;
        *ref   = {};
        return ref;
    }

    T *alloc_multiple(int size) {
        try_growing_to_fit_item_count(size);
        T *result = data + len;
        len += size;
        return result;
    }

    void add_array(T *items, int item_count) {
        for (int i = 0; i < item_count; i += 1) {
            add(items[i]);
        }
    }

    void add_array(Array<T> items) {
        add_array(items.data, items.len);
    }

    void reserve(int size) {
        if (size > cap) {
            if (!allocator.p) allocator = M_GetSystemAllocator();

            void *p = M_Realloc(allocator, data, size * sizeof(T), cap * sizeof(T));
            IO_Assert(p);

            data = (T *)p;
            cap  = size;
        }
    }

    void init(M_Allocator allocator, int size) {
        len             = 0;
        cap             = 0;
        data            = 0;
        this->allocator = allocator;
        reserve(size);
    }

    void reset() {
        len = 0;
    }

    T pop() {
        IO_Assert(len > 0);
        return data[--len];
    }

    void unordered_remove(T &item) { // DONT USE IN LOOPS !!!!
        IO_Assert(len > 0);
        IO_Assert(&item >= begin() && &item < end());
        item = data[--len];
    }

    void unordered_remove_index(int index) {
        IO_Assert(index >= 0 && index < len);
        data[index] = data[--len];
    }

    int get_index(const T &item) {
        ptrdiff_t index = (ptrdiff_t)(&item - data);
        IO_Assert(index >= 0 && index < len);
        // IO_Assert(index > INT_MIN && index < INT_MAX);
        return (int)index;
    }

    void ordered_remove(T &item) { // DONT USE IN LOOPS !!!
        IO_Assert(len > 0);
        IO_Assert(&item >= begin() && &item < end());
        int index = get_index(item);
        ordered_remove_index(index);
    }

    void ordered_remove_index(int index) {
        IO_Assert(index >= 0 && index < len);
        int right_len = len - index - 1;
        memmove(data + index, data + index + 1, right_len * sizeof(T));
        len -= 1;
    }

    void insert(T item, int index) {
        if (index == len) {
            add(item);
            return;
        }

        IO_Assert(index < len);
        IO_Assert(index >= 0);

        try_growing();
        int right_len = len - index;
        memmove(data + index + 1, data + index, sizeof(T) * right_len);
        data[index] = item;
        len += 1;
    }

    void dealloc() {
        if (data) M_Dealloc(allocator, data);
        data = 0;
        len = cap = 0;
    }

    Array<T> copy(M_Allocator allocator) {
        Array result     = {};
        result.allocator = allocator;
        result.reserve(cap);

        memmove(result.data, data, sizeof(T) * len);
        result.len = len;
        return result;
    }

    Array<T> tight_copy(M_Allocator allocator) {
        Array result     = {};
        result.allocator = allocator;
        result.reserve(len);

        memmove(result.data, data, sizeof(T) * len);
        result.len = len;
        return result;
    }

    T *first() {
        IO_Assert(len > 0);
        return data;
    }
    T *last() {
        IO_Assert(len > 0);
        return data + len - 1;
    }
    T *front() {
        IO_Assert(len > 0);
        return data;
    }
    T *back() {
        IO_Assert(len > 0);
        return data + len - 1;
    }
    T *begin() { return data; }
    T *end() { return data + len; }

    // for (auto it = integers.begin(), end = integers.end(); it != end; ++it)
    struct Reverse_Iter {
        T        *data;
        Array<T> *arr;

        Reverse_Iter operator++(int) {
            Reverse_Iter ret = *this;
            data -= 1;
            return ret;
        }
        Reverse_Iter &operator++() {
            data -= 1;
            return *this;
        }

        T &operator*() { return data[0]; }
        T *operator->() { return data; }

        friend bool operator==(const Reverse_Iter &a, const Reverse_Iter &b) { return a.data == b.data; };
        friend bool operator!=(const Reverse_Iter &a, const Reverse_Iter &b) { return a.data != b.data; };

        Reverse_Iter begin() { return Reverse_Iter{arr->end() - 1, arr}; }
        Reverse_Iter end() { return Reverse_Iter{arr->begin() - 1, arr}; }
    };

    Reverse_Iter reverse() { return {end() - 1, this}; }

    void try_growing() {
        if (len + 1 > cap) {
            int new_size = cap * 2;
            if (new_size < 16) new_size = 16;

            reserve(new_size);
        }
    }

    void try_growing_to_fit_item_count(int item_count) {
        if (len + item_count > cap) {
            int new_size = (cap + item_count) * 2;
            if (new_size < 16) new_size = 16;
            reserve(new_size);
        }
    }
};
