// gamecontext scoped minigame ddnet++ methods

#include <engine/shared/config.h>
#include <game/server/teams.h>

#include <cstring>

#include "gamecontext.h"

int CGameContext::IsMinigame(int playerID) //if you update this function please also update the '/minigames' chat command
{
	CPlayer *pPlayer = m_apPlayers[playerID];
	if(!pPlayer)
		return 0;
	CCharacter *pChr = GetPlayerChar(playerID);

	if(pPlayer->m_JailTime)
	{
		return -1;
	}
	if(pPlayer->m_IsInstaArena_gdm)
	{
		return 1;
	}
	if(pPlayer->m_IsInstaArena_idm)
	{
		return 2;
	}
	if(pPlayer->m_IsBalanceBatteling)
	{
		return 3;
	}
	if(pPlayer->m_IsSurvivaling)
	{
		return 4;
	}
	//if (pPlayer->m_Ischidraqul3) //dont return the broadcast only game because it doesnt make too much trouble. You can play chidraqul in jail or while being in insta no problem.
	//{
	//	return x;
	//}
	if(pChr)
	{
		if(pChr->m_IsBombing)
		{
			return 5;
		}
		if(pChr->m_IsPVParena)
		{
			return 6;
		}
	}
	if(pPlayer->m_IsBlockWaving)
	{
		return 7;
	}
	if(pPlayer->m_IsBlockTourning)
	{
		return 8;
	}
	if(pPlayer->m_IsBlockDeathmatch)
	{
		return 9;
	}

	return 0;
}

int CGameContext::C3_GetFreeSlots()
{
	int c = g_Config.m_SvChidraqulSlots;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i] && m_apPlayers[i]->m_C3_GameState == 2)
		{
			c--;
		}
	}
	return c;
}

int CGameContext::C3_GetOnlinePlayers()
{
	int c = 0;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i] && m_apPlayers[i]->m_C3_GameState == 2)
		{
			c++;
		}
	}
	return c;
}

void CGameContext::C3_MultiPlayer_GameTick(int id)
{
	if(m_apPlayers[id]->m_C3_UpdateFrame || Server()->Tick() % 120 == 0)
	{
		C3_RenderFrame();
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(m_apPlayers[i])
			{
				m_apPlayers[i]->m_C3_UpdateFrame = false; //only render once a tick
			}
		}
	}
}

