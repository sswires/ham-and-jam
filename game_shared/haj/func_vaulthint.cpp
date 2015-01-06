/*
		© 2009 Ham and Jam Team
		============================================================
		Author: Stephen Swires
		Purpose: Brush based entity which is used to guide vault movement
*/

#include "cbase.h"
#include "func_vaulthint.h"
#include "in_buttons.h"
#include "gamemovement.h"

#ifdef CLIENT_DLL
	#include "haj_player_c.h"
#else
	#include "haj_player.h"
#endif

#include "haj_weapon_base.h"

LINK_ENTITY_TO_CLASS( func_vaulthint, CVaultHint );

// networking, so the client can predict us
IMPLEMENT_NETWORKCLASS_ALIASED( VaultHint, DT_VaultHint )

// nwvars
BEGIN_NETWORK_TABLE( CVaultHint, DT_VaultHint )
	#ifdef CLIENT_DLL
		RecvPropBool( RECVINFO( m_bEnabled ) ),
		RecvPropBool( RECVINFO( m_bForceDuck ) ),
	#else
		SendPropBool( SENDINFO( m_bEnabled ) ),
		SendPropBool( SENDINFO( m_bForceDuck) ),
	#endif
END_NETWORK_TABLE()

// keyvalues :)
BEGIN_DATADESC( CVaultHint )
	#ifndef CLIENT_DLL
		DEFINE_KEYFIELD( m_bEnabled, FIELD_BOOLEAN, "StartActivated" ),
		DEFINE_KEYFIELD( m_bForceDuck, FIELD_BOOLEAN, "DuckVault" ),

		DEFINE_INPUTFUNC( FIELD_VOID, "Deactivate", InputDeactivate ),
		DEFINE_INPUTFUNC( FIELD_VOID, "Activate", InputActivate ),
	#endif
END_DATADESC()

// Constructor, contains defaults to keyvalues
CVaultHint::CVaultHint()
{
	m_bEnabled = true;
	m_bForceDuck = true;
}

void CVaultHint::Spawn()
{
	BaseClass::Spawn();

	SetSolid( SOLID_BSP ); //make it solid so StartTouch work
	AddSolidFlags( FSOLID_TRIGGER );
	AddSolidFlags( FSOLID_NOT_SOLID );

	SetModel(STRING(GetModelName()));

	SetRenderMode(kRenderNone);

#ifndef CLIENT_DLL
	m_Players.Purge();
#endif
}

// When a player enters the volume, at this point they can vault
void CVaultHint::StartTouch( CBaseEntity *pOther )
{
	if( !m_bEnabled ) return;
	CHajPlayer *pPlayer = ToHajPlayer( pOther );

	if( pPlayer ) // a player is in the zone
	{
		pPlayer->m_Local.m_hVaultHint.Set( this );
#ifndef CLIENT_DLL
		m_Players.AddToTail(pPlayer);
#endif
	}
}

void CVaultHint::EndTouch( CBaseEntity *pOther )
{
	CHajPlayer *pPlayer = ToHajPlayer( pOther );

	if( pPlayer )
	{
		if( !pPlayer->m_Local.m_bVaulting )
			pPlayer->m_Local.m_hVaultHint = NULL;
#ifndef CLIENT_DLL
		m_Players.FindAndRemove( pPlayer );
#endif
	}
}

// Can the player vault within our brush volume?
bool CVaultHint::CanPlayerVault( CHajPlayer *pPlayer )
{
	if( m_bEnabled )
	{
		trace_t tr;

		Vector vFrom = pPlayer->GetAbsOrigin() + Vector( 0, 0, 20 );
		QAngle aAng = QAngle( 0, pPlayer->GetAbsAngles().y, 0 );

		Vector vForward;
		AngleVectors( aAng, &vForward, NULL, NULL );

		// check if we're in front of a wall
		UTIL_TraceHull( vFrom, vFrom + (vForward*48), Vector(-4,-4,0), Vector(4, 4, 20), MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_NONE, &tr );

		if( tr.DidHit() )
		{
			trace_t tclear;
			Vector vVaultTopPos = vFrom;
			vVaultTopPos.z = GetAbsOrigin().z + CollisionProp()->OBBMaxs().z + 1;

			UTIL_TraceHull( vVaultTopPos, vVaultTopPos + (vForward * 20), VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_NONE, &tclear );

			if( !tclear.DidHit() )
				return true;
		}
	}

	return false;
}

