/* CPlayer related ddnet++ methods */

#include "gamemodes/DDRace.h"
#include <engine/shared/config.h>

#include <cinttypes>
#include <fstream>
#include <limits>

#include <game/server/ddpp/loc.h>
#include <game/server/ddpp/shop.h>

#include "gamecontext.h"

#include <game/mapitems.h>

#include "player.h"

void CPlayer::FixForNoName(int ID)
{
	m_FixNameID = ID; // 0 for just to display the name in the right moment (e.g. kill msg killer)
	m_SetRealName = true;
	m_SetRealNameTick = Server()->Tick() + Server()->TickSpeed() / 20;
}

void CPlayer::ConstructDDPP()
{
	m_pCaptcha = new CCaptcha(GameServer(), GetCID());
	m_pDummyMode = nullptr;
	newNickname[0] = '\0';
}

void CPlayer::DestructDDPP()
{
	if(m_pDummyMode)
	{
		delete m_pDummyMode;
		m_pDummyMode = nullptr;
	}
	if(m_pCaptcha)
	{
		delete m_pCaptcha;
		m_pCaptcha = nullptr;
	}
}

void CPlayer::ResetDDPP()
{
	/*****************************
	*        DDNetPP             *
	******************************/

	//ChillerDragon constructor Konstructor init
	if(g_Config.m_SvTestingCommands)
	{
		str_format(m_Account.m_aAsciiFrame[0], sizeof(m_Account.m_aAsciiFrame[0]), "x");
		str_format(m_Account.m_aAsciiFrame[1], sizeof(m_Account.m_aAsciiFrame[1]), "+");
		str_format(m_Account.m_aAsciiFrame[2], sizeof(m_Account.m_aAsciiFrame[2]), "++");
		str_format(m_Account.m_aAsciiFrame[3], sizeof(m_Account.m_aAsciiFrame[3]), "xxx");
		str_format(m_Account.m_aAsciiFrame[4], sizeof(m_Account.m_aAsciiFrame[4]), "++++");
		str_format(m_Account.m_aAsciiFrame[5], sizeof(m_Account.m_aAsciiFrame[5]), "xxxxx");
		str_format(m_Account.m_aAsciiFrame[6], sizeof(m_Account.m_aAsciiFrame[6]), "++++++");
		str_format(m_Account.m_aAsciiFrame[7], sizeof(m_Account.m_aAsciiFrame[7]), "xxxxxxx");
		str_format(m_Account.m_aAsciiFrame[8], sizeof(m_Account.m_aAsciiFrame[8]), "++++++++");
		str_format(m_Account.m_aAsciiFrame[9], sizeof(m_Account.m_aAsciiFrame[9]), "ChillerDragon's sample animation");
		str_format(m_Account.m_aAsciiFrame[10], sizeof(m_Account.m_aAsciiFrame[10]), "ChillerDragon's sample animation");
		str_format(m_Account.m_aAsciiFrame[11], sizeof(m_Account.m_aAsciiFrame[11]), "ChillerDragon's sample animation");
		str_format(m_Account.m_aAsciiFrame[12], sizeof(m_Account.m_aAsciiFrame[12]), "ChillerDragon's sample animation");
		str_format(m_Account.m_aAsciiFrame[13], sizeof(m_Account.m_aAsciiFrame[13]), ".");
		str_format(m_Account.m_aAsciiFrame[14], sizeof(m_Account.m_aAsciiFrame[14]), ":");
		str_format(m_Account.m_aAsciiFrame[15], sizeof(m_Account.m_aAsciiFrame[15]), ".:.");
	}
	for(auto &CachedID : m_aCatchedID)
		CachedID = -1;

	if(GameServer()->IsDDPPgametype("fly"))
	{
		m_IsVanillaDmg = true;
		m_IsVanillaWeapons = true;
	}
	m_vWeaponLimit.resize(6);

	m_MoneyTilesMoney = 0;
	str_copy(m_aTradeOffer, "", sizeof(m_aTradeOffer));
	str_copy(m_aEscapeReason, "unknown", 16);
	m_dmm25 = -1; //set to offline default
	m_MapSaveLoaded = false;

	if(g_Config.m_SvNoboSpawnTime)
	{
		m_IsNoboSpawn = true;
	}
	SetAccID(0);
	m_PlayerHumanLevel = 0;
	m_HumanLevelTime = 0;
	m_NoboSpawnStop = Server()->Tick() + Server()->TickSpeed() * (60 * g_Config.m_SvNoboSpawnTime);
	m_QuestPlayerID = -1;
	m_JailHammer = true;
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

	//dbg_msg("debug", "init player showhide='%s'", m_Account.m_aShowHideConfig);
	m_ShowBlockPoints = GameServer()->CharToBool(m_Account.m_aShowHideConfig[0]); //doing it manually becuase the gamecontext function cant be called here
	m_HideBlockXp = GameServer()->CharToBool(m_Account.m_aShowHideConfig[1]);
	m_xpmsg = GameServer()->CharToBool(m_Account.m_aShowHideConfig[2]);
	m_hidejailmsg = GameServer()->CharToBool(m_Account.m_aShowHideConfig[3]);
	m_HideInsta1on1_killmessages = GameServer()->CharToBool(m_Account.m_aShowHideConfig[4]);
	m_HideQuestProgress = GameServer()->CharToBool(m_Account.m_aShowHideConfig[5]);
	m_HideQuestWarning = GameServer()->CharToBool(m_Account.m_aShowHideConfig[6]);
	//GameServer()->ShowHideConfigCharToBool(this->GetCID()); //cant be called because somehow players doesnt exist for gameconext idk
	//str_format(m_Account.m_aShowHideConfig, sizeof(m_Account.m_aShowHideConfig), "%s", "0010000000000000"); // <3
	//m_xpmsg = true;

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
	m_Language = LANG_RU;
}