void CGameContext::C3_RenderFrame()
{
	char aBuf[128];
	char aHUD[64];
	char aWorld[64]; //max world size
	int players = C3_GetOnlinePlayers();

	//init world
	for(int i = 0; i < g_Config.m_SvChidraqulWorldX; i++)
	{
		aWorld[i] = '_';
	}

	//place players
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i] && m_apPlayers[i]->m_C3_GameState == 2)
		{
			aWorld[m_apPlayers[i]->m_HashPos] = m_apPlayers[i]->m_HashSkin[0];
		}
	}

	//finish string
	aWorld[g_Config.m_SvChidraqulWorldX] = '\0';

	//add hud and send to players
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i] && m_apPlayers[i]->m_C3_GameState == 2)
		{
			str_format(aHUD, sizeof(aHUD), "\n\nPos: %d Players: %d/%d", m_apPlayers[i]->m_HashPos, players, g_Config.m_SvChidraqulSlots);
			str_format(aBuf, sizeof(aBuf), "%s%s", aWorld, aHUD);

			//dbg_msg("debug", "printing: %s", aBuf);

			SendBroadcast(aBuf, i, 0);
		}
	}
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
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i])
		{
			if(weapon == 4) //grenade
			{
				if(m_apPlayers[i]->m_IsInstaArena_gdm)
				{
					SendChatTarget(i, pMsg);
				}
			}
			else if(weapon == 5) //rifle
			{
				if(m_apPlayers[i]->m_IsInstaArena_idm)
				{
					SendChatTarget(i, pMsg);
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

		for(int i = 0; i < MAX_CLIENTS; i++) //prepare array
		{
			//aaScorePlayers[i][1] = -1; //set all score to -1 to lateron filter them so please keep in mind to never let the score become negative or the poor tees will be hidden in scoreboard
			aaScorePlayers[i][0] = -1; //set all ids to -1 to lateron filter these out of scoreboard
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

		for(int i = 0; i < MAX_CLIENTS; i++) //print array in scoreboard
		{
			if(aaScorePlayers[i][0] != -1)
			{
				str_format(aBuf, sizeof(aBuf), "%d. '%s' - %d \n", Rank++, Server()->ClientName(aaScorePlayers[i][0]), aaScorePlayers[i][1]);
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

		for(int i = 0; i < MAX_CLIENTS; i++) //prepare array
		{
			//aaScorePlayers[i][1] = -1; //set all score to -1 to lateron filter them so please keep in mind to never let the score become negative or the poor tees will be hidden in scoreboard
			aaScorePlayers[i][0] = -1; //set all ids to -1 to lateron filter these out of scoreboard
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

		for(int i = 0; i < MAX_CLIENTS; i++) //print array in scoreboard
		{
			if(aaScorePlayers[i][0] != -1)
			{
				str_format(aBuf, sizeof(aBuf), "%d. '%s' - %d \n", Rank++, Server()->ClientName(aaScorePlayers[i][0]), aaScorePlayers[i][1]);
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

void CGameContext::BlockTournaTick()
{
	char aBuf[128];

	if(m_BlockTournaState == 2) //ingame
	{
		m_BlockTournaTick++;
		if(m_BlockTournaTick > g_Config.m_SvBlockTournaGameTime * Server()->TickSpeed() * 60) //time over --> draw
		{
			//kill all tournas
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if(m_apPlayers[i] && m_apPlayers[i]->m_IsBlockTourning)
				{
					m_apPlayers[i]->m_IsBlockTourning = false;
					if(m_apPlayers[i]->GetCharacter())
					{
						m_apPlayers[i]->GetCharacter()->Die(i, WEAPON_GAME);
					}
				}
			}
			SendChat(-1, CGameContext::CHAT_ALL, "[EVENT] Block tournament stopped because time was over.");
			m_BlockTournaState = 0;
		}
	}
	else if(m_BlockTournaState == 1)
	{
		m_BlockTournaLobbyTick--;
		if(m_BlockTournaLobbyTick % Server()->TickSpeed() == 0)
		{
			int blockers = CountBlockTournaAlive();
			if(blockers < 0)
			{
				blockers = 1;
			}
			str_format(aBuf, sizeof(aBuf), "[EVENT] BLOCK IN %d SECONDS\n[%d/%d] '/join'ed already", m_BlockTournaLobbyTick / Server()->TickSpeed(), blockers, g_Config.m_SvBlockTournaPlayers);
			SendBroadcastAll(aBuf, 2);
		}

		if(m_BlockTournaLobbyTick < 0)
		{
			m_BlockTournaStartPlayers = CountBlockTournaAlive();
			if(m_BlockTournaStartPlayers < g_Config.m_SvBlockTournaPlayers) //minimum x players needed to start a tourna
			{
				SendBroadcastAll("[EVENT] Block tournament failed! Not enough players.", 2);
				EndBlockTourna();
				return;
			}

			SendBroadcastAll("[EVENT] Block tournament started!", 2);
			m_BlockTournaState = 2;
			m_BlockTournaTick = 0;

			//ready all players
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if(m_apPlayers[i] && m_apPlayers[i]->m_IsBlockTourning)
				{
					if(m_apPlayers[i]->GetCharacter())
					{
						//delete weapons
						m_apPlayers[i]->GetCharacter()->SetActiveWeapon(WEAPON_GUN);
						m_apPlayers[i]->GetCharacter()->SetWeaponGot(2, false);
						m_apPlayers[i]->GetCharacter()->SetWeaponGot(3, false);
						m_apPlayers[i]->GetCharacter()->SetWeaponGot(4, false);

						//delete cosmentics (they are not competetive)
						DeleteCosmetics(i);

						//delete "cheats" from the race
						m_apPlayers[i]->GetCharacter()->m_Jetpack = false;
						m_apPlayers[i]->GetCharacter()->m_EndlessHook = false;
						m_apPlayers[i]->GetCharacter()->m_SuperJump = false;

						//kill speed
						m_apPlayers[i]->GetCharacter()->KillSpeed();

						//teleport
						vec2 BlockPlayerSpawn = Collision()->GetRandomTile(TILE_BLOCK_TOURNA_SPAWN);

						if(BlockPlayerSpawn != vec2(-1, -1))
						{
							m_apPlayers[i]->GetCharacter()->SetPosition(BlockPlayerSpawn);
						}
						else //no tile found
						{
							SendBroadcastAll("[EVENT] Block tournament failed! No spawntiles found.", 2);
							EndBlockTourna();
							return;
						}

						//freeze to get a fair start nobody should be surprised
						m_apPlayers[i]->GetCharacter()->UnFreeze();
						m_apPlayers[i]->GetCharacter()->Freeze(6);
					}
					else
					{
						m_apPlayers[i]->m_IsBlockTourning = false;
						SendChatTarget(i, "[BLOCK] you didn't join because you were dead on tournament start.");
					}
				}
			}
		}
	}
}

void CGameContext::EndBlockTourna()
{
	m_BlockTournaState = 0;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i])
		{
			m_apPlayers[i]->m_IsBlockTourning = false;
		}
	}
}

int CGameContext::CountBlockTournaAlive()
{
	int c = 0;
	int id = -404;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i])
		{
			if(m_apPlayers[i]->m_IsBlockTourning)
			{
				c++;
				id = i;
			}
		}
	}

	if(c == 1) //one alive? --> return his id negative
	{
		if(id == 0)
		{
			return -420;
		}
		else
		{
			return id * -1;
		}
	}

	return c;
}

const char *CGameContext::GetBlockSkillGroup(int id)
{
	CPlayer *pPlayer = m_apPlayers[id];
	if(!pPlayer)
		return "error";

	if(pPlayer->m_BlockSkill < 1000)
	{
		return "nameless tee";
	}
	else if(pPlayer->m_BlockSkill < 3000)
	{
		return "brainless tee";
	}
	else if(pPlayer->m_BlockSkill < 6000)
	{
		return "novice tee";
	}
	else if(pPlayer->m_BlockSkill < 9000)
	{
		return "moderate tee";
	}
	else if(pPlayer->m_BlockSkill < 15000)
	{
		return "brutal tee";
	}
	else if(pPlayer->m_BlockSkill >= 20000)
	{
		return "insane tee";
	}
	else
	{
		return "unranked";
	}
}

int CGameContext::GetBlockSkillGroupInt(int id)
{
	CPlayer *pPlayer = m_apPlayers[id];
	if(!pPlayer)
		return -1;

	if(pPlayer->m_BlockSkill < 1000)
	{
		return 1;
	}
	else if(pPlayer->m_BlockSkill < 3000)
	{
		return 2;
	}
	else if(pPlayer->m_BlockSkill < 6000)
	{
		return 3;
	}
	else if(pPlayer->m_BlockSkill < 9000)
	{
		return 4;
	}
	else if(pPlayer->m_BlockSkill < 15000)
	{
		return 5;
	}
	else if(pPlayer->m_BlockSkill >= 20000)
	{
		return 6;
	}
	else
	{
		return 0;
	}
}

void CGameContext::UpdateBlockSkill(int value, int id)
{
	CPlayer *pPlayer = m_apPlayers[id];
	if(!pPlayer)
		return;

	int oldrank = GetBlockSkillGroupInt(id);
	pPlayer->m_BlockSkill += value; //update skill
	if(pPlayer->m_BlockSkill < 0)
	{
		pPlayer->m_BlockSkill = 0; //never go less than zero
	}
	else if(pPlayer->m_BlockSkill > 25000)
	{
		pPlayer->m_BlockSkill = 25000; //max skill lvl
	}
	int newrank = GetBlockSkillGroupInt(id);
	if(newrank != oldrank)
	{
		char aBuf[128];
		if(newrank < oldrank) //downrank
		{
			str_format(aBuf, sizeof(aBuf), "[BLOCK] New skillgroup '%s' (downrank)", GetBlockSkillGroup(id));
			SendChatTarget(id, aBuf);
			UpdateBlockSkill(-590, id); //lower skill agian to not get an uprank too fast agian
		}
		else //uprank
		{
			str_format(aBuf, sizeof(aBuf), "[BLOCK] New skillgroup '%s' (uprank)", GetBlockSkillGroup(id));
			SendChatTarget(id, aBuf);
			UpdateBlockSkill(+590, id); //push skill agian to not get an downrank too fast agian
		}
	}
}

void CGameContext::BlockWaveAddBots()
{
	int OccSlots = 0;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i])
		{
			OccSlots++;
		}
	}
	int FreeSlots = MAX_CLIENTS - OccSlots;

	if(m_BlockWaveRound < 15 + 1) //max 15 bots
	{
		for(int i = 1; i < m_BlockWaveRound + 1; i++)
		{
			CreateNewDummy(-3, true);
			if(i > FreeSlots - 5) //always leave 5 slots free for people to join
			{
				dbg_msg("BlockWave", "Stopped connecting at %d/%d bots because server has only %d free slots", i, m_BlockWaveRound + 1, FreeSlots);
				break;
			}
		}
	}
	else
	{
		for(int i = 1; i < 15 + 1; i++)
		{
			CreateNewDummy(-3, true);
			if(i > FreeSlots - 5) //always leave 5 slots free for people to join
			{
				dbg_msg("BlockWave", "Stopped connecting at %d/15 + 1 bots because server has only %d free slots", i, FreeSlots);
				break;
			}
		}
	}
}

