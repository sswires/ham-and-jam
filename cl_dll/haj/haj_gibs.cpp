//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose:  HaJ specific client-side gibs
// Notes:    Includes body and helmet gibs
//
// $NoKeywords: $
//=======================================================================//


// includes
#include "cbase.h"
#include "util_shared.h"
#include "prediction.h"
#include "fx.h"
#include "c_gib.h"
#include "c_te_effect_dispatch.h"
#include "iefx.h"
#include "decals.h"
#include "takedamageinfo.h"
#include "haj_cvars.h"
#include "haj_gamerules.h"
#include "haj_player_c.h"
#include "haj_player_shared.h"
#include "haj_playerhelmet_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// list of helmet filenames held in haj_playerhelmet_shared.cpp
extern const char *g_pszHelmetModels[HELMET_COUNT];

/////////////////////////////////////////////////////////////////////////////
// Don't alias here
#if defined(CHajPlayer)
#undef CHajPlayer	
#endif

//-----------------------------------------------------------------------------
// Purpose: Gib stuff
//-----------------------------------------------------------------------------
#define	NUM_PLAYER_GIBS 4

ConVar g_player_maxgibs( "g_player_maxgibs", "16", FCVAR_ARCHIVE );

const char *pszHumanGibs_Unique[NUM_PLAYER_GIBS] = {
	"models/gibs/hgibs.mdl",
	"models/gibs/hgibs_spine.mdl",
	"models/gibs/hgibs_scapula.mdl",
	"models/gibs/hgibs_rib.mdl",
};


//-----------------------------------------------------------------------------
// Purpose: Gib manager
//-----------------------------------------------------------------------------
void CHAJGibManager::LevelInitPreEntity( void )
{
	m_LRU.Purge();
}

CHAJGibManager s_HAJGibManager( "CHAJGibManager" );

void CHAJGibManager::AddGib( C_BaseEntity *pEntity )
{
	m_LRU.AddToTail( pEntity );
}

void CHAJGibManager::RemoveGib( C_BaseEntity *pEntity )
{
	m_LRU.FindAndRemove( pEntity );
}


//-----------------------------------------------------------------------------
// Prupose: Methods of IGameSystem
//-----------------------------------------------------------------------------
void CHAJGibManager::Update( float frametime )
{
	if ( m_LRU.Count() < g_player_maxgibs.GetInt() )
		 return;
	
	int i = 0;
	i = m_LRU.Head();

	if ( m_LRU[ i ].Get() )
	{
		 m_LRU[ i ].Get()->SetNextClientThink( gpGlobals->curtime );
	}

	m_LRU.Remove(i);
}


//-----------------------------------------------------------------------------
// Purpose: Human Gib
//-----------------------------------------------------------------------------
class C_HumanGib : public C_Gib
{
	typedef C_Gib BaseClass;
public:
	
	static C_HumanGib *C_HumanGib::CreateClientsideGib( const char *pszModelName, Vector vecOrigin, Vector vecForceDir, AngularImpulse vecAngularImp, float m_flLifetime = 20.0f )
	{
		C_HumanGib *pGib = new C_HumanGib;

		if ( pGib == NULL )
			return NULL;

		if ( pGib->InitializeGib( pszModelName, vecOrigin, vecForceDir, vecAngularImp, m_flLifetime ) == false )
			return NULL;

		s_HAJGibManager.AddGib( pGib );

		return pGib;
	}

	// Decal the surface
	virtual	void HitSurface( C_BaseEntity *pOther )	{ return; }
};


