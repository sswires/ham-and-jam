//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose:  HaJ base machine gun class
// Notes:    This replaces the default HL2MP machine gun class and adds
//           the feaure of magazine based ammo management.
//
//           The magazines/ammo is handled locally in the weapon rather
//			 than taking ammo from the player "buckets".
//			
//			 Most of these functions in the class simply override the
//			 defaults to get around the bucket code and use the magazine
//			 stuff instead.
// $NoKeywords: $
//=======================================================================//
#include "weapon_hl2mpbase.h"

#ifndef HAJWEAPONMACHINEGUN_H
#define HAJWEAPONMACHINEGUN_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_hl2mpbase.h"
#include "weapon_hl2mpbase_machinegun.h"

#if defined( CLIENT_DLL )
#define CHAJMachineGun C_HAJMachineGun
#endif

// defines how many extra clips maximum , excluding the one in the weapon we can carry.
#define MAX_CLIPS_PER_WEAPON	8

//=========================================================
// Machine gun base class
//=========================================================
class CHAJMachineGun : public CWeaponHL2MPBase
{
public:
	DECLARE_CLASS( CHAJMachineGun, CWeaponHL2MPBase );
	DECLARE_DATADESC();

	CHAJMachineGun();
	
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	// we override these because we need to handle ammo differently
	// from the baseweapon class's normal way.
	void			PrimaryAttack( void );
	bool			GetNextMagazine( void );		// returns the index of the next fullest magazine
	int				MagsLeft( void );				// returns how many magazines we have spare that contain ammo
	void			PopulateMagazines ( int m );	// fix m of our spare mags to capacity

	// temp function used for COF testing
	Vector			GetCoFVector( unsigned int coneangle );

	// Default calls through to m_hOwner, but plasma weapons can override and shoot projectiles here.
	virtual void	ItemPostFrame( void );
	virtual void	FireBullets( const FireBulletsInfo_t &info );
	virtual bool	Deploy( void );
	
	virtual const	Vector &GetBulletSpread( void );

	int				WeaponSoundRealtime( WeaponSound_t shoot_type );

	// Makes our weapon model "bob" when moving
	virtual void	AddViewmodelBob( CBaseViewModel *viewmodel, Vector &origin, QAngle &angles );
	virtual	float	CalcViewmodelBob( void );

	// utility function
	static void		DoMachineGunKick( CBasePlayer *pPlayer, float dampEasy, float maxVerticleKickAngle, float fireDurationTime, float slideLimitTime );

	// Reloading - overriding defaults
	virtual bool	ReloadOrSwitchWeapons( void );
	virtual bool	DefaultReload( int iClipSize1, int iClipSize2, int iActivity );
	virtual void	FinishReload( void );
	virtual void	CheckReload( void );
	virtual bool	HasPrimaryAmmo( void );

private:
	
	CHAJMachineGun( const CHAJMachineGun & );

protected:

	int				m_nShotsFired;		// Number of consecutive shots fired
	float			m_flNextSoundTime;	// real-time clock of when to make next sound

	CNetworkArray(	int, m_iAmmoInClip, MAX_CLIPS_PER_WEAPON );	// how much ammo in each spare magazine
	int				m_iCurrentClip;		// holds the index of the next magazine to use.
};

#endif // HAJWEAPONMACHINEGUN_H