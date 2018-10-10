/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <new>
#include <cstdio>
#include <ctime>

#include <engine/server.h>
#include <engine/shared/config.h>
#include <engine/server/server.h>
#include <game/version.h>
#include <game/gamecore.h>
#include <game/server/teams.h>
#include <sqlite3/sqlite3.h>
#include "gamemodes/DDRace.h"
#include "gamecontext.h"
#include "player.h"

#include <fstream> //ChillerDragon acc sys2
#include <limits> //ChillerDragon acc sys2 get specific line

MACRO_ALLOC_POOL_ID_IMPL(CPlayer, MAX_CLIENTS)

IServer *CPlayer::Server() const { return m_pGameServer->Server(); }

CPlayer::CPlayer(CGameContext *pGameServer, int ClientID, int Team)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	m_pGameServer = pGameServer;
	m_ClientID = ClientID;
	m_Team = GameServer()->m_pController->ClampTeam(Team);
	m_pCharacter = 0;
	m_NumInputs = 0;
	m_KillMe = 0;
	Reset();
}

CPlayer::~CPlayer()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	delete m_pCharacter;
	m_pCharacter = 0;
}

void CPlayer::Reset()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	m_RespawnTick = Server()->Tick();
	m_DieTick = Server()->Tick();
	m_ScoreStartTick = Server()->Tick();
	if (m_pCharacter)
		delete m_pCharacter;
	m_pCharacter = 0;
	m_KillMe = 0;
	m_SpectatorID = SPEC_FREEVIEW;
	m_LastActionTick = Server()->Tick();
	m_TeamChangeTick = Server()->Tick();

	int* idMap = Server()->GetIdMap(m_ClientID);
	for (int i = 1;i < VANILLA_MAX_CLIENTS;i++)
	{
		idMap[i] = -1;
	}
	idMap[0] = m_ClientID;

	// DDRace

	m_vWeaponLimit.resize(5);

	m_LastCommandPos = 0;
	m_LastPlaytime = time_get();
	m_Sent1stAfkWarning = 0;
	m_Sent2ndAfkWarning = 0;
	m_ChatScore = 0;
	m_EyeEmote = true;
	m_TimerType = g_Config.m_SvDefaultTimerType;
	m_DefEmote = EMOTE_NORMAL;
	m_Afk = false;
	m_LastWhisperTo = -1;
	m_LastSetSpectatorMode = 0;
	m_TimeoutCode[0] = '\0';

	for(unsigned i = 0; i < sizeof(m_aCatchedID)/sizeof(m_aCatchedID[0]); i++)
		m_aCatchedID[i] = -1;

	m_TuneZone = 0;
	m_TuneZoneOld = m_TuneZone;
	m_Halloween = false;
	m_FirstPacket = true;

	m_SendVoteIndex = -1;

	if (g_Config.m_SvEvents)
	{
		time_t rawtime;
		struct tm* timeinfo;
		char d[16], m[16], y[16];
		int dd, mm;
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		strftime (d,sizeof(y),"%d",timeinfo);
		strftime (m,sizeof(m),"%m",timeinfo);
		strftime (y,sizeof(y),"%Y",timeinfo);
		dd = atoi(d);
		mm = atoi(m);
		if ((mm == 12 && dd == 31) || (mm == 1 && dd == 1))
		{ // New Year
			m_DefEmote = EMOTE_HAPPY;
		}
		else if ((mm == 10 && dd == 31) || (mm == 11 && dd == 1))
		{ // Halloween
			m_DefEmote = EMOTE_ANGRY;
			m_Halloween = true;
		}
		else
		{
			m_DefEmote = EMOTE_NORMAL;
		}
	}
	m_DefEmoteReset = -1;

	GameServer()->Score()->PlayerData(m_ClientID)->Reset();

	m_ClientVersion = VERSION_VANILLA;
	m_ShowOthers = g_Config.m_SvShowOthersDefault;
	m_ShowAll = g_Config.m_SvShowAllDefault;
	m_SpecTeam = 0;
	m_NinjaJetpack = false;

	m_Paused = PAUSED_NONE;
	m_DND = false;

	m_NextPauseTick = 0;

	// Variable initialized: 
	m_Last_Team = 0;
#if defined(CONF_SQL)
	m_LastSQLQuery = 0;
#endif

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

	if (g_Config.m_SvDDPPgametype == 1) //fly
	{
		m_IsVanillaDmg = true;
		m_IsVanillaWeapons = true;
	}

	str_copy(m_aTradeOffer, "", sizeof(m_aTradeOffer));
	str_copy(m_aEscapeReason, "unknown", 16);
	m_dmm25 = -1; //set to offline default
	m_pLoginData = NULL;

	if (g_Config.m_SvNoboSpawnTime)
	{
		m_IsNoboSpawn = true;
	}
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
	m_max_level = 99; //is actually 1 more
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
}

void CPlayer::Tick()
{
#ifdef CONF_DEBUG
	if(!g_Config.m_DbgDummies || m_ClientID < MAX_CLIENTS-g_Config.m_DbgDummies)
		CALL_STACK_ADD();
#endif
	if(!Server()->ClientIngame(m_ClientID))
		return;

	if(m_KillMe != 0)
	{
		KillCharacter(m_KillMe);
		m_KillMe = 0;
		return;
	}

	if (m_ChatScore > 0)
		m_ChatScore--;

	if (m_ForcePauseTime > 0)
		m_ForcePauseTime--;

	Server()->SetClientScore(m_ClientID, m_Score);

	// do latency stuff
	{
		IServer::CClientInfo Info;
		if(Server()->GetClientInfo(m_ClientID, &Info))
		{
			m_Latency.m_Accum += Info.m_Latency;
			m_Latency.m_AccumMax = max(m_Latency.m_AccumMax, Info.m_Latency);
			m_Latency.m_AccumMin = min(m_Latency.m_AccumMin, Info.m_Latency);
		}
		// each second
		if(Server()->Tick()%Server()->TickSpeed() == 0)
		{
			m_Latency.m_Avg = m_Latency.m_Accum/Server()->TickSpeed();
			m_Latency.m_Max = m_Latency.m_AccumMax;
			m_Latency.m_Min = m_Latency.m_AccumMin;
			m_Latency.m_Accum = 0;
			m_Latency.m_AccumMin = 1000;
			m_Latency.m_AccumMax = 0;
		}

	}

	if(((CServer *)Server())->m_NetServer.ErrorString(m_ClientID)[0])
	{
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "'%s' would have timed out, but can use timeout protection now", Server()->ClientName(m_ClientID));
		GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
		((CServer *)(Server()))->m_NetServer.ResetErrorString(m_ClientID);
	}

	if(!GameServer()->m_World.m_Paused)
	{
		if(!m_pCharacter && m_Team == TEAM_SPECTATORS && m_SpectatorID == SPEC_FREEVIEW)
			m_ViewPos -= vec2(clamp(m_ViewPos.x-m_LatestActivity.m_TargetX, -500.0f, 500.0f), clamp(m_ViewPos.y-m_LatestActivity.m_TargetY, -400.0f, 400.0f));

		if(!m_pCharacter && m_DieTick+Server()->TickSpeed()*3 <= Server()->Tick())
			m_Spawning = true;

		if(m_pCharacter)
		{
			if(m_pCharacter->IsAlive())
			{
				if(m_Paused >= PAUSED_FORCE)
				{
					if(m_ForcePauseTime == 0)
					m_Paused = PAUSED_NONE;
					ProcessPause();
				}
				else if(m_Paused == PAUSED_PAUSED && m_NextPauseTick < Server()->Tick())
				{
					if((!m_pCharacter->GetWeaponGot(WEAPON_NINJA) || m_pCharacter->m_FreezeTime) && m_pCharacter->IsGrounded() && m_pCharacter->m_Pos == m_pCharacter->m_PrevPos)
						ProcessPause();
				}
				else if(m_NextPauseTick < Server()->Tick())
				{
					ProcessPause();
				}
				if(!m_Paused)
					m_ViewPos = m_pCharacter->m_Pos;
			}
			else if(!m_pCharacter->IsPaused())
			{
				delete m_pCharacter;
				m_pCharacter = 0;
			}
		}
		else if(m_Spawning && m_RespawnTick <= Server()->Tick())
			TryRespawn();
	}
	else
	{
		++m_RespawnTick;
		++m_DieTick;
		++m_ScoreStartTick;
		++m_LastActionTick;
		++m_TeamChangeTick;
	}

	m_TuneZoneOld = m_TuneZone; // determine needed tunings with viewpos
	int CurrentIndex = GameServer()->Collision()->GetMapIndex(m_ViewPos);
	m_TuneZone = GameServer()->Collision()->IsTune(CurrentIndex);

	if (m_TuneZone != m_TuneZoneOld) // dont send tunigs all the time
	{
		GameServer()->SendTuningParams(m_ClientID, m_TuneZone);
	}

	// chilli clan
	/*int AbstandWarnungen = 10;
	if (str_comp_nocase(Server()->ClientClan(m_ClientID), "Chilli.*") == 0 && str_comp_nocase(m_TeeInfos.m_SkinName, "greensward") != 0)
	{
		if (m_LastWarning + AbstandWarnungen*Server()->TickSpeed() <= Server()->Tick())
		{
			m_ChilliWarnings++;

			if (m_ChilliWarnings >= 4)
			{
				char aRcon[128];
				str_format(aRcon, sizeof(aRcon), "kick %d Chilli", m_ClientID);
				GameServer()->Console()->ExecuteLine(aRcon);
			}
			else
			{
				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "You are using the wrong Skin! Change Skin or clantag! Warnings [%d/3]", m_ChilliWarnings);
				GameServer()->SendChatTarget(m_ClientID, aBuf);
			}

			m_LastWarning = Server()->Tick();
		}
	}*/



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

	if (m_AccountID > 0)
	{
		if (Server()->Tick() % (Server()->TickSpeed() * 300) == 0)
			Save(1); //SetLoggedIn true
	}
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
}