void CGameContext::BlockWaveWonRound()
{
	m_BlockWaveRound++;
	SendBlockWaveSay("[BlockWave] round survived.");
	m_BlockWaveGameState = 1;

	//respawn all dead humans
	vec2 BlockWaveSpawnTile = Collision()->GetRandomTile(TILE_BLOCKWAVE_HUMAN);

	if(BlockWaveSpawnTile != vec2(-1, -1))
	{
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(m_apPlayers[i] && m_apPlayers[i]->m_IsBlockWaving)
			{
				if((!m_apPlayers[i]->m_IsDummy && m_apPlayers[i]->GetCharacter()) && (m_apPlayers[i]->GetCharacter()->m_FreezeTime || m_apPlayers[i]->m_IsBlockWaveWaiting)) //queue dudes waiting to join on new round or frozen ingames
				{
					m_apPlayers[i]->GetCharacter()->SetPosition(BlockWaveSpawnTile);
				}
				if(!m_apPlayers[i]->GetCharacter() || m_apPlayers[i]->m_IsBlockWaveWaiting) //if some queue dude is dead while waiting to join set him unqueue --> so on respawn he will enter the area
				{
					m_apPlayers[i]->m_IsBlockWaveWaiting = false;
				}
			}
		}
	}
	else //no BlockWaveSpawnTile
	{
		//GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[BlockWave] No arena set.");
		m_BlockWaveGameState = 0;
	}

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i])
		{
			m_apPlayers[i]->m_IsBlockWaveDead = false; //noboy is dead on new round
			if(m_apPlayers[i]->m_IsBlockWaving && m_apPlayers[i]->m_IsDummy) //disconnect dummys
			{
				Server()->BotLeave(i, true);
			}
		}
	}
}

