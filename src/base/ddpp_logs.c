#include "stdio.h"

#include "system.h"

#include "ddpp_logs.h"

char aDDPPLogs[DDPP_NUM_LOGS][DDPP_LOG_SIZE][128] = {{{0}}};

void ddpp_log_print(int type)
{
    for (int i = 0; i < DDPP_LOG_SIZE; i++)
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
    for (int i = 0; i < DDPP_LOG_SIZE; i++)
    {
        if (aDDPPLogs[type][i][0])
            continue;
        str_copy(aDDPPLogs[type][i], pMsg, sizeof(aDDPPLogs[type][i]));
        return;
    }
}