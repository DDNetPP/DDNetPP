// gamecontext scoped ddnet++ methods

#include "ddpp/accounts.h"
#include "ddpp/loc.h"
#include "ddpp/shop.h"

#include <base/ddpp_logs.h>
#include <base/system_ddpp.h>
#include <cstdlib>
#include <engine/server/server.h>
#include <engine/shared/config.h>
#include <game/server/teams.h>

#include "game/generated/protocol.h"
#include "minigames/balance.h"
#include "minigames/block_tournament.h"
#include "minigames/instagib.h"

#include <cinttypes>
#include <cstring>
#include <fstream> //acc2 sys
#include <limits> //acc2 sys

#include "save.h"

#include <game/mapitems.h>

#include "gamecontext.h"

void CGameContext::ConstructDDPP()
{
	m_pShop = nullptr;
	m_pLoc = nullptr;
	m_pLetters = nullptr;
	m_pAccounts = nullptr;
	m_pBlockTournament = nullptr;
	m_pBalance = nullptr;
	m_pInstagib = nullptr;
	m_MapsavePlayers = 0;
	m_MapsaveLoadedPlayers = 0;
	m_vDropLimit.resize(2);
	m_BalanceId1 = -1;
	m_BalanceId2 = -1;
	m_survivalgamestate = 0;
	m_survival_game_countdown = 0;
	m_BlockWaveGameState = 0;
	m_insta_survival_gamestate = 0;
	m_CucumberShareValue = 10;
	m_BombTick = g_Config.m_SvBombTicks;
	m_BombStartCountDown = g_Config.m_SvBombStartDelay;
	m_WrongRconAttempts = 0;
	str_copy(m_aAllowedCharSet, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.:+@-_", sizeof(m_aAllowedCharSet));
	str_copy(m_aLastSurvivalWinnerName, "", sizeof(m_aLastSurvivalWinnerName));
	m_iBroadcastDelay = 0;
	m_BombGameState = 0;
	m_survivalgamestate = 0;
	m_BalanceBattleState = 0;
	m_CreateShopBot = false;
	m_InstaGrenadeRoundEndTickTicker = 0;
	m_InstaRifleRoundEndTickTicker = 0;
	m_NumLoginBans = 0;
	m_NumRegisterBans = 0;
	m_NumNameChangeMutes = 0;
	m_LastAccountMode = g_Config.m_SvAccountStuff;
	m_IsServerEmpty = false;
	m_IsPoliceFarmActive = true;
}

void CGameContext::DestructDDPP()
{
	if(m_pShop)
	{
		delete m_pShop;
		m_pShop = nullptr;
	}
	if(m_pLoc)
	{
		delete m_pLoc;
		m_pLoc = nullptr;
	}
	if(m_pLetters)
	{
		delete m_pLetters;
		m_pLetters = nullptr;
	}
	if(m_pAccounts)
	{
		delete m_pAccounts;
		m_pAccounts = nullptr;
	}
	for(auto &Minigame : m_vMinigames)
	{
		if(Minigame)
			Minigame->OnShutdown();
		if(Minigame)
			Minigame->CleanupMinigame();
		delete Minigame;
		Minigame = nullptr;
	}
}

bool CGameContext::DDPPOnMessage(int MsgId, void *pRawMsg, CUnpacker *pUnpacker, int ClientId)
{
	if(Server()->ClientIngame(ClientId))
	{
		if(MsgId == NETMSGTYPE_SV_MODIFYTILE)
		{
			if(!g_Config.m_SvMineTeeEditor)
				return true;

			CNetMsg_Sv_ModifyTile *pMsg = (CNetMsg_Sv_ModifyTile *)pRawMsg;
			Collision()->ModifyTile(pMsg->m_X, pMsg->m_Y, pMsg->m_Group, pMsg->m_Layer, pMsg->m_Index, pMsg->m_Flags);
			Server()->SendPackMsg(pMsg, MSGFLAG_VITAL, -1);
		}
	}
	return true;
}

int CGameContext::AmountPoliceFarmPlayers()
{
	int Num = 0;
	for(const auto &pPlayer : m_apPlayers)
	{
		if(!pPlayer)
			continue;
		if(!pPlayer->GetCharacter())
			continue;
		if(pPlayer->GetCharacter()->m_OnMoneytile == CCharacter::MONEYTILE_POLICE)
			Num++;
	}
	return Num;
}

void CGameContext::CheckDeactivatePoliceFarm()
{
	if(!g_Config.m_SvMaxPoliceFarmPlayers)
	{
		m_IsPoliceFarmActive = true;
		return;
	}
	if(Server()->Tick() % 50)
		return;

	// dbg_msg("check", "%d %d",  AmountPoliceFarmPlayers(), g_Config.m_SvMaxPoliceFarmPlayers);
	m_IsPoliceFarmActive = AmountPoliceFarmPlayers() < g_Config.m_SvMaxPoliceFarmPlayers;
}

void CGameContext::SetSpawnweapons(bool Active, int ClientId)
{
	if(ClientId < 0 || ClientId > MAX_CLIENTS)
		return;
	CPlayer *pPlayer = m_apPlayers[ClientId];
	if(!pPlayer)
		return;

	if(Active)
	{
		if(!g_Config.m_SvAllowSpawnWeapons)
		{
			SendChatTarget(ClientId, "Spawn weapons are deactivated by an administrator.");
			return;
		}

		if((!pPlayer->m_Account.m_SpawnWeaponShotgun) && (!pPlayer->m_Account.m_SpawnWeaponGrenade) && (!pPlayer->m_Account.m_SpawnWeaponRifle))
		{
			SendChatTarget(ClientId, "You don't have any spawn weapons.");
			return;
		}
	}

	// only print messages on changed state
	if(pPlayer->m_Account.m_UseSpawnWeapons == Active)
		return;

	if(!pPlayer->m_Account.m_UseSpawnWeapons)
	{
		SendChatTarget(ClientId, "Spawn weapons activated. Use '/spawnweapons' to toggle.");
	}
	else
	{
		SendChatTarget(ClientId, "Spawn weapons deactivated. Use '/spawnweapons' to toggle.");
	}

	pPlayer->m_Account.m_UseSpawnWeapons = Active;
}

void CGameContext::LoadMapLive(const char *pMapName)
{
	int LoadMap = Server()->LoadMapLive(pMapName);
	dbg_msg("live-map", "loadmap=%d", LoadMap);
	m_Layers.Init(Kernel());
	m_Collision.Init(&m_Layers);
}

void CGameContext::QueueTileForModify(int Group, int Layer, int Index, int Flags, int X, int Y)
{
	m_PendingModifyTilesMutex.lock();
	m_vPendingModifyTiles.emplace_back(Group, Layer, Index, Flags, X, Y);
	m_PendingModifyTilesMutex.unlock();
}

// should be run in a thread
void CGameContext::ModifyTileWorker()
{
	// m_PendingModifyTilesMutex.lock();
	// // TODO: copy the pending tiles to not hold the lock while processing
	// m_PendingModifyTilesMutex.unlock();

	// for (auto Tile : m_vPendingModifyTilesCopy)
	// {
	// 	std::system("./ddpp_scripts/map_set_tiles.py ....");
	// 	// m_vFinishedModifyTilesCopy.emplace_back(Tile);
	// }

	// m_FinishedModifyTilesMutex.lock();
	// // TODO: copy the finised tiles to not hold the lock while processing
	// m_FinishedModifyTilesMutex.unlock();
}

const char *CGameContext::Loc(const char *pStr, int ClientId)
{
	if(m_pLoc)
		return m_pLoc->DDPPLocalize(pStr, ClientId);
	return pStr;
}

void CGameContext::OnInitDDPP()
{
	if(!m_pAccounts)
		m_pAccounts = new CAccounts(this, ((CServer *)Server())->DDPPDbPool());
	if(!m_pShop)
		m_pShop = new CShop(this);
	if(!m_pLoc)
		m_pLoc = new CLoc(this);
	if(!m_pLetters)
		m_pLetters = new CLetters(this);

	if(!m_pBlockTournament)
		m_pBlockTournament = new CBlockTournament(this);
	if(!m_pBalance)
		m_pBalance = new CBalance(this);
	if(!m_pInstagib)
		m_pInstagib = new CInstagib(this);
	m_vMinigames.push_back(m_pBlockTournament);
	m_vMinigames.push_back(m_pBalance);
	m_vMinigames.push_back(m_pInstagib);

	for(auto &Minigame : m_vMinigames)
		Minigame->OnInit();

	LoadFNNvalues();
	m_pAccounts->CreateDatabase();
	char aBuf[512];
	str_copy(aBuf,
		"UPDATE Accounts SET IsLoggedIn = 0, LastLoginPort = ?;",
		sizeof(aBuf));
	m_pAccounts->CleanZombieAccounts(-1, g_Config.m_SvPort, aBuf);

	LoadSinglePlayer();
	//dummy_init
	if(g_Config.m_SvBasicDummys)
		CreateBasicDummys();
}

void CGameContext::OnClientEnterDDPP(int ClientId)
{
	if(IsDDPPgametype("survival"))
	{
		SetPlayerSurvival(ClientId, 1);
	}
	else if(IsDDPPgametype("vanilla"))
	{
		if(m_apPlayers[ClientId])
		{
			m_apPlayers[ClientId]->m_IsVanillaDmg = true;
			m_apPlayers[ClientId]->m_IsVanillaWeapons = true;
			m_apPlayers[ClientId]->m_IsVanillaCompetetive = true;
		}
	}
	else if(IsDDPPgametype("fng"))
	{
		if(m_apPlayers[ClientId])
		{
			m_apPlayers[ClientId]->m_IsInstaMode_idm = true;
			m_apPlayers[ClientId]->m_IsInstaMode_fng = true;
		}
	}
	InitDDPPScore(ClientId);
	CheckServerEmpty();
	CMsgPacker Msg(NETMSG_DDNETPP);
	Msg.AddInt(0); // ddnet++ feature flags1
	Msg.AddInt(0); // ddnet++ feature flags2
	Msg.AddInt(0); // ddnet++ allow flags
	char aBuf[512];
	str_format(
		aBuf,
		sizeof(aBuf),
		"DDNet++ %s (built on %s, git rev %s)",
		DDNETPP_VERSIONSTR,
		DDNETPP_BUILD_DATE,
		GIT_SHORTREV_HASH);
	Msg.AddString(aBuf);
	Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientId);
}

void CGameContext::InitDDPPScore(int ClientId)
{
	if(IsDDPPgametype("block"))
	{
		if(m_apPlayers[ClientId])
			m_apPlayers[ClientId]->m_Score = 0;
	}
	if(g_Config.m_SvDDPPscore == 0)
		if(m_apPlayers[ClientId])
			m_apPlayers[ClientId]->m_Score = 0;
}

