/* DDNet++ gamecore */

#include "gamecore.h"

#include <engine/server/server.h>
#include <engine/shared/config.h>

void CCharacterCore::setFlagPos(int id, vec2 Pos, int Stand, vec2 Vel, int carry)
{
	if(id == 0)
	{
		m_FlagPos1 = Pos;
		m_AtStand1 = Stand;
		m_FlagVel1 = Vel;
		m_carryFlagChar1 = carry;
	}
	else if(id == 1)
	{
		m_FlagPos2 = Pos;
		m_AtStand2 = Stand;
		m_FlagVel2 = Vel;
		m_carryFlagChar2 = carry;
	}
}

void CCharacterCore::DDPPWrite(CNetObj_CharacterCore *pObjCore)
{
	if(m_HookedPlayer == 98 || m_HookedPlayer == 99)
		pObjCore->m_HookedPlayer = -1;
	else
		pObjCore->m_HookedPlayer = m_HookedPlayer;
}

void CCharacterCore::DDPPRead(const CNetObj_CharacterCore *pObjCore)
{
	if(m_HookedPlayer == 98 || m_HookedPlayer == 99)
	{
		// pass
	}
	else
		m_HookedPlayer = pObjCore->m_HookedPlayer;
}

void CCharacterCore::DDPPTickHookFlying(vec2 NewPos)
{
	float PhysSize = 28.0f;
	if(m_HookState == HOOK_FLYING)
	{
		// Check against other players first
		if(this->m_Hook && m_pWorld && m_pWorld->m_Tuning[g_Config.m_ClDummy].m_PlayerHooking)
		{
			//Check against Flags: DELETE IF IT DOSNT WORK
			vec2 ClosestPoint;
			ClosestPoint = closest_point_on_line(m_HookPos, NewPos, m_FlagPos1);
			if(distance(m_FlagPos1, ClosestPoint) < PhysSize + 2.0f && m_AtStand1 == 0 && m_carryFlagChar1 == 0 && m_HookedPlayer != 99 && m_HookedPlayer != 98)
			{
				if(m_HookedPlayer == -1 /*|| distance(m_HookPos, m_FlagPos1) < Distance*/)
				{
					m_TriggeredEvents |= COREEVENT_HOOK_ATTACH_PLAYER;
					m_HookState = HOOK_GRABBED;
					m_HookedPlayer = 98;
				}
			}

			ClosestPoint = closest_point_on_line(m_HookPos, NewPos, m_FlagPos2);
			if(distance(m_FlagPos2, ClosestPoint) < PhysSize + 2.0f && m_AtStand2 == 0 && m_carryFlagChar2 == 0 && m_HookedPlayer != 98 && m_HookedPlayer != 99)
			{
				if(m_HookedPlayer == -1 /*|| distance(m_HookPos, m_FlagPos2) < Distance*/)
				{
					m_TriggeredEvents |= COREEVENT_HOOK_ATTACH_PLAYER;
					m_HookState = HOOK_GRABBED;
					m_HookedPlayer = 99;
				}
			}
			//Check against Flags: DELETE IF IT DOSNT WORK
		}
	}
}

