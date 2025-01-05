#ifndef GAME_SERVER_DDPP_TELEPORTATION_REQUEST_H
#define GAME_SERVER_DDPP_TELEPORTATION_REQUEST_H

#include <base/vmath.h>

#include <functional>

typedef std::function<void(const char *pMessage)> FTeleRequestFailure;
typedef std::function<void()> FTeleRequestSuccess;

// some minigames require you to hold still before
// entering or leaving them
// this teleportation request class handles that state
//
// example usage:
//
// CCharacter *pChr = pPlayer->GetCharacter();
// pChr->RequestTeleToTile(TILE_BLOCK_DM_A1)
// 	.SetName("1vs1")
// 	.DelayInSeconds(10)
// 	.OnSuccess([&]() { SendChatTarget(pPlayer->GetCid(), "tele worked uwu"); })
// 	.OnFailure([&](const char *pMessage) { SendChatTarget(pPlayer->GetCid(), pMessage); });
class CTeleportationRequest
{
	bool m_IsActive = false;
	class CCharacter *m_pCharacter = nullptr;
	class CCharacter *Character() { return m_pCharacter; }

	int m_TicksUntilTeleportation = 0;

	// [xxx] wait until you will be teleported.
	//  ^^^
	char m_aDestNameShortSlug[16];

	// [xxx] your teleportation to xxxxx xxxx failed because you moved
	//                             ^^^^^^^^^^
	char m_aDestNameLongDisplay[32];

	// used to trigger a deferred error
	// if set it will fire on tick
	char m_aErrorMsg[512];

	FTeleRequestFailure m_pfnFailure = nullptr;
	FTeleRequestSuccess m_pfnSuccess = nullptr;
	int m_Seconds = 10;
	vec2 m_DestinationPos;

public:
	CTeleportationRequest &TeleportToPos(class CCharacter *pCharacter, vec2 Pos);
	CTeleportationRequest &TeleportToTile(class CCharacter *pCharacter, int Tile);

	CTeleportationRequest &OnFailure(const FTeleRequestFailure &pfnFailure);
	CTeleportationRequest &OnSuccess(const FTeleRequestSuccess &pfnSuccess);
	CTeleportationRequest &SetName(const char *pName);
	CTeleportationRequest &DelayInSeconds(int Seconds);

	void Tick();
	bool IsActive() const { return m_IsActive; }
	void OnDeath();

	// register a error that will fire in the next tick
	// fails on tick to avoid breaking initialization chains
	void DeferError(const char *pMessage);

private:
	void TeleportSuccess();
	void TeleportFailure(const char *pMessage);
};

#endif
