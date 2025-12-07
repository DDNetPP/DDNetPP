// gamecontext scoped quests ddnet++ methods

#include "gamecontext.h"

#include <base/log.h>
#include <base/system.h>

#include <engine/shared/config.h>

void CGameContext::QuestReset(int ClientId)
{
	if(!m_apPlayers[ClientId])
	{
		dbg_msg("QUEST", "WARNING! [%d][%s] invalid player. Quest has been reset.", ClientId, Server()->ClientName(ClientId));
		return;
	}
	m_apPlayers[ClientId]->m_QuestProgressValue = 0;
	m_apPlayers[ClientId]->m_QuestProgressValue2 = 0;
	m_apPlayers[ClientId]->m_QuestProgressBool = false;
	m_apPlayers[ClientId]->m_QuestPlayerId = -1;
	m_apPlayers[ClientId]->m_QuestLastQuestedPlayerId = -1;
	m_apPlayers[ClientId]->m_aQuestProgress[0] = -1;
	m_apPlayers[ClientId]->m_aQuestProgress[1] = -1;
}

void CGameContext::QuestFailed(int ClientId)
{
	if(!m_apPlayers[ClientId])
	{
		dbg_msg("QUEST", "WARNING! [%d][%s] invalid player failed the quest.", ClientId, Server()->ClientName(ClientId));
		return;
	}
	if(!m_apPlayers[ClientId]->IsQuesting())
	{
		dbg_msg("QUEST", "WARNING! [%d][%s] failed a quest without being in a quest.", ClientId, Server()->ClientName(ClientId));
		return;
	}
	QuestReset(ClientId);
	if(!m_apPlayers[ClientId]->m_HideQuestWarning)
	{
		SendChatTarget(ClientId, "[QUEST] You failed the quest.");
	}
	StartQuest(ClientId);
}

void CGameContext::QuestFailed2(int ClientId)
{
	if(!m_apPlayers[ClientId])
	{
		dbg_msg("QUEST", "WARNING! [%d][%s] invalid player failed the quest", ClientId, Server()->ClientName(ClientId));
		return;
	}
	if(!m_apPlayers[ClientId]->IsQuesting())
	{
		dbg_msg("QUEST", "WARNING! [%d][%s] failed a quest without being in a quest.", ClientId, Server()->ClientName(ClientId));
		return;
	}
	if(m_apPlayers[ClientId]->m_QuestFailed)
	{
		return;
	}
	m_apPlayers[ClientId]->m_QuestFailed = true;
	//str_format(m_apPlayers[ClientId]->m_aQuestString, sizeof(m_apPlayers[ClientId]->m_aQuestString), "Quest failed."); // dont overwrite info what to do and how to start again. I added a questfailed info in ddracechat.cpp
	QuestReset(ClientId);
	if(!m_apPlayers[ClientId]->m_HideQuestWarning)
	{
		SendChatTarget(ClientId, "[QUEST] You failed the quest.");
	}
}

bool CGameContext::QuestAddProgress(int ClientId, int GlobalMax, int LocalMax)
{
	char aBuf[256];
	if(!m_apPlayers[ClientId])
	{
		dbg_msg("QUEST", "WARNING! [%d][%s] invalid player added progress.", ClientId, Server()->ClientName(ClientId));
		return false;
	}
	if(LocalMax == -1)
	{
		LocalMax = GlobalMax;
	}
	if(m_apPlayers[ClientId]->m_QuestProgressValue >= LocalMax)
	{
		return false;
	}

	m_apPlayers[ClientId]->m_QuestProgressValue++;
	m_apPlayers[ClientId]->m_aQuestProgress[0] = m_apPlayers[ClientId]->m_QuestProgressValue;
	m_apPlayers[ClientId]->m_aQuestProgress[1] = GlobalMax;

	//dbg_msg("QUEST", "Progress updated: %d/%d", m_apPlayers[ClientId]->m_aQuestProgress[0], m_apPlayers[ClientId]->m_aQuestProgress[1]);
	str_format(aBuf, sizeof(aBuf), "[QUEST] progress %d/%d", m_apPlayers[ClientId]->m_QuestProgressValue, GlobalMax);

	if(!m_apPlayers[ClientId]->m_HideQuestProgress)
		SendChatTarget(ClientId, aBuf);

	if(m_apPlayers[ClientId]->m_QuestProgressValue >= GlobalMax)
	{
		QuestCompleted(ClientId);
	}

	return true;
}

