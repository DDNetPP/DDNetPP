/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/mapitems.h>

#include <game/generated/protocol.h>

#include "entities/pickup.h"
#include "gamecontroller.h"
#include "gamecontext.h"
#include <game/server/entities/character.h>
#include <game/server/teams.h>

#include "entities/light.h"
#include "entities/dragger.h"
#include "entities/gun.h"
#include "entities/projectile.h"
#include "entities/plasma.h"
#include "entities/door.h"
#include <game/layers.h>


IGameController::IGameController(class CGameContext *pGameServer)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	m_pGameServer = pGameServer;
	m_pServer = m_pGameServer->Server();
	m_pGameType = "unknown";

	//
	DoWarmup(g_Config.m_SvWarmup);
	m_GameOverTick = -1;
	m_SuddenDeath = 0;
	m_RoundStartTick = Server()->Tick();
	m_RoundCount = 0;
	m_GameFlags = 0;
	//m_aTeamscore[TEAM_RED] = 0;
	//m_aTeamscore[TEAM_BLUE] = 0;
	m_aMapWish[0] = 0;

	m_UnbalancedTick = -1;
	m_ForceBalanced = false;

	m_aNumSpawnPoints[0] = 0;
	m_aNumSpawnPoints[1] = 0;
	m_aNumSpawnPoints[2] = 0;

	m_CurrentRecord = 0;
}

IGameController::~IGameController()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
}

float IGameController::EvaluateSpawnPos(CSpawnEval *pEval, vec2 Pos)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	float Score = 0.0f;
	CCharacter *pC = static_cast<CCharacter *>(GameServer()->m_World.FindFirst(CGameWorld::ENTTYPE_CHARACTER));
	for (; pC; pC = (CCharacter *)pC->TypeNext())
	{
		// team mates are not as dangerous as enemies
		float Scoremod = 1.0f;
		if (pEval->m_FriendlyTeam != -1 && pC->GetPlayer()->GetTeam() == pEval->m_FriendlyTeam)
			Scoremod = 0.5f;

		float d = distance(Pos, pC->m_Pos);
		Score += Scoremod * (d == 0 ? 1000000000.0f : 1.0f / d);
	}

	return Score;
}

void IGameController::EvaluateSpawnType(CSpawnEval *pEval, int Type)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	// get spawn point
	for (int i = 0; i < m_aNumSpawnPoints[Type]; i++)
	{
		// check if the position is occupado
		CCharacter *aEnts[MAX_CLIENTS];
		int Num = GameServer()->m_World.FindEntities(m_aaSpawnPoints[Type][i], 64, (CEntity**)aEnts, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
		vec2 Positions[5] = { vec2(0.0f, 0.0f), vec2(-32.0f, 0.0f), vec2(0.0f, -32.0f), vec2(32.0f, 0.0f), vec2(0.0f, 32.0f) };	// start, left, up, right, down
		int Result = -1;
		for (int Index = 0; Index < 5 && Result == -1; ++Index)
		{
			Result = Index;
			if (!GameServer()->m_World.m_Core.m_Tuning[0].m_PlayerCollision)
				break;
			for (int c = 0; c < Num; ++c)
				if (GameServer()->Collision()->CheckPoint(m_aaSpawnPoints[Type][i] + Positions[Index]) ||
					distance(aEnts[c]->m_Pos, m_aaSpawnPoints[Type][i] + Positions[Index]) <= aEnts[c]->m_ProximityRadius)
				{
					Result = -1;
					break;
				}
		}
		if (Result == -1)
		{
			//dbg_msg("cBug", "try next spawn");
			continue;	// try next spawn point
		}

		vec2 P = m_aaSpawnPoints[Type][i] + Positions[Result];
		float S = EvaluateSpawnPos(pEval, P);
		if (!pEval->m_Got || pEval->m_Score > S)
		{
			pEval->m_Got = true;
			pEval->m_Score = S;
			pEval->m_Pos = P;
		}
		else
		{
			//dbg_msg("cBug", "spawn failed");
		}
	}
}

