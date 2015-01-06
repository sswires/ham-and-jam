//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "base_playeranimstate.h"
#include "tier0/vprof.h"
#include "animation.h"
#include "studio.h"
#include "apparent_velocity_helper.h"
#include "utldict.h"

#include "haj_playeranimstate.h"
#include "base_playeranimstate.h"
#include "datacache/imdlcache.h"
#include "haj_weapon_base.h"

#ifdef CLIENT_DLL
#include "haj_player_c.h"
#else
#include "haj_player.h"
#endif

#define HAJ_RUN_SPEED				250.0f
#define HAJ_CROUCHWALK_SPEED		110.0f

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
// Output : CMultiPlayerAnimState*
//-----------------------------------------------------------------------------
CHajPlayerAnimState* CreateHajPlayerAnimState( CHajPlayer *pPlayer )
{
	MDLCACHE_CRITICAL_SECTION();

	// Setup the movement data.
	MultiPlayerMovementData_t movementData;
	movementData.m_flBodyYawRate = 720.0f;
	movementData.m_flRunSpeed = HAJ_NORM_SPEED;
	movementData.m_flWalkSpeed = HAJ_WALK_SPEED;
	movementData.m_flSprintSpeed = HAJ_RUN_SPEED;

	// Create animation state for this player.
	CHajPlayerAnimState *pRet = new CHajPlayerAnimState( pPlayer, movementData );

	// Specific SDK player initialization.
	pRet->InitHajAnimState( pPlayer );

	return pRet;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :  - 
//-----------------------------------------------------------------------------
CHajPlayerAnimState::CHajPlayerAnimState()
{
	m_pHajPlayer = NULL;

	// Don't initialize SDK specific variables here. Init them in InitSDKAnimState()
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//			&movementData - 
//-----------------------------------------------------------------------------
CHajPlayerAnimState::CHajPlayerAnimState( CBasePlayer *pPlayer, MultiPlayerMovementData_t &movementData )
: CMultiPlayerAnimState( pPlayer, movementData )
{
	m_pHajPlayer = NULL;

	// Don't initialize SDK specific variables here. Init them in InitSDKAnimState()
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :  - 
//-----------------------------------------------------------------------------
CHajPlayerAnimState::~CHajPlayerAnimState()
{
}

//-----------------------------------------------------------------------------
// Purpose: Initialize Team Fortress specific animation state.
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CHajPlayerAnimState::InitHajAnimState( CHajPlayer *pPlayer )
{
	m_pHajPlayer = pPlayer;

	m_iProneActivity = ACT_MP_STAND_TO_PRONE;
	m_bProneTransition = false;
	m_bVaulting = false;
	m_bProneTransitionFirstFrame = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHajPlayerAnimState::ClearAnimationState( void )
{
	m_bProneTransition = false;
	m_bProneTransitionFirstFrame = false;

	BaseClass::ClearAnimationState();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : actDesired - 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CHajPlayerAnimState::TranslateActivity( Activity actDesired )
{
	Activity translateActivity = BaseClass::TranslateActivity( actDesired );

	if ( GetHajPlayer()->GetActiveWeapon() )
	{
		translateActivity = GetHajPlayer()->GetActiveWeapon()->ActivityOverride( translateActivity, false );
	}

	return translateActivity;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHajPlayerAnimState::Update( float eyeYaw, float eyePitch )
{
	// Profile the animation update.
	VPROF( "CMultiPlayerAnimState::Update" );

	// Get the SDK player.
	CHajPlayer *pHajPlayer = GetHajPlayer();
	if ( !pHajPlayer )
		return;

	// Get the studio header for the player.
	CStudioHdr *pStudioHdr = pHajPlayer->GetModelPtr();
	if ( !pStudioHdr )
		return;

	// Check to see if we should be updating the animation state - dead, ragdolled?
	if ( !ShouldUpdateAnimState() )
	{
		ClearAnimationState();
		return;
	}

	// Store the eye angles.
	m_flEyeYaw = AngleNormalize( eyeYaw );
	m_flEyePitch = AngleNormalize( eyePitch );

	// Compute the player sequences.
	ComputeSequences( pStudioHdr );

	if ( SetupPoseParameters( pStudioHdr ) )
	{
		// Pose parameter - what direction are the player's legs running in.
		ComputePoseParam_MoveYaw( pStudioHdr );

		// Pose parameter - Torso aiming (up/down).
		ComputePoseParam_AimPitch( pStudioHdr );

		// Pose parameter - Torso aiming (rotation).
		ComputePoseParam_AimYaw( pStudioHdr );
	}

#ifdef CLIENT_DLL 
	m_pHajPlayer->SetPlaybackRate( 1.0f );
#endif
}
extern ConVar mp_slammoveyaw;
float SnapYawTo( float flValue );
void CHajPlayerAnimState::ComputePoseParam_MoveYaw( CStudioHdr *pStudioHdr )
{
	// Get the estimated movement yaw.
	EstimateYaw();

	// Get the view yaw.
	float flAngle = AngleNormalize( m_flEyeYaw );

	// Calc side to side turning - the view vs. movement yaw.
	float flYaw = flAngle - m_PoseParameterData.m_flEstimateYaw;
	flYaw = AngleNormalize( -flYaw );

	// Get the current speed the character is running.
	bool bIsMoving;
	float flPlaybackRate = CalcMovementPlaybackRate( &bIsMoving );

	// Setup the 9-way blend parameters based on our speed and direction.
	Vector2D vecCurrentMoveYaw( 0.0f, 0.0f );
	if ( bIsMoving )
	{
		if ( mp_slammoveyaw.GetBool() )
		{
			flYaw = SnapYawTo( flYaw );
		}
		vecCurrentMoveYaw.x = cos( DEG2RAD( flYaw ) ) * flPlaybackRate;
		vecCurrentMoveYaw.y = -sin( DEG2RAD( flYaw ) ) * flPlaybackRate;
	}

	// Set the 9-way blend movement pose parameters.
	GetBasePlayer()->SetPoseParameter( pStudioHdr, m_PoseParameterData.m_iMoveX, vecCurrentMoveYaw.x );
	GetBasePlayer()->SetPoseParameter( pStudioHdr, m_PoseParameterData.m_iMoveY, -vecCurrentMoveYaw.y ); //Tony; flip it

	m_DebugAnimData.m_vecMoveYaw = vecCurrentMoveYaw;
}
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : event - 
//-----------------------------------------------------------------------------
void CHajPlayerAnimState::DoAnimationEvent( PlayerAnimEvent_t event, int nData )
{
	Activity iGestureActivity = ACT_INVALID;

	switch( event )
	{
	case PLAYERANIMEVENT_ATTACK_PRIMARY:
		{
			// Weapon primary fire.
			CHAJWeaponBase* pWeapon = dynamic_cast<CHAJWeaponBase*>(m_pHajPlayer->GetActiveWeapon());

			if( pWeapon && ( pWeapon->m_bIsBipodDeploying || pWeapon->m_bIsBipodDeployed ) )
			{
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ( m_pHajPlayer->GetFlags() & FL_PRONING ) ? ACT_HAJ_PRIMARYATTACK_PRONE_DEPLOYED : ACT_HAJ_PRIMARYATTACK_DEPLOYED );
			}
			else if ( m_pHajPlayer->GetFlags() & FL_PRONING )
			{
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_PRONE_PRIMARYFIRE );
			}
			else if ( m_pHajPlayer->GetFlags() & FL_DUCKING )
			{
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_CROUCH_PRIMARYFIRE );
			}
			else
			{
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_STAND_PRIMARYFIRE );
			}

			//iGestureActivity = ACT_VM_PRIMARYATTACK;
			break;
		}

	case PLAYERANIMEVENT_VOICE_COMMAND_GESTURE:
		{
			if ( !IsGestureSlotActive( GESTURE_SLOT_ATTACK_AND_RELOAD ) )
			{
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, (Activity)nData );
			}
			//iGestureActivity = ACT_VM_IDLE; //TODO?
			break;
		}
	case PLAYERANIMEVENT_ATTACK_SECONDARY:
		{
			// Weapon secondary fire.
			if ( m_pHajPlayer->GetFlags() & FL_PRONING )
			{
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_PRONE_SECONDARYFIRE );
			}
			else if ( m_pHajPlayer->GetFlags() & FL_DUCKING )
			{
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_CROUCH_SECONDARYFIRE );
			}
			else
			{
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_STAND_SECONDARYFIRE );
			}

			//iGestureActivity = ACT_VM_PRIMARYATTACK;
			break;
		}
	case PLAYERANIMEVENT_ATTACK_PRE:
		{
			if ( m_pHajPlayer->GetFlags() & FL_DUCKING ) 
			{
				// Weapon pre-fire. Used for minigun windup, sniper aiming start, etc in crouch.
				iGestureActivity = ACT_MP_ATTACK_CROUCH_PREFIRE;
			}
			else
			{
				// Weapon pre-fire. Used for minigun windup, sniper aiming start, etc.
				iGestureActivity = ACT_MP_ATTACK_STAND_PREFIRE;
			}

			RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, iGestureActivity, false );
			//iGestureActivity = ACT_VM_IDLE; //TODO?

			break;
		}
	case PLAYERANIMEVENT_ATTACK_POST:
		{
			RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_ATTACK_STAND_POSTFIRE );
			//iGestureActivity = ACT_VM_IDLE; //TODO?
			break;
		}

	case PLAYERANIMEVENT_RELOAD:
		{
			// Weapon reload.
			if ( m_pHajPlayer->GetFlags() & FL_PRONING )
			{
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_RELOAD_PRONE );
			}
			else if ( GetBasePlayer()->GetFlags() & FL_DUCKING )
			{
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_RELOAD_CROUCH );
			}
			else
			{
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_RELOAD_STAND );
			}
			//iGestureActivity = ACT_VM_RELOAD; //Make view reload if it isn't already
			break;
		}
	case PLAYERANIMEVENT_RELOAD_LOOP:
		{
			// Weapon reload.
			if ( m_pHajPlayer->GetFlags() & FL_PRONING )
			{
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_RELOAD_PRONE_LOOP );
			}
			else if ( GetBasePlayer()->GetFlags() & FL_DUCKING )
			{
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_RELOAD_CROUCH_LOOP );
			}
			else
			{
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_RELOAD_STAND_LOOP );
			}
			//iGestureActivity = ACT_INVALID; //TODO: fix
			break;
		}
	case PLAYERANIMEVENT_RELOAD_END:
		{
			// Weapon reload.
			if ( m_pHajPlayer->GetFlags() & FL_PRONING )
			{
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_RELOAD_PRONE_END );
			}
			else if ( GetBasePlayer()->GetFlags() & FL_DUCKING )
			{
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_RELOAD_CROUCH_END );
			}
			else
			{
				RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_RELOAD_STAND_END );
			}
			iGestureActivity = ACT_INVALID; //TODO: fix
			break;
		}

	case PLAYERANIMEVENT_STAND_TO_PRONE:
		{
			m_bProneTransition = true;
			m_bProneTransitionFirstFrame = true;
			m_iProneActivity = ACT_MP_STAND_TO_PRONE;
			RestartMainSequence();
			//iGestureActivity = ACT_VM_IDLE; //Clear for weapon, we have no stand->prone so just idle.
		}
		break;
	case PLAYERANIMEVENT_CROUCH_TO_PRONE:
		{
			m_bProneTransition = true;
			m_bProneTransitionFirstFrame = true;
			m_iProneActivity = ACT_MP_CROUCH_TO_PRONE;
			RestartMainSequence();
			//iGestureActivity = ACT_VM_IDLE; //Clear for weapon, we have no crouch->prone so just idle.
		}
		break;
	case PLAYERANIMEVENT_PRONE_TO_STAND:
		{
			m_bProneTransition = true;
			m_bProneTransitionFirstFrame = true;
			m_iProneActivity = ACT_MP_PRONE_TO_STAND;
			RestartMainSequence();
			//iGestureActivity = ACT_VM_IDLE; //Clear for weapon, we have no prone->stand so just idle.
		}
		break;
	case PLAYERANIMEVENT_PRONE_TO_CROUCH:
		{
			m_bProneTransition = true;
			m_bProneTransitionFirstFrame = true;
			m_iProneActivity = ACT_MP_PRONE_TO_CROUCH;
			RestartMainSequence();
			//iGestureActivity = ACT_VM_IDLE; //Clear for weapon, we have no prone->crouch so just idle.
		}
		break;

	case PLAYERANIMEVENT_SPAWN:

			m_bProneTransition = false; // don't break

	default:
		{
			BaseClass::DoAnimationEvent( event, nData );
			break;
		}
	}

