//========= Copyright © 2007, Ham and Jam. ==============================//
// Purpose: Satchel Charge Weapon
// Notes: Used for blowing up stuff, like secondary objectives
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#include "haj_player_c.h"
#include "input.h"
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui_controls/Controls.h>
#else
#include "haj_player.h"
#include "haj_grenade_tnt.h"
#endif

#include "haj_gamerules.h"
#include "weapon_hl2mpbase.h"
#include "haj_weapon_base.h"
#include "weapon_satchelcharge.h"

#ifdef CLIENT_DLL
#define CWeaponSatchelCharge C_WeaponSatchelCharge
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSatchelCharge, DT_WeaponSatchelCharge )

BEGIN_NETWORK_TABLE( CWeaponSatchelCharge, DT_WeaponSatchelCharge )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_bPlanting ) ),
	RecvPropFloat( RECVINFO( m_flPlantStartTime ) ),
	RecvPropFloat( RECVINFO( m_flPlantEndTime ) ),

	RecvPropBool( RECVINFO( m_bDefusing ) ),

	RecvPropBool( RECVINFO( m_bPlantingWithUse ) ),
#else
	SendPropBool( SENDINFO( m_bPlanting ) ),
	SendPropFloat( SENDINFO( m_flPlantStartTime ) ),
	SendPropFloat( SENDINFO( m_flPlantEndTime ) ),

	SendPropBool( SENDINFO( m_bDefusing) ),

	SendPropBool( SENDINFO( m_bPlantingWithUse ) ),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponSatchelCharge )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_satchelcharge, CWeaponSatchelCharge );
PRECACHE_WEAPON_REGISTER(weapon_satchelcharge);

acttable_t	CWeaponSatchelCharge::m_acttable[] = 
{
	// attack
	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_TNT,			false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_TNT,			false },
	{ ACT_MP_ATTACK_PRONE_PRIMARYFIRE,	ACT_HAJ_PRIMARYATTACK_PRONE_TNT,	false },

	// aiming down sight, recent attack
	{ ACT_HAJ_IDLE_AIM,					ACT_HAJ_STAND_AIM_TNT,				false },
	{ ACT_HAJ_WALK_AIM,					ACT_HAJ_WALK_AIM_TNT,				false },
	{ ACT_HAJ_IDLE_CROUCH_AIM,			ACT_HAJ_CROUCH_AIM_TNT,				false },
	{ ACT_HAJ_WALK_CROUCH_AIM,			ACT_HAJ_CROUCHWALK_IDLE_TNT,		false },
	{ ACT_HAJ_RUN_AIM,					ACT_HAJ_RUN_AIM_TNT,				false },

	// movement
	{ ACT_MP_STAND_IDLE,				ACT_HAJ_STAND_AIM_TNT,				false },
	{ ACT_MP_RUN,						ACT_HAJ_RUN_AIM_TNT,				false },
	{ ACT_MP_SPRINT,					ACT_HAJ_SPRINT_AIM_TNT,				false },
	{ ACT_MP_WALK,						ACT_HAJ_WALK_AIM_TNT,				false },
	{ ACT_MP_PRONE_IDLE,				ACT_HAJ_PRONE_AIM_TNT,				false },
	{ ACT_MP_PRONE_CRAWL,				ACT_HAJ_PRONEWALK_AIM_TNT,			false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HAJ_CROUCH_AIM_TNT,				false },
	{ ACT_MP_CROUCHWALK,				ACT_HAJ_CROUCHWALK_AIM_TNT,			false },
};

IMPLEMENT_ACTTABLE(CWeaponSatchelCharge);


CWeaponSatchelCharge::CWeaponSatchelCharge()
{
	m_bPlanting = false;
	m_flPlantStartTime = -1.0f;
}

void CWeaponSatchelCharge::Precache()
{
#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "npc_grenade_tnt" );
#endif

	BaseClass::Precache();
}

