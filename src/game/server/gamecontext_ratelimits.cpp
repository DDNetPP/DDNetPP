// gamecontext scoped quests ddnet++ methods

#include "gamecontext.h"

#include <base/system.h>

#include <engine/shared/config.h>

void CGameContext::IncrementWrongRconAttempts()
{
	m_WrongRconAttempts++;
}

void CGameContext::RegisterBanCheck(int ClientId)
{
	const NETADDR *pAddr = Server()->ClientAddr(ClientId);
	char aBuf[128];
	int Found = 0;
	int Regs = 0;
	// find a matching ban for this ip, update expiration time if found
	for(int i = 0; i < m_NumRegisterBans; i++)
	{
		if(net_addr_comp_noport(&m_aRegisterBans[i].m_Addr, pAddr) == 0)
		{
			m_aRegisterBans[i].m_LastAttempt = time_get();
			Regs = ++m_aRegisterBans[i].m_NumAttempts;
			Found = 1;
			break;
		}
	}

	if(!Found) // nothing found so far, find a free slot..
	{
		if(m_NumRegisterBans < MAX_REGISTER_BANS)
		{
			m_aRegisterBans[m_NumRegisterBans].m_LastAttempt = time_get();
			m_aRegisterBans[m_NumRegisterBans].m_Addr = *pAddr;
			Regs = m_aRegisterBans[m_NumRegisterBans].m_NumAttempts = 1;
			m_NumRegisterBans++;
			Found = 1;
		}
	}

	if(Regs >= g_Config.m_SvMaxRegisterPerIp)
	{
		RegisterBan(pAddr, 60 * 60 * 12, Server()->ClientName(ClientId));
	}
	if(Found)
	{
		str_format(aBuf, sizeof(aBuf), "ClientId=%d has registered %d/%d accounts.", ClientId, Regs, g_Config.m_SvMaxRegisterPerIp);
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "accounts", aBuf);
	}
	else // no free slot found
	{
		if(g_Config.m_SvRegisterHumanLevel >= 9)
			return;
		g_Config.m_SvRegisterHumanLevel++;
		str_format(aBuf, sizeof(aBuf), "ban array is full setting human level to %d", g_Config.m_SvRegisterHumanLevel);
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "accounts", aBuf);
	}
}

void CGameContext::RegisterBan(const NETADDR *pAddr, int Secs, const char *pDisplayName)
{
	char aBuf[128];
	int Found = 0;
	// find a matching ban for this ip, update expiration time if found
	for(int i = 0; i < m_NumRegisterBans; i++)
	{
		if(net_addr_comp_noport(&m_aRegisterBans[i].m_Addr, pAddr) == 0)
		{
			m_aRegisterBans[i].m_Expire = Server()->Tick() + Secs * Server()->TickSpeed();
			Found = 1;
			break;
		}
	}

	if(!Found) // nothing found so far, find a free slot..
	{
		if(m_NumRegisterBans < MAX_REGISTER_BANS)
		{
			m_aRegisterBans[m_NumRegisterBans].m_Addr = *pAddr;
			m_aRegisterBans[m_NumRegisterBans].m_Expire = Server()->Tick() + Secs * Server()->TickSpeed();
			m_NumRegisterBans++;
			Found = 1;
		}
	}
	if(Found)
	{
		str_format(aBuf, sizeof aBuf, "'%s' has been banned from register system for %d seconds.",
			pDisplayName, Secs);
		SendChat(-1, TEAM_ALL, aBuf);
	}
	else // no free slot found
	{
		if(g_Config.m_SvRegisterHumanLevel >= 9)
			return;
		g_Config.m_SvRegisterHumanLevel++;
		str_format(aBuf, sizeof(aBuf), "ban array is full setting human level to %d", g_Config.m_SvRegisterHumanLevel);
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "accounts", aBuf);
	}
}

