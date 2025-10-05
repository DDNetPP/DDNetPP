// gamecontext scoped instagib ddnet++ methods

#include "instagib.h"

#include "../gamecontext.h"

#include <base/log.h>

#include <engine/shared/config.h>

#include <game/server/teams.h>

#include <cstring>

bool CInstagib::IsActive(int ClientId)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
	if(!pPlayer)
		return false;
	return pPlayer->m_IsInstaArena_idm || pPlayer->m_IsInstaArena_gdm;
}

void CInstagib::Leave(CPlayer *pPlayer)
{
	if(!pPlayer)
		return;

	GameServer()->LeaveInstagib(pPlayer->GetCid());
	m_aRestorePos[pPlayer->GetCid()] = true;
}

void CInstagib::Join(CPlayer *pPlayer, int Weapon, bool Fng)
{
	if(!pPlayer)
		return;

#if defined(CONF_DEBUG)
		//dbg_msg("cBug", "PLAYER '%s' Id=%d JOINED INSTAGIB WITH WEAPON = %d ANF FNG = %d", Server()->ClientName(Id), Id, weapon, fng);
#endif

	//die first to not count death
	if(pPlayer->GetCharacter())
	{
		SavePosition(pPlayer);
		pPlayer->GetCharacter()->Die(pPlayer->GetCid(), WEAPON_SELF);
	}

	//reset values
	pPlayer->m_HasInstaRoundEndPos = false;
	pPlayer->m_IsInstaArena_idm = false;
	pPlayer->m_IsInstaArena_gdm = false;
	pPlayer->m_IsInstaMode_idm = false;
	pPlayer->m_IsInstaMode_gdm = false;
	pPlayer->m_InstaScore = 0;

	pPlayer->m_IsInstaArena_fng = Fng;
	pPlayer->m_IsInstaMode_fng = Fng;
	if(Weapon == WEAPON_LASER)
	{
		pPlayer->m_IsInstaArena_idm = true;
		pPlayer->m_IsInstaMode_idm = true;
	}
	else if(Weapon == WEAPON_GRENADE)
	{
		pPlayer->m_IsInstaArena_gdm = true;
		pPlayer->m_IsInstaMode_gdm = true;
	}
	else
	{
		GameServer()->SendChatTarget(pPlayer->GetCid(), "[WARNING] Something went horrible wrong please report an admin");
		return;
	}

	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "[INSTA] '%s' joined the game.", Server()->ClientName(pPlayer->GetCid()));
	GameServer()->SendChatInsta(aBuf, Weapon);
}

bool CGameContext::CanJoinInstaArena(bool Grenade, bool PrivateMatch)
{
	int PlayerCount = 0;

	if(Grenade)
	{
		for(auto &Player : m_apPlayers)
		{
			if(Player)
			{
				if(Player->m_IsInstaArena_gdm)
				{
					PlayerCount++;
					if(Player->m_Insta1on1_id != -1) //if some1 is in 1on1
					{
						return false;
					}
				}
			}
		}

		if(PlayerCount >= g_Config.m_SvGrenadeArenaSlots)
		{
			return false;
		}
	}
	else //rifle
	{
		for(auto &Player : m_apPlayers)
		{
			if(Player)
			{
				if(Player->m_IsInstaArena_idm)
				{
					PlayerCount++;
					if(Player->m_Insta1on1_id != -1) //if some1 is in 1on1
					{
						return false;
					}
				}
			}
		}

		if(PlayerCount >= g_Config.m_SvRifleArenaSlots)
		{
			return false;
		}
	}

	return !PlayerCount || !PrivateMatch;
}