bool CWeaponSatchelCharge::CanPlant( void )
{
	if( HajGameRules()->IsFreezeTime() )
		return false;

	return HajGameRules()->CanPlayerPlantBomb( ToBasePlayer( GetOwner() ) );
}

float CWeaponSatchelCharge::GetPlantTime( void )
{
	return HajGameRules()->GetPlayerPlantTime( ToBasePlayer( GetOwner() ) );

}

/*
	Called when the planting has started
*/
void CWeaponSatchelCharge::StartPlant( void )
{
	m_bPlanting = true;
	m_flPlantStartTime = gpGlobals->curtime;
	m_flPlantEndTime = gpGlobals->curtime + GetPlantTime() - 1;

	SendWeaponAnim( ACT_VM_PULLBACK_LOW );
	SetWeaponIdleTime( m_flPlantEndTime );

	CHajPlayer *pPlayer = ToHajPlayer(GetOwner());

	if( !pPlayer )
		return;

	float flSpeed = SequenceDuration() / ( GetPlantTime() - 1 );
	
	pPlayer->GetViewModel()->SetPlaybackRate( flSpeed );
	pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );

}


void CWeaponSatchelCharge::PlantAbort( void )
{
	CHajPlayer *pOwner = ToHajPlayer(GetOwner());

	if( pOwner )
	{
		m_bPlanting = false;
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.25f;

		if( !m_bPlantingWithUse )
			SendWeaponAnim( ACT_VM_IDLE );
		else
		{
			pOwner->Weapon_Switch( pOwner->Weapon_GetLast() );
#ifndef CLIENT_DLL
			pOwner->Weapon_SetLast( m_hLastInvRestore.Get() );
#endif
		}
	}
}

void CWeaponSatchelCharge::FinishPlant( void )
{
	m_bPlanting = false;

	CHajPlayer *pPlayer = ToHajPlayer( GetOwner() );

	if( !pPlayer )
		return;


	//pPlayer->SetMaxSpeed( HAJ_NORM_SPEED );
	pPlayer->RemoveAmmo( 1, m_iPrimaryAmmoType );

	SendWeaponAnim( ACT_VM_THROW );
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
	SetWeaponIdleTime( m_flNextPrimaryAttack );

#ifndef CLIENT_DLL

	// attach the tnt to what we're looking at, or drop it to the floor in front of us
	Vector vecSrc, vecAiming, vecUp;

	// Take the eye position and direction
	vecSrc = pPlayer->EyePosition();
	QAngle angles = pPlayer->GetLocalAngles();

	angles[PITCH] = 0.0f; // don't regard pitch, only yaw

	AngleVectors( angles, &vecAiming, NULL, &vecUp );

	trace_t tr;
	UTIL_TraceLine( vecSrc, vecSrc + (vecUp * -128), MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_PLAYER, &tr );

	Vector plantPos;
	QAngle plantAngs;

	if( tr.fraction < 1.0f && ( !tr.m_pEnt || ( tr.m_pEnt && !tr.m_pEnt->IsPlayer() ) ) ) // plant it where we're looking
	{
		plantPos = tr.endpos + tr.plane.normal * 3;

		QAngle pangles;
		VectorAngles(tr.plane.normal, pangles);

		plantAngs = pangles;
		plantAngs.x += 90;
	}
	else
	{
		plantPos = pPlayer->GetAbsOrigin();
		plantPos.z += 3.0f;

		plantAngs = QAngle( 0, 0, 0 );
		plantAngs[YAW] = pPlayer->GetAbsAngles()[YAW];
	}


	float flTimer = HajGameRules()->GetBombTimer( ToBasePlayer( pPlayer ) );

	CBaseEntity *pEnt = CBaseEntity::Create( "npc_grenade_tnt", plantPos, plantAngs, GetOwner() );
	CGrenadeTNT *pGrenade = (CGrenadeTNT *)pEnt;
	if ( pGrenade )
	{
		pGrenade->SetMoveType( MOVETYPE_NONE );
		pGrenade->SetTimer( flTimer, flTimer - 2.5 );
		pGrenade->SetThrower( ToBaseCombatCharacter( GetOwner() ) );

		if( pPlayer->IsInBombZone() )
			pGrenade->SetBombZone( pPlayer->m_hBombZone.Get() );

		if( !tr.DidHitWorld() && tr.m_pEnt )
			pGrenade->SetParent( tr.m_pEnt );
	}

#endif

	if( m_bPlantingWithUse )
		m_bInstantHolster = true;

	m_bInReload = true;
}


