// gamecontext scoped OnMessage ddnet++ methods

#include <engine/shared/config.h>
#include <game/server/teams.h>

#include <cstring>

#include "gamecontext.h"

bool CGameContext::AbortTeamChange(int ClientID, CPlayer *pPlayer)
{
	if(IsMinigame(ClientID))
	{
		SendChatTarget(ClientID, "[MINIGAMES] You can't change team while playing minigames or being in jail.");
		return true;
	}

	if(m_apPlayers[ClientID]->m_SpawnBlocks > 3)
	{
		SendChatTarget(ClientID, "[SPAWNBLOCK] You can't change team because you spawnblock too much. Try agian later.");
		return true;
	}

	if(m_apPlayers[ClientID]->m_IsBlockWaving)
	{
		SendChatTarget(ClientID, "[BlockWave] you can't change team while block waving. Try '/blockwave leave'");
		return true;
	}

	//zCatch survival LMS ChillerDragon Instagib grenade rifle
	if(g_Config.m_SvInstagibMode == 2 || g_Config.m_SvInstagibMode == 4) //gLMS iLMS
	{
		SendChatTarget(ClientID, "You can't join running survival games. Wait until the round ends.");
		return true;
	}

	if(pPlayer->m_GangsterBagMoney)
	{
		SendChatTarget(ClientID, "Make sure to empty your gangsterbag before disconnecting/spectating or you will lose it.");
		SendChatTarget(ClientID, "or clear it yourself with '/gangsterbag clear'");
		return true;
	}
	return false;
}

bool CGameContext::AbortKill(int ClientID, CPlayer *pPlayer, CCharacter *pChr)
{
	if(m_InstaGrenadeRoundEndTickTicker && m_apPlayers[ClientID]->m_IsInstaArena_gdm)
		return true; //yy evil silent return
	if(m_InstaRifleRoundEndTickTicker && m_apPlayers[ClientID]->m_IsInstaArena_idm)
		return true; //yy evil silent return

	if(m_apPlayers[ClientID]->m_IsBlockTourning)
	{
		if(Server()->TickSpeed() * 5 > m_BlockTournaLobbyTick)
		{
			//silent return selfkill in last 5 secs of lobby tick to prevent the char being dead on tourna start
			return true;
		}
	}
	if(m_apPlayers[ClientID]->m_IsBlockWaving && !pPlayer->m_IsBlockWaveWaiting)
	{
		SendChatTarget(ClientID, "[BlockWave] you can't selfkill while block waving. try '/blockwave leave'.");
		return true;
	}
	if(m_apPlayers[ClientID]->m_SpawnBlocks > 3 && g_Config.m_SvSpawnBlockProtection == 2)
	{
		SendChatTarget(ClientID, "[SPAWNBLOCK] You can't selfkill because you spawnblock too much. Try agian later.");
		return true;
	}
	if(!g_Config.m_SvAllowBombSelfkill && GetPlayerChar(ClientID) && GetPlayerChar(ClientID)->m_IsBombing)
	{
		SendChatTarget(ClientID, "[BOMB] selfkill protection activated. Try '/bomb leave' to leave and get the money back. All other ways of leaving the game are leading to lose your money.");
		return true;
	}
	if(m_apPlayers[ClientID]->m_IsSurvivaling)
	{
		if(g_Config.m_SvSurvivalKillProtection == 2) //full on
		{
			SendChatTarget(ClientID, "[SURVIVAL] kill protection. '/survival leave' first to kill.");
			return true;
		}
		else if(g_Config.m_SvSurvivalKillProtection == 1 && m_apPlayers[ClientID]->m_IsSurvivalLobby == false) //allowed in lobby
		{
			SendChatTarget(ClientID, "[SURVIVAL] kill protection. '/survival leave' first to kill.");
			return true;
		}
		//else == off
	}
	if(m_apPlayers[ClientID]->m_IsInstaArena_fng && pChr->m_FreezeTime)
	{
		SendChatTarget(ClientID, "[INSTA] You can't suicide in fng games while being frozen.");
		return true;
	}
	return false;
}