bool IGameController::CanSpawn(int Team, vec2 *pOutPos, class CPlayer *pPlayer)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CSpawnEval Eval;

	// spectators can't spawn
	if (Team == TEAM_SPECTATORS)
		return false;

	/*if(IsTeamplay())
	{
	Eval.m_FriendlyTeam = Team;

	// first try own team spawn, then normal spawn and then enemy
	EvaluateSpawnType(&Eval, 1+(Team&1));
	if(!Eval.m_Got)
	{
	EvaluateSpawnType(&Eval, 0);
	if(!Eval.m_Got)
	EvaluateSpawnType(&Eval, 1+((Team+1)&1));
	}
	}
	else
	{*/

	//if (pPlayer->m_JailTime)
	//{
	//	vec2 JailSpawn = GameServer()->Collision()->GetRandomTile(TILE_JAIL);

	//	if (JailSpawn != vec2(-1, -1))
	//	{
	//		pPlayer->GetCharacter()->SetPosition(JailSpawn);
	//	}
	//	else //no jailrelease
	//	{
	//		EvaluateSpawnType(&Eval, 0); //tele to spawn
	//	}
	//}
	//else
	//{
		if (g_Config.m_SvSpawntilesMode == 1)
		{
			if (pPlayer->m_IsInstaArena_gdm)
			{
				EvaluateSpawnType(&Eval, 1); //red (not bloody anymore)
			}
			else if (pPlayer->m_IsInstaArena_idm)
			{
				EvaluateSpawnType(&Eval, 2); //blue
			}
			/* doesnt work yet no spawning like there were no spawntiles set
			else if (pPlayer->m_IsSurvivaling && pPlayer->m_IsSurvivalAlive)
			{
				EvaluateSpawnType(&Eval, 3); //survival spawntile
			}
			*/
			else
			{
				EvaluateSpawnType(&Eval, 0); //default
			}

			//EvaluateSpawnType(&Eval, 1); //red (bloody)
		}
		else
		{
			EvaluateSpawnType(&Eval, 0); //default
			EvaluateSpawnType(&Eval, 1); //red (bloody)
			EvaluateSpawnType(&Eval, 2); //blue
		}
	//}
	//}

	*pOutPos = Eval.m_Pos;
	return Eval.m_Got;
}


