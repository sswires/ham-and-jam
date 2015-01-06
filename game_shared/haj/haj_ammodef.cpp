
// haj_ammodef.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "ammodef.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/////////////////////////////////////////////////////////////////////////////
// defines
#define BULLET_MASS_GRAINS_TO_LB(grains)	(0.002285*(grains)/16.0f)
#define BULLET_MASS_GRAINS_TO_KG(grains)	lbs2kg(BULLET_MASS_GRAINS_TO_LB(grains))

// exaggerate all of the forces, but use real numbers to keep them consistent
#define BULLET_IMPULSE_EXAGGERATION			3.5
// convert a velocity in ft/sec and a mass in grains to an impulse in kg in/s
#define BULLET_IMPULSE(grains, ftpersec)	((ftpersec)*12*BULLET_MASS_GRAINS_TO_KG(grains)*BULLET_IMPULSE_EXAGGERATION)

/////////////////////////////////////////////////////////////////////////////s
CAmmoDef *GetAmmoDef()
{
	static CAmmoDef def;
	static bool bInitted = false;
	
	if ( !bInitted )
	{
		bInitted = true;

		def.AddAmmoType("slam",				DMG_BURN,					TRACER_NONE,			0,			0,			5,			0,							0 );

		// *HAJ 020* - Jed
		// HaJ ammo types
		//				name,				dmgtype,	tracer,					pldmg,	npcdmg,		carry,		magsize,	force,						flags
		def.AddAmmoType("303",				DMG_BULLET,	TRACER_LINE_AND_WHIZ,	0,		0,			10,			5,			BULLET_IMPULSE(174, 2440),	AMMO_USE_MAGAZINES); // Lee-Enfield
		def.AddAmmoType("303_tracer",		DMG_BULLET,	TRACER_LINE_AND_WHIZ,	0,		0,			2,			30,			BULLET_IMPULSE(174, 2440),	AMMO_USE_MAGAZINES); // Bren
		def.AddAmmoType("9mm",				DMG_BULLET,	TRACER_LINE_AND_WHIZ,	0,		0,			4,			32,			BULLET_IMPULSE(147, 980),	AMMO_USE_MAGAZINES); // MP40, Sten
		def.AddAmmoType("45acp",			DMG_BULLET,	TRACER_LINE_AND_WHIZ,	0,		0,			4,			20,			BULLET_IMPULSE(147, 700),	AMMO_USE_MAGAZINES); // Thompson
		def.AddAmmoType("792mm",			DMG_BULLET,	TRACER_LINE_AND_WHIZ,	0,		0,			5,			5,			BULLET_IMPULSE(197, 2600),	AMMO_USE_MAGAZINES); // Kar98k
		def.AddAmmoType("792mm_tracer",		DMG_BULLET,	TRACER_LINE_AND_WHIZ,	0,		0,			2,			50,			BULLET_IMPULSE(197, 2600),	AMMO_USE_MAGAZINES); // MG34

		// grenades
		def.AddAmmoType("mills_grenade",	DMG_BURN,					TRACER_NONE,			0,			0,			5,			0,							0 );
		def.AddAmmoType("stick_grenade",	DMG_BURN,					TRACER_NONE,			0,			0,			5,			0,							0 );
		def.AddAmmoType("smoke_grenade",	DMG_BURN,					TRACER_NONE,			0,			0,			5,			0,							0 );
	}

	return &def;
}

/////////////////////////////////////////////////////////////////////////////
