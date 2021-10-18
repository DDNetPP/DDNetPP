/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>
#include <base/system.h>

#include <engine/config.h>
#include <engine/console.h>
#include <engine/engine.h>
#include <engine/map.h>
#include <engine/masterserver.h>
#include <engine/server.h>
#include <engine/storage.h>

#include <engine/shared/compression.h>
#include <engine/shared/config.h>
#include <engine/shared/datafile.h>
#include <engine/shared/demo.h>
#include <engine/shared/econ.h>
#include <engine/shared/filecollection.h>
#include <engine/shared/mapchecker.h>
#include <engine/shared/netban.h>
#include <engine/shared/network.h>
#include <engine/shared/packer.h>
#include <engine/shared/protocol.h>
#include <engine/shared/protocol_ex.h>
#include <engine/shared/snapshot.h>
#include <engine/shared/fifo.h>

#include <mastersrv/mastersrv.h>

// DDRace
#include <string.h>
#include <vector>
#include <engine/shared/linereader.h>
#include <game/server/gamecontext.h>
#include <base/ddpp_logs.h>

#include "register.h"
#include "server.h"

#if defined(CONF_FAMILY_WINDOWS)
	#define _WIN32_WINNT 0x0501
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif

// DDPP ( ChillerDragon )
#include <string>
#include <fstream>

void CServer::BotJoin(int BotID)
{
	const char *pNames[] = { //Array für die Namen
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
	const char *pClans[] = { //Array für die Clans
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
		"64"
	};

	m_NetServer.BotInit(BotID);
	m_aClients[BotID].m_State = CClient::STATE_BOT;
	m_aClients[BotID].m_Authed = AUTHED_NO;

	str_copy(m_aClients[BotID].m_aName, pNames[BotID], MAX_NAME_LENGTH); //Namen des Jeweiligen Dummys setzten
	str_copy(m_aClients[BotID].m_aClan, pClans[BotID], MAX_CLAN_LENGTH); //Clan des jeweiligen Dummys setzten
}

void CServer::BotLeave(int BotID, bool silent)
{
	GameServer()->OnClientDrop(BotID, "", silent);

	m_aClients[BotID].m_State = CClient::STATE_EMPTY;
	m_aClients[BotID].m_aName[0] = 0;
	m_aClients[BotID].m_aClan[0] = 0;
	m_aClients[BotID].m_Country = -1;
	m_aClients[BotID].m_Authed = AUTHED_NO;
	m_aClients[BotID].m_AuthTries = 0;
	m_aClients[BotID].m_pRconCmdToSend = 0;
	m_aClients[BotID].m_Snapshots.PurgeAll();

	m_NetServer.BotDelete(BotID);
}

void CServer::ConStartBlockTourna(IConsole::IResult * pResult, void * pUser)
{
	//((CServer *)pUser)->m_pGameServer->SendBroadcastAll("hacked the world");
	//((CServer *)pUser)->GameServer()->OnClientDrop(2, "", false);
	((CServer *)pUser)->GameServer()->OnStartBlockTournament();
}

//void CServer::ConDDPPshutdown(IConsole::IResult * pResult, void * pUser)
//{
//#if defined(CONF_DEBUG)
//#endif
//	((CServer *)pUser)->GameServer()->OnDDPPshutdown();
//}