//bool IGameController::OnEntity(int Index, vec2 Pos)
bool IGameController::OnEntity(int Index, vec2 Pos, int Layer, int Flags, int Number)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (Index < 0)
		return false;

	int Type = -1;
	int SubType = 0;

	int x, y;
	x = (Pos.x - 16.0f) / 32.0f;
	y = (Pos.y - 16.0f) / 32.0f;
	int sides[8];
	sides[0] = GameServer()->Collision()->Entity(x, y + 1, Layer);
	sides[1] = GameServer()->Collision()->Entity(x + 1, y + 1, Layer);
	sides[2] = GameServer()->Collision()->Entity(x + 1, y, Layer);
	sides[3] = GameServer()->Collision()->Entity(x + 1, y - 1, Layer);
	sides[4] = GameServer()->Collision()->Entity(x, y - 1, Layer);
	sides[5] = GameServer()->Collision()->Entity(x - 1, y - 1, Layer);
	sides[6] = GameServer()->Collision()->Entity(x - 1, y, Layer);
	sides[7] = GameServer()->Collision()->Entity(x - 1, y + 1, Layer);


	if (Index == ENTITY_SPAWN)
		m_aaSpawnPoints[0][m_aNumSpawnPoints[0]++] = Pos;
	else if (Index == ENTITY_SPAWN_RED)
		m_aaSpawnPoints[1][m_aNumSpawnPoints[1]++] = Pos;
		//m_FunPoint = Pos; //Blue redspawn rainbow tile rainbowtile
	else if (Index == ENTITY_SPAWN_BLUE)
		m_aaSpawnPoints[2][m_aNumSpawnPoints[2]++] = Pos;
	else if (Index == TILE_SURVIVAL_SPAWN) //testy ddnet++ by ChillerDragon use spawn system for survival spawns
		m_aaSpawnPoints[3][m_aNumSpawnPoints[3]++] = Pos;


	else if (Index == ENTITY_DOOR)
	{
		for (int i = 0; i < 8; i++)
		{
			if (sides[i] >= ENTITY_LASER_SHORT && sides[i] <= ENTITY_LASER_LONG)
			{
				new CDoor
				(
					&GameServer()->m_World, //GameWorld
					Pos, //Pos
					pi / 4 * i, //Rotation
					32 * 3 + 32 * (sides[i] - ENTITY_LASER_SHORT) * 3, //Length
					Number //Number
				);
			}
		}
	}
	else if (Index == ENTITY_CRAZY_SHOTGUN_EX)
	{
		int Dir;
		if (!Flags)
			Dir = 0;
		else if (Flags == ROTATION_90)
			Dir = 1;
		else if (Flags == ROTATION_180)
			Dir = 2;
		else
			Dir = 3;
		float Deg = Dir * (pi / 2);
		CProjectile *bullet = new CProjectile
		(
			&GameServer()->m_World,
			WEAPON_SHOTGUN, //Type
			-1, //Owner
			Pos, //Pos
			vec2(sin(Deg), cos(Deg)), //Dir
			-2, //Span
			true, //Freeze 
			true, //Explosive
			0, //Force
			(g_Config.m_SvShotgunBulletSound) ? SOUND_GRENADE_EXPLODE : -1,//SoundImpact
			WEAPON_SHOTGUN,//Weapon
			Layer,
			Number
		);
		bullet->SetBouncing(2 - (Dir % 2));
	}
	else if (Index == ENTITY_CRAZY_SHOTGUN)
	{
		int Dir;
		if (!Flags)
			Dir = 0;
		else if (Flags == (TILEFLAG_ROTATE))
			Dir = 1;
		else if (Flags == (TILEFLAG_VFLIP | TILEFLAG_HFLIP))
			Dir = 2;
		else
			Dir = 3;
		float Deg = Dir * (pi / 2);
		CProjectile *bullet = new CProjectile
		(
			&GameServer()->m_World,
			WEAPON_SHOTGUN, //Type
			-1, //Owner
			Pos, //Pos
			vec2(sin(Deg), cos(Deg)), //Dir
			-2, //Span
			true, //Freeze
			false, //Explosive
			0,
			SOUND_GRENADE_EXPLODE,
			WEAPON_SHOTGUN, //Weapon
			Layer,
			Number
		);
		bullet->SetBouncing(2 - (Dir % 2));
	}

	if (Index == ENTITY_ARMOR_1)
		Type = POWERUP_ARMOR;
	else if (Index == ENTITY_HEALTH_1)
		Type = POWERUP_HEALTH;
	else if (Index == ENTITY_WEAPON_SHOTGUN)
	{
		Type = POWERUP_WEAPON;
		SubType = WEAPON_SHOTGUN;
	}
	else if (Index == ENTITY_WEAPON_GRENADE)
	{
		Type = POWERUP_WEAPON;
		SubType = WEAPON_GRENADE;
	}
	else if (Index == ENTITY_WEAPON_RIFLE)
	{
		Type = POWERUP_WEAPON;
		SubType = WEAPON_RIFLE;
	}
	else if (Index == ENTITY_WEAPON_GUN)
	{
		Type = POWERUP_WEAPON;
		SubType = WEAPON_GUN;
	}
	else if (Index == ENTITY_WEAPON_HAMMER)
	{
		Type = POWERUP_WEAPON;
		SubType = WEAPON_HAMMER;
	}
	//else if(Index == ENTITY_POWERUP_NINJA && g_Config.m_SvPowerups)
	else if (Index == ENTITY_POWERUP_NINJA)
	{
		Type = POWERUP_NINJA;
		SubType = WEAPON_NINJA;
	}
	else if (Index >= ENTITY_LASER_FAST_CW && Index <= ENTITY_LASER_FAST_CCW)
	{
		int sides2[8];
		sides2[0] = GameServer()->Collision()->Entity(x, y + 2, Layer);
		sides2[1] = GameServer()->Collision()->Entity(x + 2, y + 2, Layer);
		sides2[2] = GameServer()->Collision()->Entity(x + 2, y, Layer);
		sides2[3] = GameServer()->Collision()->Entity(x + 2, y - 2, Layer);
		sides2[4] = GameServer()->Collision()->Entity(x, y - 2, Layer);
		sides2[5] = GameServer()->Collision()->Entity(x - 2, y - 2, Layer);
		sides2[6] = GameServer()->Collision()->Entity(x - 2, y, Layer);
		sides2[7] = GameServer()->Collision()->Entity(x - 2, y + 2, Layer);

		float AngularSpeed = 0.0;
		int Ind = Index - ENTITY_LASER_STOP;
		int M;
		if (Ind < 0)
		{
			Ind = -Ind;
			M = 1;
		}
		else if (Ind == 0)
			M = 0;
		else
			M = -1;


		if (Ind == 0)
			AngularSpeed = 0.0f;
		else if (Ind == 1)
			AngularSpeed = pi / 360;
		else if (Ind == 2)
			AngularSpeed = pi / 180;
		else if (Ind == 3)
			AngularSpeed = pi / 90;
		AngularSpeed *= M;

		for (int i = 0; i<8; i++)
		{
			if (sides[i] >= ENTITY_LASER_SHORT && sides[i] <= ENTITY_LASER_LONG)
			{
				CLight *Lgt = new CLight(&GameServer()->m_World, Pos, pi / 4 * i, 32 * 3 + 32 * (sides[i] - ENTITY_LASER_SHORT) * 3, Layer, Number);
				Lgt->m_AngularSpeed = AngularSpeed;
				if (sides2[i] >= ENTITY_LASER_C_SLOW && sides2[i] <= ENTITY_LASER_C_FAST)
				{
					Lgt->m_Speed = 1 + (sides2[i] - ENTITY_LASER_C_SLOW) * 2;
					Lgt->m_CurveLength = Lgt->m_Length;
				}
				else if (sides2[i] >= ENTITY_LASER_O_SLOW && sides2[i] <= ENTITY_LASER_O_FAST)
				{
					Lgt->m_Speed = 1 + (sides2[i] - ENTITY_LASER_O_SLOW) * 2;
					Lgt->m_CurveLength = 0;
				}
				else
					Lgt->m_CurveLength = Lgt->m_Length;
			}
		}

	}
	else if (Index >= ENTITY_DRAGGER_WEAK && Index <= ENTITY_DRAGGER_STRONG)
	{
		CDraggerTeam(&GameServer()->m_World, Pos, Index - ENTITY_DRAGGER_WEAK + 1, false, Layer, Number);
	}
	else if (Index >= ENTITY_DRAGGER_WEAK_NW && Index <= ENTITY_DRAGGER_STRONG_NW)
	{
		CDraggerTeam(&GameServer()->m_World, Pos, Index - ENTITY_DRAGGER_WEAK_NW + 1, true, Layer, Number);
	}
	else if (Index == ENTITY_PLASMAE)
	{
		new CGun(&GameServer()->m_World, Pos, false, true, Layer, Number);
	}
	else if (Index == ENTITY_PLASMAF)
	{
		new CGun(&GameServer()->m_World, Pos, true, false, Layer, Number);
	}
	else if (Index == ENTITY_PLASMA)
	{
		new CGun(&GameServer()->m_World, Pos, true, true, Layer, Number);
	}
	else if (Index == ENTITY_PLASMAU)
	{
		new CGun(&GameServer()->m_World, Pos, false, false, Layer, Number);
	}

	if (Type != -1)
	{
		//CPickup *pPickup = new CPickup(&GameServer()->m_World, Type, SubType);
		CPickup *pPickup = new CPickup(&GameServer()->m_World, Type, SubType, Layer, Number);
		pPickup->m_Pos = Pos;
		return true;
	}

	return false;
}

