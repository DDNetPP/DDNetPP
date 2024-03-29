// Hardcoded serverside bots madness
// created by yung ChillerDragon xd

#include "character.h"

#include <engine/server/server.h>
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>

#include <game/server/ddpp/shop.h>

#include <fstream> //ChillerDragon saving bot move records
#include <string> //ChillerDragon std::getline

void CCharacter::Fire(bool Fire)
{
	if(Fire)
	{
		m_LatestInput.m_Fire++;
		m_Input.m_Fire++;
	}
	else
	{
		m_LatestInput.m_Fire = 0;
		m_Input.m_Fire = 0;
	}
}

void CCharacter::DummyTick()
{
	if(!m_pPlayer->m_IsDummy)
		return;
	if(GameServer()->IsServerEmpty())
		return;

	m_Input.m_TargetX = 200;
	m_Input.m_TargetY = 200;
	m_LatestInput.m_TargetX = 200;
	m_LatestInput.m_TargetY = 200;
	if((m_pPlayer->m_rainbow_offer != m_pPlayer->m_DummyRainbowOfferAmount) && !m_Rainbow)
	{
		m_Rainbow = true;
		m_pPlayer->m_rainbow_offer = 0;
		m_pPlayer->m_DummyRainbowOfferAmount = m_pPlayer->m_rainbow_offer;
	}
	else if((m_pPlayer->m_rainbow_offer != m_pPlayer->m_DummyRainbowOfferAmount) && m_Rainbow)
	{
		m_Rainbow = false;
		m_pPlayer->m_rainbow_offer = 0;
		m_pPlayer->m_DummyRainbowOfferAmount = m_pPlayer->m_rainbow_offer;
	}

	if(m_pPlayer->DummyMode() == DUMMYMODE_CHILLINTELLIGENCE)
		CITick();
	else if(m_pPlayer->m_pDummyMode)
		m_pPlayer->m_pDummyMode->Tick(this);
	else if(m_pPlayer->DummyMode() != DUMMYMODE_DEFAULT)
		m_pPlayer->SetDummyMode(DUMMYMODE_DEFAULT);
}
