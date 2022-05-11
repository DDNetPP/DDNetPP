// DDNet++ minigame base

#ifndef GAME_SERVER_MINIGAMES_MINIGAME_BASE_H
#define GAME_SERVER_MINIGAMES_MINIGAME_BASE_H

#include <base/system.h>

class CGameContext;

class CMinigame {
protected:
    CGameContext *m_pGameServer;
    CGameContext *GameServer();

public:
    CMinigame(CGameContext *pGameContext);
    virtual ~CMinigame(){};

    virtual void Tick() {};

    /*
        IsActive

        Returns true if the ClientID is playing the minigame
    */
    virtual bool IsActive(int ClientID) = 0;

    void SendChatAll(const char *pMessage);
    void SendBroadcastAll(const char *pMessage);

    // TODO: make this protected
    int m_State;
};

#endif