void CGameContext::CheckDeleteLoginBanEntry(int ClientId)
{
	const NETADDR *pAddr = Server()->ClientAddr(ClientId);
	// find a matching ban for this ip, delete if expired
	for(int i = 0; i < m_NumLoginBans; i++)
	{
		if(net_addr_comp_noport(&m_aLoginBans[i].m_Addr, pAddr) == 0)
		{
			int64_t BanTime = (m_aLoginBans[i].m_Expire - Server()->Tick()) / Server()->TickSpeed();
			if(BanTime > 0)
				return;
			if(m_aLoginBans[i].m_LastAttempt + (time_freq() * LOGIN_BAN_DELAY) < time_get())
			{
				// TODO: be consistent with log types... sometimes its "bans", "mutes", "login_bans", "account" like wtf?
				dbg_msg("mutes", "delete login ban entry for player=%d:'%s' due to expiration", ClientId, Server()->ClientName(ClientId));
				m_aLoginBans[m_NumLoginBans].m_NumAttempts = 0;
				if(ClientId < 0 || ClientId >= m_NumLoginBans)
					return;

				m_NumLoginBans--;
				m_aLoginBans[ClientId] = m_aLoginBans[m_NumLoginBans];
				return;
			}
		}
	}
}

void CGameContext::CheckDeleteRegisterBanEntry(int ClientId)
{
	const NETADDR *pAddr = Server()->ClientAddr(ClientId);
	// find a matching ban for this ip, delete if expired
	for(int i = 0; i < m_NumRegisterBans; i++)
	{
		if(net_addr_comp_noport(&m_aRegisterBans[i].m_Addr, pAddr) == 0)
		{
			int64_t BanTime = (m_aRegisterBans[i].m_Expire - Server()->Tick()) / Server()->TickSpeed();
			if(BanTime > 0)
				return;
			if(m_aNameChangeMutes[i].m_LastAttempt + (time_freq() * REGISTER_BAN_DELAY) < time_get())
			{
				dbg_msg("mutes", "delete register ban entry for player=%d:'%s' due to expiration", ClientId, Server()->ClientName(ClientId));
				m_aRegisterBans[m_NumRegisterBans].m_NumAttempts = 0;
				if(ClientId < 0 || ClientId >= m_NumRegisterBans)
					return;

				m_NumRegisterBans--;
				m_aRegisterBans[ClientId] = m_aRegisterBans[m_NumRegisterBans];
				return;
			}
		}
	}
}

void CGameContext::CheckDeleteNamechangeMuteEntry(int ClientId)
{
	const NETADDR *pAddr = Server()->ClientAddr(ClientId);
	// find a matching ban for this ip, delete if expired
	for(int i = 0; i < m_NumNameChangeMutes; i++)
	{
		if(net_addr_comp_noport(&m_aNameChangeMutes[i].m_Addr, pAddr) == 0)
		{
			int64_t BanTime = (m_aNameChangeMutes[i].m_Expire - Server()->Tick()) / Server()->TickSpeed();
			if(BanTime > 0)
				return;
			if(m_aNameChangeMutes[i].m_LastAttempt + (time_freq() * NAMECHANGE_BAN_DELAY) < time_get())
			{
				dbg_msg("mutes", "delete namechange mute entry for player=%d:'%s' due to expiration", ClientId, Server()->ClientName(ClientId));
				m_aNameChangeMutes[m_NumNameChangeMutes].m_NumAttempts = 0;
				if(ClientId < 0 || ClientId >= m_NumNameChangeMutes)
					return;

				m_NumNameChangeMutes--;
				m_aNameChangeMutes[ClientId] = m_aNameChangeMutes[m_NumNameChangeMutes];
				return;
			}
		}
	}
}

