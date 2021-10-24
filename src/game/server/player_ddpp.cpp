/* CPlayer related ddnet++ methods */

#include "gamemodes/DDRace.h"
#include <engine/shared/config.h>

#include <fstream>
#include <limits>

#include "player.h"

void CPlayer::FixForNoName(int ID)
{
	m_FixNameID = ID; // 0 for just to display the name in the right moment (e.g. kill msg killer)
	m_SetRealName = true;
	m_SetRealNameTick = Server()->Tick() + Server()->TickSpeed() / 20;
}

void CPlayer::ResetDDPP()
{
	/*****************************
	*        DDNetPP             *
	******************************/

	//ChillerDragon constructor Konstructor init
	if(g_Config.m_SvTestingCommands)
	{
		str_format(m_aAsciiFrame0, sizeof(m_aAsciiFrame0), "x");
		str_format(m_aAsciiFrame1, sizeof(m_aAsciiFrame1), "+");
		str_format(m_aAsciiFrame2, sizeof(m_aAsciiFrame2), "++");
		str_format(m_aAsciiFrame3, sizeof(m_aAsciiFrame3), "xxx");
		str_format(m_aAsciiFrame4, sizeof(m_aAsciiFrame4), "++++");
		str_format(m_aAsciiFrame5, sizeof(m_aAsciiFrame5), "xxxxx");
		str_format(m_aAsciiFrame6, sizeof(m_aAsciiFrame6), "++++++");
		str_format(m_aAsciiFrame7, sizeof(m_aAsciiFrame7), "xxxxxxx");
		str_format(m_aAsciiFrame8, sizeof(m_aAsciiFrame8), "++++++++");
		str_format(m_aAsciiFrame9, sizeof(m_aAsciiFrame9), "ChillerDragon's sample animation");
		str_format(m_aAsciiFrame10, sizeof(m_aAsciiFrame10), "ChillerDragon's sample animation");
		str_format(m_aAsciiFrame11, sizeof(m_aAsciiFrame11), "ChillerDragon's sample animation");
		str_format(m_aAsciiFrame12, sizeof(m_aAsciiFrame12), "ChillerDragon's sample animation");
		str_format(m_aAsciiFrame13, sizeof(m_aAsciiFrame13), ".");
		str_format(m_aAsciiFrame14, sizeof(m_aAsciiFrame14), ":");
		str_format(m_aAsciiFrame15, sizeof(m_aAsciiFrame15), ".:.");
	}
	for(unsigned i = 0; i < sizeof(m_aCatchedID) / sizeof(m_aCatchedID[0]); i++)
		m_aCatchedID[i] = -1;

	if(GameServer()->IsDDPPgametype("fly"))
	{
		m_IsVanillaDmg = true;
		m_IsVanillaWeapons = true;
	}

	m_MoneyTilesMoney = 0;
	str_copy(m_aTradeOffer, "", sizeof(m_aTradeOffer));
	str_copy(m_aEscapeReason, "unknown", 16);
	m_dmm25 = -1; //set to offline default
	m_MapSaveLoaded = false;

	if(g_Config.m_SvNoboSpawnTime)
	{
		m_IsNoboSpawn = true;
	}
	m_AccountID = 0; // SetAccID(0); the function shows old value which could cause undefined behaviour i guess
	m_PlayerHumanLevel = 0;
	m_HumanLevelTime = 0;
	m_NoboSpawnStop = Server()->Tick() + Server()->TickSpeed() * (60 * g_Config.m_SvNoboSpawnTime);
	m_QuestPlayerID = -1;
	m_JailHammer = true;
	str_format(m_aAsciiPublishState, sizeof(m_aAsciiPublishState), "0000");
	m_AsciiWatchingID = -1;
	m_AsciiAnimSpeed = 10;
	str_format(m_HashSkin, sizeof(m_HashSkin), "#");
	m_ChilliWarnings = 0;
	m_TROLL166 = false;
	m_TROLL420 = false;
	m_Dummy_nn_time = 0;
	m_Dummy_nn_latest_fitness = 0.0f;
	m_Dummy_nn_highest_fitness = 0.0f;
	m_Dummy_nn_latest_Distance = 0.0f;
	m_Dummy_nn_highest_Distance = 0.0f;
	m_Dummy_nn_highest_Distance_touched = 0.0f;
	m_Minigameworld_size_x = 30;
	m_ci_lowest_dest_dist = 2147483646; //max long len 2147483647
	m_ci_latest_dest_dist = 0;
	m_Insta1on1_id = -1;
	m_BalanceBattle_id = -1;
	m_TradeItem = -1;
	m_TradeMoney = -1;
	m_TradeID = -1;
	//m_aFngConfig[0] = '0';
	//m_aFngConfig[1] = '0';
	//m_aFngConfig[2] = '0';
	//m_aFngConfig[3] = '0';
	str_format(m_aFngConfig, sizeof(m_aFngConfig), "0000");

	//ShowHideConfig

	str_copy(m_aShowHideConfig, "0010000000", sizeof(m_aShowHideConfig));
	//dbg_msg("debug", "init player showhide='%s'", m_aShowHideConfig);
	m_ShowBlockPoints = GameServer()->CharToBool(m_aShowHideConfig[0]); //doing it manually becuase the gamecontext function cant be called here
	m_HideBlockXp = GameServer()->CharToBool(m_aShowHideConfig[1]);
	m_xpmsg = GameServer()->CharToBool(m_aShowHideConfig[2]);
	m_hidejailmsg = GameServer()->CharToBool(m_aShowHideConfig[3]);
	m_HideInsta1on1_killmessages = GameServer()->CharToBool(m_aShowHideConfig[4]);
	m_HideQuestProgress = GameServer()->CharToBool(m_aShowHideConfig[5]);
	m_HideQuestWarning = GameServer()->CharToBool(m_aShowHideConfig[6]);
	//GameServer()->ShowHideConfigCharToBool(this->GetCID()); //cant be called because somehow players doesnt exist for gameconext idk
	//str_format(m_aShowHideConfig, sizeof(m_aShowHideConfig), "%s", "0010000000000000"); // <3
	//m_xpmsg = true;

	m_LoginData.m_LoginState = LOGIN_OFF;

	// disable infinite cosmetics by default
	m_InfRainbow = false;
	m_InfBloody = false;
	m_InfAtom = false;
	m_InfTrail = false;
	m_InfAutoSpreadGun = false;
	// disable cosmetic offers by default
	m_rainbow_offer = false;
	m_bloody_offer = false;
	m_atom_offer = false;
	m_trail_offer = false;
	m_autospreadgun_offer = false;
	//Block points
	m_LastToucherID = -1;
	m_DisplayScore = SCORE_LEVEL;
}

