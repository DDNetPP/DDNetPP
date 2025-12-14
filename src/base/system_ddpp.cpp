/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "system_ddpp.h"

#include "ddpp_logs.h"
#include "system.h"

#include <sys/stat.h>
#include <sys/types.h>

#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#if defined(CONF_FAMILY_UNIX)
#include <regex.h>
#endif

bool str_to_bool(const char *pStr)
{
	bool Output = false;
	str_to_bool_strict(pStr, &Output);
	return Output;
}

bool str_to_bool_strict(const char *pStr, bool *pOutput)
{
	if(!str_comp_nocase(pStr, "on") ||
		!str_comp_nocase(pStr, "enable") ||
		!str_comp_nocase(pStr, "activate") ||
		!str_comp_nocase(pStr, "yes") ||
		!str_comp_nocase(pStr, "true") ||
		!str_comp_nocase(pStr, "1"))
	{
		if(pOutput)
			*pOutput = true;
		return true;
	}
	if(!str_comp_nocase(pStr, "off") ||
		!str_comp_nocase(pStr, "disable") ||
		!str_comp_nocase(pStr, "deactivate") ||
		!str_comp_nocase(pStr, "no") ||
		!str_comp_nocase(pStr, "false") ||
		!str_comp_nocase(pStr, "0"))
	{
		if(pOutput)
			*pOutput = false;
		return true;
	}
	return false;
}

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

	/* Compile regular expression */
	reti = regcomp(&regex, pPattern, 0);
	if(reti)
	{
		dbg_msg("regex", "Could not compile regex");
		regfree(&regex);
		return -1;
	}

	/* Execute regular expression */
	reti = regexec(&regex, pStr, 0, NULL, 0);
	if(!reti)
	{
		dbg_msg("regex", "pattern matches");
		regfree(&regex);
		return 0;
	}
	else if(reti == REG_NOMATCH)
	{
		dbg_msg("regex", "pattern does not match");
		regfree(&regex);
		return 1;
	}
	else
	{
		regerror(reti, &regex, aBuf, sizeof(aBuf));
		dbg_msg("regex", "Regex match failed: %s\n", aBuf);
		regfree(&regex);
		return -1;
	}

	/* Free memory allocated to the pattern buffer by regcomp() */
	regfree(&regex);
	return -1;
#else
	return 0;
#endif
}

// TODO: remove get_file_offset and use teeworlds styled io_tell
//       IOHANDLE instead of FILE
long get_file_offset(FILE *file)
{
	return io_tell(file);
}

// // TODO: move to detect.h in upstream
// #include <features.h>
// #ifndef __GLIBC__
//     #define __MUSL__
//     #define CONF_LIBC_MUSL
// #endif

// long long fpost_get_pos(fpos_t pos)
// {
// #if defined(CONF_FAMILY_WINDOWS)
// 	return pos;
// #elif defined(CONF_PLATFORM_MACOS)
// 	return pos;
// #elif defined(CONF_LIBC_MUSL)
// 	return pos.__lldata;
// #else
// 	return pos.__pos;
// #endif
// }
