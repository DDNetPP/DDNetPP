// DDNet++ minigame base

#ifndef GAME_SERVER_MINIGAMES_MINIGAME_BASE_H
#define GAME_SERVER_MINIGAMES_MINIGAME_BASE_H

#include <base/system.h>

#include <engine/shared/network.h>

#include <generated/protocol7.h>

#include <game/server/ddpp/enums.h>
#include <game/server/save.h>

class CCharacter;
class CPlayer;
class CGameContext;
class IGameController;
class CCollision;
class IServer;
class CSpawnEval;
class CSaveTee;
class CSaveTeeDDPP;

class CMinigame
{
protected:
	CGameContext *m_pGameServer;
	CSaveTee *m_apSavedPositions[MAX_CLIENTS];
	CSaveTeeDDPP *m_apSavedPositionsDDPP[MAX_CLIENTS];
	bool m_aRestorePos[MAX_CLIENTS];
	int m_State;

	//
	// convenience getters/wrappers in minigame scope
	//

	CGameContext *GameServer();
	IServer *Server();
	CCollision *Collision();
	IGameController *Controller();

	// wraps GameServer()->SendChatTarget()
	// see also the minigame specific methods
	// `SendChatAll()` and `SendBroadcastAll()`
	//
	// the -1 version flags is just a hack to get it to compile
	// because it depends on a cgamecontext enum
	void SendChatTarget(int To, const char *pText, int VersionFlags = -1);

public:
	CMinigame(CGameContext *pGameContext);
	virtual ~CMinigame() = default;

	void CleanupMinigame();

	// will be called every time the gamecontroller is reset
	// which is twice on normal server start
	// and will be called for every reload and similar
	//
	// so only initialize variables here
	// do not write code here that should run at max once
	// on server start
	virtual void OnInit() {}

	// do your cleanup here
	virtual void OnShutdown() {}

	// will be called every tick
	virtual void Tick() {}

	// will be called every 600 ticks
	virtual void SlowTick() {}

	// called every time the server builds a snap
	// register your custom snap items here
	virtual void Snap(int SnappingClient) {}

	// This already implements logic in the minigame base
	// it is unlikely that you need to override it!
	// If you do make sure you call the parent method
	// or understand what it does.
	// If you just want to register snap items
	// look at `Snap(int SnappingClient)` instead
	virtual void SnapSavedPositions(int SnappingClient);

	// will be called every 600 ticks for every player
	virtual void PlayerSlowTick(CPlayer *pPlayer) {}

	// will be called every tick for every alive player
	virtual void CharacterTick(CCharacter *pChr) {}

	// Called for all connected players once per tick
	// the pPlayer pointer is never invalid
	virtual void PlayerTick(CPlayer *pPlayer) {}

	// Will be called for every client that tries to selfkill
	// can be overwritten to disallow selfkill during the minigame
	virtual bool AllowSelfKill(int ClientId) { return true; }

	// Will be called when the CCharacter dies
	virtual void OnDeath(CCharacter *pChr, int Killer, int Weapon) {}

	// Will be called after the CCharacter spawned
	virtual void PostSpawn(CCharacter *pChr) {}

	// Will be called once for every player that connects
	virtual void OnPlayerConnect(class CPlayer *pPlayer) {}

	// Will be called once for every player that disconnects
	virtual void OnPlayerDisconnect(class CPlayer *pPlayer, const char *pReason) {}

	// return true if you want to set a spawnpoint
	// return false if you want to default to normal spawn
	//
	// this is a hook into CGameController::CanSpawn()
	virtual bool PickSpawn(vec2 *pPos, CPlayer *pPlayer) { return false; }

	// Will be called for every character on every tick
	// Add your minigame related tile logic here
	//
	// return true if you want to abort the other tile checks
	// useful when calling Die() to avoid crashes
	virtual bool HandleCharacterTiles(CCharacter *pChr, int MapIndex) { return false; }

	// Returns true if the ClientId is playing the minigame
	virtual bool IsActive(int ClientId) = 0;

	// Called when the player IsActive() and typed /leave into the chat
	// Returns true when the player did leave the game
	// Returns false when the leave failed
	virtual bool OnChatCmdLeave(CPlayer *pPlayer) { return true; }

	// If your minigame is race based
	// you probably want to overwrite this method
	// and return EDisplayScore::TIME
	virtual EDisplayScore ScoreType() { return EDisplayScore::BLOCK; }

	// will be included in the snapshot
	virtual int ScoreLimit(CPlayer *pPlayer) { return 0; }

	// alter the pGameInfo object if needed, its values are already filled
	virtual void SnapGameInfo(CPlayer *pPlayer, CNetObj_GameInfo *pGameInfo) {}

	// alter the snap objects if needed, its values are already filled
	virtual void SnapPlayer6(CPlayer *pPlayer, CNetObj_ClientInfo *pClientInfo, CNetObj_PlayerInfo *pPlayerInfo);

	// alter the pGameData object if needed, its values are already filled
	// it includes the flag carriers from the main game
	// but you can also have your own custom flags in the minigame by overwriting it
	virtual void SnapGameData(CPlayer *pPlayer, CNetObj_GameData *pGameData) {}

	// alter the pGameData object if needed, its values are already filled
	// it includes the flag carriers from the main game
	// but you can also have your own custom flags in the minigame by overwriting it
	virtual void SnapGameDataFlag7(CPlayer *pPlayer, protocol7::CNetObj_GameDataFlag *pGameData) {}

	// Presist player position when joining the minigame
	// to be later able to load it again
	virtual void SavePosition(CPlayer *pPlayer);

	// Make sure SavePosition is called first.
	// Use this to restore position after leaving the minigame.
	//
	// m_aRestorePos[ClientId] has to be set to true
	virtual void LoadPosition(CCharacter *pChr);

	// returns true if the player has a position saved
	// and sets the pPos passed in to that position
	// the position will be set by `SavePosition()`
	bool GetSavedPosition(CPlayer *pPlayer, vec2 *pPos);

	// clears state saved by SavePosition()
	void ClearSavedPosition(CPlayer *pPlayer);

	// Send a chat message to all minigame participants
	void SendChatAll(const char *pMessage);

	// Send a broadcast to all minigame participants
	void SendBroadcastAll(const char *pMessage);

	// m_State getter
	virtual int State() { return m_State; }

	// m_State setter. You might want to overwrite that.
	virtual int State(int State) { return m_State = State; }
};

#endif
