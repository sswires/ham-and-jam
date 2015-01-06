//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: NPC/World Mills Grenade class
//
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "basegrenade_shared.h"
#include "smoke_trail.h"
#include "haj_grenade_base.h"
#include "haj_gamerules.h"
#include "decals.h"
#include "func_break.h"
#include "soundent.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

const float GRENADE_COEFFICIENT_OF_RESTITUTION = 0.2f;

BEGIN_DATADESC( CHAJGrenade )

	// Function Pointers
	DEFINE_THINKFUNC( DelayThink ),

	// Fields
	DEFINE_FIELD( m_inSolid, FIELD_BOOLEAN ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHAJGrenade::~CHAJGrenade( void )
{
}

const char* CHAJGrenade::GetGrenadeModel( void )
{
	return "models/weapons/w_models/w_mills_item.mdl";
}

CHAJGrenade::CHAJGrenade( void )
{
	m_bImpactFuze = false;
}

void CHAJGrenade::Spawn( void )
{
	Precache( );

	SetModel( GetGrenadeModel() );

	m_flDamage		= 0;
	m_DmgRadius		= 0;
	m_takedamage	= DAMAGE_YES;
	m_iHealth		= 1;
	m_flDetonateTime = 0.0f;
	m_bCreatedEffects = false;

	SetSize( -Vector(4,4,4), Vector(4,4,4) );
	SetCollisionGroup( COLLISION_GROUP_WEAPON );
	CreateVPhysics();
	//CreateEffects();
	AddSolidFlags( FSOLID_NOT_STANDABLE );

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHAJGrenade::OnRestore( void )
{
	CreateEffects();
	BaseClass::OnRestore();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHAJGrenade::CreateEffects( void )
{
	m_bCreatedEffects = true;

	// Add fuze smoke trail
	SmokeTrail *pSmokeTrail =  SmokeTrail::CreateSmokeTrail();
	if( !pSmokeTrail )
		return;

	// Trail settings
	pSmokeTrail->m_SpawnRate = 64;
	pSmokeTrail->m_ParticleLifetime = 1;
	pSmokeTrail->m_StartColor.Init( 0.7f, 0.7f, 0.7f );
	pSmokeTrail->m_EndColor.Init( 1, 1, 1 );
	pSmokeTrail->m_StartSize = 3;
	pSmokeTrail->m_EndSize = 9;
	pSmokeTrail->m_SpawnRadius = 2.5f;
	pSmokeTrail->m_MinSpeed = 2;
	pSmokeTrail->m_MaxSpeed = 4;
	pSmokeTrail->m_Opacity = 0.2f;
	pSmokeTrail->SetLifetime( m_flDetonateTime - gpGlobals->curtime );
	pSmokeTrail->SetGravity( -0.2f );

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

bool CHAJGrenade::CreateVPhysics()
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

void CHAJGrenade::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
	if( m_bImpactFuze )
	{
		Detonate();
	}

	// check for breakables
	CBreakable *pBreakable = dynamic_cast<CBreakable*>(pEvent->pEntities[0]);

	if( pBreakable && pBreakable->IsBreakable() )
	{
		pBreakable->Break( this );
		DevMsg( "Breaking glass for grenade...\n" );
	}

	BaseClass::VPhysicsCollision( index, pEvent );
}

void CHAJGrenade::VPhysicsUpdate( IPhysicsObject *pPhysics )
{
	Vector vel;
	AngularImpulse angVel;
	pPhysics->GetVelocity( &vel, &angVel );
	
	Vector start = GetAbsOrigin();
	// find all entities that my collision group wouldn't hit, but COLLISION_GROUP_NONE would and bounce off of them as a ray cast
	CTraceFilterCollisionGroupDelta filter( this, GetCollisionGroup(), COLLISION_GROUP_NONE );
	trace_t tr;

	// UNDONE: Hull won't work with hitboxes - hits outer hull.  But the whole point of this test is to hit hitboxes.
#if 0
	UTIL_TraceHull( start, start + vel * gpGlobals->frametime, CollisionProp()->OBBMins(), CollisionProp()->OBBMaxs(), CONTENTS_HITBOX|CONTENTS_MONSTER|CONTENTS_SOLID, &filter, &tr );
#else
	UTIL_TraceLine( start, start + vel * gpGlobals->frametime, CONTENTS_HITBOX|CONTENTS_MONSTER|CONTENTS_SOLID|CONTENTS_DETAIL|CONTENTS_WINDOW, &filter, &tr );
#endif

	// hit world
	if ( tr.startsolid )
	{
		bool bInvertVel = false;
		DevMsg( "Grenade is hitting something...\r\n" );

		if ( !m_inSolid )
		{
			if ( !m_inSolid )
			{
				// UNDONE: Do a better contact solution that uses relative velocity?
				vel *= -GRENADE_COEFFICIENT_OF_RESTITUTION; // bounce backwards
				pPhysics->SetVelocity( &vel, NULL );
			}
			m_inSolid = true;
			return;
		}

		m_inSolid = true;

		BaseClass::VPhysicsUpdate( pPhysics );

		if( bInvertVel )
			vel *= -1.0f;

		return;
	}

	m_inSolid = false;
	if ( tr.DidHit() )
	{
		Vector dir = vel;
		VectorNormalize(dir);
		// send a tiny amount of damage so the character will react to getting bonked
		CTakeDamageInfo info( this, GetThrower(), pPhysics->GetMass() * vel, GetAbsOrigin(), vel.Length2D() / 15, DMG_CRUSH, HAJ_KILL_GRENADE_IMPACT );
		tr.m_pEnt->TakeDamage( info );

		DevMsg( "Dealing %f impact damage on grenade\n", info.GetDamage() );

		// reflect velocity around normal
		vel = -2.0f * tr.plane.normal * DotProduct(vel,tr.plane.normal) + vel;
		
		// absorb 80% in impact
		vel *= GRENADE_COEFFICIENT_OF_RESTITUTION;
		angVel *= -0.5f;
		pPhysics->SetVelocity( &vel, &angVel );
	}

	BaseClass::VPhysicsUpdate( pPhysics );
}


void CHAJGrenade::Precache( void )
{
	PrecacheModel( GetGrenadeModel() );
	BaseClass::Precache();
}

void CHAJGrenade::SetTimer( float detonateDelay, float warnDelay )
{
	m_flDetonateTime = gpGlobals->curtime + detonateDelay;
	m_flWarnAITime = gpGlobals->curtime + warnDelay;
	SetThink( &CHAJGrenade::DelayThink );
	SetNextThink( gpGlobals->curtime );

	if( !m_bCreatedEffects )
		CreateEffects();
}

void CHAJGrenade::DelayThink() 
{
	if( gpGlobals->curtime > m_flDetonateTime || ( m_bImpactFuze && GetGroundEntity() != NULL ) )
	{
		Detonate();
		return;
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

void CHAJGrenade::SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity )
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	if ( pPhysicsObject )
	{
		pPhysicsObject->AddVelocity( &velocity, &angVelocity );
	}
}

int CHAJGrenade::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
	// Manually apply vphysics because BaseCombatCharacter takedamage doesn't call back to CBaseEntity OnTakeDamage
	VPhysicsTakeDamage( inputInfo );

	// If you shoot a grenade it'll explode
	if( inputInfo.GetDamageType() & DMG_BULLET )
	{
		SetThrower( ToBaseCombatCharacter( inputInfo.GetAttacker() ) ); // give ownership to the person who shot this, incase they kill someone
		Detonate();
	}

	// Grenades only suffer blast damage and burn damage.
	if( !(inputInfo.GetDamageType() & (DMG_BLAST|DMG_BURN) ) )
		return 0;

	return BaseClass::OnTakeDamage( inputInfo );
}

void CHAJGrenade::ResolveFlyCollisionCustom( trace_t &trace, Vector &vecVelocity )
{
	//Assume all surfaces have the same elasticity
	float flSurfaceElasticity = 1.0;

	//Don't bounce off of players with perfect elasticity
	if( trace.m_pEnt && trace.m_pEnt->IsPlayer() )
	{
		flSurfaceElasticity = 0.3;
	}

	// if its breakable glass and we kill it, don't bounce.
	// give some damage to the glass, and if it breaks, pass 
	// through it.
	bool breakthrough = false;

	if( trace.m_pEnt && FClassnameIs( trace.m_pEnt, "func_breakable" ) )
	{
		breakthrough = true;
	}

	if( trace.m_pEnt && FClassnameIs( trace.m_pEnt, "func_breakable_surf" ) )
	{
		breakthrough = true;
	}

	if (breakthrough)
	{
		DevMsg( "Grenade breaking through breakable...\r\n" );
		CTakeDamageInfo info( this, this, 50, DMG_CLUB );
		trace.m_pEnt->DispatchTraceAttack( info, GetAbsVelocity(), &trace );

		ApplyMultiDamage();

		if( trace.m_pEnt->m_iHealth <= 0 )
		{
			// slow our flight a little bit
			Vector vel = GetAbsVelocity();

			vel *= 0.4;

			SetAbsVelocity( vel );
			return;
		}
	}

	float flTotalElasticity = GetElasticity() * flSurfaceElasticity;
	flTotalElasticity = clamp( flTotalElasticity, 0.0f, 0.9f );

	// NOTE: A backoff of 2.0f is a reflection
	Vector vecAbsVelocity;
	PhysicsClipVelocity( GetAbsVelocity(), trace.plane.normal, vecAbsVelocity, 2.0f );
	vecAbsVelocity *= flTotalElasticity;

	// Get the total velocity (player + conveyors, etc.)
	VectorAdd( vecAbsVelocity, GetBaseVelocity(), vecVelocity );
	float flSpeedSqr = DotProduct( vecVelocity, vecVelocity );

	// Stop if on ground.
	if ( trace.plane.normal.z > 0.7f )			// Floor
	{
		// Verify that we have an entity.
		CBaseEntity *pEntity = trace.m_pEnt;
		Assert( pEntity );

		SetAbsVelocity( vecAbsVelocity );

		if ( flSpeedSqr < ( 30 * 30 ) )
		{
			if ( pEntity->IsStandable() )
			{
				SetGroundEntity( pEntity );
			}

			// Reset velocities.
			SetAbsVelocity( vec3_origin );
			SetLocalAngularVelocity( vec3_angle );

			//align to the ground so we're not standing on end
			QAngle angle;
			VectorAngles( trace.plane.normal, angle );

			// rotate randomly in yaw
			angle[1] = random->RandomFloat( 0, 360 );

			// TODO: rotate around trace.plane.normal

			SetAbsAngles( angle );			
		}
		else
		{
			Vector vecDelta = GetBaseVelocity() - vecAbsVelocity;	
			Vector vecBaseDir = GetBaseVelocity();
			VectorNormalize( vecBaseDir );
			float flScale = vecDelta.Dot( vecBaseDir );

			VectorScale( vecAbsVelocity, ( 1.0f - trace.fraction ) * gpGlobals->frametime, vecVelocity ); 
			VectorMA( vecVelocity, ( 1.0f - trace.fraction ) * gpGlobals->frametime, GetBaseVelocity() * flScale, vecVelocity );
			PhysicsPushEntity( vecVelocity, &trace );
		}
	}
	else
	{
		// If we get *too* slow, we'll stick without ever coming to rest because
		// we'll get pushed down by gravity faster than we can escape from the wall.
		if ( flSpeedSqr < ( 30 * 30 ) )
		{
			// Reset velocities.
			SetAbsVelocity( vec3_origin );
			SetLocalAngularVelocity( vec3_angle );
		}
		else
		{
			SetAbsVelocity( vecAbsVelocity );
		}
	}

	BounceSound();
}