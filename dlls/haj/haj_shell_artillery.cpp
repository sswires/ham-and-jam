//========= Copyright © 2006, Ham and Jam, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "haj_shell_artillery.h"
//#include "weapon_ar2.h" 	//?
#include "soundent.h"
#include "decals.h"
#include "shake.h"
#include "smoke_trail.h"
#include "ar2_explosion.h"	//??
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "world.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define ARTILLERY_SHELL_MAX_DANGER_RADIUS	300

extern short	g_sModelIndexFireball;			// (in combatweapon.cpp) holds the index for the smoke cloud

BEGIN_DATADESC( CShellArtillery )

	DEFINE_FIELD( m_hSmokeTrail, FIELD_EHANDLE ),
	DEFINE_FIELD( m_fSpawnTime, FIELD_TIME ),
	DEFINE_FIELD( m_fDangerRadius, FIELD_FLOAT ),

	// Function pointers
	DEFINE_ENTITYFUNC( ShellTouch ),
	DEFINE_THINKFUNC( ShellThink ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( shell_artillery, CShellArtillery );

void CShellArtillery::Spawn( void )
{
	Precache( );
	SetSolid( SOLID_BBOX );
	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );

	// Hits everything but debris
	SetCollisionGroup( COLLISION_GROUP_PROJECTILE );

	// set model here
	SetModel( "models/weapons/AR2_grenade.mdl");

	UTIL_SetSize(this, Vector(-3, -3, -3), Vector(3, 3, 3));

	SetUse( &CShellArtillery::DetonateUse );
	SetTouch( &CShellArtillery::ShellTouch );
	SetThink( &CShellArtillery::ShellThink );
	SetNextThink( gpGlobals->curtime + 0.1f );

	m_flDamage		= 100.0f;
	m_DmgRadius		= 500.0f;
	m_takedamage	= DAMAGE_YES; //??
	m_bIsLive		= true;
	m_iHealth		= 1;

	// physics stuff
	SetGravity( UTIL_ScaleForGravity( 400 ) );	// use a lower gravity for grenades to make them easier to see
	SetFriction( 0.0 );
	SetSequence( 1 );

	m_fDangerRadius = 100;

	m_fSpawnTime = gpGlobals->curtime;

	// -------------
	// Smoke trail.
	// -------------
	m_hSmokeTrail = SmokeTrail::CreateSmokeTrail();
		
	if( m_hSmokeTrail )
	{
		m_hSmokeTrail->m_Opacity = 0.1f;
		m_hSmokeTrail->m_SpawnRate = 50;
		m_hSmokeTrail->m_ParticleLifetime = 0.5f;
		m_hSmokeTrail->m_StartColor.Init( 1.0f, 1.0f, 1.0f );
		m_hSmokeTrail->m_EndColor.Init( 0.65f, 0.65f, 0.65f );
		m_hSmokeTrail->m_StartSize = 2;
		m_hSmokeTrail->m_EndSize = 8;
		m_hSmokeTrail->m_SpawnRadius = 4;
		m_hSmokeTrail->m_MinSpeed = 2;
		m_hSmokeTrail->m_MaxSpeed = 16;

		m_hSmokeTrail->SetLifetime(5.0f);
		m_hSmokeTrail->FollowEntity(this);
	}
}