void CPlayer::FixForNoName(int ID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	m_FixNameID = ID;	// 0 for just to display the name in the right moment (e.g. kill msg killer)
	m_SetRealName = true;
	m_SetRealNameTick = Server()->Tick() + Server()->TickSpeed() / 20;

	return;
}

void CPlayer::PostTick()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	// update latency value
	if(m_PlayerFlags&PLAYERFLAG_SCOREBOARD)
	{
		for(int i = 0; i < MAX_CLIENTS; ++i)
		{
			if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS)
				m_aActLatency[i] = GameServer()->m_apPlayers[i]->m_Latency.m_Min;
		}
	}

	// update view pos for spectators
	if((m_Team == TEAM_SPECTATORS || m_Paused) && m_SpectatorID != SPEC_FREEVIEW && GameServer()->m_apPlayers[m_SpectatorID] && GameServer()->m_apPlayers[m_SpectatorID]->GetCharacter())
		m_ViewPos = GameServer()->m_apPlayers[m_SpectatorID]->GetCharacter()->m_Pos;
}

void CPlayer::Snap(int SnappingClient)
{
#ifdef CONF_DEBUG
	if(!g_Config.m_DbgDummies || m_ClientID < MAX_CLIENTS-g_Config.m_DbgDummies)
		CALL_STACK_ADD();
#endif
	if(!Server()->ClientIngame(m_ClientID))
		return;

	int id = m_ClientID;
	if (SnappingClient > -1 && !Server()->Translate(id, SnappingClient)) return;

	CNetObj_ClientInfo *pClientInfo = static_cast<CNetObj_ClientInfo *>(Server()->SnapNewItem(NETOBJTYPE_CLIENTINFO, id, sizeof(CNetObj_ClientInfo)));

	if(!pClientInfo)
		return;

	//survival nameplate system (not too dope yet needs some fine tuning)
	m_ShowName = false;

	CPlayer *pSnapping = GameServer()->m_apPlayers[SnappingClient];
	
	if (pSnapping)
	{
		if (pSnapping->GetTeam() == TEAM_SPECTATORS || !pSnapping->m_IsSurvivaling || !pSnapping->m_IsSurvivalAlive) //could add a bool is dead here too to activate name agian
		{
			m_ShowName = true;
		}
	}

	if (g_Config.m_SvNameplates)
	{
		m_ShowName = true;
	}

	StrToInts(&pClientInfo->m_Clan0, 3, Server()->ClientClan(m_ClientID));
	pClientInfo->m_Country = Server()->ClientCountry(m_ClientID);

	//spooky ghost
	const char *pClan;
	if (m_SpookyGhostActive)
		pClan = m_RealName;
	else
		pClan = m_RealClan;
	StrToInts(&pClientInfo->m_Clan0, 3, pClan);

	if (m_SpookyGhostActive)
	{
		m_ShowName = false;
	}

	if (m_SetRealName || m_ShowName)
	{
		StrToInts(&pClientInfo->m_Name0, 4, Server()->ClientName(m_ClientID));
	}
	else
	{
		StrToInts(&pClientInfo->m_Name0, 4, " ");
	}

	if (m_PlayerFlags&PLAYERFLAG_SCOREBOARD)
	{
		if (GetCharacter())
		{
			GetCharacter()->m_ShopMotdTick = 0;
		}
	}
	else
	{
		if (GetCharacter())
		{
			GetCharacter()->m_TimesShot = 0;
		}
	}

	if (GetCharacter() && GetCharacter()->m_IsBomb) //bomb (keep bomb 1st. Because bomb over all rainbow and other stuff shoudl be ignored if bomb)
	{
		StrToInts(&pClientInfo->m_Skin0, 6, m_TeeInfos.m_SkinName);
		pClientInfo->m_UseCustomColor = true;


		if (GameServer()->m_BombTick < 75) //red glowup right before explode
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
			if (!ColorChangeVal)
			{
				ColorChangeVal = 1;
			}

			if (GameServer()->m_BombColor > 254)
			{
				GameServer()->m_bwff = false;
			}
			if (GameServer()->m_BombColor < 1)
			{
				GameServer()->m_bwff = true;
			}

			if (GameServer()->m_bwff) //black -> white
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
	else if (m_InfRainbow || GameServer()->IsHooked(GetCID(), 1) || (GetCharacter() && GetCharacter()->m_Rainbow && !GetCharacter()->m_IsBombing)) //rainbow (hide finit rainbow if in bomb game)
	{
		StrToInts(&pClientInfo->m_Skin0, 6, m_TeeInfos.m_SkinName);
		pClientInfo->m_UseCustomColor = true;
		m_RainbowColor = (m_RainbowColor + 1) % 256;
		pClientInfo->m_ColorBody = m_RainbowColor * 0x010000 + 0xff00;
		pClientInfo->m_ColorFeet = m_RainbowColor * 0x010000 + 0xff00;
	}
	//else if (m_IsTest) //test color values
	//{
	//	StrToInts(&pClientInfo->m_Skin0, 6, m_TeeInfos.m_SkinName);
	//	pClientInfo->m_UseCustomColor = true;
	//	pClientInfo->m_ColorBody = (g_Config.m_SvTestValA * g_Config.m_SvTestValB / g_Config.m_SvTestValC);
	//	pClientInfo->m_ColorFeet = (255 * 255 / 1);
	//}
	else if (m_StolenSkin && SnappingClient != m_ClientID && g_Config.m_SvSkinStealAction == 1) //steal skin
	{
		StrToInts(&pClientInfo->m_Skin0, 6, "pinky");
		pClientInfo->m_UseCustomColor = 0;
		pClientInfo->m_ColorBody = m_TeeInfos.m_ColorBody;
		pClientInfo->m_ColorFeet = m_TeeInfos.m_ColorFeet;
	} else
	{
		StrToInts(&pClientInfo->m_Skin0, 6, m_TeeInfos.m_SkinName);
		pClientInfo->m_UseCustomColor = m_TeeInfos.m_UseCustomColor;
		pClientInfo->m_ColorBody = m_TeeInfos.m_ColorBody;
		pClientInfo->m_ColorFeet = m_TeeInfos.m_ColorFeet;
	}

	CNetObj_PlayerInfo *pPlayerInfo = static_cast<CNetObj_PlayerInfo *>(Server()->SnapNewItem(NETOBJTYPE_PLAYERINFO, id, sizeof(CNetObj_PlayerInfo)));
	if(!pPlayerInfo)
		return;

	pPlayerInfo->m_Latency = SnappingClient == -1 ? m_Latency.m_Min : GameServer()->m_apPlayers[SnappingClient]->m_aActLatency[m_ClientID];
	pPlayerInfo->m_Local = 0;
	pPlayerInfo->m_ClientID = id;
	if (g_Config.m_SvInstagibMode)
	{
		pPlayerInfo->m_Score = m_Score;
	}
	else
	{
		pPlayerInfo->m_Score = abs(m_Score) * -1;
	}
	pPlayerInfo->m_Team = (m_Paused != PAUSED_SPEC || m_ClientID != SnappingClient) && m_Paused < PAUSED_PAUSED ? m_Team : TEAM_SPECTATORS;

	if(m_ClientID == SnappingClient)
		pPlayerInfo->m_Local = 1;

	if(m_ClientID == SnappingClient && (m_Team == TEAM_SPECTATORS || m_Paused))
	{
		CNetObj_SpectatorInfo *pSpectatorInfo = static_cast<CNetObj_SpectatorInfo *>(Server()->SnapNewItem(NETOBJTYPE_SPECTATORINFO, m_ClientID, sizeof(CNetObj_SpectatorInfo)));
		if(!pSpectatorInfo)
			return;

		pSpectatorInfo->m_SpectatorID = m_SpectatorID;
		pSpectatorInfo->m_X = m_ViewPos.x;
		pSpectatorInfo->m_Y = m_ViewPos.y;
	}

	// send 0 if times of others are not shown
	if (SnappingClient != m_ClientID && g_Config.m_SvHideScore)
	{
		pPlayerInfo->m_Score = -9999;
	}
	else if (GameServer()->m_apPlayers[SnappingClient]->IsInstagibMinigame())
	{
		if (IsInstagibMinigame())
		{
			if (GameServer()->m_apPlayers[SnappingClient]->m_IsSupportedDDNet)
				pPlayerInfo->m_Score = m_InstaScore * 60;
			else
				pPlayerInfo->m_Score = m_InstaScore;
		}
		else
			pPlayerInfo->m_Score = -9999;
	}
	else if (GameServer()->m_apPlayers[SnappingClient]->m_DisplayScore != 0) // race time
	{
		if (GameServer()->m_apPlayers[SnappingClient]->m_DisplayScore == 1) // level
		{
			if (m_level > 0)
			{
				if (GameServer()->m_apPlayers[SnappingClient]->m_IsSupportedDDNet)
					pPlayerInfo->m_Score = m_level * 60;
				else
					pPlayerInfo->m_Score = m_level;
			}
			else if (!GameServer()->m_apPlayers[SnappingClient]->m_IsSupportedDDNet)
				pPlayerInfo->m_Score = 0;
			else
				pPlayerInfo->m_Score = -9999;
		}
		else if (GameServer()->m_apPlayers[SnappingClient]->m_DisplayScore == 2) // block points
		{
			if (m_BlockPoints > 0)
			{
				if (GameServer()->m_apPlayers[SnappingClient]->m_IsSupportedDDNet)
					pPlayerInfo->m_Score = m_BlockPoints * 60;
				else
					pPlayerInfo->m_Score = m_BlockPoints;
			}
			else if (!GameServer()->m_apPlayers[SnappingClient]->m_IsSupportedDDNet)
				pPlayerInfo->m_Score = 0;
			else
				pPlayerInfo->m_Score = -9999;
		}
	}
	else
	{
		if (g_Config.m_SvInstagibMode)
		{
			pPlayerInfo->m_Score = m_Score;
		}
		else
		{
			pPlayerInfo->m_Score = abs(m_Score) * -1;
		}
		
	}
}

void CPlayer::FakeSnap(int SnappingClient)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	// This is problematic when it's sent before we know whether it's a non-64-player-client
	// Then we can't spectate players at the start
	IServer::CClientInfo info;
	Server()->GetClientInfo(SnappingClient, &info);
	CGameContext *GameContext = (CGameContext *) GameServer();
	if (SnappingClient > -1 && GameContext->m_apPlayers[SnappingClient] && GameContext->m_apPlayers[SnappingClient]->m_ClientVersion >= VERSION_DDNET_OLD)
		return;

	int id = VANILLA_MAX_CLIENTS - 1;

	CNetObj_ClientInfo *pClientInfo = static_cast<CNetObj_ClientInfo *>(Server()->SnapNewItem(NETOBJTYPE_CLIENTINFO, id, sizeof(CNetObj_ClientInfo)));

	if(!pClientInfo)
		return;

	StrToInts(&pClientInfo->m_Name0, 4, " ");
	StrToInts(&pClientInfo->m_Clan0, 3, Server()->ClientClan(m_ClientID));
	StrToInts(&pClientInfo->m_Skin0, 6, m_TeeInfos.m_SkinName);
}

