// gamecontext scoped quests ddnet++ methods

#include <engine/shared/config.h>

#include "gamecontext.h"

void CGameContext::QuestReset(int playerID)
{
	if(!m_apPlayers[playerID])
	{
		dbg_msg("QUEST", "WARNING! [%d][%s] invalid player. Quest has been reset.", playerID, Server()->ClientName(playerID));
		return;
	}
	m_apPlayers[playerID]->m_QuestProgressValue = 0;
	m_apPlayers[playerID]->m_QuestProgressValue2 = 0;
	m_apPlayers[playerID]->m_QuestProgressBool = 0;
	m_apPlayers[playerID]->m_QuestPlayerID = -1;
	m_apPlayers[playerID]->m_QuestLastQuestedPlayerID = -1;
	m_apPlayers[playerID]->m_aQuestProgress[0] = -1;
	m_apPlayers[playerID]->m_aQuestProgress[1] = -1;
}

void CGameContext::QuestFailed(int playerID)
{
	if(!m_apPlayers[playerID])
	{
		dbg_msg("QUEST", "WARNING! [%d][%s] invalid player failed the quest.", playerID, Server()->ClientName(playerID));
		return;
	}
	if(!m_apPlayers[playerID]->IsQuesting())
	{
		dbg_msg("QUEST", "WARNING! [%d][%s] failed a quest without being in a quest.", playerID, Server()->ClientName(playerID));
		return;
	}
	QuestReset(playerID);
	if(!m_apPlayers[playerID]->m_HideQuestWarning)
	{
		SendChatTarget(playerID, "[QUEST] You failed the quest.");
	}
	StartQuest(playerID);
}

void CGameContext::QuestFailed2(int playerID)
{
	if(!m_apPlayers[playerID])
	{
		dbg_msg("QUEST", "WARNING! [%d][%s] invalid player failed the quest", playerID, Server()->ClientName(playerID));
		return;
	}
	if(!m_apPlayers[playerID]->IsQuesting())
	{
		dbg_msg("QUEST", "WARNING! [%d][%s] failed a quest without being in a quest.", playerID, Server()->ClientName(playerID));
		return;
	}
	if(m_apPlayers[playerID]->m_QuestFailed)
	{
		return;
	}
	m_apPlayers[playerID]->m_QuestFailed = true;
	//str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Quest failed."); //dont overwrite info what to do and how to start agian. I added a questfailed info in ddracechat.cpp
	QuestReset(playerID);
	if(!m_apPlayers[playerID]->m_HideQuestWarning)
	{
		SendChatTarget(playerID, "[QUEST] You failed the quest.");
	}
}

bool CGameContext::QuestAddProgress(int playerID, int globalMAX, int localMAX)
{
	char aBuf[256];
	if(!m_apPlayers[playerID])
	{
		dbg_msg("QUEST", "WARNING! [%d][%s] invalid player added progress.", playerID, Server()->ClientName(playerID));
		return false;
	}
	if(localMAX == -1)
	{
		localMAX = globalMAX;
	}
	if(m_apPlayers[playerID]->m_QuestProgressValue >= localMAX)
	{
		return false;
	}

	m_apPlayers[playerID]->m_QuestProgressValue++;
	m_apPlayers[playerID]->m_aQuestProgress[0] = m_apPlayers[playerID]->m_QuestProgressValue;
	m_apPlayers[playerID]->m_aQuestProgress[1] = globalMAX;

	//dbg_msg("QUEST", "Progress updated: %d/%d", m_apPlayers[playerID]->m_aQuestProgress[0], m_apPlayers[playerID]->m_aQuestProgress[1]);
	str_format(aBuf, sizeof(aBuf), "[QUEST] progress %d/%d", m_apPlayers[playerID]->m_QuestProgressValue, globalMAX);

	if(!m_apPlayers[playerID]->m_HideQuestProgress)
		SendChatTarget(playerID, aBuf);

	if(m_apPlayers[playerID]->m_QuestProgressValue >= globalMAX)
	{
		QuestCompleted(playerID);
	}

	return true;
}

