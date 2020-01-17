/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "gamecore.h"

#include <engine/shared/config.h>
#include <engine/server/server.h>
const char *CTuningParams::m_apNames[] =
{
	#define MACRO_TUNING_PARAM(Name,ScriptName,Value,Description) #ScriptName,
	#include "tuning.h"
	#undef MACRO_TUNING_PARAM
};


bool CTuningParams::Set(int Index, float Value)
{
	if(Index < 0 || Index >= Num())
		return false;
	((CTuneParam *)this)[Index] = Value;
	return true;
}

bool CTuningParams::Get(int Index, float *pValue)
{
	if(Index < 0 || Index >= Num())
		return false;
	*pValue = (float)((CTuneParam *)this)[Index];
	return true;
}

bool CTuningParams::Set(const char *pName, float Value)
{
	for(int i = 0; i < Num(); i++)
		if(str_comp_nocase(pName, m_apNames[i]) == 0)
			return Set(i, Value);
	return false;
}

bool CTuningParams::Get(const char *pName, float *pValue)
{
	for(int i = 0; i < Num(); i++)
		if(str_comp_nocase(pName, m_apNames[i]) == 0)
			return Get(i, pValue);

	return false;
}

float HermiteBasis1(float v)
{
	return 2*v*v*v - 3*v*v+1;
}

float VelocityRamp(float Value, float Start, float Range, float Curvature)
{
	if(Value < Start)
		return 1.0f;
	return 1.0f/powf(Curvature, (Value-Start)/Range);
}

void CCharacterCore::Init(CWorldCore *pWorld, CCollision *pCollision, CTeamsCore* pTeams)
{
	m_pWorld = pWorld;
	m_pCollision = pCollision;
	m_pTeleOuts = NULL;

	m_pTeams = pTeams;
	m_Id = -1;
	m_Hook = true;
	m_Collision = true;
	m_JumpedTotal = 0;
	m_Jumps = 2;
}

void CCharacterCore::Init(CWorldCore *pWorld, CCollision *pCollision, CTeamsCore* pTeams, std::map<int, std::vector<vec2> > *pTeleOuts)
{
	m_pWorld = pWorld;
	m_pCollision = pCollision;
	m_pTeleOuts = pTeleOuts;

	m_pTeams = pTeams;
	m_Id = -1;
	m_Hook = true;
	m_Collision = true;
	m_JumpedTotal = 0;
	m_Jumps = 2;
}

void CCharacterCore::Reset()
{
	m_Pos = vec2(0,0);
	m_Vel = vec2(0,0);
	m_NewHook = false;
	m_HookPos = vec2(0,0);
	m_HookDir = vec2(0,0);
	m_HookTick = 0;
	m_HookState = HOOK_IDLE;
	m_LastHookedPlayer = -1;
	m_HookedPlayer = -1;
	m_Jumped = 0;
	m_JumpedTotal = 0;
	m_Jumps = 2;
	m_TriggeredEvents = 0;
	m_Hook = true;
	m_Collision = true;
}

void CCharacterCore::setFlagPos(int id, vec2 Pos, int Stand, vec2 Vel, int carry)
{
	if (id == 0)
	{
		m_FlagPos1 = Pos;
		m_AtStand1 = Stand;
		m_FlagVel1 = Vel;
		m_carryFlagChar1 = carry;
	}
	else if (id == 1)
	{
		m_FlagPos2 = Pos;
		m_AtStand2 = Stand;
		m_FlagVel2 = Vel;
		m_carryFlagChar2 = carry;
	}
}

