//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: Bren Mk I weapon definition
// Note:	We consider the Bren MK I the baseclass, the Bren 
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"

#if defined( CLIENT_DLL )
	#include "haj_player_c.h"
#else
	#include "haj_player.h"
#endif

#include "haj_weapon_bren.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponBren, DT_WeaponBren )

BEGIN_NETWORK_TABLE( CWeaponBren, DT_WeaponBren )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponBren )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_bren, CWeaponBren );
PRECACHE_WEAPON_REGISTER( weapon_bren );

acttable_t	CWeaponBren::m_acttable[] = 
{
	// attack
	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_BREN,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_BREN,	false },
	{ ACT_MP_ATTACK_PRONE_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_PRONE_BREN, false },

	// reload
	{ ACT_MP_RELOAD_STAND,				ACT_HAJ_RELOAD_BREN,			false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HAJ_RELOAD_BREN,			false },
	{ ACT_MP_RELOAD_PRONE,				ACT_HAJ_RELOAD_PRONE_BREN,		false },

	// aiming down sight
	{ ACT_HAJ_IDLE_AIM,					ACT_HAJ_STAND_AIM_BREN,			false },
	{ ACT_HAJ_WALK_AIM,					ACT_HAJ_WALK_AIM_BREN,			false },
	{ ACT_HAJ_IDLE_CROUCH_AIM,			ACT_HAJ_CROUCH_AIM_BREN,		false },
	{ ACT_HAJ_WALK_CROUCH_AIM,			ACT_HAJ_CROUCHWALK_IDLE_BREN,	false },
	{ ACT_HAJ_RUN_AIM,					ACT_HAJ_RUN_AIM_BREN,			false },

	// movement
	{ ACT_MP_STAND_IDLE,				ACT_HAJ_STAND_IDLE_BREN,		false },
	{ ACT_MP_RUN,						ACT_HAJ_RUN_IDLE_BREN,			false },
	{ ACT_MP_SPRINT,					ACT_HAJ_SPRINT_IDLE_BREN,		false },
	{ ACT_MP_WALK,						ACT_HAJ_WALK_IDLE_BREN,			false },
	{ ACT_MP_PRONE_IDLE,				ACT_HAJ_PRONE_AIM_BREN,			false },
	{ ACT_MP_PRONE_CRAWL,				ACT_HAJ_PRONEWALK_IDLE_BREN,	false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HAJ_CROUCH_IDLE_BREN,		false },
	{ ACT_MP_CROUCHWALK,				ACT_HAJ_CROUCHWALK_IDLE_BREN,	false },

	// deploy
	{ ACT_HAJ_DEPLOY,							ACT_HAJ_DEPLOY_BREN,			false },
	{ ACT_HAJ_PRONE_DEPLOY,						ACT_HAJ_PRONE_DEPLOY_BREN,		false },
	{ ACT_HAJ_PRIMARYATTACK_DEPLOYED,			ACT_HAJ_PRIMARYATTACK_DEPLOYED_BREN, false },
	{ ACT_HAJ_PRIMARYATTACK_PRONE_DEPLOYED,		ACT_HAJ_PRIMARYATTACK_PRONE_DEPLOYED_BREN,		false },
};

IMPLEMENT_ACTTABLE(CWeaponBren);

//=========================================================
CWeaponBren::CWeaponBren( )
{
	m_fMinRange1		= 0;// No minimum range. 
	m_fMaxRange1		= 1400;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CWeaponBren::GetPrimaryAttackActivity( void )
{
	if ( m_bIsBipodDeployed )
	{
		return ACT_VM_PRIMARYATTACK_DEPLOYED;
	}

	return ACT_VM_PRIMARYATTACK;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeaponBren::Reload( void )
{
	bool fRet;
	float fCacheTime = m_flNextSecondaryAttack;

	fRet = DefaultReload( GetMaxClip1(), m_bIsBipodDeployed ? ACT_VM_RELOAD_DEPLOYED : ACT_VM_RELOAD, m_bIsBipodDeployed ? ACT_VM_RELOAD_DEPLOYED : ACT_VM_RELOAD );
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
float CWeaponBren::GetFireRate( void )
{
	// 520 rpm
	return 0.115384f; // ;)
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CWeaponBren::GetProficiencyValues()
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

void CWeaponBren::Spawn( void )
{
	BaseClass::Spawn();
}