#ifndef GAME_SERVER_DDNETPP_LUA_POSITION_H
#define GAME_SERVER_DDNETPP_LUA_POSITION_H
#ifdef CONF_LUA

#include <base/vmath.h>

extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

// translate ugly server coordinates to human friendly lua coordinates
// the midde of the top left tile in server coordinates is 16, 16
// and in lua coordinates it is 0.5, 0.5
vec2 ServerPosToLua(vec2 ServerPos);
vec2 LuaPosToServer(vec2 ServerPos);

#endif
#endif