bool CGameContext::InitTileDDPP(int Index, int x, int y)
{
	if(Index == TILE_JAIL)
	{
		CJail Jail;
		Jail.m_Center = vec2(x, y);
		dbg_msg("game layer", "got Jail tile at (%.2f|%.2f)", Jail.m_Center.x, Jail.m_Center.y);
		m_Jail.push_back(Jail);
	}
	else if(Index == TILE_JAILRELEASE)
	{
		CJailrelease Jailrelease;
		Jailrelease.m_Center = vec2(x, y);
		dbg_msg("game layer", "got Jailrelease tile at (%.2f|%.2f)", Jailrelease.m_Center.x, Jailrelease.m_Center.y);
		m_Jailrelease.push_back(Jailrelease);
	}
	else if(Index == TILE_BALANCE_BATTLE_1)
	{
		CBalanceBattleTile1 Balancebattle;
		Balancebattle.m_Center = vec2(x, y);
		dbg_msg("game layer", "got balancebattle1 tile at (%.2f|%.2f)", Balancebattle.m_Center.x, Balancebattle.m_Center.y);
		m_BalanceBattleTile1.push_back(Balancebattle);
	}
	else if(Index == TILE_BALANCE_BATTLE_2)
	{
		CBalanceBattleTile2 Balancebattle;
		Balancebattle.m_Center = vec2(x, y);
		dbg_msg("game layer", "got balancebattle2 tile at (%.2f|%.2f)", Balancebattle.m_Center.x, Balancebattle.m_Center.y);
		m_BalanceBattleTile2.push_back(Balancebattle);
	}
	else if(Index == TILE_SURVIVAL_LOBBY)
	{
		CSurvivalLobbyTile Survivallobby;
		Survivallobby.m_Center = vec2(x, y);
		dbg_msg("game layer", "got survival lobby tile at (%.2f|%.2f)", Survivallobby.m_Center.x, Survivallobby.m_Center.y);
		m_SurvivalLobby.push_back(Survivallobby);
	}
	else if(Index == TILE_SURVIVAL_SPAWN)
	{
		CSurvivalSpawnTile Survivalspawn;
		Survivalspawn.m_Center = vec2(x, y);
		dbg_msg("game layer", "got survival spawn tile at (%.2f|%.2f)", Survivalspawn.m_Center.x, Survivalspawn.m_Center.y);
		m_SurvivalSpawn.push_back(Survivalspawn);
	}
	else if(Index == TILE_SURVIVAL_DEATHMATCH)
	{
		CSurvivalDeathmatchTile Survivaldeathmatch;
		Survivaldeathmatch.m_Center = vec2(x, y);
		dbg_msg("game layer", "got survival deathmatch tile at (%.2f|%.2f)", Survivaldeathmatch.m_Center.x, Survivaldeathmatch.m_Center.y);
		m_SurvivalDeathmatch.push_back(Survivaldeathmatch);
	}
	else if(Index == TILE_BLOCKWAVE_BOT)
	{
		CBlockWaveBotTile BlockWaveBot;
		BlockWaveBot.m_Center = vec2(x, y);
		dbg_msg("game layer", "got blockwave bot spawn tile at (%.2f|%.2f)", BlockWaveBot.m_Center.x, BlockWaveBot.m_Center.y);
		m_BlockWaveBot.push_back(BlockWaveBot);
	}
	else if(Index == TILE_BLOCKWAVE_HUMAN)
	{
		CBlockWaveHumanTile BlockWaveHuman;
		BlockWaveHuman.m_Center = vec2(x, y);
		dbg_msg("game layer", "got blockwave Human spawn tile at (%.2f|%.2f)", BlockWaveHuman.m_Center.x, BlockWaveHuman.m_Center.y);
		m_BlockWaveHuman.push_back(BlockWaveHuman);
	}
	else if(Index == TILE_FNG_SCORE)
	{
		CFngScore FngScore;
		FngScore.m_Center = vec2(x, y);
		dbg_msg("game layer", "got fng score tile at (%.2f|%.2f)", FngScore.m_Center.x, FngScore.m_Center.y);
		m_FngScore.push_back(FngScore);
	}
	else if(Index == TILE_BLOCK_TOURNA_SPAWN)
	{
		CBlockTournaSpawn BlockTournaSpawn;
		BlockTournaSpawn.m_Center = vec2(x, y);
		dbg_msg("game layer", "got fng score tile at (%.2f|%.2f)", BlockTournaSpawn.m_Center.x, BlockTournaSpawn.m_Center.y);
		m_BlockTournaSpawn.push_back(BlockTournaSpawn);
	}
	else if(Index == TILE_PVP_ARENA_SPAWN)
	{
		CPVPArenaSpawn PVPArenaSpawn;
		PVPArenaSpawn.m_Center = vec2(x, y);
		dbg_msg("game layer", "got pvp arena spawn tile at (%.2f|%.2f)", PVPArenaSpawn.m_Center.x, PVPArenaSpawn.m_Center.y);
		m_PVPArenaSpawn.push_back(PVPArenaSpawn);
	}
	else if(Index == TILE_VANILLA_MODE)
	{
		CVanillaMode VanillaMode;
		VanillaMode.m_Center = vec2(x, y);
		dbg_msg("game layer", "got vanilla mode tile at (%.2f|%.2f)", VanillaMode.m_Center.x, VanillaMode.m_Center.y);
		m_VanillaMode.push_back(VanillaMode);
	}
	else if(Index == TILE_DDRACE_MODE)
	{
		CDDraceMode DDraceMode;
		DDraceMode.m_Center = vec2(x, y);
		dbg_msg("game layer", "got ddrace mode tile at (%.2f|%.2f)", DDraceMode.m_Center.x, DDraceMode.m_Center.y);
		m_DDraceMode.push_back(DDraceMode);
	}
	else if(Index == TILE_BOTSPAWN_1)
	{
		CBotSpawn1 BotSpawn1;
		BotSpawn1.m_Center = vec2(x, y);
		dbg_msg("game layer", "got botspawn1 tile at (%.2f|%.2f)", BotSpawn1.m_Center.x, BotSpawn1.m_Center.y);
		m_BotSpawn1.push_back(BotSpawn1);
	}
	else if(Index == TILE_BOTSPAWN_2)
	{
		CBotSpawn2 BotSpawn2;
		BotSpawn2.m_Center = vec2(x, y);
		dbg_msg("game layer", "got botspawn2 tile at (%.2f|%.2f)", BotSpawn2.m_Center.x, BotSpawn2.m_Center.y);
		m_BotSpawn2.push_back(BotSpawn2);
	}
	else if(Index == TILE_BOTSPAWN_3)
	{
		CBotSpawn3 BotSpawn3;
		BotSpawn3.m_Center = vec2(x, y);
		dbg_msg("game layer", "got botspawn3 tile at (%.2f|%.2f)", BotSpawn3.m_Center.x, BotSpawn3.m_Center.y);
		m_BotSpawn3.push_back(BotSpawn3);
	}
	else if(Index == TILE_BOTSPAWN_4)
	{
		CBotSpawn4 BotSpawn4;
		BotSpawn4.m_Center = vec2(x, y);
		dbg_msg("game layer", "got botspawn4 tile at (%.2f|%.2f)", BotSpawn4.m_Center.x, BotSpawn4.m_Center.y);
		m_BotSpawn4.push_back(BotSpawn4);
	}
	else if(Index == TILE_NO_HAMMER)
	{
		CNoHammer NoHammer;
		NoHammer.m_Center = vec2(x, y);
		dbg_msg("game layer", "got no hammer tile at (%.2f|%.2f)", NoHammer.m_Center.x, NoHammer.m_Center.y);
		m_NoHammer.push_back(NoHammer);
	}
	else if(Index == TILE_BLOCK_DM_A1)
	{
		CBlockDMA1 BlockDMA1;
		BlockDMA1.m_Center = vec2(x, y);
		dbg_msg("game layer", "got block deathmatch(1) tile at (%.2f|%.2f)", BlockDMA1.m_Center.x, BlockDMA1.m_Center.y);
		m_BlockDMA1.push_back(BlockDMA1);
	}
	else if(Index == TILE_BLOCK_DM_A2)
	{
		CBlockDMA2 BlockDMA2;
		BlockDMA2.m_Center = vec2(x, y);
		dbg_msg("game layer", "got block deathmatch(2) tile at (%.2f|%.2f)", BlockDMA2.m_Center.x, BlockDMA2.m_Center.y);
		m_BlockDMA2.push_back(BlockDMA2);
	}
	else
		return false;
	return true;
}

bool CGameContext::CheckAccounts(int AccountId)
{
	for(auto &Player : m_apPlayers)
	{
		if(!Player)
			continue;

		if(Player->GetAccId() == AccountId)
			return true;
	}
	return false;
}

int CGameContext::GetNextClientId()
{
	int ClientId = -1;
	for(int i = 0; i < g_Config.m_SvMaxClients; i++)
	{
		if(m_apPlayers[i])
			continue;

		ClientId = i;
		break;
	}

	return ClientId;
}

//void CGameContext::OnDDPPshutdown()
//{
//#if defined(CONF_DEBUG)
//#endif
//	SendChat(-1, TEAM_ALL, "[DDNet++] server shutdown!");
//}

void CGameContext::AbuseMotd(const char *pMsg, int ClientId)
{
	if(m_apPlayers[ClientId])
	{
		m_apPlayers[ClientId]->m_IsFakeMotd = true;
	}
	// send motd
	CNetMsg_Sv_Motd Msg;
	Msg.m_pMessage = pMsg;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientId);
}

bool CGameContext::IsDDPPgametype(const char *pGametype)
{
	return !str_comp_nocase(g_Config.m_SvDDPPgametype, pGametype);
}

int CGameContext::GetCidByName(const char *pName)
{
	int NameId = -1;
	for(auto &Player : m_apPlayers)
	{
		if(!Player)
			continue;

		if(!str_comp(pName, Server()->ClientName(Player->GetCid())))
		{
			NameId = Player->GetCid();
			break;
		}
	}
	return NameId;
}

int CGameContext::GetShopBot()
{
	for(auto &Player : m_apPlayers)
	{
		if(Player)
		{
			if(Player->DummyMode() == DUMMYMODE_SHOPBOT)
			{
				return Player->GetCid();
			}
		}
	}
	return -1;
}

int CGameContext::CountConnectedPlayers()
{
	int Num = 0;
	for(auto &Player : m_apPlayers)
		if(Player)
			Num++;
	return Num;
}

int CGameContext::CountConnectedHumans()
{
	int cPlayers = 0;
	for(auto &Player : m_apPlayers)
	{
		if(Player && !Player->m_IsDummy)
		{
			cPlayers++;
		}
	}
	return cPlayers;
}

int CGameContext::CountIngameHumans()
{
	int cPlayers = 0;
	for(auto &Player : m_apPlayers)
	{
		if(Player && Player->GetCharacter() && !Player->m_IsDummy)
		{
			cPlayers++;
		}
	}
	return cPlayers;
}

bool CGameContext::IsAllowedCharSet(const char *pStr)
{
#if defined(CONF_DEBUG)
#endif
	int i = 0;
	bool IsOk = false;
	//dbg_msg("AllowedChar", "checking str '%s'", pStr);

	while(true)
	{
		IsOk = false;
		for(int j = 0; j < str_length(m_aAllowedCharSet); j++)
		{
			if(pStr[i] == m_aAllowedCharSet[j])
			{
				//dbg_msg("AllowedChar","found valid char '%c' - '%c'", pStr[i], m_aAllowedCharSet[j]);
				IsOk = true;
				break;
			}
		}

		if(!IsOk)
		{
			//dbg_msg("AllowedChar", "found evil char '%c'", pStr[i]);
			return false;
		}
		i++;
		if(pStr[i] == '\0')
		{
			//dbg_msg("AllowedChar", "string ends at %d", i);
			return true;
		}
	}
	return true;
}

int CGameContext::GetPlayerByTimeoutcode(const char *pTimeout)
{
	for(auto &Player : m_apPlayers)
	{
		if(!Player)
			continue;
		if(!Player->m_aTimeoutCode[0])
			continue;
		if(str_comp(Player->m_aTimeoutCode, pTimeout))
			continue;
		return Player->GetCid();
	}
	return -1;
}

int CGameContext::CountConnectedBots()
{
	int lum_tt_zv_1_zz_04032018_lt3 = 0;
	for(auto &Player : m_apPlayers)
	{
		if(Player && Player->m_IsDummy)
		{
			lum_tt_zv_1_zz_04032018_lt3++;
		}
	}
	return lum_tt_zv_1_zz_04032018_lt3;
}

int CGameContext::CountTimeoutCodePlayers()
{
	int p = 0;
	for(auto &Player : m_apPlayers)
	{
		if(!Player)
			continue;
		if(!Player->m_aTimeoutCode[0])
			continue;
		p++;
	}
	return p;
}

void CGameContext::SendBroadcastAll(const char *pText, int importance, bool supermod)
{
	for(auto &Player : m_apPlayers)
	{
		if(Player)
		{
			SendBroadcast(pText, Player->GetCid(), importance, supermod);
		}
	}
}

void CGameContext::KillAll()
{
	for(auto &Player : m_apPlayers)
	{
		if(Player && Player->GetCharacter()) //only kill alive dudes
		{
			GetPlayerChar(Player->GetCid())->Die(Player->GetCid(), WEAPON_WORLD);
		}
	}
}

void CGameContext::LoadFNNvalues()
{
	std::ifstream readfile;
	char aFilePath[512];
	str_format(aFilePath, sizeof(aFilePath), "FNN/move_stats.fnn");
	readfile.open(aFilePath);
	if(readfile.is_open())
	{
		std::string line;

		std::getline(readfile, line); //distance
		m_FNN_best_distance = atoi(line.c_str());

		std::getline(readfile, line); //fitness
		m_FNN_best_fitness = atoi(line.c_str());

		std::getline(readfile, line); //distance_finish
		m_FNN_best_distance_finish = atoi(line.c_str());
	}
	else
	{
		m_FNN_best_distance = -9999999;
		m_FNN_best_fitness = -9999999;
		m_FNN_best_distance_finish = 9999999;
		dbg_msg("FNN", "LoadFNNvalues() error failed to load best stats. failed to open '%s'", aFilePath);
	}
}

bool CGameContext::IsPosition(int playerId, int pos)
{
#if defined(CONF_DEBUG)
	//dbg_msg("debug", "IsPosition(playerId = %d, pos = %d)", playerId, pos);
#endif
	if(!m_apPlayers[playerId])
	{
		return false;
	}
	if(!GetPlayerChar(playerId))
	{
		return false;
	}

	if(pos == 0) //cb5 jail release spot
	{
		if(GetPlayerChar(playerId)->m_Pos.x > 480 * 32 && GetPlayerChar(playerId)->m_Pos.x < 500 * 32 && GetPlayerChar(playerId)->m_Pos.y > 229 * 32 && GetPlayerChar(playerId)->m_Pos.y < 237 * 32)
		{
			return true;
		}
	}
	//else if (pos == 1) //cb5 spawn
	//{
	//	if (GetPlayerChar(playerId)->m_Pos.x > 325 * 32
	//		&& GetPlayerChar(playerId)->m_Pos.x < 362 * 32
	//		&& GetPlayerChar(playerId)->m_Pos.y > 191 * 32
	//		&& GetPlayerChar(playerId)->m_Pos.y < 206 * 32)
	//	{
	//		return true;
	//	}
	//}
	else if(pos == 2) //cb5 far in map (block area and race)
	{
		if(GetPlayerChar(playerId)->m_Pos.x > 415 * 32)
		{
			return true;
		}
	}
	else if(pos == 3) //configurated spawn area
	{
		if(GetPlayerChar(playerId)->m_Pos.x > g_Config.m_SvSpawnareaLowX * 32 && GetPlayerChar(playerId)->m_Pos.x < g_Config.m_SvSpawnareaHighX * 32 && GetPlayerChar(playerId)->m_Pos.y > g_Config.m_SvSpawnareaLowY * 32 && GetPlayerChar(playerId)->m_Pos.y < g_Config.m_SvSpawnareaHighY * 32)
		{
			return true;
		}
	}

	return false;
}