void CCharacterCore::Tick(bool UseInput, bool IsClient)
{
	m_updateFlagVel = 0;

	if (m_LastHookedTick != -1){m_LastHookedTick = m_LastHookedTick + 1;}

	if (m_LastHookedTick > SERVER_TICK_SPEED*10){
			m_LastHookedPlayer = -1;
			m_LastHookedTick = -1;
		}

	float PhysSize = 28.0f;
	int MapIndex = Collision()->GetPureMapIndex(m_Pos);;
	int MapIndexL = Collision()->GetPureMapIndex(vec2(m_Pos.x + (28/2)+4,m_Pos.y));
	int MapIndexR = Collision()->GetPureMapIndex(vec2(m_Pos.x - (28/2)-4,m_Pos.y));
	int MapIndexT = Collision()->GetPureMapIndex(vec2(m_Pos.x,m_Pos.y + (28/2)+4));
	int MapIndexB = Collision()->GetPureMapIndex(vec2(m_Pos.x,m_Pos.y - (28/2)-4));
	//dbg_msg("","N%d L%d R%d B%d T%d",MapIndex,MapIndexL,MapIndexR,MapIndexB,MapIndexT);
	m_TileIndex = Collision()->GetTileIndex(MapIndex);
	m_TileFlags = Collision()->GetTileFlags(MapIndex);
	m_TileIndexL = Collision()->GetTileIndex(MapIndexL);
	m_TileFlagsL = Collision()->GetTileFlags(MapIndexL);
	m_TileIndexR = Collision()->GetTileIndex(MapIndexR);
	m_TileFlagsR = Collision()->GetTileFlags(MapIndexR);
	m_TileIndexB = Collision()->GetTileIndex(MapIndexB);
	m_TileFlagsB = Collision()->GetTileFlags(MapIndexB);
	m_TileIndexT = Collision()->GetTileIndex(MapIndexT);
	m_TileFlagsT = Collision()->GetTileFlags(MapIndexT);
	m_TileFIndex = Collision()->GetFTileIndex(MapIndex);
	m_TileFFlags = Collision()->GetFTileFlags(MapIndex);
	m_TileFIndexL = Collision()->GetFTileIndex(MapIndexL);
	m_TileFFlagsL = Collision()->GetFTileFlags(MapIndexL);
	m_TileFIndexR = Collision()->GetFTileIndex(MapIndexR);
	m_TileFFlagsR = Collision()->GetFTileFlags(MapIndexR);
	m_TileFIndexB = Collision()->GetFTileIndex(MapIndexB);
	m_TileFFlagsB = Collision()->GetFTileFlags(MapIndexB);
	m_TileFIndexT = Collision()->GetFTileIndex(MapIndexT);
	m_TileFFlagsT = Collision()->GetFTileFlags(MapIndexT);
	m_TileSIndex = (UseInput && IsRightTeam(MapIndex))?Collision()->GetDTileIndex(MapIndex):0;
	m_TileSFlags = (UseInput && IsRightTeam(MapIndex))?Collision()->GetDTileFlags(MapIndex):0;
	m_TileSIndexL = (UseInput && IsRightTeam(MapIndexL))?Collision()->GetDTileIndex(MapIndexL):0;
	m_TileSFlagsL = (UseInput && IsRightTeam(MapIndexL))?Collision()->GetDTileFlags(MapIndexL):0;
	m_TileSIndexR = (UseInput && IsRightTeam(MapIndexR))?Collision()->GetDTileIndex(MapIndexR):0;
	m_TileSFlagsR = (UseInput && IsRightTeam(MapIndexR))?Collision()->GetDTileFlags(MapIndexR):0;
	m_TileSIndexB = (UseInput && IsRightTeam(MapIndexB))?Collision()->GetDTileIndex(MapIndexB):0;
	m_TileSFlagsB = (UseInput && IsRightTeam(MapIndexB))?Collision()->GetDTileFlags(MapIndexB):0;
	m_TileSIndexT = (UseInput && IsRightTeam(MapIndexT))?Collision()->GetDTileIndex(MapIndexT):0;
	m_TileSFlagsT = (UseInput && IsRightTeam(MapIndexT))?Collision()->GetDTileFlags(MapIndexT):0;
	m_TriggeredEvents = 0;

	vec2 PrevPos = m_Pos;

	// get ground state
	bool Grounded = false;
	if(m_pCollision->CheckPoint(m_Pos.x+PhysSize/2, m_Pos.y+PhysSize/2+5))
		Grounded = true;
	if(m_pCollision->CheckPoint(m_Pos.x-PhysSize/2, m_Pos.y+PhysSize/2+5))
		Grounded = true;

	vec2 TargetDirection = normalize(vec2(m_Input.m_TargetX, m_Input.m_TargetY));

	m_Vel.y += m_pWorld->m_Tuning[g_Config.m_ClDummy].m_Gravity;

	float MaxSpeed = Grounded ? m_pWorld->m_Tuning[g_Config.m_ClDummy].m_GroundControlSpeed : m_pWorld->m_Tuning[g_Config.m_ClDummy].m_AirControlSpeed;
	float Accel = Grounded ? m_pWorld->m_Tuning[g_Config.m_ClDummy].m_GroundControlAccel : m_pWorld->m_Tuning[g_Config.m_ClDummy].m_AirControlAccel;
	float Friction = Grounded ? m_pWorld->m_Tuning[g_Config.m_ClDummy].m_GroundFriction : m_pWorld->m_Tuning[g_Config.m_ClDummy].m_AirFriction;

	// handle input
	if(UseInput)
	{
		m_Direction = m_Input.m_Direction;

		// setup angle
		float a = 0;
		if(m_Input.m_TargetX == 0)
			a = atanf((float)m_Input.m_TargetY);
		else
			a = atanf((float)m_Input.m_TargetY/(float)m_Input.m_TargetX);

		if(m_Input.m_TargetX < 0)
			a = a+pi;

		m_Angle = (int)(a*256.0f);

		// handle jump
		if(m_Input.m_Jump)
		{
			if(!(m_Jumped&1))
			{
				if(Grounded)
				{
					m_TriggeredEvents |= COREEVENT_GROUND_JUMP;
					m_Vel.y = -m_pWorld->m_Tuning[g_Config.m_ClDummy].m_GroundJumpImpulse;
					m_Jumped |= 1;
					m_JumpedTotal = 1;
				}
				else if(!(m_Jumped&2))
				{
					m_TriggeredEvents |= COREEVENT_AIR_JUMP;
					m_Vel.y = -m_pWorld->m_Tuning[g_Config.m_ClDummy].m_AirJumpImpulse;
					m_Jumped |= 3;
					m_JumpedTotal++;
				}
			}
		}
		else
			m_Jumped &= ~1;

		// handle hook
		if(m_Input.m_Hook)
		{
			if(m_HookState == HOOK_IDLE)
			{
				m_HookState = HOOK_FLYING;
				m_HookPos = m_Pos+TargetDirection*PhysSize*1.5f;
				m_HookDir = TargetDirection;
				m_HookedPlayer = -1;
				m_HookTick = SERVER_TICK_SPEED * (1.25f - m_pWorld->m_Tuning[g_Config.m_ClDummy].m_HookDuration);
				m_TriggeredEvents |= COREEVENT_HOOK_LAUNCH;
			}
		}
		else
		{
			m_HookedPlayer = -1;
			m_HookState = HOOK_IDLE;
			m_HookPos = m_Pos;
		}
	}

	// add the speed modification according to players wanted direction
	if(m_Direction < 0)
		m_Vel.x = SaturatedAdd(-MaxSpeed, MaxSpeed, m_Vel.x, -Accel);
	if(m_Direction > 0)
		m_Vel.x = SaturatedAdd(-MaxSpeed, MaxSpeed, m_Vel.x, Accel);
	if(m_Direction == 0)
		m_Vel.x *= Friction;

	// handle jumping
	// 1 bit = to keep track if a jump has been made on this input (player is holding space bar)
	// 2 bit = to keep track if a air-jump has been made (tee gets dark feet)
	if(Grounded)
	{
		m_Jumped &= ~2;
		m_JumpedTotal = 0;
	}

	// do hook
	if(m_HookState == HOOK_IDLE)
	{
		m_HookedPlayer = -1;
		m_HookState = HOOK_IDLE;
		m_HookPos = m_Pos;
	}
	else if(m_HookState >= HOOK_RETRACT_START && m_HookState < HOOK_RETRACT_END)
	{
		m_HookState++;
	}
	else if(m_HookState == HOOK_RETRACT_END)
	{
		m_HookState = HOOK_RETRACTED;
		m_TriggeredEvents |= COREEVENT_HOOK_RETRACT;
		m_HookState = HOOK_RETRACTED;
	}
	else if(m_HookState == HOOK_FLYING)
	{
		vec2 NewPos = m_HookPos+m_HookDir*m_pWorld->m_Tuning[g_Config.m_ClDummy].m_HookFireSpeed;
		if((!m_NewHook && distance(m_Pos, NewPos) > m_pWorld->m_Tuning[g_Config.m_ClDummy].m_HookLength)
		|| (m_NewHook && distance(m_HookTeleBase, NewPos) > m_pWorld->m_Tuning[g_Config.m_ClDummy].m_HookLength))
		{
			m_HookState = HOOK_RETRACT_START;
			NewPos = m_Pos + normalize(NewPos-m_Pos) * m_pWorld->m_Tuning[g_Config.m_ClDummy].m_HookLength;
			m_pReset = true;
		}

		// make sure that the hook doesn't go though the ground
		bool GoingToHitGround = false;
		bool GoingToRetract = false;
		bool GoingThroughTele = false;
		int teleNr = 0;
		int Hit = m_pCollision->IntersectLineTeleHook(m_HookPos, NewPos, &NewPos, 0, &teleNr);

		//m_NewHook = false;

		if(Hit)
		{
			if(Hit == TILE_NOHOOK)
				GoingToRetract = true;
			else if (Hit == TILE_CONFIG_1)
				GoingToRetract = g_Config.m_SvCfgTile1 == CFG_TILE_UNHOOK;
			else if (Hit == TILE_CONFIG_2)
				GoingToRetract = g_Config.m_SvCfgTile2 == CFG_TILE_UNHOOK;
			else if (Hit == TILE_TELEINHOOK)
				GoingThroughTele = true;
			else
				GoingToHitGround = true;
			m_pReset = true;
		}

		// Check against other players first
		if(this->m_Hook && m_pWorld && m_pWorld->m_Tuning[g_Config.m_ClDummy].m_PlayerHooking)
		{
			float Distance = 0.0f;
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				CCharacterCore *pCharCore = m_pWorld->m_apCharacters[i];
				if(!pCharCore || pCharCore == this || !m_pTeams->CanCollide(i, m_Id))
					continue;

				vec2 ClosestPoint = closest_point_on_line(m_HookPos, NewPos, pCharCore->m_Pos);
				if(distance(pCharCore->m_Pos, ClosestPoint) < PhysSize+2.0f)
				{
					if (m_HookedPlayer == -1 || distance(m_HookPos, pCharCore->m_Pos) < Distance)
					{
						m_TriggeredEvents |= COREEVENT_HOOK_ATTACH_PLAYER;
						m_HookState = HOOK_GRABBED;
						m_HookedPlayer = i;
						Distance = distance(m_HookPos, pCharCore->m_Pos);
						pCharCore->m_LastHookedPlayer = m_Id;
						pCharCore->m_LastHookedTick = 0;
					}
				}
			}

			//Check against Flags: DELETE IF IT DOSNT WORK
			vec2 ClosestPoint;
			ClosestPoint = closest_point_on_line(m_HookPos, NewPos, m_FlagPos1);
			if(distance(m_FlagPos1, ClosestPoint) < PhysSize+2.0f && m_AtStand1 == 0 && m_carryFlagChar1 == 0 && m_HookedPlayer != 99 && m_HookedPlayer != 98)
				{
					if (m_HookedPlayer == -1 /*|| distance(m_HookPos, m_FlagPos1) < Distance*/)
					{
						m_TriggeredEvents |= COREEVENT_HOOK_ATTACH_PLAYER;
						m_HookState = HOOK_GRABBED;
						m_HookedPlayer = 98;
						Distance = distance(m_HookPos, m_FlagPos1);
					}
				}

			ClosestPoint = closest_point_on_line(m_HookPos, NewPos, m_FlagPos2);
			if(distance(m_FlagPos2, ClosestPoint) < PhysSize+2.0f && m_AtStand2 == 0 && m_carryFlagChar2 == 0 && m_HookedPlayer != 98 && m_HookedPlayer != 99)
				{
					if (m_HookedPlayer == -1 /*|| distance(m_HookPos, m_FlagPos2) < Distance*/)
					{
						m_TriggeredEvents |= COREEVENT_HOOK_ATTACH_PLAYER;
						m_HookState = HOOK_GRABBED;
						m_HookedPlayer = 99;
						Distance = distance(m_HookPos, m_FlagPos2);
					}
				}
			//Check against Flags: DELETE IF IT DOSNT WORK

		}

		if(m_HookState == HOOK_FLYING)
		{
			// check against ground
			if(GoingToHitGround)
			{
				m_TriggeredEvents |= COREEVENT_HOOK_ATTACH_GROUND;
				m_HookState = HOOK_GRABBED;
			}
			else if(GoingToRetract)
			{
				m_TriggeredEvents |= COREEVENT_HOOK_HIT_NOHOOK;
				m_HookState = HOOK_RETRACT_START;
			}

			if(GoingThroughTele && m_pTeleOuts && m_pTeleOuts->size() && (*m_pTeleOuts)[teleNr-1].size())
			{
				m_TriggeredEvents = 0;
				m_HookedPlayer = -1;

				m_NewHook = true;
				int Num = (*m_pTeleOuts)[teleNr-1].size();
				m_HookPos = (*m_pTeleOuts)[teleNr-1][(Num==1)?0:rand() % Num]+TargetDirection*PhysSize*1.5f;
				m_HookDir = TargetDirection;
				m_HookTeleBase = m_HookPos;
			}
			else
			{
				m_HookPos = NewPos;
			}
		}
	}

	if(m_HookState == HOOK_GRABBED)
	{
		// UPDATE HOOK POS ON FLAG POS!!!!!
		if(m_HookedPlayer == 98){
			if (m_carryFlagChar1 == 0 || !m_carryFlagChar1){
				m_HookPos = m_FlagPos1;
			}
			else{
				m_HookedPlayer = -1;
				m_HookState = HOOK_RETRACTED;
				m_HookPos = m_Pos;
			}
		}
		else if(m_HookedPlayer == 99){
			if (m_carryFlagChar2 == 0 || !m_carryFlagChar2){
				m_HookPos = m_FlagPos2;
			}
			else{
				m_HookedPlayer = -1;
				m_HookState = HOOK_RETRACTED;
				m_HookPos = m_Pos;
			}
		}
		// UPDATE HOOK POS ON FLAG POS!!!!!

		else if(m_HookedPlayer != -1)
		{
			CCharacterCore *pCharCore = m_pWorld->m_apCharacters[m_HookedPlayer];
			if(pCharCore && (IsClient || m_pTeams->CanKeepHook(m_Id, pCharCore->m_Id)))
				m_HookPos = pCharCore->m_Pos;
			else
			{
				// release hook
				m_HookedPlayer = -1;
				m_HookState = HOOK_RETRACTED;
				m_HookPos = m_Pos;
			}

			// keep players hooked for a max of 1.5sec
			//if(Server()->Tick() > hook_tick+(Server()->TickSpeed()*3)/2)
				//release_hooked();
		}

		// don't do this hook rutine when we are hook to a player
		if(m_HookedPlayer == -1 && distance(m_HookPos, m_Pos) > 46.0f)
		{
			vec2 HookVel = normalize(m_HookPos-m_Pos)*m_pWorld->m_Tuning[g_Config.m_ClDummy].m_HookDragAccel;
			// the hook as more power to drag you up then down.
			// this makes it easier to get on top of an platform
			if(HookVel.y > 0)
				HookVel.y *= 0.3f;

			// the hook will boost it's power if the player wants to move
			// in that direction. otherwise it will dampen everything abit
			if((HookVel.x < 0 && m_Direction < 0) || (HookVel.x > 0 && m_Direction > 0))
				HookVel.x *= 0.95f;
			else
				HookVel.x *= 0.75f;

			vec2 NewVel = m_Vel+HookVel;

			// check if we are under the legal limit for the hook
			if(length(NewVel) < m_pWorld->m_Tuning[g_Config.m_ClDummy].m_HookDragSpeed || length(NewVel) < length(m_Vel))
				m_Vel = NewVel; // no problem. apply

		}

		// release hook (max default hook time is 1.25 s)
		m_HookTick++;
		if(m_HookedPlayer != -1 && (m_HookTick > SERVER_TICK_SPEED+SERVER_TICK_SPEED/5 || (m_HookedPlayer < 97 && !m_pWorld->m_apCharacters[m_HookedPlayer])))
		{
			m_HookedPlayer = -1;
			m_HookState = HOOK_RETRACTED;
			m_HookPos = m_Pos;
		}

		if(m_HookedPlayer == 98){
			if (m_AtStand1 == 1 /*|| !m_carryFlagChar1*/ || m_carryFlagChar1 == -1){
			m_HookedPlayer = -1;
			m_HookState = HOOK_RETRACTED;
			m_HookPos = m_Pos;}

		}
		if(m_HookedPlayer == 99){
			if (m_AtStand2 == 1 /*|| !m_carryFlagChar2*/ || m_carryFlagChar2 == -1){
			m_HookedPlayer = -1;
			m_HookState = HOOK_RETRACTED;
			m_HookPos = m_Pos;}

		}
	}

	if (m_LastHookedPlayer != -1 && !m_pWorld->m_apCharacters[m_LastHookedPlayer]){
		m_LastHookedPlayer = -1;
	}

	if(m_pWorld)
	{
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			CCharacterCore *pCharCore = m_pWorld->m_apCharacters[i];
			if(!pCharCore)
				continue;

			//player *p = (player*)ent;
			//if(pCharCore == this) // || !(p->flags&FLAG_ALIVE)

			if(pCharCore == this || (m_Id != -1 && !m_pTeams->CanCollide(m_Id, i)))
				continue; // make sure that we don't nudge our self

			// handle player <-> player collision
			float Distance = distance(m_Pos, pCharCore->m_Pos);
			vec2 Dir = normalize(m_Pos - pCharCore->m_Pos);
			if(pCharCore->m_Collision && this->m_Collision && m_pWorld->m_Tuning[g_Config.m_ClDummy].m_PlayerCollision && Distance < PhysSize*1.25f && Distance > 0.0f)
			{
				float a = (PhysSize*1.45f - Distance);
				float Velocity = 0.5f;

				// make sure that we don't add excess force by checking the
				// direction against the current velocity. if not zero.
				if (length(m_Vel) > 0.0001)
					Velocity = 1-(dot(normalize(m_Vel), Dir)+1)/2;

				m_Vel += Dir*a*(Velocity*0.75f);
				m_Vel *= 0.85f;
			}

			// handle hook influence
			if(m_Hook && m_HookedPlayer == i && m_pWorld->m_Tuning[g_Config.m_ClDummy].m_PlayerHooking)
			{
				if(Distance > PhysSize*1.50f) // TODO: fix tweakable variable
				{
					float Accel = m_pWorld->m_Tuning[g_Config.m_ClDummy].m_HookDragAccel * (Distance/m_pWorld->m_Tuning[g_Config.m_ClDummy].m_HookLength);
					float DragSpeed = m_pWorld->m_Tuning[g_Config.m_ClDummy].m_HookDragSpeed;

					// add force to the hooked player
					vec2 Temp = pCharCore->m_Vel;
					Temp.x = SaturatedAdd(-DragSpeed, DragSpeed, pCharCore->m_Vel.x, Accel*Dir.x*1.5f);
					Temp.y = SaturatedAdd(-DragSpeed, DragSpeed, pCharCore->m_Vel.y, Accel*Dir.y*1.5f);
					if(Temp.x > 0 && ((pCharCore->m_TileIndex == TILE_STOP && pCharCore->m_TileFlags == ROTATION_270) || (pCharCore->m_TileIndexL == TILE_STOP && pCharCore->m_TileFlagsL == ROTATION_270) || (pCharCore->m_TileIndexL == TILE_STOPS && (pCharCore->m_TileFlagsL == ROTATION_90 || pCharCore->m_TileFlagsL ==ROTATION_270)) || (pCharCore->m_TileIndexL == TILE_STOPA) || (pCharCore->m_TileFIndex == TILE_STOP && pCharCore->m_TileFFlags == ROTATION_270) || (pCharCore->m_TileFIndexL == TILE_STOP && pCharCore->m_TileFFlagsL == ROTATION_270) || (pCharCore->m_TileFIndexL == TILE_STOPS && (pCharCore->m_TileFFlagsL == ROTATION_90 || pCharCore->m_TileFFlagsL == ROTATION_270)) || (pCharCore->m_TileFIndexL == TILE_STOPA) || (pCharCore->m_TileSIndex == TILE_STOP && pCharCore->m_TileSFlags == ROTATION_270) || (pCharCore->m_TileSIndexL == TILE_STOP && pCharCore->m_TileSFlagsL == ROTATION_270) || (pCharCore->m_TileSIndexL == TILE_STOPS && (pCharCore->m_TileSFlagsL == ROTATION_90 || pCharCore->m_TileSFlagsL == ROTATION_270)) || (pCharCore->m_TileSIndexL == TILE_STOPA)))
						Temp.x = 0;
					if(Temp.x < 0 && ((pCharCore->m_TileIndex == TILE_STOP && pCharCore->m_TileFlags == ROTATION_90) || (pCharCore->m_TileIndexR == TILE_STOP && pCharCore->m_TileFlagsR == ROTATION_90) || (pCharCore->m_TileIndexR == TILE_STOPS && (pCharCore->m_TileFlagsR == ROTATION_90 || pCharCore->m_TileFlagsR == ROTATION_270)) || (pCharCore->m_TileIndexR == TILE_STOPA) || (pCharCore->m_TileFIndex == TILE_STOP && pCharCore->m_TileFFlags == ROTATION_90) || (pCharCore->m_TileFIndexR == TILE_STOP && pCharCore->m_TileFFlagsR == ROTATION_90) || (pCharCore->m_TileFIndexR == TILE_STOPS && (pCharCore->m_TileFFlagsR == ROTATION_90 || pCharCore->m_TileFFlagsR == ROTATION_270)) || (pCharCore->m_TileFIndexR == TILE_STOPA) || (pCharCore->m_TileSIndex == TILE_STOP && pCharCore->m_TileSFlags == ROTATION_90) || (pCharCore->m_TileSIndexR == TILE_STOP && pCharCore->m_TileSFlagsR == ROTATION_90) || (pCharCore->m_TileSIndexR == TILE_STOPS && (pCharCore->m_TileSFlagsR == ROTATION_90 || pCharCore->m_TileSFlagsR == ROTATION_270)) || (pCharCore->m_TileSIndexR == TILE_STOPA)))
						Temp.x = 0;
					if(Temp.y < 0 && ((pCharCore->m_TileIndex == TILE_STOP && pCharCore->m_TileFlags == ROTATION_180) || (pCharCore->m_TileIndexB == TILE_STOP && pCharCore->m_TileFlagsB == ROTATION_180) || (pCharCore->m_TileIndexB == TILE_STOPS && (pCharCore->m_TileFlagsB == ROTATION_0 || pCharCore->m_TileFlagsB == ROTATION_180)) || (pCharCore->m_TileIndexB == TILE_STOPA) || (pCharCore->m_TileFIndex == TILE_STOP && pCharCore->m_TileFFlags == ROTATION_180) || (pCharCore->m_TileFIndexB == TILE_STOP && pCharCore->m_TileFFlagsB == ROTATION_180) || (pCharCore->m_TileFIndexB == TILE_STOPS && (pCharCore->m_TileFFlagsB == ROTATION_0 || pCharCore->m_TileFFlagsB == ROTATION_180)) || (pCharCore->m_TileFIndexB == TILE_STOPA) || (pCharCore->m_TileSIndex == TILE_STOP && pCharCore->m_TileSFlags == ROTATION_180) || (pCharCore->m_TileSIndexB == TILE_STOP && pCharCore->m_TileSFlagsB == ROTATION_180) || (pCharCore->m_TileSIndexB == TILE_STOPS && (pCharCore->m_TileSFlagsB == ROTATION_0 || pCharCore->m_TileSFlagsB == ROTATION_180)) || (pCharCore->m_TileSIndexB == TILE_STOPA)))
						Temp.y = 0;
					if(Temp.y > 0 && ((pCharCore->m_TileIndex == TILE_STOP && pCharCore->m_TileFlags == ROTATION_0) || (pCharCore->m_TileIndexT == TILE_STOP && pCharCore->m_TileFlagsT == ROTATION_0) || (pCharCore->m_TileIndexT == TILE_STOPS && (pCharCore->m_TileFlagsT == ROTATION_0 || pCharCore->m_TileFlagsT == ROTATION_180)) || (pCharCore->m_TileIndexT == TILE_STOPA) || (pCharCore->m_TileFIndex == TILE_STOP && pCharCore->m_TileFFlags == ROTATION_0) || (pCharCore->m_TileFIndexT == TILE_STOP && pCharCore->m_TileFFlagsT == ROTATION_0) || (pCharCore->m_TileFIndexT == TILE_STOPS && (pCharCore->m_TileFFlagsT == ROTATION_0 || pCharCore->m_TileFFlagsT == ROTATION_180)) || (pCharCore->m_TileFIndexT == TILE_STOPA) || (pCharCore->m_TileSIndex == TILE_STOP && pCharCore->m_TileSFlags == ROTATION_0) || (pCharCore->m_TileSIndexT == TILE_STOP && pCharCore->m_TileSFlagsT == ROTATION_0) || (pCharCore->m_TileSIndexT == TILE_STOPS && (pCharCore->m_TileSFlagsT == ROTATION_0 || pCharCore->m_TileSFlagsT == ROTATION_180)) || (pCharCore->m_TileSIndexT == TILE_STOPA)))
						Temp.y = 0;

					// add a little bit force to the guy who has the grip
					pCharCore->m_Vel = Temp;
					Temp.x = SaturatedAdd(-DragSpeed, DragSpeed, m_Vel.x, -Accel*Dir.x*0.25f);
					Temp.y = SaturatedAdd(-DragSpeed, DragSpeed, m_Vel.y, -Accel*Dir.y*0.25f);
					if(Temp.x > 0 && ((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_270) || (m_TileIndexL == TILE_STOP && m_TileFlagsL == ROTATION_270) || (m_TileIndexL == TILE_STOPS && (m_TileFlagsL == ROTATION_90 || m_TileFlagsL ==ROTATION_270)) || (m_TileIndexL == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_270) || (m_TileFIndexL == TILE_STOP && m_TileFFlagsL == ROTATION_270) || (m_TileFIndexL == TILE_STOPS && (m_TileFFlagsL == ROTATION_90 || m_TileFFlagsL == ROTATION_270)) || (m_TileFIndexL == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_270) || (m_TileSIndexL == TILE_STOP && m_TileSFlagsL == ROTATION_270) || (m_TileSIndexL == TILE_STOPS && (m_TileSFlagsL == ROTATION_90 || m_TileSFlagsL == ROTATION_270)) || (m_TileSIndexL == TILE_STOPA)))
						Temp.x = 0;
					if(Temp.x < 0 && ((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_90) || (m_TileIndexR == TILE_STOP && m_TileFlagsR == ROTATION_90) || (m_TileIndexR == TILE_STOPS && (m_TileFlagsR == ROTATION_90 || m_TileFlagsR == ROTATION_270)) || (m_TileIndexR == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_90) || (m_TileFIndexR == TILE_STOP && m_TileFFlagsR == ROTATION_90) || (m_TileFIndexR == TILE_STOPS && (m_TileFFlagsR == ROTATION_90 || m_TileFFlagsR == ROTATION_270)) || (m_TileFIndexR == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_90) || (m_TileSIndexR == TILE_STOP && m_TileSFlagsR == ROTATION_90) || (m_TileSIndexR == TILE_STOPS && (m_TileSFlagsR == ROTATION_90 || m_TileSFlagsR == ROTATION_270)) || (m_TileSIndexR == TILE_STOPA)))
						Temp.x = 0;
					if(Temp.y < 0 && ((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_180) || (m_TileIndexB == TILE_STOP && m_TileFlagsB == ROTATION_180) || (m_TileIndexB == TILE_STOPS && (m_TileFlagsB == ROTATION_0 || m_TileFlagsB == ROTATION_180)) || (m_TileIndexB == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_180) || (m_TileFIndexB == TILE_STOP && m_TileFFlagsB == ROTATION_180) || (m_TileFIndexB == TILE_STOPS && (m_TileFFlagsB == ROTATION_0 || m_TileFFlagsB == ROTATION_180)) || (m_TileFIndexB == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_180) || (m_TileSIndexB == TILE_STOP && m_TileSFlagsB == ROTATION_180) || (m_TileSIndexB == TILE_STOPS && (m_TileSFlagsB == ROTATION_0 || m_TileSFlagsB == ROTATION_180)) || (m_TileSIndexB == TILE_STOPA)))
						Temp.y = 0;
					if(Temp.y > 0 && ((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_0) || (m_TileIndexT == TILE_STOP && m_TileFlagsT == ROTATION_0) || (m_TileIndexT == TILE_STOPS && (m_TileFlagsT == ROTATION_0 || m_TileFlagsT == ROTATION_180)) || (m_TileIndexT == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_0) || (m_TileFIndexT == TILE_STOP && m_TileFFlagsT == ROTATION_0) || (m_TileFIndexT == TILE_STOPS && (m_TileFFlagsT == ROTATION_0 || m_TileFFlagsT == ROTATION_180)) || (m_TileFIndexT == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_0) || (m_TileSIndexT == TILE_STOP && m_TileSFlagsT == ROTATION_0) || (m_TileSIndexT == TILE_STOPS && (m_TileSFlagsT == ROTATION_0 || m_TileSFlagsT == ROTATION_180)) || (m_TileSIndexT == TILE_STOPA)))
						Temp.y = 0;
					m_Vel = Temp;
				}
			}
		}

				if (m_HookedPlayer == 98 || m_HookedPlayer == 99){

				float Distance;
				vec2 FlagVel;
				vec2 Dir;
				vec2 FPos;
				vec2 Temp;

				if (m_HookedPlayer == 98){ m_updateFlagVel = 98; Temp = m_FlagVel1; FlagVel = m_FlagVel1; FPos = m_FlagPos1; Distance = distance(m_Pos, m_FlagPos1); Dir = normalize(m_Pos - m_FlagPos1);}
				if (m_HookedPlayer == 99) {m_updateFlagVel = 99; Temp = m_FlagVel2; FlagVel = m_FlagVel2; FPos = m_FlagPos2; Distance = distance(m_Pos, m_FlagPos2); Dir = normalize(m_Pos - m_FlagPos2);}

				if(Distance > PhysSize*1.50f) // TODO: fix tweakable variable
				{

				float Accel = m_pWorld->m_Tuning[g_Config.m_ClDummy].m_HookDragAccel * (Distance/m_pWorld->m_Tuning[g_Config.m_ClDummy].m_HookLength);
				float DragSpeed = m_pWorld->m_Tuning[g_Config.m_ClDummy].m_HookDragSpeed;

				Temp.x = SaturatedAdd(-DragSpeed, DragSpeed, FlagVel.x, Accel*Dir.x*1.5f);
				Temp.y = SaturatedAdd(-DragSpeed, DragSpeed, FlagVel.y, Accel*Dir.y*1.5f);

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

				Temp.x = SaturatedAdd(-DragSpeed, DragSpeed, m_Vel.x, -Accel*Dir.x*0.25f);
				Temp.y = SaturatedAdd(-DragSpeed, DragSpeed, m_Vel.y, -Accel*Dir.y*0.25f);
				if(Temp.x > 0 && ((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_270) || (m_TileIndexL == TILE_STOP && m_TileFlagsL == ROTATION_270) || (m_TileIndexL == TILE_STOPS && (m_TileFlagsL == ROTATION_90 || m_TileFlagsL ==ROTATION_270)) || (m_TileIndexL == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_270) || (m_TileFIndexL == TILE_STOP && m_TileFFlagsL == ROTATION_270) || (m_TileFIndexL == TILE_STOPS && (m_TileFFlagsL == ROTATION_90 || m_TileFFlagsL == ROTATION_270)) || (m_TileFIndexL == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_270) || (m_TileSIndexL == TILE_STOP && m_TileSFlagsL == ROTATION_270) || (m_TileSIndexL == TILE_STOPS && (m_TileSFlagsL == ROTATION_90 || m_TileSFlagsL == ROTATION_270)) || (m_TileSIndexL == TILE_STOPA)))
					Temp.x = 0;
				if(Temp.x < 0 && ((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_90) || (m_TileIndexR == TILE_STOP && m_TileFlagsR == ROTATION_90) || (m_TileIndexR == TILE_STOPS && (m_TileFlagsR == ROTATION_90 || m_TileFlagsR == ROTATION_270)) || (m_TileIndexR == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_90) || (m_TileFIndexR == TILE_STOP && m_TileFFlagsR == ROTATION_90) || (m_TileFIndexR == TILE_STOPS && (m_TileFFlagsR == ROTATION_90 || m_TileFFlagsR == ROTATION_270)) || (m_TileFIndexR == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_90) || (m_TileSIndexR == TILE_STOP && m_TileSFlagsR == ROTATION_90) || (m_TileSIndexR == TILE_STOPS && (m_TileSFlagsR == ROTATION_90 || m_TileSFlagsR == ROTATION_270)) || (m_TileSIndexR == TILE_STOPA)))
					Temp.x = 0;
				if(Temp.y < 0 && ((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_180) || (m_TileIndexB == TILE_STOP && m_TileFlagsB == ROTATION_180) || (m_TileIndexB == TILE_STOPS && (m_TileFlagsB == ROTATION_0 || m_TileFlagsB == ROTATION_180)) || (m_TileIndexB == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_180) || (m_TileFIndexB == TILE_STOP && m_TileFFlagsB == ROTATION_180) || (m_TileFIndexB == TILE_STOPS && (m_TileFFlagsB == ROTATION_0 || m_TileFFlagsB == ROTATION_180)) || (m_TileFIndexB == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_180) || (m_TileSIndexB == TILE_STOP && m_TileSFlagsB == ROTATION_180) || (m_TileSIndexB == TILE_STOPS && (m_TileSFlagsB == ROTATION_0 || m_TileSFlagsB == ROTATION_180)) || (m_TileSIndexB == TILE_STOPA)))
					Temp.y = 0;
				if(Temp.y > 0 && ((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_0) || (m_TileIndexT == TILE_STOP && m_TileFlagsT == ROTATION_0) || (m_TileIndexT == TILE_STOPS && (m_TileFlagsT == ROTATION_0 || m_TileFlagsT == ROTATION_180)) || (m_TileIndexT == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_0) || (m_TileFIndexT == TILE_STOP && m_TileFFlagsT == ROTATION_0) || (m_TileFIndexT == TILE_STOPS && (m_TileFFlagsT == ROTATION_0 || m_TileFFlagsT == ROTATION_180)) || (m_TileFIndexT == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_0) || (m_TileSIndexT == TILE_STOP && m_TileSFlagsT == ROTATION_0) || (m_TileSIndexT == TILE_STOPS && (m_TileSFlagsT == ROTATION_0 || m_TileSFlagsT == ROTATION_180)) || (m_TileSIndexT == TILE_STOPA)))
					Temp.y = 0;
				m_Vel = Temp;

				}
		}

		if (m_HookState != HOOK_FLYING)
		{
			m_NewHook = false;
		}

		int Index = MapIndex;
		if(g_Config.m_ClPredictDDRace && IsClient && m_pCollision->IsSpeedup(Index))
		{
			vec2 Direction, MaxVel, TempVel = m_Vel;
			int Force, MaxSpeed = 0;
			float TeeAngle, SpeederAngle, DiffAngle, SpeedLeft, TeeSpeed;
			m_pCollision->GetSpeedup(Index, &Direction, &Force, &MaxSpeed);
			if(Force == 255 && MaxSpeed)
			{
				m_Vel = Direction * (MaxSpeed/5);
			}
			else
			{
				if(MaxSpeed > 0 && MaxSpeed < 5) MaxSpeed = 5;
				//dbg_msg("speedup tile start","Direction %f %f, Force %d, Max Speed %d", (Direction).x,(Direction).y, Force, MaxSpeed);
				if(MaxSpeed > 0)
				{
					if(Direction.x > 0.0000001f)
						SpeederAngle = -atan(Direction.y / Direction.x);
					else if(Direction.x < 0.0000001f)
						SpeederAngle = atan(Direction.y / Direction.x) + 2.0f * asin(1.0f);
					else if(Direction.y > 0.0000001f)
						SpeederAngle = asin(1.0f);
					else
						SpeederAngle = asin(-1.0f);

					if(SpeederAngle < 0)
						SpeederAngle = 4.0f * asin(1.0f) + SpeederAngle;

					if(TempVel.x > 0.0000001f)
						TeeAngle = -atan(TempVel.y / TempVel.x);
					else if(TempVel.x < 0.0000001f)
						TeeAngle = atan(TempVel.y / TempVel.x) + 2.0f * asin(1.0f);
					else if(TempVel.y > 0.0000001f)
						TeeAngle = asin(1.0f);
					else
						TeeAngle = asin(-1.0f);

					if(TeeAngle < 0)
						TeeAngle = 4.0f * asin(1.0f) + TeeAngle;

					TeeSpeed = sqrt(pow(TempVel.x, 2) + pow(TempVel.y, 2));

					DiffAngle = SpeederAngle - TeeAngle;
					SpeedLeft = MaxSpeed / 5.0f - cos(DiffAngle) * TeeSpeed;
					//dbg_msg("speedup tile debug","MaxSpeed %i, TeeSpeed %f, SpeedLeft %f, SpeederAngle %f, TeeAngle %f", MaxSpeed, TeeSpeed, SpeedLeft, SpeederAngle, TeeAngle);
					if(abs((int)SpeedLeft) > Force && SpeedLeft > 0.0000001f)
						TempVel += Direction * Force;
					else if(abs((int)SpeedLeft) > Force)
						TempVel += Direction * -Force;
					else
						TempVel += Direction * SpeedLeft;
				}
				else
					TempVel += Direction * Force;


				if(TempVel.x > 0 && ((this->m_TileIndex == TILE_STOP && this->m_TileFlags == ROTATION_270) || (this->m_TileIndexL == TILE_STOP && this->m_TileFlagsL == ROTATION_270) || (this->m_TileIndexL == TILE_STOPS && (this->m_TileFlagsL == ROTATION_90 || this->m_TileFlagsL ==ROTATION_270)) || (this->m_TileIndexL == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_270) || (m_TileFIndexL == TILE_STOP && m_TileFFlagsL == ROTATION_270) || (m_TileFIndexL == TILE_STOPS && (m_TileFFlagsL == ROTATION_90 || m_TileFFlagsL == ROTATION_270)) || (m_TileFIndexL == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_270) || (m_TileSIndexL == TILE_STOP && m_TileSFlagsL == ROTATION_270) || (m_TileSIndexL == TILE_STOPS && (m_TileSFlagsL == ROTATION_90 || m_TileSFlagsL == ROTATION_270)) || (m_TileSIndexL == TILE_STOPA)))
					TempVel.x = 0;
				if(TempVel.x < 0 && ((this->m_TileIndex == TILE_STOP && this->m_TileFlags == ROTATION_90) || (this->m_TileIndexR == TILE_STOP && this->m_TileFlagsR == ROTATION_90) || (this->m_TileIndexR == TILE_STOPS && (this->m_TileFlagsR == ROTATION_90 || this->m_TileFlagsR == ROTATION_270)) || (this->m_TileIndexR == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_90) || (m_TileFIndexR == TILE_STOP && m_TileFFlagsR == ROTATION_90) || (m_TileFIndexR == TILE_STOPS && (m_TileFFlagsR == ROTATION_90 || m_TileFFlagsR == ROTATION_270)) || (m_TileFIndexR == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_90) || (m_TileSIndexR == TILE_STOP && m_TileSFlagsR == ROTATION_90) || (m_TileSIndexR == TILE_STOPS && (m_TileSFlagsR == ROTATION_90 || m_TileSFlagsR == ROTATION_270)) || (m_TileSIndexR == TILE_STOPA)))
					TempVel.x = 0;
				if(TempVel.y < 0 && ((this->m_TileIndex == TILE_STOP && this->m_TileFlags == ROTATION_180) || (this->m_TileIndexB == TILE_STOP && this->m_TileFlagsB == ROTATION_180) || (this->m_TileIndexB == TILE_STOPS && (this->m_TileFlagsB == ROTATION_0 || this->m_TileFlagsB == ROTATION_180)) || (this->m_TileIndexB == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_180) || (m_TileFIndexB == TILE_STOP && m_TileFFlagsB == ROTATION_180) || (m_TileFIndexB == TILE_STOPS && (m_TileFFlagsB == ROTATION_0 || m_TileFFlagsB == ROTATION_180)) || (m_TileFIndexB == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_180) || (m_TileSIndexB == TILE_STOP && m_TileSFlagsB == ROTATION_180) || (m_TileSIndexB == TILE_STOPS && (m_TileSFlagsB == ROTATION_0 || m_TileSFlagsB == ROTATION_180)) || (m_TileSIndexB == TILE_STOPA)))
					TempVel.y = 0;
				if(TempVel.y > 0 && ((this->m_TileIndex == TILE_STOP && this->m_TileFlags == ROTATION_0) || (this->m_TileIndexT == TILE_STOP && this->m_TileFlagsT == ROTATION_0) || (this->m_TileIndexT == TILE_STOPS && (this->m_TileFlagsT == ROTATION_0 || this->m_TileFlagsT == ROTATION_180)) || (this->m_TileIndexT == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_0) || (m_TileFIndexT == TILE_STOP && m_TileFFlagsT == ROTATION_0) || (m_TileFIndexT == TILE_STOPS && (m_TileFFlagsT == ROTATION_0 || m_TileFFlagsT == ROTATION_180)) || (m_TileFIndexT == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_0) || (m_TileSIndexT == TILE_STOP && m_TileSFlagsT == ROTATION_0) || (m_TileSIndexT == TILE_STOPS && (m_TileSFlagsT == ROTATION_0 || m_TileSFlagsT == ROTATION_180)) || (m_TileSIndexT == TILE_STOPA)))
					TempVel.y = 0;


				m_Vel = TempVel;
				//dbg_msg("speedup tile end","(Direction*Force) %f %f   m_Vel%f %f",(Direction*Force).x,(Direction*Force).y,m_Vel.x,m_Vel.y);
				//dbg_msg("speedup tile end","Direction %f %f, Force %d, Max Speed %d", (Direction).x,(Direction).y, Force, MaxSpeed);
			}
		}

		// jetpack and ninjajetpack prediction
		if(IsClient && UseInput && (m_Input.m_Fire&1) && (m_ActiveWeapon == WEAPON_GUN || m_ActiveWeapon == WEAPON_NINJA)) {
			m_Vel += TargetDirection * -1.0f * (m_pWorld->m_Tuning[g_Config.m_ClDummy].m_JetpackStrength / 100.0f / 6.11f);
		}

		if(g_Config.m_ClPredictDDRace && IsClient)
		{
			if(((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_270) || (m_TileIndexL == TILE_STOP && m_TileFlagsL == ROTATION_270) || (m_TileIndexL == TILE_STOPS && (m_TileFlagsL == ROTATION_90 || m_TileFlagsL ==ROTATION_270)) || (m_TileIndexL == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_270) || (m_TileFIndexL == TILE_STOP && m_TileFFlagsL == ROTATION_270) || (m_TileFIndexL == TILE_STOPS && (m_TileFFlagsL == ROTATION_90 || m_TileFFlagsL == ROTATION_270)) || (m_TileFIndexL == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_270) || (m_TileSIndexL == TILE_STOP && m_TileSFlagsL == ROTATION_270) || (m_TileSIndexL == TILE_STOPS && (m_TileSFlagsL == ROTATION_90 || m_TileSFlagsL == ROTATION_270)) || (m_TileSIndexL == TILE_STOPA)) && m_Vel.x > 0)
			{
				if((int)m_pCollision->GetPos(MapIndexL).x < (int)m_Pos.x)
					m_Pos = PrevPos;
				m_Vel.x = 0;
			}
			if(((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_90) || (m_TileIndexR == TILE_STOP && m_TileFlagsR == ROTATION_90) || (m_TileIndexR == TILE_STOPS && (m_TileFlagsR == ROTATION_90 || m_TileFlagsR == ROTATION_270)) || (m_TileIndexR == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_90) || (m_TileFIndexR == TILE_STOP && m_TileFFlagsR == ROTATION_90) || (m_TileFIndexR == TILE_STOPS && (m_TileFFlagsR == ROTATION_90 || m_TileFFlagsR == ROTATION_270)) || (m_TileFIndexR == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_90) || (m_TileSIndexR == TILE_STOP && m_TileSFlagsR == ROTATION_90) || (m_TileSIndexR == TILE_STOPS && (m_TileSFlagsR == ROTATION_90 || m_TileSFlagsR == ROTATION_270)) || (m_TileSIndexR == TILE_STOPA)) && m_Vel.x < 0)
			{
				if((int)m_pCollision->GetPos(MapIndexR).x)
					if((int)m_pCollision->GetPos(MapIndexR).x < (int)m_Pos.x)
						m_Pos = PrevPos;
				m_Vel.x = 0;
			}
			if(((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_180) || (m_TileIndexB == TILE_STOP && m_TileFlagsB == ROTATION_180) || (m_TileIndexB == TILE_STOPS && (m_TileFlagsB == ROTATION_0 || m_TileFlagsB == ROTATION_180)) || (m_TileIndexB == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_180) || (m_TileFIndexB == TILE_STOP && m_TileFFlagsB == ROTATION_180) || (m_TileFIndexB == TILE_STOPS && (m_TileFFlagsB == ROTATION_0 || m_TileFFlagsB == ROTATION_180)) || (m_TileFIndexB == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_180) || (m_TileSIndexB == TILE_STOP && m_TileSFlagsB == ROTATION_180) || (m_TileSIndexB == TILE_STOPS && (m_TileSFlagsB == ROTATION_0 || m_TileSFlagsB == ROTATION_180)) || (m_TileSIndexB == TILE_STOPA)) && m_Vel.y < 0)
			{
				if((int)m_pCollision->GetPos(MapIndexB).y)
					if((int)m_pCollision->GetPos(MapIndexB).y < (int)m_Pos.y)
						m_Pos = PrevPos;
				m_Vel.y = 0;
			}
			if(((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_0) || (m_TileIndexT == TILE_STOP && m_TileFlagsT == ROTATION_0) || (m_TileIndexT == TILE_STOPS && (m_TileFlagsT == ROTATION_0 || m_TileFlagsT == ROTATION_180)) || (m_TileIndexT == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_0) || (m_TileFIndexT == TILE_STOP && m_TileFFlagsT == ROTATION_0) || (m_TileFIndexT == TILE_STOPS && (m_TileFFlagsT == ROTATION_0 || m_TileFFlagsT == ROTATION_180)) || (m_TileFIndexT == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_0) || (m_TileSIndexT == TILE_STOP && m_TileSFlagsT == ROTATION_0) || (m_TileSIndexT == TILE_STOPS && (m_TileSFlagsT == ROTATION_0 || m_TileSFlagsT == ROTATION_180)) || (m_TileSIndexT == TILE_STOPA)) && m_Vel.y > 0)
			{
				if((int)m_pCollision->GetPos(MapIndexT).y)
					if((int)m_pCollision->GetPos(MapIndexT).y < (int)m_Pos.y)
						m_Pos = PrevPos;
				m_Vel.y = 0;
				m_Jumped = 0;
				m_JumpedTotal = 0;
			}
		}
	}

	// clamp the velocity to something sane
	if(length(m_Vel) > 6000)
		m_Vel = normalize(m_Vel) * 6000;
}

