/*
 *
 */
#ifndef LVM_LUAUTILS_H
#define LVM_LUAUTILS_H

#include <stdio.h>

#include <lua.h>

#include <lvm/common.h>

LVM_BEGIN_DECLS

LVM_API void luaU_dumpstack(lua_State* L, FILE* stream);
LVM_API void luaU_copyvalues(lua_State* from, lua_State* to, int n);

LVM_END_DECLS

#endif /* LVM_LUAUTILS_H */
