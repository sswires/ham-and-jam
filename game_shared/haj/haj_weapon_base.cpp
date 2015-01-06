//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose:  HaJ base gun class
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
#include "in_buttons.h"
#include "ammodef.h"

#if defined( CLIENT_DLL )
	#include "c_hl2mp_player.h"
	#include "haj_player_c.h"
	#include "prediction.h"

	#include "haj_hintsandtips.h"
	#include "vgui/IScheme.h"
#else
	#include "hl2mp_player.h"
	#include "haj_player.h"
	#include "haj_magazines.h"
#endif

#include "predicted_viewmodel.h"
#include "haj_weapon_base.h"
#include "weapon_satchelcharge.h"
#include "decals.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifdef CLIENT_DLL
ConVar haj_cl_autoreload( "haj_cl_autoreload", "1", FCVAR_USERINFO | FCVAR_ARCHIVE, "Enable/disable auto reloading of weapon." );
ConVar haj_cl_ironsight_style( "haj_cl_ironsight_style", "0", FCVAR_USERINFO + FCVAR_ARCHIVE, "1 = hold right click for ironsights, 0 = toggle"); 

ConVar haj_cl_ammowarning( "haj_cl_ammowarning", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Warn when low on ammo" );
#endif

//ConVar haj_autoswitch( "haj_autoswitch", "1", FCVAR_USERINFO | FCVAR_ARCHIVE, "Enable/disable auto switching to next best weapon" );

ConVar vm_ironsight_adjust( "vm_ironsight_adjust", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Edit ironsight offsets" );
ConVar vm_ironsight_x( "vm_ironsight_x", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Adjust ironsight X pos" );
ConVar vm_ironsight_y( "vm_ironsight_y", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Adjust ironsight Y pos" );
ConVar vm_ironsight_z( "vm_ironsight_z", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Adjust ironsight Z pos" );
ConVar vm_ironsight_pitch( "vm_ironsight_pitch", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Adjust ironsight pitch angle" );
ConVar vm_ironsight_yaw( "vm_ironsight_yaw", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Adjust ironsight yaw angle" );
ConVar vm_ironsight_roll( "vm_ironsight_roll", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Adjust ironsight roll angly" );
ConVar vm_ironsight_fov( "vm_ironsight_fov", "60", FCVAR_REPLICATED | FCVAR_CHEAT, "Adjust ironsight FOV" );
ConVar vm_ironsight_time( "vm_ironsight_time", "0.5", FCVAR_REPLICATED | FCVAR_CHEAT, "Adjust ironsight time" );
ConVar vm_ironsight_drawcrosshair( "vm_ironsight_drawcrosshair", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Draw crosshair in ironsight mode" );
ConVar vm_prone_move_sway_amount( "vm_prone_move_sway_amount", "20.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Amount of prone move VM sway" );
//=========================================================
// HaJ Gun base class tables
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED( HAJWeaponBase, DT_HAJWeaponBase )

BEGIN_NETWORK_TABLE( CHAJWeaponBase, DT_HAJWeaponBase )

/*
	CNetworkVar		( bool, m_bIsBipodDeployed );
	CNetworkVar		( bool, m_bDoBipodDeploy );
	
	CNetworkVar		( bool, m_bIsBipodDeploying );
	CNetworkVar		( bool, m_bIsBipodUnDeploying );
*/

#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_bLowered ) ),
	RecvPropFloat( RECVINFO( m_flRaiseTime ) ),
	RecvPropBool( RECVINFO( m_bIsBipodDeployed ) ),
	RecvPropBool( RECVINFO( m_bDoBipodDeploy ) ),
	RecvPropBool( RECVINFO( m_bIsBipodDeploying ) ),
	RecvPropBool( RECVINFO( m_bIsBipodUnDeploying ) ),
	RecvPropBool( RECVINFO( m_bIronsighted ) ),
	RecvPropBool( RECVINFO( m_bQueuedIronsights ) ),
	RecvPropFloat( RECVINFO( m_flIronsightTime ) ),

	RecvPropBool( RECVINFO( m_bInDeployTransition ) ),
	RecvPropFloat( RECVINFO( m_flTransitionHeight ) ),
	RecvPropFloat( RECVINFO( m_flStartviewHeight ) ),
	RecvPropFloat( RECVINFO( m_flTransitionTime ) ),
	RecvPropFloat( RECVINFO( m_flTransitionStartTime ) ),

	RecvPropBool( RECVINFO (m_bFakeHolstered )),
	RecvPropBool( RECVINFO( m_bHolstered ) ),

	RecvPropFloat( RECVINFO( m_flReloadEndTime ) ),
	RecvPropBool(RECVINFO( m_bAwaitingHolster )),
	RecvPropFloat( RECVINFO( m_flHolsterTime ) ),
#else
	SendPropBool( SENDINFO( m_bLowered ) ),
	SendPropFloat( SENDINFO( m_flRaiseTime ) ),
	SendPropBool( SENDINFO( m_bIsBipodDeployed ) ),
	SendPropBool( SENDINFO( m_bDoBipodDeploy ) ),
	SendPropBool( SENDINFO( m_bIsBipodDeploying ) ),
	SendPropBool( SENDINFO( m_bIsBipodUnDeploying ) ),
	SendPropBool( SENDINFO( m_bIronsighted ) ),
	SendPropBool( SENDINFO( m_bQueuedIronsights ) ),
	SendPropFloat( SENDINFO( m_flIronsightTime ) ),

	SendPropBool( SENDINFO( m_bInDeployTransition ) ),
	SendPropFloat( SENDINFO( m_flTransitionHeight ) ),
	SendPropFloat( SENDINFO( m_flTransitionTime ) ),
	SendPropFloat( SENDINFO( m_flTransitionStartTime ) ),
	SendPropFloat( SENDINFO( m_flStartviewHeight ) ),

	SendPropBool( SENDINFO( m_bFakeHolstered ) ),
	SendPropBool( SENDINFO( m_bHolstered ) ), // fo' real.

	SendPropFloat( SENDINFO( m_flReloadEndTime ) ),
	SendPropBool( SENDINFO(m_bAwaitingHolster ) ),
	SendPropFloat( SENDINFO( m_flHolsterTime ) ),

#endif

END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CHAJWeaponBase )
	DEFINE_PRED_FIELD_TOL( m_flIronsightTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE ),

	DEFINE_PRED_FIELD( m_bQueuedIronsights, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bIronsighted, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flIronsightTime , FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif

#ifdef GAME_DLL
BEGIN_DATADESC( CHAJWeaponBase )

	DEFINE_FIELD( m_nShotsFired,	FIELD_INTEGER ),
	DEFINE_FIELD( m_flNextSoundTime, FIELD_TIME ),

END_DATADESC()
#endif

//-----------------------------------------------------------------------------
// Purpose: Construtor
//-----------------------------------------------------------------------------
CHAJWeaponBase::CHAJWeaponBase( void )
{
	SetPredictionEligible( true );
	AddSolidFlags( FSOLID_TRIGGER ); // Nothing collides with these but it gets touches.

	m_fEfficientDistance = -1.0f;
	m_fBulletFadeFactor = -1.0f;

	m_bFiresUnderwater = false;
	m_bFireOnEmpty = true;
	m_bIronsighted = false;
	m_flReloadEndTime = 0.0f;

	m_iClipsLeft = 0;
	m_flHolsterTime = -1.0f;

	m_bDoBipodDeploy = false;
	m_bIsBipodDeployed = false;
	m_bIsBipodDeploying = false;
	m_bIsBipodUnDeploying = false;
	m_fBipodDeployTime = 0.0f;
	m_fBipodUnDeployTime = 0.0f;

#ifdef GAME_DLL
	UseClientSideAnimation();
#else
	vgui::HScheme scheme = vgui::scheme()->GetScheme( "ClientScheme" );
	m_hInfoFont = vgui::scheme()->GetIScheme(scheme)->GetFont( "WeaponHint", true );
#endif
}


float CHAJWeaponBase::GetSpreadScalar( void )
{
	float defcone = GetHL2MPWpnData().m_flCOFStand;					// default CoF is the horrid 20 degree one.
	//float	ramp = RemapValClamped( 0.1f, 0.0f, 0.2f, 0.0f, 1.0f);	// ramping values when Lerping per axis

	CHajPlayer *pPlayer = ToHajPlayer( GetOwner() );
	if ( !pPlayer ) return defcone;

	int	flags = pPlayer->GetFlags();								// get player's flags
	float flFinalCone = GetHL2MPWpnData().m_flCOFStand;

	// Stance checks
	if( flags & FL_DUCKING )
		flFinalCone *= GetHL2MPWpnData().m_flCOFCrouch;
	else if( flags & FL_PRONING )
		flFinalCone *= GetHL2MPWpnData().m_flCOFProne;
	else if( !( flags & FL_ONGROUND ) || m_bLowered )
		flFinalCone *= GetHL2MPWpnData().m_flCOFInAir;

	// Ironsight check
	if( ( IsIronsighted() || m_bIsBipodDeployed ) && ( flags & FL_ONGROUND ) )
		flFinalCone *= GetHL2MPWpnData().m_flCOFDeployed;

	// Speed check
	float fSpeedFraction;

	if( GetHL2MPWpnData().m_flMovementAccuracyPenalty > 0.0f && pPlayer->MaxSpeed() > 0 )
	{
		fSpeedFraction = ( pPlayer->GetAbsVelocity().Length2D() / pPlayer->MaxSpeed() );

		if( fSpeedFraction > 0.25f )
		{
			flFinalCone *= 1.0 + ( fSpeedFraction * GetHL2MPWpnData().m_flMovementAccuracyPenalty );
		}
	}

	return flFinalCone;
}

//-----------------------------------------------------------------------------
// Purpose: Default cone of fire. Should be defined locally for each weapon.
//-----------------------------------------------------------------------------
const Vector &CHAJWeaponBase::GetBulletSpread( void )
{
	m_vecSpread = GetCoFVector( GetSpreadScalar() );
	return m_vecSpread;
}

void CHAJWeaponBase::AddViewKick( void )
{
#ifdef CLIENT_DLL
	float recoil_min_vert, recoil_max_vert, recoil_min_horz, recoil_max_horz, recoil_is;
	GetRecoilValues( recoil_min_vert, recoil_max_vert, recoil_min_horz, recoil_max_horz, recoil_is );

	if( m_bIsBipodDeployed || m_bIronsighted )
	{
		if( recoil_is == 0.0f )
			return;

		if( recoil_is != 1.0f )
		{
			recoil_min_vert *= recoil_is;
			recoil_max_vert *= recoil_is;
			recoil_min_horz *= recoil_is;
			recoil_max_horz *= recoil_is;
		}
	}

	DoRecoil (recoil_min_vert, recoil_max_vert, recoil_min_horz, recoil_max_horz );
#endif
}

void CHAJWeaponBase::GetRecoilValues( float &minVert, float &maxVert, float &minHoriz, float &maxHoriz, float &ironsightMulti )
{
	minVert = GetHL2MPWpnData().m_flMinVertRecoil;
	maxVert = GetHL2MPWpnData().m_flMaxVertRecoil;
	minHoriz = GetHL2MPWpnData().m_flMinHorizRecoil;
	maxHoriz = GetHL2MPWpnData().m_flMaxHorizRecoil;

	ironsightMulti = GetHL2MPWpnData().m_flDeployedRecoilMultiply;
}

float CHAJWeaponBase::GetBulletFadeFactor()
{
	if( m_fBulletFadeFactor < 0 )
	{
		if( m_bIsBipodDeployed || m_bIronsighted )
			return GetHL2MPWpnData().m_flBulletFadeFactorIS;

		return GetHL2MPWpnData().m_flBulletFadeFactor;
	}

	return m_fBulletFadeFactor;
}

// *HaJ 020* - SteveUK
// Functions for the proficient distances of weapons, this will usually be overwritten
float CHAJWeaponBase::GetEfficientDistance()
{
	if( m_fEfficientDistance < 0 )
	{
		if( m_bIsBipodDeployed || m_bIronsighted )
		{
			return GetHL2MPWpnData().m_flRangeIronsighted;
		}

		return GetHL2MPWpnData().m_flRange;
	}

	return m_fEfficientDistance;
}

void CHAJWeaponBase::SetEfficientDistance( float dist )
{
	m_fEfficientDistance = dist;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CHAJWeaponBase::GetPrimaryAttackActivity( void )
{
	if( m_bIronsighted )
		return ACT_VM_IRONSIGHT_PRIMARYATTACK;
	else if ( m_bIsBipodDeployed )
		return ACT_VM_PRIMARYATTACK_DEPLOYED;
	else
		return ACT_VM_PRIMARYATTACK;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
#ifdef CLIENT_DLL
bool CHAJWeaponBase::ShouldPredict()
{
        if ( GetOwner() && GetOwner() == C_BasePlayer::GetLocalPlayer())
                return true;

        return BaseClass::ShouldPredict();
}
#endif

int CHAJWeaponBase::GetWorldModelIndex( void )
{	
	if ( m_bIsBipodDeployed )
		return m_iWorldModelDeployedIndex;

	return m_iWorldModelIndex;
}

// ironsight macros
bool CHAJWeaponBase::HasIronsights( void )
{
	if( vm_ironsight_adjust.GetBool() == true )
		return true;

	return GetHL2MPWpnData().bHasIronsights;
}

float CHAJWeaponBase::GetIronsightFOV( void )
{
	if( vm_ironsight_adjust.GetBool() )
		return vm_ironsight_fov.GetFloat();

	return GetHL2MPWpnData().flIronsightFOV;
}

float CHAJWeaponBase::GetIronsightTime( void )
{
	if( vm_ironsight_adjust.GetBool() )
		return vm_ironsight_time.GetFloat();

	if( HasBipod() )
		return 0.5f;

	return GetHL2MPWpnData().flIronsightTime;
}

Vector CHAJWeaponBase::GetIronsightPosition( void ) const
{
	if( vm_ironsight_adjust.GetBool() )
		return Vector( vm_ironsight_x.GetFloat(), vm_ironsight_y.GetFloat(), vm_ironsight_z.GetFloat() );

	return GetHL2MPWpnData().vecIronsightOffset;
}
 
QAngle CHAJWeaponBase::GetIronsightAngles( void ) const
{
	if( vm_ironsight_adjust.GetBool() )
		return QAngle( vm_ironsight_pitch.GetFloat(), vm_ironsight_yaw.GetFloat(), vm_ironsight_roll.GetFloat() );

	return GetHL2MPWpnData().angIronsightAngs;
}

void CHAJWeaponBase::SetIronsights( bool b )
{
	if( !HasIronsights() || b == m_bIronsighted || ( b && ( m_bHolstered || m_bFakeHolstered ) ) )
		return;

	CHajPlayer *pPlayer = ToHajPlayer( GetOwner() );

	if( !pPlayer || ( b && ( pPlayer->IsSprinting() || pPlayer->m_Local.m_bProning ) ) || ( b && pPlayer->m_Local.m_bProned && pPlayer->GetAbsVelocity().Length2D() > 2 ) )
		return;

	m_bIronsighted = b;

	if( m_flIronsightTime + GetIronsightTime() > gpGlobals->curtime )
	{
		float diff =  m_flIronsightTime + GetIronsightTime() - gpGlobals->curtime;
		m_flIronsightTime = gpGlobals->curtime - diff;
	}
	else
		m_flIronsightTime = gpGlobals->curtime;

	if( gpGlobals->curtime >= m_flNextPrimaryAttack )
		SetWeaponIdleTime( gpGlobals->curtime );

#ifndef CLIENT_DLL
	pPlayer->SetFOV( this, b ? GetIronsightFOV() : 0, GetIronsightTime() );
#endif
}

void CHAJWeaponBase::EnterIronsights( void )
{
	SetIronsights( true );
}


void CHAJWeaponBase::ExitIronsights( void )
{
	SetIronsights( false );
}

/*
void CHAJWeaponBase::EnterIronsights( void )
{
#ifndef CLIENT_DLL

	if( !HasIronsights() || m_bInReload || m_bIronsighted || gpGlobals->curtime < m_flNextSecondaryAttack )
		return;

	CHajPlayer *pPlayer = ToHajPlayer( GetOwner() );

	if( !pPlayer || pPlayer->IsSprinting() )
		return;

	if( vm_ironsight_drawcrosshair.GetBool() == false )
		pPlayer->ShowCrosshair( false );

	if( gpGlobals->curtime >= m_flNextPrimaryAttack )
		SetWeaponIdleTime( gpGlobals->curtime );

	pPlayer->SetFOV( this, GetIronsightFOV(), GetIronsightTime() );
	pPlayer->SetMaxSpeed( HAJ_NORM_SPEED / 2 );
	//pPlayer->EnableSprint( false );

	m_bIronsighted = true;
	m_flIronsightTime = gpGlobals->curtime;
#endif
}


void CHAJWeaponBase::ExitIronsights( void )
{
#ifndef CLIENT_DLL

	if( !HasIronsights() || !m_bIronsighted )
		return;

	CHajPlayer *pPlayer = ToHajPlayer( GetOwner() );

	if( !pPlayer )
		return;

	pPlayer->SetFOV( this, 0.0, GetIronsightTime() );
	pPlayer->ShowCrosshair( true );

	if( !pPlayer->IsSprinting() )
		pPlayer->SetMaxSpeed( HAJ_NORM_SPEED );

	//pPlayer->EnableSprint( true );

	m_bIronsighted = false;
	m_flIronsightTime = gpGlobals->curtime;
#endif
}
*/

#define HAJ_HIDEWEAPON_THINK_CONTEXT			"HAJWeaponBase_HideThink"
ConVar haj_instant_holster( "haj_instant_holster", "0", FCVAR_ARCHIVE | FCVAR_DEMO | FCVAR_REPLICATED );

bool CHAJWeaponBase::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	if( m_bFakeHolstered )
		return false;

	if( m_bIronsighted )
		ExitIronsights();

	m_bFakeHolstered = false;

	MDLCACHE_CRITICAL_SECTION();

	CBaseCombatCharacter *pOwner = GetOwner();
	CHajPlayer *pPlayer = ToHajPlayer( pOwner );

	// reset some stufff
	if( HasBipod() )
	{
#ifndef CLIENT_DLL
		if( pPlayer )
			pPlayer->SetFOV( this, 0, 0.0f );
#endif

		if( pPlayer && m_bIsBipodDeployed )
		{
			if ( !(pPlayer->GetFlags() & FL_PRONING) ) // if we are in prone dont touch the viewoffset
			{
				Vector vecViewOffset = pPlayer->GetViewOffset();

				vecViewOffset.z = m_flStartviewHeight;
				pPlayer->SetViewOffset( vecViewOffset );
			}

			pPlayer->RemoveFlag( FL_USINGBIPOD );
			pPlayer->SetMaxSpeed( HAJ_NORM_SPEED ); // slightly slower than normal
			pPlayer->SetMoveType( MOVETYPE_WALK );
		}

		m_bIsBipodDeployed = false;
		m_bDoBipodDeploy = false;
		m_bIsBipodDeploying = false;
		m_bIsBipodUnDeploying = false;
		m_bInDeployTransition = false;
	}

	CHAJWeaponBase *pHajWeapon = dynamic_cast<CHAJWeaponBase*>(pSwitchingTo);

	// Some weapon's don't have holster anims yet, so detect that
	float flSequenceDuration = 0;

	if( !m_bAwaitingHolster && SendWeaponAnim( ACT_VM_HOLSTER ) )
	{
		SetActivity( ACT_VM_HOLSTER );
		flSequenceDuration = SequenceDuration();
	}

	if( pHajWeapon )
	{
		if( m_bAwaitingHolster )
			pHajWeapon->SetHolsterTime( m_flHolsterTime );
		else
			pHajWeapon->SetHolsterTime( gpGlobals->curtime + flSequenceDuration );
	}

	// cancel any reload in progress.
	m_bInReload = false;
	m_bHolstered = true;

	AddEffects( EF_NODRAW );
	return true;

	/*
	if( m_bShouldSwitchGuns && gpGlobals->curtime < m_flSwitchTime )
	{
		m_bShouldSwitchGuns = false;
		m_bHolstered = false;

		SetWeaponIdleTime( gpGlobals->curtime );
		return false;
	}

	// kill any think functions
	SetThink(NULL);

	if (pOwner)
	{
		pOwner->SetNextAttack( gpGlobals->curtime + flSequenceDuration + 0.25 );
	}

	// If we don't have a holster anim, hide immediately to avoid timing issues
	if( pSwitchingTo && pSwitchingTo != this && pOwner && pOwner->Weapon_CanSwitchTo( pSwitchingTo ) )
	{
		if ( !flSequenceDuration )
		{
			SetWeaponVisible( false );

			m_bShouldSwitchGuns = false;
			m_pSwitchTo = NULL;
			m_flSwitchTime = 0.0f;
			m_bHolstered = true;

			return true;
		}
		else
		{
			// Hide the weapon when the holster animation's finished
			m_bShouldSwitchGuns = true;
			m_pSwitchTo = pSwitchingTo;
			m_flSwitchTime = gpGlobals->curtime + flSequenceDuration;
			m_bHolstered = true;

			SetContextThink( &CHAJWeaponBase::DelayedSwitch, m_flSwitchTime, HAJ_HIDEWEAPON_THINK_CONTEXT );
			SetWeaponIdleTime( m_flSwitchTime + 0.25f );

			RemoveEffects( EF_NODRAW );

			return false;
		}
	}

	return true;*/
	//return BaseClass::Holster( pSwitchingTo );
}

void CHAJWeaponBase::FakeHolster()
{
#ifdef CLIENT_DLL
	if ( prediction->InPrediction() && !prediction->IsFirstTimePredicted() )
		return;
#endif

	m_bInReload = false; // stop a reload
	m_bFakeHolstered = true; // bool for fake holster

	if( m_bIronsighted )
		ExitIronsights(); // exit ironsights

	if( !m_bAwaitingHolster )
	{
		SendWeaponAnim( ACT_VM_HOLSTER ); // play holster anim
		SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );

		m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
	}
}

void CHAJWeaponBase::FakeDeploy()
{
#ifdef CLIENT_DLL
	if ( prediction->InPrediction() && !prediction->IsFirstTimePredicted() )
		return;
#endif

	m_bFakeHolstered = false;
	SendWeaponAnim( ACT_VM_DRAW ); // deploy again

	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() ); // so the anim animation plays again
	m_flNextPrimaryAttack = GetWeaponIdleTime();
}



void CHAJWeaponBase::DelayedSwitch( )
{
	SetContextThink( NULL, 0, HAJ_HIDEWEAPON_THINK_CONTEXT );

	Ready(); // just in case we started sprinting during the holster sequence

	CHajPlayer *pPlayer = ToHajPlayer( GetOwner() );

	if( m_bShouldSwitchGuns && pPlayer && pPlayer->IsAlive() )
	{	
		SetWeaponVisible( false );

		if( m_pSwitchTo && m_pSwitchTo->Deploy( ) )
		{
#ifndef CLIENT_DLL
			pPlayer->SetActiveWeapon( m_pSwitchTo );
#endif

			if ( pPlayer->Weapon_ShouldSetLast( this, m_pSwitchTo ) )
			{
				pPlayer->Weapon_SetLast( GetLastWeapon() );
			}
		}
		else
		{
			SetWeaponVisible( true );
			Deploy();
			return;
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Play empty weapon sound
//-----------------------------------------------------------------------------
bool CHAJWeaponBase::PlayEmptySound()
{
	CPASAttenuationFilter filter( this );
	filter.UsePredictionRules();

	EmitSound( filter, entindex(), "Default.ClipEmpty" );
	
	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: Handles firing the weapon
//-----------------------------------------------------------------------------
void CHAJWeaponBase::PrimaryAttack( void )
{
	if( m_bHolstered || m_bFakeHolstered || m_bLowered )
		return;

	// Only the player fires this way so we can cast
	CHajPlayer *pPlayer = ToHajPlayer( GetOwner() );
	if ( !pPlayer )
		return;

	// don't allow firing on a ladder
	if ( pPlayer->GetMoveType() == MOVETYPE_LADDER || ( pPlayer->GetFlags() & FL_ATCONTROLS ) )
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
	
	// Auto reload or play the out of ammo sound
	if ( ( UsesClipsForAmmo1() && m_iClip1 == 0) || ( !UsesClipsForAmmo1() && !pPlayer->GetAmmoCount(m_iPrimaryAmmoType) ) )
	{
		if ( !m_bFireOnEmpty && CanReload() )
		{
			m_nShotsFired = 0;
			Reload();
		}
		else
		{
			WeaponSound( EMPTY );
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.15;
		}
		return;

	}

	m_nShotsFired++;

	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.

	// Make sure we don't fire more than the amount in the clip, if this weapon uses clips
	if ( UsesClipsForAmmo1() )
	{
		m_iClip1 -= 1;
	}

	CBasePlayer *pBasePlayer = ToBasePlayer( GetOwner() );

	// Fire the bullets
	FireBulletsInfo_t info;
	info.m_iShots = 1;
	info.m_vecSrc = pPlayer->Weapon_ShootPosition( );
	info.m_vecDirShooting = pBasePlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
	info.m_vecSpread = pPlayer->GetAttackSpread( this );
	info.m_flDistance = MAX_TRACE_LENGTH;
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iTracerFreq = 5; // *HAJ 020* - Jed 1:5 ball/tracer mix
	
	pPlayer->FireBullets( info );
	pPlayer->DoMuzzleFlash();

	WeaponSound( SINGLE );
	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();

/*
#ifdef CLIENT_DLL
	if ( m_flPrevPrimaryAttack <= gpGlobals->curtime )
	{
		SendWeaponAnim( GetPrimaryAttackActivity() );
		m_flPrevPrimaryAttack = m_flNextPrimaryAttack;
	}
#else
*/
	SendWeaponAnim( GetPrimaryAttackActivity() );

//#endif

	pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );

	//Factor in the view kick
	AddViewKick();
}

float CHAJWeaponBase::GetMaxPenetrationDistance( unsigned short surfaceType )
{
	float fPenetration = 12.0f;

	switch( surfaceType )
	{
		case CHAR_TEX_GLASS:
			fPenetration = 64.0f;
			break;

		case CHAR_TEX_CONCRETE:
			fPenetration = 12.0f; //12 inches of concrete max
			break;
		case CHAR_TEX_METAL:
			fPenetration = 10.0f;
			break;
		case CHAR_TEX_SAND:
			fPenetration = 2.0f;
			break;
		case CHAR_TEX_VENT:
			fPenetration = 12.0f;
			break;
		case CHAR_TEX_WOOD:
		case CHAR_TEX_TILE:
			fPenetration = 24.0f; 
			break;
		case CHAR_TEX_PLASTIC:
		case CHAR_TEX_DIRT:
			fPenetration = 18.0f;
			break;
		case CHAR_TEX_FOLIAGE:
			fPenetration = 512.0f;
			break;
		default:
			fPenetration = 12.0f;
	}

	return fPenetration;
}

//-----------------------------------------------------------------------------
// Purpose: Reset our shots fired
//-----------------------------------------------------------------------------
bool CHAJWeaponBase::Deploy( void )
{
	CHajPlayer *pPlayer = ToHajPlayer(GetOwner());

	if( pPlayer )
	{
		pPlayer->GetAnimState()->ResetGestureSlot( GESTURE_SLOT_ATTACK_AND_RELOAD );
	}

	m_nShotsFired = 0;

	m_bShouldSwitchGuns = false;
	m_bHolstered = false;
	m_bFakeHolstered = false;

	if ( !HasAnyAmmo() && AllowsAutoSwitchFrom() )
		return false;

	if( !HasFinishedHolstering() )
	{
		m_bAwaitingHolster = true;
		return true;
	}

	m_bAwaitingHolster = false;

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: Recoil
//-----------------------------------------------------------------------------
void CHAJWeaponBase::DoRecoil( float min_vert, float max_vert, float min_horz, float max_horz )
{
	CHajPlayer *pPlayer = ToHajPlayer( GetOwner() );

	if ( pPlayer == NULL )
		return;

#ifdef CLIENT_DLL

	if ( prediction->InPrediction() && !prediction->IsFirstTimePredicted() )
		return;
    
	QAngle pAngles;
	pAngles.Init();
		
	pAngles.x = random->RandomFloat( min_vert, max_vert ); // Vertical component. Negative up.
	pAngles.y = random->RandomFloat( min_horz, max_horz ); // Horizontal component. Negative left.

	if ( pAngles.x < 0 ) pAngles.x *= -1;

	pPlayer->m_flPitchRecoilAccumulator = 0.0;
	pPlayer->m_flYawRecoilAccumulator = 0.0;

	pPlayer->m_flRecoilTimeRemaining = 0.1;
	pPlayer->m_flYawRecoilAccumulator += pAngles.y;
	pPlayer->m_flPitchRecoilAccumulator += pAngles.x;

	/* old code
	pPlayer->AddViewOffset( pAngles );
	
	// adds a little softning shimmy to stop the recoil looking so jerky
	pPlayer->ViewPunch( QAngle( -0.5, 0, 0 ) );
	*/
#endif

	pPlayer->ViewPunch( QAngle( GetHL2MPWpnData().m_flPunchPitch, GetHL2MPWpnData().m_flPunchYaw, 0.0f  ) );
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: Adds view "bob" to machine gun derived weapons
//-----------------------------------------------------------------------------
ConVar haj_cl_weaponbob_cycle_min( "haj_weaponbob_cycle_min", "0.25", FCVAR_CLIENTDLL, "Minimum bob cycle" );
ConVar haj_cl_weaponbob_cycle_max( "haj_weaponbob_cycle_max", "0.32", FCVAR_CLIENTDLL, "Maximum bob cycle" );
ConVar haj_cl_weaponbob_bob( "haj_weaponbob_bob", "0.002", FCVAR_CLIENTDLL, "Bob magnitude" );
ConVar haj_cl_weaponbob_bob_up( "haj_weaponbob_bob_up", "0.5", FCVAR_CLIENTDLL, "Bob magnitude" );

ConVar haj_cl_weaponbob_normal( "haj_weaponbob_normal", "1.2", FCVAR_CLIENTDLL );
ConVar haj_cl_weaponbob_lowered( "haj_weaponbob_lowered", "2.5", FCVAR_CLIENTDLL );
ConVar haj_cl_weaponbob_ironsights( "haj_weaponbob_ironsights", "0.3", FCVAR_CLIENTDLL );

#define	HL2_BOB_CYCLE_MIN	haj_cl_weaponbob_cycle_min.GetFloat()
#define	HL2_BOB_CYCLE_MAX	haj_cl_weaponbob_cycle_max.GetFloat()
#define	HL2_BOB				haj_cl_weaponbob_bob.GetFloat()
#define	HL2_BOB_UP			haj_cl_weaponbob_bob_up.GetFloat()

extern float	g_lateralBob;
extern float	g_verticalBob;

// Register these cvars if needed for easy tweaking
//static ConVar	v_iyaw_cycle( "v_iyaw_cycle", "2", FCVAR_REPLICATED | FCVAR_CHEAT );
//static ConVar	v_iroll_cycle( "v_iroll_cycle", "0.5", FCVAR_REPLICATED | FCVAR_CHEAT );
//static ConVar	v_ipitch_cycle( "v_ipitch_cycle", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
//static ConVar	v_iyaw_level( "v_iyaw_level", "0.3", FCVAR_REPLICATED | FCVAR_CHEAT );
//static ConVar	v_iroll_level( "v_iroll_level", "0.1", FCVAR_REPLICATED | FCVAR_CHEAT );
//static ConVar	v_ipitch_level( "v_ipitch_level", "0.3", FCVAR_REPLICATED | FCVAR_CHEAT );

#endif

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
#ifdef CLIENT_DLL
extern ConVar crosshair_dynamic;
void CHAJWeaponBase::Redraw()
{
	if( !crosshair_dynamic.GetBool() || ShouldForceTextureCrosshair() )
		BaseClass::Redraw();

	if( haj_cl_ammowarning.GetBool() && !m_bInReload && !AllowsAutoSwitchFrom() )
	{
		wchar_t noAmmo[150];

		CHajPlayer *pOwner = ToHajPlayer(GetOwner());
		int iAmmo = pOwner->GetAmmoCount( m_iPrimaryAmmoType );

		if( iAmmo > 0 && m_iClip1 <= (GetMaxClip1() * 0.15) )
			vgui::localize()->ConstructString( noAmmo, sizeof( noAmmo), "#HaJ_ReloadWeapon", 0 );
		else if( iAmmo < 1 && m_iClip1 > 0 && m_iClip1 <= (GetMaxClip1() * 0.5) )
			vgui::localize()->ConstructString( noAmmo, sizeof( noAmmo), "#HaJ_LowAmmo", 0 );
		else if( !HasAnyAmmo() )
			vgui::localize()->ConstructString( noAmmo, sizeof( noAmmo), "#HaJ_NoAmmo", 0 );
		else
			return; // nothing to warn about

		float red = 100 + ( 155 * ( ( sin( gpGlobals->curtime * 5 ) / 2 ) + 0.5 ) );
		int tw, th;

		surface()->GetTextSize( m_hInfoFont, noAmmo, tw, th );

		surface()->DrawSetTextColor( 0, 0, 0, 255 );
		surface()->DrawSetTextFont( m_hInfoFont );
		surface()->DrawSetTextPos( ( ScreenWidth() / 2 ) - ( tw / 2 ) + 1, (ScreenHeight() * 0.6) + 1 );
		surface()->DrawPrintText( noAmmo, wcslen( noAmmo ) );

		surface()->DrawSetTextColor( red, 0, 0, 255 );
		surface()->DrawSetTextFont( m_hInfoFont );
		surface()->DrawSetTextPos( ( ScreenWidth() / 2 ) - ( tw / 2 ), ScreenHeight() * 0.6 );
		surface()->DrawPrintText( noAmmo, wcslen( noAmmo ) );

	}
}

float CHAJWeaponBase::BobScale()
{
	if( m_bIronsighted )
		return haj_cl_weaponbob_ironsights.GetFloat();

	if( m_bLowered )
		return haj_cl_weaponbob_lowered.GetFloat();

	return haj_cl_weaponbob_normal.GetFloat();
}

float CHAJWeaponBase::CalcViewmodelBob( void )
{
	static	float bobtime;
	static	float lastbobtime;
	float	cycle;
	
	CBasePlayer *player = ToBasePlayer( GetOwner() );
	//Assert( player );

	//NOTENOTE: For now, let this cycle continue when in the air, because it snaps badly without it

	if( m_bDoBipodDeploy == true )
	{
		g_verticalBob = 0.0f;
		g_lateralBob = 0.0f;
		return 0.0f;
	}

	if ( ( !gpGlobals->frametime ) || ( player == NULL ) )
	{
		//NOTENOTE: We don't use this return value in our case (need to restructure the calculation function setup!)
		return 0.0f;// just use old value
	}

	//Find the speed of the player
	float speed = player->GetLocalVelocity().Length2D();

	//FIXME: This maximum speed value must come from the server.
	//		 MaxSpeed() is not sufficient for dealing with sprinting - jdw

	speed = clamp( speed, -320, 320 );

	if( player->GetFlags() & FL_PRONING )
	{
		speed = clamp( speed, 0, 42.5 );

		if( speed > 5.0f )
			speed *= 3.0f;
		else
			speed = 0.0f;
	}

	float bob_offset = RemapVal( speed, 0, 320, 0.0f, 1.0f );
	
	bobtime += ( gpGlobals->curtime - lastbobtime ) * bob_offset;
	lastbobtime = gpGlobals->curtime;

	//Calculate the vertical bob
	cycle = bobtime - (int)(bobtime/HL2_BOB_CYCLE_MAX)*HL2_BOB_CYCLE_MAX;
	cycle /= HL2_BOB_CYCLE_MAX;

	if ( cycle < HL2_BOB_UP )
	{
		cycle = M_PI * cycle / HL2_BOB_UP;
	}
	else
	{
		cycle = M_PI + M_PI*(cycle-HL2_BOB_UP)/(1.0 - HL2_BOB_UP);
	}
	
	g_verticalBob = speed*0.005f;
	g_verticalBob = g_verticalBob*0.3 + g_verticalBob*0.7*sin(cycle);

	g_verticalBob = clamp( g_verticalBob, -7.0f, 4.0f );
	g_verticalBob *= BobScale();

	//Calculate the lateral bob
	cycle = bobtime - (int)(bobtime/HL2_BOB_CYCLE_MAX*2)*HL2_BOB_CYCLE_MAX*2;
	cycle /= HL2_BOB_CYCLE_MAX*2;

	if ( cycle < HL2_BOB_UP )
	{
		cycle = M_PI * cycle / HL2_BOB_UP;
	}
	else
	{
		cycle = M_PI + M_PI*(cycle-HL2_BOB_UP)/(1.0 - HL2_BOB_UP);
	}

	g_lateralBob = speed*0.005f;
	g_lateralBob = g_lateralBob*0.3 + g_lateralBob*0.7*sin(cycle);
	g_lateralBob = clamp( g_lateralBob, -7.0f, 4.0f );
	g_lateralBob *= BobScale();
	
	//NOTENOTE: We don't use this return value in our case (need to restructure the calculation function setup!)
	return 0.0f;
}
#endif 

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &origin - 
//			&angles - 
//			viewmodelindex - 
//-----------------------------------------------------------------------------
#ifdef CLIENT_DLL
void CHAJWeaponBase::AddViewmodelBob( CBaseViewModel *viewmodel, Vector &origin, QAngle &angles )
{
	// don't do view bob when deploying
	if ( m_bDoBipodDeploy )
		return;

	CHajPlayer *pPlayer = ToHajPlayer( GetOwner() );

	if( !pPlayer )
		return;

	Vector	forward, right;
	AngleVectors( angles, &forward, &right, NULL );

	CalcViewmodelBob();

	// Apply bob, but scaled down to 40%
	VectorMA( origin, g_verticalBob * 0.1f, forward, origin );
	
	// Z bob a bit more
	origin[2] += g_verticalBob * 0.1f;
	
	// bob the angles
	angles[ ROLL ]	+= g_verticalBob * 0.5f;
	angles[ PITCH ]	-= g_verticalBob * 0.4f;

	if( pPlayer->GetFlags() & FL_PRONING && !pPlayer->m_Local.m_bDoUnProne )
		angles[ YAW ]	-= g_lateralBob  * vm_prone_move_sway_amount.GetFloat();
	else
		angles[ YAW ]	-= g_lateralBob  * 0.3f;

	// sprint
	if( pPlayer->MaxSpeed() > HAJ_NORM_SPEED )
	{
		origin.z += sin( gpGlobals->curtime * 3 ) * 0.45f;
		angles[ROLL] += sin( gpGlobals->curtime * 3 ) * 0.4f; 
	}

	VectorMA( origin, g_lateralBob * 0.8f, right, origin );
}

// How much to scale the sway on the view model by, this overrides cl_wpn_sway_scale.
float CHAJWeaponBase::SwayScale( void )
{
	if( m_bDoBipodDeploy || m_bIronsighted )
		return 0.05f;

	return 1.0f;
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Make enough sound events to fill the estimated think interval
// returns: number of shots needed
//-----------------------------------------------------------------------------
int CHAJWeaponBase::WeaponSoundRealtime( WeaponSound_t shoot_type )
{
	int numBullets = 0;

	// ran out of time, clamp to current
	if (m_flNextSoundTime < gpGlobals->curtime)
	{
		m_flNextSoundTime = gpGlobals->curtime;
	}

	// make enough sound events to fill up the next estimated think interval
	float dt = clamp( m_flAnimTime - m_flPrevAnimTime, 0, 0.2 );
	if (m_flNextSoundTime < gpGlobals->curtime + dt)
	{
		WeaponSound( SINGLE_NPC, m_flNextSoundTime );
		m_flNextSoundTime += GetFireRate();
		numBullets++;
	}
	if (m_flNextSoundTime < gpGlobals->curtime + dt)
	{
		WeaponSound( SINGLE_NPC, m_flNextSoundTime );
		m_flNextSoundTime += GetFireRate();
		numBullets++;
	}

	return numBullets;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHAJWeaponBase::ItemPostFrame( void )
{
	CHajPlayer *pOwner = dynamic_cast<CHajPlayer*>( GetOwner() );
	
	if ( pOwner == NULL )
		return;

	if( HolsterTimeThink() == true )
		return;

#ifndef CLIENT_DLL
	bool bAltIronsights = (bool)( pOwner->IsBot() || atoi( engine->GetClientConVarValue( pOwner->entindex(), "haj_cl_ironsight_style" ) ) > 0 );
#else
	bool bAltIronsights = haj_cl_ironsight_style.GetBool();
#endif

	// toggle IS as fast as player can click with COD style ironsight queuing
	if( HasIronsights() || HasBipod() )
	{
		if( bAltIronsights && !HasBipod() && !m_bInReload ) // HOLD for ironsights
		{
			if( ( pOwner->m_nButtons & IN_ATTACK2 ))
			{
				if( !IsIronsighted() )
					SetIronsights(true);
			}
			else if( IsIronsighted() )
				SetIronsights( false );
		}
		else
		{
			if ( ( pOwner->m_afButtonPressed & IN_ATTACK2 ) )
			{
				if( HasBipod() )
					TryBipod();

				if( !m_bIsBipodDeployed && !m_bIsBipodDeploying && !m_bIsBipodUnDeploying && !m_bDoBipodDeploy )
				{
					if( !m_bInReload )
						ToggleIronsights();
					else
						m_bQueuedIronsights = !m_bQueuedIronsights;
				}
			}
		}
	}

	if( m_bIsBipodDeployed && !m_bIsBipodUnDeploying )
	{
		if( pOwner->m_nButtons & (IN_FORWARD|IN_MOVELEFT|IN_MOVERIGHT|IN_BACK) )
		{
			m_bInReload = false;
			TryBipod();
		}
	}

	// Debounce the recoiling counter
	if ( ( pOwner->m_nButtons & IN_ATTACK ) == false )
	{
		m_nShotsFired = 0;
	}

	// HACK to get the weapon to unironsight if we start moving while prone
	if( pOwner->m_Local.m_bProned )
	{
		if( pOwner->GetAbsVelocity().Length() > 5 )
		{
			if( m_bIronsighted )
				ExitIronsights();

			if( !m_bLowered )
				Lower();
		}
		else if( m_bLowered )
		{
			Ready();
		}
	}

	//*HAJ 020* Ztormi
	// Check if we are in machinegun viewtransition
	DeployTransition();

// temporary stuff to show the ammo status.
/*
#ifdef CLIENT_DLL
	int i;
	engine->Con_NPrintf( 0, "Main Clip: %d", m_iClip1 );

	for (i = 0; i < MAX_CLIPS_PER_WEAPON; i++)
		engine->Con_NPrintf( i+1, "Clip %d: %d", i, m_iAmmoInClip[i] );
	engine->Con_NPrintf( i+1, "InReload: %d", m_bInReload ? 1:0 );
#endif
*/	
	HandleBipodDeployment();

	// THE OLD BASE CLASS FROM HERE
	// (modified ofc)

	//Track the duration of the fire
	//FIXME: Check for IN_ATTACK2 as well?
	//FIXME: What if we're calling ItemBusyFrame?
	m_fFireDuration = ( pOwner->m_nButtons & (IN_ATTACK|IN_ATTACK2) ) ? ( m_fFireDuration + gpGlobals->frametime ) : 0.0f;

	if ( UsesClipsForAmmo1() )
	{
		CheckReload();
	}

	// Secondary attack has priority
	if ( !HasIronsights() && (pOwner->m_nButtons & IN_ATTACK2) && (m_flNextSecondaryAttack <= gpGlobals->curtime))
	{
		if (UsesSecondaryAmmo() && pOwner->GetAmmoCount(m_iSecondaryAmmoType)<=0 )
		{
			if (m_flNextEmptySoundTime < gpGlobals->curtime)
			{
				WeaponSound(EMPTY);
				m_flNextSecondaryAttack = m_flNextEmptySoundTime = gpGlobals->curtime + 0.5;
			}
		}
		else if (pOwner->GetWaterLevel() == 3 && m_bAltFiresUnderwater == false)
		{
			// This weapon doesn't fire underwater
			WeaponSound(EMPTY);
			m_flNextSecondaryAttack = gpGlobals->curtime + 0.2;
			return;
		}
		else
		{
			// FIXME: This isn't necessarily true if the weapon doesn't have a secondary fire!
			SecondaryAttack();

			// Secondary ammo doesn't have a reload animation
			if ( UsesClipsForAmmo2() )
			{
				// reload clip2 if empty
				if (m_iClip2 < 1)
				{
					pOwner->RemoveAmmo( 1, m_iSecondaryAmmoType );
					m_iClip2 = m_iClip2 + 1;
				}
			}
		}
	}

	if ( !IsWeaponBusy() && (pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		// Clip empty? Or out of ammo on a no-clip weapon?
		if ( !IsMeleeWeapon() &&  
			(( UsesClipsForAmmo1() && m_iClip1 <= 0) || ( !UsesClipsForAmmo1() && pOwner->GetAmmoCount(m_iPrimaryAmmoType)<=0 )) )
		{
			HandleFireOnEmpty();
		}
		else if (pOwner->GetWaterLevel() == 3 && m_bFiresUnderwater == false)
		{
			// This weapon doesn't fire underwater
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
			return;
		}
		else
		{
			//NOTENOTE: There is a bug with this code with regards to the way machine guns catch the leading edge trigger
			//			on the player hitting the attack key.  It relies on the gun catching that case in the same frame.
			//			However, because the player can also be doing a secondary attack, the edge trigger may be missed.
			//			We really need to hold onto the edge trigger and only clear the condition when the gun has fired its
			//			first shot.  Right now that's too much of an architecture change -- jdw


			// *HAJ 020* - Jed
			// found this on HL coders. To quote:
			//
			//"The bug is caused by the networked variable m_flNextPrimaryAttack being
			//reset after the client has simulated firing, but the server has not
			//fired the weapon yet.  The client will fire their weapon and increase
			//m_flNextPrimaryAttack to a time in the future.  When the client receives
			//the next server update, this variable is reset to the server's value of
			//m_flNextPrimaryAttack which has not recorded that a shot has been fired
			//yet.  With the value now being less than curtime, it's ok to fire the
			//weapon again which is much sooner than it should be."
			//
			// Oddly this seems to help with the bug where aftershooting the view would try and snap back.

			//PrimaryAttack();
#ifdef CLIENT_DLL
			if ( m_flPrevPrimaryAttack <= gpGlobals->curtime )
			{
#endif
				PrimaryAttack();
#ifdef CLIENT_DLL
				m_flPrevPrimaryAttack = m_flNextPrimaryAttack;
			}
#endif
		}
	}

	// -----------------------
	//  Reload pressed / Clip Empty
	// -----------------------
	if ( pOwner->m_nButtons & IN_RELOAD && UsesClipsForAmmo1() && !m_bInReload ) 
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		Reload();
		m_fFireDuration = 0.0f;
	}

	// Idle seq
	if ( gpGlobals->curtime >= m_flNextPrimaryAttack && ( !ReloadOrSwitchWeapons() && ( m_bInReload == false ) ) )
	{
		WeaponIdle();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Override the default reload function
//-----------------------------------------------------------------------------
#ifdef CLIENT_DLL
extern CHudHintsPanel* g_HintsPanel;
#endif

bool CHAJWeaponBase::DefaultReload( int iClipSize1, int iActivityEmpty, int iActivityFull )
{
	if( IsHolstered() || !CanReload() || gpGlobals->curtime < m_flNextPrimaryAttack )
		return false;

	CHajPlayer *pOwner = ToHajPlayer( GetOwner() );
	if ( !pOwner )
		return false;

	if ( pOwner->GetMoveType() == MOVETYPE_LADDER )
		return false;

	// don't allow reload when in a prone transition
	if ( pOwner->m_Local.m_bProning )
		return false;
	
	// don't allow reload when the sprint button is pressed and your moving
	if ( pOwner->IsSprinting() && pOwner->GetAbsVelocity().Length2D() >= 5 )
		return false;

	// If the current clip isn't full and we have ammo, reload.
	int iAmmo = pOwner->GetAmmoCount( m_iPrimaryAmmoType );

	if( !(m_iClip1 < GetMaxClip1() && iAmmo > 0 ) )
		return false;

#ifdef CLIENT_DLL
	WeaponSound( RELOAD ); // Play reload sound
#endif
	
	if ( m_iClip1 > 0 )
		SendWeaponAnim( iActivityFull );
	else
		SendWeaponAnim( iActivityEmpty );

	MDLCACHE_CRITICAL_SECTION();
	float flSeqDur = SequenceDuration();
	float flSequenceEndTime = gpGlobals->curtime + flSeqDur;
	float flSpeed = 1.0f;

	// reload team bonus
	if( pOwner->GetNearbyTeammates() > 0 )
	{
		flSpeed = ( 1 + ( pOwner->GetNearbyTeammates() * 0.1 ) );
		float flCalcRed = flSeqDur / flSpeed;

		flSequenceEndTime = gpGlobals->curtime + flCalcRed;
		
		m_flPlaybackRate = flSpeed;
		pOwner->GetViewModel()->SetPlaybackRate( flSpeed );
	}

	pOwner->SetNextAttack( flSequenceEndTime );
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = flSequenceEndTime;

	if( GetHL2MPWpnData().m_flReloadTime > 0 )
	{
		if( m_iClip1 > 0 )
			m_flReloadEndTime = gpGlobals->curtime + ( GetHL2MPWpnData().m_flReloadTime / flSpeed );
		else
			m_flReloadEndTime = gpGlobals->curtime + ( GetHL2MPWpnData().m_flReloadEmptyTime / flSpeed );
	}
	else
		m_flReloadEndTime = gpGlobals->curtime + ( flSeqDur * 0.9f );

	// Play the player's reload animation
	if ( pOwner->IsPlayer() )
	{
		pOwner->DoAnimationEvent( PLAYERANIMEVENT_RELOAD );

#ifndef CLIENT_DLL
		if( m_bIsBipodDeployed )
			ToBasePlayer(pOwner)->SetFOV( this, 0, 0.3 );
#endif
	}

	if( m_bIronsighted )
		ExitIronsights();

	m_bInReload = true;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Reload has finished. Actuall updating of ammo happens here.
// TODO: Need to stop this activating all the time when autoreload is OFF!!!!
//-----------------------------------------------------------------------------
void CHAJWeaponBase::FinishReload( void )
{
	CHajPlayer *pOwner = ToHajPlayer(GetOwner());

	if ( pOwner && m_bInReload )
	{

#ifdef GAME_DLL
		if( m_bQueuedIronsights )
		{
			SetIronsights( true );
			m_bQueuedIronsights = false;
		}

		CHajPlayer *pPlayer = ToHajPlayer( pOwner );

		if( pPlayer && m_bIsBipodDeployed )
			pPlayer->SetFOV( this, 75, 0.3 );
#endif

		// REMOVE ME: old magazine system
#if 0
		// we have ammo in another clip
		if ( GetNextMagazine() )
		{
			// how much ammo do we have and how much in the new mag?
			int iCurrentAmmo = m_iClip1.Get();
			int iFromClip = m_iAmmoInClip.Get( m_iCurrentClip );
			
			// take it from the spare to our current mag
			m_iClip1.Set( iFromClip );
			m_iAmmoInClip.Set( m_iCurrentClip, iCurrentAmmo );
				
			// update the spare mag counter
			m_iClipsLeft = MagsLeft();
		}
#endif

		// ammo info
		Ammo_t* pAmmo = GetAmmoDef()->GetAmmoOfIndex( m_iPrimaryAmmoType );

		// check if we're a magazine based weapon
		if( pAmmo && FBitSet( pAmmo->nFlags, AMMO_USE_MAGAZINES ) )
		{
#ifdef GAME_DLL
			// get point to magazine instance of weapon's ammo type
			CHajWeaponMagazines *pMagazines = pOwner->GetMagazines( m_iPrimaryAmmoType );

			if( pMagazines )
			{
				pMagazines->UpdateCurrent( m_iClip1 );
				m_iClip1 = pMagazines->SwitchToBest();

				m_bInReload = false;

				return; // skip base class
			}
#else
			return;
#endif
		}

	}

	BaseClass::FinishReload(); // buckets of ammo if we don't use mags
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if the weapon currently has ammo or doesn't need ammo
// Output :
//-----------------------------------------------------------------------------
bool CHAJWeaponBase::HasPrimaryAmmo( void )
{
	return BaseClass::HasPrimaryAmmo();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHAJWeaponBase::CheckReload( void )
{
	CHajPlayer *pOwner = ToHajPlayer(GetOwner());

	if( pOwner && !m_bReloadsSingly )
	{
		if ( m_bInReload && gpGlobals->curtime >= m_flReloadEndTime )
		{
			FinishReload();
			m_bInReload = false;
		}

		return;
	}

	BaseClass::CheckReload();
}

bool CHAJWeaponBase::Reload( void )
{
	if( CanReload() )
	{
		return BaseClass::Reload();
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: If the current weapon has more ammo, reload it. Otherwise, switch 
//			to the next best weapon we've got. Returns true if it took any action.
//-----------------------------------------------------------------------------
bool CHAJWeaponBase::ReloadOrSwitchWeapons( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	Assert( pOwner );

	m_bFireOnEmpty = false;

	// If we don't have any ammo, switch to the next best weapon
	if ( !HasAnyAmmo() && m_flNextPrimaryAttack < gpGlobals->curtime && m_flNextSecondaryAttack < gpGlobals->curtime )
	{
		// weapon isn't useable, switch.
		if ( AllowsAutoSwitchFrom() && g_pGameRules->SwitchToNextBestWeapon( pOwner, this ) )
		{
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.3;
			UnlockBipod();
			return true;
		}
	}
	else
	{
		// Weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
		// control auto-reloads via console var.
		if ( UsesClipsForAmmo1() && 
			 ( m_iClip1 == 0 ) && 
			 !( pOwner->m_nButtons & (IN_ATTACK|IN_RELOAD) ) &&
#ifdef CLIENT_DLL
			 haj_cl_autoreload.GetBool() &&
#else
			 ( pOwner->IsBot() || atoi( engine->GetClientConVarValue( pOwner->entindex(), "haj_cl_autoreload" ) ) > 0 ) &&
#endif
			 m_flNextPrimaryAttack < gpGlobals->curtime && 
			 m_flNextSecondaryAttack < gpGlobals->curtime )
		{
				return  Reload();
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: returns COF angle vector based on angle in degrees.
//-----------------------------------------------------------------------------
Vector CHAJWeaponBase::GetCoFVector( float coneangle )
{
	float radangle = DEG2RAD( coneangle / 2 );
	float sine, cosine;
	SinCos( radangle, &sine, &cosine );
	
	return Vector ( sine, sine, sine );
}

//-----------------------------------------------------------------------------
// Purpose: Function to override the mouse input and scale it down for when
// moving prone. Rather than clamp our view angles it still lets us turn, just 
// at one tenth the speed.
//-----------------------------------------------------------------------------
#ifdef CLIENT_DLL
ConVar haj_cl_ironsight_sensitivity( "haj_cl_ironsight_sensitivity", "0.75", FCVAR_CLIENTDLL + FCVAR_ARCHIVE, "Sensitivity multiplier for ironsight mode.", true, 0.1f, true, 2.0f );
#endif

void CHAJWeaponBase::OverrideMouseInput( float *x, float *y )
{
#ifdef CLIENT_DLL
	CHajPlayer *pPlayer = ToHajPlayer( GetOwner() );
	if ( !pPlayer )
		return;

	if ( m_bIronsighted )
	{
		*x *= haj_cl_ironsight_sensitivity.GetFloat();
		*y *= haj_cl_ironsight_sensitivity.GetFloat();
	}

	if ( pPlayer->m_bLimitTurnSpeed )
	{
		*x *= 0.15f;
		*y *= 0.5f;
	}

	if ( m_bIsBipodDeploying || m_bIsBipodDeployed )
	{
		*x *= 0.5f;
		*y *= 0.5f;
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHAJWeaponBase::TryBipod( void )
{
	
	// *HAJ 020* - Ztormi
	// Trace the spot
	if ( !m_bIsBipodDeployed )
	{
		if ( !CanDeployBipod( true ) )
			return;
	}

	m_bDoBipodDeploy = true;
	m_bInReload = false;
}
//-----------------------------------------------------------------------------
// *HAJ 020* Ztormi
// Purpose: Does tracelines to determine if the spot is suitable for deploying the machinegun
//-----------------------------------------------------------------------------
#define MG_VIEW_TRANSITION_TIME 0.8f

bool CHAJWeaponBase::CanDeployBipod( bool bUsesTransition )
{
	//This is the old bit of code, moved it to this function from trybipod - Ztormi

	// Check to see if we are in the air.
	bool bInAir = ( GetOwner()->GetGroundEntity() == NULL );
	//bool bInProne = ( GetOwner()->GetFlags() & FL_PRONING ) ? true : false;
	//bool bIsCrouched = ( GetOwner()->GetFlags() & FL_DUCKING ) ? true : false;

	float flCurrentHeight = -1.0f;
	bool bValidHeight = false;

	CHajPlayer *pPlayer = ToHajPlayer( GetOwner() );
	//bool bCanDeployInDeployZone = false; commented this out for now, returns true for this function - Ztormi

	if (!pPlayer || pPlayer->m_Local.m_bProning || pPlayer->IsSprinting() || m_bLowered ) // false for mid-prone deploy attempt, or if the gun is lowered
		return false;

	// don't try and deploy if we're not prone, in the air
	// trying to deploy or reloading *phew!*
	if ( ( bInAir ||			//!bInProne || -Ztormi: checks the prone in CanDeployBipod()
		 m_bIsBipodDeploying ||
		 m_bIsBipodUnDeploying ||
		 m_bDoBipodDeploy )  )//&& !bCanDeployInDeployZone - Ztormi: no more deployzones
		return false;
	//---END

	// if we are proning we can always deploy
	if (pPlayer->GetFlags() & FL_PRONING)
		return true;

	//*HAJ 020* Ztormi
	//This is the new part
	//Check if we can deploy with tracelines

	float flForward = 50.0f;
	Vector vecHeightTraceline,vecEyeTraceline, vecAiming, vecEndpoint;

	QAngle angles = pPlayer->GetLocalAngles();

	AngleVectors( angles, &vecAiming );

	Vector forward, up;
	pPlayer->EyeVectors( &forward , NULL, &up );

	if( abs( angles[PITCH] ) > 30 )
		return false;

	while( flForward > 20.0f )
	{
		vecEyeTraceline = pPlayer->EyePosition();

		vecHeightTraceline = vecEyeTraceline + ( forward * flForward ); // height trace is pushed a bit forward

		trace_t tr;
		trace_t tr2;

		// Traceline which traces the height ( down )
		// If we are crouching make the traceline shorter
		if (pPlayer->GetFlags() & FL_DUCKING )
		{
			UTIL_TraceLine( vecHeightTraceline + ( -up * 5 ), vecHeightTraceline + ( -up * 15), MASK_OPAQUE, pPlayer, COLLISION_GROUP_NONE, &tr );
		}
		else
			UTIL_TraceLine( vecHeightTraceline + ( -up * 5 ), vecHeightTraceline + ( -up * 35), MASK_OPAQUE, pPlayer, COLLISION_GROUP_NONE, &tr );

		// Upper one traces if there is no wall infront of you
		UTIL_TraceLine( vecEyeTraceline, vecEyeTraceline + (vecAiming * 60), MASK_OPAQUE, pPlayer, COLLISION_GROUP_NONE, &tr2 );	

		// We've found suitable spot if traceline from eyes doesnt hit the wall
		// and the height trace and lowertrace hit
		if ( tr.fraction < 1.0 && tr2.fraction >= 1.0 )
		{
			if ( bUsesTransition )
			{
				float flThisHeight = tr.startpos.z - tr.endpos.z - 15;

				if( flThisHeight < flCurrentHeight || !bValidHeight ) // this, surprisingly, is the way it works
					flCurrentHeight = flThisHeight;

				bValidHeight = true;
			}
			else
			{
				return true;
			}
		}

		flForward -= 5.0f;
	}

	if( bValidHeight )
	{
		m_bInDeployTransition = true;		// Starting transition
		m_flTransitionHeight = flCurrentHeight; //Distance between start and endpoint, 15 is just for adjustment so gun will look like it's on bipod

		if (m_flTransitionHeight < 0 )		// We cant deploy higher than we are
			m_flTransitionHeight = 0;

		m_flTransitionTime = gpGlobals->curtime + MG_VIEW_TRANSITION_TIME;
		m_flTransitionStartTime = gpGlobals->curtime;
		m_flStartviewHeight = pPlayer->GetViewOffset().z;	// Height when we started transition

		return true;
	}
	

	return false;
}
//-----------------------------------------------------------------------------
// *HAJ 020* Ztormi
// Purpose: Performs the deploying height transition
// TODO: Make the actual transition between startheight and targetheight instead of just warping to position
//-----------------------------------------------------------------------------
void CHAJWeaponBase::DeployTransition( void )
{
		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

		if (!pOwner || !m_bInDeployTransition )
			return;

		float m_flElapsedtransitiontime = gpGlobals->curtime - m_flTransitionStartTime;
	
		if ( m_flElapsedtransitiontime > MG_VIEW_TRANSITION_TIME )
			m_flElapsedtransitiontime = MG_VIEW_TRANSITION_TIME;

		Vector m_vecViewHeight = pOwner->GetViewOffset();

		m_vecViewHeight.z = m_flStartviewHeight - m_flTransitionHeight;	//Target viewheight
		pOwner->SetViewOffset( m_vecViewHeight );	// Set it

		//Finished transition
		if ( m_flTransitionTime < gpGlobals->curtime )
			m_bInDeployTransition = false;

#ifndef CLIENT_DLL
		m_bQueuedIronsights = false;
#endif 

}

float CHAJWeaponBase::GetDeployHeight( void )
{
	return m_flStartviewHeight - m_flTransitionHeight;
}
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHAJWeaponBase::HandleBipodDeployment( void )
{
	if ( !m_bDoBipodDeploy )
		return;

	CBaseCombatCharacter *pOwner = GetOwner();
	if ( !pOwner ) return;

	CBasePlayer *player = ToBasePlayer( pOwner );
	if ( !player ) return;

	CHajPlayer *pHajPlayer = ToHajPlayer( player ); // client side only
	
	// trigger the deploy
	if ( !m_bIsBipodDeployed && !m_bIsBipodDeploying )
	{
		SendWeaponAnim( ACT_VM_DEPLOY );

		// Play the player's deploy
		if ( pOwner->IsPlayer() )
			( ( CBasePlayer * )pOwner)->SetAnimation( PLAYER_DEPLOYBIPOD );

		MDLCACHE_CRITICAL_SECTION();
		m_fBipodDeployTime = gpGlobals->curtime + SequenceDuration();
		GetOwner()->SetNextAttack( m_fBipodDeployTime );
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_fBipodDeployTime;

		// set the flag saying we're attempting to deploy
		m_bIsBipodUnDeploying = false;
		m_bIsBipodDeploying = true;
		m_bDoBipodDeploy = true;

		m_flIronsightTime = gpGlobals->curtime;
		m_bIronsighted = true;

		// we set this any time we are using or transitioning a deploy
		pOwner->AddFlag( FL_USINGBIPOD );
		
		// set speed and movetype to lock the player
		player->SetMaxSpeed( 0.0f );
		player->SetMoveType( MOVETYPE_NONE );
		player->SetAbsVelocity( Vector( 0, 0, 0 ) ); // stop 'em dead (should stop weapon sway)

		#ifdef CLIENT_DLL
		pHajPlayer->SaveViewAngles();
		#endif
	}

	// trigger the undeploy
	else if ( m_bIsBipodDeployed && !m_bIsBipodUnDeploying )
	{
		m_bInReload = false; // cancel a reload if one is happening
		
		SendWeaponAnim( ACT_VM_UNDEPLOY );

		// Play the player's deploy
		if ( pOwner->IsPlayer() )
			( ( CBasePlayer * )pOwner)->SetAnimation( PLAYER_UNDEPLOYBIPOD );
	
		MDLCACHE_CRITICAL_SECTION();
		m_fBipodUnDeployTime = gpGlobals->curtime + SequenceDuration();
		GetOwner()->SetNextAttack( m_fBipodUnDeployTime );
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_fBipodUnDeployTime;

		//*HAJ 020* Ztormi
		if ( !(player->GetFlags() & FL_PRONING) ) // if we are in prone dont touch the viewoffset
		{
			Vector vecViewOffset = pOwner->GetViewOffset();

			vecViewOffset.z = m_flStartviewHeight;
			pOwner->SetViewOffset( vecViewOffset );
		}
		//---end	

		// set the flag saying we're attempting to deploy
		m_bIsBipodDeploying = false;
		m_bIsBipodUnDeploying = true;
		m_bDoBipodDeploy = true;

		m_flIronsightTime = gpGlobals->curtime;
		m_bIronsighted = false;

		// we set this any time we are using or transitioning a deploy
		// set the flag
		pOwner->RemoveFlag( FL_USINGBIPOD );

		// set speed and movetype to lock the player
		player->SetMaxSpeed( HAJ_NORM_SPEED / 1.5 ); // slightly slower than normal
		player->SetMoveType( MOVETYPE_WALK );

#ifdef GAME_DLL
		player->SetFOV( this, 0, 0.5f );
#endif
	}

	// middle of a deploy sequence
	if ( !m_bIsBipodDeployed && m_bIsBipodDeploying )
	{
		if ( gpGlobals->curtime >= m_fBipodDeployTime )
		{
			m_bIsBipodDeployed = true;
			m_bIsBipodDeploying = false;
			m_bDoBipodDeploy = false;
			
			// set the flag
			pOwner->AddFlag( FL_USINGBIPOD );
			
			// set the worldmodel
			//SetModel( GetWorldModelDeployed() );

			// set speed and movetype to lock the player
			player->SetMaxSpeed( 0.0f );
			player->SetMoveType( MOVETYPE_NONE );

			#ifndef CLIENT_DLL
			player->SetFOV( this, 75, 0.75f );
			player->ShowCrosshair( false );
			#else
			//pHajPlayer->SaveViewAngles();
			//DevMsg( "Saving view angles...CLIENT\n");
			#endif
		}
	}

	// middle of an undeploy sequence
	else if ( m_bIsBipodDeployed && m_bIsBipodUnDeploying )
	{
		if ( gpGlobals->curtime >= m_fBipodUnDeployTime )
		{
			m_bIsBipodDeployed = false;
			m_bIsBipodUnDeploying = false;
			m_bDoBipodDeploy = false;

			//*HAJ 020* Ztormi
			//Deploy transitions reseting
			m_bInDeployTransition = false;

			// set the worldmodel
			//SetModel( GetWorldModel() );

			// set speed and movetype to lock the player
			if( !pHajPlayer->IsSprinting() )
			{
				player->SetMaxSpeed( HAJ_NORM_SPEED );
				player->SetMoveType( MOVETYPE_WALK );
			}

#ifndef CLIENT_DLL
			player->ShowCrosshair ( true );
#endif
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Allows the weapon to choose proper weapon idle animation
//-----------------------------------------------------------------------------
void CHAJWeaponBase::WeaponIdle( void )
{
	if( m_bFakeHolstered || m_bHolstered )
		return;

	CHajPlayer *pPlayer = ToHajPlayer( GetOwner() );

	if ( HasWeaponIdleTimeElapsed() ) 
	{
		if( IsIronsighted() )
			SendWeaponAnim( ACT_VM_IRONSIGHT_IDLE );
		else if( m_bLowered && pPlayer->IsSprinting() )
			SendWeaponAnim( ACT_VM_IDLE_LOWERED );	
		else if ( m_bIsBipodDeployed )
			SendWeaponAnim( ACT_VM_IDLE_DEPLOYED );
		else
			SendWeaponAnim( ACT_VM_IDLE );

		m_flTimeWeaponIdle = gpGlobals->curtime + SequenceDuration();
	}
}

bool CHAJWeaponBase::IsLowered( void )
{
	return m_bLowered;
}

//-----------------------------------------------------------------------------
// Purpose: Remove all flags that would indicate the bipod is down
//-----------------------------------------------------------------------------
void CHAJWeaponBase::UnlockBipod( void )
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if ( !pOwner ) return;

	CBasePlayer *player = ToBasePlayer( pOwner );
	if ( !player ) return;

	m_bIsBipodDeployed = false;
	m_bIsBipodUnDeploying = false;
	m_bDoBipodDeploy = false;

	// set the flag
	pOwner->RemoveFlag( FL_USINGBIPOD );
			
	// set speed and movetype to lock the player
	player->SetMaxSpeed( HAJ_NORM_SPEED );
	player->SetMoveType( MOVETYPE_WALK );

#ifndef CLIENT_DLL
	player->SetFOV( this, 0, 0.5f );
#endif

}


//-----------------------------------------------------------------------------
// Purpose: Drops the weapon into a lowered pose
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHAJWeaponBase::Lower( void )
{
	//Don't bother if we don't have the animation
	//SelectWeightedSequence( ACT_VM_IDLE_LOWERED ); // IK should take care of this anyway
	if( m_bIronsighted )
		ExitIronsights();

	if( m_bIsBipodDeployed || m_bIsBipodDeploying )
		return false;

	m_bLowered = true;

	if( !m_bInReload && GetWeaponIdleTime() > m_flNextPrimaryAttack )
		SetWeaponIdleTime( m_flNextPrimaryAttack );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Brings the weapon up to the ready position
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHAJWeaponBase::Ready( void )
{
	//Don't bother if we don't have the animation
	//if( !m_bInReload && m_flNextPrimaryAttack >= gpGlobals->curtime )
		//SelectWeightedSequence( ACT_VM_LOWERED_TO_IDLE );

	m_bLowered = false;

	if( gpGlobals->curtime >= m_flNextPrimaryAttack )
		m_flRaiseTime = gpGlobals->curtime + SequenceDuration( ACT_VM_LOWERED_TO_IDLE );
	else
		m_flRaiseTime = m_flNextPrimaryAttack ;

	//if( !m_bInReload && GetWeaponIdleTime() > m_flNextPrimaryAttack )
		//SetWeaponIdleTime( m_flNextPrimaryAttack );
	
	if( !m_bInReload || !IsHolstered() )
		SetWeaponIdleTime( m_flRaiseTime );

	m_flNextPrimaryAttack = m_flRaiseTime;
	m_flNextSecondaryAttack = m_flRaiseTime;

	return true;
}

void CHAJWeaponBase::Drop( const Vector &vecVelocity )
{
#ifdef GAME_DLL
	CBaseCombatCharacter* pOwner = GetOwner();

	if( pOwner )
		SetLastOwner( pOwner );
#endif

	m_bInReload = false;
	m_bFakeHolstered = false;
	m_bIronsighted = false;

	if( HasBipod() && ( m_bIsBipodDeployed || m_bDoBipodDeploy || m_bIsBipodDeploying || m_bIsBipodUnDeploying ) )
	{
		m_bDoBipodDeploy = false;
		m_bIsBipodDeployed = false;
		m_bIsBipodDeploying = false;
		m_bIsBipodUnDeploying = false;
		m_fBipodDeployTime = 0.0f;
		m_fBipodUnDeployTime = 0.0f;
	}

#ifndef CLIENT_DLL
	CHajPlayer *pPlayer = ToHajPlayer( GetOwner() );

	if( pPlayer )
		pPlayer->ShowCrosshair( true );
#endif

	BaseClass::Drop( vecVelocity );
}

void CHAJWeaponBase::Equip( CBaseCombatCharacter *pOwner )
{
	BaseClass::Equip( pOwner );
}

void CHAJWeaponBase::DoMuzzleFlash()
{
	if( !m_bInReload )
	{
		BaseClass::DoMuzzleFlash();
	}
}

void CHAJWeaponBase::GetViewModelOffset( Vector& pos, QAngle& ang )
{
	Vector vMoveOffset = pos = GetMoveOffset();

	/*
	Delta is basically the percentage (decimal)
	*/
	float delta = clamp( ( ( gpGlobals->curtime - m_flIronsightTime ) / GetIronsightTime() ), 0.0f, 1.0f );
	float exp = ( IsIronsighted() ) ? delta : 1.0f - delta; //reverse interpolation

	if( exp == 0.0f ) //fully not ironsighted; save performance
		return;

	exp = -pow( (exp-1), 2 ) + 1;


	pos += ( GetIronsightPosition() - vMoveOffset ) * exp;
	ang = ( GetIronsightAngles() * exp );

}

Vector CHAJWeaponBase::GetMoveOffset( void ) const
{
	if( GetOwner() && GetOwner()->IsPlayer() )
	{
		CHajPlayer *pPlayer = ToHajPlayer( GetOwner() );

		if( pPlayer->GetFlags() & FL_PRONING )
			return Vector( -1.2, -1.5, 0.1 );		

		float flViewZ = pPlayer->GetViewOffset().z;
		float flDiff = 66 - flViewZ;

		if( flDiff > 0.0f )
			return Vector( 0, 0, -( flDiff / 24 ) );
	}

	return Vector( 0, 0, 0 );
}

bool CHAJWeaponBase::HasFinishedHolstering( void )
{
	if( gpGlobals->curtime >= m_flHolsterTime || m_flHolsterTime < 0 )
		return true;

	return false;
}

bool CHAJWeaponBase::HolsterTimeThink( void )
{
	if( m_bAwaitingHolster )
	{
		if( m_bFakeHolstered )
		{
			m_bAwaitingHolster = false;
			return false;
		}

		if( HasFinishedHolstering() )
		{
			DefaultDeploy( (char*)GetViewModel(), (char*)GetWorldModel(), GetDrawActivity(), (char*)GetAnimPrefix() );
			m_bAwaitingHolster = false;

			return false;
		}

		return true;
	}

	return false;
}

#ifdef CLIENT_DLL
int CHAJWeaponBase::DrawModel( int flags )
{
	EnsureCorrectRenderingModel();
	
	// bipod bodygroup
	if( HasBipod() )
	{
		SetBodygroup( FindBodygroupByName("bipod" ), (m_bIsBipodDeployed ? 1 : 0 ) );
	}

	return BaseClass::DrawModel( flags );
}
#endif

float CHAJWeaponBase::GetPlayerSpeedMultiplier( void )
{
	CHajPlayer *pOwner = ToHajPlayer( GetOwner() );

	if( pOwner )
	{
		if( m_bIronsighted && !(pOwner->GetFlags() & (FL_PRONING|FL_DUCKING)) )
		{
			return 0.5f;
		}
	}

	return 1.0f;
}