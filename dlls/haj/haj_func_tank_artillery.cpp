//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "func_tank.h"
#include "haj_shell_artillery.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Func tank that fires an artillery shell
//-----------------------------------------------------------------------------
class CFuncTankArtillery : public CFuncTank
{
public:
	DECLARE_CLASS( CFuncTankArtillery, CFuncTank );

	CFuncTankArtillery();

	void Precache( void );
	void Fire( int bulletCount, const Vector &barrelEnd, const Vector &forward, CBaseEntity *pAttacker, bool bIgnoreSpread );
	virtual float GetShotSpeed() { return m_flRocketSpeed; }

protected:
	float	m_flRocketSpeed;

	DECLARE_DATADESC();
};

BEGIN_DATADESC( CFuncTankArtillery )

	DEFINE_KEYFIELD( m_flRocketSpeed, FIELD_FLOAT, "rocketspeed" ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( func_tankartillery, CFuncTankArtillery );

void CFuncTankArtillery::Precache( void )
{
	UTIL_PrecacheOther( "shell_artillery" );
	PrecacheScriptSound( "Weapon_Mortar.Single" );
	CFuncTank::Precache();
}

void CFuncTankArtillery::Fire( int bulletCount, const Vector &barrelEnd, const Vector &forward, CBaseEntity *pAttacker, bool bIgnoreSpread )
{
	IPredictionSystem::SuppressHostEvents( NULL );

	//Create the shell
	CShellArtillery *pShell = (CShellArtillery*)Create( "shell_artillery", barrelEnd, GetAbsAngles(), this );
	
	EmitSound( "Weapon_Mortar.Single" );

	pShell->SetAbsVelocity( forward * m_flRocketSpeed );	

	// Tumble vector
	QAngle vecTumbleVelocity( 0, 0, 0 );
	pShell->SetLocalAngularVelocity ( vecTumbleVelocity );
	
	pShell->SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE ); 
	pShell->SetThrower( GetController() );
	pShell->SetDamage( 100.0f );
	pShell->SetDamageRadius( 500.0f );

	CFuncTank::Fire( bulletCount, barrelEnd, forward, this, true );
}

CFuncTankArtillery::CFuncTankArtillery()
{
	m_flRocketSpeed = 500.0f;
}