/*
		© 2009 Ham and Jam Team
		============================================================
		Author: Stephen Swires
		Purpose: func_tank adapted for machine gun use
*/

#include "cbase.h"
#include "player.h"
#include "in_buttons.h"
#include "haj_gamemode.h"
#include "func_tank.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

enum MGMuzzle
{
	MG_TRACER_NONE = 0,
	MG_TRACER_303,
	MG_TRACER_792,
};

//-----------------------------------------------------------------------------
// Purpose: Func tank that fires an artillery shell
//-----------------------------------------------------------------------------
class CFuncTankMG : public CFuncTank
{
public:
	DECLARE_CLASS( CFuncTankMG, CFuncTank );

	CFuncTankMG();

	void Precache( void );

	void	Think( void );

	virtual void Fire( int bulletCount, const Vector &barrelEnd, const Vector &forward, CBaseEntity *pAttacker, bool bIgnoreSpread );
	virtual const char *GetTracerType( void );
	Vector	GetCoFVector( float coneangle );

	void Overheat();
	void CheckCooldown();

	void InputForceFire( inputdata_t &input );

private:
	float m_flTemperature;
	float m_flOverheatTime;
	bool m_bOverheated;

protected:
	int m_iTraceChance;
	int m_iCoolDownTime;
	int m_iTracer;
	float m_flMaxTemperature;
	float m_flBulletSpread;

	string_t m_strCustomTracer;

	DECLARE_DATADESC();
};

BEGIN_DATADESC( CFuncTankMG )
	DEFINE_KEYFIELD( m_flMaxTemperature, FIELD_FLOAT, "MaxTemperature" ),
	DEFINE_KEYFIELD( m_iCoolDownTime, FIELD_INTEGER, "CoolDownTime" ),
	DEFINE_KEYFIELD( m_iTraceChance, FIELD_INTEGER, "TracerFrequency" ),
	DEFINE_KEYFIELD( m_flBulletSpread, FIELD_FLOAT, "BulletSpread" ),
	DEFINE_KEYFIELD( m_iTracer, FIELD_INTEGER, "Tracer"),
	DEFINE_KEYFIELD( m_strCustomTracer, FIELD_STRING, "CustomTracer" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "ForceFire", InputForceFire ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( func_tank_mg, CFuncTankMG );


CFuncTankMG::CFuncTankMG()
{
	m_iTraceChance = 1;
	m_flOverheatTime = 0.0f;
	m_bOverheated = false;
	m_flTemperature = 0.0f;
	m_flMaxTemperature = -1;
	m_flBulletSpread = 0.0f;

	SetThink( &CFuncTankMG::Think );
}

void CFuncTankMG::Precache( void )
{
	BaseClass::Precache();
}


void CFuncTankMG::Think( void )
{
	CBasePlayer *pPlayer = static_cast<CBasePlayer*>( GetController() );

	if( m_flTemperature > 0.5f && gpGlobals->curtime >= GetNextAttack() && ( !pPlayer || ( pPlayer && ( pPlayer->m_nButtons & IN_ATTACK ) == 0 ) ) )
	{
		m_flTemperature -= 0.5f / m_iCoolDownTime;
		m_flTemperature = clamp( m_flTemperature, 0.0, m_flMaxTemperature );
	}

	BaseClass::Think();
}


void CFuncTankMG::Overheat()
{
	EmitSound( "MountedMG.Overheating" );
	SetNextAttack( gpGlobals->curtime + 1.5 );

	// give them a little burn damage since we're overheating
	if( GetController() )
	{
		CTakeDamageInfo info( this, this, 5.0f, DMG_BURN, HAJ_KILL_MG_OVERHEAT );
		GetController()->TakeDamage( info );
	}

	StopControl();

	m_flOverheatTime = gpGlobals->curtime;
	m_bOverheated = true;
}



void CFuncTankMG::CheckCooldown()
{
	if( m_bOverheated && gpGlobals->curtime >= m_flOverheatTime + m_iCoolDownTime )
	{
		m_bOverheated = false;
		m_flOverheatTime = 0.0f;
		m_flTemperature = 0.0f;

		DevMsg( "Cooled down!\n" );
	}
}

Vector CFuncTankMG::GetCoFVector( float coneangle )
{
	float radangle = DEG2RAD( coneangle / 2 );
	float sine, cosine;
	SinCos( radangle, &sine, &cosine );

	return Vector ( sine, sine, sine );
}

void CFuncTankMG::Fire( int bulletCount, const Vector &barrelEnd, const Vector &forward, CBaseEntity *pAttacker, bool bIgnoreSpread )
{
	CheckCooldown();

	// we're overheating, don't fire the gun
	if( m_bOverheated && gpGlobals->curtime - m_flOverheatTime < m_iCoolDownTime )
	{
		EmitSound( "MountedMG.OverheatedShot" );
		SetNextAttack( gpGlobals->curtime + 0.25 );

		DevMsg( "Still overheated!\n" );

		m_flOverheatTime += 0.25; // extend the cool down a bit more
		return;
	}
	
	// we've only just overheated
	if( !m_bOverheated && m_flMaxTemperature > 0 && m_flTemperature >= m_flMaxTemperature )
	{
		Overheat();
		return;
	}

	// shouldn't be here
	if( m_bOverheated ) return;

	// no heat problems, allow shooting
	IPredictionSystem::SuppressHostEvents( NULL );
	
	int i;

	FireBulletsInfo_t info;
	info.m_iShots = 1;
	info.m_vecSrc = barrelEnd;
	info.m_vecDirShooting = forward;
	
	if ( !bIgnoreSpread )
	{
		info.m_vecSpread = GetCoFVector( m_flBulletSpread );
	}

	info.m_flDistance = MAX_TRACE_LENGTH;
	info.m_iTracerFreq = m_iTraceChance;
	info.m_iDamage = m_iBulletDamage;
	info.m_iPlayerDamage = m_iBulletDamageVsPlayer;
	info.m_pAttacker = pAttacker;
	info.m_pAdditionalIgnoreEnt = GetParent();

	if( m_iAmmoType < 1 )
		m_iAmmoType = 2;

	for ( i = 0; i < bulletCount; i++ )
	{
		info.m_iAmmoType = m_iAmmoType;
		FireBullets( info );

		m_flTemperature += ( gpGlobals->frametime * 100 );
		DevMsg( "func_tank_mg temp: %f\n", m_flTemperature );
	}

	BaseClass::Fire( bulletCount, barrelEnd, forward, pAttacker, bIgnoreSpread );
}

const char* CFuncTankMG::GetTracerType( void )
{
	switch( m_iTracer )
	{
		case MG_TRACER_303:
			return "303Tracer";

		case MG_TRACER_792:
			return "792Tracer";

		case MG_TRACER_NONE:
			return NULL;
	}

	return m_strCustomTracer.ToCStr();
}

// this allows the gun to be fired without a controller
void CFuncTankMG::InputForceFire( inputdata_t &input )
{
	int bulletCount = (gpGlobals->curtime - m_fireLast) * m_fireRate;

	Vector forward;
	AngleVectors( GetAbsAngles(), &forward );
	m_fireLast = gpGlobals->curtime - (1/m_fireRate) - 0.01;  // to make sure the gun doesn't fire too many bullets

	Fire( bulletCount, WorldBarrelPosition(), forward, this, false );
}