void CGameContext::StartAsciiAnimation(int viewerId, int creatorId, int medium)
{
	if(!m_apPlayers[viewerId])
		return;
	if(!m_apPlayers[creatorId])
	{
		SendChatTarget(viewerId, "player not found.");
		return;
	}
	//dont start new animation while old is running
	if(m_apPlayers[viewerId]->m_AsciiWatchingId != -1)
	{
		return;
	}

	if(medium == 0) // '/ascii view <cid>'
	{
		if(m_apPlayers[creatorId]->m_Account.m_aAsciiPublishState[0] == '0')
		{
			SendChatTarget(viewerId, "ascii art not public.");
			return;
		}

		m_apPlayers[creatorId]->m_AsciiViewsDefault++;
		//COULDDO: code: cfv45
	}
	else if(medium == 1) // '/profile view <player>'
	{
		if(m_apPlayers[creatorId]->m_Account.m_aAsciiPublishState[1] == '0')
		{
			//SendChatTarget(viewerId, "ascii art not published on profile");
			return;
		}

		m_apPlayers[creatorId]->m_AsciiViewsProfile++;
	}
	else if(medium == 2) // not used yet
	{
		if(m_apPlayers[creatorId]->m_Account.m_aAsciiPublishState[2] == '0')
		{
			SendChatTarget(viewerId, "ascii art not published on medium 2");
			return;
		}
	}

	m_apPlayers[viewerId]->m_AsciiWatchingId = creatorId;
}

bool CGameContext::IsHooked(int hookedId, int power)
{
	for(auto &Player : m_apPlayers)
	{
		if(!Player)
			continue;

		CCharacter *pChar = GetPlayerChar(Player->GetCid());

		if(!pChar || !pChar->IsAlive() || pChar->GetPlayer()->GetCid() == hookedId)
			continue;
		if(pChar->Core()->HookedPlayer() == hookedId && pChar->GetPlayer()->m_HookPower == power)
		{
			return true;
		}
	}

	return false;
}

bool CGameContext::IsSameIp(int Id1, int Id2)
{
	char aIp1[64];
	char aIp2[64];
	Server()->GetClientAddr(Id1, aIp1, sizeof(aIp1));
	Server()->GetClientAddr(Id2, aIp2, sizeof(aIp2));
	return !str_comp_nocase(aIp1, aIp2);
}

char CGameContext::BoolToChar(bool b)
{
	if(b)
		return '1';
	return '0';
}

bool CGameContext::CharToBool(char c)
{
	return c != '0';
}

void CGameContext::ShowHideConfigBoolToChar(int id)
{
	CPlayer *pPlayer = m_apPlayers[id];
	if(!pPlayer)
		return;
	//[0] = blockpoints [1] = blockxp [2] = xp [3] = jail [4] = instafeed(1n1) [5] = questprogress [6] = questwarning
	pPlayer->m_Account.m_aShowHideConfig[0] = BoolToChar(pPlayer->m_ShowBlockPoints);
	pPlayer->m_Account.m_aShowHideConfig[1] = BoolToChar(pPlayer->m_HideBlockXp);
	pPlayer->m_Account.m_aShowHideConfig[2] = BoolToChar(pPlayer->m_xpmsg);
	pPlayer->m_Account.m_aShowHideConfig[3] = BoolToChar(pPlayer->m_hidejailmsg);
	pPlayer->m_Account.m_aShowHideConfig[4] = BoolToChar(pPlayer->m_HideInsta1on1_killmessages);
	pPlayer->m_Account.m_aShowHideConfig[5] = BoolToChar(pPlayer->m_HideQuestProgress);
	pPlayer->m_Account.m_aShowHideConfig[6] = BoolToChar(pPlayer->m_HideQuestWarning);
	pPlayer->m_Account.m_aShowHideConfig[7] = '\0';
#if defined(CONF_DEBUG)
	//dbg_msg("BoolToChar", "UPDATED ShowHideChar='%s'", pPlayer->m_Account.m_aShowHideConfig);
#endif
}

void CGameContext::ShowHideConfigCharToBool(int id)
{
	CPlayer *pPlayer = m_apPlayers[id];
	if(!pPlayer)
		return;
	//[0] = blockpoints [1] = blockxp [2] = xp [3] = jail [4] = instafeed(1n1) [5] = questprogress [6] = questwarning
	pPlayer->m_ShowBlockPoints = CharToBool(pPlayer->m_Account.m_aShowHideConfig[0]);
	pPlayer->m_HideBlockXp = CharToBool(pPlayer->m_Account.m_aShowHideConfig[1]);
	pPlayer->m_xpmsg = CharToBool(pPlayer->m_Account.m_aShowHideConfig[2]);
	pPlayer->m_hidejailmsg = CharToBool(pPlayer->m_Account.m_aShowHideConfig[3]);
	pPlayer->m_HideInsta1on1_killmessages = CharToBool(pPlayer->m_Account.m_aShowHideConfig[4]);
	pPlayer->m_HideQuestProgress = CharToBool(pPlayer->m_Account.m_aShowHideConfig[5]);
	pPlayer->m_HideQuestWarning = CharToBool(pPlayer->m_Account.m_aShowHideConfig[6]);
#if defined(CONF_DEBUG)
	/*
	dbg_msg("CharToBool", "ShowHideChar='%s'", pPlayer->m_Account.m_aShowHideConfig);
	dbg_msg("ShowHide", "BlockPoints	: %d", pPlayer->m_ShowBlockPoints);
	dbg_msg("ShowHide", "BlockXp		: %d", pPlayer->m_HideBlockXp);
	dbg_msg("ShowHide", "Xp				: %d", pPlayer->m_xpmsg);
	dbg_msg("ShowHide", "Jail			: %d", pPlayer->m_hidejailmsg);
	dbg_msg("ShowHide", "insta1n1		: %d", pPlayer->m_HideInsta1on1_killmessages);
	dbg_msg("ShowHide", "questprogress	: %d", pPlayer->m_HideQuestProgress);
	dbg_msg("ShowHide", "questwarning	: %d", pPlayer->m_HideQuestWarning);
	*/
#endif
}

void CGameContext::FNN_LoadRun(const char *path, int botId)
{
	CPlayer *pPlayer = m_apPlayers[botId];
	if(!pPlayer)
	{
		dbg_msg("FNN", "failed to load run player with id=%d doesn't exist", botId);
		return;
	}
	CCharacter *pChr = GetPlayerChar(botId);
	if(!pChr)
	{
		dbg_msg("FNN", "failed to load run character with id=%d, name=%s doesn't exist", botId, Server()->ClientName(botId));
		return;
	}
	if(pPlayer->DummyMode() != DUMMYMODE_FNN)
		return;

	CDummyFNN *pDummyFNN = (CDummyFNN *)pPlayer->m_pDummyMode;

	//reset values
	pDummyFNN->m_FNN_CurrentMoveIndex = 0;
	float loaded_distance = 0;
	float loaded_fitness = 0;
	float loaded_distance_finish = 0;
	char aBuf[128];

	//load run
	std::ifstream readfile;
	char aFilePath[512];
	str_copy(aFilePath, path, sizeof(aFilePath));
	readfile.open(aFilePath);
	if(readfile.is_open())
	{
		std::string line;
		int i = 0;

		//first four five are stats:
		std::getline(readfile, line); // read but ignore header

		std::getline(readfile, line); //moveticks
		pDummyFNN->m_FNN_ticks_loaded_run = atoi(line.c_str());

		std::getline(readfile, line); //distance
		loaded_distance = atof(line.c_str());

		std::getline(readfile, line); //fitness
		loaded_fitness = atof(line.c_str());

		std::getline(readfile, line); //distance_finish
		loaded_distance_finish = atof(line.c_str());

		while(std::getline(readfile, line))
		{
			pDummyFNN->m_aRecMove[i] = atoi(line.c_str());
			i++;
		}
	}
	else
	{
		dbg_msg("FNN", "failed to load move. failed to open '%s'", aFilePath);
		pPlayer->m_dmm25 = -1;
	}

	//start run
	pPlayer->m_dmm25 = 4; //replay submode
	str_format(aBuf, sizeof(aBuf), "[FNN] loaded run with ticks=%d distance=%.2f fitness=%.2f distance_finish=%.2f", pDummyFNN->m_FNN_ticks_loaded_run, loaded_distance, loaded_fitness, loaded_distance_finish);
	SendChat(botId, TEAM_ALL, aBuf);
}

void CGameContext::TestPrintTiles(int botId)
{
	CPlayer *pPlayer = m_apPlayers[botId];
	if(!pPlayer)
		return;
	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	//moved to character
}

vec2 CGameContext::GetFinishTile()
{
	/*
	int BIG_NUMBER = 0;
	BIG_NUMBER = ~BIG_NUMBER; //binary lyfe hacks with chiller
	int TODO = BIG_NUMBER; //find a better way maybe actual map index size or something

	for (int i = 0; i < TODO; i++)
	{
		if (Collision()->TileExists(i))
		{
			if (Collision()->GetTileIndex(i) == TILE_END)
			{
				dbg_msg("tile-finder","found finish tile at index=%d",i);
				return i;
			}
		}
	}
	*/

	/*
	for (int i = 0; i < Collision()->GetWidth() * Collision()->GetHeight(); i++)
	{
		if (GameServer()->m_pTiles[i].m_Index == TILE_END)
		{
			dbg_msg("tile-finder", "found finish tile at index=%d", i);
			return i;
		}
	}
	*/

	/*
	int Width = Collision()->GetWidth();
	int Height = Collision()->GetHeight();
	for (int i = 0; i < Width; i++)
	{
		for (int j = 0; j < Height; i++)
		{
			if (Collision()->GetTile(i, j) == TILE_END || Collision()->GetFTile(i, j) == TILE_END)
			{
				dbg_msg("tile-finder", "found finish tile at (%d/%d)", i,j);
				return vec2(i, j);
			}
		}
	}
	*/

	int Width = Collision()->GetWidth();
	int Height = Collision()->GetHeight();
	for(int i = 0; i < Width * Height; i++)
	{
		if(Collision()->GetTileIndex(i) == TILE_FINISH || Collision()->GetFTileIndex(i) == TILE_FINISH)
		{
			/*
			dbg_msg("tile-finder", "found finish tile at index=%d", i);
			dbg_msg("tile-finder", "height: %d", Height);
			dbg_msg("tile-finder", "width: %d", Width);
			dbg_msg("tile-finder", "x: %d", i % Width);
			dbg_msg("tile-finder", "y: %d", int(i / Width));
			*/
			return vec2(i % Width, int(i / Width));
		}
	}

	return vec2(0, 0);
}

void CGameContext::ShowInstaStats(int requestId, int requestedId)
{
	if(!m_apPlayers[requestId])
		return;
	CPlayer *pPlayer = m_apPlayers[requestedId];
	if(!pPlayer)
		return;

	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "~~~ '%s's Grenade instagib ~~~", Server()->ClientName(pPlayer->GetCid()));
	SendChatTarget(requestId, aBuf);
	str_format(aBuf, sizeof(aBuf), "Kills: %d", pPlayer->m_Account.m_GrenadeKills);
	SendChatTarget(requestId, aBuf);
	str_format(aBuf, sizeof(aBuf), "Deaths: %d", pPlayer->m_Account.m_GrenadeDeaths);
	SendChatTarget(requestId, aBuf);
	str_format(aBuf, sizeof(aBuf), "Highest spree: %d", pPlayer->m_Account.m_GrenadeSpree);
	SendChatTarget(requestId, aBuf);
	str_format(aBuf, sizeof(aBuf), "Total shots: %d", pPlayer->m_Account.m_GrenadeShots);
	SendChatTarget(requestId, aBuf);
	str_format(aBuf, sizeof(aBuf), "Shots without RJ: %d", pPlayer->m_Account.m_GrenadeShotsNoRJ);
	SendChatTarget(requestId, aBuf);
	str_format(aBuf, sizeof(aBuf), "Rocketjumps: %d", pPlayer->m_Account.m_GrenadeShots - pPlayer->m_Account.m_GrenadeShotsNoRJ);
	SendChatTarget(requestId, aBuf);
	//str_format(aBuf, sizeof(aBuf), "Failed shots (no kill, no rj): %d", pPlayer->m_GrenadeShots - (pPlayer->m_GrenadeShots - pPlayer->m_GrenadeShotsNoRJ) - pPlayer->m_Account.m_GrenadeKills); //can be negative with double and tripple kills but this isnt a bug its a feature xd
	//SendChatTarget(requestId, aBuf);
	str_format(aBuf, sizeof(aBuf), "~~~ '%s's Rifle instagib ~~~", Server()->ClientName(pPlayer->GetCid()));
	SendChatTarget(requestId, aBuf);
	str_format(aBuf, sizeof(aBuf), "Kills: %d", pPlayer->m_Account.m_RifleKills);
	SendChatTarget(requestId, aBuf);
	str_format(aBuf, sizeof(aBuf), "Deaths: %d", pPlayer->m_Account.m_RifleDeaths);
	SendChatTarget(requestId, aBuf);
	str_format(aBuf, sizeof(aBuf), "Highest spree: %d", pPlayer->m_Account.m_RifleSpree);
	SendChatTarget(requestId, aBuf);
	str_format(aBuf, sizeof(aBuf), "Total shots: %d", pPlayer->m_Account.m_RifleShots);
	SendChatTarget(requestId, aBuf);
}

