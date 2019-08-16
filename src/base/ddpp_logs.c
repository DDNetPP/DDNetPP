#include "stdio.h"

#include "system.h"

#include "ddpp_logs.h"

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
    for (int i = DDPP_LOG_SIZE; i > 0; i--)
    {
        if (!aDDPPLogs[type][i][0])
            continue;
        printf("[ddpp_logs] %s\n", aDDPPLogs[type][i]);
    }
}

void ddpp_log(int type, const char *pMsg)
{
    if (type == DDPP_LOG_MASTER)
    {
        ddpp_log_append(type, pMsg);
    }
    else
    {
        printf("[ddpp_logs] Error: invalid log type=%d\n", type);
        *((volatile unsigned*)0) = 0x0;
    }
}

void ddpp_log_append(int type, const char *pMsg)
{
    for (int i = DDPP_LOG_SIZE - 1; i >= 0; i--)
    {
        str_copy(aDDPPLogs[type][i], aDDPPLogs[type][i-1], sizeof(aDDPPLogs[type][i]));
    }
    str_copy(aDDPPLogs[type][0], pMsg, sizeof(aDDPPLogs[type][0]));
}
