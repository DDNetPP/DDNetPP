/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>
#include <base/system.h>

#ifdef CONF_PLATFORM_LINUX
#include <cerrno> // Storage()->OpenFile() fail details (it is a fopen wrapper on linux)
#endif

#include <engine/config.h>
#include <engine/console.h>
#include <engine/engine.h>
#include <engine/map.h>
#include <engine/server.h>
#include <engine/storage.h>

#include <engine/shared/compression.h>
#include <engine/shared/config.h>
#include <engine/shared/datafile.h>
#include <engine/shared/demo.h>
#include <engine/shared/econ.h>
#include <engine/shared/fifo.h>
#include <engine/shared/filecollection.h>
#include <engine/shared/netban.h>
#include <engine/shared/network.h>
#include <engine/shared/packer.h>
#include <engine/shared/protocol.h>
#include <engine/shared/protocol_ex.h>
#include <engine/shared/snapshot.h>

// DDRace
#include <base/ddpp_logs.h>
#include <cstring>
#include <engine/shared/linereader.h>
#include <game/server/gamecontext.h>
#include <zlib.h>

#include "register.h"
#include "server.h"

#if defined(CONF_FAMILY_WINDOWS)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include "databases/connection.h"
#include "databases/connection_pool.h"

void CServer::DDPPRegisterDatabases()
{
	if(g_Config.m_SvDatabasePath[0] != '\0')
	{
		if(Config()->m_SvUseMysqlForAccounts)
		{
			DDPPDbPool()->RegisterSqliteDatabase(CDbConnectionPool::WRITE_BACKUP, g_Config.m_SvDatabasePath);
		}
		else
		{
			DDPPDbPool()->RegisterSqliteDatabase(CDbConnectionPool::READ, g_Config.m_SvDatabasePath);
			DDPPDbPool()->RegisterSqliteDatabase(CDbConnectionPool::WRITE, g_Config.m_SvDatabasePath);
		}
	}
}

IEngineMap *CServer::Map()
{
	return m_pMap;
}

void CServer::OnFailedRconLoginAttempt(int ClientId, const char *pName, const char *pPassword)
{
	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "wrong rcon attempt by ClientId=%d", ClientId);
	ddpp_log(DDPP_LOG_WRONG_RCON, aBuf);
	WriteWrongRconJson(ClientId, pName, pPassword);
}

void CServer::WriteWrongRconJson(int ClientId, const char *pName, const char *pPassword)
{
	if(!g_Config.m_SvSaveWrongRcon)
	{
		return;
	}

	CJsonStringWriter JsonWriter;

	JsonWriter.BeginObject();
	JsonWriter.WriteAttribute("name");
	JsonWriter.WriteStrValue(ClientName(ClientId));

	JsonWriter.WriteAttribute("username");
	JsonWriter.WriteStrValue(pName);

	JsonWriter.WriteAttribute("password");
	JsonWriter.WriteStrValue(pPassword);

	JsonWriter.EndObject();

	const char *pJson = JsonWriter.GetOutputString().c_str();

	IOHANDLE pFile = Storage()->OpenFile(g_Config.m_SvWrongRconFile, IOFLAG_APPEND, IStorage::TYPE_SAVE);
	if(!pFile)
	{
		dbg_msg("ddnet++", "failed to open %s", g_Config.m_SvWrongRconFile);
#ifdef CONF_PLATFORM_LINUX
		dbg_msg("ddnet++", "  errno=%d (%s)", errno, strerror(errno));
#endif
		return;
	}

	// without newline
	io_write(pFile, pJson, str_length(pJson) - 1);
	io_write(pFile, ",", 1);
	io_write_newline(pFile);
	io_close(pFile);
}