void CPlayer::SetLanguage(const char *pLang)
{
	if(!str_comp(pLang, "en") || !str_comp(pLang, "english") || !str_comp(pLang, "us"))
		SetLanguage(LANG_EN);
	else if(!str_comp(pLang, "ru") || !str_comp(pLang, "rus") || !str_comp(pLang, "russan"))
		SetLanguage(LANG_RU);
	else
	{
		GameServer()->SendChatTarget(m_ClientID, "[lang] invalid language pick one of those: en, ru");
		return;
	}
	GameServer()->SendChatTarget(m_ClientID, "[lang] language set");
}

void CPlayer::DDPPTick()
{
	if(g_Config.m_SvOffDDPP)
		return;

	if(m_AccountQueryResult != nullptr && m_AccountQueryResult->m_Completed)
	{
		DDPPProcessScoreResult(*m_AccountQueryResult);
		m_AccountQueryResult = nullptr;
	}
	if(m_AdminCommandQueryResult != nullptr && m_AdminCommandQueryResult->m_Completed)
	{
		DDPPProcessAdminCommandResult(*m_AdminCommandQueryResult);
		m_AdminCommandQueryResult = nullptr;
	}
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
	if(m_Account.m_BombBanTime)
	{
		m_Account.m_BombBanTime--;
		if(m_Account.m_BombBanTime == 1)
		{
			GameServer()->SendChatTarget(m_ClientID, "Bomb bantime expired.");
		}
	}

	if(Server()->Tick() % (Server()->TickSpeed() * 300) == 0)
		if(IsLoggedIn())
			Save(1); //SetLoggedIn true

	//dragon test chillers level system xp money usw am start :3
	CheckLevel();

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

bool CPlayer::SetDummyMode(EDummyMode Mode)
{
	if(m_DummyMode == Mode)
	{
		dbg_msg("dummy", "mode already set to %d", Mode);
		return true;
	}

	m_DummyMode = Mode;
	m_aDummyMode[0] = '\0';

	if(m_pDummyMode)
	{
		delete m_pDummyMode;
		m_pDummyMode = nullptr;
	}

	switch(m_DummyMode)
	{
	case DUMMYMODE_BALANCE1: m_pDummyMode = new CDummyChillBlock5Balance(this); break;
	case DUMMYMODE_BALANCE2: m_pDummyMode = new CDummyChillBlock5Balance(this); break;
	case DUMMYMODE_BLOCKWAVE: m_pDummyMode = new CDummyBlockWave(this); break;
	case DUMMYMODE_RIFLE_FNG: m_pDummyMode = new CDummyRifleFng(this); break;
	case DUMMYMODE_GRENADE_FNG: m_pDummyMode = new CDummyGrenadeFng(this); break;
	case DUMMYMODE_BLMAPV3_ARENA: m_pDummyMode = new CDummyBlmapV3Arena(this); break;
	case DUMMYMODE_ADVENTURE: m_pDummyMode = new CDummyAdventure(this); break;
	case DUMMYMODE_DEFAULT: m_pDummyMode = new CDummySample(this); break;
	case DUMMYMODE_CHILLBLOCK5_BLOCKER_TRYHARD: m_pDummyMode = new CDummyChillBlock5BlockerTryHard(this); break;
	case DUMMYMODE_CHILLBLOCK5_RACER: m_pDummyMode = new CDummyChillBlock5Race(this); break;
	case DUMMYMODE_FNN: m_pDummyMode = new CDummyFNN(this); break;
	case DUMMYMODE_CHILLBLOCK5_RACE: m_pDummyMode = new CDummyBlmapChillPolice(this); break;
	case DUMMYMODE_CHILLBLOCK5_BLOCKER: m_pDummyMode = new CDummyChillBlock5Blocker(this); break;
	case DUMMYMODE_CHILLBLOCK5_BALANCE: m_pDummyMode = new CDummyChillBlock5Balance(this); break;
	case DUMMYMODE_CHILLBLOCK5_POLICE: m_pDummyMode = new CDummyChillBlock5Police(this); break;
	case DUMMYMODE_BLMAPCHILL_POLICE: m_pDummyMode = new CDummyBlmapChillPolice(this); break;
	case DUMMYMODE_CHILLINTELLIGENCE: m_pDummyMode = new CDummySample(this); break; // TODO:
	case DUMMYMODE_SURVIVAL: m_pDummyMode = new CDummySurvival(this); break;
	case DUMMYMODE_QUEST: m_pDummyMode = new CDummyQuest(this); break;
	case DUMMYMODE_SHOPBOT: m_pDummyMode = new CDummyShopBot(this); break;
	case DUMMYMODE_CTF5_PVP: m_pDummyMode = new CDummyCtf5Pvp(this); break;
	case DUMMYMODE_BLMAPV5_LOWER_BLOCKER: m_pDummyMode = new CDummyBlmapV5LowerBlocker(this); break;
	case DUMMYMODE_BLMAPV5_UPPER_BLOCKER: m_pDummyMode = new CDummyBlmapV5UpperBlocker(this); break;
	default:
		dbg_msg("dummy", "invalid dummy mode %d", Mode);
		return false;
		break;
	}

	str_copy(m_aDummyMode, m_pDummyMode->ModeStr(), sizeof(m_aDummyMode));
	dbg_msg("dummy", "set mode %d (%s) for player %d:'%s'", Mode, m_pDummyMode->ModeStr(), GetCID(), Server()->ClientName(GetCID()));
	return true;
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
				m_Account.m_BlockPoints > 5 ||
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
			m_Account.m_BlockPoints > 10)
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
	/* spooky ghost
	const char *pClan;
	if(m_SpookyGhostActive)
		pClan = m_RealName;
	else
		pClan = m_RealClan;
	StrToInts(&pClientInfo->m_Clan0, 3, pClan); */

	if(m_SpookyGhostActive)
	{
		m_ShowName = false;
	}

	if((m_SetRealName || m_ShowName) && !m_IsBlockTourningInArena)
	{
		if(newNickname[0] != '\0')
		{
			StrToInts(&pClientInfo->m_Name0, 4, newNickname);
		}
		else
		{
			StrToInts(&pClientInfo->m_Name0, 4, Server()->ClientName(m_ClientID));
		}
	}
	else
	{
		StrToInts(&pClientInfo->m_Name0, 4, " ");
	}

	if(m_IsBlockTourningInArena)
	{
		StrToInts(&pClientInfo->m_Skin0, 6, "default");
		pClientInfo->m_UseCustomColor = false;
	}

	if(GetCharacter())
	{
		if(m_PlayerFlags & PLAYERFLAG_SCOREBOARD)
			GameServer()->Shop()->OnOpenScoreboard(GetCID());
		else
			GetCharacter()->m_TimesShot = 0;
	}

	if(GetCharacter() && GetCharacter()->m_IsBomb) //bomb (keep bomb 1st. Because bomb over all rainbow and other stuff shoudl be ignored if bomb)
	{
		StrToInts(&pClientInfo->m_Skin0, 6, m_TeeInfos.m_aSkinName);
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
		StrToInts(&pClientInfo->m_Skin0, 6, m_TeeInfos.m_aSkinName);
		pClientInfo->m_UseCustomColor = true;
		m_RainbowColor = (m_RainbowColor + 1) % 256;
		pClientInfo->m_ColorBody = m_RainbowColor * 0x010000 + 0xff00;
		pClientInfo->m_ColorFeet = m_RainbowColor * 0x010000 + 0xff00;
	}
	else
		return m_IsBlockTourningInArena;
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
				pPlayerInfo->m_Score = m_Account.m_SurvivalKills * 60;
			else
				pPlayerInfo->m_Score = m_Account.m_SurvivalKills;
		}
		else
			pPlayerInfo->m_Score = -9999;
	}
	else if(g_Config.m_SvInstagibMode || !g_Config.m_SvDDPPscore)
	{
		pPlayerInfo->m_Score = m_Score;
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
					pPlayerInfo->m_Score = m_Account.m_BlockPoints * 60;
				else
					pPlayerInfo->m_Score = m_Account.m_BlockPoints;
			}
			else if(pSnapping->m_ScoreFixForDDNet)
				pPlayerInfo->m_Score = -9999;
			else
				pPlayerInfo->m_Score = 0;
		}
	}
}

