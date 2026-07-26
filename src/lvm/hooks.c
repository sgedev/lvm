/*
 *
 */
#include <stdio.h>

#include <lua.h>

#include <lvm/kernel.h>

void lvm_open_hook(lua_State* L) {
}

void lvm_close_hook(lua_State* L) {
}

void lvm_task_add_hook(lua_State* L, lua_State* T) {
    lvm_task_t* main = lvm_task_from_state(L);
    lvm_task_t* task = lvm_task_from_state(T);
    task->kernel = main->kernel;
    task->waiting_on = NULL;
    snprintf(task->name, sizeof(task->name), "task-%p", (void*)T);
    task->priority = main->priority;
    task->nargs = 0;
    task->nresults = 0;
    task->status = -1;
    lvm_list_reset(&task->wait_list);
    lvm_list_node_reset(&task->node);
    lvm_task_ready(task);
    uv_timer_init(&task->kernel->loop, &task->sleep_timer);
    task->sleep_timer.data = task;
}

void lvm_task_remove_hook(lua_State* L, lua_State* T) {
    lvm_task_t* task = lvm_task_from_state(T);
    uv_timer_stop(&task->sleep_timer);
    uv_close((uv_handle_t*)&task->sleep_timer, NULL);
    lvm_list_node_unlink(&task->node);
    // TODO wait_list
}

void lvm_task_resume_hook(lua_State* T, int n) {
}

void lvm_task_yield_hook(lua_State* T, int n) {
}