void CGameContext::ShowSurvivalStats(int requestId, int requestedId)
{
	if(!m_apPlayers[requestId])
		return;
	CPlayer *pPlayer = m_apPlayers[requestedId];
	if(!pPlayer)
		return;

	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "~~~ '%s's survival stats ~~~", Server()->ClientName(pPlayer->GetCid()));
	SendChatTarget(requestId, aBuf);
	str_format(aBuf, sizeof(aBuf), "Kills: %d", pPlayer->m_Account.m_SurvivalKills);
	SendChatTarget(requestId, aBuf);
	str_format(aBuf, sizeof(aBuf), "Deaths: %d", pPlayer->m_Account.m_SurvivalDeaths);
	SendChatTarget(requestId, aBuf);
	str_format(aBuf, sizeof(aBuf), "Wins: %d", pPlayer->m_Account.m_SurvivalWins);
	SendChatTarget(requestId, aBuf);
}

void CGameContext::ShowDDPPStats(int requestId, int requestedId)
{
	if(!m_apPlayers[requestId])
		return;
	CPlayer *pPlayer = m_apPlayers[requestedId];
	if(!pPlayer)
		return;

	char aBuf[128];

	str_format(aBuf, sizeof(aBuf), "--- %s's Stats ---", Server()->ClientName(requestedId));
	SendChatTarget(requestId, aBuf);
	if(pPlayer->GetLevel() == ACC_MAX_LEVEL)
		str_format(aBuf, sizeof(aBuf), "Level[%d] ( MAX LEVEL ! )", pPlayer->GetLevel());
	else
		str_format(aBuf, sizeof(aBuf), "Level[%d]", pPlayer->GetLevel());
	SendChatTarget(requestId, aBuf);
	if(!pPlayer->IsLoggedIn())
		str_format(aBuf, sizeof(aBuf), "Xp[%" PRId64 "] (not logged in)", pPlayer->GetXP());
	else
		str_format(aBuf, sizeof(aBuf), "Xp[%" PRId64 "/%" PRId64 "]", pPlayer->GetXP(), pPlayer->GetNeededXP());
	SendChatTarget(requestId, aBuf);
	str_format(aBuf, sizeof(aBuf), "Money[%" PRId64 "]", pPlayer->GetMoney());
	SendChatTarget(requestId, aBuf);
	str_format(aBuf, sizeof(aBuf), "PvP-Arena Tickets[%d]", pPlayer->m_Account.m_PvpArenaTickets);
	SendChatTarget(requestId, aBuf);
	SendChatTarget(requestId, "---- BLOCK ----");
	str_format(aBuf, sizeof(aBuf), "Points: %d", pPlayer->m_Account.m_BlockPoints);
	SendChatTarget(requestId, aBuf);
	str_format(aBuf, sizeof(aBuf), "Kills: %d", pPlayer->m_Account.m_BlockPoints_Kills);
	SendChatTarget(requestId, aBuf);
	str_format(aBuf, sizeof(aBuf), "Deaths: %d", pPlayer->m_Account.m_BlockPoints_Deaths);
	SendChatTarget(requestId, aBuf);

	// str_format(aBuf, sizeof(aBuf), "Skillgroup: %s", GetBlockSkillGroup(StatsId));
	// SendChatTarget(requestId, aBuf);
}

bool CGameContext::ChillWriteToLine(char const *filename, unsigned lineNo, char const *data)
{
	std::fstream file(filename);
	if(!file)
		return false;

	unsigned currentLine = 0;
	while(currentLine < lineNo)
	{
		// We don't actually care about the lines we're reading,
		// so just discard them.
		file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		++currentLine;
	}

	// Position the put pointer -- switching from reading to writing.
	file.seekp(file.tellg());

	dbg_msg("acc2", "writing [%s] to line [%d]", data, currentLine);

	//return file << data; //doesnt compile with MinGW
	return false;
}

int CGameContext::ChillUpdateFileAcc(const char *account, unsigned int line, const char *value, int requestingId)
{
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "%s/%s.acc", g_Config.m_SvFileAccPath, account);
	std::fstream Acc2File(aBuf);

	if(!std::ifstream(aBuf))
	{
		SendChatTarget(requestingId, "[ACCOUNT] username not found.");
		Acc2File.close();
		return -1; //return error code -1
	}

	std::string data[32];
	int index = 0;

	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] password: '%s'", index, data[index].c_str());
	index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] loggedin: '%s'", index, data[index].c_str());
	index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] port: '%s'", index, data[index].c_str());
	index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] frozen: '%s'", index, data[index].c_str());
	index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] vip: '%s'", index, data[index].c_str());
	index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] vip+: '%s'", index, data[index].c_str());
	index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] sup: '%s'", index, data[index].c_str());
	index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] money: '%s'", index, data[index].c_str());
	index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] level: '%s'", index, data[index].c_str());
	index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] xp: '%s'", index, data[index].c_str());
	index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] shit: '%s'", index, data[index].c_str());
	index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] police: '%s'", index, data[index].c_str());
	index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] taser: '%s'", index, data[index].c_str());
	index++;

	if(data[1] == "1")
	{
		str_format(aBuf, sizeof(aBuf), "[ACC2] '%s' is logged in on port '%s'", account, data[2].c_str());
		SendChatTarget(requestingId, aBuf);
		Acc2File.close();
		return -2;
	}

	if(line != 3 && data[3] == "1") //only can update the frozen value if acc is frozen
	{
		str_format(aBuf, sizeof(aBuf), "[ACC2] '%s' is frozen cant set line '%d'", account, line);
		SendChatTarget(requestingId, aBuf);
		Acc2File.close();
		return -3;
	}

	//===============
	//finish reading
	//start writing
	//===============

	//set new data
	data[line] = value;

	str_format(aBuf, sizeof(aBuf), "%s/%s.acc", g_Config.m_SvFileAccPath, account);
	std::ofstream Acc2FileW(aBuf);

	if(Acc2FileW.is_open())
	{
		dbg_msg("acc2", "write acc '%s'", account);
		index = 0;

		Acc2FileW << data[index++] << "\n"; //0 password
		Acc2FileW << data[index++] << "\n"; //1 loggedin
		Acc2FileW << data[index++] << "\n"; //2 port
		Acc2FileW << data[index++] << "\n"; //3 frozen
		Acc2FileW << data[index++] << "\n"; //4 vip
		Acc2FileW << data[index++] << "\n"; //5 vip+
		Acc2FileW << data[index++] << "\n"; //6 sup
		Acc2FileW << data[index++] << "\n"; //7 money
		Acc2FileW << data[index++] << "\n"; //8 level
		Acc2FileW << data[index++] << "\n"; //9 xp
		Acc2FileW << data[index++] << "\n"; //10 shit
		Acc2FileW << data[index++] << "\n"; //11 police
		Acc2FileW << data[index++] << "\n"; //12 taser

		Acc2FileW.close();
	}
	else
	{
		dbg_msg("acc2", "[WARNING] account '%s' (%s) failed to save", account, aBuf);
		Acc2FileW.close();
		return -4;
	}

	str_format(aBuf, sizeof(aBuf), "[ACC2] '%s' updated line [%d] to value [%s]", account, line, value);
	SendChatTarget(requestingId, aBuf);

	Acc2File.close();
	return 0; //all clean no errors --> return false
}

void CGameContext::ConnectFngBots(int amount, int mode)
{
	for(int i = 0; i < amount; i++)
	{
		if(mode == 0) //rifle
		{
			CreateNewDummy(DUMMYMODE_RIFLE_FNG);
		}
		else if(mode == 1) // grenade
		{
			CreateNewDummy(DUMMYMODE_GRENADE_FNG);
		}
		else
		{
			dbg_msg("WARNING", "ConnectFngBots() mode %d not valid.", mode);
			return;
		}
	}
}

void CGameContext::SaveCosmetics(int id)
{
	CPlayer *pPlayer = m_apPlayers[id];
	if(!pPlayer)
		return;
	CCharacter *pChr = m_apPlayers[id]->GetCharacter();
	if(!pChr)
		return;

	//backup cosmetics for lobby (save)
	pPlayer->m_IsBackupRainbow = pChr->m_Rainbow;
	pPlayer->m_IsBackupBloody = pChr->m_Bloody;
	pPlayer->m_IsBackupStrongBloody = pChr->m_StrongBloody;
	pPlayer->m_IsBackupAtom = pChr->m_Atom;
	pPlayer->m_IsBackupTrail = pChr->m_Trail;
	pPlayer->m_IsBackupAutospreadgun = pChr->m_autospreadgun;
	pPlayer->m_IsBackupWaveBloody = pChr->m_WaveBloody;
}

void CGameContext::LoadCosmetics(int id)
{
	CPlayer *pPlayer = m_apPlayers[id];
	if(!pPlayer)
		return;
	CCharacter *pChr = m_apPlayers[id]->GetCharacter();
	if(!pChr)
		return;

	//backup cosmetics for lobby (save)
	pChr->m_Rainbow = pPlayer->m_IsBackupRainbow;
	pChr->m_Bloody = pPlayer->m_IsBackupBloody;
	pChr->m_StrongBloody = pPlayer->m_IsBackupStrongBloody;
	pChr->m_Atom = pPlayer->m_IsBackupAtom;
	pChr->m_Trail = pPlayer->m_IsBackupTrail;
	pChr->m_autospreadgun = pPlayer->m_IsBackupAutospreadgun;
	pChr->m_WaveBloody = pPlayer->m_IsBackupWaveBloody;
}

void CGameContext::DeleteCosmetics(int id)
{
	CPlayer *pPlayer = m_apPlayers[id];
	if(!pPlayer)
		return;
	CCharacter *pChr = m_apPlayers[id]->GetCharacter();
	if(!pChr)
		return;

	pChr->m_Rainbow = false;
	pChr->m_Bloody = false;
	pChr->m_StrongBloody = false;
	pChr->m_Atom = false;
	pChr->m_Trail = false;
	pChr->m_autospreadgun = false;
	pChr->m_RandomCosmetics = false;
	pChr->m_WaveBloody = false;
	pChr->UnsetSpookyGhost();
}

void CGameContext::CheckDDPPshutdown()
{
	if(g_Config.m_SvDDPPshutdown)
	{
		int players = CountConnectedHumans();
		time_t now;
		struct tm *now_tm;
		int hour;
		int min;
		now = time(NULL);
		now_tm = localtime(&now);
		hour = now_tm->tm_hour;
		min = now_tm->tm_min;
		if(hour == g_Config.m_SvDDPPshutdownHour && (min == 0 || min == 5 || min == 10)) //Try it 3 times (slow tick shouldnt trigger it multiple times a minute)
		{
			if(players < g_Config.m_SvDDPPshutdownPlayers)
			{
				//SendChat(-1, TEAM_ALL, "[DDNet++] WARNING SERVER SHUTDOWN!");
				CallVetoVote(-1, "shutdown server", "shutdown", "Update", "[DDNet++] do you want to update the server now?", 0);
			}
			else
			{
				SendChat(-1, TEAM_ALL, "[DDNet++] shutdown failed: too many players online.");
			}
		}
	}
}

void CGameContext::DDPP_Tick()
{
	if(g_Config.m_SvOffDDPP)
		return;

	if(m_iBroadcastDelay > 0)
		m_iBroadcastDelay--;

	for(auto &Minigame : m_vMinigames)
		Minigame->Tick();

	CheckDeactivatePoliceFarm();

	if(m_BalanceBattleState == 1)
		BalanceBattleTick();
	if(m_BombGameState)
		BombTick();
	if(m_BlockWaveGameState)
		BlockWaveGameTick();
	if(m_survivalgamestate == 1)
		SurvivalLobbyTick();
	else
	{
		if(m_survival_game_countdown > 0)
		{
			m_survival_game_countdown--;
		}
		if(m_survival_game_countdown == 0)
		{
			SendChatSurvival("[SURVIVAL] Game ended due to timeout. Nobody won.");
			str_copy(m_aLastSurvivalWinnerName, "", sizeof(m_aLastSurvivalWinnerName));
			for(auto &Player : m_apPlayers)
			{
				if(Player && Player->m_IsSurvivaling)
				{
					SetPlayerSurvival(Player->GetCid(), SURVIVAL_LOBBY);
					if(Player->GetCharacter()) //only kill if isnt dead already or server crashes (he should respawn correctly anayways)
					{
						Player->GetCharacter()->Die(Player->GetCid(), WEAPON_GAME);
					}
				}
			}
			SurvivalSetGameState(SURVIVAL_LOBBY);
		}
		if(m_survivalgamestate == SURVIVAL_DM_COUNTDOWN)
		{
			SurvivalDeathmatchTick();
		}
	}

	if(m_CreateShopBot && (Server()->Tick() % 50 == 0))
	{
		CreateNewDummy(DUMMYMODE_SHOPBOT);
		m_CreateShopBot = false;
	}
	// all the tick stuff which needs all players
	for(auto &Player : m_apPlayers)
	{
		if(!Player)
			continue;

		int PlayerId = Player->GetCid();
		if(m_LastAccountMode != g_Config.m_SvAccountStuff)
		{
			if(Player->IsLoggedIn())
			{
				SendChatTarget(PlayerId, "[ACCOUNT] you have been logged out due to changes in the system");
				Player->Logout();
			}
		}

		ChilliClanTick(PlayerId);
		AsciiTick(PlayerId);
		InstaGrenadeRoundEndTick(PlayerId);
		InstaRifleRoundEndTick(PlayerId);
		C3_MultiPlayer_GameTick(PlayerId);
	}
	if(m_InstaGrenadeRoundEndTickTicker)
		m_InstaGrenadeRoundEndTickTicker--;
	if(m_InstaRifleRoundEndTickTicker)
		m_InstaRifleRoundEndTickTicker--;
	m_LastAccountMode = g_Config.m_SvAccountStuff;

	if(Server()->Tick() % 600 == 0) //slow ddpp sub tick
		DDPP_SlowTick();
}

