#ifndef INSTA_SERVER_SKIN_INFO_MANAGER_H
#define INSTA_SERVER_SKIN_INFO_MANAGER_H

#include <engine/shared/protocol.h>

#include <game/server/teeinfo.h>

#include <optional>
#include <string>

enum class ESkinPrio
{
	// the skin the client requested
	USER,

	// priority "low" should be used by the main color
	// enforced by a gamemode that supports rainbow.
	// it overwrites the user choice but gets overwritten by
	// rainbow.
	// Modes that use this are for example: zCatch
	LOW,

	// you should never write to this priority or you break rainbow
	RAINBOW,

	// priority "high" should be used by the main color
	// enforced by a gamemode that does not support rainbow.
	// It overwrites the user choice and rainbow.
	//
	// Modes that use this are for example: bomb
	HIGH,

	NUM_SKINPRIOS,
};

class CSkinOverrideRequest
{
public:
	// TODO: network clipping should be in here but its a bit complicated
	//       we need to figure out of the value that needs clipping is also applied
	//       and multiple override requests could apply at once
	//       so we need figure out if *this* request applied any non nullopt
	//       value with the priority that was actually used

	// // if set to true the skin change net message for
	// // 0.7 players will be network clipped to save bandwidth
	// // this means the skin info is not updating for players
	// // that are so far away that they don't see each other
	// //
	// // WARNING: this will not be resent as soon as the tees get close enough
	// //          only enable clipping for something spammy that sets a new color every
	// //          tick. Something like a bomb animation or rainbow.
	// bool m_NetworkClipped = false;

	bool HasAnyValue();

	// we need a std::string instead of C styled array
	// to avoid having messy std::optional code
	// could also use empty C array as unset but then
	// we can not explicitly set empty skins
	std::optional<std::string> m_SkinName = std::nullopt;
	std::optional<bool> m_UseCustomColor = std::nullopt;
	std::optional<int> m_ColorBody = std::nullopt;
	std::optional<int> m_ColorFeet = std::nullopt;

	// char m_aaSkinPartNames[protocol7::NUM_SKINPARTS][protocol7::MAX_SKIN_LENGTH] = {"", "", "", "", "", ""};
	// bool m_aUseCustomColors[protocol7::NUM_SKINPARTS] = {false, false, false, false, false, false};
	// int m_aSkinPartColors[protocol7::NUM_SKINPARTS] = {0, 0, 0, 0, 0, 0};
};

// O V E R V I E W:
//
// ddnet-insta class to manage skin name and color
// there is one instance per player and it manages
// which skin should be used
// allowing the server to force custom values onto the players
//
// L I M I T A T I O N
//
// This system only allows one skin per player.
// So we CAN NOT show one tee in different colors
// depending on who receives the info.
//
// I M P L E M E N T A T I O N:
//
// It is implemented by storing the user requested skin
// using `SetUserChoice()` every time the client sends its skin data.
//
// Then any kind of ddnet-insta system can request an overwrite of the skin
// by using method `NOT_IMPLEMENTED_YET`.
// That request will stay there until this priority is cleared again explicitly
// using the method `NOT_IMPLEMENTED_YET`.
//
// And then every time the server sends skin data to the clients
// we fetch the info from `TeeInfo()`
class CSkinInfoManager
{
	// TODO: should this also be a override request?
	// the tee info requested by the client
	CTeeInfo m_TeeInfoUserChoice;

	// last info we sent to 0.7 clients
	CTeeInfo m_LastInfoSent7;

	CSkinOverrideRequest m_aOverrideRequests[(int)ESkinPrio::NUM_SKINPRIOS];

public:
	bool NeedsNetMessage7();
	void OnSendNetMessage7();
	bool ShouldNetworkClip();

	void SetUserChoice(CTeeInfo Info);

	void SetSkinName(ESkinPrio Priority, const char *pSkinName);
	void UnsetSkinName(ESkinPrio Priority);

	// WARNING: this does not enable custom colors
	//          so if you actually want to see the color on all tees
	//          you need to set that too
	// TODO: should it set both?
	void SetColorBody(ESkinPrio Priority, int Color);
	void UnsetColorBody(ESkinPrio Priority);

	void SetColorFeet(ESkinPrio Priority, int Color);
	void UnsetColorFeet(ESkinPrio Priority);

	void SetUseCustomColor(ESkinPrio Priority, bool Value);
	void UnsetUseCustomColor(ESkinPrio Priority);

	void UnsetAll(ESkinPrio Priority);

	void SkinName(char *pSkinNameOut, int SizeOfSkinNameOut);
	int ColorBody();
	int ColorFeet();
	bool UseCustomColor();

	ESkinPrio SkinNamePriority();
	ESkinPrio ColorBodyPriority();
	ESkinPrio ColorFeetPriority();
	ESkinPrio UseCustomColorPriority();

	// has to be fetched by the server before
	// sending out info
	CTeeInfo TeeInfo();

	CTeeInfo UserChoice() { return m_TeeInfoUserChoice; }
};

#endif
