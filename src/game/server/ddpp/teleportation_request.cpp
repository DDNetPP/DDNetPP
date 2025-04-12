#include <base/system.h>
#include <engine/server.h>
#include <engine/shared/config.h>
#include <game/collision.h>
#include <game/server/entities/character.h>
#include <game/server/entities/laser_text.h>
#include <game/server/gamecontext.h>

#include "teleportation_request.h"

CTeleportationRequest &CTeleportationRequest::TeleportToPos(CCharacter *pCharacter, vec2 Pos)
{
	m_pCharacter = pCharacter;
	m_aErrorMsgShort[0] = '\0';
	m_aErrorMsgLong[0] = '\0';

	if(m_IsActive)
	{
		DeferError("conflicting tele request", "You already have one teleportation request pending.");
		return *this;
	}

	m_IsActive = true;
	m_MoveWarningPrinted = false;
	m_RequestStartTick = pCharacter->Server()->Tick();
	m_DestinationPos = Pos;
	DelayInSeconds(10);
	return *this;
}

CTeleportationRequest &CTeleportationRequest::TeleportToTile(class CCharacter *pCharacter, int Tile, int Offset)
{
	vec2 Pos;
	if(Offset == -1)
		Pos = pCharacter->Collision()->GetRandomTile(Tile);
	else
		Pos = pCharacter->Collision()->GetTileAtNum(Tile, Offset);
	if(Pos == vec2(-1, -1))
	{
		// fails on tick to avoid breaking initialization chains
		DeferError("missing tile", "Destination tile not found.");
	}
	return TeleportToPos(pCharacter, Pos);
}

void CTeleportationRequest::Abort()
{
	m_IsActive = false;
}

void CTeleportationRequest::DeferError(const char *pMessageShort, const char *pMessageLong)
{
	str_copy(m_aErrorMsgShort, pMessageShort, sizeof(m_aErrorMsgShort));
	str_copy(m_aErrorMsgLong, pMessageLong, sizeof(m_aErrorMsgLong));
}

CTeleportationRequest &CTeleportationRequest::OnFailure(const FTeleRequestFailure &pfnFailure)
{
	dbg_assert(IsActive(), "request not active");
	m_pfnFailure = pfnFailure;
	return *this;
}

CTeleportationRequest &CTeleportationRequest::OnPreSuccess(const FTeleRequestSuccess &pfnSuccess)
{
	dbg_assert(IsActive(), "request not active");
	m_pfnPreSuccess = pfnSuccess;
	return *this;
}

CTeleportationRequest &CTeleportationRequest::OnPostSuccess(const FTeleRequestSuccess &pfnSuccess)
{
	dbg_assert(IsActive(), "request not active");
	m_pfnPostSuccess = pfnSuccess;
	return *this;
}

CTeleportationRequest &CTeleportationRequest::DelayInSeconds(int Seconds)
{
	dbg_assert(IsActive(), "request not active");
	m_TicksUntilTeleportation = Seconds * m_pCharacter->Server()->TickSpeed();
	m_TicksUntilTeleportation++; // make sure the first laser text is printed
	m_Seconds = Seconds;

	if(!g_Config.m_SvTeleportationDelay)
		m_TicksUntilTeleportation = 0;

	return *this;
}

void CTeleportationRequest::OnDeath()
{
	if(!IsActive())
		return;

	TeleportFailure("died", "Teleportation was aborted because you died.");
}

void CTeleportationRequest::Tick()
{
	if(!IsActive())
		return;

	if(m_pCharacter->Core()->m_Vel.x < -0.06f ||
		m_pCharacter->Core()->m_Vel.x > 0.06f ||
		m_pCharacter->Core()->m_Vel.y > 0.6f ||
		m_pCharacter->Core()->m_Vel.y < -0.6f)
	{
		int SecondsSinceStart = (m_pCharacter->Server()->Tick() - m_RequestStartTick) / m_pCharacter->Server()->TickSpeed();
		if(SecondsSinceStart > 1)
		{
			DeferError("moved", "Teleportation failed because you moved.");
		}
		else
		{
			// restart timer on move
			DelayInSeconds(m_Seconds);

			if(!m_MoveWarningPrinted)
			{
				m_pCharacter->GameServer()->SendChatTarget(m_pCharacter->GetPlayer()->GetCid(), "[Teleportation] Stop moving or the request will fail.");
				m_MoveWarningPrinted = true;
			}
		}
	}

	if(m_aErrorMsgShort[0])
	{
		TeleportFailure(m_aErrorMsgShort, m_aErrorMsgLong);
		return;
	}

	m_TicksUntilTeleportation--;
	if(m_TicksUntilTeleportation <= 0)
	{
		TeleportSuccess();
		return;
	}

	if(m_TicksUntilTeleportation % m_pCharacter->Server()->TickSpeed() == 0)
	{
		int Seconds = m_TicksUntilTeleportation / m_pCharacter->Server()->TickSpeed();
		char aSeconds[16];
		str_format(aSeconds, sizeof(aSeconds), "%d", Seconds);

		new CLaserText(
			m_pCharacter->GameWorld(),
			vec2(m_pCharacter->GetPos().x - 20, m_pCharacter->GetPos().y - (5 * 32)),
			m_pCharacter->Server()->TickSpeed(),
			aSeconds);
	}
}

void CTeleportationRequest::TeleportSuccess()
{
	if(!IsActive())
		return;

	m_pfnPreSuccess();

	m_pCharacter->m_Pos = m_DestinationPos; // TODO: why does SetPosition not set the position? xd
	m_pCharacter->SetPosition(m_DestinationPos);
	m_IsActive = false;

	m_pfnPostSuccess();
}

void CTeleportationRequest::TeleportFailure(const char *pMessageShort, const char *pMessageLong)
{
	if(!IsActive())
		return;

	m_IsActive = false;

	m_pfnFailure(pMessageShort, pMessageLong);
}
