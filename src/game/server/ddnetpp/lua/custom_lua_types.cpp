#ifdef CONF_LUA

#include <engine/shared/protocol.h>

#include <game/server/ddnetpp/lua/custom_lua_types.h>
#include <game/server/ddnetpp/lua/lua_plugin.h>
#include <game/server/ddnetpp/lua/position.h>
#include <game/server/ddnetpp/lua/table_unpacker.h>
#include <game/server/entities/character.h>
#include <game/server/gamecontext.h>
#include <game/server/player.h>

extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

void LuaPushVec2(lua_State *L, vec2 Vec)
{
	lua_newtable(L);

	lua_pushstring(L, "x");
	lua_pushnumber(L, Vec.x);
	lua_settable(L, -3);

	lua_pushstring(L, "y");
	lua_pushnumber(L, Vec.y);
	lua_settable(L, -3);
}

void LuaPushPosition(lua_State *L, vec2 ServerPos)
{
	LuaPushVec2(L, ServerPosToLua(ServerPos));
}

float LuaCheckCoordinate(lua_State *L, int Index)
{
	if(lua_isnumber(L, Index))
		return luaL_checknumber(L, Index) * 32;
	return luaL_checkinteger(L, Index) * 32;
}

CPlayer *LuaCheckPlayer(lua_State *L, int Index)
{
	// int Type = lua_type(L, Index);
	// if(Type != LUA_TUSERDATA)
	// {
	// 	char aError[512];
	// 	str_format(aError, sizeof(aError), "Player expected, got %s", luaL_typename(L, Index));
	// 	luaL_argerror(L, Index, aError);
	// 	return nullptr;
	// }

	auto *pPlayerHandle = static_cast<CLuaPlayerHandle *>(luaL_checkudata(L, Index, "Player"));
	if(!pPlayerHandle)
		return nullptr;
	CPlayer *pPlayer = pPlayerHandle->m_pPlugin->Game()->GameServer()->GetPlayerByUniqueId(pPlayerHandle->m_UniqueClientId);
	if(!pPlayer)
	{
		luaL_error(L, "invalid Player");
		return nullptr;
	}
	return pPlayer;
}

CCharacter *LuaCheckCharacter(lua_State *L, int Index)
{
	auto *pPlayerHandle = static_cast<CLuaPlayerHandle *>(luaL_checkudata(L, Index, "Character"));
	if(!pPlayerHandle)
		return nullptr;
	CPlayer *pPlayer = pPlayerHandle->m_pPlugin->Game()->GameServer()->GetPlayerByUniqueId(pPlayerHandle->m_UniqueClientId);
	if(!pPlayer)
	{
		// Is it weird to say "invalid Player"
		// when calling a character instance method?
		// Its technically the correct error but weird to expose that to the user.
		// Hmm...
		luaL_error(L, "invalid Player");
		return nullptr;
	}
	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
	{
		luaL_error(L, "invalid Character");
		return nullptr;
	}
	return pChr;
}

bool LuaIsClientId(lua_State *L, int Index)
{
	if(lua_isinteger(L, Index))
		return true;
	if(lua_isuserdata(L, Index))
	{
		if(luaL_testudata(L, Index, "Player"))
			return true;
		if(luaL_testudata(L, Index, "Character"))
			return true;
	}

	return false;
}

int LuaCheckClientId(lua_State *L, int Index)
{
	// check integer first to make it the fastest
	// without wasting clock cycles on more complex object checks
	if(lua_isinteger(L, Index))
		return lua_tointeger(L, Index);

	if(lua_isuserdata(L, Index))
	{
		if(luaL_testudata(L, Index, "Player"))
		{
			const CPlayer *pPlayer = LuaCheckPlayer(L, Index);
			if(pPlayer)
				return pPlayer->GetCid();
		}
		else if(luaL_testudata(L, Index, "Character"))
		{
			CCharacter *pChr = LuaCheckCharacter(L, Index);
			if(pChr)
				return pChr->GetPlayer()->GetCid();
		}
	}

	// when getting an invalid type say we want an integer because thats the fastes
	// even if we accept character and player too
	return luaL_checkinteger(L, Index);
}

int LuaCheckPosOrXandY(lua_State *L, int Index, ivec2 &OutPos)
{
	if(lua_isinteger(L, Index))
	{
		OutPos.x = LuaCheckCoordinate(L, Index);
		OutPos.y = LuaCheckCoordinate(L, Index + 1);
		return 2;
	}

	CTableUnpacker Unpacker(L, Index, "pos");
	OutPos.x = Unpacker.GetCoordinate("x");
	OutPos.y = Unpacker.GetCoordinate("y");
	return 1;
}