void CWeaponSatchelCharge::PlantThink( void )
{
	if( gpGlobals->curtime >= m_flPlantEndTime )
	{
		FinishPlant();
	}

	/*CHajPlayer *pPlayer = ToHajPlayer(GetOwner());

	if( pPlayer && pPlayer->MaxSpeed() > (HAJ_NORM_SPEED * 0.25)  )
		pPlayer->SetMaxSpeed( HAJ_NORM_SPEED * 0.25 );*/
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if the weapon currently has ammo or doesn't need ammo
// Output :
//-----------------------------------------------------------------------------
bool CWeaponSatchelCharge::HasPrimaryAmmo( void )
{
	return CWeaponHL2MPBase::HasPrimaryAmmo();
}

void CWeaponSatchelCharge::ItemPostFrame( void )
{
	CHajPlayer *pPlayer = ToHajPlayer( GetOwner() );

	if( !pPlayer )
		return;

	// switch if we're not holding any buttons
	if( m_bPlantingWithUse && !(pPlayer->m_nButtons & (IN_ATTACK|IN_ATTACK2|IN_USE)) )
	{
		pPlayer->Weapon_Switch( pPlayer->Weapon_GetLast() );
#ifndef CLIENT_DLL
		pPlayer->Weapon_SetLast( m_hLastInvRestore.Get() );
#endif
	}

	if( m_bDefusing )
	{
		if( pPlayer->m_nButtons & IN_USE )
		{
			if( gpGlobals->curtime >= m_flPlantEndTime )
				FinishDefusing();
		}

		else
		{
#ifdef CLIENT_DLL
			if( !pPlayer->GetLastWeapon() )
				input->MakeWeaponSelection( g_pGameRules->GetNextBestWeapon( pPlayer, this ) );
			else
				input->MakeWeaponSelection( pPlayer->GetLastWeapon() );
#endif
		}


		return;
	}

	if( HolsterTimeThink() == true )
		return;

	if( gpGlobals->curtime >= m_flNextPrimaryAttack && m_bInReload && HasPrimaryAmmo() )
	{
		m_bInReload = false;

		if( m_bPlantingWithUse )
		{
			pPlayer->Weapon_Switch( pPlayer->Weapon_GetLast() );
#ifndef CLIENT_DLL
			pPlayer->Weapon_SetLast( m_hLastInvRestore.Get() );
#endif
		}
		else
		{
			SendWeaponAnim( ACT_VM_DRAW );
			SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() + 2.0 );
		}

		return;
	}

	if( pPlayer->m_nButtons & (IN_ATTACK|IN_ATTACK2|IN_USE) && HasPrimaryAmmo() && !m_bInReload ) // primary button down
	{
		if( !m_bPlanting )
		{
			if( gpGlobals->curtime >= m_flNextPrimaryAttack )
			{
				if( CanPlant() )
					StartPlant();
				else
				{
#ifndef CLIENT_DLL
					pPlayer->SendNotification( "#HaJ_CantPlantHere", NOTIFICATION_BASIC );
#endif
					m_flNextPrimaryAttack = gpGlobals->curtime + 0.75;
				}
			}
		}
		else
		{
			if( CanPlant() )
				PlantThink();
			else
				PlantAbort();
		}

		return;
	}
	else if( m_bPlanting )
	{
		PlantAbort();
		return;
	}

	if( HasWeaponIdleTimeElapsed() && !m_bDefusing )
	{
		WeaponIdle();
	}

	if( !HasPrimaryAmmo() )
	{
		ReloadOrSwitchWeapons();
	}


}

