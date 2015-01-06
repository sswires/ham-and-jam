//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef HL2MP_WEAPON_PARSE_H
#define HL2MP_WEAPON_PARSE_H
#ifdef _WIN32
#pragma once
#endif


#include "weapon_parse.h"
#include "networkvar.h"


//--------------------------------------------------------------------------------------------------------
class CHL2MPSWeaponInfo : public FileWeaponInfo_t
{
public:
	DECLARE_CLASS_GAMEROOT( CHL2MPSWeaponInfo, FileWeaponInfo_t );
	
	CHL2MPSWeaponInfo();
	
	virtual void Parse( ::KeyValues *pKeyValuesData, const char *szWeaponName );


public:

	int m_iPlayerDamage;

	float m_flMinVertRecoil, m_flMaxVertRecoil, m_flMinHorizRecoil, m_flMaxHorizRecoil, m_flDeployedRecoilMultiply; // recoil
	float m_flPunchPitch, m_flPunchYaw;
	float m_flCOFStand, m_flCOFCrouch, m_flCOFProne, m_flCOFDeployed, m_flCOFInAir, m_flMovementAccuracyPenalty; // cof
	float m_flRange, m_flRangeIronsighted, m_flBulletFadeFactor, m_flBulletFadeFactorIS;
	float m_flReloadTime, m_flReloadEmptyTime;
	bool m_bUsesBipod;

	// ironsights
	bool					bHasIronsights;
	Vector					vecIronsightOffset;
	QAngle					angIronsightAngs;
	float					flIronsightFOV;
	float					flIronsightTime;
};


#endif // HL2MP_WEAPON_PARSE_H
