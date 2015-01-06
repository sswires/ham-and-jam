
// haj_capturepoint.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "haj_bombzone.h"
#include "haj_gameevents.h"
#include "haj_gamerules.h"
#include "haj_misc.h"
#include "haj_objectivemanager.h"
#include "haj_team.h"
#include "haj_player.h"
#include "haj_grenade_tnt.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


/////////////////////////////////////////////////////////////////////////////
LINK_ENTITY_TO_CLASS(func_bombzone, CBombZone);



/////////////////////////////////////////////////////////////////////////////
// data description
BEGIN_DATADESC( CBombZone )
	DEFINE_KEYFIELD(m_initialTeam,        FIELD_INTEGER, "DefaultOwnership"),
	DEFINE_KEYFIELD(m_bCWealthCanCapture, FIELD_BOOLEAN, "CanCWealthCapture"),
	DEFINE_KEYFIELD(m_bAxisCanCapture,    FIELD_BOOLEAN, "CanAxisCapture"),

	DEFINE_KEYFIELD(m_iStages,			  FIELD_INTEGER, "BombStages"),

	DEFINE_KEYFIELD(m_fExtraTime,		  FIELD_FLOAT,	 "ExtraTimeOnCap" ),

	// inputs
	DEFINE_INPUTFUNC(FIELD_VOID, "EnableAxisCapture", InputEnableAxisCapture),
	DEFINE_INPUTFUNC(FIELD_VOID, "DisableAxisCapture", InputDisableAxisCapture),
	DEFINE_INPUTFUNC(FIELD_VOID, "EnableCWealthCapture", InputEnableCWealthCapture),
	DEFINE_INPUTFUNC(FIELD_VOID, "DisableCWealthCapture", InputDisableCWealthCapture),

	// outputs
	DEFINE_OUTPUT(m_eOnDestroyed,		"OnDestroyed"),
	DEFINE_OUTPUT(m_eOnDamaged,			"OnDamaged"),
	DEFINE_OUTPUT(m_eOnCWealthDestroy,	"OnCwealthDestroyed"),
	DEFINE_OUTPUT(m_eOnAxisDestroy,		"OnAxisDestroyed"),
	DEFINE_OUTPUT(m_eOnDefused,			"OnDefused"),
	DEFINE_OUTPUT(m_eOnPlanted,			"OnPlanted"),

END_DATADESC()

/////////////////////////////////////////////////////////////////////////////
// network data table
IMPLEMENT_SERVERCLASS_ST(CBombZone, DT_BombZone)
SendPropInt(SENDINFO(m_ownerTeam), 8, 0),
SendPropInt(SENDINFO(m_capturingTeam), 8, 0),
SendPropBool( SENDINFO( m_bAxisCanCapture ) ),
SendPropBool( SENDINFO( m_bCWealthCanCapture ) ),
SendPropInt( SENDINFO( m_iBombStage ), 8, 0 ),
SendPropInt( SENDINFO( m_iStages ), 8, 0 ),
SendPropBool( SENDINFO( m_bDestroyed )),
SendPropBool( SENDINFO( m_bPlanted )),
SendPropEHandle( SENDINFO( m_hPlantedBomb )),
SendPropFloat( SENDINFO( m_flExplodeTime )),
END_SEND_TABLE()

/////////////////////////////////////////////////////////////////////////////
CBombZone::CBombZone()
{
	m_sortIndex = 0;
	m_clientUpdateInterval = CAPTUREPOINT_UPDATE_INTERVAL;
	m_lastThinkTime = 0.0f;
	m_ownerTeam = TEAM_UNASSIGNED;
	m_capturingTeam = TEAM_INVALID;
	m_bAxisCanCapture = true;
	m_bCWealthCanCapture = true;
	m_iStages = 1;
	m_iBombStage = 0;
	m_bDestroyed = false;
	m_bPlanted = false;

	m_bShowOnHud = true;

	_objectiveman.AddObjective( this );
}

