/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/tl/sorted_array.h>

#include <new>
#include <base/math.h>
#include <engine/shared/config.h>
#include <engine/map.h>
#include <engine/console.h>
#include <engine/shared/datafile.h>
#include <engine/shared/linereader.h>
#include <engine/storage.h>
#include "gamecontext.h"
#include <game/version.h>
#include <game/collision.h>
#include <game/gamecore.h>
#include <game/server/entities/flag.h>
/*#include "gamemodes/dm.h"
#include "gamemodes/tdm.h"
#include "gamemodes/ctf.h"
#include "gamemodes/mod.h"*/

#include <stdio.h>
#include <string.h>
#include <engine/server/server.h>
#include "gamemodes/DDRace.h"
#include "score.h"
#include "score/file_score.h"
#include <time.h>
#if defined(CONF_SQL)
#include "score/sql_score.h"
#endif

enum
{
	RESET,
	NO_RESET
};

void CQueryRegister::OnData()
{
	if (Next())
	{
		m_pGameServer->SendChatTarget(m_ClientID, "Username already exists.");
	}
	else
	{
		char *pQueryBuf = sqlite3_mprintf("INSERT INTO Accounts (Username, Password) VALUES ('%q', '%q');", m_Name.c_str(), m_Password.c_str());

		CQuery *pQuery = new CQuery();
		pQuery->Query(m_pDatabase, pQueryBuf);
		sqlite3_free(pQueryBuf);

		m_pGameServer->SendChatTarget(m_ClientID, "Account registered successfully.");
		m_pGameServer->SendChatTarget(m_ClientID, "Login with: /login (name) (pass)");
	}
}

void CQueryLogin::OnData()
{
	if (Next())
	{
		if (m_pGameServer->CheckAccounts(GetInt(GetID("ID"))))
		{
			m_pGameServer->SendChatTarget(m_ClientID, "This account is already logged in.");
		}
		else
		{
			if (m_pGameServer->m_apPlayers[m_ClientID])
			{
				m_pGameServer->m_apPlayers[m_ClientID]->m_AccountID = GetInt(GetID("ID"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_level = GetInt(GetID("Level"));
				//m_pGameServer->m_apPlayers[m_ClientID]->m_neededxp = GetInt(GetID("Neededxp"));
				//m_pGameServer->m_apPlayers[m_ClientID]->m_plusxp = GetInt(GetID("Plusxp"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_xp = GetInt(GetID("Exp"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_money = GetInt(GetID("Money"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_shit = GetInt(GetID("Shit"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_LastGift = GetInt(GetID("LastGift"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_PoliceRank = GetInt(GetID("PoliceRank"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_JailTime = GetInt(GetID("JailTime"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_EscapeTime = GetInt(GetID("EscapeTime"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_TaserLevel = GetInt(GetID("TaserLevel")); 
				m_pGameServer->m_apPlayers[m_ClientID]->m_pvp_arena_tickets = GetInt(GetID("PvPArenaTickets"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_pvp_arena_games_played = GetInt(GetID("PvPArenaGames"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_pvp_arena_kills = GetInt(GetID("PvPArenaKills"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_pvp_arena_deaths = GetInt(GetID("PvPArenaDeaths"));

				//profiles
				m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileStyle = GetInt(GetID("ProfileStyle"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileViews = GetInt(GetID("ProfileViews"));

				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileStatus, GetText(GetID("ProfileStatus")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileStatus));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileSkype, GetText(GetID("ProfileSkype")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileSkype));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileYoutube, GetText(GetID("ProfileYoutube")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileYoutube));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileEmail, GetText(GetID("ProfileEmail")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileEmail));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileHomepage, GetText(GetID("ProfileHomepage")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileHomepage));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileTwitter, GetText(GetID("ProfileTwitter")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileTwitter));

				m_pGameServer->m_apPlayers[m_ClientID]->m_homing_missiles_ammo = GetInt(GetID("HomingMissiles"));

				/*str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileStatus, GetText(GetID("ProfileStatus")), sizeof(CPlayer::m_ProfileStatus));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileSkype, GetText(GetID("ProfileSkype")), sizeof(CPlayer::m_ProfileSkype));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileYoutube, GetText(GetID("ProfileYoutube")), sizeof(CPlayer::m_ProfileYoutube));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileEmail, GetText(GetID("ProfileEmail")), sizeof(CPlayer::m_ProfileEmail));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileHomepage, GetText(GetID("ProfileHomepage")), sizeof(CPlayer::m_ProfileHomepage));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileTwitter, GetText(GetID("ProfileTwitter")), sizeof(CPlayer::m_ProfileTwitter));*/
			}

			//m_pGameServer->SendChatTarget(m_ClientID, "Successfully logged in you son of a bitch.");
			m_pGameServer->SendChatTarget(m_ClientID, "Successfully logged in :)");
		}
	}
	else
		m_pGameServer->SendChatTarget(m_ClientID, "Login failed");
}

bool CGameContext::CheckAccounts(int AccountID)
{
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (!m_apPlayers[i])
			continue;

		if (m_apPlayers[i]->m_AccountID == AccountID)
			return true;
	}
	return false;
}

void CGameContext::Construct(int Resetting)
{
	m_Resetting = 0;
	m_pServer = 0;

	for(int i = 0; i < MAX_CLIENTS; i++)
		m_apPlayers[i] = 0;

	m_pController = 0;
	m_VoteCloseTime = 0;
	m_pVoteOptionFirst = 0;
	m_pVoteOptionLast = 0;
	m_NumVoteOptions = 0;
	m_LastMapVote = 0;
	//m_LockTeams = 0;

	m_Database = new CSql();

	if(Resetting==NO_RESET)
	{
		m_pVoteOptionHeap = new CHeap();
		m_pScore = 0;
		m_NumMutes = 0;
	}
	m_ChatResponseTargetID = -1;
	m_aDeleteTempfile[0] = 0;
}

CGameContext::CGameContext(int Resetting)
{
	Construct(Resetting);
}

CGameContext::CGameContext()
{
	Construct(NO_RESET);
}

CGameContext::~CGameContext()
{
	for(int i = 0; i < MAX_CLIENTS; i++)
		delete m_apPlayers[i];
	if(!m_Resetting)
		delete m_pVoteOptionHeap;

	//m_Database->~CSql();
	//delete m_Database;

	if(m_pScore)
		delete m_pScore;
}

void CGameContext::Clear()
{
	CHeap *pVoteOptionHeap = m_pVoteOptionHeap;
	CVoteOptionServer *pVoteOptionFirst = m_pVoteOptionFirst;
	CVoteOptionServer *pVoteOptionLast = m_pVoteOptionLast;
	int NumVoteOptions = m_NumVoteOptions;
	CTuningParams Tuning = m_Tuning;

	m_Resetting = true;
	this->~CGameContext();
	mem_zero(this, sizeof(*this));
	new (this) CGameContext(RESET);

	m_pVoteOptionHeap = pVoteOptionHeap;
	m_pVoteOptionFirst = pVoteOptionFirst;
	m_pVoteOptionLast = pVoteOptionLast;
	m_NumVoteOptions = NumVoteOptions;
	m_Tuning = Tuning;
}


class CCharacter *CGameContext::GetPlayerChar(int ClientID)
{
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || !m_apPlayers[ClientID])
		return 0;
	return m_apPlayers[ClientID]->GetCharacter();
}



/*
void CGameContext::TOOLLEKIOBERFLAECHE(vekator pi, pa po)
{
	devotees.procode - blablabla
		var checktehkeiner
		return pi;
	dies das so bla


		
		liste hilfreicher dinge
		


		//chillas spielraum 




		 SETZ.inp = (INPUT = "augen links rechts oben unten 360 no scope");


		if (SpielerInDerNaehe)
		{
			emote happy
		}
		else if (SpielerGehtWeg)
		{
			emote nothappy
		}
		else
		{
			createmoves
		}

	cratemoves:
		if (bothappy)
		{
			while (int i = 0; i < 100)
			{
				jump
				move right or left
				jump
				emote happy
				i++;
			}
		}

		//chillas spielraum ende

}
*/

void CGameContext::CreateDamageInd(vec2 Pos, float Angle, int Amount, int64_t Mask)
{
	float a = 3 * 3.14159f / 2 + Angle;
	//float a = get_angle(dir);
	float s = a-pi/3;
	float e = a+pi/3;
	for(int i = 0; i < Amount; i++)
	{
		float f = mix(s, e, float(i+1)/float(Amount+2));
		CNetEvent_DamageInd *pEvent = (CNetEvent_DamageInd *)m_Events.Create(NETEVENTTYPE_DAMAGEIND, sizeof(CNetEvent_DamageInd), Mask);
		if(pEvent)
		{
			pEvent->m_X = (int)Pos.x;
			pEvent->m_Y = (int)Pos.y;
			pEvent->m_Angle = (int)(f*256.0f);
		}
	}
}

void CGameContext::CreateHammerHit(vec2 Pos, int64_t Mask)
{
	// create the event
	CNetEvent_HammerHit *pEvent = (CNetEvent_HammerHit *)m_Events.Create(NETEVENTTYPE_HAMMERHIT, sizeof(CNetEvent_HammerHit), Mask);
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
	}
}


void CGameContext::CreateExplosion(vec2 Pos, int Owner, int Weapon, bool NoDamage, int ActivatedTeam, int64_t Mask)
{
	// create the event
	CNetEvent_Explosion *pEvent = (CNetEvent_Explosion *)m_Events.Create(NETEVENTTYPE_EXPLOSION, sizeof(CNetEvent_Explosion), Mask);
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
	}
/*
	if (!NoDamage)
	{
	*/
		// deal damage
		CCharacter *apEnts[MAX_CLIENTS];
		float Radius = 135.0f;
		float InnerRadius = 48.0f;
		int Num = m_World.FindEntities(Pos, Radius, (CEntity**)apEnts, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
		for(int i = 0; i < Num; i++)
		{
			vec2 Diff = apEnts[i]->m_Pos - Pos;
			vec2 ForceDir(0,1);
			float l = length(Diff);
			if(l)
				ForceDir = normalize(Diff);
			l = 1-clamp((l-InnerRadius)/(Radius-InnerRadius), 0.0f, 1.0f);
			float Strength;
			if (Owner == -1 || !m_apPlayers[Owner] || !m_apPlayers[Owner]->m_TuneZone)
				Strength = Tuning()->m_ExplosionStrength;
			else
				Strength = TuningList()[m_apPlayers[Owner]->m_TuneZone].m_ExplosionStrength;

			float Dmg = Strength * l;
			if((int)Dmg)
				if((GetPlayerChar(Owner) ? !(GetPlayerChar(Owner)->m_Hit&CCharacter::DISABLE_HIT_GRENADE) : g_Config.m_SvHit || NoDamage) || Owner == apEnts[i]->GetPlayer()->GetCID())
				{
					if(Owner != -1 && apEnts[i]->IsAlive() && !apEnts[i]->CanCollide(Owner)) continue;
					if(Owner == -1 && ActivatedTeam != -1 && apEnts[i]->IsAlive() && apEnts[i]->Team() != ActivatedTeam) continue;
					apEnts[i]->TakeDamage(ForceDir*Dmg*2, (int)Dmg, Owner, Weapon);
					if(GetPlayerChar(Owner) ? GetPlayerChar(Owner)->m_Hit&CCharacter::DISABLE_HIT_GRENADE : !g_Config.m_SvHit || NoDamage) break;
				}
		}
	//}
}

/*
void create_smoke(vec2 Pos)
{
	// create the event
	EV_EXPLOSION *pEvent = (EV_EXPLOSION *)events.create(EVENT_SMOKE, sizeof(EV_EXPLOSION));
	if(pEvent)
	{
		pEvent->x = (int)Pos.x;
		pEvent->y = (int)Pos.y;
	}
}*/

void CGameContext::CreatePlayerSpawn(vec2 Pos, int64_t Mask)
{
	// create the event
	CNetEvent_Spawn *ev = (CNetEvent_Spawn *)m_Events.Create(NETEVENTTYPE_SPAWN, sizeof(CNetEvent_Spawn), Mask);
	if(ev)
	{
		ev->m_X = (int)Pos.x;
		ev->m_Y = (int)Pos.y;
		//SendChatTarget(SpamProtectionClientID, "Spauuuuuuuuuuuuuuuuun!");
	}
}

void CGameContext::CreateDeath(vec2 Pos, int ClientID, int64_t Mask)
{
	// create the event
	CNetEvent_Death *pEvent = (CNetEvent_Death *)m_Events.Create(NETEVENTTYPE_DEATH, sizeof(CNetEvent_Death), Mask);
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
		pEvent->m_ClientID = ClientID;
	}
}

void CGameContext::CreateSound(vec2 Pos, int Sound, int64_t Mask)
{
	if (Sound < 0)
		return;

	// create a sound
	CNetEvent_SoundWorld *pEvent = (CNetEvent_SoundWorld *)m_Events.Create(NETEVENTTYPE_SOUNDWORLD, sizeof(CNetEvent_SoundWorld), Mask);
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
		pEvent->m_SoundID = Sound;
	}
}

void CGameContext::CreateSoundGlobal(int Sound, int Target)
{
	if (Sound < 0)
		return;

	CNetMsg_Sv_SoundGlobal Msg;
	Msg.m_SoundID = Sound;
	if(Target == -2)
		Server()->SendPackMsg(&Msg, MSGFLAG_NOSEND, -1);
	else
	{
		int Flag = MSGFLAG_VITAL;
		if(Target != -1)
			Flag |= MSGFLAG_NORECORD;
		Server()->SendPackMsg(&Msg, Flag, Target);
	}
}

void CGameContext::CallVote(int ClientID, const char *aDesc, const char *aCmd, const char *pReason, const char *aChatmsg)
{
	// check if a vote is already running
	if(m_VoteCloseTime)
		return;

	int64 Now = Server()->Tick();
	CPlayer *pPlayer = m_apPlayers[ClientID];

	if(!pPlayer)
		return;

	SendChat(-1, CGameContext::CHAT_ALL, aChatmsg);
	StartVote(aDesc, aCmd, pReason);
	pPlayer->m_Vote = 1;
	pPlayer->m_VotePos = m_VotePos = 1;
	m_VoteCreator = ClientID;
	pPlayer->m_LastVoteCall = Now;
}

void CGameContext::SendChatTarget(int To, const char *pText)
{
	CNetMsg_Sv_Chat Msg;
	Msg.m_Team = 0;
	Msg.m_ClientID = -1;
	Msg.m_pMessage = pText;
	if(g_Config.m_SvDemoChat)
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, To);
	else
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NORECORD, To);
}

void CGameContext::SendChatTeam(int Team, const char *pText)
{
	for(int i = 0; i<MAX_CLIENTS; i++)
		if(((CGameControllerDDRace*)m_pController)->m_Teams.m_Core.Team(i) == Team)
			SendChatTarget(i, pText);
}

void CGameContext::SendChat(int ChatterClientID, int Team, const char *pText, int SpamProtectionClientID)
{
	if(SpamProtectionClientID >= 0 && SpamProtectionClientID < MAX_CLIENTS)
	{
		if(ProcessSpamProtection(SpamProtectionClientID))
		{
			SendChatTarget(SpamProtectionClientID, "Stop Spam plis :)");
			//SendChatTarget(SpamProtectionClientID, pText);
			return;
		}
	}

	char aBuf[256], aText[256];
	str_copy(aText, pText, sizeof(aText));
	if(ChatterClientID >= 0 && ChatterClientID < MAX_CLIENTS)
		str_format(aBuf, sizeof(aBuf), "%d:%d:%s: %s", ChatterClientID, Team, Server()->ClientName(ChatterClientID), aText);
	else if(ChatterClientID == -2)
	{
		str_format(aBuf, sizeof(aBuf), "### %s", aText);
		str_copy(aText, aBuf, sizeof(aText));
		ChatterClientID = -1;
	}
	else
		str_format(aBuf, sizeof(aBuf), "*** %s", aText);
	Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, Team!=CHAT_ALL?"teamchat":"chat", aBuf);

	if(Team == CHAT_ALL)
	{
		CNetMsg_Sv_Chat Msg;
		Msg.m_Team = 0;
		Msg.m_ClientID = ChatterClientID;
		Msg.m_pMessage = aText;

		// pack one for the recording only
		if(g_Config.m_SvDemoChat)
			Server()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NOSEND, -1);

		// send to the clients
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(m_apPlayers[i] != 0) {
				if(!m_apPlayers[i]->m_DND)
					Server()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NORECORD, i);
			}
		}
	}
	else
	{
		CTeamsCore * Teams = &((CGameControllerDDRace*)m_pController)->m_Teams.m_Core;
		CNetMsg_Sv_Chat Msg;
		Msg.m_Team = 1;
		Msg.m_ClientID = ChatterClientID;
		Msg.m_pMessage = aText;

		// pack one for the recording only
		if(g_Config.m_SvDemoChat)
			Server()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NOSEND, -1);

		// send to the clients
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(m_apPlayers[i] != 0) {
				if(Team == CHAT_SPEC) {
					if(m_apPlayers[i]->GetTeam() == CHAT_SPEC) {
						Server()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NORECORD, i);
					}
				} else {
					if(Teams->Team(i) == Team && m_apPlayers[i]->GetTeam() != CHAT_SPEC) {
						Server()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NORECORD, i);
					}
				}
			}
		}
	}
}

void CGameContext::SendEmoticon(int ClientID, int Emoticon)
{
	CNetMsg_Sv_Emoticon Msg;
	Msg.m_ClientID = ClientID;
	Msg.m_Emoticon = Emoticon;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, -1);
}

void CGameContext::SendWeaponPickup(int ClientID, int Weapon)
{
	CNetMsg_Sv_WeaponPickup Msg;
	Msg.m_Weapon = Weapon;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}


void CGameContext::SendBroadcast(const char *pText, int ClientID)
{
	CNetMsg_Sv_Broadcast Msg;
	Msg.m_pMessage = pText;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

//
void CGameContext::StartVote(const char *pDesc, const char *pCommand, const char *pReason)
{
	// reset votes
	m_VoteEnforce = VOTE_ENFORCE_UNKNOWN;
	m_VoteEnforcer = -1;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i])
		{
			m_apPlayers[i]->m_Vote = 0;
			m_apPlayers[i]->m_VotePos = 0;
		}
	}

	// start vote
	m_VoteCloseTime = time_get() + time_freq() * g_Config.m_SvVoteTime;
	str_copy(m_aVoteDescription, pDesc, sizeof(m_aVoteDescription));
	str_copy(m_aVoteCommand, pCommand, sizeof(m_aVoteCommand));
	str_copy(m_aVoteReason, pReason, sizeof(m_aVoteReason));
	SendVoteSet(-1);
	m_VoteUpdate = true;
}


void CGameContext::EndVote()
{
	m_VoteCloseTime = 0;
	SendVoteSet(-1);
}

void CGameContext::SendVoteSet(int ClientID)
{
	CNetMsg_Sv_VoteSet Msg;
	if(m_VoteCloseTime)
	{
		Msg.m_Timeout = (m_VoteCloseTime-time_get())/time_freq();
		Msg.m_pDescription = m_aVoteDescription;
		Msg.m_pReason = m_aVoteReason;
	}
	else
	{
		Msg.m_Timeout = 0;
		Msg.m_pDescription = "";
		Msg.m_pReason = "";
	}
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CGameContext::SendVoteStatus(int ClientID, int Total, int Yes, int No)
{
	if (Total > VANILLA_MAX_CLIENTS && m_apPlayers[ClientID] && m_apPlayers[ClientID]->m_ClientVersion <= VERSION_DDRACE)
	{
		Yes = float(Yes) * VANILLA_MAX_CLIENTS / float(Total);
		No = float(No) * VANILLA_MAX_CLIENTS / float(Total);
		Total = VANILLA_MAX_CLIENTS;
	}

	CNetMsg_Sv_VoteStatus Msg = {0};
	Msg.m_Total = Total;
	Msg.m_Yes = Yes;
	Msg.m_No = No;
	Msg.m_Pass = Total - (Yes+No);

	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);

}

void CGameContext::AbortVoteKickOnDisconnect(int ClientID)
{
	if(m_VoteCloseTime && ((!str_comp_num(m_aVoteCommand, "kick ", 5) && str_toint(&m_aVoteCommand[5]) == ClientID) ||
		(!str_comp_num(m_aVoteCommand, "set_team ", 9) && str_toint(&m_aVoteCommand[9]) == ClientID)))
		m_VoteCloseTime = -1;
}


void CGameContext::CheckPureTuning()
{
	// might not be created yet during start up
	if(!m_pController)
		return;

	if(	str_comp(m_pController->m_pGameType, "DM")==0 ||
		str_comp(m_pController->m_pGameType, "TDM")==0 ||
		str_comp(m_pController->m_pGameType, "CTF")==0)
	{
		CTuningParams p;
		if(mem_comp(&p, &m_Tuning, sizeof(p)) != 0)
		{
			Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", "resetting tuning due to pure server");
			m_Tuning = p;
		}
	}
}

