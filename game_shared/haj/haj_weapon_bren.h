//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: Bren MK I base class definition
//
// $NoKeywords: $
//=======================================================================//
#include "weapon_hl2mpbase.h"
//#include "weapon_hl2mpbase_machinegun.h"
#include "haj_weapon_base.h"

#ifndef HAJWEAPONSTEN_H
#define HAJWEAPONSTEN_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
#define CWeaponBren C_WeaponBren
#endif

// Define the basic Sten class
class CWeaponBren : public CHAJWeaponBase
{
public:
	DECLARE_CLASS( CWeaponBren, CHAJWeaponBase );

	CWeaponBren();

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	int				GetMinBurst() { return 2; }
	int				GetMaxBurst() { return 5; }

	const char		*GetTracerType( void ) { return "303Tracer"; }

	bool			Reload( void );
	virtual void	Spawn();

	float			GetFireRate( void );
	Activity		GetPrimaryAttackActivity( void );

private:
	const WeaponProficiencyInfo_t *GetProficiencyValues();

	DECLARE_ACTTABLE();

private:

	CWeaponBren( const CWeaponBren & );
};

#endif // HAJWEAPONSTEN_H