/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "4laserbox.h"

/* Cyser!xXx's better Box Code 10.06.2012*/
///////////////////////////////////////////
//
//  CBOX = 4laserbox
//
///////////////////////////////////////////
CBox::CBox(CGameWorld *pGameWorld, vec2 Pos, int Owner)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER)
{ 
	m_Pos = Pos;            
	m_Owner = Owner;      

	GameWorld()->InsertEntity(this);  

	this->m_ID2 = Server()->SnapNewID(); 
	this->m_ID3 = Server()->SnapNewID();
	this->m_ID4 = Server()->SnapNewID();
}

void CBox::Reset() 
{
	GameServer()->m_World.DestroyEntity(this); 
}

void CBox::Tick()
{
	CCharacter *pOwnerchar = GameServer()->GetPlayerChar(m_Owner);
	CCharacter *pChr = GameServer()->m_World.ClosestCharacter(m_Pos, 20.0f, 0); 

	/*Check*/
	if(m_Owner != -1 && !pOwnerchar)
		m_Owner = -1;
	
	/*Where, the box must be placed*/
	if(m_Owner != -1)
	{
		if(pOwnerchar)
			m_Pos = pOwnerchar->m_Pos; 
	}
	
	//BoxPhysics(); 

	if(m_Owner == -1) //if player is dead or not available:
	{
		/*Fall down of the box*/
		m_Vel.y += GameWorld()->m_Core.m_Tuning.m_Gravity;
		m_Vel.x*=0.97f;

		if(m_Vel.x < 0.01f && m_Vel.x > -0.01f)
			m_Vel.x = 0;
		
		GameServer()->Collision()->MoveBox(&m_Pos,&m_Vel, vec2(28.0f*5, 28.0f*5), 0.5f);
	
		/*Enter in the box*/
		if(pChr)
		{
			m_Owner = pChr->GetPlayer()->GetCID();
			//pChr->GetPlayer()->m_IsInBox = true;
		}
	}
}

void CBox::BoxPhysics()
{
	//CCharacter *pChr = GameServer()->GetPlayerChar(m_Owner);
	
	//
}

void CBox::Snap(int SnappingClient)
{
	CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_ID, sizeof(CNetObj_Laser)));  
	if(!pObj)
		return;

	pObj->m_X = (int)m_Pos.x-50;        
	pObj->m_Y = (int)m_Pos.y+35;      
	pObj->m_FromX = (int)m_Pos.x+50;   
	pObj->m_FromY = (int)m_Pos.y+35;
	pObj->m_StartTick = Server()->Tick()-1.5;
	
	
	CNetObj_Laser *pObj2 = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_ID2, sizeof(CNetObj_Laser))); 
	if(!pObj2)
		return;

	pObj2->m_X = (int)m_Pos.x+50;
	pObj2->m_Y = (int)m_Pos.y+35;
	pObj2->m_FromX = (int)m_Pos.x+50;
	pObj2->m_FromY = (int)m_Pos.y-35;
	pObj2->m_StartTick = Server()->Tick()-1.5;	
	
	CNetObj_Laser *pObj3 = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_ID3, sizeof(CNetObj_Laser)));
	if(!pObj3)
		return;

	pObj3->m_X = (int)m_Pos.x+50;
	pObj3->m_Y = (int)m_Pos.y-35;
	pObj3->m_FromX = (int)m_Pos.x-50;
	pObj3->m_FromY = (int)m_Pos.y-35;
	pObj3->m_StartTick = Server()->Tick()-1.5;
	
	CNetObj_Laser *pObj4 = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_ID4, sizeof(CNetObj_Laser))); 
	if(!pObj4)
		return;

	pObj4->m_X = (int)m_Pos.x-50;
	pObj4->m_Y = (int)m_Pos.y-35;
	pObj4->m_FromX = (int)m_Pos.x-50;
	pObj4->m_FromY = (int)m_Pos.y+35;
	pObj4->m_StartTick = Server()->Tick()-1.5;
}
