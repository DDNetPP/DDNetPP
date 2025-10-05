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
	case EDisplayScore::NUM_SCORES:
		return "(invalid)";
	}

	return "(invalid)";
}
