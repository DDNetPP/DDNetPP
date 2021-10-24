// gamecontext scoped quests ddnet++ methods

#include <engine/shared/config.h>

#include "gamecontext.h"

void CGameContext::IncrementWrongRconAttempts()
{
	m_WrongRconAttempts++;
}

void CGameContext::RegisterBanCheck(int ClientID)
{
	NETADDR Addr;
	Server()->GetClientAddr(ClientID, &Addr);
	Addr.port = 0;
	char aBuf[128];
	int Found = 0;
	int regs = 0;
	// find a matching ban for this ip, update expiration time if found
	for(int i = 0; i < m_NumRegisterBans; i++)
	{
		if(net_addr_comp(&m_aRegisterBans[i].m_Addr, &Addr) == 0)
		{
			m_aRegisterBans[i].m_LastAttempt = time_get();
			regs = ++m_aRegisterBans[i].m_NumAttempts;
			Found = 1;
			break;
		}
	}

	if(!Found) // nothing found so far, find a free slot..
	{
		if(m_NumRegisterBans < MAX_REGISTER_BANS)
		{
			m_aRegisterBans[m_NumRegisterBans].m_LastAttempt = time_get();
			m_aRegisterBans[m_NumRegisterBans].m_Addr = Addr;
			regs = m_aRegisterBans[m_NumRegisterBans].m_NumAttempts = 1;
			m_NumRegisterBans++;
			Found = 1;
		}
	}

	if(regs >= g_Config.m_SvMaxRegisterPerIp)
	{
		RegisterBan(&Addr, 60 * 60 * 12, Server()->ClientName(ClientID));
	}
	if(Found)
	{
		str_format(aBuf, sizeof(aBuf), "ClientID=%d has registered %d/%d accounts.", ClientID, regs, g_Config.m_SvMaxRegisterPerIp);
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

void CGameContext::RegisterBan(NETADDR *Addr, int Secs, const char *pDisplayName)
{
	char aBuf[128];
	int Found = 0;
	NETADDR NoPortAddr = *Addr;
	NoPortAddr.port = 0;
	// find a matching ban for this ip, update expiration time if found
	for(int i = 0; i < m_NumRegisterBans; i++)
	{
		if(net_addr_comp(&m_aRegisterBans[i].m_Addr, &NoPortAddr) == 0)
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
			m_aRegisterBans[m_NumRegisterBans].m_Addr = NoPortAddr;
			m_aRegisterBans[m_NumRegisterBans].m_Expire = Server()->Tick() + Secs * Server()->TickSpeed();
			m_NumRegisterBans++;
			Found = 1;
		}
	}
	if(Found)
	{
		str_format(aBuf, sizeof aBuf, "'%s' has been banned from register system for %d seconds.",
			pDisplayName, Secs);
		SendChat(-1, CHAT_ALL, aBuf);
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

void CGameContext::CheckDeleteLoginBanEntry(int ClientID)
{
	NETADDR Addr;
	Server()->GetClientAddr(ClientID, &Addr);
	Addr.port = 0;
	// find a matching ban for this ip, delete if expired
	for(int i = 0; i < m_NumLoginBans; i++)
	{
		if(net_addr_comp(&m_aLoginBans[i].m_Addr, &Addr) == 0)
		{
			int64 BanTime = (m_aLoginBans[i].m_Expire - Server()->Tick()) / Server()->TickSpeed();
			if(BanTime > 0)
				return;
			if(m_aLoginBans[i].m_LastAttempt + (time_freq() * LOGIN_BAN_DELAY) < time_get())
			{
				// TODO: be consistent with log types... sometimes its "bans", "mutes", "login_bans", "account" like wtf?
				dbg_msg("mutes", "delete login ban entry for player=%d:'%s' due to expiration", ClientID, Server()->ClientName(ClientID));
				m_aLoginBans[m_NumLoginBans].m_NumAttempts = 0;
				if(ClientID < 0 || ClientID >= m_NumLoginBans)
					return;

				m_NumLoginBans--;
				m_aLoginBans[ClientID] = m_aLoginBans[m_NumLoginBans];
				return;
			}
		}
	}
}

void CGameContext::CheckDeleteRegisterBanEntry(int ClientID)
{
	NETADDR Addr;
	Server()->GetClientAddr(ClientID, &Addr);
	Addr.port = 0;
	// find a matching ban for this ip, delete if expired
	for(int i = 0; i < m_NumRegisterBans; i++)
	{
		if(net_addr_comp(&m_aRegisterBans[i].m_Addr, &Addr) == 0)
		{
			int64 BanTime = (m_aRegisterBans[i].m_Expire - Server()->Tick()) / Server()->TickSpeed();
			if(BanTime > 0)
				return;
			if(m_aNameChangeMutes[i].m_LastAttempt + (time_freq() * REGISTER_BAN_DELAY) < time_get())
			{
				dbg_msg("mutes", "delete register ban entry for player=%d:'%s' due to expiration", ClientID, Server()->ClientName(ClientID));
				m_aRegisterBans[m_NumRegisterBans].m_NumAttempts = 0;
				if(ClientID < 0 || ClientID >= m_NumRegisterBans)
					return;

				m_NumRegisterBans--;
				m_aRegisterBans[ClientID] = m_aRegisterBans[m_NumRegisterBans];
				return;
			}
		}
	}
}

void CGameContext::CheckDeleteNamechangeMuteEntry(int ClientID)
{
	NETADDR Addr;
	Server()->GetClientAddr(ClientID, &Addr);
	Addr.port = 0;
	// find a matching ban for this ip, delete if expired
	for(int i = 0; i < m_NumNameChangeMutes; i++)
	{
		if(net_addr_comp(&m_aNameChangeMutes[i].m_Addr, &Addr) == 0)
		{
			int64 BanTime = (m_aNameChangeMutes[i].m_Expire - Server()->Tick()) / Server()->TickSpeed();
			if(BanTime > 0)
				return;
			if(m_aNameChangeMutes[i].m_LastAttempt + (time_freq() * NAMECHANGE_BAN_DELAY) < time_get())
			{
				dbg_msg("mutes", "delete namechange mute entry for player=%d:'%s' due to expiration", ClientID, Server()->ClientName(ClientID));
				m_aNameChangeMutes[m_NumNameChangeMutes].m_NumAttempts = 0;
				if(ClientID < 0 || ClientID >= m_NumNameChangeMutes)
					return;

				m_NumNameChangeMutes--;
				m_aNameChangeMutes[ClientID] = m_aNameChangeMutes[m_NumNameChangeMutes];
				return;
			}
		}
	}
}

void CGameContext::LoginBanCheck(int ClientID)
{
	NETADDR Addr;
	Server()->GetClientAddr(ClientID, &Addr);
	Addr.port = 0;
	char aBuf[128];
	int Found = 0;
	int atts = 0;
	int64 BanTime = 0;
	// find a matching ban for this ip, update expiration time if found
	for(int i = 0; i < m_NumLoginBans; i++)
	{
		if(net_addr_comp(&m_aLoginBans[i].m_Addr, &Addr) == 0)
		{
			BanTime = (m_aLoginBans[i].m_Expire - Server()->Tick()) / Server()->TickSpeed();
			m_aLoginBans[m_NumLoginBans].m_LastAttempt = time_get();
			atts = ++m_aLoginBans[i].m_NumAttempts;
			Found = 1;
			// dbg_msg("login", "found ClientID=%d with %d failed attempts.", ClientID, atts);
			break;
		}
	}

	if(!Found) // nothing found so far, find a free slot..
	{
		if(m_NumLoginBans < MAX_LOGIN_BANS)
		{
			m_aLoginBans[m_NumLoginBans].m_LastAttempt = time_get();
			m_aLoginBans[m_NumLoginBans].m_Expire = 0;
			m_aLoginBans[m_NumLoginBans].m_Addr = Addr;
			atts = m_aLoginBans[m_NumLoginBans].m_NumAttempts = 1;
			m_NumLoginBans++;
			Found = 1;
			// dbg_msg("login", "adding ClientID=%d with %d failed attempts.", ClientID, atts);
		}
	}

	if(atts >= g_Config.m_SvMaxLoginPerIp)
		LoginBan(&Addr, 60 * 60 * 12, Server()->ClientName(ClientID));
	else if(atts % 3 == 0 && BanTime < 60) // rate limit every 3 fails for 1 minute ( only if bantime is less than 1 min )
		LoginBan(&Addr, 60, Server()->ClientName(ClientID));

	if(Found)
	{
		str_format(aBuf, sizeof(aBuf), "ClientID=%d has %d/%d failed account login attempts.", ClientID, atts, g_Config.m_SvMaxLoginPerIp);
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

void CGameContext::LoginBan(NETADDR *Addr, int Secs, const char *pDisplayName)
{
	char aBuf[128];
	int Found = 0;
	NETADDR NoPortAddr = *Addr;
	NoPortAddr.port = 0;
	// find a matching ban for this ip, update expiration time if found
	for(int i = 0; i < m_NumLoginBans; i++)
	{
		if(net_addr_comp(&m_aLoginBans[i].m_Addr, &NoPortAddr) == 0)
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
			m_aLoginBans[m_NumLoginBans].m_Addr = NoPortAddr;
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
		SendChat(-1, CHAT_ALL, aBuf);
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

int64 CGameContext::NameChangeMuteCheck(int ClientID)
{
	int64 muteTime = NameChangeMuteTime(ClientID);
	NETADDR Addr;
	Server()->GetClientAddr(ClientID, &Addr);
	Addr.port = 0;
	char aBuf[128];
	int Found = 0;
	int changes = 0;
	static const int NAME_CHANGE_DELAY = 60 * 60; // reset name changes counter every hour
	// find a matching ban for this ip, update expiration time if found
	for(int i = 0; i < m_NumNameChangeMutes; i++)
	{
		if(net_addr_comp(&m_aNameChangeMutes[i].m_Addr, &Addr) == 0)
		{
			if(m_aNameChangeMutes[i].m_LastAttempt + (time_freq() * NAME_CHANGE_DELAY) < time_get())
			{
				// dbg_msg("mutes", "name changes mute counter reset for player=%d:'%s'", ClientID, Server()->ClientName(ClientID));
				m_aNameChangeMutes[i].m_NumAttempts = 0;
			}
			changes = ++m_aNameChangeMutes[i].m_NumAttempts;
			m_aNameChangeMutes[i].m_LastAttempt = time_get();
			Found = 1;
			break;
		}
	}

	if(!Found) // nothing found so far, find a free slot..
	{
		if(m_NumNameChangeMutes < MAX_REGISTER_BANS)
		{
			m_aNameChangeMutes[m_NumNameChangeMutes].m_Addr = Addr;
			changes = m_aNameChangeMutes[m_NumNameChangeMutes].m_NumAttempts = 1;
			m_aNameChangeMutes[m_NumNameChangeMutes].m_LastAttempt = time_get();
			m_aNameChangeMutes[m_NumNameChangeMutes].m_Expire = 0;
			m_NumNameChangeMutes++;
			Found = 1;
		}
	}

	if(changes >= g_Config.m_SvMaxNameChangesPerIp)
	{
		if(!muteTime)
			NameChangeMute(&Addr, 60 * 60 * 12, Server()->ClientName(ClientID));
	}
	if(Found)
	{
		str_format(aBuf, sizeof(aBuf), "ClientID=%d has changed name %d/%d times.", ClientID, changes, g_Config.m_SvMaxNameChangesPerIp);
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "mute", aBuf);
	}
	else // no free slot found
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "mute", "warning name change mute array is full!");
	}
	return muteTime;
}

int64 CGameContext::NameChangeMuteTime(int ClientID)
{
	NETADDR Addr;
	Server()->GetClientAddr(ClientID, &Addr);
	Addr.port = 0;
	for(int i = 0; i < m_NumNameChangeMutes; i++)
		if(!net_addr_comp(&Addr, &m_aNameChangeMutes[i].m_Addr))
			return m_aNameChangeMutes[i].m_Expire ? (m_aNameChangeMutes[i].m_Expire - Server()->Tick()) / Server()->TickSpeed() : 0;
	return 0;
}

void CGameContext::NameChangeMute(NETADDR *Addr, int Secs, const char *pDisplayName)
{
	char aBuf[128];
	int Found = 0;
	NETADDR NoPortAddr = *Addr;
	NoPortAddr.port = 0;
	// find a matching Mute for this ip, update expiration time if found
	for(int i = 0; i < m_NumNameChangeMutes; i++)
	{
		if(net_addr_comp(&m_aNameChangeMutes[i].m_Addr, &NoPortAddr) == 0)
		{
			m_aNameChangeMutes[i].m_Expire = Server()->Tick() + Secs * Server()->TickSpeed();
			Found = 1;
		}
	}

	if(!Found) // nothing found so far, find a free slot..
	{
		if(m_NumNameChangeMutes < MAX_MUTES)
		{
			m_aNameChangeMutes[m_NumNameChangeMutes].m_Addr = NoPortAddr;
			m_aNameChangeMutes[m_NumNameChangeMutes].m_Expire = Server()->Tick() + Secs * Server()->TickSpeed();
			m_NumNameChangeMutes++;
			Found = 1;
		}
	}
	if(Found)
	{
		str_format(aBuf, sizeof aBuf, "'%s' has been name change muted for %d seconds.",
			pDisplayName, Secs);
		SendChat(-1, CHAT_ALL, aBuf);
	}
	else // no free slot found
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "mute", "name change mute array is full!");
	}
}