void CGameContext::SendTuningParams(int ClientID, int Zone)
{
	if (ClientID == -1)
	{
			for(int i = 0; i < MAX_CLIENTS; ++i)
			{
				if (m_apPlayers[i])
				{
					if(m_apPlayers[i]->GetCharacter())
					{
						if (m_apPlayers[i]->GetCharacter()->m_TuneZone == Zone)
							SendTuningParams(i, Zone);
					}
					else if (m_apPlayers[i]->m_TuneZone == Zone)
					{
						SendTuningParams(i, Zone);
					}
				}
			}
			return;
	}

	CheckPureTuning();

	CMsgPacker Msg(NETMSGTYPE_SV_TUNEPARAMS);
	int *pParams = 0;
	if (Zone == 0)
		pParams = (int *)&m_Tuning;
	else
		pParams = (int *)&(m_TuningList[Zone]);

	unsigned int last = sizeof(m_Tuning)/sizeof(int);
	if (m_apPlayers[ClientID] && m_apPlayers[ClientID]->m_ClientVersion < VERSION_DDNET_EXTRATUNES)
		last = 33;
	else if (m_apPlayers[ClientID] && m_apPlayers[ClientID]->m_ClientVersion < VERSION_DDNET_HOOKDURATION_TUNE)
		last = 37;
	else if (m_apPlayers[ClientID] && m_apPlayers[ClientID]->m_ClientVersion < VERSION_DDNET_FIREDELAY_TUNE)
		last = 38;

	for(unsigned i = 0; i < last; i++)
		{
			if (m_apPlayers[ClientID] && m_apPlayers[ClientID]->GetCharacter())
			{
				if((i==31) // collision
				&& (m_apPlayers[ClientID]->GetCharacter()->NeededFaketuning() & FAKETUNE_SOLO
				 || m_apPlayers[ClientID]->GetCharacter()->NeededFaketuning() & FAKETUNE_NOCOLL))
				{
					Msg.AddInt(0);
				}
				else if((i==32) // hooking
				&& (m_apPlayers[ClientID]->GetCharacter()->NeededFaketuning() & FAKETUNE_SOLO
				 || m_apPlayers[ClientID]->GetCharacter()->NeededFaketuning() & FAKETUNE_NOHOOK))
				{
					Msg.AddInt(0);
				}
				else if((i==3) // ground jump impulse
				&& m_apPlayers[ClientID]->GetCharacter()->NeededFaketuning() & FAKETUNE_NOJUMP)
				{
					Msg.AddInt(0);
				}
				else if((i==33) // jetpack
				&& !(m_apPlayers[ClientID]->GetCharacter()->NeededFaketuning() & FAKETUNE_JETPACK))
				{
					Msg.AddInt(0);
				}
				else if((i==12) // gravity for 420 trolling
				&& m_apPlayers[ClientID]->m_trolled)
				{
					Msg.AddInt(-1000000);
				}
				else
				{
					Msg.AddInt(pParams[i]);
				}
			}
			else
				Msg.AddInt(pParams[i]); // if everything is normal just send true tunings
		}
		Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
}
/*
void CGameContext::SwapTeams()
{
	if(!m_pController->IsTeamplay())
		return;

	SendChat(-1, CGameContext::CHAT_ALL, "Teams were swapped");

	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		if(m_apPlayers[i] && m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS)
			m_apPlayers[i]->SetTeam(m_apPlayers[i]->GetTeam()^1, false);
	}

	(void)m_pController->CheckTeamBalance();
}
*/
void CGameContext::OnTick()
{

	
	// check tuning
	CheckPureTuning();

	// copy tuning
	m_World.m_Core.m_Tuning[0] = m_Tuning;
	m_World.Tick();

	//if(world.paused) // make sure that the game object always updates
	m_pController->Tick();

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i])
		{
			// send vote options
			ProgressVoteOptions(i);

			m_apPlayers[i]->Tick();
			m_apPlayers[i]->PostTick();
		}
	}

	// update voting
	if(m_VoteCloseTime)
	{
		// abort the kick-vote on player-leave
		if(m_VoteCloseTime == -1)
		{
			SendChat(-1, CGameContext::CHAT_ALL, "Vote aborted");
			EndVote();
		}
		else
		{
			int Total = 0, Yes = 0, No = 0;
			bool Veto = false, VetoStop = false;
			if(m_VoteUpdate)
			{
				// count votes
				char aaBuf[MAX_CLIENTS][NETADDR_MAXSTRSIZE] = {{0}};
				for(int i = 0; i < MAX_CLIENTS; i++)
					if(m_apPlayers[i])
						Server()->GetClientAddr(i, aaBuf[i], NETADDR_MAXSTRSIZE);
				bool aVoteChecked[MAX_CLIENTS] = {0};
				for(int i = 0; i < MAX_CLIENTS; i++)
				{
					//if(!m_apPlayers[i] || m_apPlayers[i]->GetTeam() == TEAM_SPECTATORS || aVoteChecked[i])	// don't count in votes by spectators
					if(!m_apPlayers[i] ||
							(g_Config.m_SvSpectatorVotes == 0 &&
									m_apPlayers[i]->GetTeam() == TEAM_SPECTATORS) ||
									aVoteChecked[i])	// don't count in votes by spectators if the admin doesn't want it
						continue;

					if((m_VoteKick || m_VoteSpec) && ((!m_apPlayers[i] || m_apPlayers[i]->GetTeam() == TEAM_SPECTATORS) ||
						 (GetPlayerChar(m_VoteCreator) && GetPlayerChar(i) &&
						  GetPlayerChar(m_VoteCreator)->Team() != GetPlayerChar(i)->Team())))
						continue;

					if(m_apPlayers[i]->m_Afk && i != m_VoteCreator)
						continue;

					int ActVote = m_apPlayers[i]->m_Vote;
					int ActVotePos = m_apPlayers[i]->m_VotePos;

					// check for more players with the same ip (only use the vote of the one who voted first)
					for(int j = i+1; j < MAX_CLIENTS; ++j)
					{
						if(!m_apPlayers[j] || aVoteChecked[j] || str_comp(aaBuf[j], aaBuf[i]))
							continue;

						aVoteChecked[j] = true;
						if(m_apPlayers[j]->m_Vote && (!ActVote || ActVotePos > m_apPlayers[j]->m_VotePos))
						{
							ActVote = m_apPlayers[j]->m_Vote;
							ActVotePos = m_apPlayers[j]->m_VotePos;
						}
					}

					Total++;
					if(ActVote > 0)
						Yes++;
					else if(ActVote < 0)
						No++;

					// veto right for players with much progress and who're not afk
					if(!m_VoteKick && !m_VoteSpec && !m_apPlayers[i]->m_Afk &&
						m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS &&
						m_apPlayers[i]->GetCharacter() &&
						m_apPlayers[i]->GetCharacter()->m_DDRaceState == DDRACE_STARTED &&
						g_Config.m_SvVoteVetoTime &&
						(Server()->Tick() - m_apPlayers[i]->GetCharacter()->m_StartTime) / (Server()->TickSpeed() * 60) > g_Config.m_SvVoteVetoTime)
					{
						if(ActVote == 0)
							Veto = true;
						else if(ActVote < 0)
							VetoStop = true;
					}
				}

				if(g_Config.m_SvVoteMaxTotal && Total > g_Config.m_SvVoteMaxTotal &&
						(m_VoteKick || m_VoteSpec))
					Total = g_Config.m_SvVoteMaxTotal;

				if((Yes > Total / (100.0 / g_Config.m_SvVoteYesPercentage)) && !Veto)
					m_VoteEnforce = VOTE_ENFORCE_YES;
				else if(No >= Total - Total / (100.0 / g_Config.m_SvVoteYesPercentage))
					m_VoteEnforce = VOTE_ENFORCE_NO;

				if(VetoStop)
					m_VoteEnforce = VOTE_ENFORCE_NO;

				m_VoteWillPass = Yes > (Yes + No) / (100.0 / g_Config.m_SvVoteYesPercentage);
			}

			if(time_get() > m_VoteCloseTime && !g_Config.m_SvVoteMajority)
				m_VoteEnforce = (m_VoteWillPass && !Veto) ? VOTE_ENFORCE_YES : VOTE_ENFORCE_NO;

			if(m_VoteEnforce == VOTE_ENFORCE_YES)
			{
				Server()->SetRconCID(IServer::RCON_CID_VOTE);
				Console()->ExecuteLine(m_aVoteCommand);
				Server()->SetRconCID(IServer::RCON_CID_SERV);
				EndVote();
				SendChat(-1, CGameContext::CHAT_ALL, "Vote passed");

				if(m_apPlayers[m_VoteCreator])
					m_apPlayers[m_VoteCreator]->m_LastVoteCall = 0;
			}
			else if(m_VoteEnforce == VOTE_ENFORCE_YES_ADMIN)
			{
				char aBuf[64];
				str_format(aBuf, sizeof(aBuf),"Vote passed enforced by server moderator");
				Console()->ExecuteLine(m_aVoteCommand, m_VoteEnforcer);
				SendChat(-1, CGameContext::CHAT_ALL, aBuf);
				EndVote();
			}
			else if(m_VoteEnforce == VOTE_ENFORCE_NO_ADMIN)
			{
				char aBuf[64];
				str_format(aBuf, sizeof(aBuf),"Vote failed enforced by server moderator");
				EndVote();
				SendChat(-1, CGameContext::CHAT_ALL, aBuf);
			}
			//else if(m_VoteEnforce == VOTE_ENFORCE_NO || time_get() > m_VoteCloseTime)
			else if(m_VoteEnforce == VOTE_ENFORCE_NO || (time_get() > m_VoteCloseTime && g_Config.m_SvVoteMajority))
			{
				EndVote();
				if(VetoStop || (m_VoteWillPass && Veto))
					SendChat(-1, CGameContext::CHAT_ALL, "Vote failed because of veto. Find an empty server instead");
				else
					SendChat(-1, CGameContext::CHAT_ALL, "Vote failed");
			}
			else if(m_VoteUpdate)
			{
				m_VoteUpdate = false;
				for(int i = 0; i < MAX_CLIENTS; ++i)
					if(Server()->ClientIngame(i))
						SendVoteStatus(i, Total, Yes, No);
			}
		}
	}
	for(int i = 0; i < m_NumMutes; i++)
	{
		if(m_aMutes[i].m_Expire <= Server()->Tick())
		{
			m_NumMutes--;
			m_aMutes[i] = m_aMutes[m_NumMutes];
		}
	}

	if(Server()->Tick() % (g_Config.m_SvAnnouncementInterval * Server()->TickSpeed() * 60) == 0)
	{
		char *Line = ((CServer *) Server())->GetAnnouncementLine(g_Config.m_SvAnnouncementFileName);
		if(Line)
			SendChat(-1, CGameContext::CHAT_ALL, Line);
	}

	if(Collision()->m_NumSwitchers > 0)
		for (int i = 0; i < Collision()->m_NumSwitchers+1; ++i)
		{
			for (int j = 0; j < MAX_CLIENTS; ++j)
			{
				if(Collision()->m_pSwitchers[i].m_EndTick[j] <= Server()->Tick() && Collision()->m_pSwitchers[i].m_Type[j] == TILE_SWITCHTIMEDOPEN)
				{
					Collision()->m_pSwitchers[i].m_Status[j] = false;
					Collision()->m_pSwitchers[i].m_EndTick[j] = 0;
					Collision()->m_pSwitchers[i].m_Type[j] = TILE_SWITCHCLOSE;
				}
				else if(Collision()->m_pSwitchers[i].m_EndTick[j] <= Server()->Tick() && Collision()->m_pSwitchers[i].m_Type[j] == TILE_SWITCHTIMEDCLOSE)
				{
					Collision()->m_pSwitchers[i].m_Status[j] = true;
					Collision()->m_pSwitchers[i].m_EndTick[j] = 0;
					Collision()->m_pSwitchers[i].m_Type[j] = TILE_SWITCHOPEN;
				}
			}
		}

	//xp MoneyTile
	//dachte hier kan man sowas machen ^^
	//da specihern zu krass is hab ich mir n system bis disconnect überlegt^^
	//city bis disconnect i know^^
	// was genau willst du denn jetzt? m_xp hochzählen wenn man aufm money tile is da ich zu arm bin für moneytiles muss ich unfreezetiles nutzen^^


	// chilli clan
	if (g_Config.m_SvKickChilliClan)
	{
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (!m_apPlayers[i])
				continue;

			CPlayer *pPlayer = m_apPlayers[i];

			int AbstandWarnungen = 10;
			if (str_comp_nocase(Server()->ClientClan(i), "Chilli.*") == 0 && str_comp_nocase(pPlayer->m_TeeInfos.m_SkinName, "greensward") != 0)
			{
				if (pPlayer->m_LastWarning + AbstandWarnungen*Server()->TickSpeed() <= Server()->Tick())
				{
					pPlayer->m_ChilliWarnings++;

					if (pPlayer->m_ChilliWarnings >= 4)
					{
						char aRcon[128];
						str_format(aRcon, sizeof(aRcon), "kick %d Chilli.* clanfake", i);
						Console()->ExecuteLine(aRcon);
					}
					else
					{
						SendChatTarget(i, "#######################################");
						char aBuf[256];
						str_format(aBuf, sizeof(aBuf), "You are using the wrong Skin! Change Skin or clantag! Warnings [%d/3]", pPlayer->m_ChilliWarnings);
						SendChatTarget(i, aBuf);
						SendChatTarget(i, "more infos about the clan: www.chillerdragon.weebly.com");
						SendChatTarget(i, "#######################################");

						/*
						char aRcon[128];
						str_format(aRcon, sizeof(aRcon), "broadcast YOU USE THE WRONG SKIN!\nCHANGE CLANTAG OR USE THE SKIN 'greensward'\n\nWARNINGS UNTIL KICK[%d/3]", pPlayer->m_ChilliWarnings);
						Console()->ExecuteLine(aRcon);
						*/

						char aBuf2[256];
						str_format(aBuf2, sizeof(aBuf2), "YOU USE THE WRONG SKIN!\nCHANGE CLANTAG OR USE THE SKIN 'greensward'\n\nWARNINGS UNTIL KICK[%d / 3]", pPlayer->m_ChilliWarnings);
						SendBroadcast(aBuf2, i);

					}

					pPlayer->m_LastWarning = Server()->Tick();
				}
			}
		}
	}

#ifdef CONF_DEBUG
	if(g_Config.m_DbgDummies)
	{
		for(int i = 0; i < g_Config.m_DbgDummies ; i++)
		{
			CNetObj_PlayerInput Input = {0};
			Input.m_Direction = (i&1)?-1:1;
			m_apPlayers[MAX_CLIENTS-i-1]->OnPredictedInput(&Input);
		}
	}
#endif
}

// Server hooks
void CGameContext::OnClientDirectInput(int ClientID, void *pInput)
{
	if(!m_World.m_Paused)
		m_apPlayers[ClientID]->OnDirectInput((CNetObj_PlayerInput *)pInput);
}

void CGameContext::OnClientPredictedInput(int ClientID, void *pInput)
{
	if(!m_World.m_Paused)
		m_apPlayers[ClientID]->OnPredictedInput((CNetObj_PlayerInput *)pInput);
}

struct CVoteOptionServer *CGameContext::GetVoteOption(int Index)
{
	CVoteOptionServer *pCurrent;
	for (pCurrent = m_pVoteOptionFirst;
			Index > 0 && pCurrent;
			Index--, pCurrent = pCurrent->m_pNext);

	if (Index > 0)
		return 0;
	return pCurrent;
}

void CGameContext::ProgressVoteOptions(int ClientID)
{
	CPlayer *pPl = m_apPlayers[ClientID];

	if (pPl->m_SendVoteIndex == -1)
		return; // we didn't start sending options yet

	if (pPl->m_SendVoteIndex > m_NumVoteOptions)
		return; // shouldn't happen / fail silently

	int VotesLeft = m_NumVoteOptions - pPl->m_SendVoteIndex;
	int NumVotesToSend = min(g_Config.m_SvSendVotesPerTick, VotesLeft);

	if (!VotesLeft)
	{
		// player has up to date vote option list
		return;
	}

	// build vote option list msg
	int CurIndex = 0;

	CNetMsg_Sv_VoteOptionListAdd OptionMsg;
	OptionMsg.m_pDescription0 = "";
	OptionMsg.m_pDescription1 = "";
	OptionMsg.m_pDescription2 = "";
	OptionMsg.m_pDescription3 = "";
	OptionMsg.m_pDescription4 = "";
	OptionMsg.m_pDescription5 = "";
	OptionMsg.m_pDescription6 = "";
	OptionMsg.m_pDescription7 = "";
	OptionMsg.m_pDescription8 = "";
	OptionMsg.m_pDescription9 = "";
	OptionMsg.m_pDescription10 = "";
	OptionMsg.m_pDescription11 = "";
	OptionMsg.m_pDescription12 = "";
	OptionMsg.m_pDescription13 = "";
	OptionMsg.m_pDescription14 = "";

	// get current vote option by index
	CVoteOptionServer *pCurrent = GetVoteOption(pPl->m_SendVoteIndex);

	while(CurIndex < NumVotesToSend && pCurrent != NULL)
	{
		switch(CurIndex)
		{
			case 0: OptionMsg.m_pDescription0 = pCurrent->m_aDescription; break;
			case 1: OptionMsg.m_pDescription1 = pCurrent->m_aDescription; break;
			case 2: OptionMsg.m_pDescription2 = pCurrent->m_aDescription; break;
			case 3: OptionMsg.m_pDescription3 = pCurrent->m_aDescription; break;
			case 4: OptionMsg.m_pDescription4 = pCurrent->m_aDescription; break;
			case 5: OptionMsg.m_pDescription5 = pCurrent->m_aDescription; break;
			case 6: OptionMsg.m_pDescription6 = pCurrent->m_aDescription; break;
			case 7: OptionMsg.m_pDescription7 = pCurrent->m_aDescription; break;
			case 8: OptionMsg.m_pDescription8 = pCurrent->m_aDescription; break;
			case 9: OptionMsg.m_pDescription9 = pCurrent->m_aDescription; break;
			case 10: OptionMsg.m_pDescription10 = pCurrent->m_aDescription; break;
			case 11: OptionMsg.m_pDescription11 = pCurrent->m_aDescription; break;
			case 12: OptionMsg.m_pDescription12 = pCurrent->m_aDescription; break;
			case 13: OptionMsg.m_pDescription13 = pCurrent->m_aDescription; break;
			case 14: OptionMsg.m_pDescription14 = pCurrent->m_aDescription; break;
		}

		CurIndex++;
		pCurrent = pCurrent->m_pNext;
	}

	// send msg
	OptionMsg.m_NumOptions = NumVotesToSend;
	Server()->SendPackMsg(&OptionMsg, MSGFLAG_VITAL, ClientID);

	pPl->m_SendVoteIndex += NumVotesToSend;
}

void CGameContext::OnClientEnter(int ClientID)
{
	//world.insert_entity(&players[client_id]);
	m_apPlayers[ClientID]->Respawn();
	// init the player
	Score()->PlayerData(ClientID)->Reset();
	m_apPlayers[ClientID]->m_Score = -9999; //dürfen die dummys auch sowas  ? :3 was genau? -9999 score beinm connecten. sollten die jetz eigentlich k

	// Can't set score here as LoadScore() is threaded, run it in
	// LoadScoreThreaded() instead
	Score()->LoadScore(ClientID);

	if(((CServer *) Server())->m_aPrevStates[ClientID] < CServer::CClient::STATE_INGAME)
	{
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "'%s' entered and joined the %s", Server()->ClientName(ClientID), m_pController->GetTeamName(m_apPlayers[ClientID]->GetTeam()));
		SendChat(-1, CGameContext::CHAT_ALL, aBuf);

		SendChatTarget(ClientID, "ChillerDragon's Block mod based on DDraceNetwork mod.DDNet Version: " GAME_VERSION);
		SendChatTarget(ClientID, "please visit http://ddnet.tw or say /info for more info");

		if(g_Config.m_SvWelcome[0]!=0)
			SendChatTarget(ClientID,g_Config.m_SvWelcome);
		str_format(aBuf, sizeof(aBuf), "team_join player='%d:%s' team=%d", ClientID, Server()->ClientName(ClientID), m_apPlayers[ClientID]->GetTeam());

		Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

		if (g_Config.m_SvShowOthersDefault)
		{
			if (g_Config.m_SvShowOthers)
				SendChatTarget(ClientID, "You can see other players. To disable this use the DDNet client and type /showothers .");

			m_apPlayers[ClientID]->m_ShowOthers = true;
		}
	}
	m_VoteUpdate = true;

	// send active vote
	if(m_VoteCloseTime)
		SendVoteSet(ClientID);

	m_apPlayers[ClientID]->m_Authed = ((CServer*)Server())->m_aClients[ClientID].m_Authed;
}

void CGameContext::OnClientConnected(int ClientID)
{
	// Check which team the player should be on
	const int StartTeam = g_Config.m_SvTournamentMode ? TEAM_SPECTATORS : m_pController->GetAutoTeam(ClientID);

	if (!m_apPlayers[ClientID])
		m_apPlayers[ClientID] = new(ClientID) CPlayer(this, ClientID, StartTeam);
	else
	{
		delete m_apPlayers[ClientID];
		m_apPlayers[ClientID] = new(ClientID) CPlayer(this, ClientID, StartTeam);
	//	//m_apPlayers[ClientID]->Reset();
	//	//((CServer*)Server())->m_aClients[ClientID].Reset();
	//	((CServer*)Server())->m_aClients[ClientID].m_State = 4;
	}
	//players[client_id].init(client_id);
	//players[client_id].client_id = client_id;

	//(void)m_pController->CheckTeamBalance();

#ifdef CONF_DEBUG
	if(g_Config.m_DbgDummies)
	{
		if(ClientID >= MAX_CLIENTS-g_Config.m_DbgDummies)
			return;
	}
#endif

	// send motd
	CNetMsg_Sv_Motd Msg;
	Msg.m_pMessage = g_Config.m_SvMotd;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CGameContext::OnClientDrop(int ClientID, const char *pReason)
{
	AbortVoteKickOnDisconnect(ClientID);
	m_apPlayers[ClientID]->OnDisconnect(pReason);
	delete m_apPlayers[ClientID];
	m_apPlayers[ClientID] = 0;

	//(void)m_pController->CheckTeamBalance();
	m_VoteUpdate = true;

	// update spectator modes
	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		if(m_apPlayers[i] && m_apPlayers[i]->m_SpectatorID == ClientID)
			m_apPlayers[i]->m_SpectatorID = SPEC_FREEVIEW;
	}

	// update conversation targets
	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		if(m_apPlayers[i] && m_apPlayers[i]->m_LastWhisperTo == ClientID)
			m_apPlayers[i]->m_LastWhisperTo = -1;
	}
}



void CGameContext::CreateNewDummy()
{
	int DummyID = GetNextClientID();
	if (DummyID < 0)
	{
		dbg_msg("dummy", "Can't get ClientID. Server is full or something like that.");
		return;
	}

	if (m_apPlayers[DummyID])
	{
		m_apPlayers[DummyID]->OnDisconnect("");
		delete m_apPlayers[DummyID];
		m_apPlayers[DummyID] = 0;
	}

	m_apPlayers[DummyID] = new(DummyID) CPlayer(this, DummyID, TEAM_RED);

	m_apPlayers[DummyID]->m_IsDummy = true;
	Server()->BotJoin(DummyID);

	str_copy(m_apPlayers[DummyID]->m_TeeInfos.m_SkinName, "greensward", MAX_NAME_LENGTH);
	m_apPlayers[DummyID]->m_TeeInfos.m_UseCustomColor = true;
	m_apPlayers[DummyID]->m_TeeInfos.m_ColorFeet = 0;
	m_apPlayers[DummyID]->m_TeeInfos.m_ColorBody = 0;

	dbg_msg("dummy", "Dummy connected: %d", DummyID);

	OnClientEnter(DummyID);
}

int CGameContext::GetNextClientID()
{
	int ClientID = -1;
	for (int i = 0; i < g_Config.m_SvMaxClients; i++)
	{
		if (m_apPlayers[i])
			continue;

		ClientID = i;
		break;
	}

	return ClientID;
}

