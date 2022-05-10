// gamecontext scoped instagib ddnet++ methods

#include <engine/shared/config.h>
#include <game/server/teams.h>

#include <cinttypes>
#include <cstring>

#include "../gamecontext.h"

bool CGameContext::CanJoinInstaArena(bool grenade, bool PrivateMatch)
{
	int cPlayer = 0;

	if(grenade)
	{
		for(auto &Player : m_apPlayers)
		{
			if(Player)
			{
				if(Player->m_IsInstaArena_gdm)
				{
					cPlayer++;
					if(Player->m_Insta1on1_id != -1) //if some1 is in 1on1
					{
						return false;
					}
				}
			}
		}

		if(cPlayer >= g_Config.m_SvGrenadeArenaSlots)
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
					cPlayer++;
					if(Player->m_Insta1on1_id != -1) //if some1 is in 1on1
					{
						return false;
					}
				}
			}
		}

		if(cPlayer >= g_Config.m_SvRifleArenaSlots)
		{
			return false;
		}
	}

	if(cPlayer && PrivateMatch)
	{
		return false;
	}

	return true;
}

void CGameContext::WinInsta1on1(int WinnerID, int LooserID)
{
#if defined(CONF_DEBUG)
	if(!m_apPlayers[WinnerID])
		dbg_msg("cBug", "[WARNING] WinInsta1on1() at gamecontext.cpp");
#endif

	char aBuf[128];

	//WINNER
	if(m_apPlayers[WinnerID])
	{
		SendChatTarget(WinnerID, "==== Insta 1on1 WON ====");
		str_format(aBuf, sizeof(aBuf), "1. '%s' %d", Server()->ClientName(WinnerID), m_apPlayers[WinnerID]->m_Insta1on1_score);
		SendChatTarget(WinnerID, aBuf);
		str_format(aBuf, sizeof(aBuf), "2. '%s' %d", Server()->ClientName(LooserID), m_apPlayers[LooserID]->m_Insta1on1_score);
		SendChatTarget(WinnerID, aBuf);
		SendChatTarget(WinnerID, "==================");
		SendChatTarget(WinnerID, "+200 money for winning 1on1"); // actually it is only +100 because you have to pay to start an 1on1
		m_apPlayers[WinnerID]->MoneyTransaction(+200, "won insta 1on1");

		m_apPlayers[WinnerID]->m_IsInstaArena_gdm = false;
		m_apPlayers[WinnerID]->m_IsInstaArena_idm = false;
		m_apPlayers[WinnerID]->m_IsInstaArena_fng = false;
		m_apPlayers[WinnerID]->m_Insta1on1_id = -1;
		if(m_apPlayers[WinnerID]->GetCharacter())
		{
			m_apPlayers[WinnerID]->GetCharacter()->Die(WinnerID, WEAPON_SELF);
		}
	}

	//LOOSER
	if(LooserID != -1)
	{
		SendChatTarget(LooserID, "==== Insta 1on1 LOST ====");
		str_format(aBuf, sizeof(aBuf), "1. '%s' %d", Server()->ClientName(WinnerID), m_apPlayers[WinnerID]->m_Insta1on1_score);
		SendChatTarget(LooserID, aBuf);
		str_format(aBuf, sizeof(aBuf), "2. '%s' %d", Server()->ClientName(LooserID), m_apPlayers[LooserID]->m_Insta1on1_score);
		SendChatTarget(LooserID, aBuf);
		SendChatTarget(LooserID, "==================");

		m_apPlayers[LooserID]->m_IsInstaArena_gdm = false;
		m_apPlayers[LooserID]->m_IsInstaArena_idm = false;
		m_apPlayers[LooserID]->m_IsInstaArena_fng = false;
		m_apPlayers[LooserID]->m_Insta1on1_id = -1;
		m_apPlayers[LooserID]->m_Insta1on1_score = 0;
		if(m_apPlayers[LooserID]->GetCharacter())
		{
			m_apPlayers[LooserID]->GetCharacter()->Die(LooserID, WEAPON_SELF); //needed for /insta leave where the looser culd be alive
		}
	}

	//RESET SCORE LAST CUZ SCOREBOARD
	if(m_apPlayers[WinnerID])
		m_apPlayers[WinnerID]->m_Insta1on1_score = 0;
	if(m_apPlayers[LooserID])
		m_apPlayers[LooserID]->m_Insta1on1_score = 0;
}


