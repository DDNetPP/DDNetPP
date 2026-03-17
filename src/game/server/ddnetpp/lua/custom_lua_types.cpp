#ifdef CONF_LUA

#include <game/server/ddnetpp/lua/custom_lua_types.h>
#include <game/server/ddnetpp/lua/lua_plugin.h>
#include <game/server/ddnetpp/lua/table_unpacker.h>
#include <game/server/entities/character.h>
#include <game/server/gamecontext.h>
#include <game/server/player.h>

extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

int LuaCheckCoordinate(lua_State *L, int Index)
{
	if(lua_isinteger(L, Index))
		return luaL_checkinteger(L, Index) * 32;
	return ((int)luaL_checknumber(L, Index)) * 32;
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
		return 1;
	}

	CTableUnpacker Unpacker(L, Index, "pos");
	OutPos.x = Unpacker.GetCoordinate("x");
	OutPos.y = Unpacker.GetCoordinate("y");
	return 2;
}

#endif
