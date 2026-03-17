// ddnet-insta specific gamecontroller methods
#include <base/log.h>
#include <base/system.h>

#include <engine/shared/config.h>

#include <generated/protocol.h>

#include <game/mapitems.h>
#include <game/server/entities/character.h>
#include <game/server/entities/door.h>
#include <game/server/gamecontext.h>
#include <game/server/gamecontroller.h>
#include <game/server/player.h>
#include <game/server/score.h>

void IGameController::OnCharacterDeathImpl(CCharacter *pVictim, int Killer, int Weapon, bool SendKillMsg)
{
	if(Killer != WEAPON_GAME && pVictim->m_SetSavePos[RESCUEMODE_AUTO])
		pVictim->GetPlayer()->m_LastDeath = pVictim->m_RescueTee[RESCUEMODE_AUTO];
	pVictim->StopRecording();

	// TODO: remove DDPP_DIE and use the ddnet-insta approach instead
	bool TodoRemoveFngScore = false;
	Killer = pVictim->DDPP_DIE(Killer, Weapon, TodoRemoveFngScore);

	int ModeSpecial = GameServer()->m_pController->OnCharacterDeath(pVictim, (Killer < 0) ? nullptr : GameServer()->m_apPlayers[Killer], Weapon);

	LogKillMessage(pVictim, Killer, Weapon, ModeSpecial);

	// TODO: move this to ddnet++ controller
	// ddnet++
	if(Killer < 0 || Killer == pVictim->GetPlayer()->GetCid())
	{
		pVictim->m_LastHitWeapon = -1;
		Weapon = -1;
	}

	if(SendKillMsg)
	{
		SendDeathInfoMessage(pVictim, Killer, Weapon, ModeSpecial);
	}

	// a nice sound
	// and exploding tee death effect
	SendDeathEvent(pVictim, Killer, Weapon);

	// this is to rate limit respawning to 3 secs
	pVictim->GetPlayer()->m_PreviousDieTick = pVictim->GetPlayer()->m_DieTick;
	pVictim->GetPlayer()->m_DieTick = Server()->Tick();

	pVictim->m_Alive = false;
	pVictim->SetSolo(false);

	GameServer()->m_World.RemoveEntity(pVictim);
	GameServer()->m_World.m_Core.m_apCharacters[pVictim->GetPlayer()->GetCid()] = nullptr;
	pVictim->Teams()->OnCharacterDeath(pVictim->GetPlayer()->GetCid(), Weapon);
	pVictim->CancelSwapRequests();
}

void IGameController::SendDeathInfoMessage(CCharacter *pVictim, int Killer, int Weapon, int ModeSpecial)
{
	pVictim->SendDeathMessageIfNotInLockedTeam(Killer, Weapon, ModeSpecial);
}

void IGameController::SendDeathEvent(CCharacter *pVictim, int Killer, int Weapon)
{
	GameServer()->CreateSound(pVictim->m_Pos, SOUND_PLAYER_DIE, pVictim->TeamMask());
	GameServer()->CreateDeath(pVictim->m_Pos, pVictim->GetPlayer()->GetCid(), pVictim->TeamMask());
}

void IGameController::LogKillMessage(CCharacter *pVictim, int Killer, int Weapon, int ModeSpecial)
{
	// ddnet-insta added this branch
	// inspired by upstream https://github.com/teeworlds/teeworlds/blob/5d682733e482950f686663c129adc4b751c8d790/src/game/server/entities/character.cpp#L665
	// to fix a crash bug
	if(Killer < 0 || !GameServer()->m_apPlayers[Killer])
	{
		log_info("game", "kill killer='%d:%d:' victim='%d:%s' weapon=%d special=%d",
			Killer, -1 - Killer,
			pVictim->GetPlayer()->GetCid(), Server()->ClientName(pVictim->GetPlayer()->GetCid()), Weapon, ModeSpecial);
	}
	else
	{
		log_info("game", "kill killer='%d:%s' victim='%d:%s' weapon=%d special=%d",
			Killer, Server()->ClientName(Killer),
			pVictim->GetPlayer()->GetCid(), Server()->ClientName(pVictim->GetPlayer()->GetCid()), Weapon, ModeSpecial);
	}
}
