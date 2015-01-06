//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: MG34 7.62mm base class definition
//
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

#include "weapon_hl2mpbase.h"
//#include "weapon_hl2mpbase_machinegun.h"
#include "haj_weapon_base.h"

#ifdef CLIENT_DLL
#define CWeaponMG34 C_WeaponMG34
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponMG34 : public CHAJWeaponBase
{
public:
	DECLARE_CLASS( CWeaponMG34, CHAJWeaponBase );

	CWeaponMG34();

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	
	void			Precache( void );

	int				GetMinBurst() { return 2; }
	int				GetMaxBurst() { return 5; }

	const char		*GetTracerType( void ) { return "792Tracer"; }
	virtual void	Equip( CBaseCombatCharacter *pOwner );
	bool			Reload( void );
	virtual void	Spawn();

	float			GetFireRate( void ) { return 0.070588f; }	// 850rpm
	Activity		GetPrimaryAttackActivity( void );

	bool			Holster( CBaseCombatWeapon *pSwitchingTo );

	bool			CanReload();
	
	bool			HasBipod( void ) { return true; }
	virtual void	ItemPostFrame();

	const WeaponProficiencyInfo_t *GetProficiencyValues();

	DECLARE_ACTTABLE();
	
private:
	CWeaponMG34( const CWeaponMG34 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponMG34, DT_WeaponMG34 )

BEGIN_NETWORK_TABLE( CWeaponMG34, DT_WeaponMG34 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponMG34 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_mg34, CWeaponMG34 );
PRECACHE_WEAPON_REGISTER(weapon_mg34);

acttable_t	CWeaponMG34::m_acttable[] = 
{
	// attack
	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,			ACT_HAJ_PRIMARYATTACK_MG34,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,			ACT_HAJ_PRIMARYATTACK_MG34,	false },
	{ ACT_MP_ATTACK_PRONE_PRIMARYFIRE,			ACT_HAJ_PRIMARYATTACK_PRONE_MG34, false },

	// reload
	{ ACT_MP_RELOAD_STAND,						ACT_HAJ_RELOAD_MG34,							false },
	{ ACT_MP_RELOAD_CROUCH,						ACT_HAJ_RELOAD_MG34,							false },
	{ ACT_MP_RELOAD_PRONE,						ACT_HAJ_RELOAD_PRONE_MG34,						false },

	// aiming down sight
	{ ACT_HAJ_IDLE_AIM,							ACT_HAJ_STAND_AIM_MG34,							false },
	{ ACT_HAJ_WALK_AIM,							ACT_HAJ_WALK_AIM_MG34,							false },
	{ ACT_HAJ_IDLE_CROUCH_AIM,					ACT_HAJ_CROUCH_AIM_MG34,						false },
	{ ACT_HAJ_WALK_CROUCH_AIM,					ACT_HAJ_CROUCHWALK_IDLE_MG34,					false },
	{ ACT_HAJ_RUN_AIM,							ACT_HAJ_RUN_AIM_MG34,							false },

	// movement
	{ ACT_MP_STAND_IDLE,						ACT_HAJ_STAND_IDLE_MG34,						false },
	{ ACT_MP_RUN,								ACT_HAJ_RUN_IDLE_MG34,							false },
	{ ACT_MP_SPRINT,							ACT_HAJ_SPRINT_IDLE_MG34,						false },
	{ ACT_MP_WALK,								ACT_HAJ_WALK_IDLE_MG34,							false },
	{ ACT_MP_PRONE_IDLE,						ACT_HAJ_PRONE_AIM_MG34,							false },
	{ ACT_MP_PRONE_CRAWL,						ACT_HAJ_PRONEWALK_IDLE_MG34,					false },
	{ ACT_MP_CROUCH_IDLE,						ACT_HAJ_CROUCH_IDLE_MG34,						false },
	{ ACT_MP_CROUCHWALK,						ACT_HAJ_CROUCHWALK_IDLE_MG34,					false },

	// deploy
	{ ACT_HAJ_DEPLOY,							ACT_HAJ_DEPLOY_MG34,							false },
	{ ACT_HAJ_PRONE_DEPLOY,						ACT_HAJ_PRONE_DEPLOY_MG34,						false },
	{ ACT_HAJ_PRIMARYATTACK_DEPLOYED,			ACT_HAJ_PRIMARYATTACK_DEPLOYED_MG34,			false },

	// deploy
	{ ACT_HAJ_DEPLOY,							ACT_HAJ_DEPLOY_MG34,							false },
	{ ACT_HAJ_PRONE_DEPLOY,						ACT_HAJ_PRONE_DEPLOY_MG34,						false },
	{ ACT_HAJ_PRIMARYATTACK_DEPLOYED,			ACT_HAJ_PRIMARYATTACK_DEPLOYED_MG34,			false },
	{ ACT_HAJ_PRIMARYATTACK_PRONE_DEPLOYED,		ACT_HAJ_PRIMARYATTACK_PRONE_DEPLOYED_MG34,		false },
};

IMPLEMENT_ACTTABLE(CWeaponMG34);

//=========================================================
CWeaponMG34::CWeaponMG34( )
{
	m_fMinRange1		= 0;// No minimum range. 
	m_fMaxRange1		= 1400;
}

bool CWeaponMG34::Holster( CBaseCombatWeapon *pSwitchingTo )
{
#ifndef CLIENT_DLL
	CHajPlayer *pPlayer = ToHajPlayer( GetOwner() );

	if( pPlayer->IsAlive() == false )
	{
		return true;
	}
#endif

	return BaseClass::Holster( pSwitchingTo );
}

bool CWeaponMG34::CanReload( void )
{
	if( m_bIsBipodDeployed )
		return true;

	return false;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMG34::Precache( void )
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Give this weapon longer range when wielded by an ally NPC.
//-----------------------------------------------------------------------------
void CWeaponMG34::Equip( CBaseCombatCharacter *pOwner )
{
	m_fMaxRange1 = 1400;

	BaseClass::Equip( pOwner );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CWeaponMG34::GetPrimaryAttackActivity( void )
{
	if ( m_bIsBipodDeployed )
	{
		return ACT_VM_PRIMARYATTACK_DEPLOYED;
	}
	else
	{
		return ACT_VM_PRIMARYATTACK;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeaponMG34::Reload( void )
{
	if( !CanReload() )
		return false;

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
const WeaponProficiencyInfo_t *CWeaponMG34::GetProficiencyValues()
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

void CWeaponMG34::Spawn( void )
{
	BaseClass::Spawn();
}

void CWeaponMG34::ItemPostFrame()
{
	BaseClass::ItemPostFrame();
}