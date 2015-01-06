
// haj_capturepoint.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "haj_capturepoint.h"
#include "haj_gameevents.h"
#include "haj_gamerules.h"
#include "haj_misc.h"
#include "haj_objectivemanager.h"
#include "haj_team.h"
#include "haj_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define COUNTER_STYLE_ZERO					0
#define COUNTER_STYLE_RESET_TIMER			1
#define COUNTER_STYLE_STALL					2
#define COUNTER_STYLE_REVERSE				3

/////////////////////////////////////////////////////////////////////////////
// globals
//#ifdef _DEBUG
//	bool _bCapturePointDebug = true;
//#else
	bool _bCapturePointDebug = false;
//#endif

/////////////////////////////////////////////////////////////////////////////
LINK_ENTITY_TO_CLASS(func_capture_point, CCapturePoint);

/////////////////////////////////////////////////////////////////////////////
// data description
BEGIN_DATADESC( CCapturePoint )
	DEFINE_KEYFIELD(m_initialTeam,        FIELD_INTEGER, "DefaultOwnership"),
	DEFINE_KEYFIELD(m_bCWealthCanCapture, FIELD_BOOLEAN, "CanCWealthCapture"),
	DEFINE_KEYFIELD(m_bAxisCanCapture,    FIELD_BOOLEAN, "CanAxisCapture"),
	DEFINE_KEYFIELD(m_cwealthCapTime,     FIELD_FLOAT,   "CWealthCapSpeed"),
	DEFINE_KEYFIELD(m_axisCapTime,	      FIELD_FLOAT,   "AxisCapSpeed"),
	DEFINE_KEYFIELD(m_nMinCWealthPlayers, FIELD_INTEGER, "CWealthMinCap"),
	DEFINE_KEYFIELD(m_nMinAxisPlayers,    FIELD_INTEGER, "AxisMinCap"),
	DEFINE_KEYFIELD(m_counterStyle,       FIELD_INTEGER, "CounterCapStyle"),
	DEFINE_KEYFIELD(m_nCounterPlayers,    FIELD_INTEGER, "CounterCapMin"),
	DEFINE_KEYFIELD(m_flDeCapSpeed,		  FIELD_FLOAT,	 "CapCooldownSpeed" ),

	DEFINE_KEYFIELD(m_fExtraPeopleCap,	  FIELD_FLOAT,	 "ExtraCapSpeed" ),
	DEFINE_KEYFIELD(m_fExtraTime,		  FIELD_FLOAT,	 "ExtraTimeOnCap" ),

	// inputs
	DEFINE_INPUTFUNC(FIELD_VOID, "EnableAxisCapture", InputEnableAxisCapture),
	DEFINE_INPUTFUNC(FIELD_VOID, "DisableAxisCapture", InputDisableAxisCapture),
	DEFINE_INPUTFUNC(FIELD_VOID, "EnableCWealthCapture", InputEnableCWealthCapture),
	DEFINE_INPUTFUNC(FIELD_VOID, "DisableCWealthCapture", InputDisableCWealthCapture),

	// outputs
	DEFINE_OUTPUT(m_eOnCWealthCapture,	  "OnCWealthCapture"),
	DEFINE_OUTPUT(m_eOnAxisCapture,		  "OnAxisCapture"),

END_DATADESC()

/////////////////////////////////////////////////////////////////////////////
// network data table
IMPLEMENT_SERVERCLASS_ST(CCapturePoint, DT_CapturePoint)
	SendPropInt(SENDINFO(m_ownerTeam), 8, 0),
	SendPropInt(SENDINFO(m_capturingTeam), 8, 0),
	SendPropFloat(SENDINFO(m_capturePercentage), 12, SPROP_NORMAL),
	SendPropBool( SENDINFO( m_bAxisCanCapture ) ),
	SendPropBool( SENDINFO( m_bCWealthCanCapture ) ),
	SendPropInt(SENDINFO(m_nMinCWealthPlayers), 8, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_nMinAxisPlayers), 8, SPROP_UNSIGNED),
END_SEND_TABLE()

