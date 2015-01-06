//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose:  HaJ base machine gun class
// Notes:    This replaces the default HL2MP machine gun class and adds
//           the feaure of magazine based ammo management.
//
//           The magazines/ammo is handled locally in the weapon rather
//			 than taking ammo from the player "buckets".
//			
//			 Most of these functions in the class simply override the
//			 defaults to get around the bucket code and use the magazine
//			 stuff instead.
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "datacache/imdlcache.h"	

#if defined( CLIENT_DLL )
	#include "c_hl2mp_player.h"
	#include "haj_player_c.h"
	#include "prediction.h"

	extern ConVar cl_returntoironsights( "cl_returntoironsights", "1", FCVAR_USERINFO + FCVAR_ARCHIVE, "Return to ironsights after shooting?"); 
#else
	#include "hl2mp_player.h"
	#include "haj_player.h"
#endif

#include "haj_weapon_boltrifle.h"
#include "in_buttons.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( HAJBoltRifle, DT_HAJBoltRifle )

BEGIN_NETWORK_TABLE( CHAJBoltRifle, DT_HAJBoltRifle )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_bSighted ) ),
	RecvPropBool( RECVINFO( m_bWasSighted )),
	RecvPropBool( RECVINFO( m_bNeedsCocking )),
	RecvPropBool( RECVINFO( m_bIsCocking )),
#else
	SendPropBool( SENDINFO( m_bSighted )),
	SendPropBool( SENDINFO( m_bWasSighted)),
	SendPropBool( SENDINFO( m_bNeedsCocking ) ),
	SendPropBool( SENDINFO( m_bIsCocking ) ),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CHAJBoltRifle )
END_PREDICTION_DATA()

//=========================================================
//	>> CHLSelectFireMachineGun
//=========================================================
BEGIN_DATADESC( CHAJBoltRifle )
END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: Construtor
//-----------------------------------------------------------------------------
CHAJBoltRifle::CHAJBoltRifle( void )
{
	SetPredictionEligible( true );
	AddSolidFlags( FSOLID_TRIGGER ); // Nothing collides with these but it gets touches.

	m_bFiresUnderwater = false;
	m_bFireOnEmpty = true;

	m_iClipsLeft = 0;
	m_bSighted = false;

	m_bNeedsCocking = false;
	m_bIsCocking = false;

#ifdef GAME_DLL
	UseClientSideAnimation();
#endif
}

bool CHAJBoltRifle::CanReload( void )
{
	if( m_bIsCocking || m_bNeedsCocking )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Handles firing the weapon
//-----------------------------------------------------------------------------
void CHAJBoltRifle::PrimaryAttack( void )
{
	if( IsHolstered() || IsLowered() || m_nShotsFired >= 1 )
		return;

	// Only the player fires this way so we can cast
	CHajPlayer *pPlayer = ToHajPlayer( GetOwner() );
	if ( !pPlayer || ( pPlayer->GetFlags() & FL_ATCONTROLS ) )
		return;

	if( m_bNeedsCocking || m_bIsCocking )
	{
		if( !m_bIsCocking )
			CockWeapon();

		return;
	}
	
	// don't allow firing on a ladder
	if ( pPlayer->GetMoveType() == MOVETYPE_LADDER )
	{
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.15;
		return;
	}

	// don't allow firing when in a prone transition
	// don't allow firing when the sprint button is pressed and your moving	
	if( !m_bIsBipodDeployed )
	{
		if ( pPlayer->m_Local.m_bProning || ( pPlayer->m_Local.m_bProned && pPlayer->GetAbsVelocity().Length2D() >= 5 ) || ( pPlayer->IsSprinting() && pPlayer->GetAbsVelocity().Length2D() >= 5 ) )
		{
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.15;
			return;
		}
	}

	// reload if we're out of ammo
	if ( ( UsesClipsForAmmo1() && m_iClip1 == 0) || ( !UsesClipsForAmmo1() && !pPlayer->GetAmmoCount(m_iPrimaryAmmoType) ) )
	{
		if ( !m_bFireOnEmpty )
		{
			Reload();
		}
		else
		{
			WeaponSound( EMPTY );
			m_flNextPrimaryAttack = 0.15;
		}
		return;

	}

	CBasePlayer *pBasePlayer = ToBasePlayer( GetOwner() );
	
	// Fire the bullets
	FireBulletsInfo_t info;
	info.m_iShots = 1;
	info.m_vecSrc = pPlayer->Weapon_ShootPosition( );
	info.m_vecDirShooting = pBasePlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if( IsIronsighted() )
	{
		Vector NewSpread;
		VectorMultiply( pPlayer->GetAttackSpread( this ), Vector( 0.6, 0.6, 0.6 ), NewSpread );
		info.m_vecSpread = NewSpread;
	}
	else
		info.m_vecSpread = pPlayer->GetAttackSpread( this );

	info.m_flDistance = MAX_TRACE_LENGTH;
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iTracerFreq = 1; // for rifles, assume every round is tracer as we use invisible tracers that just make a whizz sound

	pPlayer->FireBullets( info );

	m_nShotsFired++;
	
	WeaponSound( SINGLE );
	pPlayer->DoMuzzleFlash();

	// set animations
	SendWeaponAnim( GetPrimaryAttackActivity() );

	// set the next attack time to after the length of the shoot
	// sequence. this means we can't fire again until after the bolt
	// working anim has done.
	MDLCACHE_CRITICAL_SECTION();
	float flSequenceEndTime = gpGlobals->curtime + SequenceDuration();
	//pPlayer->SetNextAttack( flSequenceEndTime );
	m_flNextPrimaryAttack = flSequenceEndTime;

	pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );

	SetWeaponIdleTime( flSequenceEndTime );

	m_iClip1 -= 1;

	if( m_iClip1 > 0 )
		m_bNeedsCocking = true;

	AddViewKick();
}