void CPlayer::OnDisconnect(const char *pReason, bool silent)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

	if (m_Insta1on1_id != -1 && (m_IsInstaArena_gdm || m_IsInstaArena_idm))
	{
		GameServer()->WinInsta1on1(m_Insta1on1_id, GetCID());
	}

	KillCharacter();

	Logout();
	if(Server()->ClientIngame(m_ClientID) && !silent && (g_Config.m_SvHideJoinLeaveMessages == 3 || g_Config.m_SvHideJoinLeaveMessages == 2) && (g_Config.m_SvHideJoinLeaveMessagesPlayer != Server()->ClientName(m_ClientID)))
	{
		char aBuf[512];
		if (!str_comp(g_Config.m_SvHideJoinLeaveMessagesPlayer, Server()->ClientName(m_ClientID)))
		{
			str_format(aBuf, sizeof(aBuf), "player='%d:%s' leave (message hidden)", m_ClientID, Server()->ClientName(m_ClientID));
			GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
		}
		else if (g_Config.m_SvActivatePatternFilter)
		{
			if (str_find(Server()->ClientName(GetCID()), g_Config.m_SvHideJoinLeaveMessagesPattern))
			{
				//hide pattern
			}
			else
			{
				if (pReason && *pReason)
					str_format(aBuf, sizeof(aBuf), "'%s' has left the game (%s)", Server()->ClientName(m_ClientID), pReason);
				else
					str_format(aBuf, sizeof(aBuf), "'%s' has left the game", Server()->ClientName(m_ClientID));
				GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);

				str_format(aBuf, sizeof(aBuf), "leave player='%d:%s'", m_ClientID, Server()->ClientName(m_ClientID));
				GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "game", aBuf);
			}
		}
		else
		{
			if (pReason && *pReason)
				str_format(aBuf, sizeof(aBuf), "'%s' has left the game (%s)", Server()->ClientName(m_ClientID), pReason);
			else
				str_format(aBuf, sizeof(aBuf), "'%s' has left the game", Server()->ClientName(m_ClientID));
			GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);

			str_format(aBuf, sizeof(aBuf), "leave player='%d:%s'", m_ClientID, Server()->ClientName(m_ClientID));
			GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "game", aBuf);
		}
	}

	CGameControllerDDRace* Controller = (CGameControllerDDRace*)GameServer()->m_pController;
	Controller->m_Teams.SetForceCharacterTeam(m_ClientID, 0);
}

void CPlayer::OnPredictedInput(CNetObj_PlayerInput *NewInput)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	// skip the input if chat is active
	if((m_PlayerFlags&PLAYERFLAG_CHATTING) && (NewInput->m_PlayerFlags&PLAYERFLAG_CHATTING))
		return;

	AfkVoteTimer(NewInput);

	m_NumInputs++;

	if(m_pCharacter && !m_Paused)
		m_pCharacter->OnPredictedInput(NewInput);

	// Magic number when we can hope that client has successfully identified itself
	/*if(m_NumInputs == 20)
	{
		if(g_Config.m_SvClientSuggestion[0] != '\0' && m_ClientVersion <= VERSION_DDNET_OLD)
			GameServer()->SendBroadcast(g_Config.m_SvClientSuggestion, m_ClientID);
	}*/
}

void CPlayer::OnDirectInput(CNetObj_PlayerInput *NewInput)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (AfkTimer(NewInput->m_TargetX, NewInput->m_TargetY))
		return; // we must return if kicked, as player struct is already deleted
	AfkVoteTimer(NewInput);

	if(NewInput->m_PlayerFlags&PLAYERFLAG_CHATTING)
	{
	// skip the input if chat is active
		if(m_PlayerFlags&PLAYERFLAG_CHATTING)
		return;

		// reset input
		if(m_pCharacter)
			m_pCharacter->ResetInput();

		m_PlayerFlags = NewInput->m_PlayerFlags;
		return;
	}

	m_PlayerFlags = NewInput->m_PlayerFlags;

	if(m_pCharacter)
	{
		if(!m_Paused)
			m_pCharacter->OnDirectInput(NewInput);
		else
			m_pCharacter->ResetInput();
	}

	if(!m_pCharacter && m_Team != TEAM_SPECTATORS && (NewInput->m_Fire&1))
		m_Spawning = true;

	if(((!m_pCharacter && m_Team == TEAM_SPECTATORS) || m_Paused) && m_SpectatorID == SPEC_FREEVIEW)
		m_ViewPos = vec2(NewInput->m_TargetX, NewInput->m_TargetY);

	// check for activity
	if(NewInput->m_Direction || m_LatestActivity.m_TargetX != NewInput->m_TargetX ||
		m_LatestActivity.m_TargetY != NewInput->m_TargetY || NewInput->m_Jump ||
		NewInput->m_Fire&1 || NewInput->m_Hook)
	{
		m_LatestActivity.m_TargetX = NewInput->m_TargetX;
		m_LatestActivity.m_TargetY = NewInput->m_TargetY;
		m_LastActionTick = Server()->Tick();
	}
}

CCharacter *CPlayer::GetCharacter()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if(m_pCharacter && m_pCharacter->IsAlive())
		return m_pCharacter;
	return 0;
}

void CPlayer::ThreadKillCharacter(int Weapon)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	m_KillMe = Weapon;
}

void CPlayer::KillCharacter(int Weapon)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if(m_pCharacter)
	{
		if (m_RespawnTick > Server()->Tick())
			return;

		m_pCharacter->Die(m_ClientID, Weapon);

		delete m_pCharacter;
		m_pCharacter = 0;
	}
}

void CPlayer::Respawn()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if(m_Team != TEAM_SPECTATORS)
		m_Spawning = true;
}

CCharacter* CPlayer::ForceSpawn(vec2 Pos)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	m_Spawning = false;
	m_pCharacter = new(m_ClientID) CCharacter(&GameServer()->m_World);
	m_pCharacter->Spawn(this, Pos);
	m_Team = 0;
	return m_pCharacter;
}