void CGameContext::LoginBanCheck(int ClientId)
{
	const NETADDR *pAddr = Server()->ClientAddr(ClientId);
	char aBuf[128];
	int Found = 0;
	int atts = 0;
	int64_t BanTime = 0;
	// find a matching ban for this ip, update expiration time if found
	for(int i = 0; i < m_NumLoginBans; i++)
	{
		if(net_addr_comp_noport(&m_aLoginBans[i].m_Addr, pAddr) == 0)
		{
			BanTime = (m_aLoginBans[i].m_Expire - Server()->Tick()) / Server()->TickSpeed();
			m_aLoginBans[m_NumLoginBans].m_LastAttempt = time_get();
			atts = ++m_aLoginBans[i].m_NumAttempts;
			Found = 1;
			// dbg_msg("login", "found ClientId=%d with %d failed attempts.", ClientId, atts);
			break;
		}
	}

	if(!Found) // nothing found so far, find a free slot..
	{
		if(m_NumLoginBans < MAX_LOGIN_BANS)
		{
			m_aLoginBans[m_NumLoginBans].m_LastAttempt = time_get();
			m_aLoginBans[m_NumLoginBans].m_Expire = 0;
			m_aLoginBans[m_NumLoginBans].m_Addr = *pAddr;
			atts = m_aLoginBans[m_NumLoginBans].m_NumAttempts = 1;
			m_NumLoginBans++;
			Found = 1;
			// dbg_msg("login", "adding ClientId=%d with %d failed attempts.", ClientId, atts);
		}
	}

	if(atts >= g_Config.m_SvMaxLoginPerIp)
		LoginBan(pAddr, 60 * 60 * 12, Server()->ClientName(ClientId));
	else if(atts % 3 == 0 && BanTime < 60) // rate limit every 3 fails for 1 minute ( only if bantime is less than 1 min )
		LoginBan(pAddr, 60, Server()->ClientName(ClientId));

	if(Found)
	{
		str_format(aBuf, sizeof(aBuf), "ClientId=%d has %d/%d failed account login attempts.", ClientId, atts, g_Config.m_SvMaxLoginPerIp);
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "accounts", aBuf);
	}
	else // no free slot found
	{
		if(g_Config.m_SvLoginHumanLevel >= 9)
			return;
		g_Config.m_SvLoginHumanLevel++;
		str_format(aBuf, sizeof(aBuf), "ban array is full setting human level to %d", g_Config.m_SvLoginHumanLevel);
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "accounts", aBuf);
	}
}

void CGameContext::LoginBan(const NETADDR *pAddr, int Secs, const char *pDisplayName)
{
	char aBuf[128];
	int Found = 0;
	// find a matching ban for this ip, update expiration time if found
	for(int i = 0; i < m_NumLoginBans; i++)
	{
		if(net_addr_comp_noport(&m_aLoginBans[i].m_Addr, pAddr) == 0)
		{
			m_aLoginBans[i].m_Expire = Server()->Tick() + Secs * Server()->TickSpeed();
			Found = 1;
			break;
		}
	}

	if(!Found) // nothing found so far, find a free slot..
	{
		if(m_NumLoginBans < MAX_LOGIN_BANS)
		{
			m_aLoginBans[m_NumLoginBans].m_Addr = *pAddr;
			m_aLoginBans[m_NumLoginBans].m_Expire = Server()->Tick() + Secs * Server()->TickSpeed();
			m_NumLoginBans++;
			Found = 1;
		}
	}
	if(Found)
	{
		if(Secs == 0)
			str_format(aBuf, sizeof aBuf, "'%s' has been unbanned from login system.", pDisplayName);
		else
			str_format(aBuf, sizeof aBuf, "'%s' has been banned from login system for %d seconds.", pDisplayName, Secs);
		SendChat(-1, TEAM_ALL, aBuf);
	}
	else // no free slot found
	{
		if(g_Config.m_SvLoginHumanLevel >= 9)
			return;
		g_Config.m_SvLoginHumanLevel++;
		str_format(aBuf, sizeof(aBuf), "ban array is full setting human level to %d", g_Config.m_SvLoginHumanLevel);
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "accounts", aBuf);
	}
}