void CServer::BotJoin(int BotId)
{
	const char *pNames[] = {//Array für die Namen
		"flappy.*",
		"Chillingo.*",
		"Fluffy.*",
		"MLG_PRO.*",
		"Enzym.*",
		"ZillyDreck.*",
		"ciliDR[HUN].*",
		"fuzzle.*",
		"Piko.*",
		"chilliger.*",
		"ChilligerDrago",
		"GubbaFubba",
		"fuZZle.*",
		"<bot>",
		"<noob>",
		"<police>",
		"<train>",
		"<boat>",
		"<blocker>",
		"<racer>",
		"<hyper>",
		"$heeP",
		"b3ep",
		"chilluminatee.*",
		"auftragschiller",
		"abcJuhee",
		"BANANA.*",
		"POTATO.*",
		"<cucumber>",
		"<rape>",
		"<_BoT__>",
		"NotMyName",
		"NotChiller",
		"NotChiIIer",
		"NotChlIer",
		"fuckmesoon.*",
		"DataNub",
		"149.202.127.134",
		"<hacker>",
		"<cheater>",
		"<glitcher>",
		"__ERROR",
		"404_kein_tier",
		"ZitrusFRUCHT",
		"BAUMKIND",
		"KELLERKIND",
		"KINDERKIND",
		"einZug-",
		"<bob>",
		"BezzyHill",
		"BeckySkill",
		"Skilli.*",
		"UltraVa.",
		"DONATE!",
		"SUBSCRIBE!",
		"SHARE!",
		"#like",
		"<#name_>",
		"KRISTIAN-.",
		".,-,08/524",
		"3113pimml34",
		"NotAB0t",
		"Hurman",
		"xxlddnnet64",
		"flappy.*", //64igster name
		"steeeve",
		"naki.*",
		"tuba.*",
		"higge.*",
		"linux_uzer3k",
		"hubbat.*",
		"Proviet-",
		"7h89",
		"1276.*",
		"SchinKKKen",
		"FOSSIELamKIEL",
		"apfelFUZ",
		"cron_tabur",
		"hinter_c_dur",
		"equariator",
		"deckztinator",
		"intezinatoha",
		"defquirlibaor",
		"enmuhinakur",
		"wooknazitur",
		"demnatura",
		"intranuza",
		"eggspikuza",
		"finaluba",
		"denkrikator",
		"nihilatur",
		"Goethe[HUN]",
		"RightIsRight.*",
		"Egg_user_xd",
		"avucadur",
		"NoeeoN",
		"wuuuzzZZZa",
		"JezzicaP",
		"Jeqqicaqua",
		"analyticus",
		"haspiclecane",
		"nameus",
		"tahdequz",
		"rostBEULEH",
		"regenwurm674",
		"mc_cm",
		"ddpp",
		"DDNet++",
		"pidgin.,a",
		"bibubablbl",
		"randomNAME2",
		"Mircaduzla",
		"zer0_brain",
		"haxxor-420",
		"fok-me-fok",
		"fok-fee-san",
		"denzulat",
		"epsilat",
		"destructat",
		"hinzuckat",
		"penZilin",
		"deszilin",
		"VogelFisch7",
		"Dont4sk",
		"i_fokmen_i",
		"noobScout24",
		"geneticual",
		"fokkoNUT"

	};
	const char *pClans[] = {//Array für die Clans
		"Chilli.*",
		"Chilli.*",
		"Chilli.*",
		"Chilli.*",
		"Chilli.*",
		"Chilli.*",
		"Chilli.*",
		"Chilli.*",
		"Chilli.*",
		"Chilli.*",
		"Chilli.*",
		"Chilli.*",
		"Chilli.*",
		"Chilli.*",
		"Chilli.*",
		"Chilli.*",
		"Chilli.*",
		"Chilli.*",
		"Chilli.*",
		"Chilli.*",
		"21",
		"22",
		"23",
		"24",
		"25",
		"26",
		"27",
		"28",
		"29",
		"30",
		"31",
		"32",
		"33",
		"34",
		"35",
		"36",
		"37",
		"38",
		"39",
		"40",
		"41",
		"42",
		"43",
		"44",
		"45",
		"46",
		"47",
		"48",
		"49",
		"50",
		"51",
		"52",
		"53",
		"54",
		"55",
		"56",
		"57",
		"58",
		"59",
		"60",
		"61",
		"62",
		"63",
		"64"};

	m_aClients[BotId].m_State = CClient::STATE_PREAUTH;
	m_aClients[BotId].m_DnsblState = CClient::DNSBL_STATE_NONE;
	m_aClients[BotId].m_aName[0] = 0;
	m_aClients[BotId].m_aClan[0] = 0;
	m_aClients[BotId].m_Country = -1;
	m_aClients[BotId].m_Authed = AUTHED_NO;
	m_aClients[BotId].m_AuthKey = -1;
	m_aClients[BotId].m_AuthTries = 0;
	m_aClients[BotId].m_pRconCmdToSend = 0;
	m_aClients[BotId].m_Traffic = 0;
	m_aClients[BotId].m_TrafficSince = 0;
	m_aClients[BotId].m_ShowIps = false;
	m_aClients[BotId].m_DDNetVersion = VERSION_NONE;
	m_aClients[BotId].m_GotDDNetVersionPacket = false;
	m_aClients[BotId].m_DDNetVersionSettled = false;
	memset(&m_aClients[BotId].m_Addr, 0, sizeof(NETADDR));
	m_aClients[BotId].Reset();
	m_aClients[BotId].m_Sixup = false;

	m_NetServer.BotInit(BotId);
	m_aClients[BotId].m_State = CClient::STATE_BOT;

	str_copy(m_aClients[BotId].m_aName, pNames[BotId], MAX_NAME_LENGTH); //Namen des Jeweiligen Dummys setzten
	str_copy(m_aClients[BotId].m_aClan, pClans[BotId], MAX_CLAN_LENGTH); //Clan des jeweiligen Dummys setzten
}

