// ddnet++ quest character stuff

#include "character.h"
#include "laser.h"
#include "plasmabullet.h"
#include "projectile.h"

#include <engine/server/server.h>
#include <engine/shared/config.h>

#include <game/server/gamecontext.h>
#include <game/server/gamemodes/DDRace.h>
#include <game/server/player.h>

void CCharacter::QuestHammerHit(CCharacter *pTarget)
{
	//Quests  (before police so no confusion i hope)
	if(m_pPlayer->m_QuestState == CPlayer::QUEST_HAMMER) //if is questing and hammer quest
	{
		if(GameServer()->IsSameIp(m_pPlayer->GetCid(), pTarget->GetPlayer()->GetCid()))
		{
			if((m_pPlayer->m_QuestStateLevel == 4 && pTarget->m_FreezeTime == 0) || // freezed quest
				m_pPlayer->m_QuestStateLevel == 5 || // <specific player> quest
				m_pPlayer->m_QuestStateLevel == 6 || // <specific player> quest
				m_pPlayer->m_QuestStateLevel == 7) // <specific player> quest
			{
				//dont send message here
			}
			else
			{
				if(!m_pPlayer->m_HideQuestWarning)
				{
					GameServer()->SendChatTarget(m_pPlayer->GetCid(), "[QUEST] your dummy doesnt count.");
				}
			}
		}
		else
		{
			if(m_pPlayer->m_QuestStateLevel == 0)
			{
				GameServer()->QuestCompleted(m_pPlayer->GetCid());
			}
			else if(m_pPlayer->m_QuestStateLevel == 1)
			{
				if(m_pPlayer->m_QuestLastQuestedPlayerId == pTarget->GetPlayer()->GetCid())
				{
					if(!m_pPlayer->m_HideQuestWarning)
					{
						GameServer()->SendChatTarget(m_pPlayer->GetCid(), "[QUEST] hammer a different tee.");
					}
				}
				else
				{
					GameServer()->QuestAddProgress(m_pPlayer->GetCid(), 2);
					m_pPlayer->m_QuestLastQuestedPlayerId = pTarget->GetPlayer()->GetCid();
				}
			}
			else if(m_pPlayer->m_QuestStateLevel == 2)
			{
				if(m_pPlayer->m_QuestLastQuestedPlayerId == pTarget->GetPlayer()->GetCid())
				{
					if(!m_pPlayer->m_HideQuestWarning)
					{
						GameServer()->SendChatTarget(m_pPlayer->GetCid(), "[QUEST] hammer a different tee.");
					}
				}
				else
				{
					GameServer()->QuestAddProgress(m_pPlayer->GetCid(), 3);
					m_pPlayer->m_QuestLastQuestedPlayerId = pTarget->GetPlayer()->GetCid();
				}
			}
			else if(m_pPlayer->m_QuestStateLevel == 3)
			{
				if(m_pPlayer->m_QuestLastQuestedPlayerId == pTarget->GetPlayer()->GetCid())
				{
					if(!m_pPlayer->m_HideQuestWarning)
					{
						GameServer()->SendChatTarget(m_pPlayer->GetCid(), "[QUEST] hammer a different tee.");
					}
				}
				else
				{
					GameServer()->QuestAddProgress(m_pPlayer->GetCid(), 5);
					m_pPlayer->m_QuestLastQuestedPlayerId = pTarget->GetPlayer()->GetCid();
				}
			}
			else if(m_pPlayer->m_QuestStateLevel == 4)
			{
				if(pTarget->m_FreezeTime == 0)
				{
					//GameServer()->SendChatTarget(m_pPlayer->GetCid(), "[QUEST] this tee is not freezed.");
				}
				else if(m_pPlayer->m_QuestLastQuestedPlayerId == pTarget->GetPlayer()->GetCid())
				{
					if(!m_pPlayer->m_HideQuestWarning)
					{
						GameServer()->SendChatTarget(m_pPlayer->GetCid(), "[QUEST] hammer a different tee.");
					}
				}
				else
				{
					GameServer()->QuestAddProgress(m_pPlayer->GetCid(), 10);
					m_pPlayer->m_QuestLastQuestedPlayerId = pTarget->GetPlayer()->GetCid();
				}
			}
			else if(m_pPlayer->m_QuestStateLevel == 5)
			{
				if(m_pPlayer->m_QuestPlayerId != pTarget->GetPlayer()->GetCid())
				{
					//GameServer()->SendChatTarget(m_pPlayer->GetCid(), "[QUEST] you hammered the wrong tee.");
				}
				else
				{
					GameServer()->QuestAddProgress(m_pPlayer->GetCid(), 20);
					m_pPlayer->m_QuestLastQuestedPlayerId = pTarget->GetPlayer()->GetCid();
				}
			}
			else if(m_pPlayer->m_QuestStateLevel == 6)
			{
				if(m_pPlayer->m_QuestPlayerId != pTarget->GetPlayer()->GetCid())
				{
					//GameServer()->SendChatTarget(m_pPlayer->GetCid(), "[QUEST] you hammered the wrong tee.");
				}
				else if(pTarget->m_FreezeTime == 0)
				{
					//GameServer()->SendChatTarget(m_pPlayer->GetCid(), "[QUEST] this tee is not freezed.");
				}
				else
				{
					GameServer()->QuestAddProgress(m_pPlayer->GetCid(), 3);
					m_pPlayer->m_QuestLastQuestedPlayerId = pTarget->GetPlayer()->GetCid();
				}
			}
			else if(m_pPlayer->m_QuestStateLevel == 7)
			{
				if(m_pPlayer->m_QuestPlayerId != pTarget->GetPlayer()->GetCid())
				{
					//GameServer()->SendChatTarget(m_pPlayer->GetCid(), "[QUEST] you hammered the wrong tee.");
				}
				else
				{
					GameServer()->QuestAddProgress(m_pPlayer->GetCid(), 11, 10);
					m_pPlayer->m_QuestLastQuestedPlayerId = pTarget->GetPlayer()->GetCid();
				}
			}
			else if(m_pPlayer->m_QuestStateLevel == 9)
			{
				if(((CGameControllerDDRace *)GameServer()->m_pController)->HasFlag(this) != -1) //has flag
				{
					if(m_pPlayer->m_QuestLastQuestedPlayerId == pTarget->GetPlayer()->GetCid())
					{
						if(!m_pPlayer->m_HideQuestWarning)
						{
							GameServer()->SendChatTarget(m_pPlayer->GetCid(), "[QUEST] hammer a different tee.");
						}
					}
					else if(pTarget->m_FreezeTime == 0)
					{
						//GameServer()->SendChatTarget(m_pPlayer->GetCid(), "[QUEST] this tee is not freezed.");
					}
					else
					{
						GameServer()->QuestAddProgress(m_pPlayer->GetCid(), 10);
						m_pPlayer->m_QuestLastQuestedPlayerId = pTarget->GetPlayer()->GetCid();
					}
				}
				else
				{
					//GameServer()->SendChatTarget(m_pPlayer->GetCid(), "[QUEST] you have to carry the flag");
				}
			}
			else
			{
				dbg_msg("QUEST", "WARNING! character.cpp undefined quest level %d", m_pPlayer->m_QuestStateLevel);
			}
		}
	}
	else if(m_pPlayer->m_QuestState == CPlayer::QUEST_RACE)
	{
		if(m_pPlayer->m_QuestStateLevel == 9) //race with conditions
		{
			if(g_Config.m_SvQuestRaceCondition == 0) //no hammer
			{
				GameServer()->QuestFailed2(m_pPlayer->GetCid());
			}
		}
	}
}