void CPlayer::DDPPTick()
{
	//ChillerDragon chidraqul3 the hash game
	if(m_C3_GameState == 1) //singleplayer
	{
		chidraqul3_GameTick();
	}

	//profile views
	if(Server()->Tick() % 1000 == 0)
	{
		m_IsProfileViewLoaded = true;
		//GameServer()->SendChatTarget(m_ClientID, "View loaded");
	}

	//bomb
	if(m_BombBanTime)
	{
		m_BombBanTime--;
		if(m_BombBanTime == 1)
		{
			GameServer()->SendChatTarget(m_ClientID, "Bomb bantime expired.");
		}
	}

	if(Server()->Tick() % (Server()->TickSpeed() * 300) == 0)
		if(IsLoggedIn())
			Save(1); //SetLoggedIn true

	//dragon test chillers level system xp money usw am start :3
	CheckLevel();

	ThreadLoginDone();

	if(m_ChangeTeamOnFlag || (Server()->Tick() % 600 == 0))
	{
		if((((CGameControllerDDRace *)GameServer()->m_pController)->HasFlag(GetCharacter()) == -1) && m_IsDummy && ((g_Config.m_SvShowBotsInScoreboard == 1 && (m_DummyMode >= -6 && m_DummyMode <= -1)) || g_Config.m_SvShowBotsInScoreboard == 0))
		{
			m_Team = TEAM_BLUE;
		}
		else
		{
			m_Team = TEAM_RED;
		}
		m_ChangeTeamOnFlag = false;
	}

	if(m_SetRealName)
	{
		if(m_SetRealNameTick < Server()->Tick())
		{
			if(m_FixNameID == 1)
				GameServer()->SendChat(m_ClientID, m_ChatTeam, m_ChatText, m_ClientID);
			else if(m_FixNameID == 2)
			{
				CNetMsg_Sv_KillMsg Msg;
				Msg.m_Killer = m_MsgKiller;
				Msg.m_Victim = GetCID();
				Msg.m_Weapon = m_MsgWeapon;
				Msg.m_ModeSpecial = m_MsgModeSpecial;
				Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, -1);
			}

			m_SetRealName = false;
		}
	}
	PlayerHumanLevelTick();
}

void CPlayer::PlayerHumanLevelTick()
{
	if(m_HumanLevelTime >= 1)
	{
		m_HumanLevelTime--;
	}

	if(m_PlayerHumanLevel == 0)
	{
		if(GetCharacter() && GetCharacter()->InputActive())
		{
			m_PlayerHumanLevel++;
			m_HumanLevelTime = Server()->TickSpeed() * 10; // 10 sec
		}
	}
	else if(m_PlayerHumanLevel == 1)
	{
		if(m_HumanLevelTime <= 0)
		{
			m_PlayerHumanLevel++;
			m_PlayerHumanLevelState = 0;
		}
	}
	else if(m_PlayerHumanLevel == 2)
	{
		if(Server()->Tick() % 40 == 0)
		{
			if(GetCharacter() && GetCharacter()->InputActive())
			{
				m_PlayerHumanLevelState++;
			}
		}
		if(m_PlayerHumanLevelState > 3)
		{
			m_PlayerHumanLevel++;
			m_HumanLevelTime = Server()->TickSpeed() * 10; // 10 sec
		}
	}
	else if(m_PlayerHumanLevel == 3)
	{
		if(m_HumanLevelTime <= 0)
		{
			m_PlayerHumanLevel++;
			m_PlayerHumanLevelState = 0;
		}
	}
	else if(m_PlayerHumanLevel == 4)
	{
		if(GetCharacter())
		{
			if(GetCharacter()->m_DDRaceState == DDRACE_FINISHED ||
				m_BlockPoints > 5 ||
				IsLoggedIn())
			{
				m_PlayerHumanLevel++;
				m_HumanLevelTime = Server()->TickSpeed() * 20; // 20 sec
			}
		}
	}
	else if(m_PlayerHumanLevel == 5)
	{
		if(m_HumanLevelTime <= 0)
		{
			m_PlayerHumanLevel++;
			m_PlayerHumanLevelState = 0;
		}
	}
	else if(m_PlayerHumanLevel == 6)
	{
		if(m_pCaptcha->IsHuman())
		{
			m_PlayerHumanLevel++;
		}
	}
	else if(m_PlayerHumanLevel == 7)
	{
		if((m_QuestLevelUnlocked > 0 || m_QuestUnlocked > 2) || // played quest until finish map
			m_BlockPoints > 10)
		{
			m_PlayerHumanLevel++;
			m_HumanLevelTime = Server()->TickSpeed() * 60; // 1 min
		}
	}
	else if(m_PlayerHumanLevel == 8)
	{
		if(m_HumanLevelTime <= 0)
		{
			m_PlayerHumanLevel++;
			m_PlayerHumanLevelState = 0;
		}
	}
}

bool CPlayer::DDPPSnapChangeSkin(CNetObj_ClientInfo *pClientInfo)
{
	//spooky ghost
	const char *pClan;
	if(m_SpookyGhostActive)
		pClan = m_RealName;
	else
		pClan = m_RealClan;
	StrToInts(&pClientInfo->m_Clan0, 3, pClan);

	if(m_SpookyGhostActive)
	{
		m_ShowName = false;
	}

	if(m_SetRealName || m_ShowName)
	{
		StrToInts(&pClientInfo->m_Name0, 4, Server()->ClientName(m_ClientID));
	}
	else
	{
		StrToInts(&pClientInfo->m_Name0, 4, " ");
	}

	if(m_PlayerFlags & PLAYERFLAG_SCOREBOARD)
	{
		if(GetCharacter())
		{
			GetCharacter()->m_ShopMotdTick = 0;
		}
	}
	else
	{
		if(GetCharacter())
		{
			GetCharacter()->m_TimesShot = 0;
		}
	}

	if(GetCharacter() && GetCharacter()->m_IsBomb) //bomb (keep bomb 1st. Because bomb over all rainbow and other stuff shoudl be ignored if bomb)
	{
		StrToInts(&pClientInfo->m_Skin0, 6, m_TeeInfos.m_SkinName);
		pClientInfo->m_UseCustomColor = true;

		if(GameServer()->m_BombTick < 75) //red glowup right before explode
		{
			//if (GameServer()->m_bwff) //old not working blackwhite flick flack
			//{
			//	pClientInfo->m_ColorBody = (255 * 255 / 360);
			//	pClientInfo->m_ColorFeet = (255 * 255 / 360);
			//	GameServer()->m_bwff = false;
			//}
			//else
			//{
			//	pClientInfo->m_ColorBody = (0 * 255 / 360);
			//	pClientInfo->m_ColorFeet = (0 * 255 / 360);
			//	GameServer()->m_bwff = true;
			//}

			pClientInfo->m_ColorBody = (GameServer()->m_BombFinalColor * 255 / 1);
			pClientInfo->m_ColorFeet = (GameServer()->m_BombFinalColor * 255 / 1);

			GameServer()->m_BombFinalColor++;
		}
		else
		{
			int ColorChangeVal = (255000 - GameServer()->m_BombTick) * 0.0001;
			if(!ColorChangeVal)
			{
				ColorChangeVal = 1;
			}

			if(GameServer()->m_BombColor > 254)
			{
				GameServer()->m_bwff = false;
			}
			if(GameServer()->m_BombColor < 1)
			{
				GameServer()->m_bwff = true;
			}

			if(GameServer()->m_bwff) //black -> white
			{
				GameServer()->m_BombColor += ColorChangeVal;
			}
			else //white -> black
			{
				GameServer()->m_BombColor -= ColorChangeVal;
			}

			pClientInfo->m_ColorBody = (GameServer()->m_BombColor * 255 / 360);
			pClientInfo->m_ColorFeet = (GameServer()->m_BombColor * 255 / 360);
		}
	}
	else if(m_InfRainbow || GameServer()->IsHooked(GetCID(), 1) || (GetCharacter() && GetCharacter()->m_Rainbow && !GetCharacter()->m_IsBombing)) //rainbow (hide finit rainbow if in bomb game)
	{
		StrToInts(&pClientInfo->m_Skin0, 6, m_TeeInfos.m_SkinName);
		pClientInfo->m_UseCustomColor = true;
		m_RainbowColor = (m_RainbowColor + 1) % 256;
		pClientInfo->m_ColorBody = m_RainbowColor * 0x010000 + 0xff00;
		pClientInfo->m_ColorFeet = m_RainbowColor * 0x010000 + 0xff00;
	}
	else
		return false;
	return true;
}

