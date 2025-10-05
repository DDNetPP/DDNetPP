// Hardcoded serverside bots madness
// created by yung ChillerDragon xd

#include "character.h"

#include <engine/server/server.h>
#include <engine/shared/config.h>

#include <game/server/ddpp/enums.h>
#include <game/server/ddpp/shop.h>
#include <game/server/gamecontext.h>

// twbl
#include <bots/follow/follow.h>
#include <bots/sample/sample.h>
#include <server/set_state.h>
#include <twbl/types.h>

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
	State.m_GameTick = GameServer()->Server()->Tick();
	State.m_pCollision = Collision();
	State.m_ppPlayers = GameServer()->m_apPlayers;
	State.m_pCallbackCtx = &GameServer()->m_TwblCallbackCtx;

	if(m_pPlayer->DummyMode() == DUMMYMODE_TWBL_TEST)
		Twbl_FollowTick(&State, &Bot, &GetPlayer()->m_TwblPersistentState, sizeof(GetPlayer()->m_TwblPersistentState));
	else
		dbg_msg("twbl", "error unknown mode");

	TWBL_SET_INPUT(m_SavedInput, Bot);
}

void CCharacter::DummyTick()
{
	if(!m_pPlayer->m_IsDummy)
		return;
	// save cpu if the server is empty
	// unless an integration test is running in the CI (DummyTest)
	if(GameServer()->IsServerEmpty() && m_pPlayer->m_DummyTest == EDummyTest::NONE)
		return;

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

	// this is a bit of a mess and i am not really sure why it is needed
	// but if this is not done the dummy input become unreliable
	// for example the BlmapChill police bot fails when reaching the
	// new spawn with 5 jumps but not always in the same way
	//
	// related issues
	// https://github.com/DDNetPP/DDNetPP/issues/393
	// https://github.com/ddnet/ddnet/issues/9281
	mem_copy(&m_LatestInput, &m_Input, sizeof(m_LatestInput));
	mem_copy(&m_SavedInput, &m_Input, sizeof(m_SavedInput));
}
