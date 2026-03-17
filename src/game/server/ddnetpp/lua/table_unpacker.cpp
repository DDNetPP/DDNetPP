#ifdef CONF_LUA

#include <base/str.h>

#include <game/server/ddnetpp/lua/custom_lua_types.h>
#include <game/server/ddnetpp/lua/position.h>
#include <game/server/ddnetpp/lua/table_unpacker.h>

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

CTableUnpacker::CTableUnpacker(lua_State *L, int Index, const char *pTableName, const char *pSourceFilename, int SourceLineNumber)
// : m_LuaStackChecker(L, pSourceFilename, SourceLineNumber, pTableName)
{
	m_pLuaState = L;
	m_Index = Index;
	str_copy(m_aTableName, pTableName);
	str_copy(m_aSourceFilename, pSourceFilename);
	m_SourceLineNumber = SourceLineNumber;

	if(!lua_istable(LuaState(), m_Index))
	{
		Error("value is not a table");
	}
}

void CTableUnpacker::Error(const char *pReason)
{
	m_IsError = true;
	luaL_error(LuaState(), "error in table '%s': %s", m_aTableName, pReason);
}

int CTableUnpacker::GetIntOrFloat(const char *pKey)
{
	if(m_IsError)
		return 0;
	lua_getfield(LuaState(), m_Index, pKey);
	if(lua_isnoneornil(LuaState(), -1))
	{
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "missing key '%s'", pKey);
		Error(aBuf);
		return 0;
	}
	if(lua_isinteger(LuaState(), -1))
	{
		int Val = lua_tointeger(LuaState(), -1);
		lua_pop(LuaState(), 1);
		return Val;
	}
	if(lua_isnumber(LuaState(), -1))
	{
		int Val = (int)lua_tonumber(LuaState(), -1);
		lua_pop(LuaState(), 1);
		return Val;
	}
	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "expected key '%s' to hold an integer or number instead got %s", pKey, luaL_typename(LuaState(), -1));
	Error(aBuf);
	return 0;
}

int CTableUnpacker::GetInt(const char *pKey)
{
	if(m_IsError)
		return 0;
	lua_getfield(LuaState(), m_Index, pKey);
	if(lua_isnoneornil(LuaState(), -1))
	{
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "missing key '%s'", pKey);
		Error(aBuf);
		return 0;
	}
	if(!lua_isinteger(LuaState(), -1))
	{
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "expected key '%s' to hold an integer instead got %s", pKey, luaL_typename(LuaState(), -1));
		Error(aBuf);
		return 0;
	}
	int Val = lua_tointeger(LuaState(), -1);
	lua_pop(LuaState(), 1);
	return Val;
}

std::optional<bool> CTableUnpacker::GetBooleanOptional(const char *pKey)
{
	if(m_IsError)
		return std::nullopt;
	lua_getfield(LuaState(), m_Index, pKey);
	if(lua_isnoneornil(LuaState(), -1))
	{
		lua_pop(LuaState(), 1);
		return std::nullopt;
	}
	if(!lua_isboolean(LuaState(), -1))
	{
		lua_pop(LuaState(), 1);
		return std::nullopt;
	}
	bool Val = lua_toboolean(LuaState(), -1);
	lua_pop(LuaState(), 1);
	return Val;
}

std::optional<int> CTableUnpacker::GetIntOptional(const char *pKey)
{
	if(m_IsError)
		return std::nullopt;
	lua_getfield(LuaState(), m_Index, pKey);
	if(lua_isnoneornil(LuaState(), -1))
	{
		lua_pop(LuaState(), 1);
		return std::nullopt;
	}
	if(!lua_isinteger(LuaState(), -1))
	{
		lua_pop(LuaState(), 1);
		return std::nullopt;
	}
	int Val = lua_tointeger(LuaState(), -1);
	lua_pop(LuaState(), 1);
	return Val;
}

bool CTableUnpacker::GetStringOrFalse(const char *pKey, char *pValue, size_t ValueSize)
{
	pValue[0] = '\0';
	if(m_IsError)
		return false;
	lua_getfield(LuaState(), m_Index, pKey);
	if(lua_isnoneornil(LuaState(), -1))
	{
		lua_pop(LuaState(), 1);
		return false;
	}
	if(!lua_isstring(LuaState(), -1))
	{
		lua_pop(LuaState(), 1);
		return false;
	}
	str_copy(pValue, lua_tostring(LuaState(), -1), ValueSize);
	lua_pop(LuaState(), 1);
	return true;
}

int CTableUnpacker::GetIntOrDefault(const char *pKey, int Default)
{
	return GetIntOptional(pKey).value_or(Default);
}

float CTableUnpacker::GetFloat(const char *pKey)
{
	if(m_IsError)
		return 0.0f;
	lua_getfield(LuaState(), m_Index, pKey);
	if(lua_isnoneornil(LuaState(), -1))
	{
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "missing key '%s'", pKey);
		Error(aBuf);
		return 0.0f;
	}
	if(!lua_isnumber(LuaState(), -1))
	{
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "expected key '%s' to hold a number", pKey);
		Error(aBuf);
		return 0.0f;
	}
	float Val = lua_tonumber(LuaState(), -1);
	lua_pop(LuaState(), 1);
	return Val;
}

vec2 CTableUnpacker::GetVec2(const char *pKey)
{
	if(m_IsError)
		return vec2(0.0f, 0.0f);
	lua_getfield(LuaState(), m_Index, pKey);
	char aNestedName[512];
	str_format(aNestedName, sizeof(aNestedName), "%s.%s", m_aTableName, pKey);
	vec2 Pos = vec2(0.0f, 0.0f);

	// RAII scope
	{
		CTableUnpacker Unpacker(LuaState(), m_Index, aNestedName, m_aSourceFilename, m_SourceLineNumber);
		Pos.x = Unpacker.GetFloat("x");
		Pos.y = Unpacker.GetFloat("y");
		if(Unpacker.IsError())
		{
			Error("nested error");
			return vec2(0.0f, 0.0f);
		}
	}

	// pop nested table
	lua_pop(LuaState(), 1);
	return Pos;
}

vec2 CTableUnpacker::GetPosition(const char *pKey)
{
	return LuaPosToServer(GetVec2(pKey));
}

int CTableUnpacker::GetCoordinate(const char *pKey)
{
	if(m_IsError)
		return 0;
	lua_getfield(LuaState(), m_Index, pKey);
	if(lua_isnoneornil(LuaState(), -1))
	{
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "missing key '%s'", pKey);
		Error(aBuf);
		return 0;
	}
	if(!lua_isnumber(LuaState(), -1) && !lua_isinteger(LuaState(), -1))
	{
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "expected key '%s' to hold a number", pKey);
		Error(aBuf);
		return 0;
	}
	int Val = LuaCheckCoordinate(LuaState(), -1);
	lua_pop(LuaState(), 1);
	return Val;
}

#endif