// CGameMovement will call this function as a player is vaulting and overrides the player's movement
void CVaultHint::ProcessVaultMovement( CHajPlayer *pPlayer, CMoveData* pMove )
{
	if( !pPlayer->IsAlive() )
	{
		EndVault( pPlayer );
		return;
	}

#ifndef CLIENT_DLL
	pPlayer->m_Local.m_hVaultHint = this;
#endif

	Vector vPos = pPlayer->GetAbsOrigin();
	QAngle aAng = pPlayer->GetAbsAngles();

	Vector vForward, vRight, vUp;
	AngleVectors( aAng, &vForward, &vRight, &vUp );

	pMove->m_flSideMove = 0;
	pMove->m_flForwardMove = 0;
	pMove->m_flUpMove = 0;
	pMove->m_nButtons = 0;

	// elevator
	if( pPlayer->m_Local.m_iVaultStage == 1 )
	{

		if( vPos.z >= pPlayer->m_Local.m_iVaultHeight )
		{
			pPlayer->m_Local.m_iVaultStage = 2;
			pPlayer->ViewPunch( QAngle( 15, 0, 5 ) );
			return;
		}

		pPlayer->SetMoveType( MOVETYPE_FLY );
		pMove->m_flUpMove = 75;
		pMove->m_vecVelocity = Vector( 0, 0, 75 );

		return;
	}

	// push along top
	if( pPlayer->m_Local.m_iVaultStage == 2 )
	{
		pPlayer->SetMoveType( MOVETYPE_WALK );

		// are we actually walking on anything?
		if( pPlayer->GetGroundEntity() )
			pPlayer->m_Local.m_flVaultGroundTime += gpGlobals->frametime;
		else
		{
			if( pPlayer->m_Local.m_flVaultGroundTime > 0.0f  || pPlayer->m_Local.m_flVaultGroundTime < -0.5f )
				pPlayer->m_Local.m_iVaultStage = 3; // landing stage
			else
				pMove->m_flUpMove = -25;		

			pPlayer->m_Local.m_flVaultGroundTime -= gpGlobals->frametime;
		}

		// are we landing?
		if( (pPlayer->m_Local.m_flVaultGroundTime > 0.0f && !pPlayer->GetGroundEntity()) || pPlayer->m_Local.m_flVaultGroundTime > 0.5f )
			pPlayer->m_Local.m_iVaultStage = 3; // landing stage

		pMove->m_flForwardMove = 110;
		pMove->m_vecVelocity = vForward * 110;
		pMove->m_vecVelocity.z = pMove->m_flUpMove;

		return;
	}

	// landing
	if( pPlayer->m_Local.m_iVaultStage == 3 )
	{
		pPlayer->SetMoveType( MOVETYPE_WALK );
		pPlayer->SetGravity( 0.75f );

		if( pPlayer->GetGroundEntity() )
			EndVault( pPlayer );

		return;
	}

}

void CVaultHint::StartVault( CHajPlayer *pPlayer )
{
	if( CanPlayerVault( pPlayer ) )
	{
		pPlayer->m_Local.m_bVaulting = true;
		pPlayer->m_Local.m_iVaultHeight = GetAbsOrigin().z + CollisionProp()->OBBMaxs().z + 1;
		pPlayer->m_Local.m_flVaultGroundTime = 0.0f;
		pPlayer->m_Local.m_hVaultHint = this;
		pPlayer->m_Local.m_iVaultStage = 1;

		pPlayer->SetRenderOrigin( pPlayer->GetAbsOrigin() );

		if( ShouldForceDuck() )
		{
			pPlayer->m_Local.m_bDucked = true;
			pPlayer->AddFlag( FL_DUCKING );
			pPlayer->SetCollisionBounds( VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX );
		}

		pPlayer->ViewPunch( QAngle( 10, 0, 0 ) );

#ifdef CLIENT_DLL
		if( pPlayer->IsLocalPlayer() )
			pPlayer->SaveViewAngles();
#endif

		CHAJWeaponBase *pWeapon = dynamic_cast<CHAJWeaponBase*>(pPlayer->GetActiveWeapon());

		if( pWeapon )
			pWeapon->Holster( NULL );
	}
}

void CVaultHint::EndVault( CHajPlayer *pPlayer )
{
	pPlayer->m_Local.m_bVaulting = false;

#ifndef CLIENT_DLL
	if( !m_Players.HasElement( pPlayer ) )
		pPlayer->m_Local.m_hVaultHint = NULL;
#endif

	pPlayer->m_Local.m_iVaultStage = 0;
	pPlayer->SetRenderOrigin( vec3_origin );
	pPlayer->SetGravity( 1.0f );

	if( ShouldForceDuck() )
	{
		pPlayer->m_Local.m_bDucked = true;
		pPlayer->AddFlag( FL_DUCKING );
		pPlayer->SetCollisionBounds( VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX );
	}

	pPlayer->SetMoveType( MOVETYPE_WALK );

	pPlayer->ViewPunch( QAngle( -5, 0, 0 ) );

	CHAJWeaponBase *pWeapon = dynamic_cast<CHAJWeaponBase*>(pPlayer->GetActiveWeapon());

	if( pWeapon )
		pWeapon->Deploy();
}

#ifndef CLIENT_DLL
void CVaultHint::InputDeactivate( inputdata_t &inputdata )
{
	m_Players.Purge();
	m_bEnabled = false;
}

void CVaultHint::InputActivate( inputdata_t &inputdata )
{
	m_bEnabled = true;
}
#endif