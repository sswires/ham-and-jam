/*
		© 2009 Ham and Jam Team
		============================================================
		Author: Stephen Swires
		Purpose: Mortar field, for explodin' people etc
*/

#include "cbase.h"
#include "haj_shell_artillery.h"
#include "explode.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CMortarField : public CBaseEntity
{
public:
	DECLARE_CLASS( CMortarField, CBaseEntity );
	DECLARE_DATADESC();

	CMortarField();

	virtual void Spawn();
	virtual void Precache();
	virtual bool ShouldCollide( int collisionGroup, int contentsMask ) const;

	CBaseEntity* FindTarget( string_t targetName, CBaseEntity *pActivator );

	void ExplodeThink( void );
	void SetAttacker( CBaseEntity *pEnt );

	void InputSetAttacker( inputdata_t &data );
	void InputSetAttackerName( inputdata_t &data );
	void InputExplode( inputdata_t &data );

private:
	string_t m_strIncomingSound; // KEY VALUES
	float m_flExplodeDelay;
	float m_flExplosionMagnitude;
	float m_flExplosionDamage;
	float m_flExplosionForce;
	int m_iShellCount;
	float m_flClusterDelay;

	float m_flDoExplosionTime;
	bool m_bButtonPressed;

	int m_iShotsToFire;

	bool m_bUseArtilleryShell; // use physics-simulated artillery shell

	CBaseEntity *m_pAttacker;
};

LINK_ENTITY_TO_CLASS( func_mortarfield, CMortarField );

// keyvalues :)
BEGIN_DATADESC( CMortarField )
	DEFINE_FIELD( m_flDoExplosionTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_bButtonPressed, FIELD_BOOLEAN ),

	DEFINE_KEYFIELD( m_iShellCount, FIELD_INTEGER, "ShellCount" ),
	DEFINE_KEYFIELD( m_flExplosionDamage, FIELD_FLOAT, "ExplosionDamage" ),
	DEFINE_KEYFIELD( m_flExplosionMagnitude, FIELD_FLOAT, "ExplosionMagnitude"),
	DEFINE_KEYFIELD( m_flExplodeDelay, FIELD_FLOAT, "ExplosionDelay" ),
	DEFINE_KEYFIELD( m_flClusterDelay, FIELD_FLOAT, "ClusterDelay" ),
	DEFINE_KEYFIELD( m_flExplosionForce, FIELD_FLOAT, "ExplosionForce" ),
	DEFINE_KEYFIELD( m_strIncomingSound, FIELD_SOUNDNAME, "IncomingSound" ),

	DEFINE_KEYFIELD( m_bUseArtilleryShell, FIELD_BOOLEAN, "UseArtillery" ),

	DEFINE_INPUTFUNC( FIELD_EHANDLE, "SetAttacker", InputSetAttacker ),
	DEFINE_INPUTFUNC( FIELD_STRING, "SetAttackerName", InputSetAttackerName ),
	DEFINE_INPUTFUNC( FIELD_VOID, "SendMortars", InputExplode ),

	DEFINE_THINKFUNC( ExplodeThink ),
END_DATADESC()


CMortarField::CMortarField()
{
	m_pAttacker = NULL;
	m_iShellCount = 1;
	m_bButtonPressed = false;

	m_flExplosionDamage = 200;
	m_flExplosionForce = 5;
	m_flClusterDelay = 0.1f;

	m_bUseArtilleryShell = false;
}

void CMortarField::Spawn()
{
	BaseClass::Spawn();

	SetSolid( SOLID_BBOX ); //make it solid
	SetSolidFlags( FSOLID_NOT_SOLID );
	SetCollisionGroup( COLLISION_GROUP_NONE );

	const char* szModelName = STRING(GetModelName());
	SetModel(szModelName);

	SetRenderMode(kRenderNone);

	
	PrecacheSound( STRING( m_strIncomingSound ) ); // may bitch about late precache but fuck it.
}

void CMortarField::Precache()
{
	PrecacheSound( STRING( m_strIncomingSound ) );
}