void CCharacterCore::Move()
{
	float RampValue = VelocityRamp(length(m_Vel)*50, m_pWorld->m_Tuning[g_Config.m_ClDummy].m_VelrampStart, m_pWorld->m_Tuning[g_Config.m_ClDummy].m_VelrampRange, m_pWorld->m_Tuning[g_Config.m_ClDummy].m_VelrampCurvature);

	m_Vel.x = m_Vel.x*RampValue;

	vec2 NewPos = m_Pos;

	vec2 OldVel = m_Vel;
	m_pCollision->MoveBox(&NewPos, &m_Vel, vec2(28.0f, 28.0f), 0);

	m_Colliding = 0;
	if(m_Vel.x < 0.001 && m_Vel.x > -0.001)
	{
		if(OldVel.x > 0)
			m_Colliding = 1;
		else if(OldVel.x < 0)
			m_Colliding = 2;
	}
	else
		m_LeftWall = true;

	m_Vel.x = m_Vel.x*(1.0f/RampValue);

	if(m_pWorld && m_pWorld->m_Tuning[g_Config.m_ClDummy].m_PlayerCollision && this->m_Collision)
	{
		// check player collision
		float Distance = distance(m_Pos, NewPos);
		int End = Distance+1;
		vec2 LastPos = m_Pos;
		for(int i = 0; i < End; i++)
		{
			float a = i/Distance;
			vec2 Pos = mix(m_Pos, NewPos, a);
			for(int p = 0; p < MAX_CLIENTS; p++)
			{
				CCharacterCore *pCharCore = m_pWorld->m_apCharacters[p];
				if(!pCharCore || pCharCore == this || !pCharCore->m_Collision || (m_Id != -1 && !m_pTeams->CanCollide(m_Id, p)))
					continue;
				float D = distance(Pos, pCharCore->m_Pos);
				if(D < 28.0f && D > 0.0f)
				{
					if(a > 0.0f)
						m_Pos = LastPos;
					else if(distance(NewPos, pCharCore->m_Pos) > D)
						m_Pos = NewPos;
					return;
				}
				else if(D <= 0.001f && D >= -0.001f)
				{
					if(a > 0.0f)
						m_Pos = LastPos;
					else if(distance(NewPos, pCharCore->m_Pos) > D)
						m_Pos = NewPos;
					return;
				}
			}
			LastPos = Pos;
		}
	}

	m_Pos = NewPos;
}

