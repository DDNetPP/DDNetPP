#include "debug.h"

#if defined(CONF_DEBUG)

std::vector<CStackEntry> gCallStack;
CCrashHandler gCrashHandler;

void Stacktrace(IOHANDLE ErrorFile)
{
	for (unsigned int i = 0; i < gCallStack.size(); i++)
	{
		char aLine[1024];
		str_format(aLine, sizeof(aLine), "#%i - %s:%i:%s - %s", i, gCallStack[i].m_pFile, gCallStack[i].m_Line, gCallStack[i].m_pFunction, gCallStack[i].m_aBuffer);
		dbg_msg("Stack", aLine);

		//add a line end for the file
		str_append(aLine, "\n", sizeof(aLine));
		if (ErrorFile)
			io_write(ErrorFile, aLine, str_length(aLine));
	}
}


void CCrashHandler::OnCrash(int Parameter)
{
	/* TODO add prev handler*/
	/*if (gpPrevHandler)
	gpPrevHandler(Parameter);*/
	char aTime[128];
	char aFilename[1024];
	str_timestamp(aTime, sizeof(aTime));
	str_format(aFilename, sizeof(aFilename), "crash_%s.log", aTime);
	IOHANDLE ErrorFile = io_open(aFilename, IOFLAG_WRITE);
	Stacktrace(ErrorFile);
	io_close(ErrorFile);

#ifndef CONF_DEBUG
	exit(0);
#else
	dbg_break();
#endif
}

#endif