bool CWeaponSatchelCharge::Holster( CBaseCombatWeapon *pSwitchingTo )
{

	// NO AMMO	
#ifndef CLIENT_DLL
	if( !HasPrimaryAmmo() )
		UTIL_Remove( this );
#endif

	// DEFUSE HANDLING
	if( m_bDefusing )
	{
		AbandonDefuse();
		return true;
	}

	if( m_bPlanting )
	{
		return false;
	}

	m_bPlantingWithUse = false;

	// NO AMMO
	if( !HasPrimaryAmmo() || m_bInstantHolster )
	{
		m_bInstantHolster = false;
		return true;
	}

	return BaseClass::Holster( pSwitchingTo );
}

void CWeaponSatchelCharge::FakeHolster()
{
	m_bFakeHolstered = true;

	if( !m_bPlanting && !m_bDefusing )
	{
		BaseClass::FakeHolster();
	}
}

void CWeaponSatchelCharge::FakeDeploy()
{
	m_bFakeHolstered = false;

	if( !m_bPlanting && !m_bDefusing )
	{
		BaseClass::FakeDeploy();
	}
}

#ifndef CLIENT_DLL
void CWeaponSatchelCharge::SetDefusing( CGrenadeTNT *pTNT )
{
	pTNT->AddEffects( EF_NODRAW );
	m_flPlantStartTime = gpGlobals->curtime;
	m_flPlantEndTime = gpGlobals->curtime + HajGameRules()->GetPlayerDefuseTime( ToBasePlayer(GetOwner()) );

	m_pDefusee = pTNT;
	m_bDefusing = true;
}
#endif



void CWeaponSatchelCharge::AbandonDefuse( void )
{
	// abort defuse
	if( m_bDefusing )
	{
		m_bDefusing = false;

#ifndef CLIENT_DLL
		if( m_pDefusee )
		{
			m_pDefusee->RemoveEffects( EF_NODRAW );
			m_pDefusee->m_bDefusing = false;
		}
#endif
	}
}

void CWeaponSatchelCharge::FinishDefusing( void )
{
	m_bDefusing = false;
	m_bInstantHolster = true;

	CHajPlayer *pPlayer = ToHajPlayer( GetOwner() );

	if( pPlayer )
	{
#ifdef CLIENT_DLL
		if( !pPlayer->GetLastWeapon() )
			input->MakeWeaponSelection( g_pGameRules->GetNextBestWeapon( pPlayer, this ) );
		else
			input->MakeWeaponSelection( pPlayer->GetLastWeapon() );
#else

		CBombZone *pBombZone = m_pDefusee->GetBombZone();

		if( pBombZone )
		{
			pBombZone->OnBombDefuse( pPlayer, m_pDefusee );
		}
#endif
	}

#ifndef CLIENT_DLL
	if( m_pDefusee )
	{
		m_pDefusee->m_bIsLive = false;
		m_pDefusee->StopSound( "Weapon_SatchelCharge.Fuze" );
		UTIL_Remove( m_pDefusee );
	}
#endif
}

Activity CWeaponSatchelCharge::GetDrawActivity( void )
{
	if( m_bDefusing )
		ACT_VM_PULLBACK_HIGH;

	return ACT_VM_DRAW;
}


void CWeaponSatchelCharge::Drop( const Vector &vecVelocity )
{
	AbandonDefuse();

#ifndef CLIENT_DLL
	UTIL_Remove( this );
#endif
}