void CGameContext::LogoutAllPlayers()
{
	dbg_msg("ddnet++", "loggin out all players (not working yet...)");
	for(auto &Player : m_apPlayers)
	{
		if(!Player)
			continue;

		if(Player->IsLoggedIn())
		{
			dbg_msg("ddnet++", "logging out id=%d", Player->GetCid());
			Player->Logout();
		}
	}
}

void CGameContext::LogoutAllPlayersMessage()
{
	for(auto &Player : m_apPlayers)
	{
		if(!Player)
			continue;

		if(Player->IsLoggedIn())
		{
			dbg_msg("ddnet++", "logging out id=%d", Player->GetCid());
			Player->Logout();
			SendChatTarget(Player->GetCid(), "[ACCOUNT] you were logged out.");
		}
	}
}

void CGameContext::ConnectAdventureBots()
{
	CreateNewDummy(DUMMYMODE_ADVENTURE, true, 1);
}

void CGameContext::DDPP_SlowTick()
{
	bool StopSurvival = true;
	int NumQuesting = 0;
	int TotalPlayers = 0;
	int NumAdventureBots = 0;
	for(auto &Player : m_apPlayers)
	{
		if(!Player)
			continue;

		int PlayerId = Player->GetCid();
		TotalPlayers++;
		CheckDeleteLoginBanEntry(PlayerId);
		CheckDeleteRegisterBanEntry(PlayerId);
		CheckDeleteNamechangeMuteEntry(PlayerId);
		if(Player->IsQuesting())
		{
			NumQuesting++;
			if(Player->m_QuestPlayerId != -1) //if player is on a <specfic player> quest
			{
				if(!m_apPlayers[Player->m_QuestPlayerId])
				{
					SendChatTarget(PlayerId, "[QUEST] Looks like your quest destination left the server.");
					QuestFailed(PlayerId);
				}
				else if(m_apPlayers[Player->m_QuestPlayerId]->GetTeam() == TEAM_SPECTATORS)
				{
					SendChatTarget(PlayerId, "[QUEST] Looks like your quest destination is a spectator.");
					QuestFailed(PlayerId);
				}
			}
		}
		if(Player->m_IsSurvivaling)
		{
			StopSurvival = false;
		}
		if(Player->m_IsDummy)
		{
			if(Player->DummyMode() == DUMMYMODE_ADVENTURE)
				NumAdventureBots++;
		}
	}

	if(TotalPlayers + 3 > g_Config.m_SvMaxClients ||
		NumQuesting == 0)
	{
		for(auto &Player : m_apPlayers)
		{
			if(!Player)
				continue;

			if(!Player->m_IsDummy)
				continue;
			if(Player->DummyMode() == DUMMYMODE_QUEST)
				Server()->BotLeave(Player->GetCid());
		}
	}
	if(NumAdventureBots < g_Config.m_SvAdventureBots)
	{
		ConnectAdventureBots();
	}

	if(StopSurvival)
	{
		m_survivalgamestate = 0; //don't waste ressource on lobby checks if nobody is playing
	}
	if(g_Config.m_SvAllowGlobalChat)
	{
		GlobalChatPrintMessage();
	}

	if(g_Config.m_SvMinDoubleTilePlayers > 0)
	{
		if(CountIngameHumans() >= g_Config.m_SvMinDoubleTilePlayers && MoneyDoubleEnoughPlayers) // MoneyTileDouble();  bla bla
		{
			SendChat(-1, TEAM_ALL, "The double-moneytile has been activated!");
			MoneyDoubleEnoughPlayers = false;
		}
		if(CountIngameHumans() < g_Config.m_SvMinDoubleTilePlayers && !MoneyDoubleEnoughPlayers)
		{
			SendChat(-1, TEAM_ALL, "The double-moneytile has been deactivated!");
			MoneyDoubleEnoughPlayers = true;
		}
	}
	CheckDDPPshutdown();
	if(g_Config.m_SvAutoFixBrokenAccs)
		SQLcleanZombieAccounts(-1);

	CheckServerEmpty();

	for(auto &Minigame : m_vMinigames)
		Minigame->SlowTick();
}

void CGameContext::CheckServerEmpty()
{
	m_IsServerEmpty = CountConnectedHumans() == 0;
}

void CGameContext::ChilliClanTick(int i)
{
	if(!g_Config.m_SvKickChilliClan)
		return;

	if(!m_apPlayers[i])
		return;

	CPlayer *pPlayer = m_apPlayers[i];

	int AbstandWarnungen = 10;
	if((str_comp_nocase(Server()->ClientClan(i), "Chilli.*") == 0 && str_comp_nocase(pPlayer->m_TeeInfos.m_aSkinName, "greensward") != 0) && (!pPlayer->m_SpookyGhostActive))
	{
		if(pPlayer->m_LastWarning + AbstandWarnungen * Server()->TickSpeed() <= Server()->Tick())
		{
			pPlayer->m_ChilliWarnings++;

			if(pPlayer->m_ChilliWarnings >= 4)
			{
				if(g_Config.m_SvKickChilliClan == 1)
				{
					GetPlayerChar(i)->m_FreezeTime = 1000;
					SendBroadcast("WARNING! You are using the wrong 'Chilli.*' clanskin.\n Leave the clan or change skin.", i);
					SendChatTarget(i, "You got freezed by Chilli.* clanportection. Change skin or clantag!");
				}
				else if(g_Config.m_SvKickChilliClan == 2)
				{
					char aRcon[128];
					str_format(aRcon, sizeof(aRcon), "kick %d Chilli.* clanfake", i);
					Console()->ExecuteLine(aRcon);
				}
			}
			else
			{
				SendChatTarget(i, "#######################################");
				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "You are using the wrong skin! Change skin or clantag! Warning: [%d/3]", pPlayer->m_ChilliWarnings);
				SendChatTarget(i, aBuf);
				SendChatTarget(i, "For more information about the clan visit: www.chillerdragon.weebly.com");
				SendChatTarget(i, "#######################################");

				/*
				char aRcon[128];
				str_format(aRcon, sizeof(aRcon), "broadcast YOU USE THE WRONG SKIN!\nCHANGE CLANTAG OR USE THE SKIN 'greensward'\n\nWARNINGS UNTIL KICK[%d/3]", pPlayer->m_ChilliWarnings);
				Console()->ExecuteLine(aRcon);
				*/

				char aBuf2[256];
				if(g_Config.m_SvKickChilliClan == 1)
				{
					str_format(aBuf2, sizeof(aBuf2), "Your are using the wrong skin!\nChange you clantag or use skin 'greensward'!\n\nWARNINGS UNTIL FREEZE[%d / 3]", pPlayer->m_ChilliWarnings);
				}
				else if(g_Config.m_SvKickChilliClan == 2)
				{
					str_format(aBuf2, sizeof(aBuf2), "Your are using the wrong skin!\nChange you clantag or use skin 'greensward'!\n\nWARNINGS UNTIL KICK[%d / 3]", pPlayer->m_ChilliWarnings);
				}
				SendBroadcast(aBuf2, i);
			}

			pPlayer->m_LastWarning = Server()->Tick();
		}
	}
}

void CGameContext::AsciiTick(int i)
{
	if(!m_apPlayers[i])
		return;

	if(m_apPlayers[i]->m_AsciiWatchingId != -1)
	{
		if(!m_apPlayers[m_apPlayers[i]->m_AsciiWatchingId]) //creator left -> stop animation
		{
			//SendChatTarget(i, "Ascii animation stopped because the creator left the server.");
			//SendBroadcast(" ERROR LOADING ANIMATION ", i);
			m_apPlayers[i]->m_AsciiWatchingId = -1;
			m_apPlayers[i]->m_AsciiWatchTicker = 0;
			m_apPlayers[i]->m_AsciiWatchFrame = 0;
		}
		else
		{
			m_apPlayers[i]->m_AsciiWatchTicker++;
			if(m_apPlayers[i]->m_AsciiWatchTicker >= m_apPlayers[m_apPlayers[i]->m_AsciiWatchingId]->m_AsciiAnimSpeed) //new frame
			{
				m_apPlayers[i]->m_AsciiWatchTicker = 0;
				if(m_apPlayers[i]->m_AsciiWatchFrame > 15) //animation over -> stop animation
				{
					//SendChatTarget(i, "Ascii animation is over.");
					//SendBroadcast(" ANIMATION OVER ", i);
					m_apPlayers[i]->m_AsciiWatchingId = -1;
					m_apPlayers[i]->m_AsciiWatchTicker = 0;
					m_apPlayers[i]->m_AsciiWatchFrame = 0;
				}
				else //display new frame
				{
					if(m_apPlayers[i]->m_AsciiWatchFrame == 0)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingId]->m_Account.m_aAsciiFrame[0], i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 1)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingId]->m_Account.m_aAsciiFrame[1], i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 2)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingId]->m_Account.m_aAsciiFrame[2], i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 3)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingId]->m_Account.m_aAsciiFrame[3], i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 4)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingId]->m_Account.m_aAsciiFrame[4], i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 5)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingId]->m_Account.m_aAsciiFrame[5], i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 6)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingId]->m_Account.m_aAsciiFrame[6], i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 7)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingId]->m_Account.m_aAsciiFrame[7], i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 8)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingId]->m_Account.m_aAsciiFrame[8], i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 9)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingId]->m_Account.m_aAsciiFrame[9], i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 10)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingId]->m_Account.m_aAsciiFrame[10], i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 11)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingId]->m_Account.m_aAsciiFrame[11], i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 12)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingId]->m_Account.m_aAsciiFrame[12], i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 13)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingId]->m_Account.m_aAsciiFrame[13], i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 14)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingId]->m_Account.m_aAsciiFrame[14], i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 15)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingId]->m_Account.m_aAsciiFrame[15], i);
					}
					else
					{
						SendChatTarget(i, "error loading frame");
					}
				}
				m_apPlayers[i]->m_AsciiWatchFrame++;
			}
		}
	}
}

void CGameContext::LoadSinglePlayer()
{
	FILE *pFile;
	struct CBinaryStorage statsBuff;

	pFile = fopen("ddpp-stats.dat", "rb");
	if(!pFile)
	{
		dbg_msg("ddpp-stats", "[load] failed to open ddpp singleplayer stats");
		return;
	}

	if(!fread(&statsBuff, sizeof(struct CBinaryStorage), 1, pFile))
	{
		dbg_msg("ddpp-stats", "failed to read ddpp singleplayer stats");
		return;
	}
	dbg_msg("ddpp-stats", "loaded data UnlockedLevel=%d", statsBuff.x);
	m_MissionUnlockedLevel = statsBuff.x;
	if(!fread(&statsBuff, sizeof(struct CBinaryStorage), 1, pFile))
	{
		dbg_msg("ddpp-stats", "failed to read ddpp singleplayer stats");
		return;
	}
	dbg_msg("ddpp-stats", "loaded data CurrentLevel=%d", statsBuff.x);
	m_MissionCurrentLevel = statsBuff.x;

	if(fclose(pFile))
		dbg_msg("ddpp-stats", "failed to close singleplayer file='%s' errno=%d", "ddpp-stats.dat", errno);
}

void CGameContext::SaveSinglePlayer()
{
	FILE *pFile;
	struct CBinaryStorage statsBuff;

	pFile = fopen("ddpp-stats.dat", "wb");
	if(!pFile)
	{
		dbg_msg("ddpp-stats", "[save] failed to open ddpp singleplayer stats");
		return;
	}
	statsBuff.x = m_MissionUnlockedLevel;
	fwrite(&statsBuff, sizeof(struct CBinaryStorage), 1, pFile);
	statsBuff.x = m_MissionCurrentLevel;
	fwrite(&statsBuff, sizeof(struct CBinaryStorage), 1, pFile);

	fclose(pFile);
}