void CPlayer::SetTeam(int Team, bool DoChatMsg)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	// clamp the team
	Team = GameServer()->m_pController->ClampTeam(Team);
	if(m_Team == Team)
		return;

	char aBuf[512];
	if(DoChatMsg)
	{
		if (!str_comp(g_Config.m_SvHideJoinLeaveMessagesPlayer, Server()->ClientName(GetCID())))
		{
			//send it in admin console
		}
		else if (g_Config.m_SvHideJoinLeaveMessages < 3)
		{
			//send it in admin console
		}
		else if (g_Config.m_SvActivatePatternFilter)
		{
			if (str_find(Server()->ClientName(GetCID()), g_Config.m_SvHideJoinLeaveMessagesPattern))
			{
				//hide pattern
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "'%s' joined the %s", Server()->ClientName(m_ClientID), GameServer()->m_pController->GetTeamName(Team));
				GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
			}
		}
		else
		{
			str_format(aBuf, sizeof(aBuf), "'%s' joined the %s", Server()->ClientName(m_ClientID), GameServer()->m_pController->GetTeamName(Team));
			GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
		}
	}

	if(Team == TEAM_SPECTATORS)
	{
		CGameControllerDDRace* Controller = (CGameControllerDDRace*)GameServer()->m_pController;
		Controller->m_Teams.SetForceCharacterTeam(m_ClientID, 0);
	}

	KillCharacter();

	m_Team = Team;
	m_LastSetTeam = Server()->Tick();
	m_LastActionTick = Server()->Tick();
	m_SpectatorID = SPEC_FREEVIEW;
	m_RespawnTick = Server()->Tick();
	//m_RespawnTick = 0;
	str_format(aBuf, sizeof(aBuf), "team_join player='%d:%s' m_Team=%d", m_ClientID, Server()->ClientName(m_ClientID), m_Team);
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

	//GameServer()->m_pController->OnPlayerInfoChange(GameServer()->m_apPlayers[m_ClientID]);

	if(Team == TEAM_SPECTATORS)
	{
		// update spectator modes
		for(int i = 0; i < MAX_CLIENTS; ++i)
		{
			if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->m_SpectatorID == m_ClientID)
				GameServer()->m_apPlayers[i]->m_SpectatorID = SPEC_FREEVIEW;
		}
	}
}

void CPlayer::TryRespawn()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	vec2 SpawnPos;

	if(!GameServer()->m_pController->CanSpawn(m_Team, &SpawnPos, this))
		return;

	CGameControllerDDRace* Controller = (CGameControllerDDRace*)GameServer()->m_pController;

	m_Spawning = false;
	m_pCharacter = new(m_ClientID) CCharacter(&GameServer()->m_World);
	m_pCharacter->Spawn(this, SpawnPos);
	GameServer()->CreatePlayerSpawn(SpawnPos, m_pCharacter->Teams()->TeamMask(m_pCharacter->Team(), -1, m_ClientID));

	if(g_Config.m_SvTeam == 3)
	{
		int NewTeam = 0;
		for(; NewTeam < TEAM_SUPER; NewTeam++)
			if(Controller->m_Teams.Count(NewTeam) == 0)
				break;

		if(NewTeam == TEAM_SUPER)
			NewTeam = 0;

		Controller->m_Teams.SetForceCharacterTeam(GetCID(), NewTeam);
	}
}

bool CPlayer::AfkTimer(int NewTargetX, int NewTargetY)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	/*
		afk timer (x, y = mouse coordinates)
		Since a player has to move the mouse to play, this is a better method than checking
		the player's position in the game world, because it can easily be bypassed by just locking a key.
		Frozen players could be kicked as well, because they can't move.
		It also works for spectators.
		returns true if kicked
	*/

	if(m_Authed)
		return false; // don't kick admins
	if(g_Config.m_SvMaxAfkTime == 0)
		return false; // 0 = disabled

	if(NewTargetX != m_LastTarget_x || NewTargetY != m_LastTarget_y)
	{
		m_LastPlaytime = time_get();
		m_LastTarget_x = NewTargetX;
		m_LastTarget_y = NewTargetY;
		m_Sent1stAfkWarning = 0; // afk timer's 1st warning after 50% of sv_max_afk_time
		m_Sent2ndAfkWarning = 0;

	}
	else
	{
		if(!m_Paused)
		{
			// not playing, check how long
			if(m_Sent1stAfkWarning == 0 && m_LastPlaytime < time_get()-time_freq()*(int)(g_Config.m_SvMaxAfkTime*0.5))
			{
				sprintf(
					m_pAfkMsg,
					"You have been afk for %d seconds now. Please note that you get kicked after not playing for %d seconds.",
					(int)(g_Config.m_SvMaxAfkTime*0.5),
					g_Config.m_SvMaxAfkTime
				);
				m_pGameServer->SendChatTarget(m_ClientID, m_pAfkMsg);
				m_Sent1stAfkWarning = 1;
			}
			else if(m_Sent2ndAfkWarning == 0 && m_LastPlaytime < time_get()-time_freq()*(int)(g_Config.m_SvMaxAfkTime*0.9))
			{
				sprintf(
					m_pAfkMsg,
					"You have been afk for %d seconds now. Please note that you get kicked after not playing for %d seconds.",
					(int)(g_Config.m_SvMaxAfkTime*0.9),
					g_Config.m_SvMaxAfkTime
				);
				m_pGameServer->SendChatTarget(m_ClientID, m_pAfkMsg);
				m_Sent2ndAfkWarning = 1;
			}
			else if(m_LastPlaytime < time_get()-time_freq()*g_Config.m_SvMaxAfkTime)
			{
				CServer* serv =	(CServer*)m_pGameServer->Server();
				serv->Kick(m_ClientID,"Away from keyboard");
				return true;
			}
		}
	}
	return false;
}

void CPlayer::AfkVoteTimer(CNetObj_PlayerInput *NewTarget)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if(g_Config.m_SvMaxAfkVoteTime == 0)
		return;

	if(mem_comp(NewTarget, &m_LastTarget, sizeof(CNetObj_PlayerInput)) != 0)
	{
		m_LastPlaytime = time_get();
		mem_copy(&m_LastTarget, NewTarget, sizeof(CNetObj_PlayerInput));
	}
	else if(m_LastPlaytime < time_get()-time_freq()*g_Config.m_SvMaxAfkVoteTime)
	{
		m_Afk = true;
		return;
	}

	m_Afk = false;
}

void CPlayer::ProcessPause()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if(!m_pCharacter)
		return;

	char aBuf[128];
	if(m_Paused >= PAUSED_PAUSED)
	{
		if(!m_pCharacter->IsPaused())
		{
			m_pCharacter->Pause(true);
			if(g_Config.m_SvPauseMessages)
			{
				str_format(aBuf, sizeof(aBuf), (m_Paused == PAUSED_PAUSED) ? "'%s' paused" : "'%s' was force-paused for %ds", Server()->ClientName(m_ClientID), m_ForcePauseTime/Server()->TickSpeed());
				GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
			}
			GameServer()->CreateDeath(m_pCharacter->m_Pos, m_ClientID, m_pCharacter->Teams()->TeamMask(m_pCharacter->Team(), -1, m_ClientID));
			GameServer()->CreateSound(m_pCharacter->m_Pos, SOUND_PLAYER_DIE, m_pCharacter->Teams()->TeamMask(m_pCharacter->Team(), -1, m_ClientID));
			m_NextPauseTick = Server()->Tick() + g_Config.m_SvPauseFrequency * Server()->TickSpeed();
		}
	}
	else
	{
		if(m_pCharacter->IsPaused())
		{
			m_pCharacter->Pause(false);
			if(g_Config.m_SvPauseMessages)
			{
				str_format(aBuf, sizeof(aBuf), "'%s' resumed", Server()->ClientName(m_ClientID));
				GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
			}
			GameServer()->CreatePlayerSpawn(m_pCharacter->m_Pos, m_pCharacter->Teams()->TeamMask(m_pCharacter->Team(), -1, m_ClientID));
			m_NextPauseTick = Server()->Tick() + g_Config.m_SvPauseFrequency * Server()->TickSpeed();
		}
	}
}

bool CPlayer::IsPlaying()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if(m_pCharacter && m_pCharacter->IsAlive())
		return true;
	return false;
}

void CPlayer::FindDuplicateSkins()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (m_TeeInfos.m_UseCustomColor == 0 && !m_StolenSkin) return;
	m_StolenSkin = 0;
	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (i == m_ClientID) continue;
		if(GameServer()->m_apPlayers[i])
		{
			if (GameServer()->m_apPlayers[i]->m_StolenSkin) continue;
			if ((GameServer()->m_apPlayers[i]->m_TeeInfos.m_UseCustomColor == m_TeeInfos.m_UseCustomColor) &&
			(GameServer()->m_apPlayers[i]->m_TeeInfos.m_ColorFeet == m_TeeInfos.m_ColorFeet) &&
			(GameServer()->m_apPlayers[i]->m_TeeInfos.m_ColorBody == m_TeeInfos.m_ColorBody) &&
			!str_comp(GameServer()->m_apPlayers[i]->m_TeeInfos.m_SkinName, m_TeeInfos.m_SkinName))
			{
				m_StolenSkin = 1;
				return;
			}
		}
	}
}

