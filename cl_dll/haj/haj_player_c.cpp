
// haj_player_c.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "util_shared.h"
#include "prediction.h"
#include "fx.h"
#include "c_gib.h"
#include "c_te_effect_dispatch.h"
#include "iefx.h"
#include "decals.h"
#include "c_basetempentity.h"
#include "datacache/imdlcache.h"

#include "input.h"
#include "in_main.h"
#include "in_buttons.h"
#include "menu.h"

#include "takedamageinfo.h"
#include "haj_cvars.h"
#include "haj_gamerules.h"
#include "haj_player_c.h"
#include "haj_player_shared.h"
#include "haj_weapon_base.h"
#include "haj_playeranimstate.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


/////////////////////////////////////////////////////////////////////////////
// Don't alias here
#if defined(CHajPlayer)
	#undef CHajPlayer	
#endif

// -------------------------------------------------------------------------------- //
// Player animation event. Sent to the client when a player fires, jumps, reloads, etc..
// -------------------------------------------------------------------------------- //

class C_TEPlayerAnimEvent : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEPlayerAnimEvent, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

	virtual void PostDataUpdate( DataUpdateType_t updateType )
	{
		// Create the effect.
		C_HajPlayer *pPlayer = dynamic_cast< C_HajPlayer* >( m_hPlayer.Get() );
		if ( pPlayer && !pPlayer->IsDormant() )
		{
			pPlayer->DoAnimationEvent( (PlayerAnimEvent_t)m_iEvent.Get(), m_nData );
		}	
	}

public:
	CNetworkHandle( CBasePlayer, m_hPlayer );
	CNetworkVar( int, m_iEvent );
	CNetworkVar( int, m_nData );
};

IMPLEMENT_CLIENTCLASS_EVENT( C_TEPlayerAnimEvent, DT_TEPlayerAnimEvent, CTEPlayerAnimEvent );

BEGIN_RECV_TABLE_NOBASE( C_TEPlayerAnimEvent, DT_TEPlayerAnimEvent )
	RecvPropEHandle( RECVINFO( m_hPlayer ) ),
	RecvPropInt( RECVINFO( m_iEvent ) ),
	RecvPropInt( RECVINFO( m_nData ) )
END_RECV_TABLE()


/////////////////////////////////////////////////////////////////////////////
LINK_ENTITY_TO_CLASS(player, C_HajPlayer);

/////////////////////////////////////////////////////////////////////////////
// network data
IMPLEMENT_CLIENTCLASS_DT(C_HajPlayer, DT_HajPlayer, CHajPlayer)
	RecvPropEHandle(RECVINFO(m_hPickup)),
	RecvPropInt( RECVINFO( m_nNearbyTeamMates ) ),
	RecvPropBool( RECVINFO( m_bWeaponLowered ) ),
	RecvPropEHandle( RECVINFO( m_hHelmetEnt ) ),
	RecvPropInt( RECVINFO( m_nHelmetId ) ),
	RecvPropBool( RECVINFO( m_bInDeployZone ) ),
	RecvPropInt( RECVINFO( m_iDeployZStance ) ),
	RecvPropInt( RECVINFO( m_iObjectiveScore ) ),
	RecvPropInt( RECVINFO( m_iObjectivesCapped ) ),
	RecvPropInt( RECVINFO( m_iObjectivesDefended ) ),
	RecvPropBool( RECVINFO( m_bInBombZone ) ),
	RecvPropEHandle( RECVINFO( m_hBombZone ) ),
	RecvPropFloat( RECVINFO( m_flLastShotTime) ),
	RecvPropVector( RECVINFO( m_vecRenderOrigin ) ),
END_RECV_TABLE()

/////////////////////////////////////////////////////////////////////////////
// prediction
BEGIN_PREDICTION_DATA(C_HajPlayer)
	DEFINE_PRED_FIELD( m_flCycle, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_flLastShotTime, FIELD_FLOAT, FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_vecRenderOrigin, FIELD_VECTOR, FTYPEDESC_INSENDTABLE | FTYPEDESC_NOERRORCHECK ),