void CGameContext::SaveMapPlayerData()
{
	FILE *pFile;
	char aSaveFile[256];
	str_format(aSaveFile, sizeof(aSaveFile), "%s_playerdata.dat", Server()->GetMapName());
	pFile = fopen(aSaveFile, "wb");
	if(!pFile)
	{
		dbg_msg("ddpp-mapsave", "failed to write ddpp player data file '%s'.", aSaveFile);
		return;
	}
	int saved = 0;
	int players = CountTimeoutCodePlayers();
	fwrite(&players, sizeof(players), 1, pFile);
	for(auto &Player : m_apPlayers)
	{
		if(!Player)
			continue;
		CCharacter *pChr = Player->GetCharacter();
		if(!pChr)
			continue;
		if(!Player->m_aTimeoutCode[0])
			continue;
		if(!pChr)
			continue;
		fwrite(&Player->m_aTimeoutCode, 64, 1, pFile);
		char IsLoaded = 0;
		dbg_msg("ddpp-mapsave", "writing isloaded at pos %ld", get_file_offset(pFile));
		fwrite(&IsLoaded, sizeof(IsLoaded), 1, pFile);

		CSaveTee savetee;
		savetee.Save(pChr);
		fwrite(&savetee, sizeof(savetee), 1, pFile);

		dbg_msg("ddpp-mapsave", "save player=%s code=%s", Server()->ClientName(Player->GetCid()), Player->m_aTimeoutCode);
		saved++;
	}
	fclose(pFile);
	dbg_msg("ddpp-mapsave", "saved %d/%d players", saved, players);
}

void CGameContext::LoadMapPlayerData()
{
	FILE *pFile;
	char aSaveFile[256];
	str_format(aSaveFile, sizeof(aSaveFile), "%s_playerdata.dat", Server()->GetMapName());
	pFile = fopen(aSaveFile, "rb+");
	if(!pFile)
	{
		m_MapsavePlayers = 0;
		dbg_msg("ddpp-mapload", "failed to open ddpp player data file '%s'.", aSaveFile);
		return;
	}
	int loaded = 0;
	int players = 0;
	if(!fread(&players, sizeof(players), 1, pFile))
	{
		dbg_msg("ddpp-mapload", "failed to read data");
		return;
	}
	for(int i = 0; i < players; i++)
	{
		// ValidPlayer replaces continue to make sure the binary cursor is at the right offset
		bool ValidPlayer = true;
		char aTimeoutCode[64];
		dbg_msg("ddpp-mapload", "read timeout code at %ld", get_file_offset(pFile));
		if(!fread(&aTimeoutCode, 64, 1, pFile))
		{
			dbg_msg("ddpp-mapload", "failed to read data");
			return;
		}
		int id = GetPlayerByTimeoutcode(aTimeoutCode);
		if(id == -1)
		{
			dbg_msg("ddpp-mapload", "player not online code=%s", aTimeoutCode);
			ValidPlayer = false;
		}
		else
		{
			CPlayer *pPlayer = m_apPlayers[id];
			if(!pPlayer)
			{
				dbg_assert(false, "loadmap invalid player");
				return;
			}
			CCharacter *pChr = pPlayer->GetCharacter();
			if(ValidPlayer && !pChr)
			{
				dbg_msg("ddpp-mapload", "player not alive id=%d code=%s", id, aTimeoutCode);
				ValidPlayer = false;
			}
			if(ValidPlayer && str_comp(aTimeoutCode, pPlayer->m_aTimeoutCode))
			{
				dbg_msg("ddpp-mapload", "wrong timeout code player=%s file=%s", pPlayer->m_aTimeoutCode, aTimeoutCode);
				ValidPlayer = false;
			}
			if(ValidPlayer && pPlayer->m_MapSaveLoaded)
			{
				// shouldn't happen? couldn't happen? too lazy to think probably possible by abusing /timeout command or share
				// and can be bypassed by reconnect lol
				dbg_msg("ddpp-mapload", "Warning: %d:'%s' code=%s is loaded already", id, Server()->ClientName(id), pPlayer->m_aTimeoutCode);
				ValidPlayer = false;
			}
		}
		char IsLoaded;
		dbg_msg("ddpp-mapload", "reading isloaded at pos %ld", get_file_offset(pFile));
		if(!fread(&IsLoaded, sizeof(IsLoaded), 1, pFile))
		{
			dbg_msg("ddpp-mapload", "failed to read data");
			return;
		}
		// reset file pos after read to overwrite isloaded or keep clean offset on continue
		if(IsLoaded)
		{
			dbg_msg("ddpp-mapload", "loaded already");
			ValidPlayer = false;
		}
		IsLoaded = ValidPlayer ? 1 : IsLoaded; // only change loaded state if the player is actually loaded
		fpos_t pos;
		fsetpos(pFile, &pos);
		fwrite(&IsLoaded, sizeof(IsLoaded), 1, pFile);

		CSaveTee savetee;
		if(!fread(&savetee, sizeof(savetee), 1, pFile))
		{
			dbg_msg("ddpp-mapload", "failed to read data");
			return;
		}

		if(ValidPlayer)
		{
			CPlayer *pPlayer = m_apPlayers[id];
			CCharacter *pChr = pPlayer->GetCharacter();

			savetee.Load(pChr, 0); // load to team0 always xd cuz teams sokk!

			m_MapsaveLoadedPlayers++;
			pPlayer->m_MapSaveLoaded = true;
			dbg_msg("ddpp-mapload", "load player=%s code=%s fp=%ld", Server()->ClientName(id), pPlayer->m_aTimeoutCode, get_file_offset(pFile));
			loaded++;
		}
	}
	if(fclose(pFile))
		dbg_msg("ddpp-mapload", "failed to close file '%s' errno=%d", aSaveFile, errno);
	m_MapsavePlayers = players;
	dbg_msg("ddpp-mapload", "loaded %d/%d players", loaded, players);
}

void CGameContext::ReadMapPlayerData(int ClientId)
{
	FILE *pFile;
	char aSaveFile[256];
	str_format(aSaveFile, sizeof(aSaveFile), "%s_playerdata.dat", Server()->GetMapName());
	pFile = fopen(aSaveFile, "rb");
	if(!pFile)
	{
		dbg_msg("ddpp-mapread", "failed to read ddpp player data file '%s'.", aSaveFile);
		return;
	}
	int loaded = 0;
	int red = 0;
	int players = 0;
	if(!fread(&players, sizeof(players), 1, pFile))
	{
		dbg_msg("ddpp-mapread", "failed to read data");
		return;
	}
	for(int i = 0; i < players; i++)
	{
		char aTimeoutCode[64];
		dbg_msg("ddpp-mapread", "read timeout code at %ld", get_file_offset(pFile));
		if(!fread(&aTimeoutCode, 64, 1, pFile))
		{
			dbg_msg("ddpp-mapread", "failed to read data");
			return;
		}
		int id = GetPlayerByTimeoutcode(aTimeoutCode);
		char IsLoaded;
		if(!fread(&IsLoaded, sizeof(IsLoaded), 1, pFile))
		{
			dbg_msg("ddpp-mapread", "failed to read data");
			return;
		}
		if(IsLoaded)
			loaded++;

		CSaveTee savetee;
		if(!fread(&savetee, sizeof(savetee), 1, pFile))
		{
			dbg_msg("ddpp-mapread", "failed to read data");
			return;
		}

		dbg_msg("ddpp-mapread", "read player=%d code=%s loaded=%d fp=%ld", id, aTimeoutCode, IsLoaded, get_file_offset(pFile));
		red++;
	}
	if(fclose(pFile))
		dbg_msg("ddpp-mapread", "failed to close file '%s' errno=%d", aSaveFile, errno);
	if(ClientId != -1)
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "[MAPSAVE] Debug: loaded %d/%d players", loaded, players);
		SendChatTarget(ClientId, aBuf);
		if(red != players)
		{
			str_format(aBuf, sizeof(aBuf), "[MAPSAVE] Debug: WARNING only found %d/%d players", red, players);
			SendChatTarget(ClientId, aBuf);
		}
	}
	dbg_msg("ddpp-mapread", "red %d/%d players (%d loaded)", red, players, loaded);
}

void CGameContext::GlobalChatPrintMessage()
{
	char aData[1024];
	std::string data;

	std::fstream ChatReadFile(g_Config.m_SvGlobalChatFile);

	if(!std::ifstream(g_Config.m_SvGlobalChatFile))
	{
		SendChat(-1, TEAM_ALL, "[CHAT] global chat stopped working.");
		g_Config.m_SvAllowGlobalChat = 0;
		ChatReadFile.close();
		return;
	}

	getline(ChatReadFile, data);
	str_format(aData, sizeof(aData), "%s", data.c_str());
	aData[0] = ' '; //remove the confirms before print in chat

	if(!str_comp(m_aLastPrintedGlobalChatMessage, aData))
	{
		//SendChat(-1, TEAM_ALL, "[CHAT] no new global message");
		ChatReadFile.close();
		return;
	}

	GlobalChatUpdateConfirms(data.c_str());
	str_format(m_aLastPrintedGlobalChatMessage, sizeof(m_aLastPrintedGlobalChatMessage), "%s", aData);
	SendChat(-1, TEAM_ALL, aData);
	//str_format(aBuf, sizeof(aBuf), "[CHAT] '%s'", aData);
	//SendChat(-1, TEAM_ALL, aBuf);

	ChatReadFile.close();
}

void CGameContext::GlobalChatUpdateConfirms(const char *pStr)
{
	char aBuf[1024];
	str_format(aBuf, sizeof(aBuf), "%s", pStr);
	int confirms = 0;
	if(pStr[0] == '1')
		confirms = 1;
	else if(pStr[0] == '2')
		confirms = 2;
	else if(pStr[0] == '3')
		confirms = 3;
	else if(pStr[0] == '4')
		confirms = 4;
	else if(pStr[0] == '5')
		confirms = 5;
	else if(pStr[0] == '6')
		confirms = 6;
	else if(pStr[0] == '7')
		confirms = 7;
	else if(pStr[0] == '8')
		confirms = 8;
	else if(pStr[0] == '9')
		confirms = 9;

	std::ofstream ChatFile(g_Config.m_SvGlobalChatFile);
	if(!ChatFile)
	{
		SendChat(-1, TEAM_ALL, "[CHAT] global chat failed.... deactivating it.");
		dbg_msg("CHAT", "ERROR1 writing file '%s'", g_Config.m_SvGlobalChatFile);
		g_Config.m_SvAllowGlobalChat = 0;
		ChatFile.close();
		return;
	}

	if(ChatFile.is_open())
	{
		confirms++;
		aBuf[0] = confirms + '0';
		ChatFile << aBuf << "\n";
	}
	else
	{
		SendChat(-1, TEAM_ALL, "[CHAT] global chat failed.... deactivating it.");
		dbg_msg("CHAT", "ERROR2 writing file '%s'", g_Config.m_SvGlobalChatFile);
		g_Config.m_SvAllowGlobalChat = 0;
	}

	ChatFile.close();
}

void CGameContext::SendAllPolice(const char *pMessage)
{
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "[POLICE-CHANNEL] %s", pMessage);
	for(auto &Player : m_apPlayers)
	{
		if(Player && Player->m_Account.m_PoliceRank)
		{
			SendChatTarget(Player->GetCid(), aBuf);
		}
	}
}

void CGameContext::AddEscapeReason(int Id, const char *pReason)
{
	//dbg_msg("cBug", "current reaso is %s", m_apPlayers[Id]->m_aEscapeReason);

	//dont add already exsisting reasons agian
	if(str_find(m_apPlayers[Id]->m_aEscapeReason, pReason))
	{
		//dbg_msg("cBug", "skipping exsisting reason %s", pReason);
		return;
	}
	//reset all
	if(!str_comp(pReason, "unknown"))
	{
		str_format(m_apPlayers[Id]->m_aEscapeReason, sizeof(m_apPlayers[Id]->m_aEscapeReason), "%s", pReason);
		//dbg_msg("cBug", "resetting to reason %s", pReason);
		return;
	}

	if(!str_comp(m_apPlayers[Id]->m_aEscapeReason, "unknown")) //keine vorstrafen
	{
		str_format(m_apPlayers[Id]->m_aEscapeReason, sizeof(m_apPlayers[Id]->m_aEscapeReason), "%s", pReason);
		dbg_msg("cBug", "set escape reason to %s -> %s", pReason, m_apPlayers[Id]->m_aEscapeReason);
	}
	else
	{
		str_format(m_apPlayers[Id]->m_aEscapeReason, sizeof(m_apPlayers[Id]->m_aEscapeReason), "%s, %s", m_apPlayers[Id]->m_aEscapeReason, pReason);
	}
	//wtf doesnt work with the seconds first then the reason always gets printed as (null) wtf !)!)!)!
	/*
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "'%s' seconds [%d] reason [%s]", Server()->ClientName(Id), m_apPlayers[Id]->m_EscapeTime / Server()->TickSpeed(), m_apPlayers[Id]->m_aEscapeReason);
	dbg_msg("DEBUG", aBuf);
	dbg_msg("cBug", "set escape reason to %s -> %s", pReason, m_apPlayers[Id]->m_aEscapeReason);
	str_format(aBuf, sizeof(aBuf), " seconds [%d] reason [%s]", m_apPlayers[Id]->m_EscapeTime / Server()->TickSpeed(), m_apPlayers[Id]->m_aEscapeReason);
	SendChatTarget(Id, aBuf);
	*/
}