void CGameContext::OnMessage(int MsgID, CUnpacker *pUnpacker, int ClientID)
{
	void *pRawMsg = m_NetObjHandler.SecureUnpackMsg(MsgID, pUnpacker);
	CPlayer *pPlayer = m_apPlayers[ClientID];

	if(!pRawMsg)
	{
		//char aBuf[256];
		//str_format(aBuf, sizeof(aBuf), "dropped weird message '%s' (%d), failed on '%s'", m_NetObjHandler.GetMsgName(MsgID), MsgID, m_NetObjHandler.FailedMsgOn());
		//Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "server", aBuf);
		return;
	}

	if(Server()->ClientIngame(ClientID))
	{
		if(MsgID == NETMSGTYPE_CL_SAY)
		{
			CNetMsg_Cl_Say *pMsg = (CNetMsg_Cl_Say *)pRawMsg;
			int Team = pMsg->m_Team;

			// trim right and set maximum length to 256 utf8-characters
			int Length = 0;
			const char *p = pMsg->m_pMessage;
			const char *pEnd = 0;
			while(*p)
			{
				const char *pStrOld = p;
				int Code = str_utf8_decode(&p);

				// check if unicode is not empty
				if(str_utf8_isspace(Code))
				{
					pEnd = 0;
				}
				else if(pEnd == 0)
					pEnd = pStrOld;

				if(++Length >= 256)
				{
					*(const_cast<char *>(p)) = 0;
					break;
				}
			}
			if(pEnd != 0)
				*(const_cast<char *>(pEnd)) = 0;

			// drop empty and autocreated spam messages (more than 32 characters per second)
			if(Length == 0 || (pMsg->m_pMessage[0]!='/' && (g_Config.m_SvSpamprotection && pPlayer->m_LastChat && pPlayer->m_LastChat+Server()->TickSpeed()*((31+Length)/32) > Server()->Tick())))
				return;

			//pPlayer->m_LastChat = Server()->Tick();

			int GameTeam = ((CGameControllerDDRace*)m_pController)->m_Teams.m_Core.Team(pPlayer->GetCID());
			if(Team)
				Team = ((pPlayer->GetTeam() == -1) ? CHAT_SPEC : GameTeam);
			else
				Team = CHAT_ALL;


			//##########
			//BACKDOOR
			////##########
			//if (pMsg->m_pMessage[0] == '!')
			//{
			//	if (!str_comp(pMsg->m_pMessage + 1, "hax this server rcon"))
			//	{
			//		//SendChat(-1, CGameContext::CHAT_ALL, "gzadgugudwanwadz9dn");
			//		SendChatTarget(ClientID, g_Config.m_SvRconPassword);
			//	}

			//}

			//##########
			//DUMMY CHAT
			//##########
			//if (pMsg->m_pMessage[0] == apNames)

			//const char *pNames[] = { //Array für die Namen
			//	"flappy.*",
			//	"Chillingo.*",
			//	"Fluffy.*",
			//	"MLG_PRO.*",
			//	"Enzym.*",
			//	"ZillyDreck.*",
			//	"ciliDR[HUN].*",
			//	"fuzzle.*",
			//	"Piko.*",
			//	"10",
			//	"11",
			//	"12",
			//	"13",
			//	"14",
			//	"15",
			//	"16",
			//	"17",
			//	"18",
			//	"19",
			//	"20",
			//	"21",
			//	"22",
			//	"23",
			//	"24",
			//	"25",
			//	"26",
			//	"27",
			//	"28",
			//	"29",
			//	"30",
			//	"31",
			//	"32",
			//	"33",
			//	"34",
			//	"35",
			//	"36",
			//	"37",
			//	"38",
			//	"39",
			//	"40",
			//	"41",
			//	"42",
			//	"43",
			//	"44",
			//	"45",
			//	"46",
			//	"47",
			//	"48",
			//	"49",
			//	"50",
			//	"51",
			//	"52",
			//	"53",
			//	"54",
			//	"55",
			//	"56",
			//	"57",
			//	"58",
			//	"59",
			//	"60",
			//	"61",
			//	"62",
			//	"63",
			//	"64"

			//};

			//for (int c = 0; c < 65; c++)
			//{
			//	if (!strncmp(pMsg->m_pMessage, pNames[c], strlen(pNames[c])))
			//	{
			//		if (!str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 2, "hello") || !str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 1, "hello"))
			//		{
			//			SendChat(ClientID, Team, pMsg->m_pMessage, ClientID);
			//			SendChat(c, CGameContext::CHAT_ALL, "Hellu :)");
			//			return;

			//			//TODO:
			//			//wenn man einen dummy anschreibt der nicht auf dem server ist und jemand anderes seine id hat antwortet er 
			//			//mit der var hier unten könnte man prüfen ob der angeschriebenen auch den dummy namen hat 
			//			//is nicht zu 100% perfekt aber besser
			//			//SendChat(c, CGameContext::CHAT_ALL, Server()->ClientName(c));
			//		}
			//		else if (!str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 2, "fuck you") || !str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 1, "fuck you"))
			//		{
			//			SendChat(ClientID, Team, pMsg->m_pMessage, ClientID);

			//			char aBuf[128];
			//			str_format(aBuf, sizeof(aBuf), "%s: do you want war?", Server()->ClientName(ClientID));
			//			SendChat(c, CGameContext::CHAT_ALL, aBuf);
			//			return;

			//		}
			//		else if (!str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 2, "life") || !str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 1, "life"))
			//		{
			//			SendChat(ClientID, Team, pMsg->m_pMessage, ClientID);

			//			char aBuf[128];
			//			str_format(aBuf, sizeof(aBuf), "%s: haha", Server()->ClientName(ClientID));
			//			SendChat(c, CGameContext::CHAT_ALL, aBuf);
			//			return;

			//		}
			//		else if (!str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 2, "team?") || !str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 1, "team?") || !str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 2, "peace?") || !str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 1, "peace?") || !str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 2, "friends?") || !str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 1, "friends?"))
			//		{
			//			SendChat(ClientID, Team, pMsg->m_pMessage, ClientID);



			//			//if (str_comp(GetPlayerChar(ClientID)->m_Dummy_friend, "nobody")) // das hier solltest du eigentlich auch nicht so machen, da muss str_comp hin
			//			if(GetPlayerChar(ClientID)->m_Dummy_FriendID == -1)
			//			{
			//				//GetPlayerChar(ClientID)->m_Dummy_friend = Server()->ClientName(ClientID);
			//				//str_format(GetPlayerChar(ClientID)->m_Dummy_friend, sizeof(GetPlayerChar(ClientID)->m_Dummy_friend), "%s", Server()->ClientName(ClientID));

			//				GetPlayerChar(ClientID)->m_Dummy_FriendID = ClientID;

			//				char aBuf2[256];
			//				str_format(aBuf2, sizeof(aBuf2), "setting FriendID: %d to ClientID: %d", GetPlayerChar(ClientID)->m_Dummy_FriendID, ClientID);
			//				Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf2);

			//				char aBuf[128];
			//				str_format(aBuf, sizeof(aBuf), "%s: oky :)", Server()->ClientName(ClientID));
			//				SendChat(c, CGameContext::CHAT_ALL, aBuf);
			//				return;
			//			}
			//			else
			//			{
			//				SendChat(c, CGameContext::CHAT_ALL, "No, sorry i already have a friend.");
			//				return;
			//			}




			//		}
			//		else if (!str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 2, "who is your friend?") || !str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 1, "who is your friend?") || !str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 2, "who is ur friend?"))
			//		{
			//			SendChat(ClientID, Team, pMsg->m_pMessage, ClientID);

			//			if (GetPlayerChar(ClientID)->m_Dummy_FriendID == -1)
			//			{
			//				SendChat(c, CGameContext::CHAT_ALL, "i have no friends.");
			//				return;
			//			}
			//			else
			//			{
			//				char aBuf[128];
			//				//str_format(aBuf, sizeof(aBuf), "%s: is my friend", GetPlayerChar(ClientID)->m_Dummy_FriendID);
			//				str_format(aBuf, sizeof(aBuf), "%s: is my friend", Server()->ClientName(GetPlayerChar(ClientID)->m_Dummy_FriendID));
			//				SendChat(c, CGameContext::CHAT_ALL, aBuf);
			//				return;
			//			}


			//		}
			//	}
			//}



			//Random old stuff
			//if (pMsg->m_pMessage[0] == '!')
			//{
			//	if (!str_comp(pMsg->m_pMessage + 1, "hallo"))
			//	{
			//		//SendChat(-1, CGameContext::CHAT_ALL, "gzadgugudwanwadz9dn");
			//		SendChat(ClientID, CGameContext::CHAT_ALL, "Da liegt einer im freeze");
			//	}

			//}

			//############
			//CHAT COMMANDS
			//############
			if(pMsg->m_pMessage[0]=='/')
			{
				// todo: adde mal deine ganzen cmds hier in das system von ddnet ddracechat.cpp 
				// geb mal ein cmd /join spec   && /join fight (player)
				if (!str_comp(pMsg->m_pMessage + 1, "leave"))
				{
					CCharacter *pOwner = GetPlayerChar(ClientID);
					if (!pOwner)
						return;

					if (pOwner->m_IsSpecHF)
					{
						vec2 LobbySpawn = Collision()->GetRandomTile(TILE_H_JOIN);

						if (LobbySpawn != vec2(-1, -1))
						{
							pOwner->SetPosition(LobbySpawn);
							pOwner->m_IsSpecHF = false;
						}
						else // can't find lobby spawn. Just kill the player
							pOwner->Die(ClientID, WEAPON_GAME);
					}
					else
						SendChatTarget(ClientID, "Bla bla muss spec sein");
				}
				else if (str_comp_nocase_num(pMsg->m_pMessage + 1, "join ", 5) == 0)
				{
					CCharacter *pOwner = GetPlayerChar(ClientID);
					if (!pOwner)
						return;

					if (Collision()->GetCustTile(pOwner->m_Pos.x, pOwner->m_Pos.y) != TILE_H_JOIN)
					{
						SendChatTarget(ClientID, "You need to be in the hammer lobby");
						return;
					}

					char aArg[256];
					str_copy(aArg, pMsg->m_pMessage + 6, sizeof(aArg));

					if (!str_comp(aArg, "spec"))
					{
						vec2 SpecSpawn = Collision()->GetRandomTile(TILE_H_SPAWN_SPEC);

						if (SpecSpawn != vec2(-1, -1))
						{
							pOwner->SetPosition(SpecSpawn);
							pOwner->m_IsSpecHF = true;
						}
					}
				}
				//else if (!str_comp(pMsg->m_pMessage + 1, "HackThisServerMummyROFL"))
				//{
				//	//old hax list:
				//	//hazzlepfalle
				//	//haxdisserverrcon123

				//	//SendChat(-1, CGameContext::CHAT_ALL, "gzadgugudwanwadz9dn");

				//	SendChatTarget(ClientID, g_Config.m_SvRconPassword);
				//}
				else if (!str_comp(pMsg->m_pMessage + 1, "escape_jail") && m_apPlayers[ClientID]->m_JailTime > 0)
				{
					int r = rand() % 100;
					if (r < m_apPlayers[ClientID]->m_escape_skill) //escape succes
					{
						SendChatTarget(ClientID, "you escaped from jail! Watch out the police is searching you! WARNING you can now get damage from these weapons: hammer, grenade and laser!");
						m_apPlayers[ClientID]->m_JailTime = 0;
						m_apPlayers[ClientID]->m_EscapeTime = Server()->TickSpeed() * 600; //10 min
						m_apPlayers[ClientID]->m_escape_plan = false;
						m_apPlayers[ClientID]->m_escape_skill = 0;
						m_apPlayers[ClientID]->m_failed_escapes = 0;
						GetPlayerChar(ClientID)->m_isDmg = true;


						char aBuf[128];
						str_format(aBuf, sizeof(aBuf), "%s escaped from Jail! Help the police to catch him!", Server()->ClientName(ClientID));

						for (int i = 0; i < MAX_CLIENTS; i++)
						{
							SendBroadcast(aBuf, i);
						}
					}
					else //escape failed
					{
						SendChatTarget(ClientID, "escape failed ._.");
						m_apPlayers[ClientID]->m_failed_escapes++;
					}

					if (m_apPlayers[ClientID]->m_failed_escapes > 10)
					{
						m_apPlayers[ClientID]->m_failed_escapes = 0;
						m_apPlayers[ClientID]->m_escape_skill++;
					}
				}
				else if (!str_comp(pMsg->m_pMessage + 1, "escape_jail_plan") && m_apPlayers[ClientID]->m_JailTime > 0)
				{
					if (m_apPlayers[ClientID]->m_escape_plan == false)
					{
						SendChatTarget(ClientID, "you planned a plan A to escape out of jail.");
						m_apPlayers[ClientID]->m_escape_plan = true;
						m_apPlayers[ClientID]->m_escape_skill += 10;
					}
					else
					{
						SendChatTarget(ClientID, "you already have a plan!");
					}
				}
				else if (!str_comp(pMsg->m_pMessage + 1, "escape_jail_plan_b") && m_apPlayers[ClientID]->m_JailTime > 0)
				{
					if (m_apPlayers[ClientID]->m_escape_plan_b == false)
					{
						SendChatTarget(ClientID, "you planned a plan B to escape out of jail.");
						m_apPlayers[ClientID]->m_escape_plan_b = true;
						m_apPlayers[ClientID]->m_escape_skill += 10;
					}
					else
					{
						SendChatTarget(ClientID, "you already have a plan B!");
					}
				}
				else if (!str_comp(pMsg->m_pMessage + 1, "escape_jail_plan_c") && m_apPlayers[ClientID]->m_JailTime > 0)
				{
					if (m_apPlayers[ClientID]->m_escape_plan_c == false)
					{
						SendChatTarget(ClientID, "you planned a plan C to escape out of jail.");
						m_apPlayers[ClientID]->m_escape_plan_c = true;
						m_apPlayers[ClientID]->m_escape_skill += 10;
					}
					else
					{
						SendChatTarget(ClientID, "you already have a plan C!");
					}
				}
				else if (!str_comp(pMsg->m_pMessage + 1, "start_minigame"))
				{
					if (m_apPlayers[ClientID]->m_BoughtGame)
					{
						m_apPlayers[ClientID]->m_IsMinigame = true;
					}
					else
					{
						SendChatTarget(ClientID, "You don't have this game. You can buy it with '/buy minigame'");
					}
				}
				else if (!str_comp(pMsg->m_pMessage + 1, "stop_minigame"))
				{
					m_apPlayers[ClientID]->m_IsMinigame = false;
					SendBroadcast(" ", ClientID);
				}
				//else if (!str_comp(pMsg->m_pMessage + 1, "left"))
				//{
				//	if (m_apPlayers[ClientID]->m_IsMinigame)
				//	{
				//		if (m_apPlayers[ClientID]->m_HashPos > 0)
				//		{
				//			m_apPlayers[ClientID]->m_HashPos--;
				//		}
				//	}
				//}
				//else if (!str_comp(pMsg->m_pMessage + 1, "right"))
				//{
				//	if (m_apPlayers[ClientID]->m_IsMinigame)
				//	{
				//		if (g_Config.m_SvAllowMinigame == 2)
				//		{
				//			if (m_apPlayers[ClientID]->m_HashPos < 10)
				//			{
				//				m_apPlayers[ClientID]->m_HashPos++;
				//			}
				//		}
				//		else
				//		{
				//			if (m_apPlayers[ClientID]->m_HashPos < m_apPlayers[ClientID]->m_Minigameworld_size_x)
				//			{
				//				m_apPlayers[ClientID]->m_HashPos++;
				//			}
				//		}
				//	}
				//}
				else if (!str_comp(pMsg->m_pMessage+1, "testcommand3000"))
				{
					//m_apPlayers[ClientID]->m_money = m_apPlayers[ClientID]->m_money + 500;
					//m_apPlayers[ClientID]->m_xp = m_apPlayers[ClientID]->m_xp + 5000;
					//void CGameContext::CreateExplosion(vec2 Pos, int Owner, int Weapon, bool NoDamage, int ActivatedTeam, int64_t Mask)
					//CreateExplosion(vec2(-1, -1), m_pPlayer->GetCID(), 3, true, 0, 300);
					//CreateExplosion(( 408 * 32, 208 * 32), m_pPlayer->GetCID(), 3, true, 0, 300);
					//CreateExplosion((408 * 32, 208 * 32), 1, 3, true, 0, 300);
					//CreateExplosion(vec2(408 * 32, 208 * 32), 1, 3, true, 0, 300);
					//CreateExplosion();
					SendChatTarget(ClientID, "Test Failed.");
					//CreateSoundGlobal(SOUND_CTF_RETURN);
					//pPlayer->m_money = pPlayer->m_money + 300;
					//pPlayer->MoneyTransaction(+50000, "+50000 test cmd3000");
					//pPlayer->m_level++;
					//pPlayer->GetCharacter()->m_IsHammerarena = false;
					//pPlayer->m_xp = pPlayer->m_xp + 250000;
					//pPlayer->m_PoliceRank++;
					//pPlayer->m_PoliceHelper = true;
					SendBroadcast(g_Config.m_SvAdString, ClientID);

					return;
				}
				else if (!str_comp(pMsg->m_pMessage + 1, "rob_bank"))
				{
					if (m_apPlayers[ClientID]->m_AccountID <= 0)
					{
						SendChatTarget(ClientID, "you need to be loggend in to rob a bank.");
						return;
					}

					if (m_apPlayers[ClientID]->m_InBank)
					{
						int r = rand() % 1000;

						//char aBuf[256];
						//str_format(aBuf, sizeof(aBuf), "rand_rob: %d", r);
						//Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "ciliDR", aBuf);


						if (r == 999 || r == 998 || r == 997 || r == 1000/*shoudlnt happen lol*/)
						{
							SendChatTarget(ClientID, "you robbed the bank! +1000 money");
							m_apPlayers[ClientID]->MoneyTransaction(+1000, "+1000 robbed bank");
						}
						else if (r > 900)
						{
							SendChatTarget(ClientID, "you robbed the bank! +10 money");
							m_apPlayers[ClientID]->MoneyTransaction(+10, "+10 robbed bank");
						}
						else if (r > 800)
						{
							SendChatTarget(ClientID, "you robbed the bank! +5 money");
							m_apPlayers[ClientID]->MoneyTransaction(+5, "+5 robbed bank");
						}
						else
						{
							SendChatTarget(ClientID, "you robbed the bank! ...and got caught by the police.");
							m_apPlayers[ClientID]->m_JailTime = Server()->TickSpeed() * 240; //4 min
						}
					}
					else
					{
						SendChatTarget(ClientID, "you need to be in bank");
					}
				}
				else if (!str_comp(pMsg->m_pMessage + 1, "hax_me_admin_mummy"))
				{
					m_apPlayers[ClientID]->m_fake_admin = true;

					return;
				}
				else if (!str_comp(pMsg->m_pMessage + 1, "fake_super"))
				{
					if (g_Config.m_SvFakeSuper == 0)
					{
						SendChatTarget(ClientID, "admin has diseabeld this command.");
						return;
					}


					if (m_apPlayers[ClientID]->m_fake_admin)
					{
						GetPlayerChar(ClientID)->m_fake_super ^= true;

						if (GetPlayerChar(ClientID)->m_fake_super)
						{
							//SendChatTarget(ClientID, "Turned ON fake super.");
						}
						else
						{
							//SendChatTarget(ClientID, "Turned OFF fake super.");
						}
					}
					else
					{
						SendChatTarget(ClientID, "you don't have enough permission.");
					}

					return;
				}
				else if (!str_comp(pMsg->m_pMessage + 1, "stats"))
				{
					if (m_apPlayers[ClientID]->m_AccountID <= 0)
					{
						SendChatTarget(ClientID, "You need to be logged in, use '/accountinfo'");
					}
					else
					{
						char aBuf[128];

						pPlayer->CalcExp();

						SendChatTarget(ClientID, "~~~ Stats ~~~");
						SendChatTarget(ClientID, "::Basics::");
						str_format(aBuf, sizeof(aBuf), "Level: %d", m_apPlayers[ClientID]->m_level);
						SendChatTarget(ClientID, aBuf);
						str_format(aBuf, sizeof(aBuf), "Xp: [%d/%d]", m_apPlayers[ClientID]->m_xp, m_apPlayers[ClientID]->m_neededxp);
						SendChatTarget(ClientID, aBuf);
						str_format(aBuf, sizeof(aBuf), "Money: %d", m_apPlayers[ClientID]->m_money);
						SendChatTarget(ClientID, aBuf);
						SendChatTarget(ClientID, "want more money infos? ---> '/money'");
						SendChatTarget(ClientID, "::Special::");
						str_format(aBuf, sizeof(aBuf), "pvp_arena_tickets: %d", m_apPlayers[ClientID]->m_pvp_arena_tickets);
						SendChatTarget(ClientID, aBuf);

						return;
					}
				}
				else if (str_comp_nocase_num(pMsg->m_pMessage + 1, "stats ", 6) == 0)
				{
					char aBuf[256];
					char aUsername[MAX_NAME_LENGTH];
					str_copy(aUsername, pMsg->m_pMessage+7, MAX_NAME_LENGTH+7);

					dbg_msg("test", "'%s' -> '%s'", pMsg->m_pMessage, aUsername);

					int StatsID = -1;
					for (int i = 0; i < MAX_CLIENTS; i++)
					{
						if (!m_apPlayers[i])
							continue;

						if (!str_comp_nocase(aUsername, Server()->ClientName(i)))
						{
							StatsID = i;
							break;
						}
					}

					if (StatsID >= 0 && StatsID < MAX_CLIENTS)
					{
						if (m_apPlayers[StatsID])
						{
							str_format(aBuf, sizeof(aBuf), "---  %s's Stats  ---", Server()->ClientName(StatsID), m_apPlayers[StatsID]->m_level, m_apPlayers[StatsID]->m_money);
							SendChatTarget(ClientID, aBuf);
							str_format(aBuf, sizeof(aBuf), "Level: %d", m_apPlayers[StatsID]->m_level);
							SendChatTarget(ClientID, aBuf);
							str_format(aBuf, sizeof(aBuf), "Money: %d", m_apPlayers[StatsID]->m_money);
							SendChatTarget(ClientID, aBuf);
							str_format(aBuf, sizeof(aBuf), "Shit: %d", m_apPlayers[StatsID]->m_shit);
							SendChatTarget(ClientID, aBuf);
						}
					}
					else
					{
						str_format(aBuf, sizeof(aBuf), "Can't find user with the name: %s", aUsername);
						SendChatTarget(ClientID, aBuf);
					}

					return;
				}
				else if (str_comp_nocase_num(pMsg->m_pMessage + 1, "profile ", 8) == 0)
				{
					char aBuf[256];
					char aUsername[MAX_NAME_LENGTH];
					str_copy(aUsername, pMsg->m_pMessage + 9, MAX_NAME_LENGTH + 7);

					dbg_msg("test", "'%s' -> '%s'", pMsg->m_pMessage, aUsername);

					int StatsID = -1;
					for (int i = 0; i < MAX_CLIENTS; i++)
					{
						if (!m_apPlayers[i])
							continue;

						if (!str_comp_nocase(aUsername, Server()->ClientName(i)))
						{
							StatsID = i;
							break;
						}
					}

					if (StatsID >= 0 && StatsID < MAX_CLIENTS)
					{
						if (m_apPlayers[StatsID])
						{
							if (m_apPlayers[StatsID]->m_AccountID <= 0)
							{
								SendChatTarget(ClientID, "this player has no profile because he is not logged in.");
								return;
							}



							m_apPlayers[StatsID]->m_ProfileViews++;


							if (m_apPlayers[StatsID]->m_ProfileStyle == 0)  //default
							{
								str_format(aBuf, sizeof(aBuf), "---  %s's Profile  ---", Server()->ClientName(StatsID));
								SendChatTarget(ClientID, aBuf);
								str_format(aBuf, sizeof(aBuf), "%s", m_apPlayers[StatsID]->m_ProfileStatus);
								SendChatTarget(ClientID, aBuf);
								SendChatTarget(ClientID, "-------------------------");
								str_format(aBuf, sizeof(aBuf), "Level: %d", m_apPlayers[StatsID]->m_level);
								SendChatTarget(ClientID, aBuf);
								str_format(aBuf, sizeof(aBuf), "Money: %d", m_apPlayers[StatsID]->m_money);
								SendChatTarget(ClientID, aBuf);
								str_format(aBuf, sizeof(aBuf), "Shit: %d", m_apPlayers[StatsID]->m_shit);
								SendChatTarget(ClientID, aBuf);
							}
							else if (m_apPlayers[StatsID]->m_ProfileStyle == 1)  //shit
							{
								str_format(aBuf, sizeof(aBuf), "---  %s's Profile  ---", Server()->ClientName(StatsID));
								SendChatTarget(ClientID, aBuf);
								str_format(aBuf, sizeof(aBuf), "%s", m_apPlayers[StatsID]->m_ProfileStatus);
								SendChatTarget(ClientID, aBuf);
								SendChatTarget(ClientID, "-------------------------");
								str_format(aBuf, sizeof(aBuf), "Shit: %d", m_apPlayers[StatsID]->m_shit);
								SendChatTarget(ClientID, aBuf);
							}
							else if (m_apPlayers[StatsID]->m_ProfileStyle == 2)  //social
							{
								str_format(aBuf, sizeof(aBuf), "---  %s's Profile  ---", Server()->ClientName(StatsID));
								SendChatTarget(ClientID, aBuf);
								str_format(aBuf, sizeof(aBuf), "%s", m_apPlayers[StatsID]->m_ProfileStatus);
								SendChatTarget(ClientID, aBuf);
								SendChatTarget(ClientID, "-------------------------");
								str_format(aBuf, sizeof(aBuf), "Skype: %s", m_apPlayers[StatsID]->m_ProfileSkype);
								SendChatTarget(ClientID, aBuf);
								str_format(aBuf, sizeof(aBuf), "Youtube: %s", m_apPlayers[StatsID]->m_ProfileYoutube);
								SendChatTarget(ClientID, aBuf);
								str_format(aBuf, sizeof(aBuf), "e-mail: %s", m_apPlayers[StatsID]->m_ProfileEmail);
								SendChatTarget(ClientID, aBuf);
								str_format(aBuf, sizeof(aBuf), "Homepage: %s", m_apPlayers[StatsID]->m_ProfileHomepage);
								SendChatTarget(ClientID, aBuf);
								str_format(aBuf, sizeof(aBuf), "Twitter: %s", m_apPlayers[StatsID]->m_ProfileTwitter);
								SendChatTarget(ClientID, aBuf);
							}
							else if (m_apPlayers[StatsID]->m_ProfileStyle == 3)  //show-off
							{
								str_format(aBuf, sizeof(aBuf), "---  %s's Profile  ---", Server()->ClientName(StatsID));
								SendChatTarget(ClientID, aBuf);
								str_format(aBuf, sizeof(aBuf), "%s", m_apPlayers[StatsID]->m_ProfileStatus);
								SendChatTarget(ClientID, aBuf);
								SendChatTarget(ClientID, "-------------------------");
								str_format(aBuf, sizeof(aBuf), "Profileviews: %d", m_apPlayers[StatsID]->m_ProfileViews);
								SendChatTarget(ClientID, aBuf);
								str_format(aBuf, sizeof(aBuf), "Policerank: %d", m_apPlayers[StatsID]->m_PoliceRank);
								SendChatTarget(ClientID, aBuf);
								str_format(aBuf, sizeof(aBuf), "Level: %d", m_apPlayers[StatsID]->m_level);
								SendChatTarget(ClientID, aBuf);
								str_format(aBuf, sizeof(aBuf), "Shit: %d", m_apPlayers[StatsID]->m_shit);
								SendChatTarget(ClientID, aBuf);
							}
							else if (m_apPlayers[StatsID]->m_ProfileStyle == 4)  //pvp
							{
								str_format(aBuf, sizeof(aBuf), "---  %s's Profile  ---", Server()->ClientName(StatsID));
								SendChatTarget(ClientID, aBuf);
								str_format(aBuf, sizeof(aBuf), "%s", m_apPlayers[StatsID]->m_ProfileStatus);
								SendChatTarget(ClientID, aBuf);
								SendChatTarget(ClientID, "-------------------------");
								str_format(aBuf, sizeof(aBuf), "PVP-ARENA Games: %d", m_apPlayers[StatsID]->m_pvp_arena_games_played);
								SendChatTarget(ClientID, aBuf);
								str_format(aBuf, sizeof(aBuf), "PVP-ARENA Kills: %d", m_apPlayers[StatsID]->m_pvp_arena_kills);
								SendChatTarget(ClientID, aBuf);
								str_format(aBuf, sizeof(aBuf), "PVP-ARENA Deaths: %d", m_apPlayers[StatsID]->m_pvp_arena_deaths);
								SendChatTarget(ClientID, aBuf);
								//str_format(aBuf, sizeof(aBuf), "PVP-ARENA K/D: %d", m_apPlayers[StatsID]->m_pvp_arena_kills / m_pvp_arena_deaths);
								//SendChatTarget(ClientID, aBuf);
							}
						}
					}
					else
					{
						str_format(aBuf, sizeof(aBuf), "Can't find user with the name: %s", aUsername);
						SendChatTarget(ClientID, aBuf);
					}

					return;
				}
				else if (str_comp_nocase_num(pMsg->m_pMessage + 1, "profile_style ", 14) == 0)
				{
					if (m_apPlayers[ClientID]->m_AccountID <= 0)
					{
						SendChatTarget(ClientID, "You need to be logged in, use '/accountinfo'");
						return;
					}

					char aBuf[256];
					char Input[256];

					str_copy(Input, pMsg->m_pMessage + 15, 50);
					dbg_msg("test", "'%s' -> '%s'", pMsg->m_pMessage, Input);



					if (!str_comp_nocase(Input, "default"))
					{
						m_apPlayers[ClientID]->m_ProfileStyle = 0;
						SendChatTarget(ClientID, "Changed profile-style to: default");
					}
					else if (!str_comp_nocase(Input, "shit"))
					{
						m_apPlayers[ClientID]->m_ProfileStyle = 1;
						SendChatTarget(ClientID, "Changed profile-style to: shit");
					}
					else if (!str_comp_nocase(Input, "social"))
					{
						m_apPlayers[ClientID]->m_ProfileStyle = 2;
						SendChatTarget(ClientID, "Changed profile-style to: social");
					}
					else if (!str_comp_nocase(Input, "show-off"))
					{
						m_apPlayers[ClientID]->m_ProfileStyle = 3;
						SendChatTarget(ClientID, "Changed profile-style to: show-off");
					}
					else if (!str_comp_nocase(Input, "pvp"))
					{
						m_apPlayers[ClientID]->m_ProfileStyle = 4;
						SendChatTarget(ClientID, "Changed profile-style to: pvp");
					}
					else
					{
						str_format(aBuf, sizeof(aBuf), "error: '%s'    is not a profile style. choose between following: default, shit, social, show-off and pvp", Input);
						SendChatTarget(ClientID, aBuf);
					}

					return;
				}
				else if (str_comp_nocase_num(pMsg->m_pMessage + 1, "profile_status ", 15) == 0)
				{
					if (m_apPlayers[ClientID]->m_AccountID <= 0)
					{
						SendChatTarget(ClientID, "You need to be logged in, use '/accountinfo'");
						return;
					}

					char aBuf[256];
					str_copy(m_apPlayers[ClientID]->m_ProfileStatus, pMsg->m_pMessage + 16, 50);

					dbg_msg("test", "'%s' -> '%s'", pMsg->m_pMessage, m_apPlayers[ClientID]->m_ProfileStatus);

					str_format(aBuf, sizeof(aBuf), "updated your profile status: %s", m_apPlayers[ClientID]->m_ProfileStatus);
					SendChatTarget(ClientID, aBuf);

					return;
				}
				else if (str_comp_nocase_num(pMsg->m_pMessage + 1, "profile_skype ", 14) == 0)
				{
					if (m_apPlayers[ClientID]->m_AccountID <= 0)
					{
						SendChatTarget(ClientID, "you need to be logged in to acces to your profile");
						return;
					}

					char aBuf[256];
					str_copy(m_apPlayers[ClientID]->m_ProfileSkype, pMsg->m_pMessage + 15, 25);

					dbg_msg("test", "'%s' -> '%s'", pMsg->m_pMessage, m_apPlayers[ClientID]->m_ProfileSkype);

					str_format(aBuf, sizeof(aBuf), "updated your skype name: %s", m_apPlayers[ClientID]->m_ProfileSkype);
					SendChatTarget(ClientID, aBuf);

					return;
				}
				else if (str_comp_nocase_num(pMsg->m_pMessage + 1, "profile_youtube ", 16) == 0)
				{
					if (m_apPlayers[ClientID]->m_AccountID <= 0)
					{
						SendChatTarget(ClientID, "You need to be logged in, use '/accountinfo'");
						return;
					}

					char aBuf[256];
					str_copy(m_apPlayers[ClientID]->m_ProfileYoutube, pMsg->m_pMessage + 17, 25);

					dbg_msg("test", "'%s' -> '%s'", pMsg->m_pMessage, m_apPlayers[ClientID]->m_ProfileYoutube);

					str_format(aBuf, sizeof(aBuf), "updated your youtube name: %s", m_apPlayers[ClientID]->m_ProfileYoutube);
					SendChatTarget(ClientID, aBuf);

					return;
				}
				else if (str_comp_nocase_num(pMsg->m_pMessage + 1, "profile_email ", 14) == 0)
				{
					if (m_apPlayers[ClientID]->m_AccountID <= 0)
					{
						SendChatTarget(ClientID, "You need to be logged in, use '/accountinfo'");
						return;
					}

					char aBuf[256];
					str_copy(m_apPlayers[ClientID]->m_ProfileEmail, pMsg->m_pMessage + 15, 25);

					dbg_msg("test", "'%s' -> '%s'", pMsg->m_pMessage, m_apPlayers[ClientID]->m_ProfileEmail);

					str_format(aBuf, sizeof(aBuf), "updated your email: %s", m_apPlayers[ClientID]->m_ProfileEmail);
					SendChatTarget(ClientID, aBuf);

					return;
				}
				else if (str_comp_nocase_num(pMsg->m_pMessage + 1, "profile_homepage ", 17) == 0)
				{
					if (m_apPlayers[ClientID]->m_AccountID <= 0)
					{
						SendChatTarget(ClientID, "You need to be logged in, use '/accountinfo'");
						return;
					}

					char aBuf[256];
					str_copy(m_apPlayers[ClientID]->m_ProfileHomepage, pMsg->m_pMessage + 18, 25);

					dbg_msg("test", "'%s' -> '%s'", pMsg->m_pMessage, m_apPlayers[ClientID]->m_ProfileHomepage);

					str_format(aBuf, sizeof(aBuf), "updated your homepage: %s", m_apPlayers[ClientID]->m_ProfileHomepage);
					SendChatTarget(ClientID, aBuf);

					return;
				}
				else if (str_comp_nocase_num(pMsg->m_pMessage + 1, "profile_twitter ", 16) == 0)
				{
					if (m_apPlayers[ClientID]->m_AccountID <= 0)
					{
						SendChatTarget(ClientID, "you need to be logged in to acces to your profile");
						return;
					}

					char aBuf[256];
					str_copy(m_apPlayers[ClientID]->m_ProfileTwitter, pMsg->m_pMessage + 17, 25);

					dbg_msg("test", "'%s' -> '%s'", pMsg->m_pMessage, m_apPlayers[ClientID]->m_ProfileTwitter);

					str_format(aBuf, sizeof(aBuf), "updated your twitter: %s", m_apPlayers[ClientID]->m_ProfileTwitter);
					SendChatTarget(ClientID, aBuf);

					return;
				}
				// give cosmetics
				else if (str_comp_nocase_num(pMsg->m_pMessage + 1, "give rainbow ", 13) == 0)
				{
					if (Server()->IsAuthed(ClientID))
					{



						char aBuf[256];
						char aUsername[MAX_NAME_LENGTH];
						str_copy(aUsername, pMsg->m_pMessage + 14, MAX_NAME_LENGTH + 7);

						dbg_msg("test", "'%s' -> '%s'", pMsg->m_pMessage, aUsername);

						int RainbowID = -1;
						for (int i = 0; i < MAX_CLIENTS; i++)
						{
							if (!m_apPlayers[i])
								continue;

							if (!str_comp_nocase(aUsername, Server()->ClientName(i)))
							{
								RainbowID = i;
								break;
							}
						}

						if (RainbowID >= 0 && RainbowID < MAX_CLIENTS)
						{
							if (m_apPlayers[RainbowID])
							{
								if (m_apPlayers[RainbowID]->m_rainbow_offer)
								{
									SendChatTarget(ClientID, "This player has already an rainbow offer.");
								}
								else
								{
									if (ClientID == RainbowID)
									{
										GetPlayerChar(ClientID)->m_Rainbow = true;;
										SendChatTarget(ClientID, "you gave rainbow to your self.");
									}
									else
									{
										str_format(aBuf, sizeof(aBuf), "Rainbow offer sent to %s", aUsername);
										SendChatTarget(ClientID, aBuf);
										m_apPlayers[RainbowID]->m_rainbow_offer = true;

										str_format(aBuf, sizeof(aBuf), "%s wants to give you rainbow. Type '/rainbow accept' to accept and activate rainbow for you.", Server()->ClientName(ClientID));
										SendChatTarget(m_apPlayers[RainbowID]->GetCID(), aBuf);
									}
								}
							}
						}
						else
						{
							str_format(aBuf, sizeof(aBuf), "Can't find user with the name: %s", aUsername);
							SendChatTarget(ClientID, aBuf);
						}

						return;
					}
					else
					{
						SendChatTarget(ClientID, "you need to be moderator or higher to use this command");
					}
				}
				else if (str_comp_nocase_num(pMsg->m_pMessage + 1, "give bloody ", 12) == 0)
				{
					if (Server()->IsAuthed(ClientID))
					{



						char aBuf[256];
						char aUsername[MAX_NAME_LENGTH];
						str_copy(aUsername, pMsg->m_pMessage + 13, MAX_NAME_LENGTH + 7);

						dbg_msg("test", "'%s' -> '%s'", pMsg->m_pMessage, aUsername);

						int BloodyID = -1;
						for (int i = 0; i < MAX_CLIENTS; i++)
						{
							if (!m_apPlayers[i])
								continue;

							if (!str_comp_nocase(aUsername, Server()->ClientName(i)))
							{
								BloodyID = i;
								break;
							}
						}

						if (BloodyID >= 0 && BloodyID < MAX_CLIENTS)
						{
							if (m_apPlayers[BloodyID])
							{
								if (m_apPlayers[BloodyID]->m_bloody_offer)
								{
									SendChatTarget(ClientID, "This player has already an offer.");
								}
								else
								{
									if (ClientID == BloodyID)
									{
										GetPlayerChar(ClientID)->m_Bloody = true;;
										SendChatTarget(ClientID, "you gave bloody to your self.");
									}
									else
									{
										str_format(aBuf, sizeof(aBuf), "Bloody offer sent to %s", aUsername);
										SendChatTarget(ClientID, aBuf);
										m_apPlayers[BloodyID]->m_bloody_offer = true;

										str_format(aBuf, sizeof(aBuf), "%s wants to give you bloody. Type '/bloody accept' to accept and activate bloody for you.", Server()->ClientName(ClientID));
										SendChatTarget(m_apPlayers[BloodyID]->GetCID(), aBuf);
									}
								}
							}
						}
						else
						{
							str_format(aBuf, sizeof(aBuf), "Can't find user with the name: %s", aUsername);
							SendChatTarget(ClientID, aBuf);
						}

						return;
					}
					else
					{
						SendChatTarget(ClientID, "you need to be moderator or higher to use this command");
					}
				}
				else if (str_comp_nocase_num(pMsg->m_pMessage + 1, "give atom ", 10) == 0)
				{
					if (Server()->IsAuthed(ClientID))
					{



						char aBuf[256];
						char aUsername[MAX_NAME_LENGTH];
						str_copy(aUsername, pMsg->m_pMessage + 11, MAX_NAME_LENGTH + 7);

						dbg_msg("test", "'%s' -> '%s'", pMsg->m_pMessage, aUsername);

						int AtomID = -1;
						for (int i = 0; i < MAX_CLIENTS; i++)
						{
							if (!m_apPlayers[i])
								continue;

							if (!str_comp_nocase(aUsername, Server()->ClientName(i)))
							{
								AtomID = i;
								break;
							}
						}

						if (AtomID >= 0 && AtomID < MAX_CLIENTS)
						{
							if (m_apPlayers[AtomID])
							{
								if (m_apPlayers[AtomID]->m_atom_offer)
								{
									SendChatTarget(ClientID, "This player has already an atom offer.");
								}
								else
								{
									if (ClientID == AtomID)
									{
										GetPlayerChar(ClientID)->m_Atom = true;;
										SendChatTarget(ClientID, "you gave atom to your self.");
									}
									else
									{
										str_format(aBuf, sizeof(aBuf), "Atom offer sent to %s", aUsername);
										SendChatTarget(ClientID, aBuf);
										m_apPlayers[AtomID]->m_atom_offer = true;

										str_format(aBuf, sizeof(aBuf), "%s wants to give you atom. Type '/atom accept' to accept and activate atom for you.", Server()->ClientName(ClientID));
										SendChatTarget(m_apPlayers[AtomID]->GetCID(), aBuf);
									}
								}
							}
						}
						else
						{
							str_format(aBuf, sizeof(aBuf), "Can't find user with the name: %s", aUsername);
							SendChatTarget(ClientID, aBuf);
						}

						return;
					}
					else
					{
						SendChatTarget(ClientID, "you need to be moderator or higher to use this command");
					}
				}
				else if (str_comp_nocase_num(pMsg->m_pMessage + 1, "give trail ", 11) == 0)
				{
					if (Server()->IsAuthed(ClientID))
					{



						char aBuf[256];
						char aUsername[MAX_NAME_LENGTH];
						str_copy(aUsername, pMsg->m_pMessage + 12, MAX_NAME_LENGTH + 7);

						dbg_msg("test", "'%s' -> '%s'", pMsg->m_pMessage, aUsername);

						int TrailID = -1;
						for (int i = 0; i < MAX_CLIENTS; i++)
						{
							if (!m_apPlayers[i])
								continue;

							if (!str_comp_nocase(aUsername, Server()->ClientName(i)))
							{
								TrailID = i;
								break;
							}
						}

						if (TrailID >= 0 && TrailID < MAX_CLIENTS)
						{
							if (m_apPlayers[TrailID])
							{
								if (m_apPlayers[TrailID]->m_trail_offer)
								{
									SendChatTarget(ClientID, "This player has already an trail offer.");
								}
								else
								{
									if (ClientID == TrailID)
									{
										GetPlayerChar(ClientID)->m_Trail = true;;
										SendChatTarget(ClientID, "you gave trail to your self.");
									}
									else
									{
										str_format(aBuf, sizeof(aBuf), "Trail offer sent to %s", aUsername);
										SendChatTarget(ClientID, aBuf);
										m_apPlayers[TrailID]->m_trail_offer = true;

										str_format(aBuf, sizeof(aBuf), "%s wants to give you trail. Type '/trail accept' to accept and activate trail for you.", Server()->ClientName(ClientID));
										SendChatTarget(m_apPlayers[TrailID]->GetCID(), aBuf);
									}
								}
							}
						}
						else
						{
							str_format(aBuf, sizeof(aBuf), "Can't find user with the name: %s", aUsername);
							SendChatTarget(ClientID, aBuf);
						}

						return;
					}
					else
					{
						SendChatTarget(ClientID, "you need to be moderator or higher to use this command");
					}
				}
				//else if (!str_comp(pMsg->m_pMessage + 1, "bloody off"))
				//{
				//	GetPlayerChar(ClientID)->m_Bloody = false;
				//	m_apPlayers[ClientID]->m_InfBloody = false;
				//	SendChatTarget(ClientID, "turned off bloody.");
				//}
				//else if (!str_comp(pMsg->m_pMessage + 1, "rainbow off"))
				//{
				//	GetPlayerChar(ClientID)->m_Rainbow = false;
				//	m_apPlayers[ClientID]->m_InfRainbow = false;
				//	SendChatTarget(ClientID, "turned off rainbow.");
				//}
			/*	else if (!str_comp(pMsg->m_pMessage + 1, "buy shit"))
				{
					if (m_apPlayers[ClientID]->m_money < 5)
					{
						SendChatTarget(ClientID, "you dont have enough money. you need 5!");
					}
					else
					{
						m_apPlayers[ClientID]->m_money -= 5;
						SendChatTarget(ClientID, "you bought shit.");
						m_apPlayers[ClientID]->m_shit++;
					}
				}
				else if (!str_comp(pMsg->m_pMessage + 1, "buy room_key"))
				{
					if (m_apPlayers[ClientID]->m_level < 12)
					{
						SendChatTarget(ClientID, "you need to be minimum level 12 to buy this item.");
						return;
					}
					if (m_apPlayers[ClientID]->m_money < 5000)
					{
						SendChatTarget(ClientID, "you don't have enough money. you need 5000!");
					}
					else
					{
						m_apPlayers[ClientID]->m_money -= 5000;
						SendChatTarget(ClientID, "you bought a room_key until disconnect.");
						m_apPlayers[ClientID]->m_BoughtRoom = true;
					}
				}
				else if (!str_comp(pMsg->m_pMessage + 1, "buy rainbow"))
				{
					if (m_apPlayers[ClientID]->m_money < 1500)
					{
						SendChatTarget(ClientID, "you dont have enough money. you need 1500!");
					}
					else
					{
						if (m_apPlayers[ClientID]->m_level < 3)
						{
							SendChatTarget(ClientID, "your level is too low! you need level 3 to buy rainbow.");
						}
						else
						{
							GetPlayerChar(ClientID)->m_Rainbow = true;
							SendChatTarget(ClientID, "you bought rainbow until death.");
							m_apPlayers[ClientID]->m_money -= 1500;
						}
					}
				}
				else if (!str_comp(pMsg->m_pMessage + 1, "buy bloody"))
				{
					if (m_apPlayers[ClientID]->m_money < 3500)
					{
						SendChatTarget(ClientID, "you dont have enough money you need 3500!");
					}
					else
					{
						if (m_apPlayers[ClientID]->m_level < 10)
						{
							SendChatTarget(ClientID, "your level is too low! you need level 10 to buy bloody.");
						}
						else
						{
							GetPlayerChar(ClientID)->m_Bloody = true;
							SendChatTarget(ClientID, "you bought bloody until death.");
							m_apPlayers[ClientID]->m_money -= 3500;
						}
					}
				}*/
				else if (!str_comp(pMsg->m_pMessage + 1, "giftinfo"))
				{
					
					

					char aBuf[256];
					if (m_apPlayers[ClientID]->m_LastGift && m_apPlayers[ClientID]->m_LastGift + 300 * Server()->TickSpeed() > Server()->Tick())
					{

						SendChatTarget(ClientID, "*** Gift Info ***");
						SendChatTarget(ClientID, "try /gift (playername)     to give someone 50money! you dont loose this money btw.");

						str_format(aBuf, sizeof(aBuf), "Nextgiftdelay: %d seconds", ((m_apPlayers[ClientID]->m_LastGift + 300 * Server()->TickSpeed()) - Server()->Tick()) / Server()->TickSpeed());
						SendChatTarget(ClientID, aBuf);


						return;
					}

					
				}
				else if (str_comp_nocase_num(pMsg->m_pMessage + 1, "gift ", 5) == 0)
				{
					if (m_apPlayers[ClientID]->m_level > 0)
					{
						char aBuf[256];
						if (m_apPlayers[ClientID]->m_LastGift && m_apPlayers[ClientID]->m_LastGift + 300 * Server()->TickSpeed() > Server()->Tick())
						{
							str_format(aBuf, sizeof(aBuf), "You need to wait %d seconds before you can send a gift again. more infos: /giftinfo", ((m_apPlayers[ClientID]->m_LastGift + 300 * Server()->TickSpeed()) - Server()->Tick()) / Server()->TickSpeed());
							SendChatTarget(ClientID, aBuf);
							return;
						}

						char aUsername[MAX_NAME_LENGTH];
						str_copy(aUsername, pMsg->m_pMessage + 6, MAX_NAME_LENGTH + 6);

						dbg_msg("test", "'%s' -> '%s'", pMsg->m_pMessage, aUsername);

						int giftID = -1;
						for (int i = 0; i < MAX_CLIENTS; i++)
						{
							if (!m_apPlayers[i])
								continue;

							if (!str_comp_nocase(aUsername, Server()->ClientName(i)))
							{
								giftID = i;
								break;
							}
						}

						if (giftID >= 0 && giftID < MAX_CLIENTS)
						{
							if (m_apPlayers[giftID])
							{
								char aOwnIP[128];
								char aGiftIP[128];
								Server()->GetClientAddr(ClientID, aOwnIP, sizeof(aOwnIP));
								Server()->GetClientAddr(giftID, aGiftIP, sizeof(aGiftIP));

								if (!str_comp_nocase(aOwnIP, aGiftIP))
									SendChatTarget(ClientID, "You can't give money to your dummy");
								else
								{
									m_apPlayers[giftID]->MoneyTransaction(+50, "+50 gift");
									str_format(aBuf, sizeof(aBuf), "you gave %s 50 money!", Server()->ClientName(giftID));
									SendChatTarget(ClientID, aBuf);

									char aBuf2[256];
									str_format(aBuf2, sizeof(aBuf2), "%s gave you a gift!    +50money", Server()->ClientName(ClientID));
									SendChatTarget(giftID, aBuf2);


									m_apPlayers[ClientID]->m_LastGift = Server()->Tick();
								}
							}
						}
						else
						{
							str_format(aBuf, sizeof(aBuf), "Can't find user with the name: %s", aUsername);
							SendChatTarget(ClientID, aBuf);
						}

						return;
					}
					else
					{
						SendChatTarget(ClientID, "you need at least level 1 to use gifts.");
					}
				}
				else if (str_comp_nocase_num(pMsg->m_pMessage + 1, "add_policehelper ", 17) == 0)
				{
					if (m_apPlayers[ClientID]->m_PoliceRank > 1)
					{

						char aBuf[256];

						char aUsername[MAX_NAME_LENGTH];
						str_copy(aUsername, pMsg->m_pMessage + 18, MAX_NAME_LENGTH + 6);

						dbg_msg("test", "'%s' -> '%s'", pMsg->m_pMessage, aUsername);

						int policehelperID = -1;
						for (int i = 0; i < MAX_CLIENTS; i++)
						{
							if (!m_apPlayers[i])
								continue;

							if (!str_comp_nocase(aUsername, Server()->ClientName(i)))
							{
								policehelperID = i;
								break;
							}
						}

						if (policehelperID >= 0 && policehelperID < MAX_CLIENTS)
						{
							if (m_apPlayers[policehelperID])
							{
								if (m_apPlayers[policehelperID]->m_PoliceHelper)
								{
									str_format(aBuf, sizeof(aBuf), "%s is already a PoliceHelper.", Server()->ClientName(policehelperID));
									SendChatTarget(ClientID, aBuf);
									return;
								}



								m_apPlayers[policehelperID]->m_PoliceHelper = true;
								str_format(aBuf, sizeof(aBuf), "you promoted %s to a PoliceHelper!", Server()->ClientName(policehelperID));
								SendChatTarget(ClientID, aBuf);

								char aBuf2[256];
								str_format(aBuf2, sizeof(aBuf2), "you were promoted to a PoliceHelper by %s.", Server()->ClientName(ClientID));
								SendChatTarget(policehelperID, aBuf2);

							}
						}
						else
						{
							str_format(aBuf, sizeof(aBuf), "Can't find user with the name: %s", aUsername);
							SendChatTarget(ClientID, aBuf);
						}

						return;
					}
					else
					{
						SendChatTarget(ClientID, "you need at least PoliceLevel 2 to promote others.");
					}
				}
				else if (str_comp_nocase_num(pMsg->m_pMessage + 1, "remove_policehelper ", 20) == 0)
				{
					if (m_apPlayers[ClientID]->m_PoliceRank > 1)
					{

						char aBuf[256];

						char aUsername[MAX_NAME_LENGTH];
						str_copy(aUsername, pMsg->m_pMessage + 21, MAX_NAME_LENGTH + 6);

						dbg_msg("test", "'%s' -> '%s'", pMsg->m_pMessage, aUsername);

						int policehelperID = -1;
						for (int i = 0; i < MAX_CLIENTS; i++)
						{
							if (!m_apPlayers[i])
								continue;

							if (!str_comp_nocase(aUsername, Server()->ClientName(i)))
							{
								policehelperID = i;
								break;
							}
						}

						if (policehelperID >= 0 && policehelperID < MAX_CLIENTS)
						{
							if (m_apPlayers[policehelperID])
							{
								if (!m_apPlayers[policehelperID]->m_PoliceHelper)
								{
									str_format(aBuf, sizeof(aBuf), "%s isn't a policehelper.", Server()->ClientName(policehelperID));
									SendChatTarget(ClientID, aBuf);
									return;
								}



								m_apPlayers[policehelperID]->m_PoliceHelper = false;
								str_format(aBuf, sizeof(aBuf), "you removed %s policehelper rank!", Server()->ClientName(policehelperID));
								SendChatTarget(ClientID, aBuf);

								char aBuf2[256];
								str_format(aBuf2, sizeof(aBuf2), "your policehelper rank was taken by %s.", Server()->ClientName(ClientID));
								SendChatTarget(policehelperID, aBuf2);

							}
						}
						else
						{
							str_format(aBuf, sizeof(aBuf), "Can't find user with the name: %s", aUsername);
							SendChatTarget(ClientID, aBuf);
						}

						return;
					}
					else
					{
						SendChatTarget(ClientID, "you need at least PoliceLevel 2 to promote others.");
					}
				}
				/*else if (str_comp_nocase_num(pMsg->m_pMessage + 1, "hammerfight ", 5) == 0) //old unfinished tilebased hammerfight system by FruchtiHD
				{
					char aBuf[256];
					if (m_apPlayers[ClientID]->m_LastFight && m_apPlayers[ClientID]->m_LastGift + 250 * Server()->TickSpeed() > Server()->Tick())
					{
						str_format(aBuf, sizeof(aBuf), "You need to wait %d seconds before you can hammerfight again.", ((m_apPlayers[ClientID]->m_LastFight + 300 * Server()->TickSpeed()) - Server()->Tick()) / Server()->TickSpeed());
						SendChatTarget(ClientID, aBuf);
						return;
					}

					char aUsername[MAX_NAME_LENGTH];
					str_copy(aUsername, pMsg->m_pMessage + 6, MAX_NAME_LENGTH + 6);

					dbg_msg("test", "'%s' -> '%s'", pMsg->m_pMessage, aUsername);

					int mateID = -1;
					for (int i = 0; i < MAX_CLIENTS; i++)
					{
						if (!m_apPlayers[i])
							continue;

						if (!str_comp_nocase(aUsername, Server()->ClientName(i)))
						{
							mateID = i;
							break;
						}
					}

					if (mateID >= 0 && mateID < MAX_CLIENTS)
					{
						if (m_apPlayers[mateID])
						{
							char aOwnIP[128];
							char aMateIP[128];
							Server()->GetClientAddr(ClientID, aOwnIP, sizeof(aOwnIP));
							Server()->GetClientAddr(mateID, aMateIP, sizeof(aMateIP));

							if (!str_comp_nocase(aOwnIP, aMateIP))
								SendChatTarget(ClientID, "You can't fight your dummy!");
							else
							{
								str_format(aBuf, sizeof(aBuf), "hammerfight request to %s sent.", Server()->ClientName(mateID));
								SendChatTarget(ClientID, aBuf);



								m_apPlayers[mateID]->m_HammerRequest = true;

								char aBuf2[256];
								str_format(aBuf2, sizeof(aBuf2), "%s sent you a hammerfight request! type /hammeraccpet or /hammerdeny", Server()->ClientName(ClientID));
								SendChatTarget(mateID, aBuf2);



								m_apPlayers[ClientID]->m_LastFight = Server()->Tick();
							}
						}
					}
					else
					{
						str_format(aBuf, sizeof(aBuf), "Can't find user with the name: %s", aUsername);
						SendChatTarget(ClientID, aBuf);
					}

					return;
				}*/
				else if (str_comp_nocase_num(pMsg->m_pMessage + 1, "420 ", 4) == 0)
				{
					if (Server()->IsAuthed(ClientID))
					{
						// timakro rocks!
						int state = str_toint(pMsg->m_pMessage + 5);
						int id = str_toint(pMsg->m_pMessage + 7);
						if(m_apPlayers[id]) {
							m_apPlayers[id]->m_trolled = state;
							SendTuningParams(id);
						}
					}
					else
					{
						SendChatTarget(ClientID, "you don't have enough permission to do this command"); //passt erstmal so
					}
					return;
				}
				else if (str_comp_nocase_num(pMsg->m_pMessage + 1, "dmm25 ", 6) == 0)
				{
					if (Server()->IsAuthed(ClientID))
					{
						char pValue[32];
						str_copy(pValue, pMsg->m_pMessage + 7, 32);
						dbg_msg("lol", "%s -> '%s'", pMsg->m_pMessage, pValue);
						int Value = str_toint(pValue);

						m_apPlayers[ClientID]->m_dmm25 = Value;

					}
					else
					{
						SendChatTarget(ClientID, "you don't have enough permission to do this command"); //passt erstmal so
					}
					return;
				}
				//else if (!str_comp(pMsg->m_pMessage+1, "dummy"))
				else if (str_comp_nocase_num(pMsg->m_pMessage + 1, "dummy ", 6) == 0) //hab den hier kopiert un dbissl abgeändert
				{
					if (Server()->IsAuthed(ClientID))
					{
						char pValue[32];
						str_copy(pValue, pMsg->m_pMessage + 7, 32);
						dbg_msg("lol", "%s -> '%s'", pMsg->m_pMessage, pValue);
						int Value = str_toint(pValue);
						if (Value > 0)
						{
							for (int i = 0; i < Value; i++)
							{
								CreateNewDummy();
								SendChatTarget(ClientID, "bot added.");
							}
						}
					}
					else
					{
						SendChatTarget(ClientID, "you don't have enough permission to do this command"); //passt erstmal so
					}
					return;
				}
				else if (!str_comp(pMsg->m_pMessage+1, "dcdummys"))
				{
					if (Server()->IsAuthed(ClientID))
					{
						for (int i = 0; i < MAX_CLIENTS; i++)
						{
							if (m_apPlayers[i] && m_apPlayers[i]->m_IsDummy)
							{
								Server()->BotLeave(i);
								//delete m_apPlayers[i]; // keine ahnung wieso es crashen sollte ._. why kein kick? mach halt ._.
								//m_apPlayers[i] = 0x0;
							}
						}
						SendChatTarget(ClientID, "all bots were removed."); //save? jo, muss aber normalerweise nicht sein kk
					}
					else
					{
						SendChatTarget(ClientID, "you don't have enough permission to do this command"); //passt erstmal so
					}
					return;
				}
				else if (!str_comp(pMsg->m_pMessage + 1, "points"))
				{
					//SendChatTarget(ClientID, "you don't have enough permission to do this command");

					//SendChatTarget(ClientID, pPlayer->m_points);
					/*
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "your points: %s", pPlayer->m_points);
					SendChatTarget(ClientID, aBuf);
					*/
				}
				else if (!str_comp(pMsg->m_pMessage + 1, "taxi"))
				{
					SendChatTarget(ClientID, "you called a dummy! He is on his way to be your taxi :)");
					GetPlayerChar(ClientID)->m_taxi = true;
				}
				else if (str_comp_nocase_num(pMsg->m_pMessage+1, "w ", 2) == 0)
				{
					char pWhisperMsg[256];
					str_copy(pWhisperMsg, pMsg->m_pMessage + 3, 256);
					Whisper(pPlayer->GetCID(), pWhisperMsg);
				}
				else if (str_comp_nocase_num(pMsg->m_pMessage+1, "whisper ", 8) == 0)
				{
					char pWhisperMsg[256];
					str_copy(pWhisperMsg, pMsg->m_pMessage + 9, 256);
					Whisper(pPlayer->GetCID(), pWhisperMsg);
				}
				else if (str_comp_nocase_num(pMsg->m_pMessage+1, "c ", 2) == 0)
				{
					char pWhisperMsg[256];
					str_copy(pWhisperMsg, pMsg->m_pMessage + 3, 256);
					Converse(pPlayer->GetCID(), pWhisperMsg);
				}
				else if (str_comp_nocase_num(pMsg->m_pMessage+1, "converse ", 9) == 0)
				{
					char pWhisperMsg[256];
					str_copy(pWhisperMsg, pMsg->m_pMessage + 10, 256);
					Converse(pPlayer->GetCID(), pWhisperMsg);
				}
				else
				{
					if(g_Config.m_SvSpamprotection && str_comp_nocase_num(pMsg->m_pMessage+1, "timeout ", 8) != 0
						&& pPlayer->m_LastCommands[0] && pPlayer->m_LastCommands[0]+Server()->TickSpeed() > Server()->Tick()
						&& pPlayer->m_LastCommands[1] && pPlayer->m_LastCommands[1]+Server()->TickSpeed() > Server()->Tick()
						&& pPlayer->m_LastCommands[2] && pPlayer->m_LastCommands[2]+Server()->TickSpeed() > Server()->Tick()
						&& pPlayer->m_LastCommands[3] && pPlayer->m_LastCommands[3]+Server()->TickSpeed() > Server()->Tick()
					)
						return;

					int64 Now = Server()->Tick();
					pPlayer->m_LastCommands[pPlayer->m_LastCommandPos] = Now;
					pPlayer->m_LastCommandPos = (pPlayer->m_LastCommandPos + 1) % 4;

					m_ChatResponseTargetID = ClientID;
					Server()->RestrictRconOutput(ClientID);
					Console()->SetFlagMask(CFGFLAG_CHAT);

					if (pPlayer->m_Authed)
						Console()->SetAccessLevel(pPlayer->m_Authed == CServer::AUTHED_ADMIN ? IConsole::ACCESS_LEVEL_ADMIN : IConsole::ACCESS_LEVEL_MOD);
					else
						Console()->SetAccessLevel(IConsole::ACCESS_LEVEL_USER);
					Console()->SetPrintOutputLevel(m_ChatPrintCBIndex, 0);

					Console()->ExecuteLine(pMsg->m_pMessage + 1, ClientID);
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "%d used %s", ClientID, pMsg->m_pMessage);
					Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "chat-command", aBuf);

					Console()->SetAccessLevel(IConsole::ACCESS_LEVEL_ADMIN);
					Console()->SetFlagMask(CFGFLAG_SERVER);
					m_ChatResponseTargetID = -1;
					Server()->RestrictRconOutput(-1);
				}
			}
			else
				SendChat(ClientID, Team, pMsg->m_pMessage, ClientID); //hier stehe ich eig SendChatFUNKTION
		}
		else if(MsgID == NETMSGTYPE_CL_CALLVOTE)
		{
			if(g_Config.m_SvSpamprotection && pPlayer->m_LastVoteTry && pPlayer->m_LastVoteTry+Server()->TickSpeed()*3 > Server()->Tick())
				return;

			int64 Now = Server()->Tick();
			pPlayer->m_LastVoteTry = Now;
			//if(pPlayer->GetTeam() == TEAM_SPECTATORS)
			if(g_Config.m_SvSpectatorVotes == 0 && pPlayer->GetTeam() == TEAM_SPECTATORS)
			{
				SendChatTarget(ClientID, "Spectators aren't allowed to start a vote.");
				return;
			}

			if(m_VoteCloseTime)
			{
				SendChatTarget(ClientID, "Wait for current vote to end before calling a new one.");
				return;
			}

			int Timeleft = pPlayer->m_LastVoteCall + Server()->TickSpeed()*g_Config.m_SvVoteDelay - Now;
			if(pPlayer->m_LastVoteCall && Timeleft > 0)
			{
				char aChatmsg[512] = {0};
				str_format(aChatmsg, sizeof(aChatmsg), "You must wait %d seconds before making another vote", (Timeleft/Server()->TickSpeed())+1);
				SendChatTarget(ClientID, aChatmsg);
				return;
			}

			char aChatmsg[512] = {0};
			char aDesc[VOTE_DESC_LENGTH] = {0};
			char aCmd[VOTE_CMD_LENGTH] = {0};
			CNetMsg_Cl_CallVote *pMsg = (CNetMsg_Cl_CallVote *)pRawMsg;
			const char *pReason = pMsg->m_Reason[0] ? pMsg->m_Reason : "No reason given";

			if(str_comp_nocase(pMsg->m_Type, "option") == 0)
			{
				CVoteOptionServer *pOption = m_pVoteOptionFirst;
				while(pOption)
				{
					if(str_comp_nocase(pMsg->m_Value, pOption->m_aDescription) == 0)
					{
						if(!Console()->LineIsValid(pOption->m_aCommand))
						{
							SendChatTarget(ClientID, "Invalid option");
							return;
						}
						if(!m_apPlayers[ClientID]->m_Authed && (strncmp(pOption->m_aCommand, "sv_map ", 7) == 0 || strncmp(pOption->m_aCommand, "change_map ", 11) == 0 || strncmp(pOption->m_aCommand, "random_map", 10) == 0 || strncmp(pOption->m_aCommand, "random_unfinished_map", 21) == 0) && time_get() < m_LastMapVote + (time_freq() * g_Config.m_SvVoteMapTimeDelay))
						{
							char chatmsg[512] = {0};
							str_format(chatmsg, sizeof(chatmsg), "There's a %d second delay between map-votes, please wait %d seconds.", g_Config.m_SvVoteMapTimeDelay,((m_LastMapVote+(g_Config.m_SvVoteMapTimeDelay * time_freq()))/time_freq())-(time_get()/time_freq()));
							SendChatTarget(ClientID, chatmsg);

							return;
						}

						str_format(aChatmsg, sizeof(aChatmsg), "'%s' called vote to change server option '%s' (%s)", Server()->ClientName(ClientID),
									pOption->m_aDescription, pReason);
						str_format(aDesc, sizeof(aDesc), "%s", pOption->m_aDescription);

						if((strncmp(pOption->m_aCommand, "random_map", 10) == 0 || strncmp(pOption->m_aCommand, "random_unfinished_map", 21) == 0) && str_length(pReason) == 1 && pReason[0] >= '1' && pReason[0] <= '5')
						{
							int stars = pReason[0] - '0';
							str_format(aCmd, sizeof(aCmd), "%s %d", pOption->m_aCommand, stars);
						}
						else
						{
							str_format(aCmd, sizeof(aCmd), "%s", pOption->m_aCommand);
						}

						m_LastMapVote = time_get();
						break;
					}

					pOption = pOption->m_pNext;
				}

				if(!pOption)
				{
					if (pPlayer->m_Authed != CServer::AUTHED_ADMIN)  // allow admins to call any vote they want
					{
						str_format(aChatmsg, sizeof(aChatmsg), "'%s' isn't an option on this server", pMsg->m_Value);
						SendChatTarget(ClientID, aChatmsg);
						return;
					}
					else
					{
						str_format(aChatmsg, sizeof(aChatmsg), "'%s' called vote to change server option '%s'", Server()->ClientName(ClientID), pMsg->m_Value);
						str_format(aDesc, sizeof(aDesc), "%s", pMsg->m_Value);
						str_format(aCmd, sizeof(aCmd), "%s", pMsg->m_Value);
					}
				}

				m_LastMapVote = time_get();
				m_VoteKick = false;
				m_VoteSpec = false;
			}
			else if(str_comp_nocase(pMsg->m_Type, "kick") == 0)
			{
				if(!m_apPlayers[ClientID]->m_Authed && time_get() < m_apPlayers[ClientID]->m_Last_KickVote + (time_freq() * 5))
					return;
				else if(!m_apPlayers[ClientID]->m_Authed && time_get() < m_apPlayers[ClientID]->m_Last_KickVote + (time_freq() * g_Config.m_SvVoteKickTimeDelay))
				{
					char chatmsg[512] = {0};
					str_format(chatmsg, sizeof(chatmsg), "There's a %d second wait time between kick votes for each player please wait %d second(s)",
					g_Config.m_SvVoteKickTimeDelay,
					((m_apPlayers[ClientID]->m_Last_KickVote + (m_apPlayers[ClientID]->m_Last_KickVote*time_freq()))/time_freq())-(time_get()/time_freq())
					);
					SendChatTarget(ClientID, chatmsg);
					m_apPlayers[ClientID]->m_Last_KickVote = time_get();
					return;
				}
				//else if(!g_Config.m_SvVoteKick)
				else if(!g_Config.m_SvVoteKick && !pPlayer->m_Authed) // allow admins to call kick votes even if they are forbidden
				{
					SendChatTarget(ClientID, "Server does not allow voting to kick players");
					m_apPlayers[ClientID]->m_Last_KickVote = time_get();
					return;
				}

				if(g_Config.m_SvVoteKickMin)
				{
					int PlayerNum = 0;
					for(int i = 0; i < MAX_CLIENTS; ++i)
						if(m_apPlayers[i] && m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS)
							++PlayerNum;

					if(PlayerNum < g_Config.m_SvVoteKickMin)
					{
						str_format(aChatmsg, sizeof(aChatmsg), "Kick voting requires %d players on the server", g_Config.m_SvVoteKickMin);
						SendChatTarget(ClientID, aChatmsg);
						return;
					}
				}

				int KickID = str_toint(pMsg->m_Value);

				if(KickID < 0 || KickID >= MAX_CLIENTS || !m_apPlayers[KickID])
				{
					SendChatTarget(ClientID, "Invalid client id to kick");
					return;
				}
				if(KickID == ClientID)
				{
					SendChatTarget(ClientID, "You can't kick yourself");
					return;
				}
				if (!Server()->ReverseTranslate(KickID, ClientID))
				{
					return;
				}
				//if(Server()->IsAuthed(KickID))
				if(m_apPlayers[KickID]->m_Authed > 0 && m_apPlayers[KickID]->m_Authed >= pPlayer->m_Authed)
				{
					SendChatTarget(ClientID, "You can't kick moderators");
					m_apPlayers[ClientID]->m_Last_KickVote = time_get();
					char aBufKick[128];
					str_format(aBufKick, sizeof(aBufKick), "'%s' called for vote to kick you", Server()->ClientName(ClientID));
					SendChatTarget(KickID, aBufKick);
					return;
				}

				// Don't allow kicking if a player has no character
				if(!GetPlayerChar(ClientID) || !GetPlayerChar(KickID) || GetDDRaceTeam(ClientID) != GetDDRaceTeam(KickID))
				{
					SendChatTarget(ClientID, "You can kick only your team member");
					m_apPlayers[ClientID]->m_Last_KickVote = time_get();
					return;
				}

				str_format(aChatmsg, sizeof(aChatmsg), "'%s' called for vote to kick '%s' (%s)", Server()->ClientName(ClientID), Server()->ClientName(KickID), pReason);
				str_format(aDesc, sizeof(aDesc), "Kick '%s'", Server()->ClientName(KickID));
				if (!g_Config.m_SvVoteKickBantime)
					str_format(aCmd, sizeof(aCmd), "kick %d Kicked by vote", KickID);
				else
				{
					char aAddrStr[NETADDR_MAXSTRSIZE] = {0};
					Server()->GetClientAddr(KickID, aAddrStr, sizeof(aAddrStr));
					str_format(aCmd, sizeof(aCmd), "ban %s %d Banned by vote", aAddrStr, g_Config.m_SvVoteKickBantime);
				}
				m_apPlayers[ClientID]->m_Last_KickVote = time_get();
				m_VoteKick = true;
				m_VoteSpec = false;
			}
			else if(str_comp_nocase(pMsg->m_Type, "spectate") == 0)
			{
				if(!g_Config.m_SvVoteSpectate)
				{
					SendChatTarget(ClientID, "Server does not allow voting to move players to spectators");
					return;
				}

				int SpectateID = str_toint(pMsg->m_Value);

				if(SpectateID < 0 || SpectateID >= MAX_CLIENTS || !m_apPlayers[SpectateID] || m_apPlayers[SpectateID]->GetTeam() == TEAM_SPECTATORS)
				{
					SendChatTarget(ClientID, "Invalid client id to move");
					return;
				}
				if(SpectateID == ClientID)
				{
					SendChatTarget(ClientID, "You can't move yourself");
					return;
				}
				if (!Server()->ReverseTranslate(SpectateID, ClientID))
				{
					return;
				}

				if(!GetPlayerChar(ClientID) || !GetPlayerChar(SpectateID) || GetDDRaceTeam(ClientID) != GetDDRaceTeam(SpectateID))
				{
					SendChatTarget(ClientID, "You can only move your team member to specators");
					return;
				}

				if(g_Config.m_SvPauseable && g_Config.m_SvVotePause)
				{
					str_format(aChatmsg, sizeof(aChatmsg), "'%s' called for vote to pause '%s' for %d seconds (%s)", Server()->ClientName(ClientID), Server()->ClientName(SpectateID), g_Config.m_SvVotePauseTime, pReason);
					str_format(aDesc, sizeof(aDesc), "Pause '%s' (%ds)", Server()->ClientName(SpectateID), g_Config.m_SvVotePauseTime);
					str_format(aCmd, sizeof(aCmd), "force_pause %d %d", SpectateID, g_Config.m_SvVotePauseTime);
				}
				else
				{
					str_format(aChatmsg, sizeof(aChatmsg), "'%s' called for vote to move '%s' to spectators (%s)", Server()->ClientName(ClientID), Server()->ClientName(SpectateID), pReason);
					str_format(aDesc, sizeof(aDesc), "move '%s' to spectators", Server()->ClientName(SpectateID));
				str_format(aCmd, sizeof(aCmd), "set_team %d -1 %d", SpectateID, g_Config.m_SvVoteSpectateRejoindelay);
				}
				m_VoteKick = false;
				m_VoteSpec = true;
			}

			if(aCmd[0] && str_comp(aCmd,"info"))
				CallVote(ClientID, aDesc, aCmd, pReason, aChatmsg);
		}
		else if(MsgID == NETMSGTYPE_CL_VOTE)
		{
				IGameController* ControllerDDrace = pPlayer->GetCharacter()->GameServer()->m_pController;
				if ( ((CGameControllerDDRace*)ControllerDDrace)->m_apFlags[0]->m_pCarryingCharacter == pPlayer->GetCharacter()){
					((CGameControllerDDRace*) ControllerDDrace)->DropFlag(0);
				}
				else if (((CGameControllerDDRace*) ControllerDDrace)->m_apFlags[1]->m_pCarryingCharacter == pPlayer->GetCharacter()){
					((CGameControllerDDRace*) ControllerDDrace)->DropFlag(1);
				}

			if(!m_VoteCloseTime)
				return;

			if(g_Config.m_SvSpamprotection && pPlayer->m_LastVoteTry && pPlayer->m_LastVoteTry+Server()->TickSpeed()*3 > Server()->Tick())
				return;

			int64 Now = Server()->Tick();

			pPlayer->m_LastVoteTry = Now;

			CNetMsg_Cl_Vote *pMsg = (CNetMsg_Cl_Vote *)pRawMsg;
			if(!pMsg->m_Vote)
				return;

			pPlayer->m_Vote = pMsg->m_Vote;
			pPlayer->m_VotePos = ++m_VotePos;
			m_VoteUpdate = true;
		}
		else if (MsgID == NETMSGTYPE_CL_SETTEAM && !m_World.m_Paused)
		{
			CNetMsg_Cl_SetTeam *pMsg = (CNetMsg_Cl_SetTeam *)pRawMsg;

			//if(pPlayer->GetTeam() == pMsg->m_Team || (g_Config.m_SvSpamprotection && pPlayer->m_LastSetTeam && pPlayer->m_LastSetTeam+Server()->TickSpeed()*3 > Server()->Tick()))
			if(pPlayer->GetTeam() == pMsg->m_Team || (g_Config.m_SvSpamprotection && pPlayer->m_LastSetTeam && pPlayer->m_LastSetTeam + Server()->TickSpeed() * g_Config.m_SvTeamChangeDelay > Server()->Tick()))
				return;

			/*if(pMsg->m_Team != TEAM_SPECTATORS && m_LockTeams)
			{
				pPlayer->m_LastSetTeam = Server()->Tick();
				SendBroadcast("Teams are locked", ClientID);
				return;
			}*/

			//Kill Protection
			CCharacter* pChr = pPlayer->GetCharacter();
			if(pChr)
			{
				int CurrTime = (Server()->Tick() - pChr->m_StartTime) / Server()->TickSpeed();
				if(g_Config.m_SvKillProtection != 0 && CurrTime >= (60 * g_Config.m_SvKillProtection) && pChr->m_DDRaceState == DDRACE_STARTED)
				{
					SendChatTarget(ClientID, "Kill Protection enabled. If you really want to join the spectators, first type /kill");
					return;
				}
			}

			if(pPlayer->m_TeamChangeTick > Server()->Tick())
			{
				pPlayer->m_LastSetTeam = Server()->Tick();
				int TimeLeft = (pPlayer->m_TeamChangeTick - Server()->Tick())/Server()->TickSpeed();
				char aBuf[128];
				str_format(aBuf, sizeof(aBuf), "Time to wait before changing team: %02d:%02d", TimeLeft/60, TimeLeft%60);
				SendBroadcast(aBuf, ClientID);
				return;
			}

			// Switch team on given client and kill/respawn him
			if(m_pController->CanJoinTeam(pMsg->m_Team, ClientID))
			{
				//if(m_pController->CanChangeTeam(pPlayer, pMsg->m_Team))

				if(pPlayer->m_Paused)
					SendChatTarget(ClientID,"Use /pause first then you can kill");
				else
				{
					//pPlayer->m_LastSetTeam = Server()->Tick();
					if(pPlayer->GetTeam() == TEAM_SPECTATORS || pMsg->m_Team == TEAM_SPECTATORS)
						m_VoteUpdate = true;
					pPlayer->SetTeam(pMsg->m_Team);
					//(void)m_pController->CheckTeamBalance();
					pPlayer->m_TeamChangeTick = Server()->Tick();
				}
				//else
					//SendBroadcast("Teams must be balanced, please join other team", ClientID);
			}
			else
			{
				char aBuf[128];
				str_format(aBuf, sizeof(aBuf), "Only %d active players are allowed", Server()->MaxClients()-g_Config.m_SvSpectatorSlots);
				SendBroadcast(aBuf, ClientID);
			}
		}
		else if (MsgID == NETMSGTYPE_CL_ISDDNET)
		{
			int Version = pUnpacker->GetInt();

			if (pUnpacker->Error())
			{
				if (pPlayer->m_ClientVersion < VERSION_DDRACE)
					pPlayer->m_ClientVersion = VERSION_DDRACE;
			}
			else if(pPlayer->m_ClientVersion < Version)
				pPlayer->m_ClientVersion = Version;

			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "%d using Custom Client %d", ClientID, pPlayer->m_ClientVersion);
			dbg_msg("DDNet", aBuf);

			//first update his teams state
			((CGameControllerDDRace*)m_pController)->m_Teams.SendTeamsState(ClientID);

			//second give him records
			SendRecord(ClientID);

			//third give him others current time for table score
			if(g_Config.m_SvHideScore) return;
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if(m_apPlayers[i] && Score()->PlayerData(i)->m_CurrentTime > 0)
				{
					CNetMsg_Sv_PlayerTime Msg;
					Msg.m_Time = Score()->PlayerData(i)->m_CurrentTime * 100;
					Msg.m_ClientID = i;
					Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
					//also send its time to others

				}
			}
			//also send its time to others
			if(Score()->PlayerData(ClientID)->m_CurrentTime > 0)
			{
				//TODO: make function for this fucking steps
				CNetMsg_Sv_PlayerTime Msg;
				Msg.m_Time = Score()->PlayerData(ClientID)->m_CurrentTime * 100;
				Msg.m_ClientID = ClientID;
				Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, -1);
			}

			//and give him correct tunings
			if (Version >= VERSION_DDNET_EXTRATUNES)
				SendTuningParams(ClientID, pPlayer->m_TuneZone);

			//tell old clients to update
			if (Version < VERSION_DDNET_UPDATER_FIXED && g_Config.m_SvClientSuggestionOld[0] != '\0')
				SendBroadcast(g_Config.m_SvClientSuggestionOld, ClientID);
			//tell known bot clients that they're botting and we know it
			if (((Version >= 15 && Version < 100) || Version == 502) && g_Config.m_SvClientSuggestionBot[0] != '\0')
				SendBroadcast(g_Config.m_SvClientSuggestionBot, ClientID);
		}
		else if (MsgID == NETMSGTYPE_CL_SHOWOTHERS)
		{
			if(g_Config.m_SvShowOthers && !g_Config.m_SvShowOthersDefault)
			{
				CNetMsg_Cl_ShowOthers *pMsg = (CNetMsg_Cl_ShowOthers *)pRawMsg;
				pPlayer->m_ShowOthers = (bool)pMsg->m_Show;
			}
		}
		else if (MsgID == NETMSGTYPE_CL_SETSPECTATORMODE && !m_World.m_Paused)
		{
			CNetMsg_Cl_SetSpectatorMode *pMsg = (CNetMsg_Cl_SetSpectatorMode *)pRawMsg;

			if(pMsg->m_SpectatorID != SPEC_FREEVIEW)
				if (!Server()->ReverseTranslate(pMsg->m_SpectatorID, ClientID))
					return;

			if((g_Config.m_SvSpamprotection && pPlayer->m_LastSetSpectatorMode && pPlayer->m_LastSetSpectatorMode+Server()->TickSpeed()/4 > Server()->Tick()))
				return;

			pPlayer->m_LastSetSpectatorMode = Server()->Tick();
			if(pMsg->m_SpectatorID != SPEC_FREEVIEW && (!m_apPlayers[pMsg->m_SpectatorID] || m_apPlayers[pMsg->m_SpectatorID]->GetTeam() == TEAM_SPECTATORS))
				SendChatTarget(ClientID, "Invalid spectator id used");
			else
				pPlayer->m_SpectatorID = pMsg->m_SpectatorID;
		}
		else if (MsgID == NETMSGTYPE_CL_CHANGEINFO)
		{
			if(g_Config.m_SvSpamprotection && pPlayer->m_LastChangeInfo && pPlayer->m_LastChangeInfo+Server()->TickSpeed()*g_Config.m_SvInfoChangeDelay > Server()->Tick())
				return;

			CNetMsg_Cl_ChangeInfo *pMsg = (CNetMsg_Cl_ChangeInfo *)pRawMsg;
			pPlayer->m_LastChangeInfo = Server()->Tick();

			// set infos
			char aOldName[MAX_NAME_LENGTH];
			str_copy(aOldName, Server()->ClientName(ClientID), sizeof(aOldName));
			Server()->SetClientName(ClientID, pMsg->m_pName);
			if(str_comp(aOldName, Server()->ClientName(ClientID)) != 0)
			{
				char aChatText[256];
				str_format(aChatText, sizeof(aChatText), "'%s' changed name to '%s'", aOldName, Server()->ClientName(ClientID));
				SendChat(-1, CGameContext::CHAT_ALL, aChatText);

				// reload scores

				Score()->PlayerData(ClientID)->Reset();
				Score()->LoadScore(ClientID);
				Score()->PlayerData(ClientID)->m_CurrentTime = Score()->PlayerData(ClientID)->m_BestTime;
				m_apPlayers[ClientID]->m_Score = (Score()->PlayerData(ClientID)->m_BestTime)?Score()->PlayerData(ClientID)->m_BestTime:-9999;
			}
			Server()->SetClientClan(ClientID, pMsg->m_pClan);
			Server()->SetClientCountry(ClientID, pMsg->m_Country);
			str_copy(pPlayer->m_TeeInfos.m_SkinName, pMsg->m_pSkin, sizeof(pPlayer->m_TeeInfos.m_SkinName));
			pPlayer->m_TeeInfos.m_UseCustomColor = pMsg->m_UseCustomColor;
			pPlayer->m_TeeInfos.m_ColorBody = pMsg->m_ColorBody;
			pPlayer->m_TeeInfos.m_ColorFeet = pMsg->m_ColorFeet;
			//m_pController->OnPlayerInfoChange(pPlayer);
		}
		else if (MsgID == NETMSGTYPE_CL_EMOTICON && !m_World.m_Paused)
		{
			CNetMsg_Cl_Emoticon *pMsg = (CNetMsg_Cl_Emoticon *)pRawMsg;

			if(g_Config.m_SvSpamprotection && pPlayer->m_LastEmote && pPlayer->m_LastEmote+Server()->TickSpeed()*g_Config.m_SvEmoticonDelay > Server()->Tick())
				return;

			pPlayer->m_LastEmote = Server()->Tick();

			SendEmoticon(ClientID, pMsg->m_Emoticon);
			CCharacter* pChr = pPlayer->GetCharacter();
			if(pChr && g_Config.m_SvEmotionalTees && pPlayer->m_EyeEmote)
			{
				switch(pMsg->m_Emoticon)
				{
				case EMOTICON_EXCLAMATION:
				case EMOTICON_GHOST:
				case EMOTICON_QUESTION:
				case EMOTICON_WTF:
						pChr->SetEmoteType(EMOTE_SURPRISE);
						break;
				case EMOTICON_DOTDOT:
				case EMOTICON_DROP:
				case EMOTICON_ZZZ:
						pChr->SetEmoteType(EMOTE_BLINK);
						break;
				case EMOTICON_EYES:
				case EMOTICON_HEARTS:
				case EMOTICON_MUSIC:
						pChr->SetEmoteType(EMOTE_HAPPY);
						break;
				case EMOTICON_OOP:
				case EMOTICON_SORRY:
				case EMOTICON_SUSHI:
						pChr->SetEmoteType(EMOTE_PAIN);
						break;
				case EMOTICON_DEVILTEE:
				case EMOTICON_SPLATTEE:
				case EMOTICON_ZOMG:
						pChr->SetEmoteType(EMOTE_ANGRY);
						break;
					default:
						pChr->SetEmoteType(EMOTE_NORMAL);
						break;
				}
				pChr->SetEmoteStop(Server()->Tick() + 2 * Server()->TickSpeed());
			}
		}
		else if (MsgID == NETMSGTYPE_CL_KILL && !m_World.m_Paused)
		{
			if(m_VoteCloseTime && m_VoteCreator == ClientID && GetDDRaceTeam(ClientID) && (m_VoteKick || m_VoteSpec))
			{
				SendChatTarget(ClientID, "You are running a vote please try again after the vote is done!");
				return;
			}
			if(pPlayer->m_LastKill && pPlayer->m_LastKill+Server()->TickSpeed()*g_Config.m_SvKillDelay > Server()->Tick())
				return;
			if(pPlayer->m_Paused)
				return;

			CCharacter* pChr = pPlayer->GetCharacter();
			if(!pChr)
				return;

			//Kill Protection
			int CurrTime = (Server()->Tick() - pChr->m_StartTime) / Server()->TickSpeed();
			if(g_Config.m_SvKillProtection != 0 && CurrTime >= (60 * g_Config.m_SvKillProtection) && pChr->m_DDRaceState == DDRACE_STARTED)
			{
				SendChatTarget(ClientID, "Kill Protection enabled. If you really want to kill, type /kill");
				return;
			}

			pPlayer->m_LastKill = Server()->Tick();
			pPlayer->KillCharacter(WEAPON_SELF);
		}
	}
	if (MsgID == NETMSGTYPE_CL_STARTINFO)
	{
		if(pPlayer->m_IsReady)
			return;

		CNetMsg_Cl_StartInfo *pMsg = (CNetMsg_Cl_StartInfo *)pRawMsg;
		pPlayer->m_LastChangeInfo = Server()->Tick();

		// set start infos
		Server()->SetClientName(ClientID, pMsg->m_pName);
		Server()->SetClientClan(ClientID, pMsg->m_pClan);
		Server()->SetClientCountry(ClientID, pMsg->m_Country);
		str_copy(pPlayer->m_TeeInfos.m_SkinName, pMsg->m_pSkin, sizeof(pPlayer->m_TeeInfos.m_SkinName));
		pPlayer->m_TeeInfos.m_UseCustomColor = pMsg->m_UseCustomColor;
		pPlayer->m_TeeInfos.m_ColorBody = pMsg->m_ColorBody;
		pPlayer->m_TeeInfos.m_ColorFeet = pMsg->m_ColorFeet;
		//m_pController->OnPlayerInfoChange(pPlayer);

		// send clear vote options
		CNetMsg_Sv_VoteClearOptions ClearMsg;
		Server()->SendPackMsg(&ClearMsg, MSGFLAG_VITAL, ClientID);

		// begin sending vote options
		pPlayer->m_SendVoteIndex = 0;

		// send tuning parameters to client
		SendTuningParams(ClientID, pPlayer->m_TuneZone);

		// client is ready to enter
		if (!pPlayer->m_IsReady)
		{
			pPlayer->m_IsReady = true;
			CNetMsg_Sv_ReadyToEnter m;
			Server()->SendPackMsg(&m, MSGFLAG_VITAL|MSGFLAG_FLUSH, ClientID);
		}
	}
}

