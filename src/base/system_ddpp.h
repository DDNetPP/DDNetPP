/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

/*
	Title: OS Abstraction
*/

#ifndef BASE_SYSTEM_DDPP_H
#define BASE_SYSTEM_DDPP_H

#include "detect.h"
#include "stddef.h"
#include <stdio.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* * * * * * * * * * *
 *                   *
 *      DDNet++      *
 *                   *
 * * * * * * * * * * */

/*
	Function: regex_compile
		checks if a regex pattern matches

	Retrurns:
	   -1 - on error
		0 - if pattern matches
		1 - if pattern doesn't match
*/
int regex_compile(const char *pPattern, const char *pStr);

/*
	Function: fpost_get_pos
		ensures windows and unix support for fpos_t

	Parameters:
		pos - of type fpos_t

	Returns:
		a file position as long long
*/
long long fpost_get_pos(fpos_t pos);

#ifdef __cplusplus
}
#endif

#endif
