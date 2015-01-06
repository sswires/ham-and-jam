//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: Player Stielhandgranate weapon
//
// $NoKeywords: $
//=======================================================================//
#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
	#include "c_te_effect_dispatch.h"
#else
	#include "te_effect_dispatch.h"
	#include "haj_grenade_base.h"
	#include "haj_grenade_stiel.h"
	#include "haj_grenade_smoke.h"
#endif

#include "effect_dispatch_data.h"
#include "haj_weapon_grenadebase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
#define CWeaponStiel C_WeaponStiel
#endif

//-----------------------------------------------------------------------------
// Fragmentation grenades
//-----------------------------------------------------------------------------
class CWeaponStiel: public CWeaponGrenade
{
	DECLARE_CLASS( CWeaponStiel, CWeaponGrenade );
public:

	virtual void Precache( void );

	virtual float GetDamageRadius( void )
	{
		return 250.0f;
	}

	virtual float GetThrowPowerMultiplier( void ) { return 1.15f; }

#ifndef CLIENT_DLL
	virtual CHAJGrenade* CreateGrenade( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer );
#endif

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	DECLARE_ACTTABLE();
};

acttable_t	CWeaponStiel::m_acttable[] = 
{
	// attack
	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_GREN_STICK,		false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_GREN_STICK,		false },
	{ ACT_MP_ATTACK_PRONE_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_PRONE_GREN_STICK,	false },

	// aiming down sight, recent attack
	{ ACT_HAJ_IDLE_AIM,					ACT_HAJ_STAND_AIM_GREN_STICK,			false },
	{ ACT_HAJ_WALK_AIM,					ACT_HAJ_WALK_AIM_GREN_STICK,			false },
	{ ACT_HAJ_IDLE_CROUCH_AIM,			ACT_HAJ_CROUCH_AIM_GREN_STICK,			false },
	{ ACT_HAJ_WALK_CROUCH_AIM,			ACT_HAJ_CROUCHWALK_IDLE_GREN_STICK,		false },
	{ ACT_HAJ_RUN_AIM,					ACT_HAJ_RUN_AIM_GREN_STICK,				false },

	// movement
	{ ACT_MP_STAND_IDLE,				ACT_HAJ_STAND_AIM_GREN_STICK,			false },
	{ ACT_MP_RUN,						ACT_HAJ_RUN_AIM_GREN_STICK,				false },
	{ ACT_MP_SPRINT,					ACT_HAJ_SPRINT_AIM_GREN_STICK,			false },
	{ ACT_MP_WALK,						ACT_HAJ_WALK_AIM_GREN_STICK,			false },
	{ ACT_MP_PRONE_IDLE,				ACT_HAJ_PRONE_AIM_GREN_STICK,			false },
	{ ACT_MP_PRONE_CRAWL,				ACT_HAJ_PRONEWALK_AIM_GREN_STICK,		false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HAJ_CROUCH_AIM_GREN_STICK,			false },
	{ ACT_MP_CROUCHWALK,				ACT_HAJ_CROUCHWALK_AIM_GREN_STICK,		false },
};

IMPLEMENT_ACTTABLE(CWeaponStiel);

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponStiel, DT_WeaponStiel )

BEGIN_NETWORK_TABLE( CWeaponStiel, DT_WeaponStiel )
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponStiel )
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_stielgranate, CWeaponStiel );
PRECACHE_WEAPON_REGISTER(weapon_stielgranate);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponStiel::Precache( void )
{
	BaseClass::Precache();

#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "npc_grenade_stiel" );
#endif

}

#ifndef CLIENT_DLL
CHAJGrenade* CWeaponStiel::CreateGrenade( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer )
{
	// Don't set the owner here, or the player can't interact with grenades he's thrown
	CHAJGrenade *pGrenade = (CHAJGrenade *)CBaseEntity::Create( "npc_grenade_stiel", position, angles, pOwner );
	
	pGrenade->SetTimer( timer, timer - 1.5f );
	pGrenade->SetVelocity( velocity, angVelocity );
	pGrenade->SetThrower( ToBaseCombatCharacter( pOwner ) );
	pGrenade->m_takedamage = DAMAGE_YES;

	return pGrenade;
}
#endif


