
// haj_team.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "haj_team.h"
#include "haj_spawnpoint.h"

#include "haj_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/////////////////////////////////////////////////////////////////////////////
LINK_ENTITY_TO_CLASS(team_manager, CHajTeam);

/////////////////////////////////////////////////////////////////////////////
CHajTeam::CHajTeam()
: m_pLastSpawnPoint(NULL)
{

}

/////////////////////////////////////////////////////////////////////////////
CHajTeam::~CHajTeam()
{

}

/////////////////////////////////////////////////////////////////////////////
int CHajTeam::GetFirstCombatTeamIndex()
{
	return TEAM_CWEALTH;
}

/////////////////////////////////////////////////////////////////////////////
CBaseEntity* CHajTeam::GetNextSpawnPoint(CBasePlayer* pPlayer)
{
	const char* szSpawnPointName = GetSpawnPointClassName();

	// check for invalid spawn point name
	if(!szSpawnPointName)
		return NULL;

	CBaseEntity* pSpot = m_pLastSpawnPoint;

	for(int i = random->RandomInt(1,2); i > 0; i--)
		pSpot = gEntList.FindEntityByClassname(pSpot, szSpawnPointName);

	CBaseEntity* pFirstSpot = pSpot;

	do
	{
		if(pSpot)
		{
			// check if pSpot is valid
			if(g_pGameRules->IsSpawnPointValid(pSpot, pPlayer))
			{
				// if it is a Haj spawn point, then check if it is enabled
				// for now we can let other types of spawn points pass
				CHajSpawnPoint* pSpawnPoint = dynamic_cast<CHajSpawnPoint*>(pSpot);

				if(pSpot->GetLocalOrigin() == vec3_origin || ( pSpawnPoint && !pSpawnPoint->IsEnabled() ) )
				{
					pSpot = gEntList.FindEntityByClassname(pSpot, szSpawnPointName);
					continue;
				}

				m_pLastSpawnPoint = pSpot;
				return pSpot;
			}
		}

		// increment pSpot
		pSpot = gEntList.FindEntityByClassname(pSpot, szSpawnPointName);
	} while(pSpot != pFirstSpot); // loop if we're not back to the start

	// fallback to default spawn point
	if(!pSpot)
	{
		pSpot = gEntList.FindEntityByClassname(pSpot, "info_player_start");

		if(pSpot)
		{
			m_pLastSpawnPoint = pSpot;
			return pSpot;
		}
	}

	m_pLastSpawnPoint = pSpot;

	return pSpot;
}

/////////////////////////////////////////////////////////////////////////////
const char* CHajTeam::GetSpawnPointClassName() const
{
	switch(m_iTeamNum)
	{
	case TEAM_AXIS:
		return ENTITY_SPAWNPOINT_AXIS;

	case TEAM_CWEALTH:
		return ENTITY_SPAWNPOINT_CWEALTH;

	default:
		return "info_player_start";
	}
	
	assert(false);
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
bool CHajTeam::IsCombatTeam(int teamId)
{
	return ((teamId == TEAM_CWEALTH) || (teamId == TEAM_AXIS));
}

/////////////////////////////////////////////////////////////////////////////
void CHajTeam::RegisterCapturePoint(CHajObjective* pCapturePoint)
{
	if( !pCapturePoint )
		return;

	// add the capture point to the list if it is not already there
	int idx = m_capturePoints.AddToTail();
	m_capturePoints[ idx ] = pCapturePoint;

	//Msg( "Registered %s with team %d\n", pCapturePoint->GetNameOfZone(), m_iTeamNum );
}

/////////////////////////////////////////////////////////////////////////////
void CHajTeam::ResetRound()
{
	m_pLastSpawnPoint = NULL;
	m_capturePoints.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
void CHajTeam::UnregisterCapturePoint(CHajObjective* pCapturePoint)
{
	if( !pCapturePoint )
		return;

	// remove the spawn point from the list
	for( int i = 0; i < m_capturePoints.Count(); i++ )
	{
		if( m_capturePoints[ i ] )
		{
			if( pCapturePoint->GetZoneId() == m_capturePoints[ i ]->GetZoneId() )
				m_capturePoints.Remove( i );
		}
	}

	//Msg( "Unregistered %s with team %d\n", pCapturePoint->GetNameOfZone(), m_iTeamNum );
}

/////////////////////////////////////////////////////////////////////////////
