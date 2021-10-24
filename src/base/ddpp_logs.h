
#ifndef BASE_DDPP_LOGS_H
#define BASE_DDPP_LOGS_H

#ifdef __cplusplus
extern "C" {
#endif

enum
{
	DDPP_LOG_MASTER,
	DDPP_LOG_AUTH_RCON,
	DDPP_LOG_WRONG_RCON,
	DDPP_NUM_LOGS,

	DDPP_LOG_SIZE = 8
};

extern char aDDPPLogs[DDPP_NUM_LOGS][DDPP_LOG_SIZE][128];

/*
    ddpp_log_print(type)
    print all log lines of a given type to stdout
*/
void ddpp_log_print(int type);

/*
    ddpp_log_print_all()
    print all log lines of all types
*/
void ddpp_log_print_all();

/*
    ddpp_log(type, message)
    use this method to store ddnet++ logs
    those can be viewed with the chat command '/logs'
*/
void ddpp_log(int type, const char *pMsg);

/*
    ddpp_log_append(type, message)
    do not use this function directly
    only a helper for ddpp_log()
*/
void ddpp_log_append(int type, const char *pMsg);

#ifdef __cplusplus
}
#endif

#endif