//========= Copyright � 2007, Ham and Jam. ==============================//
// Purpose: Satchel detontation charge
// Note:	
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "gamerules.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "engine/IEngineSound.h"

#if defined( CLIENT_DLL )
	#include "c_hl2mp_player.h"
	#include "haj_player_c.h"
	#include "prediction.h"
#else
	#include "hl2mp_player.h"
	#include "haj_player.h"
	#include "grenade_satchel.h"
	#include "haj_grenade_tnt.h"
	#include "entitylist.h"
	#include "eventqueue.h"
#endif

#include "haj_gamerules.h"
#include "haj_weapon_satchelcharge.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	SATCHEL_PRIMARY_VOLUME		450

IMPLEMENT_NETWORKCLASS_ALIASED( Weapon_Satchel, DT_Weapon_Satchel )

BEGIN_NETWORK_TABLE( CWeapon_Satchel, DT_Weapon_Satchel )

#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_tSlamState ) ),
	RecvPropBool( RECVINFO( m_bDetonatorArmed ) ),
	RecvPropBool( RECVINFO( m_bNeedDetonatorDraw ) ),
	RecvPropBool( RECVINFO( m_bNeedDetonatorHolster ) ),
	RecvPropBool( RECVINFO( m_bNeedReload ) ),
	RecvPropBool( RECVINFO( m_bClearReload ) ),
	RecvPropBool( RECVINFO( m_bThrowSatchel ) ),
	RecvPropBool( RECVINFO( m_bAttachSatchel ) ),
	RecvPropBool( RECVINFO( m_bAttachTripmine ) ),
#else
	SendPropInt( SENDINFO( m_tSlamState ) ),
	SendPropBool( SENDINFO( m_bDetonatorArmed ) ),
	SendPropBool( SENDINFO( m_bNeedDetonatorDraw ) ),
	SendPropBool( SENDINFO( m_bNeedDetonatorHolster ) ),
	SendPropBool( SENDINFO( m_bNeedReload ) ),
	SendPropBool( SENDINFO( m_bClearReload ) ),
	SendPropBool( SENDINFO( m_bThrowSatchel ) ),
	SendPropBool( SENDINFO( m_bAttachSatchel ) ),
	SendPropBool( SENDINFO( m_bAttachTripmine ) ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA( CWeapon_Satchel )
	DEFINE_PRED_FIELD( m_tSlamState, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bDetonatorArmed, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bNeedDetonatorDraw, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bNeedDetonatorHolster, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bNeedReload, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bClearReload, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bThrowSatchel, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bAttachSatchel, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bAttachTripmine, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()

#endif

LINK_ENTITY_TO_CLASS( weapon_satchel, CWeapon_Satchel );
PRECACHE_WEAPON_REGISTER( weapon_satchel );

#ifndef CLIENT_DLL

BEGIN_DATADESC( CWeapon_Satchel )

	DEFINE_FIELD( m_tSlamState, FIELD_INTEGER ),
	DEFINE_FIELD( m_bDetonatorArmed, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bNeedDetonatorDraw, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bNeedDetonatorHolster, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bNeedReload, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bClearReload, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bThrowSatchel, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bAttachSatchel, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bAttachTripmine, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flWallSwitchTime, FIELD_TIME ),

	// Function Pointers
	DEFINE_FUNCTION( SatchelTouch ),

END_DATADESC()

acttable_t	CWeapon_Satchel::m_acttable[] = 
{
	// attack
	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_TNT,			false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_TNT,			false },
	{ ACT_MP_ATTACK_PRONE_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_PRONE_TNT,	false },

	// aiming down sight, recent attack
	{ ACT_HAJ_IDLE_AIM,					ACT_HAJ_STAND_AIM_TNT,				false },
	{ ACT_HAJ_WALK_AIM,					ACT_HAJ_WALK_AIM_TNT,				false },
	{ ACT_HAJ_IDLE_CROUCH_AIM,			ACT_HAJ_CROUCH_AIM_TNT,				false },
	{ ACT_HAJ_WALK_CROUCH_AIM,			ACT_HAJ_CROUCHWALK_IDLE_TNT,		false },
	{ ACT_HAJ_RUN_AIM,					ACT_HAJ_RUN_AIM_TNT,				false },

	// movement
	{ ACT_MP_STAND_IDLE,				ACT_HAJ_STAND_IDLE_TNT,				false },
	{ ACT_MP_RUN,						ACT_HAJ_RUN_IDLE_TNT,				false },
	{ ACT_MP_SPRINT,					ACT_HAJ_SPRINT_IDLE_TNT,			false },
	{ ACT_MP_WALK,						ACT_HAJ_WALK_IDLE_TNT,				false },
	{ ACT_MP_PRONE_IDLE,				ACT_HAJ_PRONE_AIM_TNT,				false },
	{ ACT_MP_PRONE_CRAWL,				ACT_HAJ_PRONEWALK_IDLE_TNT,			false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HAJ_CROUCH_IDLE_TNT,			false },
	{ ACT_MP_CROUCHWALK,				ACT_HAJ_CROUCHWALK_IDLE_TNT,		false },
};

IMPLEMENT_ACTTABLE( CWeapon_Satchel );
#endif


void CWeapon_Satchel::Spawn( )
{
	BaseClass::Spawn();

	Precache( );

	FallInit();// get ready to fall down

	m_tSlamState		= (int)SATCHEL_THROW;
	m_flWallSwitchTime	= 0;

	// Give 1 piece of default ammo when first picked up
	m_iClip2 = 1;
}

void CWeapon_Satchel::Precache( void )
{
	BaseClass::Precache();

#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "npc_grenade_mills" );
	UTIL_PrecacheOther( "npc_satchel" );
#endif

	PrecacheScriptSound( "Weapon_SLAM.TripMineMode" );
	PrecacheScriptSound( "Weapon_SLAM.SatchelDetonate" );
	PrecacheScriptSound( "Weapon_SLAM.SatchelThrow" );
}

//------------------------------------------------------------------------------
// Purpose : Override to use slam's pickup touch function
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CWeapon_Satchel::SetPickupTouch( void )
{
	SetTouch(&CWeapon_Satchel::SatchelTouch);
}

//-----------------------------------------------------------------------------
// Purpose: Override so give correct ammo
// Input  : pOther - the entity that touched me
// Output :
//-----------------------------------------------------------------------------
void CWeapon_Satchel::SatchelTouch( CBaseEntity *pOther )
{
	// Can I even pick stuff up?
	if ( pOther->IsEFlagSet( EFL_NO_WEAPON_PICKUP ) )
		return;

	// ---------------------------------------------------
	//  First give weapon to touching entity if allowed
	// ---------------------------------------------------
	BaseClass::DefaultTouch(pOther);
}

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
bool CWeapon_Satchel::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	SetThink(NULL);
	return BaseClass::Holster(pSwitchingTo);
}

