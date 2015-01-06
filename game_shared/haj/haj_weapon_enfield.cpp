//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: Enfield 303 weapon definition
// Note:	The Enfield can only be reloaded with an empty or half full
//			clip.
//			The magazines for the Enfield are 10 rounds so we use
//			10 round clips in our ammo system but when reloading only
//			take 5 from them each time to reload.
//			We also override some functions to report how many "half-clips"
//			you have left as a 10 round magazine = 2x5 round clips.
//			I hope you follow!
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "datacache/imdlcache.h"
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
#define CWeaponEnfield C_WeaponEnfield
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//console variables
ConVar haj_enfield_cof_stand( "haj_enfield_cof_stand", "3.28", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED, "Min Standing CoF angle - enfield" );
ConVar haj_enfield_cof_crouch( "haj_enfield_cof_crouch", "2.46", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED, "Min Crouched CoF angle - enfield" );
ConVar haj_enfield_cof_prone( "haj_enfield_cof_prone", "1.64", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED, "Min Prone CoF angle - enfield" );
ConVar haj_enfield_cof_ironsights( "haj_enfield_cof_ironsights", "0.15", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED, "Ironsight CoF multiplier" );
ConVar haj_enfield_move_accuracy( "haj_enfield_move_accuracy", "1.4", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED, "What to multiply spread by when moving at max speed - Enfield" );

class CWeaponEnfield : public CHAJBoltRifle
{
public:
	DECLARE_CLASS( CWeaponEnfield, CHAJBoltRifle );

	CWeaponEnfield();

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	
	void			Precache( void );
	void			AddViewKick( void );
	void			SecondaryAttack( void );

	const char		*GetTracerType( void ) { return "WhizTracer"; }
	virtual void	Equip( CBaseCombatCharacter *pOwner );

	virtual bool	DefaultReload( int iClipSize1, int iActivityEmpty, int iActivityFull );
	virtual void	FinishReload( void );
	bool			Reload( void );

	virtual void	Spawn();

	float			GetFireRate( void ) { return 1.0f; }
	Activity		GetPrimaryAttackActivity( void );

	const WeaponProficiencyInfo_t *GetProficiencyValues();

	DECLARE_ACTTABLE();
	
private:
	CWeaponEnfield( const CWeaponEnfield & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponEnfield, DT_WeaponEnfield )

BEGIN_NETWORK_TABLE( CWeaponEnfield, DT_WeaponEnfield )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponEnfield )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_enfield, CWeaponEnfield );
PRECACHE_WEAPON_REGISTER(weapon_enfield);

acttable_t	CWeaponEnfield::m_acttable[] = 
{
	// attack
	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_ENFIELD,		false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_ENFIELD,		false },
	{ ACT_MP_ATTACK_PRONE_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_PRONE_ENFIELD, false },

	// reload
	{ ACT_MP_RELOAD_STAND,				ACT_HAJ_RELOAD_ENFIELD,				false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HAJ_RELOAD_ENFIELD,				false },
	{ ACT_MP_RELOAD_PRONE,				ACT_HAJ_RELOAD_PRONE_ENFIELD,		false },

	// aiming down sight
	{ ACT_HAJ_IDLE_AIM,					ACT_HAJ_STAND_AIM_ENFIELD,			false },
	{ ACT_HAJ_WALK_AIM,					ACT_HAJ_WALK_AIM_ENFIELD,			false },
	{ ACT_HAJ_IDLE_CROUCH_AIM,			ACT_HAJ_CROUCH_AIM_ENFIELD,			false },
	{ ACT_HAJ_WALK_CROUCH_AIM,			ACT_HAJ_CROUCHWALK_IDLE_ENFIELD,	false },
	{ ACT_HAJ_RUN_AIM,					ACT_HAJ_RUN_AIM_ENFIELD,			false },

	// movement
	{ ACT_MP_STAND_IDLE,				ACT_HAJ_STAND_IDLE_ENFIELD,			false },
	{ ACT_MP_RUN,						ACT_HAJ_RUN_IDLE_ENFIELD,			false },
	{ ACT_MP_SPRINT,					ACT_HAJ_SPRINT_IDLE_ENFIELD,		false },
	{ ACT_MP_WALK,						ACT_HAJ_WALK_IDLE_ENFIELD,			false },
	{ ACT_MP_PRONE_IDLE,				ACT_HAJ_PRONE_AIM_ENFIELD,			false },
	{ ACT_MP_PRONE_CRAWL,				ACT_HAJ_PRONEWALK_IDLE_ENFIELD,		false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HAJ_CROUCH_IDLE_ENFIELD,		false },
	{ ACT_MP_CROUCHWALK,				ACT_HAJ_CROUCHWALK_IDLE_ENFIELD,	false },
};