//-----------------------------------------------------------------------------
// Purpose:  The shell has a slight delay before it goes live.  That way the
//			 person firing it can bounce it off a nearby wall.  However if it
//			 hits another character it blows up immediately
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CShellArtillery::ShellThink( void )
{
	SetNextThink( gpGlobals->curtime + 0.05f );

	if (!m_bIsLive)
	{
		// Go live after a short delay
		if (m_fSpawnTime + MAX_SHELL_NO_COLLIDE_TIME < gpGlobals->curtime)
		{
			m_bIsLive  = true;
		}
	}
	
	// If I just went solid and my velocity is zero, it means I'm resting on
	// the floor already when I went solid so blow up
	if (m_bIsLive)
	{
		if (GetAbsVelocity().Length() == 0.0 ||
			GetGroundEntity() != NULL )
		{
			Detonate();
		}
	}

	// The old way of making danger sounds would scare the crap out of EVERYONE between you and where the grenade
	// was going to hit. The radius of the danger sound now 'blossoms' over the grenade's lifetime, making it seem
	// dangerous to a larger area downrange than it does from where it was fired.
	if( m_fDangerRadius <= ARTILLERY_SHELL_MAX_DANGER_RADIUS )
	{
		m_fDangerRadius += ( ARTILLERY_SHELL_MAX_DANGER_RADIUS * 0.05 );
	}

	CSoundEnt::InsertSound( SOUND_DANGER, GetAbsOrigin() + GetAbsVelocity() * 0.5, m_fDangerRadius, 0.2, this, SOUNDENT_CHANNEL_REPEATED_DANGER );
}

void CShellArtillery::Event_Killed( const CTakeDamageInfo &info )
{
	Detonate( );
}

void CShellArtillery::ShellTouch( CBaseEntity *pOther )
{
	Assert( pOther );
	if ( !pOther->IsSolid() )
		return;

	// If I'm live go ahead and blow up
	if (m_bIsLive)
	{
		Detonate();
	}
	else
	{
		// If I'm not live, only blow up if I'm hitting an chacter that
		// is not the owner of the weapon
		CBaseCombatCharacter *pBCC = ToBaseCombatCharacter( pOther );
		if (pBCC && GetThrower() != pBCC)
		{
			m_bIsLive = true;
			Detonate();
		}
	}
}

// BOOM!
void CShellArtillery::Detonate(void)
{
	if (!m_bIsLive)
	{
		return;
	}
		
	m_bIsLive		= false;
	m_takedamage	= DAMAGE_NO;
	SetSolid( SOLID_NONE );
		
	Vector vecForward = GetAbsVelocity();
	VectorNormalize(vecForward);
	trace_t		tr;

	// Don't explode against the skybox. Just pretend that 
	// the shell flies off into the distance.
	Vector forward;
	GetVectors( &forward, NULL, NULL );
	UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + forward * 16, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );
	
	// didn't hit skybox
	if( tr.fraction == 1.0 || !(tr.surface.flags & SURF_SKY) )
	{
		CPASFilter filter( GetAbsOrigin() );

		te->Explosion( filter, 0.0,
			&GetAbsOrigin(), 
			g_sModelIndexFireball,
			2.0, 
			15,
			TE_EXPLFLAG_NONE,
			m_DmgRadius,
			m_flDamage );

		UTIL_TraceLine ( GetAbsOrigin(), GetAbsOrigin() + 60 * vecForward, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

		if ((tr.m_pEnt != GetWorldEntity()) || (tr.hitbox != 0))
		{
			// non-world needs smaller decals
			if( tr.m_pEnt && !tr.m_pEnt->IsNPC() )
			{
				UTIL_DecalTrace( &tr, "SmallScorch" );
			}
		}
		else
		{
			UTIL_DecalTrace( &tr, "Scorch" );
		}

		UTIL_ScreenShake( GetAbsOrigin(), 25.0, 150.0, 1.0, 750, SHAKE_START );

		RadiusDamage ( CTakeDamageInfo( this, GetThrower(), m_flDamage, DMG_BLAST ), GetAbsOrigin(), m_DmgRadius, CLASS_NONE, NULL );

	}
	else	// did hit skybox
	{
		if( m_hSmokeTrail )
		{
			m_hSmokeTrail->SetLifetime(0.1f);
			m_hSmokeTrail = NULL;
		}
	}

	UTIL_Remove( this );
	
}

void CShellArtillery::Precache( void )
{
	PrecacheModel("models/weapons/AR2_grenade.mdl"); 
}


CShellArtillery::CShellArtillery(void)
{
	m_hSmokeTrail  = NULL;
}
