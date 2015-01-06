#include "cbase.h"
#include "fx.h"
#include "c_te_effect_dispatch.h"
#include "tier0/vprof.h"
#include "fx_line.h"
#include "fx_quad.h"
#include "view.h"
#include "particles_localspace.h"
#include "dlight.h"
#include "iefx.h"
#include "ClientEffectPrecacheSystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern Vector GetTracerOrigin( const CEffectData &data );
extern void FX_TracerSound( const Vector &start, const Vector &end, int iTracerType );

extern ConVar muzzleflash_light;

CLIENTEFFECT_REGISTER_BEGIN( PrecacheHaJTracers )
CLIENTEFFECT_MATERIAL( "effects/british_tracer" )
CLIENTEFFECT_MATERIAL( "effects/german_tracer" )
CLIENTEFFECT_REGISTER_END()

#define	TRACER_BASE_OFFSET	8
#define	TRACER_SPEED		5000 

// *HAJ 020 - Jed*  -----------------------------------------------------------
// Purpose: British .303 green tracer
//-----------------------------------------------------------------------------

// Player (Client-side) tracer
void FX_Player303Tracer( const Vector &start, const Vector &end )
{
	VPROF_BUDGET( "FX_Player303Tracer", VPROF_BUDGETGROUP_PARTICLE_RENDERING );
	Vector	shotDir, dStart, dEnd;
	float	length;

	//Find the direction of the tracer
	VectorSubtract( end, start, shotDir );
	length = VectorNormalize( shotDir );

	//We don't want to draw them if they're too close to us
	if ( length < 256 )
		return;

	//Randomly place the tracer along this line, with a random length
	VectorMA( start, TRACER_BASE_OFFSET + random->RandomFloat( -24.0f, 64.0f ), shotDir, dStart );
	VectorMA( dStart, ( length * random->RandomFloat( 0.1f, 0.6f ) ), shotDir, dEnd );

	//Create the line
	CFXStaticLine	*t;

	t = new CFXStaticLine( "Tracer", dStart, dEnd, random->RandomFloat( 0.5f, 0.75f ), 0.01f, "effects/british_tracer", 0 );
	assert( t );

	//Throw it into the list
	clienteffects->AddEffect( t );
}

// Server side tracer
void FX_303Tracer( Vector& start, Vector& end, int velocity )
{
	VPROF_BUDGET( "FX_303Tracer", VPROF_BUDGETGROUP_PARTICLE_RENDERING );
	
	float dist;
	Vector dir;

	VectorSubtract( end, start, dir );
	dist = VectorNormalize( dir );

	// Don't make short tracers.
	if ( dist >= 256 )
	{
		float length = random->RandomFloat( 64.0f, 128.0f );
		float life = ( dist + length ) / velocity;	//NOTENOTE: We want the tail to finish its run as well
		
		//Add it
		FX_AddDiscreetLine( start, dir, velocity, length, dist, random->RandomFloat( 0.75f, 0.9f ), life, "effects/british_tracer" );
	}

	FX_TracerSound( start, end, TRACER_TYPE_DEFAULT );	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Tracer303Callback( const CEffectData &data )
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if ( !player )
		return;

	// Grab the data
	Vector vecStart = GetTracerOrigin( data );
	float flVelocity = data.m_flScale;
	int iEntIndex = data.entindex();

	if ( iEntIndex && iEntIndex == player->index )
	{
		Vector	foo = data.m_vStart;
		QAngle	vangles;
		Vector	vforward, vright, vup;

		engine->GetViewAngles( vangles );
		AngleVectors( vangles, &vforward, &vright, &vup );

		VectorMA( data.m_vStart, 4, vright, foo );
		foo[2] -= 0.5f;

		FX_Player303Tracer( foo, (Vector&)data.m_vOrigin );
		return;
	}
	
	// Use default velocity if none specified
	if ( !flVelocity )
	{
		flVelocity = TRACER_SPEED;
	}

	// Do tracer effect
	FX_303Tracer( (Vector&)vecStart, (Vector&)data.m_vOrigin, flVelocity );
}

