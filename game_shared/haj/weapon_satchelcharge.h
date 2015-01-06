//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: Satchel Charge Weapon
// Notes: Used for blowing up stuff, like secondary objectives
// $NoKeywords: $
//=======================================================================//

#ifndef HAJ_SATCHEL_WEAP
#define HAJ_SATCHEL_WEAP

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#include "haj_player_c.h"
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui_controls/Controls.h>
#else
#include "haj_player.h"
#include "haj_grenade_tnt.h"
#endif

#include "haj_gamerules.h"
#include "weapon_hl2mpbase.h"
#include "haj_weapon_base.h"
#include "weapon_satchelcharge.h"

#ifdef CLIENT_DLL
#define CWeaponSatchelCharge C_WeaponSatchelCharge
#else
class CGrenadeTNT;
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponSatchelCharge : public CHAJWeaponBase
{
public:
	DECLARE_CLASS( CWeaponSatchelCharge, CHAJWeaponBase );

	CWeaponSatchelCharge();

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	void				ItemPostFrame( void );

	virtual bool		CanPlant( void );
	virtual float		GetPlantTime( void );

	virtual void		StartPlant( void );
	virtual void		FinishPlant( void );
	virtual void		PlantThink( void );
	virtual void		PlantAbort( void );

	virtual void		SetViewModel();
	virtual int			GetWorldModelIndex( void );

#ifndef CLIENT_DLL
	virtual void		SetDefusing( CGrenadeTNT *pTNT );
#endif
	virtual void		FinishDefusing( void );
	virtual void		AbandonDefuse( void );

	virtual bool		AllowsAutoSwitchFrom( void ) const { if( m_bDefusing ) { return false; } return true; };

	virtual bool		Holster( CBaseCombatWeapon *pSwitchingTo );
	void				Drop( const Vector &vecVelocity );

	virtual void		FakeHolster();
	virtual void		FakeDeploy();
	virtual Activity	GetDrawActivity( void );

	virtual void		Precache();

	virtual bool		HasPrimaryAmmo();
	virtual bool		Deploy();

	virtual float		GetPlayerSpeedMultiplier( void );

	void				SetPlantingWithUse( bool b ) { m_bPlantingWithUse = b; }

	DECLARE_ACTTABLE();

#ifdef CLIENT_DLL
	virtual void		Redraw();
	virtual bool		ShouldForceTextureCrosshair( void ) { return true; }
#else
	void				SetLastWeaponWithUse( CBaseCombatWeapon* pWeap ) { m_hLastInvRestore.Set( pWeap ); }
#endif

	float   GetSpreadScalar() { return 3.5f; }
	bool	IsDefusing() { return m_bDefusing; }

protected:

	CNetworkVar( bool, m_bPlanting );
	CNetworkVar( float, m_flPlantStartTime );
	CNetworkVar( float, m_flPlantEndTime );

	CNetworkVar( bool, m_bDefusing ); // in defuse mode

	CNetworkVar( bool, m_bPlantingWithUse );

	bool m_bInstantHolster;

#ifndef CLIENT_DLL
	CGrenadeTNT *m_pDefusee;
	CHandle<CBaseCombatWeapon> m_hLastInvRestore;
#endif

private:
	CWeaponSatchelCharge( const CWeaponSatchelCharge & );
};

#endif