void IGameController::EndRound()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (m_Warmup) // game can't end when we are running warmup
		return;

	if (g_Config.m_SvDDPPgametype != 5)
		return;

	dbg_msg("cBug", "round end");
	GameServer()->m_World.m_Paused = true;
	m_GameOverTick = Server()->Tick();
	m_SuddenDeath = 0;
}

void IGameController::ResetGame()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	GameServer()->m_World.m_ResetRequested = true;
}

const char *IGameController::GetTeamName(int Team)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	/*
	if(IsTeamplay())
	{
	if(Team == TEAM_RED)
	return "red team";
	else if(Team == TEAM_BLUE)
	return "blue team";
	}
	else
	{
	if(Team == 0)
	return "game";
	}*/

	if (Team == 0)
		return "game";
	return "spectators";
}

//static bool IsSeparator(char c) { return c == ';' || c == ' ' || c == ',' || c == '\t'; }

void IGameController::StartRound()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	ResetGame();

	m_RoundStartTick = Server()->Tick();
	m_SuddenDeath = 0;
	m_GameOverTick = -1;
	GameServer()->m_World.m_Paused = false;
	m_aTeamscore[TEAM_RED] = 0;
	m_aTeamscore[TEAM_BLUE] = 0;
	m_ForceBalanced = false;
	Server()->DemoRecorder_HandleAutoStart();
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "start round type='%s' teamplay='%d'", m_pGameType, m_GameFlags&GAMEFLAG_TEAMS);
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
}

void IGameController::ChangeMap(const char *pToMap)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	/*str_copy(m_aMapWish, pToMap, sizeof(m_aMapWish));
	EndRound();*/
	str_copy(g_Config.m_SvMap, pToMap, sizeof(m_aMapWish));
}

/*void IGameController::CycleMap()
{
#if defined(CONF_DEBUG)
CALL_STACK_ADD();
#endif
if(m_aMapWish[0] != 0)
{
char aBuf[256];
str_format(aBuf, sizeof(aBuf), "rotating map to %s", m_aMapWish);
GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
str_copy(g_Config.m_SvMap, m_aMapWish, sizeof(g_Config.m_SvMap));
m_aMapWish[0] = 0;
m_RoundCount = 0;
return;
}
if(!str_length(g_Config.m_SvMaprotation))
return;

if(m_RoundCount < g_Config.m_SvRoundsPerMap-1)
{
if(g_Config.m_SvRoundSwap)
GameServer()->SwapTeams();
return;
}

// handle maprotation
const char *pMapRotation = g_Config.m_SvMaprotation;
const char *pCurrentMap = g_Config.m_SvMap;

int CurrentMapLen = str_length(pCurrentMap);
const char *pNextMap = pMapRotation;
while(*pNextMap)
{
int WordLen = 0;
while(pNextMap[WordLen] && !IsSeparator(pNextMap[WordLen]))
WordLen++;

if(WordLen == CurrentMapLen && str_comp_num(pNextMap, pCurrentMap, CurrentMapLen) == 0)
{
// map found
pNextMap += CurrentMapLen;
while(*pNextMap && IsSeparator(*pNextMap))
pNextMap++;

break;
}

pNextMap++;
}

// restart rotation
if(pNextMap[0] == 0)
pNextMap = pMapRotation;

// cut out the next map
char aBuf[512] = {0};
for(int i = 0; i < 511; i++)
{
aBuf[i] = pNextMap[i];
if(IsSeparator(pNextMap[i]) || pNextMap[i] == 0)
{
aBuf[i] = 0;
break;
}
}

// skip spaces
int i = 0;
while(IsSeparator(aBuf[i]))
i++;

m_RoundCount = 0;

char aBufMsg[256];
str_format(aBufMsg, sizeof(aBufMsg), "rotating map to %s", &aBuf[i]);
GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
str_copy(g_Config.m_SvMap, &aBuf[i], sizeof(g_Config.m_SvMap));
}*/