void CCharacterCore::Write(CNetObj_CharacterCore *pObjCore)
{
	pObjCore->m_X = round_to_int(m_Pos.x);
	pObjCore->m_Y = round_to_int(m_Pos.y);

	pObjCore->m_VelX = round_to_int(m_Vel.x*256.0f);
	pObjCore->m_VelY = round_to_int(m_Vel.y*256.0f);
	pObjCore->m_HookState = m_HookState;
	pObjCore->m_HookTick = m_HookTick;
	pObjCore->m_HookX = round_to_int(m_HookPos.x);
	pObjCore->m_HookY = round_to_int(m_HookPos.y);
	pObjCore->m_HookDx = round_to_int(m_HookDir.x*256.0f);
	pObjCore->m_HookDy = round_to_int(m_HookDir.y*256.0f);
	if (m_HookedPlayer == 98 || m_HookedPlayer == 99){
		pObjCore->m_HookedPlayer = -1;
	}
	else{pObjCore->m_HookedPlayer = m_HookedPlayer;}
	pObjCore->m_Jumped = m_Jumped;
	pObjCore->m_Direction = m_Direction;
	pObjCore->m_Angle = m_Angle;
}

void CCharacterCore::Read(const CNetObj_CharacterCore *pObjCore)
{
	m_Pos.x = pObjCore->m_X;
	m_Pos.y = pObjCore->m_Y;
	m_Vel.x = pObjCore->m_VelX/256.0f;
	m_Vel.y = pObjCore->m_VelY/256.0f;
	m_HookState = pObjCore->m_HookState;
	m_HookTick = pObjCore->m_HookTick;
	m_HookPos.x = pObjCore->m_HookX;
	m_HookPos.y = pObjCore->m_HookY;
	m_HookDir.x = pObjCore->m_HookDx/256.0f;
	m_HookDir.y = pObjCore->m_HookDy/256.0f;
	if (m_HookedPlayer == 98 || m_HookedPlayer == 99){}
	else{
	m_HookedPlayer = pObjCore->m_HookedPlayer;
	}
	m_Jumped = pObjCore->m_Jumped;
	m_Direction = pObjCore->m_Direction;
	m_Angle = pObjCore->m_Angle;
}

