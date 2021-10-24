#include <new>

#include <base/color.h>
#include <base/ddpp_logs.h>
#include <base/math.h>
#include <base/system.h>
#include <base/vmath.h>

#include <engine/shared/protocol.h>
#include <engine/storage.h>

#include "config.h"
#include "console.h"
#include "linereader.h"

void CConsole::PrintDDPPLogs(int type)
{
	/*
        start from the oldest log
        so the last line printed in the console is the latest log
        scroll up to go in the past
    */
	for(int i = DDPP_LOG_SIZE - 1; i >= 0; i--)
	{
		if(!aDDPPLogs[type][i][0])
			continue;
		Print(OUTPUT_LEVEL_STANDARD, "ddpp_logs", aDDPPLogs[type][i]);
	}
}
