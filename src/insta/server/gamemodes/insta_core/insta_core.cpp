#include "insta_core.h"

#include <base/log.h>
#include <base/system.h>

#include <engine/console.h>
#include <engine/server/server.h>
#include <engine/shared/config.h>
#include <engine/shared/linereader.h>
#include <engine/shared/network.h>
#include <engine/shared/packer.h>
#include <engine/shared/protocol.h>

#include <generated/protocol.h>
#include <generated/protocol7.h>

#include <game/gamecore.h>
#include <game/server/entities/character.h>
#include <game/server/gamecontroller.h>
#include <game/server/gamemodes/ddnet.h>
#include <game/server/player.h>
#include <game/server/score.h>
#include <game/server/teams.h>
#include <game/version.h>

CGameControllerInstaCore::CGameControllerInstaCore(class CGameContext *pGameServer) :
	CGameControllerDDNet(pGameServer)
{
	// log_info("ddnet-insta", "initializing insta core ...");
}

CGameControllerInstaCore::~CGameControllerInstaCore() = default;

void CGameControllerInstaCore::Tick()
{
	CGameControllerDDNet::Tick();

	for(CPlayer *pPlayer : GameServer()->m_apPlayers)
	{
		if(!pPlayer)
			continue;
		if(!pPlayer->GetCharacter())
			continue;

		// ratelimit the 0.7 stuff because it requires net messages
		if(Server()->Tick() % 4 == 0)
		{
			if(pPlayer->m_SkinInfoManager.NeedsNetMessage7())
			{
				pPlayer->m_SkinInfoManager.OnSendNetMessage7();
				CTeeInfo TeeInfo = pPlayer->m_SkinInfoManager.TeeInfo();
				protocol7::CNetMsg_Sv_SkinChange Msg;
				Msg.m_ClientId = pPlayer->GetCid();
				for(int p = 0; p < protocol7::NUM_SKINPARTS; p++)
				{
					Msg.m_apSkinPartNames[p] = TeeInfo.m_aaSkinPartNames[p];
					Msg.m_aSkinPartColors[p] = TeeInfo.m_aSkinPartColors[p];
					Msg.m_aUseCustomColors[p] = TeeInfo.m_aUseCustomColors[p];
				}

				bool NetworkClip = pPlayer->GetCharacter()->HasRainbow();
				SendSkinChangeToAllSixup(&Msg, pPlayer, NetworkClip);
			}
		}
	}
}

void CGameControllerInstaCore::SendSkinChangeToAllSixup(protocol7::CNetMsg_Sv_SkinChange *pMsg, CPlayer *pPlayer, bool ApplyNetworkClipping)
{
	if(!pPlayer->GetCharacter())
		return;

	if(!ApplyNetworkClipping)
	{
		Server()->SendPackMsg(pMsg, MSGFLAG_VITAL | MSGFLAG_NORECORD, -1);
		return;
	}

	for(CPlayer *pReceivingPlayer : GameServer()->m_apPlayers)
	{
		if(!pReceivingPlayer)
			continue;
		if(!Server()->IsSixup(pReceivingPlayer->GetCid()))
			continue;

		// ddnet-insta does figure out a top scorer here but thats broken anyways
		// in ddnet++ we just break 0.7 bottom right hud rainbow and if ddnet-insta resolves
		// https://github.com/ddnet-insta/ddnet-insta/issues/633
		// we can adapt
		const bool IsTopscorer = false;

		// never clip when in scoreboard or the top scorer
		// to see the rainbow in scoreboard and hud in the bottom right
		if(!(pReceivingPlayer->m_PlayerFlags & PLAYERFLAG_SCOREBOARD) && !IsTopscorer)
			if(NetworkClipped(GameServer(), pReceivingPlayer->GetCid(), pPlayer->GetCharacter()->GetPos()))
				continue;

		Server()->SendPackMsg(pMsg, MSGFLAG_VITAL | MSGFLAG_NORECORD, pReceivingPlayer->GetCid());
	}
}