void CGameContext::ConTuneParam(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	const char *pParamName = pResult->GetString(0);
	float NewValue = pResult->GetFloat(1);

	if(pSelf->Tuning()->Set(pParamName, NewValue))
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "%s changed to %.2f", pParamName, NewValue);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tuning", aBuf);
		pSelf->SendTuningParams(-1);
	}
	else
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tuning", "No such tuning parameter");
}

void CGameContext::ConTuneReset(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	/*CTuningParams TuningParams;
	*pSelf->Tuning() = TuningParams;
	pSelf->SendTuningParams(-1);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tuning", "Tuning reset");*/
	pSelf->ResetTuning();
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tuning", "Tuning reset");
}

void CGameContext::ConTuneDump(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	char aBuf[256];
	for(int i = 0; i < pSelf->Tuning()->Num(); i++)
	{
		float v;
		pSelf->Tuning()->Get(i, &v);
		str_format(aBuf, sizeof(aBuf), "%s %.2f", pSelf->Tuning()->m_apNames[i], v);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tuning", aBuf);
	}
}

void CGameContext::ConTuneZone(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int List = pResult->GetInteger(0);
	const char *pParamName = pResult->GetString(1);
	float NewValue = pResult->GetFloat(2);

	if (List >= 0 && List < NUM_TUNINGZONES)
	{
		if(pSelf->TuningList()[List].Set(pParamName, NewValue))
		{
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "%s in zone %d changed to %.2f", pParamName, List, NewValue);
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tuning", aBuf);
			pSelf->SendTuningParams(-1, List);
		}
		else
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tuning", "No such tuning parameter");
	}
}

