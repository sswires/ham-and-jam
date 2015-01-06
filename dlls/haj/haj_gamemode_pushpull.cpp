
// haj_gamemode_pushpull.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "haj_gamemode_pushpull.h"
#include "haj_gamerules.h"
#include "haj_objectivemanager.h"
#include "haj_capturepoint.h"
#include "haj_team.h"
#include "haj_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar haj_pp_roundtime;

/////////////////////////////////////////////////////////////////////////////
LINK_ENTITY_TO_CLASS(haj_gamemode_pushpull, CHajGameModePushPull);

/////////////////////////////////////////////////////////////////////////////
// data description
BEGIN_DATADESC(CHajGameModePushPull)
	DEFINE_KEYFIELD(m_bMajorityWins,		FIELD_BOOLEAN,	"MajorityWins"), // this makes it so when the time runs out, whichever team owns the most cap points wins
	DEFINE_KEYFIELD(m_bSuddenDeath,			FIELD_BOOLEAN,	"SuddenDeathOnTimeExpire" ), // when time runs out, cut respawns and extend the round until an entire team is dead
	DEFINE_KEYFIELD(m_fOvertimeMultiplier, FIELD_FLOAT,		"OvertimeMultiplier"),
	DEFINE_THINKFUNC(OnThink)
END_DATADESC()

