
// haj_clientdll.h

/////////////////////////////////////////////////////////////////////////////
#ifndef __INC_CLIENTDLL
#define __INC_CLIENTDLL

/////////////////////////////////////////////////////////////////////////////
// includes
#ifndef CDLL_CLIENT_INT_H
	#include "cdll_client_int.h"
#endif

/////////////////////////////////////////////////////////////////////////////
class CHajClientDLL : public CHLClient
{
	typedef CHLClient BaseClass;

public:
	virtual int Init(CreateInterfaceFn appSystemFactory, CreateInterfaceFn physicsFactory, CGlobalVarsBase *pGlobals);
	virtual void Shutdown();

	virtual void LevelInitPreEntity(const char *pMapName);
	virtual void LevelShutdown();
};

/////////////////////////////////////////////////////////////////////////////
#endif
