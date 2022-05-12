// DDNet++ minigame base

#ifndef GAME_SERVER_MINIGAMES_MINIGAME_BASE_H
#define GAME_SERVER_MINIGAMES_MINIGAME_BASE_H

#include <base/system.h>
#include <engine/shared/network.h>
#include <game/server/save.h>

class CCharacter;
class CPlayer;
class CGameContext;
class CCollision;
class IServer;
class CSpawnEval;
class CSaveTee;

class CMinigame
{
protected:
	CGameContext *m_pGameServer;
	CGameContext *GameServer();
	IServer *Server();
	CCollision *Collision();
	CSaveTee *m_apSavedPositions[MAX_CLIENTS];
	bool m_aRestorePos[MAX_CLIENTS];

public:
	CMinigame(CGameContext *pGameContext);
	virtual ~CMinigame(){};

	/*
        OnInit

        will be called every time the gamecontroller is reset
        which is twice on normal server start
        and will be called for every reload and similar

        so only initialize variables here
        do not write code here that should run at max once
        on server start
    */
	virtual void OnInit(){};

	/*
        Tick

        will be called every tick
    */
	virtual void Tick(){};

	/*
        OnDeath

        Will be called when the CCharacter dies
    */
	virtual void OnDeath(CCharacter *pChr, int Killer){};

	/*
        PostSpawn

        Will be called after the CCharacter spawned
    */
	virtual void PostSpawn(CCharacter *pChr, vec2 Pos){};

	/*
        PickSpawn

        return true if you want to set a spawnpoint
        return false if you want to default to normal spawn

        this is a hook into CGameController::CanSpawn()
    */
	virtual bool PickSpawn(vec2 *pPos, CPlayer *pPlayer) { return false; }

	/*
        IsActive

        Returns true if the ClientID is playing the minigame
    */
	virtual bool IsActive(int ClientID) = 0;

	/*
        SavePosition

        Presist player position when joining the minigame
        to be later able to load it again
    */
	virtual void SavePosition(CPlayer *pPlayer);

	/*
        LoadPosition

        Make sure SavePosition is called first.
        Use this to restore position after leaving the minigame.

        m_aRestorePos[ClientID] has to be set to true
    */
	virtual void LoadPosition(CCharacter *pChr);

	/*
        SendChatAll

        Send a chat message to all minigame participants
    */
	void SendChatAll(const char *pMessage);

	/*
        SendBroadcastAll

        Send a broadcast to all minigame participants
    */
	void SendBroadcastAll(const char *pMessage);

	// TODO: make this protected
	int m_State;
};

#endif
