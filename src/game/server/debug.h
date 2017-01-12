#ifndef ENGINE_DEBUG_H
#define ENGINE_DEBUG_H

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <base/system.h>
#include <vector>
#include <stdarg.h>

#if defined(CONF_DEBUG)

#define ON_CRASH_RECOVER false
#define USE_BUGLOGGER

struct CStackEntry
{
	const char *m_pFile;
	const char *m_pFunction;
	int m_Line;
	char m_aBuffer[256];
	int64 m_Time;
	CStackEntry(const char *pFile, const char *pFunction, int Line, const char *pBuffer)
	{
		m_pFile = pFile;
		m_pFunction = pFunction;
		m_Line = Line;
		m_Time = time_get();
		str_copy(m_aBuffer, pBuffer, sizeof(m_aBuffer));
	}
	CStackEntry()
	{
		m_pFile = 0;
		m_pFunction = 0;
		m_Line = -1;
	}
	~CStackEntry()
	{
#ifdef CONF_DEBUG
		//float TimeDiff = (time_get() - m_Time) / (float)time_freq();
		//dbg_msg("", "%s %f", m_pFunction, TimeDiff);
#endif
	}
};

extern std::vector<CStackEntry> gCallStack;

class CFunctionEntry
{
public:
	CFunctionEntry(const char *pFile, const char *pFunction, int Line, const char *fmt = 0, ...)
	{
		char aBuffer[256] = { 0 };
		if (fmt)
		{
			va_list args;

			va_start(args, fmt);
#if defined(CONF_FAMILY_WINDOWS)
			_vsnprintf(aBuffer, sizeof(aBuffer), fmt, args);
#else
			vsnprintf(aBuffer, sizeof(aBuffer), fmt, args);
#endif
			va_end(args);
		}


		CStackEntry StackEntry(pFile, pFunction, Line, aBuffer);
		gCallStack.push_back(StackEntry);
	}
	~CFunctionEntry()
	{
		gCallStack.pop_back();
	}
};

void Stacktrace(IOHANDLE ErrorFile = 0);

//static void (*gpPrevHandler)(int);

struct CCrashFunction
{
	void(*m_pFunction)(int Parameter, void *pUser, bool Recover);
	void *m_pUser;
};

class CCrashHandler
{
public:
	static void OnCrash(int Parameter);
	CCrashHandler()
	{
#ifdef USE_BUGLOGGER
		signal(SIGINT, OnCrash);
		signal(SIGILL, OnCrash);
		signal(SIGSEGV, OnCrash);
		signal(SIGFPE, OnCrash);
		signal(SIGABRT, OnCrash);
		//signal(SIGABRT_COMPAT, OnCrash);
		//gpPrevHandler = signal(SIGILL, OnCrash);
#endif
	}
};

extern CCrashHandler gCrashHandler;

#define CONEX(x, y) x ## y
#define CON(x, y) CONEX(x, y)

#if defined(CONF_FAMILY_UNIX)
#define CALL_STACK_ADD() CFunctionEntry CON(_CFunctionEntry_, __COUNTER__)(__FILE__, __func__, __LINE__)
#define CALL_STACK_ADD_INFO_FMT(fmt, ...) CFunctionEntry CON(_CFunctionEntry_, __COUNTER__)(__FILE__, __func__, __LINE__, fmt, __VA_ARGS__)
#elif defined(CONF_FAMILY_WINDOWS)
#define CALL_STACK_ADD() CFunctionEntry CON(_CFunctionEntry_, __COUNTER__)(__FILE__, __FUNCTION__, __LINE__)
#define CALL_STACK_ADD_INFO_FMT(fmt, ...) CFunctionEntry CON(_CFunctionEntry_, __COUNTER__)(__FILE__, __FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif
#define CALL_STACK_ADD_INFO(Name) CFunctionEntry CON(_CFunctionEntry_, __COUNTER__)(__FILE__, Name, __LINE__)

#endif
#endif