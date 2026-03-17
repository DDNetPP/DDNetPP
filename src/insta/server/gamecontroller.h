#ifndef INSTA_SERVER_GAMECONTROLLER_H
#define INSTA_SERVER_GAMECONTROLLER_H
#undef INSTA_SERVER_GAMECONTROLLER_H
// hack for headerguard linter
#endif

#ifndef IN_CLASS_IGAMECONTROLLER

#include <base/vmath.h>

#include <engine/map.h>
#include <engine/shared/http.h> // ddnet-insta
#include <engine/shared/protocol.h>

#include <generated/protocol.h>
#include <generated/protocol7.h>

#include <game/server/gamecontext.h>
#include <game/server/teams.h>

struct CScoreLoadBestTimeResult;

class IGameController
{
#endif // IN_CLASS_IGAMECONTROLLER

public:
	//      _     _            _        _           _
	//   __| | __| |_ __   ___| |_     (_)_ __  ___| |_ __ _
	//  / _` |/ _` | '_ \ / _ \ __|____| | '_ \/ __| __/ _` |
	// | (_| | (_| | | | |  __/ ||_____| | | | \__ \ || (_| |
	//  \__,_|\__,_|_| |_|\___|\__|    |_|_| |_|___/\__\__,_|
	//
	//

	/*
		Function: OnCharacterDeathImpl
			Called when a CCharacter in the world dies.
			This contains the full death implementation that in regular ddnet lives
			in the `CCharacter::Die` method. You will most likely never
			have to call or override this method.
			If you want to hook into the death event have a look at
			`IGameController::OnCharacterDeath` instead.
			If you want to change parts of the implementation. Look at which
			methods the implementation calls and override those.

		Arguments:
			pVictim - The CCharacter that died.
			Killer - The client id of the killer. Can be negative!
			Weapon - What weapon that killed it. Can be -1 for undefined
				weapon when switching team or player suicides.
			SendKillMsg - if the kill infomessage for the death event should be sent to clients
	*/
	virtual void OnCharacterDeathImpl(class CCharacter *pVictim, int Killer, int Weapon, bool SendKillMsg);

	/*
		Function: SendDeathInfoMessage
			Called on character death.
			Sends the info message shown in the top right kill feed on the client.

		Arguments:
			pVictim - The CCharacter that died.
			Killer - The client id of the killer. Can be negative!
			Weapon - What weapon that killed it. Can be -1 for undefined
				weapon when switching team or player suicides.
			ModeSpecial - 0 in most cases can hold information if a flagger made a kill or was killed
				      see https://github.com/MilkeeyCat/ddnet_protocol/issues/143 for more details
	*/
	virtual void SendDeathInfoMessage(CCharacter *pVictim, int Killer, int Weapon, int ModeSpecial);

	/*
		Function: SendDeathEvent
			Called on character death.
			Plays the death sound.
			Sends the death effect snap item that will render a bursting tee on the client side.

		Arguments:
			pVictim - The CCharacter that died.
			Killer - The client id of the killer. Can be negative!
			Weapon - What weapon that killed it. Can be -1 for undefined
				weapon when switching team or player suicides.
			ModeSpecial - 0 in most cases can hold information if a flagger made a kill or was killed
				      see https://github.com/MilkeeyCat/ddnet_protocol/issues/143 for more details
	*/
	void SendDeathEvent(CCharacter *pVictim, int Killer, int Weapon);

	/*
		Function: LogKillMessage
			Called on character death.
			Prints a log message to the console about the kill.

		Arguments:
			pVictim - The CCharacter that died.
			Killer - The client id of the killer.
			Weapon - What weapon that killed it. Can be -1 for undefined
				weapon when switching team or player suicides.
			ModeSpecial - 0 in most cases can hold information if a flagger made a kill or was killed
				      see https://github.com/MilkeeyCat/ddnet_protocol/issues/143 for more details
	*/
	virtual void LogKillMessage(class CCharacter *pVictim, int Killer, int Weapon, int ModeSpecial);

private:
#ifndef IN_CLASS_IGAMECONTROLLER
};
#endif
