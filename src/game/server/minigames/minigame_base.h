// DDNet++ minigame base

#ifndef GAME_SERVER_MINIGAMES_MINIGAME_BASE_H
#define GAME_SERVER_MINIGAMES_MINIGAME_BASE_H

#include <base/system.h>

class CGameContext;

class CMinigame {
protected:
    CGameContext *m_pGameServer;

public:
    CMinigame(CGameContext *pGameContext);
    virtual ~CMinigame(){};

    virtual void Tick() {};

private:
};

#endif
