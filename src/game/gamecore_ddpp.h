#include <base/vmath.h>

class CCharacterCoreDDPP
{
public:
	class CFlagCore
	{
	public:
		vec2 m_Pos = vec2(0, 0);
		vec2 m_Vel = vec2(0, 0);
		bool m_AtStand = true;
		int m_CarrierId = -1;
	};
	CFlagCore m_aFlags[2];

	void SetFlagPos(int FlagId, vec2 Pos, int Stand, vec2 Vel, int CarrierId);

	int m_LastHookedPlayer;
	int m_LastHookedTick;
};
