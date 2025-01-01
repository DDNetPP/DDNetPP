#ifndef GAME_COLLISION_DDPP_H
#define GAME_COLLISION_DDPP_H

class CDDNetPPMoveRestrictionData
{
public:
	bool m_CanEnterRoom = false;

	// used to print a chat message if we hit the room tile
	bool m_RoomEnterBlocked = false;
};

#endif