//-----------------------------------------------------------------------------
// Purpose: Gib generator
//-----------------------------------------------------------------------------
void FX_HumanGib( const Vector &origin, const Vector &direction, float scale )
{
	Vector	offset;

	int numGibs = 4;

	// Spawn all the unique gibs
	for ( int i = 0; i < numGibs; i++ )
	{
		offset = RandomVector( -16, 16 ) + origin;

		C_HumanGib::CreateClientsideGib( pszHumanGibs_Unique[i], offset, ( direction + RandomVector( -0.8f, 0.8f ) ) * ( 150 * scale ), RandomAngularImpulse( -32, 32 ), 20.0f);
	}

	// blood
	CSmartPtr<CSimpleEmitter> pSimple = CSimpleEmitter::Create( "FX_HumanGib" );
	pSimple->SetSortOrigin( origin );

	PMaterialHandle	hMaterial = pSimple->GetPMaterial( "effects/blood" );

	Vector	vDir;

	vDir.Random( -1.0f, 1.0f );

	for ( int i = 0; i < 4; i++ )
	{
		SimpleParticle *sParticle = (SimpleParticle *) pSimple->AddParticle( sizeof( SimpleParticle ), hMaterial, origin );

		if ( sParticle == NULL )
			return;

		sParticle->m_flLifetime		= 0.0f;
		sParticle->m_flDieTime		= random->RandomFloat( 1.0f, 1.5f );

		float	speed = random->RandomFloat( 16.0f, 64.0f );

		sParticle->m_vecVelocity	= vDir * -speed;
		sParticle->m_vecVelocity[2] += 16.0f;

		sParticle->m_uchColor[0]	= 192;
		sParticle->m_uchColor[1]	= 0;
		sParticle->m_uchColor[2]	= 0;
		sParticle->m_uchStartAlpha	= 255;
		sParticle->m_uchEndAlpha	= 0;
		sParticle->m_uchStartSize	= random->RandomInt( 16, 32 );
		sParticle->m_uchEndSize		= sParticle->m_uchStartSize * 2;
		sParticle->m_flRoll			= random->RandomInt( 0, 360 );
		sParticle->m_flRollDelta	= random->RandomFloat( -1.0f, 1.0f );
	}

	hMaterial = pSimple->GetPMaterial( "effects/blood2" );

	for ( int i = 0; i < 4; i++ )
	{
		SimpleParticle *sParticle = (SimpleParticle *) pSimple->AddParticle( sizeof( SimpleParticle ), hMaterial, origin );

		if ( sParticle == NULL )
		{
			return;
		}

		sParticle->m_flLifetime		= 0.0f;
		sParticle->m_flDieTime		= random->RandomFloat( 1.0f, 1.5f );

		float	speed = random->RandomFloat( 16.0f, 64.0f );

		sParticle->m_vecVelocity	= vDir * -speed;
		sParticle->m_vecVelocity[2] += 16.0f;

		sParticle->m_uchColor[0]	= 192;
		sParticle->m_uchColor[1]	= 0;
		sParticle->m_uchColor[2]	= 0;
		sParticle->m_uchStartAlpha	= random->RandomInt( 128, 192 );
		sParticle->m_uchEndAlpha	= 0;
		sParticle->m_uchStartSize	= random->RandomInt( 16, 32 );
		sParticle->m_uchEndSize		= sParticle->m_uchStartSize * 2;
		sParticle->m_flRoll			= random->RandomInt( 0, 360 );
		sParticle->m_flRollDelta	= random->RandomFloat( -1.0f, 1.0f );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Gib callback
//-----------------------------------------------------------------------------
void HumanGibCallback( const CEffectData &data )
{
	FX_HumanGib( data.m_vOrigin, data.m_vNormal, data.m_flScale );
}

DECLARE_CLIENT_EFFECT( "HumanGib", HumanGibCallback );



//-----------------------------------------------------------------------------
// Purpose: Helmet Gib
// Notes: Triggered server side, spawns a helmet that lasts for 20 secs.
//-----------------------------------------------------------------------------
class C_HelmetGib : public C_Gib
{
	typedef C_Gib BaseClass;
public:
	
	static C_HelmetGib *C_HelmetGib::CreateClientsideGib( const char *pszModelName, unsigned int nSubModel, unsigned int nSkin, Vector vecOrigin, Vector vecForceDir, AngularImpulse vecAngularImp, float m_flLifetime = 20.0f )
	{
		C_HelmetGib *pGib = new C_HelmetGib;

		if ( pGib == NULL )
			return NULL;

		if ( pGib->InitializeGib( pszModelName, vecOrigin, vecForceDir, vecAngularImp, m_flLifetime ) == false )
			return NULL;

		// set the submodel/skin
		pGib->SetBodygroup( 0, nSubModel );
		
		return pGib;
	}

	// Decal the surface
	virtual	void HitSurface( C_BaseEntity *pOther )	{ return; }
};


//-----------------------------------------------------------------------------
// Purpose: Figure out which model/submodel for helmet gib based on player model
// Todo: Expand this as needed when we add more player models/helmet types.
//-----------------------------------------------------------------------------
bool PrepHelmetModel( char *pzHelmetModel, unsigned int &nHelmetSubmodel, unsigned int &nHelmetSkin, const CEffectData data )
{
	// get a hook to the victim
	C_HajPlayer *pPlayer  = (C_HajPlayer*) C_BaseEntity::Instance( data.m_hEntity );
	
	if ( pPlayer == NULL)
		return false;

	int n = pPlayer->GetHelmet();

	// for now, only deal with the commonwealth
	if ( n != HELMET_NONE )
	{
		Q_strcpy( pzHelmetModel, g_pszHelmetModels[n] );
		nHelmetSubmodel = 0;
		nHelmetSkin = 0;
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------
// Purpose: HelmetGib callback
//-----------------------------------------------------------------------------
void HelmetGibCallback( const CEffectData &data )
{
	char			pzHelmetModel[128];
	unsigned int	nHelmetSubmodel = 0;
	unsigned int	nHelmetSkin = 0;

	if ( PrepHelmetModel( pzHelmetModel, nHelmetSubmodel, nHelmetSkin, data ) )
	{
		if ( data.m_flScale > 0.0f )
			C_HelmetGib::CreateClientsideGib( pzHelmetModel, nHelmetSubmodel, nHelmetSkin, data.m_vOrigin, data.m_vNormal * ( 150 * data.m_flScale ) , RandomAngularImpulse( -32, 32 ), 20.0f);
		else
			C_HelmetGib::CreateClientsideGib( pzHelmetModel, nHelmetSubmodel, nHelmetSkin, data.m_vOrigin, data.m_vNormal * 0, RandomAngularImpulse( 0, 0 ), 20.0f);
	}
}

DECLARE_CLIENT_EFFECT( "HelmetGib", HelmetGibCallback );