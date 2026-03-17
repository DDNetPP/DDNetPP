#ifdef CONF_LUA

#include <game/server/ddnetpp/lua/position.h>

extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

// translate ugly server coordinates to human friendly lua coordinates
// the midde of the top left tile in server coordinates is 16, 16
// and in lua coordinates it is 0.5, 0.5
vec2 ServerPosToLua(vec2 ServerPos)
{
	return vec2(
		ServerPos.x / 32.0f,
		ServerPos.y / 32.0f);
}

vec2 LuaPosToServer(vec2 ServerPos)
{
	return vec2(
		ServerPos.x * 32.0f,
		ServerPos.y * 32.0f);
}

int LuaCheckCoordinate(lua_State *L, int Index)
{
	if(lua_isinteger(L, Index))
		return luaL_checkinteger(L, Index) * 32;
	return ((int)luaL_checknumber(L, Index)) * 32;
}

#endif
