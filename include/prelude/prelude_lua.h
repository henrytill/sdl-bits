#pragma once

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include "macro.h"

DEFINE_TRIVIAL_CLEANUP_FUNC(lua_State*, lua_close)
#define SCOPED_lua_State __attribute__((cleanup(lua_closep))) lua_State*
