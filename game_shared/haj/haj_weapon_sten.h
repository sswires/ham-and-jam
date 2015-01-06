//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: Sten 9mm base class definition
//
// $NoKeywords: $
//=======================================================================//
#ifndef HAJWEAPONSTEN_H
#define HAJWEAPONSTEN_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_hl2mpbase.h"
//#include "weapon_hl2mpbase_machinegun.h"
#include "haj_weapon_base.h"

#ifdef CLIENT_DLL
#define CWeaponSten C_WeaponSten
#endif

// Define the basic Sten class
class CWeaponSten : public CHAJWeaponBase
{
public:
	DECLARE_CLASS( CWeaponSten, CHAJWeaponBase );

	CWeaponSten();

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	void	Precache( void );
	void	SecondaryAttack( void );

	int		GetMinBurst() { return 2; }
	int		GetMaxBurst() { return 5; }

	const char *GetTracerType( void ) { return "WhizTracer"; }

	virtual void	Equip( CBaseCombatCharacter *pOwner );
	bool			Reload( void );
	virtual void	Spawn();

	float			GetFireRate( void );
	virtual float	GetMaxPenetrationDistance( unsigned short surfaceType );

protected:
	Vector	m_vecTossVelocity;
	
private:
	const WeaponProficiencyInfo_t *GetProficiencyValues();

	DECLARE_ACTTABLE();

private:

	CWeaponSten( const CWeaponSten & );
};

#endif // HAJWEAPONSTEN_H