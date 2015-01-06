//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: NPC/World Stielhandgranate class
//
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "basegrenade_shared.h"
#include "haj_grenade_stiel.h"
#include "soundent.h"
#include "smoke_trail.h"
#include "haj_grenade_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CGrenadeStiel : public CHAJGrenade
{
	DECLARE_CLASS( CGrenadeStiel, CHAJGrenade );

#if !defined( CLIENT_DLL )
	DECLARE_DATADESC();
#endif
					
public:
	const char* GetGrenadeModel( void )
	{
		return "models/weapons/w_models/w_stiel_item.mdl";
	}

};

LINK_ENTITY_TO_CLASS( npc_grenade_stiel, CGrenadeStiel );

BEGIN_DATADESC( CGrenadeStiel )
END_DATADESC()

CBaseGrenade *StielGrenade_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer )
{
	// Don't set the owner here, or the player can't interact with grenades he's thrown
	CGrenadeStiel *pGrenade = (CGrenadeStiel *)CBaseEntity::Create( "npc_grenade_stiel", position, angles, pOwner );
	
	pGrenade->SetTimer( timer, timer - 1.5f );
	pGrenade->SetVelocity( velocity, angVelocity );
	pGrenade->SetThrower( ToBaseCombatCharacter( pOwner ) );
	pGrenade->m_takedamage = DAMAGE_YES;

	return pGrenade;
}