int LuaCheckVec2OrXandY(lua_State *L, int Index, vec2 &OutVec, const char *pTableName)
{
	if(lua_isnumber(L, Index))
	{
		OutVec.x = luaL_checknumber(L, Index);
		OutVec.y = luaL_checknumber(L, Index + 1);
		return 2;
	}

	CTableUnpacker Unpacker(L, Index, pTableName);
	OutVec.x = Unpacker.GetFloat("x");
	OutVec.y = Unpacker.GetFloat("y");
	return 1;
}

CClientMask LuaCheckArgClientMask(lua_State *L, int Index)
{
	// LUA_CHECK_STACK(L);
	CClientMask Mask;
	char aError[512];
	if(lua_type(L, Index) != LUA_TTABLE)
	{
		str_format(aError, sizeof(aError), "mask table expected, got %s", luaL_typename(L, Index));
		luaL_argerror(L, Index, aError);
		return Mask;
	}

	// TODO: go as fast as possible here
	//       if there are only a few keys set in the table iterate all keys
	//       if there are many keys set possibly even more than MAX_CLIENTS or close to MAX_CLIENTS
	//       then iterate MAX_CLIENTS instead of all keys
	//       ---
	//       or should i not try to optimize code without benchmarks
	//       without fully understanding it? xd
	//       i feel like rawgeti should be fast enough to always iterate 128 times here

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		lua_rawgeti(L, Index, i);
		if(!lua_isnil(L, -1))
			Mask.set(i);
		// TODO: i feel like it might be faster to checkstack first
		//       and then pop all 128 after the loop
		lua_pop(L, 1);
	}
	// lua_pop(L, MAX_CLIENTS);

	return Mask;
}

vec2 LuaCheckArgVec2(lua_State *L, int Index, const char *pTableName)
{
	char aError[512];
	if(lua_type(L, Index) != LUA_TTABLE)
	{
		str_format(aError, sizeof(aError), "%s table expected, got %s", pTableName, luaL_typename(L, Index));
		luaL_argerror(L, Index, aError);
		return vec2(0.0f, 0.0f);
	}

	vec2 Pos;
	CTableUnpacker Unpacker(L, Index, pTableName);

	auto CoordInt = Unpacker.GetIntOptional("x");
	if(CoordInt.has_value())
	{
		Pos.x = CoordInt.value();
	}
	else
	{
		auto CoordFloat = Unpacker.GetFloatOptional("x");
		if(!CoordFloat.has_value())
		{
			str_format(aError, sizeof(aError), "in table %s missing key 'x'", pTableName);
			luaL_argerror(L, Index, aError);
			return vec2(0.0f, 0.0f);
		}
		Pos.x = CoordFloat.value();
	}

	CoordInt = Unpacker.GetIntOptional("y");
	if(CoordInt.has_value())
	{
		Pos.y = CoordInt.value();
	}
	else
	{
		auto CoordFloat = Unpacker.GetFloatOptional("y");
		if(!CoordFloat.has_value())
		{
			str_format(aError, sizeof(aError), "in table %s missing key 'y'", pTableName);
			luaL_argerror(L, Index, aError);
			return vec2(0.0f, 0.0f);
		}
		Pos.y = CoordFloat.value();
	}

	return Pos;
}

vec2 LuaCheckArgPosition(lua_State *L, int Index)
{
	char aError[512];
	if(lua_type(L, Index) != LUA_TTABLE)
	{
		str_format(aError, sizeof(aError), "pos table expected, got %s", luaL_typename(L, Index));
		luaL_argerror(L, Index, aError);
		return vec2(0.0f, 0.0f);
	}

	vec2 Pos;
	CTableUnpacker Unpacker(L, Index, "pos");

	auto Coord = Unpacker.GetCoordinateOptional("x");
	if(!Coord.has_value())
	{
		luaL_argerror(L, Index, "in table pos missing key 'x'");
		return vec2(0.0f, 0.0f);
	}
	Pos.x = Coord.value();

	Coord = Unpacker.GetCoordinateOptional("y");
	if(!Coord.has_value())
	{
		luaL_argerror(L, Index, "in table pos missing key 'y'");
		return vec2(0.0f, 0.0f);
	}
	Pos.y = Coord.value();

	return Pos;
}

const char *LuaCheckArgStringStrict(lua_State *L, int Index)
{
	int Type = lua_type(L, Index);
	if(Type == LUA_TSTRING)
		return luaL_checkstring(L, Index);

	char aError[512];
	str_format(aError, sizeof(aError), "string expected, got %s", luaL_typename(L, Index));
	luaL_argerror(L, Index, aError);

	// unreachable
	return "";
}

#endif