/////////////////////////////////////////////////////////////////////////////
CBombZone::~CBombZone()
{
	_objectiveman.RemoveObjective(this);
}

/////////////////////////////////////////////////////////////////////////////
void CBombZone::Activate()
{
	if(m_initialTeam == 2)	// unassigned
		m_ownerTeam = TEAM_UNASSIGNED;
	else
		m_ownerTeam = CHajTeam::GetFirstCombatTeamIndex() + m_initialTeam;

	BaseClass::Activate();
}

void CBombZone::OnPlayerEnter( CBasePlayer* pPlayer )
{
	CHajPlayer *pHajPlayer = ToHajPlayer( pPlayer );

	if( pHajPlayer )
	{
		pHajPlayer->SetInBombZone( true, this );
		DevMsg( "Set player %s in bomb zone\n", pHajPlayer->GetPlayerName() );
	}
}


void CBombZone::OnPlayerLeave( CBasePlayer* pPlayer )
{
	CHajPlayer *pHajPlayer = ToHajPlayer( pPlayer );

	if( pHajPlayer )
	{
		pHajPlayer->SetInBombZone( false );
		DevMsg( "Set player %s out of bomb zone\n", pHajPlayer->GetPlayerName() );
	}
}

/////////////////////////////////////////////////////////////////////////////
bool CBombZone::CanBeCapturedByTeam(int teamId) const
{
	if( m_bDestroyed || m_bPlanted )
		return false;

	if(teamId == TEAM_CWEALTH)
		return m_bCWealthCanCapture;

	else if(teamId == TEAM_AXIS)
		return m_bAxisCanCapture;

	return false;
}

void CBombZone::OnBombExploded( CBasePlayer *pAttacker, CGrenadeTNT *pTNT )
{
	m_bPlanted = false;
	m_hPlantedBomb.Set( NULL );

	if( pAttacker && CanBeCapturedByTeam( pAttacker->GetTeamNumber() ) && m_ownerTeam != pAttacker->GetTeamNumber() && !m_bDestroyed )
	{
		m_iBombStage++;

		if( m_iBombStage >= m_iStages )
		{
			Captured( pAttacker->GetTeamNumber(), pAttacker );
		}
		else
		{
			Damaged( pAttacker->GetTeamNumber(), pAttacker );
		}

		CHajGameMode *pGamemode = HajGameRules()->GetGamemode();
		if( pGamemode ) pGamemode->OnBombExplode( this );
	}

	m_capturingTeam = TEAM_INVALID;
}


void CBombZone::OnBombPlant( CBasePlayer *pAttacker, CGrenadeTNT *pTNT )
{
	m_bPlanted = true;
	m_hPlantedBomb.Set( pTNT );
	m_capturingTeam = pAttacker->GetTeamNumber();

	m_flExplodeTime = pTNT->m_flDetonateTime;

	CHajPlayer *pPlayer = ToHajPlayer( pAttacker );

	if( !HajGameRules()->IsFreeplay() )
	{
		pPlayer->IncrementFragCount( 3 );
		pPlayer->SendNotification( "#HaJ_NotificationPlantBomb", NOTIFICATION_MENTIONS_OBJECTIVE, 3, this, STRING( GetNameOfZone() ) );
	}

	HajGameRules()->SendNotification( "#HaJ_PlantedExplosives", NOTIFICATION_MENTIONS_OBJECTIVE, TEAM_INVALID, 0, pPlayer, STRING( GetNameOfZone() ) );
	//UTIL_ClientPrintAll( HUD_PRINTCENTER, "#HaJ_PlantedExplosives", pPlayer->GetPlayerName(), GetNameOfZone().ToCStr() );

	IGameEvent * event = gameeventmanager->CreateEvent( "zone_plantedtnt" );

	if ( event )
	{
		event->SetString( "zone", STRING( GetNameOfZone() ) );
		event->SetInt( "entityid", entindex() );
		event->SetInt( "teamid", GetOwnerTeamId() );
		event->SetInt( "planter", pPlayer->GetUserID() );

		gameeventmanager->FireEvent( event );
	}

	m_capturingPlayers.RemoveAll();

	// for people helping the planter
	for(unsigned int i = 0 ; i < MAX_PLAYERS ; ++i)
	{
		if( m_playerList.IsValidIndex(i) )
		{
			CBasePlayer* pPlayer = UTIL_PlayerByUserId(m_playerList[i]);
			if( pPlayer && pPlayer->GetTeamNumber() == m_capturingTeam )
				m_capturingPlayers.AddToTail(pPlayer);
		}
	}

	m_eOnPlanted.FireOutput( pAttacker, pTNT );

	CHajGameMode *pGamemode = HajGameRules()->GetGamemode();
	if( pGamemode ) pGamemode->OnBombPlant( pAttacker, this );
}


