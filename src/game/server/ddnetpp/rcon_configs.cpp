#include <base/log.h>
#include <base/system.h>

#include <engine/server/server.h>
#include <engine/shared/config.h>

#include <game/server/entities/character.h>
#include <game/server/gamecontext.h>
#include <game/server/gamecontroller.h>
#include <game/server/player.h>

void CGameContext::RegisterDDNetPPCommands()
{
	// config chains
	Console()->Chain("sv_captcha_room", ConchainCaptchaRoom, this);
	Console()->Chain("sv_display_score", ConchainDisplayScore, this);

	// chat commands
#define CHAT_COMMAND(name, params, flags, callback, userdata, help) Console()->Register(name, params, flags, callback, userdata, help);
#include <game/server/ddnetpp/chat_commands.h>
#undef CHAT_COMMAND

	// rcon commands
#define CONSOLE_COMMAND(name, params, flags, callback, userdata, help) Console()->Register(name, params, flags, callback, userdata, help);
#include <game/ddracecommands_ddpp.h>
#undef CONSOLE_COMMAND
}

