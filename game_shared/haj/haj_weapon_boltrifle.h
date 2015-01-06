//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose:  HaJ base machine gun class
// Notes:    This is based off the HaJ machine class and basically acts
//			 as a bolt action rifle which uses magazines. Mostly it's a 
//			 case of modifying the time you can next fire to use the
//			 animation length (which cycles the bolt) than a pre-determined
//			 rate of fire.
//
// $NoKeywords: $
//=======================================================================//
#include "weapon_hl2mpbase.h"

#ifndef HAJWEAPONBOLTRIFLE_H
#define HAJWEAPONBOLTRIFLE_H
#ifdef _WIN32
#pragma once
#endif

#include "haj_weapon_base.h"

#define HAJ_FOV_RIFLES 55

#if defined( CLIENT_DLL )
#define CHAJBoltRifle C_HAJBoltRifle
#endif

//=========================================================
// Bolt Rifle base class
//=========================================================
class CHAJBoltRifle : public CHAJWeaponBase
{
public:
	DECLARE_CLASS( CHAJBoltRifle, CHAJWeaponBase );
	DECLARE_DATADESC();

	CHAJBoltRifle();
	
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	// we override these because we need to handle ammo differently
	// from the baseweapon class's normal way.
	virtual bool	CanReload();
	void			PrimaryAttack( void );
	void			SecondaryAttack( void );
	Activity		GetPrimaryAttackActivity( void );
	void			WeaponIdle( void );
	bool			Holster( CBaseCombatWeapon *pSwitchingTo = NULL );

	virtual void	Drop( const Vector &vecVelocity );

	virtual void	ItemPostFrame( void );
	virtual void	CockWeapon( void );
	virtual void	FinishCocking( void );

	virtual void	FakeHolster();

	virtual bool	IsWeaponBusy();


#ifdef CLIENT_DLL
	virtual float	SwayScale( void );
#endif

	CNetworkVar( bool, m_bSighted);
	CNetworkVar( bool, m_bWasSighted );
	CNetworkVar( bool, m_bNeedsCocking );
	CNetworkVar( bool, m_bIsCocking );

	float m_flFinishCockingTime;

private:
	
	CHAJBoltRifle( const CHAJBoltRifle & );

};

#endif // HAJWEAPONBOLTRIFLE_H