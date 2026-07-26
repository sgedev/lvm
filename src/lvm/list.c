/*
 *
 */
#include <lvm/list.h>

LVM_API size_t lvm_list_length(const lvm_list_t* list) {
    size_t length = 0;
    const lvm_list_node_t* p;
    LVM_ASSERT(list != NULL);
    for (p = list->knot.next; p != &list->knot; p = p->next) {
        length += 1;
    }
    return length;
}

LVM_API bool lvm_list_contains(const lvm_list_t* list, const lvm_list_node_t* node) {
    const lvm_list_node_t* p;
    LVM_ASSERT(list != NULL);
    LVM_ASSERT(node != NULL);
    for (p = list->knot.next; p != &list->knot; p = p->next) {
        if (p == node) {
            return true;
        }
    }
    return false;
}

LVM_API void lvm_list_clear(lvm_list_t* list) {
    const lvm_list_node_t* p;
    LVM_ASSERT(list != NULL);
    while (!lvm_list_is_empty(list)) {
        lvm_list_remove_first(list);
    }
}
