//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: Player Millsbomb (greande) weapon
//
// $NoKeywords: $
//=======================================================================//
#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
	#include "haj_player_c.h"
	#include "c_te_effect_dispatch.h"
#else
	#include "hl2mp_player.h"
	#include "haj_player.h"
	#include "te_effect_dispatch.h"
	#include "haj_grenade_base.h"
#endif

#include "weapon_ar2.h"
#include "effect_dispatch_data.h"
#include "haj_weapon_base.h"
#include "haj_weapon_grenadebase.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define GRENADE_PAUSED_NO			0
#define GRENADE_PAUSED_PRIMARY		1
#define GRENADE_PAUSED_SECONDARY	2

#define GRENADE_RADIUS	4.0f // inches

#define GRENADE_MAX_THROW_VELOCITY	800.0f
#define GRENADE_MAX_LOB_VELOCITY	500.0f
#define GRENADE_MAX_ROLL_VELOCITY	400.0f

#define GRENADE_MAX_HOLD_TIME		2.0f

#ifndef CLIENT_DLL

acttable_t	CWeaponGrenade::m_acttable[] = 
{
	// attack
	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_GREN_FRAG,		false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_GREN_FRAG,		false },
	{ ACT_MP_ATTACK_PRONE_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_PRONE_GREN_FRAG,	false },

	// aiming down sight, recent attack
	{ ACT_HAJ_IDLE_AIM,					ACT_HAJ_STAND_AIM_GREN_FRAG,			false },
	{ ACT_HAJ_WALK_AIM,					ACT_HAJ_WALK_AIM_GREN_FRAG,				false },
	{ ACT_HAJ_IDLE_CROUCH_AIM,			ACT_HAJ_CROUCH_AIM_GREN_FRAG,			false },
	{ ACT_HAJ_WALK_CROUCH_AIM,			ACT_HAJ_CROUCHWALK_IDLE_GREN_FRAG,		false },
	{ ACT_HAJ_RUN_AIM,					ACT_HAJ_RUN_AIM_GREN_FRAG,				false },

	// movement
	{ ACT_MP_STAND_IDLE,				ACT_HAJ_STAND_AIM_GREN_FRAG,			false },
	{ ACT_MP_RUN,						ACT_HAJ_RUN_AIM_GREN_FRAG,				false },
	{ ACT_MP_SPRINT,					ACT_HAJ_SPRINT_AIM_GREN_FRAG,			false },
	{ ACT_MP_WALK,						ACT_HAJ_WALK_AIM_GREN_FRAG,				false },
	{ ACT_MP_PRONE_IDLE,				ACT_HAJ_PRONE_AIM_GREN_FRAG,			false },
	{ ACT_MP_PRONE_CRAWL,				ACT_HAJ_PRONEWALK_AIM_GREN_FRAG,		false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HAJ_CROUCH_AIM_GREN_FRAG,			false },
	{ ACT_MP_CROUCHWALK,				ACT_HAJ_CROUCHWALK_AIM_GREN_FRAG,		false },
};

IMPLEMENT_ACTTABLE(CWeaponGrenade);

#endif

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponGrenade, DT_WeaponGrenade )

BEGIN_NETWORK_TABLE( CWeaponGrenade, DT_WeaponGrenade )

#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_bRedraw ) ),
	RecvPropBool( RECVINFO( m_fDrawbackFinished ) ),
	RecvPropInt( RECVINFO( m_AttackPaused ) ),
	RecvPropBool( RECVINFO( m_bPullback ) ),
	RecvPropFloat( RECVINFO( m_flAttackStartTime )),
	RecvPropFloat( RECVINFO( m_flAttackHoldTimer )),
	RecvPropFloat( RECVINFO( m_flAttackReleaseTime )),
	RecvPropBool( RECVINFO( m_bUsingGrenadeKey ) ),
#else
	SendPropBool( SENDINFO( m_bRedraw ) ),
	SendPropBool( SENDINFO( m_fDrawbackFinished ) ),
	SendPropInt( SENDINFO( m_AttackPaused ) ),
	SendPropBool( SENDINFO( m_bPullback ) ),
	SendPropFloat( SENDINFO( m_flAttackStartTime )),
	SendPropFloat( SENDINFO( m_flAttackHoldTimer )),
	SendPropFloat( SENDINFO( m_flAttackReleaseTime )),
	SendPropBool( SENDINFO( m_bUsingGrenadeKey ) ),
