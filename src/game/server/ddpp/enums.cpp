#include "enums.h"

#include <base/system.h>

bool str_to_display_score(const char *pInputText, EDisplayScore *pDisplayScore)
{
	if(!pInputText || pInputText[0] == '\0')
		return false;

	if(!str_comp_nocase(pInputText, "time"))
		*pDisplayScore = EDisplayScore::TIME;
	else if(!str_comp_nocase(pInputText, "level"))
		*pDisplayScore = EDisplayScore::LEVEL;
	else if(!str_comp_nocase(pInputText, "block"))
		*pDisplayScore = EDisplayScore::BLOCK;
	else if(!str_comp_nocase(pInputText, "current_spree"))
		*pDisplayScore = EDisplayScore::CURRENT_SPREE;
	else if(!str_comp_nocase(pInputText, "king_of_the_hill"))
		*pDisplayScore = EDisplayScore::KING_OF_THE_HILL;
	else if(!str_comp_nocase(pInputText, "plugin_points"))
		*pDisplayScore = EDisplayScore::PLUGIN_POINTS;
	else if(!str_comp_nocase(pInputText, "plugin_time"))
		*pDisplayScore = EDisplayScore::PLUGIN_TIME;
	else
		return false;
	return true;
}

const char *display_score_to_str(EDisplayScore Score)
{
	switch(Score)
	{
	case EDisplayScore::TIME:
		return "time";
	case EDisplayScore::LEVEL:
		return "level";
	case EDisplayScore::BLOCK:
		return "block";
	case EDisplayScore::CURRENT_SPREE:
		return "current_spree";
	case EDisplayScore::KING_OF_THE_HILL:
		return "king_of_the_hill";
	case EDisplayScore::PLUGIN_POINTS:
		return "plugin_points"; // TODO: this userfacing string would be cleaner as just "points" but too lazy for now xd
	case EDisplayScore::PLUGIN_TIME:
		return "plugin_time";
	case EDisplayScore::NUM_SCORES:
		return "(invalid)";
	}

	return "(invalid)";
}
