/* CLaser ddnet++ methods */
#include "laser.h"

#include <engine/shared/config.h>

#include <generated/protocol.h>

#include <game/server/gamecontext.h>
#include <game/server/gamemodes/ddnet.h>
#include <game/server/teams.h>

bool CLaser::HitCharacterDDPP(vec2 From, vec2 To, CCharacter *pHit)
{
	CCharacter *pOwnerChar = GameServer()->GetPlayerChar(m_Owner);
	if(m_Type == WEAPON_SHOTGUN)
	{
		if(pHit->GetPlayer()->GetCid() != m_Owner)
		{
			if(GameServer()->m_apPlayers[m_Owner]) // make sure hitter didnt rq during wallshot
			{
				pHit->GetPlayer()->UpdateLastToucher(m_Owner);
				pHit->m_LastHitWeapon = WEAPON_SHOTGUN;
			}
		}
	}
	else if(m_Type == WEAPON_LASER)
	{
		if(!pOwnerChar)
		{
			pHit->UnFreeze();
			return false;
		}
		//quests (before unfreeze to have information about the tee was being frozzn)
		QuestHitCharacter(pHit, pOwnerChar);

		if(pOwnerChar->GetPlayer()->m_IsInstaMode_fng)
		{
			if(g_Config.m_SvOnFireMode == 1 && pHit->m_FreezeTime == 0)
			{
				if(pHit->GetPlayer() && pHit->GetPlayer()->GetCid() != m_Owner)
				{
					pOwnerChar->m_OnFire = true;
				}
			}
			pHit->TakeDamage(vec2(0.f, 0.f), 100, m_Owner, WEAPON_LASER);
		}
		else
		{
			if(pOwnerChar->GetPlayer()->m_TaserOn)
			{
				pHit->FreezeFloat(GameServer()->GetPlayerChar(m_Owner)->GetPlayer()->TaserFreezeTime());
				pHit->TakeDamage(vec2(0.f, 0.f), 100, m_Owner, WEAPON_LASER);
				pHit->m_GotTasered = true;
			}
			else
			{
				pHit->UnFreeze();
				if(pOwnerChar->GetPlayer()->m_IsVanillaCompetitive)
				{
					pHit->TakeDamage(vec2(0.f, 0.f), 5, m_Owner, WEAPON_LASER);
				}
				else
				{
					pHit->TakeDamage(vec2(0.f, 0.f), 100, m_Owner, WEAPON_LASER);
				}
			}
		}
	}
	return false;
}

