/*
 *
 */
#ifndef LVM_HOOKS_H
#define LVM_HOOKS_H

#include <lvm/common.h>
#include <lvm/list.h>

#undef LUA_EXTRASPACE
#undef luai_userstateopen
#undef luai_userstateclose
#undef luai_userstatethread
#undef luai_userstatefree
#undef luai_userstateresume
#undef luai_userstateyield

LVM_BEGIN_DECLS

typedef struct lvm_task {
    lvm_kernel_t* kernel;
    lvm_list_node_t node;
    lvm_list_t wait_list;
    struct lvm_task* waiting_on;
    uv_timer_t sleep_timer;
    char name[LVM_TASK_NAME_MAX];
    unsigned int priority;
    int nargs;
    int nresults;
    int status;
} lvm_task_t;

void lvm_open_hook(lua_State* L);
void lvm_close_hook(lua_State* L);
void lvm_task_add_hook(lua_State* L, lua_State* T);
void lvm_task_remove_hook(lua_State* L, lua_State* T);
void lvm_task_resume_hook(lua_State* T, int n);
void lvm_task_yield_hook(lua_State* T, int n);

LVM_END_DECLS

#define LUA_EXTRASPACE sizeof(lvm_task_t)
#define luai_userstateopen(L) lvm_open_hook(L)
#define luai_userstateclose(L) lvm_close_hook(L)
#define luai_userstatethread(L, T) lvm_task_add_hook((L), (T))
#define luai_userstatefree(L, T) lvm_task_remove_hook((L), (T))
#define luai_userstateresume(T, n) lvm_task_resume_hook((T), (n))
#define luai_userstateyield(T, n) lvm_task_yield_hook((T), (n))

#endif /* LVM_HOOKS_H */