void IGameController::PostReset()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (GameServer()->m_apPlayers[i])
		{
			GameServer()->m_apPlayers[i]->Respawn();
			//GameServer()->m_apPlayers[i]->m_Score = 0;
			//GameServer()->m_apPlayers[i]->m_ScoreStartTick = Server()->Tick();
			//GameServer()->m_apPlayers[i]->m_RespawnTick = Server()->Tick()+Server()->TickSpeed()/2;
		}
	}
}

/*void IGameController::OnPlayerInfoChange(class CPlayer *pP)
{
#if defined(CONF_DEBUG)
CALL_STACK_ADD();
#endif
const int aTeamColors[2] = {65387, 10223467};
if(IsTeamplay())
{
pP->m_TeeInfos.m_UseCustomColor = 1;
if(pP->GetTeam() >= TEAM_RED && pP->GetTeam() <= TEAM_BLUE)
{
pP->m_TeeInfos.m_ColorBody = aTeamColors[pP->GetTeam()];
pP->m_TeeInfos.m_ColorFeet = aTeamColors[pP->GetTeam()];
}
else
{
pP->m_TeeInfos.m_ColorBody = 12895054;
pP->m_TeeInfos.m_ColorFeet = 12895054;
}
}
}*/


int IGameController::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

	/*// do scoreing
	if(!pKiller || Weapon == WEAPON_GAME)
	return 0;
	if(pKiller == pVictim->GetPlayer())
	pVictim->GetPlayer()->m_Score--; // suicide
	else
	{
	if(IsTeamplay() && pVictim->GetPlayer()->GetTeam() == pKiller->GetTeam())
	pKiller->m_Score--; // teamkill
	else
	pKiller->m_Score++; // normal kill
	}
	if(Weapon == WEAPON_SELF)
	pVictim->GetPlayer()->m_RespawnTick = Server()->Tick()+Server()->TickSpeed()*3.0f;*/



	//pKiller->m_KillStreak++;
	//char aBuf[256];
	//str_format(aBuf, sizeof(aBuf), "%s's killing Spree has ended by %s (%d Kills)", Server()->ClientName(pVictim->GetPlayer()->GetCID()), Server()->ClientName(pKiller->GetCID()), pVictim->GetPlayer()->m_KillStreak);
	//if (pVictim->GetPlayer()->m_KillStreak >= 5)
	//{
	//	GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
	//}
	//pVictim->GetPlayer()->m_KillStreak = 0;
	//char m_SpreeMsg[10][100] = { "on a killing spree", "on a rampage", "dominating", "unstoppable", "skilled", "hammerfight boss", "doing great", "the master","the best","the king" };
	//int iBuf = ((pKiller->m_KillStreak / 5) - 1) % 10;
	//str_format(aBuf, sizeof(aBuf), "%s is %s with %d Hammerkills!", Server()->ClientName(pKiller->GetCID()), m_SpreeMsg[iBuf], pKiller->m_KillStreak);
	//if (pKiller->m_KillStreak % 5 == 0 && pKiller->m_KillStreak >= 5)
	//	GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);


	GameServer()->SendChat(-1, CGameContext::CHAT_ALL, "KILL");
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", "pkiller %d", pKiller);

	if (!pKiller || Weapon == WEAPON_GAME)
		return 0;
	if (pKiller == pVictim->GetPlayer())
		return 0; // suicide

	pKiller->GetCharacter()->m_Bloody = true;



	return 0;
}

void IGameController::OnCharacterSpawn(class CCharacter *pChr)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	// default health
	pChr->IncreaseHealth(10);

	//zCatch ChillerDragon
	if (pChr->GetPlayer()->m_IsInstaMode_fng)
	{
		pChr->GiveWeapon(WEAPON_HAMMER, -1);
	}

	if (g_Config.m_SvInstagibMode == 1 || g_Config.m_SvInstagibMode == 2 || pChr->GetPlayer()->m_IsInstaMode_gdm) //gdm & zCatch grenade
	{
		pChr->GiveWeapon(WEAPON_GRENADE, -1);
	}
	else if (g_Config.m_SvInstagibMode == 3 || g_Config.m_SvInstagibMode == 4 || pChr->GetPlayer()->m_IsInstaMode_idm) // idm & zCatch rifle
	{
		pChr->GiveWeapon(WEAPON_RIFLE, -1);
	}
	else
	{
		// give default weapons              ---    dragon test disarm
		if (pChr->GetPlayer()->m_disarm)
		{
			pChr->GiveWeapon(WEAPON_GUN, -1);
		}
		//else if (pChr->GetPlayer()->m_hammerfight)
		//{  
		//pChr->GiveWeapon(WEAPON_HAMMER, -1);
		//}
		else
		{
			pChr->GiveWeapon(WEAPON_HAMMER, -1);
			pChr->GiveWeapon(WEAPON_GUN, -1);
		}
	}
}

void IGameController::DoWarmup(int Seconds)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (Seconds < 0)
		m_Warmup = 0;
	else
		m_Warmup = Seconds*Server()->TickSpeed();
}

