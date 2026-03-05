#ifdef CONF_LUA
#include "stack_checker.h"

#include <base/dbg.h>
#include <base/str.h>

extern "C" {
#include <lua.h>
}

CLuaStackChecker::CLuaStackChecker(lua_State *L)
{
	m_pLuaState = L;
	m_StackTop = lua_gettop(L);
}

CLuaStackChecker::CLuaStackChecker(lua_State *L, const char *pFile, int Line) :
	CLuaStackChecker(L)
{
	str_copy(m_aFile, pFile);
	m_Line = Line;
}

CLuaStackChecker::~CLuaStackChecker()
{
	if(m_StackTop == lua_gettop(m_pLuaState))
		return;

	if(m_aFile[0])
		dbg_assert_imp(m_aFile, m_Line, "lua stack corrupted");
	else
		dbg_assert_failed("lua stack corrupted");
}
#endif