void CGameContext::JoinInstagib(int weapon, bool fng, int ID)
{
#if defined(CONF_DEBUG)
	//dbg_msg("cBug", "PLAYER '%s' ID=%d JOINED INSTAGIB WITH WEAPON = %d ANF FNG = %d", Server()->ClientName(ID), ID, weapon, fng);
#endif

	//die first to not count death
	if(m_apPlayers[ID]->GetCharacter())
	{
		m_apPlayers[ID]->GetCharacter()->Die(ID, WEAPON_SELF);
	}

	//reset values
	m_apPlayers[ID]->m_HasInstaRoundEndPos = false;
	m_apPlayers[ID]->m_IsInstaArena_idm = false;
	m_apPlayers[ID]->m_IsInstaArena_gdm = false;
	m_apPlayers[ID]->m_IsInstaMode_idm = false;
	m_apPlayers[ID]->m_IsInstaMode_gdm = false;
	m_apPlayers[ID]->m_InstaScore = 0;

	m_apPlayers[ID]->m_IsInstaArena_fng = fng;
	m_apPlayers[ID]->m_IsInstaMode_fng = fng;
	if(weapon == 5)
	{
		m_apPlayers[ID]->m_IsInstaArena_idm = true;
		m_apPlayers[ID]->m_IsInstaMode_idm = true;
	}
	else if(weapon == 4)
	{
		m_apPlayers[ID]->m_IsInstaArena_gdm = true;
		m_apPlayers[ID]->m_IsInstaMode_gdm = true;
	}
	else
	{
		SendChatTarget(ID, "[WARNING] Something went horrible wrong please report an admin");
		return;
	}

	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "[INSTA] '%s' joined the game.", Server()->ClientName(ID));
	SayInsta(aBuf, weapon);
}

void CGameContext::LeaveInstagib(int ID)
{
	CPlayer *pPlayer = m_apPlayers[ID];
	if(!pPlayer)
		return;
	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "[INSTA] '%s' left the game.", Server()->ClientName(ID));
	if(pPlayer->m_IsInstaArena_gdm)
	{
		SayInsta(aBuf, 4);
	}
	else if(pPlayer->m_IsInstaArena_idm)
	{
		SayInsta(aBuf, 5);
	}

	if((pPlayer->m_IsInstaArena_gdm || pPlayer->m_IsInstaArena_idm) && pPlayer->m_Insta1on1_id != -1)
	{
		WinInsta1on1(pPlayer->m_Insta1on1_id, ID);
		SendChatTarget(ID, "[INSTA] You left the 1on1.");
		SendBroadcast("", ID);
		return;
	}

	bool left = true;

	if(pPlayer->m_IsInstaArena_fng)
	{
		if(pPlayer->m_IsInstaArena_gdm)
		{
			SendChatTarget(ID, "[INSTA] You left boomfng.");
		}
		else if(pPlayer->m_IsInstaArena_idm)
		{
			SendChatTarget(ID, "[INSTA] You left fng.");
		}
		else
		{
			left = false;
		}
	}
	else
	{
		if(pPlayer->m_IsInstaArena_gdm)
		{
			SendChatTarget(ID, "[INSTA] You left grenade deathmatch.");
		}
		else if(pPlayer->m_IsInstaArena_idm)
		{
			SendChatTarget(ID, "[INSTA] You left rifle deathmatch.");
		}
		else
		{
			left = false;
		}
	}

	if(left)
	{
		pPlayer->m_IsInstaArena_gdm = false;
		pPlayer->m_IsInstaArena_idm = false;
		pPlayer->m_IsInstaArena_fng = false;
		pPlayer->m_IsInstaMode_gdm = false;
		pPlayer->m_IsInstaMode_idm = false;
		pPlayer->m_IsInstaMode_fng = false;
		if(pChr)
		{
			pChr->Die(pPlayer->GetCID(), WEAPON_SELF);
		}
		SendBroadcast("", ID); //clear score
	}
	else
	{
		SendChatTarget(ID, "[INSTA] You are not in a instagib game.");
	}
}