#ifdef CLIENT_DLL
	// Make the weapon play the animation as well
	if ( iGestureActivity != ACT_INVALID && GetHajPlayer() != CHajPlayer::GetLocalHajPlayer())
	{
		CBaseCombatWeapon *pWeapon = GetHajPlayer()->GetActiveWeapon();
		if ( pWeapon )
		{
			pWeapon->EnsureCorrectRenderingModel();
			pWeapon->SendWeaponAnim( iGestureActivity );
			// Force animation events!
			pWeapon->ResetEventsParity();		// reset event parity so the animation events will occur on the weapon. 
			pWeapon->DoAnimationEvents( pWeapon->GetModelPtr() );
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *idealActivity - 
//-----------------------------------------------------------------------------
bool CHajPlayerAnimState::HandleSwimming( Activity &idealActivity )
{
	bool bInWater = BaseClass::HandleSwimming( idealActivity );

	return bInWater;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *idealActivity - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHajPlayerAnimState::HandleMoving( Activity &idealActivity )
{
	// In TF we run all the time now.
	float flSpeed = GetOuterXYSpeed();
	float flMaxSpeed = GetHajPlayer()->MaxSpeed();

	CHAJWeaponBase* pWeapon = dynamic_cast<CHAJWeaponBase*>(m_pHajPlayer->GetActiveWeapon());
	bool bAimed = (bool)( (pWeapon && pWeapon->IsIronsighted()) || m_pHajPlayer->GetLastShotTime() >= gpGlobals->curtime - 3.0f );

	if( flSpeed < 2.0f )
		idealActivity = bAimed ? ACT_HAJ_IDLE_AIM: ACT_MP_STAND_IDLE;
	else if ( flMaxSpeed > HAJ_NORM_SPEED )
		idealActivity = ACT_MP_SPRINT;
	else if( flMaxSpeed < HAJ_NORM_SPEED )
		idealActivity = bAimed ? ACT_HAJ_WALK_AIM : ACT_MP_WALK;
	else
		idealActivity = bAimed ? ACT_HAJ_RUN_AIM : ACT_MP_RUN;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *idealActivity - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHajPlayerAnimState::HandleDucking( Activity &idealActivity )
{
	CHAJWeaponBase* pWeapon = dynamic_cast<CHAJWeaponBase*>(m_pHajPlayer->GetActiveWeapon());
	bool bAimed = (bool)( (pWeapon && pWeapon->IsIronsighted()) || m_pHajPlayer->GetLastShotTime() >= gpGlobals->curtime - 3.0f );

	if ( m_pHajPlayer->GetFlags() & FL_DUCKING )
	{
		if ( GetOuterXYSpeed() < MOVING_MINIMUM_SPEED )
		{
			idealActivity = bAimed ? ACT_HAJ_IDLE_CROUCH_AIM : ACT_MP_CROUCH_IDLE;		
		}
		else
		{
			idealActivity = bAimed ? ACT_HAJ_WALK_CROUCH_AIM : ACT_MP_CROUCHWALK;		
		}

		return true;
	}
	
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *idealActivity - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHajPlayerAnimState::HandleProne( Activity &idealActivity )
{
	if ( m_pHajPlayer->GetFlags() & FL_PRONING )
	{
		if ( GetOuterXYSpeed() < MOVING_MINIMUM_SPEED )
		{
			idealActivity = ACT_MP_PRONE_IDLE;		
		}
		else
		{
			idealActivity = ACT_MP_PRONE_CRAWL;		
		}

		return true;
	}
	
	return false;
}
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *idealActivity - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHajPlayerAnimState::HandleProneTransition( Activity &idealActivity )
{
	if ( m_bProneTransition )
	{
		if (m_bProneTransitionFirstFrame)
		{
			m_bProneTransitionFirstFrame = false;
			RestartMainSequence();	// Reset the animation.
		}

		//Tony; check the cycle, and then stop overriding
		if ( GetBasePlayer()->GetCycle() >= 0.99 )
			m_bProneTransition = false;
		else
			idealActivity = m_iProneActivity;
	}

	return m_bProneTransition;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *idealActivity - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHajPlayerAnimState::HandleSprinting( Activity &idealActivity )
{
	if ( m_pHajPlayer->MaxSpeed() > HAJ_NORM_SPEED )
	{
		idealActivity = ACT_MP_SPRINT;	

		return true;
	}
	
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
bool CHajPlayerAnimState::HandleJumping( Activity &idealActivity )
{
	Vector vecVelocity;
	GetOuterAbsVelocity( vecVelocity );

	if ( m_bJumping )
	{
		static bool bNewJump = false; //Tony; the sample dod player models that I'm using don't have the jump anims split up like tf2.

		if ( m_bFirstJumpFrame )
		{
			m_bFirstJumpFrame = false;
			RestartMainSequence();	// Reset the animation.
		}

		// Reset if we hit water and start swimming.
		if ( m_pHajPlayer->GetWaterLevel() >= WL_Waist )
		{
			m_bJumping = false;
			RestartMainSequence();
		}
		// Don't check if he's on the ground for a sec.. sometimes the client still has the
		// on-ground flag set right when the message comes in.
		else if ( gpGlobals->curtime - m_flJumpStartTime > 0.2f )
		{
			if ( m_pHajPlayer->GetFlags() & FL_ONGROUND )
			{
				m_bJumping = false;
				RestartMainSequence();

				if ( bNewJump )
				{
					RestartGesture( GESTURE_SLOT_JUMP, ACT_MP_JUMP_LAND );					
				}
			}
		}

		// if we're still jumping
		if ( m_bJumping )
		{
			if ( bNewJump )
			{
				if ( gpGlobals->curtime - m_flJumpStartTime > 0.5 )
				{
					idealActivity = ACT_MP_JUMP_FLOAT;
				}
				else
				{
					idealActivity = ACT_MP_JUMP_START;
				}
			}
			else
			{
				idealActivity = ACT_MP_JUMP;
			}
		}
	}	

	if ( m_bJumping )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Overriding CMultiplayerAnimState to add prone and sprinting checks as necessary.
// Input  :  - 
// Output : Activity
//-----------------------------------------------------------------------------
#ifdef CLIENT_DLL
extern ConVar anim_showmainactivity;
#endif

Activity CHajPlayerAnimState::CalcMainActivity()
{
	Activity idealActivity = ACT_MP_STAND_IDLE;

	if ( HandleJumping( idealActivity ) || 
		HandleDeployedWeapon( idealActivity ) ||
		HandleVaulting( idealActivity ) ||
		//Tony; handle these before ducking !!
		HandleProneTransition( idealActivity ) ||
		HandleProne( idealActivity ) ||
		HandleDucking( idealActivity ) || 
		HandleSwimming( idealActivity ) || 
		HandleDying( idealActivity ) ||
		HandleSprinting( idealActivity )
		)
	{
		// intentionally blank
	}
	else
	{
		HandleMoving( idealActivity );
	}

	ShowDebugInfo();

	// Client specific.
#ifdef CLIENT_DLL

	if ( anim_showmainactivity.GetBool() )
	{
		DebugShowActivity( idealActivity );
	}

#endif

	return idealActivity;
}

// idle pose
bool CHajPlayerAnimState::HandleDeployedWeapon( Activity &idealActivity )
{
	CHAJWeaponBase* pWeapon = dynamic_cast<CHAJWeaponBase*>(m_pHajPlayer->GetActiveWeapon());

	if( pWeapon && ( pWeapon->m_bIsBipodDeploying || pWeapon->m_bIsBipodDeployed ) && !pWeapon->m_bIsBipodUnDeploying )
	{
		idealActivity = ( m_pHajPlayer->GetFlags() & FL_PRONING ) ? ACT_HAJ_PRONE_DEPLOY : ACT_HAJ_DEPLOY;
		m_pHajPlayer->SetPoseParameter( "body_height", pWeapon->m_flTransitionHeight );
		
		return true;
	}

	return false;
}

bool CHajPlayerAnimState::HandleVaulting( Activity &idealActivity )
{
	if( GetHajPlayer()->m_Local.m_bVaulting != m_bVaulting )
	{
		m_bVaulting = GetHajPlayer()->m_Local.m_bVaulting;

		if( m_bVaulting )
		{
			RestartMainSequence();
		}
	}

	if( m_bVaulting )
	{
		idealActivity = ACT_MP_VAULT2;
		return true;
	}

	return false;
}