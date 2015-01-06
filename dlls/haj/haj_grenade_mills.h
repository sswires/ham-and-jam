//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: NPC/World mills grenade class
//
// $NoKeywords: $
//=======================================================================//

#ifndef GRENADE_MILLS_H
#define GRENADE_MILLS_H
#pragma once

#include "basegrenade_shared.h"
#include "haj_grenade_base.h"

class CGrenadeMills : public CHAJGrenade
{
	DECLARE_CLASS( CGrenadeMills, CHAJGrenade );

#if !defined( CLIENT_DLL )
	DECLARE_DATADESC();
#endif
					
	~CGrenadeMills( void );

public:
	virtual const char*		GetGrenadeModel( void );

protected:
	
	bool	m_inSolid;
};

CBaseGrenade *MillsGrenade_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer );

#endif // GRENADE_MILLS_H
