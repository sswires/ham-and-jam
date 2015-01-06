//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: Player Millsbomb (greande) weapon
//
// $NoKeywords: $
//=======================================================================//
#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"

#include "haj_weapon_grenadebase.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
	#include "c_te_effect_dispatch.h"
#else
	#include "hl2mp_player.h"
	#include "te_effect_dispatch.h"
	#include "haj_grenade_mills.h"
#endif


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
#define CWeaponMills C_WeaponMills
#endif

//-----------------------------------------------------------------------------
// Fragmentation grenades
//-----------------------------------------------------------------------------
class CWeaponMills: public CWeaponGrenade
{
	DECLARE_CLASS( CWeaponMills, CWeaponGrenade );
public:

	virtual void Precache( void );

	virtual float GetDamageRadius( void )
	{
		return 350.0f;
	}

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	DECLARE_ACTTABLE();
};

acttable_t	CWeaponMills::m_acttable[] = 
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

IMPLEMENT_ACTTABLE(CWeaponMills);

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponMills, DT_WeaponMills )

BEGIN_NETWORK_TABLE( CWeaponMills, DT_WeaponMills )
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponMills )
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_millsbomb, CWeaponMills );
PRECACHE_WEAPON_REGISTER(weapon_millsbomb);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMills::Precache( void )
{
	BaseClass::Precache();

#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "npc_grenade_mills" );
#endif

	PrecacheScriptSound( "WeaponFrag.Throw" );
	PrecacheScriptSound( "WeaponFrag.Roll" );

}