void CCharacterCore::Quantize()
{
	CNetObj_CharacterCore Core;
	Write(&Core);
	Read(&Core);
}

// DDRace

bool CCharacterCore::IsRightTeam(int MapIndex)
{
	if(Collision()->m_pSwitchers)
		if(m_pTeams->Team(m_Id) != (m_pTeams->m_IsDDRace16 ? VANILLA_TEAM_SUPER : TEAM_SUPER))
			return Collision()->m_pSwitchers[Collision()->GetDTileNumber(MapIndex)].m_Status[m_pTeams->Team(m_Id)];
	return false;
}

void CCharacterCore::LimitForce(vec2 *Force)
{
	vec2 Temp = *Force;
	if(Temp.x > 0 && ((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_270) || (m_TileIndexL == TILE_STOP && m_TileFlagsL == ROTATION_270) || (m_TileIndexL == TILE_STOPS && (m_TileFlagsL == ROTATION_90 || m_TileFlagsL ==ROTATION_270)) || (m_TileIndexL == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_270) || (m_TileFIndexL == TILE_STOP && m_TileFFlagsL == ROTATION_270) || (m_TileFIndexL == TILE_STOPS && (m_TileFFlagsL == ROTATION_90 || m_TileFFlagsL == ROTATION_270)) || (m_TileFIndexL == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_270) || (m_TileSIndexL == TILE_STOP && m_TileSFlagsL == ROTATION_270) || (m_TileSIndexL == TILE_STOPS && (m_TileSFlagsL == ROTATION_90 || m_TileSFlagsL == ROTATION_270)) || (m_TileSIndexL == TILE_STOPA)))
		Temp.x = 0;
	if(Temp.x < 0 && ((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_90) || (m_TileIndexR == TILE_STOP && m_TileFlagsR == ROTATION_90) || (m_TileIndexR == TILE_STOPS && (m_TileFlagsR == ROTATION_90 || m_TileFlagsR == ROTATION_270)) || (m_TileIndexR == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_90) || (m_TileFIndexR == TILE_STOP && m_TileFFlagsR == ROTATION_90) || (m_TileFIndexR == TILE_STOPS && (m_TileFFlagsR == ROTATION_90 || m_TileFFlagsR == ROTATION_270)) || (m_TileFIndexR == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_90) || (m_TileSIndexR == TILE_STOP && m_TileSFlagsR == ROTATION_90) || (m_TileSIndexR == TILE_STOPS && (m_TileSFlagsR == ROTATION_90 || m_TileSFlagsR == ROTATION_270)) || (m_TileSIndexR == TILE_STOPA)))
		Temp.x = 0;
	if(Temp.y < 0 && ((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_180) || (m_TileIndexB == TILE_STOP && m_TileFlagsB == ROTATION_180) || (m_TileIndexB == TILE_STOPS && (m_TileFlagsB == ROTATION_0 || m_TileFlagsB == ROTATION_180)) || (m_TileIndexB == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_180) || (m_TileFIndexB == TILE_STOP && m_TileFFlagsB == ROTATION_180) || (m_TileFIndexB == TILE_STOPS && (m_TileFFlagsB == ROTATION_0 || m_TileFFlagsB == ROTATION_180)) || (m_TileFIndexB == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_180) || (m_TileSIndexB == TILE_STOP && m_TileSFlagsB == ROTATION_180) || (m_TileSIndexB == TILE_STOPS && (m_TileSFlagsB == ROTATION_0 || m_TileSFlagsB == ROTATION_180)) || (m_TileSIndexB == TILE_STOPA)))
		Temp.y = 0;
	if(Temp.y > 0 && ((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_0) || (m_TileIndexT == TILE_STOP && m_TileFlagsT == ROTATION_0) || (m_TileIndexT == TILE_STOPS && (m_TileFlagsT == ROTATION_0 || m_TileFlagsT == ROTATION_180)) || (m_TileIndexT == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_0) || (m_TileFIndexT == TILE_STOP && m_TileFFlagsT == ROTATION_0) || (m_TileFIndexT == TILE_STOPS && (m_TileFFlagsT == ROTATION_0 || m_TileFFlagsT == ROTATION_180)) || (m_TileFIndexT == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_0) || (m_TileSIndexT == TILE_STOP && m_TileSFlagsT == ROTATION_0) || (m_TileSIndexT == TILE_STOPS && (m_TileSFlagsT == ROTATION_0 || m_TileSFlagsT == ROTATION_180)) || (m_TileSIndexT == TILE_STOPA)))
		Temp.y = 0;
	*Force = Temp;
}