void CGameContext::WinInsta1on1(int WinnerId, int LooserId)
{
#if defined(CONF_DEBUG)
	if(!m_apPlayers[WinnerId])
		dbg_msg("cBug", "[WARNING] WinInsta1on1() at gamecontext.cpp");
#endif

	char aBuf[128];

	//WINNER
	if(m_apPlayers[WinnerId])
	{
		SendChatTarget(WinnerId, "==== Insta 1on1 WON ====");
		str_format(aBuf, sizeof(aBuf), "1. '%s' %d", Server()->ClientName(WinnerId), m_apPlayers[WinnerId]->m_Insta1on1_score);
		SendChatTarget(WinnerId, aBuf);
		str_format(aBuf, sizeof(aBuf), "2. '%s' %d", Server()->ClientName(LooserId), m_apPlayers[LooserId]->m_Insta1on1_score);
		SendChatTarget(WinnerId, aBuf);
		SendChatTarget(WinnerId, "==================");
		SendChatTarget(WinnerId, "+200 money for winning 1on1"); // actually it is only +100 because you have to pay to start an 1on1
		m_apPlayers[WinnerId]->MoneyTransaction(+200, "won insta 1on1");

		m_apPlayers[WinnerId]->m_IsInstaArena_gdm = false;
		m_apPlayers[WinnerId]->m_IsInstaArena_idm = false;
		m_apPlayers[WinnerId]->m_IsInstaArena_fng = false;
		m_apPlayers[WinnerId]->m_Insta1on1_id = -1;
		if(m_apPlayers[WinnerId]->GetCharacter())
		{
			m_apPlayers[WinnerId]->GetCharacter()->Die(WinnerId, WEAPON_SELF);
		}
	}

	//LOOSER
	if(LooserId != -1)
	{
		SendChatTarget(LooserId, "==== Insta 1on1 LOST ====");
		str_format(aBuf, sizeof(aBuf), "1. '%s' %d", Server()->ClientName(WinnerId), m_apPlayers[WinnerId]->m_Insta1on1_score);
		SendChatTarget(LooserId, aBuf);
		str_format(aBuf, sizeof(aBuf), "2. '%s' %d", Server()->ClientName(LooserId), m_apPlayers[LooserId]->m_Insta1on1_score);
		SendChatTarget(LooserId, aBuf);
		SendChatTarget(LooserId, "==================");

		m_apPlayers[LooserId]->m_IsInstaArena_gdm = false;
		m_apPlayers[LooserId]->m_IsInstaArena_idm = false;
		m_apPlayers[LooserId]->m_IsInstaArena_fng = false;
		m_apPlayers[LooserId]->m_Insta1on1_id = -1;
		m_apPlayers[LooserId]->m_Insta1on1_score = 0;
		if(m_apPlayers[LooserId]->GetCharacter())
		{
			m_apPlayers[LooserId]->GetCharacter()->Die(LooserId, WEAPON_SELF); //needed for /insta leave where the looser culd be alive
		}
	}

	//RESET SCORE LAST CUZ SCOREBOARD
	if(m_apPlayers[WinnerId])
		m_apPlayers[WinnerId]->m_Insta1on1_score = 0;
	if(m_apPlayers[LooserId])
		m_apPlayers[LooserId]->m_Insta1on1_score = 0;
}

void CGameContext::LeaveInstagib(int Id)
{
	CPlayer *pPlayer = m_apPlayers[Id];
	if(!pPlayer)
		return;
	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "[INSTA] '%s' left the game.", Server()->ClientName(Id));
	if(pPlayer->m_IsInstaArena_gdm)
	{
		SendChatInsta(aBuf, 4);
	}
	else if(pPlayer->m_IsInstaArena_idm)
	{
		SendChatInsta(aBuf, 5);
	}

	if((pPlayer->m_IsInstaArena_gdm || pPlayer->m_IsInstaArena_idm) && pPlayer->m_Insta1on1_id != -1)
	{
		WinInsta1on1(pPlayer->m_Insta1on1_id, Id);
		SendChatTarget(Id, "[INSTA] You left the 1on1.");
		SendBroadcast("", Id);
		return;
	}

	bool Left = true;

	if(pPlayer->m_IsInstaArena_fng)
	{
		if(pPlayer->m_IsInstaArena_gdm)
		{
			SendChatTarget(Id, "[INSTA] You left boomfng.");
		}
		else if(pPlayer->m_IsInstaArena_idm)
		{
			SendChatTarget(Id, "[INSTA] You left fng.");
		}
		else
		{
			Left = false;
		}
	}
	else
	{
		if(pPlayer->m_IsInstaArena_gdm)
		{
			SendChatTarget(Id, "[INSTA] You left grenade deathmatch.");
		}
		else if(pPlayer->m_IsInstaArena_idm)
		{
			SendChatTarget(Id, "[INSTA] You left rifle deathmatch.");
		}
		else
		{
			Left = false;
		}
	}

	if(Left)
	{
		pPlayer->m_IsInstaArena_gdm = false;
		pPlayer->m_IsInstaArena_idm = false;
		pPlayer->m_IsInstaArena_fng = false;
		pPlayer->m_IsInstaMode_gdm = false;
		pPlayer->m_IsInstaMode_idm = false;
		pPlayer->m_IsInstaMode_fng = false;
		if(pChr)
		{
			pChr->Die(pPlayer->GetCid(), WEAPON_SELF);
		}
		SendBroadcast("", Id); //clear score
	}
	else
	{
		SendChatTarget(Id, "[INSTA] You are not in a instagib game.");
	}
}

