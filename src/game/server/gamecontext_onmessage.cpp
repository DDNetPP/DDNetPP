// gamecontext scoped OnMessage ddnet++ methods

#include "gamecontext.h"

#include <engine/shared/config.h>

#include <game/server/teams.h>

#include <cstring>

bool CGameContext::AbortTeamChange(int ClientId, CPlayer *pPlayer)
{
	if(IsMinigame(ClientId))
	{
		SendChatTarget(ClientId, "[MINIGAMES] You can't change team while playing minigames or being in jail.");
		return true;
	}

	if(m_apPlayers[ClientId]->m_SpawnBlocks > 3)
	{
		SendChatTarget(ClientId, "[SPAWNBLOCK] You can't change team because you spawnblock too much. Try again later.");
		return true;
	}

	if(m_apPlayers[ClientId]->m_IsBlockWaving)
	{
		SendChatTarget(ClientId, "[BlockWave] you can't change team while block waving. Try '/blockwave leave'");
		return true;
	}

	//zCatch survival LMS ChillerDragon Instagib grenade rifle
	if(g_Config.m_SvInstagibMode == 2 || g_Config.m_SvInstagibMode == 4) //gLMS iLMS
	{
		SendChatTarget(ClientId, "You can't join running survival games. Wait until the round ends.");
		return true;
	}

	if(pPlayer->m_GangsterBagMoney)
	{
		SendChatTarget(ClientId, "Make sure to empty your gangsterbag before disconnecting/spectating or you will lose it.");
		SendChatTarget(ClientId, "or clear it yourself with '/gangsterbag clear'");
		return true;
	}
	return false;
}

bool CGameContext::AbortKill(int ClientId, CPlayer *pPlayer, CCharacter *pChr)
{
	if(m_InstaGrenadeRoundEndTickTicker && m_apPlayers[ClientId]->m_IsInstaArena_gdm)
		return true; //yy evil silent return
	if(m_InstaRifleRoundEndTickTicker && m_apPlayers[ClientId]->m_IsInstaArena_idm)
		return true; //yy evil silent return

	for(auto &Minigame : m_vMinigames)
		if(!Minigame->AllowSelfKill(ClientId))
			return true;

	if(m_apPlayers[ClientId]->m_IsBlockWaving && !pPlayer->m_IsBlockWaveWaiting)
	{
		SendChatTarget(ClientId, "[BlockWave] you can't selfkill while block waving. try '/blockwave leave'.");
		return true;
	}
	if(m_apPlayers[ClientId]->m_SpawnBlocks > 3 && g_Config.m_SvSpawnBlockProtection == 2)
	{
		SendChatTarget(ClientId, "[SPAWNBLOCK] You can't selfkill because you spawnblock too much. Try again later.");
		return true;
	}
	if(!g_Config.m_SvAllowBombSelfkill && GetPlayerChar(ClientId) && GetPlayerChar(ClientId)->m_IsBombing)
	{
		SendChatTarget(ClientId, "[BOMB] selfkill protection activated. Try '/bomb leave' to leave and get the money back. All other ways of leaving the game are leading to lose your money.");
		return true;
	}
	if(m_apPlayers[ClientId]->m_IsSurvivaling)
	{
		if(g_Config.m_SvSurvivalKillProtection == 2) //full on
		{
			SendChatTarget(ClientId, "[SURVIVAL] kill protection. '/survival leave' first to kill.");
			return true;
		}
		else if(g_Config.m_SvSurvivalKillProtection == 1 && !m_apPlayers[ClientId]->m_IsSurvivalLobby) //allowed in lobby
		{
			SendChatTarget(ClientId, "[SURVIVAL] kill protection. '/survival leave' first to kill.");
			return true;
		}
		//else == off
	}
	if(m_apPlayers[ClientId]->m_IsInstaArena_fng && pChr->m_FreezeTime)
	{
		SendChatTarget(ClientId, "[INSTA] You can't suicide in fng games while being frozen.");
		return true;
	}
	return false;
}