int64_t CGameContext::NameChangeMuteCheck(int ClientId)
{
	int64_t MuteTime = NameChangeMuteTime(ClientId);
	const NETADDR *pAddr = Server()->ClientAddr(ClientId);
	char aBuf[128];
	int Found = 0;
	int Changes = 0;
	static const int NAME_CHANGE_DELAY = 60 * 60; // reset name changes counter every hour
	// find a matching ban for this ip, update expiration time if found
	for(int i = 0; i < m_NumNameChangeMutes; i++)
	{
		if(net_addr_comp_noport(&m_aNameChangeMutes[i].m_Addr, pAddr) == 0)
		{
			if(m_aNameChangeMutes[i].m_LastAttempt + (time_freq() * NAME_CHANGE_DELAY) < time_get())
			{
				// dbg_msg("mutes", "name changes mute counter reset for player=%d:'%s'", ClientId, Server()->ClientName(ClientId));
				m_aNameChangeMutes[i].m_NumAttempts = 0;
			}
			Changes = ++m_aNameChangeMutes[i].m_NumAttempts;
			m_aNameChangeMutes[i].m_LastAttempt = time_get();
			Found = 1;
			break;
		}
	}

	if(!Found) // nothing found so far, find a free slot..
	{
		if(m_NumNameChangeMutes < MAX_REGISTER_BANS)
		{
			m_aNameChangeMutes[m_NumNameChangeMutes].m_Addr = *pAddr;
			Changes = m_aNameChangeMutes[m_NumNameChangeMutes].m_NumAttempts = 1;
			m_aNameChangeMutes[m_NumNameChangeMutes].m_LastAttempt = time_get();
			m_aNameChangeMutes[m_NumNameChangeMutes].m_Expire = 0;
			m_NumNameChangeMutes++;
			Found = 1;
		}
	}

	if(Changes >= g_Config.m_SvMaxNameChangesPerIp)
	{
		if(!MuteTime)
			NameChangeMute(pAddr, 60 * 60 * 12, Server()->ClientName(ClientId));
	}
	if(Found)
	{
		str_format(aBuf, sizeof(aBuf), "ClientId=%d has changed name %d/%d times.", ClientId, Changes, g_Config.m_SvMaxNameChangesPerIp);
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "mute", aBuf);
	}
	else // no free slot found
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "mute", "warning name change mute array is full!");
	}
	return MuteTime;
}

int64_t CGameContext::NameChangeMuteTime(int ClientId)
{
	const NETADDR *pAddr = Server()->ClientAddr(ClientId);
	for(int i = 0; i < m_NumNameChangeMutes; i++)
		if(!net_addr_comp_noport(pAddr, &m_aNameChangeMutes[i].m_Addr))
			return m_aNameChangeMutes[i].m_Expire ? (m_aNameChangeMutes[i].m_Expire - Server()->Tick()) / Server()->TickSpeed() : 0;
	return 0;
}

void CGameContext::NameChangeMute(const NETADDR *pAddr, int Secs, const char *pDisplayName)
{
	char aBuf[128];
	int Found = 0;
	// find a matching Mute for this ip, update expiration time if found
	for(int i = 0; i < m_NumNameChangeMutes; i++)
	{
		if(net_addr_comp_noport(&m_aNameChangeMutes[i].m_Addr, pAddr) == 0)
		{
			m_aNameChangeMutes[i].m_Expire = Server()->Tick() + Secs * Server()->TickSpeed();
			Found = 1;
		}
	}

	if(!Found) // nothing found so far, find a free slot..
	{
		if(m_NumNameChangeMutes < MAX_NAME_CHANGE_MUTES)
		{
			m_aNameChangeMutes[m_NumNameChangeMutes].m_Addr = *pAddr;
			m_aNameChangeMutes[m_NumNameChangeMutes].m_Expire = Server()->Tick() + Secs * Server()->TickSpeed();
			m_NumNameChangeMutes++;
			Found = 1;
		}
	}
	if(Found)
	{
		str_format(aBuf, sizeof aBuf, "'%s' has been name change muted for %d seconds.",
			pDisplayName, Secs);
		SendChat(-1, TEAM_ALL, aBuf);
	}
	else // no free slot found
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "mute", "name change mute array is full!");
	}
}