DECLARE_CLIENT_EFFECT( "303Tracer", Tracer303Callback );


// *HAJ 020 - Jed*  -----------------------------------------------------------
// Purpose: German 7.92mm tracer
//-----------------------------------------------------------------------------

// Player (Client-side) tracer
void FX_Player792Tracer( const Vector &start, const Vector &end )
{
	VPROF_BUDGET( "FX_Player792Tracer", VPROF_BUDGETGROUP_PARTICLE_RENDERING );
	Vector	shotDir, dStart, dEnd;
	float	length;

	//Find the direction of the tracer
	VectorSubtract( end, start, shotDir );
	length = VectorNormalize( shotDir );

	//We don't want to draw them if they're too close to us
	if ( length < 256 )
		return;

	//Randomly place the tracer along this line, with a random length
	VectorMA( start, TRACER_BASE_OFFSET + random->RandomFloat( -24.0f, 64.0f ), shotDir, dStart );
	VectorMA( dStart, ( length * random->RandomFloat( 0.1f, 0.6f ) ), shotDir, dEnd );

	//Create the line
	CFXStaticLine	*t;

	t = new CFXStaticLine( "Tracer", dStart, dEnd, random->RandomFloat( 0.5f, 0.75f ), 0.01f, "effects/german_tracer", 0 );
	assert( t );

	//Throw it into the list
	clienteffects->AddEffect( t );
}

// Server side tracer
void FX_792Tracer( Vector& start, Vector& end, int velocity )
{
	VPROF_BUDGET( "FX_792Tracer", VPROF_BUDGETGROUP_PARTICLE_RENDERING );
	
	float dist;
	Vector dir;

	VectorSubtract( end, start, dir );
	dist = VectorNormalize( dir );

	// Don't make short tracers.
	if ( dist >= 256 )
	{
		float length = random->RandomFloat( 64.0f, 128.0f );
		float life = ( dist + length ) / velocity;	//NOTENOTE: We want the tail to finish its run as well
		
		//Add it
		FX_AddDiscreetLine( start, dir, velocity, length, dist, random->RandomFloat( 0.75f, 0.9f ), life, "effects/german_tracer" );
	}

	FX_TracerSound( start, end, TRACER_TYPE_DEFAULT );	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void Tracer792Callback( const CEffectData &data )
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if ( !player )
		return;

	// Grab the data
	Vector vecStart = GetTracerOrigin( data );
	float flVelocity = data.m_flScale;
	int iEntIndex = data.entindex();

	if ( iEntIndex && iEntIndex == player->index )
	{
		Vector	foo = data.m_vStart;
		QAngle	vangles;
		Vector	vforward, vright, vup;

		engine->GetViewAngles( vangles );
		AngleVectors( vangles, &vforward, &vright, &vup );

		VectorMA( data.m_vStart, 4, vright, foo );
		foo[2] -= 0.5f;

		FX_Player792Tracer( foo, (Vector&)data.m_vOrigin );
		return;
	}
	
	// Use default velocity if none specified
	if ( !flVelocity )
	{
		flVelocity = TRACER_SPEED;
	}

	// Do tracer effect
	FX_792Tracer( (Vector&)vecStart, (Vector&)data.m_vOrigin, flVelocity );
}

DECLARE_CLIENT_EFFECT( "792Tracer", Tracer792Callback );


//-----------------------------------------------------------------------------
// Purpose: Special tracer type that makes a noise only
//-----------------------------------------------------------------------------
void TracerWhizCallback( const CEffectData &data )
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if ( !player )
		return;

	if ( data.entindex() && data.entindex() == player->index )
	{
		// don't do whiz for the player, we only want other players.
		return;
	}

	// Grab the data
	Vector vecStart = GetTracerOrigin( data );
	float flVelocity = data.m_flScale;

	if ( !flVelocity )
		flVelocity = TRACER_SPEED;
	
	// Do tracer effect
	FX_TracerSound( vecStart, data.m_vOrigin, TRACER_TYPE_DEFAULT );	
}

DECLARE_CLIENT_EFFECT( "WhizTracer", TracerWhizCallback );