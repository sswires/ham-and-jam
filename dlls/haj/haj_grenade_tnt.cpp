//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: NPC/World Mills Grenade class
//
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "basegrenade_shared.h"
#include "haj_grenade_tnt.h"
#include "soundent.h"
#include "smoke_trail.h"
#include "explode.h"
#include "haj_gamemode.h"
#include "haj_bombzone.h"
#include "weapon_satchelcharge.h"

extern short	g_sModelIndexFireball;		// (in combatweapon.cpp) holds the index for the fireball 
extern short	g_sModelIndexWExplosion;	// (in combatweapon.cpp) holds the index for the underwater explosion
extern short	g_sModelIndexSmoke;			// (in combatweapon.cpp) holds the index for the smoke cloud

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

const float GRENADE_COEFFICIENT_OF_RESTITUTION = 0.2f;

#define GRENADE_MODEL "models/weapons/w_tnt.mdl"

LINK_ENTITY_TO_CLASS( npc_grenade_tnt, CGrenadeTNT );

BEGIN_DATADESC( CGrenadeTNT )

	// Fields
	DEFINE_FIELD( m_inSolid, FIELD_BOOLEAN ),
	
	// Function Pointers
	DEFINE_THINKFUNC( DelayThink ),

END_DATADESC()

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



//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CGrenadeTNT::~CGrenadeTNT( void )
{
}

void CGrenadeTNT::Spawn( void )
{
	// our team is same as our owners' - at spawn
	if( GetOwnerEntity() )
		ChangeTeam( GetOwnerEntity()->GetTeamNumber() );

	Precache( );

	if( GetTeamNumber() == TEAM_CWEALTH )
		SetModel( "models/weapons/w_models/w_brit_tnt.mdl" );
	else if( GetTeamNumber() == TEAM_AXIS )
		SetModel( "models/weapons/w_models/w_axis_tnt.mdl" );
	else
		SetModel( GRENADE_MODEL );

	m_flDamage		= 0;
	m_DmgRadius		= 0;
	m_takedamage	= DAMAGE_EVENTS_ONLY;
	m_iHealth		= 1;
	m_bIsAttached	= false;
	m_bIsLive		= true;
	m_bDefusing		= false;
	
	SetSize( -Vector( 16, 16, 16), Vector( 16, 16, 16 ) );

	IPhysicsObject *pObject = VPhysicsInitNormal( SOLID_BBOX, GetSolidFlags() | FSOLID_TRIGGER, true );
	pObject->EnableMotion( !m_bIsAttached );

	SetCollisionGroup( COLLISION_GROUP_WEAPON );
	CreateVPhysics();
	CreateEffects();
	AddSolidFlags( FSOLID_NOT_STANDABLE );

	BaseClass::Spawn();

	SetDamage( 750 );
	SetDamageRadius( 512.0f );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeTNT::OnRestore( void )
{
	CreateEffects();
	BaseClass::OnRestore();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeTNT::CreateEffects( void )
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
	pSmokeTrail->m_MaxSpeed = 3;
	pSmokeTrail->m_Opacity = 0.5f;
	pSmokeTrail->SetLifetime( m_flDetonateTime - gpGlobals->curtime );

	// see if the greande model has an attachment called "fuse"
	int nAttachment = LookupAttachment( "wick" ); // technicall its's "fuze" but I wont argue...

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

bool CGrenadeTNT::CreateVPhysics()
{
	// Create the object in the physics system
	VPhysicsInitNormal( SOLID_BBOX, 0, false );
	return true;
}

void CGrenadeTNT::VPhysicsUpdate( IPhysicsObject *pPhysics )
{
	BaseClass::VPhysicsUpdate( pPhysics );
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
	UTIL_TraceLine( start, start + vel * gpGlobals->frametime, CONTENTS_HITBOX|CONTENTS_MONSTER|CONTENTS_SOLID, &filter, &tr );
#endif
	if ( tr.startsolid )
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
	m_inSolid = false;
	if ( tr.DidHit() )
	{
		Vector dir = vel;
		VectorNormalize(dir);
		// send a tiny amount of damage so the character will react to getting bonked
		CTakeDamageInfo info( this, GetThrower(), pPhysics->GetMass() * vel, GetAbsOrigin(), 0.1f, DMG_CRUSH );
		tr.m_pEnt->TakeDamage( info );

		// reflect velocity around normal
		vel = -2.0f * tr.plane.normal * DotProduct(vel,tr.plane.normal) + vel;
		
		// absorb 80% in impact
		vel *= GRENADE_COEFFICIENT_OF_RESTITUTION;
		angVel *= -0.5f;
		pPhysics->SetVelocity( &vel, &angVel );
	}
}


void CGrenadeTNT::Precache( void )
{
	PrecacheModel( GRENADE_MODEL );
	PrecacheModel( "models/weapons/w_models/w_brit_tnt.mdl");
	PrecacheModel( "models/weapons/w_models/w_axis_tnt.mdl");

	PrecacheScriptSound( "Weapon_SatchelCharge.Fuze" );

	BaseClass::Precache();
}

void CGrenadeTNT::SetTimer( float detonateDelay, float warnDelay )
{
	m_flDetonateTime = gpGlobals->curtime + detonateDelay;
	m_flWarnAITime = gpGlobals->curtime + warnDelay;
	SetThink( &CGrenadeTNT::DelayThink );
	SetNextThink( gpGlobals->curtime );

	m_bPlayingSound = true;
	m_flNextPlay = gpGlobals->curtime;
}

void CGrenadeTNT::DelayThink() 
{
	if( m_bIsLive && gpGlobals->curtime > m_flDetonateTime )
	{
		Detonate();
		return;
	}

	if( m_bPlayingSound && gpGlobals->curtime >= m_flNextPlay )
	{
		EmitSound( "Weapon_SatchelCharge.Fuze" );
		m_flNextPlay = gpGlobals->curtime + 2.0;
	}

	if( !m_bHasWarnedAI && gpGlobals->curtime >= m_flWarnAITime )
	{
#if !defined( CLIENT_DLL )
		CSoundEnt::InsertSound ( SOUND_DANGER, GetAbsOrigin(), 400, 1.5, this );
#endif
		m_bHasWarnedAI = true;
	}

	// Safe guards for defusing
	if( m_bDefusing && m_pDefuser )
	{
		CWeaponSatchelCharge *pWeap = dynamic_cast<CWeaponSatchelCharge*>( m_pDefuser->GetActiveWeapon() );

		if( !pWeap || !pWeap->IsDefusing() || !m_pDefuser->IsAlive() )
		{
			RemoveEffects( EF_NODRAW );
			m_bDefusing = false;
		}
	}
	
	SetNextThink( gpGlobals->curtime + 0.1 );
}

void CGrenadeTNT::SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity )
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	if ( pPhysicsObject )
	{
		pPhysicsObject->AddVelocity( &velocity, &angVelocity );
	}
}

