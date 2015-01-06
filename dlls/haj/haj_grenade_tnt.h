//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: NPC/TNT Satchel charge
//
// $NoKeywords: $
//=======================================================================//

#ifndef GRENADE_TNT_H
#define GRENADE_TNT_H
#pragma once

#include "basegrenade_shared.h"

class CBaseGrenade;
class CBombZone;
class CHajPlayer;

struct edict_t;

class CGrenadeTNT : public CBaseGrenade
{
	DECLARE_CLASS( CGrenadeTNT, CBaseGrenade );

#if !defined( CLIENT_DLL )
	DECLARE_DATADESC();
#endif
					
	~CGrenadeTNT( void );

public:
	void	Spawn( void );
	void	OnRestore( void );
	void	Precache( void );
	bool	CreateVPhysics( void );
	void	CreateEffects( void );
	void	SetTimer( float detonateDelay, float warnDelay );
	void	SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity );
	int		OnTakeDamage( const CTakeDamageInfo &inputInfo );
	void	DelayThink();
	void	VPhysicsUpdate( IPhysicsObject *pPhysics );
	void	Detonate();
	void	Explode( trace_t *pTrace, int bitsDamageType );

	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	void	SetBombZone( CBombZone *pZone );
	CBombZone* GetBombZone( void ) { return m_pPlantedZone; }

	int UpdateTransmitState() {	return SetTransmitState(FL_EDICT_ALWAYS); }

	bool	m_bIsAttached;
	bool	m_bDefusing;
	CHajPlayer *m_pDefuser;

protected:
	
	bool	m_inSolid;
	bool	m_bPlayingSound;
	float	m_flNextPlay;

	CBombZone *m_pPlantedZone;
};

CBaseGrenade *TNTGrenade_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer );

#endif // GRENADE_TNT_H
