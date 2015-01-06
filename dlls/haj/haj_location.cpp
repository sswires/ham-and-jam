//========= Copyright © 2009, Ham and Jam. ==============================//
// Purpose:  Point entity that contains the name of the current location
// Notes:
// $NoKeywords: $
//=======================================================================//
#include "cbase.h"
#include "haj_player.h"
#include "haj_location.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( haj_location, CHajLocation );

BEGIN_DATADESC( CHajLocation )
	DEFINE_KEYFIELD( m_iszLocationName, FIELD_STRING, "LocationName" ),
END_DATADESC()

CHajLocation::CHajLocation()
{

}

void CHajLocation::Spawn()
{
	// setup as a volume trigger
	SetSolid(SOLID_VPHYSICS);
	SetSolidFlags(FSOLID_NOT_SOLID|FSOLID_TRIGGER);

	const char* szModelName = STRING(GetModelName());
	SetModel(szModelName);

	SetRenderMode(kRenderNone);

	BaseClass::Spawn();
}

const char* CHajLocation::GetLocationName()
{
	return STRING( m_iszLocationName );
}

void CHajLocation::StartTouch( CBaseEntity *pOther )
{
	CHajPlayer *pPlayer = ToHajPlayer( pOther );

	if( pPlayer )
	{
		DevMsg( "Player %s entered zone %s\n", pPlayer->GetPlayerName(), GetLocationName() );
		pPlayer->UpdateLocation( STRING( m_iszLocationName ) );
	}

	BaseClass::StartTouch( pOther );
}

void CHajLocation::EndTouch( CBaseEntity *pOther )
{
	BaseClass::EndTouch( pOther );
}