void CGameContext::QuestCompleted(int playerID)
{
	CPlayer *pPlayer = m_apPlayers[playerID];
	if(!pPlayer)
	{
		dbg_msg("QUEST", "WARNING! [%d][%s] invalid player completed the quest", playerID, Server()->ClientName(playerID));
		return;
	}
	if(!pPlayer->IsQuesting())
	{
		dbg_msg("QUEST", "Warning! [%d][%s] completed quest without having it activated", pPlayer->GetCID(), Server()->ClientName(pPlayer->GetCID()));
		return;
	}

	// reward
	char aBuf[256];
	int RewardMoney = pPlayer->m_QuestStateLevel ? 100 : 50;
	int RewardXP = QuestReward(playerID);
	if(pPlayer->IsMaxLevel())
		str_format(aBuf, sizeof(aBuf), "[QUEST] Quest %d (lvl %d) completed. [+%d money]", pPlayer->m_QuestState, pPlayer->m_QuestStateLevel, RewardMoney);
	else // xp msg only if not max lvl
		str_format(aBuf, sizeof(aBuf), "[QUEST] Quest %d (lvl %d) completed. [+%d xp] [+%d money]", pPlayer->m_QuestState, pPlayer->m_QuestStateLevel, RewardXP, RewardMoney);
	SendChatTarget(playerID, aBuf);
	pPlayer->MoneyTransaction(+100, "quest reward");
	pPlayer->GiveXP(RewardXP);

	// next quest
	QuestReset(playerID);
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
			SendChatTarget(playerID, "[QUEST] Hurray you finished all Quests !!!");
			return;
		}
		pPlayer->m_QuestLevelUnlocked = pPlayer->m_QuestStateLevel; // save highscore
		SendChatTarget(playerID, aBuf);
	}
	StartQuest(playerID);
}

int CGameContext::QuestReward(int playerID)
{
	if(!m_apPlayers[playerID])
	{
		return 0;
	}

	if(!m_apPlayers[playerID]->m_QuestStateLevel)
	{
		return 10;
	}
	else
	{
		return m_apPlayers[playerID]->m_QuestStateLevel * 100;
	}
}

//void CGameContext::PickNextQuest(int playerID)
//{
//#if defined(CONF_DEBUG)
//#endif
//	m_apPlayers[playerID]->m_QuestState++;
//	StartQuest(playerID);
//}