void CPlayer::Logout(int SetLoggedIn)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (m_AccountID <= 0)
		return;

	Save(SetLoggedIn);
	dbg_msg("account", "Logged out: %d", m_AccountID);

	//reset values to default to prevent cheating
	m_AccountID = 0;
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
	str_format(m_ProfileStatus, sizeof(m_ProfileStatus), "");
	str_format(m_ProfileSkype, sizeof(m_ProfileSkype), "");
	str_format(m_ProfileYoutube, sizeof(m_ProfileYoutube), "");
	str_format(m_ProfileEmail, sizeof(m_ProfileEmail), "");
	str_format(m_ProfileHomepage, sizeof(m_ProfileHomepage), "");
	str_format(m_ProfileTwitter, sizeof(m_ProfileTwitter), "");
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
	str_format(m_money_transaction9, sizeof(m_money_transaction9), "");
	str_format(m_money_transaction8, sizeof(m_money_transaction8), "");
	str_format(m_money_transaction7, sizeof(m_money_transaction7), "");
	str_format(m_money_transaction6, sizeof(m_money_transaction6), "");
	str_format(m_money_transaction5, sizeof(m_money_transaction5), "");
	str_format(m_money_transaction4, sizeof(m_money_transaction4), "");
	str_format(m_money_transaction3, sizeof(m_money_transaction3), "");
	str_format(m_money_transaction2, sizeof(m_money_transaction2), "");
	str_format(m_money_transaction1, sizeof(m_money_transaction1), "");
	str_format(m_money_transaction0, sizeof(m_money_transaction0), "");
}

void CPlayer::JailPlayer(int seconds)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	vec2 JailPlayerSpawn = GameServer()->Collision()->GetRandomTile(TILE_JAIL);
	//vec2 DefaultSpawn = GameServer()->Collision()->GetRandomTile(ENTITY_SPAWN);

	m_JailTime = Server()->TickSpeed() * seconds;

	if (GetCharacter())
	{
		if (JailPlayerSpawn != vec2(-1, -1))
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (m_AccountID <= 0)
		return;

	char *pQueryBuf = sqlite3_mprintf("UPDATE `Accounts` SET `Password` = '%q'  WHERE `ID` = %i", m_aChangePassword, m_AccountID);

	dbg_msg("sql", "pass: %s id: %d", m_aChangePassword, m_AccountID);

	CQuery *pQuery = new CQuery();
	pQuery->Query(GameServer()->m_Database, pQueryBuf);
	sqlite3_free(pQueryBuf);
}

void CPlayer::Save(int SetLoggedIn)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
	//dbg_msg("cBug", "saving acc '%s' CID=%d", Server()->ClientName(GetCID()), GetCID());
#endif
	if (m_AccountID <= 0)
		return;

	if (m_IsFileAcc)
	{
		SaveFileBased(SetLoggedIn);
		return;
	}

	//Proccess Clan Data...
	char aClan[32];
	str_copy(aClan, Server()->ClientClan(m_ClientID), sizeof(aClan));

	if (str_comp(aClan, m_aClan1) && str_comp(aClan, m_aClan2) && str_comp(aClan, m_aClan3))
	{
		//dbg_msg("save", "update clan '%s'", aClan);
		str_format(m_aClan3, sizeof(m_aClan3), "%s", m_aClan2);
		str_format(m_aClan2, sizeof(m_aClan2), "%s", m_aClan1);
		str_format(m_aClan1, sizeof(m_aClan1), "%s", aClan);
	}

	//Proccess IP ADDR...
	char aIP[32];
	Server()->GetClientAddr(GetCID(), aIP, sizeof(aIP));

	if (str_comp(aIP, m_aIP_1) && str_comp(aIP, m_aIP_2) && str_comp(aIP, m_aIP_3))
	{
		//dbg_msg("save", "updated ip '%s'", aIP);
		str_format(m_aIP_3, sizeof(m_aIP_3), "%s", m_aIP_2);
		str_format(m_aIP_2, sizeof(m_aIP_2), "%s", m_aIP_1);
		str_format(m_aIP_1, sizeof(m_aIP_1), "%s", aIP);
	}

	//Proccess IngameName Data...
	char aName[32];
	str_copy(aName, Server()->ClientName(m_ClientID), sizeof(aName));

	if (!str_comp(aName, m_LastLogoutIGN1) || !str_comp(aName, m_LastLogoutIGN2) || !str_comp(aName, m_LastLogoutIGN3) || !str_comp(aName, m_LastLogoutIGN4) || !str_comp(aName, m_LastLogoutIGN5))
	{
		if (!str_comp(aName, m_LastLogoutIGN1))
		{
			m_iLastLogoutIGN1_usage++;
		}
		else if (!str_comp(aName, m_LastLogoutIGN2))
		{
			m_iLastLogoutIGN2_usage++;
		}
		else if (!str_comp(aName, m_LastLogoutIGN3))
		{
			m_iLastLogoutIGN3_usage++;
		}
		else if (!str_comp(aName, m_LastLogoutIGN4))
		{
			m_iLastLogoutIGN4_usage++;
		}
		else if (!str_comp(aName, m_LastLogoutIGN5))
		{
			m_iLastLogoutIGN5_usage++;
		}
	}
	else //new name --> add it in history and overwrite the oldest
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

	//read showhide bools to char array that is being saved
	//GameServer()->ShowHideConfigBoolToChar(this->GetCID());

	//not working
	//char *pQueryBuf = sqlite3_mprintf("UPDATE `Accounts` SET `Level` = %i, `Exp` = %i, `Money` = %i, `Shit` = %i, `LastGift` = %i, `PoliceRank` = %i, `JailTime` = %i, `EscapeTime` = %i, `TaserLevel` = %i, `PvPArenaTickets` = %i, `PvPArenaGames` = %i, `PvPArenaKills` = %i, `PvPArenaDeaths` = %i,`ProfileStyle` = %i, `ProfileViews` = %i, `ProfileStatus` = %s, `ProfileSkype` = %s, `ProfileYoutube` = %s, `ProfileEmail` = %s, `ProfileHomepage` = %s, `ProfileTwitter` = %s WHERE `ID` = %i",
	//	m_level, m_xp, m_money, m_shit, m_LastGift, m_PoliceRank, m_JailTime, m_EscapeTime, m_TaserLevel, m_pvp_arena_tickets, m_pvp_arena_games_played, m_pvp_arena_kills, m_pvp_arena_deaths, m_ProfileStyle, m_ProfileViews, m_ProfileStatus, m_ProfileYoutube, m_ProfileEmail, m_ProfileHomepage, m_ProfileTwitter, m_AccountID);

	//working without txt
	//char *pQueryBuf = sqlite3_mprintf("UPDATE `Accounts` SET `Level` = %i, `Exp` = %i, `Money` = %i, `Shit` = %i, `LastGift` = %i, `PoliceRank` = %i, `JailTime` = %i, `EscapeTime` = %i, `TaserLevel` = %i, `PvPArenaTickets` = %i, `PvPArenaGames` = %i, `PvPArenaKills` = %i, `PvPArenaDeaths` = %i, `ProfileStyle` = %i, `ProfileViews` = %i WHERE `ID` = %i",
	//	m_level, m_xp, m_money, m_shit, m_LastGift, m_PoliceRank, m_JailTime, m_EscapeTime, m_TaserLevel, m_pvp_arena_tickets, m_pvp_arena_games_played, m_pvp_arena_kills, m_pvp_arena_deaths, m_ProfileStyle, m_ProfileViews, m_AccountID);

	//5 last names (broke :c)
	//char *pQueryBuf = sqlite3_mprintf("UPDATE `Accounts` SET `Level` = %i, `Exp` = %i, `Money` = %i, `Shit` = %i, `LastGift` = %i, `PoliceRank` = %i, `JailTime` = %i, `EscapeTime` = %i, `TaserLevel` = %i, `PvPArenaTickets` = %i, `PvPArenaGames` = %i, `PvPArenaKills` = %i, `PvPArenaDeaths` = %i,`ProfileStyle` = %i, `ProfileViews` = %i, `ProfileStatus` = '%s', `ProfileSkype` = '%s', `ProfileYoutube` = '%s', `ProfileEmail` = '%s', `ProfileHomepage` = '%s', `ProfileTwitter` = '%s', `HomingMissiles` = '%i', `BlockPoints` = '%i', `BlockKills` = '%i', `BlockDeaths` = '%i', `IsModerator` = '%i', `IsSuperModerator` = '%i', `IsAccFrozen` = '%i', `LastLogoutIGN1` = '%s', `LastLogoutIGN2` = '%s', `LastLogoutIGN3` = '%s', `LastLogoutIGN4` = '%s', `LastLogoutIGN5` = '%s' WHERE `ID` = %i",
	//	m_level, m_xp, m_money, m_shit, m_LastGift, m_PoliceRank, m_JailTime, m_EscapeTime, m_TaserLevel, m_pvp_arena_tickets, m_pvp_arena_games_played, m_pvp_arena_kills, m_pvp_arena_deaths, m_ProfileStyle, m_ProfileViews, m_ProfileStatus, m_ProfileSkype, m_ProfileYoutube, m_ProfileEmail, m_ProfileHomepage, m_ProfileTwitter, m_homing_missiles_ammo, m_BlockPoints, m_BlockPoints_Kills, m_BlockPoints_Deaths, m_IsModerator, m_IsSuperModerator, m_IsAccFrozen, m_LastLogoutIGN1, m_LastLogoutIGN2, m_LastLogoutIGN3, m_LastLogoutIGN4, m_LastLogoutIGN4, m_LastLogoutIGN5, m_AccountID);

	//1 last name (working)
	//char *pQueryBuf = sqlite3_mprintf("UPDATE `Accounts` SET `Level` = %i, `Exp` = %i, `Money` = %i, `Shit` = %i, `LastGift` = %i, `PoliceRank` = %i, `JailTime` = %i, `EscapeTime` = %i, `TaserLevel` = %i, `PvPArenaTickets` = %i, `PvPArenaGames` = %i, `PvPArenaKills` = %i, `PvPArenaDeaths` = %i,`ProfileStyle` = %i, `ProfileViews` = %i, `ProfileStatus` = '%s', `ProfileSkype` = '%s', `ProfileYoutube` = '%s', `ProfileEmail` = '%s', `ProfileHomepage` = '%s', `ProfileTwitter` = '%s', `HomingMissiles` = '%i', `BlockPoints` = '%i', `BlockKills` = '%i', `BlockDeaths` = '%i', `IsModerator` = '%i', `IsSuperModerator` = '%i', `IsAccFrozen` = '%i', `LastLogoutIGN1` = '%s' WHERE `ID` = %i",
	//	m_level, m_xp, m_money, m_shit, m_LastGift, m_PoliceRank, m_JailTime, m_EscapeTime, m_TaserLevel, m_pvp_arena_tickets, m_pvp_arena_games_played, m_pvp_arena_kills, m_pvp_arena_deaths, m_ProfileStyle, m_ProfileViews, m_ProfileStatus, m_ProfileSkype, m_ProfileYoutube, m_ProfileEmail, m_ProfileHomepage, m_ProfileTwitter, m_homing_missiles_ammo, m_BlockPoints, m_BlockPoints_Kills, m_BlockPoints_Deaths, m_IsModerator, m_IsSuperModerator, m_IsAccFrozen, Server()->ClientName(GetCID()), m_AccountID);

	//test more last igns (working)
	char *pQueryBuf = sqlite3_mprintf("UPDATE `Accounts` SET"
											  "  `Password` = '%q', `Level` = %i, `Exp` = %i, `Money` = %i, `Shit` = %i"
											  ", `LastGift` = %i" /*is actually m_GiftDelay*/
											  ", `PoliceRank` = %i"
											  ", `JailTime` = %i, `EscapeTime` = %i"
											  ", `TaserLevel` = %i"
										      ", `NinjaJetpackBought` = %i"
											  ", `SpookyGhost` = %i"
											  ", `UseSpawnWeapons` = %i"
											  ", `SpawnWeaponShotgun` = %i"
											  ", `SpawnWeaponGrenade` = %i"
											  ", `SpawnWeaponRifle` = %i"
											  ", `PvPArenaTickets` = %i, `PvPArenaGames` = %i, `PvPArenaKills` = %i, `PvPArenaDeaths` = %i"
											  ", `ProfileStyle` = %i, `ProfileViews` = %i, `ProfileStatus` = '%q', `ProfileSkype` = '%q', `ProfileYoutube` = '%q', `ProfileEmail` = '%q', `ProfileHomepage` = '%q', `ProfileTwitter` = '%q'"
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
											  " WHERE `ID` = %i",
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
												m_AccountID
	);

	CQuery *pQuery = new CQuery();
	pQuery->Query(GameServer()->m_Database, pQueryBuf);
	sqlite3_free(pQueryBuf);
}

