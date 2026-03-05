#ifdef CONF_LUA
#include "stack_checker.h"

#include <base/dbg.h>
#include <base/str.h>

#include <cstdlib>
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

	int Diff = m_StackTop - lua_gettop(m_pLuaState);
	char aError[512];
	if(Diff < 0)
	{
		str_format(
			aError,
			sizeof(aError),
			"lua stack corrupted (%d too many items on the stack)",
			std::abs(Diff));
	}
	else
	{
		str_format(
			aError,
			sizeof(aError),
			"lua stack corrupted (%d too few items on the stack)",
			Diff);
	}

	if(m_aFile[0])
		dbg_assert_imp(m_aFile, m_Line, "%s", aError);
	else
		dbg_assert_failed("%s", aError);
}
#endif
