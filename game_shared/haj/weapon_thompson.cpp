//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: Sten 9mm Mk5 weapon definition
// Notes: This basically duplicates the Mk5 only changine the anims, ROF
//        and classname.
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#include "c_hl2mp_player.h"
#else
#include "hl2mp_player.h"
#endif

#include "haj_weapon_base.h"

#ifdef CLIENT_DLL
#define CWeaponThompson C_WeaponThompson
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponThompson : public CHAJWeaponBase
{
public:
	DECLARE_CLASS( CWeaponThompson, CHAJWeaponBase );

	CWeaponThompson();

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	virtual void		Precache( void );
	virtual float		GetFireRate( void );
	virtual void		Spawn( void );
	virtual bool		Reload( void );

	const char *GetTracerType( void ) { return "WhizTracer"; }

	DECLARE_ACTTABLE();

private:
	CWeaponThompson( const CWeaponThompson & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponThompson, DT_WeaponThompson )

BEGIN_NETWORK_TABLE( CWeaponThompson, DT_WeaponThompson )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponThompson )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_thompson, CWeaponThompson );
PRECACHE_WEAPON_REGISTER(weapon_thompson);

acttable_t	CWeaponThompson::m_acttable[] = 
{
	// attack
	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_TOMMY,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_TOMMY,	false },
	{ ACT_MP_ATTACK_PRONE_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_PRONE_TOMMY, false },

	// reload
	{ ACT_MP_RELOAD_STAND,				ACT_HAJ_RELOAD_TOMMY,			false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HAJ_RELOAD_TOMMY,			false },
	{ ACT_MP_RELOAD_PRONE,				ACT_HAJ_RELOAD_PRONE_TOMMY,		false },

	// aiming down sight
	{ ACT_HAJ_IDLE_AIM,					ACT_HAJ_STAND_AIM_TOMMY,		false },
	{ ACT_HAJ_WALK_AIM,					ACT_HAJ_WALK_AIM_TOMMY,			false },
	{ ACT_HAJ_IDLE_CROUCH_AIM,			ACT_HAJ_CROUCH_AIM_TOMMY,		false },
	{ ACT_HAJ_WALK_CROUCH_AIM,			ACT_HAJ_CROUCHWALK_IDLE_TOMMY,	false },
	{ ACT_HAJ_RUN_AIM,					ACT_HAJ_RUN_AIM_TOMMY,			false },

	// movement
	{ ACT_MP_STAND_IDLE,				ACT_HAJ_STAND_IDLE_TOMMY,		false },
	{ ACT_MP_RUN,						ACT_HAJ_RUN_IDLE_TOMMY,			false },
	{ ACT_MP_SPRINT,					ACT_HAJ_SPRINT_IDLE_TOMMY,		false },
	{ ACT_MP_WALK,						ACT_HAJ_WALK_IDLE_TOMMY,		false },
	{ ACT_MP_PRONE_IDLE,				ACT_HAJ_PRONE_AIM_TOMMY,		false },
	{ ACT_MP_PRONE_CRAWL,				ACT_HAJ_PRONEWALK_IDLE_TOMMY,	false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HAJ_CROUCH_IDLE_TOMMY,		false },
	{ ACT_MP_CROUCHWALK,				ACT_HAJ_CROUCHWALK_IDLE_TOMMY,	false },
};

IMPLEMENT_ACTTABLE(CWeaponThompson);

//=========================================================
CWeaponThompson::CWeaponThompson()
{
}

void CWeaponThompson::Spawn( void )
{
	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: rate of fire = "1/(R/60)" where R is rounds per minute
//-----------------------------------------------------------------------------
float CWeaponThompson::GetFireRate( void )
{
	// 700 rpm
	return 0.085f;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeaponThompson::Reload( void )
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
void CWeaponThompson::Precache( void )
{
	BaseClass::Precache();
}