void CLaser::QuestHitCharacter(CCharacter *pHit, CCharacter *pOwnerChar)
{
	// only check for quests in case both the hitter and the hit are alive
	// currently all quests are related to the quester hitting tees
	// but make sure when you add a quest where the quester gets hit
	// for example: "quest: get hit by rifle", "quest: block 3 tees without getting hit by rilfe"
	// if this becomes a thing and somebody wallshot hits the quester and kills/leaves before hit it wont be tracked.

	// for the current quests it is way to much struggle to support tracking laser hits after death
	// and users probably wont notice it.

	// anyways there is a !pOwnerChar guad directly before this function is called so keep that in mind lol

	// this comment is pretty much senesless since m_Owner is the client id so it could get the player while having a dead character
	// ddnet++ style write thoughts as comments and then commit them to master :troll:

	// lol i just realized i am debugging a crashbug in line 84 anyways so this code is fine until someone proofs its brokenness.

	// TL;DR: tried to justify the guard and failed

	/*
	if (!pHit || !pOwnerChar)
		return;
	*/

	char aBuf[256];
	if(pOwnerChar->GetPlayer()->m_QuestState == CPlayer::QUEST_RIFLE)
	{
		if(pOwnerChar->GetPlayer()->m_QuestStateLevel == 0)
		{
			if(pOwnerChar->GetPlayer()->GetCid() == pHit->GetPlayer()->GetCid())
			{
				GameServer()->SendChatTarget(pOwnerChar->GetPlayer()->GetCid(), "[QUEST] your own tee doesn't count");
			}
			else if(GameServer()->IsSameIp(pOwnerChar->GetPlayer()->GetCid(), pHit->GetPlayer()->GetCid()))
			{
				GameServer()->SendChatTarget(pOwnerChar->GetPlayer()->GetCid(), "[QUEST] your dummy doesn't count");
			}
			else
			{
				GameServer()->QuestCompleted(pOwnerChar->GetPlayer()->GetCid());
			}
		}
		else if(pOwnerChar->GetPlayer()->m_QuestStateLevel == 1)
		{
			if(pHit->GetPlayer()->GetCid() == pOwnerChar->GetPlayer()->m_QuestPlayerId)
			{
				GameServer()->QuestAddProgress(pOwnerChar->GetPlayer()->GetCid(), 5);
			}
			else
			{
				//GameServer()->SendChatTarget(pOwnerChar->GetPlayer()->GetCid(), "[QUEST] wrong tee");
			}
		}
		else if(pOwnerChar->GetPlayer()->m_QuestStateLevel == 2) //rifle freezed <specific player> 5 times
		{
			if(pHit->GetPlayer()->GetCid() == pOwnerChar->GetPlayer()->m_QuestPlayerId)
			{
				if(pHit->m_FreezeTime)
				{
					GameServer()->QuestAddProgress(pOwnerChar->GetPlayer()->GetCid(), 5);
				}
				else
				{
					GameServer()->SendChatTarget(pOwnerChar->GetPlayer()->GetCid(), "[QUEST] the target has to be freezed");
				}
			}
			else
			{
				//GameServer()->SendChatTarget(pOwnerChar->GetPlayer()->GetCid(), "[QUEST] wrong tee");
			}
		}
		else if(pOwnerChar->GetPlayer()->m_QuestStateLevel == 3) //rifle 10 tees and <specific player>
		{
			if(pOwnerChar->GetPlayer()->GetCid() == pHit->GetPlayer()->GetCid())
			{
				GameServer()->SendChatTarget(pOwnerChar->GetPlayer()->GetCid(), "[QUEST] your own tee doesn't count");
			}
			else if(GameServer()->IsSameIp(pOwnerChar->GetPlayer()->GetCid(), pHit->GetPlayer()->GetCid()))
			{
				GameServer()->SendChatTarget(pOwnerChar->GetPlayer()->GetCid(), "[QUEST] your dummy doesn't count");
			}
			else
			{
				if(pHit->GetPlayer()->GetCid() == pOwnerChar->GetPlayer()->m_QuestPlayerId)
				{
					pOwnerChar->GetPlayer()->m_QuestProgressBool = true;
				}

				if(pOwnerChar->GetPlayer()->m_QuestLastQuestedPlayerId == pHit->GetPlayer()->GetCid()) //hitting the same player again
				{
					GameServer()->SendChatTarget(pOwnerChar->GetPlayer()->GetCid(), "[QUEST] rifle a different tee");
				}
				else //hitting a new player
				{
					if(pOwnerChar->GetPlayer()->m_QuestProgressBool)
					{
						GameServer()->QuestAddProgress(pOwnerChar->GetPlayer()->GetCid(), 11);
						pOwnerChar->GetPlayer()->m_QuestLastQuestedPlayerId = pHit->GetPlayer()->GetCid();
					}
					else
					{
						GameServer()->QuestAddProgress(pOwnerChar->GetPlayer()->GetCid(), 11, 10);
						pOwnerChar->GetPlayer()->m_QuestLastQuestedPlayerId = pHit->GetPlayer()->GetCid();
					}
				}
			}
		}
		else if(pOwnerChar->GetPlayer()->m_QuestStateLevel == 4) //rifle 10 freezed tees
		{
			if(pOwnerChar->GetPlayer()->GetCid() == pHit->GetPlayer()->GetCid())
			{
				GameServer()->SendChatTarget(pOwnerChar->GetPlayer()->GetCid(), "[QUEST] your own tee doesn't count");
			}
			else if(GameServer()->IsSameIp(pOwnerChar->GetPlayer()->GetCid(), pHit->GetPlayer()->GetCid()))
			{
				GameServer()->SendChatTarget(pOwnerChar->GetPlayer()->GetCid(), "[QUEST] your dummy doesn't count");
			}
			else if(pOwnerChar->GetPlayer()->m_QuestLastQuestedPlayerId == pHit->GetPlayer()->GetCid())
			{
				GameServer()->SendChatTarget(pOwnerChar->GetPlayer()->GetCid(), "[QUEST] rifle a different tee");
			}
			else if(!pHit->m_FreezeTime)
			{
				GameServer()->SendChatTarget(pOwnerChar->GetPlayer()->GetCid(), "[QUEST] the target has to be freezed");
			}
			else
			{
				GameServer()->QuestAddProgress(pOwnerChar->GetPlayer()->GetCid(), 10);
				pOwnerChar->GetPlayer()->m_QuestLastQuestedPlayerId = pHit->GetPlayer()->GetCid();
			}
		}
		else if(pOwnerChar->GetPlayer()->m_QuestStateLevel == 5) //freeze selfrifle
		{
			if(pOwnerChar->GetPlayer()->GetCid() == pHit->GetPlayer()->GetCid())
			{
				if(!pHit->m_FreezeTime)
				{
					GameServer()->SendChatTarget(pOwnerChar->GetPlayer()->GetCid(), "[QUEST] the target has to be freezed");
				}
				else
				{
					GameServer()->QuestCompleted(pOwnerChar->GetPlayer()->GetCid());
				}
			}
		}
		else if(pOwnerChar->GetPlayer()->m_QuestStateLevel == 6) //freeze selfrifle 10 times
		{
			if(pOwnerChar->GetPlayer()->GetCid() == pHit->GetPlayer()->GetCid())
			{
				if(!pHit->m_FreezeTime)
				{
					GameServer()->SendChatTarget(pOwnerChar->GetPlayer()->GetCid(), "[QUEST] the target has to be freezed");
				}
				else
				{
					GameServer()->QuestAddProgress(pOwnerChar->GetPlayer()->GetCid(), 10);
				}
			}
		}
		else if(pOwnerChar->GetPlayer()->m_QuestStateLevel == 7) //rifle <specific player> and then block him
		{
			if(pOwnerChar->GetPlayer()->m_QuestPlayerId != pHit->GetPlayer()->GetCid())
			{
				//GameServer()->SendChatTarget(pOwnerChar->GetPlayer()->GetCid(), "[QUEST] wrong tee");
			}
			else
			{
				GameServer()->QuestAddProgress(pOwnerChar->GetPlayer()->GetCid(), 2, 1);
			}
		}
		else if(pOwnerChar->GetPlayer()->m_QuestStateLevel == 8) //rifle 5 tees before blocking them
		{
			if(pOwnerChar->GetPlayer()->GetCid() == pHit->GetPlayer()->GetCid())
			{
				GameServer()->SendChatTarget(pOwnerChar->GetPlayer()->GetCid(), "[QUEST] your own tee doesn't count");
			}
			else if(GameServer()->IsSameIp(pOwnerChar->GetPlayer()->GetCid(), pHit->GetPlayer()->GetCid()))
			{
				GameServer()->SendChatTarget(pOwnerChar->GetPlayer()->GetCid(), "[QUEST] your dummy doesn't count");
			}
			else
			{
				//if (!pOwnerChar->GetPlayer()->m_QuestProgressBool) //not used anymore because whole system updated. Old was [hit a tee and block exactly this tee] New system [Block the the you hit last]. Because you can hit multiple tees and change ur block destination i swapped from a 10progress to a 5 progress (not counting the rifle hits only block kills)
				//{
				//	//GameServer()->QuestAddProgress(pOwnerChar->GetPlayer()->GetCid(), 10, pOwnerChar->GetPlayer()->m_QuestProgressValue2 + 1); //crazy limit stuff not needed cuz of ze bool
				//	GameServer()->QuestAddProgress(pOwnerChar->GetPlayer()->GetCid(), 10);
				//}

				pOwnerChar->GetPlayer()->m_QuestProgressBool = true;
				pOwnerChar->GetPlayer()->m_QuestLastQuestedPlayerId = pHit->GetPlayer()->GetCid();
				str_format(aBuf, sizeof(aBuf), "[QUEST] Riflemarker set. Now block '%s'.", Server()->ClientName(pOwnerChar->GetPlayer()->m_QuestLastQuestedPlayerId));
				GameServer()->SendChatTarget(pOwnerChar->GetPlayer()->GetCid(), aBuf);
			}
		}
		else if(pOwnerChar->GetPlayer()->m_QuestStateLevel == 9) //rifle 20 freezed tees while having the flag
		{
			if(pOwnerChar->GetPlayer()->GetCid() == pHit->GetPlayer()->GetCid())
			{
				GameServer()->SendChatTarget(pOwnerChar->GetPlayer()->GetCid(), "[QUEST] your own tee doesn't count");
			}
			else if(GameServer()->IsSameIp(pOwnerChar->GetPlayer()->GetCid(), pHit->GetPlayer()->GetCid()))
			{
				GameServer()->SendChatTarget(pOwnerChar->GetPlayer()->GetCid(), "[QUEST] your dummy doesn't count");
			}
			else if(!pHit->m_FreezeTime)
			{
				GameServer()->SendChatTarget(pOwnerChar->GetPlayer()->GetCid(), "[QUEST] the target has to be freezed");
			}
			else if(pOwnerChar->GetPlayer()->m_QuestLastQuestedPlayerId == pHit->GetPlayer()->GetCid())
			{
				GameServer()->SendChatTarget(pOwnerChar->GetPlayer()->GetCid(), "[QUEST] rifle a different tee");
			}
			else if(GameServer()->m_pController->HasFlag(pOwnerChar) == -1) //no flag
			{
				GameServer()->SendChatTarget(pOwnerChar->GetPlayer()->GetCid(), "[QUEST] you need the flag");
			}
			else
			{
				GameServer()->QuestAddProgress(pOwnerChar->GetPlayer()->GetCid(), 20);
				pOwnerChar->GetPlayer()->m_QuestLastQuestedPlayerId = pHit->GetPlayer()->GetCid();
			}
		}
	}
}
