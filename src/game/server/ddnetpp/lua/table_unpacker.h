#ifndef GAME_SERVER_DDNETPP_LUA_TABLE_UNPACKER_H
#define GAME_SERVER_DDNETPP_LUA_TABLE_UNPACKER_H

#ifdef CONF_LUA

#include <base/vmath.h>

#include <game/server/ddnetpp/lua/stack_checker.h>

#include <optional>

extern "C" {
#include "lua.h"
}

class CTableUnpacker
{
	char m_aTableName[512] = "";
	char m_aSourceFilename[512] = "";
	int m_SourceLineNumber = -1;
	int m_Index;
	lua_State *m_pLuaState = nullptr;
	lua_State *LuaState() { return m_pLuaState; }
	bool m_IsError = false;

	// nice for debugging but it makes the unpacker annoying to use
	// because after you are done using the unpacker you need to
	// cleanup the scope before being able to use the lua api again
	// this just creates the need for useless scopes
	// so far the table unpacker seems to be correct so we no longer need
	// to assert this anyways we can now just assert it in the outer scope
	// CLuaStackChecker m_LuaStackChecker;

public:
	bool IsError() const { return m_IsError; }

	CTableUnpacker(lua_State *L, int Index, const char *pTableName, const char *pSourceFilename = "", int SourceLineNumber = -1);
	void Error(const char *pReason);
	int GetIntOrFloat(const char *pKey);
	int GetInt(const char *pKey);
	std::optional<bool> GetBooleanOptional(const char *pKey);
	std::optional<int> GetIntOptional(const char *pKey);
	// writes string to pValue or returns false if the string was not found
	bool GetStringOrFalse(const char *pKey, char *pValue, size_t ValueSize);
	int GetIntOrDefault(const char *pKey, int Default);
	float GetFloat(const char *pKey);
	vec2 GetVec2(const char *pKey);
	vec2 GetPosition(const char *pKey);
	int GetCoordinate(const char *pKey);
};

#endif

#endif