void CMortarField::ExplodeThink( void )
{
	const Vector vMins = WorldAlignMins();
	const Vector vMaxs = WorldAlignMaxs();

	EntityMatrix tmp;
	tmp.InitFromEntity( this );

	float expx = random->RandomFloat( vMins.x, vMaxs.y );
	float expy = random->RandomFloat( vMins.y, vMaxs.y );

	Vector vWorldPos = tmp.LocalToWorld( Vector( expx, expy, vMaxs.z) );

	// trace to ground
	trace_t tr;
	UTIL_TraceLine( vWorldPos, vWorldPos + Vector( 0, 0, -1024 ), MASK_SOLID, NULL, COLLISION_GROUP_NONE, &tr );

	Vector vOldWorldPos = vWorldPos;
	vWorldPos.z = tr.endpos.z;

	DevMsg( "DEBUG: x: %f y: %f z: %f\n", vWorldPos.x, vWorldPos.y, vWorldPos.z );

	if( m_bUseArtilleryShell )
	{
		trace_t skyTrace;
		UTIL_TraceLine( vWorldPos, vOldWorldPos + Vector( 0, 0, 1024 ), MASK_SOLID, NULL, COLLISION_GROUP_DEBRIS, &tr );

		Vector vecSourcePos = tr.endpos + Vector( 0, 0, -45 );

		Vector vecDir = vecSourcePos - vWorldPos;
		QAngle aDirection;
		VectorAngles( vecDir, aDirection );

		CShellArtillery *pShell = (CShellArtillery*)Create( "shell_artillery", vecSourcePos, aDirection, this );

		DevMsg( "DEBUG shell_artillery: x: %f y: %f z: %f\n", vecSourcePos.x, vecSourcePos.y, vecSourcePos.z );

		pShell->SetAbsVelocity( skyTrace.plane.normal * 500 );	

		pShell->SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE ); 
		pShell->SetThrower( ToBaseCombatCharacter( m_pAttacker ) );
		pShell->SetDamage( 100.0f );
		pShell->SetDamageRadius( 500.0f );

	}
	else
	{
		ExplosionCreate( vWorldPos, QAngle( 0, 0, 0 ), m_pAttacker, m_flExplosionDamage, m_flExplosionMagnitude, 0, m_flExplosionForce, this );
		RadiusDamage( CTakeDamageInfo( this, m_pAttacker, m_flExplosionDamage, DMG_BLAST ), vWorldPos, m_flExplosionMagnitude, CLASS_NONE, NULL );
	}

	m_iShotsToFire--;

	if( m_iShotsToFire > 0 )
	{
		SetNextThink( gpGlobals->curtime + m_flClusterDelay );

		if( STRING( m_strIncomingSound)[0] != '\0' ) // Emit sound if one specified
			EmitSound( STRING( m_strIncomingSound ) );
	}
	else
	{
		m_bButtonPressed = false;
		SetThink( NULL );
	}
}

bool CMortarField::ShouldCollide( int collisionGroup, int contentsMask ) const
{
	return false;
}



void CMortarField::SetAttacker( CBaseEntity *pEnt )
{
	m_pAttacker = pEnt;

	if( m_pAttacker )
	{
		DevMsg( "SetAttacker: %s ", m_pAttacker->GetClassname() );

		if( m_pAttacker->IsPlayer() )
		{
			DevMsg( "%s", ToBasePlayer( m_pAttacker )->GetPlayerName() );
		}

		DevMsg( "\n" );
	}
}

void CMortarField::InputSetAttacker( inputdata_t &data )
{
	DevMsg( "SetAttacker called by entity I/O\n");
	SetAttacker( (CBaseEntity*)data.value.Entity() );
}

CBaseEntity *CMortarField::FindTarget( string_t targetName, CBaseEntity *pActivator ) 
{
	return gEntList.FindEntityGenericNearest( STRING( targetName ), GetAbsOrigin(), 0, this, pActivator );
}


void CMortarField::InputSetAttackerName( inputdata_t &data )
{
	SetAttacker( FindTarget( data.value.StringID(), data.pActivator ) );
}

void CMortarField::InputExplode( inputdata_t &data )
{
	if( !m_bButtonPressed )
	{
		m_bButtonPressed = true;
		m_flDoExplosionTime = gpGlobals->curtime + m_flExplodeDelay;
		m_iShotsToFire = m_iShellCount;

		if( STRING( m_strIncomingSound)[0] != '\0' ) // Emit sound if one specified
			EmitSound( STRING( m_strIncomingSound ) );

		SetThink( &CMortarField::ExplodeThink );
		SetNextThink( m_flDoExplosionTime );
	}
}