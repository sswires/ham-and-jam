//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: Template thrown timed fuze smoke grenade template.
//
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "basegrenade_shared.h"
#include "haj_grenade_smoke.h"
#include "haj_grenade_base.h"
#include "soundent.h"
#include "particle_smokegrenade.h"
#include "smoke_trail.h"
#include "haj_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

const float GRENADE_COEFFICIENT_OF_RESTITUTION = 0.2f;

#define GRENADE_MODEL "models/weapons/w_grenade.mdl"

class CGrenadeSmoke : public CHAJGrenade
{
	DECLARE_CLASS( CGrenadeSmoke, CHAJGrenade );

#if !defined( CLIENT_DLL )
	DECLARE_DATADESC();
#endif

public:

	~CGrenadeSmoke( void );

	void	Spawn( void );
	void	OnRestore( void );
	void	Precache( void );
	bool	CreateVPhysics( void );
	void	CreateEffects( void );
	void	SetTimer( float detonateDelay, float warnDelay );
	void	SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity );
	int		OnTakeDamage( const CTakeDamageInfo &inputInfo );
	void	DelayThink();
	virtual void	VPhysicsUpdate( IPhysicsObject *pPhysics );

	virtual void Detonate( void );	// override the detonate function from CBaseGrenade
	void	SetGrenadeModel( const char *pModelName );

	float	m_flFuzeTime;		// length of the fuze in seconds
	float	m_flDuration;		// ammount of time in seconds to emit smoke for.

protected:
	float	m_flRemoveTime;		// When to remove the grenade from the world
	bool	m_bHasDetonated;	// set if the grenades gone off
	
	bool	m_bIsFrozen;		// set if the grenades physics model is frozen or not
	IPhysicsObject *pObj;		// pointer to the objects physics model
	
	bool	m_inSolid;
	float m_flNextChoke;
};

LINK_ENTITY_TO_CLASS( npc_grenade_smoke, CGrenadeSmoke );

