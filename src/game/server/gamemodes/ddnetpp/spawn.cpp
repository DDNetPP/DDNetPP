#include <base/system.h>
#include <game/mapitems_ddpp.h>

#include "ddnetpp.h"

bool CGameControllerDDNetPP::CanSpawn(int Team, vec2 *pOutPos, class CPlayer *pPlayer, int DDTeam)
{
	// spectators can't spawn
	if(Team == TEAM_SPECTATORS)
		return false;

	CSpawnEval Eval;
	CCharacter *pChr = pPlayer->GetCharacter();

	if(pPlayer->DummyMode() == DUMMYMODE_SHOPBOT)
	{
		vec2 ShopSpawn = GameServer()->Collision()->GetRandomTile(TILE_SHOP_SPAWN);

		if(ShopSpawn != vec2(-1, -1))
		{
			Eval.m_Pos = ShopSpawn;
			Eval.m_Got = true;
		}
		else // no shop spawn tile -> fallback to shop tile
		{
			vec2 ShopTile = GameServer()->Collision()->GetRandomTile(TILE_SHOP);

			if(ShopTile != vec2(-1, -1))
			{
				Eval.m_Pos = ShopTile;
				Eval.m_Got = true;
				if(pChr)
					pChr->m_IsFreeShopBot = true;
			}
			else // no shop tile
			{
				GameServer()->SendChatTarget(pPlayer->GetCid(), "No shop spawn set.");
			}
		}
	}
	else if(pPlayer->m_DummySpawnTile)
	{
		vec2 SpawnTile(0.0f, 0.0f);
		if(pPlayer->m_DummySpawnTile == 1)
			SpawnTile = GameServer()->Collision()->GetRandomTile(TILE_BOTSPAWN_1);
		else if(pPlayer->m_DummySpawnTile == 2)
			SpawnTile = GameServer()->Collision()->GetRandomTile(TILE_BOTSPAWN_2);
		else if(pPlayer->m_DummySpawnTile == 3)
			SpawnTile = GameServer()->Collision()->GetRandomTile(TILE_BOTSPAWN_3);
		else if(pPlayer->m_DummySpawnTile == 4)
			SpawnTile = GameServer()->Collision()->GetRandomTile(TILE_BOTSPAWN_4);

		if(SpawnTile != vec2(-1, -1))
		{
			Eval.m_Pos = SpawnTile;
			Eval.m_Got = true;
		}
		else //no botspawn tile
		{
			dbg_msg("WARNING", "player [%d][%s] failed to botspwan tile=%d",
				pPlayer->GetCid(), Server()->ClientName(pPlayer->GetCid()), pPlayer->m_DummySpawnTile);
			pPlayer->m_DummySpawnTile = 0;
		}
	}
	else if(pPlayer->m_Account.m_JailTime)
	{
		vec2 JailPlayerSpawn = GameServer()->Collision()->GetRandomTile(TILE_JAIL);

		if(JailPlayerSpawn != vec2(-1, -1))
		{
			Eval.m_Pos = JailPlayerSpawn;
			Eval.m_Got = true;
		}
		else //no jailplayer
		{
			GameServer()->SendChatTarget(pPlayer->GetCid(), "No jail set.");
		}
	}
	else if(pPlayer->m_IsSuperModSpawn && !pPlayer->IsInstagibMinigame())
	{
		Eval.m_Pos.x = g_Config.m_SvSuperSpawnX * 32;
		Eval.m_Pos.y = g_Config.m_SvSuperSpawnY * 32;
		Eval.m_Got = true;
	}
	else if(pPlayer->m_IsInstaArena_gdm)
	{
		EvaluateSpawnType(&Eval, 1, DDTeam); //red (not bloody anymore)
	}
	else if(pPlayer->m_IsInstaArena_idm)
	{
		EvaluateSpawnType(&Eval, 2, DDTeam); //blue
	}
	else if(pPlayer->m_IsSurvivaling)
	{
		int Id = pPlayer->GetCid();
		Eval.m_Pos = pPlayer->m_IsSurvivalAlive ? GameServer()->GetNextSurvivalSpawn(Id) : GameServer()->GetSurvivalLobbySpawn(Id);
		if(Eval.m_Pos == vec2(-1, -1)) // fallback to ddr spawn if there is no arena
			EvaluateSpawnType(&Eval, 0, DDTeam); //default
		else
			Eval.m_Got = true;
	}
	else if(pPlayer->m_PendingCaptcha)
	{
		vec2 CaptchaSpawn = GameServer()->Collision()->GetRandomTile(TILE_CAPTCHA_SPAWN);

		if(CaptchaSpawn != vec2(-1, -1))
		{
			if(pChr)
				pChr->SetSolo(true);
			Eval.m_Pos = CaptchaSpawn;
			Eval.m_Got = true;
		}
		else // no captcha spawn!
		{
			// this can happen if sv_captcha_room is set in the config
			// the map is loaded after the config is executed
			// so the con chain hook does not work
			GameServer()->SendChat(-1, TEAM_ALL, "ERROR: deactivating sv_captcha_room because the captcha spawn tile is missing!");
			g_Config.m_SvCaptchaRoom = 0;
		}
	}
	else if(pPlayer->m_IsBlockWaving && !pPlayer->m_IsBlockWaveWaiting)
	{
		// TODO: move to minigame
		if(pPlayer->m_IsDummy)
		{
			vec2 BlockWaveSpawnTile = GameServer()->Collision()->GetRandomTile(TILE_BLOCKWAVE_BOT);

			if(BlockWaveSpawnTile != vec2(-1, -1))
			{
				Eval.m_Pos = BlockWaveSpawnTile;
				Eval.m_Got = true;
			}
			else //no BlockWaveSpawnTile
			{
				//GameServer()->SendChatTarget(m_pPlayer->GetCid(), "[BlockWave] No arena set.");
				GameServer()->m_BlockWaveGameState = 0;
			}
		}
		else
		{
			vec2 BlockWaveSpawnTile = GameServer()->Collision()->GetRandomTile(TILE_BLOCKWAVE_HUMAN);

			if(BlockWaveSpawnTile != vec2(-1, -1))
			{
				Eval.m_Pos = BlockWaveSpawnTile;
				Eval.m_Got = true;
			}
			else //no BlockWaveSpawnTile
			{
				GameServer()->SendChatTarget(pPlayer->GetCid(), "[BlockWave] No arena set.");
				GameServer()->m_BlockWaveGameState = 0;
			}
		}
	}
	else if(pPlayer->m_IsBlockDeathmatch)
	{
		// TODO: move to minigame
		if(g_Config.m_SvBlockDMarena == 1)
		{
			vec2 BlockDMSpawn = GameServer()->Collision()->GetRandomTile(TILE_BLOCK_DM_A1);
			if(BlockDMSpawn != vec2(-1, -1))
			{
				Eval.m_Pos = BlockDMSpawn;
				Eval.m_Got = true;
			}
			else
			{
				GameServer()->SendChatTarget(pPlayer->GetCid(), "[BLOCK] no deathmatch arena 1 found.");
				pPlayer->m_IsBlockDeathmatch = false;
			}
		}
		else if(g_Config.m_SvBlockDMarena == 2)
		{
			vec2 BlockDMSpawn = GameServer()->Collision()->GetRandomTile(TILE_BLOCK_DM_A2);
			if(BlockDMSpawn != vec2(-1, -1))
			{
				Eval.m_Pos = BlockDMSpawn;
				Eval.m_Got = true;
			}
			else
			{
				GameServer()->SendChatTarget(pPlayer->GetCid(), "[BLOCK] no deathmatch arena 2 found.");
				pPlayer->m_IsBlockDeathmatch = false;
			}
		}
		else
		{
			dbg_msg("WARNING", "Invalid block deathmatch arena");
		}
	}
	else if(pPlayer->m_IsBalanceBatteling || pPlayer->m_IsBalanceBattleDummy)
	{
		// TODO: move to minigame
		if(pPlayer->m_IsBalanceBattlePlayer1)
		{
			vec2 BalanceBattleSpawn = GameServer()->Collision()->GetRandomTile(TILE_BALANCE_BATTLE_1);

			if(BalanceBattleSpawn != vec2(-1, -1))
			{
				Eval.m_Pos = BalanceBattleSpawn;
				Eval.m_Got = true;
			}
			else //no balance battle spawn tile
			{
				GameServer()->SendChatTarget(pPlayer->GetCid(), "[balance] no battle arena found.");
				pPlayer->m_IsBalanceBatteling = false;
			}
		}
		else
		{
			vec2 BalanceBattleSpawn = GameServer()->Collision()->GetRandomTile(TILE_BALANCE_BATTLE_2);

			if(BalanceBattleSpawn != vec2(-1, -1))
			{
				Eval.m_Pos = BalanceBattleSpawn;
				Eval.m_Got = true;
			}
			else //no balance battle spawn tile
			{
				GameServer()->SendChatTarget(pPlayer->GetCid(), "[balance] no battle arena found.");
				pPlayer->m_IsBalanceBatteling = false;
			}
		}
	}
	else
	{
		bool IsMinigameSpawn = false;
		for(auto &Minigame : GameServer()->m_vMinigames)
		{
			if(Minigame->PickSpawn(&Eval.m_Pos, pPlayer))
			{
				Eval.m_Got = true;
				IsMinigameSpawn = true;
				break;
			}
		}
		if(!IsMinigameSpawn)
		{
			if(pPlayer->m_IsNoboSpawn)
			{
				char aBuf[128];
				if(pPlayer->m_NoboSpawnStop > Server()->Tick())
				{
					str_format(aBuf, sizeof(aBuf), "[NoboSpawn] Time until real spawn is unlocked: %" PRId64 " sec", (pPlayer->m_NoboSpawnStop - Server()->Tick()) / Server()->TickSpeed());
					GameServer()->SendChatTarget(pPlayer->GetCid(), aBuf);
					Eval.m_Pos.x = g_Config.m_SvNoboSpawnX * 32;
					Eval.m_Pos.y = g_Config.m_SvNoboSpawnY * 32;
					Eval.m_Got = true;
				}
				else
				{
					pPlayer->m_IsNoboSpawn = false;
					str_copy(aBuf, "[NoboSpawn] Welcome to the real spawn!", sizeof(aBuf));
					GameServer()->SendChatTarget(pPlayer->GetCid(), aBuf);
					EvaluateSpawnType(&Eval, 0, DDTeam); //default
				}
			}
			else
			{
				EvaluateSpawnType(&Eval, 0, DDTeam); //default
			}
		}
	}

	if(Eval.m_Got)
	{
		*pOutPos = Eval.m_Pos;
		return true;
	}

	return CGameControllerDDRace::CanSpawn(Team, pOutPos, pPlayer, DDTeam);
}
