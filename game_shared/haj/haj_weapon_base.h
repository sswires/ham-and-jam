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
#ifndef HAJWEAPONBASE_H
#define HAJWEAPONBASE_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_hl2mpbase.h"

#if defined( CLIENT_DLL )
	#define CHAJWeaponBase C_HAJWeaponBase
#endif

// defines how many extra clips maximum , excluding the one in the weapon we can carry.
#define MAX_CLIPS_PER_WEAPON	8

//=========================================================
// HaJ Gun base class
//=========================================================
class CHAJWeaponBase : public CWeaponHL2MPBase
{
public:
	DECLARE_CLASS( CHAJWeaponBase, CWeaponHL2MPBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CHAJWeaponBase();

#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif

	// All predicted weapons need to implement and return true
	virtual bool	IsPredicted() const { return true; }

#ifdef CLIENT_DLL
	virtual bool	ShouldPredict();
#endif

	virtual int		GetWorldModelIndex( void );

	// override to play custom empty sounds
	virtual bool	PlayEmptySound();

	

#ifdef GAME_DLL
	// virtual void	SendReloadEvents();
#endif

	// Default calls through to m_hOwner, but plasma weapons can override and shoot projectiles here.
	virtual void	ItemPostFrame( void );
	virtual bool	Deploy( void );

	virtual bool	AllowsAutoSwitchFrom( void ) const { return false; };

	virtual float	GetSpreadScalar( void );
	virtual const	Vector &GetBulletSpread( void );
	virtual void	AddViewKick( void );
	virtual void	GetRecoilValues( float &minVert, float &maxVert, float &minHoriz, float &maxHoriz, float &ironsightMulti );

	virtual void	DoMuzzleFlash();	// Force a muzzle flash event. Note: this only QUEUES an event, so

	virtual bool	IsWeaponBusy() { return m_bInReload; }

	// *HaJ 020* - SteveUk
	// Get the maximum distance that this weapon is effective
	virtual float			GetEfficientDistance();
	virtual void			SetEfficientDistance( float );

	float			GetBulletFadeFactor();
	void			SetBulletFadeFactor( float f ) { m_fBulletFadeFactor = f; };

	virtual bool	HasFinishedHolstering( void );
	void			SetHolsterTime( float f ) { m_flHolsterTime = f; }
	virtual bool	HolsterTimeThink( void );

#ifdef CLIENT_DLL
	// Makes our weapon model "bob" when moving
	virtual void	AddViewmodelBob( CBaseViewModel *viewmodel, Vector &origin, QAngle &angles );
	virtual float	SwayScale( void );
	virtual	float	CalcViewmodelBob( void );

	virtual int		DrawModel( int flags );

	virtual float	BobScale( void );
	virtual void	Redraw();
	virtual bool	ShouldForceTextureCrosshair( void ) { return false; }

protected:
	vgui::HFont		m_hInfoFont;

public:
#endif

	// Recoil
	void			DoRecoil( float min_vert, float max_vert, float min_horz, float max_horz );
	Activity		GetPrimaryAttackActivity( void );
	
	// Reloading - overriding defaults
	virtual bool	Reload( void );
	virtual bool	ReloadOrSwitchWeapons( void );
	//virtual bool	DefaultReload( int iClipSize1, int iClipSize2, int iActivity );
	bool			DefaultReload( int iClipSize1, int iActivityEmpty, int iActivityFull );
	virtual void	FinishReload( void );
	virtual void	CheckReload( void );
	virtual bool	HasPrimaryAmmo( void );
	virtual bool	CanReload( void ) { return true; }

	// we override these because we need to handle ammo differently from the baseweapon class's normal way.
	void			PrimaryAttack( void );
	virtual float	GetMaxPenetrationDistance( unsigned short surfaceType );

	virtual bool	Ready( void );
	virtual bool	Lower( void );

	virtual void	Equip( CBaseCombatCharacter *pOwner );
	virtual void	Drop( const Vector &vecVelocity );

#ifdef GAME_DLL
	virtual CBaseEntity* GetLastOwner( void ) { return (CBaseEntity*)m_hOldOwner; }
	virtual void SetLastOwner( CBaseEntity *pOldOwner ) { Msg( "Set an old owner\n" ); m_hOldOwner = pOldOwner; }
#endif

	// mouse override to slow movement when prone
	virtual void	OverrideMouseInput( float *x, float *y );

	// Cone of Fire functions
	Vector			GetCoFVector( float coneangle );

	int				WeaponSoundRealtime( WeaponSound_t shoot_type );

	// Bipod related stuff
	virtual bool	HasBipod( void ) { return GetHL2MPWpnData().m_bUsesBipod; }
	void			TryBipod( void );
	bool			CanDeployBipod( bool bUsesTransition  ); // *HAJ 020* - Ztormi
	void			DeployTransition( void ); // *HAJ 020* - Ztormi
	float			GetDeployHeight( void );
	void			UnlockBipod( void );
	void			HandleBipodDeployment ( void );
	virtual void	WeaponIdle( void );
	virtual bool	IsLowered( void );

	virtual float	GetPlayerSpeedMultiplier( void );

	CNetworkVar		( bool, m_bIsBipodDeployed );
	CNetworkVar		( bool, m_bDoBipodDeploy );
	
	CNetworkVar		( bool, m_bIsBipodDeploying );
	CNetworkVar		( bool, m_bIsBipodUnDeploying );

	CNetworkVar		( bool, m_bIronsighted );
	CNetworkVar		( float, m_flIronsightTime );

	//Transition stuff
	CNetworkVar		( bool, m_bInDeployTransition );
	CNetworkVar		( float, m_flTransitionHeight );
	CNetworkVar		( float, m_flStartviewHeight );
	CNetworkVar		( float, m_flTransitionTime );
	CNetworkVar		( float, m_flTransitionStartTime );

	CNetworkVar		( float, m_flReloadEndTime );

	CNetworkVar		( bool, m_bAwaitingHolster );
	CNetworkVar		( bool, m_bQueuedIronsights );
	
	float			m_fBipodDeployTime;
	float			m_fBipodUnDeployTime;

	// Ironsight funcs
	virtual bool	HasIronsights( void );
	float			GetIronsightFOV( void );
	float			GetIronsightTime( void );
	Vector			GetIronsightPosition( void ) const;
	QAngle			GetIronsightAngles( void ) const;
	bool			IsIronsighted( void ) { return m_bIronsighted; }
	void			SetIronsights( bool b );
	void			ToggleIronsights( void ) { SetIronsights( !m_bIronsighted ); }
	void			EnterIronsights( void );
	void			ExitIronsights( void );

	virtual bool	Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual void	FakeHolster();
	virtual void	FakeDeploy();
	virtual void	DelayedSwitch( void );

	bool			IsHolstered( ) { return ( m_bHolstered || m_bFakeHolstered ); }

	virtual void	GetViewModelOffset( Vector& pos, QAngle& ang );
	virtual Vector	GetMoveOffset( void ) const;

protected:
	// holster
	CNetworkVar( bool, m_bFakeHolstered );
	float					m_fEfficientDistance; // *HaJ 020* - SteveUK - the efficient distance of the weapon
	Vector					m_vecSpread;

	CNetworkVar( float,		m_flHolsterTime );

private:
	CHAJWeaponBase( const CHAJWeaponBase & );

	CNetworkVar( bool, m_bHolstered );

	bool			m_bShouldSwitchGuns;
	CBaseCombatWeapon * m_pSwitchTo;
	float				m_flSwitchTime;

protected:
#ifdef GAME_DLL
	EHANDLE m_hOldOwner;
#endif

	int				m_nShotsFired;		// Number of consecutive shots fired
	float			m_flNextSoundTime;	// real-time clock of when to make next sound

	float			m_fBulletFadeFactor;	// User to extend the bullet fade curve
	CNetworkVar( bool,	m_bLowered );
	CNetworkVar( float,	m_flRaiseTime );
};

#endif