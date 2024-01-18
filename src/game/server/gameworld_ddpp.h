// This file can be included several times.

#ifndef IN_CLASS_GAMEWORLD

#include <game/gamecore.h>

#include <list>

class CEntity;
class CCharacter;

/*
	Class: Game World
		Tracks all entities in the game. Propagates tick and
		snap calls to all entities.
*/
class CGameWorld
{
#endif

public:
	class CCharacter *ClosestCharType(vec2 Pos, bool Human = true, CCharacter *pNotThis = 0, bool SeeAll = true); //piku pro code

	class CCharacter *ClosestCharTypeFreeze(vec2 Pos, bool Human = true, CCharacter *pNotThis = 0, bool SeeAll = true);

	class CCharacter *ClosestCharTypeNotInFreeze(vec2 Pos, bool Human = true, CCharacter *pNotThis = 0, bool SeeAll = true);

	class CCharacter *ClosestCharTypeRuler(vec2 Pos, bool Human = true, CCharacter *pNotThis = 0); //chiller was here o.O

	class CCharacter *ClosestCharTypeRuler2(vec2 Pos, bool Human = true, CCharacter *pNotThis = 0); //Ruler2 == Ruler (nur der unfreez bereieich ohne linke fwand)

	class CCharacter *ClosestCharTypeRulerWB(vec2 Pos, bool Human = true, CCharacter *pNotThis = 0);

	class CCharacter *ClosestCharTypeRulerWBBottom(vec2 Pos, bool Human = true, CCharacter *pNotThis = 0);

	class CCharacter *ClosestCharTypeTunnel(vec2 Pos, bool Human = true, CCharacter *pNotThis = 0);

	class CCharacter *ClosestCharTypeRulerLeftFreeze(vec2 Pos, bool Human = true, CCharacter *pNotThis = 0); //chiller was here o.O 2

	class CCharacter *ClosestCharTypeDummy(vec2 Pos, CCharacter *pNotThis = 0);

	class CCharacter *ClosestCharTypeFarInRace(vec2 Pos, bool Human = true, CCharacter *pNotThis = 0);

	class CCharacter *ClosestCharTypePoliceFreezeHole(vec2 Pos, bool Human, CCharacter *pNotThis = 0);

	class CCharacter *ClosestCharTypePoliceFreezePitLeft(vec2 Pos, bool Human, CCharacter *pNotThis = 0);

	class CCharacter *ClosestCharTypeUnfreezedArea5(vec2 Pos, bool Human = true, CCharacter *pNotThis = 0, bool SeeAll = true);

	class CCharacter *ClosestCharacterNoRange(vec2 Pos, CEntity *ppNotThis);

#ifndef IN_CLASS_GAMEWORLD
}
#endif