void CBombZone::OnBombDefuse( CBasePlayer *pDefuser, CGrenadeTNT *pTNT )
{
	m_bPlanted = false;
	m_hPlantedBomb.Set( NULL );

	CHajPlayer *pPlayer = ToHajPlayer( pDefuser );

	if( !HajGameRules()->IsFreeplay() )
	{
		pPlayer->DefendedObjective( this, NULL, true );
		pPlayer->SendNotification( "#HaJ_NotificationDefused", NOTIFICATION_MENTIONS_OBJECTIVE, 3 + pPlayer->GetNearbyTeammates(), this, STRING( GetNameOfZone() ) );
	}

	m_eOnDefused.FireOutput( pDefuser, pTNT );
	m_capturingTeam = TEAM_INVALID;

	CHajGameMode *pGamemode = HajGameRules()->GetGamemode();
	if( pGamemode ) pGamemode->OnBombDefuse( pDefuser, this );
}

/////////////////////////////////////////////////////////////////////////////
void CBombZone::Captured(int iCapturingTeam, CBasePlayer* pPlanter )
{

	CHajGameRules *pGameRules = HajGameRules();

	switch( iCapturingTeam ) // no breaks because we want the default condition to go off
	{
		case TEAM_AXIS:
			m_eOnAxisDestroy.FireOutput( pPlanter, pPlanter );
		
		case TEAM_CWEALTH:
			m_eOnCWealthDestroy.FireOutput( pPlanter, pPlanter );

		default:
			m_eOnDestroyed.FireOutput( pPlanter, pPlanter );
	}

	// for people helping the planter
	for( int i = 0; i <= m_capturingPlayers.Count(); i++ )
	{
		if( m_capturingPlayers.IsValidIndex( i ) )
		{
			CHajPlayer *pPlayer = ToHajPlayer( m_capturingPlayers[i] );

			if( pPlayer )
			{
				int nearbyTeam = m_capturingPlayers.Count() - 1;

				if( !pGameRules->IsFreeplay() )
				{
					pPlayer->IncrementFragCount( 10 + nearbyTeam );
					pPlayer->IncreaseObjectiveScore( 1 );
					pPlayer->m_iObjectivesCapped++;
				}

				if( pPlayer == pPlanter )
					pPlayer->SendNotification( "#HaJ_NotificationYourBombExplode", NOTIFICATION_MENTIONS_OBJECTIVE, 10 + nearbyTeam, this, STRING( GetNameOfZone() ) );
				else
					pPlayer->SendNotification( "#HaJ_NotificationPlantAssist", NOTIFICATION_MENTIONS_OBJECTIVE, 10 + nearbyTeam, this, STRING( GetNameOfZone() ) );
			}
		}
	}

	if( !m_capturingPlayers.HasElement( pPlanter ) )
	{
		CHajPlayer *pHAJPlayer = ToHajPlayer(pPlanter);

		if( !pGameRules->IsFreeplay() )
		{
			pHAJPlayer->IncrementFragCount( 10 );
			pHAJPlayer->IncreaseObjectiveScore( 1 );
			pHAJPlayer->m_iObjectivesCapped++;
		}

		pHAJPlayer->SendNotification( "#HaJ_NotificationYourBombExplode", NOTIFICATION_MENTIONS_OBJECTIVE, 5, this, STRING( GetNameOfZone() ) );

		m_capturingPlayers.AddToTail( pPlanter );
	}


	// send event to clients
	char szCapPoint[256];
	Q_snprintf( szCapPoint, 256, "%s", m_nameOfZone );
	CHajGameEvents::ControlPointCaptured(iCapturingTeam, m_ownerTeam, m_capturingPlayers, szCapPoint, entindex() );

	pGameRules->SendNotification( "#HaJ_DestroyedEquipment", NOTIFICATION_MENTIONS_OBJECTIVE, TEAM_INVALID, 0, this, szCapPoint);

	m_ownerTeam = iCapturingTeam;

	// clear the "in-capture" flag
	m_capturingTeam = TEAM_INVALID;
	m_bDestroyed = true;

#ifndef CLIENT_DLL
	if( m_fExtraTime >= 0.0f )
	{
		pGameRules->ExtendTime( m_fExtraTime );
	}
#endif

	CHajGameMode *pGamemode = HajGameRules()->GetGamemode();
	if( pGamemode ) pGamemode->OnBombExplode( this );
}