void CGameContext::ShowProfile(int ViewerId, int ViewedId)
{
	char aBuf[128];
	int GiveView = 1;

	if(!m_apPlayers[ViewedId] || !m_apPlayers[ViewerId])
	{
		return;
	}

	if(!m_apPlayers[ViewedId]->IsLoggedIn())
	{
		SendChatTarget(ViewerId, "Player has to be logged in to view his profile.");
		return;
	}

	if(!str_comp(m_apPlayers[ViewerId]->m_LastViewedProfile, Server()->ClientName(ViewedId)) && !m_apPlayers[ViewerId]->m_IsProfileViewLoaded) //repeated same profile and view not loaded yet
	{
		GiveView = 0;
	}
	else
	{
		if(!str_comp(Server()->ClientName(ViewedId), Server()->ClientName(ViewerId))) //viewing own profile --> random xd
		{
			GiveView = rand() % 2;
		}
	}

	if(GiveView)
	{
		m_apPlayers[ViewedId]->m_Account.m_ProfileViews++;
		str_copy(m_apPlayers[ViewerId]->m_LastViewedProfile, Server()->ClientName(ViewedId), 32);
		m_apPlayers[ViewerId]->m_IsProfileViewLoaded = false;
	}

	//ASCII - ANIMATIONS
	StartAsciiAnimation(ViewerId, ViewedId, 1);

	if(m_apPlayers[ViewedId]->m_Account.m_ProfileStyle == 0) //default
	{
		str_format(aBuf, sizeof(aBuf), "---  %s's Profile  ---", Server()->ClientName(ViewedId));
		SendChatTarget(ViewerId, aBuf);
		str_format(aBuf, sizeof(aBuf), "%s", m_apPlayers[ViewedId]->m_Account.m_ProfileStatus);
		SendChatTarget(ViewerId, aBuf);
		SendChatTarget(ViewerId, "-------------------------");
		str_format(aBuf, sizeof(aBuf), "Level: %d", m_apPlayers[ViewedId]->GetLevel());
		SendChatTarget(ViewerId, aBuf);
		str_format(aBuf, sizeof(aBuf), "Money: %" PRId64, m_apPlayers[ViewedId]->GetMoney());
		SendChatTarget(ViewerId, aBuf);
		str_format(aBuf, sizeof(aBuf), "Shit: %d", m_apPlayers[ViewedId]->m_Account.m_Shit);
		SendChatTarget(ViewerId, aBuf);
	}
	else if(m_apPlayers[ViewedId]->m_Account.m_ProfileStyle == 1) //shit
	{
		str_format(aBuf, sizeof(aBuf), "---  %s's Profile  ---", Server()->ClientName(ViewedId));
		SendChatTarget(ViewerId, aBuf);
		str_format(aBuf, sizeof(aBuf), "%s", m_apPlayers[ViewedId]->m_Account.m_ProfileStatus);
		SendChatTarget(ViewerId, aBuf);
		SendChatTarget(ViewerId, "-------------------------");
		str_format(aBuf, sizeof(aBuf), "Shit: %d", m_apPlayers[ViewedId]->m_Account.m_Shit);
		SendChatTarget(ViewerId, aBuf);
	}
	else if(m_apPlayers[ViewedId]->m_Account.m_ProfileStyle == 2) //social
	{
		str_format(aBuf, sizeof(aBuf), "---  %s's Profile  ---", Server()->ClientName(ViewedId));
		SendChatTarget(ViewerId, aBuf);
		str_format(aBuf, sizeof(aBuf), "%s", m_apPlayers[ViewedId]->m_Account.m_ProfileStatus);
		SendChatTarget(ViewerId, aBuf);
		SendChatTarget(ViewerId, "-------------------------");
		str_format(aBuf, sizeof(aBuf), "Skype: %s", m_apPlayers[ViewedId]->m_Account.m_ProfileSkype);
		SendChatTarget(ViewerId, aBuf);
		str_format(aBuf, sizeof(aBuf), "Youtube: %s", m_apPlayers[ViewedId]->m_Account.m_ProfileYoutube);
		SendChatTarget(ViewerId, aBuf);
		str_format(aBuf, sizeof(aBuf), "e-mail: %s", m_apPlayers[ViewedId]->m_Account.m_ProfileEmail);
		SendChatTarget(ViewerId, aBuf);
		str_format(aBuf, sizeof(aBuf), "Homepage: %s", m_apPlayers[ViewedId]->m_Account.m_ProfileHomepage);
		SendChatTarget(ViewerId, aBuf);
		str_format(aBuf, sizeof(aBuf), "Twitter: %s", m_apPlayers[ViewedId]->m_Account.m_ProfileTwitter);
		SendChatTarget(ViewerId, aBuf);
	}
	else if(m_apPlayers[ViewedId]->m_Account.m_ProfileStyle == 3) //show-off
	{
		str_format(aBuf, sizeof(aBuf), "---  %s's Profile  ---", Server()->ClientName(ViewedId));
		SendChatTarget(ViewerId, aBuf);
		str_format(aBuf, sizeof(aBuf), "%s", m_apPlayers[ViewedId]->m_Account.m_ProfileStatus);
		SendChatTarget(ViewerId, aBuf);
		SendChatTarget(ViewerId, "-------------------------");
		str_format(aBuf, sizeof(aBuf), "Profileviews: %d", m_apPlayers[ViewedId]->m_Account.m_ProfileViews);
		SendChatTarget(ViewerId, aBuf);
		str_format(aBuf, sizeof(aBuf), "Policerank: %d", m_apPlayers[ViewedId]->m_Account.m_PoliceRank);
		SendChatTarget(ViewerId, aBuf);
		str_format(aBuf, sizeof(aBuf), "Level: %d", m_apPlayers[ViewedId]->GetLevel());
		SendChatTarget(ViewerId, aBuf);
		str_format(aBuf, sizeof(aBuf), "Shit: %d", m_apPlayers[ViewedId]->m_Account.m_Shit);
		SendChatTarget(ViewerId, aBuf);
	}
	else if(m_apPlayers[ViewedId]->m_Account.m_ProfileStyle == 4) //pvp
	{
		str_format(aBuf, sizeof(aBuf), "---  %s's Profile  ---", Server()->ClientName(ViewedId));
		SendChatTarget(ViewerId, aBuf);
		str_format(aBuf, sizeof(aBuf), "%s", m_apPlayers[ViewedId]->m_Account.m_ProfileStatus);
		SendChatTarget(ViewerId, aBuf);
		SendChatTarget(ViewerId, "-------------------------");
		str_format(aBuf, sizeof(aBuf), "PVP-ARENA Games: %d", m_apPlayers[ViewedId]->m_Account.m_PvpArenaGamesPlayed);
		SendChatTarget(ViewerId, aBuf);
		str_format(aBuf, sizeof(aBuf), "PVP-ARENA Kills: %d", m_apPlayers[ViewedId]->m_Account.m_PvpArenaKills);
		SendChatTarget(ViewerId, aBuf);
		str_format(aBuf, sizeof(aBuf), "PVP-ARENA Deaths: %d", m_apPlayers[ViewedId]->m_Account.m_PvpArenaDeaths);
		SendChatTarget(ViewerId, aBuf);
		//str_format(aBuf, sizeof(aBuf), "PVP-ARENA K/D: %d", m_apPlayers[ViewedId]->m_Account.m_PvpArenaKills / m_Account.m_PvpArenaDeaths);
		//SendChatTarget(ViewerId, aBuf);
	}
	else if(m_apPlayers[ViewedId]->m_Account.m_ProfileStyle == 5) //bomber
	{
		str_format(aBuf, sizeof(aBuf), "---  %s's Profile  ---", Server()->ClientName(ViewedId));
		SendChatTarget(ViewerId, aBuf);
		str_format(aBuf, sizeof(aBuf), "%s", m_apPlayers[ViewedId]->m_Account.m_ProfileStatus);
		SendChatTarget(ViewerId, aBuf);
		SendChatTarget(ViewerId, "-------------------------");
		str_format(aBuf, sizeof(aBuf), "Bomb Games Played: %d", m_apPlayers[ViewedId]->m_Account.m_BombGamesPlayed);
		SendChatTarget(ViewerId, aBuf);
		str_format(aBuf, sizeof(aBuf), "Bomb Games Won: %d", m_apPlayers[ViewedId]->m_Account.m_BombGamesWon);
		SendChatTarget(ViewerId, aBuf);
		//str_format(aBuf, sizeof(aBuf), "PVP-ARENA K/D: %d", m_apPlayers[ViewedId]->m_Account.m_PvpArenaKills / m_Account.m_PvpArenaDeaths);
		//SendChatTarget(ViewerId, aBuf);
	}
}

void CGameContext::ShowAdminWelcome(int Id)
{
	SendChatTarget(Id, "============= admin login =============");
	char aBuf[128];
	if(m_WrongRconAttempts >= g_Config.m_SvRconAttemptReport)
	{
		str_format(aBuf, sizeof(aBuf), "Warning %d failed rcon attempts since last successful login! 'logs wrong_rcon'", m_WrongRconAttempts);
		// Server()->SendRconLine(Id, aBuf); // TODO: uncomment
	}
	if(aDDPPLogs[DDPP_LOG_AUTH_RCON][1][0]) // index 1 because index 0 is current login
	{
		str_format(aBuf, sizeof(aBuf), "last login %s", aDDPPLogs[DDPP_LOG_AUTH_RCON][1]);
		// Server()->SendRconLine(Id, aBuf); // TODO: uncomment
	}
	int surv_error = TestSurvivalSpawns();
	if(surv_error == -1)
	{
		SendChatTarget(Id, "[ADMIN:Test] WARNING: less survival spawns on map than slots possible in ddnet++ (no problem as long as slots stay how they are)");
	}
	else if(surv_error == -2)
	{
		SendChatTarget(Id, "[ADMIN:Test] WARNING: not enough survival spawns (less survival spawns than slots)");
	}
	int protections = 0;
	if(g_Config.m_SvRegisterHumanLevel)
	{
		str_format(aBuf, sizeof(aBuf), "[ADMIN:Prot] Warning sv_register_human_level = %d", g_Config.m_SvRegisterHumanLevel);
		SendChatTarget(Id, aBuf);
		protections++;
	}
	if(g_Config.m_SvChatHumanLevel)
	{
		str_format(aBuf, sizeof(aBuf), "[ADMIN:Prot] Warning sv_chat_human_level = %d", g_Config.m_SvChatHumanLevel);
		SendChatTarget(Id, aBuf);
		protections++;
	}
	if(g_Config.m_SvShowConnectionMessages != CON_SHOW_ALL)
	{
		str_format(aBuf, sizeof(aBuf), "[ADMIN:Prot] Warning sv_show_connection_msg = %d", g_Config.m_SvShowConnectionMessages);
		SendChatTarget(Id, aBuf);
		protections++;
	}
	if(g_Config.m_SvHideConnectionMessagesPattern[0])
	{
		str_format(aBuf, sizeof(aBuf), "[ADMIN:Prot] Warning sv_hide_connection_msg_pattern = %s", g_Config.m_SvHideConnectionMessagesPattern);
		SendChatTarget(Id, aBuf);
		protections++;
	}
	if(protections)
	{
		str_format(aBuf, sizeof(aBuf), "[ADMIN:Prot] Warning you have %d protective systems running!", protections);
		SendChatTarget(Id, aBuf);
		SendChatTarget(Id, "[ADMIN:Prot] As effective those are under attack and as good protection sounds.");
		SendChatTarget(Id, "[ADMIN:Prot] Those should not run if there is no attack since they lower UX.");
		protections++;
	}
	PrintSpecialCharUsers(Id);
}

int CGameContext::PrintSpecialCharUsers(int Id)
{
	char aUsers[2048]; //wont show all users if too many special char users are there but this shouldnt be the case
	int users = 0;
	for(auto &Player : m_apPlayers)
	{
		if(Player && Player->IsLoggedIn())
		{
			if(!IsAllowedCharSet(Player->m_Account.m_aUsername))
			{
				if(!users)
				{
					str_format(aUsers, sizeof(aUsers), "[id='%d' acc='%s']", Player->GetCid(), Player->m_Account.m_aUsername);
				}
				else
				{
					str_format(aUsers, sizeof(aUsers), "%s, [id='%d' acc='%s']", aUsers, Player->GetCid(), Player->m_Account.m_aUsername);
				}
				users++;
			}
		}
	}

	if(users)
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "[#########] %d special char user online [#########]", users);
		SendChatTarget(Id, aBuf);
		SendChatTarget(Id, aUsers);
	}
	return users;
}

