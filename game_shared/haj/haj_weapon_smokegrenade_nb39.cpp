//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: Player Nb39 Smoke Grenade (Axis)
//
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#include "c_hl2mp_player.h"
#include "c_te_effect_dispatch.h"
#else
#include "hl2mp_player.h"
#include "te_effect_dispatch.h"
#include "haj_grenade_base.h"
#include "haj_grenade_stiel.h"
#include "haj_grenade_smoke.h"
#endif

#include "weapon_ar2.h"
#include "effect_dispatch_data.h"
#include "haj_weapon_grenadebase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// CRITICAL - You must define these correctly!
//#define GRENADE_MODEL "models/weapons/w_models/w_models/w_no80smoke.mdl"
#define GRENADE_MODEL "models/weapons/w_models/w_nebel_item.mdl"
#define GRENADE_TIMER	4.5f	//Seconds
#define GRENADE_NB39_DURATION 20.0f	// How long in seconds to make smoke

#define GRENADE_PAUSED_NO			0
#define GRENADE_PAUSED_PRIMARY		1
#define GRENADE_PAUSED_SECONDARY	2

#define GRENADE_RADIUS	4.0f // inches
#define GRENADE_DAMAGE_RADIUS 250.0f

#define GRENADE_MAX_HOLD_TIME		2.0f


#ifdef CLIENT_DLL
#define CWeaponSmokeNb39 C_WeaponSmokeNb39
#endif

//-----------------------------------------------------------------------------
// Fragmentation grenades
//-----------------------------------------------------------------------------
class CWeaponSmokeNb39: public CWeaponGrenade
{
	DECLARE_CLASS( CWeaponSmokeNb39, CWeaponGrenade );
public:

	virtual void Precache( void );

	virtual bool CanCookGrenade() { return false; }
	virtual float GetThrowPowerMultiplier( void ) { return 1.15f; }

#ifndef CLIENT_DLL
	virtual CHAJGrenade* CreateGrenade( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer );
#endif

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	DECLARE_ACTTABLE();
};

acttable_t	CWeaponSmokeNb39::m_acttable[] = 
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

IMPLEMENT_ACTTABLE(CWeaponSmokeNb39);

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSmokeNb39, DT_WeaponSmokeNb39 )

BEGIN_NETWORK_TABLE( CWeaponSmokeNb39, DT_WeaponSmokeNb39 )
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponSmokeNb39 )
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_nb39smoke, CWeaponSmokeNb39 );
PRECACHE_WEAPON_REGISTER( weapon_nb39smoke );
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSmokeNb39::Precache( void )
{
	BaseClass::Precache();

#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "npc_grenade_smoke" );
#endif

	PrecacheScriptSound( "WeaponFrag.Throw" );
	PrecacheScriptSound( "WeaponFrag.Roll" );

}

#ifndef CLIENT_DLL
CHAJGrenade* CWeaponSmokeNb39::CreateGrenade( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer )
{
	CHAJGrenade *pGrenade = SmokeGrenade_Create( position, angles, velocity, angVelocity, pOwner, GRENADE_TIMER, GRENADE_NB39_DURATION, GRENADE_MODEL );
	return pGrenade;
}
#endif