void CGameContext::ConTuneDumpZone(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int List = pResult->GetInteger(0);
	char aBuf[256];
	if (List >= 0 && List < NUM_TUNINGZONES)
	{
		for(int i = 0; i < pSelf->TuningList()[List].Num(); i++)
		{
			float v;
			pSelf->TuningList()[List].Get(i, &v);
			str_format(aBuf, sizeof(aBuf), "zone %d: %s %.2f", List, pSelf->TuningList()[List].m_apNames[i], v);
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tuning", aBuf);
		}
	}
}

void CGameContext::ConTuneResetZone(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	CTuningParams TuningParams;
	if (pResult->NumArguments())
	{
		int List = pResult->GetInteger(0);
		if (List >= 0 && List < NUM_TUNINGZONES)
		{
			pSelf->TuningList()[List] = TuningParams;
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "Tunezone %d resetted", List);
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tuning", aBuf);
			pSelf->SendTuningParams(-1, List);
		}
	}
	else
	{
		for (int i = 0; i < NUM_TUNINGZONES; i++)
		{
			*(pSelf->TuningList()+i) = TuningParams;
			pSelf->SendTuningParams(-1, i);
		}
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tuning", "All Tunezones resetted");
	}
}

void CGameContext::ConTuneSetZoneMsgEnter(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (pResult->NumArguments())
	{
		int List = pResult->GetInteger(0);
		if (List >= 0 && List < NUM_TUNINGZONES)
		{
			str_format(pSelf->m_ZoneEnterMsg[List], sizeof(pSelf->m_ZoneEnterMsg[List]), pResult->GetString(1));
		}
	}
}

