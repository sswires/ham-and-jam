//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef PLAYERLOCALDATA_H
#define PLAYERLOCALDATA_H
#ifdef _WIN32
#pragma once
#endif

#include "playernet_vars.h"
#include "networkvar.h"
#include "func_vaulthint.h"

class CVaultHint;

//-----------------------------------------------------------------------------
// Purpose: Player specific data ( sent only to local player, too )
//-----------------------------------------------------------------------------
class CPlayerLocalData
{
public:
	// Save/restore
	DECLARE_SIMPLE_DATADESC();
	// Prediction data copying
	DECLARE_CLASS_NOBASE( CPlayerLocalData );
	DECLARE_EMBEDDED_NETWORKVAR();

	CPlayerLocalData();

	void UpdateAreaBits( CBasePlayer *pl, unsigned char chAreaPortalBits[MAX_AREA_PORTAL_STATE_BYTES] );


public:

	CNetworkArray( unsigned char, m_chAreaBits, MAX_AREA_STATE_BYTES );								// Which areas are potentially visible to the client?
	CNetworkArray( unsigned char, m_chAreaPortalBits, MAX_AREA_PORTAL_STATE_BYTES );	// Which area portals are open?

	CNetworkVar( int,	m_iHideHUD );		// bitfields containing sections of the HUD to hide
	CNetworkVar( float, m_flFOVRate );		// rate at which the FOV changes (defaults to 0)
		
	Vector				m_vecOverViewpoint;			// Viewpoint overriding the real player's viewpoint
	
	// Fully ducked
	CNetworkVar( bool, m_bDucked );
	// In process of ducking
	CNetworkVar( bool, m_bDucking );

	// *HAJ 020* - Jed
	CNetworkVar( bool, m_bProned );				// Fully proned
	CNetworkVar( bool, m_bProning );			// In process of proning
	CNetworkVar( bool, m_bDuckProne );
	CNetworkVar( bool, m_bDoProne );
	CNetworkVar( bool, m_bDoUnProne );

	CNetworkVar( float, m_flNextJumpTime );

	// Vaulting
	CNetworkVar( bool, m_bVaulting );
	CNetworkVar( bool, m_bVaultingDisabled );
	CNetworkVar( int, m_iVaultHeight );
	CNetworkVar( int, m_iVaultStage );
	CNetworkVar( float, m_flVaultGroundTime ); 
	CNetworkHandle( CVaultHint, m_hVaultHint );

	bool					IsVaulting() { return m_bVaulting; }
	bool					IsVaultingDisabled() { return m_bVaultingDisabled; }
	void					SetVaulting( bool b ) { m_bVaulting = b; }
	int						GetVaultHeight() { return m_iVaultHeight; }
	void					SetVaultHeight( int i ) { m_iVaultHeight = i; }

	// In process of duck-jumping
	CNetworkVar( bool, m_bInDuckJump );
	// During ducking process, amount of time before full duc
	CNetworkVar( float, m_flDucktime );
	CNetworkVar( float, m_flDuckJumpTime );

	CNetworkVar( float, m_flNextStanceChange );

	// *HAJ 020* - Jed
	// During proning process, amount of time before full prone
	CNetworkVar( float, m_flPronetime );

	// Jump time, time to auto unduck (since we auto crouch jump now).
	CNetworkVar( float, m_flJumpTime );
	// Step sound side flip/flip
	int m_nStepside;;
	// Velocity at time when we hit ground
	CNetworkVar( float, m_flFallVelocity );
	// Previous button state
	int m_nOldButtons;
	class CSkyCamera *m_pOldSkyCamera;
	// Base velocity that was passed in to server physics so 
	//  client can predict conveyors correctly.  Server zeroes it, so we need to store here, too.
	// auto-decaying view angle adjustment
	CNetworkQAngle( m_vecPunchAngle );		
	CNetworkQAngle( m_vecPunchAngleVel );
	// Draw view model for the player
	CNetworkVar( bool, m_bDrawViewmodel );

	// Is the player wearing the HEV suit
	CNetworkVar( bool, m_bWearingSuit );
	CNetworkVar( bool, m_bPoisoned );
	CNetworkVar( float, m_flStepSize );
	CNetworkVar( bool, m_bAllowAutoMovement );

	// 3d skybox
	CNetworkVarEmbedded( sky3dparams_t, m_skybox3d );
	// wold fog
	CNetworkVarEmbedded( fogparams_t, m_fog );
	// audio environment
	CNetworkVarEmbedded( audioparams_t, m_audio );

	CNetworkVar( bool, m_bSlowMovement );
};

EXTERN_SEND_TABLE(DT_Local);


#endif // PLAYERLOCALDATA_H
