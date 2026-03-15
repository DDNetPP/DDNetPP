#include "skin_info_manager.h"

#include <base/str.h>

#include <generated/protocol7.h>

#include <game/client/skin.h>
#include <game/server/teeinfo.h>

#include <optional>

bool CSkinOverrideRequest::HasAnyValue()
{
	return m_SkinName.has_value() ||
	       m_ColorBody.has_value() ||
	       m_ColorFeet.has_value() ||
	       m_UseCustomColor.has_value();
}

// CSkinInfoManager::CSkinInfoManager()
// {
// 	m_aOverrideRequests[(int)ESkinPrio::RAINBOW].m_NetworkClipped = true;
// }

bool CSkinInfoManager::NeedsNetMessage7()
{
	// TODO: use cached reference to avoid duplicated memory copy
	CTeeInfo Info = TeeInfo();

	// TODO: checking 0.6 values for 0.7 update makes little sense
	//       but it kinda works because we translate it first

	if(str_comp(m_LastInfoSent7.m_aSkinName, Info.m_aSkinName))
		return true;
	if(m_LastInfoSent7.m_ColorBody != Info.m_ColorBody)
		return true;
	if(m_LastInfoSent7.m_ColorFeet != Info.m_ColorFeet)
		return true;
	if(m_LastInfoSent7.m_UseCustomColor != Info.m_UseCustomColor)
		return true;

	return false;
}

void CSkinInfoManager::OnSendNetMessage7()
{
	// TODO: use cached reference to avoid duplicated memory copy
	CTeeInfo Info = TeeInfo();

	// TODO: storing 0.6 values for 0.7 update makes little sense
	//       but it kinda works because we translate it first
	str_copy(m_LastInfoSent7.m_aSkinName, Info.m_aSkinName);
	m_LastInfoSent7.m_ColorBody = Info.m_ColorBody;
	m_LastInfoSent7.m_ColorFeet = Info.m_ColorFeet;
	m_LastInfoSent7.m_UseCustomColor = Info.m_UseCustomColor;
}

void CSkinInfoManager::SetUserChoice(CTeeInfo Info)
{
	m_TeeInfoUserChoice = Info;
}

void CSkinInfoManager::SetSkinName(ESkinPrio Priority, const char *pSkinName)
{
	m_aOverrideRequests[(int)Priority].m_SkinName = std::string(pSkinName);
}

void CSkinInfoManager::UnsetSkinName(ESkinPrio Priority)
{
	m_aOverrideRequests[(int)Priority].m_SkinName = std::nullopt;
}

void CSkinInfoManager::SetColorBody(ESkinPrio Priority, int Color)
{
	m_aOverrideRequests[(int)Priority].m_ColorBody = Color;
}

void CSkinInfoManager::UnsetColorBody(ESkinPrio Priority)
{
	m_aOverrideRequests[(int)Priority].m_ColorBody = std::nullopt;
}

void CSkinInfoManager::SetColorFeet(ESkinPrio Priority, int Color)
{
	m_aOverrideRequests[(int)Priority].m_ColorFeet = Color;
}

void CSkinInfoManager::UnsetColorFeet(ESkinPrio Priority)
{
	m_aOverrideRequests[(int)Priority].m_ColorFeet = std::nullopt;
}

void CSkinInfoManager::SetUseCustomColor(ESkinPrio Priority, bool Value)
{
	m_aOverrideRequests[(int)Priority].m_UseCustomColor = Value;
}

void CSkinInfoManager::UnsetUseCustomColor(ESkinPrio Priority)
{
	m_aOverrideRequests[(int)Priority].m_UseCustomColor = std::nullopt;
}

void CSkinInfoManager::UnsetAll(ESkinPrio Priority)
{
	UnsetSkinName(Priority);
	UnsetColorBody(Priority);
	UnsetColorFeet(Priority);
	UnsetUseCustomColor(Priority);
}

void CSkinInfoManager::SkinName(char *pSkinNameOut, int SizeOfSkinNameOut)
{
	for(int i = (int)ESkinPrio::NUM_SKINPRIOS - 1; i > 0; i--)
	{
		if(!m_aOverrideRequests[i].m_SkinName.has_value())
			continue;
		str_copy(pSkinNameOut, m_aOverrideRequests[i].m_SkinName.value().c_str(), SizeOfSkinNameOut);
		return;
	}
	str_copy(pSkinNameOut, m_TeeInfoUserChoice.m_aSkinName, SizeOfSkinNameOut);
}

int CSkinInfoManager::ColorBody()
{
	for(int i = (int)ESkinPrio::NUM_SKINPRIOS - 1; i > 0; i--)
	{
		if(!m_aOverrideRequests[i].m_ColorBody.has_value())
			continue;
		return m_aOverrideRequests[i].m_ColorBody.value();
	}
	return m_TeeInfoUserChoice.m_ColorBody;
}

