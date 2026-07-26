/*
 *
 */
#ifndef LVM_LIST_H
#define LVM_LIST_H

#include <lvm/common.h>

LVM_BEGIN_DECLS

typedef struct lvm_list_node {
    struct lvm_list_node* prev;
    struct lvm_list_node* next;
} lvm_list_node_t;

typedef struct {
    lvm_list_node_t knot;
} lvm_list_t;

LVM_API size_t lvm_list_length(const lvm_list_t* list);
LVM_API bool lvm_list_contains(
    const lvm_list_t* list,
    const lvm_list_node_t* node);
LVM_API void lvm_list_clear(lvm_list_t* list);

LVM_FORCE_INLINE void lvm_list_node_reset(lvm_list_node_t* node) {
    LVM_ASSERT(node != NULL);
    node->prev = node;
    node->next = node;
}

LVM_FORCE_INLINE bool lvm_list_node_is_linked(
    const lvm_list_node_t* node) {
    LVM_ASSERT(node != NULL);
    return node->next != node;
}

LVM_FORCE_INLINE void lvm_list_node_link(
    lvm_list_node_t* node,
    lvm_list_node_t* prev,
    lvm_list_node_t* next) {
    LVM_ASSERT(node != NULL);
    LVM_ASSERT(prev != NULL);
    LVM_ASSERT(next != NULL);
    node->prev = prev;
    node->next = next;
    prev->next = node;
    next->prev = node;
}

LVM_FORCE_INLINE void lvm_list_node_unlink(lvm_list_node_t* node) {
    LVM_ASSERT(node != NULL);
    LVM_ASSERT(node->prev != NULL);
    LVM_ASSERT(node->next != NULL);
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->prev = node;
    node->next = node;
}

LVM_FORCE_INLINE lvm_list_node_t* lvm_list_node_prev(
    lvm_list_node_t* node) {
    LVM_ASSERT(node != NULL);
    return node->prev;
}

LVM_FORCE_INLINE lvm_list_node_t* lvm_list_node_next(
    lvm_list_node_t* node) {
    LVM_ASSERT(node != NULL);
    return node->next;
}

LVM_FORCE_INLINE void lvm_list_reset(lvm_list_t* list) {
    LVM_ASSERT(list != NULL);
    lvm_list_node_reset(&list->knot);
}

LVM_FORCE_INLINE bool lvm_list_is_empty(const lvm_list_t* list) {
    LVM_ASSERT(list != NULL);
    return !lvm_list_node_is_linked(&list->knot);
}

LVM_FORCE_INLINE lvm_list_node_t* lvm_list_knot(lvm_list_t* list) {
    LVM_ASSERT(list != NULL);
    return &list->knot;
}

LVM_FORCE_INLINE lvm_list_node_t* lvm_list_first(lvm_list_t* list) {
    LVM_ASSERT(list != NULL);
    return list->knot.next;
}

LVM_FORCE_INLINE lvm_list_node_t* lvm_list_last(lvm_list_t* list) {
    LVM_ASSERT(list != NULL);
    return list->knot.prev;
}

LVM_FORCE_INLINE void lvm_list_prepend(
    lvm_list_t* list,
    lvm_list_node_t* node) {
    LVM_ASSERT(list != NULL);
    LVM_ASSERT(node != NULL);
    lvm_list_node_link(node, lvm_list_knot(list), lvm_list_first(list));
}

LVM_FORCE_INLINE void lvm_list_append(
    lvm_list_t* list,
    lvm_list_node_t* node) {
    LVM_ASSERT(list != NULL);
    LVM_ASSERT(node != NULL);
    lvm_list_node_link(node, lvm_list_last(list), lvm_list_knot(list));
}

LVM_FORCE_INLINE void lvm_list_insert_before(
    lvm_list_node_t* curr,
    lvm_list_node_t* node) {
    LVM_ASSERT(curr != NULL);
    LVM_ASSERT(node != NULL);
    lvm_list_node_link(node, lvm_list_node_prev(curr), curr);
}

LVM_FORCE_INLINE void lvm_list_insert_after(
    lvm_list_node_t* curr,
    lvm_list_node_t* node) {
    LVM_ASSERT(curr != NULL);
    LVM_ASSERT(node != NULL);
    lvm_list_node_link(node, curr, lvm_list_node_next(curr));
}

LVM_FORCE_INLINE lvm_list_node_t* lvm_list_remove_first(
    lvm_list_t* list) {
    lvm_list_node_t* node;
    LVM_ASSERT(list != NULL);
    node = lvm_list_first(list);
    lvm_list_node_unlink(node);
    return node;
}

LVM_FORCE_INLINE lvm_list_node_t* lvm_list_remove_last(
    lvm_list_t* list) {
    lvm_list_node_t* node;
    LVM_ASSERT(list != NULL);
    node = lvm_list_last(list);
    lvm_list_node_unlink(node);
    return node;
}

LVM_FORCE_INLINE void lvm_list_remove(
    lvm_list_t* list,
    lvm_list_node_t* node) {
    LVM_ASSERT(list != NULL);
    LVM_ASSERT(node != NULL);
    LVM_ASSERT(lvm_list_contains(list, node));
    lvm_list_node_unlink(node);
}

LVM_END_DECLS

#define LVM_LIST_FOREACH(pnode, plist) \
    for ((pnode) = lvm_list_first(plist); \
         (pnode) != lvm_list_knot(plist); \
         (pnode) = lvm_list_node_next(pnode))

#define LVM_LIST_FOREACH_R(pnode, plist) \
    for ((pnode) = lvm_list_last(plist); \
         (pnode) != lvm_list_knot(plist); \
         (pnode) = lvm_list_node_prev(pnode))

#endif /* LVM_LIST_H */
