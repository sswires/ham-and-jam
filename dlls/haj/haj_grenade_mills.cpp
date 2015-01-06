//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: NPC/World Mills Grenade class
//
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "basegrenade_shared.h"
#include "haj_grenade_mills.h"
#include "soundent.h"
#include "smoke_trail.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


LINK_ENTITY_TO_CLASS( npc_grenade_mills, CGrenadeMills );

BEGIN_DATADESC( CGrenadeMills )
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CGrenadeMills::~CGrenadeMills( void )
{
}

const char* CGrenadeMills::GetGrenadeModel( void )
{
	return "models/weapons/w_models/w_mills_item.mdl";
}

CBaseGrenade *MillsGrenade_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer )
{
	// Don't set the owner here, or the player can't interact with grenades he's thrown
	CGrenadeMills *pGrenade = (CGrenadeMills *)CBaseEntity::Create( "npc_grenade_mills", position, angles, pOwner );
	
	pGrenade->SetTimer( timer, timer - 1.5f );
	pGrenade->SetVelocity( velocity, angVelocity );
	pGrenade->SetThrower( ToBaseCombatCharacter( pOwner ) );
	pGrenade->m_takedamage = DAMAGE_YES;

	return pGrenade;
}