
// haj_gamemode_attackdefend.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "haj_gamemode_attackdefend.h"
#include "haj_gamerules.h"
#include "haj_objectivemanager.h"
#include "haj_capturepoint.h"
#include "haj_team.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/////////////////////////////////////////////////////////////////////////////
LINK_ENTITY_TO_CLASS(haj_gamemode_attackdefend, CHajGameModeAttackDefend);

/////////////////////////////////////////////////////////////////////////////
// data description
BEGIN_DATADESC(CHajGameModeAttackDefend)
	DEFINE_THINKFUNC(OnThink),

	DEFINE_KEYFIELD(m_iTeamAttacker,       FIELD_INTEGER,   "Attacker"),
	DEFINE_KEYFIELD(m_timelimit,           FIELD_FLOAT,     "Timelimit"),
	DEFINE_KEYFIELD(m_soundCWealthVictory, FIELD_STRING,	"CWealthVictorySound"),
	DEFINE_KEYFIELD(m_soundAxisVictory,    FIELD_STRING,	"AxisVictorySound"),
	DEFINE_KEYFIELD(m_fOvertimeMultiplier, FIELD_FLOAT,		"OvertimeMultiplier"),
END_DATADESC()

/////////////////////////////////////////////////////////////////////////////
CHajGameModeAttackDefend::CHajGameModeAttackDefend()
{
	m_timelimit = m_timer = 30.0f;
	m_iTeamAttacker = TEAM_INVALID;
	m_fOvertimeMultiplier = 1.25f;
}

/////////////////////////////////////////////////////////////////////////////
void CHajGameModeAttackDefend::Activate()
{
	BaseClass::Activate();

	m_timer = m_timelimit * 60.0f;
}

/////////////////////////////////////////////////////////////////////////////
void CHajGameModeAttackDefend::OnThink()
{
	CHajGameRules* pGameRules = HajGameRules();

	if( !pGameRules )
		return;

	// check if the timer has expired
	if( !ShouldFreezeTime() )
		m_timer -= gpGlobals->interval_per_tick;

	// check if attacking team controls all the capture points
	if(CHajTeam::IsCombatTeam(m_iTeamAttacker))
	{
		CHajTeam* pTeam = (CHajTeam*)g_Teams[m_iTeamAttacker];

		int nCPs = _objectiveman.GetNumCapturePoints();

		if(nCPs > 0)
		{
			int nTeamCPs = pTeam->GetCapturePointsCount();
			if(nCPs == nTeamCPs)
			{
				PlayVictorySound(m_iTeamAttacker);
				pGameRules->TeamVictoryResetRound(m_iTeamAttacker);
				return;
			}

			if(m_timer <= 0.0f)
			{
				// overtime logic
				const CUtlLinkedList<CCapturePoint*>& objectives = _objectiveman.GetCapturePoints();
				bool bWaitingForCap = false;

				unsigned short it = objectives.Head();
				while(objectives.IsValidIndex(it))
				{
					CCapturePoint* pObjective = objectives.Element(it);
					//int numteam_teamid = m_iTeamAttacker - CHajTeam::GetFirstCombatTeamIndex();

					if( pObjective && pObjective->GetOwnerTeamId() != m_iTeamAttacker && pObjective->GetCapturingTeam() == m_iTeamAttacker && pObjective->GetCapturePercentage() > 0.0f &&
							( nTeamCPs == nCPs - 1 || pObjective->GetExtraTimeForCap() > 0.0f ) ) // this will be the last capture point
						bWaitingForCap = true;

					it = objectives.Next(it);				
				}

				if( !bWaitingForCap )
				{
					int iDefendingTeam = TEAM_CWEALTH;

					if(m_iTeamAttacker == TEAM_CWEALTH)
					{
						iDefendingTeam = TEAM_AXIS;
					}
					else if(m_iTeamAttacker == TEAM_AXIS)
					{
						iDefendingTeam = TEAM_CWEALTH;
					}
					else
					{
						// invalid team setup
						assert(false);
					}

					PlayVictorySound(iDefendingTeam);
					pGameRules->TeamVictoryResetRound(iDefendingTeam);
					return;
				}
			}
		}
	}

	// update every frame
	SetNextThink(gpGlobals->curtime + TICK_INTERVAL);
}

/////////////////////////////////////////////////////////////////////////////
void CHajGameModeAttackDefend::Spawn()
{
	CHajGameRules* pGameRules = HajGameRules();
	pGameRules->SetRoundTimerOverride(this);

	pGameRules->SetAttackingTeam( m_iTeamAttacker );

	if( m_iTeamAttacker == TEAM_CWEALTH )
		pGameRules->SetDefendingTeam( TEAM_AXIS );
	else
		pGameRules->SetDefendingTeam( TEAM_CWEALTH );

	SetThink(&CHajGameModeAttackDefend::OnThink);
	SetNextThink(gpGlobals->curtime + TICK_INTERVAL);
	
	BaseClass::Spawn();
}


void CHajGameModeAttackDefend::ResetRound( void )
{
	m_timer = m_timelimit * 60.0f;

	SetThink(&CHajGameModeAttackDefend::OnThink);
	SetNextThink(gpGlobals->curtime + TICK_INTERVAL);
}


const char* CHajGameModeAttackDefend::GetWinString( int iTeam )
{
	if( m_iTeamAttacker == iTeam )
		return "#HaJ_Win_SecuredObjectives";

	return "#HaJ_Win_SuccessfulDefense";
}

void CHajGameModeAttackDefend::Precache()
{
}
/////////////////////////////////////////////////////////////////////////////