void CGameContext::QuestCompleted(int ClientId)
{
	CPlayer *pPlayer = m_apPlayers[ClientId];
	if(!pPlayer)
	{
		dbg_msg("QUEST", "WARNING! [%d][%s] invalid player completed the quest", ClientId, Server()->ClientName(ClientId));
		return;
	}
	if(!pPlayer->IsQuesting())
	{
		dbg_msg("QUEST", "Warning! [%d][%s] completed quest without having it activated", pPlayer->GetCid(), Server()->ClientName(pPlayer->GetCid()));
		return;
	}

	// reward
	char aBuf[256];
	int RewardMoney = pPlayer->m_QuestStateLevel ? 100 : 50;
	int RewardXP = QuestReward(ClientId);
	if(pPlayer->IsMaxLevel())
		str_format(aBuf, sizeof(aBuf), "[QUEST] Quest %d (lvl %d) completed. [+%d money]", pPlayer->m_QuestState, pPlayer->m_QuestStateLevel, RewardMoney);
	else // xp msg only if not max lvl
		str_format(aBuf, sizeof(aBuf), "[QUEST] Quest %d (lvl %d) completed. [+%d xp] [+%d money]", pPlayer->m_QuestState, pPlayer->m_QuestStateLevel, RewardXP, RewardMoney);
	SendChatTarget(ClientId, aBuf);
	pPlayer->MoneyTransaction(+100, "quest reward");
	pPlayer->GiveXP(RewardXP);

	// next quest
	QuestReset(ClientId);
	pPlayer->m_QuestState++;
	pPlayer->m_QuestUnlocked = pPlayer->m_QuestState; //save highscore
	if(pPlayer->m_QuestState > CPlayer::QUEST_NUM)
	{
		pPlayer->m_QuestState = CPlayer::QUEST_HAMMER; // start at quest 1 in the next level
		pPlayer->m_QuestStateLevel++;
		str_format(aBuf, sizeof(aBuf), "[QUEST] level up... you are now level %d !", pPlayer->m_QuestStateLevel);
		if(pPlayer->m_QuestStateLevel > CPlayer::QUEST_NUM_LEVEL)
		{
			pPlayer->m_QuestState = CPlayer::QUEST_OFF;
			pPlayer->m_QuestStateLevel = 0;
			SendChatTarget(ClientId, "[QUEST] Hurray you finished all Quests !!!");
			return;
		}
		pPlayer->m_QuestLevelUnlocked = pPlayer->m_QuestStateLevel; // save highscore
		SendChatTarget(ClientId, aBuf);
	}
	StartQuest(ClientId);
}

int CGameContext::QuestReward(int ClientId)
{
	if(!m_apPlayers[ClientId])
	{
		return 0;
	}

	if(!m_apPlayers[ClientId]->m_QuestStateLevel)
	{
		return 10;
	}
	else
	{
		return m_apPlayers[ClientId]->m_QuestStateLevel * 100;
	}
}

//void CGameContext::PickNextQuest(int ClientId)
//{
//#if defined(CONF_DEBUG)
//#endif
//	m_apPlayers[ClientId]->m_QuestState++;
//	StartQuest(ClientId);
//}