void CGameContext::StartQuest(int playerID)
{
	if(!m_apPlayers[playerID])
	{
		dbg_msg("QUEST", "WARNING! [%d][%s] invalid player has started the quest.", playerID, Server()->ClientName(playerID));
		return;
	}

	char aBuf[256];
	QuestReset(playerID); //not needed but save clearup (should already be cleared up on every quest exit but save is save)
	int level = m_apPlayers[playerID]->m_QuestStateLevel;
	int quest = m_apPlayers[playerID]->m_QuestState; //old and bad because with many quests this can take forever and easts ressources of server or players have to do quests over and over agian rand() % 4 + 1; //valid quests + 1
	str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "ERROR invalid quest loaded");

	if(quest == 0)
	{
		dbg_msg("debug", "WARNING: QuestPicker triggered on non-questing player [%d][%s] [QUEST=%d LEVEL=%d]", m_apPlayers[playerID]->GetCID(), Server()->ClientName(m_apPlayers[playerID]->GetCID()), quest, level);
		return;
	}
	else if(quest == 1)
	{
		if(level == 0)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Hammer 1 tee.");
		}
		else if(level == 1)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Hammer 2 tees.");
		}
		else if(level == 2)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Hammer 3 tees.");
		}
		else if(level == 3)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Hammer 5 tees.");
		}
		else if(level == 4)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Hammer 10 freezed tees.");
		}
		else if(level == 5)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Hammer '%s' 20 times.", Server()->ClientName(PickQuestPlayer(playerID)));
		}
		else if(level == 6)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Hammer freezed '%s' 3 times.", Server()->ClientName(PickQuestPlayer(playerID)));
		}
		else if(level == 7)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Hammer '%s' 10 times and then block him.", Server()->ClientName(PickQuestPlayer(playerID)));
		}
		else if(level == 8)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Hammer 2 tees with one hit.");
		}
		else if(level == 9)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Hammer 10 freezed tees while holding the flag.");
		}
		else
		{
			dbg_msg("debug", "ERROR: invalid quest level [QUEST=%d LEVEL=%d]", quest, level);
			quest = 0;
		}
	}
	else if(quest == 2)
	{
		if(level == 0)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Block 1 tee.");
		}
		else if(level == 1)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Block 2 tees.");
		}
		else if(level == 2)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Block 3 tees.");
		}
		else if(level == 3)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Block 5 tees.");
		}
		else if(level == 4)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Block 10 tees without using any weapons.");
		}
		else if(level == 5)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Block 5 tees and then block '%s'.", Server()->ClientName(PickQuestPlayer(playerID)));
		}
		else if(level == 6)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Block a tee which is on a 5 blockingspree.");
		}
		else if(level == 7)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Block 11 tees without getting blocked.");
		}
		else if(level == 8)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Block 3 tees without using hook.");
		}
		else if(level == 9)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Block 11 tees while holding the flag without dying.");
		}
		else
		{
			dbg_msg("debug", "ERROR: invalid quest level [QUEST=%d LEVEL=%d]", quest, level);
			quest = 0;
		}
	}
	else if(quest == 3)
	{
		if(level == 0)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the race.");
		}
		else if(level == 1)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the race in under %d seconds.", g_Config.m_SvQuestRaceTime1);
		}
		else if(level == 2)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the race in under %d seconds.", g_Config.m_SvQuestRaceTime2);
		}
		else if(level == 3)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the race backwards.");
		}
		else if(level == 4)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the race in under %d seconds.", g_Config.m_SvQuestRaceTime3);
		}
		else if(level == 5)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the race with the flag.");
		}
		else if(level == 6)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the special race.");
		}
		else if(level == 7)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the special race in under %d seconds.", g_Config.m_SvQuestSpecialRaceTime);
		}
		else if(level == 8)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the special race backwards.");
		}
		else if(level == 9)
		{
			if(g_Config.m_SvQuestRaceCondition == 0)
			{
				str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the race without using hammer.");
			}
			else if(g_Config.m_SvQuestRaceCondition == 1)
			{
				str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the race without using gun.");
			}
			else if(g_Config.m_SvQuestRaceCondition == 2)
			{
				str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the race without using shotgun.");
			}
			else if(g_Config.m_SvQuestRaceCondition == 3)
			{
				str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the race without using grenade.");
			}
			else if(g_Config.m_SvQuestRaceCondition == 4)
			{
				str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the race without using rifle.");
			}
			else if(g_Config.m_SvQuestRaceCondition == 5)
			{
				str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the race without using ninja.");
			}
			else
			{
				dbg_msg("debug", "ERROR: invalid race condition [%d] at [QUEST=%d LEVEL=%d]", g_Config.m_SvQuestRaceCondition, quest, level);
				quest = 0;
			}
		}
		else
		{
			dbg_msg("debug", "ERROR: invalid quest level [QUEST=%d LEVEL=%d]", quest, level);
			quest = 0;
		}
	}
	else if(quest == 4)
	{
		if(level == 0)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Rifle 1 tee");
		}
		else if(level == 1)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Rifle '%s' 5 times", Server()->ClientName(PickQuestPlayer(playerID)));
		}
		else if(level == 2)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Rifle freezed '%s' 5 times", Server()->ClientName(PickQuestPlayer(playerID)));
		}
		else if(level == 3)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Rifle '%s' and 10 others", Server()->ClientName(PickQuestPlayer(playerID)));
		}
		else if(level == 4)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Rifle 10 freezed tees");
		}
		else if(level == 5)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Rifle yourself while being freezed");
		}
		else if(level == 6)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Rifle yourself while being freezed 10 times");
		}
		else if(level == 7)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Rifle '%s' and then block him", Server()->ClientName(PickQuestPlayer(playerID)));
		}
		else if(level == 8)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Rifle 5 tees before blocking them");
		}
		else if(level == 9)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Rifle 20 freezed tees while having the flag");
		}
		else
		{
			dbg_msg("debug", "ERROR: invalid quest level [QUEST=%d LEVEL=%d]", quest, level);
			quest = 0;
		}
	}
	else if(quest == 5)
	{
		if(level == 0)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Farm 10 money on a moneytile");
		}
		else if(level == 1)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Farm 20 money on a moneytile");
		}
		else if(level == 2)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Farm 30 money on a moneytile");
		}
		else if(level == 3)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Farm 40 money on a moneytile");
		}
		else if(level == 4)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Farm 50 money on a moneytile");
		}
		else if(level == 5)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Farm 60 money on a moneytile");
		}
		else if(level == 6)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Farm 70 money on a moneytile");
		}
		else if(level == 7)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Farm 100 money on a police moneytile");
		}
		else if(level == 8)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Farm 100 money on a moneytile");
		}
		else if(level == 9)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Farm 200 xp with the flag");
		}
		else
		{
			dbg_msg("debug", "ERROR: invalid quest level [QUEST=%d LEVEL=%d]", quest, level);
			quest = 0;
		}
	}
	else
	{
		dbg_msg("debug", "ERROR: invalid quest [QUEST=%d LEVEL=%d]", quest, level);
		quest = 0;
	}

	if(m_apPlayers[playerID]->IsQuesting() && quest)
	{
		str_format(aBuf, sizeof(aBuf), "[QUEST] %s", m_apPlayers[playerID]->m_aQuestString);
		SendBroadcast(aBuf, m_apPlayers[playerID]->GetCID());
		str_format(aBuf, sizeof(aBuf), "[QUEST] New Quest: %s", m_apPlayers[playerID]->m_aQuestString);
		SendChatTarget(m_apPlayers[playerID]->GetCID(), aBuf);
		return;
	}

	//quest stopped during the next quest election
	SendBroadcast("[QUEST] stopped", m_apPlayers[playerID]->GetCID());
	m_apPlayers[playerID]->m_QuestState = CPlayer::QUEST_OFF;
}

