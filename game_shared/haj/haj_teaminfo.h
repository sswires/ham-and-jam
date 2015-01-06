/*
	© 2010 Ham and Jam Team
	============================================================
	Author: Stephen Swires
	Purpose: Contains information specific to each team
*/

#ifndef HAJ_TEAMINFO
#define HAJ_TEAMINFO

#include "cbase.h"

class KeyValues;

class CTeamInfo
{
public:
	CTeamInfo( int iTeam );

	const char* GetUnitName() { return m_szUnitName; }
	const char* GetPropagandaPoster() { return m_szPropaganda; }

	void ParseKeyValues( KeyValues *pValues );
	bool CheckConsistency( void );

private:
	char m_szUnitName[150];
	char m_szPropaganda[200];

	int m_iTeam;
	int m_iNation;
	int m_iUnit;
};

#endif