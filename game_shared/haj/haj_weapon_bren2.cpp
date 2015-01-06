//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: Bren 303 Mk2 weapon definition
// Notes: This basically duplicates the Mk1 only changine the entity name
//        and classname.
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "haj_weapon_bren.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
#define CWeaponBren2 C_WeaponBren2
#endif

class CWeaponBren2 : public CWeaponBren
{
public:
	DECLARE_CLASS( CWeaponBren2, CWeaponBren );

	CWeaponBren2();

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	DECLARE_ACTTABLE();

private:
	CWeaponBren2( const CWeaponBren2 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponBren2, DT_WeaponBren2 )

BEGIN_NETWORK_TABLE( CWeaponBren2, DT_WeaponBren2 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponBren2 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_bren2, CWeaponBren2 );
PRECACHE_WEAPON_REGISTER(weapon_bren2);

acttable_t	CWeaponBren2::m_acttable[] = 
{
	// attack
	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,			ACT_HAJ_PRIMARYATTACK_BREN,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,			ACT_HAJ_PRIMARYATTACK_BREN,	false },
	{ ACT_MP_ATTACK_PRONE_PRIMARYFIRE,			ACT_HAJ_PRIMARYATTACK_PRONE_BREN, false },

	// reload
	{ ACT_MP_RELOAD_STAND,						ACT_HAJ_RELOAD_BREN,			false },
	{ ACT_MP_RELOAD_CROUCH,						ACT_HAJ_RELOAD_BREN,			false },
	{ ACT_MP_RELOAD_PRONE,						ACT_HAJ_RELOAD_PRONE_BREN,		false },

	// aiming down sight
	{ ACT_HAJ_IDLE_AIM,							ACT_HAJ_STAND_AIM_BREN,			false },
	{ ACT_HAJ_WALK_AIM,							ACT_HAJ_WALK_AIM_BREN,			false },
	{ ACT_HAJ_IDLE_CROUCH_AIM,					ACT_HAJ_CROUCH_AIM_BREN,		false },
	{ ACT_HAJ_WALK_CROUCH_AIM,					ACT_HAJ_CROUCHWALK_IDLE_BREN,	false },
	{ ACT_HAJ_RUN_AIM,							ACT_HAJ_RUN_AIM_BREN,			false },

	// movement
	{ ACT_MP_STAND_IDLE,						ACT_HAJ_STAND_IDLE_BREN,		false },
	{ ACT_MP_RUN,								ACT_HAJ_RUN_IDLE_BREN,			false },
	{ ACT_MP_SPRINT,							ACT_HAJ_SPRINT_IDLE_BREN,		false },
	{ ACT_MP_WALK,								ACT_HAJ_WALK_IDLE_BREN,			false },
	{ ACT_MP_PRONE_IDLE,						ACT_HAJ_PRONE_AIM_BREN,			false },
	{ ACT_MP_PRONE_CRAWL,						ACT_HAJ_PRONEWALK_IDLE_BREN,	false },
	{ ACT_MP_CROUCH_IDLE,						ACT_HAJ_CROUCH_IDLE_BREN,		false },
	{ ACT_MP_CROUCHWALK,						ACT_HAJ_CROUCHWALK_IDLE_BREN,	false },
	
	// deploy
	{ ACT_HAJ_DEPLOY,							ACT_HAJ_DEPLOY_BREN,			false },
	{ ACT_HAJ_PRONE_DEPLOY,						ACT_HAJ_PRONE_DEPLOY_BREN,		false },
	{ ACT_HAJ_PRIMARYATTACK_DEPLOYED,			ACT_HAJ_PRIMARYATTACK_DEPLOYED_BREN, false },
	{ ACT_HAJ_PRIMARYATTACK_PRONE_DEPLOYED,		ACT_HAJ_PRIMARYATTACK_PRONE_DEPLOYED_BREN,		false },

};

IMPLEMENT_ACTTABLE(CWeaponBren2);

//=========================================================
CWeaponBren2::CWeaponBren2()
{
	m_fMinRange1		= 0;// No minimum range. 
	m_fMaxRange1		= 1400;
}
