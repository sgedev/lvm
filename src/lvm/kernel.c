/*
 *
 */
#define LUA_LIB

#include <string.h>

#include <lvm/kernel.h>
#include <lvm/luautils.h>

#include <lualib.h>
#include <lauxlib.h>

static void lvm_wake_waiters(lvm_task_t* target) {
    lvm_list_node_t* node;
    lvm_task_t* waiter;
    lua_State* target_state = lvm_task_to_state(target);
    lua_State* waiter_state;

    while (!lvm_list_is_empty(&target->wait_list)) {
        node = lvm_list_remove_first(&target->wait_list);
        waiter = LVM_MEMBEROF(node, lvm_task_t, node);
        waiter_state = lvm_task_to_state(waiter);
        waiter->waiting_on = NULL;
        uv_timer_stop(&waiter->sleep_timer);

        if (target->status == LUA_OK) {
            lua_pushinteger(waiter_state, LUA_OK);
            luaU_copyvalues(target_state, waiter_state, target->nresults);
            waiter->nargs = target->nresults + 1;
        } else {
            lua_pushinteger(waiter_state, target->status);
            luaU_copyvalues(target_state, waiter_state, 1);
            waiter->nargs = 2;
        }

        lvm_task_ready(waiter);
    }
}

static void lvm_sys_sleep_cb(uv_timer_t* p) {
    lvm_task_t* task = LVM_MEMBEROF(p, lvm_task_t, sleep_timer);
    task->nargs = 0;
    lvm_task_ready(task);
}

static int lvm_sys_sleep(lua_State* T) {
    int ret;
    int ms = luaL_checkinteger(T, 1);
    lvm_task_t* task = lvm_task_from_state(T);
    if (ms > 0) {
        ret = uv_timer_start(&task->sleep_timer, lvm_sys_sleep_cb, ms, 0);
        if (ret < 0) {
            luaL_error(T, "Failed to start sleep timer.");
        }
        return lua_yield(T, 0);
    }
    return 0;
}

LVM_API int lvm_setenv(lua_State* L, int func_index, int env_index) {
    const char* name;
    int i;
    int match;

    LVM_ASSERT(L != NULL);

    func_index = lua_absindex(L, func_index);
    env_index = lua_absindex(L, env_index);
    if (!lua_isfunction(L, func_index) || lua_iscfunction(L, func_index) || !lua_istable(L, env_index)) {
        return LVM_EPARAM;
    }

    for (i = 1; (name = lua_getupvalue(L, func_index, i)) != NULL; ++i) {
        match = (strcmp(name, "_ENV") == 0);
        lua_pop(L, 1);
        if (match) {
            lua_pushvalue(L, env_index);
            lua_setupvalue(L, func_index, i);
            return LVM_OK;
        }
    }

    return LVM_ESTATE;
}

static int lvm_sys_spawn(lua_State* T) {
    lua_Integer priority = -1;
    const char* name = NULL;
    lvm_task_t* task;
    lua_State* NT;
    int func_index = 1;
    int nargs;

    if (lua_isinteger(T, func_index)) {
        priority = lua_tointeger(T, func_index);
        if (priority < 0 || priority >= LVM_TASK_PRIORITY_MAX) {
            return luaL_argerror(T, func_index, "priority out of range");
        }
        ++func_index;
    }
    if (lua_type(T, func_index) == LUA_TSTRING) {
        name = lua_tostring(T, func_index);
        ++func_index;
    }

    luaL_checkany(T, func_index);
    nargs = lua_gettop(T) - func_index;
    NT = lua_newthread(T);
    task = lvm_task_from_state(NT);
    if (name != NULL) {
        snprintf(task->name, sizeof(task->name), "%s", name);
    }
    lua_insert(T, func_index);
    lua_xmove(T, NT, nargs + 1);
    while (--func_index > 0) {
        lua_remove(T, 1);
    }
    task->nargs = nargs;

    if (priority >= 0 && task->priority != (unsigned int)priority) {
        lvm_list_node_unlink(&task->node);
        task->priority = (unsigned int)priority;
        lvm_task_ready(task);
    }

    return 1;
}

static int lvm_sys_wait_continue(lua_State* T, int status, lua_KContext context) {
    int original_nargs = (int)context;
    int resume_nargs = lua_gettop(T) - original_nargs;
    int wait_status;

    (void)status;
    if (resume_nargs < 1) {
        return luaL_error(T, "wait resumed without a result.");
    }

    wait_status = (int)lua_tointeger(T, original_nargs + 1);
    if (wait_status == LUA_OK) {
        return resume_nargs - 1;
    }
    if (wait_status == LUA_YIELD) {
        return 2;
    }
    if (wait_status >= LUA_ERRRUN) {
        return lua_error(T);
    }
    return luaL_error(T, "invalid wait result.");
}