void CBombZone::Damaged( int iCapturingTeam, CBasePlayer *pPlanter )
{
	m_eOnDamaged.FireOutput( pPlanter, pPlanter );

	CHajPlayer *pHAJPlayer = ToHajPlayer(pPlanter);

	if( !HajGameRules()->IsFreeplay() )
	{
		pHAJPlayer->IncrementFragCount( 7 );
		pHAJPlayer->IncreaseObjectiveScore( 1 );
	}

	// send event to clients
	char szCapPoint[256];
	Q_snprintf( szCapPoint, 256, "%s", m_nameOfZone );

	pHAJPlayer->SendNotification( "#HaJ_NotificationDamagingEquipment", NOTIFICATION_MENTIONS_OBJECTIVE, 7, this, STRING( GetNameOfZone() ) );
	HajGameRules()->SendNotification( "#HaJ_DamagedEquipment", NOTIFICATION_MENTIONS_OBJECTIVE, TEAM_INVALID, 0, this, szCapPoint);

}

/////////////////////////////////////////////////////////////////////////////
void CBombZone::Init()
{
	BaseClass::Init();
}

/////////////////////////////////////////////////////////////////////////////
void CBombZone::InputEnableAxisCapture(inputdata_t& inputdata)
{
	m_bAxisCanCapture = true;
}

/////////////////////////////////////////////////////////////////////////////
void CBombZone::InputDisableAxisCapture(inputdata_t& inputdata)
{
	m_bAxisCanCapture = false;
}

/////////////////////////////////////////////////////////////////////////////
void CBombZone::InputEnableCWealthCapture(inputdata_t& inputdata)
{
	m_bCWealthCanCapture = true;
}

/////////////////////////////////////////////////////////////////////////////
void CBombZone::InputDisableCWealthCapture(inputdata_t& inputdata)
{
	m_bCWealthCanCapture = false;
}

/////////////////////////////////////////////////////////////////////////////
void CBombZone::Spawn()
{
	SetThink(&CBombZone::BrushThink);
	BaseClass::Spawn();
}

/////////////////////////////////////////////////////////////////////////////
void CBombZone::DoThink()
{

}

bool CBombZone::PlayerKilledOnPoint( CHajPlayer *pAttacker, CHajPlayer* pVictim, const CTakeDamageInfo &info )
{
	if( !pAttacker )
		return false;

	if( ( pAttacker->GetTeamNumber() == m_ownerTeam && ( !m_bDestroyed || !m_bPlanted ) ) || ( pAttacker->GetTeamNumber() == m_capturingTeam && m_bPlanted ) )
		return BaseClass::PlayerKilledOnPoint(pAttacker,pVictim,info); // return true if handled

	return false;
}