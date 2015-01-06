//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Defines the player specific data that is sent only to the player
//			to whom it belongs.
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_PLAYERLOCALDATA_H
#define C_PLAYERLOCALDATA_H
#ifdef _WIN32
#pragma once
#endif

#include "basetypes.h"
#include "vector.h"
#include "playernet_vars.h"

class C_VaultHint;

//-----------------------------------------------------------------------------
// Purpose: Player specific data ( sent only to local player, too )
//-----------------------------------------------------------------------------
class CPlayerLocalData
{
public:
	DECLARE_PREDICTABLE();
	DECLARE_CLASS_NOBASE( CPlayerLocalData );
	DECLARE_EMBEDDED_NETWORKVAR();

	CPlayerLocalData() :
		m_iv_vecPunchAngle( "CPlayerLocalData::m_iv_vecPunchAngle" )
	{
		m_iv_vecPunchAngle.Setup( &m_vecPunchAngle.m_Value, LATCH_SIMULATION_VAR );
		m_flFOVRate = 0;
	}

	unsigned char			m_chAreaBits[MAX_AREA_STATE_BYTES];				// Area visibility flags.
	unsigned char			m_chAreaPortalBits[MAX_AREA_PORTAL_STATE_BYTES];// Area portal visibility flags.

	int						m_iHideHUD;			// bitfields containing sections of the HUD to hide
	
	float					m_flFOVRate;		// rate at which the FOV changes
	

	bool					m_bDucked;
	bool					m_bDucking;

	// *HAJ 020* - Jed
	bool					m_bProned;
	bool					m_bProning;
	bool					m_bDuckProne; 
	bool					m_bDoProne;
	bool					m_bDoUnProne;

	float					m_flNextStanceChange;
	float					m_flNextJumpTime;

	// Vaulting
	bool					m_bVaulting;
	bool					m_bVaultingDisabled;
	int						m_iVaultHeight;
	int						m_iVaultStage;
	float					m_flVaultGroundTime;

	CNetworkHandle( C_VaultHint, m_hVaultHint );

	bool					IsVaulting() { return m_bVaulting; }
	bool					IsVaultingDisabled() { return m_bVaultingDisabled; }
	void					SetVaulting( bool b ) { m_bVaulting = b; }
	int						GetVaultHeight() { return m_iVaultHeight; }
	void					SetVaultHeight( int i ) { m_iVaultHeight = i; }

	bool					m_bInDuckJump;
	float					m_flDucktime;

	float					m_flPronetime;	// *HAJ 020* - Jed

	float					m_flDuckJumpTime;
	float					m_flJumpTime;
	int						m_nStepside;
	float					m_flFallVelocity;
	int						m_nOldButtons;
	// Base velocity that was passed in to server physics so 
	//  client can predict conveyors correctly.  Server zeroes it, so we need to store here, too.
	Vector					m_vecClientBaseVelocity;  
	CNetworkQAngle( m_vecPunchAngle );		// auto-decaying view angle adjustment
	CInterpolatedVar< QAngle >	m_iv_vecPunchAngle;

	CNetworkQAngle( m_vecPunchAngleVel );		// velocity of auto-decaying view angle adjustment
	bool					m_bDrawViewmodel;
	bool					m_bWearingSuit;
	bool					m_bPoisoned;
	float					m_flStepSize;
	bool					m_bAllowAutoMovement;

	// 3d skybox
	sky3dparams_t			m_skybox3d;
	// wold fog
	fogparams_t				m_fog;
	// audio environment
	audioparams_t			m_audio;

	bool					m_bSlowMovement;

};

#endif // C_PLAYERLOCALDATA_H