static void lvm_sys_wait_timeout(uv_timer_t* p) {
    lvm_task_t* waiter = LVM_MEMBEROF(p, lvm_task_t, sleep_timer);
    lua_State* T;

    if (waiter->waiting_on == NULL) {
        return;
    }

    lvm_list_node_unlink(&waiter->node);
    waiter->waiting_on = NULL;

    T = lvm_task_to_state(waiter);
    lua_pushinteger(T, LUA_YIELD);
    lua_pushnil(T);
    lua_pushliteral(T, "timeout");
    waiter->nargs = 3;
    lvm_task_ready(waiter);
}

static int lvm_sys_wait(lua_State* T) {
    lvm_task_t* waiter = lvm_task_from_state(T);
    lvm_task_t* target;
    lua_State* NT;
    lua_Integer timeout;
    int nargs = lua_gettop(T);
    int ret;

    luaL_checktype(T, 1, LUA_TTHREAD);
    timeout = luaL_optinteger(T, 2, -1);
    if (timeout < -1) {
        return luaL_argerror(T, 2, "timeout must be non-negative");
    }

    NT = lua_tothread(T, 1);
    target = lvm_task_from_state(NT);
    if (target == waiter) {
        return luaL_argerror(T, 1, "task cannot wait for itself");
    }
    if (target->kernel != waiter->kernel) {
        return luaL_argerror(T, 1, "task belongs to another kernel");
    }

    if (target->status == LUA_OK) {
        luaU_copyvalues(NT, T, target->nresults);
        return target->nresults;
    }

    if (target->status != -1 && target->status != LUA_YIELD) {
        luaU_copyvalues(NT, T, 1);
        return lua_error(T);
    }

    if (timeout == 0) {
        lua_pushnil(T);
        lua_pushliteral(T, "timeout");
        return 2;
    }

    waiter->waiting_on = target;
    lvm_list_append(&target->wait_list, &waiter->node);

    if (timeout > 0) {
        ret = uv_timer_start(&waiter->sleep_timer, lvm_sys_wait_timeout, timeout, 0);
        if (ret < 0) {
            lvm_list_node_unlink(&waiter->node);
            waiter->waiting_on = NULL;
            return luaL_error(T, "failed to start wait timer: %s", uv_strerror(ret));
        }
    }

    return lua_yieldk(T, 0, (lua_KContext)nargs, lvm_sys_wait_continue);
}

static void lvm_init_syscalls(lua_State* L) {
    lua_pushcfunction(L, lvm_sys_sleep);
    lua_setglobal(L, "sleep");

    lua_pushcfunction(L, lvm_sys_spawn);
    lua_setglobal(L, "spawn");

    lua_pushcfunction(L, lvm_sys_wait);
    lua_setglobal(L, "wait");
}

static void lvm_schedule(uv_prepare_t* p) {
    lvm_task_t* entry = p->data;
    lvm_kernel_t* kernel = entry->kernel;
    lvm_list_node_t* node;
    lvm_task_t* task;
    lua_State* T;
    unsigned int priority;
    int nresults;
    int status;

    for (;;) {
        node = NULL;
        for (priority = 0; priority < LVM_TASK_PRIORITY_MAX; ++priority) {
            if (!lvm_list_is_empty(&kernel->task_list[priority])) {
                node = lvm_list_remove_first(&kernel->task_list[priority]);
                break;
            }
        }
        if (node == NULL) {
            break;
        }

        task = LVM_MEMBEROF(node, lvm_task_t, node);
        T = lvm_task_to_state(task);

        nresults = 0;
        status = lua_resume(T, kernel->state, task->nargs, &nresults);
        task->nargs = 0;
        task->nresults = nresults;
        task->status = status;

        if (status != LUA_YIELD) {
            lvm_wake_waiters(task);
        }

        if (task == entry && status != LUA_YIELD) {
            uv_stop(&kernel->loop);
            break;
        }
    }
}

static void lvm_async_quit(uv_async_t* p) {
    uv_stop(p->loop);
}

