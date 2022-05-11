// DDNet++ minigame base

#include <game/server/gamecontext.h>

#include "minigame_base.h"

CMinigame::CMinigame(CGameContext *pGameContext)
{
    m_pGameServer = pGameContext;
    m_State = 0;
}

CGameContext *CMinigame::GameServer()
{
    return m_pGameServer;
}

void CMinigame::SendChatAll(const char *pMessage)
{
    for(auto &Player : GameServer()->m_apPlayers)
        if(Player)
            if(IsActive(Player->GetCID()))
                GameServer()->SendChatTarget(Player->GetCID(), pMessage);
}

void CMinigame::SendBroadcastAll(const char *pMessage)
{
    for(auto &Player : GameServer()->m_apPlayers)
        if(Player)
            if(IsActive(Player->GetCID()))
                GameServer()->SendChatTarget(Player->GetCID(), pMessage);
}
