// model: models/surgeon/mortarcrate_closed.mdl

#include "cbase.h"
#include "prop_tntdispenser.h"

#ifdef CLIENT_DLL
	#include "haj_player_c.h"

	#define CTNTDispenser C_TNTDispenser
#else
	#include "haj_player.h"
#endif


LINK_ENTITY_TO_CLASS( prop_tntdispenser, CTNTDispenser );
IMPLEMENT_NETWORKCLASS_ALIASED( TNTDispenser, DT_TNTDispenser );

// Network table
BEGIN_NETWORK_TABLE( CTNTDispenser, DT_TNTDispenser )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_bEnabled ) ),
	RecvPropInt( RECVINFO( m_iMaxGive ) ),
	RecvPropInt( RECVINFO( m_iLimitTeam ) ),
#else
	SendPropBool( SENDINFO( m_bEnabled ) ),
	SendPropInt( SENDINFO( m_iMaxGive ) ),
	SendPropInt( SENDINFO( m_iLimitTeam ) ),
#endif
END_NETWORK_TABLE()

// Datadesc
BEGIN_DATADESC( CTNTDispenser )
#ifndef CLIENT_DLL
	DEFINE_KEYFIELD( m_iMaxGive, FIELD_INTEGER, "MaxCarry" ),
	DEFINE_KEYFIELD( m_iLimitTeam, FIELD_INTEGER, "LimitTeam" ),
	
	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Toggle", InputToggle )
#endif
END_DATADESC()

CTNTDispenser::CTNTDispenser()
{
#ifndef CLIENT_DLL
	m_bEnabled = true;
	m_iMaxGive = 3;
	m_iLimitTeam = TEAM_INVALID;
#endif
}

void CTNTDispenser::Precache()
{
	PrecacheModel( STRING( GetModelName() ) );
}

void CTNTDispenser::Activate()
{
	PrecacheModel( STRING( GetModelName() ) );

	SetModel( STRING( GetModelName() ) );
	SetSolid( SOLID_BBOX );

	VPhysicsInitNormal( SOLID_BBOX, GetSolidFlags() | FSOLID_TRIGGER, true );
	SetMoveType( MOVETYPE_NONE );

#ifndef CLIENT_DLL
	m_bEnabled = !HasSpawnFlags( SF_START_DISABLED );

	for( int i = 0; i <= MAX_PLAYERS; i++ )
		m_flPlayerTakeTime[i] = 0.0f;
#endif

	BaseClass::Activate();
}

#ifndef CLIENT_DLL
void CTNTDispenser::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CHajPlayer *pPlayer = ToHajPlayer( pActivator );

	if( pPlayer )
		GiveTNT( pPlayer );
}

void CTNTDispenser::GiveTNT( CHajPlayer *pPlayer )
{

	if( !m_bEnabled || ( m_iLimitTeam != TEAM_INVALID && pPlayer->GetTeamNumber() != m_iLimitTeam ) )
		return;

	CBaseCombatWeapon *pWeapon = pPlayer->Weapon_OwnsThisType( "weapon_satchelcharge" );

	if( !pWeapon )
	{
		pWeapon = (CBaseCombatWeapon*)pPlayer->GiveNamedItem( "weapon_satchelcharge" );
		pPlayer->SetAmmoCount( 1, pWeapon->GetPrimaryAmmoType() );
	}
	else
	{
		if( pPlayer->GetAmmoCount( pWeapon->GetPrimaryAmmoType() ) < m_iMaxGive )	
			pPlayer->CBasePlayer::GiveAmmo( 1, pWeapon->GetPrimaryAmmoType(), false );
	}

}

void CTNTDispenser::Touch( CBaseEntity *pOther )
{
	CHajPlayer *pPlayer = ToHajPlayer(pOther);

	if( m_bEnabled && pPlayer )
	{
		if( gpGlobals->curtime >= m_flPlayerTakeTime[pPlayer->entindex()] + 0.1 )
		{
			GiveTNT( pPlayer );
			m_flPlayerTakeTime[pPlayer->entindex()] = gpGlobals->curtime;
		}
	}

}

void CTNTDispenser::InputDisable( inputdata_t &inputdata )
{
	m_bEnabled = false;
}

void CTNTDispenser::InputEnable( inputdata_t &inputdata )
{
	m_bEnabled = true;
}

void CTNTDispenser::InputToggle( inputdata_t &inputdata )
{
	m_bEnabled = !m_bEnabled;
}


bool CTNTDispenser::ShouldCollide( int collisionGroup, int contentsMask ) const
{
	if( !m_bEnabled )
		return false;

	return BaseClass::ShouldCollide( collisionGroup, contentsMask );
}
#else // CLIENT

int CTNTDispenser::DrawModel( int flags )
{
	if( !m_bEnabled )
		return 0;

	return BaseClass::DrawModel( flags );
}
#endif