void CGameContext::SendChatInsta(const char *pMsg, int Weapon)
{
#if defined(CONF_DEBUG)
	//dbg_msg("cBug", "SendChatInsta got called with weapon %d and message '%s'", Weapon, pMsg);
#endif
	for(auto &Player : m_apPlayers)
	{
		if(Player)
		{
			if(Weapon == WEAPON_GRENADE) //grenade
			{
				if(Player->m_IsInstaArena_gdm)
				{
					SendChatTarget(Player->GetCid(), pMsg);
				}
			}
			else if(Weapon == WEAPON_LASER) //rifle
			{
				if(Player->m_IsInstaArena_idm)
				{
					SendChatTarget(Player->GetCid(), pMsg);
				}
			}
		}
	}
}

void CGameContext::DoInstaScore(int Score, int Id)
{
#if defined(CONF_DEBUG)
	log_debug("insta", "'%s' scored %d in instagib [score: %d]", Server()->ClientName(Id), Score, m_apPlayers[Id]->m_InstaScore);
#endif
	CPlayer *pPlayer = m_apPlayers[Id];
	if(!pPlayer)
		return;

	pPlayer->m_InstaScore += Score;
	if(pPlayer->GetCharacter())
		if(pPlayer->m_ShowInstaScoreBroadcast)
			pPlayer->GetCharacter()->m_UpdateInstaScoreBoard = true;
	CheckInstaWin(Id);
}

void CGameContext::CheckInstaWin(int Id)
{
	if(m_apPlayers[Id]->m_IsInstaArena_gdm)
	{
		if(m_apPlayers[Id]->m_InstaScore >= g_Config.m_SvGrenadeScorelimit)
		{
			m_InstaGrenadeRoundEndDelay = Server()->TickSpeed() * 20; //stored the value to be on the save side. I have no idea how this func works and i need the EXACT value lateron
			m_InstaGrenadeRoundEndTickTicker = m_InstaGrenadeRoundEndDelay; //start grenade round end tick
			m_InstaGrenadeWinnerId = Id;
		}
	}
	else if(m_apPlayers[Id]->m_IsInstaArena_idm)
	{
		if(m_apPlayers[Id]->m_InstaScore >= g_Config.m_SvRifleScorelimit)
		{
			m_InstaRifleRoundEndDelay = Server()->TickSpeed() * 20; //stored the value to be on the save side. I have no idea how this func works and i need the EXACT value lateron
			m_InstaRifleRoundEndTickTicker = m_InstaRifleRoundEndDelay; //start grenade round end tick
			m_InstaRifleWinnerId = Id;
		}
	}
}

