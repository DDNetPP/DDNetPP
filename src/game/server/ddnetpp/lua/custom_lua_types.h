#ifndef GAME_SERVER_DDNETPP_LUA_CUSTOM_LUA_TYPES_H
#define GAME_SERVER_DDNETPP_LUA_CUSTOM_LUA_TYPES_H
#ifdef CONF_LUA

#include <base/vmath.h>

class CPlayer;
class CCharacter;

extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

int LuaCheckCoordinate(lua_State *L, int Index);

CPlayer *LuaCheckPlayer(lua_State *L, int Index);
CCharacter *LuaCheckCharacter(lua_State *L, int Index);

// Implements the flexible client identifier type defined in luacats like this
// ---@alias ClientId integer|Character|Player
bool LuaIsClientId(lua_State *L, int Index);

// Implements the flexible client identifier type defined in luacats like this
// ---@alias ClientId integer|Character|Player
int LuaCheckClientId(lua_State *L, int Index);

#endif
#endif
