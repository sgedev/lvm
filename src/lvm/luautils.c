/*
 *
 */
#include <lvm/luautils.h>

#include <lauxlib.h>

LVM_API void luaU_dumpstack(lua_State* L, FILE* stream) {
    int top = lua_gettop(L);
    int i;

    fprintf(stream, "Lua stack top = %d\n", top);
    for (i = 1; i <= top; ++i) {
        int type = lua_type(L, i);
        fprintf(stream, "[%d] %s", i, lua_typename(L, type));
        switch (type) {
        case LUA_TSTRING:
            fprintf(stream, " = \"%s\"", lua_tostring(L, i));
            break;
        case LUA_TBOOLEAN:
            fprintf(stream, " = %s", lua_toboolean(L, i) ? "true" : "false");
            break;
        case LUA_TNUMBER:
            fprintf(stream, " = %g", lua_tonumber(L, i));
            break;
        case LUA_TFUNCTION:
        case LUA_TTABLE:
        case LUA_TUSERDATA:
        case LUA_TTHREAD:
            fprintf(stream, " = %p", lua_topointer(L, i));
            break;
        }
        fprintf(stream, "\n");
    }
}

LVM_API void luaU_copyvalues(lua_State* from, lua_State* to, int n) {
    int first;
    int i;

    if (n <= 0) {
        return;
    }

    luaL_checkstack(from, n, "too many values to copy");
    luaL_checkstack(to, n, "too many values to copy");
    first = lua_gettop(from) - n + 1;
    for (i = 0; i < n; ++i) {
        lua_pushvalue(from, first + i);
    }
    lua_xmove(from, to, n);
}
