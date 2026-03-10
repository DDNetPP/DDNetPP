#ifndef GAME_SERVER_DDNETPP_LUA_LUA_RCON_COMMAND_H
#define GAME_SERVER_DDNETPP_LUA_LUA_RCON_COMMAND_H

#include <base/str.h>

#include <vector>

#ifdef CONF_LUA
extern "C" {
#include "lauxlib.h"
#include "lua.h"
}
#else
#define LUA_REFNIL -1
#endif

class CLuaRconCommand
{
	class CConstructorArgs
	{
	public:
		const char *m_pName = "";
		const char *m_pHelp = "";
		const char *m_pParams = "";
		int m_LuaCallbackRef = LUA_REFNIL;
	};

public:
	class CParam
	{
	public:
		enum EType
		{
			INT,
			STRING,
			REST,
			INVALID
		};

		EType m_Type = EType::STRING;
		char m_aName[512] = "";
		bool m_Optional = false;

		void Reset()
		{
			m_Optional = false;
			m_aName[0] = '\0';
			m_Type = EType::INVALID;
		}
	};

	std::vector<CParam> m_vParsedParams;

	CLuaRconCommand(const char *pName, const char *pHelp, const char *pParams, int LuaCallbackRef)
	{
		str_copy(m_aName, pName);
		str_copy(m_aHelp, pHelp);
		str_copy(m_aParams, pParams);
		m_LuaCallbackRef = LuaCallbackRef;
		ParseParameters(m_vParsedParams, pParams, nullptr, 0);
	}
	CLuaRconCommand(CConstructorArgs Args) :
		CLuaRconCommand(Args.m_pName, Args.m_pHelp, Args.m_pParams, Args.m_LuaCallbackRef)
	{
	}

	// returns true on success and writes the parsed params to &vResult
	// takes the console params string as input in the pParameters argument
	// that should be in the format of the teeworlds console parameter description
	// like those: "ss", "sssi" or "s[name]?i[seconds]"
	//
	// On error it writes to the pError buffer
	static bool ParseParameters(std::vector<CParam> &vResult, const char *pParameters, char *pError, int ErrorLen);

	// the name of the rcon command
	// that has to match the first word of the rcon line
	// that was executed
	char m_aName[128] = "";

	// helptext that will be shown to users in the console
	char m_aHelp[512] = "";

	// parameters described in the teeworlds console syntax
	// for example "sss" to take 3 unnamed non optional strings
	// or "s[name]?i[seconds]" to take the named non optional strinng argument "name"
	// followed by the optional integer argument "seconds"
	char m_aParams[512] = "";

	// The integer referencing the lua function to be called
	// when the rcon command is being executed
	// Stored in the LUA_REGISTRYINDEX
	//
	// You can push it onto the stack like this:
	//
	// ```C++
	// lua_rawgeti(LuaState(), LUA_REGISTRYINDEX, pCmd->m_LuaCallbackRef);
	// ```
	int m_LuaCallbackRef = LUA_REFNIL;

	const char *Name() const { return m_aName; }
	const char *Help() const { return m_aHelp; }
	const char *Params() const { return m_aParams; }
};

#endif
