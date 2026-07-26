/*
 *
 */
#ifndef LVM_MODULE_H
#define LVM_MODULE_H

#include <lua.h>

#include <lvm/common.h>
#include <lvm/list.h>

LVM_BEGIN_DECLS

typedef struct {
    const char* name;
    const char* description;
    lvm_list_node_t node;
    lua_CFunction init;
    lua_CFunction exit;
} lvm_module_t;

LVM_API int lvm_module_init(
    lvm_module_t* module,
    const char* name,
    const char* description);

LVM_END_DECLS

#endif /* LVM_MODULE_H */