void CServer::BotLeave(int BotId, bool Silent)
{
	GameServer()->OnClientDrop(BotId, "", Silent);

	m_aClients[BotId].m_State = CClient::STATE_EMPTY;
	m_aClients[BotId].m_aName[0] = 0;
	m_aClients[BotId].m_aClan[0] = 0;
	m_aClients[BotId].m_Country = -1;
	m_aClients[BotId].m_Authed = AUTHED_NO;
	m_aClients[BotId].m_AuthTries = 0;
	m_aClients[BotId].m_pRconCmdToSend = 0;
	m_aClients[BotId].m_Snapshots.PurgeAll();

	m_NetServer.BotDelete(BotId);
}

void CServer::ConRedirect(IConsole::IResult *pResult, void *pUser)
{
	CServer *pThis = (CServer *)pUser;
	char aBuf[512];

	int VictimId = pResult->GetVictim();
	int Port = pResult->GetInteger(1);

	if(VictimId == pResult->m_ClientId)
	{
		pThis->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "ddnet++", "You can not redirect your self");
		return;
	}

	if(VictimId < 0 || VictimId >= MAX_CLIENTS)
	{
		str_format(aBuf, sizeof(aBuf), "Invalid ClientId %d", VictimId);
		pThis->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "ddnet++", aBuf);
		return;
	}
	if(!pThis->ClientIngame(VictimId))
	{
		return;
	}
	pThis->RedirectClient(VictimId, Port);
}

void CServer::ConStartBlockTourna(IConsole::IResult *pResult, void *pUser)
{
	//((CServer *)pUser)->m_pGameServer->SendBroadcastAll("hacked the world");
	//((CServer *)pUser)->GameServer()->OnClientDrop(2, "", false);
	((CServer *)pUser)->GameServer()->OnStartBlockTournament();
}

void CServer::ConAddAccountsSqlServer(IConsole::IResult *pResult, void *pUserData)
{
	CServer *pSelf = (CServer *)pUserData;

	if(!MysqlAvailable())
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "ddnet++", "can't add MySQL server: compiled without MySQL support");
		return;
	}

	if(!pSelf->Config()->m_SvUseMysqlForAccounts)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "ddnet++", "ignoring MySQL server because sv_accounts_mysql is 0");
		return;
	}

	if(pResult->NumArguments() != 7 && pResult->NumArguments() != 8)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "ddnet++", "7 or 8 arguments are required");
		return;
	}

	CMysqlConfig Config;
	bool Write;
	if(str_comp_nocase(pResult->GetString(0), "w") == 0)
		Write = false;
	else if(str_comp_nocase(pResult->GetString(0), "r") == 0)
		Write = true;
	else
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "ddnet++", "choose either 'r' for SqlReadServer or 'w' for SqlWriteServer");
		return;
	}

	str_copy(Config.m_aDatabase, pResult->GetString(1), sizeof(Config.m_aDatabase));
	str_copy(Config.m_aPrefix, pResult->GetString(2), sizeof(Config.m_aPrefix));
	str_copy(Config.m_aUser, pResult->GetString(3), sizeof(Config.m_aUser));
	str_copy(Config.m_aPass, pResult->GetString(4), sizeof(Config.m_aPass));
	str_copy(Config.m_aIp, pResult->GetString(5), sizeof(Config.m_aIp));
	Config.m_aBindaddr[0] = '\0';
	Config.m_Port = pResult->GetInteger(6);
	Config.m_Setup = pResult->NumArguments() == 8 ? pResult->GetInteger(7) : true;

	char aBuf[512];
	str_format(aBuf, sizeof(aBuf),
		"Adding new Sql%sServer: DB: '%s' Prefix: '%s' User: '%s' IP: <{%s}> Port: %d",
		Write ? "Write" : "Read",
		Config.m_aDatabase, Config.m_aPrefix, Config.m_aUser, Config.m_aIp, Config.m_Port);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "ddnet++", aBuf);
	pSelf->DDPPDbPool()->RegisterMysqlDatabase(Write ? CDbConnectionPool::WRITE : CDbConnectionPool::READ, &Config);
}

