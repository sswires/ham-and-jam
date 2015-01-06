#include "cbase.h"

#ifndef HAJ_GAMEMODE_C_H
#define HAJ_GAMEMODE_C_H

#define CHajGameMode C_HajGameMode

class C_BombZone;

/////////////////////////////////////////////////////////////////////////////
class CHajGameMode : public CBaseEntity
{
public:
	DECLARE_CLASS(CHajGameMode, CBaseEntity );

public:
	// 'structors
	CHajGameMode() {}
	virtual ~CHajGameMode() {}

	DECLARE_NETWORKCLASS(); 

	virtual void DrawHUD() {}
	virtual bool ShouldDrawCaptureStrip() { return true; }

	// predicted
	virtual bool CanPlantBomb( C_BasePlayer *pPlanter, C_BombZone *pZone = NULL );

	virtual bool IsRoundBased( void ) { return false; }

	// client needs to know this for satchel prediction
	virtual float GetPlantTime( void ) { return 5.0f; }
	virtual float GetDefuseTime( void ) { return 5.0f; }
	virtual float GetBombTimer( void ) { return 35.0f; }
};

#endif