void CGameContext::StartBlockWaveGame()
{
#if defined(CONF_DEBUG)
	dbg_msg("Blockwave", "Game started.");
#endif
	if(m_BlockWaveGameState)
	{
		return;
	} //no resatrt only start if not started yet
	m_BlockWaveGameState = 1;
	m_BlockWaveRound = 1; //reset rounds
	m_BlockWavePrepareDelay = (10 * Server()->TickSpeed());
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i])
		{
			m_apPlayers[i]->m_IsBlockWaveDead = false;
		}
	}
}

void CGameContext::BlockWaveGameTick()
{
	char aBuf[256];

	if(m_BlockWaveGameState == 1)
	{
		m_BlockWavePrepareDelay--;
		if(m_BlockWavePrepareDelay % Server()->TickSpeed() == 0)
		{
			str_format(aBuf, sizeof(aBuf), "[BlockWave] round %d starts in %d seconds", m_BlockWaveRound, m_BlockWavePrepareDelay / Server()->TickSpeed());
			SendBlockWaveBroadcast(aBuf);
		}
		if(m_BlockWavePrepareDelay < 0)
		{
			SendBlockWaveBroadcast("[BlockWave] Have fun and good luck!");
			m_BlockWaveGameState = 2; //start round!
			m_BlockWavePrepareDelay = (10 * Server()->TickSpeed()); //could add a cfg var in secs instead of 10 here
			BlockWaveAddBots();
		}
	}
	else //running round
	{
		//check for rip round or win round
		if(Server()->Tick() % 60 == 0)
		{
			bool ripall = true;
			bool won = true;
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if(m_apPlayers[i] && m_apPlayers[i]->m_IsBlockWaving && !m_apPlayers[i]->m_IsBlockWaveDead && !m_apPlayers[i]->m_IsDummy)
				{
					ripall = false;
					break;
				}
			}
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if(m_apPlayers[i] && m_apPlayers[i]->m_IsBlockWaving && !m_apPlayers[i]->m_IsBlockWaveDead && m_apPlayers[i]->m_IsDummy)
				{
					won = false;
					break;
				}
			}
			if(ripall)
			{
				BlockWaveStartNewGame();
			}
			if(won)
			{
				BlockWaveWonRound();
			}
		}
	}
}

void CGameContext::BlockWaveEndGame()
{
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "[BlockWave] You lost! Survived %d rounds.", m_BlockWaveRound);
	SendBlockWaveSay(aBuf);
}

void CGameContext::BlockWaveStartNewGame()
{
	BlockWaveEndGame(); //send message to all players
	m_BlockWaveGameState = 0; //end old game
	StartBlockWaveGame(); //start new game
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i] && m_apPlayers[i]->m_IsBlockWaving && m_apPlayers[i]->GetCharacter())
		{
			m_apPlayers[i]->GetCharacter()->Die(i, WEAPON_GAME);
			if(m_apPlayers[i]->m_IsDummy)
			{
				Server()->BotLeave(i, true);
			}
		}
	}
}