void CPlayer::DDPPSnapChangePlayerInfo(int SnappingClient, CPlayer *pSnapping, CNetObj_PlayerInfo *pPlayerInfo)
{
	// send 0 if times of others are not shown
	if(SnappingClient != m_ClientID && g_Config.m_SvHideScore)
	{
		pPlayerInfo->m_Score = -9999;
	}
	else if(pSnapping->IsInstagibMinigame())
	{
		if(IsInstagibMinigame())
		{
			if(pSnapping->m_ScoreFixForDDNet)
				pPlayerInfo->m_Score = m_InstaScore * 60;
			else
				pPlayerInfo->m_Score = m_InstaScore;
		}
		else
			pPlayerInfo->m_Score = -9999;
	}
	else if(pSnapping->m_IsSurvivaling)
	{
		if(m_IsSurvivaling)
		{
			if(pSnapping->m_ScoreFixForDDNet)
				pPlayerInfo->m_Score = m_SurvivalKills * 60;
			else
				pPlayerInfo->m_Score = m_SurvivalKills;
		}
		else
			pPlayerInfo->m_Score = -9999;
	}
	else if(pSnapping->m_DisplayScore != SCORE_TIME)
	{
		if(pSnapping->m_DisplayScore == SCORE_LEVEL)
		{
			if(IsLoggedIn())
			{
				if(pSnapping->m_ScoreFixForDDNet)
					pPlayerInfo->m_Score = GetLevel() * 60;
				else
					pPlayerInfo->m_Score = GetLevel();
			}
			else if(pSnapping->m_ScoreFixForDDNet)
				pPlayerInfo->m_Score = -9999;
			else
				pPlayerInfo->m_Score = 0;
		}
		else if(pSnapping->m_DisplayScore == SCORE_BLOCK)
		{
			if(IsLoggedIn())
			{
				if(pSnapping->m_ScoreFixForDDNet)
					pPlayerInfo->m_Score = m_BlockPoints * 60;
				else
					pPlayerInfo->m_Score = m_BlockPoints;
			}
			else if(pSnapping->m_ScoreFixForDDNet)
				pPlayerInfo->m_Score = -9999;
			else
				pPlayerInfo->m_Score = 0;
		}
	}
	else
	{
		if(g_Config.m_SvInstagibMode || !g_Config.m_SvDDPPscore)
		{
			pPlayerInfo->m_Score = m_Score;
		}
		else
		{
			pPlayerInfo->m_Score = abs(m_Score) * -1;
		}
	}
}

void CPlayer::OnDisconnectDDPP()
{
	if(m_Insta1on1_id != -1 && (m_IsInstaArena_gdm || m_IsInstaArena_idm))
	{
		GameServer()->WinInsta1on1(m_Insta1on1_id, GetCID());
	}
	if(m_JailTime)
	{
		GameServer()->SetIpJailed(GetCID());
	}
}

void CPlayer::Logout(int SetLoggedIn)
{
	if(!IsLoggedIn())
		return;

	Save(SetLoggedIn);
	dbg_msg("account", "logging out AccountID=%d SetLoggedIn=%d", GetAccID(), SetLoggedIn);

	//reset values to default to prevent cheating
	SetAccID(0);
	m_level = 0;
	m_IsModerator = 0;
	m_IsSuperModerator = 0;
	m_IsAccFrozen = 0;
	m_xp = 0;
	m_neededxp = 0;
	m_money = 0;
	m_shit = 0;
	//m_LastGift = Server(WhatEver)->Trick ** 420; //let gift delay also last in logout makes sense
	m_PoliceRank = 0;
	//m_JailTime = 0; //logout doesnt release :p
	//m_EscapeTime = 0;
	m_TaserLevel = 0;
	m_NinjaJetpackBought = 0;
	m_SpookyGhost = 0;
	m_UseSpawnWeapons = 0;
	m_SpawnWeaponShotgun = 0;
	m_SpawnWeaponGrenade = 0;
	m_SpawnWeaponRifle = 0;
	m_TaserOn = false;
	m_pvp_arena_tickets = 0;
	m_pvp_arena_games_played = 0;
	m_pvp_arena_kills = 0;
	m_pvp_arena_deaths = 0;
	m_ProfileStyle = 0;
	m_ProfileViews = 0;
	m_ProfileStatus[0] = '\0';
	m_ProfileSkype[0] = '\0';
	m_ProfileYoutube[0] = '\0';
	m_ProfileEmail[0] = '\0';
	m_ProfileHomepage[0] = '\0';
	m_ProfileTwitter[0] = '\0';
	m_homing_missiles_ammo = 0;
	m_BlockPoints = 0;
	m_BlockPoints_Kills = 0;
	m_BlockPoints_Deaths = 0;
	m_BombGamesPlayed = 0;
	m_BombGamesWon = 0;
	//m_BombBanTime = 0; //could be set to 0 because you need to belogged in anyways to play bomb            ...but yolo comments are kewl and stuff
	m_GrenadeKills = 0;
	m_GrenadeDeaths = 0;
	m_GrenadeSpree = 0;
	m_GrenadeShots = 0;
	m_GrenadeShotsNoRJ = 0;
	m_GrenadeWins = 0;
	m_RifleKills = 0;
	m_RifleDeaths = 0;
	m_RifleDeaths = 0;
	m_RifleShots = 0;
	m_RifleWins = 0;
	m_money_transaction9[0] = '\0';
	m_money_transaction8[0] = '\0';
	m_money_transaction7[0] = '\0';
	m_money_transaction6[0] = '\0';
	m_money_transaction5[0] = '\0';
	m_money_transaction4[0] = '\0';
	m_money_transaction3[0] = '\0';
	m_money_transaction2[0] = '\0';
	m_money_transaction1[0] = '\0';
	m_money_transaction0[0] = '\0';
}

void CPlayer::JailPlayer(int seconds)
{
	vec2 JailPlayerSpawn = GameServer()->Collision()->GetRandomTile(TILE_JAIL);
	//vec2 DefaultSpawn = GameServer()->Collision()->GetRandomTile(ENTITY_SPAWN);

	m_JailTime = Server()->TickSpeed() * seconds;

	if(GetCharacter())
	{
		if(JailPlayerSpawn != vec2(-1, -1))
		{
			GetCharacter()->SetPosition(JailPlayerSpawn);
		}
		else //no jailplayer
		{
			//GetCharacter()->SetPosition(DefaultSpawn); //crashbug for mod stealer
			GameServer()->SendChatTarget(GetCID(), "No jail set.");
		}
	}
}

void CPlayer::ChangePassword() //DROPS AN : "NO SUCH COLUM %m_aChangePassword%" SQLite ERROR
{
	if(!IsLoggedIn())
		return;

	dbg_msg("sql", "pass: %s id: %d", m_aChangePassword, GetAccID());
	GameServer()->ExecuteSQLf("UPDATE `Accounts` SET `Password` = '%q'  WHERE `ID` = %i", m_aChangePassword, GetAccID());
}

