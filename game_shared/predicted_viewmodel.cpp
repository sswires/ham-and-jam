//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//
#include "cbase.h"

#ifdef CLIENT_DLL
#include "iprediction.h"
#include "prediction.h"
#include "ivieweffects.h"
#include "c_hl2mp_player.h"
#include "haj_player_c.h"
#include "haj_weapon_base.h"
#else
#include "haj_player.h"
#endif

#include "predicted_viewmodel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( predicted_viewmodel, CPredictedViewModel );

IMPLEMENT_NETWORKCLASS_ALIASED( PredictedViewModel, DT_PredictedViewModel )

BEGIN_NETWORK_TABLE( CPredictedViewModel, DT_PredictedViewModel )
END_NETWORK_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
#ifdef CLIENT_DLL
CPredictedViewModel::CPredictedViewModel() : m_LagAnglesHistory("CPredictedViewModel::m_LagAnglesHistory")
{
	m_vLagAngles.Init();
	m_LagAnglesHistory.Setup( &m_vLagAngles, 0 );
}
#else
CPredictedViewModel::CPredictedViewModel()
{
}
#endif


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CPredictedViewModel::~CPredictedViewModel()
{

}

#ifdef CLIENT_DLL
ConVar cl_wpn_sway_interp( "cl_wpn_sway_interp", "0.1", FCVAR_CLIENTDLL );
ConVar cl_wpn_sway_scale( "cl_wpn_sway_scale", "1.0", FCVAR_CLIENTDLL );
#endif

void CPredictedViewModel::CalcViewModelLag( Vector& origin, QAngle& angles, QAngle& original_angles )
{
	#ifdef CLIENT_DLL
		// Calculate our drift
		Vector	forward, right, up;
		AngleVectors( angles, &forward, &right, &up );
		
		// Add an entry to the history.
		m_vLagAngles = angles;
		m_LagAnglesHistory.NoteChanged( gpGlobals->curtime, cl_wpn_sway_interp.GetFloat() );
		
		// Interpolate back 100ms.
		m_LagAnglesHistory.Interpolate( gpGlobals->curtime, cl_wpn_sway_interp.GetFloat() );
		
		// Now take the 100ms angle difference and figure out how far the forward vector moved in local space.
		Vector vLaggedForward;
		QAngle angleDiff = m_vLagAngles - angles;
		AngleVectors( -angleDiff, &vLaggedForward, 0, 0 );
		Vector vForwardDiff = Vector(1,0,0) - vLaggedForward;

		float Scale = 1.0f;
		C_HAJWeaponBase *pWeapon = dynamic_cast< C_HAJWeaponBase* >( GetOwningWeapon() );

		if( pWeapon )
		{
			Scale = pWeapon->SwayScale();
		}

		// Now offset the origin using that.
		vForwardDiff *= Scale;
		origin += forward*vForwardDiff.x + right*-vForwardDiff.y + up*vForwardDiff.z;
	#endif
}

void CPredictedViewModel::CalcViewModelView( CBasePlayer *owner, const Vector& eyePosition, const QAngle& eyeAngles )
{
	// UNDONE: Calc this on the server?  Disabled for now as it seems unnecessary to have this info on the server
#if defined( CLIENT_DLL )
	QAngle vmangoriginal = eyeAngles;
	QAngle vmangles = eyeAngles;
	Vector vmorigin = eyePosition;

	C_HAJWeaponBase *pWeapon = dynamic_cast< C_HAJWeaponBase* >( GetOwningWeapon() );
	//Allow weapon lagging
	if ( pWeapon )
	{
	#if defined( CLIENT_DLL )
		if ( !prediction->InPrediction() )
	#endif
		{
			// add weapon-specific bob 
			pWeapon->AddViewmodelBob( this, vmorigin, vmangles );
		}
		
		CalcIronsights( vmorigin, vmangles );

		C_HajPlayer *pHajOwner = ToHajPlayer( pWeapon->GetOwner() );

		// MATCH OBSERVED PLAYER OTHERWISE
		if( pHajOwner && pHajOwner->IsObserver() && pHajOwner->GetObserverTarget() )
			pHajOwner = ToHajPlayer( pHajOwner->GetObserverTarget() );

		if( pHajOwner && pHajOwner->IsAlive() )
		{
			Vector vVel = pHajOwner->GetAbsVelocity();
			float flSpeed = vVel.Length();

			// Prone VM movement
			if( pHajOwner && ( pHajOwner->GetFlags() & FL_PRONING ) && flSpeed > 5.0f )
			{
				vmorigin.z -= clamp( flSpeed / 25, 0, 8 );
				vmangles[PITCH] += clamp( flSpeed / 10, 0, 15 );
				vmangles[YAW] += clamp( flSpeed / 3, 0, 35 );

				vmorigin.y += clamp( pHajOwner->GetLocalVelocity().y / 25, -8, 4 );
				vmorigin.x -= clamp( pHajOwner->GetLocalVelocity().x / 25, 0, 9 );
			}
		}
	}

	// Add model-specific bob even if no weapon associated (for head bob for off hand models)
	//AddViewModelBob( owner, vmorigin, vmangles );
	// Add lag
	CalcViewModelLag( vmorigin, vmangles, vmangoriginal );

#if defined( CLIENT_DLL )
	if ( !prediction->InPrediction() )
	{
		// Let the viewmodel shake at about 10% of the amplitude of the player's view
		vieweffects->ApplyShake( vmorigin, vmangles, 0.1 );	
	}
#endif

	SetLocalOrigin( vmorigin );
	SetLocalAngles( vmangles );

#endif
}

#ifdef CLIENT_DLL
void CPredictedViewModel::CalcIronsights( Vector& pos, QAngle& ang )
{
	C_HAJWeaponBase *pWeapon = dynamic_cast< C_HAJWeaponBase* >( GetOwningWeapon() );
 
	if ( !pWeapon )
		return;

	C_HajPlayer *pOwner = ToHajPlayer( pWeapon->GetOwner() );

	QAngle newAng = ang;
	Vector newPos = pos;

	Vector vForward, vRight, vUp;
	AngleVectors( newAng, &vForward, &vRight, &vUp );

	Vector vOffset = Vector( 0, 0, 0 );
	QAngle angOffset = QAngle( 0, 0, 0 );
	
	pWeapon->GetViewModelOffset( vOffset, angOffset );

	//vOffset = pWeapon->GetIronsightPosition() - vMoveOffset;

	if( vOffset.Length() != 0.0f )
	{
		newPos += vForward * vOffset.x;
		newPos += vRight * vOffset.y;
		newPos += vUp * vOffset.z;
		newAng += angOffset;

		pos += ( newPos - pos );
		ang += ( newAng - ang );
	}

}
#endif
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *modelname - 
//-----------------------------------------------------------------------------
void CPredictedViewModel::SetWeaponModel( const char *modelname, CBaseCombatWeapon *weapon )
{
	// SET UP HANDS
	CHajPlayer *pPlayer = ToHajPlayer( weapon->GetOwner() );

	if( pPlayer && FindBodygroupByName( "sleeve" ) != -1 )
	{
		SetBodygroup( FindBodygroupByName( "sleeve" ), pPlayer->GetTeamNumber() - 2 );
	}

	BaseClass::SetWeaponModel( modelname, weapon );
}