int CGameContext::CountBlockWavePlayers()
{
	int c = 0;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i] && m_apPlayers[i]->m_IsBlockWaving)
		{
			c++;
		}
	}
	return c;
}

void CGameContext::SendBlockWaveBroadcast(const char *pMsg)
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i] && m_apPlayers[i]->m_IsBlockWaving)
		{
			SendBroadcast(pMsg, i);
		}
	}
}

void CGameContext::SendBlockWaveSay(const char *pMsg)
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i] && m_apPlayers[i]->m_IsBlockWaving)
		{
			SendChatTarget(i, pMsg);
		}
	}
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

bool CGameContext::CanJoinInstaArena(bool grenade, bool PrivateMatch)
{
	int cPlayer = 0;

	if(grenade)
	{
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(m_apPlayers[i])
			{
				if(m_apPlayers[i]->m_IsInstaArena_gdm)
				{
					cPlayer++;
					if(m_apPlayers[i]->m_Insta1on1_id != -1) //if some1 is in 1on1
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
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(m_apPlayers[i])
			{
				if(m_apPlayers[i]->m_IsInstaArena_idm)
				{
					cPlayer++;
					if(m_apPlayers[i]->m_Insta1on1_id != -1) //if some1 is in 1on1
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

void CGameContext::StopBalanceBattle()
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i])
		{
			if(m_apPlayers[i]->m_BalanceBattle_id != -1)
			{
				m_apPlayers[i]->m_BalanceBattle_id = -1;
			}
			if(m_apPlayers[i]->m_IsBalanceBattleDummy)
			{
				Server()->BotLeave(i, true);
			}
		}
	}
	m_BalanceID1 = -1;
	m_BalanceID2 = -1;
	m_BalanceBattleState = 0; //set offline
}

void CGameContext::StartBalanceBattle(int ID1, int ID2)
{
	if(m_apPlayers[ID1] && !m_apPlayers[ID2])
	{
		SendChatTarget(ID1, "[balance] can't start a battle because your mate left.");
	}
	else if(!m_apPlayers[ID1] && m_apPlayers[ID2])
	{
		SendChatTarget(ID2, "[balance] can't start a battle because your mate left.");
	}
	else if(m_BalanceBattleState)
	{
		SendChatTarget(ID1, "[balance] can't start a battle because arena is full.");
		SendChatTarget(ID2, "[balance] can't start a battle because arena is full.");
	}
	else if(m_apPlayers[ID1] && m_apPlayers[ID2])
	{
		//moved to tick func
		//m_apPlayers[ID1]->m_IsBalanceBatteling = true;
		//m_apPlayers[ID2]->m_IsBalanceBatteling = true;
		//m_apPlayers[ID1]->m_IsBalanceBattlePlayer1 = true;
		//m_apPlayers[ID2]->m_IsBalanceBattlePlayer1 = false;
		//SendChatTarget(ID1, "[balance] BATTLE STARTED!");
		//SendChatTarget(ID2, "[balance] BATTLE STARTED!");
		//m_apPlayers[ID1]->GetCharacter()->Die(ID1, WEAPON_SELF);
		//m_apPlayers[ID2]->GetCharacter()->Die(ID2, WEAPON_SELF);

		m_BalanceDummyID1 = CreateNewDummy(-1, true);
		m_BalanceDummyID2 = CreateNewDummy(-2, true);
		m_BalanceID1 = ID1;
		m_BalanceID2 = ID2;
		m_BalanceBattleCountdown = Server()->TickSpeed() * 10;
		m_BalanceBattleState = 1; //set state to preparing
	}
}

void CGameContext::BalanceBattleTick()
{
	char aBuf[128];

	if(m_BalanceBattleState == 1) //preparing
	{
		m_BalanceBattleCountdown--;
		if(m_BalanceBattleCountdown % Server()->TickSpeed() == 0)
		{
			str_format(aBuf, sizeof(aBuf), "[balance] battle starts in %d seconds", m_BalanceBattleCountdown / Server()->TickSpeed());
			SendBroadcast(aBuf, m_BalanceID1);
			SendBroadcast(aBuf, m_BalanceID2);
		}
		if(!m_BalanceBattleCountdown)
		{
			//move the dummys
			if(m_apPlayers[m_BalanceDummyID1] && m_apPlayers[m_BalanceDummyID1]->GetCharacter())
			{
				m_apPlayers[m_BalanceDummyID1]->GetCharacter()->MoveTee(-4, -2);
			}
			if(m_apPlayers[m_BalanceDummyID2] && m_apPlayers[m_BalanceDummyID2]->GetCharacter())
			{
				m_apPlayers[m_BalanceDummyID2]->GetCharacter()->MoveTee(-4, -2);
			}

			if(m_apPlayers[m_BalanceID1] && m_apPlayers[m_BalanceID2]) //both on server
			{
				m_apPlayers[m_BalanceID1]->m_IsBalanceBatteling = true;
				m_apPlayers[m_BalanceID2]->m_IsBalanceBatteling = true;
				m_apPlayers[m_BalanceID1]->m_IsBalanceBattlePlayer1 = true;
				m_apPlayers[m_BalanceID2]->m_IsBalanceBattlePlayer1 = false;
				SendChatTarget(m_BalanceID1, "[balance] BATTLE STARTED!");
				SendChatTarget(m_BalanceID2, "[balance] BATTLE STARTED!");
				m_apPlayers[m_BalanceID1]->GetCharacter()->Die(m_BalanceID1, WEAPON_SELF);
				m_apPlayers[m_BalanceID2]->GetCharacter()->Die(m_BalanceID2, WEAPON_SELF);
				SendBroadcast("[balance] BATTLE STARTED", m_BalanceID1);
				SendBroadcast("[balance] BATTLE STARTED", m_BalanceID2);
				m_BalanceBattleState = 2; //set ingame
			}
			else if(m_apPlayers[m_BalanceID1])
			{
				SendBroadcast("[balance] BATTLE STOPPED (because mate left)", m_BalanceID1);
				StopBalanceBattle();
			}
			else if(m_apPlayers[m_BalanceID2])
			{
				SendBroadcast("[balance] BATTLE STOPPED (because mate left)", m_BalanceID2);
				StopBalanceBattle();
			}
			else
			{
				StopBalanceBattle();
			}
		}
	}
	//else if (m_BalanceBattleState == 2) //ingame //moved to die(); because it is less ressource to avoid it in tick functions
	//{
	//	if (m_apPlayers[m_BalanceID1] && m_apPlayers[m_BalanceID2])
	//	{
	//		if (!m_apPlayers[m_BalanceID1]->GetCharacter())
	//		{
	//			SendChatTarget(m_BalanceID1, "[balance] you lost!");
	//			SendChatTarget(m_BalanceID2, "[balance] you won!");
	//		}
	//	}
	//	else if (!m_apPlayers[m_BalanceID1] && !m_apPlayers[m_BalanceID2]) //all lef --> close game
	//	{
	//		m_BalanceBattleState = 0;
	//	}
	//}
}

void CGameContext::EndBombGame(int WinnerID)
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GetPlayerChar(i))
		{
			if(GetPlayerChar(i)->m_IsBombing)
			{
				GetPlayerChar(i)->m_IsBombing = false;
			}
			if(GetPlayerChar(i)->m_IsBomb)
			{
				GetPlayerChar(i)->m_IsBomb = false;
			}
			if(GetPlayerChar(i)->m_IsBombReady)
			{
				GetPlayerChar(i)->m_IsBombReady = false;
			}
		}
	}
	m_BombGameState = 0;
	m_BombTick = g_Config.m_SvBombTicks;

	if(WinnerID == -1)
	{
		return;
	}

	//winner private
	char aBuf[128];
	m_apPlayers[WinnerID]->MoneyTransaction(m_BombMoney * m_BombStartPlayers, "won bomb");
	str_format(aBuf, sizeof(aBuf), "[BOMB] You won the bomb game. +%lld money.", m_BombMoney * m_BombStartPlayers);
	SendChatTarget(WinnerID, aBuf);
	m_apPlayers[WinnerID]->m_BombGamesWon++;
	m_apPlayers[WinnerID]->m_BombGamesPlayed++;
	if(!str_comp_nocase(m_BombMap, "NoArena"))
	{
		//GetPlayerChar(i)->ChillTelePortTile(GetPlayerChar(i)->m_BombPosX, GetPlayerChar(i)->m_BombPosY); //dont tele back in no arena
	}
	else
	{
		GetPlayerChar(WinnerID)->ChillTelePortTile(GetPlayerChar(WinnerID)->m_BombPosX, GetPlayerChar(WinnerID)->m_BombPosY); //tele on pos where game started
	}

	//winner public
	str_format(aBuf, sizeof(aBuf), "[BOMB] '%s' won and got %lld money!", Server()->ClientName(WinnerID), m_BombMoney * m_BombStartPlayers);
	SendChat(-1, CGameContext::CHAT_ALL, aBuf);
}