void CGameContext::SayInsta(const char *pMsg, int weapon)
{
#if defined(CONF_DEBUG)
	//dbg_msg("cBug", "SayInsta got called with weapon %d and message '%s'", weapon, pMsg);
#endif
	for(auto &Player : m_apPlayers)
	{
		if(Player)
		{
			if(weapon == 4) //grenade
			{
				if(Player->m_IsInstaArena_gdm)
				{
					SendChatTarget(Player->GetCID(), pMsg);
				}
			}
			else if(weapon == 5) //rifle
			{
				if(Player->m_IsInstaArena_idm)
				{
					SendChatTarget(Player->GetCID(), pMsg);
				}
			}
		}
	}
}

void CGameContext::DoInstaScore(int score, int id)
{
#if defined(CONF_DEBUG)
	dbg_msg("insta", "'%s' scored %d in instagib [score: %d]", Server()->ClientName(id), score, m_apPlayers[id]->m_InstaScore);
#endif
	CPlayer *pPlayer = m_apPlayers[id];
	if(!pPlayer)
		return;

	pPlayer->m_InstaScore += score;
	if(pPlayer->GetCharacter())
		if(pPlayer->m_ShowInstaScoreBroadcast)
			pPlayer->GetCharacter()->m_UpdateInstaScoreBoard = true;
	CheckInstaWin(id);
}

void CGameContext::CheckInstaWin(int ID)
{
	if(m_apPlayers[ID]->m_IsInstaArena_gdm)
	{
		if(m_apPlayers[ID]->m_InstaScore >= g_Config.m_SvGrenadeScorelimit)
		{
			m_InstaGrenadeRoundEndDelay = Server()->TickSpeed() * 20; //stored the value to be on the save side. I have no idea how this func works and i need the EXACT value lateron
			m_InstaGrenadeRoundEndTickTicker = m_InstaGrenadeRoundEndDelay; //start grenade round end tick
			m_InstaGrenadeWinnerID = ID;
		}
	}
	else if(m_apPlayers[ID]->m_IsInstaArena_idm)
	{
		if(m_apPlayers[ID]->m_InstaScore >= g_Config.m_SvRifleScorelimit)
		{
			m_InstaRifleRoundEndDelay = Server()->TickSpeed() * 20; //stored the value to be on the save side. I have no idea how this func works and i need the EXACT value lateron
			m_InstaRifleRoundEndTickTicker = m_InstaRifleRoundEndDelay; //start grenade round end tick
			m_InstaRifleWinnerID = ID;
		}
	}
}

