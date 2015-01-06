//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: German Entrenching Tool
// Notes:	Spong!
//=======================================================================//

#include "cbase.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "gamerules.h"
#include "ammodef.h"
#include "mathlib.h"
#include "in_buttons.h"
#include "weapon_hl2mpbasebasebludgeon.h"
#include "npcevent.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	SHOVEL_RANGE_NORMAL	62.0f	// effective range normally
#define	SHOVEL_REFIRE	0.75f		// time between stabs/slash/swings, etc.

#ifdef CLIENT_DLL
#define CWeaponShovel C_WeaponShovel
#endif

//-----------------------------------------------------------------------------
// CWeaponShovel
//-----------------------------------------------------------------------------

class CWeaponShovel : public CBaseHL2MPBludgeonWeapon
{
public:
	DECLARE_CLASS( CWeaponShovel, CBaseHL2MPBludgeonWeapon );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	DECLARE_ACTTABLE();

	CWeaponShovel();

	float		GetRange( void )			{	return	SHOVEL_RANGE_NORMAL;	}
	float		GetFireRate( void )			{	return	SHOVEL_REFIRE;	}

	float		GetDamageForActivity( Activity hitActivity );
	void		SecondaryAttack( void )	{	return;	}	// tweak this to add stealth mode

	void		Drop( const Vector &vecVelocity );

	// Animation event
#ifndef CLIENT_DLL
	virtual void	Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	void			HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	virtual bool	ShouldDrawPickup() { return false; }
#endif

	CWeaponShovel( const CWeaponShovel & );

private:
		
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponShovel, DT_WeaponShovel )

BEGIN_NETWORK_TABLE( CWeaponShovel, DT_WeaponShovel )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponShovel )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_shovel, CWeaponShovel );
PRECACHE_WEAPON_REGISTER( weapon_shovel );

acttable_t	CWeaponShovel::m_acttable[] = 
{
	// attack
	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_SPADE,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_SPADE,	false },
	{ ACT_MP_ATTACK_PRONE_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_PRONE_SPADE, false },

	// aiming down sight, recent attack
	{ ACT_HAJ_IDLE_AIM,					ACT_HAJ_STAND_AIM_SPADE,		false },
	{ ACT_HAJ_WALK_AIM,					ACT_HAJ_WALK_AIM_SPADE,			false },
	{ ACT_HAJ_IDLE_CROUCH_AIM,			ACT_HAJ_CROUCH_AIM_SPADE,		false },
	{ ACT_HAJ_WALK_CROUCH_AIM,			ACT_HAJ_CROUCHWALK_IDLE_SPADE,	false },
	{ ACT_HAJ_RUN_AIM,					ACT_HAJ_RUN_AIM_SPADE,			false },

	// movement
	{ ACT_MP_STAND_IDLE,				ACT_HAJ_STAND_AIM_SPADE,		false },
	{ ACT_MP_RUN,						ACT_HAJ_RUN_AIM_SPADE,			false },
	{ ACT_MP_SPRINT,					ACT_HAJ_SPRINT_AIM_SPADE,		false },
	{ ACT_MP_WALK,						ACT_HAJ_WALK_AIM_SPADE,			false },
	{ ACT_MP_PRONE_IDLE,				ACT_HAJ_PRONE_AIM_SPADE,		false },
	{ ACT_MP_PRONE_CRAWL,				ACT_HAJ_PRONEWALK_AIM_SPADE,	false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HAJ_CROUCH_AIM_SPADE,		false },
	{ ACT_MP_CROUCHWALK,				ACT_HAJ_CROUCHWALK_AIM_SPADE,	false },
};

IMPLEMENT_ACTTABLE(CWeaponShovel);


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponShovel::CWeaponShovel( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Get the damage amount for the animation we're doing
// Input  : hitActivity - currently played activity
// Output : Damage amount
//-----------------------------------------------------------------------------
float CWeaponShovel::GetDamageForActivity( Activity hitActivity )
{
#ifndef CLIENT_DLL
	// TODO - tune this so that it does a different ammount for the stealth kill
	CBaseCombatCharacter* pOwner = ToBaseCombatCharacter( GetOwner() );

	if( pOwner )
	{
		Vector vecDirection;
		AngleVectors( GetAbsAngles(), &vecDirection );

		Vector vecEnd;
		VectorMA( pOwner->Weapon_ShootPosition(), SHOVEL_RANGE_NORMAL, vecDirection, vecEnd );

		trace_t traceHit;
		UTIL_TraceLine( pOwner->Weapon_ShootPosition(), vecEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit );

		if( traceHit.DidHit() && traceHit.m_pEnt && traceHit.hitgroup == HITGROUP_HEAD )
		{
			return 100.0f;
		}
	}
#endif

	return 75.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponShovel::Drop( const Vector &vecVelocity )
{
	// You can't drop a shovel so if, somehow, you do manage to, delete it.
#ifndef CLIENT_DLL
	UTIL_Remove( this );
#endif
}

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Animation event handlers
//-----------------------------------------------------------------------------
void CWeaponShovel::HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	// Trace up or down based on where the enemy is...
	// But only if we're basically facing that direction
	Vector vecDirection;
	AngleVectors( GetAbsAngles(), &vecDirection );

	Vector vecEnd;
	VectorMA( pOperator->Weapon_ShootPosition(), 50, vecDirection, vecEnd );
	
	CBaseEntity *pHurt = pOperator->CheckTraceHullAttack( pOperator->Weapon_ShootPosition(),
														  vecEnd,
														  Vector(-16,-16,-16),
														  Vector(36,36,36),
														  GetDamageForActivity( GetActivity() ),
														  DMG_CLUB,
														  0.75 ); // check this force scale value?
	
	// did I hit someone?
	if ( pHurt )
	{

		// Fake a trace impact, so the effects work out like a player's crowbaw
		trace_t traceHit;
		UTIL_TraceLine( pOperator->Weapon_ShootPosition(),
						pHurt->GetAbsOrigin(),
						MASK_SHOT_HULL | CONTENTS_HITBOX,
						pOperator,
						COLLISION_GROUP_NONE,
						&traceHit );

		// do the slash decal
		ImpactEffect( traceHit );
			
	}
	else
	{
		//WeaponSound( MELEE_MISS );
	}
}

//-----------------------------------------------------------------------------
// Animation event
//-----------------------------------------------------------------------------
void CWeaponShovel::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
	case EVENT_WEAPON_MELEE_HIT:
		HandleAnimEventMeleeHit( pEvent, pOperator );
		break;

	default:
		BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
		break;
	}
}

#endif