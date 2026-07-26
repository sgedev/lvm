/*
 *
 */
#ifndef LVM_KERNEL_H
#define LVM_KERNEL_H

#include <stdio.h>

#include <lua.h>

#include <lvm/common.h>
#include <lvm/error.h>
#include <lvm/list.h>
#include <lvm/module.h>
#include <lvm/context.h>

LVM_BEGIN_DECLS

LVM_FORCE_INLINE lua_State* lvm_task_to_state(lvm_task_t* task) {
    return (lua_State*)LVM_PMOVB(task, sizeof(lvm_task_t));
}

LVM_FORCE_INLINE lvm_task_t* lvm_task_from_state(lua_State* L) {
    return (lvm_task_t*)lua_getextraspace(L);
}

struct lvm_kernel {
    lua_State* state;
    uv_loop_t loop;
    uv_async_t stopper;
    uv_prepare_t scheduler;
    lvm_list_t task_list[LVM_TASK_PRIORITY_MAX];
    lvm_list_t module_list;
    FILE* log;
};

LVM_FORCE_INLINE void lvm_task_ready(lvm_task_t* task) {
    LVM_ASSERT(task != NULL);
    LVM_ASSERT(task->kernel != NULL);
    LVM_ASSERT(task->priority < LVM_TASK_PRIORITY_MAX);
    lvm_list_append(&task->kernel->task_list[task->priority], &task->node);
}

LVM_FORCE_INLINE lvm_kernel_t* lvm_from_task(lvm_task_t* task) {
    return task->kernel;
}

LVM_FORCE_INLINE lvm_kernel_t* lvm_from_state(lua_State* L) {
    return lvm_from_task(lvm_task_from_state(L));
}

LVM_FORCE_INLINE uv_loop_t* lvm_loop(lvm_kernel_t* kernel) {
    LVM_ASSERT(kernel != NULL);
    return &kernel->loop;
}

LVM_FORCE_INLINE lua_State* lvm_state(lvm_kernel_t* kernel) {
    LVM_ASSERT(kernel != NULL);
    return kernel->state;
}

LVM_API int lvm_init(lvm_kernel_t* kernel);
LVM_API void lvm_close(lvm_kernel_t* kernel);
LVM_API int lvm_run(lvm_kernel_t* kernel, int nargs);
LVM_API void lvm_stop(lvm_kernel_t* kernel);
LVM_API int lvm_setenv(lua_State* L, int func_index, int env_index);
LVM_API int lvm_register_module(lvm_kernel_t* kernel, lvm_module_t* module);
LVM_API int lvm_unregister_module(lvm_kernel_t* kernel, lvm_module_t* module);

LVM_END_DECLS

#endif /* LVM_KERNEL_H */
