//========= Copyright © 2009, Ham and Jam. ==============================//
// Purpose: NPC/World base grenade class
//
// $NoKeywords: $
//=======================================================================//

#ifndef GRENADE_HAJBASE_H
#define GRENADE_HAJBASE_H
#pragma once

#include "basegrenade_shared.h"

class CBaseGrenade;
struct edict_t;

class CHAJGrenade : public CBaseGrenade
{
	DECLARE_CLASS( CHAJGrenade, CBaseGrenade );

#if !defined( CLIENT_DLL )
	DECLARE_DATADESC();
#endif

public:

	CHAJGrenade( void );
	~CHAJGrenade( void );

	virtual const char*		GetGrenadeModel( void );
	void					Spawn( void );
	void					OnRestore( void );
	void					Precache( void );
	bool					CreateVPhysics( void );
	void					CreateEffects( void );
	void					SetTimer( float detonateDelay, float warnDelay );
	void					SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity );
	int						OnTakeDamage( const CTakeDamageInfo &inputInfo );
	void					DelayThink();
	virtual void			ResolveFlyCollisionCustom( trace_t &trace, Vector &vecVelocity );

	virtual void			VPhysicsUpdate( IPhysicsObject *pPhysics );
	virtual void			VPhysicsCollision( int index, gamevcollisionevent_t *pEvent );

	bool	m_bImpactFuze;

protected:
	
	bool	m_inSolid;
};

#endif // GRENADE_HAJBASE_H