void CPlayer::DDPPSnapChangePlayerInfo7(int SnappingClient, CPlayer *pSnapping, protocol7::CNetObj_PlayerInfo *pPlayerInfo)
{
	DDPPSnapChangePlayerInfo(SnappingClient, pSnapping, (CNetObj_PlayerInfo *)pPlayerInfo);
}

void CPlayer::OnDisconnectDDPP()
{
	if(m_Insta1on1_id != -1 && (m_IsInstaArena_gdm || m_IsInstaArena_idm))
	{
		GameServer()->WinInsta1on1(m_Insta1on1_id, GetCID());
	}
	if(m_Account.m_JailTime)
	{
		GameServer()->SetIpJailed(GetCID());
	}
	Logout();
}

void CPlayer::Logout(int SetLoggedIn)
{
	if(!IsLoggedIn())
		return;

	Save(SetLoggedIn);
	dbg_msg("account", "logging out AccountID=%d SetLoggedIn=%d", GetAccID(), SetLoggedIn);

	// Keep jail and escape time on logout
	int64_t JailTime = m_Account.m_JailTime;
	int64_t EscapeTime = m_Account.m_EscapeTime;

	m_Account = CAccountData();

	m_Account.m_JailTime = JailTime;
	m_Account.m_EscapeTime = EscapeTime;

	//reset values to default to prevent cheating
	m_neededxp = 0;
	m_TaserOn = false;
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

	m_Account.m_JailTime = Server()->TickSpeed() * seconds;

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

float CPlayer::TaserFreezeTime()
{
	if(!IsLoggedIn())
		return 0;
	if(!m_Account.m_TaserLevel)
		return 0;

	int64_t LastUse = 0;
	CCharacter *pChr = GetCharacter();
	if(pChr)
		LastUse = pChr->m_LastTaserUse;

	int Strength = clamp((int)((Server()->Tick() - LastUse) * 2 / Server()->TickSpeed()),
		0,
		m_Account.m_TaserLevel);

	dbg_msg("taser", "stregth=%d taserlvel=%d lastuse=%" PRId64 " time=%.2f", Strength, m_Account.m_TaserLevel, LastUse, (float)(Strength / 10.0f));
	float Time = (float)(Strength / 10.0f);

	dbg_msg("taser", "return time=%.2f", Time);

	return Time;
}

void CPlayer::Save(int SetLoggedIn)
{
#if defined(CONF_DEBUG)
	dbg_msg("account", "saving account '%s' CID=%d AccountID=%d SetLoggedIn=%d", Server()->ClientName(GetCID()), GetCID(), GetAccID(), SetLoggedIn);
#endif
	if(!IsLoggedIn())
		return;

	m_Account.m_IsLoggedIn = SetLoggedIn;

	// Proccess Clan Data...
	char aClan[32];
	str_copy(aClan, Server()->ClientClan(m_ClientID), sizeof(aClan));

	if(str_comp(aClan, m_Account.m_aClan1) && str_comp(aClan, m_Account.m_aClan2) && str_comp(aClan, m_Account.m_aClan3))
	{
		//dbg_msg("save", "update clan '%s'", aClan);
		str_format(m_Account.m_aClan3, sizeof(m_Account.m_aClan3), "%s", m_Account.m_aClan2);
		str_format(m_Account.m_aClan2, sizeof(m_Account.m_aClan2), "%s", m_Account.m_aClan1);
		str_format(m_Account.m_aClan1, sizeof(m_Account.m_aClan1), "%s", aClan);
	}

	// Proccess IP ADDR...
	char aIP[32];
	Server()->GetClientAddr(GetCID(), aIP, sizeof(aIP));

	if(str_comp(aIP, m_Account.m_aIP_1) && str_comp(aIP, m_Account.m_aIP_2) && str_comp(aIP, m_Account.m_aIP_3))
	{
		//dbg_msg("save", "updated ip '%s'", aIP);
		str_format(m_Account.m_aIP_3, sizeof(m_Account.m_aIP_3), "%s", m_Account.m_aIP_2);
		str_format(m_Account.m_aIP_2, sizeof(m_Account.m_aIP_2), "%s", m_Account.m_aIP_1);
		str_format(m_Account.m_aIP_1, sizeof(m_Account.m_aIP_1), "%s", aIP);
	}

	// Proccess IngameName Data...
	char aName[32];
	str_copy(aName, Server()->ClientName(m_ClientID), sizeof(aName));

	// new name --> add it in history and overwrite the oldest
	if(str_comp(aName, m_Account.m_LastLogoutIGN1) && str_comp(aName, m_Account.m_LastLogoutIGN2) && str_comp(aName, m_Account.m_LastLogoutIGN3) && str_comp(aName, m_Account.m_LastLogoutIGN4) && str_comp(aName, m_Account.m_LastLogoutIGN5))
	{
		//dbg_msg("debug", "'%s' was not equal to...", aName);
		//dbg_msg("debug", "'%s'", m_Account.m_LastLogoutIGN1);
		//dbg_msg("debug", "'%s'", m_Account.m_LastLogoutIGN2);
		//dbg_msg("debug", "'%s'", m_Account.m_LastLogoutIGN3);
		//dbg_msg("debug", "'%s'", m_Account.m_LastLogoutIGN4);
		//dbg_msg("debug", "'%s'", m_Account.m_LastLogoutIGN5);

		str_format(m_Account.m_LastLogoutIGN5, sizeof(m_Account.m_LastLogoutIGN5), "%s", m_Account.m_LastLogoutIGN4);
		str_format(m_Account.m_LastLogoutIGN4, sizeof(m_Account.m_LastLogoutIGN4), "%s", m_Account.m_LastLogoutIGN3);
		str_format(m_Account.m_LastLogoutIGN3, sizeof(m_Account.m_LastLogoutIGN3), "%s", m_Account.m_LastLogoutIGN2);
		str_format(m_Account.m_LastLogoutIGN2, sizeof(m_Account.m_LastLogoutIGN2), "%s", m_Account.m_LastLogoutIGN1);
		str_format(m_Account.m_LastLogoutIGN1, sizeof(m_Account.m_LastLogoutIGN1), "%s", aName);
	}

	// read showhide bools to char array that is being saved
	// GameServer()->ShowHideConfigBoolToChar(this->GetCID());

	if(m_IsFileAcc)
		SaveFileBased();
	else
		GameServer()->Accounts()->Save(GetCID(), &m_Account);
}

void CPlayer::SaveFileBased()
{
	std::string data;
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "%s/%s.acc", g_Config.m_SvFileAccPath, m_Account.m_aUsername);
	std::ofstream Acc2File(aBuf);

	if(Acc2File.is_open())
	{
		dbg_msg("acc2", "saved acc '%s'", m_Account.m_aUsername);

		Acc2File << m_Account.m_aPassword << "\n";
		Acc2File << m_Account.m_IsLoggedIn << "\n";
		Acc2File << g_Config.m_SvPort << "\n";
		Acc2File << m_Account.m_IsAccFrozen << "\n";
		Acc2File << m_Account.m_IsModerator << "\n";
		Acc2File << m_Account.m_IsSuperModerator << "\n";
		Acc2File << m_Account.m_IsSupporter << "\n";
		Acc2File << GetMoney() << "\n";
		Acc2File << GetLevel() << "\n";
		Acc2File << GetXP() << "\n";
		Acc2File << m_Account.m_Shit << "\n";
		Acc2File << m_Account.m_PoliceRank << "\n";
		Acc2File << m_Account.m_TaserLevel << "\n";

		Acc2File.close();
	}
	else
	{
		dbg_msg("acc2", "[WARNING] account '%s' (%s) failed to save", m_Account.m_aUsername, aBuf);
		Acc2File.close();
	}
}