static int lvm_pmain(lua_State* L) {
    int ret;
    int nargs;
    lua_State* T;
    lvm_task_t* task;
    lvm_kernel_t* kernel = lvm_from_state(L);

    if (!lua_isfunction(L, 1)) {
        return luaL_error(L, "application entry is not a function.");
    }

    nargs = lua_gettop(L) - 1;

    luaL_openlibs(L);
    lvm_init_syscalls(L);

    T = lua_newthread(L);
    lua_insert(L, 1);
    lua_xmove(L, T, nargs + 1);
    task = lvm_task_from_state(T);
    task->nargs = nargs;

    kernel->scheduler.data = task;
    ret = uv_prepare_start(&kernel->scheduler, lvm_schedule);
    if (ret < 0) {
        kernel->scheduler.data = NULL;
        return luaL_error(L, "failed to start scheduler.");
    }

    uv_run(&kernel->loop, UV_RUN_DEFAULT);
    uv_prepare_stop(&kernel->scheduler);
    kernel->scheduler.data = NULL;

    if (task->status != LUA_OK) {
        lua_xmove(T, L, 1);
        return lua_error(L);
    }

    return 0;
}

LVM_API int lvm_init(lvm_kernel_t* kernel) {
    LVM_ASSERT(kernel != NULL);

    unsigned int priority;
    int ret = uv_loop_init(&kernel->loop);
    if (ret < 0) {
        goto bad0;
    }

    ret = uv_async_init(&kernel->loop, &kernel->stopper, lvm_async_quit);
    if (ret < 0) {
        goto bad1;
    }
    kernel->stopper.data = kernel;

    ret = uv_prepare_init(&kernel->loop, &kernel->scheduler);
    if (ret < 0) {
        goto bad2;
    }
    kernel->scheduler.data = NULL;

    for (priority = 0; priority < LVM_TASK_PRIORITY_MAX; ++priority) {
        lvm_list_reset(&kernel->task_list[priority]);
    }

    lvm_list_reset(&kernel->module_list);

    kernel->state = luaL_newstate();
    if (kernel->state == NULL) {
        goto bad3;
    }

    lvm_task_t* init = lvm_task_from_state(kernel->state);
    init->kernel = kernel;
    init->waiting_on = NULL;
    snprintf(init->name, sizeof(init->name), "main");
    init->priority = 0;
    init->nargs = 0;
    init->nresults = 0;
    init->status = LUA_OK;
    lvm_list_node_reset(&init->node);
    lvm_list_reset(&init->wait_list);

    return 0;

//bad4:
//    lua_close(kernel->state);
//    kernel->state = NULL;

bad3:
    uv_close((uv_handle_t*)&kernel->scheduler, NULL);

bad2:
    uv_close((uv_handle_t*)&kernel->stopper, NULL);
    uv_run(&kernel->loop, UV_RUN_DEFAULT);

bad1:
    uv_loop_close(&kernel->loop);

bad0:
    return -1;
}

LVM_API void lvm_close(lvm_kernel_t* kernel) {
    LVM_ASSERT(kernel != NULL);

    lua_close(kernel->state);
    kernel->state = NULL;

    uv_close((uv_handle_t*)&kernel->scheduler, NULL);
    uv_close((uv_handle_t*)&kernel->stopper, NULL);

    uv_run(&kernel->loop, UV_RUN_DEFAULT);
    uv_loop_close(&kernel->loop);
}

LVM_API int lvm_run(lvm_kernel_t* kernel, int nargs) {
    lua_State* L;
    int func_index;

    LVM_ASSERT(kernel != NULL);
    LVM_ASSERT(kernel->state != NULL);

    L = kernel->state;

    if (nargs < 0 || lua_gettop(L) < nargs + 1) {
        lua_pushliteral(L, "not enough values for application call.");
        return LUA_ERRRUN;
    }

    func_index = lua_gettop(L) - nargs;
    if (!lua_isfunction(L, func_index)) {
        lua_settop(L, func_index - 1);
        lua_pushliteral(L, "application entry is not a function.");
        return LUA_ERRRUN;
    }

    lua_pushcfunction(L, lvm_pmain);
    lua_insert(L, func_index);

    return lua_pcall(L, nargs + 1, 0, 0);
}

LVM_API void lvm_stop(lvm_kernel_t* kernel) {
    LVM_ASSERT(kernel != NULL);
    uv_async_send(&kernel->stopper);
}

LVM_API int lvm_register_module(lvm_kernel_t* kernel, lvm_module_t* mod) {
    LVM_ASSERT(kernel != NULL);
    LVM_ASSERT(mod != NULL);

    lvm_list_append(&kernel->module_list, &mod->node);

    return 0;
}

LVM_API int lvm_unregister_module(lvm_kernel_t* kernel, lvm_module_t* mod) {
    LVM_ASSERT(kernel != NULL);
    LVM_ASSERT(mod != NULL);

    lvm_list_remove(&kernel->module_list, &mod->node);

    return 0;
}
