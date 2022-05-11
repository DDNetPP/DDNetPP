// DDNet++ minigame base

#ifndef GAME_SERVER_MINIGAMES_MINIGAME_BASE_H
#define GAME_SERVER_MINIGAMES_MINIGAME_BASE_H

#include <base/system.h>

class CCharacter;
class CPlayer;
class CGameContext;
class IServer;
class CSpawnEval;

class CMinigame
{
protected:
	CGameContext *m_pGameServer;
	CGameContext *GameServer();
	IServer *Server();

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
