
// haj_gameinterface.h

/////////////////////////////////////////////////////////////////////////////
#ifndef __INC_GAMEINTERFACE
#define __INC_GAMEINTERFACE

/////////////////////////////////////////////////////////////////////////////
// include
#ifndef GAMEINTERFACE_H
	#include "gameinterface.h"
#endif

/////////////////////////////////////////////////////////////////////////////
class CHajServerGameDLL : public CServerGameDLL
{
public:
	// 'structors
	CHajServerGameDLL();
	virtual ~CHajServerGameDLL();

public:
	// CServerGameDLL overrides
	bool LevelInit( const char *pMapName, char const *pMapEntities, char const *pOldLevel, char const *pLandmarkName, bool loadGame, bool background );
	void LevelInit_ParseAllEntities(const char *pMapEntities);
	void LevelShutdown();
	const char* GetGameDescription();

	bool GameInit();
	void GameShutdown();

	bool DLLInit(CreateInterfaceFn engineFactory,
				 CreateInterfaceFn physicsFactory, 
				 CreateInterfaceFn fileSystemFactory,
				 CGlobalVars *pGlobals);
	void DLLShutdown();

public:
	// implementation
	const char* GetCurrentMapEntities() const { return m_szMapEntities; }

private:
	char* m_szMapEntities;
};

/////////////////////////////////////////////////////////////////////////////
// globals
extern CHajServerGameDLL _serverGameDLL;

/////////////////////////////////////////////////////////////////////////////
#endif