void CPlayer::SaveFileBased(int SetLoggedIn)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

	std::string data;
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "%s/%s.acc", g_Config.m_SvFileAccPath, m_aAccountLoginName);
	std::ofstream Acc2File(aBuf);

	if (Acc2File.is_open())
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
		dbg_msg("acc2","[WARNING] account '%s' (%s) failed to save", m_aAccountLoginName, aBuf);
		Acc2File.close();
	}
}

void CPlayer::CalcExp()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

	dbg_msg("debug", "caling exp");

	//old dynamic shit rofl
	//if (m_level < 1)
	//	m_neededxp = 5000;
	//else
	//	//m_neededxp = 5000 * (m_level * 5.5);
	//	//old fucked the account system because neededxp wasnt saved and if u disconnect u need more xp
	//	//m_neededxp = m_xp + (m_level * 2500);   <-- old one had the xp value in it what changes so the neededxp changes on account load -.-
	//	m_neededxp = (m_level * 3500) + (m_level-- * 3500);

	//new cuz hardcode is best
	//Old is a bit too exponential
	//								//Collected xp
	//if (m_level == 0)
	//	m_neededxp = 5000;			
	//else if (m_level == 1)			//5 000
	//	m_neededxp = 10000;			
	//else if (m_level == 2)			//15 000
	//	m_neededxp = 20000;
	//else if (m_level == 3)			//35 000			Rainbow
	//	m_neededxp = 40000;
	//else if (m_level == 4)			//75 000
	//	m_neededxp = 50000;
	//else if (m_level == 5)			//125 000
	//	m_neededxp = 100000;
	//else if (m_level == 6)			//225 000
	//	m_neededxp = 100000;
	//else if (m_level == 7)			//325 000
	//	m_neededxp = 100000;
	//else if (m_level == 8)			//425 000			Bloody
	//	m_neededxp = 100000;
	//else if (m_level == 9)			//525 000			room_key
	//	m_neededxp = 200000;
	//else if (m_level == 10)			//625 000			
	//	m_neededxp = 300000;
	//else if (m_level == 11)			//925 000			Police[1]
	//	m_neededxp = 400000;
	//else if (m_level == 12)			//1 325 000
	//	m_neededxp = 500000;
	//else if (m_level == 13)			//1 825 000
	//	m_neededxp = 800000;
	//else if (m_level == 14)			//2 625 000
	//	m_neededxp = 1000000;
	//else if (m_level == 15)			//3 625 000			Police[2]			Taser
	//	m_neededxp = 2000000;
	//else if (m_level == 16)			//5 625 000
	//	m_neededxp = 3000000;
	//else if (m_level == 17)			//8 625 000
	//	m_neededxp = 4000000;
	//else if (m_level == 18)			//12 625 000
	//	m_neededxp = 5000000;
	//else if (m_level == 19)			//17 625 000
	//	m_neededxp = 10000000;
	//else if (m_level == 20)			//27 625 000
	//	m_neededxp = 10000000;
	//else if (m_level == 21)			//37 625 000
	//	m_neededxp = 10000000;
	//else if (m_level == 22)			//47 625 000
	//	m_neededxp = 10000000;
	//else if (m_level == 23)			//57 625 000
	//	m_neededxp = 10500000;
	//else if (m_level == 24)			//68 125 000
	//	m_neededxp = 15000000;
	//else if (m_level == 25)
	//	m_neededxp = 20000000;
	//else if (m_level == 26)
	//	m_neededxp = 20000000;
	//else if (m_level == 27)
	//	m_neededxp = 20000000;
	//else if (m_level == 28)
	//	m_neededxp = 20000000;
	//else if (m_level == 29)
	//	m_neededxp = 20000000;
	//else if (m_level == 30)
	//	m_neededxp = 20000000;
	//else if (m_level == 31)
	//	m_neededxp = 20000000;
	//else if (m_level == 32)
	//	m_neededxp = 20000000;
	//else if (m_level == 33)
	//	m_neededxp = 20000000;
	//else if (m_level == 34)
	//	m_neededxp = 20000000;
	//else if (m_level == 35)
	//	m_neededxp = 20000000;
	//else if (m_level == 36)
	//	m_neededxp = 20000000;
	//else if (m_level == 37)
	//	m_neededxp = 20000000;
	//else if (m_level == 38)
	//	m_neededxp = 20000000;
	//else if (m_level == 39)
	//	m_neededxp = 50000000;
	//else if (m_level == 40)
	//	m_neededxp = 50000000;
	//else if (m_level == 41)
	//	m_neededxp = 50000000;
	//else if (m_level == 42)
	//	m_neededxp = 50000000;
	//else if (m_level == 43)
	//	m_neededxp = 50000000;
	//else if (m_level == 44)
	//	m_neededxp = 50000000;
	//else if (m_level == 45)
	//	m_neededxp = 50000000;
	//else if (m_level == 46)
	//	m_neededxp = 50000000;
	//else if (m_level == 47)
	//	m_neededxp = 50000000;
	//else if (m_level == 48)
	//	m_neededxp = 50000000;
	//else if (m_level == 49)
	//	m_neededxp = 100000000;
	//else if (m_level == 50)
	//	m_neededxp = 100000000;

	//New less exponential

	//										xp diff
	if (m_level == 0)
		m_neededxp = 5000;
	else if (m_level == 1)					//5 000
		m_neededxp = 15000;
	else if (m_level == 2)					//10 000
		m_neededxp = 25000;
	else if (m_level == 3)					//10 000			
		m_neededxp = 35000;
	else if (m_level == 4)					//10 000
		m_neededxp = 50000;
	else if (m_level == 5)					//15 000			Rainbow
		m_neededxp = 65000;
	else if (m_level == 6)					//15 000
		m_neededxp = 80000;
	else if (m_level == 7)					//15 000
		m_neededxp = 100000;
	else if (m_level == 8)					//20 000			
		m_neededxp = 120000;
	else if (m_level == 9)					//20 000
		m_neededxp = 130000;
	else if (m_level == 10)					//30 000			
		m_neededxp = 160000;
	else if (m_level == 11)					//30 000			
		m_neededxp = 200000;
	else if (m_level == 12)					//40 000
		m_neededxp = 240000;
	else if (m_level == 13)					//40 000
		m_neededxp = 280000;
	else if (m_level == 14)					//40 000
		m_neededxp = 325000;
	else if (m_level == 15)					//45 000			Bloody
		m_neededxp = 370000;
	else if (m_level == 16)					//50 000			room_key
		m_neededxp = 420000;
	else if (m_level == 17)					//50 000
		m_neededxp = 470000;
	else if (m_level == 18)					//50 000			Police[1]
		m_neededxp = 520000;
	else if (m_level == 19)					//50 000
		m_neededxp = 600000;
	else if (m_level == 20)					//80 000
		m_neededxp = 680000;
	else if (m_level == 21)					//80 000			Ninja jetpack
		m_neededxp = 760000;
	else if (m_level == 22)					//90 000
		m_neededxp = 850000;
	else if (m_level == 23)					//100 000
		m_neededxp = 950000;
	else if (m_level == 24)					//150 000
		m_neededxp = 1200000;
	else if (m_level == 25)					//200 000			Police[2]		policehelper && jail codes
		m_neededxp = 1400000;
	else if (m_level == 26)					//200 000
		m_neededxp = 1600000;
	else if (m_level == 27)					//200 000
		m_neededxp = 1800000;
	else if (m_level == 28)					//200 000
		m_neededxp = 2000000;
	else if (m_level == 29)					//210 000
		m_neededxp = 2210000;
	else if (m_level == 30)					//220 000			Police[3]		taser
		m_neededxp = 2430000;
	else if (m_level == 31)					//230 000
		m_neededxp = 2660000;
	else if (m_level == 32)					//240 000
		m_neededxp = 2900000;
	else if (m_level == 33)					//250 000
		m_neededxp = 3150000;
	else if (m_level == 34)					//350 000                      
		m_neededxp = 3500000;
	else if (m_level == 35)					//450 000			
		m_neededxp = 3950000;
	else if (m_level == 36)					//550 000
		m_neededxp = 4500000;
	else if (m_level == 37)					//750 000
		m_neededxp = 5250000;
	else if (m_level == 38)					//850 000			spawn weapons
		m_neededxp = 6100000;
	else if (m_level == 39)					//900 000
		m_neededxp = 7000000;
	else if (m_level == 40)					//1 000 000			Police[4]		homing missels
		m_neededxp = 8000000;
	else if (m_level == 41)					//1 000 000
		m_neededxp = 9000000;
	else if (m_level == 42)					//1 000 000
		m_neededxp = 10000000;
	else if (m_level == 43)					//1 000 000
		m_neededxp = 11000000;
	else if (m_level == 44)					//1 000 000
		m_neededxp = 12000000;
	else if (m_level == 45)					//1 000 000
		m_neededxp = 13000000;
	else if (m_level == 46)					//1 000 000
		m_neededxp = 14000000;
	else if (m_level == 47)					//1 000 000
		m_neededxp = 15000000;
	else if (m_level == 48)					//1 000 000
		m_neededxp = 16000000;
	else if (m_level == 49)					//1 000 000
		m_neededxp = 17000000;
	else if (m_level == 50)					//1 000 000			Police[5]		'/jail arrest <time>' hammer command
		m_neededxp = 18000000;
	else if (m_level == 51)					//1 000 000
		m_neededxp = 19000000;
	else if (m_level == 52)					//1 000 000
		m_neededxp = 20000000;
	else if (m_level == 53)					//1 000 000
		m_neededxp = 21000000;
	else if (m_level == 54)					//1 000 000
		m_neededxp = 22000000;
	else if (m_level == 55)					//1 000 000
		m_neededxp = 23000000;
	else if (m_level == 56)					//1 000 000
		m_neededxp = 24000000;
	else if (m_level == 57)					//1 000 000
		m_neededxp = 25000000;
	else if (m_level == 58)					//1 000 000
		m_neededxp = 26000000;
	else if (m_level == 59)					//1 000 000
		m_neededxp = 27000000;
	else if (m_level == 60)					//1 000 000
		m_neededxp = 28000000;
	else if (m_level == 61)					//1 000 000
		m_neededxp = 29000000;
	else if (m_level == 62)					//1 000 000
		m_neededxp = 30000000;
	else if (m_level == 63)					//1 000 000
		m_neededxp = 31000000;
	else if (m_level == 64)					//1 000 000
		m_neededxp = 32000000;
	else if (m_level == 65)					//1 000 000
		m_neededxp = 33000000;
	else if (m_level == 66)					//1 000 000
		m_neededxp = 34000000;
	else if (m_level == 67)					//1 000 000
		m_neededxp = 35000000;
	else if (m_level == 68)					//1 000 000
		m_neededxp = 36000000;
	else if (m_level == 69)					//1 000 000
		m_neededxp = 37000000;
	else if (m_level == 70)					//1 000 000
		m_neededxp = 38000000;
	else if (m_level == 71)					//1 000 000
		m_neededxp = 39000000;
	else if (m_level == 72)					//1 000 000
		m_neededxp = 40000000;
	else if (m_level == 73)					//1 010 000
		m_neededxp = 41010000;
	else if (m_level == 74)					//1 010 000
		m_neededxp = 42020000;
	else if (m_level == 75)					//1 010 000
		m_neededxp = 43030000;
	else if (m_level == 76)					//1 010 000
		m_neededxp = 44040000;
	else if (m_level == 77)					//1 010 000
		m_neededxp = 45050000;
	else if (m_level == 78)					//1 010 000
		m_neededxp = 46060000;
	else if (m_level == 79)					//1 010 000
		m_neededxp = 47070000;
	else if (m_level == 80)					//1 010 000
		m_neededxp = 48080000;
	else if (m_level == 81)					//1 010 000
		m_neededxp = 49090000;
	else if (m_level == 82)					//1 010 000
		m_neededxp = 50100000;
	else if (m_level == 83)					//1 010 000
		m_neededxp = 51110000;
	else if (m_level == 84)					//1 010 000
		m_neededxp = 52120000;
	else if (m_level == 85)					//1 010 000
		m_neededxp = 53130000;
	else if (m_level == 86)					//1 010 000
		m_neededxp = 54140000;
	else if (m_level == 87)					//1 010 000
		m_neededxp = 55150000;
	else if (m_level == 88)					//1 010 000
		m_neededxp = 56160000;
	else if (m_level == 89)					//1 010 000
		m_neededxp = 57170000;
	else if (m_level == 90)					//1 010 000
		m_neededxp = 58180000;
	else if (m_level == 91)					//1 010 000
		m_neededxp = 59190000;
	else if (m_level == 92)					//1 010 000
		m_neededxp = 60200000;
	else if (m_level == 93)					//1 100 000
		m_neededxp = 61300000;
	else if (m_level == 94)					//1 100 000
		m_neededxp = 62400000;
	else if (m_level == 95)					//1 100 000
		m_neededxp = 63500000;
	else if (m_level == 96)					//1 100 000
		m_neededxp = 64600000;
	else if (m_level == 97)					//1 100 000
		m_neededxp = 65700000;
	else if (m_level == 98)					//1 100 000
		m_neededxp = 66800000;
	else if (m_level == 99)					//1 100 000
		m_neededxp = 67900000;
	else
		m_neededxp = 404000000000000;    //404 error         



		//WARNING!
		/*

		OLD!!!

		by increasing max level you need to change the hardcodet max level 99:
		you need to make some changes in the following places:

		player.ccp (CheckLevel())
		character.cpp(HasFlag)
		character.cpp(Moneytile)
		character.cpp(Moneytile2)
		character.cpp(Moneytileplus)
		character.cpp(Finish)
		character.cpp(void CCharacter::Die(int Killer, int Weapon))  (hammerfight)


		NEW!!!

		made it dynamic!
		there is a var called m_max_level
		update this var if u increase the level sys


		TODO: add a makro for max lvl

		*/

}