void CGameContext::StartQuest(int ClientId)
{
	if(!m_apPlayers[ClientId])
	{
		dbg_msg("QUEST", "WARNING! [%d][%s] invalid player has started the quest.", ClientId, Server()->ClientName(ClientId));
		return;
	}

	char aBuf[256];
	QuestReset(ClientId); //not needed but save clearup (should already be cleared up on every quest exit but save is save)
	int Level = m_apPlayers[ClientId]->m_QuestStateLevel;
	int Quest = m_apPlayers[ClientId]->m_QuestState; //old and bad because with many quests this can take forever and easts resources of server or players have to do quests over and over again rand() % 4 + 1; //valid quests + 1
	str_copy(m_apPlayers[ClientId]->m_aQuestString, "ERROR invalid quest loaded");

	if(Quest == 0)
	{
		dbg_msg("debug", "WARNING: QuestPicker triggered on non-questing player [%d][%s] [QUEST=%d LEVEL=%d]", m_apPlayers[ClientId]->GetCid(), Server()->ClientName(m_apPlayers[ClientId]->GetCid()), Quest, Level);
		return;
	}
	else if(Quest == 1)
	{
		if(Level == 0)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Hammer 1 tee.");
		}
		else if(Level == 1)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Hammer 2 tees.");
		}
		else if(Level == 2)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Hammer 3 tees.");
		}
		else if(Level == 3)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Hammer 5 tees.");
		}
		else if(Level == 4)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Hammer 10 freezed tees.");
		}
		else if(Level == 5)
		{
			str_format(m_apPlayers[ClientId]->m_aQuestString, sizeof(m_apPlayers[ClientId]->m_aQuestString), "Hammer '%s' 20 times.", Server()->ClientName(PickQuestPlayer(ClientId)));
		}
		else if(Level == 6)
		{
			str_format(m_apPlayers[ClientId]->m_aQuestString, sizeof(m_apPlayers[ClientId]->m_aQuestString), "Hammer freezed '%s' 3 times.", Server()->ClientName(PickQuestPlayer(ClientId)));
		}
		else if(Level == 7)
		{
			str_format(m_apPlayers[ClientId]->m_aQuestString, sizeof(m_apPlayers[ClientId]->m_aQuestString), "Hammer '%s' 10 times and then block him.", Server()->ClientName(PickQuestPlayer(ClientId)));
		}
		else if(Level == 8)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Hammer 2 tees with one hit.");
		}
		else if(Level == 9)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Hammer 10 freezed tees while holding the flag.");
		}
		else
		{
			dbg_msg("debug", "ERROR: invalid quest level [QUEST=%d LEVEL=%d]", Quest, Level);
			Quest = 0;
		}
	}
	else if(Quest == 2)
	{
		if(Level == 0)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Block 1 tee.");
		}
		else if(Level == 1)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Block 2 tees.");
		}
		else if(Level == 2)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Block 3 tees.");
		}
		else if(Level == 3)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Block 5 tees.");
		}
		else if(Level == 4)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Block 10 tees without using any weapons.");
		}
		else if(Level == 5)
		{
			str_format(m_apPlayers[ClientId]->m_aQuestString, sizeof(m_apPlayers[ClientId]->m_aQuestString), "Block 5 tees and then block '%s'.", Server()->ClientName(PickQuestPlayer(ClientId)));
		}
		else if(Level == 6)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Block a tee which is on a 5 blockingspree.");
		}
		else if(Level == 7)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Block 11 tees without getting blocked.");
		}
		else if(Level == 8)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Block 3 tees without using hook.");
		}
		else if(Level == 9)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Block 11 tees while holding the flag without dying.");
		}
		else
		{
			dbg_msg("debug", "ERROR: invalid quest level [QUEST=%d LEVEL=%d]", Quest, Level);
			Quest = 0;
		}
	}
	else if(Quest == 3)
	{
		if(Level == 0)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Finish the race.", sizeof(m_apPlayers[ClientId]->m_aQuestString));
		}
		else if(Level == 1)
		{
			str_format(m_apPlayers[ClientId]->m_aQuestString, sizeof(m_apPlayers[ClientId]->m_aQuestString), "Finish the race in under %d seconds.", g_Config.m_SvQuestRaceTime1);
		}
		else if(Level == 2)
		{
			str_format(m_apPlayers[ClientId]->m_aQuestString, sizeof(m_apPlayers[ClientId]->m_aQuestString), "Finish the race in under %d seconds.", g_Config.m_SvQuestRaceTime2);
		}
		else if(Level == 3)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Finish the race backwards.");
		}
		else if(Level == 4)
		{
			str_format(m_apPlayers[ClientId]->m_aQuestString, sizeof(m_apPlayers[ClientId]->m_aQuestString), "Finish the race in under %d seconds.", g_Config.m_SvQuestRaceTime3);
		}
		else if(Level == 5)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Finish the race with the flag.");
		}
		else if(Level == 6)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Finish the special race.");
		}
		else if(Level == 7)
		{
			str_format(m_apPlayers[ClientId]->m_aQuestString, sizeof(m_apPlayers[ClientId]->m_aQuestString), "Finish the special race in under %d seconds.", g_Config.m_SvQuestSpecialRaceTime);
		}
		else if(Level == 8)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Finish the special race backwards.");
		}
		else if(Level == 9)
		{
			if(g_Config.m_SvQuestRaceCondition == 0)
			{
				str_copy(m_apPlayers[ClientId]->m_aQuestString, "Finish the race without using hammer.");
			}
			else if(g_Config.m_SvQuestRaceCondition == 1)
			{
				str_copy(m_apPlayers[ClientId]->m_aQuestString, "Finish the race without using gun.");
			}
			else if(g_Config.m_SvQuestRaceCondition == 2)
			{
				str_copy(m_apPlayers[ClientId]->m_aQuestString, "Finish the race without using shotgun.");
			}
			else if(g_Config.m_SvQuestRaceCondition == 3)
			{
				str_copy(m_apPlayers[ClientId]->m_aQuestString, "Finish the race without using grenade.");
			}
			else if(g_Config.m_SvQuestRaceCondition == 4)
			{
				str_copy(m_apPlayers[ClientId]->m_aQuestString, "Finish the race without using rifle.");
			}
			else if(g_Config.m_SvQuestRaceCondition == 5)
			{
				str_copy(m_apPlayers[ClientId]->m_aQuestString, "Finish the race without using ninja.");
			}
			else
			{
				dbg_msg("debug", "ERROR: invalid race condition [%d] at [QUEST=%d LEVEL=%d]", g_Config.m_SvQuestRaceCondition, Quest, Level);
				Quest = 0;
			}
		}
		else
		{
			dbg_msg("debug", "ERROR: invalid quest level [QUEST=%d LEVEL=%d]", Quest, Level);
			Quest = 0;
		}
	}
	else if(Quest == 4)
	{
		if(Level == 0)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Rifle 1 tee", sizeof(m_apPlayers[ClientId]->m_aQuestString));
		}
		else if(Level == 1)
		{
			str_format(m_apPlayers[ClientId]->m_aQuestString, sizeof(m_apPlayers[ClientId]->m_aQuestString), "Rifle '%s' 5 times", Server()->ClientName(PickQuestPlayer(ClientId)));
		}
		else if(Level == 2)
		{
			str_format(m_apPlayers[ClientId]->m_aQuestString, sizeof(m_apPlayers[ClientId]->m_aQuestString), "Rifle freezed '%s' 5 times", Server()->ClientName(PickQuestPlayer(ClientId)));
		}
		else if(Level == 3)
		{
			str_format(m_apPlayers[ClientId]->m_aQuestString, sizeof(m_apPlayers[ClientId]->m_aQuestString), "Rifle '%s' and 10 others", Server()->ClientName(PickQuestPlayer(ClientId)));
		}
		else if(Level == 4)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Rifle 10 freezed tees");
		}
		else if(Level == 5)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Rifle yourself while being freezed");
		}
		else if(Level == 6)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Rifle yourself while being freezed 10 times");
		}
		else if(Level == 7)
		{
			str_format(m_apPlayers[ClientId]->m_aQuestString, sizeof(m_apPlayers[ClientId]->m_aQuestString), "Rifle '%s' and then block him", Server()->ClientName(PickQuestPlayer(ClientId)));
		}
		else if(Level == 8)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Rifle 5 tees before blocking them");
		}
		else if(Level == 9)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Rifle 20 freezed tees while having the flag");
		}
		else
		{
			dbg_msg("debug", "ERROR: invalid quest level [QUEST=%d LEVEL=%d]", Quest, Level);
			Quest = 0;
		}
	}
	else if(Quest == 5)
	{
		if(Level == 0)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Farm 10 money on a moneytile");
		}
		else if(Level == 1)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Farm 20 money on a moneytile");
		}
		else if(Level == 2)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Farm 30 money on a moneytile");
		}
		else if(Level == 3)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Farm 40 money on a moneytile");
		}
		else if(Level == 4)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Farm 50 money on a moneytile");
		}
		else if(Level == 5)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Farm 60 money on a moneytile");
		}
		else if(Level == 6)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Farm 70 money on a moneytile");
		}
		else if(Level == 7)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Farm 100 money on a police moneytile");
		}
		else if(Level == 8)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Farm 100 money on a moneytile");
		}
		else if(Level == 9)
		{
			str_copy(m_apPlayers[ClientId]->m_aQuestString, "Farm 200 xp with the flag");
		}
		else
		{
			log_error("quest", "ERROR: invalid quest level [QUEST=%d LEVEL=%d]", Quest, Level);
			Quest = 0;
		}
	}
	else
	{
		log_error("quest", "ERROR: invalid quest [QUEST=%d LEVEL=%d]", Quest, Level);
		Quest = 0;
	}

	if(m_apPlayers[ClientId]->IsQuesting() && Quest)
	{
		str_format(aBuf, sizeof(aBuf), "[QUEST] %s", m_apPlayers[ClientId]->m_aQuestString);
		SendBroadcast(aBuf, m_apPlayers[ClientId]->GetCid());
		str_format(aBuf, sizeof(aBuf), "[QUEST] New Quest: %s", m_apPlayers[ClientId]->m_aQuestString);
		SendChatTarget(m_apPlayers[ClientId]->GetCid(), aBuf);
		return;
	}

	//quest stopped during the next quest election
	SendBroadcast("[QUEST] stopped", m_apPlayers[ClientId]->GetCid());
	m_apPlayers[ClientId]->m_QuestState = CPlayer::QUEST_OFF;
}

