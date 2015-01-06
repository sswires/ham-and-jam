//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: Kar 98 Mauser Rifle
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
#include "haj_weapon_boltrifle.h"

#ifdef CLIENT_DLL
#define CWeaponKar98 C_WeaponKar98
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponKar98 : public CHAJBoltRifle
{
public:
	DECLARE_CLASS( CWeaponKar98, CHAJBoltRifle );

	CWeaponKar98();

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	
	void			Precache( void );
	void			SecondaryAttack( void );

	const char		*GetTracerType( void ) { return "WhizTracer"; }
	virtual void	Equip( CBaseCombatCharacter *pOwner );

	bool			Reload( void );
	virtual void	Spawn();

	float			GetFireRate( void ) { return 1.0f; }
	Activity		GetPrimaryAttackActivity( void );
	const WeaponProficiencyInfo_t *GetProficiencyValues();

	DECLARE_ACTTABLE();
	
private:
	CWeaponKar98( const CWeaponKar98 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponKar98, DT_WeaponKar98 )

BEGIN_NETWORK_TABLE( CWeaponKar98, DT_WeaponKar98 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponKar98 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_kar98, CWeaponKar98 );
PRECACHE_WEAPON_REGISTER(weapon_kar98);

acttable_t	CWeaponKar98::m_acttable[] = 
{
	// attack
	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_KAR,		false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_KAR,		false },
	{ ACT_MP_ATTACK_PRONE_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_PRONE_KAR, false },

	// reload
	{ ACT_MP_RELOAD_STAND,				ACT_HAJ_RELOAD_KAR,				false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HAJ_RELOAD_KAR,				false },
	{ ACT_MP_RELOAD_PRONE,				ACT_HAJ_RELOAD_PRONE_KAR,		false },

	// aiming down sight
	{ ACT_HAJ_IDLE_AIM,					ACT_HAJ_STAND_AIM_KAR,			false },
	{ ACT_HAJ_WALK_AIM,					ACT_HAJ_WALK_AIM_KAR,			false },
	{ ACT_HAJ_IDLE_CROUCH_AIM,			ACT_HAJ_CROUCH_AIM_KAR,			false },
	{ ACT_HAJ_WALK_CROUCH_AIM,			ACT_HAJ_CROUCHWALK_IDLE_KAR,	false },
	{ ACT_HAJ_RUN_AIM,					ACT_HAJ_RUN_AIM_KAR,			false },

	// movement
	{ ACT_MP_STAND_IDLE,				ACT_HAJ_STAND_IDLE_KAR,			false },
	{ ACT_MP_RUN,						ACT_HAJ_RUN_IDLE_KAR,			false },
	{ ACT_MP_SPRINT,					ACT_HAJ_SPRINT_IDLE_KAR,		false },
	{ ACT_MP_WALK,						ACT_HAJ_WALK_IDLE_KAR,			false },
	{ ACT_MP_PRONE_IDLE,				ACT_HAJ_PRONE_AIM_KAR,			false },
	{ ACT_MP_PRONE_CRAWL,				ACT_HAJ_PRONEWALK_IDLE_KAR,		false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HAJ_CROUCH_IDLE_KAR,		false },
	{ ACT_MP_CROUCHWALK,				ACT_HAJ_CROUCHWALK_IDLE_KAR,	false },
};

IMPLEMENT_ACTTABLE(CWeaponKar98);

//-----------------------------------------------------------------------------
// Purpose: Construtor
//-----------------------------------------------------------------------------
CWeaponKar98::CWeaponKar98( )
{
	m_fMinRange1		= 0;// No minimum range. 
	m_fMaxRange1		= 1400;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponKar98::Precache( void )
{
	BaseClass::Precache();
}


//-----------------------------------------------------------------------------
// Purpose: Give this weapon longer range when wielded by an ally NPC.
//-----------------------------------------------------------------------------
void CWeaponKar98::Equip( CBaseCombatCharacter *pOwner )
{
	BaseClass::Equip( pOwner );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CWeaponKar98::GetPrimaryAttackActivity( void )
{
	return BaseClass::GetPrimaryAttackActivity();
}


//-----------------------------------------------------------------------------
// Purpose: Called when we reload (duh!)
//-----------------------------------------------------------------------------
bool CWeaponKar98::Reload( void )
{
	// Only allowed a reload if the magazine is empty
	if ( !CanReload() /* || m_iClip1 > 0 */ )
		return false;

	ExitIronsights();

	float	fCacheTime = m_flNextSecondaryAttack;
	bool	fRet = DefaultReload( GetMaxClip1(), ACT_VM_RELOAD, ACT_VM_RELOAD );
	
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
// Purpose: Secondary attack
//-----------------------------------------------------------------------------
void CWeaponKar98::SecondaryAttack( void )
{
	// don't much else than go bang bang...
	BaseClass::SecondaryAttack();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CWeaponKar98::GetProficiencyValues()
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

//-----------------------------------------------------------------------------
// Purpose: Called when the weapon spawns
//-----------------------------------------------------------------------------
void CWeaponKar98::Spawn( void )
{
	BaseClass::Spawn();
}