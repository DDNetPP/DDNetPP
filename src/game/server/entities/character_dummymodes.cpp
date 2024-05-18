// Hardcoded serverside bots madness
// created by yung ChillerDragon xd

#include "character.h"

#include <engine/server/server.h>
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>

#include <game/server/ddpp/shop.h>

// twbl
#include <bots/ddpp_test.h>
#include <server/set_state.h>
#include <shared/types.h>

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

void CCharacter::TwblTick()
{
	CServerBotStateIn State;
	CServerBotStateOut Bot;
	TWBL::SetState(this, &State);

	if(m_pPlayer->DummyMode() == DUMMYMODE_TWBL_TEST)
		TwblTestTick(&State, &Bot);
	else
		dbg_msg("twbl", "error unknown mode");

	// TODO: this should be moved to a method or macro in twbl/server/*
	m_SavedInput.m_Direction = Bot.m_Direction;
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
	else if(m_pPlayer->DummyMode() > DUMMYMODE_TWBL_START && m_pPlayer->DummyMode() < DUMMYMODE_TWBL_END)
		TwblTick();
	else if(m_pPlayer->m_pDummyMode)
		m_pPlayer->m_pDummyMode->Tick(this);
	else if(m_pPlayer->DummyMode() != DUMMYMODE_DEFAULT)
		m_pPlayer->SetDummyMode(DUMMYMODE_DEFAULT);
}
