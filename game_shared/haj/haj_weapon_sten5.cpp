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

#include "haj_weapon_sten.h"

#ifdef CLIENT_DLL
#define CWeaponSten5 C_WeaponSten5
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponSten5 : public CWeaponSten
{
public:
	DECLARE_CLASS( CWeaponSten5, CWeaponSten );

	CWeaponSten5();

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	virtual void		Precache( void );
	virtual float		GetFireRate( void );

	DECLARE_ACTTABLE();

private:
	CWeaponSten5( const CWeaponSten5 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSten5, DT_WeaponSten5 )

BEGIN_NETWORK_TABLE( CWeaponSten5, DT_WeaponSten5 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponSten5 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_sten5, CWeaponSten5 );
PRECACHE_WEAPON_REGISTER(weapon_sten5);

acttable_t	CWeaponSten5::m_acttable[] = 
{
	// attack
	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_STEN5,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_STEN5,	false },
	{ ACT_MP_ATTACK_PRONE_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_PRONE_STEN5, false },

	// reload
	{ ACT_MP_RELOAD_STAND,				ACT_HAJ_RELOAD_STEN5,			false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HAJ_RELOAD_STEN5,			false },
	{ ACT_MP_RELOAD_PRONE,				ACT_HAJ_RELOAD_PRONE_STEN5,		false },

	// aiming down sight
	{ ACT_HAJ_IDLE_AIM,					ACT_HAJ_STAND_AIM_STEN5,		false },
	{ ACT_HAJ_WALK_AIM,					ACT_HAJ_WALK_AIM_STEN5,			false },
	{ ACT_HAJ_IDLE_CROUCH_AIM,			ACT_HAJ_CROUCH_AIM_STEN5,		false },
	{ ACT_HAJ_WALK_CROUCH_AIM,			ACT_HAJ_CROUCHWALK_IDLE_STEN5,	false },
	{ ACT_HAJ_RUN_AIM,					ACT_HAJ_RUN_AIM_STEN5,			false },

	// movement
	{ ACT_MP_STAND_IDLE,				ACT_HAJ_STAND_IDLE_STEN5,		false },
	{ ACT_MP_RUN,						ACT_HAJ_RUN_IDLE_STEN5,			false },
	{ ACT_MP_SPRINT,					ACT_HAJ_SPRINT_IDLE_STEN5,		false },
	{ ACT_MP_WALK,						ACT_HAJ_WALK_IDLE_STEN5,		false },
	{ ACT_MP_PRONE_IDLE,				ACT_HAJ_PRONE_AIM_STEN5,		false },
	{ ACT_MP_PRONE_CRAWL,				ACT_HAJ_PRONEWALK_IDLE_STEN5,	false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HAJ_CROUCH_IDLE_STEN5,		false },
	{ ACT_MP_CROUCHWALK,				ACT_HAJ_CROUCHWALK_IDLE_STEN5,	false },
};

IMPLEMENT_ACTTABLE(CWeaponSten5);

//=========================================================
CWeaponSten5::CWeaponSten5()
{
}

//-----------------------------------------------------------------------------
// Purpose: rate of fire = "1/(R/60)" where R is rounds per minute
//-----------------------------------------------------------------------------
float CWeaponSten5::GetFireRate( void )
{
	// 500 rpm
	return 0.12f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSten5::Precache( void )
{
	BaseClass::Precache();
}