/*bool IGameController::IsFriendlyFire(int ClientID1, int ClientID2)
{
#if defined(CONF_DEBUG)
CALL_STACK_ADD();
#endif
if(ClientID1 == ClientID2)
return false;

if(IsTeamplay())
{
if(!GameServer()->m_apPlayers[ClientID1] || !GameServer()->m_apPlayers[ClientID2])
return false;

if(GameServer()->m_apPlayers[ClientID1]->GetTeam() == GameServer()->m_apPlayers[ClientID2]->GetTeam())
return true;
}

return false;
}*/

bool IGameController::IsForceBalanced()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	/*if(m_ForceBalanced)
	{
	m_ForceBalanced = false;
	return true;
	}
	else*/
	return false;
}

bool IGameController::CanBeMovedOnBalance(int ClientID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	return true;
}

void IGameController::Tick()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

	/*if (m_FunPoint) {

		CCharacter *apCloseCCharacters[MAX_CLIENTS];
		int Num = m_pGameServer->m_World.FindEntities(m_FunPoint, 140, (CEntity**)apCloseCCharacters, 1, CGameWorld::ENTTYPE_CHARACTER);

		if (Num == 1) {

			m_pGameServer->CreateDeath(m_FunPoint, apCloseCCharacters[0]->GetPlayer()->GetCID(), apCloseCCharacters[0]->Teams()->TeamMask(apCloseCCharacters[0]->Team()));

		}

	}*/

	// do warmup
	if (m_Warmup)
	{
		m_Warmup--;
		if (!m_Warmup)
			StartRound();
	}

	if (m_GameOverTick != -1)
	{
		// game over.. wait for restart
		if (Server()->Tick() > m_GameOverTick + Server()->TickSpeed() * 10)
		{
			//CycleMap();
			StartRound();
			m_RoundCount++;
		}
	}

	// game is Paused
	if(GameServer()->m_World.m_Paused)
		++m_RoundStartTick;

	/*
	// do team-balancing
	if (IsTeamplay() && m_UnbalancedTick != -1 && Server()->Tick() > m_UnbalancedTick + g_Config.m_SvTeambalanceTime*Server()->TickSpeed() * 60)
	{
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", "Balancing teams");

		int aT[2] = { 0,0 };
		float aTScore[2] = { 0,0 };
		float aPScore[MAX_CLIENTS] = { 0.0f };
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS)
			{
				aT[GameServer()->m_apPlayers[i]->GetTeam()]++;
				aPScore[i] = GameServer()->m_apPlayers[i]->m_Score*Server()->TickSpeed()*60.0f /
					(Server()->Tick() - GameServer()->m_apPlayers[i]->m_ScoreStartTick);
				aTScore[GameServer()->m_apPlayers[i]->GetTeam()] += aPScore[i];
			}
		}

		// are teams unbalanced?
		if (absolute(aT[0] - aT[1]) >= 2)
		{
			int M = (aT[0] > aT[1]) ? 0 : 1;
			int NumBalance = absolute(aT[0] - aT[1]) / 2;

			do
			{
				CPlayer *pP = 0;
				float PD = aTScore[M];
				for (int i = 0; i < MAX_CLIENTS; i++)
				{
					if (!GameServer()->m_apPlayers[i] || !CanBeMovedOnBalance(i))
						continue;
					// remember the player who would cause lowest score-difference
					if (GameServer()->m_apPlayers[i]->GetTeam() == M && (!pP || absolute((aTScore[M ^ 1] + aPScore[i]) - (aTScore[M] - aPScore[i])) < PD))
					{
						pP = GameServer()->m_apPlayers[i];
						PD = absolute((aTScore[M ^ 1] + aPScore[i]) - (aTScore[M] - aPScore[i]));
					}
				}

				// move the player to the other team
				int Temp = pP->m_LastActionTick;
				pP->SetTeam(M ^ 1);
				pP->m_LastActionTick = Temp;

				pP->Respawn();
				pP->m_ForceBalanced = true;
			} while (--NumBalance);

			m_ForceBalanced = true;
		}
		m_UnbalancedTick = -1;
	}
	*/

	// check for inactive players
	if (g_Config.m_SvInactiveKickTime > 0)
	{
		for (int i = 0; i < MAX_CLIENTS; ++i)
		{
#ifdef CONF_DEBUG
			if (g_Config.m_DbgDummies)
			{
				if (i >= MAX_CLIENTS - g_Config.m_DbgDummies)
					break;
			}
#endif
			if (GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS && !Server()->IsAuthed(i))
			{
				if (Server()->Tick() > GameServer()->m_apPlayers[i]->m_LastActionTick + g_Config.m_SvInactiveKickTime*Server()->TickSpeed() * 60)
				{
					switch (g_Config.m_SvInactiveKick)
					{
					case 0:
					{
						// move player to spectator
						GameServer()->m_apPlayers[i]->SetTeam(TEAM_SPECTATORS);
					}
					break;
					case 1:
					{
						// move player to spectator if the reserved slots aren't filled yet, kick him otherwise
						int Spectators = 0;
						for (int j = 0; j < MAX_CLIENTS; ++j)
							if (GameServer()->m_apPlayers[j] && GameServer()->m_apPlayers[j]->GetTeam() == TEAM_SPECTATORS)
								++Spectators;
						if (Spectators >= g_Config.m_SvSpectatorSlots)
							Server()->Kick(i, "Kicked for inactivity");
						else
							GameServer()->m_apPlayers[i]->SetTeam(TEAM_SPECTATORS);
					}
					break;
					case 2:
					{
						// kick the player
						Server()->Kick(i, "Kicked for inactivity");
					}
					}
				}
			}
		}
	}
	DoWincheck();
}