void CGameContext::InstaGrenadeRoundEndTick(int Id)
{
	if(!m_InstaGrenadeRoundEndTickTicker)
	{
		return;
	}
	if(!m_apPlayers[Id]->m_IsInstaArena_gdm)
	{
		return;
	}

	char aBuf[256];

	if(m_InstaGrenadeRoundEndTickTicker == m_InstaGrenadeRoundEndDelay)
	{
		str_format(aBuf, sizeof(aBuf), "[INSTA] '%s' won the grenade game", Server()->ClientName(m_InstaGrenadeWinnerId));
		SendChatTarget(Id, aBuf);
		str_format(aBuf, sizeof(aBuf), "'%s' won the grenade game", Server()->ClientName(m_InstaGrenadeWinnerId));
		SendBroadcast(aBuf, Id);
		m_apPlayers[Id]->m_HasInstaRoundEndPos = false;

		//PlayerArryaId / PlayerTeeworldsId / PlayerScore == 64x2
		int aaScorePlayers[MAX_CLIENTS][2];

		for(auto &aScorePlayer : aaScorePlayers) //prepare array
		{
			//aaScorePlayers[i][1] = -1; //set all score to -1 to lateron filter them so please keep in mind to never let the score become negative or the poor tees will be hidden in scoreboard
			aScorePlayer[0] = -1; //set all ids to -1 to lateron filter these out of scoreboard
		}

		for(int i = 0; i < MAX_CLIENTS; i++) //fill array
		{
			if(m_apPlayers[i] && m_apPlayers[i]->m_IsInstaArena_gdm)
			{
				aaScorePlayers[i][1] = m_apPlayers[i]->m_InstaScore;
				aaScorePlayers[i][0] = i;
			}
		}

		for(int i = 0; i < MAX_CLIENTS; i++) //sort array (bubble mubble)
		{
			for(int k = 0; k < MAX_CLIENTS - 1; k++)
			{
				if(aaScorePlayers[k][1] < aaScorePlayers[k + 1][1])
				{
					//move ids
					int tmp = aaScorePlayers[k][0];
					aaScorePlayers[k][0] = aaScorePlayers[k + 1][0];
					aaScorePlayers[k + 1][0] = tmp;
					//move score
					tmp = aaScorePlayers[k][1];
					aaScorePlayers[k][1] = aaScorePlayers[k + 1][1];
					aaScorePlayers[k + 1][1] = tmp;
				}
			}
		}

		str_copy(m_aInstaGrenadeScoreboard, "==== Scoreboard [GRENADE] ====\n", sizeof(m_aInstaGrenadeScoreboard));
		int Rank = 1;

		for(auto &aScorePlayer : aaScorePlayers) //print array in scoreboard
		{
			if(aScorePlayer[0] != -1)
			{
				str_format(aBuf, sizeof(aBuf), "%d. '%s' - %d \n", Rank++, Server()->ClientName(aScorePlayer[0]), aScorePlayer[1]);
				str_append(m_aInstaGrenadeScoreboard, aBuf, sizeof(m_aInstaGrenadeScoreboard));
			}
		}
	}
	if(m_InstaGrenadeRoundEndTickTicker == 1)
	{
		//reset stats
		m_apPlayers[Id]->m_InstaScore = 0;

		if(m_apPlayers[Id]->GetCharacter())
		{
			m_apPlayers[Id]->GetCharacter()->Die(Id, WEAPON_WORLD);
		}
		SendChatTarget(Id, "[INSTA] new round new luck.");
	}

	if(m_apPlayers[Id]->GetCharacter())
	{
		if(!m_apPlayers[Id]->m_HasInstaRoundEndPos)
		{
			m_apPlayers[Id]->m_InstaRoundEndPos = m_apPlayers[Id]->GetCharacter()->GetPosition();
			m_apPlayers[Id]->m_HasInstaRoundEndPos = true;
		}

		if(m_apPlayers[Id]->m_HasInstaRoundEndPos)
		{
			m_apPlayers[Id]->GetCharacter()->SetPosition(m_apPlayers[Id]->m_InstaRoundEndPos);
		}
	}

	AbuseMotd(m_aInstaGrenadeScoreboard, Id); //send the scoreboard every fokin tick hehe
}

