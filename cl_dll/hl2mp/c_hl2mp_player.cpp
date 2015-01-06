//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for HL2.
//
//=============================================================================//

#include "cbase.h"
#include "vcollide_parse.h"
#include "c_hl2mp_player.h"
#include "haj_player_shared.h"
#include "view.h"
#include "takedamageinfo.h"
#include "haj_gamerules.h"
#include "in_buttons.h"
#include "iviewrender_beams.h"			// flashlight beam
#include "r_efx.h"
#include "dlight.h"
#include "haj_gamerules.h"


// Don't alias here
#if defined( CHL2MP_Player )
#undef CHL2MP_Player	
#endif

//LINK_ENTITY_TO_CLASS( player, C_HL2MP_Player );

IMPLEMENT_CLIENTCLASS_DT(C_HL2MP_Player, DT_HL2MP_Player, CHL2MP_Player)
	RecvPropFloat( RECVINFO( m_angEyeAngles[0] ) ),
	RecvPropFloat( RECVINFO( m_angEyeAngles[1] ) ),	
	RecvPropEHandle( RECVINFO( m_hRagdoll ) ),
	RecvPropInt( RECVINFO( m_iSpawnInterpCounter ) ),
	RecvPropInt( RECVINFO( m_iPlayerSoundType) ),
	RecvPropInt( RECVINFO( m_iCurrentClass ) ), // *HAJ 020* - Jed - Player class
	RecvPropInt( RECVINFO( m_iRank ) ), // *HAJ 020* - Jed - Player rank
	RecvPropInt( RECVINFO( m_iDesiredClass )),
	RecvPropInt( RECVINFO( m_iDesiredTeam )),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_HL2MP_Player )
END_PREDICTION_DATA()

// *HAJ 020* - Jed
// Taken this out as we dont use it.
//static ConVar cl_playermodel( "cl_playermodel", "none", FCVAR_USERINFO | FCVAR_ARCHIVE, "Default Player Model");

static ConVar cl_defaultweapon( "cl_defaultweapon", "weapon_physcannon", FCVAR_USERINFO | FCVAR_ARCHIVE, "Default Spawn Weapon");

// first person ragdolls
static ConVar haj_fp_ragdoll ( "haj_fp_ragdoll", "1", FCVAR_ARCHIVE, "Allow first person ragdolls" );
static ConVar haj_fp_ragdoll_auto ( "haj_fp_ragdoll_auto", "1", FCVAR_ARCHIVE, "Autoswitch to ragdoll thirdperson-view when necessary" );
static ConVar haj_ragdoll_fx ( "haj_ragdoll_fx", "1", FCVAR_DONTRECORD | FCVAR_ARCHIVE, "Apply impact effects to ragdolls" );

