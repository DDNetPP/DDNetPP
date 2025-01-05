#include <base/system.h>
#include <engine/server.h>
#include <game/collision.h>
#include <game/server/entities/character.h>
#include <game/server/gamecontext.h>

#include "game/server/entities/laser_text.h"
#include "teleportation_request.h"

CTeleportationRequest &CTeleportationRequest::TeleportToPos(CCharacter *pCharacter, vec2 Pos)
{
	m_pCharacter = pCharacter;
	m_aErrorMsg[0] = '\0';

	if(m_IsActive)
	{
		DeferError("you already have one teleportation request pending");
		return *this;
	}

	m_IsActive = true;
	m_DestinationPos = Pos;
	str_copy(m_aDestNameLongDisplay, "teleportation", sizeof(m_aDestNameLongDisplay));
	SetName("tele");
	DelayInSeconds(10);
	return *this;
}

CTeleportationRequest &CTeleportationRequest::TeleportToTile(class CCharacter *pCharacter, int Tile)
{
	vec2 Pos = pCharacter->Collision()->GetRandomTile(Tile);
	if(Pos == vec2(-1, -1))
	{
		// fails on tick to avoid breaking initialization chains
		DeferError("destination tile not found");
	}
	return TeleportToPos(pCharacter, Pos);
}

void CTeleportationRequest::DeferError(const char *pMessage)
{
	str_copy(m_aErrorMsg, pMessage, sizeof(m_aErrorMsg));
}

CTeleportationRequest &CTeleportationRequest::OnFailure(const FTeleRequestFailure &pfnFailure)
{
	dbg_assert(IsActive(), "request not active");
	m_pfnFailure = pfnFailure;
	return *this;
}

CTeleportationRequest &CTeleportationRequest::OnSuccess(const FTeleRequestSuccess &pfnSuccess)
{
	dbg_assert(IsActive(), "request not active");
	m_pfnSuccess = pfnSuccess;
	return *this;
}

CTeleportationRequest &CTeleportationRequest::SetName(const char *pName)
{
	dbg_assert(IsActive(), "request not active");
	str_copy(m_aDestNameShortSlug, pName, sizeof(m_aDestNameShortSlug));
	return *this;
}

CTeleportationRequest &CTeleportationRequest::DelayInSeconds(int Seconds)
{
	dbg_assert(IsActive(), "request not active");
	m_TicksUntilTeleportation = Seconds * m_pCharacter->Server()->TickSpeed();
	m_Seconds = Seconds;
	return *this;
}

void CTeleportationRequest::OnDeath()
{
	dbg_assert(IsActive(), "request not active");
	TeleportFailure("request was aborted because you died");
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
		DeferError("teleport failed because you moved");
	}

	if(m_aErrorMsg[0])
	{
		TeleportFailure(m_aErrorMsg);
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
			vec2(m_pCharacter->GetPos().x + 10, m_pCharacter->GetPos().y - (5 * 32)),
			m_pCharacter->Server()->TickSpeed(),
			aSeconds);
	}
}

void CTeleportationRequest::TeleportSuccess()
{
	m_pCharacter->m_Pos = m_DestinationPos; // TODO: why does SetPosition not set the position? xd
	m_pCharacter->SetPosition(m_DestinationPos);

	m_pfnSuccess();
	m_IsActive = false;
}

void CTeleportationRequest::TeleportFailure(const char *pMessage)
{
	m_pfnFailure(pMessage);
	m_IsActive = false;
}