void CGameContext::CheckStartBomb()
{
	char aBuf[128];
	bool AllReady = true;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i] && GetPlayerChar(i) && GetPlayerChar(i)->m_IsBombing && !GetPlayerChar(i)->m_IsBombReady)
		{
			AllReady = false;
			//break; //back in the times this was an performance improvement but nowerdays we need all id's of the unready players to kick em

			//Kick unready players
			m_apPlayers[i]->m_BombTicksUnready++;
			if(m_apPlayers[i]->m_BombTicksUnready + 500 == g_Config.m_SvBombUnreadyKickDelay)
			{
				SendChatTarget(i, "[BOMB] WARNING! Type '/bomb start' or you will be kicked out of the bomb game.");
			}
			if(m_apPlayers[i]->m_BombTicksUnready > g_Config.m_SvBombUnreadyKickDelay)
			{
				SendBroadcast("", i); //send empty broadcast to signalize lobby leave
				SendChatTarget(i, "[BOMB] you got kicked out of lobby. (Reason: too late '/bomb start')");

				GetPlayerChar(i)->m_IsBombing = false;
				GetPlayerChar(i)->m_IsBomb = false;
				GetPlayerChar(i)->m_IsBombReady = false;
			}
		}
	}
	//if (CountReadyBombPlayers() == CountBombPlayers()) //eats more ressources than the other way
	if(AllReady)
	{
		if(m_BombStartCountDown > 1)
		{
			if(Server()->Tick() % 40 == 0)
			{
				for(int i = 0; i < MAX_CLIENTS; i++)
				{
					if(m_apPlayers[i] && GetPlayerChar(i) && GetPlayerChar(i)->m_IsBombing)
					{
						str_format(aBuf, sizeof(aBuf), "[BOMB] game starts in %d ...", m_BombStartCountDown);
						SendBroadcast(aBuf, i);
					}
				}
				m_BombStartCountDown--;
			}
		}
		else
		{
			m_BombStartPlayers = CountBombPlayers();
			m_BombGameState = 3;
			m_BombStartCountDown = g_Config.m_SvBombStartDelay;
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if(m_apPlayers[i] && GetPlayerChar(i) && GetPlayerChar(i)->m_IsBombing)
				{
					if(!str_comp_nocase(m_BombMap, "Default"))
					{
						//GetPlayerChar(i)->m_Pos.x = g_Config.m_SvBombSpawnX + m_apPlayers[i]->GetCID() * 2; //spread the spawns round the cfg var depending on cid max distance is 63 * 2 = 126 = almost 4 tiles
						//GetPlayerChar(i)->m_Pos.x = g_Config.m_SvBombSpawnX;
						//GetPlayerChar(i)->m_Pos.y = g_Config.m_SvBombSpawnY;
						GetPlayerChar(i)->ChillTelePort((g_Config.m_SvBombSpawnX * 32) + m_apPlayers[i]->GetCID() * 2, g_Config.m_SvBombSpawnY * 32);
						//GetPlayerChar(i)->m_Pos = vec2(g_Config.m_SvBombSpawnX + m_apPlayers[i]->GetCID() * 2, g_Config.m_SvBombSpawnY); //doesnt tele but would freeze the tees (which could be nice but idk ... its scary)
					}
					str_format(aBuf, sizeof(aBuf), "Bomb game has started! +%lld money for the winner!", m_BombMoney * m_BombStartPlayers);
					SendBroadcast(aBuf, i);
					GetPlayerChar(i)->m_BombPosX = GetPlayerChar(i)->m_Pos.x / 32;
					GetPlayerChar(i)->m_BombPosY = GetPlayerChar(i)->m_Pos.y / 32;
				}
			}
		}
	}
}