void CPlayer::Save(int SetLoggedIn)
{
#if defined(CONF_DEBUG)
	dbg_msg("account", "saving account '%s' CID=%d AccountID=%d SetLoggedIn=%d", Server()->ClientName(GetCID()), GetCID(), GetAccID(), SetLoggedIn);
#endif
	if(!IsLoggedIn())
		return;

	if(m_IsFileAcc)
	{
		SaveFileBased(SetLoggedIn);
		return;
	}

	// Proccess Clan Data...
	char aClan[32];
	str_copy(aClan, Server()->ClientClan(m_ClientID), sizeof(aClan));

	if(str_comp(aClan, m_aClan1) && str_comp(aClan, m_aClan2) && str_comp(aClan, m_aClan3))
	{
		//dbg_msg("save", "update clan '%s'", aClan);
		str_format(m_aClan3, sizeof(m_aClan3), "%s", m_aClan2);
		str_format(m_aClan2, sizeof(m_aClan2), "%s", m_aClan1);
		str_format(m_aClan1, sizeof(m_aClan1), "%s", aClan);
	}

	// Proccess IP ADDR...
	char aIP[32];
	Server()->GetClientAddr(GetCID(), aIP, sizeof(aIP));

	if(str_comp(aIP, m_aIP_1) && str_comp(aIP, m_aIP_2) && str_comp(aIP, m_aIP_3))
	{
		//dbg_msg("save", "updated ip '%s'", aIP);
		str_format(m_aIP_3, sizeof(m_aIP_3), "%s", m_aIP_2);
		str_format(m_aIP_2, sizeof(m_aIP_2), "%s", m_aIP_1);
		str_format(m_aIP_1, sizeof(m_aIP_1), "%s", aIP);
	}

	// Proccess IngameName Data...
	char aName[32];
	str_copy(aName, Server()->ClientName(m_ClientID), sizeof(aName));

	if(!str_comp(aName, m_LastLogoutIGN1) || !str_comp(aName, m_LastLogoutIGN2) || !str_comp(aName, m_LastLogoutIGN3) || !str_comp(aName, m_LastLogoutIGN4) || !str_comp(aName, m_LastLogoutIGN5))
	{
		if(!str_comp(aName, m_LastLogoutIGN1))
		{
			m_iLastLogoutIGN1_usage++;
		}
		else if(!str_comp(aName, m_LastLogoutIGN2))
		{
			m_iLastLogoutIGN2_usage++;
		}
		else if(!str_comp(aName, m_LastLogoutIGN3))
		{
			m_iLastLogoutIGN3_usage++;
		}
		else if(!str_comp(aName, m_LastLogoutIGN4))
		{
			m_iLastLogoutIGN4_usage++;
		}
		else if(!str_comp(aName, m_LastLogoutIGN5))
		{
			m_iLastLogoutIGN5_usage++;
		}
	}
	else // new name --> add it in history and overwrite the oldest
	{
		//dbg_msg("debug", "'%s' was not equal to...", aName);
		//dbg_msg("debug", "'%s'", m_LastLogoutIGN1);
		//dbg_msg("debug", "'%s'", m_LastLogoutIGN2);
		//dbg_msg("debug", "'%s'", m_LastLogoutIGN3);
		//dbg_msg("debug", "'%s'", m_LastLogoutIGN4);
		//dbg_msg("debug", "'%s'", m_LastLogoutIGN5);

		str_format(m_LastLogoutIGN5, sizeof(m_LastLogoutIGN5), "%s", m_LastLogoutIGN4);
		str_format(m_LastLogoutIGN4, sizeof(m_LastLogoutIGN4), "%s", m_LastLogoutIGN3);
		str_format(m_LastLogoutIGN3, sizeof(m_LastLogoutIGN3), "%s", m_LastLogoutIGN2);
		str_format(m_LastLogoutIGN2, sizeof(m_LastLogoutIGN2), "%s", m_LastLogoutIGN1);
		str_format(m_LastLogoutIGN1, sizeof(m_LastLogoutIGN1), "%s", aName);

		m_iLastLogoutIGN5_usage = m_iLastLogoutIGN4_usage;
		m_iLastLogoutIGN4_usage = m_iLastLogoutIGN3_usage;
		m_iLastLogoutIGN3_usage = m_iLastLogoutIGN2_usage;
		m_iLastLogoutIGN2_usage = m_iLastLogoutIGN1_usage;
		m_iLastLogoutIGN1_usage = 0;
	}

	// read showhide bools to char array that is being saved
	// GameServer()->ShowHideConfigBoolToChar(this->GetCID());

	/*
		It was planned to use the function pointer
		to switch between ExecuteSQLf and ExecuteSQLBlockingf
		to ensure execution on mapchange and server shutdown
		but somehow it didnt block anyways :c
		i left the function pointer here in case i pick this up in the future.
	*/
	// void (CGameContext::*ExecSql)(const char *, ...) = &CGameContext::ExecuteSQLBlockingf;
	void (CGameContext::*ExecSql)(const char *, ...) = &CGameContext::ExecuteSQLf;

	(*GameServer().*ExecSql)("UPDATE `Accounts` SET"
				 "  `Password` = '%q', `Level` = '%i', `Exp` = '%llu', `Money` = '%llu', `Shit` = '%i'"
				 ", `LastGift` = '%i'" /*is actually m_GiftDelay*/
				 ", `PoliceRank` = '%i'"
				 ", `JailTime` = '%llu', `EscapeTime` = '%llu'"
				 ", `TaserLevel` = '%i'"
				 ", `NinjaJetpackBought` = '%i'"
				 ", `SpookyGhost` = '%i'"
				 ", `UseSpawnWeapons` = '%i'"
				 ", `SpawnWeaponShotgun` = '%i'"
				 ", `SpawnWeaponGrenade` = '%i'"
				 ", `SpawnWeaponRifle` = '%i'"
				 ", `PvPArenaTickets` = '%i', `PvPArenaGames` = '%i', `PvPArenaKills` = '%i', `PvPArenaDeaths` = '%i'"
				 ", `ProfileStyle` = '%i', `ProfileViews` = '%i', `ProfileStatus` = '%q', `ProfileSkype` = '%q', `ProfileYoutube` = '%q', `ProfileEmail` = '%q', `ProfileHomepage` = '%q', `ProfileTwitter` = '%q'"
				 ", `HomingMissiles` = '%i'"
				 ", `BlockPoints` = '%i', `BlockKills` = '%i', `BlockDeaths` = '%i', `BlockSkill` = '%i'"
				 ", `IsModerator` = '%i', `IsSuperModerator` = '%i', `IsSupporter` = '%i',`IsAccFrozen` = '%i', `IsLoggedIn` = '%i'"
				 ", `LastLogoutIGN1` = '%q', `LastLogoutIGN2` = '%q', `LastLogoutIGN3` = '%q', `LastLogoutIGN4` = '%q', `LastLogoutIGN5` = '%q'"
				 ", `IP_1` = '%q', `IP_2` = '%q', `IP_3` = '%q'"
				 ", `Clan1` = '%q', `Clan2` = '%q', `Clan3` = '%q'"
				 ", `Skin` = '%q'"
				 ", `BombGamesPlayed` = '%i', `BombGamesWon` = '%i', `BombBanTime` = '%i'"
				 ", `GrenadeKills` = '%i', `GrenadeDeaths` = '%i', `GrenadeSpree` = '%i', `GrenadeShots` = '%i',  `GrenadeShotsNoRJ` = '%i', `GrenadeWins` = '%i'"
				 ", `RifleKills` = '%i', `RifleDeaths` = '%i', `RifleSpree` = '%i', `RifleShots` = '%i', `RifleWins` = '%i', `FngConfig` = '%q'"
				 ", `ShowHideConfig` = '%q'"
				 ", `SurvivalKills` = '%i', `SurvivalDeaths` = '%i', `SurvivalWins` = '%i'"
				 ", `AsciiState` = '%q', `AsciiViewsDefault` = '%i', `AsciiViewsProfile` = '%i'"
				 ", `AsciiFrame0` = '%q', `AsciiFrame1` = '%q', `AsciiFrame2` = '%q', `AsciiFrame3` = '%q', `AsciiFrame4` = '%q', `AsciiFrame5` = '%q', `AsciiFrame6` = '%q', `AsciiFrame7` = '%q', `AsciiFrame8` = '%q', `AsciiFrame9` = '%q', `AsciiFrame10` = '%q', `AsciiFrame11` = '%q', `AsciiFrame12` = '%q', `AsciiFrame13` = '%q', `AsciiFrame14` = '%q', `AsciiFrame15` = '%q'"
				 " WHERE `ID` = '%i'",
		m_aAccountPassword, m_level, m_xp, m_money, m_shit,
		m_GiftDelay,
		m_PoliceRank,
		m_JailTime, m_EscapeTime,
		m_TaserLevel,
		m_NinjaJetpackBought,
		m_SpookyGhost,
		m_UseSpawnWeapons,
		m_SpawnWeaponShotgun,
		m_SpawnWeaponGrenade,
		m_SpawnWeaponRifle,
		m_pvp_arena_tickets, m_pvp_arena_games_played, m_pvp_arena_kills, m_pvp_arena_deaths,
		m_ProfileStyle, m_ProfileViews, m_ProfileStatus, m_ProfileSkype, m_ProfileYoutube, m_ProfileEmail, m_ProfileHomepage, m_ProfileTwitter,
		m_homing_missiles_ammo,
		m_BlockPoints, m_BlockPoints_Kills, m_BlockPoints_Deaths, m_BlockSkill,
		m_IsModerator, m_IsSuperModerator, m_IsSupporter, m_IsAccFrozen, SetLoggedIn,
		m_LastLogoutIGN1, m_LastLogoutIGN2, m_LastLogoutIGN3, m_LastLogoutIGN4, m_LastLogoutIGN5,
		m_aIP_1, m_aIP_2, m_aIP_3,
		m_aClan1, m_aClan2, m_aClan3,
		m_TeeInfos.m_SkinName,
		m_BombGamesPlayed, m_BombGamesWon, m_BombBanTime,
		m_GrenadeKills, m_GrenadeDeaths, m_GrenadeSpree, m_GrenadeShots, m_GrenadeShotsNoRJ, m_GrenadeWins,
		m_RifleKills, m_RifleDeaths, m_RifleSpree, m_RifleShots, m_RifleWins, m_aFngConfig,
		m_aShowHideConfig,
		m_SurvivalKills, m_SurvivalDeaths, m_SurvivalWins,
		m_aAsciiPublishState, m_AsciiViewsDefault, m_AsciiViewsProfile,
		m_aAsciiFrame0, m_aAsciiFrame1, m_aAsciiFrame2, m_aAsciiFrame3, m_aAsciiFrame4, m_aAsciiFrame5, m_aAsciiFrame6, m_aAsciiFrame7, m_aAsciiFrame8, m_aAsciiFrame9, m_aAsciiFrame10, m_aAsciiFrame11, m_aAsciiFrame12, m_aAsciiFrame13, m_aAsciiFrame14, m_aAsciiFrame15,
		m_AccountID);
}

