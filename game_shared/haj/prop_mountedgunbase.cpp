/*
		© 2010 Ham and Jam Team
		============================================================
		Author: Stephen Swires
		Purpose: Base for mounted MGs (that are also predicted)
*/

#include "cbase.h"
#include "in_buttons.h"
#include "prop_mountedgunbase.h"

#ifdef CLIENT_DLL
#include "haj_player_c.h"
#else	
#include "haj_player.h"
#endif

LINK_ENTITY_TO_CLASS( prop_mountedgun, CMountedGunBase );
IMPLEMENT_NETWORKCLASS_ALIASED( MountedGunBase, DT_MountedGunBase );

// Network table
BEGIN_NETWORK_TABLE( CMountedGunBase, DT_MountedGunBase )
#ifdef CLIENT_DLL
	RecvPropFloat( RECVINFO( m_flNextShoot ) ),
	RecvPropBool( RECVINFO( m_bEnabled ) ),
	RecvPropEHandle( RECVINFO( m_hController ) ),

	RecvPropFloat( RECVINFO( m_flDelay ) ),
#else
	SendPropFloat( SENDINFO( m_flNextShoot ) ),
	SendPropBool( SENDINFO( m_bEnabled ) ),
	SendPropEHandle( SENDINFO( m_hController ) ),

	SendPropFloat( SENDINFO( m_flDelay ) ),
#endif
END_NETWORK_TABLE()

// predicted values
#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CMountedGunBase )
	DEFINE_PRED_FIELD( m_flNextShoot, FIELD_TIME, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif

// Datadesc
BEGIN_DATADESC( CMountedGunBase )
	DEFINE_FIELD( m_flNextShoot, FIELD_FLOAT ),

#ifndef CLIENT_DLL
	DEFINE_KEYFIELD( m_flDelay, FIELD_FLOAT, "ShootDelay" ),
	
	DEFINE_OUTPUT( m_OnFired, "OnFired" ),
#endif
END_DATADESC()

// Constructor, set everything to default
CMountedGunBase::CMountedGunBase()
{
	m_flNextShoot = 0.0f;
	m_bEnabled = true;
	m_hController.Set( NULL );

	m_flDelay = 0.1f;

	SetThink( &CMountedGunBase::DoThink );
}

void CMountedGunBase::Precache()
{
	PrecacheModel( STRING( GetModelName() ) );
}

void CMountedGunBase::Activate()
{
	SetModel( STRING( GetModelName() ) );
	SetSolid( SOLID_VPHYSICS );

	BaseClass::Activate();
}

void CMountedGunBase::DoMGThink()
{
	CHajPlayer *pController = m_hController.Get();

	if( pController )
	{
		if( ( pController->m_nButtons & IN_ATTACK ) && gpGlobals->curtime >= m_flNextShoot )
		{
			FireWeapon( pController );

#ifndef CLIENT_DLL
			m_OnFired.FireOutput( pController, this );
#endif
		}
	}
}

void CMountedGunBase::FireWeapon( CHajPlayer* pController )
{
	m_flNextShoot = gpGlobals->curtime + m_flDelay;
}