#ifdef CLIENT_DLL
float CHAJBoltRifle::SwayScale( void )
{
	if( m_bSighted )
		return 0.1f;

	return BaseClass::SwayScale( );
}
#endif

void CHAJBoltRifle::ItemPostFrame( void )
{
	if( HolsterTimeThink() )
		return;

	if ( !IsHolstered() && HasWeaponIdleTimeElapsed() ) 
	{
		if( m_bNeedsCocking && !m_bIsCocking )
		{
			CockWeapon();
			return;
		}
	}

	if( gpGlobals->curtime >= m_flFinishCockingTime && m_bIsCocking )
	{
		FinishCocking();
	}

	BaseClass::ItemPostFrame( );
}

void CHAJBoltRifle::WeaponIdle( void )
{
	if( IsHolstered() )
		return;

	if ( HasWeaponIdleTimeElapsed() && !m_bIsCocking ) 
	{
		if( IsIronsighted() )
			SendWeaponAnim( ACT_VM_IRONSIGHT_IDLE );
		else if( IsLowered() )
			SendWeaponAnim( ACT_VM_IDLE_LOWERED );	
		else
			BaseClass::WeaponIdle();
	}
}

void CHAJBoltRifle::CockWeapon( void )
{
	if( IsIronsighted() )
		SendWeaponAnim( ACT_VM_IRONSIGHT_COCK );
	else
		SendWeaponAnim( ACT_VM_COCK );
	
	m_flFinishCockingTime = gpGlobals->curtime + SequenceDuration();
	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );

	m_bIsCocking = true;
	m_bNeedsCocking = true;
}

void CHAJBoltRifle::FinishCocking()
{
	m_bNeedsCocking = false;
	m_bIsCocking = false;
	m_flFinishCockingTime = -1;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CHAJBoltRifle::GetPrimaryAttackActivity( void )
{
	if( IsIronsighted() )
		return ACT_VM_IRONSIGHT_PRIMARYATTACK;

	return ACT_VM_PRIMARYATTACK;
}

bool CHAJBoltRifle::Holster( CBaseCombatWeapon *pSwitchingTo )
{
#ifndef CLIENT_DLL
	ExitIronsights();
#endif

	if( m_bIsCocking && m_bNeedsCocking )
	{
		m_bIsCocking = false;
	}

	return BaseClass::Holster( pSwitchingTo );
}

void CHAJBoltRifle::Drop( const Vector &vecVelocity )
{
#ifndef CLIENT_DLL
	ExitIronsights();
#endif

	if( m_bIsCocking && m_bNeedsCocking )
	{
		m_bIsCocking = false;
	}

	BaseClass::Drop( vecVelocity );
}

void CHAJBoltRifle::SecondaryAttack( void )
{

}

void CHAJBoltRifle::FakeHolster()
{
	if( !IsHolstered() && m_bIsCocking )
	{
		m_bIsCocking = false;
	}

	BaseClass::FakeHolster();
}

bool CHAJBoltRifle::IsWeaponBusy()
{
	if( m_bIsCocking )
		return true;

	return BaseClass::IsWeaponBusy();
}