void CCharacter::QuestShotgun()
{
	//race quest (shotgun)
	if(m_pPlayer->m_QuestState == CPlayer::QUEST_RACE)
	{
		if(m_pPlayer->m_QuestStateLevel == 9) //race with conditions
		{
			if(g_Config.m_SvQuestRaceCondition == 2) //no shotgun
			{
				GameServer()->QuestFailed2(m_pPlayer->GetCid());
			}
		}
	}
}

void CCharacter::QuestGrenade()
{
	if(m_pPlayer->m_QuestState == CPlayer::QUEST_RACE)
	{
		if(m_pPlayer->m_QuestStateLevel == 9) //race with conditions
		{
			if(g_Config.m_SvQuestRaceCondition == 3) //no grenade
			{
				GameServer()->QuestFailed2(m_pPlayer->GetCid());
			}
		}
	}
}

void CCharacter::QuestRifle()
{
	if(m_pPlayer->m_QuestState == CPlayer::QUEST_RACE)
	{
		if(m_pPlayer->m_QuestStateLevel == 9) //race with conditions
		{
			if(g_Config.m_SvQuestRaceCondition == 4) //no rifle
			{
				GameServer()->QuestFailed2(m_pPlayer->GetCid());
			}
		}
	}
}

void CCharacter::QuestNinja()
{
	if(m_pPlayer->m_QuestState == CPlayer::QUEST_RACE)
	{
		if(m_pPlayer->m_QuestStateLevel == 9) // race with conditions
		{
			if(g_Config.m_SvQuestRaceCondition == 5) // no ninja
			{
				GameServer()->QuestFailed2(m_pPlayer->GetCid());
			}
		}
	}
}

void CCharacter::QuestFireWeapon()
{
	if(m_pPlayer->m_QuestState == CPlayer::QUEST_BLOCK)
	{
		if(m_pPlayer->m_QuestStateLevel == 4) // Block 10 tees without fireing a weapon
		{
			GameServer()->QuestFailed(m_pPlayer->GetCid());
		}
	}
}
