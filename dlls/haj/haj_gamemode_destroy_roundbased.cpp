
// haj_gamemode_Destro_yRoundBased.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "haj_gamemode_destroy_roundbased.h"
#include "haj_gamerules.h"
#include "haj_objectivemanager.h"
#include "haj_capturepoint.h"
#include "haj_team.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/////////////////////////////////////////////////////////////////////////////
LINK_ENTITY_TO_CLASS(haj_gamemode_destroy_rb, CHajGameModeDestroyRoundBased);

/////////////////////////////////////////////////////////////////////////////
// data description
BEGIN_DATADESC(CHajGameModeDestroyRoundBased)
DEFINE_THINKFUNC(OnThink),

DEFINE_KEYFIELD( m_iTeamAttacker, FIELD_INTEGER, "Attacker" ),
END_DATADESC()

/////////////////////////////////////////////////////////////////////////////
CHajGameModeDestroyRoundBased::CHajGameModeDestroyRoundBased()
{
	m_timelimit = m_timer = haj_rbdestroy_roundtime.GetFloat();
	m_iTeamAttacker = TEAM_INVALID;
}

/////////////////////////////////////////////////////////////////////////////
void CHajGameModeDestroyRoundBased::Activate()
{
	BaseClass::Activate();

	m_timer = m_timelimit * 60.0f;
}

int CHajGameModeDestroyRoundBased::GetDefendingTeam()
{
	if( m_iTeamAttacker == TEAM_AXIS )
		return TEAM_CWEALTH;
	else if( m_iTeamAttacker == TEAM_CWEALTH )
		return TEAM_AXIS;

	return TEAM_UNASSIGNED;
}

/////////////////////////////////////////////////////////////////////////////
void CHajGameModeDestroyRoundBased::OnThink()
{
	CHajGameRules* pGameRules = HajGameRules();

	if( !pGameRules->IsFreezeTime() )
	{
		// check if the timer has expired
		m_timer -= gpGlobals->interval_per_tick;

		int iDefendingTeam = GetDefendingTeam();

		// time expired, hand victory to defenders
		if( !m_bPlanted && m_timer <= 0.0f )
		{
			PlayVictorySound( iDefendingTeam );
			pGameRules->TeamVictoryResetRound( iDefendingTeam );

			return;
		}

		// alive player check
		int iAlivePlayers[TEAM_LAST];
		iAlivePlayers[TEAM_AXIS] = 0;
		iAlivePlayers[TEAM_CWEALTH] = 0;

		int iTotalPlayers[TEAM_LAST];
		iTotalPlayers[TEAM_AXIS] = 0;
		iTotalPlayers[TEAM_CWEALTH] = 0;

		// check for dead players
		for ( int i = 0; i < MAX_PLAYERS; i++ )
		{
			CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );

			if (pPlayer )
			{
				if( pPlayer->IsAlive() ) // count the alive players
					iAlivePlayers[pPlayer->GetTeamNumber()]++;

				iTotalPlayers[pPlayer->GetTeamNumber()]++;
			}
		}	

		// if all defenders are dead, attackers win
		if( iTotalPlayers[ iDefendingTeam ] > 0 && iAlivePlayers[ iDefendingTeam ] < 1 )
		{
			PlayVictorySound( m_iTeamAttacker );
			pGameRules->TeamVictoryResetRound( m_iTeamAttacker );

			return;
		}

		// attack
		if( !m_bPlanted && iTotalPlayers[ m_iTeamAttacker ] > 0 && iTotalPlayers[ iDefendingTeam ] > 0 && iAlivePlayers[ m_iTeamAttacker ] < 1 )
		{
			PlayVictorySound( iDefendingTeam );
			pGameRules->TeamVictoryResetRound( iDefendingTeam );

			return;
		}

		// if all attackers are dead and explosives are not planted, defenders win

	}

	// update every frame
	SetNextThink(gpGlobals->curtime + TICK_INTERVAL);

}

/////////////////////////////////////////////////////////////////////////////
void CHajGameModeDestroyRoundBased::Spawn()
{
	CHajGameRules* pGameRules = HajGameRules();
	pGameRules->SetRoundTimerOverride(this);
	pGameRules->SetGamemode( this );

	pGameRules->SetGametype( HAJ_GAMEMODE_ATTACKDEFEND );
	pGameRules->SetAttackingTeam( m_iTeamAttacker );

	if( m_iTeamAttacker == TEAM_CWEALTH )
		pGameRules->SetDefendingTeam( TEAM_AXIS );
	else
		pGameRules->SetDefendingTeam( TEAM_CWEALTH );

	SetThink(&CHajGameModeDestroyRoundBased::OnThink);
	SetNextThink(gpGlobals->curtime + TICK_INTERVAL);

	m_bPlanted = false;
}

void CHajGameModeDestroyRoundBased::OnBombPlant( CBasePlayer *pPlanter, CBombZone* pZone )
{
	m_timer = haj_rbdestroy_bombtime.GetFloat();
	m_bPlanted = true;
}

void CHajGameModeDestroyRoundBased::OnBombDefuse( CBasePlayer *pDefuser, CBombZone* pZone )
{
	m_bPlanted = false;

	PlayVictorySound( GetDefendingTeam() );
	HajGameRules()->TeamVictoryResetRound( GetDefendingTeam() );

	SetThink(NULL);
}

void CHajGameModeDestroyRoundBased::OnBombExplode( CBombZone* pZone )
{
	m_bPlanted = false;

	PlayVictorySound( m_iTeamAttacker );
	HajGameRules()->TeamVictoryResetRound( m_iTeamAttacker );

	SetThink(NULL);
}

bool CHajGameModeDestroyRoundBased::CanPlantBomb( CBasePlayer *pPlanter, CBombZone *pZone /*= NULL */ )
{
	if( !pZone || m_bPlanted ) return false;
	return true;
}

void CHajGameModeDestroyRoundBased::ResetRound( void )
{
	SetThink(&CHajGameModeDestroyRoundBased::OnThink);
	SetNextThink(gpGlobals->curtime + TICK_INTERVAL);

	m_timer = haj_rbdestroy_roundtime.GetFloat() * 60;
	m_bPlanted = false;

}
/////////////////////////////////////////////////////////////////////////////