#ifdef CLIENT_DLL
using namespace vgui;
void CWeaponSatchelCharge::Redraw()
{
	if( m_bPlanting || m_bDefusing )
	{
		float fillAmount = (gpGlobals->curtime - m_flPlantStartTime ) / ( m_flPlantEndTime - m_flPlantStartTime );
		float mul = ScreenHeight() / 500;

		surface()->DrawSetColor( 0, 0, 0, 150 );
		surface()->DrawFilledRect( (ScreenWidth()/2)-(100*mul), ScreenHeight()/2+(60*mul), ((ScreenWidth()/2)+(100*mul)), ScreenHeight()/2+(75*mul) );

		surface()->DrawSetColor( 255, 255, 255, 200 );
		surface()->DrawFilledRect( (ScreenWidth()/2)-(100*mul), ScreenHeight()/2+(60*mul), ((ScreenWidth()/2)-(100*mul)) + ((200*mul)*fillAmount), ScreenHeight()/2+(75*mul) );

		if( m_bPlanting )
			surface()->DrawSetColor( 200, 25, 25, 200 );
		else
			surface()->DrawSetColor( 25, 200, 25, 200 );

		surface()->DrawLine( (ScreenWidth()/2)-(102*mul), (ScreenHeight()/2)+(58*mul), (ScreenWidth()/2)+(102*mul), (ScreenHeight()/2)+(58*mul) ); // top
		surface()->DrawLine( (ScreenWidth()/2)-(102*mul), (ScreenHeight()/2)+(58*mul), (ScreenWidth()/2)-(102*mul), (ScreenHeight()/2)+(77*mul) ); // left
		surface()->DrawLine( (ScreenWidth()/2)+(102*mul), (ScreenHeight()/2)+(58*mul), (ScreenWidth()/2)+(102*mul), (ScreenHeight()/2)+(77*mul) ); // right
		surface()->DrawLine( (ScreenWidth()/2)-(102*mul), (ScreenHeight()/2)+(77*mul), (ScreenWidth()/2)+(102*mul), (ScreenHeight()/2)+(77*mul) ); // bottom
	}

	CBaseCombatWeapon::Redraw();
}
#endif

bool CWeaponSatchelCharge::Deploy()
{
	if( BaseClass::Deploy() )
	{
		if( m_bDefusing )
			SendWeaponAnim( ACT_VM_PULLBACK_HIGH );

		m_bInstantHolster = ( m_bDefusing || m_bPlantingWithUse );

		return true;
	}

	return false;
}

float CWeaponSatchelCharge::GetPlayerSpeedMultiplier( void )
{
	if( m_bDefusing || m_bPlanting )
		return 0.0f;

	return 1.0f;
}

void CWeaponSatchelCharge::SetViewModel()
{
	BaseClass::SetViewModel();

	CHajPlayer *pPlayer = ToHajPlayer(GetOwner());

	if( pPlayer )
	{
		CBaseViewModel *pVM = pPlayer->GetViewModel( m_nViewModelIndex );

		if( pVM )
		{
			int iTeamWeapon = (pPlayer->GetTeamNumber() - 2 );

			if( m_bDefusing && !m_bPlanting )
			{
				switch( pPlayer->GetTeamNumber() )
				{
					case TEAM_CWEALTH:
						iTeamWeapon = 1;
						break;

					default:
						iTeamWeapon = 0;
				}
			}

			pVM->SetBodygroup( pVM->FindBodygroupByName( "weapon" ), iTeamWeapon );
		}
	}

}

int CWeaponSatchelCharge::GetWorldModelIndex( void )
{
	CHajPlayer *pPlayer = ToHajPlayer(GetOwner());

	if( pPlayer )
	{
		switch( pPlayer->GetTeamNumber())
		{
			case TEAM_CWEALTH:
				if( m_bDefusing ) return m_iWorldModelDeployedIndex;
				return m_iWorldModelIndex;

				break;

			case TEAM_AXIS:
				if( m_bDefusing ) return m_iWorldModelIndex;
				return m_iWorldModelDeployedIndex;

				break;

			default:
				return m_iWorldModelIndex;
		}
	}

	return m_iWorldModelIndex;
}