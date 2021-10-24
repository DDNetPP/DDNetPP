/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ddpp_logs.h"
#include "system.h"

#include <sys/stat.h>
#include <sys/types.h>

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(CONF_FAMILY_UNIX)
#include <regex.h>
#endif

int regex_compile(const char *pPattern, const char *pStr)
{
#if defined(CONF_FAMILY_UNIX)
	/*
		credits go to Per-Olof Pettersson
		found through: https://stackoverflow.com/a/1085120

		modified by ChillerDragon
	*/
	regex_t regex;
	int reti;
	char aBuf[128];
	int ret = -1;

	/* Compile regular expression */
	reti = regcomp(&regex, pPattern, 0);
	if(reti)
	{
		dbg_msg("regex", "Could not compile regex");
		ret = -1;
		goto end;
	}

	/* Execute regular expression */
	reti = regexec(&regex, pStr, 0, NULL, 0);
	if(!reti)
	{
		dbg_msg("regex", "pattern matches");
		ret = 0;
		goto end;
	}
	else if(reti == REG_NOMATCH)
	{
		dbg_msg("regex", "pattern does not match");
		ret = 1;
		goto end;
	}
	else
	{
		regerror(reti, &regex, aBuf, sizeof(aBuf));
		dbg_msg("regex", "Regex match failed: %s\n", aBuf);
		ret = -1;
		goto end;
	}

end:
	/* Free memory allocated to the pattern buffer by regcomp() */
	regfree(&regex);
	return ret;
	;
#else
	return 0;
#endif
}

long long fpost_get_pos(fpos_t pos)
{
#if defined(CONF_FAMILY_WINDOWS)
	return pos;
#elif defined(CONF_PLATFORM_MACOSX)
	return pos;
#else
	return pos.__pos;
#endif
}

#if defined(__cplusplus)
}
#endif