void CGameContext::InstaRifleRoundEndTick(int Id)
{
	if(!m_InstaRifleRoundEndTickTicker)
	{
		return;
	}
	if(!m_apPlayers[Id]->m_IsInstaArena_idm)
	{
		return;
	}

	char aBuf[256];

	if(m_InstaRifleRoundEndTickTicker == m_InstaRifleRoundEndDelay)
	{
		str_format(aBuf, sizeof(aBuf), "[INSTA] '%s' won the rifle game", Server()->ClientName(m_InstaRifleWinnerId));
		SendChatTarget(Id, aBuf);
		str_format(aBuf, sizeof(aBuf), "'%s' won the rifle game", Server()->ClientName(m_InstaRifleWinnerId));
		SendBroadcast(aBuf, Id);
		m_apPlayers[Id]->m_HasInstaRoundEndPos = false;

		//PlayerArryaId / PlayerTeeworldsId / PlayerScore == 64x2
		int aaScorePlayers[MAX_CLIENTS][2];

		for(auto &aScorePlayer : aaScorePlayers) //prepare array
		{
			//aScorePlayer[1] = -1; //set all score to -1 to lateron filter them so please keep in mind to never let the score become negative or the poor tees will be hidden in scoreboard
			aScorePlayer[0] = -1; //set all ids to -1 to lateron filter these out of scoreboard
		}

		for(int i = 0; i < MAX_CLIENTS; i++) //fill array
		{
			if(m_apPlayers[i] && m_apPlayers[i]->m_IsInstaArena_idm)
			{
				aaScorePlayers[i][1] = m_apPlayers[i]->m_InstaScore;
				aaScorePlayers[i][0] = i;
			}
		}

		for(int i = 0; i < MAX_CLIENTS; i++) //sort array (bubble mubble)
		{
			for(int k = 0; k < MAX_CLIENTS - 1; k++)
			{
				if(aaScorePlayers[k][1] < aaScorePlayers[k + 1][1])
				{
					//move ids
					int tmp = aaScorePlayers[k][0];
					aaScorePlayers[k][0] = aaScorePlayers[k + 1][0];
					aaScorePlayers[k + 1][0] = tmp;
					//move score
					tmp = aaScorePlayers[k][1];
					aaScorePlayers[k][1] = aaScorePlayers[k + 1][1];
					aaScorePlayers[k + 1][1] = tmp;
				}
			}
		}

		str_copy(m_aInstaRifleScoreboard, "==== Scoreboard [Rifle] ====\n", sizeof(m_aInstaRifleScoreboard));
		int Rank = 1;

		for(auto &aScorePlayer : aaScorePlayers) //print array in scoreboard
		{
			if(aScorePlayer[0] != -1)
			{
				str_format(aBuf, sizeof(aBuf), "%d. '%s' - %d \n", Rank++, Server()->ClientName(aScorePlayer[0]), aScorePlayer[1]);
				str_append(m_aInstaRifleScoreboard, aBuf, sizeof(m_aInstaRifleScoreboard));
			}
		}
	}
	if(m_InstaRifleRoundEndTickTicker == 1)
	{
		//reset stats
		m_apPlayers[Id]->m_InstaScore = 0;

		m_apPlayers[Id]->GetCharacter()->Die(Id, WEAPON_WORLD);
		SendChatTarget(Id, "[INSTA] new round new luck.");
	}

	if(m_apPlayers[Id]->GetCharacter())
	{
		if(!m_apPlayers[Id]->m_HasInstaRoundEndPos)
		{
			m_apPlayers[Id]->m_InstaRoundEndPos = m_apPlayers[Id]->GetCharacter()->GetPosition();
			m_apPlayers[Id]->m_HasInstaRoundEndPos = true;
		}

		if(m_apPlayers[Id]->m_HasInstaRoundEndPos)
		{
			m_apPlayers[Id]->GetCharacter()->SetPosition(m_apPlayers[Id]->m_InstaRoundEndPos);
		}
	}

	AbuseMotd(m_aInstaRifleScoreboard, Id); //send the scoreboard every fokin tick hehe
}
