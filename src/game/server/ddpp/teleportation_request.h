#ifndef GAME_SERVER_DDPP_TELEPORTATION_REQUEST_H
#define GAME_SERVER_DDPP_TELEPORTATION_REQUEST_H

#include <base/vmath.h>

#include <cstdint>
#include <functional>

typedef std::function<void(const char *pErrorShort, const char *pErrorLong)> FTeleRequestFailure;
typedef std::function<void()> FTeleRequestSuccess;

// some minigames require you to hold still before
// entering or leaving them
// this teleportation request class handles that state
//
// example usage:
//
// pChr->RequestTeleToTile(TILE_BLOCK_DM_A1)
// 	.DelayInSeconds(3)
// 	.OnPreSuccess([=]() {
// 		SavePosition(pChr->GetPlayer());
// 	})
// 	.OnPostSuccess([=]() {
// 		OnTeleportSuccess(pGameState, pChr->GetPlayer());
// 	})
// 	.OnFailure([=](const char *pShort, const char *pLong) {
// 		char aError[512];
// 		str_format(aError, sizeof(aError), "[1vs1] game aborted '%s' failed to teleport (%s)", Server()->ClientName(pChr->GetPlayer()->GetCid()), pShort);
// 		SendChatTarget(pChr->GetPlayer()->GetCid(), aError);
// 	});
class CTeleportationRequest
{
	bool m_IsActive = false;
	class CCharacter *m_pCharacter = nullptr;
	class CCharacter *Character() { return m_pCharacter; }

	int m_TicksUntilTeleportation = 0;

	// used to trigger a deferred error
	// if set it will fire on tick
	char m_aErrorMsgLong[512];
	char m_aErrorMsgShort[512];

	FTeleRequestFailure m_pfnFailure = nullptr;
	FTeleRequestSuccess m_pfnPreSuccess = nullptr;
	FTeleRequestSuccess m_pfnPostSuccess = nullptr;
	int m_Seconds = 10;
	bool m_DropFlag = true;
	vec2 m_DestinationPos;
	int64_t m_RequestStartTick = 0;
	bool m_MoveWarningPrinted = false;

public:
	CTeleportationRequest &TeleportToPos(class CCharacter *pCharacter, vec2 Pos);

	// Teleports *pCharacter* to a tile in the map with index *Tile*
	// If *Offset* is -1 it will be a random tile otherwise it will be the n'th
	// occurrence of that tile in the map
	CTeleportationRequest &TeleportToTile(class CCharacter *pCharacter, int Tile, int Offset = -1);
	void Abort();

	CTeleportationRequest &OnFailure(const FTeleRequestFailure &pfnFailure);
	CTeleportationRequest &OnPreSuccess(const FTeleRequestSuccess &pfnSuccess);
	CTeleportationRequest &OnPostSuccess(const FTeleRequestSuccess &pfnSuccess);
	CTeleportationRequest &DelayInSeconds(int Seconds);
	CTeleportationRequest &KeepFlag(bool Keep);

	void Tick();
	bool IsActive() const { return m_IsActive; }
	void OnDeath();

	// register a error that will fire in the next tick
	// fails on tick to avoid breaking initialization chains
	void DeferError(const char *pMessageShort, const char *pMessageLong);

private:
	void TeleportSuccess();
	void TeleportFailure(const char *pMessageShort, const char *pMessageLong);
};

#endif
