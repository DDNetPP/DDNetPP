#ifndef ENGINE_SHARED_DDNETPP_LUA_CONTROLLER_H
#define ENGINE_SHARED_DDNETPP_LUA_CONTROLLER_H

#include <engine/kernel.h>

class ILuaController : public IInterface
{
	MACRO_INTERFACE("luacontroller")
public:
	virtual void OnInit() = 0;
	virtual bool OnClientMessage(int ClientId, const void *pData, int Size, int Flags) = 0;
	virtual bool OnServerMessage(int ClientId, const void *pData, int Size, int Flags) = 0;
};

extern ILuaController *CreateLuaController();

#endif
