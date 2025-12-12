#include <base/log.h>
#include <base/system.h>

#include <engine/server/server.h>
#include <engine/shared/config.h>

#include <game/server/entities/character.h>
#include <game/server/gamecontext.h>
#include <game/server/gamecontroller.h>
#include <game/server/player.h>

void CGameContext::RegisterDDNetPPCommands()
{
	// config chains
	Console()->Chain("sv_captcha_room", ConchainCaptchaRoom, this);
	Console()->Chain("sv_display_score", ConchainDisplayScore, this);
	Console()->Chain("sv_accounts", ConchainAccounts, this);
	Console()->Chain("sv_port", ConchainAccounts, this);
	Console()->Chain("sv_hostname", ConchainAccounts, this);

	// chat commands
#define CHAT_COMMAND(name, params, flags, callback, userdata, help) Console()->Register(name, params, flags, callback, userdata, help);
#include <game/server/ddnetpp/chat_commands.h>
#undef CHAT_COMMAND

	// rcon commands
#define CONSOLE_COMMAND(name, params, flags, callback, userdata, help) Console()->Register(name, params, flags, callback, userdata, help);
#include <game/server/ddnetpp/rcon_commands.h>
#undef CONSOLE_COMMAND
}

void CGameContext::ConchainAccounts(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	bool AccountsWereOn = g_Config.m_SvAccounts != 0;
	char aOldHostname[512];
	str_copy(aOldHostname, g_Config.m_SvHostname);

	pfnCallback(pResult, pCallbackUserData);

	if(pResult->NumArguments() == 0)
		return;

	// check disallow changing sv_hostname
	if(g_Config.m_SvAccounts && aOldHostname[0] != '\0' && g_Config.m_SvHostname[0] != '\0' && str_comp(aOldHostname, g_Config.m_SvHostname))
	{
		log_error("ddnet++", "changing sv_hostname is not allowed while sv_accounts is on");
		str_copy(g_Config.m_SvHostname, aOldHostname);
		return;
	}

	// check activate
	if(g_Config.m_SvAccounts == 0 && !AccountsWereOn && pSelf->m_LastAccountTurnOnAttempt)
	{
		bool PortAndHostSet = g_Config.m_SvPort != 0 && g_Config.m_SvHostname[0] != '\0';
		int SecondsSinceLastAttempt = (time_get() - pSelf->m_LastAccountTurnOnAttempt) / time_freq();
		if(PortAndHostSet && SecondsSinceLastAttempt < 10)
		{
			log_warn("ddnet++", "sv_accounts turned on because sv_port and sv_hostname are now set. Please set sv_accounts after sv_port and sv_hostname in your config.");
			g_Config.m_SvAccounts = 1;
		}
	}

	// check deactivate
	if(g_Config.m_SvAccounts)
	{
		if(g_Config.m_SvPort == 0)
		{
			log_error("ddnet++", "sv_accounts can not be turned on if sv_port is 0");
			g_Config.m_SvAccounts = 0;
		}
		if(g_Config.m_SvHostname[0] == '\0')
		{
			log_error("ddnet++", "sv_accounts can not be turned on if sv_hostname is unset");
			g_Config.m_SvAccounts = 0;
		}

		if(g_Config.m_SvAccounts == 0)
		{
			pSelf->m_LastAccountTurnOnAttempt = time_get();
		}
	}

	// on deactivate
	if(!g_Config.m_SvAccounts && pSelf->m_pController && AccountsWereOn)
	{
		log_info("ddnet++", "logging out all players ...");
		pSelf->m_pController->LogoutAllAccounts();
	}

	// on activate
	if(g_Config.m_SvAccounts && pSelf->m_pController && !AccountsWereOn)
	{
		pSelf->m_pAccounts->CreateDatabase();
		char aBuf[512];
		str_copy(aBuf,
			"UPDATE Accounts SET IsLoggedIn = 0 WHERE IsLoggedIn = 1 AND server_ip = ? AND LastLoginPort = ?;",
			sizeof(aBuf));
		pSelf->m_pAccounts->CleanZombieAccounts(-1, g_Config.m_SvPort, aBuf);
	}
}
