/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>
#include <base/system.h>

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
		DDPPDbPool()->RegisterSqliteDatabase(CDbConnectionPool::READ, g_Config.m_SvDatabasePath);
		DDPPDbPool()->RegisterSqliteDatabase(CDbConnectionPool::WRITE, g_Config.m_SvDatabasePath);
	}
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

void CServer::ConStartBlockTourna(IConsole::IResult *pResult, void *pUser)
{
	//((CServer *)pUser)->m_pGameServer->SendBroadcastAll("hacked the world");
	//((CServer *)pUser)->GameServer()->OnClientDrop(2, "", false);
	((CServer *)pUser)->GameServer()->OnStartBlockTournament();
}

int CServer::LoadMapLive(const char *pMapName)
{
	char aBuf[IO_MAX_PATH_LENGTH];
	str_format(aBuf, sizeof(aBuf), "maps/%s.map", pMapName);
	GameServer()->OnMapChange(aBuf, sizeof(aBuf));

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
