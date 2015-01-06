//========= Copyright © 2010, Ham and Jam. ==============================//
// Purpose:  Brush entity that prevents vaulting in its volume
// Notes:
// $NoKeywords: $
//=======================================================================//

#include "cbase.h"
#include "triggers.h"

class CNoVaultTrigger : public CBaseTrigger
{
public:
	DECLARE_CLASS( CNoVaultTrigger, CBaseTrigger );
	
	virtual void Spawn();

	virtual void StartTouch(CBaseEntity *pOther);
	virtual void EndTouch(CBaseEntity *pOther);

	void SetVaultingDisabled( CBaseEntity *pEnt, bool b );
};

LINK_ENTITY_TO_CLASS( func_novault, CNoVaultTrigger );

void CNoVaultTrigger::Spawn()
{
	// setup as a volume trigger
	SetSolid(SOLID_VPHYSICS);
	SetSolidFlags(FSOLID_NOT_SOLID|FSOLID_TRIGGER);

	const char* szModelName = STRING(GetModelName());
	SetModel(szModelName);

	SetRenderMode(kRenderNone);

	BaseClass::Spawn();
}
void CNoVaultTrigger::StartTouch( CBaseEntity *pOther )
{
	if( pOther && pOther->IsPlayer() )
	{
		SetVaultingDisabled( pOther, true );
	}

	BaseClass::StartTouch( pOther );
}

void CNoVaultTrigger::EndTouch( CBaseEntity *pOther )
{
	if( pOther && pOther->IsPlayer() )
	{
		SetVaultingDisabled( pOther, false );
	}

	BaseClass::EndTouch( pOther );
}

void CNoVaultTrigger::SetVaultingDisabled( CBaseEntity *pEnt, bool b )
{
	CBasePlayer *pPlayer = ToBasePlayer( pEnt );

	if( pPlayer )
	{
		DevMsg( "m_bVaultingDisabled = %d on %s\n", b, pPlayer->GetPlayerName() );
		pPlayer->m_Local.m_bVaultingDisabled = b;
	}
}