#include "ddpp_logs.h"

#include "system.h"

#include <cstdio>

/*
    aDDPPLogs

    array of log arrays (all types)
    index 0 is the newest log line
*/
char aDDPPLogs[DDPP_NUM_LOGS][DDPP_LOG_SIZE][128] = {{{0}}};

void ddpp_log_print(int type)
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
		printf("[ddpp_logs] %s\n", aDDPPLogs[type][i]);
	}
}

void ddpp_log_print_all()
{
	for(int i = 0; i < DDPP_NUM_LOGS; i++)
	{
		// printf("[ddpp_logs] printing type=%d\n", i);
		ddpp_log_print(i);
	}
}

void ddpp_log(int type, const char *pMsg)
{
	if(type < 0 || type > DDPP_NUM_LOGS)
	{
		printf("[ddpp_logs] Error: invalid log type=%d (max %d)\n", type, DDPP_NUM_LOGS);
		dbg_break();
	}

	char aMsg[512];
	char aTimestamp[64];
	str_timestamp_format(aTimestamp, sizeof(aTimestamp), FORMAT_SPACE);
	str_format(aMsg, sizeof(aMsg), "[%s] %s", aTimestamp, pMsg);

	ddpp_log_append(type, aMsg);
}

void ddpp_log_append(int type, const char *pMsg)
{
	for(int i = DDPP_LOG_SIZE - 1; i >= 1; i--)
	{
		str_copy(aDDPPLogs[type][i], aDDPPLogs[type][i - 1], sizeof(aDDPPLogs[type][i]));
	}
	str_copy(aDDPPLogs[type][0], pMsg, sizeof(aDDPPLogs[type][0]));
}
