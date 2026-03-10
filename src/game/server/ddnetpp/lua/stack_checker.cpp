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

CLuaStackChecker::CLuaStackChecker(lua_State *L, const char *pFile, int Line, const char *pDetail) :
	CLuaStackChecker(L, pFile, Line)
{
	str_copy(m_aDetail, pDetail);
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

	if(m_aDetail[0])
	{
		str_append(aError, " (");
		str_append(aError, m_aDetail);
		str_append(aError, ")");
	}

	if(m_aFile[0])
		dbg_assert_imp(m_aFile, m_Line, "%s", aError);
	else
		dbg_assert_failed("%s", aError);
}
#endif
