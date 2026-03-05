#ifndef GAME_SERVER_DDNETPP_LUA_STACK_CHECKER_H
#define GAME_SERVER_DDNETPP_LUA_STACK_CHECKER_H

#ifdef CONF_LUA

extern "C" {
#include <lua.h>
}

class CLuaStackChecker
{
	lua_State *m_pLuaState = nullptr;
	int m_StackTop = -1;

	char m_aFile[512] = "";
	int m_Line = 0;

public:
	CLuaStackChecker(lua_State *L);
	CLuaStackChecker(lua_State *L, const char *pFile, int Line);
	~CLuaStackChecker();
};

#define LUA_CHECK_STACK(lua_state) CLuaStackChecker LuaStackChecker(lua_state, __FILE__, __LINE__)

#endif

#endif