bool IGameController::IsTeamplay() const
{
#if defined(CONF_DEBUG)
CALL_STACK_ADD();
#endif
return m_GameFlags&GAMEFLAG_TEAMS;
}

void IGameController::Snap(int SnappingClient)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CNetObj_GameInfo *pGameInfoObj = (CNetObj_GameInfo *)Server()->SnapNewItem(NETOBJTYPE_GAMEINFO, 0, sizeof(CNetObj_GameInfo));
	if (!pGameInfoObj)
		return;

	pGameInfoObj->m_GameFlags = m_GameFlags;
	pGameInfoObj->m_GameStateFlags = 0;
	if (m_GameOverTick != -1)
		pGameInfoObj->m_GameStateFlags |= GAMESTATEFLAG_GAMEOVER;
	if (m_SuddenDeath)
		pGameInfoObj->m_GameStateFlags |= GAMESTATEFLAG_SUDDENDEATH;
	if (GameServer()->m_World.m_Paused)
		pGameInfoObj->m_GameStateFlags |= GAMESTATEFLAG_PAUSED;
	pGameInfoObj->m_RoundStartTick = m_RoundStartTick;
	pGameInfoObj->m_WarmupTimer = m_Warmup;

	pGameInfoObj->m_ScoreLimit = g_Config.m_SvScorelimit;
	//pGameInfoObj->m_TimeLimit = g_Config.m_SvTimelimit;

	pGameInfoObj->m_RoundNum = /*(str_length(g_Config.m_SvMaprotation) && g_Config.m_SvRoundsPerMap) ? g_Config.m_SvRoundsPerMap :*/ 0;
	pGameInfoObj->m_RoundCurrent = m_RoundCount + 1;

	CCharacter *pChr;
	CPlayer *pPlayer = SnappingClient > -1 ? GameServer()->m_apPlayers[SnappingClient] : 0;
	CPlayer *pPlayer2;

	if (pPlayer && (pPlayer->m_TimerType == 0 || pPlayer->m_TimerType == 2))
	{
		if (pPlayer->m_TROLL166)
		{
			pGameInfoObj->m_RoundStartTick = Server()->Tick()-500001;
		}
		else if ((pPlayer->GetTeam() == -1 || pPlayer->m_Paused)
			&& pPlayer->m_SpectatorID != SPEC_FREEVIEW
			&& (pPlayer2 = GameServer()->m_apPlayers[pPlayer->m_SpectatorID]))
		{
			if ((pChr = pPlayer2->GetCharacter()))
				pGameInfoObj->m_RoundStartTick = (pChr->m_DDRaceState == DDRACE_STARTED) ? pChr->m_StartTime : m_RoundStartTick;
		}
		else if ((pChr = pPlayer->GetCharacter()))
		{
			pGameInfoObj->m_RoundStartTick = (pChr->m_DDRaceState == DDRACE_STARTED) ? pChr->m_StartTime : m_RoundStartTick;
		}
	}

	if (g_Config.m_SvDDPPgametype == 5)
		pGameInfoObj->m_ScoreLimit = g_Config.m_SvScorelimit;
	else if (pPlayer->IsInstagibMinigame())
	{
		if (pPlayer->m_IsInstaMode_fng)
		{
			if (pPlayer->m_IsInstaMode_idm)
				pGameInfoObj->m_ScoreLimit = g_Config.m_SvRifleScorelimit;
			else if (pPlayer->m_IsInstaMode_gdm)
				pGameInfoObj->m_ScoreLimit = g_Config.m_SvGrenadeScorelimit;
		}
	}
	else
		pGameInfoObj->m_ScoreLimit = 0;
}

int IGameController::GetAutoTeam(int NotThisID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	// this will force the auto balancer to work overtime aswell
	if (g_Config.m_DbgStress)
		return 0;

	int aNumplayers[2] = { 0,0 };
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (GameServer()->m_apPlayers[i] && i != NotThisID)
		{
			if (GameServer()->m_apPlayers[i]->GetTeam() >= TEAM_RED && GameServer()->m_apPlayers[i]->GetTeam() <= TEAM_BLUE)
				aNumplayers[GameServer()->m_apPlayers[i]->GetTeam()]++;
		}
	}

	int Team = 0;
	//if(IsTeamplay())
	//Team = aNumplayers[TEAM_RED] > aNumplayers[TEAM_BLUE] ? TEAM_BLUE : TEAM_RED;

	if (CanJoinTeam(Team, NotThisID))
		return Team;
	return -1;
}