void CGameContext::BombTick()
{
	char aBuf[512];

	//bomb tickin'
	m_BombTick--;
	if(m_BombTick == 0) //time over --> kill the bomb (bomb explode)
	{
		m_BombTick = g_Config.m_SvBombTicks;
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(GetPlayerChar(i))
			{
				if(GetPlayerChar(i)->m_IsBomb)
				{
					m_apPlayers[i]->m_BombGamesPlayed++;
					CreateExplosion(GetPlayerChar(i)->m_Pos, i, WEAPON_GRENADE, false, 0, GetPlayerChar(i)->Teams()->TeamMask(0)); //bomb explode! (think this explosion is always team 0 but yolo)
					str_format(aBuf, sizeof(aBuf), "'%s' exploded as bomb", Server()->ClientName(i));
					Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "bomb", aBuf);
					GetPlayerChar(i)->Die(i, WEAPON_GAME);
					break;
				}
			}
		}
	}

	//check start game
	if(m_BombGameState < 3) //not ingame
	{
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(GetPlayerChar(i) && GetPlayerChar(i)->m_IsBombing)
			{
				if(Server()->Tick() % 40 == 0)
				{
					if(GetPlayerChar(i)->m_IsBombReady)
					{
						str_format(aBuf, sizeof(aBuf), "--== Bomb Lobby ==--\n[%d/%d] players ready\nMap: %s   Money: %lld", CountReadyBombPlayers(), CountBombPlayers(), m_BombMap, m_BombMoney);
					}
					else
					{
						str_format(aBuf, sizeof(aBuf), "--== Bomb Lobby ==--\n[%d/%d] players ready\nMap: %s   Money: %lld\n\n\nType '/bomb start' to start.", CountReadyBombPlayers(), CountBombPlayers(), m_BombMap, m_BombMoney);
					}
					SendBroadcast(aBuf, i);
				}
			}
		}
		if(CountBombPlayers() > 1) //2+ tees required to start a game
		{
			CheckStartBomb();
		}
		else
		{
			m_BombGameState = 1; //unlock bomb lobbys with only 1 tee
		}
	}

	//check end game (no players)
	if(!CountBombPlayers())
	{
		EndBombGame(-1);
	}

	//check end game (only 1 player -> winner)
	if(CountBombPlayers() == 1 && m_BombGameState == 3)
	{
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(GetPlayerChar(i))
			{
				if(GetPlayerChar(i)->m_IsBombing)
				{
					EndBombGame(i);
					break;
				}
			}
		}
	}

	//check for missing bomb
	if(m_BombGameState == 3)
	{
		bool BombFound = false;
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(GetPlayerChar(i))
			{
				if(GetPlayerChar(i)->m_IsBomb)
				{
					BombFound = true;
					break;
				}
			}
		}
		if(!BombFound) //nobody bomb? -> pick new1
		{
			m_BombTick = g_Config.m_SvBombTicks;
			m_BombFinalColor = 180;

			//str_format(aBuf, sizeof(aBuf), "Bombfound: %d", FindNextBomb());
			//Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "bomb", aBuf);

			if(FindNextBomb() != -1)
			{
				GetPlayerChar(FindNextBomb())->m_IsBomb = true;
				SendChatTarget(FindNextBomb(), "The server has picked you as bomb.");
			}
			else
			{
				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "Failed to pick new bomb. Bombfound: %d", FindNextBomb());
				Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "bomb", aBuf);
			}
		}
	}
}