int CSkinInfoManager::ColorFeet()
{
	for(int i = (int)ESkinPrio::NUM_SKINPRIOS - 1; i > 0; i--)
	{
		if(!m_aOverrideRequests[i].m_ColorFeet.has_value())
			continue;
		return m_aOverrideRequests[i].m_ColorFeet.value();
	}
	return m_TeeInfoUserChoice.m_ColorFeet;
}

bool CSkinInfoManager::UseCustomColor()
{
	for(int i = (int)ESkinPrio::NUM_SKINPRIOS - 1; i > 0; i--)
	{
		if(!m_aOverrideRequests[i].m_UseCustomColor.has_value())
			continue;
		return m_aOverrideRequests[i].m_UseCustomColor.value();
	}
	return m_TeeInfoUserChoice.m_UseCustomColor;
}

ESkinPrio CSkinInfoManager::SkinNamePriority()
{
	for(int i = (int)ESkinPrio::NUM_SKINPRIOS - 1; i > 0; i--)
	{
		if(!m_aOverrideRequests[i].m_SkinName.has_value())
			continue;
		return (ESkinPrio)i;
	}
	return ESkinPrio::USER;
}

ESkinPrio CSkinInfoManager::ColorBodyPriority()
{
	for(int i = (int)ESkinPrio::NUM_SKINPRIOS - 1; i > 0; i--)
	{
		if(!m_aOverrideRequests[i].m_ColorBody.has_value())
			continue;
		return (ESkinPrio)i;
	}
	return ESkinPrio::USER;
}

ESkinPrio CSkinInfoManager::ColorFeetPriority()
{
	for(int i = (int)ESkinPrio::NUM_SKINPRIOS - 1; i > 0; i--)
	{
		if(!m_aOverrideRequests[i].m_ColorFeet.has_value())
			continue;
		return (ESkinPrio)i;
	}
	return ESkinPrio::USER;
}

ESkinPrio CSkinInfoManager::UseCustomColorPriority()
{
	for(int i = (int)ESkinPrio::NUM_SKINPRIOS - 1; i > 0; i--)
	{
		if(!m_aOverrideRequests[i].m_UseCustomColor.has_value())
			continue;
		return (ESkinPrio)i;
	}
	return ESkinPrio::USER;
}

CTeeInfo CSkinInfoManager::TeeInfo()
{
	// TODO: cache this
	CTeeInfo Info = m_TeeInfoUserChoice;

	// 0.6
	SkinName(Info.m_aSkinName, sizeof(Info.m_aSkinName));
	Info.m_ColorBody = ColorBody();
	Info.m_ColorFeet = ColorFeet();
	Info.m_UseCustomColor = UseCustomColor();

	// 0.7

	// The server requests skin overrides in the 0.6 format
	// so in that case we are translating the request to 0.7 manually and partially here.
	//
	// Could also use the ddnet skin translation helper ToSixup() here
	// but I decided against that to be able to mix user choice with server overrides
	// and only translate what the server forcefully changed.
	//
	// related issues:
	// https://github.com/ddnet-insta/ddnet-insta/issues/595
	// https://github.com/ddnet-insta/ddnet-insta/issues/547
	//
	// ^
	// you should read, understand and retest these issues before and after
	// changing this code

	if(SkinNamePriority() > ESkinPrio::USER)
	{
		SkinName(Info.m_aaSkinPartNames[protocol7::SKINPART_BODY], sizeof(Info.m_aaSkinPartNames[protocol7::SKINPART_BODY]));
	}

	if(ColorBodyPriority() > ESkinPrio::USER)
	{
		const int ColorBodySeven = ColorHSLA(ColorBody()).UnclampLighting(ColorHSLA::DARKEST_LGT).Pack(ColorHSLA::DARKEST_LGT7);
		Info.m_aSkinPartColors[protocol7::SKINPART_BODY] = ColorBodySeven;
		Info.m_aSkinPartColors[protocol7::SKINPART_MARKING] = 0x22FFFFFF; // magic value from ddnet teeinfo.cpp
		Info.m_aSkinPartColors[protocol7::SKINPART_DECORATION] = ColorBodySeven;
		Info.m_aSkinPartColors[protocol7::SKINPART_HANDS] = ColorBodySeven;
	}

	if(ColorFeetPriority() > ESkinPrio::USER)
	{
		const int ColorFeetSeven = ColorHSLA(ColorFeet()).UnclampLighting(ColorHSLA::DARKEST_LGT).Pack(ColorHSLA::DARKEST_LGT7);
		Info.m_aSkinPartColors[protocol7::SKINPART_FEET] = ColorFeetSeven;
	}

	if(UseCustomColorPriority() > ESkinPrio::USER)
	{
		// could also only apply it to all parts except feet
		// or only feet depending on which color was set by the server

		for(bool &PartUseCustomColor : Info.m_aUseCustomColors)
			PartUseCustomColor = UseCustomColor();
	}

	return Info;
}
