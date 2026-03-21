#include <base/str.h>
#include <game/server/ddnetpp/lua/lua_rcon_command.h>

#include <vector>

// https://github.com/DDNetPP/DDNetPP/issues/512
//
// this is basically copy pasted from here
// https://github.com/ChillerDragon/antibob/blob/b8d4c5e37ef5c0d72ab1fd0d6e5d0b805f49e744/src/antibob/bob/console.cpp#L18
bool CLuaRconCommand::ParseParameters(std::vector<CParam> &vResult, const char *pParameters, char *pError, int ErrorLen)
{
	if(pError && ErrorLen)
		pError[0] = '\0';

	const int ParamsLen = str_length(pParameters);
	bool InDesc = false;
	CParam Param;
	Param.Reset();
	bool ExpectOnlyOptional = false;

	for(int i = 0; i < ParamsLen; i++)
	{
		if(pParameters[i] == '[')
		{
			if(InDesc)
			{
				if(pError)
					str_format(pError, ErrorLen, "nested braces in params '%s'", pParameters);
				return false;
			}
			InDesc = true;
			continue;
		}
		if(pParameters[i] == ']')
		{
			if(!InDesc)
			{
				if(pError)
					str_format(pError, ErrorLen, "unexpected closing brace '%s'", pParameters);
				return false;
			}

			InDesc = false;
			if(ExpectOnlyOptional && !Param.m_Optional)
			{
				if(pError)
					str_format(pError, ErrorLen, "got non optional param after optional one '%s'", pParameters);
				return false;
			}
			vResult.emplace_back(Param);
			Param.Reset();
			continue;
		}

		if(pParameters[i] == '?')
		{
			if(Param.m_Optional)
			{
				if(pError)
					str_format(pError, ErrorLen, "nameless optional parameter in %s", pParameters);
				return false;
			}

			Param.m_Optional = true;
			ExpectOnlyOptional = true;
		}
		else if(InDesc)
		{
			char aDescChar[4];
			str_format(aDescChar, sizeof(aDescChar), "%c", pParameters[i]);
			str_append(Param.m_aName, aDescChar);
		}
		else if(!InDesc)
		{
			switch(pParameters[i])
			{
			case 'i':
				Param.m_Type = CParam::EType::INT;
				break;
			case 's':
				Param.m_Type = CParam::EType::STRING;
				break;
			case 'r':
				Param.m_Type = CParam::EType::REST;
				break;
			case ' ':
				// skip spaces, but expect them to be followed
				// by a valid type
				Param.m_Type = CParam::EType::INVALID;
				break;
			default:
				if(pError)
					str_format(pError, ErrorLen, "invalid parameter type '%c' in params %s", pParameters[i], pParameters);
				return false;
			}
			if(pParameters[i + 1] != '[' && Param.m_Type != CParam::EType::INVALID)
			{
				if(ExpectOnlyOptional && !Param.m_Optional)
				{
					if(pError)
						str_format(pError, ErrorLen, "got non optional param after optional one '%s'", pParameters);
					return false;
				}
				vResult.emplace_back(Param);
				Param.Reset();
			}
		}
	}

	if(Param.m_Optional)
	{
		if(pError)
			str_format(pError, ErrorLen, "nameless optional parameter in %s", pParameters);
		return false;
	}

	if(InDesc)
	{
		if(pError)
			str_format(pError, ErrorLen, "missing ] in params '%s'", pParameters);
		return false;
	}

	return true;
}