void CPlayer::SaveFileBased(int SetLoggedIn)
{
	std::string data;
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "%s/%s.acc", g_Config.m_SvFileAccPath, m_aAccountLoginName);
	std::ofstream Acc2File(aBuf);

	if(Acc2File.is_open())
	{
		dbg_msg("acc2", "saved acc '%s'", m_aAccountLoginName);

		Acc2File << m_aAccountPassword << "\n";
		Acc2File << SetLoggedIn << "\n";
		Acc2File << g_Config.m_SvPort << "\n";
		Acc2File << m_IsAccFrozen << "\n";
		Acc2File << m_IsModerator << "\n";
		Acc2File << m_IsSuperModerator << "\n";
		Acc2File << m_IsSupporter << "\n";
		Acc2File << m_money << "\n";
		Acc2File << m_level << "\n";
		Acc2File << m_xp << "\n";
		Acc2File << m_shit << "\n";
		Acc2File << m_PoliceRank << "\n";
		Acc2File << m_TaserLevel << "\n";

		Acc2File.close();
	}
	else
	{
		dbg_msg("acc2", "[WARNING] account '%s' (%s) failed to save", m_aAccountLoginName, aBuf);
		Acc2File.close();
	}
}

void CPlayer::CalcExp()
{
	int64 OldNeededXp = m_neededxp;
	dbg_msg("account", "CalcExp() neededxp=%lld xp=%lld", OldNeededXp, m_xp);

	//										xp diff
	if(m_level == 0)
		m_neededxp = 5000;
	else if(m_level == 1) //5 000
		m_neededxp = 15000;
	else if(m_level == 2) //10 000
		m_neededxp = 25000;
	else if(m_level == 3) //10 000
		m_neededxp = 35000;
	else if(m_level == 4) //10 000
		m_neededxp = 50000;
	else if(m_level == 5) //15 000			Rainbow
		m_neededxp = 65000;
	else if(m_level == 6) //15 000
		m_neededxp = 80000;
	else if(m_level == 7) //15 000
		m_neededxp = 100000;
	else if(m_level == 8) //20 000
		m_neededxp = 120000;
	else if(m_level == 9) //20 000
		m_neededxp = 130000;
	else if(m_level == 10) //30 000
		m_neededxp = 160000;
	else if(m_level == 11) //30 000
		m_neededxp = 200000;
	else if(m_level == 12) //40 000
		m_neededxp = 240000;
	else if(m_level == 13) //40 000
		m_neededxp = 280000;
	else if(m_level == 14) //40 000
		m_neededxp = 325000;
	else if(m_level == 15) //45 000			Bloody
		m_neededxp = 370000;
	else if(m_level == 16) //50 000			room_key
		m_neededxp = 420000;
	else if(m_level == 17) //50 000
		m_neededxp = 470000;
	else if(m_level == 18) //50 000			Police[1]
		m_neededxp = 520000;
	else if(m_level == 19) //50 000
		m_neededxp = 600000;
	else if(m_level == 20) //80 000
		m_neededxp = 680000;
	else if(m_level == 21) //80 000			Ninja jetpack
		m_neededxp = 760000;
	else if(m_level == 22) //90 000
		m_neededxp = 850000;
	else if(m_level == 23) //100 000
		m_neededxp = 950000;
	else if(m_level == 24) //150 000
		m_neededxp = 1200000;
	else if(m_level == 25) //200 000			Police[2]		policehelper && jail codes
		m_neededxp = 1400000;
	else if(m_level == 26) //200 000
		m_neededxp = 1600000;
	else if(m_level == 27) //200 000
		m_neededxp = 1800000;
	else if(m_level == 28) //200 000
		m_neededxp = 2000000;
	else if(m_level == 29) //210 000
		m_neededxp = 2210000;
	else if(m_level == 30) //220 000			Police[3]		taser
		m_neededxp = 2430000;
	else if(m_level == 31) //230 000
		m_neededxp = 2660000;
	else if(m_level == 32) //240 000
		m_neededxp = 2900000;
	else if(m_level == 33) //250 000
		m_neededxp = 3150000;
	else if(m_level == 34) //350 000
		m_neededxp = 3500000;
	else if(m_level == 35) //450 000
		m_neededxp = 3950000;
	else if(m_level == 36) //550 000
		m_neededxp = 4500000;
	else if(m_level == 37) //750 000
		m_neededxp = 5250000;
	else if(m_level == 38) //850 000			spawn weapons
		m_neededxp = 6100000;
	else if(m_level == 39) //900 000
		m_neededxp = 7000000;
	else if(m_level == 40) //1 000 000			Police[4]		homing missels
		m_neededxp = 8000000;
	else if(m_level == 41) //1 000 000
		m_neededxp = 9000000;
	else if(m_level == 42) //1 000 000
		m_neededxp = 10000000;
	else if(m_level == 43) //1 000 000
		m_neededxp = 11000000;
	else if(m_level == 44) //1 000 000
		m_neededxp = 12000000;
	else if(m_level == 45) //1 000 000
		m_neededxp = 13000000;
	else if(m_level == 46) //1 000 000
		m_neededxp = 14000000;
	else if(m_level == 47) //1 000 000
		m_neededxp = 15000000;
	else if(m_level == 48) //1 000 000
		m_neededxp = 16000000;
	else if(m_level == 49) //1 000 000
		m_neededxp = 17000000;
	else if(m_level == 50) //1 000 000			Police[5]		'/jail arrest <time>' hammer command
		m_neededxp = 18000000;
	else if(m_level == 51) //1 000 000
		m_neededxp = 19000000;
	else if(m_level == 52) //1 000 000
		m_neededxp = 20000000;
	else if(m_level == 53) //1 000 000
		m_neededxp = 21000000;
	else if(m_level == 54) //1 000 000
		m_neededxp = 22000000;
	else if(m_level == 55) //1 000 000
		m_neededxp = 23000000;
	else if(m_level == 56) //1 000 000
		m_neededxp = 24000000;
	else if(m_level == 57) //1 000 000
		m_neededxp = 25000000;
	else if(m_level == 58) //1 000 000
		m_neededxp = 26000000;
	else if(m_level == 59) //1 000 000
		m_neededxp = 27000000;
	else if(m_level == 60) //1 000 000
		m_neededxp = 28000000;
	else if(m_level == 61) //1 000 000
		m_neededxp = 29000000;
	else if(m_level == 62) //1 000 000
		m_neededxp = 30000000;
	else if(m_level == 63) //1 000 000
		m_neededxp = 31000000;
	else if(m_level == 64) //1 000 000
		m_neededxp = 32000000;
	else if(m_level == 65) //1 000 000
		m_neededxp = 33000000;
	else if(m_level == 66) //1 000 000
		m_neededxp = 34000000;
	else if(m_level == 67) //1 000 000
		m_neededxp = 35000000;
	else if(m_level == 68) //1 000 000
		m_neededxp = 36000000;
	else if(m_level == 69) //1 000 000
		m_neededxp = 37000000;
	else if(m_level == 70) //1 000 000
		m_neededxp = 38000000;
	else if(m_level == 71) //1 000 000
		m_neededxp = 39000000;
	else if(m_level == 72) //1 000 000
		m_neededxp = 40000000;
	else if(m_level == 73) //1 010 000
		m_neededxp = 41010000;
	else if(m_level == 74) //1 010 000
		m_neededxp = 42020000;
	else if(m_level == 75) //1 010 000
		m_neededxp = 43030000;
	else if(m_level == 76) //1 010 000
		m_neededxp = 44040000;
	else if(m_level == 77) //1 010 000
		m_neededxp = 45050000;
	else if(m_level == 78) //1 010 000
		m_neededxp = 46060000;
	else if(m_level == 79) //1 010 000
		m_neededxp = 47070000;
	else if(m_level == 80) //1 010 000
		m_neededxp = 48080000;
	else if(m_level == 81) //1 010 000
		m_neededxp = 49090000;
	else if(m_level == 82) //1 010 000
		m_neededxp = 50100000;
	else if(m_level == 83) //1 010 000
		m_neededxp = 51110000;
	else if(m_level == 84) //1 010 000
		m_neededxp = 52120000;
	else if(m_level == 85) //1 010 000
		m_neededxp = 53130000;
	else if(m_level == 86) //1 010 000
		m_neededxp = 54140000;
	else if(m_level == 87) //1 010 000
		m_neededxp = 55150000;
	else if(m_level == 88) //1 010 000
		m_neededxp = 56160000;
	else if(m_level == 89) //1 010 000
		m_neededxp = 57170000;
	else if(m_level == 90) //1 010 000
		m_neededxp = 58180000;
	else if(m_level == 91) //1 010 000
		m_neededxp = 59190000;
	else if(m_level == 92) //1 010 000
		m_neededxp = 60200000;
	else if(m_level == 93) //1 100 000
		m_neededxp = 61300000;
	else if(m_level == 94) //1 100 000
		m_neededxp = 62400000;
	else if(m_level == 95) //1 100 000
		m_neededxp = 63500000;
	else if(m_level == 96) //1 100 000
		m_neededxp = 64600000;
	else if(m_level == 97) //1 100 000
		m_neededxp = 65700000;
	else if(m_level == 98) //1 100 000
		m_neededxp = 66800000;
	else if(m_level == 99) //1 100 000
		m_neededxp = 67900000;
	else if(m_level == 100) //12 100 000
		m_neededxp = 80000000;
	else if(m_level == 101) //20 000 000
		m_neededxp = 100000000;
	else if(m_level == 102) //20 000 000
		m_neededxp = 120000000;
	else if(m_level == 103) //20 000 000
		m_neededxp = 140000000;
	else if(m_level == 104) //20 000 000
		m_neededxp = 160000000;
	else if(m_level == 105) //20 000 000
		m_neededxp = 180000000;
	else if(m_level == 106) //20 000 000
		m_neededxp = 200000000;
	else if(m_level == 107) //20 000 000
		m_neededxp = 220000000;
	else if(m_level == 108) //20 000 000
		m_neededxp = 240000000;
	else if(m_level == 109) //20 000 000
		m_neededxp = 260000000;
	else
		m_neededxp = 404000000000000; //404 error

	// make sure to update ACC_MAX_LEVEL when adding more level (neededxp has only to be defined until max level - 1)

	if(IsMaxLevel())
	{
		GameServer()->SendChatTarget(m_ClientID, "[ACCOUNT] GRATULATIONS !!! you reached the maximum level.");
		m_xp = OldNeededXp;
		// m_neededxp = OldNeededXp; // covered by the 404 else if ACC_MAX_LEVEL is if branch limit if it is less it uses next levels neededxp which doesnt hurt either
	}
}