void CCharacterCore::DDPPTick()
{
	float PhysSize = 28.0f;
	m_updateFlagVel = 0;

	if(m_LastHookedTick != -1)
	{
		m_LastHookedTick = m_LastHookedTick + 1;
	}

	if(m_LastHookedTick > SERVER_TICK_SPEED * 10)
	{
		m_LastHookedPlayer = -1;
		m_LastHookedTick = -1;
	}
	if(m_HookState == HOOK_GRABBED)
	{
		// UPDATE HOOK POS ON FLAG POS!!!!!
		if(m_HookedPlayer == 98)
		{
			if(!m_carryFlagChar1)
			{
				m_HookPos = m_FlagPos1;
			}
			else
			{
				m_HookedPlayer = -1;
				m_HookState = HOOK_RETRACTED;
				m_HookPos = m_Pos;
			}
		}
		else if(m_HookedPlayer == 99)
		{
			if(!m_carryFlagChar2)
			{
				m_HookPos = m_FlagPos2;
			}
			else
			{
				m_HookedPlayer = -1;
				m_HookState = HOOK_RETRACTED;
				m_HookPos = m_Pos;
			}
		}
		// UPDATE HOOK POS ON FLAG POS!!!!!

		if(m_HookedPlayer == 98)
		{
			if(m_AtStand1 == 1 /*|| !m_carryFlagChar1*/ || m_carryFlagChar1 == -1)
			{
				m_HookedPlayer = -1;
				m_HookState = HOOK_RETRACTED;
				m_HookPos = m_Pos;
			}
		}
		if(m_HookedPlayer == 99)
		{
			if(m_AtStand2 == 1 /*|| !m_carryFlagChar2*/ || m_carryFlagChar2 == -1)
			{
				m_HookedPlayer = -1;
				m_HookState = HOOK_RETRACTED;
				m_HookPos = m_Pos;
			}
		}
	}

	if(m_LastHookedPlayer != -1 && !m_pWorld->m_apCharacters[m_LastHookedPlayer])
	{
		m_LastHookedPlayer = -1;
	}

	if(m_pWorld)
	{
		if(m_HookedPlayer == 98 || m_HookedPlayer == 99)
		{
			float Distance;
			vec2 FlagVel;
			vec2 Dir;
			vec2 FPos;
			vec2 Temp;

			if(m_HookedPlayer == 98)
			{
				m_updateFlagVel = 98;
				Temp = m_FlagVel1;
				FlagVel = m_FlagVel1;
				FPos = m_FlagPos1;
				Distance = distance(m_Pos, m_FlagPos1);
				Dir = normalize(m_Pos - m_FlagPos1);
			}
			if(m_HookedPlayer == 99)
			{
				m_updateFlagVel = 99;
				Temp = m_FlagVel2;
				FlagVel = m_FlagVel2;
				FPos = m_FlagPos2;
				Distance = distance(m_Pos, m_FlagPos2);
				Dir = normalize(m_Pos - m_FlagPos2);
			}

			if(Distance > PhysSize * 1.50f) // TODO: fix tweakable variable
			{
				float Accel = m_pWorld->m_Tuning[g_Config.m_ClDummy].m_HookDragAccel * (Distance / m_pWorld->m_Tuning[g_Config.m_ClDummy].m_HookLength);
				float DragSpeed = m_pWorld->m_Tuning[g_Config.m_ClDummy].m_HookDragSpeed;

				Temp.x = SaturatedAdd(-DragSpeed, DragSpeed, FlagVel.x, Accel * Dir.x * 1.5f);
				Temp.y = SaturatedAdd(-DragSpeed, DragSpeed, FlagVel.y, Accel * Dir.y * 1.5f);

				/*int FMapIndex = Collision()->GetPureMapIndex(FPos);int FMapIndexL = Collision()->GetPureMapIndex(vec2(FPos.x + (28/2)+4,FPos.y));int FMapIndexR = Collision()->GetPureMapIndex(vec2(FPos.x - (28/2)-4,FPos.y));int FMapIndexT = Collision()->GetPureMapIndex(vec2(FPos.x,FPos.y + (28/2)+4));int FMapIndexB = Collision()->GetPureMapIndex(vec2(FPos.x,FPos.y - (28/2)-4));
				
				int m_FTileIndex = Collision()->GetTileIndex(FMapIndex);int m_FTileFlags = Collision()->GetTileFlags(FMapIndex);int m_FTileIndexL = Collision()->GetTileIndex(FMapIndexL);int m_FTileFlagsL = Collision()->GetTileFlags(FMapIndexL);int m_FTileIndexR = Collision()->GetTileIndex(FMapIndexR);
				int m_FTileFlagsR = Collision()->GetTileFlags(FMapIndexR);int m_FTileIndexB = Collision()->GetTileIndex(FMapIndexB);int m_FTileFlagsB = Collision()->GetTileFlags(FMapIndexB);int m_FTileIndexT = Collision()->GetTileIndex(FMapIndexT);int m_FTileFlagsT = Collision()->GetTileFlags(FMapIndexT);
				int m_FTileFIndex = Collision()->GetFTileIndex(FMapIndex);int m_FTileFFlags = Collision()->GetFTileFlags(FMapIndex);int m_FTileFIndexL = Collision()->GetFTileIndex(FMapIndexL);int m_FTileFFlagsL = Collision()->GetFTileFlags(FMapIndexL);int m_FTileFIndexR = Collision()->GetFTileIndex(FMapIndexR);int m_FTileFFlagsR = Collision()->GetFTileFlags(FMapIndexR);
				int m_FTileFIndexB = Collision()->GetFTileIndex(FMapIndexB);int m_FTileFFlagsB = Collision()->GetFTileFlags(FMapIndexB);int m_FTileFIndexT = Collision()->GetFTileIndex(FMapIndexT);int m_FTileFFlagsT = Collision()->GetFTileFlags(FMapIndexT);int m_FTileSIndex = (UseInput && IsRightTeam(FMapIndex))?Collision()->GetDTileIndex(FMapIndex):0;int m_FTileSFlags = (UseInput && IsRightTeam(FMapIndex))?Collision()->GetDTileFlags(FMapIndex):0;
				int m_FTileSIndexL = (UseInput && IsRightTeam(FMapIndexL))?Collision()->GetDTileIndex(FMapIndexL):0;int m_FTileSFlagsL = (UseInput && IsRightTeam(FMapIndexL))?Collision()->GetDTileFlags(FMapIndexL):0;int m_FTileSIndexR = (UseInput && IsRightTeam(FMapIndexR))?Collision()->GetDTileIndex(FMapIndexR):0;int m_FTileSFlagsR = (UseInput && IsRightTeam(FMapIndexR))?Collision()->GetDTileFlags(FMapIndexR):0;
				int m_FTileSIndexB = (UseInput && IsRightTeam(FMapIndexB))?Collision()->GetDTileIndex(FMapIndexB):0;int m_FTileSFlagsB = (UseInput && IsRightTeam(FMapIndexB))?Collision()->GetDTileFlags(FMapIndexB):0;int m_FTileSIndexT = (UseInput && IsRightTeam(FMapIndexT))?Collision()->GetDTileIndex(FMapIndexT):0;int m_FTileSFlagsT = (UseInput && IsRightTeam(FMapIndexT))?Collision()->GetDTileFlags(FMapIndexT):0;

				if(Temp.x > 0 && ((m_FTileIndex == TILE_STOP && m_FTileFlags == ROTATION_270) || (m_FTileIndexL == TILE_STOP && m_FTileFlagsL == ROTATION_270) || (m_FTileIndexL == TILE_STOPS && (m_FTileFlagsL == ROTATION_90 || m_FTileFlagsL ==ROTATION_270)) || (m_FTileIndexL == TILE_STOPA) || (m_FTileFIndex == TILE_STOP && m_FTileFFlags == ROTATION_270) || (m_FTileFIndexL == TILE_STOP && m_FTileFFlagsL == ROTATION_270) || (m_FTileFIndexL == TILE_STOPS && (m_FTileFFlagsL == ROTATION_90 || m_FTileFFlagsL == ROTATION_270)) || (m_FTileFIndexL == TILE_STOPA) || (m_FTileSIndex == TILE_STOP && m_FTileSFlags == ROTATION_270) || (m_FTileSIndexL == TILE_STOP && m_FTileSFlagsL == ROTATION_270) || (m_FTileSIndexL == TILE_STOPS && (m_FTileSFlagsL == ROTATION_90 || m_FTileSFlagsL == ROTATION_270)) || (m_FTileSIndexL == TILE_STOPA)))
					Temp.x = 0;
				if(Temp.x < 0 && ((m_FTileIndex == TILE_STOP && m_FTileFlags == ROTATION_90) || (m_FTileIndexR == TILE_STOP && m_FTileFlagsR == ROTATION_90) || (m_FTileIndexR == TILE_STOPS && (m_FTileFlagsR == ROTATION_90 || m_FTileFlagsR == ROTATION_270)) || (m_FTileIndexR == TILE_STOPA) || (m_FTileFIndex == TILE_STOP && m_FTileFFlags == ROTATION_90) || (m_FTileFIndexR == TILE_STOP && m_FTileFFlagsR == ROTATION_90) || (m_FTileFIndexR == TILE_STOPS && (m_FTileFFlagsR == ROTATION_90 || m_FTileFFlagsR == ROTATION_270)) || (m_FTileFIndexR == TILE_STOPA) || (m_FTileSIndex == TILE_STOP && m_FTileSFlags == ROTATION_90) || (m_FTileSIndexR == TILE_STOP && m_FTileSFlagsR == ROTATION_90) || (m_FTileSIndexR == TILE_STOPS && (m_FTileSFlagsR == ROTATION_90 || m_FTileSFlagsR == ROTATION_270)) || (m_FTileSIndexR == TILE_STOPA)))
					Temp.x = 0;
				if(Temp.y < 0 && ((m_FTileIndex == TILE_STOP && m_FTileFlags == ROTATION_180) || (m_FTileIndexB == TILE_STOP && m_FTileFlagsB == ROTATION_180) || (m_FTileIndexB == TILE_STOPS && (m_FTileFlagsB == ROTATION_0 || m_FTileFlagsB == ROTATION_180)) || (m_FTileIndexB == TILE_STOPA) || (m_FTileFIndex == TILE_STOP && m_FTileFFlags == ROTATION_180) || (m_FTileFIndexB == TILE_STOP && m_FTileFFlagsB == ROTATION_180) || (m_FTileFIndexB == TILE_STOPS && (m_FTileFFlagsB == ROTATION_0 || m_FTileFFlagsB == ROTATION_180)) || (m_FTileFIndexB == TILE_STOPA) || (m_FTileSIndex == TILE_STOP && m_FTileSFlags == ROTATION_180) || (m_FTileSIndexB == TILE_STOP && m_FTileSFlagsB == ROTATION_180) || (m_FTileSIndexB == TILE_STOPS && (m_FTileSFlagsB == ROTATION_0 || m_FTileSFlagsB == ROTATION_180)) || (m_FTileSIndexB == TILE_STOPA)))
					Temp.y = 0;
				if(Temp.y > 0 && ((m_FTileIndex == TILE_STOP && m_FTileFlags == ROTATION_0) || (m_FTileIndexT == TILE_STOP && m_FTileFlagsT == ROTATION_0) || (m_FTileIndexT == TILE_STOPS && (m_FTileFlagsT == ROTATION_0 || m_FTileFlagsT == ROTATION_180)) || (m_FTileIndexT == TILE_STOPA) || (m_FTileFIndex == TILE_STOP && m_FTileFFlags == ROTATION_0) || (m_FTileFIndexT == TILE_STOP && m_FTileFFlagsT == ROTATION_0) || (m_FTileFIndexT == TILE_STOPS && (m_FTileFFlagsT == ROTATION_0 || m_FTileFFlagsT == ROTATION_180)) || (m_FTileFIndexT == TILE_STOPA) || (m_FTileSIndex == TILE_STOP && m_FTileSFlags == ROTATION_0) || (m_FTileSIndexT == TILE_STOP && m_FTileSFlagsT == ROTATION_0) || (m_FTileSIndexT == TILE_STOPS && (m_FTileSFlagsT == ROTATION_0 || m_FTileSFlagsT == ROTATION_180)) || (m_FTileSIndexT == TILE_STOPA)))
					Temp.y = 0;*/

				m_UFlagVel = Temp;

				Temp.x = SaturatedAdd(-DragSpeed, DragSpeed, m_Vel.x, -Accel * Dir.x * 0.25f);
				Temp.y = SaturatedAdd(-DragSpeed, DragSpeed, m_Vel.y, -Accel * Dir.y * 0.25f);
				m_Vel = ClampVel(m_MoveRestrictions, Temp);
			}
		}
	}
}
