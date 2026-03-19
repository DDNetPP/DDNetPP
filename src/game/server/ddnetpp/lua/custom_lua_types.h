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

// Push a vec2 as lua table with x and y on the lua stack
void LuaPushVec2(lua_State *L, vec2 Vec);

// Push a vec2 server pos as table onto the lua stack
// it will translate it to friendly lua coordinates unit
void LuaPushPosition(lua_State *L, vec2 ServerPos);

float LuaCheckCoordinate(lua_State *L, int Index);

CPlayer *LuaCheckPlayer(lua_State *L, int Index);
CCharacter *LuaCheckCharacter(lua_State *L, int Index);

// Implements the flexible client identifier type defined in luacats like this
// ---@alias ClientId integer|Character|Player
bool LuaIsClientId(lua_State *L, int Index);

// Implements the flexible client identifier type defined in luacats like this
// ---@alias ClientId integer|Character|Player
int LuaCheckClientId(lua_State *L, int Index);

// Read a lua given position consisting of two coordinates
// from the stack at given index.
// The position can be one table or two integers.
// Writes the result to Pos as already translated coordinate.
//
// returns the amount of elements consumed
// either 1 for a {x, y} table or 2 for two positional arguments x and y
int LuaCheckPosOrXandY(lua_State *L, int Index, ivec2 &OutPos);

// Call this from your C++ callback to check arguments passed by lua.
// Expects a lua table with x and y at Index and returns it as a float vec2
// Or throws an argument error
vec2 LuaCheckArgVec2(lua_State *L, int Index, const char *pTableName);

// Call this from your C++ callback to check arguments passed by lua.
// Expects a lua position at Index and returns the translated server position
// Or throws an argument error
vec2 LuaCheckArgPosition(lua_State *L, int Index);

const char *LuaCheckArgStringStrict(lua_State *L, int Index);

#endif
#endif