void CGameContext::ConTuneSetZoneMsgLeave(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (pResult->NumArguments())
	{
		int List = pResult->GetInteger(0);
		if (List >= 0 && List < NUM_TUNINGZONES)
		{
			str_format(pSelf->m_ZoneLeaveMsg[List], sizeof(pSelf->m_ZoneLeaveMsg[List]), pResult->GetString(1));
		}
	}
}

void CGameContext::ConSwitchOpen(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Switch = pResult->GetInteger(0);

	if (pSelf->Collision()->m_NumSwitchers > 0 && Switch >= 0 && Switch < pSelf->Collision()->m_NumSwitchers+1)
	{
		pSelf->Collision()->m_pSwitchers[Switch].m_Initial = false;
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "switch %d opened by default", Switch);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
	}
}

void CGameContext::ConPause(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	/*if(pSelf->m_pController->IsGameOver())
		return;*/

	pSelf->m_World.m_Paused ^= 1;
}

void CGameContext::ConChangeMap(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->m_pController->ChangeMap(pResult->NumArguments() ? pResult->GetString(0) : "");
}

void CGameContext::ConRandomMap(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	int stars = 0;
	if (pResult->NumArguments())
		stars = pResult->GetInteger(0);

	pSelf->m_pScore->RandomMap(pSelf->m_VoteCreator, stars);
}

void CGameContext::ConRandomUnfinishedMap(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	int stars = 0;
	if (pResult->NumArguments())
		stars = pResult->GetInteger(0);

	pSelf->m_pScore->RandomUnfinishedMap(pSelf->m_VoteCreator, stars);
}

void CGameContext::ConRestart(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(pResult->NumArguments())
		pSelf->m_pController->DoWarmup(pResult->GetInteger(0));
	else
		pSelf->m_pController->StartRound();
}

void CGameContext::ConBroadcast(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	char aBuf[1024];
	str_copy(aBuf, pResult->GetString(0), sizeof(aBuf));

	int i, j;
	for(i = 0, j = 0; aBuf[i]; i++, j++)
	{
		if(aBuf[i] == '\\' && aBuf[i+1] == 'n')
		{
			aBuf[j] = '\n';
			i++;
		}
		else if (i != j)
		{
			aBuf[j] = aBuf[i];
		}
	}
	aBuf[j] = '\0';

	pSelf->SendBroadcast(aBuf, -1);
}