void CPlayer::CheckLevel()
{
	if(!IsLoggedIn())
		return;
	if(IsMaxLevel())
		return;

	if(m_neededxp <= 0)
		CalcExp();

	if(m_xp >= m_neededxp)
	{
		m_level++;

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "You are now Level %d!   +50money", m_level);
		GameServer()->SendChatTarget(m_ClientID, aBuf); //woher weiss ich dass? mit dem GameServer()-> und m_Cli...
		MoneyTransaction(+50, "level up");

		CalcExp();
	}
}

void CPlayer::MoneyTransaction(int Amount, const char *Description)
{
	m_money += Amount;
#if defined(CONF_DEBUG)
	if(m_money < 0)
	{
		dbg_msg("MoneyTransaction", "WARNING money went negative! id=%d name=%s value=%lld", GetCID(), Server()->ClientName(GetCID()), m_money);
	}
#endif
	if(!str_comp(Description, ""))
		return;
	char aDesc[64];
	str_format(aDesc, sizeof(aDesc), "%s%d (%s)", Amount > 0 ? "+" : "", Amount, Description);
	str_format(m_money_transaction9, sizeof(m_money_transaction9), "%s", m_money_transaction9);
	str_format(m_money_transaction8, sizeof(m_money_transaction8), "%s", m_money_transaction8);
	str_format(m_money_transaction7, sizeof(m_money_transaction7), "%s", m_money_transaction7);
	str_format(m_money_transaction6, sizeof(m_money_transaction6), "%s", m_money_transaction5);
	str_format(m_money_transaction5, sizeof(m_money_transaction5), "%s", m_money_transaction4);
	str_format(m_money_transaction4, sizeof(m_money_transaction4), "%s", m_money_transaction3);
	str_format(m_money_transaction3, sizeof(m_money_transaction3), "%s", m_money_transaction2);
	str_format(m_money_transaction2, sizeof(m_money_transaction2), "%s", m_money_transaction1);
	str_format(m_money_transaction1, sizeof(m_money_transaction1), "%s", m_money_transaction0);
	str_format(m_money_transaction0, sizeof(m_money_transaction0), "%s", aDesc);
}

