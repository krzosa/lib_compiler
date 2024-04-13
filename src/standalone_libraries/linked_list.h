#ifndef FIRST_LL_HEADER
#define FIRST_LL_HEADER
#define SLL_QUEUE_ADD_MOD(f, l, n, next) \
    do {                                 \
        (n)->next = 0;                   \
        if ((f) == 0) {                  \
            (f) = (l) = (n);             \
        } else {                         \
            (l) = (l)->next = (n);       \
        }                                \
    } while (0)
#define SLL_QUEUE_ADD(f, l, n) SLL_QUEUE_ADD_MOD(f, l, n, next)

#define SLL_QUEUE_POP_FIRST_MOD(f, l, next) \
    do {                                    \
        if ((f) == (l)) {                   \
            (f) = (l) = 0;                  \
        } else {                            \
            (f) = (f)->next;                \
        }                                   \
    } while (0)
#define SLL_QUEUE_POP_FIRST(f, l) SLL_QUEUE_POP_FIRST_MOD(f, l, next)

#define SLL_STACK_ADD_MOD(stack_base, new_stack_base, next) \
    do {                                                    \
        (new_stack_base)->next = (stack_base);              \
        (stack_base)           = (new_stack_base);          \
    } while (0)
#define SLL_STACK_ADD(stack_base, new_stack_base) \
    SLL_STACK_ADD_MOD(stack_base, new_stack_base, next)

#define SLL_STACK_POP_AND_STORE(stack_base, out_node) \
    do {                                              \
        if (stack_base) {                             \
            (out_node)       = (stack_base);          \
            (stack_base)     = (stack_base)->next;    \
            (out_node)->next = 0;                     \
        }                                             \
    } while (0)

#define DLL_QUEUE_ADD_MOD(f, l, node, next, prev) \
    do {                                          \
        if ((f) == 0) {                           \
            (f) = (l)    = (node);                \
            (node)->prev = 0;                     \
            (node)->next = 0;                     \
        } else {                                  \
            (l)->next    = (node);                \
            (node)->prev = (l);                   \
            (node)->next = 0;                     \
            (l)          = (node);                \
        }                                         \
    } while (0)
#define DLL_QUEUE_ADD(f, l, node) DLL_QUEUE_ADD_MOD(f, l, node, next, prev)
#define DLL_QUEUE_ADD_FRONT(f, l, node) DLL_QUEUE_ADD_MOD(l, f, node, prev, next)
#define DLL_QUEUE_REMOVE_MOD(first, last, node, next, prev) \
    do {                                                    \
        if ((first) == (last)) {                            \
            (first) = (last) = 0;                           \
        } else if ((last) == (node)) {                      \
            (last)       = (last)->prev;                    \
            (last)->next = 0;                               \
        } else if ((first) == (node)) {                     \
            (first)       = (first)->next;                  \
            (first)->prev = 0;                              \
        } else {                                            \
            (node)->prev->next = (node)->next;              \
            (node)->next->prev = (node)->prev;              \
        }                                                   \
        if (node) {                                         \
            (node)->prev = 0;                               \
            (node)->next = 0;                               \
        }                                                   \
    } while (0)
#define DLL_QUEUE_REMOVE(first, last, node) DLL_QUEUE_REMOVE_MOD(first, last, node, next, prev)

#define DLL_STACK_ADD_MOD(first, node, next, prev) \
    do {                                           \
        (node)->next = (first);                    \
        if ((first))                               \
            (first)->prev = (node);                \
        (first)      = (node);                     \
        (node)->prev = 0;                          \
    } while (0)
#define DLL_STACK_ADD(first, node) DLL_STACK_ADD_MOD(first, node, next, prev)
#define DLL_STACK_REMOVE_MOD(first, node, next, prev) \
    do {                                              \
        if ((node) == (first)) {                      \
            (first) = (first)->next;                  \
            if ((first))                              \
                (first)->prev = 0;                    \
        } else {                                      \
            (node)->prev->next = (node)->next;        \
            if ((node)->next)                         \
                (node)->next->prev = (node)->prev;    \
        }                                             \
        if (node) {                                   \
            (node)->prev = 0;                         \
            (node)->next = 0;                         \
        }                                             \
    } while (0)
#define DLL_STACK_REMOVE(first, node) DLL_STACK_REMOVE_MOD(first, node, next, prev)

#define DLL_INSERT_NEXT_MOD(base, new, next, prev)      \
    do {                                                \
        if ((base) == 0) {                              \
            (base)      = (new);                        \
            (new)->next = 0;                            \
            (new)->prev = 0;                            \
        } else {                                        \
            (new)->next  = (base)->next;                \
            (base)->next = (new);                       \
            (new)->prev  = (base);                      \
            if ((new)->next) (new)->next->prev = (new); \
        }                                               \
    } while (0)
#define DLL_INSERT_NEXT(base, new) DLL_INSERT_NEXT_MOD(base, new, next, prev)
#define DLL_INSERT_PREV(base, new) DLL_INSERT_NEXT_MOD(base, new, next, prev)
#endif