int CGameContext::PickQuestPlayer(int playerID)
{
#if defined(CONF_DEBUG)
#endif
	if(!m_apPlayers[playerID])
	{
		dbg_msg("QUEST", "WARNING! [%d][%s] invalid player picked a quest", playerID, Server()->ClientName(playerID));
		return -1;
	}

	int ID = -1;
	int FoundTees[MAX_CLIENTS];
	int Index = 0;
	int FoundDeadTees[MAX_CLIENTS];
	int IndexDead = 0;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!m_apPlayers[i])
		{
			//dbg_msg("QUEST", "<PickPlayer> warning not exsisting player found");
			continue;
		}
		if(m_apPlayers[i]->GetTeam() == TEAM_SPECTATORS)
		{
			dbg_msg("QUEST", "<PickPlayer> warning spec player found");
			continue;
		}
		if(IsSameIP(i, playerID))
		{
			//dummy found (also used to ignore the questing player it self. Keep this in mind if you remove or edit this one day)
			dbg_msg("QUEST", "<PickPlayer> warning dummy found [%s]", Server()->ClientName(i)); //this will be triggerd for all serverside dummys if ur playing local -.-
			continue;
		}
		if(m_apPlayers[i]->m_IsDummy && !g_Config.m_SvQuestCountBots)
		{
			//server side bot found
			dbg_msg("QUEST", "<PickPlayer> warning found bot [%s]", Server()->ClientName(i));
			continue;
		}

		//found valid non dummy or serverside bot
		if(!GetPlayerChar(i)) //spec/dead players always second choice
		{
			// ---> store id + 1 in array so no 0 is in array and we can check if a tee is existing and stuff
			FoundDeadTees[Index] = i + 1;
			IndexDead++;
			//dbg_msg("QUEST", "+1 dead player");
		}
		else //alive players primary choice
		{
			// ---> store id + 1 in array so no 0 is in array and we can check if a tee is existing and stuff
			FoundTees[Index] = i + 1;
			Index++;
			//dbg_msg("QUEST", "+1 alive player");
		}
	}

	if(Index < g_Config.m_SvQuestNeededPlayers) //not enough alive tees ---> check spectators
	{
		if(Index + IndexDead < g_Config.m_SvQuestNeededPlayers) //not enough dead or alive valid tees --> stop quest
		{
			m_apPlayers[playerID]->m_QuestState = CPlayer::QUEST_OFF;
			SendChatTarget(playerID, "[QUEST] Quest stopped because there are not enough tees on the server.");
			//dbg_msg("QUEST", "alive %d + dead %d = %d/%d tees to start a quest", Index, IndexDead, Index + IndexDead, g_Config.m_SvQuestNeededPlayers);
			return -1;
		}
		else
		{
			ID = FoundDeadTees[rand() % IndexDead]; //choose random one of the valid tees
			if(!ID)
			{
				dbg_msg("QUEST", "WARNING! player [%d][%s] got invalid player [%d][%s] as specific quest", playerID, Server()->ClientName(playerID), ID, Server()->ClientName(ID));
				m_apPlayers[playerID]->m_QuestState = CPlayer::QUEST_OFF;
				SendChatTarget(playerID, "[QUEST] Quest stopped because something went wrong. (please contact an admin)");
				SendChatTarget(playerID, "[QUEST] Try '/quest start' agian to load and start your quest agian");
				return -1;
			}

			m_apPlayers[playerID]->m_QuestPlayerID = ID - 1;
			return ID - 1; //before we stored id + 1 to have an better handling with false values
		}
	}

	ID = FoundTees[rand() % Index]; //choose random one of the valid alive tees
	if(!ID)
	{
		dbg_msg("QUEST", "WARNING! player [%d][%s] got invalid player [%d][%s] as specific quest", playerID, Server()->ClientName(playerID), ID, Server()->ClientName(ID));
		m_apPlayers[playerID]->m_QuestState = CPlayer::QUEST_OFF;
		SendChatTarget(playerID, "[QUEST] Quest stopped because something went wrong. (please contact an admin)");
		SendChatTarget(playerID, "[QUEST] Try '/quest start' agian to load and start your quest agian");
		return -1;
	}

	m_apPlayers[playerID]->m_QuestPlayerID = ID - 1;
	return ID - 1; //before we stored id + 1 to have an better handling with false values
}

void CGameContext::CheckConnectQuestBot()
{
	int NumQuestBots = 0;
	int NumQuestPlayers = 0;
	int NumConnectedPlayers = 0;
	int NumIngamePlayers = 0;
	int NumIngameHumans = 0;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!m_apPlayers[i])
			continue;

		NumConnectedPlayers++;
		if(m_apPlayers[i]->IsQuesting())
			NumQuestPlayers++;

		if(!m_apPlayers[i]->GetCharacter())
			continue;

		NumIngamePlayers++;

		if(!m_apPlayers[i]->m_IsDummy)
			NumIngameHumans++;
		else if(m_apPlayers[i]->m_DummyMode == CCharacter::DUMMYMODE_QUEST)
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

	CreateNewDummy(CCharacter::DUMMYMODE_QUEST);
}