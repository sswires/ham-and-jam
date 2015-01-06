//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include <KeyValues.h>
#include "hl2mp_weapon_parse.h"
#include "ammodef.h"

#include "SoundEmitterSystem/isoundemittersystembase.h"


#ifdef CLIENT_DLL
#include "c_hl2mp_player.h"
#else
#include "hl2mp_player.h"
#endif

#include "weapon_hl2mpbasehlmpcombatweapon.h"

FileWeaponInfo_t* CreateWeaponInfo()
{
	return new CHL2MPSWeaponInfo;
}



CHL2MPSWeaponInfo::CHL2MPSWeaponInfo()
{
	m_iPlayerDamage = 0;

	// default recoil
	m_flMinVertRecoil = 0.0f;
	m_flMaxVertRecoil = 0.0f;
	m_flMinHorizRecoil = 0.0f;
	m_flMaxHorizRecoil = 0.0f;

	// cof
	m_flCOFCrouch = 0.75f;
	m_flCOFDeployed = 0.25;
	m_flCOFProne = 0.5f;
	m_flCOFStand = 1.0f;
	m_flMovementAccuracyPenalty = 1.2f;

	m_flReloadTime = -1.0f;
}


void CHL2MPSWeaponInfo::Parse( KeyValues *pKeyValuesData, const char *szWeaponName )
{
	BaseClass::Parse( pKeyValuesData, szWeaponName );

	KeyValues *Sounds = pKeyValuesData->FindKey( "SoundData" );

	if( Sounds )
	{
		for ( KeyValues *sub = Sounds->GetFirstSubKey(); sub; sub = sub->GetNextKey() )
		{
			CBaseEntity::PrecacheScriptSound( sub->GetString() );
		}
	}

	KeyValues *Ironsights = pKeyValuesData->FindKey( "Ironsights" );

	if( Ironsights )
	{
		bHasIronsights = true;

		vecIronsightOffset = Vector();
		vecIronsightOffset.x = Ironsights->GetFloat( "x", 0.0f );
		vecIronsightOffset.y = Ironsights->GetFloat( "y", 0.0f );
		vecIronsightOffset.z = Ironsights->GetFloat( "z", 0.0f );
		
		angIronsightAngs = QAngle();
		angIronsightAngs[ PITCH ] = Ironsights->GetFloat( "pitch", 0.0f );
		angIronsightAngs[ YAW ] = Ironsights->GetFloat( "yaw", 0.0f );
		angIronsightAngs[ ROLL ] = Ironsights->GetFloat( "roll", 0.0f );

		flIronsightTime = Ironsights->GetFloat( "time", 0.33f ); // time to ironsight
		flIronsightFOV = Ironsights->GetFloat( "FOV", 60.0f );
	}
	else
	{
		bHasIronsights = false;
	}

	// recoil values
	KeyValues *Recoil = pKeyValuesData->FindKey( "Recoil" );

	if( Recoil )
	{
		m_flMinVertRecoil = Recoil->GetFloat( "MinVert" );
		m_flMaxVertRecoil = Recoil->GetFloat( "MaxVert" );
		m_flMinHorizRecoil = Recoil->GetFloat( "MinHoriz" );
		m_flMaxHorizRecoil = Recoil->GetFloat( "MaxHoriz" );

		m_flPunchPitch = Recoil->GetFloat( "PunchPitch", 0.0f );
		m_flPunchYaw = Recoil->GetFloat( "PunchYaw", 0.0f );

		m_flDeployedRecoilMultiply = Recoil->GetFloat( "IronsightMulti", 1.0f );
	}

	// cone of fire
	KeyValues *Accuracy = pKeyValuesData->FindKey( "Accuracy" );

	if( Accuracy )
	{
		m_flCOFStand = Accuracy->GetFloat( "Base", 1.0f );
		m_flCOFCrouch = Accuracy->GetFloat( "Crouch", 0.75f );
		m_flCOFProne = Accuracy->GetFloat( "Prone", 0.5f );
		m_flCOFDeployed = Accuracy->GetFloat( "Deployed", 0.25f );
		m_flCOFInAir = Accuracy->GetFloat( "InAir", 5.0f );

		m_flMovementAccuracyPenalty = Accuracy->GetFloat( "MovementPenalty", 1.2f );
	}

	m_iPlayerDamage = pKeyValuesData->GetInt( "damage", 0 );
	m_bUsesBipod = (bool)( pKeyValuesData->GetInt( "Deployable", 0 ) != 0 );

	m_flRange = pKeyValuesData->GetFloat( "Range", 1400 );
	m_flRangeIronsighted = pKeyValuesData->GetFloat( "RangeIronsighted", m_flRange );
	m_flBulletFadeFactor = pKeyValuesData->GetFloat( "BulletFadeFactor", 1.5f );
	m_flBulletFadeFactorIS = pKeyValuesData->GetFloat( "BulletFadeFactorIS", m_flBulletFadeFactor );

	m_flReloadTime = pKeyValuesData->GetFloat( "ReloadTime", -1.0f );
	m_flReloadEmptyTime = pKeyValuesData->GetFloat( "ReloadEmptyTime", m_flReloadTime );

}