void CPlayer::CheckLevel()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (m_level > m_max_level)
		return;

	if (m_AccountID <= 0)
		return;

	if (m_neededxp <= 0)
		CalcExp();

	if (m_xp >= m_neededxp)
	{
		m_level++;

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "You are now Level %d!   +50money", m_level);
		GameServer()->SendChatTarget(m_ClientID, aBuf);  //woher weiss ich dass? mit dem GameServer()-> und m_Cli...
		MoneyTransaction(+50, "+50 level up");

		CalcExp();
	}
}


void CPlayer::MoneyTransaction(int Amount, const char *Description)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	m_money += Amount;
#if defined(CONF_DEBUG)
	if (m_money < 0)
	{
		dbg_msg("cBug", "WARNING money went negative! id=%d name=%s value=%d", GetCID(), Server()->ClientName(GetCID()), m_money);
	}
#endif
	str_format(m_money_transaction9, sizeof(m_money_transaction9), "%s", m_money_transaction9);
	str_format(m_money_transaction8, sizeof(m_money_transaction8), "%s", m_money_transaction8);
	str_format(m_money_transaction7, sizeof(m_money_transaction7), "%s", m_money_transaction7);
	str_format(m_money_transaction6, sizeof(m_money_transaction6), "%s", m_money_transaction5);
	str_format(m_money_transaction5, sizeof(m_money_transaction5), "%s", m_money_transaction4);
	str_format(m_money_transaction4, sizeof(m_money_transaction4), "%s", m_money_transaction3);
	str_format(m_money_transaction3, sizeof(m_money_transaction3), "%s", m_money_transaction2);
	str_format(m_money_transaction2, sizeof(m_money_transaction2), "%s", m_money_transaction1);
	str_format(m_money_transaction1, sizeof(m_money_transaction1), "%s", m_money_transaction0);
	str_format(m_money_transaction0, sizeof(m_money_transaction0), Description);
}

