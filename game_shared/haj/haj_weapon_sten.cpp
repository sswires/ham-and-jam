//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: Sten 9mm Mk3 weapon definition
//
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
	#include "haj_player_c.h"
#else
	#include "haj_player.h"
#endif

#include "haj_weapon_sten.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSten, DT_WeaponSten )

BEGIN_NETWORK_TABLE( CWeaponSten, DT_WeaponSten )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponSten )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_sten, CWeaponSten );
PRECACHE_WEAPON_REGISTER(weapon_sten);

acttable_t	CWeaponSten::m_acttable[] = 
{
	// attack
	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_STEN,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_STEN,	false },
	{ ACT_MP_ATTACK_PRONE_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_PRONE_STEN, false },

	// reload
	{ ACT_MP_RELOAD_STAND,				ACT_HAJ_RELOAD_STEN,			false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HAJ_RELOAD_STEN,			false },
	{ ACT_MP_RELOAD_PRONE,				ACT_HAJ_RELOAD_PRONE_STEN,		false },

	// aiming down sight
	{ ACT_HAJ_IDLE_AIM,					ACT_HAJ_STAND_AIM_STEN,			false },
	{ ACT_HAJ_WALK_AIM,					ACT_HAJ_WALK_AIM_STEN,			false },
	{ ACT_HAJ_IDLE_CROUCH_AIM,			ACT_HAJ_CROUCH_AIM_STEN,		false },
	{ ACT_HAJ_WALK_CROUCH_AIM,			ACT_HAJ_CROUCHWALK_IDLE_STEN,	false },
	{ ACT_HAJ_RUN_AIM,					ACT_HAJ_RUN_AIM_STEN,			false },

	// movement
	{ ACT_MP_STAND_IDLE,				ACT_HAJ_STAND_IDLE_STEN,		false },
	{ ACT_MP_RUN,						ACT_HAJ_RUN_IDLE_STEN,			false },
	{ ACT_MP_SPRINT,					ACT_HAJ_SPRINT_IDLE_STEN,		false },
	{ ACT_MP_WALK,						ACT_HAJ_WALK_IDLE_STEN,			false },
	{ ACT_MP_PRONE_IDLE,				ACT_HAJ_PRONE_AIM_STEN,			false },
	{ ACT_MP_PRONE_CRAWL,				ACT_HAJ_PRONEWALK_IDLE_STEN,	false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HAJ_CROUCH_IDLE_STEN,		false },
	{ ACT_MP_CROUCHWALK,				ACT_HAJ_CROUCHWALK_IDLE_STEN,	false },
};

IMPLEMENT_ACTTABLE(CWeaponSten);

//=========================================================
CWeaponSten::CWeaponSten( )
{
	m_fMinRange1		= 0;// No minimum range. 
	m_fMaxRange1		= 700;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSten::Precache( void )
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Give this weapon longer range when wielded by an ally NPC.
//-----------------------------------------------------------------------------
void CWeaponSten::Equip( CBaseCombatCharacter *pOwner )
{
	BaseClass::Equip( pOwner );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeaponSten::Reload( void )
{
	bool fRet;
	float fCacheTime = m_flNextSecondaryAttack;

	fRet = DefaultReload( GetMaxClip1(), ACT_VM_RELOAD, ACT_VM_RELOADMID );
	if ( fRet )
	{
		// Undo whatever the reload process has done to our secondary
		// attack timer. We allow you to interrupt reloading to fire
		// a grenade.
		m_flNextSecondaryAttack = GetOwner()->m_flNextAttack = fCacheTime;

		WeaponSound( RELOAD );
	}

	return fRet;
}

//-----------------------------------------------------------------------------
// Purpose: rate of fire = "1/(R/60)" where R is rounds per minute
//-----------------------------------------------------------------------------
float CWeaponSten::GetFireRate( void )
{
	// 500 rpm
	return 0.12f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSten::SecondaryAttack( void )
{

}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CWeaponSten::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0,		0.75	},
		{ 5.00,		0.75	},
		{ 10.0/3.0, 0.75	},
		{ 5.0/3.0,	0.75	},
		{ 1.00,		1.0		},
	};

	COMPILE_TIME_ASSERT( ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}

void CWeaponSten::Spawn( void )
{
	BaseClass::Spawn();
}

float CWeaponSten::GetMaxPenetrationDistance( unsigned short surfaceType )
{
	return BaseClass::GetMaxPenetrationDistance( surfaceType ) * 0.5;
}