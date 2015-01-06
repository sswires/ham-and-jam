//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef HAJ_PLAYERANIMSTATE_H
#define HAJ_PLAYERANIMSTATE_H
#ifdef _WIN32
#pragma once
#endif


#include "convar.h"
#include "sdk/multiplayer_animstate.h"

#if defined( CLIENT_DLL )
class C_HajPlayer;
#define CHajPlayer C_HajPlayer
#else
class CHajPlayer;
#endif

// ------------------------------------------------------------------------------------------------ //
// CPlayerAnimState declaration.
// ------------------------------------------------------------------------------------------------ //
class CHajPlayerAnimState : public CMultiPlayerAnimState
{
public:
	
	DECLARE_CLASS( CHajPlayerAnimState, CMultiPlayerAnimState );

	CHajPlayerAnimState();
	CHajPlayerAnimState( CBasePlayer *pPlayer, MultiPlayerMovementData_t &movementData );
	~CHajPlayerAnimState();

	void InitHajAnimState( CHajPlayer *pPlayer );
	CHajPlayer *GetHajPlayer( void )							{ return m_pHajPlayer; }

	virtual void ClearAnimationState();
	virtual Activity TranslateActivity( Activity actDesired );
	virtual void Update( float eyeYaw, float eyePitch );

	void	DoAnimationEvent( PlayerAnimEvent_t event, int nData = 0 );

	bool	HandleDeployedWeapon( Activity &idealActivity );
	bool	HandleMoving( Activity &idealActivity );
	bool	HandleJumping( Activity &idealActivity );
	bool	HandleDucking( Activity &idealActivity );
	bool	HandleSwimming( Activity &idealActivity );
	bool	HandleVaulting( Activity &idealActivity );

	bool	HandleProne( Activity &idealActivity );
	bool	HandleProneTransition( Activity &idealActivity );
	bool	HandleSprinting( Activity &idealActivity );

	//Tony; overriding because the SDK Player models pose parameter is flipped the opposite direction
	virtual void		ComputePoseParam_MoveYaw( CStudioHdr *pStudioHdr );

	virtual Activity CalcMainActivity();	

private:
	
	CHajPlayer   *m_pHajPlayer;
	bool		m_bInAirWalk;
	Activity	m_iProneActivity;
	bool		m_bProneTransition;
	bool		m_bProneTransitionFirstFrame;

	bool		m_bVaulting;

	float		m_flHoldDeployedPoseUntilTime;
};

CHajPlayerAnimState *CreateHajPlayerAnimState( CHajPlayer *pPlayer );

#endif // HAJ_PLAYERANIMSTATE_H