bool IGameController::CanJoinTeam(int Team, int NotThisID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (Team == TEAM_SPECTATORS || (GameServer()->m_apPlayers[NotThisID] && GameServer()->m_apPlayers[NotThisID]->GetTeam() != TEAM_SPECTATORS))
		return true;

	int aNumplayers[2] = { 0,0 };
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (GameServer()->m_apPlayers[i] && i != NotThisID)
		{
			if (GameServer()->m_apPlayers[i]->GetTeam() >= TEAM_RED && GameServer()->m_apPlayers[i]->GetTeam() <= TEAM_BLUE)
				aNumplayers[GameServer()->m_apPlayers[i]->GetTeam()]++;
		}
	}

	return (aNumplayers[0] + aNumplayers[1]) < Server()->MaxClients() - g_Config.m_SvSpectatorSlots;
}

/*bool IGameController::CheckTeamBalance()
{
#if defined(CONF_DEBUG)
CALL_STACK_ADD();
#endif
if(!IsTeamplay() || !g_Config.m_SvTeambalanceTime)
return true;

int aT[2] = {0, 0};
for(int i = 0; i < MAX_CLIENTS; i++)
{
CPlayer *pP = GameServer()->m_apPlayers[i];
if(pP && pP->GetTeam() != TEAM_SPECTATORS)
aT[pP->GetTeam()]++;
}

char aBuf[256];
if(absolute(aT[0]-aT[1]) >= 2)
{
str_format(aBuf, sizeof(aBuf), "Teams are NOT balanced (red=%d blue=%d)", aT[0], aT[1]);
GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
if(GameServer()->m_pController->m_UnbalancedTick == -1)
GameServer()->m_pController->m_UnbalancedTick = Server()->Tick();
return false;
}
else
{
str_format(aBuf, sizeof(aBuf), "Teams are balanced (red=%d blue=%d)", aT[0], aT[1]);
GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
GameServer()->m_pController->m_UnbalancedTick = -1;
return true;
}
}

bool IGameController::CanChangeTeam(CPlayer *pPlayer, int JoinTeam)
{
#if defined(CONF_DEBUG)
CALL_STACK_ADD();
#endif
int aT[2] = {0, 0};

if (!IsTeamplay() || JoinTeam == TEAM_SPECTATORS || !g_Config.m_SvTeambalanceTime)
return true;

for(int i = 0; i < MAX_CLIENTS; i++)
{
CPlayer *pP = GameServer()->m_apPlayers[i];
if(pP && pP->GetTeam() != TEAM_SPECTATORS)
aT[pP->GetTeam()]++;
}

// simulate what would happen if changed team
aT[JoinTeam]++;
if (pPlayer->GetTeam() != TEAM_SPECTATORS)
aT[JoinTeam^1]--;

// there is a player-difference of at least 2
if(absolute(aT[0]-aT[1]) >= 2)
{
// player wants to join team with less players
if ((aT[0] < aT[1] && JoinTeam == TEAM_RED) || (aT[0] > aT[1] && JoinTeam == TEAM_BLUE))
return true;
else
return false;
}
else
return true;
}
*/

void IGameController::DoWincheck()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (m_GameOverTick == -1 && !m_Warmup && !GameServer()->m_World.m_ResetRequested)
	{
		if (IsTeamplay())
		{
			// check score win condition
			if ((g_Config.m_SvScorelimit > 0 && (m_aTeamscore[TEAM_RED] >= g_Config.m_SvScorelimit || m_aTeamscore[TEAM_BLUE] >= g_Config.m_SvScorelimit)) ||
				(g_Config.m_SvTimelimit > 0 && (Server()->Tick() - m_RoundStartTick) >= g_Config.m_SvTimelimit*Server()->TickSpeed() * 60))
			{
				if (m_aTeamscore[TEAM_RED] != m_aTeamscore[TEAM_BLUE])
					EndRound();
				else
					m_SuddenDeath = 1;
			}
		}
		else
		{
			// gather some stats
			int Topscore = 0;
			int TopscoreCount = 0;
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (GameServer()->m_apPlayers[i])
				{
					if (GameServer()->m_apPlayers[i]->m_Score > Topscore)
					{
						Topscore = GameServer()->m_apPlayers[i]->m_Score;
						TopscoreCount = 1;
					}
					else if (GameServer()->m_apPlayers[i]->m_Score == Topscore)
						TopscoreCount++;
				}
			}

			// check score win condition
			if ((g_Config.m_SvScorelimit > 0 && Topscore >= g_Config.m_SvScorelimit) ||
				(g_Config.m_SvTimelimit > 0 && (Server()->Tick() - m_RoundStartTick) >= g_Config.m_SvTimelimit*Server()->TickSpeed() * 60))
			{
				if (TopscoreCount == 1)
					EndRound();
				else
					m_SuddenDeath = 1;
			}
		}
	}
}

int IGameController::ClampTeam(int Team)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (Team < 0)
		return TEAM_SPECTATORS;
	//if(IsTeamplay())
	//return Team&1;
	return 0;
}
