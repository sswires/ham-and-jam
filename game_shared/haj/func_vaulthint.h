/*
		© 2009 Ham and Jam Team
		============================================================
		Author: Stephen Swires
		Purpose: Shared brush based entity which is used to guide vault movement
*/

#ifndef HAJ_FUNC_VAULTHINT
#define HAJ_FUNC_VAULTHINT

#include "cbase.h"

#ifdef CLIENT_DLL
	#include "c_baseentity.h"

	#define CVaultHint C_VaultHint
	#define CHajPlayer C_HajPlayer
#else
	#include "baseentity.h"
#endif

class CHajPlayer;
class CMoveData;

class CVaultHint : public CBaseEntity
{
public:
	DECLARE_CLASS( CVaultHint, CBaseEntity );
	DECLARE_DATADESC();

	CVaultHint();

	DECLARE_NETWORKCLASS(); 

	void Spawn();
	void StartTouch( CBaseEntity *pOther );
	void EndTouch( CBaseEntity *pOther );

	bool CanPlayerVault( CHajPlayer *pPlayer );
	void ProcessVaultMovement( CHajPlayer *pPlayer, CMoveData *pMove );
	void StartVault( CHajPlayer *pPlayer );
	void EndVault( CHajPlayer *pPlayer );

	bool ShouldForceDuck( ) { return m_bForceDuck; }

#ifndef CLIENT_DLL
	void InputDeactivate( inputdata_t &inputdata );
	void InputActivate( inputdata_t &inputdata );

	CUtlVector< CHajPlayer* > m_Players;
#endif

private:
	CNetworkVar( bool, m_bEnabled );
	CNetworkVar( bool, m_bForceDuck );

};

#endif