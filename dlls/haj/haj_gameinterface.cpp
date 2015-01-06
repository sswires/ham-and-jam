
// haj_gameinterface.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "haj_gameinterface.h"
#include "haj_gamerules.h"

#include "mapentities.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/////////////////////////////////////////////////////////////////////////////
// globals
CHajServerGameDLL _serverGameDLL;

/////////////////////////////////////////////////////////////////////////////
// expose singleton interface
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CHajServerGameDLL,
								  IServerGameDLL,
								  INTERFACEVERSION_SERVERGAMEDLL,
								  _serverGameDLL);

/////////////////////////////////////////////////////////////////////////////
// CHajServerGameDLL
/////////////////////////////////////////////////////////////////////////////
CHajServerGameDLL::CHajServerGameDLL()
: m_szMapEntities(NULL)
{

}

/////////////////////////////////////////////////////////////////////////////
CHajServerGameDLL::~CHajServerGameDLL()
{
	if(m_szMapEntities)
	{
		delete [] m_szMapEntities;
		m_szMapEntities = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
bool CHajServerGameDLL::DLLInit(CreateInterfaceFn engineFactory,
				 CreateInterfaceFn physicsFactory, 
				 CreateInterfaceFn fileSystemFactory,
				 CGlobalVars *pGlobals)
{
	bool bRet = CServerGameDLL::DLLInit(engineFactory,
				 physicsFactory, 
				 fileSystemFactory,
				 pGlobals);

	return bRet;
}

/////////////////////////////////////////////////////////////////////////////
void CHajServerGameDLL::DLLShutdown()
{
	
}

/////////////////////////////////////////////////////////////////////////////
bool CHajServerGameDLL::GameInit()
{
	bool bRet = CServerGameDLL::GameInit();
	return bRet;
}

/////////////////////////////////////////////////////////////////////////////
void CHajServerGameDLL::GameShutdown()
{
	CServerGameDLL::GameShutdown();
}

/////////////////////////////////////////////////////////////////////////////
const char* CHajServerGameDLL::GetGameDescription()
{
	if( HajGameRules() )
	{
		return HajGameRules()->GetGameDescription();
	}

	return "Ham and Jam";
}

/////////////////////////////////////////////////////////////////////////////
bool CHajServerGameDLL::LevelInit( const char *pMapName, char const *pMapEntities, char const *pOldLevel, char const *pLandmarkName, bool loadGame, bool background )
{
	bool bRet = CServerGameDLL::LevelInit(pMapName, pMapEntities, pOldLevel, pLandmarkName, loadGame, background);
	return bRet;
}

/////////////////////////////////////////////////////////////////////////////
void CHajServerGameDLL::LevelInit_ParseAllEntities(const char *pMapEntities)
{
	// store the entities list so we can
	// essentially reset the level without loading the map again
	if(m_szMapEntities)
	{
		delete [] m_szMapEntities;
		m_szMapEntities = NULL;
	}

	unsigned int bufferSize = strlen(pMapEntities);
	m_szMapEntities = new char[bufferSize + 1];
	Q_strcpy(m_szMapEntities, pMapEntities);

	// load the entities
	MapEntity_ParseAllEntities(pMapEntities, NULL);
}

/////////////////////////////////////////////////////////////////////////////
void CHajServerGameDLL::LevelShutdown()
{
	CServerGameDLL::LevelShutdown();
}

/////////////////////////////////////////////////////////////////////////////
// CServerGameClients
/////////////////////////////////////////////////////////////////////////////
void CServerGameClients::GetPlayerLimits(int& minplayers, int& maxplayers, int &defaultMaxPlayers) const
{
	defaultMaxPlayers = 8;
	minplayers = 2; 
	maxplayers = 32;
}

/////////////////////////////////////////////////////////////////////////////
