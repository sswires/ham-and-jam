
// haj_pickup.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "haj_pickup.h"
#include "haj_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/////////////////////////////////////////////////////////////////////////////
LINK_ENTITY_TO_CLASS(haj_pickup, CHajPickup);

/////////////////////////////////////////////////////////////////////////////
// data description
BEGIN_DATADESC(CHajPickup)
	
	DEFINE_KEYFIELD( m_bBoneMerge, FIELD_BOOLEAN, "BoneMerge" ),
	DEFINE_KEYFIELD( m_iPickupTeam, FIELD_INTEGER, "PickupTeam" ), // what team should pick this up
	DEFINE_KEYFIELD( m_bAllowRecovery, FIELD_BOOLEAN, "AllowRecovery" ), // allow defending team to return pickup to base

END_DATADESC()

/////////////////////////////////////////////////////////////////////////////
void CHajPickup::Precache()
{
	PrecacheModel(STRING(GetModelName()));
}

/////////////////////////////////////////////////////////////////////////////
void CHajPickup::Spawn()
{
	Precache();

	// set model
	SetModel(STRING(GetModelName()));
	SetSolid(SOLID_VPHYSICS);
	SetMoveType(MOVETYPE_PUSH);

	// no update. this entity is user-impulse driven
	SetNextThink(TICK_NEVER_THINK);

	m_bDropped = false;
	m_vecSpawn = GetAbsOrigin(); // if recovery is enabled, it'll go back to its spawn point
	m_angSpawn = GetAbsAngles();
}

/////////////////////////////////////////////////////////////////////////////
void CHajPickup::StartTouch(CBaseEntity* pOther)
{
	// check if the player bumped us
	CHajPlayer* pPlayer = ToHajPlayer(pOther);
	if(!pPlayer)
		return;

	bool bPlayerAccepted = pPlayer->OnPickupTouch(this);
	if( bPlayerAccepted )
	{
		// make the object invisible and uncollidable
		if( m_bBoneMerge )
			AddEffects( EF_BONEMERGE );
		else
			AddEffects( EF_NODRAW );

		SetParent( pPlayer ); // follow player
		SetSolid(SOLID_NONE);

		return;
	}

	if( m_bAllowRecovery && pPlayer->GetTeamNumber() != m_iPickupTeam )
	{

	}
}

void CHajPickup::OwnerKilled( CHajPlayer* pPlayer, const CTakeDamageInfo &info )
{
	CHajPlayer *pKiller = ToHajPlayer( info.GetAttacker() );

	// increment score for defence
	if( pKiller && pKiller->IsPlayer() )
	{
		pKiller->IncreaseObjectiveScore( 1 );
		pKiller->IncrementFragCount( 1 );
		pKiller->m_iObjectivesDefended++;
	}

	OnDropped();
}

void CHajPickup::OnDropped()
{
	CHajPlayer *pPlayer = ToHajPlayer( GetParent() );

	if( pPlayer )
	{
		pPlayer->IncrementFragCount( -1 ); // -1 for losing pickup (you get 2 for picking it up)
	}

	Reset();
	m_bDropped = true;
}

void CHajPickup::Reset()
{

	// reset everything back to before
	if( m_bBoneMerge )
		RemoveEffects( EF_BONEMERGE );
	else
		RemoveEffects( EF_NODRAW );

	SetParent( NULL );
	SetSolid( SOLID_VPHYSICS );

	m_bDropped = false;

}