//-----------------------------------------------------------------------------
// Purpose: SLAM has no reload, but must call weapon idle to update state
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CWeapon_Satchel::Reload( void )
{
	WeaponIdle( );
	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_Satchel::PrimaryAttack( void )
{
	CBaseCombatCharacter *pOwner  = GetOwner();
	if (!pOwner)
	{ 
		return;
	}

	if (pOwner->GetAmmoCount(m_iSecondaryAmmoType) <= 0)
	{
		return;
	}

	if (CanAttachSatchel( true ))
	{
		StartTripmineAttach();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Secondary attack switches between satchel charge and tripmine mode
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_Satchel::SecondaryAttack( void )
{
	CBaseCombatCharacter *pOwner  = GetOwner();
	if (!pOwner)
	{
		return;
	}

	if (m_bDetonatorArmed)
	{
		//StartSatchelDetonate();
	}
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_Satchel::SatchelDetonate()
{
#ifndef CLIENT_DLL
	CBaseEntity *pEntity = NULL;

	while ((pEntity = gEntList.FindEntityByClassname( pEntity, "npc_satchel" )) != NULL)
	{
		CSatchelCharge *pSatchel = dynamic_cast<CSatchelCharge *>(pEntity);
		if (pSatchel->m_bIsLive && pSatchel->GetThrower() && GetOwner() && pSatchel->GetThrower() == GetOwner())
		{
			//pSatchel->Use( GetOwner(), GetOwner(), USE_ON, 0 );
			//variant_t emptyVariant;
			//pSatchel->AcceptInput( "Explode", NULL, NULL, emptyVariant, 5 );
			g_EventQueue.AddEvent( pSatchel, "Explode", 0.20, GetOwner(), GetOwner() );
		}
	}
#endif
	// Play sound for pressing the detonator
	EmitSound( "Weapon_SLAM.SatchelDetonate" );

	m_bDetonatorArmed	= false;
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if there are any undetonated charges in the world
//			that belong to this player
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CWeapon_Satchel::AnyUndetonatedCharges(void)
{
#ifndef CLIENT_DLL
	CBaseEntity *pEntity = NULL;

	while ((pEntity = gEntList.FindEntityByClassname( pEntity, "npc_satchel" )) != NULL)
	{
		CSatchelCharge* pSatchel = dynamic_cast<CSatchelCharge *>(pEntity);
		if (pSatchel->m_bIsLive && pSatchel->GetThrower() && pSatchel->GetThrower() == GetOwner())
		{
			return true;
		}
	}
#endif
	return false;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_Satchel::StartSatchelDetonate()
{

	if ( GetActivity() != ACT_SLAM_DETONATOR_IDLE && GetActivity() != ACT_SLAM_THROW_IDLE )
		 return;
	
	// -----------------------------------------
	//  Play detonate animation
	// -----------------------------------------
	if (m_bNeedReload)
	{
		SendWeaponAnim(ACT_SLAM_DETONATOR_DETONATE);
	}
	else if (m_tSlamState == SATCHEL_ATTACH)
	{
		SendWeaponAnim(ACT_SLAM_STICKWALL_DETONATE);
	}
	else if (m_tSlamState == SATCHEL_THROW)
	{
		SendWeaponAnim(ACT_SLAM_THROW_DETONATE);
	}
	else
	{
		return;
	}
	SatchelDetonate();

	m_flNextPrimaryAttack	= gpGlobals->curtime + SequenceDuration();
	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
}


//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_Satchel::TripmineAttach( void )
{
	CHL2MP_Player *pOwner  = ToHL2MPPlayer( GetOwner() );
	if (!pOwner)
	{
		return;
	}

	m_bAttachTripmine = false;

	Vector vecSrc, vecAiming;

	// Take the eye position and direction
	vecSrc = pOwner->EyePosition();
	
	QAngle angles = pOwner->GetLocalAngles();

	AngleVectors( angles, &vecAiming );

	trace_t tr;

	UTIL_TraceLine( vecSrc, vecSrc + (vecAiming * 256), MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr );
	
	if (tr.fraction < 1.0)
	{
		CBaseEntity *pEntity = tr.m_pEnt;
		if (pEntity && !(pEntity->GetFlags() & FL_CONVEYOR))
		{

#ifndef CLIENT_DLL
			QAngle angles;
			VectorAngles(tr.plane.normal, angles);

			angles.x += 90;

			CBaseEntity *pEnt = CBaseEntity::Create( "npc_grenade_tnt", tr.endpos + tr.plane.normal * 3, angles, NULL );
			CGrenadeTNT *pGrenade = (CGrenadeTNT *)pEnt;
			if ( pGrenade )
			{
				pGrenade->SetMoveType( MOVETYPE_NONE );
				pGrenade->SetTimer( 20.0f, 17.5f );
				pGrenade->SetThrower( ToBaseCombatCharacter( pOwner ) );
				pGrenade->SetParent( pEntity );
			}
#endif
			pOwner->RemoveAmmo( 1, m_iSecondaryAmmoType );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_Satchel::StartTripmineAttach( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if (!pPlayer)
	{
		return;
	}

	Vector vecSrc, vecAiming;

	// Take the eye position and direction
	vecSrc = pPlayer->EyePosition();
	
	QAngle angles = pPlayer->GetLocalAngles();

	AngleVectors( angles, &vecAiming );

	trace_t tr;

	UTIL_TraceLine( vecSrc, vecSrc + (vecAiming * 256), MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr );
	
	if (tr.fraction < 1.0)
	{
		// ALERT( at_console, "hit %f\n", tr.flFraction );

		CBaseEntity *pEntity = tr.m_pEnt;
		if (pEntity && !(pEntity->GetFlags() & FL_CONVEYOR))
		{
			// player "shoot" animation
			ToHajPlayer(pPlayer)->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );

			// -----------------------------------------
			//  Play attach animation
			// -----------------------------------------

			if (m_bDetonatorArmed)
			{
				SendWeaponAnim(ACT_SLAM_STICKWALL_ATTACH);
			}
			else
			{
				SendWeaponAnim(ACT_SLAM_TRIPMINE_ATTACH);
			}

			m_bNeedReload		= true;
			m_bAttachTripmine	= true;
			m_bNeedDetonatorDraw = m_bDetonatorArmed;
		}
		else
		{
			// ALERT( at_console, "no deploy\n" );
		}
	}
	
	m_flNextPrimaryAttack	= gpGlobals->curtime + SequenceDuration();
	m_flNextSecondaryAttack	= gpGlobals->curtime + SequenceDuration();
//	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_Satchel::SatchelThrow( void )
{	
	m_bThrowSatchel = false;

	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

#ifndef CLIENT_DLL

	Vector vecSrc	 = pPlayer->WorldSpaceCenter();
	Vector vecFacing = pPlayer->BodyDirection3D( );
	vecSrc = vecSrc + vecFacing * 18.0;
	// BUGBUG: is this because vecSrc is not from Weapon_ShootPosition()???
	vecSrc.z += 24.0f;

	Vector vecThrow;
	GetOwner()->GetVelocity( &vecThrow, NULL );
	vecThrow += vecFacing * 500;

	// Player may have turned to face a wall during the throw anim in which case
	// we don't want to throw the SLAM into the wall
	if (CanAttachSatchel())
	{
		vecThrow = vecFacing;
		vecSrc   = pPlayer->WorldSpaceCenter() + vecFacing * 5.0;
	}	

	CBaseEntity *pEnt = CBaseEntity::Create( "npc_grenade_tnt", vecSrc, vec3_angle, GetOwner() );
	CGrenadeTNT *pGrenade = (CGrenadeTNT *)pEnt;
	if ( pGrenade )
	{
		pGrenade->SetTimer( 20.0f, 17.5f );
		pGrenade->SetThrower( GetOwner() );
		pGrenade->ApplyAbsVelocityImpulse( vecThrow );
		pGrenade->SetLocalAngularVelocity( QAngle( 0, 100, 0 ) );
	}

	pPlayer->RemoveAmmo( 1, m_iSecondaryAmmoType );

#endif

	ToHajPlayer(pPlayer)->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );

	// Play throw sound
	EmitSound( "Weapon_SLAM.SatchelThrow" );
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_Satchel::StartSatchelThrow( void )
{
	// -----------------------------------------
	//  Play throw animation
	// -----------------------------------------
	if (m_bDetonatorArmed)
	{
		SendWeaponAnim(ACT_SLAM_THROW_THROW);
	}
	else
	{
		SendWeaponAnim(ACT_SLAM_THROW_THROW_ND);
		if (!m_bDetonatorArmed)
		{
			m_bDetonatorArmed		= true;
			m_bNeedDetonatorDraw	= true;
		}
	}
	
	m_bNeedReload		= true;
	m_bThrowSatchel		= true;

	m_flNextPrimaryAttack	= gpGlobals->curtime + SequenceDuration();
	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_Satchel::SatchelAttach( void )
{
#ifndef CLIENT_DLL
	CBaseCombatCharacter *pOwner  = GetOwner();
	if (!pOwner)
	{
		return;
	}

	m_bAttachSatchel = false;

	Vector vecSrc	 = pOwner->Weapon_ShootPosition( );
	Vector vecAiming = pOwner->BodyDirection2D( );

	trace_t tr;

	UTIL_TraceLine( vecSrc, vecSrc + (vecAiming * 128), MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr );
	
	if (tr.fraction < 1.0)
	{
		CBaseEntity *pEntity = tr.m_pEnt;
		if (pEntity && !(pEntity->GetFlags() & FL_CONVEYOR))
		{
			QAngle angles;
			VectorAngles(tr.plane.normal, angles);
			angles.y -= 90;
			angles.z -= 90;
			tr.endpos.z -= 6.0f;

			CBaseEntity *pEnt = CBaseEntity::Create( "npc_grenade_tnt", tr.endpos + tr.plane.normal * 3, angles, GetOwner() );
			CGrenadeTNT *pGrenade = (CGrenadeTNT *)pEnt;
			if ( pGrenade )
			{
				pGrenade->SetMoveType( MOVETYPE_FLY ); // no gravity
				pGrenade->SetTimer( 20.0f, 17.5f );
				pGrenade->SetThrower( GetOwner() );
			}

			pOwner->RemoveAmmo( 1, m_iSecondaryAmmoType );
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_Satchel::StartSatchelAttach( void )
{
#ifndef CLIENT_DLL
	CBaseCombatCharacter *pOwner  = GetOwner();
	if (!pOwner)
	{
		return;
	}

	Vector vecSrc	 = pOwner->Weapon_ShootPosition( );
	Vector vecAiming = pOwner->BodyDirection2D( );

	trace_t tr;

	UTIL_TraceLine( vecSrc, vecSrc + (vecAiming * 128), MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr );
	
	if (tr.fraction < 1.0)
	{
		CBaseEntity *pEntity = tr.m_pEnt;
		if (pEntity && !(pEntity->GetFlags() & FL_CONVEYOR))
		{
			// Only the player fires this way so we can cast
			CHajPlayer *pPlayer = ToHajPlayer( pOwner );

			// player "shoot" animation
			pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );

			// -----------------------------------------
			//  Play attach animation
			// -----------------------------------------
			if (m_bDetonatorArmed)
			{
				SendWeaponAnim(ACT_SLAM_STICKWALL_ATTACH);
			}
			else
			{
				SendWeaponAnim(ACT_SLAM_STICKWALL_ND_ATTACH);
				if (!m_bDetonatorArmed)
				{
					m_bDetonatorArmed		= true;
					m_bNeedDetonatorDraw	= true;
				}
			}
			
			m_bNeedReload		= true;
			m_bAttachSatchel	= true;

			m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_Satchel::SetSatchelState( int newState )
{
	// Set set and set idle time so animation gets updated with state change
	m_tSlamState = newState;
	SetWeaponIdleTime( gpGlobals->curtime );
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_Satchel::SatchelThink( void )
{
	if ( m_flWallSwitchTime > gpGlobals->curtime )
		 return;


	// If not in tripmine mode we need to check to see if we are close to
	// a wall. If we are we go into satchel_attach mode
	CBaseCombatCharacter *pOwner  = GetOwner();

	if ( (pOwner && pOwner->GetAmmoCount(m_iSecondaryAmmoType) > 0))
	{	
		if (CanAttachSatchel())
		{
			if (m_tSlamState == SATCHEL_THROW)
			{
				SetSatchelState(SATCHEL_ATTACHED_READY);
				int iAnim =	m_bDetonatorArmed ? ACT_SLAM_THROW_TO_STICKWALL : ACT_SLAM_THROW_TO_TRIPMINE_ND;
				SendWeaponAnim( iAnim );
				m_flWallSwitchTime = gpGlobals->curtime + SequenceDuration();
				m_bNeedReload = false;
			}
		}
		else
		{
			if (m_tSlamState == SATCHEL_ATTACHED_READY)
			{
				SetSatchelState(SATCHEL_THROW);
				int iAnim =	m_bDetonatorArmed ? ACT_SLAM_STICKWALL_TO_THROW : ACT_SLAM_TRIPMINE_TO_THROW_ND;
				SendWeaponAnim( iAnim );
				m_flWallSwitchTime = gpGlobals->curtime + SequenceDuration();
				m_bNeedReload = false;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CWeapon_Satchel::CanAttachSatchel( bool bAttack )
{
	CHajPlayer *pOwner = ToHajPlayer( GetOwner() );
	CHajGameRules *pGameRules = HajGameRules(); 

	if (!pOwner || !pGameRules)
	{
		return false;
	}

	if( !pGameRules->CanPlayerPlantBomb( ToBasePlayer( GetOwner() ) ) )
	{
		if( bAttack )
			ClientPrint( pOwner, HUD_PRINTCENTER, "#HaJ_CantPlantHere" );

		return false;
	}

	Vector vecSrc, vecAiming;

	// Take the eye position and direction
	vecSrc = pOwner->EyePosition();
	
	QAngle angles = pOwner->GetLocalAngles();

	AngleVectors( angles, &vecAiming );

	trace_t tr;

	Vector	vecEnd = vecSrc + (vecAiming * 42);
	UTIL_TraceLine( vecSrc, vecEnd, MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr );
	
	if (tr.fraction < 1.0)
	{
		// Don't attach to a living creature
		if (tr.m_pEnt)
		{
			CBaseEntity *pEntity = tr.m_pEnt;
			CBaseCombatCharacter *pBCC		= ToBaseCombatCharacter( pEntity );
			if (pBCC)
			{
				return false;
			}
		}
		return true;
	}
	else
	{
		return false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Override so SLAM to so secondary attack when no secondary ammo
//			but satchel is in the world
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_Satchel::ItemPostFrame( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if (!pOwner)
	{
		return;
	}

	SatchelThink();

	if ((pOwner->m_nButtons & IN_ATTACK2) && (m_flNextSecondaryAttack <= gpGlobals->curtime))
	{
		SecondaryAttack();
	}
	else if (!m_bNeedReload && (pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		PrimaryAttack();
	}

	// -----------------------
	//  No buttons down
	// -----------------------
	else 
	{
		WeaponIdle( );
		return;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Switch to next best weapon
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_Satchel::Weapon_Switch( void )
{  
	// Note that we may pick the SLAM again, when we switch
	// weapons, in which case we have to save and restore the 
	// detonator armed state.
	// The SLAMs may be about to blow up, but haven't done so yet
	// and the deploy function will find the undetonated charges
	// and we are armed
	bool saveState = m_bDetonatorArmed;
	CBaseCombatCharacter *pOwner  = GetOwner();
	pOwner->SwitchToNextBestWeapon( pOwner->GetActiveWeapon() );
	if (pOwner->GetActiveWeapon() == this)
	{
		m_bDetonatorArmed = saveState;
	}

#ifndef CLIENT_DLL
	// If not armed and have no ammo
	if (!m_bDetonatorArmed && pOwner->GetAmmoCount(m_iSecondaryAmmoType) <= 0)
	{
		pOwner->ClearActiveWeapon();
	}
#endif

}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_Satchel::WeaponIdle( void )
{
	// Ready to switch animations?
 	if ( HasWeaponIdleTimeElapsed() )
	{
		// Don't allow throw to attach switch unless in idle
		if (m_bClearReload)
		{
			m_bNeedReload  = false;
			m_bClearReload = false;
		}
		CBaseCombatCharacter *pOwner  = GetOwner();
		if (!pOwner)
		{
			return;
		}

		int iAnim = 0;

		if (m_bThrowSatchel)
		{
			SatchelThrow();
			if (m_bDetonatorArmed && !m_bNeedDetonatorDraw)
			{
				iAnim = ACT_SLAM_THROW_THROW2;
			}
			else
			{
				iAnim = ACT_SLAM_THROW_THROW_ND2;
			}
		}
		else if (m_bAttachSatchel)
		{
			SatchelAttach();
			if (m_bDetonatorArmed && !m_bNeedDetonatorDraw)
			{
				iAnim = ACT_SLAM_STICKWALL_ATTACH2;
			}
			else
			{
				iAnim = ACT_SLAM_STICKWALL_ND_ATTACH2;
			}
		}
		else if (m_bAttachTripmine)
		{
			TripmineAttach();
			//SatchelAttach();
			iAnim = m_bNeedDetonatorDraw ? ACT_SLAM_STICKWALL_ATTACH2 : ACT_SLAM_TRIPMINE_ATTACH2;
		}	
		else if ( m_bNeedReload )
		{	
			// If owner had ammo draw the correct SLAM type
			if (pOwner->GetAmmoCount(m_iSecondaryAmmoType) > 0)
			{
				switch( m_tSlamState)
				{
					case SATCHEL_ATTACHED_READY:
						{
							iAnim = m_bNeedDetonatorDraw ? ACT_SLAM_STICKWALL_DRAW : ACT_SLAM_TRIPMINE_DRAW;
						}
						break;
					case SATCHEL_ATTACH:
						{
							if (m_bNeedDetonatorHolster)
							{
								iAnim = ACT_SLAM_STICKWALL_DETONATOR_HOLSTER;
								m_bNeedDetonatorHolster = false;
							}
							else if (m_bDetonatorArmed)
							{
								iAnim =	m_bNeedDetonatorDraw ? ACT_SLAM_DETONATOR_STICKWALL_DRAW : ACT_SLAM_STICKWALL_DRAW;
								m_bNeedDetonatorDraw = false;
							}
							else
							{
								iAnim =	ACT_SLAM_STICKWALL_ND_DRAW;
							}
						}
						break;
					case SATCHEL_THROW:
						{
							if (m_bNeedDetonatorHolster)
							{
								iAnim = ACT_SLAM_THROW_DETONATOR_HOLSTER;
								m_bNeedDetonatorHolster = false;
							}
							else if (m_bDetonatorArmed)
							{
								iAnim =	m_bNeedDetonatorDraw ? ACT_SLAM_DETONATOR_THROW_DRAW : ACT_SLAM_THROW_DRAW;
								m_bNeedDetonatorDraw = false;
							}
							else
							{
								iAnim =	ACT_SLAM_THROW_ND_DRAW;
							}
						}
						break;
				}
				m_bClearReload			= true;
			}
			// If no ammo and armed, idle with only the detonator
			else if (m_bDetonatorArmed)
			{
				iAnim =	m_bNeedDetonatorDraw ? ACT_SLAM_DETONATOR_DRAW : ACT_SLAM_DETONATOR_IDLE;
				m_bNeedDetonatorDraw = false;
			}
			else
			{
#ifndef CLIENT_DLL
				pOwner->Weapon_Drop( this );
				UTIL_Remove(this);
#endif
			}
		}
		else if (pOwner->GetAmmoCount(m_iSecondaryAmmoType) <= 0)
		{
#ifndef CLIENT_DLL
			pOwner->Weapon_Drop( this );
			UTIL_Remove(this);
#endif
		}

		// If I don't need to reload just do the appropriate idle
		else
		{
			switch( m_tSlamState)
			{
				case SATCHEL_ATTACHED_READY:
					{
						iAnim = m_bDetonatorArmed ? ACT_SLAM_STICKWALL_IDLE : ACT_SLAM_TRIPMINE_IDLE;
						m_flWallSwitchTime = 0;
					}
					break;
				case SATCHEL_THROW:
					{
						if (m_bNeedDetonatorHolster)
						{
							iAnim = ACT_SLAM_THROW_DETONATOR_HOLSTER;
							m_bNeedDetonatorHolster = false;
						}
						else
						{
							iAnim = m_bDetonatorArmed ? ACT_SLAM_THROW_IDLE : ACT_SLAM_THROW_ND_IDLE;
							m_flWallSwitchTime = 0;
						}
					}
					break;
				case SATCHEL_ATTACH:
					{
						if (m_bNeedDetonatorHolster)
						{
							iAnim = ACT_SLAM_STICKWALL_DETONATOR_HOLSTER;
							m_bNeedDetonatorHolster = false;
						}
						else
						{
							iAnim = m_bDetonatorArmed ? ACT_SLAM_STICKWALL_IDLE : ACT_SLAM_TRIPMINE_IDLE;
							m_flWallSwitchTime = 0;
						}
					}
					break;
			}
		}
		SendWeaponAnim( iAnim );
	}
}

bool CWeapon_Satchel::Deploy( void )
{
	CBaseCombatCharacter *pOwner  = GetOwner();
	if (!pOwner)
	{
		return false;
	}

	m_bDetonatorArmed = AnyUndetonatedCharges();


	SetModel( GetViewModel() );

	m_tSlamState		= (int)SATCHEL_THROW;

	// ------------------------------
	// Pick the right draw animation
	// ------------------------------
	int iActivity;

	// If detonator is already armed
	m_bNeedReload = false;
	if (m_bDetonatorArmed)
	{
		if (pOwner->GetAmmoCount(m_iSecondaryAmmoType) <= 0)
		{
			iActivity = ACT_SLAM_DETONATOR_DRAW;
			m_bNeedReload = true;
		}
		else if (CanAttachSatchel())
		{
			iActivity = ACT_SLAM_DETONATOR_STICKWALL_DRAW; 
			SetSatchelState(SATCHEL_ATTACHED_READY);
		}
		else
		{
			iActivity = ACT_SLAM_DETONATOR_THROW_DRAW; 
			SetSatchelState(SATCHEL_THROW);
		}
	}
	else
	{	
		if (CanAttachSatchel())
		{
			iActivity = ACT_SLAM_TRIPMINE_DRAW; 
			SetSatchelState(SATCHEL_ATTACHED_READY);
		}
		else
		{
			iActivity = ACT_SLAM_THROW_ND_DRAW; 
			SetSatchelState(SATCHEL_THROW);
		}
	}

	return DefaultDeploy( (char*)GetViewModel(), (char*)GetWorldModel(), iActivity, (char*)GetAnimPrefix() );
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
// Input  :
// Output :
//-----------------------------------------------------------------------------
CWeapon_Satchel::CWeapon_Satchel(void)
{
	m_tSlamState			= (int)SATCHEL_THROW;
	m_bDetonatorArmed		= false;
	m_bNeedReload			= true;
	m_bClearReload			= false;
	m_bThrowSatchel			= false;
	m_bAttachSatchel		= false;
	m_bAttachTripmine		= false;
	m_bNeedDetonatorDraw	= false;
	m_bNeedDetonatorHolster	= false;
}