int CGameContext::PickQuestPlayer(int ClientId)
{
	if(!m_apPlayers[ClientId])
	{
		dbg_msg("QUEST", "WARNING! [%d][%s] invalid player picked a quest", ClientId, Server()->ClientName(ClientId));
		return -1;
	}

	int Id = -1;
	int FoundTees[MAX_CLIENTS];
	int Index = 0;
	int FoundDeadTees[MAX_CLIENTS];
	int IndexDead = 0;

	for(CPlayer *pPlayer : m_apPlayers)
	{
		if(!pPlayer)
		{
			// dbg_msg("QUEST", "<PickPlayer> warning not existing player found");
			continue;
		}
		if(pPlayer->GetTeam() == TEAM_SPECTATORS)
		{
			dbg_msg("QUEST", "<PickPlayer> warning spec player found");
			continue;
		}
		if(IsSameIp(pPlayer->GetCid(), ClientId))
		{
			// dummy found (also used to ignore the questing player it self. Keep this in mind if you remove or edit this one day)
			// this will be triggered for all serverside dummys if ur playing local -.-
			dbg_msg("QUEST", "<PickPlayer> warning dummy found [%s]", Server()->ClientName(pPlayer->GetCid()));
			continue;
		}
		if(pPlayer->m_IsDummy && !g_Config.m_SvQuestCountBots)
		{
			//server side bot found
			dbg_msg("QUEST", "<PickPlayer> warning found bot [%s]", Server()->ClientName(pPlayer->GetCid()));
			continue;
		}
		if(GetDDRaceTeam(pPlayer->GetCid()))
		{
			continue;
		}

		//found valid non dummy or serverside bot
		if(!GetPlayerChar(pPlayer->GetCid())) //spec/dead players always second choice
		{
			// ---> store id + 1 in array so no 0 is in array and we can check if a tee is existing and stuff
			FoundDeadTees[Index] = pPlayer->GetCid() + 1;
			IndexDead++;
			//dbg_msg("QUEST", "+1 dead player");
		}
		else //alive players primary choice
		{
			// ---> store id + 1 in array so no 0 is in array and we can check if a tee is existing and stuff
			FoundTees[Index] = pPlayer->GetCid() + 1;
			Index++;
			// dbg_msg("QUEST", "+1 alive player");
		}
	}

	if(Index < g_Config.m_SvQuestNeededPlayers) // not enough alive tees ---> check spectators
	{
		if(Index + IndexDead < g_Config.m_SvQuestNeededPlayers) // not enough dead or alive valid tees --> stop quest
		{
			m_apPlayers[ClientId]->m_QuestState = CPlayer::QUEST_OFF;
			SendChatTarget(ClientId, "[QUEST] Quest stopped because there are not enough tees on the server.");
			// dbg_msg("QUEST", "alive %d + dead %d = %d/%d tees to start a quest", Index, IndexDead, Index + IndexDead, g_Config.m_SvQuestNeededPlayers);
			return -1;
		}
		else
		{
			Id = FoundDeadTees[rand() % IndexDead];
			if(!Id)
			{
				dbg_msg("QUEST", "WARNING! player [%d][%s] got invalid player [%d][%s] as specific quest", ClientId, Server()->ClientName(ClientId), Id, Server()->ClientName(Id));
				m_apPlayers[ClientId]->m_QuestState = CPlayer::QUEST_OFF;
				SendChatTarget(ClientId, "[QUEST] Quest stopped because something went wrong. (please contact an admin)");
				SendChatTarget(ClientId, "[QUEST] Try '/quest start' again to load and start your quest again");
				return -1;
			}

			m_apPlayers[ClientId]->m_QuestPlayerId = Id - 1;
			return Id - 1; // before we stored id + 1 to have an better handling with false values
		}
	}

	Id = FoundTees[rand() % Index];
	if(!Id)
	{
		dbg_msg("QUEST", "WARNING! player [%d][%s] got invalid player [%d][%s] as specific quest", ClientId, Server()->ClientName(ClientId), Id, Server()->ClientName(Id));
		m_apPlayers[ClientId]->m_QuestState = CPlayer::QUEST_OFF;
		SendChatTarget(ClientId, "[QUEST] Quest stopped because something went wrong. (please contact an admin)");
		SendChatTarget(ClientId, "[QUEST] Try '/quest start' again to load and start your quest again");
		return -1;
	}

	m_apPlayers[ClientId]->m_QuestPlayerId = Id - 1;
	return Id - 1; // before we stored id + 1 to have an better handling with false values
}

void CGameContext::CheckConnectQuestBot()
{
	int NumQuestBots = 0;
	int NumQuestPlayers = 0;
	int NumConnectedPlayers = 0;
	int NumIngameHumans = 0;
	for(auto &Player : m_apPlayers)
	{
		if(!Player)
			continue;

		NumConnectedPlayers++;
		if(Player->IsQuesting())
			NumQuestPlayers++;

		if(!Player->GetCharacter())
			continue;

		if(!Player->m_IsDummy)
			NumIngameHumans++;
		else if(Player->DummyMode() == DUMMYMODE_QUEST)
			NumQuestBots++;
	}

	if(NumQuestBots > NumQuestPlayers)
		return;
	if(NumQuestBots > 3)
		return;
	if(NumIngameHumans > 3)
		return;
	if(NumConnectedPlayers + 3 > g_Config.m_SvMaxClients)
		return;

	CreateNewDummy(DUMMYMODE_QUEST);
}
