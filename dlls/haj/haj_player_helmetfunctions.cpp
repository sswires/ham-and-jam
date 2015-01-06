//========= Copyright © 2010, Ham and Jam. ==============================//
// Purpose:  HaJ server side helmet functions.
// Notes:    Put these into a seperate file for clarity.
//
// $NoKeywords: $
//=======================================================================//

// includes
#include "cbase.h"
#include "te_effect_dispatch.h"
#include "haj_gamerules.h"
#include "haj_player.h"
#include "haj_team.h"
#include "haj_player_shared.h"
#include "haj_mapsettings_enums.h"
#include "haj_playerhelmet_shared.h"
#include "haj_playerhelmet.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern const char *g_pszHelmetModels[HELMET_COUNT];

//-----------------------------------------------------------------------------
// Purpose: Precache helmet models
//-----------------------------------------------------------------------------
void CHajPlayer::PrecacheHelmets( void )
{
	// precache helmet models
	for ( int i = 0; i < ARRAYSIZE( g_pszHelmetModels ); ++i )
		PrecacheModel( g_pszHelmetModels[i] );	
}


//-----------------------------------------------------------------------------
// Purpose: Creates the helmet entity which follows the player.
//-----------------------------------------------------------------------------
void CHajPlayer::CreateHelmet( void )
{
	SetHelmet( HELMET_NONE );

	// why try an create a helmet if you're not ready to go into battle?
	if ( !IsValidTeam( false ) || !IsValidClass( GetCurrentClass() ) )
		return;

	DestroyHelmet();

	// create the helmet entity
	if ( !m_hHelmetEnt )
	{
		m_hHelmetEnt = (CBaseEntity*)CreateEntityByName( "player_helmet" );

		// failure!
		if ( !m_hHelmetEnt )
		{
			DevMsg(1, "Could not create player helmet entity!");
			m_bHasHelmet = false;
			return;
		}

		// get the helmet index
		unsigned int n = ChooseHelmet();

		if ( n > HELMET_NONE )
		{
			m_hHelmetEnt->SetOwnerEntity( this );
			m_hHelmetEnt->SetModel( g_pszHelmetModels[n] );

			// must spawn before attaching.
			m_hHelmetEnt->Spawn();
		
			m_hHelmetEnt->FollowEntity( this, true );
			m_hHelmetEnt->AddEffects( EF_BONEMERGE_FASTCULL );
			m_hHelmetEnt->AddEffects( EF_PARENT_ANIMATES );
			m_hHelmetEnt->RemoveEFlags( EFL_USE_PARTITION_WHEN_NOT_SOLID );

			SetHelmet( n );
			m_bHasHelmet = true;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Remove the players helmet entity
//-----------------------------------------------------------------------------
void CHajPlayer::DestroyHelmet( void )
{
	// remove the helmet entity if we have one.
	if ( m_hHelmetEnt )
	{
		UTIL_RemoveImmediate( m_hHelmetEnt );
		m_hHelmetEnt = NULL;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Pop-off the players helmet
//-----------------------------------------------------------------------------
void CHajPlayer::CreateHelmetGib( const CTakeDamageInfo &info, bool spawnHelmet = true, bool dropOnly = true )
{
	// don't bother if we're not wearing one.
	if ( !m_bHasHelmet )
		return;

	// do we want a helmet?
	if ( spawnHelmet )
	{
		CEffectData	data;
		Vector origin;
		QAngle angles;

		// grab our helmet attachment in the model and use it as the origin
		int i = LookupAttachment( "helmet" );
		if ( i > 0 )
		{
			GetAttachment( i, origin, angles );
		
			data.m_vOrigin = origin;
			data.m_vAngles = angles;
			data.m_vNormal = data.m_vOrigin - info.GetDamagePosition();
			VectorNormalize( data.m_vNormal );
		
			if ( dropOnly )
				data.m_flScale = 0.0f;
			else
			{
				data.m_flScale = RemapVal( m_iHealth, 0, -500, 1, 3 );
				data.m_flScale = clamp( data.m_flScale, 1, 3 );
			}

			// pointer to the player player index for the person killed
			data.m_nEntIndex = entindex();
			
			// call the effect
			CDisablePredictionFiltering disabler;
			DispatchEffect( "HelmetGib", data );
		}
	}

	// set the player helmet variable to zero and remove it from the model.
	m_bHasHelmet = false;
	DestroyHelmet();
}

//-----------------------------------------------------------------------------
// Purpose: Returns ID of helmet model to use based on various logic
//-----------------------------------------------------------------------------
unsigned int CHajPlayer::ChooseHelmet(void)
{
	int i = GetTeamNumber();

	if ( i == TEAM_CWEALTH )
		return ChooseCommonwealthHelmet();
	
	if ( i == TEAM_AXIS )
		return ChooseAxisHelmet();

	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: Choose a helmet for Commonwealth players
//-----------------------------------------------------------------------------
unsigned int CHajPlayer::ChooseCommonwealthHelmet(void)
{
/*
	CHajGameRules *pGameRules = HajGameRules();

	if( !pGameRules )
	{
		DevMsg( 1, "Could not get GameRules when choosing Commonwealthhelmet!\n" );
		return 0;
	}
*/
	return (unsigned int)RandomInt( HELMET_MK2_STANDARD, HELMET_MK2_NET );
}

//-----------------------------------------------------------------------------
// Purpose: Choose a helmet for German players
//-----------------------------------------------------------------------------
unsigned int CHajPlayer::ChooseAxisHelmet(void)
{
	int i = GetCurrentClass();
	int r = 0;

	// for assault class you get the cap or standard helmet
	if ( i == CLASS_ASSAULT )
	{
		r = RandomInt( 1, 2 );
		if ( r == 1 )
			return CAP_M43_STANDARD;
		else
			return HELMET_M40_STANDARD;
	}
	else
	{
		r = RandomInt( 1, 3 );
		if ( r == 1 )
			return HELMET_M40_STANDARD;
		else if ( r == 2 )
			return HELMET_M40_NET;
		else
			return HELMET_M40_CAMO;
	}
}