bool CPlayer::IsInstagibMinigame()
{
	if(m_IsInstaArena_gdm || m_IsInstaArena_idm || m_IsInstaArena_fng)
		return true;
	return false;
}

void CPlayer::ThreadLoginStart(const char *pUsername, const char *pPassword)
{
	m_LoginData.m_pGameContext = GameServer();
	m_LoginData.m_LoginState = LOGIN_WAIT;
	m_LoginData.m_ClientID = GetCID();
	str_copy(m_LoginData.m_aUsername, pUsername, sizeof(m_LoginData.m_aUsername));
	str_copy(m_LoginData.m_aPassword, pPassword, sizeof(m_LoginData.m_aPassword));
	thread_init(*ThreadLoginWorker, &m_LoginData, "sql login"); //setzte die werte von pTmpPlayer
}

void CPlayer::ThreadLoginWorker(void *pArg) //is the actual thread
{
	struct CLoginData *pData = static_cast<struct CLoginData *>(pArg);
	CGameContext *pGS = static_cast<CGameContext *>(pData->m_pGameContext);
	// pGS->SendChat(-1, CGameContext::CHAT_ALL, "hello work from thread");
	char *pQueryBuf = sqlite3_mprintf("SELECT * FROM Accounts WHERE Username='%q' AND Password='%q'", pData->m_aUsername, pData->m_aPassword);
	CQueryLoginThreaded *pQuery = new CQueryLoginThreaded();
	pQuery->m_ClientID = pData->m_ClientID;
	pQuery->m_pGameServer = pGS;
	pQuery->Query(pGS->m_Database, pQueryBuf);
	sqlite3_free(pQueryBuf);
	pGS->SendChat(-1, CGameContext::CHAT_ALL, "hello work from thread");
}

void CPlayer::ThreadLoginDone() //get called every tick
{
	if(m_LoginData.m_LoginState != LOGIN_DONE)
		return;

	//basic
	str_copy(m_aAccountLoginName, m_LoginData.m_aUsername, sizeof(m_aAccountLoginName));
	str_copy(m_aAccountPassword, m_LoginData.m_aPassword, sizeof(m_aAccountPassword));
	str_copy(m_aAccountRegDate, m_LoginData.m_aAccountRegDate, sizeof(m_aAccountRegDate));
	SetAccID(m_LoginData.m_AccountID);

	//Accounts
	m_IsModerator = m_LoginData.m_IsModerator;
	m_IsSuperModerator = m_LoginData.m_IsSuperModerator;
	m_IsSupporter = m_LoginData.m_IsSupporter;
	m_IsAccFrozen = m_LoginData.m_IsAccFrozen;

	//city
	m_level = m_LoginData.m_level;
	m_xp = m_LoginData.m_xp;
	m_money = m_LoginData.m_money;
	m_shit = m_LoginData.m_shit;
	m_GiftDelay = m_LoginData.m_GiftDelay;

	GameServer()->SendChat(-1, CGameContext::CHAT_ALL, "[THREAD] login done");
	m_LoginData.m_LoginState = LOGIN_OFF;
}