void CPlayer::CalcExp()
{
	int64_t OldNeededXp = m_neededxp;
	dbg_msg("account", "CalcExp() neededxp=%" PRId64 " xp=%" PRId64 "", OldNeededXp, GetXP());

	//										xp diff
	if(GetLevel() == 0)
		m_neededxp = 5000;
	else if(GetLevel() == 1) //5 000
		m_neededxp = 15000;
	else if(GetLevel() == 2) //10 000
		m_neededxp = 25000;
	else if(GetLevel() == 3) //10 000
		m_neededxp = 35000;
	else if(GetLevel() == 4) //10 000
		m_neededxp = 50000;
	else if(GetLevel() == 5) //15 000			Rainbow
		m_neededxp = 65000;
	else if(GetLevel() == 6) //15 000
		m_neededxp = 80000;
	else if(GetLevel() == 7) //15 000
		m_neededxp = 100000;
	else if(GetLevel() == 8) //20 000
		m_neededxp = 120000;
	else if(GetLevel() == 9) //20 000
		m_neededxp = 130000;
	else if(GetLevel() == 10) //30 000
		m_neededxp = 160000;
	else if(GetLevel() == 11) //30 000
		m_neededxp = 200000;
	else if(GetLevel() == 12) //40 000
		m_neededxp = 240000;
	else if(GetLevel() == 13) //40 000
		m_neededxp = 280000;
	else if(GetLevel() == 14) //40 000
		m_neededxp = 325000;
	else if(GetLevel() == 15) //45 000			Bloody
		m_neededxp = 370000;
	else if(GetLevel() == 16) //50 000			room_key
		m_neededxp = 420000;
	else if(GetLevel() == 17) //50 000
		m_neededxp = 470000;
	else if(GetLevel() == 18) //50 000			Police[1]
		m_neededxp = 520000;
	else if(GetLevel() == 19) //50 000
		m_neededxp = 600000;
	else if(GetLevel() == 20) //80 000
		m_neededxp = 680000;
	else if(GetLevel() == 21) //80 000			Ninja jetpack
		m_neededxp = 760000;
	else if(GetLevel() == 22) //90 000
		m_neededxp = 850000;
	else if(GetLevel() == 23) //100 000
		m_neededxp = 950000;
	else if(GetLevel() == 24) //150 000
		m_neededxp = 1200000;
	else if(GetLevel() == 25) //200 000			Police[2]		policehelper && jail codes
		m_neededxp = 1400000;
	else if(GetLevel() == 26) //200 000
		m_neededxp = 1600000;
	else if(GetLevel() == 27) //200 000
		m_neededxp = 1800000;
	else if(GetLevel() == 28) //200 000
		m_neededxp = 2000000;
	else if(GetLevel() == 29) //210 000
		m_neededxp = 2210000;
	else if(GetLevel() == 30) //220 000			Police[3]		taser
		m_neededxp = 2430000;
	else if(GetLevel() == 31) //230 000
		m_neededxp = 2660000;
	else if(GetLevel() == 32) //240 000
		m_neededxp = 2900000;
	else if(GetLevel() == 33) //250 000
		m_neededxp = 3150000;
	else if(GetLevel() == 34) //350 000
		m_neededxp = 3500000;
	else if(GetLevel() == 35) //450 000
		m_neededxp = 3950000;
	else if(GetLevel() == 36) //550 000
		m_neededxp = 4500000;
	else if(GetLevel() == 37) //750 000
		m_neededxp = 5250000;
	else if(GetLevel() == 38) //850 000			spawn weapons
		m_neededxp = 6100000;
	else if(GetLevel() == 39) //900 000
		m_neededxp = 7000000;
	else if(GetLevel() == 40) //1 000 000			Police[4]		homing missels
		m_neededxp = 8000000;
	else if(GetLevel() == 41) //1 000 000
		m_neededxp = 9000000;
	else if(GetLevel() == 42) //1 000 000
		m_neededxp = 10000000;
	else if(GetLevel() == 43) //1 000 000
		m_neededxp = 11000000;
	else if(GetLevel() == 44) //1 000 000
		m_neededxp = 12000000;
	else if(GetLevel() == 45) //1 000 000
		m_neededxp = 13000000;
	else if(GetLevel() == 46) //1 000 000
		m_neededxp = 14000000;
	else if(GetLevel() == 47) //1 000 000
		m_neededxp = 15000000;
	else if(GetLevel() == 48) //1 000 000
		m_neededxp = 16000000;
	else if(GetLevel() == 49) //1 000 000
		m_neededxp = 17000000;
	else if(GetLevel() == 50) //1 000 000			Police[5]		'/jail arrest <time>' hammer command
		m_neededxp = 18000000;
	else if(GetLevel() == 51) //1 000 000
		m_neededxp = 19000000;
	else if(GetLevel() == 52) //1 000 000
		m_neededxp = 20000000;
	else if(GetLevel() == 53) //1 000 000
		m_neededxp = 21000000;
	else if(GetLevel() == 54) //1 000 000
		m_neededxp = 22000000;
	else if(GetLevel() == 55) //1 000 000
		m_neededxp = 23000000;
	else if(GetLevel() == 56) //1 000 000
		m_neededxp = 24000000;
	else if(GetLevel() == 57) //1 000 000
		m_neededxp = 25000000;
	else if(GetLevel() == 58) //1 000 000
		m_neededxp = 26000000;
	else if(GetLevel() == 59) //1 000 000
		m_neededxp = 27000000;
	else if(GetLevel() == 60) //1 000 000
		m_neededxp = 28000000;
	else if(GetLevel() == 61) //1 000 000
		m_neededxp = 29000000;
	else if(GetLevel() == 62) //1 000 000
		m_neededxp = 30000000;
	else if(GetLevel() == 63) //1 000 000
		m_neededxp = 31000000;
	else if(GetLevel() == 64) //1 000 000
		m_neededxp = 32000000;
	else if(GetLevel() == 65) //1 000 000
		m_neededxp = 33000000;
	else if(GetLevel() == 66) //1 000 000
		m_neededxp = 34000000;
	else if(GetLevel() == 67) //1 000 000
		m_neededxp = 35000000;
	else if(GetLevel() == 68) //1 000 000
		m_neededxp = 36000000;
	else if(GetLevel() == 69) //1 000 000
		m_neededxp = 37000000;
	else if(GetLevel() == 70) //1 000 000
		m_neededxp = 38000000;
	else if(GetLevel() == 71) //1 000 000
		m_neededxp = 39000000;
	else if(GetLevel() == 72) //1 000 000
		m_neededxp = 40000000;
	else if(GetLevel() == 73) //1 010 000
		m_neededxp = 41010000;
	else if(GetLevel() == 74) //1 010 000
		m_neededxp = 42020000;
	else if(GetLevel() == 75) //1 010 000
		m_neededxp = 43030000;
	else if(GetLevel() == 76) //1 010 000
		m_neededxp = 44040000;
	else if(GetLevel() == 77) //1 010 000
		m_neededxp = 45050000;
	else if(GetLevel() == 78) //1 010 000
		m_neededxp = 46060000;
	else if(GetLevel() == 79) //1 010 000
		m_neededxp = 47070000;
	else if(GetLevel() == 80) //1 010 000
		m_neededxp = 48080000;
	else if(GetLevel() == 81) //1 010 000
		m_neededxp = 49090000;
	else if(GetLevel() == 82) //1 010 000
		m_neededxp = 50100000;
	else if(GetLevel() == 83) //1 010 000
		m_neededxp = 51110000;
	else if(GetLevel() == 84) //1 010 000
		m_neededxp = 52120000;
	else if(GetLevel() == 85) //1 010 000
		m_neededxp = 53130000;
	else if(GetLevel() == 86) //1 010 000
		m_neededxp = 54140000;
	else if(GetLevel() == 87) //1 010 000
		m_neededxp = 55150000;
	else if(GetLevel() == 88) //1 010 000
		m_neededxp = 56160000;
	else if(GetLevel() == 89) //1 010 000
		m_neededxp = 57170000;
	else if(GetLevel() == 90) //1 010 000
		m_neededxp = 58180000;
	else if(GetLevel() == 91) //1 010 000
		m_neededxp = 59190000;
	else if(GetLevel() == 92) //1 010 000
		m_neededxp = 60200000;
	else if(GetLevel() == 93) //1 100 000
		m_neededxp = 61300000;
	else if(GetLevel() == 94) //1 100 000
		m_neededxp = 62400000;
	else if(GetLevel() == 95) //1 100 000
		m_neededxp = 63500000;
	else if(GetLevel() == 96) //1 100 000
		m_neededxp = 64600000;
	else if(GetLevel() == 97) //1 100 000
		m_neededxp = 65700000;
	else if(GetLevel() == 98) //1 100 000
		m_neededxp = 66800000;
	else if(GetLevel() == 99) //1 100 000
		m_neededxp = 67900000;
	else if(GetLevel() == 100) //12 100 000
		m_neededxp = 80000000;
	else if(GetLevel() == 101) //20 000 000
		m_neededxp = 100000000;
	else if(GetLevel() == 102) //20 000 000
		m_neededxp = 120000000;
	else if(GetLevel() == 103) //20 000 000
		m_neededxp = 140000000;
	else if(GetLevel() == 104) //20 000 000
		m_neededxp = 160000000;
	else if(GetLevel() == 105) //20 000 000
		m_neededxp = 180000000;
	else if(GetLevel() == 106) //20 000 000
		m_neededxp = 200000000;
	else if(GetLevel() == 107) //20 000 000
		m_neededxp = 220000000;
	else if(GetLevel() == 108) //20 000 000
		m_neededxp = 240000000;
	else if(GetLevel() == 109) //20 000 000
		m_neededxp = 260000000;
	else
		m_neededxp = 404000000000000; //404 error

	// make sure to update ACC_MAX_LEVEL when adding more level (neededxp has only to be defined until max level - 1)

	if(IsMaxLevel())
	{
		GameServer()->SendChatTarget(m_ClientID, "[ACCOUNT] GRATULATIONS !!! you reached the maximum level.");
		SetXP(OldNeededXp);
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

	if(GetXP() >= m_neededxp)
	{
		SetLevel(GetLevel() + 1);

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "You are now Level %d!   +50money", GetLevel());
		GameServer()->SendChatTarget(m_ClientID, aBuf); //woher weiss ich dass? mit dem GameServer()-> und m_Cli...
		MoneyTransaction(+50, "level up");

		CalcExp();
	}
}

