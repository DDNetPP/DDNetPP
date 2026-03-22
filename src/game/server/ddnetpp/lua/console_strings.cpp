#include "console_strings.h"

#include <base/str.h>

// TODO: move this to own library
bool SplitConsoleStatements(const char *apStmts[], size_t MaxStmts, size_t *pNumStmts, char *pLine, char *pError, size_t ErrorLen)
{
	if(pError && ErrorLen)
		pError[0] = '\0';
	*pNumStmts = 0;
	if(pLine[0] == '\0')
		return true;

	char *pStr = pLine;
	apStmts[(*pNumStmts)++] = pStr;
	bool InString = false;

	do
	{
		if(*pNumStmts >= MaxStmts)
		{
			if(pError)
			{
				str_copy(pError, "too many statements", ErrorLen);
			}
			return false;
		}

		if(*pStr == '"')
		{
			InString ^= true;
		}
		else if(*pStr == '\\') // escape sequences
		{
			if(pStr[1] == '"')
				pStr++;
		}

		if(InString)
			continue;

		if(*pStr == ';')
		{
			pStr[0] = '\0';
			if(pStr[1] == '\0')
				break;
			if(pStr[1] == ';')
			{
				if(pError)
				{
					str_copy(pError, "syntax error near unexpected token `;;'", ErrorLen);
				}
				return false;
			}
			pStr++;
			apStmts[(*pNumStmts)++] = pStr;
		}
		else if(*pStr == '#')
		{
			pStr[0] = '\0';
			break;
		}
	} while(*pStr++);

	return true;
}

// TODO: move this to a own library
bool SplitConsoleArgs(const char *apArgs[], size_t MaxArgs, size_t *pNumArgs, char *pInput, char *pError, size_t ErrorLen)
{
	if(pError && ErrorLen)
		pError[0] = '\0';
	*pNumArgs = 0;

	char *pStr = pInput;

	while(*pStr)
	{
		pStr = str_skip_whitespaces(pStr);
		if(*pStr == '"')
		{
			pStr++;
			apArgs[(*pNumArgs)++] = pStr;

			char *pDst = pStr; // we might have to process escape data
			while(pStr[0] != '"')
			{
				if(pStr[0] == '\\')
				{
					if(pStr[1] == '\\')
						pStr++; // skip due to escape
					else if(pStr[1] == '"')
						pStr++; // skip due to escape
				}
				else if(pStr[0] == '\0')
				{
					if(pError)
						str_copy(pError, "missing closing quote", ErrorLen);
					return false;
				}

				*pDst = *pStr;
				pDst++;
				pStr++;
			}
			*pDst = '\0';

			pStr++;
		}
		else
		{
			// this piece of code was added by ChillerDragon
			// to fix the arg count with trailing spaces
			// the rest of the code is from teeworlds
			const char *pEnd = str_skip_whitespaces_const(pStr);
			if(pEnd[0] == '\0')
				return true;

			apArgs[(*pNumArgs)++] = pStr;
			pStr = str_skip_to_whitespace(pStr);
			if(pStr[0] != '\0') // check for end of string
			{
				pStr[0] = '\0';
				pStr++;
			}
		}

		if(pStr[0] != '\0')
		{
			if(*pNumArgs >= MaxArgs)
			{
				if(pError)
					str_copy(pError, "too many arguments", ErrorLen);
				return false;
			}
		}
	}

	return true;
}

bool SplitConsoleArgsWithParams(const char *apArgs[], size_t MaxArgs, size_t *pNumArgs, char *pInput, const std::vector<CLuaRconCommand::CParam> &vParams, char *pError, size_t ErrorLen)
{
	if(pError && ErrorLen)
		pError[0] = '\0';
	*pNumArgs = 0;

	char *pStr = pInput;

	while(*pStr)
	{
		pStr = str_skip_whitespaces(pStr);
		if(*pStr == '"')
		{
			pStr++;
			apArgs[(*pNumArgs)++] = pStr;

			char *pDst = pStr; // we might have to process escape data
			while(pStr[0] != '"')
			{
				if(pStr[0] == '\\')
				{
					if(pStr[1] == '\\')
						pStr++; // skip due to escape
					else if(pStr[1] == '"')
						pStr++; // skip due to escape
				}
				else if(pStr[0] == '\0')
				{
					if(pError)
						str_copy(pError, "missing closing quote", ErrorLen);
					return false;
				}

				*pDst = *pStr;
				pDst++;
				pStr++;
			}
			*pDst = '\0';

			pStr++;
		}
		else
		{
			// this piece of code was added by ChillerDragon
			// to fix the arg count with trailing spaces
			// the rest of the code is from teeworlds
			const char *pEnd = str_skip_whitespaces_const(pStr);
			if(pEnd[0] == '\0')
				return true;

			if(vParams.size() > *pNumArgs && vParams[*pNumArgs].m_Type == CLuaRconCommand::CParam::REST)
			{
				apArgs[(*pNumArgs)++] = pStr;
				return true;
			}

			apArgs[(*pNumArgs)++] = pStr;
			pStr = str_skip_to_whitespace(pStr);
			if(pStr[0] != '\0') // check for end of string
			{
				pStr[0] = '\0';
				pStr++;
			}
		}

		if(pStr[0] != '\0')
		{
			if(*pNumArgs > MaxArgs)
			{
				if(pError)
					str_copy(pError, "too many arguments", ErrorLen);
				return false;
			}
		}
	}

	return true;
}