int CGrenadeTNT::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
	// Manually apply vphysics because BaseCombatCharacter takedamage doesn't call back to CBaseEntity OnTakeDamage
	VPhysicsTakeDamage( inputInfo );

	// Grenades only suffer blast damage and burn damage.
	if( !(inputInfo.GetDamageType() & (DMG_BLAST|DMG_BURN) ) )
		return 0;

	return BaseClass::OnTakeDamage( inputInfo );
}

void CGrenadeTNT::Detonate()
{
	trace_t		tr;
	Vector		vecSpot;// trace starts here!

	SetThink( NULL );

	vecSpot = GetAbsOrigin() + Vector ( 0 , 0 , 8 );
	UTIL_TraceLine ( vecSpot, vecSpot + Vector ( 0, 0, -32 ), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, & tr);

	Explode( &tr, DMG_BLAST | DMG_ALWAYSGIB | DMG_ENERGYBEAM );

	if ( GetShakeAmplitude() )
	{
		UTIL_ScreenShake( GetAbsOrigin(), GetShakeAmplitude(), 150.0, 1.0, GetShakeRadius(), SHAKE_START );
	}

	if( m_pPlantedZone )
	{
		m_pPlantedZone->OnBombExploded( ToBasePlayer( GetThrower() ), this );

		if( HajGameRules()->GetGamemode() != NULL )
		{
			 HajGameRules()->GetGamemode()->OnBombExplode( m_pPlantedZone );
		}
	}

	StopSound( "Weapon_SatchelCharge.Fuze" );
	m_bPlayingSound = false;
}

