//========= Copyright © 2006, Ham and Jam, All rights reserved. ============//
//
// Purpose:		Projectile shot from and artillery cannon
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef	SHELLARTILLERY_H
#define	SHELLARTILLERY_H

#include "basegrenade_shared.h"

#define	MAX_SHELL_NO_COLLIDE_TIME 0.2

class SmokeTrail;
//class CWeaponAR2; //???

class CShellArtillery : public CBaseGrenade
{
public:
	DECLARE_CLASS( CShellArtillery, CBaseGrenade );

	CHandle< SmokeTrail >	m_hSmokeTrail;
	float					m_fSpawnTime;
	float					m_fDangerRadius;

	void		Spawn( void );
	void		Precache( void );
	void 		ShellTouch( CBaseEntity *pOther );
	void		ShellThink( void );
	void		Event_Killed( const CTakeDamageInfo &info );

public:
	void EXPORT				Detonate(void);
	CShellArtillery(void);

	DECLARE_DATADESC();
};

#endif	//SHELLARTILLERY_H