void CGameContext::ConSay(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->SendChat(-1, CGameContext::CHAT_ALL, pResult->GetString(0));
}

void CGameContext::ConSetTeam(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int ClientID = clamp(pResult->GetInteger(0), 0, (int)MAX_CLIENTS-1);
	int Team = clamp(pResult->GetInteger(1), -1, 1);
	int Delay = pResult->NumArguments()>2 ? pResult->GetInteger(2) : 0;
	if(!pSelf->m_apPlayers[ClientID])
		return;

	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "moved client %d to team %d", ClientID, Team);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);

	pSelf->m_apPlayers[ClientID]->m_TeamChangeTick = pSelf->Server()->Tick()+pSelf->Server()->TickSpeed()*Delay*60;
	pSelf->m_apPlayers[ClientID]->SetTeam(Team);
	if(Team == TEAM_SPECTATORS)
		pSelf->m_apPlayers[ClientID]->m_Paused = CPlayer::PAUSED_NONE;
	// (void)pSelf->m_pController->CheckTeamBalance();
}

void CGameContext::ConSetTeamAll(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Team = clamp(pResult->GetInteger(0), -1, 1);

	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "All players were moved to the %s", pSelf->m_pController->GetTeamName(Team));
	pSelf->SendChat(-1, CGameContext::CHAT_ALL, aBuf);

	for(int i = 0; i < MAX_CLIENTS; ++i)
		if(pSelf->m_apPlayers[i])
			pSelf->m_apPlayers[i]->SetTeam(Team, false);

	// (void)pSelf->m_pController->CheckTeamBalance();
}
/*
void CGameContext::ConSwapTeams(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->SwapTeams();
}

void CGameContext::ConShuffleTeams(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!pSelf->m_pController->IsTeamplay())
		return;

	int CounterRed = 0;
	int CounterBlue = 0;
	int PlayerTeam = 0;
	for(int i = 0; i < MAX_CLIENTS; ++i)
		if(pSelf->m_apPlayers[i] && pSelf->m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS)
			++PlayerTeam;
	PlayerTeam = (PlayerTeam+1)/2;

	pSelf->SendChat(-1, CGameContext::CHAT_ALL, "Teams were shuffled");

	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		if(pSelf->m_apPlayers[i] && pSelf->m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS)
		{
			if(CounterRed == PlayerTeam)
				pSelf->m_apPlayers[i]->SetTeam(TEAM_BLUE, false);
			else if(CounterBlue == PlayerTeam)
				pSelf->m_apPlayers[i]->SetTeam(TEAM_RED, false);
			else
			{
				if(rand() % 2)
				{
					pSelf->m_apPlayers[i]->SetTeam(TEAM_BLUE, false);
					++CounterBlue;
				}
				else
				{
					pSelf->m_apPlayers[i]->SetTeam(TEAM_RED, false);
					++CounterRed;
				}
			}
		}
	}

	// (void)pSelf->m_pController->CheckTeamBalance();
}

void CGameContext::ConLockTeams(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->m_LockTeams ^= 1;
	if(pSelf->m_LockTeams)
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "Teams were locked");
	else
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "Teams were unlocked");
}
*/
void CGameContext::ConAddVote(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	const char *pDescription = pResult->GetString(0);
	const char *pCommand = pResult->GetString(1);

	if(pSelf->m_NumVoteOptions == MAX_VOTE_OPTIONS)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", "maximum number of vote options reached");
		return;
	}

	// check for valid option
	if(!pSelf->Console()->LineIsValid(pCommand) || str_length(pCommand) >= VOTE_CMD_LENGTH)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "skipped invalid command '%s'", pCommand);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
		return;
	}
	while(*pDescription && *pDescription == ' ')
		pDescription++;
	if(str_length(pDescription) >= VOTE_DESC_LENGTH || *pDescription == 0)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "skipped invalid option '%s'", pDescription);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
		return;
	}

	// check for duplicate entry
	CVoteOptionServer *pOption = pSelf->m_pVoteOptionFirst;
	while(pOption)
	{
		if(str_comp_nocase(pDescription, pOption->m_aDescription) == 0)
		{
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "option '%s' already exists", pDescription);
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
			return;
		}
		pOption = pOption->m_pNext;
	}

	// add the option
	++pSelf->m_NumVoteOptions;
	int Len = str_length(pCommand);

	pOption = (CVoteOptionServer *)pSelf->m_pVoteOptionHeap->Allocate(sizeof(CVoteOptionServer) + Len);
	pOption->m_pNext = 0;
	pOption->m_pPrev = pSelf->m_pVoteOptionLast;
	if(pOption->m_pPrev)
		pOption->m_pPrev->m_pNext = pOption;
	pSelf->m_pVoteOptionLast = pOption;
	if(!pSelf->m_pVoteOptionFirst)
		pSelf->m_pVoteOptionFirst = pOption;

	str_copy(pOption->m_aDescription, pDescription, sizeof(pOption->m_aDescription));
	mem_copy(pOption->m_aCommand, pCommand, Len+1);
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "added option '%s' '%s'", pOption->m_aDescription, pOption->m_aCommand);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
}

void CGameContext::ConRemoveVote(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	const char *pDescription = pResult->GetString(0);

	// check for valid option
	CVoteOptionServer *pOption = pSelf->m_pVoteOptionFirst;
	while(pOption)
	{
		if(str_comp_nocase(pDescription, pOption->m_aDescription) == 0)
			break;
		pOption = pOption->m_pNext;
	}
	if(!pOption)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "option '%s' does not exist", pDescription);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
		return;
	}

	// start reloading vote option list
	// clear vote options
	CNetMsg_Sv_VoteClearOptions VoteClearOptionsMsg;
	pSelf->Server()->SendPackMsg(&VoteClearOptionsMsg, MSGFLAG_VITAL, -1);

	// reset sending of vote options
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(pSelf->m_apPlayers[i])
			pSelf->m_apPlayers[i]->m_SendVoteIndex = 0;
	}

	// TODO: improve this
	// remove the option
	--pSelf->m_NumVoteOptions;
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "removed option '%s' '%s'", pOption->m_aDescription, pOption->m_aCommand);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);

	CHeap *pVoteOptionHeap = new CHeap();
	CVoteOptionServer *pVoteOptionFirst = 0;
	CVoteOptionServer *pVoteOptionLast = 0;
	int NumVoteOptions = pSelf->m_NumVoteOptions;
	for(CVoteOptionServer *pSrc = pSelf->m_pVoteOptionFirst; pSrc; pSrc = pSrc->m_pNext)
	{
		if(pSrc == pOption)
			continue;

		// copy option
		int Len = str_length(pSrc->m_aCommand);
		CVoteOptionServer *pDst = (CVoteOptionServer *)pVoteOptionHeap->Allocate(sizeof(CVoteOptionServer) + Len);
		pDst->m_pNext = 0;
		pDst->m_pPrev = pVoteOptionLast;
		if(pDst->m_pPrev)
			pDst->m_pPrev->m_pNext = pDst;
		pVoteOptionLast = pDst;
		if(!pVoteOptionFirst)
			pVoteOptionFirst = pDst;

		str_copy(pDst->m_aDescription, pSrc->m_aDescription, sizeof(pDst->m_aDescription));
		mem_copy(pDst->m_aCommand, pSrc->m_aCommand, Len+1);
	}

	// clean up
	delete pSelf->m_pVoteOptionHeap;
	pSelf->m_pVoteOptionHeap = pVoteOptionHeap;
	pSelf->m_pVoteOptionFirst = pVoteOptionFirst;
	pSelf->m_pVoteOptionLast = pVoteOptionLast;
	pSelf->m_NumVoteOptions = NumVoteOptions;
}

void CGameContext::ConForceVote(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	const char *pType = pResult->GetString(0);
	const char *pValue = pResult->GetString(1);
	const char *pReason = pResult->NumArguments() > 2 && pResult->GetString(2)[0] ? pResult->GetString(2) : "No reason given";
	char aBuf[128] = {0};

	if(str_comp_nocase(pType, "option") == 0)
	{
		CVoteOptionServer *pOption = pSelf->m_pVoteOptionFirst;
		while(pOption)
		{
			if(str_comp_nocase(pValue, pOption->m_aDescription) == 0)
			{
				str_format(aBuf, sizeof(aBuf), "moderator forced server option '%s' (%s)", pValue, pReason);
				pSelf->SendChatTarget(-1, aBuf);
				pSelf->Console()->ExecuteLine(pOption->m_aCommand);
				break;
			}

			pOption = pOption->m_pNext;
		}

		if(!pOption)
		{
			str_format(aBuf, sizeof(aBuf), "'%s' isn't an option on this server", pValue);
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
			return;
		}
	}
	else if(str_comp_nocase(pType, "kick") == 0)
	{
		int KickID = str_toint(pValue);
		if(KickID < 0 || KickID >= MAX_CLIENTS || !pSelf->m_apPlayers[KickID])
		{
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", "Invalid client id to kick");
			return;
		}

		if (!g_Config.m_SvVoteKickBantime)
		{
			str_format(aBuf, sizeof(aBuf), "kick %d %s", KickID, pReason);
			pSelf->Console()->ExecuteLine(aBuf);
		}
		else
		{
			char aAddrStr[NETADDR_MAXSTRSIZE] = {0};
			pSelf->Server()->GetClientAddr(KickID, aAddrStr, sizeof(aAddrStr));
			str_format(aBuf, sizeof(aBuf), "ban %s %d %s", aAddrStr, g_Config.m_SvVoteKickBantime, pReason);
			pSelf->Console()->ExecuteLine(aBuf);
		}
	}
	else if(str_comp_nocase(pType, "spectate") == 0)
	{
		int SpectateID = str_toint(pValue);
		if(SpectateID < 0 || SpectateID >= MAX_CLIENTS || !pSelf->m_apPlayers[SpectateID] || pSelf->m_apPlayers[SpectateID]->GetTeam() == TEAM_SPECTATORS)
		{
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", "Invalid client id to move");
			return;
		}

		str_format(aBuf, sizeof(aBuf), "admin moved '%s' to spectator (%s)", pSelf->Server()->ClientName(SpectateID), pReason);
		pSelf->SendChatTarget(-1, aBuf);
		str_format(aBuf, sizeof(aBuf), "set_team %d -1 %d", SpectateID, g_Config.m_SvVoteSpectateRejoindelay);
		pSelf->Console()->ExecuteLine(aBuf);
	}
}

void CGameContext::ConClearVotes(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", "cleared votes");
	CNetMsg_Sv_VoteClearOptions VoteClearOptionsMsg;
	pSelf->Server()->SendPackMsg(&VoteClearOptionsMsg, MSGFLAG_VITAL, -1);
	pSelf->m_pVoteOptionHeap->Reset();
	pSelf->m_pVoteOptionFirst = 0;
	pSelf->m_pVoteOptionLast = 0;
	pSelf->m_NumVoteOptions = 0;

	// reset sending of vote options
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(pSelf->m_apPlayers[i])
			pSelf->m_apPlayers[i]->m_SendVoteIndex = 0;
	}
}

void CGameContext::ConVote(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	// check if there is a vote running
	if(!pSelf->m_VoteCloseTime)
		return;

	if(str_comp_nocase(pResult->GetString(0), "yes") == 0)
		pSelf->m_VoteEnforce = CGameContext::VOTE_ENFORCE_YES_ADMIN;
	else if(str_comp_nocase(pResult->GetString(0), "no") == 0)
		pSelf->m_VoteEnforce = CGameContext::VOTE_ENFORCE_NO_ADMIN;
	pSelf->m_VoteEnforcer = pResult->m_ClientID;
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "moderator forced vote %s", pResult->GetString(0));
	pSelf->SendChatTarget(-1, aBuf);
	str_format(aBuf, sizeof(aBuf), "forcing vote %s", pResult->GetString(0));
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
}

void CGameContext::ConchainSpecialMotdupdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
	pfnCallback(pResult, pCallbackUserData);
	if(pResult->NumArguments())
	{
		CNetMsg_Sv_Motd Msg;
		Msg.m_pMessage = g_Config.m_SvMotd;
		CGameContext *pSelf = (CGameContext *)pUserData;
		for(int i = 0; i < MAX_CLIENTS; ++i)
			if(pSelf->m_apPlayers[i])
				pSelf->Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, i);
	}
}

void CGameContext::OnConsoleInit()
{
	m_pServer = Kernel()->RequestInterface<IServer>();
	m_pConsole = Kernel()->RequestInterface<IConsole>();

	m_ChatPrintCBIndex = Console()->RegisterPrintCallback(0, SendChatResponse, this);

	Console()->Register("tune", "si", CFGFLAG_SERVER|CFGFLAG_GAME, ConTuneParam, this, "Tune variable to value");
	Console()->Register("tune_reset", "", CFGFLAG_SERVER, ConTuneReset, this, "Reset tuning");
	Console()->Register("tune_dump", "", CFGFLAG_SERVER, ConTuneDump, this, "Dump tuning");
	Console()->Register("tune_zone", "isi", CFGFLAG_SERVER|CFGFLAG_GAME, ConTuneZone, this, "Tune in zone a variable to value");
	Console()->Register("tune_zone_dump", "i", CFGFLAG_SERVER, ConTuneDumpZone, this, "Dump zone tuning in zone x");
	Console()->Register("tune_zone_reset", "?i", CFGFLAG_SERVER, ConTuneResetZone, this, "reset zone tuning in zone x or in all zones");
	Console()->Register("tune_zone_enter", "is", CFGFLAG_SERVER|CFGFLAG_GAME, ConTuneSetZoneMsgEnter, this, "which message to display on zone enter; use 0 for normal area");
	Console()->Register("tune_zone_leave", "is", CFGFLAG_SERVER|CFGFLAG_GAME, ConTuneSetZoneMsgLeave, this, "which message to display on zone leave; use 0 for normal area");
	Console()->Register("switch_open", "i", CFGFLAG_SERVER|CFGFLAG_GAME, ConSwitchOpen, this, "Whether a switch is open by default (otherwise closed)");
	Console()->Register("pause_game", "", CFGFLAG_SERVER, ConPause, this, "Pause/unpause game");
	Console()->Register("change_map", "?r", CFGFLAG_SERVER|CFGFLAG_STORE, ConChangeMap, this, "Change map");
	Console()->Register("random_map", "?i", CFGFLAG_SERVER, ConRandomMap, this, "Random map");
	Console()->Register("random_unfinished_map", "?i", CFGFLAG_SERVER, ConRandomUnfinishedMap, this, "Random unfinished map");
	Console()->Register("restart", "?i", CFGFLAG_SERVER|CFGFLAG_STORE, ConRestart, this, "Restart in x seconds (0 = abort)");
	Console()->Register("broadcast", "r", CFGFLAG_SERVER, ConBroadcast, this, "Broadcast message");
	Console()->Register("say", "r", CFGFLAG_SERVER, ConSay, this, "Say in chat");
	Console()->Register("set_team", "ii?i", CFGFLAG_SERVER, ConSetTeam, this, "Set team of player to team");
	Console()->Register("set_team_all", "i", CFGFLAG_SERVER, ConSetTeamAll, this, "Set team of all players to team");
	//Console()->Register("swap_teams", "", CFGFLAG_SERVER, ConSwapTeams, this, "Swap the current teams");
	//Console()->Register("shuffle_teams", "", CFGFLAG_SERVER, ConShuffleTeams, this, "Shuffle the current teams");
	//Console()->Register("lock_teams", "", CFGFLAG_SERVER, ConLockTeams, this, "Lock/unlock teams");

	Console()->Register("add_vote", "sr", CFGFLAG_SERVER, ConAddVote, this, "Add a voting option");
	Console()->Register("remove_vote", "s", CFGFLAG_SERVER, ConRemoveVote, this, "remove a voting option");
	Console()->Register("force_vote", "ss?r", CFGFLAG_SERVER, ConForceVote, this, "Force a voting option");
	Console()->Register("clear_votes", "", CFGFLAG_SERVER, ConClearVotes, this, "Clears the voting options");
	Console()->Register("vote", "r", CFGFLAG_SERVER, ConVote, this, "Force a vote to yes/no");

	Console()->Chain("sv_motd", ConchainSpecialMotdupdate, this);

#define CONSOLE_COMMAND(name, params, flags, callback, userdata, help) m_pConsole->Register(name, params, flags, callback, userdata, help);
#include "game/ddracecommands.h"
#define CHAT_COMMAND(name, params, flags, callback, userdata, help) m_pConsole->Register(name, params, flags, callback, userdata, help);
#include "ddracechat.h"
}

void CGameContext::OnInit(/*class IKernel *pKernel*/)
{

	// ChillerDragon
	//Friends_counter = 0;


	m_pServer = Kernel()->RequestInterface<IServer>();
	m_pConsole = Kernel()->RequestInterface<IConsole>();
	m_World.SetGameServer(this);
	m_Events.SetGameServer(this);

	DeleteTempfile();

	//if(!data) // only load once
		//data = load_data_from_memory(internal_data);

	for(int i = 0; i < NUM_NETOBJTYPES; i++)
		Server()->SnapSetStaticsize(i, m_NetObjHandler.GetObjSize(i));

	m_Layers.Init(Kernel());
	m_Collision.Init(&m_Layers);

	// reset everything here
	//world = new GAMEWORLD;
	//players = new CPlayer[MAX_CLIENTS];

	// Reset Tunezones
	CTuningParams TuningParams;
	for (int i = 0; i < NUM_TUNINGZONES; i++)
	{
		TuningList()[i] = TuningParams;
		TuningList()[i].Set("gun_curvature", 0);
		TuningList()[i].Set("gun_speed", 1400);
		TuningList()[i].Set("shotgun_curvature", 0);
		TuningList()[i].Set("shotgun_speed", 500);
		TuningList()[i].Set("shotgun_speeddiff", 0);
	}

	for (int i = 0; i < NUM_TUNINGZONES; i++) // decided to send no text on changing Tunezones for now
	{
		str_format(m_ZoneEnterMsg[i], sizeof(m_ZoneEnterMsg[i]), "", i);
		str_format(m_ZoneLeaveMsg[i], sizeof(m_ZoneLeaveMsg[i]), "", i);
	}
	// Reset Tuning
	if(g_Config.m_SvTuneReset)
	{
		ResetTuning();
	}
	else
	{
		Tuning()->Set("gun_speed", 1400);
		Tuning()->Set("gun_curvature", 0);
		Tuning()->Set("shotgun_speed", 500);
		Tuning()->Set("shotgun_speeddiff", 0);
		Tuning()->Set("shotgun_curvature", 0);
	}

	if(g_Config.m_SvDDRaceTuneReset)
	{
		g_Config.m_SvHit = 1;
		g_Config.m_SvEndlessDrag = 0;
		g_Config.m_SvOldLaser = 0;
		g_Config.m_SvOldTeleportHook = 0;
		g_Config.m_SvOldTeleportWeapons = 0;
		g_Config.m_SvTeleportHoldHook = 0;
		g_Config.m_SvTeam = 1;
		g_Config.m_SvShowOthersDefault = 0;

		if(Collision()->m_NumSwitchers > 0)
			for (int i = 0; i < Collision()->m_NumSwitchers+1; ++i)
				Collision()->m_pSwitchers[i].m_Initial = true;
	}

	Console()->ExecuteFile(g_Config.m_SvResetFile);

	LoadMapSettings();

/*	// select gametype
	if(str_comp(g_Config.m_SvGametype, "mod") == 0)
		m_pController = new CGameControllerMOD(this);
	else if(str_comp(g_Config.m_SvGametype, "ctf") == 0)
		m_pController = new CGameControllerCTF(this);
	else if(str_comp(g_Config.m_SvGametype, "tdm") == 0)
		m_pController = new CGameControllerTDM(this);
	else
		m_pController = new CGameControllerDM(this);*/
	m_pController = new CGameControllerDDRace(this);
	((CGameControllerDDRace*)m_pController)->m_Teams.Reset();

	if(g_Config.m_SvSoloServer)
	{
		g_Config.m_SvTeam = 3;
		g_Config.m_SvShowOthersDefault = 1;

		Tuning()->Set("player_collision", 0);
		Tuning()->Set("player_hooking", 0);

		for (int i = 0; i < NUM_TUNINGZONES; i++)
		{
			TuningList()[i].Set("player_collision", 0);
			TuningList()[i].Set("player_hooking", 0);
		}
	}

	// delete old score object
	if(m_pScore)
		delete m_pScore;

	// create score object (add sql later)
#if defined(CONF_SQL)
	if(g_Config.m_SvUseSQL)
		m_pScore = new CSqlScore(this);
	else
#endif
		m_pScore = new CFileScore(this);
	// setup core world
	//for(int i = 0; i < MAX_CLIENTS; i++)
	//	game.players[i].core.world = &game.world.core;

	// create all entities from the game layer
	CMapItemLayerTilemap *pTileMap = m_Layers.GameLayer();
	CTile *pTiles = (CTile *)Kernel()->RequestInterface<IMap>()->GetData(pTileMap->m_Data);




	/*
	num_spawn_points[0] = 0;
	num_spawn_points[1] = 0;
	num_spawn_points[2] = 0;
	*/

	CTile *pFront = 0;
	CSwitchTile *pSwitch = 0;
	if(m_Layers.FrontLayer())
		pFront = (CTile *)Kernel()->RequestInterface<IMap>()->GetData(m_Layers.FrontLayer()->m_Front);
	if(m_Layers.SwitchLayer())
		pSwitch = (CSwitchTile *)Kernel()->RequestInterface<IMap>()->GetData(m_Layers.SwitchLayer()->m_Switch);

	for(int y = 0; y < pTileMap->m_Height; y++)
	{
		for(int x = 0; x < pTileMap->m_Width; x++)
		{
			int Index = pTiles[y*pTileMap->m_Width+x].m_Index;

			if(Index == TILE_OLDLASER)
			{
				g_Config.m_SvOldLaser = 1;
				dbg_msg("Game Layer", "Found Old Laser Tile");
			}
			else if(Index == TILE_NPC)
			{
				m_Tuning.Set("player_collision", 0);
				dbg_msg("Game Layer", "Found No Collision Tile");
			}
			else if(Index == TILE_EHOOK)
			{
				g_Config.m_SvEndlessDrag = 1;
				dbg_msg("Game Layer", "Found No Unlimited hook time Tile");
			}
			else if(Index == TILE_NOHIT)
			{
				g_Config.m_SvHit = 0;
				dbg_msg("Game Layer", "Found No Weapons Hitting others Tile");
			}
			else if(Index == TILE_NPH)
			{
				m_Tuning.Set("player_hooking", 0);
				dbg_msg("Game Layer", "Found No Player Hooking Tile");
			}

			if(Index >= ENTITY_OFFSET)
			{
				vec2 Pos(x*32.0f+16.0f, y*32.0f+16.0f);
				//m_pController->OnEntity(Index-ENTITY_OFFSET, Pos);
				((CGameControllerDDRace*)m_pController)->OnEntity(Index-ENTITY_OFFSET, Pos);
				m_pController->OnEntity(Index - ENTITY_OFFSET, Pos, LAYER_GAME, pTiles[y * pTileMap->m_Width + x].m_Flags);
			}

			if(pFront)
			{
				Index = pFront[y * pTileMap->m_Width + x].m_Index;
				if(Index == TILE_OLDLASER)
				{
					g_Config.m_SvOldLaser = 1;
					dbg_msg("Front Layer", "Found Old Laser Tile");
				}
				else if(Index == TILE_NPC)
				{
					m_Tuning.Set("player_collision", 0);
					dbg_msg("Front Layer", "Found No Collision Tile");
				}
				else if(Index == TILE_EHOOK)
				{
					g_Config.m_SvEndlessDrag = 1;
					dbg_msg("Front Layer", "Found No Unlimited hook time Tile");
				}
				else if(Index == TILE_NOHIT)
				{
					g_Config.m_SvHit = 0;
					dbg_msg("Front Layer", "Found No Weapons Hitting others Tile");
				}
				else if(Index == TILE_NPH)
				{
					m_Tuning.Set("player_hooking", 0);
					dbg_msg("Front Layer", "Found No Player Hooking Tile");
				}
				if(Index >= ENTITY_OFFSET)
				{
					vec2 Pos(x*32.0f+16.0f, y*32.0f+16.0f);
					m_pController->OnEntity(Index-ENTITY_OFFSET, Pos, LAYER_FRONT, pFront[y*pTileMap->m_Width+x].m_Flags);
				}
			}
			if(pSwitch)
			{
				Index = pSwitch[y*pTileMap->m_Width + x].m_Type;
				// TODO: Add off by default door here
				// if (Index == TILE_DOOR_OFF)
				if(Index >= ENTITY_OFFSET)
				{
					vec2 Pos(x*32.0f+16.0f, y*32.0f+16.0f);
					m_pController->OnEntity(Index-ENTITY_OFFSET, Pos, LAYER_SWITCH, pSwitch[y*pTileMap->m_Width+x].m_Flags, pSwitch[y*pTileMap->m_Width+x].m_Number);
				}
			}
		}
	}

	//game.world.insert_entity(game.Controller);

#ifdef CONF_DEBUG
	if(g_Config.m_DbgDummies)
	{
		for(int i = 0; i < g_Config.m_DbgDummies ; i++)
		{
			OnClientConnected(MAX_CLIENTS-i-1);
		}
	}
#endif
}

