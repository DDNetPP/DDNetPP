#ifndef GAME_SERVER_TWBL_CALLBACK_CTX_H
#define GAME_SERVER_TWBL_CALLBACK_CTX_H

#include <twbl/callback_ctx.h>

class CGameContext;

// currently using the default ddnet context shipped by twbl

// class CTwblCallbackCtx : public TWBL::CCallbackCtx
// {
// public:
// 	CGameContext *m_pGameServer;
// 	CGameContext *GameServer() const;
//
// 	void SendChat(int ClientId, int Team, const char *pText) override;
// 	void Die(int ClientId) override;
// };

#endif