void SpawnBlood (Vector vecSpot, const Vector &vecDir, int bloodColor, float flDamage);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_HL2MP_Player::C_HL2MP_Player() : /*m_PlayerAnimState( this ),*/ m_iv_angEyeAngles( "C_HL2MP_Player::m_iv_angEyeAngles" )
{
	m_iIDEntIndex = 0;
	m_iSpawnInterpCounterCache = 0;
	
	m_angEyeAngles.Init();

	AddVar( &m_angEyeAngles, &m_iv_angEyeAngles, LATCH_SIMULATION_VAR );

	//m_EntClientFlags |= ENTCLIENTFLAG_DONTUSEIK;
	m_blinkTimer.Invalidate();

	m_pFlashlightBeam = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
C_HL2MP_Player::~C_HL2MP_Player( void )
{
	ReleaseFlashlight();
}


int C_HL2MP_Player::GetIDTarget() const
{
	return m_iIDEntIndex;
}

//-----------------------------------------------------------------------------
// HAJ 020
// Purpose: Adds view offset to existing EyeAngles
// Client only
// B0B 5/9/2006
// Fixed BOB's code... works good now  - Pie
//-----------------------------------------------------------------------------
void C_HL2MP_Player::AddViewOffset( const QAngle &angleOffset )
{
#ifdef CLIENT_DLL
	QAngle vAngle;
	vAngle = EyeAngles();
	vAngle += angleOffset;
	engine->SetViewAngles(vAngle);
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Update this client's target entity
//-----------------------------------------------------------------------------
void C_HL2MP_Player::UpdateIDTarget()
{
	if ( !IsLocalPlayer() )
		return;

	// Clear old target and find a new one
	m_iIDEntIndex = 0;

	// don't show IDs in chase spec mode
	if ( GetObserverMode() == OBS_MODE_CHASE || 
		 GetObserverMode() == OBS_MODE_DEATHCAM )
		 return;

	trace_t tr;
	Vector vecStart, vecEnd;
	VectorMA( MainViewOrigin(), 1500, MainViewForward(), vecEnd );
	VectorMA( MainViewOrigin(), 10,   MainViewForward(), vecStart );
	UTIL_TraceLine( vecStart, vecEnd, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );

	if ( !tr.startsolid && tr.DidHitNonWorldEntity() )
	{
		C_BaseEntity *pEntity = tr.m_pEnt;

		if ( pEntity && (pEntity != this) )
		{
			m_iIDEntIndex = pEntity->entindex();
		}
	}
}

void C_HL2MP_Player::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr )
{
	Vector vecOrigin = ptr->endpos - vecDir * 4;

	float flDistance = 0.0f;
	
	if ( info.GetAttacker() )
	{
		flDistance = (ptr->endpos - info.GetAttacker()->GetAbsOrigin()).Length();
	}

	if ( m_takedamage )
	{
		AddMultiDamage( info, this );

		int blood = BloodColor();
		
		CBaseEntity *pAttacker = info.GetAttacker();

		if ( pAttacker )
		{
			if ( HL2MPRules()->IsTeamplay() && pAttacker->InSameTeam( this ) == true )
				return;
		}

		if ( blood != DONT_BLEED )
		{
			SpawnBlood( vecOrigin, vecDir, blood, flDistance );// a little surface blood.
			TraceBleed( flDistance, vecDir, ptr, info.GetDamageType() );
		}
	}
}


C_HL2MP_Player* C_HL2MP_Player::GetLocalHL2MPPlayer()
{
	return (C_HL2MP_Player*)C_BasePlayer::GetLocalPlayer();
}

void C_HL2MP_Player::Initialize( void )
{
	m_headYawPoseParam = LookupPoseParameter( "head_yaw" );
	GetPoseParameterRange( m_headYawPoseParam, m_headYawMin, m_headYawMax );

	m_headPitchPoseParam = LookupPoseParameter( "head_pitch" );
	GetPoseParameterRange( m_headPitchPoseParam, m_headPitchMin, m_headPitchMax );

	CStudioHdr *hdr = GetModelPtr();
	for ( int i = 0; i < hdr->GetNumPoseParameters() ; i++ )
	{
		SetPoseParameter( hdr, i, 0.0 );
	}
}

CStudioHdr *C_HL2MP_Player::OnNewModel( void )
{
	CStudioHdr *hdr = BaseClass::OnNewModel();
	
	Initialize( );

	return hdr;
}

void C_HL2MP_Player::ClientThink( void )
{
	
	// HAJ 020 - Jed
	//As we don't want out models looking around, we can save
	//some clock cycles by not bothing to figure out a vector for
	//the eyes to look at.
	//m_PlayerAnimState.Update();
	
	Vector vForward;
	AngleVectors( GetLocalAngles(), &vForward );
	
	m_viewtarget = GetAbsOrigin() + vForward * 512;
	
	// blinking
	if (m_blinkTimer.IsElapsed())
	{
		m_blinktoggle = !m_blinktoggle;
		m_blinkTimer.Start( RandomFloat( 1.5f, 4.0f ) );
	}

	UpdateIDTarget();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int C_HL2MP_Player::DrawModel( int flags )
{
	if ( !m_bReadyToDraw )
		return 0;

    return BaseClass::DrawModel(flags);
}

//-----------------------------------------------------------------------------
// Should this object receive shadows?
//-----------------------------------------------------------------------------
bool C_HL2MP_Player::ShouldReceiveProjectedTextures( int flags )
{
	Assert( flags & SHADOW_FLAGS_PROJECTED_TEXTURE_TYPE_MASK );

	if ( IsEffectActive( EF_NODRAW ) )
		 return false;

	if( flags & SHADOW_FLAGS_FLASHLIGHT )
	{
		return true;
	}

	return BaseClass::ShouldReceiveProjectedTextures( flags );
}

void C_HL2MP_Player::DoImpactEffect( trace_t &tr, int nDamageType )
{
	if ( GetActiveWeapon() )
	{
		GetActiveWeapon()->DoImpactEffect( tr, nDamageType );
		return;
	}

	BaseClass::DoImpactEffect( tr, nDamageType );
}

void C_HL2MP_Player::PreThink( void )
{

	QAngle vTempAngles = GetLocalAngles();

	if ( GetLocalPlayer() == this )
	{
		vTempAngles[PITCH] = EyeAngles()[PITCH];
	}
	else
	{
		vTempAngles[PITCH] = m_angEyeAngles[PITCH];
	}

	if ( vTempAngles[YAW] < 0.0f )
	{
		vTempAngles[YAW] += 360.0f;
	}

	SetLocalAngles( vTempAngles );

	BaseClass::PreThink();

	HandleSpeedChanges();

	if( IsSprinting() )
	{
		if ( !CanSprint() || m_HL2Local.m_flSuitPower <= 0.0f )
		{
			StopSprinting();
			m_nButtons &= ~IN_SPEED;
		}
	}

	//engine->Con_NPrintf( 0, "Absolute Velocity: %f", GetAbsVelocity().Length2D() );
}

const QAngle &C_HL2MP_Player::EyeAngles()
{
	if( IsLocalPlayer() )
	{
		return BaseClass::EyeAngles();
	}
	else
	{
		return m_angEyeAngles;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_HL2MP_Player::AddEntity( void )
{
	BaseClass::AddEntity();

	QAngle vTempAngles = GetLocalAngles();
	vTempAngles[PITCH] = m_angEyeAngles[PITCH];

	SetLocalAngles( vTempAngles );
		
	//m_PlayerAnimState.Update();

	// Zero out model pitch, blending takes care of all of it.
	SetLocalAnglesDim( X_INDEX, 0 );

	if( this != C_BasePlayer::GetLocalPlayer() )
	{
		if ( IsEffectActive( EF_DIMLIGHT ) )
		{
			int iAttachment = LookupAttachment( "anim_attachment_RH" );

			if ( iAttachment < 0 )
				return;

			Vector vecOrigin;
			QAngle eyeAngles = m_angEyeAngles;
	
			GetAttachment( iAttachment, vecOrigin, eyeAngles );

			Vector vForward;
			AngleVectors( eyeAngles, &vForward );
				
			trace_t tr;
			UTIL_TraceLine( vecOrigin, vecOrigin + (vForward * 200), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

			if( !m_pFlashlightBeam )
			{
				BeamInfo_t beamInfo;
				beamInfo.m_nType = TE_BEAMPOINTS;
				beamInfo.m_vecStart = tr.startpos;
				beamInfo.m_vecEnd = tr.endpos;
				beamInfo.m_pszModelName = "sprites/glow01.vmt";
				beamInfo.m_pszHaloName = "sprites/glow01.vmt";
				beamInfo.m_flHaloScale = 3.0;
				beamInfo.m_flWidth = 8.0f;
				beamInfo.m_flEndWidth = 35.0f;
				beamInfo.m_flFadeLength = 300.0f;
				beamInfo.m_flAmplitude = 0;
				beamInfo.m_flBrightness = 60.0;
				beamInfo.m_flSpeed = 0.0f;
				beamInfo.m_nStartFrame = 0.0;
				beamInfo.m_flFrameRate = 0.0;
				beamInfo.m_flRed = 255.0;
				beamInfo.m_flGreen = 255.0;
				beamInfo.m_flBlue = 255.0;
				beamInfo.m_nSegments = 8;
				beamInfo.m_bRenderable = true;
				beamInfo.m_flLife = 0.5;
				beamInfo.m_nFlags = FBEAM_FOREVER | FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM;
				
				m_pFlashlightBeam = beams->CreateBeamPoints( beamInfo );
			}

			if( m_pFlashlightBeam )
			{
				BeamInfo_t beamInfo;
				beamInfo.m_vecStart = tr.startpos;
				beamInfo.m_vecEnd = tr.endpos;
				beamInfo.m_flRed = 255.0;
				beamInfo.m_flGreen = 255.0;
				beamInfo.m_flBlue = 255.0;

				beams->UpdateBeamInfo( m_pFlashlightBeam, beamInfo );

				dlight_t *el = effects->CL_AllocDlight( 0 );
				el->origin = tr.endpos;
				el->radius = 50; 
				el->color.r = 200;
				el->color.g = 200;
				el->color.b = 200;
				el->die = gpGlobals->curtime + 0.1;
			}
		}
		else if ( m_pFlashlightBeam )
		{
			ReleaseFlashlight();
		}
	}
}

ShadowType_t C_HL2MP_Player::ShadowCastType( void ) 
{
	if ( !IsVisible() )
		 return SHADOWS_NONE;

	return SHADOWS_RENDER_TO_TEXTURE_DYNAMIC;
}


const QAngle& C_HL2MP_Player::GetRenderAngles()
{
	if ( IsRagdoll() )
	{
		return vec3_angle;
	}
	else
	{
		return BaseClass::GetRenderAngles();
	}
}

bool C_HL2MP_Player::ShouldDraw( void )
{
	// If we're dead, our ragdoll will be drawn for us instead.
	if ( !IsAlive() )
		return false;

	if( GetTeamNumber() == TEAM_SPECTATOR )
		return false;

	if( IsLocalPlayer() && IsRagdoll() )
		return true;
	
	if ( IsRagdoll() )
		return false;

	return BaseClass::ShouldDraw();
}

void C_HL2MP_Player::NotifyShouldTransmit( ShouldTransmitState_t state )
{
	if ( state == SHOULDTRANSMIT_END )
	{
		if( m_pFlashlightBeam != NULL )
		{
			ReleaseFlashlight();
		}
	}

	BaseClass::NotifyShouldTransmit( state );
}

void C_HL2MP_Player::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( type == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}

	UpdateVisibility();
}

void C_HL2MP_Player::PostDataUpdate( DataUpdateType_t updateType )
{
	if ( m_iSpawnInterpCounter != m_iSpawnInterpCounterCache )
	{
		MoveToLastReceivedPosition( true );
		ResetLatched();
		m_iSpawnInterpCounterCache = m_iSpawnInterpCounter;
	}

	BaseClass::PostDataUpdate( updateType );
}

void C_HL2MP_Player::ReleaseFlashlight( void )
{
	if( m_pFlashlightBeam )
	{
		m_pFlashlightBeam->flags = 0;
		m_pFlashlightBeam->die = gpGlobals->curtime - 1;

		m_pFlashlightBeam = NULL;
	}
}

float C_HL2MP_Player::GetFOV( void )
{
	//Find our FOV with offset zoom value
	float flFOVOffset = C_BasePlayer::GetFOV() + GetZoom();

	// Clamp FOV in MP
	int min_fov = GetMinFOV();
	
	// Don't let it go too low
	flFOVOffset = max( min_fov, flFOVOffset );

	return flFOVOffset;
}

//=========================================================
// Autoaim
// set crosshair position to point to enemey
//=========================================================
Vector C_HL2MP_Player::GetAutoaimVector( float flDelta )
{
	// Never autoaim a predicted weapon (for now)
	Vector	forward;
	AngleVectors( EyeAngles() + m_Local.m_vecPunchAngle, &forward );
	return	forward;
}

//-----------------------------------------------------------------------------
// Purpose: Returns whether or not we are allowed to sprint now.
//-----------------------------------------------------------------------------
bool C_HL2MP_Player::CanSprint( void )
{
	// *HAJ 020* - Jed
	return ( ( !m_Local.m_bDucked || !m_Local.m_bDucking ) && 
		     ( !m_Local.m_bProned || !m_Local.m_bProning || !m_Local.m_bDoProne ) &&
			 ( m_nButtons & (IN_FORWARD|IN_BACK) ) &&
			 ( GetAbsVelocity().Length() > 5.0f ) &&
			 ( GetWaterLevel() != 3 ) );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void C_HL2MP_Player::StartSprinting( void )
{
	if( m_HL2Local.m_flSuitPower < 10 )
	{
		// Don't sprint unless there's a reasonable
		// amount of suit power.
		CPASAttenuationFilter filter( this );
		filter.UsePredictionRules();
		EmitSound( filter, entindex(), "HL2Player.SprintNoPower" );
		return;
	}

	CPASAttenuationFilter filter( this );
	filter.UsePredictionRules();
	EmitSound( filter, entindex(), "HL2Player.SprintStart" );

	SetMaxSpeed( HAJ_SPRINT_SPEED );
	m_fIsSprinting = true;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void C_HL2MP_Player::StopSprinting( void )
{
	SetMaxSpeed( HAJ_NORM_SPEED );
	m_fIsSprinting = false;
	m_nButtons &= ~IN_SPEED;
}

void C_HL2MP_Player::HandleSpeedChanges( void )
{
	int buttonsChanged = m_afButtonPressed | m_afButtonReleased;

	if( buttonsChanged & IN_SPEED )
	{
		// The state of the sprint/run button has changed.
		if ( IsSuitEquipped() )
		{
			if ( !(m_afButtonPressed & IN_SPEED)  && IsSprinting() )
			{
				StopSprinting();
			}
			else if ( (m_afButtonPressed & IN_SPEED) && !IsSprinting() )
			{
				if ( CanSprint() )
				{
					StartSprinting();
				}
				else
				{
					// Reset key, so it will be activated post whatever is suppressing it.
					m_nButtons &= ~IN_SPEED;
				}
			}
		}
	}
}

void C_HL2MP_Player::ItemPreFrame( void )
{
	if ( GetFlags() & FL_FROZEN )
		 return;

	// Disallow shooting while zooming
	if ( m_nButtons & IN_ZOOM )
	{
		//FIXME: Held weapons like the grenade get sad when this happens
		m_nButtons &= ~(IN_ATTACK|IN_ATTACK2);
	}

	BaseClass::ItemPreFrame();

}
	
void C_HL2MP_Player::ItemPostFrame( void )
{
	if ( GetFlags() & FL_FROZEN )
		 return;

	BaseClass::ItemPostFrame();
}

C_BaseAnimating *C_HL2MP_Player::BecomeRagdollOnClient( bool bCopyEntity )
{
	// Let the C_CSRagdoll entity do this.
	// m_builtRagdoll = true;
	return NULL;
}

// *HAJ 020* - Jed
//-----------------------------------------------------------------------------
// Purpose: Input handling
//-----------------------------------------------------------------------------
void C_HL2MP_Player::CreateMove( float flInputSampleTime, CUserCmd *pCmd )
{
	// I'll probably use this function for locking the player when deployed
	// for now, just pass through
	BaseClass::CreateMove( flInputSampleTime, pCmd );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_HL2MP_Player::CalcView( Vector &eyeOrigin, QAngle &eyeAngles, float &zNear, float &zFar, float &fov )
{

	if ( m_lifeState != LIFE_ALIVE && !IsObserver() )
	{
		// *HAJ 020* - Jed
		// First person ragdolls
		if ( haj_fp_ragdoll.GetBool() && m_hRagdoll.Get() )
		{
			// pointer to the ragdoll
			C_HL2MPRagdoll *pCRagdoll = (C_HL2MPRagdoll*)m_hRagdoll.Get();
			
			if ( !pCRagdoll )
				goto thirdperson;

			// gets its origin and angles
			pCRagdoll->GetAttachment( pCRagdoll->LookupAttachment( "eyes" ), eyeOrigin, eyeAngles );
			Vector vForward; 
			AngleVectors( eyeAngles, &vForward );

			if ( haj_fp_ragdoll_auto.GetBool() )
			{
				// DM: Don't use first person view when we are very close to something
				trace_t tr;
				UTIL_TraceLine( eyeOrigin, eyeOrigin + ( vForward * 10000 ), MASK_ALL, pCRagdoll, COLLISION_GROUP_NONE, &tr );

				if ( (!(tr.fraction < 1) || (tr.endpos.DistTo(eyeOrigin) > 25)) )
					return;
			}
			else
				return;
		}

	thirdperson:
		
		eyeOrigin = vec3_origin;
		eyeAngles = vec3_angle;
		
		Vector origin = EyePosition();			

		IRagdoll *pRagdoll = GetRepresentativeRagdoll();

		if ( pRagdoll )
		{
			origin = pRagdoll->GetRagdollOrigin();
			origin.z += VEC_DEAD_VIEWHEIGHT.z; // look over ragdoll, not through
		}
		else
		{
			origin = GetAbsOrigin();
			origin.z += VEC_DEAD_VIEWHEIGHT.z; // look over ragdoll, not through
		}

		BaseClass::CalcView( eyeOrigin, eyeAngles, zNear, zFar, fov );

		eyeOrigin = origin;
		
		Vector vForward; 
		AngleVectors( eyeAngles, &vForward );

		VectorNormalize( vForward );
		VectorMA( origin, -CHASE_CAM_DISTANCE, vForward, eyeOrigin );

		Vector WALL_MIN( -WALL_OFFSET, -WALL_OFFSET, -WALL_OFFSET );
		Vector WALL_MAX( WALL_OFFSET, WALL_OFFSET, WALL_OFFSET );

		trace_t trace; // clip against world
		C_BaseEntity::PushEnableAbsRecomputations( false ); // HACK don't recompute positions while doing RayTrace
		UTIL_TraceHull( origin, eyeOrigin, WALL_MIN, WALL_MAX, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &trace );
		C_BaseEntity::PopEnableAbsRecomputations();

		if (trace.fraction < 1.0)
		{
			eyeOrigin = trace.endpos;
		}
		
		return;
	}

	BaseClass::CalcView( eyeOrigin, eyeAngles, zNear, zFar, fov );
}

IRagdoll* C_HL2MP_Player::GetRepresentativeRagdoll() const
{
	if ( m_hRagdoll.Get() )
	{
		C_HL2MPRagdoll *pRagdoll = (C_HL2MPRagdoll*)m_hRagdoll.Get();

		return pRagdoll->GetIRagdoll();
	}
	else
	{
		return NULL;
	}
}

//HL2MPRAGDOLL


IMPLEMENT_CLIENTCLASS_DT_NOBASE( C_HL2MPRagdoll, DT_HL2MPRagdoll, CHL2MPRagdoll )
	RecvPropVector( RECVINFO(m_vecRagdollOrigin) ),
	RecvPropEHandle( RECVINFO( m_hPlayer ) ),
	RecvPropInt( RECVINFO( m_nModelIndex ) ),
	RecvPropInt( RECVINFO(m_nForceBone) ),
	RecvPropVector( RECVINFO(m_vecForce) ),
	RecvPropVector( RECVINFO( m_vecRagdollVelocity ) )
END_RECV_TABLE()



C_HL2MPRagdoll::C_HL2MPRagdoll()
{

}

C_HL2MPRagdoll::~C_HL2MPRagdoll()
{
	PhysCleanupFrictionSounds( this );

	if ( m_hPlayer )
	{
		m_hPlayer->CreateModelInstance();
	}
}

void C_HL2MPRagdoll::Interp_Copy( C_BaseAnimatingOverlay *pSourceEntity )
{
	if ( !pSourceEntity )
		return;
	
	VarMapping_t *pSrc = pSourceEntity->GetVarMapping();
	VarMapping_t *pDest = GetVarMapping();
    	
	// Find all the VarMapEntry_t's that represent the same variable.
	for ( int i = 0; i < pDest->m_Entries.Count(); i++ )
	{
		VarMapEntry_t *pDestEntry = &pDest->m_Entries[i];
		const char *pszName = pDestEntry->watcher->GetDebugName();
		for ( int j=0; j < pSrc->m_Entries.Count(); j++ )
		{
			VarMapEntry_t *pSrcEntry = &pSrc->m_Entries[j];
			if ( !Q_strcmp( pSrcEntry->watcher->GetDebugName(), pszName ) )
			{
				pDestEntry->watcher->Copy( pSrcEntry->watcher );
				break;
			}
		}
	}
}

void C_HL2MPRagdoll::ImpactTrace( trace_t *pTrace, int iDamageType, char *pCustomImpactName )
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();

	if( !pPhysicsObject )
		return;

	Vector dir = pTrace->endpos - pTrace->startpos;

	if ( iDamageType == DMG_BLAST )
	{
		dir *= 4000;  // adjust impact strenght
				
		// apply force at object mass center
		pPhysicsObject->ApplyForceCenter( dir );
	}
	else
	{
		Vector hitpos;  
	
		VectorMA( pTrace->startpos, pTrace->fraction, dir, hitpos );
		VectorNormalize( dir );

		dir *= 4000;  // adjust impact strenght

		// apply force where we hit it
		pPhysicsObject->ApplyForceOffset( dir, hitpos );	

		// Blood spray!

		// *HAJ 020* - Jed
		//apply ragdoll impact effects
		if ( haj_ragdoll_fx.GetBool() )
		{

			if ( iDamageType == DMG_BULLET )
			{
				SpawnBlood( pTrace->endpos, dir, BloodColor(), 5.0f );	//blood spray
				TraceBleed( 1.0f, dir, pTrace, iDamageType );			// blood decal
			}
			else if ( iDamageType == DMG_CLUB )
			{
				SpawnBlood( pTrace->endpos, dir, BloodColor(), 2.5f );	//blood spray
			}

			/* Commented out for now - too buggy

			// call base impact trace to apply bloody decals to model
			CBaseEntity *pEntity = pTrace->m_pEnt;
	 
			if ( pEntity )
			{
				// Build the impact data
				CEffectData data;
				data.m_vOrigin = pTrace->endpos;
				data.m_vStart = pTrace->startpos;
				data.m_nSurfaceProp = pTrace->surface.surfaceProps;
				data.m_nDamageType = iDamageType;
				data.m_nHitBox = pTrace->hitbox;
				data.m_nEntIndex = pEntity->entindex();

				// Send it on its way
				if ( !pCustomImpactName )
				{
					DispatchEffect( "Impact", data );
				}
				else
				{
					DispatchEffect( pCustomImpactName, data );
				}
			}
			*/
		}
	}

	m_pRagdoll->ResetRagdollSleepAfterTime();
}


void C_HL2MPRagdoll::CreateHL2MPRagdoll( void )
{
	// First, initialize all our data. If we have the player's entity on our client,
	// then we can make ourselves start out exactly where the player is.
	C_HL2MP_Player *pPlayer = dynamic_cast< C_HL2MP_Player* >( m_hPlayer.Get() );
	
	// *HAJ 020* - Jed
	// Call me picky but I don't like the idea of blindly assuming we got a valid pointer...
	if ( !pPlayer )
		return;

	if ( pPlayer && !pPlayer->IsDormant() )
	{
		// move my current model instance to the ragdoll's so decals are preserved.
		pPlayer->SnatchModelInstance( this );

		VarMapping_t *varMap = GetVarMapping();

		// Copy all the interpolated vars from the player entity.
		// The entity uses the interpolated history to get bone velocity.
		bool bRemotePlayer = (pPlayer != C_BasePlayer::GetLocalPlayer());			
		if ( bRemotePlayer )
		{
			Interp_Copy( pPlayer );

			SetAbsAngles( pPlayer->GetRenderAngles() );
			GetRotationInterpolator().Reset();

			m_flAnimTime = pPlayer->m_flAnimTime;
			SetSequence( pPlayer->GetSequence() );
			m_flPlaybackRate = pPlayer->GetPlaybackRate();
		}
		else
		{
			// This is the local player, so set them in a default
			// pose and slam their velocity, angles and origin
			SetAbsOrigin( m_vecRagdollOrigin );
			
			SetAbsAngles( pPlayer->GetRenderAngles() );

			SetAbsVelocity( m_vecRagdollVelocity );

			int iSeq = pPlayer->GetSequence();
			if ( iSeq == -1 )
			{
				Assert( false );	// missing walk_lower?
				iSeq = 0;
			}
			
			SetSequence( iSeq );	// walk_lower, basic pose
			SetCycle( 0.0 );

			Interp_Reset( varMap );
		}		
	}
	else
	{
		// overwrite network origin so later interpolation will
		// use this position
		SetNetworkOrigin( m_vecRagdollOrigin );

		SetAbsOrigin( m_vecRagdollOrigin );
		SetAbsVelocity( m_vecRagdollVelocity );

		Interp_Reset( GetVarMapping() );
		
	}

	SetModelIndex( m_nModelIndex );
	
	// *HAJ 020* - Jed
	// Give our ragdoll the same rank badge as the person we killed
	SetBodygroup( PLAYER_BODYGROUP_RANK, pPlayer->GetCurrentRank() );

	// Make us a ragdoll..
	m_nRenderFX = kRenderFxRagdoll;

	BecomeRagdollOnClient( false );
}


void C_HL2MPRagdoll::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( type == DATA_UPDATE_CREATED )
	{
		CreateHL2MPRagdoll();
	
		IPhysicsObject *pPhysicsObject = VPhysicsGetObject();

		if( pPhysicsObject )
		{
			AngularImpulse aVelocity(0,0,0);

			Vector vecExaggeratedVelocity = 3 * m_vecRagdollVelocity;

			pPhysicsObject->AddVelocity( &vecExaggeratedVelocity, &aVelocity );
		}
	}
}

IRagdoll* C_HL2MPRagdoll::GetIRagdoll() const
{
	return m_pRagdoll;
}

void C_HL2MPRagdoll::UpdateOnRemove( void )
{
	VPhysicsSetObject( NULL );

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose: clear out any face/eye values stored in the material system
//-----------------------------------------------------------------------------
void C_HL2MPRagdoll::SetupWeights( void )
{
	BaseClass::SetupWeights( );

	static float destweight[128];
	static bool bIsInited = false;

	CStudioHdr *hdr = GetModelPtr();
	if ( !hdr )
	{
		return;
	}

	if (hdr->numflexdesc() > 0)
	{
		if (!bIsInited)
		{
			int i;
			for (i = 0; i < 128; i++)
			{
				destweight[i] = 0.0f;
			}
			bIsInited = true;
		}
		modelrender->SetFlexWeights( hdr->numflexdesc(), destweight );
	}

	if (m_iEyeAttachment > 0)
	{
		matrix3x4_t attToWorld;
		if (GetAttachment( m_iEyeAttachment, attToWorld ))
		{
			Vector local, tmp;
			local.Init( 1000.0f, 0.0f, 0.0f );
			VectorTransform( local, attToWorld, tmp );
			modelrender->SetViewTarget( tmp );
		}
	}
}