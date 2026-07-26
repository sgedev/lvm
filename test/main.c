/*
 *
 */
#include <stdio.h>
#include <stdlib.h>

#include <lvm/kernel.h>
#include <lauxlib.h>

int main(int argc, char* argv[]) {
    lvm_kernel_t lk;
    lua_State* L;
    int status;

    if (lvm_init(&lk) != LVM_OK) {
        return EXIT_FAILURE;
    }

    L = lvm_state(&lk);
    status = luaL_loadstring(L,
        "local task = spawn(function(a, b) return a + b end, 20, 22)\n"
        "assert(wait(task) == 42)\n"
        "assert(wait(spawn(0, 'answer', function() return 42 end)) == 42)\n"
        "local callable = setmetatable({}, {\n"
        "  __call = function(_, value) return value * 2 end\n"
        "})\n"
        "assert(wait(spawn(callable, 21)) == 42)\n"
        "local slow = spawn(function() sleep(20); return 7 end)\n"
        "local value, err = wait(slow, 1)\n"
        "assert(value == nil and err == 'timeout')\n"
        "assert(wait(slow) == 7)\n");
    if (status == LUA_OK) {
        status = lvm_run(&lk, 0);
    }
    if (status != LUA_OK) {
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        lvm_close(&lk);
        return EXIT_FAILURE;
    }

    lvm_close(&lk);

    return EXIT_SUCCESS;
}
