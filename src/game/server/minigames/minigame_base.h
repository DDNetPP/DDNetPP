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
class CSaveTeeDDPP;

enum EScore
{
	SCORE_TIME,
	SCORE_LEVEL,
	SCORE_BLOCK
};

class CMinigame
{
protected:
	CGameContext *m_pGameServer;
	CGameContext *GameServer();
	IServer *Server();
	CCollision *Collision();
	CSaveTee *m_apSavedPositions[MAX_CLIENTS];
	CSaveTeeDDPP *m_apSavedPositionsDDPP[MAX_CLIENTS];
	bool m_aRestorePos[MAX_CLIENTS];
	int m_State;

public:
	CMinigame(CGameContext *pGameContext);
	virtual ~CMinigame(){};

	void CleanupMinigame();

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
        OnShutdown

        do your cleanup here
    */
	virtual void OnShutdown(){};

	/*
        Tick

        will be called every tick
    */
	virtual void Tick(){};

	/*
        SlowTick

        will be called every 600 ticks
    */
	virtual void SlowTick(){};

	/*
        CharacterTick

        will be called every tick for every alive player
    */
	virtual void CharacterTick(CCharacter *pChr){};

	/*
        AllowSelfKill

        Will be called for every client that tries to selfkill
        can be overwritten to disallow selfkill during the minigame
    */
	virtual bool AllowSelfKill(int ClientId) { return true; }

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
        HandleCharacterTiles

        Will be called for every character on every tick
        Add your minigame related tile logic here

        return true if you want to abort the other tile checks
        useful when calling Die() to avoid crashes
    */
	virtual bool HandleCharacterTiles(CCharacter *pChr, int MapIndex) { return false; };

	/*
        IsActive

        Returns true if the ClientId is playing the minigame
    */
	virtual bool IsActive(int ClientId) = 0;

	/*
        ScoreType

        If your minigame is race based
        you probably want to overwrite this method
        and return EScore::SCORE_TIME
    */
	virtual EScore ScoreType() { return EScore::SCORE_BLOCK; }

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

        m_aRestorePos[ClientId] has to be set to true
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

	/*
        State

        m_State getter
    */
	virtual int State() { return m_State; }

	/*
        State

        m_State setter. You might want to overwrite that.
    */
	virtual int State(int State) { return m_State = State; }
};

#endif