END_PREDICTION_DATA()

/////////////////////////////////////////////////////////////////////////////

// *HAJ 020* Jed
// prone toggle variable defined in in_main.cpp
extern bool g_bProneToggled;


//-----------------------------------------------------------------------------
// Purpose: Spawn some blood particles
//-----------------------------------------------------------------------------
extern void SpawnBlood(Vector vecSpot, const Vector &vecDir, int bloodColor, float flDamage);

/////////////////////////////////////////////////////////////////////////////
C_HajPlayer::C_HajPlayer()
{
	m_pHajAnimState = CreateHajPlayerAnimState( this );
	m_hPickup = NULL;
	m_hHelmetEnt = NULL;
	
	m_flRecoilTimeRemaining = 0;
	m_flPitchRecoilAccumulator = 0;
	m_flYawRecoilAccumulator = 0;
	
	m_DeployedAngles.Init();
	m_bPlayerIsProne = false;
	m_bLimitTurnSpeed = false;
	m_flForwardPressedTime = -1.0f;

	g_bProneToggled = false;
	m_bWeaponLowered = false;

	m_vecRenderOrigin = vec3_origin;

	m_fShowHUDAgain = 0;
	m_fDoJPEGCommandTime = 0;

	m_flLastProne = 0.0f;

	m_pTeamSprite = NULL;
}

/////////////////////////////////////////////////////////////////////////////
C_HajPlayer::~C_HajPlayer()
{
	m_pHajAnimState->Release();
}

void C_HajPlayer::Precache (void)
{
	BaseClass::Precache();
}


