#ifndef INSTA_SERVER_GAMEMODES_INSTA_CORE_INSTA_CORE_H
#define INSTA_SERVER_GAMEMODES_INSTA_CORE_INSTA_CORE_H

#include <generated/protocol7.h>

#include <game/server/gamemodes/ddnet.h>

class CGameControllerInstaCore : public CGameControllerDDNet
{
public:
	CGameControllerInstaCore(class CGameContext *pGameServer);
	~CGameControllerInstaCore() override;

	// convenience accessors to copy code from gamecontext to the instagib controller
	class IServer *Server() const { return GameServer()->Server(); }
	class CConfig *Config() { return GameServer()->Config(); }
	class IConsole *Console() { return GameServer()->Console(); }
	class IStorage *Storage() { return GameServer()->Storage(); }

	void Tick() override;

private:
	// pPlayer is the player whose skin changed
	// not the receiver of the message
	// the message is sent to all 0.7 players that are in clip region
	void SendSkinChangeToAllSixup(protocol7::CNetMsg_Sv_SkinChange *pMsg, CPlayer *pPlayer, bool ApplyNetworkClipping);
};
#endif
