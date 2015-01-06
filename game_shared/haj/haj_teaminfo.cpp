/*
	© 2010 Ham and Jam Team
	============================================================
	Author: Stephen Swires
	Purpose: Contains information specific to each team
*/

#include "cbase.h"
#include "haj_teaminfo.h"
#include "KeyValues.h"
#include "haj_gamerules.h"

CTeamInfo::CTeamInfo( int iTeam )
{
	m_iTeam = iTeam;
	m_iUnit = 0;
	m_iNation = 0;
}

void CTeamInfo::ParseKeyValues( KeyValues *pValues )
{
	CHajGameRules *pRules = HajGameRules();
	KeyValues *pChain = NULL;
	char numBuffer[4];

	// chain the keys together
	if( pRules )
	{
		m_iNation = pRules->GetTeamNation( m_iTeam );
		m_iUnit = pRules->GetTeamUnit( m_iTeam );

		Q_snprintf( numBuffer, sizeof( numBuffer ), "%d", m_iTeam );
		KeyValues *pTeamKeys = pValues->FindKey( numBuffer );

		if( pTeamKeys )
		{
			KeyValues *pNationKeys = pTeamKeys->FindKey( "Nations" ); // always gonna assume this is not null
			pChain = pTeamKeys;

			if( pNationKeys )
			{
				Q_snprintf( numBuffer, sizeof( numBuffer ), "%d", m_iNation );
				KeyValues *pNationSubKey = pNationKeys->FindKey( numBuffer );

				if( pNationSubKey )
				{
					pNationSubKey->ChainKeyValue( pTeamKeys );

					Q_snprintf( numBuffer, sizeof( numBuffer ), "%d", m_iUnit );
					KeyValues *pUnitSubKey = pNationSubKey->FindKey( numBuffer );

					if( pUnitSubKey )
					{
						pChain = pUnitSubKey;
						pChain->ChainKeyValue( pNationSubKey );
					}
					else
					{
						pChain = pNationSubKey;
					}
				}

			}
		}
		else
		{
			Warning( "No team info found for team %d\n", m_iTeam );
		}
	}

	// search for information we would like
	if( pChain )
	{
		Q_strcpy( m_szPropaganda, pChain->GetString( "propaganda", "" ) );
		Q_strcpy( m_szUnitName, pChain->GetString( "unit", "Unknown Unit" ) );
	}
	else
	{
		Warning( "No team info to parse! pChain is null\n");
	}
}

bool CTeamInfo::CheckConsistency( void )
{
	CHajGameRules *pRules = HajGameRules();

	if( !pRules )
		return true;

	return (bool)( m_iNation == pRules->GetTeamNation(m_iTeam) && m_iUnit == pRules->GetTeamUnit(m_iTeam) );
}