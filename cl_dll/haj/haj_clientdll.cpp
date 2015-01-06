
// haj_clientdll.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "haj_clientdll.h"
#include "haj_gameevents.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/////////////////////////////////////////////////////////////////////////////
CON_COMMAND(cl_version, "Prints out the version info for the client.")
{
	Msg("Ham and Jam\nCompiled on %s at %s\n", __DATE__, __TIME__);
}

/////////////////////////////////////////////////////////////////////////////
// globals
CHajClientDLL _hajClientDLL;
IBaseClientDLL *clientdll = &_hajClientDLL;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CHajClientDLL,
								  IBaseClientDLL,
								  CLIENT_DLL_INTERFACE_VERSION,
								  _hajClientDLL);

/////////////////////////////////////////////////////////////////////////////
int CHajClientDLL::Init(CreateInterfaceFn appSystemFactory, 
						CreateInterfaceFn physicsFactory, 
						CGlobalVarsBase *pGlobals)
{
	Msg("Compiled on %s at %s\n", __DATE__, __TIME__);

	int ret = BaseClass::Init(appSystemFactory, physicsFactory, pGlobals);

	_gameEvents.SetupEventListeners();

	return ret;
}

/////////////////////////////////////////////////////////////////////////////
void CHajClientDLL::LevelInitPreEntity(const char *pMapName)
{
	BaseClass::LevelInitPreEntity(pMapName);
}

/////////////////////////////////////////////////////////////////////////////
void CHajClientDLL::LevelShutdown()
{
	BaseClass::LevelShutdown();
}

/////////////////////////////////////////////////////////////////////////////
void CHajClientDLL::Shutdown()
{
	BaseClass::Shutdown();
}