void CPlayer::chidraqul3_GameTick()
{
	//if (m_C3_GameState == 2) //multiplayer
	//	return; //handled in gamecontext

	if(g_Config.m_SvAllowChidraqul == 0)
	{
		GameServer()->SendChatTarget(m_ClientID, "Admin has disabled chidraqul3.");
		m_C3_GameState = false;
	}
	else if(g_Config.m_SvAllowChidraqul == 1) //dynamic but resourcy way (doesnt work on linux)
	{
		char aBuf[512];

		char m_minigame_world[512];
		m_minigame_world[0] = '\0';

		//spawn gold
		if(!m_GoldAlive)
		{
			m_GoldPos = -1;
			if(m_GoldRespawnDelay <= 0)
			{
				m_GoldPos = rand() % 25 + 1;
				m_GoldAlive = true;
				m_GoldRespawnDelay = 100;
			}
			else
			{
				m_GoldRespawnDelay--;
			}
		}

		//Check for hittin stuff
		//collecting gold
		if(m_GoldPos == m_HashPos && m_HashPosY == 0)
		{
			m_HashGold++;
			m_GoldAlive = false;
		}

		//create world chararray
		//y: 3
		//y: 2
		//y: 1
		for(int i = 0; i < m_Minigameworld_size_x; i++)
		{
			char create_world[126];
			if(i == m_HashPos && m_HashPosY == 1)
			{
				str_format(create_world, sizeof(create_world), "%s", m_HashSkin);
			}
			else
			{
				str_format(create_world, sizeof(create_world), "_");
			}

			str_format(m_minigame_world, sizeof(m_minigame_world), "%s%s", m_minigame_world, create_world);
		}
		str_format(m_minigame_world, sizeof(m_minigame_world), "%s\n", m_minigame_world);
		//y: 0
		for(int i = 0; i < m_Minigameworld_size_x; i++)
		{
			char create_world[126];
			if(i == m_HashPos && m_HashPosY == 0)
			{
				str_format(create_world, sizeof(create_world), "%s", m_HashSkin);
			}
			else if(i == m_GoldPos)
			{
				str_format(create_world, sizeof(create_world), "$");
			}
			else
			{
				str_format(create_world, sizeof(create_world), "_");
			}

			str_format(m_minigame_world, sizeof(m_minigame_world), "%s%s", m_minigame_world, create_world);
		}

		//add stuff to the print string
		str_format(aBuf, sizeof(aBuf), "\n\n\n%s\nPos: [%d/%d] Gold: %d", m_minigame_world, m_HashPos, m_HashPosY, m_HashGold);

		//print all
		GameServer()->SendBroadcast(aBuf, m_ClientID);
	}
	else if(g_Config.m_SvAllowChidraqul == 2) //old hardcodet
	{
		char aBuf[512];

		if(m_HashPos == 0)
		{
			str_format(aBuf, sizeof(aBuf), "%s___________", m_HashSkin);
			GameServer()->SendBroadcast(aBuf, m_ClientID);
		}
		else if(m_HashPos == 1)
		{
			str_format(aBuf, sizeof(aBuf), "_%s__________", m_HashSkin);
			GameServer()->SendBroadcast(aBuf, m_ClientID);
		}
		else if(m_HashPos == 2)
		{
			str_format(aBuf, sizeof(aBuf), "__%s_________", m_HashSkin);
			GameServer()->SendBroadcast(aBuf, m_ClientID);
		}
		else if(m_HashPos == 3)
		{
			str_format(aBuf, sizeof(aBuf), "___%s________", m_HashSkin);
			GameServer()->SendBroadcast(aBuf, m_ClientID);
		}
		else if(m_HashPos == 4)
		{
			str_format(aBuf, sizeof(aBuf), "_____%s______", m_HashSkin);
			GameServer()->SendBroadcast(aBuf, m_ClientID);
		}
		else if(m_HashPos == 5)
		{
			str_format(aBuf, sizeof(aBuf), "______%s_____", m_HashSkin);
			GameServer()->SendBroadcast(aBuf, m_ClientID);
		}
		else if(m_HashPos == 6)
		{
			str_format(aBuf, sizeof(aBuf), "_______%s____", m_HashSkin);
			GameServer()->SendBroadcast(aBuf, m_ClientID);
		}
		else if(m_HashPos == 7)
		{
			str_format(aBuf, sizeof(aBuf), "________%s___", m_HashSkin);
			GameServer()->SendBroadcast(aBuf, m_ClientID);
		}
		else if(m_HashPos == 8)
		{
			str_format(aBuf, sizeof(aBuf), "_________%s__", m_HashSkin);
			GameServer()->SendBroadcast(aBuf, m_ClientID);
		}
		else if(m_HashPos == 9)
		{
			str_format(aBuf, sizeof(aBuf), "__________%s_", m_HashSkin);
			GameServer()->SendBroadcast(aBuf, m_ClientID);
		}
		else if(m_HashPos == 10)
		{
			str_format(aBuf, sizeof(aBuf), "___________%s", m_HashSkin);
			GameServer()->SendBroadcast(aBuf, m_ClientID);
		}
	}
	else if(g_Config.m_SvAllowChidraqul == 3) //next generation
	{
		if(m_C3_UpdateFrame)
		{
			m_C3_UpdateFrame = false;
			char aBuf[128];
			char aHUD[64];
			char aWorld[64]; //max world size

			for(int i = 0; i < g_Config.m_SvChidraqulWorldX; i++)
			{
				aWorld[i] = '_';
			}

			aWorld[m_HashPos] = m_HashSkin[0];
			aWorld[g_Config.m_SvChidraqulWorldX] = '\0';

			str_format(aHUD, sizeof(aHUD), "\n\nPos: %d", m_HashPos);
			str_format(aBuf, sizeof(aBuf), "%s%s", aWorld, aHUD);

			GameServer()->SendBroadcast(aWorld, m_ClientID, 0);
		}
		if(Server()->Tick() % 120 == 0)
		{
			m_C3_UpdateFrame = true;
		}
	}
}

bool CPlayer::JoinMultiplayer()
{
	if(GameServer()->C3_GetFreeSlots() > 0)
	{
		GameServer()->SendChatTarget(GetCID(), "[chidraqul] joined multiplayer.");
		m_C3_UpdateFrame = true;
		m_C3_GameState = 2;
		return true;
	}
	GameServer()->SendChatTarget(GetCID(), "[chidraqul] multiplayer is full.");
	return false;
}

void CPlayer::UpdateLastToucher(int ID)
{
#if defined(CONF_DEBUG)
	// dbg_msg("ddnet++", "UpdateLastToucher(%d) oldID=%d player=%d:'%s'", ID, m_LastToucherID, GetCID(), Server()->ClientName(GetCID()));
#endif
	m_LastToucherID = ID;
	m_LastTouchTicks = 0;
	if(ID == -1)
		return;
	CPlayer *pToucher = GameServer()->m_apPlayers[ID];
	if(!pToucher)
		return;
	str_copy(m_aLastToucherName, Server()->ClientName(ID), sizeof(m_aLastToucherName));
	m_LastToucherTeeInfos.m_ColorBody = pToucher->m_TeeInfos.m_ColorBody;
	m_LastToucherTeeInfos.m_ColorFeet = pToucher->m_TeeInfos.m_ColorFeet;
	str_copy(m_LastToucherTeeInfos.m_SkinName, pToucher->m_TeeInfos.m_SkinName, sizeof(pToucher->m_TeeInfos.m_SkinName));
	m_LastToucherTeeInfos.m_UseCustomColor = pToucher->m_TeeInfos.m_UseCustomColor;
}

void CPlayer::GiveBlockPoints(int Points)
{
	char aBuf[128];
	bool FlagBonus = false;

	if(GetCharacter() && ((CGameControllerDDRace *)GameServer()->m_pController)->HasFlag(GetCharacter()) != -1)
	{
		Points++;
		FlagBonus = true;
	}

	m_BlockPoints += Points;
	if(m_ShowBlockPoints)
	{
		if(IsLoggedIn())
		{
			str_format(aBuf, sizeof(aBuf), "+%d point%s%s", Points, Points == 1 ? "" : "s", FlagBonus ? " (flag bonus)" : "");
		}
		else
		{
			str_format(aBuf, sizeof(aBuf), "+%d point%s (warning! use '/login' to save your '/points')", Points, Points == 1 ? "" : "s");
		}

		GameServer()->SendChatTarget(GetCID(), aBuf);
	}
	else // chat info deactivated
	{
		if(IsLoggedIn())
		{
			// after 5 and 10 unsaved kills and no messages actiavted --> inform the player about accounts
			if(m_BlockPoints == 5 || m_BlockPoints == 10)
			{
				str_format(aBuf, sizeof(aBuf), "you made %d unsaved block points. Use '/login' to save your '/points'.", m_BlockPoints);
				GameServer()->SendChatTarget(GetCID(), aBuf);
				GameServer()->SendChatTarget(GetCID(), "Use '/accountinfo' for more information.");
			}
		}
	}
}

void CPlayer::SetAccID(int ID)
{
#if defined(CONF_DEBUG)
	// dbg_msg("account", "SetAccID(%d) oldID=%d player=%d:'%s'", ID, GetAccID(), GetCID(), Server()->ClientName(GetCID()));
#endif
	m_AccountID = ID;
}

void CPlayer::GiveXP(int value)
{
	if(IsMaxLevel())
		return;

	m_xp += value;
}

void CPlayer::SetXP(int xp)
{
#if defined(CONF_DEBUG)
	// dbg_msg("account", "SetXP(%d) oldID=%d player=%d:'%s'", xp, GetXP(), GetCID(), Server()->ClientName(GetCID()));
#endif
	m_xp = xp;
}

void CPlayer::SetLevel(int level)
{
#if defined(CONF_DEBUG)
	// dbg_msg("account", "SetLevel(%d) oldID=%d player=%d:'%s'", level, GetLevel(), GetCID(), Server()->ClientName(GetCID()));
#endif
	m_level = level;
}

void CPlayer::SetMoney(int money)
{
#if defined(CONF_DEBUG)
	// dbg_msg("account", "SetMoney(%d) oldID=%d player=%d:'%s'", money, GetMoney(), GetCID(), Server()->ClientName(GetCID()));
#endif
	m_money = money;
}