void CGrenadeTNT::Explode( trace_t *pTrace, int bitsDamageType )
{
	BaseClass::Explode( pTrace, bitsDamageType );
	/*
#if !defined( CLIENT_DLL )
	
	SetModelName( NULL_STRING );//invisible
	AddSolidFlags( FSOLID_NOT_SOLID );
	m_takedamage = DAMAGE_NO;

	// Pull out of the wall a bit
	if ( pTrace->fraction != 1.0 )
	{
		SetAbsOrigin( pTrace->endpos + (pTrace->plane.normal * 0.6) );
	}

	Vector vecAbsOrigin = GetAbsOrigin();
	int contents = UTIL_PointContents ( vecAbsOrigin );

#if defined( TF_DLL )
	// Since this code only runs on the server, make sure it shows the tempents it creates.
	// This solves a problem with remote detonating the pipebombs (client wasn't seeing the explosion effect)
	CDisablePredictionFiltering disabler;
#endif

	//UTIL_ScreenShake( GetAbsOrigin(), 25.0, 150.0, 1.0, 750, SHAKE_START );
	ExplosionCreate( vecAbsOrigin, GetAbsAngles(), GetThrower(), 2048.0f, 768.0f, 
		SF_ENVEXPLOSION_NOCLAMPMAX, 0.0f, this);

	/*
	if ( pTrace->fraction != 1.0 )
	{
		Vector vecNormal = pTrace->plane.normal;
		surfacedata_t *pdata = physprops->GetSurfaceData( pTrace->surface.surfaceProps );	
		CPASFilter filter( vecAbsOrigin );

		te->Explosion( filter, -1.0, // don't apply cl_interp delay
			&vecAbsOrigin,
			!( contents & MASK_WATER ) ? g_sModelIndexFireball : g_sModelIndexWExplosion,
			m_DmgRadius * 100, 
			5,
			TE_EXPLFLAG_NONE,
			m_DmgRadius,
			m_flDamage,
			&vecNormal,
			(char) pdata->game.material );
	}
	else
	{
		CPASFilter filter( vecAbsOrigin );
		te->Explosion( filter, -1.0, // don't apply cl_interp delay
			&vecAbsOrigin, 
			!( contents & MASK_WATER ) ? g_sModelIndexFireball : g_sModelIndexWExplosion,
			m_DmgRadius * 100, 
			5,
			TE_EXPLFLAG_NONE,
			m_DmgRadius,
			m_flDamage );
	}

#if !defined( CLIENT_DLL )
	CSoundEnt::InsertSound ( SOUND_COMBAT, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0 );
#endif

	// Use the thrower's position as the reported position
	Vector vecReported = GetThrower() ? GetThrower()->GetAbsOrigin() : vec3_origin;
	
	CTakeDamageInfo info( this, GetThrower(), GetBlastForce(), GetAbsOrigin(), m_flDamage, bitsDamageType, 0, &vecReported );

	RadiusDamage( info, GetAbsOrigin(), m_DmgRadius, CLASS_NONE, NULL );

	UTIL_DecalTrace( pTrace, "Scorch" );

	EmitSound( "BaseGrenade.Explode" );
*/
/*
	SetThink( &CBaseGrenade::SUB_Remove );
	SetTouch( NULL );
	
	AddEffects( EF_NODRAW );
	SetAbsVelocity( vec3_origin );
	SetNextThink( gpGlobals->curtime );
#endif
*/
}

void CGrenadeTNT::SetBombZone( CBombZone *pZone )
{
	m_pPlantedZone = pZone;

	if( pZone )
	{
		pZone->OnBombPlant( ToBasePlayer( GetThrower() ), this );
	}
}

extern ConVar haj_bomb_test_mode;
void CGrenadeTNT::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CHajPlayer *pUser = ToHajPlayer( pActivator );

	if( !m_bDefusing && pUser && GetThrower() && ( haj_bomb_test_mode.GetBool() || GetTeamNumber() != pUser->GetTeamNumber() ) ) // valid and on opposite team
	{
		// this code is a bit shit but it works, it gives them a weapon_satchelcharge if they don't have one
		CWeaponSatchelCharge *pSatchel = NULL;
		
		pSatchel = (CWeaponSatchelCharge*)pUser->Weapon_OwnsThisType( "weapon_satchelcharge" );

		if( !pSatchel )
		{
			pSatchel = (CWeaponSatchelCharge*)pUser->GiveNamedItem( "weapon_satchelcharge" );
			pUser->SetAmmoCount( 0, pSatchel->GetPrimaryAmmoType() );
		}

		pSatchel->SetDefusing( this );

		if( pSatchel != pUser->GetActiveWeapon() )
		{
			pUser->Weapon_Switch( pSatchel );
			//pSatchel->SendWeaponAnim( ACT_VM_PULLBACK_HIGH );
		}

		m_bDefusing = true;
		m_pDefuser = pUser;
	}
}

CBaseGrenade *TNTGrenade_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer )
{
	// Don't set the owner here, or the player can't interact with grenades he's thrown
	CGrenadeTNT *pGrenade = (CGrenadeTNT *)CBaseEntity::Create( "npc_grenade_tnt", position, angles, pOwner );
	
	pGrenade->SetTimer( timer, timer - 1.5f );
	pGrenade->SetVelocity( velocity, angVelocity );
	pGrenade->SetThrower( ToBaseCombatCharacter( pOwner ) );
	pGrenade->m_takedamage = DAMAGE_EVENTS_ONLY;

	return pGrenade;
}