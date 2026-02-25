#ifdef CONF_LUA
#include "delulu.h"

#include <base/log.h>

#include <lua.hpp>

static int luacallback_foo(lua_State *L)
{
	int Arg = luaL_checkinteger(L, 1);
	log_info("lua", "got %d from plugin", Arg);
	lua_pushinteger(L, 420);
	return 1;
}

void ddnetpp_init_lua()
{
	log_info("ddnet++", "init lua...");
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	lua_register(L, "foo", luacallback_foo);
	if(luaL_dofile(L, "plugin.lua") != LUA_OK)
	{
		const char *pLuaError = lua_tostring(L, -1);
		log_error("lua", "%s", pLuaError);
		lua_pop(L, 1);
	}
	lua_close(L);
}
#endif