#endif
	
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponGrenade )
	DEFINE_PRED_FIELD( m_bRedraw, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_fDrawbackFinished, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_AttackPaused, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bPullback, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWeaponGrenade::CWeaponGrenade( void )
{
	m_flAttackStartTime = 0.0f;
	m_flAttackHoldTimer = 0.0f;
	m_flAttackReleaseTime = 0.0f;

#ifndef CLIENT_DLL
	m_bSuicideNade = false;
#endif

	m_bPullback = false;
	m_fDrawbackFinished = true;
	m_bUsingGrenadeKey = false;

	//UseClientSideAnimation();
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGrenade::Precache( void )
{
	BaseClass::Precache();

	PrecacheScriptSound( "WeaponFrag.Throw" );
	PrecacheScriptSound( "WeaponFrag.Roll" );

	m_bRedraw = false;
}

bool CWeaponGrenade::IsPrimed( void )
{
	return ( m_AttackPaused != 0 && !m_bInReload );
}

bool CWeaponGrenade::Lower( void )
{
	if( IsPrimed() )
	{
		m_bLowered = true;
		return true;
	}

	return BaseClass::Lower();
}

bool CWeaponGrenade::Ready( void )
{
	if( IsPrimed() )
	{
		m_bLowered = false;
		return true;
	}

	return BaseClass::Ready();
}

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponGrenade::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	bool fThrewGrenade = false;

	switch( pEvent->event )
	{
		case EVENT_WEAPON_SEQUENCE_FINISHED:
			m_fDrawbackFinished = true;
			m_bInReload = false;
			break;

		case EVENT_WEAPON_THROW:
			ThrowGrenade( pOwner );
			DecrementAmmo( pOwner );
			fThrewGrenade = true;
			break;

		case EVENT_WEAPON_THROW2:
			RollGrenade( pOwner );
			DecrementAmmo( pOwner );
			fThrewGrenade = true;
			break;

		case EVENT_WEAPON_THROW3:
			LobGrenade( pOwner );
			DecrementAmmo( pOwner );
			fThrewGrenade = true;
			break;

		default:
			BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
			break;
	}

#define RETHROW_DELAY	1.0
	if( fThrewGrenade )
	{
		m_flNextPrimaryAttack	= gpGlobals->curtime + RETHROW_DELAY;
		m_flNextSecondaryAttack	= gpGlobals->curtime + RETHROW_DELAY;
		m_flTimeWeaponIdle = FLT_MAX; //NOTE: This is set once the animation has finished up!
	}
}

#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponGrenade::Deploy( void )
{
	CHajPlayer *pPlayer = ToHajPlayer(GetOwner());

	if( pPlayer )
		m_bUsingGrenadeKey = (pPlayer->m_nButtons & (IN_GRENADE1|IN_GRENADE2)) ? true : false;
	else
		m_bUsingGrenadeKey = false;

	bool bRet = BaseClass::Deploy();

	if( bRet )
		Reload();

	return bRet;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponGrenade::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

#ifndef CLIENT_DLL
	if( m_bSuicideNade )
		return false;
#endif

	if( IsPrimed() )
	{
		if( pSwitchingTo == NULL ) // done when going on a func_tank or something
		{
			ThrowGrenade( pOwner ); // just throw it if we're not switching weapons
			DecrementAmmo( pOwner );

			m_bPullback = false;
			m_AttackPaused = 0;

			return true;
		}

		return false; // no holstering after priming nade
	}

	// reset vars
	//m_bRedraw = false;
	m_fDrawbackFinished = true;
	m_bPullback = false;
	m_flNextPrimaryAttack = gpGlobals->curtime;
	m_AttackPaused = 0;
	m_flAttackStartTime = 0.0f;
	m_flAttackHoldTimer = 0.0f;
	m_flAttackReleaseTime = 0.0f;

#ifndef CLIENT_DLL
	if( pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0 )
	{
		ToBaseCombatCharacter(pOwner)->Weapon_Drop( this );
		UTIL_Remove(this);
	}
#endif

	if( !HasPrimaryAmmo() || m_bUsingGrenadeKey )
		return true;

	return BaseClass::Holster( pSwitchingTo );
}

void CWeaponGrenade::FakeHolster()
{
	if( !IsPrimed() )
	{
		BaseClass::FakeHolster();
	}
}

void CWeaponGrenade::FakeDeploy()
{
	if( !IsPrimed() )
	{
		BaseClass::FakeDeploy();
	}
}

void CWeaponGrenade::CheckGrenadeKeySwitch( void )
{
	CHajPlayer *pOwner = ToHajPlayer(GetOwner());

	if( pOwner && m_bUsingGrenadeKey )
	{
		pOwner->Weapon_Switch( pOwner->Weapon_GetLast() );
#ifndef CLIENT_DLL
		pOwner->Weapon_SetLast( m_hRestoreLastWeapon.Get() );
#endif
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponGrenade::Reload( void )
{
	CHajPlayer *pOwner = ToHajPlayer(GetOwner());

	if( !pOwner )
		return false;

	if ( !HasPrimaryAmmo() )
	{
		CheckGrenadeKeySwitch();

#ifdef GAME_DLL
		pOwner->Weapon_Drop( this, NULL, NULL );
		UTIL_Remove( this );
#endif
		pOwner->SwitchToNextBestWeapon( NULL );

		return false;
	}

	if ( ( m_bRedraw ) && ( m_flNextPrimaryAttack <= gpGlobals->curtime ) && ( m_flNextSecondaryAttack <= gpGlobals->curtime ) )
	{
		//Redraw the weapon
		if( !m_bUsingGrenadeKey )
			SendWeaponAnim( ACT_VM_DRAW );

		//Update our times
		m_flNextPrimaryAttack	= gpGlobals->curtime + 0.2;
		m_flNextSecondaryAttack	= gpGlobals->curtime + 0.2;
		m_flTimeWeaponIdle = gpGlobals->curtime + 0.2;

		m_flAttackStartTime = 0.0f;
		m_flAttackHoldTimer = 0.0f;
		m_flAttackReleaseTime = 0.0f;

		//Mark this as done
		m_bRedraw = false;
		m_bPullback = false;
		m_bInReload = false;
		m_AttackPaused = 0;

		CheckGrenadeKeySwitch();
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if the weapon currently has ammo or doesn't need ammo
// Output :
//-----------------------------------------------------------------------------
bool CWeaponGrenade::HasPrimaryAmmo( void )
{
	return CWeaponHL2MPBase::HasPrimaryAmmo();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGrenade::SecondaryAttack( void )
{
	if ( m_bRedraw || m_AttackPaused > 0 || m_bPullback || !HasPrimaryAmmo() )
		return;

	CBaseCombatCharacter *pOwner  = GetOwner();

	if ( pOwner == NULL )
		return;

	CBasePlayer *pPlayer = ToBasePlayer( pOwner );
	
	if ( pPlayer == NULL )
		return;

	// record when the attack began
	m_flAttackStartTime = gpGlobals->curtime;

	// Note that this is a secondary attack and prepare the grenade attack to pause.
	m_AttackPaused = GRENADE_PAUSED_SECONDARY;
	SendWeaponAnim( ACT_VM_PULLBACK_LOW );

	// Don't let weapon idle interfere in the middle of a throw!
	m_flTimeWeaponIdle = FLT_MAX;
	m_flNextSecondaryAttack	= FLT_MAX;

	m_bPullback = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGrenade::PrimaryAttack( void )
{
	if ( m_bRedraw || m_AttackPaused > 0 || m_bPullback )
		return;

	if ( !HasPrimaryAmmo() )
		return;

	CBaseCombatCharacter *pOwner  = GetOwner();

	if ( pOwner == NULL )
		return;

	CBasePlayer *pPlayer = ToBasePlayer( pOwner );
	
	if ( pPlayer == NULL || ( pPlayer->GetFlags() & FL_ATCONTROLS ) )
		return;

	// record when the attack began
	m_flAttackStartTime = gpGlobals->curtime;

	// Note that this is a primary attack and prepare the grenade attack to pause.
	m_AttackPaused = GRENADE_PAUSED_PRIMARY;
	SendWeaponAnim( ACT_VM_PULLBACK_HIGH );
	
	// Put both of these off indefinitely. We do not know how long
	// the player will hold the grenade.
	m_flTimeWeaponIdle = FLT_MAX;
	m_flNextPrimaryAttack = FLT_MAX;

	m_bPullback = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOwner - 
//-----------------------------------------------------------------------------
void CWeaponGrenade::DecrementAmmo( CBaseCombatCharacter *pOwner )
{
	pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGrenade::ItemPostFrame( void )
{
	CHajPlayer *pOwner = ToHajPlayer( GetOwner() );

	if( m_fDrawbackFinished )
	{

		if (pOwner)
		{
			switch( m_AttackPaused )
			{
			case GRENADE_PAUSED_PRIMARY:
				if( !(pOwner->m_nButtons & (IN_ATTACK|IN_GRENADE1|IN_GRENADE2)) )
				{
					m_flAttackReleaseTime = gpGlobals->curtime; // record the time the throw was made
					//SetGrenadeThrowTimer ( 0.0f );	// reset - we'll use the local copy for force calculations.
					m_AttackPaused = 0;

					SendWeaponAnim( ACT_VM_THROW );
					
					// player "shoot" animation
					pOwner->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );

					m_fDrawbackFinished = false;
				}
				else
				{
					// primary attack and player is still holding down the attack button
					m_flAttackHoldTimer = gpGlobals->curtime - m_flAttackStartTime;

					if ( CanCookGrenade() && m_flAttackHoldTimer > GetFuseTimer() )
						ExplodeGrenadeInHand( pOwner );
					
					// store the hold time in the baseweapon variabl
					//SetGrenadeThrowTimer( m_flAttackHoldTimer );
				}
				break;

			case GRENADE_PAUSED_SECONDARY:
				if( !(pOwner->m_nButtons & IN_ATTACK2) )
				{
					m_flAttackReleaseTime = gpGlobals->curtime; // record the time the throw was made
					//SetGrenadeThrowTimer ( 0.0f );	// reset - we'll use the local copy for force calculations.

					m_AttackPaused = 0;

					//See if we're ducking
					if ( pOwner->m_nButtons & IN_DUCK )
					{
						//Send the weapon animation
						SendWeaponAnim( ACT_VM_SECONDARYATTACK );
					}
					else
					{
						//Send the weapon animation
						SendWeaponAnim( ACT_VM_HAULBACK );
					}

					// player "shoot" animation
					pOwner->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );

					m_fDrawbackFinished = false;
				}
				else
				{
					// primary attack and player is still holding down the attack button
					m_flAttackHoldTimer = gpGlobals->curtime - m_flAttackStartTime;

					if ( CanCookGrenade() && m_flAttackHoldTimer > GetFuseTimer() )
						ExplodeGrenadeInHand( pOwner );
					
					// store the hold time in the baseweapon variabl
					//SetGrenadeThrowTimer( m_flAttackHoldTimer );
				}
				break;

			default:
				m_AttackPaused = 0;
				break;
			}
		}
	}

	if( pOwner && m_bUsingGrenadeKey && ( pOwner->m_nButtons & (IN_GRENADE1|IN_GRENADE2) || !IsPrimed() ) )
		pOwner->m_nButtons |= IN_ATTACK; // simulate +attack

	BaseClass::ItemPostFrame();

	if ( m_bRedraw )
	{
		if ( IsViewModelSequenceFinished() )
		{
			Reload();
		}
	}
}

	// check a throw from vecSrc.  If not valid, move the position back along the line to vecEye
void CWeaponGrenade::CheckThrowPosition( CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc )
{
	trace_t tr;

	UTIL_TraceHull( vecEye, vecSrc, -Vector(GRENADE_RADIUS+2,GRENADE_RADIUS+2,GRENADE_RADIUS+2), Vector(GRENADE_RADIUS+2,GRENADE_RADIUS+2,GRENADE_RADIUS+2), 
		pPlayer->PhysicsSolidMaskForEntity(), pPlayer, pPlayer->GetCollisionGroup(), &tr );
	
	if ( tr.DidHit() )
	{
		vecSrc = tr.endpos;
	}
}


#ifndef CLIENT_DLL
CHAJGrenade* CWeaponGrenade::CreateGrenade( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer )
{
	// Don't set the owner here, or the player can't interact with grenades he's thrown
	CHAJGrenade *pGrenade = (CHAJGrenade *)CBaseEntity::Create( "npc_grenade_mills", position, angles, pOwner );
	
	pGrenade->SetTimer( timer, timer - 1.5f );
	pGrenade->SetVelocity( velocity, angVelocity );
	pGrenade->SetThrower( ToBaseCombatCharacter( pOwner ) );
	pGrenade->m_takedamage = DAMAGE_YES;

	return pGrenade;
}
#endif

float CWeaponGrenade::GetDamageRadius( void )
{
	return 250.0f;
}

float CWeaponGrenade::GetFuseTimer( void )
{
	return 4.5f;
}

void CWeaponGrenade::ExplodeGrenadeInHand( CBasePlayer *pPlayer )
{
#ifndef CLIENT_DLL
	if( m_bSuicideNade )
		return;

	if( !pPlayer )
		return;

	Vector	vecEye = pPlayer->EyePosition();
	Vector	vForward, vRight;

	pPlayer->EyeVectors( &vForward, &vRight, NULL );
	Vector vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f;

	CHAJGrenade *pGrenade = CreateGrenade( vecSrc, vec3_angle, Vector( 0, 0, 0 ), AngularImpulse(0, 0, 0), pPlayer, 0 );

	if ( pGrenade )
	{
		pGrenade->SetDamage( GetHL2MPWpnData().m_iPlayerDamage );
		pGrenade->SetDamageRadius( GetDamageRadius() );
		pGrenade->Detonate();
	}

	m_bSuicideNade = true;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Throw a grenade
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeaponGrenade::ThrowGrenade( CBasePlayer *pPlayer )
{
#ifndef CLIENT_DLL
	Vector	vecEye = pPlayer->EyePosition();
	Vector	vForward, vRight;

	pPlayer->EyeVectors( &vForward, &vRight, NULL );
	Vector vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f;
	CheckThrowPosition( pPlayer, vecEye, vecSrc );
//	vForward[0] += 0.1f;
	vForward[2] += 0.1f;

	Vector vecThrow;
	pPlayer->GetVelocity( &vecThrow, NULL );
	vecThrow += ( vForward * GetThrowVelocity() );

	CHAJGrenade *pGrenade = CreateGrenade( vecSrc, vec3_angle, vecThrow, AngularImpulse(600,random->RandomInt(-1200,1200),0), pPlayer, clamp( GetFuseTimer() - m_flAttackHoldTimer, 0, 5 ) );
	if ( pGrenade )
	{
		if ( pPlayer && pPlayer->m_lifeState != LIFE_ALIVE )
		{
			pPlayer->GetVelocity( &vecThrow, NULL );

			IPhysicsObject *pPhysicsObject = pGrenade->VPhysicsGetObject();
			if ( pPhysicsObject )
			{
				pPhysicsObject->SetVelocity( &vecThrow, NULL );
			}
		}
		
		pGrenade->SetDamage( GetHL2MPWpnData().m_iPlayerDamage );
		pGrenade->SetDamageRadius( GetDamageRadius() );
	}
#endif
	
	m_flAttackHoldTimer = 0.0f;
	SetGrenadeThrowTimer( 0.0f );

	WeaponSound( SINGLE );
	
	m_bRedraw = true;
	m_bInReload = true;
}

float CWeaponGrenade::GetThrowVelocity()
{
	return GRENADE_MAX_THROW_VELOCITY * GetThrowPowerMultiplier();
}

float CWeaponGrenade::GetLobVelocity()
{
	return GRENADE_MAX_LOB_VELOCITY * GetThrowPowerMultiplier();
}

//-----------------------------------------------------------------------------
// Purpose: Underhand "lob" a grenade
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeaponGrenade::LobGrenade( CBasePlayer *pPlayer )
{
#ifndef CLIENT_DLL
	Vector	vecEye = pPlayer->EyePosition();
	Vector	vForward, vRight;

	pPlayer->EyeVectors( &vForward, &vRight, NULL );
	Vector vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f + Vector( 0, 0, -8 );
	CheckThrowPosition( pPlayer, vecEye, vecSrc );
	
	Vector vecThrow;
	pPlayer->GetVelocity( &vecThrow, NULL );
	vecThrow += ( vForward * GetLobVelocity() + Vector( 0, 0, 50 ) );

	CHAJGrenade *pGrenade = CreateGrenade( vecSrc, vec3_angle, vecThrow, AngularImpulse(600,random->RandomInt(-600,600),0), pPlayer, clamp( GetFuseTimer() - m_flAttackHoldTimer, 0, 5 ) );
	
	if ( pGrenade )
	{
		pGrenade->SetDamage( GetHL2MPWpnData().m_iPlayerDamage );
		pGrenade->SetDamageRadius( GetDamageRadius() );
	}
#endif

	m_flAttackHoldTimer = 0.0f;
	SetGrenadeThrowTimer( 0.0f );

	WeaponSound( WPN_DOUBLE );

	// player "shoot" animation
	ToHajPlayer(pPlayer)->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );

	m_bRedraw = true;
	m_bInReload = true;
}

//-----------------------------------------------------------------------------
// Purpose: Underhand roll a grenade on the floor
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeaponGrenade::RollGrenade( CBasePlayer *pPlayer )
{
#ifndef CLIENT_DLL
	// BUGBUG: Hardcoded grenade width of 4 - better not change the model :)
	Vector vecSrc;
	pPlayer->CollisionProp()->NormalizedToWorldSpace( Vector( 0.5f, 0.5f, 0.0f ), &vecSrc );
	vecSrc.z += GRENADE_RADIUS;

	Vector vecFacing = pPlayer->BodyDirection2D( );
	// no up/down direction
	vecFacing.z = 0;
	VectorNormalize( vecFacing );
	trace_t tr;
	UTIL_TraceLine( vecSrc, vecSrc - Vector(0,0,16), MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_NONE, &tr );
	if ( tr.fraction != 1.0 )
	{
		// compute forward vec parallel to floor plane and roll grenade along that
		Vector tangent;
		CrossProduct( vecFacing, tr.plane.normal, tangent );
		CrossProduct( tr.plane.normal, tangent, vecFacing );
	}
	vecSrc += (vecFacing * 18.0);
	CheckThrowPosition( pPlayer, pPlayer->WorldSpaceCenter(), vecSrc );

	Vector vecThrow;
	pPlayer->GetVelocity( &vecThrow, NULL );
	vecThrow += ( vecFacing * GRENADE_MAX_ROLL_VELOCITY );
	// put it on its side
	QAngle orientation(0,pPlayer->GetLocalAngles().y,-90);
	// roll it
	AngularImpulse rotSpeed(0,0,720);

	CHAJGrenade *pGrenade = CreateGrenade( vecSrc, orientation, vecThrow, rotSpeed, pPlayer, clamp( GetFuseTimer() - m_flAttackHoldTimer, 0, 5 ) );
	if ( pGrenade )
	{
		pGrenade->SetDamage( GetHL2MPWpnData().m_iPlayerDamage );
		pGrenade->SetDamageRadius( GetDamageRadius() );
	}

#endif

	m_flAttackHoldTimer = 0.0f;
	SetGrenadeThrowTimer( 0.0f );

	WeaponSound( SPECIAL1 );

	// player "shoot" animation
	ToHajPlayer(pPlayer)->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );

	m_bRedraw = true;
	m_bInReload = true;
}


//-----------------------------------------------------------------------------
// Purpose: Drop a grenade
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeaponGrenade::DropGrenade( CBasePlayer *pPlayer )
{
#ifndef CLIENT_DLL
	if( g_fGameOver )
		return;

	// BUGBUG: Hardcoded grenade width of 4 - better not change the model :)
	Vector vecSrc;
	pPlayer->CollisionProp()->NormalizedToWorldSpace( Vector( 0.5f, 0.5f, 0.0f ), &vecSrc );
	vecSrc.z += GRENADE_RADIUS;

	Vector vecFacing = pPlayer->BodyDirection2D( );
	// no up/down direction
	vecFacing.z = 0;
	VectorNormalize( vecFacing );
	trace_t tr;
	UTIL_TraceLine( vecSrc, vecSrc - Vector(0,0,16), MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_NONE, &tr );
	if ( tr.fraction != 1.0 )
	{
		// compute forward vec parallel to floor plane and roll grenade along that
		Vector tangent;
		CrossProduct( vecFacing, tr.plane.normal, tangent );
		CrossProduct( tr.plane.normal, tangent, vecFacing );
	}
	vecSrc += (vecFacing * 18.0);
	CheckThrowPosition( pPlayer, pPlayer->WorldSpaceCenter(), vecSrc );

	Vector vecThrow;
	pPlayer->GetVelocity( &vecThrow, NULL );
	vecThrow += ( vecFacing * ( GRENADE_MAX_ROLL_VELOCITY / 3 ) );
	// put it on its side
	QAngle orientation(0,pPlayer->GetLocalAngles().y,-90);
	// roll it
	AngularImpulse rotSpeed(0,0,720);

	CHAJGrenade *pGrenade = CreateGrenade( vecSrc, orientation, vecThrow, rotSpeed, pPlayer, clamp( GetFuseTimer() - m_flAttackHoldTimer, 0, 5 ) );
	if ( pGrenade )
	{
		pGrenade->SetDamage( GetHL2MPWpnData().m_iPlayerDamage );
		pGrenade->SetDamageRadius( GetDamageRadius() );
	}

#endif

	m_flAttackHoldTimer = 0.0f;
	SetGrenadeThrowTimer( 0.0f );

	WeaponSound( SPECIAL1 );

	// player "shoot" animation
	ToHajPlayer(pPlayer)->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );

	m_bRedraw = true;
}

