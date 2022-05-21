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

	switch(m_pPlayer->m_DummyMode)
	{
	case CGameContext::DUMMYMODE_DEFAULT:
		m_pDummySample->Tick();
		break;
	case CGameContext::DUMMYMODE_BLOCKWAVE:
		m_pDummyBlockWave->Tick();
		break;
	case CGameContext::DUMMYMODE_RIFLE_FNG:
		m_pDummyRifleFng->Tick();
		break;
	case CGameContext::DUMMYMODE_GRENADE_FNG:
		m_pDummyGrenadeFng->Tick();
		break;
	case CGameContext::DUMMYMODE_BLMAPV3_ARENA:
		m_pDummyBlmapV3Arena->Tick();
		break;
	case CGameContext::DUMMYMODE_ADVENTURE:
		m_pDummyAdventure->Tick();
		break;
	case CGameContext::DUMMYMODE_CHILLBLOCK5_BLOCKER_TRYHARD:
		m_pDummyChillBlock5BlockerTryHard->Tick();
		break;
	case CGameContext::DUMMYMODE_CHILLBLOCK5_RACER:
		m_pDummyChillBlock5Race->Tick();
		break;
	case CGameContext::DUMMYMODE_FNN:
		m_pDummyFNN->Tick();
		break;
	case CGameContext::DUMMYMODE_CHILLBLOCK5_BALANCE:
		m_pDummyChillBlock5Balance->Tick();
		break;
	case CGameContext::DUMMYMODE_CHILLBLOCK5_POLICE:
		m_pDummyChillBlock5Police->Tick();
		break;
	case CGameContext::DUMMYMODE_CHILLBLOCK5_BLOCKER:
		m_pDummyChillBlock5Blocker->Tick();
		break;
	case CGameContext::DUMMYMODE_BLMAPCHILL_POLICE:
		m_pDummyBlmapChillPolice->Tick();
		break;
	case CGameContext::DUMMYMODE_CHILLINTELLIGENCE:
		CITick();
		break;
	case CGameContext::DUMMYMODE_SURVIVAL:
		m_pDummySurvival->Tick();
		break;
	case CGameContext::DUMMYMODE_QUEST:
		m_pDummyQuest->Tick();
		break;
	case CGameContext::DUMMYMODE_SHOPBOT:
		m_pDummyShopBot->Tick();
		break;
	case CGameContext::DUMMYNODE_CTF5_PVP:
		m_pDummyCtf5Pvp->Tick();
		break;
	case CGameContext::DUMMYMODE_BLMAPV5_LOWER_BLOCKER:
		m_pDummyBlmapV5LowerBlocker->Tick();
		break;
	case CGameContext::DUMMYMODE_BLMAPV5_UPPER_BLOCKER:
		m_pDummyBlmapV5UpperBlocker->Tick();
		break;
	default:
		m_pPlayer->m_DummyMode = CGameContext::DUMMYMODE_DEFAULT;
		break;
	}
}