int CServer::LoadMapLive(const char *pMapName)
{
	char aBuf[IO_MAX_PATH_LENGTH];
	str_format(aBuf, sizeof(aBuf), "maps/%s.map", pMapName);
	if(!GameServer()->OnMapChange(aBuf, sizeof(aBuf)))
		return 0;

	if(!m_pMap->Load(aBuf))
		return 0;

	// reinit snapshot ids
	m_IdPool.TimeoutIds();

	// get the crc of the map
	m_aCurrentMapSha256[MAP_TYPE_SIX] = m_pMap->Sha256();
	m_aCurrentMapCrc[MAP_TYPE_SIX] = m_pMap->Crc();
	char aBufMsg[256];
	char aSha256[SHA256_MAXSTRSIZE];
	sha256_str(m_aCurrentMapSha256[MAP_TYPE_SIX], aSha256, sizeof(aSha256));
	str_format(aBufMsg, sizeof(aBufMsg), "%s sha256 is %s", aBuf, aSha256);
	Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "server", aBufMsg);

	str_copy(m_aCurrentMap, pMapName, sizeof(m_aCurrentMap));

	// load complete map into memory for download
	{
		IOHANDLE File = Storage()->OpenFile(aBuf, IOFLAG_READ, IStorage::TYPE_ALL);
		m_aCurrentMapSize[MAP_TYPE_SIX] = (unsigned int)io_length(File);
		free(m_apCurrentMapData[MAP_TYPE_SIX]);
		m_apCurrentMapData[MAP_TYPE_SIX] = (unsigned char *)malloc(m_aCurrentMapSize[MAP_TYPE_SIX]);
		io_read(File, m_apCurrentMapData[MAP_TYPE_SIX], m_aCurrentMapSize[MAP_TYPE_SIX]);
		io_close(File);
	}

	// load sixup version of the map
	if(Config()->m_SvSixup)
	{
		str_format(aBuf, sizeof(aBuf), "maps7/%s.map", pMapName);
		IOHANDLE File = Storage()->OpenFile(aBuf, IOFLAG_READ, IStorage::TYPE_ALL);
		if(!File)
		{
			Config()->m_SvSixup = 0;
			str_format(aBufMsg, sizeof(aBufMsg), "couldn't load map %s", aBuf);
			Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "sixup", aBufMsg);
			Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "sixup", "disabling 0.7 compatibility");
		}
		else
		{
			m_aCurrentMapSize[MAP_TYPE_SIXUP] = (unsigned int)io_length(File);
			free(m_apCurrentMapData[MAP_TYPE_SIXUP]);
			m_apCurrentMapData[MAP_TYPE_SIXUP] = (unsigned char *)malloc(m_aCurrentMapSize[MAP_TYPE_SIXUP]);
			io_read(File, m_apCurrentMapData[MAP_TYPE_SIXUP], m_aCurrentMapSize[MAP_TYPE_SIXUP]);
			io_close(File);

			m_aCurrentMapSha256[MAP_TYPE_SIXUP] = sha256(m_apCurrentMapData[MAP_TYPE_SIXUP], m_aCurrentMapSize[MAP_TYPE_SIXUP]);
			m_aCurrentMapCrc[MAP_TYPE_SIXUP] = crc32(0, m_apCurrentMapData[MAP_TYPE_SIXUP], m_aCurrentMapSize[MAP_TYPE_SIXUP]);
			sha256_str(m_aCurrentMapSha256[MAP_TYPE_SIXUP], aSha256, sizeof(aSha256));
			str_format(aBufMsg, sizeof(aBufMsg), "%s sha256 is %s", aBuf, aSha256);
			Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "sixup", aBufMsg);
		}
	}
	if(!Config()->m_SvSixup && m_apCurrentMapData[MAP_TYPE_SIXUP])
	{
		free(m_apCurrentMapData[MAP_TYPE_SIXUP]);
		m_apCurrentMapData[MAP_TYPE_SIXUP] = 0;
	}

	for(int i = 0; i < MAX_CLIENTS; i++)
		m_aPrevStates[i] = m_aClients[i].m_State;

	return 1;
}

//void CServer::ConDDPPshutdown(IConsole::IResult * pResult, void * pUser)
//{
//#if defined(CONF_DEBUG)
//#endif
//	((CServer *)pUser)->GameServer()->OnDDPPshutdown();
//}