void CGameContext::InstaGrenadeRoundEndTick(int ID)
{
	if(!m_InstaGrenadeRoundEndTickTicker)
	{
		return;
	}
	if(!m_apPlayers[ID]->m_IsInstaArena_gdm)
	{
		return;
	}

	char aBuf[256];

	if(m_InstaGrenadeRoundEndTickTicker == m_InstaGrenadeRoundEndDelay)
	{
		str_format(aBuf, sizeof(aBuf), "[INSTA] '%s' won the grenade game", Server()->ClientName(m_InstaGrenadeWinnerID));
		SendChatTarget(ID, aBuf);
		str_format(aBuf, sizeof(aBuf), "'%s' won the grenade game", Server()->ClientName(m_InstaGrenadeWinnerID));
		SendBroadcast(aBuf, ID);
		m_apPlayers[ID]->m_HasInstaRoundEndPos = false;

		//PlayerArryaID / PlayerTeeworldsID / PlayerScore == 64x2
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

		str_format(m_aInstaGrenadeScoreboard, sizeof(m_aInstaGrenadeScoreboard), "==== Scoreboard [GRENADE] ====\n");
		int Rank = 1;

		for(auto &aScorePlayer : aaScorePlayers) //print array in scoreboard
		{
			if(aScorePlayer[0] != -1)
			{
				str_format(aBuf, sizeof(aBuf), "%d. '%s' - %d \n", Rank++, Server()->ClientName(aScorePlayer[0]), aScorePlayer[1]);
				strcat(m_aInstaGrenadeScoreboard, aBuf);
			}
		}
	}
	if(m_InstaGrenadeRoundEndTickTicker == 1)
	{
		//reset stats
		m_apPlayers[ID]->m_InstaScore = 0;

		if(m_apPlayers[ID]->GetCharacter())
		{
			m_apPlayers[ID]->GetCharacter()->Die(ID, WEAPON_WORLD);
		}
		SendChatTarget(ID, "[INSTA] new round new luck.");
	}

	if(m_apPlayers[ID]->GetCharacter())
	{
		if(!m_apPlayers[ID]->m_HasInstaRoundEndPos)
		{
			m_apPlayers[ID]->m_InstaRoundEndPos = m_apPlayers[ID]->GetCharacter()->GetPosition();
			m_apPlayers[ID]->m_HasInstaRoundEndPos = true;
		}

		if(m_apPlayers[ID]->m_HasInstaRoundEndPos)
		{
			m_apPlayers[ID]->GetCharacter()->SetPosition(m_apPlayers[ID]->m_InstaRoundEndPos);
		}
	}

	AbuseMotd(m_aInstaGrenadeScoreboard, ID); //send the scoreboard every fokin tick hehe
}

void CGameContext::InstaRifleRoundEndTick(int ID)
{
	if(!m_InstaRifleRoundEndTickTicker)
	{
		return;
	}
	if(!m_apPlayers[ID]->m_IsInstaArena_idm)
	{
		return;
	}

	char aBuf[256];

	if(m_InstaRifleRoundEndTickTicker == m_InstaRifleRoundEndDelay)
	{
		str_format(aBuf, sizeof(aBuf), "[INSTA] '%s' won the rifle game", Server()->ClientName(m_InstaRifleWinnerID));
		SendChatTarget(ID, aBuf);
		str_format(aBuf, sizeof(aBuf), "'%s' won the rifle game", Server()->ClientName(m_InstaRifleWinnerID));
		SendBroadcast(aBuf, ID);
		m_apPlayers[ID]->m_HasInstaRoundEndPos = false;

		//PlayerArryaID / PlayerTeeworldsID / PlayerScore == 64x2
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

		str_format(m_aInstaRifleScoreboard, sizeof(m_aInstaRifleScoreboard), "==== Scoreboard [Rifle] ====\n");
		int Rank = 1;

		for(auto &aScorePlayer : aaScorePlayers) //print array in scoreboard
		{
			if(aScorePlayer[0] != -1)
			{
				str_format(aBuf, sizeof(aBuf), "%d. '%s' - %d \n", Rank++, Server()->ClientName(aScorePlayer[0]), aScorePlayer[1]);
				strcat(m_aInstaRifleScoreboard, aBuf);
			}
		}
	}
	if(m_InstaRifleRoundEndTickTicker == 1)
	{
		//reset stats
		m_apPlayers[ID]->m_InstaScore = 0;

		m_apPlayers[ID]->GetCharacter()->Die(ID, WEAPON_WORLD);
		SendChatTarget(ID, "[INSTA] new round new luck.");
	}

	if(m_apPlayers[ID]->GetCharacter())
	{
		if(!m_apPlayers[ID]->m_HasInstaRoundEndPos)
		{
			m_apPlayers[ID]->m_InstaRoundEndPos = m_apPlayers[ID]->GetCharacter()->GetPosition();
			m_apPlayers[ID]->m_HasInstaRoundEndPos = true;
		}

		if(m_apPlayers[ID]->m_HasInstaRoundEndPos)
		{
			m_apPlayers[ID]->GetCharacter()->SetPosition(m_apPlayers[ID]->m_InstaRoundEndPos);
		}
	}

	AbuseMotd(m_aInstaRifleScoreboard, ID); //send the scoreboard every fokin tick hehe
}