void CCharacterCore::ApplyForce(vec2 Force)
{
	vec2 Temp = m_Vel + Force;
	LimitForce(&Temp);
	m_Vel = Temp;
}

bool UseExtraInfo(const CNetObj_Projectile *pProj)
{
	bool ExtraInfoFlag = ((abs(pProj->m_VelY) & (1<<9)) != 0);
	return ExtraInfoFlag;
}

void ExtractInfo(const CNetObj_Projectile *pProj, vec2 *StartPos, vec2 *StartVel, bool IsDDNet)
{
	if(!UseExtraInfo(pProj) || !IsDDNet)
	{
		StartPos->x = pProj->m_X;
		StartPos->y = pProj->m_Y;
		StartVel->x = pProj->m_VelX/100.0f;
		StartVel->y = pProj->m_VelY/100.0f;
	}
	else
	{
		StartPos->x = pProj->m_X/100.0f;
		StartPos->y = pProj->m_Y/100.0f;
		float Angle = pProj->m_VelX/1000000.0f;
		StartVel->x = sin(-Angle);
		StartVel->y = cos(-Angle);
	}
}

void ExtractExtraInfo(const CNetObj_Projectile *pProj, int *Owner, bool *Explosive, int *Bouncing, bool *Freeze)
{
	int Data = pProj->m_VelY;
	if(Owner)
	{
		*Owner = Data & 255;
		if((Data>>8) & 1)
			*Owner = -(*Owner);
	}
	if(Bouncing)
		*Bouncing = (Data>>10) & 3;
	if(Explosive)
		*Explosive = (Data>>12) & 1;
	if(Freeze)
		*Freeze = (Data>>13) & 1;
}

void SnapshotRemoveExtraInfo(unsigned char *pData)
{
	CSnapshot *pSnap = (CSnapshot*) pData;
	for(int Index = 0; Index < pSnap->NumItems(); Index++)
	{
		CSnapshotItem *pItem = pSnap->GetItem(Index);
		if(pItem->Type() == NETOBJTYPE_PROJECTILE)
		{
			CNetObj_Projectile* pProj = (CNetObj_Projectile*) ((void*)pItem->Data());
			if(UseExtraInfo(pProj))
			{
				vec2 Pos;
				vec2 Vel;
				ExtractInfo(pProj, &Pos, &Vel, 1);
				pProj->m_X = Pos.x;
				pProj->m_Y = Pos.y;
				pProj->m_VelX = (int)(Vel.x*100.0f);
				pProj->m_VelY = (int)(Vel.y*100.0f);
			}
		}
	}
}
