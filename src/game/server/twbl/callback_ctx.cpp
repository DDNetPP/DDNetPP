#include "callback_ctx.h"

#include <engine/shared/protocol.h>

#include <game/server/gamecontext.h>

// currently using the default ddnet context shipped by twbl

// CGameContext *CTwblCallbackCtx::GameServer() const
// {
// 	return m_pGameServer;
// }
//
// void CTwblCallbackCtx::SendChat(int ClientId, int Team, const char *pText)
// {
// 	GameServer()->SendChat(ClientId, Team, pText);
// }
//
// void CTwblCallbackCtx::Die(int ClientId)
// {
// 	if(ClientId < 0 || ClientId >= MAX_CLIENTS)
// 		return;
//
// 	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
// 	if(!pPlayer)
// 		return;
//
// 	pPlayer->KillCharacter(WEAPON_SELF);
// }