bool CPlayer::IsInstagibMinigame()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (m_IsInstaArena_gdm || m_IsInstaArena_idm || m_IsInstaArena_fng)
		return true;
	return false;
}

//void CPlayer::ThreadLoginStart(CGameContext * pGameContext, CQueryLogin * pSQL) //starts the thread gets called on login
void CPlayer::ThreadLoginStart(/*CGameContext * pGameContext, */void * pSQL)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	dbg_msg("cBug", "login0");
	m_pLoginData = (struct CLoginData*)malloc(sizeof(struct CLoginData));
	dbg_msg("cBug", "login1");
	//m_pLoginData->m_pGameContext = pGameContext;
	dbg_msg("cBug", "login2");
	m_pLoginData->m_pTmpPlayer = this; //new CPlayer(GameServer(), GetCID(), m_Team);
	dbg_msg("cBug", "login3");
	m_pLoginData->m_pSQL = pSQL;
	dbg_msg("cBug", "login4");
	m_pLoginData->m_Done = false;
	dbg_msg("cBug", "login5");
	m_pLoginData->m_Lock = lock_create();
	void *pt = thread_init(*ThreadLoginWorker, m_pLoginData); //setzte die werte von pTmpPlayer
	dbg_msg("cBug", "login6");

	m_pLoginData->m_Done = true; //the thread result gets catched in ThreadLoginDone function called everytick by checking this var
	dbg_msg("cBug", "loginDONE");
}

void CPlayer::ThreadLoginWorker(void * pArg) //is the actual thread
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	dbg_msg("cBug", "worker0");
	struct CLoginData *pData = static_cast<struct CLoginData*>(pArg);
	dbg_msg("cBug", "worker1");
	//CGameContext *pGS = static_cast<CGameContext*>(pData->m_pGameContext);
	dbg_msg("cBug", "worker2");
	CQueryLogin *pSQL = static_cast<CQueryLogin*>(pData->m_pSQL);
	dbg_msg("cBug", "worker3");
	CPlayer *pPlayer = static_cast<CPlayer*>(pData->m_pTmpPlayer);
	dbg_msg("cBug", "worker4");
	dbg_msg("cBug", "worker5");
	//str_format(aBuf, sizeof(aBuf), "[THREAD] hello world4 your id=%d should be id=%d", pPlayer->GetCID(), /*GetCID() //doesnt work cuz static*/ 404);
	dbg_msg("cBug", "worker6");
	//pGS->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
	dbg_msg("cBug", "worker7");
	pPlayer->m_money = 420;
	dbg_msg("cBug", "workerDONE");
}

void CPlayer::ThreadLoginDone() //get called every tick
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (!m_pLoginData)
		return;

	dbg_msg("cBug", "done0");
	lock_wait(m_pLoginData->m_Lock);
	dbg_msg("cBug", "done1");
	if (!m_pLoginData->m_Done)
		return;

	dbg_msg("cBug", "done2");
	char aBuf[128];
	dbg_msg("cBug", "done3");
	str_format(aBuf, sizeof(aBuf), "[THREAD] login done");
	dbg_msg("cBug", "done4");
	GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
	delete m_pLoginData->m_pTmpPlayer;
	dbg_msg("cBug", "done5");
	lock_unlock(m_pLoginData->m_Lock);
	dbg_msg("cBug", "done6");

	lock_destroy(m_pLoginData->m_Lock);
	free(m_pLoginData);
	dbg_msg("cBug", "doneDONE");
}

void CPlayer::chidraqul3_GameTick()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	//if (m_C3_GameState == 2) //multiplayer
	//	return; //handled in gamecontext


	if (g_Config.m_SvAllowChidraqul == 0)
	{
		GameServer()->SendChatTarget(m_ClientID, "Admin has disabled chidraqul3.");
		m_C3_GameState = false;
	}
	else if (g_Config.m_SvAllowChidraqul == 1) //dynamic but resourcy way (doesnt work on linux)
	{
		char aBuf[512];

		char m_minigame_world[512];
		str_format(m_minigame_world, sizeof(m_minigame_world), "");




		//spawn gold
		if (!m_GoldAlive)
		{
			m_GoldPos = -1;
			if (m_GoldRespawnDelay <= 0)
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
		if (m_GoldPos == m_HashPos && m_HashPosY == 0)
		{
			m_HashGold++;
			m_GoldAlive = false;
		}





		//create world chararray
		//y: 3
		//y: 2
		//y: 1
		for (int i = 0; i < m_Minigameworld_size_x; i++)
		{
			char create_world[126];
			if (i == m_HashPos && m_HashPosY == 1)
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
		for (int i = 0; i < m_Minigameworld_size_x; i++)
		{
			char create_world[126];
			if (i == m_HashPos && m_HashPosY == 0)
			{
				str_format(create_world, sizeof(create_world), "%s", m_HashSkin);
			}
			else if (i == m_GoldPos)
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
	else if (g_Config.m_SvAllowChidraqul == 2) //old hardcodet 
	{
		char aBuf[512];


		if (m_HashPos == 0)
		{
			str_format(aBuf, sizeof(aBuf), "%s___________", m_HashSkin);
			GameServer()->SendBroadcast(aBuf, m_ClientID);
		}
		else if (m_HashPos == 1)
		{
			str_format(aBuf, sizeof(aBuf), "_%s__________", m_HashSkin);
			GameServer()->SendBroadcast(aBuf, m_ClientID);
		}
		else if (m_HashPos == 2)
		{
			str_format(aBuf, sizeof(aBuf), "__%s_________", m_HashSkin);
			GameServer()->SendBroadcast(aBuf, m_ClientID);
		}
		else if (m_HashPos == 3)
		{
			str_format(aBuf, sizeof(aBuf), "___%s________", m_HashSkin);
			GameServer()->SendBroadcast(aBuf, m_ClientID);
		}
		else if (m_HashPos == 3)
		{
			str_format(aBuf, sizeof(aBuf), "____%s_______", m_HashSkin);
			GameServer()->SendBroadcast(aBuf, m_ClientID);
		}
		else if (m_HashPos == 4)
		{
			str_format(aBuf, sizeof(aBuf), "_____%s______", m_HashSkin);
			GameServer()->SendBroadcast(aBuf, m_ClientID);
		}
		else if (m_HashPos == 5)
		{
			str_format(aBuf, sizeof(aBuf), "______%s_____", m_HashSkin);
			GameServer()->SendBroadcast(aBuf, m_ClientID);
		}
		else if (m_HashPos == 6)
		{
			str_format(aBuf, sizeof(aBuf), "_______%s____", m_HashSkin);
			GameServer()->SendBroadcast(aBuf, m_ClientID);
		}
		else if (m_HashPos == 7)
		{
			str_format(aBuf, sizeof(aBuf), "________%s___", m_HashSkin);
			GameServer()->SendBroadcast(aBuf, m_ClientID);
		}
		else if (m_HashPos == 8)
		{
			str_format(aBuf, sizeof(aBuf), "_________%s__", m_HashSkin);
			GameServer()->SendBroadcast(aBuf, m_ClientID);
		}
		else if (m_HashPos == 9)
		{
			str_format(aBuf, sizeof(aBuf), "__________%s_", m_HashSkin);
			GameServer()->SendBroadcast(aBuf, m_ClientID);
		}
		else if (m_HashPos == 10)
		{
			str_format(aBuf, sizeof(aBuf), "___________%s", m_HashSkin);
			GameServer()->SendBroadcast(aBuf, m_ClientID);
		}
	}
	else if (g_Config.m_SvAllowChidraqul == 3) //next generation
	{
		if (m_C3_UpdateFrame)
		{
			m_C3_UpdateFrame = false;
			char aBuf[128];
			char aHUD[64];
			char aWorld[64]; //max world size

			for (int i = 0; i < g_Config.m_SvChidraqulWorldX; i++)
			{
				aWorld[i] = '_';
			}

			aWorld[m_HashPos] = m_HashSkin[0];
			aWorld[g_Config.m_SvChidraqulWorldX] = '\0';

			str_format(aHUD, sizeof(aHUD), "\n\nPos: %d", m_HashPos);
			str_format(aBuf, sizeof(aBuf), "%s%s", aWorld, aHUD);

			GameServer()->SendBroadcast(aWorld, m_ClientID, 0);
		}
		if (Server()->Tick() % 120 == 0)
		{
			m_C3_UpdateFrame = true;
		}
	}
}

bool CPlayer::JoinMultiplayer()
{
	if (GameServer()->C3_GetFreeSlots() > 0)
	{
		GameServer()->SendChatTarget(GetCID(), "[chidraqul] joined multiplayer.");
		m_C3_UpdateFrame = true;
		m_C3_GameState = 2;
		return true;
	}
	GameServer()->SendChatTarget(GetCID(), "[chidraqul] multiplayer is full.");
	return false;
}