/////////////////////////////////////////////////////////////////////////////
void CHajGameModePushPull::OnThink()
{

	CHajGameRules* pGameRules = HajGameRules();

	// check if one team controls all the capture points
	int nCapturePoints = _objectiveman.GetNumCapturePoints();
	if(nCapturePoints > 0)
	{
		for(int i = CHajTeam::GetFirstCombatTeamIndex(); i < TEAM_LAST; ++i)
		{
			CHajTeam* pTeam = (CHajTeam*)g_Teams[i];
			if(pTeam->GetCapturePointsCount() == nCapturePoints)
			{
				PlayVictorySound(i);

				// end-condition met, so end the round
				pGameRules->TeamVictoryResetRound(i);
				return;
			}
		}
	}

	// we'll only get here if there's no winner yet, so check the round hasn't expired
	if( pGameRules->GetRoundTimeLeft() <= 0 )
	{
		if( m_bMajorityWins ) // checking team with the majority of the cap points, if enabled
		{
			int iWinningTeam = TEAM_INVALID;
			int iWinningTeamCaps = 0;

			for(int i = CHajTeam::GetFirstCombatTeamIndex(); i < TEAM_LAST; ++i)
			{
				CHajTeam* pTeam = (CHajTeam*)g_Teams[i];

				if( pTeam->GetCapturePointsCount() == iWinningTeamCaps )
					iWinningTeam = TEAM_INVALID; // this is a draw

				if(pTeam->GetCapturePointsCount() > iWinningTeamCaps )
				{
					iWinningTeam = pTeam->GetTeamNumber();
					iWinningTeamCaps = pTeam->GetCapturePointsCount();
				}
			}	

			PlayVictorySound( iWinningTeam );
			pGameRules->TeamVictoryResetRound( iWinningTeam );
			return;
		}
		else if( m_bSuddenDeath ) // sudden death mode
		{
			if( !HajGameRules()->IsInOvertime() )
			{
				HajGameRules()->StartSuddenDeath();
			}
			else
			{
				DevMsg( "CHECKING FOR DEAD PLAYERS IN SUDDEN DEATH\n");

				int iAlivePlayers[TEAM_LAST];
				iAlivePlayers[TEAM_AXIS] = 0;
				iAlivePlayers[TEAM_CWEALTH] = 0;

				// check for dead players
				for ( int i = 0; i < MAX_PLAYERS; i++ )
				{
					CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );

					if (pPlayer && pPlayer->IsAlive()) // count the alive players
						iAlivePlayers[pPlayer->GetTeamNumber()]++;
				}

				DevMsg( "ALIVE CWEALTH: %d, ALIVE AXIS: %d\n", iAlivePlayers[TEAM_AXIS],iAlivePlayers[TEAM_CWEALTH]);

				if( iAlivePlayers[TEAM_AXIS] == 0 && iAlivePlayers[TEAM_CWEALTH] == 0 )
				{
					PlayVictorySound( -1 );
					pGameRules->TeamVictoryResetRound( -1 );
					return;
				}
				else if( iAlivePlayers[TEAM_AXIS] == 0 )
				{
					PlayVictorySound( TEAM_CWEALTH );
					pGameRules->TeamVictoryResetRound( TEAM_CWEALTH );
					return;
				}
				else if( iAlivePlayers[TEAM_CWEALTH] == 0 )
				{
					PlayVictorySound( TEAM_AXIS );
					pGameRules->TeamVictoryResetRound( TEAM_AXIS );
					return;
				}
					
			}
		}
		else if( m_fOvertimeMultiplier > 0.0f && !HajGameRules()->IsInOvertime() ) // overtime
		{
			for(int i = CHajTeam::GetFirstCombatTeamIndex(); i < TEAM_LAST; ++i)
			{
				CHajTeam* pTeam = (CHajTeam*)g_Teams[i];

				if( pTeam->GetCapturePointsCount() == nCapturePoints - 1 ) // only got one left
				{
					// loop through the cap points
					const CUtlLinkedList<CCapturePoint*>& objectives = _objectiveman.GetCapturePoints();

					unsigned short it = objectives.Head();
					while(objectives.IsValidIndex(it))
					{
						CCapturePoint* pObjective = objectives.Element(it);

						int m_iTeamAttacker = pTeam->GetTeamNumber();
						int numteam_teamid = m_iTeamAttacker - CHajTeam::GetFirstCombatTeamIndex();

						if( pObjective && pObjective->GetOwnerTeamId() != m_iTeamAttacker && pObjective->GetCapturingTeam() == m_iTeamAttacker && pObjective->GetCapturePercentage() > 0.0f && pObjective->GetPlayersOnTeam( numteam_teamid ) ) // this will be the last capture point
						{
							m_fElapsedTime -= pObjective->GetTimeForCapture() * m_fOvertimeMultiplier; // add 1.5x the capture time left
							HajGameRules()->StartOvertime( pObjective->GetTimeForCapture() * m_fOvertimeMultiplier );
						}

						it = objectives.Next(it);
					}						
				}
				else
				{
					PlayVictorySound( -1 );
					pGameRules->TeamVictoryResetRound( -1 );
					return;
				}
			}			
		}
		else
		{
			PlayVictorySound( -1 );
			pGameRules->TeamVictoryResetRound( -1 );
			return;
		}
	}

	if( !ShouldFreezeTime() )
		m_fElapsedTime += gpGlobals->interval_per_tick;

	// update every frame
	SetNextThink(gpGlobals->curtime + TICK_INTERVAL);
}

// CRoundTimerOverride overrides
float CHajGameModePushPull::GetRoundTimeLeft()
{
	return ( haj_pp_roundtime.GetFloat() * 60 ) - m_fElapsedTime;
}


/////////////////////////////////////////////////////////////////////////////
void CHajGameModePushPull::Spawn()
{
	CHajGameRules* pGameRules = HajGameRules();
	pGameRules->SetRoundTimerOverride(this);
	
	pGameRules->SetGametype( HAJ_GAMEMODE_PUSHPULL );

	SetThink(&CHajGameModePushPull::OnThink);
	SetNextThink(gpGlobals->curtime + TICK_INTERVAL);

	BaseClass::Spawn();
}

void CHajGameModePushPull::Precache()
{

}

void CHajGameModePushPull::ResetRound( void )
{
	m_fElapsedTime = 0.0f;

	SetThink(&CHajGameModePushPull::OnThink);
	SetNextThink(gpGlobals->curtime + TICK_INTERVAL);
}

/////////////////////////////////////////////////////////////////////////////
