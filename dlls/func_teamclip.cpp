/*
		© 2009 Ham and Jam Team
		============================================================
		Author: Stephen Swires
		Purpose: Team clip to block off unwanted enemies
*/

#include "cbase.h"

#ifndef CLIENT_DLL
#include "haj_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

enum {
	TEAMCLIP_ALLIES = 1,
	TEAMCLIP_AXIS
};

#ifdef CLIENT_DLL
#define CTeamClip C_TeamClip
#endif

class CTeamClip : public CBaseEntity
{
public:
	DECLARE_CLASS( CTeamClip, CBaseEntity );
	DECLARE_DATADESC();
	DECLARE_NETWORKCLASS(); 

	CTeamClip();

	void Spawn();
	bool ShouldCollide( int collisionGroup, int contentsMask ) const;

#ifndef CLIENT_DLL
	void StartTouch( CBaseEntity *pOther );

	void InputDeactivate( inputdata_t &inputdata );
	void InputActivate( inputdata_t &inputdata );
	void InputSetBlockedTeam( inputdata_t &inputdata );

protected:
	COutputEvent m_OnBlockedTouch;
#endif

private:
	CNetworkVar( int, m_iTeamContents );
	CNetworkVar( bool, m_bEnabled );
};

LINK_ENTITY_TO_CLASS( func_teamclip, CTeamClip );

// networking, so the client can predict us
IMPLEMENT_NETWORKCLASS_ALIASED( TeamClip, DT_TeamClip )

BEGIN_NETWORK_TABLE( CTeamClip, DT_TeamClip )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_iTeamContents ) ),
	RecvPropBool( RECVINFO( m_bEnabled ) ),
#else
	SendPropInt( SENDINFO( m_iTeamContents ), 8, SPROP_UNSIGNED ),
	SendPropBool( SENDINFO( m_bEnabled ) ),
#endif
END_NETWORK_TABLE()

// keyvalues :)
BEGIN_DATADESC( CTeamClip )
#ifndef CLIENT_DLL
	DEFINE_KEYFIELD( m_iTeamContents, FIELD_INTEGER, "blockedteam" ),
	DEFINE_KEYFIELD( m_bEnabled, FIELD_BOOLEAN, "StartActivated" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Deactivate", InputDeactivate ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Activate", InputActivate ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "SetBlockedTeam", InputSetBlockedTeam ),

	DEFINE_OUTPUT( m_OnBlockedTouch, "OnBlockedTouch"),
#endif
END_DATADESC()


CTeamClip::CTeamClip()
{
	m_bEnabled = true;
}

void CTeamClip::Spawn()
{
	BaseClass::Spawn();

	SetSolid( SOLID_BBOX ); //make it solid
	SetCollisionGroup( COLLISION_GROUP_TEAMCLIP ); // special col group

	const char* szModelName = STRING(GetModelName());
	SetModel(szModelName);

	SetRenderMode(kRenderNone);
}

#ifndef CLIENT_DLL
void CTeamClip::StartTouch( CBaseEntity *pOther )
{
	if( pOther->IsPlayer() )
	{
		CHajPlayer *pPlayer = ToHajPlayer( pOther );

		if( pPlayer->GetTeamNumber() == ( m_iTeamContents + 1 ) )
		{
			pPlayer->SendHint( "HaJ_Tip_NoEntry" );
			m_OnBlockedTouch.FireOutput( this, pPlayer );
		}
	}

	BaseClass::StartTouch( pOther );
}

void CTeamClip::InputDeactivate( inputdata_t &inputdata )
{
	m_bEnabled = false;
}

void CTeamClip::InputActivate( inputdata_t &inputdata )
{
	m_bEnabled = true;
}

void CTeamClip::InputSetBlockedTeam( inputdata_t &inputdata )
{
	m_iTeamContents = inputdata.value.Int();
}
#endif

bool CTeamClip::ShouldCollide( int collisionGroup, int contentsMask ) const
{
	if( !m_bEnabled )
		return false;

	if( collisionGroup == COLLISION_GROUP_NONE )
		return false;

	if ( collisionGroup == COLLISION_GROUP_PLAYER_MOVEMENT || collisionGroup == COLLISION_GROUP_PLAYER )
	{
		if( !( contentsMask & CONTENTS_TEAM1 ) && !( contentsMask & CONTENTS_TEAM2 ) )
			return false;
		
		if( ( contentsMask & CONTENTS_TEAM1 ) && m_iTeamContents == TEAMCLIP_ALLIES )
			return true;
		else if( ( contentsMask & CONTENTS_TEAM2 ) && m_iTeamContents == TEAMCLIP_AXIS )
			return true;
		if ( ( contentsMask & CONTENTS_TEAM1 ) && m_iTeamContents == TEAMCLIP_AXIS )
			return false;
		else if( ( contentsMask & CONTENTS_TEAM2 ) && m_iTeamContents == TEAMCLIP_ALLIES )
			return false;
	}

	return BaseClass::ShouldCollide( collisionGroup, contentsMask );
}