int CGameContext::TestSurvivalSpawns()
{
	vec2 SurvivalGameSpawnTile = Collision()->GetSurvivalSpawn(g_Config.m_SvMaxClients);
	vec2 SurvivalGameSpawnTile2 = Collision()->GetSurvivalSpawn(MAX_CLIENTS);

	if(SurvivalGameSpawnTile == vec2(-1, -1))
	{
		//SendChatTarget(ClientId, "[ADMIN:Test] ERROR: not enough survival spawns (less survival spawns than slots)");
		return -2;
	}
	else if(SurvivalGameSpawnTile2 == vec2(-1, -1))
	{
		//SendChatTarget(ClientId, "[ADMIN:Test] WARNING: less survival spawns on map than slots possible in ddnet++ (no problem as long as slots stay how they are)");
		return -1;
	}
	else
	{
		//SendChatTarget(ClientId, "[ADMIN:Test] Test Finished. Everything looks good c:");
		return 0;
	}
	return 0;
}

void CGameContext::ChatCommands()
{
}

void CGameContext::DummyChat()
{
	//unused cuz me knoop putting all the stuff here
}

void CGameContext::CreateBasicDummys()
{
	if(!str_comp(Server()->GetMapName(), "ChillBlock5"))
	{
		CreateNewDummy(DUMMYMODE_CHILLBLOCK5_POLICE);
		CreateNewDummy(DUMMYMODE_CHILLBLOCK5_BLOCKER);
		CreateNewDummy(DUMMYMODE_CHILLBLOCK5_BLOCKER);
		CreateNewDummy(DUMMYMODE_CHILLBLOCK5_RACER);
		CreateNewDummy(DUMMYMODE_BLMAPV3_ARENA);
	}
	else if(!str_comp(Server()->GetMapName(), "BlmapChill"))
	{
		CreateNewDummy(DUMMYMODE_BLMAPCHILL_POLICE);
	}
	else if(!str_comp(Server()->GetMapName(), "blmapV5"))
	{
		CreateNewDummy(DUMMYMODE_BLMAPV5_LOWER_BLOCKER);
		CreateNewDummy(DUMMYMODE_BLMAPV5_LOWER_BLOCKER);
		CreateNewDummy(DUMMYMODE_BLMAPV5_UPPER_BLOCKER);
	}
	else if(!str_comp(Server()->GetMapName(), "blmapV5_ddpp"))
	{
		CreateNewDummy(DUMMYMODE_BLMAPV5_LOWER_BLOCKER);
		CreateNewDummy(DUMMYMODE_BLMAPV5_LOWER_BLOCKER);
		CreateNewDummy(DUMMYMODE_BLMAPV5_UPPER_BLOCKER);
		g_Config.m_SvDummyMapOffsetX = -226;
	}
	else if(!str_comp(Server()->GetMapName(), "ddpp_survival"))
	{
		CreateNewDummy(DUMMYMODE_SURVIVAL);
		CreateNewDummy(DUMMYMODE_SURVIVAL);
	}
	else
	{
		CreateNewDummy(DUMMYMODE_DEFAULT);
		dbg_msg("basic_dummys", "waring map=%s not supported", Server()->GetMapName());
	}
	if(m_ShopBotTileExists)
	{
		m_CreateShopBot = true;
	}
	dbg_msg("basic_dummys", "map=%s", Server()->GetMapName());
}

int CGameContext::CreateNewDummy(EDummyMode Mode, bool Silent, int Tile)
{
	int DummyId = GetNextClientId();
	if(DummyId < 0 || DummyId >= MAX_CLIENTS)
	{
		dbg_msg("dummy", "Can't get ClientId. Server is full or something like that.");
		return -1;
	}
	if(m_apPlayers[DummyId])
	{
		dbg_msg("dummy", "Can't create dummy. ID occopied already");
		return -1;
	}

	m_apPlayers[DummyId] = new(DummyId) CPlayer(this, NextUniqueClientId, DummyId, TEAM_RED, true);
	NextUniqueClientId += 1;

	m_apPlayers[DummyId]->m_NoboSpawnStop = 0;
	m_apPlayers[DummyId]->SetDummyMode(Mode);
	Server()->BotJoin(DummyId);

	str_copy(m_apPlayers[DummyId]->m_TeeInfos.m_aSkinName, "greensward", MAX_NAME_LENGTH);
	m_apPlayers[DummyId]->m_TeeInfos.m_UseCustomColor = true;
	m_apPlayers[DummyId]->m_TeeInfos.m_ColorFeet = 0;
	m_apPlayers[DummyId]->m_TeeInfos.m_ColorBody = 0;
	m_apPlayers[DummyId]->m_DummySpawnTile = Tile;

	dbg_msg("dummy", "Dummy connected: %d", DummyId);

	if(Mode == DUMMYMODE_BALANCE1)
	{
		m_apPlayers[DummyId]->m_IsBalanceBattlePlayer1 = true;
		m_apPlayers[DummyId]->m_IsBalanceBattleDummy = true;
	}
	else if(Mode == DUMMYMODE_BALANCE2)
	{
		m_apPlayers[DummyId]->m_IsBalanceBattlePlayer1 = false;
		m_apPlayers[DummyId]->m_IsBalanceBattleDummy = true;
	}
	else if(Mode == DUMMYMODE_BLOCKWAVE)
	{
		m_apPlayers[DummyId]->m_IsBlockWaving = true;
	}
	else if(Mode == DUMMYMODE_RIFLE_FNG)
	{
		m_pInstagib->Join(m_apPlayers[DummyId], WEAPON_GRENADE, true);
	}
	else if(Mode == DUMMYMODE_GRENADE_FNG)
	{
		m_pInstagib->Join(m_apPlayers[DummyId], WEAPON_LASER, true);
	}
	else if(Mode == DUMMYMODE_BLMAPV3_ARENA)
	{
		m_apPlayers[DummyId]->m_IsBlockDeathmatch = true;
	}
	else if(Mode == DUMMYMODE_ADVENTURE)
	{
		m_apPlayers[DummyId]->m_IsVanillaDmg = true;
		m_apPlayers[DummyId]->m_IsVanillaWeapons = true;
		m_apPlayers[DummyId]->m_IsVanillaCompetetive = true;
	}

	OnClientEnter(DummyId, Silent);

	return DummyId;
}

bool CGameContext::CheckIpJailed(int ClientId)
{
	if(!m_apPlayers[ClientId])
		return false;
	NETADDR Addr;
	Server()->GetClientAddr(ClientId, &Addr);
	Addr.port = 0;
	for(int i = 0; i < m_NumJailIps; i++)
	{
		if(!net_addr_comp(&Addr, &m_aJailIps[i]))
		{
			SendChatTarget(ClientId, "[JAIL] you have been jailed for 2 minutes.");
			m_apPlayers[ClientId]->JailPlayer(120);
			return true;
		}
	}
	return false;
}

void CGameContext::SetIpJailed(int ClientId)
{
	char aBuf[128];
	int Found = 0;
	NETADDR NoPortAddr;
	Server()->GetClientAddr(ClientId, &NoPortAddr);
	NoPortAddr.port = 0;
	// find a matching Mute for this ip, update expiration time if found
	for(int i = 0; i < m_NumJailIps; i++)
	{
		if(net_addr_comp(&m_aJailIps[i], &NoPortAddr) == 0)
		{
			Found = 1;
			break;
		}
	}
	if(!Found) // nothing found so far, find a free slot..
	{
		if(m_NumJailIps < MAX_MUTES)
		{
			m_aJailIps[m_NumJailIps] = NoPortAddr;
			m_NumJailIps++;
			Found = 1;
		}
	}
	if(Found)
	{
		str_format(aBuf, sizeof aBuf, "'%s' has been ip jailed.", Server()->ClientName(ClientId));
		SendChat(-1, TEAM_ALL, aBuf);
	}
	else // no free slot found
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "mute", "name change mute array is full!");
	}
}

void CGameContext::SaveWrongLogin(const char *pLogin)
{
	if(!g_Config.m_SvSaveWrongLogin)
		return;

	std::ofstream LoginFile(g_Config.m_SvWrongLoginFile, std::ios::app);
	if(!LoginFile)
	{
		dbg_msg("login_sniff", "ERROR1 writing file '%s'", g_Config.m_SvWrongLoginFile);
		g_Config.m_SvSaveWrongLogin = 0;
		LoginFile.close();
		return;
	}

	if(LoginFile.is_open())
	{
		//dbg_msg("login_sniff", "sniffed msg [ %s ]", pLogin);
		LoginFile << pLogin << "\n";
	}
	else
	{
		dbg_msg("login_sniff", "ERROR2 writing file '%s'", g_Config.m_SvWrongLoginFile);
		g_Config.m_SvSaveWrongLogin = 0;
	}

	LoginFile.close();
}

bool CGameContext::AdminChatPing(const char *pMsg)
{
	if(!g_Config.m_SvMinAdminPing)
		return false;

	return std::any_of(std::begin(m_apPlayers), std::end(m_apPlayers), [this, pMsg](const CPlayer *pPlayer) {
		if(!pPlayer)
			return false;
		if(!Server()->GetAuthedState(pPlayer->GetCid()))
			return false;
		if(str_find_nocase(pMsg, Server()->ClientName(pPlayer->GetCid())))
		{
			int len_name = str_length(Server()->ClientName(pPlayer->GetCid()));
			int len_msg = str_length(pMsg);
			if(len_msg - len_name - 2 < g_Config.m_SvMinAdminPing)
				return true;
		}
		return false;
	});
}

bool CGameContext::ShowJoinMessage(int ClientId)
{
	if(!m_apPlayers[ClientId])
		return false;
	if(g_Config.m_SvShowConnectionMessages == CON_SHOW_NONE)
		return false;
	if(g_Config.m_SvHideConnectionMessagesPattern[0]) // if regex filter active
		if(!regex_compile(g_Config.m_SvHideConnectionMessagesPattern, Server()->ClientName(ClientId)))
			return false;
	return true;
}

bool CGameContext::ShowLeaveMessage(int ClientId)
{
	if(!m_apPlayers[ClientId])
		return false;
	if(g_Config.m_SvShowConnectionMessages == CON_SHOW_NONE)
		return false;
	if(g_Config.m_SvShowConnectionMessages == CON_SHOW_JOIN)
		return false;
	if(g_Config.m_SvHideConnectionMessagesPattern[0]) // if regex filter active
		if(!regex_compile(g_Config.m_SvHideConnectionMessagesPattern, Server()->ClientName(ClientId)))
			return false;
	return true;
}

bool CGameContext::ShowTeamSwitchMessage(int ClientId)
{
	if(!m_apPlayers[ClientId])
		return false;
	if(g_Config.m_SvShowConnectionMessages != CON_SHOW_ALL)
		return false;
	if(g_Config.m_SvHideConnectionMessagesPattern[0]) // if regex filter active
		if(!regex_compile(g_Config.m_SvHideConnectionMessagesPattern, Server()->ClientName(ClientId)))
			return false;
	return true;
}

void CGameContext::GetSpreeType(int ClientId, char *pBuf, size_t BufSize, bool IsRecord)
{
	CPlayer *pPlayer = m_apPlayers[ClientId];
	if(!pPlayer)
		return;

	if(pPlayer->m_IsInstaArena_fng && (pPlayer->m_IsInstaArena_gdm || pPlayer->m_IsInstaArena_idm))
	{
		if(pPlayer->m_IsInstaArena_gdm)
			str_copy(pBuf, "boomfng", BufSize);
		else if(pPlayer->m_IsInstaArena_idm)
			str_copy(pBuf, "fng", BufSize);
	}
	else if(pPlayer->m_IsInstaArena_gdm)
	{
		if(IsRecord && pPlayer->m_KillStreak > pPlayer->m_Account.m_GrenadeSpree)
		{
			pPlayer->m_Account.m_GrenadeSpree = pPlayer->m_KillStreak;
			SendChatTarget(pPlayer->GetCid(), "New grenade spree record!");
		}
		str_copy(pBuf, "grenade", BufSize);
	}
	else if(pPlayer->m_IsInstaArena_idm)
	{
		if(IsRecord && pPlayer->m_KillStreak > pPlayer->m_Account.m_RifleSpree)
		{
			pPlayer->m_Account.m_RifleSpree = pPlayer->m_KillStreak;
			SendChatTarget(pPlayer->GetCid(), "New rifle spree record!");
		}
		str_copy(pBuf, "rifle", BufSize);
	}
	else if(pPlayer->m_IsVanillaDmg)
	{
		str_copy(pBuf, "killing", BufSize);
	}
	else //no insta at all
	{
		if(IsRecord && pPlayer->m_KillStreak > pPlayer->m_BlockSpreeHighscore)
		{
			pPlayer->m_BlockSpreeHighscore = pPlayer->m_KillStreak;
			SendChatTarget(pPlayer->GetCid(), "New Blockspree record!");
		}
		str_copy(pBuf, "blocking", BufSize);
	}
}

void CGameContext::CallVetoVote(int ClientId, const char *pDesc, const char *pCmd, const char *pReason, const char *pChatmsg, const char *pSixupDesc)
{
	// check if a vote is already running
	if(m_VoteCloseTime)
		return;

	m_IsDDPPVetoVote = true;
	if(ClientId == -1) //Server vote
	{
		SendChat(-1, TEAM_ALL, pChatmsg);
		if(!pSixupDesc)
			pSixupDesc = pDesc;

		StartVote(pDesc, pCmd, pReason, pSixupDesc);
		m_VoteCreator = ClientId;
		return;
	}
	else
		CallVote(ClientId, pDesc, pCmd, pReason, pChatmsg, pSixupDesc);
}
