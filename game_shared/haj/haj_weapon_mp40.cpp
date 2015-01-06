//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: MP40 9mm base class definition
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
	#include "weapon_flaregun.h"
#endif

#include "weapon_hl2mpbase.h"
//#include "weapon_hl2mpbase_machinegun.h"
#include "haj_weapon_base.h"



#ifdef CLIENT_DLL
#define CWeaponMP40 C_WeaponMP40
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponMP40 : public CHAJWeaponBase
{
public:
	DECLARE_CLASS( CWeaponMP40, CHAJWeaponBase );

	CWeaponMP40();

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	
	void				Precache( void );
	void				SecondaryAttack( void );

	int					GetMinBurst() { return 2; }
	int					GetMaxBurst() { return 5; }

	const char			*GetTracerType( void ) { return "WhizTracer"; }
	virtual void		Equip( CBaseCombatCharacter *pOwner );
	bool				Reload( void );
	virtual void		Spawn();
	float				GetFireRate( void ) { return 0.12f; }	// 500rpm?

	virtual float		GetMaxPenetrationDistance( unsigned short surfaceType );

	const WeaponProficiencyInfo_t *GetProficiencyValues();

	DECLARE_ACTTABLE();

protected:

	Vector	m_vecTossVelocity;
	
private:
	CWeaponMP40( const CWeaponMP40 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponMP40, DT_WeaponMP40 )

BEGIN_NETWORK_TABLE( CWeaponMP40, DT_WeaponMP40 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponMP40 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_mp40, CWeaponMP40 );
PRECACHE_WEAPON_REGISTER(weapon_mp40);

acttable_t	CWeaponMP40::m_acttable[] = 
{
	// attack
	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_MP40,		false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_MP40,		false },
	{ ACT_MP_ATTACK_PRONE_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_PRONE_MP40, false },

	// reload
	{ ACT_MP_RELOAD_STAND,				ACT_HAJ_RELOAD_MP40,			false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HAJ_RELOAD_MP40,			false },
	{ ACT_MP_RELOAD_PRONE,				ACT_HAJ_RELOAD_PRONE_MP40,		false },

	// aiming down sight
	{ ACT_HAJ_IDLE_AIM,					ACT_HAJ_STAND_AIM_MP40,			false },
	{ ACT_HAJ_WALK_AIM,					ACT_HAJ_WALK_AIM_MP40,			false },
	{ ACT_HAJ_IDLE_CROUCH_AIM,			ACT_HAJ_CROUCH_AIM_MP40,		false },
	{ ACT_HAJ_WALK_CROUCH_AIM,			ACT_HAJ_CROUCHWALK_IDLE_MP40,	false },
	{ ACT_HAJ_RUN_AIM,					ACT_HAJ_RUN_AIM_MP40,			false },

	// movement
	{ ACT_MP_STAND_IDLE,				ACT_HAJ_STAND_IDLE_MP40,		false },
	{ ACT_MP_RUN,						ACT_HAJ_RUN_IDLE_MP40,			false },
	{ ACT_MP_SPRINT,					ACT_HAJ_SPRINT_IDLE_MP40,		false },
	{ ACT_MP_WALK,						ACT_HAJ_WALK_IDLE_MP40,			false },
	{ ACT_MP_PRONE_IDLE,				ACT_HAJ_PRONE_AIM_MP40,			false },
	{ ACT_MP_PRONE_CRAWL,				ACT_HAJ_PRONEWALK_IDLE_MP40,	false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HAJ_CROUCH_IDLE_MP40,		false },
	{ ACT_MP_CROUCHWALK,				ACT_HAJ_CROUCHWALK_IDLE_MP40,	false },
};

IMPLEMENT_ACTTABLE(CWeaponMP40);

//=========================================================
CWeaponMP40::CWeaponMP40( )
{
	m_fMinRange1		= 0;// No minimum range. 
	m_fMaxRange1		= 700;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMP40::Precache( void )
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Give this weapon longer range when wielded by an ally NPC.
//-----------------------------------------------------------------------------
void CWeaponMP40::Equip( CBaseCombatCharacter *pOwner )
{
	BaseClass::Equip( pOwner );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeaponMP40::Reload( void )
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
// Purpose: 
//-----------------------------------------------------------------------------
ConVar haj_flare_test( "haj_flare_test", "0", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar haj_flare_gravity( "haj_flare_gravity", "1.0", FCVAR_CHEAT | FCVAR_REPLICATED );

void CWeaponMP40::SecondaryAttack( void )
{

#ifdef GAME_DLL
	if ( haj_flare_test.GetBool() )
	{
		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
		
		if ( pOwner == NULL )
			return;

		CFlare *pFlare = CFlare::Create( pOwner->Weapon_ShootPosition(), pOwner->EyeAngles(), pOwner, FLARE_DURATION );

		if ( pFlare == NULL )
			return;

		Vector forward;
		pOwner->EyeVectors( &forward );
		pFlare->SetMoveType( MOVETYPE_FLYGRAVITY );
		pFlare->SetGravity( haj_flare_gravity.GetFloat() );
		pFlare->SetAbsVelocity( forward * 1500 );
		pFlare->m_bSmoke = false;
		
		m_flNextSecondaryAttack = gpGlobals->curtime + 3.0f;
	}
#endif
}

float CWeaponMP40::GetMaxPenetrationDistance( unsigned short surfaceType )
{
	return BaseClass::GetMaxPenetrationDistance( surfaceType ) * 0.5;
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CWeaponMP40::GetProficiencyValues()
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


void CWeaponMP40::Spawn( void )
{
	BaseClass::Spawn();
}