IMPLEMENT_ACTTABLE(CWeaponEnfield);

//-----------------------------------------------------------------------------
// Purpose: Construtor
//-----------------------------------------------------------------------------
CWeaponEnfield::CWeaponEnfield( )
{
	m_fMinRange1		= 0;// No minimum range. 
	m_fMaxRange1		= 1400;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponEnfield::Precache( void )
{
	BaseClass::Precache();
}


//-----------------------------------------------------------------------------
// Purpose: Give this weapon longer range when wielded by an ally NPC.
//-----------------------------------------------------------------------------
void CWeaponEnfield::Equip( CBaseCombatCharacter *pOwner )
{
	BaseClass::Equip( pOwner );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CWeaponEnfield::GetPrimaryAttackActivity( void )
{
	return BaseClass::GetPrimaryAttackActivity();
}


//-----------------------------------------------------------------------------
// Purpose: Called when we reload (duh!)
//-----------------------------------------------------------------------------
bool CWeaponEnfield::Reload( void )
{
	// The Enfield has a 10 round magazine. Only allow a reload when it's
	// empty or half full.
	if ( !CanReload() || m_iClip1 > 5 )
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
// Purpose: Vertical recoil
//-----------------------------------------------------------------------------

//console variables
ConVar haj_enfield_rvmin( "haj_enfield_rvmin", "-10.0", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED, "Minimum vertical recoil - Enfield" );
ConVar haj_enfield_rvmax( "haj_enfield_rvmax", "-5.0", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED, "Maximum vertical recoil - Enfield" );
ConVar haj_enfield_rhmin( "haj_enfield_rhmin", "-0.5", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED, "Minimum horizontal recoil - Enfield" );
ConVar haj_enfield_rhmax( "haj_enfield_rhmax", "0.5", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED, "Maximum horizontal recoil - Enfield" );

void CWeaponEnfield::AddViewKick( void )
{
#ifdef CLIENT_DLL
	// default values
	/*
	float recoil_min_vert = -1.5f;
	float recoil_max_vert = -0.5f;
	float recoil_min_horz = -0.1f;
	float recoil_max_horz =  0.1f;
	*/

	// use console vars
	// TODO: Revert before release!
	float recoil_min_vert = haj_enfield_rvmin.GetFloat();
	float recoil_max_vert = haj_enfield_rvmax.GetFloat();
	float recoil_min_horz = haj_enfield_rhmin.GetFloat();
	float recoil_max_horz = haj_enfield_rhmax.GetFloat();

	DoRecoil (recoil_min_vert, recoil_max_vert, recoil_min_horz, recoil_max_horz );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Secondary attack
//-----------------------------------------------------------------------------
void CWeaponEnfield::SecondaryAttack( void )
{
	// don't much else than go bang bang...
	BaseClass::SecondaryAttack();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CWeaponEnfield::GetProficiencyValues()
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
void CWeaponEnfield::Spawn( void )
{
	BaseClass::Spawn();
}


//-----------------------------------------------------------------------------
// Purpose: Override the default reload function
//-----------------------------------------------------------------------------
bool CWeaponEnfield::DefaultReload( int iClipSize1, int iActivityEmpty, int iActivityFull )
{
	return BaseClass::DefaultReload( iClipSize1, iActivityEmpty, iActivityFull );
}


//-----------------------------------------------------------------------------
// Purpose: Reload has finished. Actuall updating of ammo happens here.
//-----------------------------------------------------------------------------
void CWeaponEnfield::FinishReload( void )
{
	CHajPlayer *pOwner = ToHajPlayer(GetOwner());

	if ( pOwner && m_bInReload )
	{

#ifdef GAME_DLL
		if( m_bQueuedIronsights )
		{
			SetIronsights( true );
			m_bQueuedIronsights = false;
		}

		CHajPlayer *pPlayer = ToHajPlayer( pOwner );

		if( pPlayer && m_bIsBipodDeployed )
			pPlayer->SetFOV( this, 75, 0.3 );

		// get point to magazine instance of weapon's ammo type
		CHajWeaponMagazines *pMagazines = pOwner->GetMagazines( m_iPrimaryAmmoType );

		if( pMagazines )
		{
			if( m_iClip1 <= 0 ) // empty
			{
				// two mags
				pMagazines->UpdateCurrent( m_iClip1 );
				m_iClip1 = pMagazines->SwitchToBest();
				
				pMagazines->UpdateCurrent( 0 );
				m_iClip1 += pMagazines->SwitchToBest();
			}
			else // less than 5 left
			{
				pMagazines->UpdateCurrent(0);
				m_iClip1 += pMagazines->SwitchToBest();
			}
		}
#endif

		m_bInReload = false;
		return; // skip base class
	}
}