/////////////////////////////////////////////////////////////////////////////
CCapturePoint::CCapturePoint()
{
	m_sortIndex = 0;
	m_clientUpdateInterval = CAPTUREPOINT_UPDATE_INTERVAL;
	m_lastThinkTime = 0.0f;
	m_ownerTeam = TEAM_UNASSIGNED;
	m_capturingTeam = TEAM_INVALID;
	m_timeLeftForCapture = 5.0f;
	m_capturePercentage = 0.0f;
	m_bAxisCanCapture = true;
	m_bCWealthCanCapture = true;
	m_axisCapTime = m_cwealthCapTime = 5.0f;
	m_nMinAxisPlayers = 1;
	m_nMinCWealthPlayers = 1;
	m_counterStyle = COUNTER_STYLE_REVERSE;
	m_nCounterPlayers = 1;

	m_fExtraPeopleCap = 0.15; // 15% faster per player (by default)
	m_fExtraTime = 0.0f;
	m_flDeCapSpeed = 0.5f;

	m_bShowOnHud = true;

	_objectiveman.AddCapturePoint(this);
}

/////////////////////////////////////////////////////////////////////////////
CCapturePoint::~CCapturePoint()
{
	_objectiveman.RemoveCapturePoint(this);
}

/////////////////////////////////////////////////////////////////////////////
void CCapturePoint::Activate()
{
	if(m_initialTeam == 2)	// unassigned
		m_ownerTeam = TEAM_UNASSIGNED;
	else
		m_ownerTeam = CHajTeam::GetFirstCombatTeamIndex() + m_initialTeam;

	m_CapInitialisers.RemoveAll();

	BaseClass::Activate();
}