BEGIN_DATADESC( CGrenadeSmoke )

	// Fields
	DEFINE_FIELD( m_inSolid, FIELD_BOOLEAN ),
	
	// Function Pointers
	DEFINE_THINKFUNC( DelayThink ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CGrenadeSmoke::~CGrenadeSmoke( void )
{
	pObj = NULL;
}

void CGrenadeSmoke::Spawn( void )
{
	Precache();

	SetModel ( GRENADE_MODEL );

	m_flDamage		= 0;
	m_DmgRadius		= 0;
	m_takedamage	= DAMAGE_YES;
	m_iHealth		= 1;

	m_flRemoveTime	= gpGlobals->curtime + 60.0f;	// remove the grenade after a minute (default)
	m_bHasDetonated	= false;						// grenade hasn't gone off yet

	SetSize( -Vector(4,4,4), Vector(4,4,4) );
	SetCollisionGroup( COLLISION_GROUP_WEAPON );
	CreateVPhysics();
	CreateEffects();
	AddSolidFlags( FSOLID_NOT_STANDABLE );

	m_bIsFrozen = false;
	pObj = VPhysicsGetObject();

	m_flNextChoke = 0.0f;

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeSmoke::SetGrenadeModel( const char *pModelName )
{
	SetModel ( pModelName );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeSmoke::OnRestore( void )
{
	CreateEffects();
	BaseClass::OnRestore();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeSmoke::CreateEffects( void )
{

	// Add fuze smoke trail
	SmokeTrail *pSmokeTrail =  SmokeTrail::CreateSmokeTrail();
	if( !pSmokeTrail )
		return;

	// Trail settings
	pSmokeTrail->m_SpawnRate = 64;
	pSmokeTrail->m_ParticleLifetime = 1;
	pSmokeTrail->m_StartColor.Init( 0.7f, 0.7f, 0.7f );
	pSmokeTrail->m_EndColor.Init( 1, 1, 1 );
	pSmokeTrail->m_StartSize = 1;
	pSmokeTrail->m_EndSize = 2;
	pSmokeTrail->m_SpawnRadius = 1;
	pSmokeTrail->m_MinSpeed = 1;
	pSmokeTrail->m_MaxSpeed = 2;
	pSmokeTrail->m_Opacity = 0.2f;
	pSmokeTrail->SetLifetime( 6.0f );

	// see if the greande model has an attachment called "fuse"
	int nAttachment = LookupAttachment( "fuse" ); // technicall its's "fuze" but I wont argue...

	// we have one so make the smoke come from that.
	if ( nAttachment != -1 )
	{
		Vector effect_origin;
		QAngle effect_angles;
		effect_angles.Init();
			
		GetAttachment( nAttachment, effect_origin, NULL );
		pSmokeTrail->SetLocalOrigin( effect_origin );
		pSmokeTrail->SetLocalAngles( effect_angles );
		pSmokeTrail->SetParent( this, nAttachment );
	}
	else
	{
		// no attachment, just follow the entity
		pSmokeTrail->SetParent( this );
	}
}

bool CGrenadeSmoke::CreateVPhysics()
{
	// Create the object in the physics system
	VPhysicsInitNormal( SOLID_BBOX, 0, false );
	return true;
}

// this will hit only things that are in newCollisionGroup, but NOT in collisionGroupAlreadyChecked
class CTraceFilterCollisionGroupDelta : public CTraceFilterEntitiesOnly
{
public:
	// It does have a base, but we'll never network anything below here..
	DECLARE_CLASS_NOBASE( CTraceFilterCollisionGroupDelta );
	
	CTraceFilterCollisionGroupDelta( const IHandleEntity *passentity, int collisionGroupAlreadyChecked, int newCollisionGroup )
		: m_pPassEnt(passentity), m_collisionGroupAlreadyChecked( collisionGroupAlreadyChecked ), m_newCollisionGroup( newCollisionGroup )
	{
	}
	
	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
	{
		if ( !PassServerEntityFilter( pHandleEntity, m_pPassEnt ) )
			return false;
		CBaseEntity *pEntity = EntityFromEntityHandle( pHandleEntity );

		if ( pEntity )
		{
			if ( g_pGameRules->ShouldCollide( m_collisionGroupAlreadyChecked, pEntity->GetCollisionGroup() ) )
				return false;
			if ( g_pGameRules->ShouldCollide( m_newCollisionGroup, pEntity->GetCollisionGroup() ) )
				return true;
		}

		return false;
	}

protected:
	const IHandleEntity *m_pPassEnt;
	int		m_collisionGroupAlreadyChecked;
	int		m_newCollisionGroup;
};

void CGrenadeSmoke::VPhysicsUpdate( IPhysicsObject *pPhysics )
{
	BaseClass::VPhysicsUpdate( pPhysics );
}

void CGrenadeSmoke::Precache( void )
{
	// CRITICAL! add all the possible smoke grenade models here.
	PrecacheModel( GRENADE_MODEL );
	PrecacheModel( "models/weapons/w_models/w_nebel_item.mdl" );
	
	PrecacheScriptSound( "BaseGrenade.SmokeStart" );
	BaseClass::Precache();
}

void CGrenadeSmoke::SetTimer( float detonateDelay, float warnDelay )
{
	m_flDetonateTime = gpGlobals->curtime + detonateDelay;
	m_flWarnAITime = gpGlobals->curtime + warnDelay;
	SetThink( &CGrenadeSmoke::DelayThink );
	SetNextThink( gpGlobals->curtime );
}

extern ConVar haj_realism_smoke_choke;
void CGrenadeSmoke::DelayThink() 
{
	// remove ourself if we've gone past our lifetime
	if( gpGlobals->curtime > m_flRemoveTime )
	{
		UTIL_Remove( this );
		return;
	}

	if( m_bImpactFuze && GetGroundEntity() )
	{
		Detonate();
	}

	// choking hazard
	if( haj_realism_smoke_choke.GetBool() && m_bHasDetonated && gpGlobals->curtime >= m_flNextChoke )
	{
		float flRemoveTime = m_flRemoveTime - 15.0;
		float flDurationTime = flRemoveTime - 20.0f;

		float radiusMulti = 1.0f;

		if( flRemoveTime - gpGlobals->curtime < 4 )
			radiusMulti = ( flRemoveTime - gpGlobals->curtime ) / 4;
		else if( gpGlobals->curtime < ( flDurationTime - m_flDuration ) + 5 )
			radiusMulti = 0.25;

		RadiusDamage( CTakeDamageInfo( this, GetThrower(), 2.0f, DMG_NERVEGAS, HAJ_KILL_CHOKING_HAZARD ), GetAbsOrigin(), 250.0f * clamp( radiusMulti, 0, 1 ), CLASS_NONE, this );
		m_flNextChoke = gpGlobals->curtime + 1.25f;
	}

	// if the grenades gone off and finished burning, let it be moved again.
	if ( m_bHasDetonated && gpGlobals->curtime > ( m_flDetonateTime + m_flDuration ) )
	{
		if ( pObj && m_bIsFrozen )
		{
			m_bIsFrozen = false;
			VPhysicsGetObject()->EnableMotion( true );	// enabled movement
			PhysForceEntityToSleep( this, this->VPhysicsGetObject() );	// make it sit still until hit by something
		}
	}

	// If we've not gone off yet, check if we should and do so
	// NOTE: This will cause one issue if the grenade is in the air at the time as it
	// will "hover". We need to check if its on the ground on not but the FL_ONGROUND
	// flag is client side only??

	if (!m_bHasDetonated)
		if( gpGlobals->curtime > m_flDetonateTime && m_flFuzeTime != -1.0f )
		{
			if ( pObj )
				m_bIsFrozen = true;
				VPhysicsGetObject()->EnableMotion(false);	// freeze the grenade model
			
			Detonate();
		}
	
	if( !m_bHasWarnedAI && gpGlobals->curtime >= m_flWarnAITime )
	{
#if !defined( CLIENT_DLL )
		CSoundEnt::InsertSound ( SOUND_DANGER, GetAbsOrigin(), 400, 1.5, this );
#endif
		m_bHasWarnedAI = true;
	}
	
	SetNextThink( gpGlobals->curtime + 0.1 );
}

void CGrenadeSmoke::SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity )
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	if ( pPhysicsObject )
	{
		pPhysicsObject->AddVelocity( &velocity, &angVelocity );
	}
}

int CGrenadeSmoke::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
	// Manually apply vphysics because BaseCombatCharacter takedamage doesn't call back to CBaseEntity OnTakeDamage
	VPhysicsTakeDamage( inputInfo );

	// Grenades only suffer blast damage and burn damage.
	if( !(inputInfo.GetDamageType() & (DMG_BLAST|DMG_BURN) ) )
		return 0;

	return BaseClass::OnTakeDamage( inputInfo );
}

void CGrenadeSmoke::Detonate( )
{
	if( m_bHasDetonated ) return;

	// Get rid of the nade 20 seconds after it stops emitting smoke
	m_flRemoveTime = gpGlobals->curtime + ( m_flDuration + 20.0f );

	// create our smoke cloud
	ParticleSmokeGrenade *pSmoke = dynamic_cast<ParticleSmokeGrenade*>( CreateEntityByName(PARTICLESMOKEGRENADE_ENTITYNAME) );

	pSmoke->FollowEntity( this );
	pSmoke->SetLocalOrigin( GetLocalOrigin() );
	pSmoke->SetFadeTime( m_flDuration, ( m_flDuration + 10.0f) ); // Fade out between for 10 secs after the emit time
    pSmoke->Activate();
    pSmoke->SetLifetime( m_flDuration + 12.0f ); // kill the smoke emitter 12 seconds after lifetime
	pSmoke->m_CurrentStage = 1;

	EmitSound( "BaseGrenade.SmokeStart" );

	m_bHasDetonated = true;	// flag that we've gone 'orf.
}

CHAJGrenade *SmokeGrenade_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, float duration, const char *pszModelname )
{
	// Don't set the owner here, or the player can't interact with grenades he's thrown
	CGrenadeSmoke *pGrenade = (CGrenadeSmoke *)CBaseEntity::Create( "npc_grenade_smoke", position, angles, pOwner );
	
	pGrenade->SetTimer( timer, timer - 1.5f );
	pGrenade->SetVelocity( velocity, angVelocity );
	pGrenade->SetThrower( ToBaseCombatCharacter( pOwner ) );
	pGrenade->m_takedamage = DAMAGE_EVENTS_ONLY;
	pGrenade->m_flFuzeTime = timer;
	pGrenade->m_flDuration = duration;
	pGrenade->SetModel( pszModelname );

	return pGrenade;
}