int CGameContext::FindNextBomb()
{
	//Check who has the furthest distance to all other players (no average middle needed)
	//New version with pythagoras
	int MaxDist = 0;
	int NextBombID = -1;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GetPlayerChar(i) && GetPlayerChar(i)->m_IsBombing)
		{
			int Dist = 0;
			for(int i_comp = 0; i_comp < MAX_CLIENTS; i_comp++)
			{
				if(GetPlayerChar(i_comp) && GetPlayerChar(i_comp)->m_IsBombing)
				{
					int a = GetPlayerChar(i)->m_Pos.x - GetPlayerChar(i_comp)->m_Pos.x;
					int b = GetPlayerChar(i)->m_Pos.y - GetPlayerChar(i_comp)->m_Pos.y;

					//|a| |b|
					a = abs(a);
					b = abs(b);

					int c = sqrt((double)(a + b)); //pythagoras rocks
					Dist += c; //store all distances to all players
				}
			}
			if(Dist > MaxDist)
			{
				MaxDist = Dist;
				NextBombID = i;
			}
		}
	}
	return NextBombID;
}

int CGameContext::CountBannedBombPlayers()
{
	int BannedPlayers = 0;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i] && m_apPlayers[i]->m_BombBanTime)
		{
			BannedPlayers++;
		}
	}

	return BannedPlayers;
}

int CGameContext::CountBombPlayers()
{
	int BombPlayers = 0;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GetPlayerChar(i))
		{
			if(GetPlayerChar(i)->m_IsBombing)
			{
				BombPlayers++;
			}
		}
	}
	return BombPlayers;
}

int CGameContext::CountReadyBombPlayers()
{
	int RdyPlrs = 0;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GetPlayerChar(i) && GetPlayerChar(i)->m_IsBombing && GetPlayerChar(i)->m_IsBombReady)
		{
			RdyPlrs++;
		}
	}
	return RdyPlrs;
}