void CCapturePoint::OnPlayerEnter( CBasePlayer* pPlayer )
{
	int teamId = pPlayer->GetTeamNumber();
	teamId -= CHajTeam::GetFirstCombatTeamIndex();

	// check for counter condition. it only occurs if the player
	// has entered a control point that is currently being
	// captured by an opposing team
	if(CHajTeam::IsCombatTeam(m_capturingTeam) && (teamId != m_capturingTeam))
	{
		switch(m_counterStyle)
		{
		case COUNTER_STYLE_ZERO:
			{
				// control point becomes neutral
				CHajTeam* pTeam = GetOwnerTeam();
				if(pTeam)
				{
					UnregisterWithTeam();
					m_ownerTeam = TEAM_UNASSIGNED;
				}
			}
			break;

		case COUNTER_STYLE_RESET_TIMER:
			// reset capture timer
			m_timeLeftForCapture = GetTeamCaptureTime(m_capturingTeam);
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
bool CCapturePoint::CanBeCapturedByTeam(int teamId) const
{
	if(teamId == TEAM_CWEALTH)
		return m_bCWealthCanCapture;

	else if(teamId == TEAM_AXIS)
		return m_bAxisCanCapture;

	return false;
}

/////////////////////////////////////////////////////////////////////////////
void CCapturePoint::Captured(int iCapturingTeam)
{
	// be sure the team ids are valid and supported
	if(!CHajTeam::IsCombatTeam(iCapturingTeam))
		return;

	int iTeamLosingCP = GetOwnerTeamId();

	// check if the capturing team is the team
	// that already controls the control point
	if(iTeamLosingCP == iCapturingTeam)
		return;

	// check if a team already controlled this
	// if so, notify it of the event
	if(iTeamLosingCP != TEAM_SPECTATOR)
	{
		CHajTeam* pTeam = (CHajTeam*)g_Teams[iTeamLosingCP];
		pTeam->UnregisterCapturePoint(this);
	}

	// notify the capturing team of the event
	CHajTeam* pCapturingTeam = (CHajTeam*)g_Teams[iCapturingTeam];
	pCapturingTeam->RegisterCapturePoint(this);


	switch(iCapturingTeam)
	{
	case TEAM_CWEALTH:
		m_eOnCWealthCapture.FireOutput(this, this);
		break;

	case TEAM_AXIS:
		m_eOnAxisCapture.FireOutput(this, this);
		break;
	}

	// send the event to the clients. it should be safe to
	// assume that only players of the capturing team are in the list
	CUtlVector<CBasePlayer*> capturingPlayers;

	for(unsigned int i = 0 ; i < MAX_PLAYERS ; ++i)
	{
		if( m_playerList.IsValidIndex(i) )
		{
			CBasePlayer* pPlayer = UTIL_PlayerByUserId(m_playerList[i]);
			CHajPlayer* pHAJPlayer = ToHajPlayer( pPlayer );
			CHajGameRules *pGameRules = HajGameRules();

			if( pPlayer )
			{
				if( pHAJPlayer && pGameRules && !pGameRules->IsFreeplay() )
				{
					int nearbyTeam = m_playerList.Count() - 1;

					pHAJPlayer->IncrementFragCount( 10 + nearbyTeam );
					pHAJPlayer->IncreaseObjectiveScore( 1 );
					pHAJPlayer->m_iObjectivesCapped;
					
					pHAJPlayer->SendNotification( "#HaJ_PointCapture", NOTIFICATION_BASIC, 10 + nearbyTeam );
				}

				capturingPlayers.AddToTail(pPlayer);
			}
		}
	}


	// Cap initialisers
	for( int i = 0; i < m_CapInitialisers.Count(); i++ )
	{
		CHajPlayer *pPlayer = m_CapInitialisers.Element(i);

		if( pPlayer && !capturingPlayers.HasElement( ToBasePlayer( pPlayer ) ) )
		{
			pPlayer->IncrementFragCount( 5 + pPlayer->GetNearbyTeammates() );
			pPlayer->IncreaseObjectiveScore( 1 );
			pPlayer->m_iObjectivesCapped++;

			pPlayer->SendNotification( "#HaJ_CaptureInitializer", NOTIFICATION_BASIC, 5 + pPlayer->GetNearbyTeammates() );
		}
	}

	m_CapInitialisers.RemoveAll();

	// send event to clients
	char szCapPoint[256];
	Q_snprintf( szCapPoint, 256, "%s", m_nameOfZone );
	CHajGameEvents::ControlPointCaptured(iCapturingTeam, m_ownerTeam, capturingPlayers, szCapPoint, entindex() );

	HajGameRules()->SendNotification( "#HaJ_LostPoint", NOTIFICATION_MENTIONS_OBJECTIVE, iTeamLosingCP, 0, this, STRING( GetNameOfZone() ) );
	HajGameRules()->SendNotification( "#HaJ_TeamCaptured", NOTIFICATION_MENTIONS_OBJECTIVE, iCapturingTeam, 0, this, STRING( GetNameOfZone() ) );

	m_ownerTeam = iCapturingTeam;

	// clear the "in-capture" flag
	m_capturingTeam = TEAM_INVALID;
	m_capturePercentage = 0.0f;

#ifndef CLIENT_DLL
	if( m_fExtraTime != 0.0f && HajGameRules() )
	{
		HajGameRules()->ExtendTime( m_fExtraTime );
	}
#endif
}

/////////////////////////////////////////////////////////////////////////////
int CCapturePoint::GetMinPlayersForCapture(int teamId) const
{
	if(teamId == TEAM_CWEALTH)
		return m_nMinCWealthPlayers;

	else if(teamId == TEAM_AXIS)
		return m_nMinAxisPlayers;

	return 65535;
}

/////////////////////////////////////////////////////////////////////////////
float CCapturePoint::GetTeamCaptureTime(int teamId) const
{
	if(teamId == TEAM_CWEALTH)
		return m_cwealthCapTime;

	else if(teamId == TEAM_AXIS)
		return m_axisCapTime;

	return 0.0f;
}

/////////////////////////////////////////////////////////////////////////////
void CCapturePoint::Init()
{
	BaseClass::Init();
}

/////////////////////////////////////////////////////////////////////////////
void CCapturePoint::InputEnableAxisCapture(inputdata_t& inputdata)
{
	m_bAxisCanCapture = true;
}

/////////////////////////////////////////////////////////////////////////////
void CCapturePoint::InputDisableAxisCapture(inputdata_t& inputdata)
{
	m_bAxisCanCapture = false;
}

/////////////////////////////////////////////////////////////////////////////
void CCapturePoint::InputEnableCWealthCapture(inputdata_t& inputdata)
{
	m_bCWealthCanCapture = true;
}

/////////////////////////////////////////////////////////////////////////////
void CCapturePoint::InputDisableCWealthCapture(inputdata_t& inputdata)
{
	m_bCWealthCanCapture = false;
}

/////////////////////////////////////////////////////////////////////////////
void CCapturePoint::Spawn()
{
	SetThink(&CCapturePoint::BrushThink);
	BaseClass::Spawn();
}

/////////////////////////////////////////////////////////////////////////////
void CCapturePoint::ProcessOneTeamCapture(PlayersPerTeamMap& sortedPlayersInTeam)
{
	// the first element should have the team with
	// the most players in the zone of the control point
	CUtlMap<int, int>::IndexType_t itMostPlayers = sortedPlayersInTeam.FirstInorder();
	int nPlayers = sortedPlayersInTeam.Key(itMostPlayers);
	int iTeam = sortedPlayersInTeam.Element(itMostPlayers);

	float dt = gpGlobals->curtime - m_lastThinkTime;

	// decapping (bug #98 - turns out it's not implemented)
	if( iTeam == m_ownerTeam || ( m_capturingTeam != TEAM_INVALID && CanBeCapturedByTeam(iTeam) && iTeam != m_capturingTeam && m_capturePercentage > 0.0f ) )
	{
		if( m_capturingTeam != TEAM_INVALID )
		{
			float fTimeMultiplier = 1.0f;
			float nReqTime = GetTeamCaptureTime(m_capturingTeam);

			if( nPlayers > 1 ) // we're over the required amount of people
			{
				fTimeMultiplier = 1.0f + ( nPlayers * m_fExtraPeopleCap ); // speed multiplier
			}

			// adjust timer
			m_timeLeftForCapture += dt * fTimeMultiplier;
			m_timeLeftForCapture = clamp(m_timeLeftForCapture, 0.0f, nReqTime);

			// compute percentage capture
			m_capturePercentage = 1.0f - ( m_timeLeftForCapture / nReqTime );
			m_capturePercentage = clamp(m_capturePercentage, 0.0f, 1.0f);
			
			// check if the capturing process has expired
			if(m_capturePercentage <= 0.0f)
			{
				m_CapInitialisers.RemoveAll();
				m_capturingTeam = TEAM_INVALID;
			}
		}

		return; // don't need to handle anything else
	}

	// check if the team is allowed to capture the area
	if(!CanBeCapturedByTeam(iTeam))
		return;

	if( m_capturingTeam == TEAM_INVALID )
	{
		m_timeLeftForCapture = GetTeamCaptureTime(iTeam);
	}

	if( m_capturePercentage > 0.0f && iTeam != m_capturingTeam )
		m_capturePercentage = 0.0f;

	m_capturingTeam = iTeam;

	// check if there are enough players for the capture
	int nReqPlayers = GetMinPlayersForCapture(iTeam);
	if(nPlayers >= nReqPlayers)
	{
		float nReqTime = GetTeamCaptureTime(iTeam);

		float fTimeMultiplier = 1.0f;
		int iPeopleOver = nPlayers - nReqPlayers;

		if( iPeopleOver > 0 ) // we're over the required amount of people
		{
			fTimeMultiplier = 1.0f + ( iPeopleOver * m_fExtraPeopleCap ); // speed multiplier
		}

		if( m_capturePercentage == 0.0f )
		{
			// initialize timer
			m_timeLeftForCapture = nReqTime;

			char szCapPoint[256];
			Q_snprintf( szCapPoint, 256, "%s", m_nameOfZone );

			CHajGameEvents::ControlPointUnderAttack( m_capturingTeam, GetOwnerTeamId(), szCapPoint, entindex() );

			for(unsigned int i = 0 ; i < MAX_PLAYERS ; ++i)
			{
				if( m_playerList.IsValidIndex(i) )
				{
					CHajPlayer *pPlayer = ToHajPlayer( UTIL_PlayerByUserId(m_playerList[i]) );

					if( pPlayer && pPlayer->GetTeamNumber() == m_capturingTeam )
						m_CapInitialisers.AddToTail( pPlayer );
				}
			}
		}

		// adjust timer
		m_timeLeftForCapture -= dt * fTimeMultiplier;
		m_timeLeftForCapture = clamp(m_timeLeftForCapture, 0.0f, nReqTime);

		// control point is captured if timer expires
		if(m_timeLeftForCapture <= 0.0f)
			Captured(m_capturingTeam);

		// compute percentage capture
		m_capturePercentage = 1.0f - ( m_timeLeftForCapture / nReqTime );
		m_capturePercentage = clamp(m_capturePercentage, 0.0f, 1.0f);
	}
	else
	{
		if( nPlayers < 1 )
		{
			// hold the capture progress if a team mate is in the capture volume
			int nReqPlayers	= GetMinPlayersForCapture(m_capturingTeam);
			float nReqTime	= GetTeamCaptureTime(m_capturingTeam);
			float contribPerPlayer = nReqTime / max(nReqPlayers, 1);
			float fadeTime	= contribPerPlayer * (nReqPlayers-nPlayers);

			m_timeLeftForCapture += dt * (nReqTime/fadeTime);
			m_timeLeftForCapture  = clamp(m_timeLeftForCapture, 0.0f, nReqTime);

			// compute percentage capture
			m_capturePercentage = 1.0f - (m_timeLeftForCapture / nReqTime);
			m_capturePercentage = clamp(m_capturePercentage, 0.0f, 1.0f);
		}
		
		// check if the capturing process has expired
		if( nPlayers < 1 && m_capturePercentage <= 0.0f)
			m_capturingTeam = TEAM_INVALID;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CCapturePoint::ProcessMultipleTeamCapture(PlayersPerTeamMap& sortedPlayersInTeam)
{
	// do nothing in the case that players from more than one team
	// has entered into the zone on the same frame
	if(!CHajTeam::IsCombatTeam(m_capturingTeam))
		return;

	float dt = gpGlobals->curtime - m_lastThinkTime;

	// check if any team has enough players to trigger a counter
	bool bCounterTriggered = false;

	PlayersPerTeamMap::IndexType_t it = sortedPlayersInTeam.FirstInorder();
	while(sortedPlayersInTeam.IsValidIndex(it) && !bCounterTriggered)
	{
		int iTeam = sortedPlayersInTeam.Element(it);

		if(iTeam != m_capturingTeam)
		{
			// check if team has enough players to trigger the counter
			int nTestPlayers = sortedPlayersInTeam.Key(it);
			if(nTestPlayers >= m_nCounterPlayers)
			{
				switch(m_counterStyle)
				{
				case COUNTER_STYLE_STALL:
					// do thing
					break;

				case COUNTER_STYLE_REVERSE:
					{
						m_timeLeftForCapture += dt * m_flDeCapSpeed;
						m_timeLeftForCapture = clamp(m_timeLeftForCapture, 0.0f, GetTeamCaptureTime(m_capturingTeam));

						// compute percentage capture
						float nReqTime = GetTeamCaptureTime(iTeam);
						m_capturePercentage = 1.0f - (m_timeLeftForCapture / nReqTime);
					}
					break;
				}

				bCounterTriggered = true;
			}
		}

		// try next team
		it = sortedPlayersInTeam.NextInorder(it);
	}
	
	if(!bCounterTriggered)
	{
		// Jed's idea is to have opposing players in the zone
		// offset players from the capturing team.
		// disabling behavior for now since it kinda conflicts with counter options
		ProcessOneTeamCapture(sortedPlayersInTeam);

		/*
		// get number of players in team with most players in the zone
		it = sortedPlayersInTeam.FirstInorder();
		int nPlayersTopTeam = sortedPlayersInTeam.Key(it);

		// get number of players in other teams
		int nPlayersOtherTeams = 0;
		it = sortedPlayersInTeam.NextInorder(it);
		while(sortedPlayersInTeam.IsValidIndex(it))
		{
			nPlayersOtherTeams += sortedPlayersInTeam.Key(it);
			it = sortedPlayersInTeam.NextInorder(it);
		}

		// compute number of players, if any, currently contributing
		// to the capture of the capture zone
		int nCapturingPlayers = nPlayersTopTeam - nPlayersOtherTeams;

		// compute fade time
		int nReqPlayers	= GetMinPlayersForCapture(m_capturingTeam);
		float nReqTime	= GetTeamCaptureTime(m_capturingTeam);
		float contribPerPlayer = nReqTime / max(nReqPlayers, 1);
		float fadeTime	= contribPerPlayer * (nReqPlayers-nCapturingPlayers);

		m_timeLeftForCapture += dt * (nReqTime/fadeTime);
		m_timeLeftForCapture  = clamp(m_timeLeftForCapture, 0.0f, nReqTime);

		// compute percentage capture
		m_capturePercentage = 1.0f - (m_timeLeftForCapture / nReqTime);
		m_capturePercentage = clamp(m_capturePercentage, 0.0f, 1.0f);
		
		// check if the capturing process has expired
		if(m_capturePercentage <= 0.0f)
			m_capturingTeam = TEAM_INVALID;
		*/
	}
}

/////////////////////////////////////////////////////////////////////////////
void CCapturePoint::ProcessZeroTeamCapture(PlayersPerTeamMap& sortedPlayersInTeam)
{
	// nothing to do unless a capture is in progress
	if(!CHajTeam::IsCombatTeam(m_capturingTeam))
		return;

	float dt = gpGlobals->curtime - m_lastThinkTime;

	// compute fade
	float nReqTime	= GetTeamCaptureTime(m_capturingTeam);

	m_timeLeftForCapture += dt * m_flDeCapSpeed;
	m_timeLeftForCapture  = clamp(m_timeLeftForCapture, 0.0f, nReqTime);

	// compute percentage capture
	m_capturePercentage = 1.0f - (m_timeLeftForCapture / nReqTime);
	m_capturePercentage = clamp(m_capturePercentage, 0.0f, 1.0f);
	
	// check if the capturing process has expired
	if(m_capturePercentage <= 0.0f)
	{
		m_CapInitialisers.RemoveAll();
		m_capturingTeam = TEAM_INVALID;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CCapturePoint::DoThink()
{
	// early out if the capture point is not capturable
	if(!IsCapturable())
		return;

	// sort teams by the number of players within
	// the zone of the control point.
	// key = num. of players, value = team index
	CUtlMap<int, int> sortedPlayersInTeam;
	sortedPlayersInTeam.SetLessFunc(IntLessFunc_Descending);

	for(int iTeam = 0 ; iTeam < m_playersInTeam.Count() ; ++iTeam)
	{
		int nPlayers = m_playersInTeam[iTeam];

		if(nPlayers > 0)
			sortedPlayersInTeam.Insert(nPlayers, iTeam + CHajTeam::GetFirstCombatTeamIndex());
	}

	// process capturing based on number of team with players 
	// inside the are of the capture zone
	int nTeams = sortedPlayersInTeam.Count();

	if(nTeams < 1)
		ProcessZeroTeamCapture(sortedPlayersInTeam);
	else if(nTeams == 1)
		ProcessOneTeamCapture(sortedPlayersInTeam);
	else
		ProcessMultipleTeamCapture(sortedPlayersInTeam);

	if(_bCapturePointDebug && m_capturingTeam)
	{
		DevMsg("Capturing (%d%%, %fs left)...\n",
			(int)(m_capturePercentage * 100),
			m_timeLeftForCapture);
	}
}

bool CCapturePoint::PlayerKilledOnPoint( CHajPlayer *pAttacker, CHajPlayer* pVictim, const CTakeDamageInfo &info )
{
	if( !pAttacker )
		return false;

	if( IsPlayerInList( pVictim->GetUserID() ) && !IsPlayerInList( pAttacker->GetUserID() ) && pVictim->GetTeamNumber() == GetOwnerTeamId() && CanBeCapturedByTeam( pAttacker->GetTeamNumber() ) )
	{
		pAttacker->IncrementFragCount( 1 + pAttacker->GetNearbyTeammates() );
		pAttacker->SendNotification( "#HaJ_OffensiveKill", NOTIFICATION_BASIC, 1 + pAttacker->GetNearbyTeammates() );

		return true;
	}

	return BaseClass::PlayerKilledOnPoint(pAttacker,pVictim,info);
}