class CGrenadeSmokeNo77 : public CGrenadeSmoke
{
	DECLARE_CLASS( CGrenadeSmokeNo77, CGrenadeSmoke );
	DECLARE_DATADESC();

public:

	CGrenadeSmokeNo77();

	int		OnTakeDamage( const CTakeDamageInfo &inputInfo );
	virtual void Detonate( void );	// override the detonate function from CBaseGrenade

};


CGrenadeSmokeNo77::CGrenadeSmokeNo77()
{
	m_bImpactFuze = true;
}

int CGrenadeSmokeNo77::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
	if( inputInfo.GetDamage() > 0 && inputInfo.GetInflictor() != this )
	{
		Detonate();
	}

	return BaseClass::OnTakeDamage( inputInfo );
}


void CGrenadeSmokeNo77::Detonate( void )
{
	if( m_bHasDetonated ) return;

	// Get rid of the nade 20 seconds after it stops emitting smoke
	m_flRemoveTime = gpGlobals->curtime + ( m_flDuration + 20.0f );

	// create our smoke cloud
	ParticleSmokeGrenade *pSmoke = dynamic_cast<ParticleSmokeGrenade*>( CreateEntityByName(PARTICLESMOKEGRENADE_ENTITYNAME) );

	pSmoke->FollowEntity( this );
	pSmoke->SetLocalOrigin( GetLocalOrigin() );
	pSmoke->SetFadeTime( m_flDuration, ( m_flDuration + 10.0f) ); // Fade out between for 10 secs after the emit time
	pSmoke->Activate();
	pSmoke->SetLifetime( m_flDuration + 12.0f ); // kill the smoke emitter 12 seconds after lifetime
	pSmoke->m_CurrentStage = 1;

	EmitSound( "BaseGrenade.SmokeStart" );

	m_bHasDetonated = true;	// flag that we've gone 'orf.

	RadiusDamage( CTakeDamageInfo( this, GetThrower(), GetDamage(), DMG_BURN ), GetAbsOrigin(), 200.0f, CLASS_NONE, this );

	IPhysicsObject *pPhysics = VPhysicsGetObject();

	if( pPhysics )
	{
		pPhysics->EnableMotion( false );
		pPhysics->Sleep();

		AddEffects( EF_NODRAW );
	}
}



LINK_ENTITY_TO_CLASS( npc_grenade_no77, CGrenadeSmokeNo77 );

BEGIN_DATADESC( CGrenadeSmokeNo77 )
END_DATADESC()

CHAJGrenade *SmokeGrenadeNo77_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float burndamage, float duration, const char *pszModelname )
{
	// Don't set the owner here, or the player can't interact with grenades he's thrown
	CGrenadeSmokeNo77 *pGrenade = (CGrenadeSmokeNo77 *)CBaseEntity::Create( "npc_grenade_no77", position, angles, pOwner );

	pGrenade->SetTimer( 5.0, 3.5f );
	pGrenade->SetVelocity( velocity, angVelocity );
	pGrenade->SetThrower( ToBaseCombatCharacter( pOwner ) );
	pGrenade->SetDamage( burndamage );
	pGrenade->m_takedamage = DAMAGE_EVENTS_ONLY;
	pGrenade->m_flFuzeTime = -1;
	pGrenade->m_bImpactFuze = true;
	pGrenade->m_flDuration = duration;
	pGrenade->SetModel( pszModelname );

	return pGrenade;
}