//-----------------------------------------------------------------------------
// Purpose: Spawn
//-----------------------------------------------------------------------------
void C_HajPlayer::Spawn()
{
	//Force an unprone
	g_bProneToggled = false;

	m_pTeamSprite = NULL;

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: Saves the players view angles
//-----------------------------------------------------------------------------
void C_HajPlayer::SaveViewAngles()
{
	engine->GetViewAngles( m_DeployedAngles );
}

//-----------------------------------------------------------------------------
// Purpose: Client think function
//-----------------------------------------------------------------------------
extern bool g_bProneToggled;
extern float g_fLastProne;
extern kbutton_t in_prone;
void C_HajPlayer::ClientThink( void )
{
	// prone reset timer
	if( g_fLastProne > 0 && ( IsObserver() || !IsAlive() || GetTeamNumber() == TEAM_UNASSIGNED || GetTeamNumber() == TEAM_SPECTATOR ) )
	{
		g_bProneToggled = false;
		g_fLastProne = 0.0f;

		KeyUp( &in_prone );
	}

	// apply any recoil
	ProcessRecoil();

	// if we're deployed grab our view angles
	// as the clamp code will need it
	C_HAJWeaponBase *pWpn = dynamic_cast<C_HAJWeaponBase *>( GetActiveWeapon() );

	// Not prone, MG deploying/ed or on a ladder
	if ( m_bPlayerIsProne == false && !(pWpn && ( pWpn->m_bIsBipodDeployed || pWpn->m_bIsBipodDeploying ) ) && GetMoveType() != MOVETYPE_LADDER && !m_Local.m_bVaulting )
		SaveViewAngles();

	// if we're prone, set our deployed flag
	if ( m_Local.m_bProned && !m_Local.m_bProning )
		 m_bPlayerIsProne = true;
	else
		 m_bPlayerIsProne = false;

	if( m_pTeamSprite && !IsAlive() )
		m_pTeamSprite = NULL; // nullify team sprite if dead.

	if( pWpn && ( GetFlags() & FL_USINGBIPOD ) && !( GetFlags() & FL_PRONING ) )
	{
		SetPoseParameter( "body_height", pWpn->GetDeployHeight() - 16 );
	}

	// SCREENSHOT STUFF
	if( m_fDoJPEGCommandTime > 0.0f && gpGlobals->curtime >= m_fDoJPEGCommandTime )
	{
		ConVar *jpeg_quality  = cvar->FindVar( "jpeg_quality" );

		if( jpeg_quality && jpeg_quality->GetInt() < 95 )
			jpeg_quality->SetValue( 100 );

		engine->ClientCmd( "jpeg" );
		m_fDoJPEGCommandTime = 0.0f;
	}

	if( m_fShowHUDAgain > 0.0f && gpGlobals->curtime >= m_fShowHUDAgain )
	{
		ConVar *cl_drawhud = cvar->FindVar( "cl_drawhud" );
		ConVar *r_drawviewmodel = cvar->FindVar( "r_drawviewmodel" );

		if( cl_drawhud && r_drawviewmodel )
		{
			cl_drawhud->SetValue( 1 );
			r_drawviewmodel->SetValue( 1 );
		}

		m_fShowHUDAgain = 0.0f;
	}
	// END SCREENSHOT STUFF

	BaseClass::ClientThink();

	//m_pHajAnimState->Update( EyeAngles()[YAW], EyeAngles()[PITCH] );
}

//-----------------------------------------------------------------------------
// Purpose: Applies recoil effects from weapons
//-----------------------------------------------------------------------------
void C_HajPlayer::ProcessRecoil()
{
	if ( prediction->InPrediction() && !prediction->IsFirstTimePredicted() )
		return;

	// Recoil
    QAngle viewangles;
	engine->GetViewAngles( viewangles );

	//QAngle viewangles = EyeAngles();
	
	float flYawRecoil;
	float flPitchRecoil;
	GetRecoilToAddThisFrame( flPitchRecoil, flYawRecoil );

	// Recoil
	if( flPitchRecoil > 0 )
	{
		//add the recoil pitch
		viewangles[PITCH] -= flPitchRecoil;
		viewangles[YAW] += flYawRecoil;
	}

	engine->SetViewAngles( viewangles );
}

//-----------------------------------------------------------------------------
// Purpose: How much recoil to add this frame. effectively smooths the effect
//-----------------------------------------------------------------------------
void C_HajPlayer::GetRecoilToAddThisFrame( float &flPitchRecoil, float &flYawRecoil )
{
	if( m_flRecoilTimeRemaining <= 0 )
    {
		flPitchRecoil = 0.0;
		flYawRecoil = 0.0;
		return;
	}

	float flRemaining = min( m_flRecoilTimeRemaining, gpGlobals->frametime );
	float flRecoilProportion = ( flRemaining / 0.1 ); //RECOIL_DURATION

	flPitchRecoil = m_flPitchRecoilAccumulator * flRecoilProportion;
	flYawRecoil = m_flYawRecoilAccumulator * flRecoilProportion;

	m_flRecoilTimeRemaining -= gpGlobals->frametime;
}

//-----------------------------------------------------------------------------
// Purpose: Handles input that creates player movement
//-----------------------------------------------------------------------------
void C_HajPlayer::CreateMove( float flInputSampleTime, CUserCmd *pCmd )
{
	C_HAJWeaponBase *pWpn = dynamic_cast<C_HAJWeaponBase *>( GetActiveWeapon() );

	if( IsAlive() && !IsObserver() )
	{
		// if we're prone
		if ( m_bPlayerIsProne == true || GetMoveType() == MOVETYPE_LADDER || m_Local.m_bVaulting )
		{
			// if we're prone and barely moving, don't limit turn speed but clamp view angles
			if ( GetAbsVelocity().Length2D() <= 5 ||
				( pWpn && pWpn->m_bIsBipodDeployed && !pWpn->m_bIsBipodUnDeploying ) ||
				GetMoveType() == MOVETYPE_LADDER ||
				m_Local.m_bVaulting )
			{
				m_bLimitTurnSpeed = false;

				/*if( m_bPlayerIsProne && pWpn && pWpn->m_bIsBipodDeploying )
				{
				SaveViewAngles();
				m_bLimitTurnSpeed = true;
				}*/

				ClampProneAngles ( &pCmd->viewangles );
			}
			// if we're moving forwards or backwards, limit the view turn speed and only clamp vertically
			else
			{
				m_bLimitTurnSpeed = true;
				SaveViewAngles();
				ClampProneAngles ( &pCmd->viewangles, true, false );
			}
		}
		else if( pWpn && ( pWpn->m_bIsBipodDeployed || pWpn->m_bIsBipodDeploying ) && !pWpn->m_bIsBipodUnDeploying ) // we're crouched or stood up and deployed our weapon.
		{
			ClampProneAngles ( &pCmd->viewangles ); // temp temp
		}
		// we're not deployed so don't limit speed at all
		else
		{
			m_bLimitTurnSpeed = false;
		}
	}

	BaseClass::CreateMove( flInputSampleTime, pCmd );
}

//-----------------------------------------------------------------------------
// Purpose: Clamps our view so we can only look within a range of angles
//-----------------------------------------------------------------------------

#define MAX_PRONE_PITCH -65.0
#define MIN_PRONE_PITCH 65.0
#define MAX_PRONE_YAW -60.0
#define MIN_PRONE_YAW 60.0

#define MAX_BIPOD_PITCH -30.0
#define MIN_BIPOD_PITCH 30.0
#define MAX_BIPOD_YAW -35.0
#define MIN_BIPOD_YAW 35.0

#define MAX_LADDER_PITCH -50.0
#define MIN_LADDER_PITCH 80.0
#define MAX_LADDER_YAW -45.0
#define MIN_LADDER_YAW 45.0

#define MAX_VAULT_YAW -25.0
#define MIN_VAULT_YAW 25.0

void C_HajPlayer::ClampProneAngles( QAngle *vecTestAngles, bool clampX, bool clampY )
{
	Assert( vecTestAngles );

	C_HAJWeaponBase *pWpn = dynamic_cast<C_HAJWeaponBase *>( GetActiveWeapon() );

	float max_pitch, min_pitch;
	float max_yaw, min_yaw;

	if ( GetMoveType() == MOVETYPE_LADDER )
	{
		max_pitch	= MAX_LADDER_PITCH;
		min_pitch	= MIN_LADDER_PITCH;
		max_yaw		= MAX_LADDER_YAW;
		min_yaw		= MIN_LADDER_YAW;
	}
	else if( m_Local.m_bVaulting )
	{
		max_pitch	= MAX_LADDER_PITCH;
		min_pitch	= MIN_LADDER_PITCH;
		max_yaw		= MAX_VAULT_YAW;
		min_yaw		= MIN_VAULT_YAW;
	}
	else if ( pWpn && ( pWpn->m_bIsBipodDeployed || pWpn->m_bIsBipodDeploying ) )
	{
		max_pitch	= MAX_BIPOD_PITCH;
		min_pitch	= MIN_BIPOD_PITCH;
		max_yaw		= MAX_BIPOD_YAW;
		min_yaw		= MIN_BIPOD_YAW;
	}
	else
	{
		max_pitch	= MAX_PRONE_PITCH;
		min_pitch	= MIN_PRONE_PITCH;
		max_yaw		= MAX_PRONE_YAW;
		min_yaw		= MIN_PRONE_YAW;
	}
	
	
	// Clamp Pitch
	if ( clampX )
		vecTestAngles->x = clamp( vecTestAngles->x, max_pitch, min_pitch );

	if ( clampY )
	{
		// Clamp Yaw - do a bit more work as yaw will wrap around and cause problems
		float flDeployedYawCenter = m_DeployedAngles.y;
		float flDelta = AngleNormalize( vecTestAngles->y - flDeployedYawCenter );

		if( flDelta < max_yaw )
		{
			vecTestAngles->y = flDeployedYawCenter + max_yaw;
		}
		else if( flDelta > min_yaw )
		{
			vecTestAngles->y = flDeployedYawCenter + min_yaw;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_HajPlayer::Weapon_DropPrimary( void )
{
	// only allow dropping of the primary if its the active weapon
	if ( !GetActiveWeapon() || GetActiveWeapon()->GetSlot() != WEAPON_PRIMARY_SLOT )
		return;

	engine->ServerCmd( "DropPrimary" );
}

//-----------------------------------------------------------------------------
// Handle being hit
//-----------------------------------------------------------------------------
void C_HajPlayer::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr )
{
	Vector vecOrigin = ptr->endpos - vecDir * 4;

	float flDistance = 0.0f;
	
	C_HajPlayer *pAttacker = ToHajPlayer( info.GetAttacker() );
	if ( pAttacker )
	{
		// skip is same team and friendly fire off
		if ( HajGameRules()->IsTeamplay() && pAttacker->InSameTeam( this ) == true )
			return;

		flDistance = ( ptr->endpos - info.GetAttacker()->GetAbsOrigin()).Length();
	
		// scale damage based on hitbox
	
		switch ( ptr->hitgroup )
		{
			case HITGROUP_GENERIC:
				break;
			case HITGROUP_HEAD:
				flDistance *= 1.0f;
				break;
			case HITGROUP_CHEST:
				flDistance *= 0.6f;
				break;
			case HITGROUP_STOMACH:
				flDistance *= 0.6f;
				break;
			case HITGROUP_LEFTARM:
			case HITGROUP_RIGHTARM:
				flDistance *= 0.3;
				break;
			case HITGROUP_LEFTLEG:
			case HITGROUP_RIGHTLEG:
				flDistance *= 0.3f;
				break;
			default:
				flDistance *= 1.0f;
				break;
		}

		C_HAJWeaponBase *pWeapon = dynamic_cast<C_HAJWeaponBase *>( pAttacker->GetActiveWeapon() );
		if ( pWeapon )
		{
			float distance = ( ptr->startpos - ptr->endpos ).Length();
			float effectivedist = pWeapon->GetEfficientDistance();

			// if this shot went over the max effective distance
			if( distance > effectivedist && effectivedist != 0 )
			{
				// make sure our excess distance is equal or less than the effective distance.
				float over = distance - effectivedist;
				over = clamp( over, 0, effectivedist );
					
				// return the damage factor based on a falling curve in the rance 1 - 0.
				float dmgfactor = FastCos( ( 1.57079633 / effectivedist ) * over );
				dmgfactor = clamp( dmgfactor, 0, 1 );

				flDistance *= dmgfactor;
			}
		}

		// scale down as by defauly, it scales up by 5.
		flDistance *=0.2f;

		SpawnBlood( vecOrigin, vecDir, BloodColor(), flDistance );// a little surface blood.
		//UTIL_BloodDrips( vecOrigin, vecDir, BloodColor(), (int)flDistance );
		TraceBleed( flDistance, vecDir, ptr, info.GetDamageType() );

		AddMultiDamage( info, this );
	}
}

C_HajPlayer* C_HajPlayer::GetLocalHajPlayer()
{
	return (C_HajPlayer*)C_BasePlayer::GetLocalPlayer();
}

float C_HajPlayer::CalcRoll( const QAngle& angles, const Vector& velocity, float rollangle, float rollspeed )
{
	return BaseClass::CalcRoll( angles, velocity, 3, 500 );
}

void C_HajPlayer::EnableSprint( bool bEnable )
{
	if ( !bEnable && IsSprinting() )
	{
		StopSprinting();
	}
}

void C_HajPlayer::AddEntity( void )
{
	BaseClass::AddEntity();
	//m_pHajAnimState->Update( EyeAngles()[YAW], EyeAngles()[PITCH] );
}

void C_HajPlayer::DoAnimationEvent( PlayerAnimEvent_t event, int nData /*= 0 */ )
{
	if ( IsLocalPlayer() )
	{
		if ( ( prediction->InPrediction() && !prediction->IsFirstTimePredicted() ) )
			return;
	}

	MDLCACHE_CRITICAL_SECTION();
	m_pHajAnimState->DoAnimationEvent( event, nData );
}

void C_HajPlayer::UpdateClientSideAnimation()
{
	m_pHajAnimState->Update( EyeAngles()[YAW], EyeAngles()[PITCH] );
	BaseClass::UpdateClientSideAnimation();
}


CStudioHdr * C_HajPlayer::OnNewModel( void )
{
	CStudioHdr *hdr = BaseClass::OnNewModel();

	InitializePoseParams();

	// Reset the players animation states, gestures
	if ( m_pHajAnimState )
	{
		m_pHajAnimState->OnNewModel();
	}

	return hdr;
}

const QAngle& C_HajPlayer::EyeAngles( void )
{
	return BaseClass::EyeAngles();
}

const QAngle& C_HajPlayer::GetRenderAngles()
{
	if ( IsRagdoll() )
	{
		return vec3_angle;
	}
	else
	{
		return m_pHajAnimState->GetRenderAngles();
	}
}

void C_HajPlayer::InitializePoseParams( void )
{
	CStudioHdr *hdr = GetModelPtr();
	Assert( hdr );
	if ( !hdr )
		return;

	m_headYawPoseParam = LookupPoseParameter( "head_yaw" );
	GetPoseParameterRange( m_headYawPoseParam, m_headYawMin, m_headYawMax );

	m_headPitchPoseParam = LookupPoseParameter( "head_pitch" );
	GetPoseParameterRange( m_headPitchPoseParam, m_headPitchMin, m_headPitchMax );

	for ( int i = 0; i < hdr->GetNumPoseParameters() ; i++ )
	{
		SetPoseParameter( hdr, i, 0.0 );
	}
}

void C_HajPlayer::PostDataUpdate( DataUpdateType_t updateType )
{
	SetNetworkAngles( GetLocalAngles() );
	BaseClass::PostDataUpdate( updateType );
}

void C_HajPlayer::SetLastShotTime( float fTime )
{
	if( prediction->IsFirstTimePredicted() )
	{
		m_flLastShotTime = fTime;
	}
}

const Vector& C_HajPlayer::GetRenderOrigin( void )
{
	if( m_vecRenderOrigin != vec3_origin )
		return m_vecRenderOrigin;

	return BaseClass::GetRenderOrigin();
}

CON_COMMAND( jpeg_media, "Takes a screenshot without the HUD enabled" )
{
	ConVar *cl_drawhud = cvar->FindVar( "cl_drawhud" );
	C_HajPlayer *pPlayer = C_HajPlayer::GetLocalHajPlayer();

	if( cl_drawhud && pPlayer )
	{
		cl_drawhud->SetValue( 0 );
		pPlayer->m_fShowHUDAgain = gpGlobals->curtime + 0.2;
		pPlayer->m_fDoJPEGCommandTime = gpGlobals->curtime + 0.1;
	}
}

CON_COMMAND( jpeg_media_novm, "Takes a screenshot without the HUD or view model enabled" )
{
	ConVar *cl_drawhud = cvar->FindVar( "cl_drawhud" );
	ConVar *r_drawviewmodel = cvar->FindVar( "r_drawviewmodel" );

	C_HajPlayer *pPlayer = C_HajPlayer::GetLocalHajPlayer();

	if( cl_drawhud && r_drawviewmodel && pPlayer )
	{
		cl_drawhud->SetValue( 0 );
		r_drawviewmodel->SetValue( 0 );

		pPlayer->m_fShowHUDAgain = gpGlobals->curtime + 0.2;
		pPlayer->m_fDoJPEGCommandTime = gpGlobals->curtime + 0.1;
	}
}