void CPlayer::MoneyTransaction(int Amount, const char *Description)
{
	SetMoney(GetMoney() + Amount);
#if defined(CONF_DEBUG)
	if(GetMoney() < 0)
	{
		dbg_msg("MoneyTransaction", "WARNING money went negative! id=%d name=%s value=%" PRId64 "", GetCID(), Server()->ClientName(GetCID()), GetMoney());
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
	return m_IsInstaArena_gdm || m_IsInstaArena_idm || m_IsInstaArena_fng;
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
	str_copy(m_LastToucherTeeInfos.m_aSkinName, pToucher->m_TeeInfos.m_aSkinName, sizeof(pToucher->m_TeeInfos.m_aSkinName));
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

	m_Account.m_BlockPoints += Points;
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
		if(!IsLoggedIn())
		{
			// after 5 and 10 unsaved kills and no messages actiavted --> inform the player about accounts
			if(m_Account.m_BlockPoints == 5 || m_Account.m_BlockPoints == 10)
			{
				str_format(aBuf, sizeof(aBuf), "you made %d unsaved block points. Use '/login' to save your '/points'.", m_Account.m_BlockPoints);
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
	m_Account.m_ID = ID;
}

void CPlayer::GiveXP(int value)
{
	if(IsMaxLevel())
		return;

	m_Account.m_XP += value;
}

void CPlayer::SetXP(int xp)
{
#if defined(CONF_DEBUG)
	// dbg_msg("account", "SetXP(%d) oldID=%d player=%d:'%s'", xp, GetXP(), GetCID(), Server()->ClientName(GetCID()));
#endif
	m_Account.m_XP = xp;
}

void CPlayer::SetLevel(int Level)
{
#if defined(CONF_DEBUG)
	// dbg_msg("account", "SetLevel(%d) oldID=%d player=%d:'%s'", Level, GetLevel(), GetCID(), Server()->ClientName(GetCID()));
#endif
	m_Account.m_Level = Level;
}

void CPlayer::SetMoney(int Money)
{
#if defined(CONF_DEBUG)
	// dbg_msg("account", "SetMoney(%d) oldID=%d player=%d:'%s'", Money, GetMoney(), GetCID(), Server()->ClientName(GetCID()));
#endif
	m_Account.m_Money = Money;
}