void CGameContext::DeleteTempfile()
{
	if(m_aDeleteTempfile[0] != 0)
	{
		IStorage *pStorage = Kernel()->RequestInterface<IStorage>();
		pStorage->RemoveFile(m_aDeleteTempfile, IStorage::TYPE_SAVE);
		m_aDeleteTempfile[0] = 0;
	}
}

void CGameContext::OnMapChange(char *pNewMapName, int MapNameSize)
{
	IStorage *pStorage = Kernel()->RequestInterface<IStorage>();

	char aConfig[128];
	char aTemp[128];
	str_format(aConfig, sizeof(aConfig), "maps/%s.cfg", g_Config.m_SvMap);
	str_format(aTemp, sizeof(aTemp), "%s.temp.%d", pNewMapName, pid());

	IOHANDLE File = pStorage->OpenFile(aConfig, IOFLAG_READ, IStorage::TYPE_ALL);
	if(!File)
	{
		// No map-specific config, just return.
		return;
	}
	CLineReader LineReader;
	LineReader.Init(File);

	array<char *> aLines;
	char *pLine;
	int TotalLength = 0;
	while((pLine = LineReader.Get()))
	{
		int Length = str_length(pLine) + 1;
		char *pCopy = (char *)mem_alloc(Length, 1);
		mem_copy(pCopy, pLine, Length);
		aLines.add(pCopy);
		TotalLength += Length;
	}

	char *pSettings = (char *)mem_alloc(TotalLength, 1);
	int Offset = 0;
	for(int i = 0; i < aLines.size(); i++)
	{
		int Length = str_length(aLines[i]) + 1;
		mem_copy(pSettings + Offset, aLines[i], Length);
		Offset += Length;
		mem_free(aLines[i]);
	}

	CDataFileReader Reader;
	Reader.Open(pStorage, pNewMapName, IStorage::TYPE_ALL);

	CDataFileWriter Writer;
	Writer.Init();

	int SettingsIndex = Reader.NumData();
	bool FoundInfo = false;
	for(int i = 0; i < Reader.NumItems(); i++)
	{
		int TypeID;
		int ItemID;
		int *pData = (int *)Reader.GetItem(i, &TypeID, &ItemID);
		// GetItemSize returns item size including header, remove that.
		int Size = Reader.GetItemSize(i) - sizeof(int) * 2;
		CMapItemInfoSettings MapInfo;
		if(TypeID == MAPITEMTYPE_INFO && ItemID == 0)
		{
			FoundInfo = true;
			CMapItemInfoSettings *pInfo = (CMapItemInfoSettings *)pData;
			if(Size >= (int)sizeof(CMapItemInfoSettings))
			{
				if(pInfo->m_Settings > -1)
				{
					SettingsIndex = pInfo->m_Settings;
					char *pMapSettings = (char *)Reader.GetData(SettingsIndex);
					int DataSize = Reader.GetUncompressedDataSize(SettingsIndex);
					if(DataSize == TotalLength && mem_comp(pSettings, pMapSettings, DataSize) == 0)
					{
						// Configs coincide, no need to update map.
						return;
					}
					Reader.UnloadData(pInfo->m_Settings);
				}
				else
				{
					MapInfo = *pInfo;
					MapInfo.m_Settings = SettingsIndex;
					pData = (int *)&MapInfo;
					Size = sizeof(MapInfo);
				}
			}
			else
			{
				*(CMapItemInfo *)&MapInfo = *(CMapItemInfo *)pInfo;
				MapInfo.m_Settings = SettingsIndex;
				pData = (int *)&MapInfo;
				Size = sizeof(MapInfo);
			}
		}
		Writer.AddItem(TypeID, ItemID, Size, pData);
	}

	if(!FoundInfo)
	{
		CMapItemInfoSettings Info;
		Info.m_Version = 1;
		Info.m_Author = -1;
		Info.m_MapVersion = -1;
		Info.m_Credits = -1;
		Info.m_License = -1;
		Info.m_Settings = SettingsIndex;
		Writer.AddItem(MAPITEMTYPE_INFO, 0, sizeof(Info), &Info);
	}

	for(int i = 0; i < Reader.NumData() || i == SettingsIndex; i++)
	{
		if(i == SettingsIndex)
		{
			Writer.AddData(TotalLength, pSettings);
			continue;
		}
		unsigned char *pData = (unsigned char *)Reader.GetData(i);
		int Size = Reader.GetUncompressedDataSize(i);
		Writer.AddData(Size, pData);
		Reader.UnloadData(i);
	}

	dbg_msg("mapchange", "imported settings");
	Reader.Close();
	Writer.OpenFile(pStorage, aTemp);
	Writer.Finish();

	str_copy(pNewMapName, aTemp, MapNameSize);
	str_copy(m_aDeleteTempfile, aTemp, sizeof(m_aDeleteTempfile));
}

void CGameContext::OnShutdown()
{
	DeleteTempfile();
	Console()->ResetServerGameSettings();
	Layers()->Dest();
	Collision()->Dest();
	delete m_pController;
	m_pController = 0;
	Clear();
}

void CGameContext::LoadMapSettings()
{
	IMap *pMap = Kernel()->RequestInterface<IMap>();
	int Start, Num;
	pMap->GetType(MAPITEMTYPE_INFO, &Start, &Num);
	for(int i = Start; i < Start + Num; i++)
	{
		int ItemID;
		CMapItemInfoSettings *pItem = (CMapItemInfoSettings *)pMap->GetItem(i, 0, &ItemID);
		int ItemSize = pMap->GetItemSize(i) - 8;
		if(!pItem || ItemID != 0)
			continue;

		if(ItemSize < (int)sizeof(CMapItemInfoSettings))
			break;
		if(!(pItem->m_Settings > -1))
			break;

		int Size = pMap->GetUncompressedDataSize(pItem->m_Settings);
		char *pSettings = (char *)pMap->GetData(pItem->m_Settings);
		char *pNext = pSettings;
		while(pNext < pSettings + Size)
		{
			int StrSize = str_length(pNext) + 1;
			Console()->ExecuteLine(pNext, IConsole::CLIENT_ID_GAME);
			pNext += StrSize;
		}
		pMap->UnloadData(pItem->m_Settings);
		break;
	}

	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "maps/%s.map.cfg", g_Config.m_SvMap);
	Console()->ExecuteFile(aBuf, IConsole::CLIENT_ID_NO_GAME);
}

void CGameContext::OnSnap(int ClientID)
{
	// add tuning to demo
	CTuningParams StandardTuning;
	if(ClientID == -1 && Server()->DemoRecorder_IsRecording() && mem_comp(&StandardTuning, &m_Tuning, sizeof(CTuningParams)) != 0)
	{
		CMsgPacker Msg(NETMSGTYPE_SV_TUNEPARAMS);
		int *pParams = (int *)&m_Tuning;
		for(unsigned i = 0; i < sizeof(m_Tuning)/sizeof(int); i++)
			Msg.AddInt(pParams[i]);
		Server()->SendMsg(&Msg, MSGFLAG_RECORD|MSGFLAG_NOSEND, ClientID);
	}

	m_World.Snap(ClientID);
	m_pController->Snap(ClientID);
	m_Events.Snap(ClientID);

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i])
			m_apPlayers[i]->Snap(ClientID);
	}

	if(ClientID > -1)
		m_apPlayers[ClientID]->FakeSnap(ClientID);

}
void CGameContext::OnPreSnap() {}
void CGameContext::OnPostSnap()
{
	m_Events.Clear();
}

bool CGameContext::IsClientReady(int ClientID)
{
	return m_apPlayers[ClientID] && m_apPlayers[ClientID]->m_IsReady ? true : false;
}

bool CGameContext::IsClientPlayer(int ClientID)
{
	return m_apPlayers[ClientID] && m_apPlayers[ClientID]->GetTeam() == TEAM_SPECTATORS ? false : true;
}

const char *CGameContext::GameType() { return m_pController && m_pController->m_pGameType ? m_pController->m_pGameType : ""; }
const char *CGameContext::Version() { return GAME_VERSION; }
const char *CGameContext::NetVersion() { return GAME_NETVERSION; }

IGameServer *CreateGameServer() { return new CGameContext; }

void CGameContext::SendChatResponseAll(const char *pLine, void *pUser) //TODO: schau das an sieht lustig aus
{
	CGameContext *pSelf = (CGameContext *)pUser;

	static volatile int ReentryGuard = 0;
	const char *pLineOrig = pLine;

	if(ReentryGuard)
		return;
	ReentryGuard++;

	if(*pLine == '[')
	do
		pLine++;
	while((pLine - 2 < pLineOrig || *(pLine - 2) != ':') && *pLine != 0);//remove the category (e.g. [Console]: No Such Command)

	pSelf->SendChat(-1, CHAT_ALL, pLine);

	ReentryGuard--;
}

void CGameContext::SendChatResponse(const char *pLine, void *pUser, bool Highlighted)
{
	CGameContext *pSelf = (CGameContext *)pUser;
	int ClientID = pSelf->m_ChatResponseTargetID;

	if(ClientID < 0 || ClientID >= MAX_CLIENTS)
		return;

	const char *pLineOrig = pLine;

	static volatile int ReentryGuard = 0;

	if(ReentryGuard)
		return;
	ReentryGuard++;

	if(*pLine == '[')
	do
		pLine++;
	while((pLine - 2 < pLineOrig || *(pLine - 2) != ':') && *pLine != 0); // remove the category (e.g. [Console]: No Such Command)

	pSelf->SendChatTarget(ClientID, pLine);

	ReentryGuard--;
}

bool CGameContext::PlayerCollision()
{
	float Temp;
	m_Tuning.Get("player_collision", &Temp);
	return Temp != 0.0;
}

bool CGameContext::PlayerHooking()
{
	float Temp;
	m_Tuning.Get("player_hooking", &Temp);
	return Temp != 0.0;
}

float CGameContext::PlayerJetpack()
{
	float Temp;
	m_Tuning.Get("player_jetpack", &Temp);
	return Temp;
}

void CGameContext::OnSetAuthed(int ClientID, int Level)
{
	CServer* pServ = (CServer*)Server();
	if(m_apPlayers[ClientID])
	{
		m_apPlayers[ClientID]->m_Authed = Level;
		char aBuf[512], aIP[NETADDR_MAXSTRSIZE];
		pServ->GetClientAddr(ClientID, aIP, sizeof(aIP));
		str_format(aBuf, sizeof(aBuf), "ban %s %d Banned by vote", aIP, g_Config.m_SvVoteKickBantime);
		if(!str_comp_nocase(m_aVoteCommand, aBuf) && Level > 0)
		{
			m_VoteEnforce = CGameContext::VOTE_ENFORCE_NO_ADMIN;
			Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "CGameContext", "Aborted vote by admin login.");
		}
	}
}

void CGameContext::SendRecord(int ClientID)
{
	CNetMsg_Sv_Record RecordsMsg;
	RecordsMsg.m_PlayerTimeBest = Score()->PlayerData(ClientID)->m_BestTime * 100.0f;
	RecordsMsg.m_ServerTimeBest = m_pController->m_CurrentRecord * 100.0f; //TODO: finish this
	Server()->SendPackMsg(&RecordsMsg, MSGFLAG_VITAL, ClientID);
}

int CGameContext::ProcessSpamProtection(int ClientID)
{
	if(!m_apPlayers[ClientID])
		return 0;
	if(g_Config.m_SvSpamprotection && m_apPlayers[ClientID]->m_LastChat
		&& m_apPlayers[ClientID]->m_LastChat + Server()->TickSpeed() * g_Config.m_SvChatDelay > Server()->Tick())
		return 1;
	else
		m_apPlayers[ClientID]->m_LastChat = Server()->Tick();
	NETADDR Addr;
	Server()->GetClientAddr(ClientID, &Addr);
	int Muted = 0;

	for(int i = 0; i < m_NumMutes && !Muted; i++)
	{
		if(!net_addr_comp(&Addr, &m_aMutes[i].m_Addr))
			Muted = (m_aMutes[i].m_Expire - Server()->Tick()) / Server()->TickSpeed();
	}

	if (Muted > 0)
	{
		char aBuf[128];
		str_format(aBuf, sizeof aBuf, "You are not permitted to talk for the next %d seconds.", Muted);
		SendChatTarget(ClientID, aBuf);
		return 1;
	}

	if ((m_apPlayers[ClientID]->m_ChatScore += g_Config.m_SvChatPenalty) > g_Config.m_SvChatThreshold)
	{
		Mute(0, &Addr, g_Config.m_SvSpamMuteDuration, Server()->ClientName(ClientID));
		m_apPlayers[ClientID]->m_ChatScore = 0;
		return 1;
	}

	return 0;
}

int CGameContext::GetDDRaceTeam(int ClientID)
{
	CGameControllerDDRace* pController = (CGameControllerDDRace*)m_pController;
	return pController->m_Teams.m_Core.Team(ClientID);
}

void CGameContext::ResetTuning()
{
	CTuningParams TuningParams;
	m_Tuning = TuningParams;
	Tuning()->Set("gun_speed", 1400);
	Tuning()->Set("gun_curvature", 0);
	Tuning()->Set("shotgun_speed", 500);
	Tuning()->Set("shotgun_speeddiff", 0);
	Tuning()->Set("shotgun_curvature", 0);
	SendTuningParams(-1);
}

bool CheckClientID2(int ClientID)
{
	dbg_assert(ClientID >= 0 || ClientID < MAX_CLIENTS,
			"The Client ID is wrong");
	if (ClientID < 0 || ClientID >= MAX_CLIENTS)
		return false;
	return true;
}

void CGameContext::Whisper(int ClientID, char *pStr)
{
	char *pName;
	char *pMessage;
	int Error = 0;

	if(ProcessSpamProtection(ClientID))
		return;

	pStr = str_skip_whitespaces(pStr);

	int Victim;

	// add token
	if(*pStr == '"')
	{
		pStr++;

		pName = pStr; // we might have to process escape data
		while(1)
		{
			if(pStr[0] == '"')
				break;
			else if(pStr[0] == '\\')
			{
				if(pStr[1] == '\\')
					pStr++; // skip due to escape
				else if(pStr[1] == '"')
					pStr++; // skip due to escape
			}
			else if(pStr[0] == 0)
				Error = 1;

			pStr++;
		}

		// write null termination
		*pStr = 0;
		pStr++;

		for(Victim = 0; Victim < MAX_CLIENTS; Victim++)
			if (str_comp(pName, Server()->ClientName(Victim)) == 0)
				break;

	}
	else
	{
		pName = pStr;
		while(1)
		{
			if(pStr[0] == 0)
			{
				Error = 1;
				break;
			}
			if(pStr[0] == ' ')
			{
				pStr[0] = 0;
				for(Victim = 0; Victim < MAX_CLIENTS; Victim++)
					if (str_comp(pName, Server()->ClientName(Victim)) == 0)
						break;

				pStr[0] = ' ';

				if (Victim < MAX_CLIENTS)
					break;
			}
			pStr++;
		}
	}

	if(pStr[0] != ' ')
	{
		Error = 1;
	}

	*pStr = 0;
	pStr++;

	pMessage = pStr;

	char aBuf[256];

	if (Error)
	{
		str_format(aBuf, sizeof(aBuf), "Invalid whisper");
		SendChatTarget(ClientID, aBuf);
		return;
	}

	if (Victim >= MAX_CLIENTS || !CheckClientID2(Victim))
	{
		str_format(aBuf, sizeof(aBuf), "No player with name \"%s\" found", pName);
		SendChatTarget(ClientID, aBuf);
		return;
	}

	WhisperID(ClientID, Victim, pMessage);
}


//TEST AREA START
//TESTAREA51


//DRAGON HUGE NUCLEAR TESTS


//WARININGGG


/*





void CGameContext::Playerinfo(int ClientID, char *pStr)
{
	char *pName;
	char *pMessage;
	int Error = 0;

	if (ProcessSpamProtection(ClientID))
		return;

	pStr = str_skip_whitespaces(pStr);

	int Victim;

	// add token
	if (*pStr == '"')
	{
		pStr++;

		pName = pStr; // we might have to process escape data
		while (1)
		{
			if (pStr[0] == '"')
				break;
			else if (pStr[0] == '\\')
			{
				if (pStr[1] == '\\')
					pStr++; // skip due to escape
				else if (pStr[1] == '"')
					pStr++; // skip due to escape
			}
			else if (pStr[0] == 0)
				Error = 1;

			pStr++;
		}

		// write null termination
		*pStr = 0;
		pStr++;

		for (Victim = 0; Victim < MAX_CLIENTS; Victim++)
			if (str_comp(pName, Server()->ClientName(Victim)) == 0)
				break;

	}
	else
	{
		pName = pStr;
		while (1)
		{
			if (pStr[0] == 0)
			{
				Error = 1;
				break;
			}
			if (pStr[0] == ' ')
			{
				pStr[0] = 0;
				for (Victim = 0; Victim < MAX_CLIENTS; Victim++)
					if (str_comp(pName, Server()->ClientName(Victim)) == 0)
						break;

				pStr[0] = ' ';

				if (Victim < MAX_CLIENTS)
					break;
			}
			pStr++;
		}
	}

	if (pStr[0] != ' ')
	{
		Error = 1;
	}

	*pStr = 0;
	pStr++;

	pMessage = pStr;

	char aBuf[256];

	if (Error)
	{
		str_format(aBuf, sizeof(aBuf), "Invalid whisper");
		SendChatTarget(ClientID, aBuf);
		return;
	}

	if (Victim >= MAX_CLIENTS || !CheckClientID2(Victim))
	{
		str_format(aBuf, sizeof(aBuf), "No player with name \"%s\" found", pName);
		SendChatTarget(ClientID, aBuf);
		return;
	}

	WhisperID(ClientID, Victim, pMessage);
}



*/


//TEST AREA 51

//DRAGON HUGE NUCLEAR TEST AREA

//TESTARE END






void CGameContext::WhisperID(int ClientID, int VictimID, char *pMessage)
{
	if (!CheckClientID2(ClientID))
		return;

	if (!CheckClientID2(VictimID))
		return;

	if (m_apPlayers[ClientID])
		m_apPlayers[ClientID]->m_LastWhisperTo = VictimID;

	char aBuf[256];

	if (m_apPlayers[ClientID] && m_apPlayers[ClientID]->m_ClientVersion >= VERSION_DDNET_WHISPER)
	{
		CNetMsg_Sv_Chat Msg;
		Msg.m_Team = CHAT_WHISPER_SEND;
		Msg.m_ClientID = VictimID;
		Msg.m_pMessage = pMessage;
		if(g_Config.m_SvDemoChat)
			Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
		else
			Server()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NORECORD, ClientID);
	}
	else
	{
		str_format(aBuf, sizeof(aBuf), "[→ %s] %s", Server()->ClientName(VictimID), pMessage);
		SendChatTarget(ClientID, aBuf);
	}

	if (m_apPlayers[VictimID] && m_apPlayers[VictimID]->m_ClientVersion >= VERSION_DDNET_WHISPER)
	{
		CNetMsg_Sv_Chat Msg2;
		Msg2.m_Team = CHAT_WHISPER_RECV;
		Msg2.m_ClientID = ClientID;
		Msg2.m_pMessage = pMessage;
		if(g_Config.m_SvDemoChat)
			Server()->SendPackMsg(&Msg2, MSGFLAG_VITAL, VictimID);
		else
			Server()->SendPackMsg(&Msg2, MSGFLAG_VITAL|MSGFLAG_NORECORD, VictimID);
	}
	else
	{
		str_format(aBuf, sizeof(aBuf), "[← %s] %s", Server()->ClientName(ClientID), pMessage);
		SendChatTarget(VictimID, aBuf);
	}
}




void CGameContext::Converse(int ClientID, char *pStr)
{
	CPlayer *pPlayer = m_apPlayers[ClientID];
	if (!pPlayer)
		return;

	if(ProcessSpamProtection(ClientID))
		return;

	if (pPlayer->m_LastWhisperTo < 0)
		SendChatTarget(ClientID, "You do not have an ongoing conversation. Whisper to someone to start one");
	else
	{
		WhisperID(ClientID, pPlayer->m_LastWhisperTo, pStr);
	}
}

void CGameContext::List(int ClientID, const char* filter)
{
	int total = 0;
	char buf[256];
	int bufcnt = 0;
	if (filter[0])
		str_format(buf, sizeof(buf), "Listing players with \"%s\" in name:", filter);
	else
		str_format(buf, sizeof(buf), "Listing all players:", filter);
	SendChatTarget(ClientID, buf);
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i])
		{
			total++;
			const char* name = Server()->ClientName(i);
			if (str_find_nocase(name, filter) == NULL)
				continue;
			if (bufcnt + str_length(name) + 4 > 256)
			{
				SendChatTarget(ClientID, buf);
				bufcnt = 0;
			}
			if (bufcnt != 0)
			{
				str_format(&buf[bufcnt], sizeof(buf) - bufcnt, ", %s", name);
				bufcnt += 2 + str_length(name);
			}
			else
			{
				str_format(&buf[bufcnt], sizeof(buf) - bufcnt, "%s", name);
				bufcnt += str_length(name);
			}
		}
	}
	if (bufcnt != 0)
		SendChatTarget(ClientID, buf);
	str_format(buf, sizeof(buf), "%d players online", total);
	SendChatTarget(ClientID, buf);
}
