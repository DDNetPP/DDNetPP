/* CPlayer related ddnet++ methods */

#include <engine/shared/config.h>
#include "gamemodes/DDRace.h"

#include "player.h"

void CPlayer::ResetDDPP()
{
	/*****************************
	*        DDNetPP             *
	******************************/

	//ChillerDragon constructor Konstructor init
	if (g_Config.m_SvTestingCommands)
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

	if (GameServer()->IsDDPPgametype("fly"))
	{
		m_IsVanillaDmg = true;
		m_IsVanillaWeapons = true;
	}

	m_MoneyTilesMoney = 0;
	str_copy(m_aTradeOffer, "", sizeof(m_aTradeOffer));
	str_copy(m_aEscapeReason, "unknown", 16);
	m_dmm25 = -1; //set to offline default
	m_MapSaveLoaded = false;

	if (g_Config.m_SvNoboSpawnTime)
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
	if (m_C3_GameState == 1) //singleplayer
	{
		chidraqul3_GameTick();
	}

	//profile views
	if (Server()->Tick() % 1000 == 0)
	{
		m_IsProfileViewLoaded = true;
		//GameServer()->SendChatTarget(m_ClientID, "View loaded");
	}


	//bomb
	if (m_BombBanTime)
	{
		m_BombBanTime--;
		if (m_BombBanTime == 1)
		{
			GameServer()->SendChatTarget(m_ClientID, "Bomb bantime expired.");
		}
	}

	if (Server()->Tick() % (Server()->TickSpeed() * 300) == 0)
		if (IsLoggedIn())
			Save(1); //SetLoggedIn true

	//dragon test chillers level system xp money usw am start :3
	CheckLevel();

	ThreadLoginDone();

	if (m_ChangeTeamOnFlag || (Server()->Tick() % 600 == 0))
	{
		if ((((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(GetCharacter()) == -1) && m_IsDummy && ((g_Config.m_SvShowBotsInScoreboard == 1 && (m_DummyMode >= -6 && m_DummyMode <= -1)) || g_Config.m_SvShowBotsInScoreboard == 0))
		{
			m_Team = TEAM_BLUE;
		}
		else
		{
			m_Team = TEAM_RED;
		}
		m_ChangeTeamOnFlag = false;
	}

	if (m_SetRealName)
	{
		if (m_SetRealNameTick < Server()->Tick())
		{
			if (m_FixNameID == 1)
				GameServer()->SendChat(m_ClientID, m_ChatTeam, m_ChatText, m_ClientID);
			else if (m_FixNameID == 2)
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
	if (m_HumanLevelTime >= 1)
	{
		m_HumanLevelTime--;
	}

	if (m_PlayerHumanLevel == 0)
	{
		if (GetCharacter() && GetCharacter()->InputActive())
		{
			m_PlayerHumanLevel++;
			m_HumanLevelTime = Server()->TickSpeed() * 10; // 10 sec
		}
	}
	else if (m_PlayerHumanLevel == 1)
	{
		if (m_HumanLevelTime <= 0)
		{
			m_PlayerHumanLevel++;
			m_PlayerHumanLevelState = 0;
		}
	}
	else if (m_PlayerHumanLevel == 2)
	{
		if (Server()->Tick() % 40 == 0)
		{
			if (GetCharacter() && GetCharacter()->InputActive())
			{
				m_PlayerHumanLevelState++;
			}
		}
		if (m_PlayerHumanLevelState > 3)
		{
			m_PlayerHumanLevel++;
			m_HumanLevelTime = Server()->TickSpeed() * 10; // 10 sec
		}
	}
	else if (m_PlayerHumanLevel == 3)
	{
		if (m_HumanLevelTime <= 0)
		{
			m_PlayerHumanLevel++;
			m_PlayerHumanLevelState = 0;
		}
	}
	else if (m_PlayerHumanLevel == 4)
	{
		if (GetCharacter())
		{
			if (GetCharacter()->m_DDRaceState == DDRACE_FINISHED ||
				m_BlockPoints > 5 ||
				IsLoggedIn())
			{
				m_PlayerHumanLevel++;
				m_HumanLevelTime = Server()->TickSpeed() * 20; // 20 sec
			}
		}
	}
	else if (m_PlayerHumanLevel == 5)
	{
		if (m_HumanLevelTime <= 0)
		{
			m_PlayerHumanLevel++;
			m_PlayerHumanLevelState = 0;
		}
	}
	else if (m_PlayerHumanLevel == 6)
	{
		if (m_pCaptcha->IsHuman())
		{
			m_PlayerHumanLevel++;
		}
	}
	else if (m_PlayerHumanLevel == 7)
	{
		if ((m_QuestLevelUnlocked > 0 || m_QuestUnlocked > 2) || // played quest until finish map
			m_BlockPoints > 10)
		{
			m_PlayerHumanLevel++;
			m_HumanLevelTime = Server()->TickSpeed() * 60; // 1 min
		}
	}
	else if (m_PlayerHumanLevel == 8)
	{
		if (m_HumanLevelTime <= 0)
		{
			m_PlayerHumanLevel++;
			m_PlayerHumanLevelState = 0;
		}
	}
}
